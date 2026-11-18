# Image 图片异步解码流程分析报告

## 1. 范围与结论

分析对象：`duilib/Image` 目录下的图片加载与解码流程，重点是多线程（异步）解码链路，以及它与
`Control`（绘制调度）、`ImageManager`（缓存/延迟释放/延迟绘制）、`ImagePlayer`（动画播放）的交互。

总体结论：主流程是通的，但存在若干真实缺陷，其中两个会直接导致画面异常或稳定性问题：

| 编号 | 严重程度 | 问题 | 状态 |
| ---- | -------- | ---- | ---- |
| E1 | 高 | `Image_JPEG` 异步状态无并发保护（跨线程读写 `shared_ptr`） | 已修复 |
| E2 | 高 | `async_load`（Image 级）与实际异步状态（ImageData 级）不一致，导致图片永久空白 | 已修复 |
| E3 | 中 | UI 线程合并帧数据与工作线程启动之间存在窗口竞争 | 已修复 |
| E4 | 中 | 动画被全量预解码、无回压、无法取消，长动画内存失控 | 已补充预解码窗口 |
| E5 | 低 | GIF 工作线程调用访问 UI 侧帧数据的接口 | 已修复 |
| E6 | 低 | PNG 解码被中止时会误触发断言 | 已修复 |

## 2. 异步解码链路（现状）

```
Control::PaintImage
  └─ Control::LoadImageInfo (UI 线程)
       ├─ ImageManager::GetImage            // UI 线程：解析文件头/签名，建立 ImageData 缓存
       └─ Control::AsyncDecodeImageData     // UI 线程：调度解码任务
            └─ ThreadManager::PostTask(kThreadImage1 / kThreadImage2 / kThreadWorker)
                 └─ IImage::AsyncDecode → Image_XXX::DelayDecode   // 工作线程：逐帧解码
                      └─ PostTask(UI) 完成通知
                           ├─ MergeAsyncDecodeData     // UI 线程：把解码结果并入帧列表
                           ├─ DelayPaintImage(imageKey) // 触发相关控件重绘
                           └─ 满足条件则投递下一帧任务
```

关键参与者：

- `Image_PNG` / `Image_GIF` / `Image_WEBP` / `Image_JPEG`：具体解码实现（`IImageDelayDecode`）。
- `Image_Animation` / `Image_Bitmap`：对上面解码实现的包装（`IImageAsyncDecode`），也负责单帧图片的承载。
- `ImageInfo`：图片信息缓存，持有 `IImage`，负责帧读取、缓存与资源释放。
- `ImageManager`：`ImageData`/`ImageInfo` 两级缓存、延迟释放队列、延迟绘制列表。
- `ImagePlayer`：动画播放（定时器、帧索引推进）。

## 3. 真实错误

### E1【高】JPEG 异步解码状态没有并发保护

`Image_JPEG::IsDelayDecodeEnabled()` / `IsDelayDecodeFinished()` 直接读写 `m_pDelayBitmap`，
而该成员由工作线程在 `DelayDecode()` 中赋值。PNG/GIF/WEBP 都有 `std::atomic<bool> m_bAsyncDecoding`
用于隔离两个线程，JPEG 唯独没有。对 `std::shared_ptr` 成员的跨线程并发读写属于未定义行为，
可能读到脏值，严重时控制块引用计数错乱导致崩溃。

触发场景：同一张 JPEG 被第二个控件加载时，UI 线程会调用 `IsAsyncDecodeFinished()`，此时第一个控件的解码任务可能正在工作线程中执行。

修复：为 `Image_JPEG` 增加 `std::atomic<bool> m_bAsyncDecoding`，并在
`IsDelayDecodeEnabled()` / `IsDelayDecodeFinished()` 中先判断该状态。

### E2【高】`async_load` 是 Image 级属性，异步状态却是 ImageData 级

`ImageLoadParam::GetLoadKey()` 不包含异步解码标志，因此同一个图片文件被多个控件使用时，
共享的 `ImageData` 采用"先加载者"的 `m_bAsyncDecode`。而绘制侧判断"是否需要等待异步解码"
使用的是**当前控件自己的** `ImageAttribute::m_bAsyncLoad`：

```cpp
// 修复前（Control::PaintImage）
if (duiImage.GetImageAttribute().m_bAsyncLoad) {
    bDataPending = true;
}
```

当 A 控件以 `async_load="true"` 先加载、B 控件以 `async_load="false"` 复用同一 `ImageData` 时：
B 拿到空位图，既不会标记解码错误，也不会登记延迟绘制，解码完成后 `DelayPaintImage()` 也刷不到 B，
于是 B 一直空白，直到其它原因触发重绘（Debug 下还会命中断言）。

修复：绘制阶段不再依赖控件侧的 `m_bAsyncLoad`；只要"位图为空且没有解码错误"，就按等待异步解码处理，
并登记延迟绘制。

### E3【中】UI 线程合并与工作线程启动之间存在窗口竞争

`m_bAsyncDecoding` 原先是在工作线程的 `DelayDecode()` 内部才置位的。在置位之前，UI 线程若正好重绘
并走到 `MergeDelayDecodeData()`，会读到 `false` 然后访问 `m_delayFrames`，而工作线程紧接着
`push_back()`，两者与 `std::vector` 的扩容并发。

修复：把"开始解码"的置位提前到 UI 线程提交任务之前（新增 `IImageDelayDecode::SetAsyncDecoding()`），
工作线程结束时再清除。这样 UI 线程在任务在途期间永远不会读取 `m_delayFrames`。

### E4【中】动画被全量预解码，没有回压，也无法取消

完成通知里只要"未解码完成"就继续投递下一帧，没有播放位置限制；`m_frames` 解码后永不释放。
由于 `ImageManager` 默认 `m_bImageAsyncLoad = true`，一个几百帧的 GIF/APNG 会在一秒内把
**所有帧**解码出来常驻内存（同步解码时是按需解码到当前帧的）。

此外取消只依赖 `use_count() == 1` 这一启发式判断；控件换图/销毁后，只要 `ImageData` 还在缓存或
延迟释放队列中，解码链就会继续跑完，且从不取消线程任务。

修复：引入预解码窗口——只保持播放位置前方若干帧（`kAsyncDecodeLookahead`）的预解码；
播放推进到窗口边缘时，由 `ImagePlayer` 主动请求继续解码。窗口关闭时静默停止，不触发解码完成/错误事件。

### E5【低】GIF 工作线程调用了访问 UI 侧数据的接口

`Image_GIF::DelayDecode()` 在工作线程调用 `GetFrameDelayMs()`，其内部会读取 UI 线程合并的
`m_frames`。虽然逻辑上不会命中该分支，但 `size()` 与 `push_back()` 并发仍是竞态。
建议直接取 `m_framesDelayMs`。

### E6【低】PNG 中止解码会误触发断言

`Image_PNG::DelayDecode()` 中 `bDecodeResult` 初值为 `false`；若循环一次都没进入（进入时即被
`IsAborted()` 中止），末尾的 `ASSERT(m_bDecodeError)` 必然失败。应区分"被取消"与"真出错"。

## 4. 可优化点

1. **每帧一次 UI 往返、且一次只解一帧**：N 帧动画需要 2N 次跨线程调度。改为一次任务解 k 帧 + 一次通知，
   可显著降低调度与 UI 唤醒开销。
2. **大文件 I/O 仍在 UI 线程**：JPEG/WEBP 在文件超过阈值（128KB）时于加载阶段把整个文件读入内存，
   而 PNG/GIF 是把读盘推迟到工作线程的。异步解码只异步了"解码"，没有异步"读盘"。
3. **帧就绪不能立即推进播放**：`PlayingImageAnimation` 在下一帧未就绪时直接返回，需要等一个定时器周期后重试。
4. **单帧图片的缩放留在 UI 线程**：`ImageInfo::GetBitmap()` 中的 `ResizeImageBitmap()` 发生在绘制路径上。
5. **延迟释放队列的定时任务重复投递**：每次 `ReleaseImage()` 都投递一个延迟任务，整体复杂度 O(N²)；
   引用计数为 1 时可以直接释放。
6. **延迟绘制列表在绘制热路径上是 O(n)**：`AddDelayPaintData()` 每次先全表扫描去重，动画每帧重绘都会走一次。
7. **解码线程数量少**：`kThreadImage1` / `kThreadImage2` 各只有一个线程，一张超大图会阻塞同线程上的其它图片。
8. **部分解码器完全没走异步**：`ImageDecoder_Common`、ICO/Icon 仍在加载阶段整体解码，属于 UI 线程上的同步耗时。

## 5. 修复说明

- E1：`Image_JPEG` 增加原子状态，并调整 `IsDelayDecodeEnabled()` / `IsDelayDecodeFinished()` 的判断顺序。
- E2：`Control::PaintImage()` 改为按"空位图且无错误"判定等待异步解码，不再使用 `m_bAsyncLoad`。
- E3：新增 `IImageDelayDecode::SetAsyncDecoding()` / `IImageAsyncDecode::SetAsyncDecoding()`，
  UI 线程在投递任务前置位（投递失败时回滚），工作线程解码结束时清除，UI 线程在处理完成通知时再兜底清除一次；
  PNG/GIF/WEBP/JPEG 移除原有的"重入即返回"判断。
- E4：`Control::AsyncDecodeImageData()` 的续解判断增加预解码窗口；新增
  `Control::RequestAsyncDecodeImage()`，由 `ImagePlayer` 在帧未就绪时请求续解。
- E5：`Image_GIF::DelayDecode()` 不再调用 `GetFrameDelayMs()`，改为直接读取加载阶段写入的
  `m_framesDelayMs`，避免工作线程访问 UI 线程合并的 `m_frames`。
- E6：`Image_PNG::DelayDecode()` 在未解出帧时先判断是否被 `IsAborted()` 中止，仅在确属解码错误时才断言。
