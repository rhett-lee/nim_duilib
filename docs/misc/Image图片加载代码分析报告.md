# Image 目录图片加载代码分析报告

分析范围：`duilib/Image` 目录（约 60 个文件），涵盖图片加载主链路、各格式解码器、动画播放、状态图与图片属性解析。

以下问题均基于实际代码核对，按严重程度排列。

---

## 一、高风险（真实缺陷）

### 1. `ResizeImageBitmap` 源位图未解锁（句柄状态失衡）

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

建议：使用 RAII 守卫成对加解锁。

### 2. 重采样写死为预乘 Alpha，非预乘位图会被算错

位置：`duilib/Image/ImageUtil.cpp`（`ResizeImageData`）固定使用 `STBIR_RGBA_PM`；调用方 `duilib/RenderSkia/Bitmap_Skia.cpp`（`Bitmap_Skia::Init`）是按传入的 `alphaType` 建位图的，且确实存在 `kUnpremul`（如 `duilib/Control/ColorSlider.cpp`）。

非预乘数据被当作预乘做双线性插值，会出现彩色边缘、半透明区域发黑/发灰——这是典型观感问题。

建议：按 `AlphaType` 在 `STBIR_RGBA_PM` / `STBIR_RGBA` 之间选择。

### 3. SVG 解码分支存在除零/NaN

位置：`duilib/Image/ImageDecoderFactory.cpp`（`DecodeImageData`）

```cpp
if ((cx > 0) && (pImage->GetWidth() > 0)) { ... }          // X 有保护
if (cy > 0) { fRealScaleY = cy / pImage->GetHeight(); }   // Y 没有保护
```

高度为 0 时得到 `inf`，随后 `(int32_t)std::round(inf)` 是未定义行为。

建议：补充 `pImage->GetHeight() > 0` 判断，并对计算结果做有限性校验。

### 4. SVG 拉伸判断用 `||`，图标会被拉伸变糊

位置：`duilib/Image/Image.cpp`（`Image::GetCurrentBitmap`）

```cpp
else if ((rcDest.Width() == rcSource.Width()) ||
         (rcDest.Height() == rcSource.Height())) {
    return GetBitmapData(...);   // 不按目标尺寸重新栅格化
}
```

只要有一个维度相等就放弃矢量缩放。例如 100×100 的 SVG 画到 100×300 区域时，会先按 100×100 栅格化再拉伸 3 倍。判断条件应为 `&&`。这一条直接影响 SVG 图标/背景在高分屏下的清晰度。

### 5. `ImageInfo::SetImageData` 未清理旧位图缓存

位置：`duilib/Image/ImageInfo.cpp`（`ImageInfo::SetImageData`）

该方法只重置了 `m_pLastFrame`，没有 `m_pBitmap.reset()`，也没有对旧的 `m_pImageData` 走 `ReleaseImage()`（会丢失延迟释放登记）。当前 `ImageManager` 每次新建 `ImageInfo`，所以暂时不触发；一旦复用对象就会返回尺寸错误的旧位图。

### 6. `Image` 析构期间向 `Control` 发绘制请求

位置：`duilib/Image/Image.cpp`（`Image::~Image`）→ `duilib/Image/ImagePlayer.cpp`（`StopImageAnimation`）→ `RedrawImage()` → `m_pControl->InvalidateRect()`。

`Image` 通常是 `Control` 的成员，`Control` 析构时 `Image` 析构会回调一个半销毁的 `Control`，属于悬垂/重入风险。

建议：析构路径只取消定时器、不重绘。

---

## 二、中风险（性能与健壮性）

### 7. SVG 完全不缓存，每次绘制都重新栅格化

位置：`duilib/Image/ImageInfo.cpp`（`ImageInfo::GetSvgBitmap`，代码中明确注释「SVG图片，无缓存」）。

列表滚动、按钮 hover 时每帧都会 `GetBitmap` 生成新位图，是绘制路径上最明显的 CPU 热点。

建议：按 `(宽, 高, 颜色替换串)` 做 2～4 项 LRU 缓存。

### 8. 图片缓存 Key 用文件名字符拼接，存在冲突与重复计算

位置：`duilib/Image/ImageLoadParam.cpp`（`ImageLoadParam::GetLoadKey`）用 `@` `#` `$` `&` 作为分隔符，而这些字符在合法文件名中也会出现（如 `icon#1.png`），可能产生 key 冲突；且 `ImageInfo::GetLoadKey()` 在绘制路径上被反复调用，每次都重新拼字符串。

建议：改成结构化 key（path + dpi + flags + size）或对分隔符转义，并缓存结果。

### 9. 注释与实现不一致：偏移缩放用的是 `ceil` 而非四舍五入

位置：`duilib/Image/ImageUtil.cpp`（`ImageUtil::GetScaledImageOffset`）

```cpp
//计算后，进行四舍五入
nImageOffset = static_cast<int32_t>(std::ceil(nImageOffset * fImageSizeScale));
```

`ceil` 对负数向 0 取整，导致左右/上下偏移不对称，DPI 非整数倍（如 125%、150%）下出现 1px 抖动。应改用 `std::lround`。

### 10. 禁用 DPI 缩放 + DPI 变体图时，图片静默消失

位置：`duilib/Image/ImageInfo.cpp`（`ImageInfo::CalcImageInfoSize`）：Release 下直接 `return`（宽高为 0），整张图不显示。

建议：降级为按实际像素尺寸使用，而不是丢弃。

### 11. `ModifyAttribute` 每次重置 DPI 相关开关

位置：`duilib/Image/ImageAttribute.cpp`（`ImageAttribute::ModifyAttribute`）：`Image::UpdateImageAttribute` 只传部分属性时（`duilib/Image/Image.cpp`），之前设置的 `dpi_scale="false"`、`dest_scale="false"` 会被重置为 `true`。

### 12. 未知属性仅在 Debug 断言

位置：`duilib/Image/ImageAttribute.cpp`（`ImageAttribute::ModifyAttribute` 末尾）：XML 里拼错属性名，Release 版本完全静默。

建议：输出一条带图片名和属性名的警告日志。

### 13. 停止播放会永久关闭自动播放

位置：`duilib/Image/ImagePlayer.cpp`（`ImagePlayer::StopImageAnimation`）：无条件 `m_bAutoPlay = false`，连播放到次数上限自动停止也会置 false，因此 `ClearImageCache`、`PauseImageAnimation` 都必须手工恢复。

建议：区分「用户停止」与「播放结束」，避免依赖调用方补救。

### 14. 更换图片时未重置播放状态

位置：`duilib/Image/ImagePlayer.cpp`（`ImagePlayer::SetImage`）：只更新指针与计数，未 `Cancel()` 定时器、未清 `m_bAnimationPlaying`/`m_nCycledCount`，换图后可能沿用旧的播放次数与播放态。

### 15. 异步解码未完成时，每帧都新建帧对象

位置：`duilib/Image/ImageInfo.cpp`（`ImageInfo::GetFrame`）：`m_bDataPending` 分支每次 `make_shared<AnimationFrame>()` 后立即丢弃，滚动时产生持续的堆分配抖动。

建议：缓存一个 pending 对象。

### 16. 每次格式探测都拆分字符串

位置：`duilib/Image/ImageDecoder_Common.cpp`（`ImageDecoder_Common::CanDecode`）：每次调用都 `GetSupportedFileExtentions()` + `Split` 生成 `std::list<DString>`，而解码分发会对每个解码器、每张图调用一次。

建议：缓存为静态集合。

---

## 三、界面美观度相关

1. **SVG 缩放策略偏保守**：`duilib/Image/ImageInfo.cpp`（`ImageInfo::GetSvgBitmap(rcDest, ...)`）用 `min(scaleX, scaleY)` 栅格化，随后位图被拉伸到目标矩形，非等比区域会变模糊。UI 场景建议按 `max` 或直接按目标矩形栅格化。
2. **Alpha 预乘/非预乘混用**（第 2 条）：这是半透明圆角、阴影、渐变图标边缘发黑/发彩的最常见根因。
3. **`ceil` 缩放**（第 9 条）：125%/150% 缩放下边框、图标出现 1px 错位和抖动。
4. **九宫格边距只做 clamp**：`duilib/Image/ImageInfo.cpp`（`ImageInfo::ScaleImageSourceRect`）在目标小于四角边距之和时，中间区域会被压成负尺寸，表现为圆角按钮被「切角/变形」。建议边距按比例压缩（等比缩放四边）。
5. **上采样滤波**：`duilib/Image/ImageUtil.cpp` 用 `STBIR_FILTER_TRIANGLE`，小图标放大时偏软；可考虑 `STBIR_FILTER_CATMULLROM` 提升锐度，下采样显式指定 Mitchell 以减轻摩尔纹。

---

## 四、可维护性

- `duilib/Image/ImageAttribute.cpp` 用裸 `new/delete` 管理 4 个矩形成员（`m_rcDest`/`m_rcSource`/`m_rcCorner`/`m_rcMargin`），建议改为 `std::unique_ptr<UiRect>`（拷贝赋值/析构可整体简化）。
- 各 `Image_PNG/GIF/WEBP/JPEG::LoadImageFile` 骨架高度重复，可抽公共基类。
- `duilib/Image/Image.cpp` 中 `SetImageFade(uint8_t nFade)` 的命名与「透明度」语义不一致（成员叫 `m_bFade`）。

---

## 五、建议修复顺序

1. 第 1、2、3 条（泄漏、颜色错误、UB）——改动小、收益最大
2. 第 4 条 `||` → `&&`（一行改动，直接提升 SVG 清晰度）
3. 第 7、16 条（SVG / 解码器缓存）——绘制性能
4. 第 13、14 条（动画状态机收敛）
5. 其余按需
