# RenderSkia 目录代码审查与优化方案

## Context

用户要求对 `duilib/RenderSkia` 目录全部代码做 Review：修错误、做低风险优化、改进性能，且**不做大改**，避免引入新 bug。只要求 Debug 编译通过，不提交代码。

已完成 40 个文件的通读/定向阅读（Render_Skia、Bitmap_Skia、Path_Skia、Font_Skia、Pen/Brush/Matrix、SkTextBox、DrawSkiaText、SkiaTextData、FontMgr_Skia、DrawSkiaImage、FastVector、WindowRgn_Windows、Render_Skia_Windows、SkRasterWindowContext_Windows、SkUTF 等）。下面列出**已确认的缺陷与优化点**，均为小范围改动。

## 一、必须修复的缺陷（正确性）

### 1. Path_Skia.cpp — AddBeziers 越界读（真实缺陷）
`AddBeziers` 的 `for (; i < count; i += 3)` 没有保护 `points[i + 1]` / `points[i + 2]`。当 `count` 不是 `1 + 3n`（如 count=5）时读取越界，仅在 Debug 的 `ASSERT(i == count - 1)` 才暴露。
- 修改：循环条件改为 `i + 2 < count`，两处（`UiPoint` 与 `UiPointF` 重载）。

### 2. Path_Skia.cpp — GetBounds 返回未初始化数据
`SkRect bounds;` 未初始化，`computeFiniteBounds()` 在路径非有限/为空时返回 `nullopt`，随后用未初始化浮点做 `SkScalarTruncToInt`。
- 修改：`SkRect bounds = SkRect::MakeEmpty();`。

### 3. FastVector.h — reserve 内存泄漏 + 分配失败后写入空指针
- `reserve()`：当 `m_data != nullptr` 且 `m_size == 0`（`clear()` 之后扩容）时，既不 `memcpy` 也不 `free`，旧缓冲区泄漏。改为“有旧数据就 memcpy，最后无条件 free 旧缓冲区”。
- `push_back()` / `resize()`：`malloc` 失败时容量未增加，仍写 `m_data[m_size++]`，空指针写入崩溃。改为分配失败时直接返回（push_back）/ 不改变 size（resize）。

### 4. WindowRgn_Windows.cpp — SetWindowRgn 失败时 HRGN 泄漏
Windows 文档：`SetWindowRgn` 成功时系统接管 HRGN，失败时调用方必须 `DeleteObject`。
- 修改：返回失败时 `::DeleteObject(hRgn)`；`hRgn` 为 nullptr 时不调用 SetWindowRgn（避免误清除窗口区域）。

### 5. Render_Skia.cpp — ReadPixels/WritePixels 长度校验存在 int 溢出
`rc.Width() * rc.Height() * sizeof(uint32_t)` 在 int 中计算，超大矩形会溢出使校验失效。
- 修改：统一提升为 `(size_t)rc.Width() * (size_t)rc.Height() * sizeof(uint32_t)`（共 4 处）。

## 二、性能优化（低风险）

### 6. Render_Skia.cpp — SetPaintByPen 为“实线”创建 DashPathEffect
`kDashStyleSolid` 分支创建 `{w, 0}` 的 `SkDashPathEffect`，导致每条实线都走 path-effect 绘制路径（更慢，且 `0` 间隔属于退化参数）。
- 修改：solid 分支不设置 path effect（保持 nullptr），其余样式不变。

### 7. Render_Skia.cpp — DrawPath / FillPath（2 处）重复深拷贝路径
当前先 `SkPathBuilder` 拷贝构造，再 `offset()`，再 `snapshot()`，产生两次深拷贝。
- 修改：改为 `SkPath skPath = builder->snapshot(); skPath.offset(dx, dy);`，只一次拷贝 + 一次偏移。

### 8. Render_Skia.cpp — MeasureString 多行测量产生大量临时字符串
当前用 `vector<DString>` 逐行 `substr` 后再测量。
- 修改：直接用 `strText.c_str() + nTextPos * sizeof(value_type)` 与行字节长度调用 `DrawSkiaText::MeasureText`，删除 `lineTextList`。测量结果完全一致。

### 9. Path_Skia.cpp — AddPolygon 使用裸 new[]/delete[]
- 修改：改用 `std::vector<SkPoint>`（异常安全，且免手动释放），保持 `SkSpan` 调用不变。

## 三、执行阶段继续核对的项（不改动行为，只做定点确认）

以下文件尚未逐行读完，实现阶段用定向方式核对，若发现同类“越界/空指针/未初始化/临时对象重复构造”问题按同一标准小改；若无问题则不改：
- `DrawRichText.cpp/.h`（55KB，富文本排版，重点：行缓存边界、`nStartLine`/`modifiedLines` 下标校验、重复 `std::vector` 分配）
- `HorizontalDrawText.cpp` / `VerticalDrawText.cpp`（字间距/两端对齐绘制与测量，重点：测量与绘制路径一致性、循环边界）
- `SkGLWindowContext_Windows.cpp`、`Render_Skia_SDL.cpp`（Windows 构建下不参与编译的路径，只做安全核对，不改）

**明确不改的项（避免引入风险）**：
- `Render_Skia::SetRoundClip` 的 `SkRegion` 裁剪实现（改成 `clipRRect` 会改变抗锯齿表现与 `GetClipInfo` 返回值）
- `DrawRect`/`DrawRoundRect` 中 `SkIntToScalar(fWidth)`（与 Direct2D 行为、既有视觉一致，属有意取整）
- `Bitmap_Skia` 增加 `SkImage` 缓存（位图可被 `LockPixelBits` 外部修改，缓存会读到脏数据）
- `FastVector` 的“resize 不初始化”语义（性能设计，调用方随后会填充）

## 四、验证

1. Debug 增量编译（仅 duilib target，不并发执行）：
```
$msvc="C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231"; $sdk="C:\Program Files (x86)\Windows Kits\10"; $sdkver="10.0.26100.0"; $env:INCLUDE="$msvc\include;$sdk\Include\$sdkver\ucrt;$sdk\Include\$sdkver\um;$sdk\Include\$sdkver\shared;$sdk\Include\$sdkver\winrt;$sdk\Include\$sdkver\cppwinrt"; $env:LIB="$msvc\lib\x64;$sdk\Lib\$sdkver\ucrt\x64;$sdk\Lib\$sdkver\um\x64"; cmake --build "E:\develop\nim_duilib\out\x64\Debug" --target duilib
```
2. 确认无新增警告、无 C1090（不做并行构建）。
3. 输出中文修复清单（问题 → 修改 → 影响面）与性能收益说明，只输出文本，不提交代码。
