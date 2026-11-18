# Image 目录图片加载代码分析报告

分析范围：`duilib/Image` 目录（约 60 个文件），涵盖图片加载主链路、各格式解码器、动画播放、状态图与图片属性解析。

以下问题均基于实际代码核对，按严重程度排列。

## 修复状态总览

| 序号 | 问题 | 状态 |
|:---|:---|:---|
| 1 | `ResizeImageBitmap` 源位图未解锁 | 已修复 |
| 2 | 重采样写死预乘 Alpha | 已修复 |
| 3 | SVG 解码分支除零 | 已修复 |
| 4 | SVG 拉伸判断用 `\|\|` | 已修复 |
| 5 | `SetImageData` 未清理旧缓存 | 已修复 |
| 6 | `Image` 析构期间回调控件 | 已修复 |
| 7 | SVG 位图缓存 | 已优化（单槽位改为 LRU） |
| 8 | 图片缓存 Key 拼接冲突 | 已修复 |
| 9 | 偏移缩放用 `ceil` | 已修复 |
| 10 | DPI 变体图静默消失 | 已修复 |
| 11 | `ModifyAttribute` 重置 DPI 开关 | 已修复 |
| 12 | 未知属性仅在 Debug 断言 | 已修复 |
| 13 | 停止播放关闭自动播放 | 已复核：属设计需要，保留原行为并补充注释 |
| 14 | 更换图片未重置播放状态 | 已修复 |
| 15 | 异步解码期间重复分配帧对象 | 已修复 |
| 16 | 每次格式探测都拆分字符串 | 已修复 |

---

## 一、高风险（真实缺陷）

### 1. `ResizeImageBitmap` 源位图未解锁（句柄状态失衡）— 已修复

位置：`duilib/Image/ImageUtil.cpp`（`ResizeImageBitmap`）

```cpp
void* pPixelBits = pBitmap->LockPixelBits();   // 源位图加锁
...
if (ResizeImageData(...)) {
    pNewBitmap->UnLockPixelBits();             // 只解锁了新位图
}
else {
    pNewBitmap.reset();                        // 失败分支，两个都没解锁
}
```

源位图在多条路径上都缺少 `UnLockPixelBits()`。Skia/GDI 位图长期处于映射状态会导致后续 `LockPixelBits` 失败或绘制性能退化。

修复：重采样结果先保存到 `bResized`，无论成功与否都先释放源位图的像素锁，再按结果处理新位图。

### 2. 重采样写死为预乘 Alpha，非预乘位图会被算错 — 已修复

位置：`duilib/Image/ImageUtil.cpp`（`ResizeImageData`）固定使用 `STBIR_RGBA_PM`；调用方 `duilib/RenderSkia/Bitmap_Skia.cpp`（`Bitmap_Skia::Init`）是按传入的 `alphaType` 建位图的，且确实存在 `kUnpremul`（如 `duilib/Control/ColorSlider.cpp`）。

非预乘数据被当作预乘做双线性插值，会出现彩色边缘、半透明区域发黑/发灰——这是典型观感问题。

修复：两个 `ResizeImageData` 重载新增 `AlphaType` 参数，`kUnpremul` 使用 `STBIR_RGBA`，其余保持 `STBIR_RGBA_PM`；`ResizeImageBitmap` 读取源位图的 Alpha 类型并让新位图保持一致，`Bitmap_Skia::Init` 传入自身的 `alphaType`。

### 3. SVG 解码分支存在除零/NaN — 已修复

位置：`duilib/Image/ImageDecoderFactory.cpp`（`DecodeImageData`）

```cpp
if ((cx > 0) && (pImage->GetWidth() > 0)) { ... }          // X 有保护
if (cy > 0) { fRealScaleY = cy / pImage->GetHeight(); }   // Y 没有保护
```

高度为 0 时得到 `inf`，随后 `(int32_t)std::round(inf)` 是未定义行为。

修复：Y 轴补充 `pImage->GetHeight() > 0` 判断，与 X 轴逻辑保持一致。

### 4. SVG 拉伸判断用 `||`，图标会被拉伸变糊 — 已修复

位置：`duilib/Image/Image.cpp`（`Image::GetCurrentBitmap`）

```cpp
else if ((rcDest.Width() == rcSource.Width()) ||
         (rcDest.Height() == rcSource.Height())) {
    return GetBitmapData(...);   // 不按目标尺寸重新栅格化
}
```

只要有一个维度相等就放弃矢量缩放。判断条件应为 `&&`。

修复：条件改为 `&&`（只有宽高都相同才跳过矢量缩放）。同时把 `ImageInfo::GetSvgBitmap(rcDest, ...)` 的栅格化比例由 `min(scaleX, scaleY)` 改为 `max`，否则单轴拉伸时仍会按较小比例栅格化、再被放大绘制，模糊问题并未真正解决。

注意：`max` 策略在极端宽高比拉伸（如细长条 SVG 铺满大区域）时会产生较大的临时位图，这是保证清晰度的代价；九宫格绘制的路径不经过此分支。

### 5. `ImageInfo::SetImageData` 未清理旧位图缓存 — 已修复

位置：`duilib/Image/ImageInfo.cpp`（`ImageInfo::SetImageData`）

该方法只重置了 `m_pLastFrame`，没有 `m_pBitmap.reset()`，也没有对旧的 `m_pImageData` 走 `ReleaseImage()`（会丢失延迟释放登记）。当前 `ImageManager` 每次新建 `ImageInfo`，所以暂时不触发；一旦复用对象就会返回尺寸错误的旧位图。

修复：在更新 `m_loadParam` 之前调用 `ReleaseImage()`（该方法依赖旧的加载路径、且完成延迟释放登记），并清除 `m_pBitmap`、`m_pLastFrame`、`m_pPendingFrame`。

### 6. `Image` 析构期间向 `Control` 发绘制请求 — 已修复

位置：`duilib/Image/Image.cpp`（`Image::~Image`）→ `duilib/Image/ImagePlayer.cpp`（`StopImageAnimation`）→ `RedrawImage()` → `m_pControl->InvalidateRect()`。

`Image` 通常是 `Control` 的成员，`Control` 析构时 `Image` 析构会回调一个半销毁的 `Control`，属于悬垂/重入风险。

修复：析构时先调用 `m_pImagePlayer->SetControl(nullptr)` 解除关联，再停止动画；定时器照常取消、帧索引照常复位，但不再触发重绘。

---

## 二、中风险（性能与健壮性）

### 7. SVG 位图缓存 — 已优化

位置：`duilib/Image/ImageInfo.cpp`（`ImageInfo::GetSvgBitmap`）与 `duilib/Image/ImageDecoder_SVG.cpp`（`SvgImageImpl::GetBitmap`）。

**复核更正**：`SvgImageImpl` 原本已有一个单槽位缓存（按尺寸命中），并非完全无缓存。真正的瓶颈是同一张 SVG 在正常/悬停/按下、不同 DPI、布局尺寸微调等场景下会在几个尺寸间来回切换，单槽位缓存每次都被覆盖，导致反复栅格化。

修复：把单槽位缓存升级为按 `(宽, 高)` 的 3 项 LRU（`std::list` + `splice`，命中/淘汰均为常数复杂度）；颜色替换内容变化、DOM 重新加载时清空全部缓存；栅格化失败时不再把空位图写入缓存。

### 8. 图片缓存 Key 用文件名字符拼接，存在冲突与重复计算 — 已修复（冲突）

位置：`duilib/Image/ImageLoadParam.cpp`（`ImageLoadParam::GetLoadKey`）用 `@` `#` `$` `&` 作为分隔符，而这些字符在合法文件名中也会出现（如 `icon#1.png`），可能产生 key 冲突。

修复：变长字段（图片路径、`srcWidth`、`srcHeight`、`svgReplaceColors`）改为「长度:内容」的格式拼接，消除分隔符带来的歧义。

关于「重复计算」：复核后 `GetLoadKey` 的调用点只有 `Control.cpp` 的属性更新路径和 `ImageManager` 的加载/查找路径，并不在逐帧绘制路径上，因此没有为它增加缓存成员，避免引入可变状态与失效逻辑。

### 9. 注释与实现不一致：偏移缩放用的是 `ceil` 而非四舍五入 — 已修复

位置：`duilib/Image/ImageUtil.cpp`（`ImageUtil::GetScaledImageOffset`）

```cpp
//计算后，进行四舍五入
nImageOffset = static_cast<int32_t>(std::ceil(nImageOffset * fImageSizeScale));
```

`ceil` 对负数向 0 取整，导致左右/上下偏移不对称，DPI 非整数倍（如 125%、150%）下出现 1px 抖动。

修复：改用 `std::lround`。

### 10. 禁用 DPI 缩放 + DPI 变体图时，图片静默消失 — 已修复

位置：`duilib/Image/ImageInfo.cpp`（`ImageInfo::CalcImageInfoSize`）：Release 下直接 `return`（宽高为 0），整张图不显示。

修复：保留 `ASSERT` 作为排查提示，但不再提前返回，直接按图片文件的实际像素尺寸使用。

### 11. `ModifyAttribute` 每次重置 DPI 相关开关 — 已修复

位置：`duilib/Image/ImageAttribute.cpp`（`ImageAttribute::ModifyAttribute`）：`Image::UpdateImageAttribute` 只传部分属性时（`duilib/Image/Image.cpp`），之前设置的 `dpi_scale="false"`、`dest_scale="false"` 会被重置为 `true`。

修复：移除 `ModifyAttribute` 中的重置，默认值只由 `ImageAttribute::Init()` 负责。

### 12. 未知属性仅在 Debug 断言 — 已修复

位置：`duilib/Image/ImageAttribute.cpp`（`ImageAttribute::ModifyAttribute` 末尾）：XML 里拼错属性名，Release 版本完全静默。

修复：输出一条包含图片名、属性名与属性值的日志（`LogUtil::OutputLine`，Release 版本同样生效），保留原有断言。

### 13. 停止播放会永久关闭自动播放 — 复核后保留原行为

位置：`duilib/Image/ImagePlayer.cpp`（`ImagePlayer::StopImageAnimation`）：无条件 `m_bAutoPlay = false`。

**复核结论**：`Control::PaintImage` 每次绘制都会调用 `CheckStartImageAnimation()`，而后者只检查 `m_bAutoPlay`。如果不在这里关闭自动播放，播放次数已用尽的动画、或用户主动停止的动画，会在下一次重绘时被重新启动。因此该行为是必要的，不属于缺陷。

处理：仅在代码中补充注释说明原因；`Image::ClearImageCache`、`Image::PauseImageAnimation` 恢复 `m_bAutoPlay` 的做法保持不变。

### 14. 更换图片时未重置播放状态 — 已修复

位置：`duilib/Image/ImagePlayer.cpp`（`ImagePlayer::SetImage`）：只更新指针与计数，未 `Cancel()` 定时器、未清 `m_bAnimationPlaying`/`m_nCycledCount`，换图后可能沿用旧的播放次数与播放态。

修复：`SetImage` 入口先取消定时器并复位 `m_bAnimationPlaying`、`m_nCycledCount`，再重新读取播放次数与自动播放属性；同时在 `Image::SetImageInfo` 中，图片确实发生变化且播放器已存在时重新调用 `SetImage(this)`，使切换图片后播放参数同步更新。

### 15. 异步解码未完成时，每帧都新建帧对象 — 已修复

位置：`duilib/Image/ImageInfo.cpp`（`ImageInfo::GetFrame`）：`m_bDataPending` 分支每次 `make_shared<AnimationFrame>()` 后立即丢弃，滚动时产生持续的堆分配抖动。

修复：新增 `m_pPendingFrame`/`m_pendingFrameDestSize`，当帧号与目标尺寸未变时复用同一个等待帧对象，让异步解码结果直接落到该对象上；数据就绪或图片数据更新时清除该缓存。

### 16. 每次格式探测都拆分字符串 — 已修复

位置：`duilib/Image/ImageDecoder_Common.cpp`（`ImageDecoder_Common::CanDecode`）：每次调用都 `GetSupportedFileExtentions()` + `Split` 生成 `std::list<DString>`，而解码分发会对每个解码器、每张图调用一次。

修复：改为直接扫描分号分隔的后缀列表逐个比较，不再生成容器（同时避免空扩展名被误判）。没有采用静态缓存，因为该函数可能被异步解码线程调用，静态可变缓存会带来线程安全问题。

---

## 三、界面美观度相关

1. **SVG 缩放策略偏保守** — 已修复：栅格化比例由 `min` 改为 `max`（见第 4 条）。
2. **Alpha 预乘/非预乘混用** — 已修复（见第 2 条）。这是半透明圆角、阴影、渐变图标边缘发黑/发彩的最常见根因。
3. **`ceil` 缩放导致 1px 错位/抖动** — 已修复（见第 9 条）。
4. **九宫格边距：目标区域小于边距之和时出现负尺寸** — 已修复。实际位置不在 `ImageInfo`，而在 `duilib/RenderSkia/Render_Skia.cpp`（`Render_Skia::DrawImage`）：原实现直接用图片边距计算目标中间区域，目标区域小于边距之和时 `rcDrawDest` 左右（或上下）反转，中间区域被压成负尺寸，表现为圆角按钮被「切角/变形」。修复：新增 `rcDestBorder`，目标区域不足时按比例压缩目标边距（且保证压缩后四边之和等于目标尺寸），源图片边距保持不变，四角/四边按压缩后的目标边距绘制。
5. **上采样滤波** — 已修复：`duilib/Image/ImageUtil.cpp`（`ImageUtil::ResizeImageData`）改用 stb 的扩展接口按缩放方向选择滤波——放大时用 `STBIR_FILTER_CATMULLROM`（小图标放大更锐利），缩小时用 `STBIR_FILTER_MITCHELL`（减轻摩尔纹）。

---

## 四、可维护性

- **已修复**：`duilib/Image/ImageAttribute` 的 4 个矩形成员（`m_rcDest`/`m_rcSource`/`m_rcCorner`/`m_rcMargin`）由裸指针改为 `std::unique_ptr`，拷贝赋值简化为 4 行，`Init()`/析构函数中的手工 `delete` 全部移除。
- **不修改（按要求保留）**：各 `Image_PNG/GIF/WEBP/JPEG::LoadImageFile` 骨架重复，可抽公共基类。
- **未修改（兼容性原因）**：`ImageAttribute::m_bFade` 是公共成员（`Control.cpp`、`StateImage.cpp` 均直接访问），改名 `m_nFade` 会破坏下游源码兼容性，因此保留原名；其注释已明确标注「透明度（0 - 255）」。

---

## 五、修改文件清单

| 文件 | 涉及条目 |
|:---|:---|
| `duilib/Image/ImageUtil.cpp` | 1、2、9、美观度 5 |
| `duilib/Image/ImageUtil.h` | 2 |
| `duilib/RenderSkia/Bitmap_Skia.cpp` | 2 |
| `duilib/RenderSkia/Render_Skia.cpp` | 美观度 4（九宫格边距压缩） |
| `duilib/Image/ImageDecoderFactory.cpp` | 3 |
| `duilib/Image/Image.cpp` | 4、6、14 |
| `duilib/Image/ImageInfo.cpp` / `ImageInfo.h` | 4、5、10、15 |
| `duilib/Image/ImageDecoder_SVG.cpp` | 7 |
| `duilib/Image/ImageLoadParam.cpp` | 8 |
| `duilib/Image/ImageAttribute.cpp` / `ImageAttribute.h` | 11、12、可维护性（unique_ptr） |
| `duilib/Image/ImagePlayer.cpp` | 13、14 |
| `duilib/Image/ImageDecoder_Common.cpp` | 16 |

验证方式：`python build/build.py build --config Debug --arch x64 --offline --no-deps --target duilib`，`/W4 /WX` 无警告。
