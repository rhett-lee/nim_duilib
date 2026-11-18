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
            └─ ThreadManager::PostTask(kThreadImage1~4 轮询 / kThreadWorker 兜底)
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
   已实现：单次任务最多解码 `kAsyncDecodeBatchSize`（3）帧，且受预解码窗口限制，详见 6.6 节。
2. **大文件 I/O 仍在 UI 线程**：JPEG/WEBP 在文件超过阈值（128KB）时于加载阶段把整个文件读入内存，
   而 PNG/GIF 是把读盘推迟到工作线程的。异步解码只异步了"解码"，没有异步"读盘"。
   已实现：JPEG 与静态 WEBP 在加载阶段只读 4KB 文件头解析宽高，完整文件数据与解码句柄推迟到解码线程准备，
   详见 6.5 节。
3. **帧就绪不能立即推进播放**：`PlayingImageAnimation` 在下一帧未就绪时直接返回，需要等一个定时器周期后重试。
   已实现：解码完成通知中回调 `Image::OnImageFrameDataReady()`，播放等待缺帧时立即推进，详见 6.6 节。
4. **单帧图片的缩放留在 UI 线程**：`ImageInfo::GetBitmap()` 中的 `ResizeImageBitmap()` 发生在绘制路径上。
5. **延迟释放队列的定时任务重复投递**：每次 `ReleaseImage()` 都投递一个延迟任务，整体复杂度 O(N²)；
   引用计数为 1 时可以直接释放。
6. **延迟绘制列表在绘制热路径上是 O(n)**：`AddDelayPaintData()` 每次先全表扫描去重，动画每帧重绘都会走一次。
7. **解码线程数量少**：`kThreadImage1` / `kThreadImage2` 各只有一个线程，一张超大图会阻塞同线程上的其它图片。
   已按"可配置线程数（2~4）+ 轮询选择"的方案实现，详见第 6 节。
8. **部分解码器完全没走异步**：`ImageDecoder_Common`、ICO/Icon 仍在加载阶段整体解码，属于 UI 线程上的同步耗时。
   已实现：`ImageDecoder_Common`（BMP/DIB 等）与单帧 ICO 改为"加载阶段只解析文件头 + 解码线程中执行像素解码"，
   详见 6.4 节；`ImageDecoder_Icon` 的数据本身就是已解码的系统图标像素，继续走同步路径。

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
- 优化点 8：新增 `Image_DelayBitmap`；`ImageDecoder_Common` 与单帧 `ImageDecoder_ICO` 改为
  "加载阶段只解析文件头 + 解码线程执行像素解码"，详见 6.4 节。
- 优化点 2：`Image_JPEG` 与 `Image_WEBP` 在加载阶段只读 4KB 文件头解析宽高，
  完整的文件数据与解码句柄推迟到解码线程准备，详见 6.5 节。
- 优化点 1：单次解码任务解码多帧（批量大小 3，且受预解码窗口限制），详见 6.6 节。
- 优化点 3：新增 `Image::OnImageFrameDataReady()` / `ImagePlayer::OnImageFrameDataReady()`，
  解码完成通知中立即推进因缺帧而等待的播放，详见 6.6 节。

## 6. 图片解码线程数量与选择策略

针对优化点 7（解码线程数量少，一张超大图会阻塞同线程上的其它图片），采用"可配置线程数 + 轮询选择"的方案。

### 6.1 线程数量

- 默认按 CPU 核数计算：`核数 / 2 - 1`，并限制在 2~4 之间；获取不到核数时取 2。

| CPU 核数 | 默认解码线程数 |
| -------- | -------------- |
| 4 核 | 2 |
| 6~8 核 | 3 |
| ≥12 核 | 4（上限，不再增加） |

- 上限取 4 的原因：解码是纯 CPU 密集型任务，线程数超过可用核数不会提升吞吐量，只会互相抢占 CPU；
  同时每并发解码一张图就多一份 RGBA 缓冲（3840×2160 约 33MB），并发数越高内存峰值越大。
- 对外接口：
  - `GlobalManager::SetImageDecodeThreadCount(int32_t)`：配置线程数量，取值越界时按 2/4 边界处理。
    `Startup()` 之前调用时，启动阶段按该数量创建线程；`Startup()` 之后调用时立即启动/停止线程使数量一致（需在 UI 线程调用）。
  - `GlobalManager::GetImageDecodeThreadCount()`：获取当前线程数量。
- 线程标识号为 `kThreadImage1` ~ `kThreadImage4`，`StartInnerThread()` / `StopInnerThread()` 均支持。

### 6.2 选择策略

- `GlobalManager::GetNextImageDecodeThread()` 采用轮询：每次取下一个可用线程，跳过未启动的线程；
  全部不可用时返回 `kThreadNone`。
- `Control::AsyncDecodeImageData()` 不再硬编码 `{Image2, Image1, Worker}`，改为调用上述接口获取线程；
  取不到时依次回退到 `kThreadWorker`、UI 线程（兜底行为与原来一致）。

### 6.3 后续可继续优化的方向

- 大图（像素数超过阈值）单独走一条解码通道，避免 3~4 张大图同时占满全部解码线程。
- 按可见区域排优先级（可见控件优先、已滚出视图的延后），减少无效解码。
- 解码阶段直接降采样（JPEG 已支持，PNG/WEBP 尚未），可同时降低 CPU 占用与内存峰值。

### 6.4 加载阶段不解码的解码器改造（优化点 8）

新增 `Image_DelayBitmap`（`duilib/Image/Image_DelayBitmap.h/.cpp`）：单帧位图的延迟解码实现，
加载阶段只记录宽度、高度、缩放比例和解码函数，像素解码在 `DelayDecode()` 中由解码线程执行，
解码结果通过 `MergeDelayDecodeData()` 合入 UI 侧，完整复用 E1~E3 的并发保护机制。

- **ImageDecoder_Common（BMP/DIB 等）**：新增 `ImageDecoderUtil::GetImageSizeFromFile/GetImageSizeFromMemory`，
  用 `stbi_info` 只解析文件头得到宽高；宽高解析成功时用 `Image_DelayBitmap` 延迟解码，
  解析失败则回退到原有的同步解码路径。
- **ImageDecoder_ICO（单帧 ICO/CUR）**：新增 `ImageDecoderUtil::GetIcoImageInfo()`（解析 ICO 目录，只读文件头）
  与 `SelectIcoImageInfo()`（按"最接近尺寸 + 颜色数 0/256/16/2 优先级"选择图标项，规则与解码后的选择一致）。
  加载阶段据此得到宽高与缩放比例，像素解码在解码线程中通过 `LoadIcoFromMemory()` 执行。
  `bIconAsAnimation == true`（多帧 ICO 作为动画）时仍走原有的同步路径。
- **ImageDecoder_Icon（系统图标）**：数据本身已是解码后的像素数据（HICON 转换结果，尺寸很小），
  继续走同步路径，避免引入额外的跨线程开销。
- 性能提示：单帧 ICO 在异步路径中会先把文件读入内存以解析目录信息（仅一次读取，内存由延迟解码对象持有），
  真正的 `CxImage` 逐像素解码与数据翻转已全部移到解码线程。

### 6.5 大文件延迟读盘（优化点 2）

改造目标：加载阶段（UI 线程）不再读取大文件的完整数据，只读取文件头得到布局所需的宽高；
完整文件数据与解码句柄都在解码线程中原地补齐。

背景约束：`ImageManager` 在文件大于 128KB 时不会预读文件内容（`m_pFileData == nullptr`），
只有路径可用。而 `IBitmapImage::GetWidth()/GetHeight()` 在加载阶段就要被读取（`ImageInfo` 用它计算布局尺寸），
因此头部解析必须能给出准确的宽高。

- **Image_JPEG**
  - 加载阶段：`bAsyncDecode == true` 时调用 `FileUtil::ReadFileHeaderData(path, 4 * 1024, headerData)`，
    新增的静态函数 `GetImageSizeFromHeader()` 校验 SOI（`0xFFD8`）后逐段扫描 JPEG 段标记，
    跳过无长度字段的标记（`D8/D9/01/D0~D7`），在 SOF 段（`0xC0~0xCF` 且排除 `C4/C8/CC`）中读取高度与宽度。
    随后用 `CalcImageSize()`（由原加载逻辑抽出）按缩放比例与目标区域计算尺寸，宽高有效即返回成功。
  - 解析失败（头部不足 4KB、无 SOF 段、宽高非法）时回退到原有的"读取完整文件"路径，行为与改造前一致。
  - 解码线程：新增 `PrepareDecodeData()`，在 `m_fileData` 为空时按 `m_imageFilePath` 读取完整文件，
    并在 `m_tjInstance == nullptr` 时调用 `tjInitDecompress()` 懒创建句柄；
    `DecodeBitmap()`（改为非 `const`）一进入即调用它，失败直接返回 `nullptr`。
  - `IsDelayDecodeEnabled()` / `DelayDecode()` 的可解码条件由 `m_tjInstance != nullptr` 改为
    `!m_fileData.empty() || !m_imageFilePath.IsEmpty()`，不再依赖加载阶段已建好句柄。
- **Image_WEBP**
  - 加载阶段：新增 `LoadImageInfoFromFileHeader()`，用 4KB 头部构造 `WebPDemuxPartial()`，
    当解复用状态超过 `WEBP_DEMUX_PARSING_HEADER` 时读取 `WEBP_FF_FORMAT_FLAGS / CANVAS_WIDTH / CANVAS_HEIGHT`。
  - **仅对静态图延迟**：动画 WEBP 的帧数只有解析完整文件后才能得到，因此判断 `(formatFlags & ANIMATION_FLAG) == 0`
    且宽高有效时才设 `m_nFrameCount = 1`、记录路径并返回成功；动画 WEBP 仍走原有"读取完整文件 + 建解码器"的路径。
  - 解码线程：新增 `PrepareDecodeData()`，在文件数据为空时按路径读全文件、`m_pWebPAnimDecoder == nullptr` 时
    设置 `m_webpData` 并 `WebPAnimDecoderNew()`（Windows 用 `MODE_bgrA`，其他平台 `MODE_rgbA`，`use_threads = 1`），
    随后用 `WebPAnimDecoderGetInfo()` 回填帧数与循环次数；`DelayDecode()` 在置位解码状态后立即调用它，失败按解码错误处理。
  - `IsDelayDecodeEnabled()` 的条件改为
    `m_bAsyncDecode && (!m_fileData.empty() || !m_imageFilePath.IsEmpty()) && (m_nFrameCount > 0) && !m_bDecodeError`。
- 统一前提：只有 `bAsyncDecode == true` 时才走延迟读盘。无异步解码管道时，`DelayDecode()` 不会被调用，
  若在加载阶段不读数据会导致 `GetBitmap()` 永久为空，因此必须保留同步回退。

### 6.6 批量解码与帧就绪立即推进播放（优化点 1、3）

#### 批量解码（优化点 1）

- 新增常量 `kAsyncDecodeBatchSize = 3`：`Control::AsyncDecodeImageData()` 计算本次任务的目标帧索引时，
  由原来的"已解码帧数"（每次只解 1 帧）改为"已解码帧数 + 3"（上限为总帧数），一次任务解码多帧。
- 目标帧索引仍受预解码窗口 `kAsyncDecodeLookahead`（3）限制：
  `nDecodeEnd = min(已解码帧数 + 批量大小, 播放位置 + 预解码窗口, 总帧数)`，
  当窗口已满（`nDecodeEnd <= 已解码帧数`）时直接暂停解码并清除任务 ID，等播放推进后再继续。
- 解码器无需改动：`Image_PNG` / `Image_GIF` / `Image_WEBP` 的 `DelayDecode(nMinFrameIndex, ...)`
  本身就是"循环解码直到已解码帧数超过 `nMinFrameIndex`"，传入更大的目标索引即可批量解码；
  `Image_JPEG`、`Image_DelayBitmap` 等单帧图片忽略该参数，行为不变。
- 通知次数：每批解码完成后只投递一次 `kThreadUI` 通知，UI 线程的唤醒次数与跨线程调度次数降为约 1/3。
- 移除 `TAsyncImageDecode::m_nDecodeCount`（按任务计数，批量后不再等于帧数）及其相关断言，
  解码进度统一通过 `IImageDelayDecode::GetDecodedFrameIndex()` 与 `IsAsyncDecodeFinished()` 获取。

#### 帧就绪立即推进播放（优化点 3）

- `ImagePlayer` 新增成员 `m_bWaitingFrameData`：`PlayingImageAnimation()` 在"当前帧未就绪"或
  "下一帧未就绪"而返回时置位；成功推进到下一帧、`StartImageAnimation()`、`StopImageAnimation()`、
  `SetImage()` 时清除。
- 新增 `ImagePlayer::OnImageFrameDataReady()`：仅当 `m_bWaitingFrameData == true`（说明播放确实因缺帧而停住）
  且动画正在播放时，直接调用 `PlayingImageAnimation()` 立即推进；未处于等待状态时不做任何事，
  避免解码完成通知打乱正常定时器的播放节奏（例如动画刚开始时提前切换帧）。
- 新增 `Image::OnImageFrameDataReady()` 转发到内部 `ImagePlayer`；
  `Control::AsyncDecodeImageData()` 的解码完成通知在 `MergeAsyncDecodeData()` 与 `DelayPaintImage()` 之后调用它。
- 效果：此前缺帧时需等一个完整定时器周期才重试，最坏情况下帧间隔接近两个周期；
  现在数据一就绪即推进，动画不再出现卡顿式停顿。
