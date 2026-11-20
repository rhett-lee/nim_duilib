# RenderSkia 目录代码审查报告

## 一、审查范围

- 审查目录：`duilib/RenderSkia`
- 审查文件：
  - 渲染实现：`Render_Skia.h/.cpp`、`Render_Skia_Bitmap.h/.cpp`、`Render_Skia_Offscreen.h/.cpp`、
    `Render_Skia_Windows.h/.cpp`、`Render_Skia_SDL.h/.cpp`
  - 渲染工厂与资源管理：`RenderFactory_Skia.h/.cpp`、`FontMgr_Skia.h/.cpp`
  - 基础对象封装：`Bitmap_Skia`、`Brush_Skia`、`Pen_Skia`、`Path_Skia`、`Font_Skia`、`Matrix_Skia`
  - 文本与编码工具：`SkUTF.h/.cpp`、`SkiaTextData.h/.cpp`、`TextGlyphRun.h/.cpp`、
    `DrawSkiaText.cpp`、`SkTextBox.h/.cpp`、`DrawRichText.cpp`、`HorizontalDrawText.cpp`、
    `VerticalDrawText.cpp`
  - 图片绘制：`DrawSkiaImage.h/.cpp`
  - 容器工具：`FastVector.h`
- 审查重点：
  1. 画布状态保存/恢复（save/restore、裁剪栈、视区原点）的正确性；
  2. 像素级接口（`Clear`、`ClearRect`、`ReadPixels`、`WritePixels`、`MakeImageSnapshot`）
     的坐标换算、越界判断与缓冲区长度校验；
  3. 文本测量与绘制（`MeasureString`、`DrawString`、`SkTextBox`、字形 run 构建）的
     行距/断行规则是否一致；
  4. 图片缩放（OpenCV / stb_image 两条路径）的数据所有性与编译条件；
  5. 资源管理接口的空指针防护与生命周期。
- 审查原则：不做大的修改，只修复确认的必要缺陷；需要较大范围调整或改变既有行为的
  问题记录在“五、建议后续处理（本轮未修改）”。

## 二、总体结论

`duilib/RenderSkia` 是实现 `IRender` 接口的 Skia 渲染后端，代码分层清晰：
`Render_Skia` 负责通用绘制与状态管理，`Render_Skia_Bitmap` / `Render_Skia_Offscreen` /
`Render_Skia_Windows` / `Render_Skia_SDL` 以“包装画布”的方式复用同一套绘制逻辑，
文本与图片各有独立工具类。整体实现质量较好：

1. 画布状态管理使用 `save()/restoreToCount()` 并同步维护 `m_saveCount`，`RestoreClip`
   的语义与接口约定一致；
2. 像素级接口统一按“客户区坐标 + 视区原点”换算到设备坐标，`ReadPixels/WritePixels`
   要求矩形完全位于画布内，并对 `dstPixelsLen/srcPixelsLen` 做完整尺寸校验，
   宽度×高度的字节数统一提升为 `size_t` 计算，避免 `int` 溢出；
3. `Clear/ClearRect` 在可访问像素时直接填充内存（整行覆盖时合并为一次连续填充），
   在 GPU 后端下退化为 Skia 接口，并正确地对待填充颜色做 Alpha 预乘；
4. `MeasureString` 的单行/多行、纵向、两端对齐、字间距分支划分清楚，多行断行与
   行宽复用了 `DrawSkiaText` 的布局结果；
5. `SkUTF` 与 Skia 原始实现（`skia/src/base/SkUTF.cpp`）保持等价，`FastVector`
   在分配失败时会放弃本次写入而不是写空指针；
6. `DrawSkiaImage` 的缩放缓存按“源图标识 + 目标尺寸”索引，命中时避免重复缩放。

本轮修正了 4 处代码问题（其中 2 处属于会直接导致编译失败或像素数据被破坏的缺陷）
和 1 处注释笔误，并新增了覆盖 `SkUTF`、`FastVector`、`SkiaTextData`、`TextGlyphRunList`、
位图渲染与文本行距的单元测试。

## 三、已修复的问题

### 1. `DrawSkiaImage.cpp`：OpenCV 缩放路径原地交换通道，破坏共享的源图片

- 位置：[DrawSkiaImage.cpp](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawSkiaImage.cpp#L343-L366)
- 问题：`SkImageToCvMat_Opt` 通过 `pixmap.writable_addr()` 取得可写指针，然后在原图上
  执行 `cv::mixChannels(viewMat, viewMat, { 2,0, 1,1, 0,2, 3,3 })` 原地交换 R/B 通道。
  `SkImage` 的像素可能被多次绘制或被缩放缓存共享，原地修改会导致：
  1. 同一张图片第二次绘制时颜色反转；
  2. 缓存中保存的已经是“交换过一次”的像素，后续命中缓存时颜色错误；
  3. 对非 RGBA 格式直接返回共享内存的 `cv::Mat`，一旦上层写入就会修改源图片。
- 修复：
  - 非 RGBA 分支直接返回共享内存的只读 `cv::Mat`（`const_cast<void*>(pixmap.addr())`），
    并在注释中说明其生命周期必须短于 `SkImage`；
  - RGBA 分支先 `srcMat.create(...)` 分配新缓冲区，再用
    `cv::mixChannels(viewMat, srcMat, { 2,0, 1,1, 0,2, 3,3 })` 把通道重排结果写入
    新缓冲区，源图片保持不变；
- 影响：修正了“同一图片多次缩放绘制后颜色反转”的像素级缺陷，缩放路径由“就地修改”
  改为“复制后转换”，仅在 OpenCV 缩放路径下多一次内存拷贝。

### 2. `DrawSkiaImage.cpp`：启用 OpenCV 时产生悬空 `else` 的编译错误

- 位置：[DrawSkiaImage.cpp](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawSkiaImage.cpp#L530-L545)
- 问题：原代码结构为
  `#ifdef DUILIB_HAVE_OPENCV ... if (...) {...} #else ... if (...) {...} #endif else {...}`。
  在启用 OpenCV（`DUILIB_HAVE_OPENCV` 已定义）时，预处理后只剩下
  `if (...) {...} else {...}` 看似合法，但宏分支内部还包含独立的 `if` 块，实际展开后
  会出现“`else` 找不到匹配的 `if`”的编译错误；关闭 OpenCV 时则能正常编译，问题只在
  特定构建配置下暴露。
- 修复：把两种实现的调用统一改为先求值并保存结果，再统一判断：

  ```cpp
  #ifdef DUILIB_HAVE_OPENCV
      const bool bResized = ResizeSkiaImageByOpenCV(...);
  #else
      const bool bResized = ResizeSkiaImageByStbImage(...);
  #endif
      if (bResized) {
          // 绘制缩放后的图片
      }
      else {
          // 交给 Skia 直接缩放绘制
      }
  ```

- 影响：两种构建配置下都能正常编译，绘制分支语义不变。

### 3. `Render_Skia::MeasureString`：多行测量忽略行间距参数

- 位置：[Render_Skia.cpp](file:///e:/develop/nim_duilib/duilib/RenderSkia/Render_Skia.cpp#L2369-L2375)
- 问题：多行分支原实现写死了 `float spacingMul = 1.0f;`（注释为“暂不支持设置”），
  然后 `SkScalar scaledSpacing = fontHeight * spacingMul;`，完全忽略入参
  `measureParam.fSpacingMul` 与 `measureParam.fSpacingAdd`。而 `DrawString` 使用
  `SkTextBox` 时的行距规则是“每行行距 = 字体高度 × 倍数 + 附加量”，导致
  **设置了行间距的文本，测量结果比实际绘制结果偏小**，进而影响控件布局。
- 修复：

  ```cpp
  //行间距：与 DrawString 中 SkTextBox 的计算规则保持一致
  const SkScalar scaledSpacing = fontHeight * measureParam.fSpacingMul + measureParam.fSpacingAdd;
  SkScalar textHeight = fontHeight;
  if (lineCount > 0) {
      textHeight += scaledSpacing * (lineCount - 1);
  }
  ```

- 影响：`MeasureString` 与 `DrawString` 的行距口径一致；默认参数（倍数 1.0、附加 0）
  下结果不变，设置行间距后测量高度正确增大。

### 4. `FontMgr_Skia::LoadFontFileData`：使用字体管理器前缺少空指针判断

- 位置：[FontMgr_Skia.cpp](file:///e:/develop/nim_duilib/duilib/RenderSkia/FontMgr_Skia.cpp#L355-L374)
- 问题：同类的 `CreateFont/AddFontFile/AddFontDir` 等方法在使用
  `m_impl->m_pSkFontMgr` 前都有空指针判断，但 `LoadFontFileData` 直接调用
  `m_impl->m_pSkFontMgr->makeFromData(skData)`，在 Release 构建下（`ASSERT` 为空操作）
  字体管理器初始化失败时会空指针解引用崩溃。
- 修复：在调用前补充与同类方法一致的防护：

  ```cpp
  ASSERT(m_impl->m_pSkFontMgr != nullptr);
  if (m_impl->m_pSkFontMgr == nullptr) {
      return false;
  }
  ```

- 影响：异常场景下返回 `false`，不再崩溃。

### 5. `Render_Skia.h`：成员注释笔误

- 位置：[Render_Skia.h](file:///e:/develop/nim_duilib/duilib/RenderSkia/Render_Skia.h)
- 问题：注释中的 `Canval保存的状态` 应为 `Canvas保存的状态`；
- 修复：更正拼写；
- 影响：仅注释，无代码行为变化。

## 四、已核实、无需修改的点

- `Matrix_Skia`：`IMatrix` 接口语义是“设置矩阵”而非“叠加矩阵”，
  `setTranslate/setScale/setRotate` 的使用与接口注释一致，不属于缺陷；
- `Bitmap_Skia::Init(w, h, nullptr)`：接口文档已明确“像素数据未初始化”，
  调用方负责写入全部像素，保持现状；
- `Render_Skia::RestoreClip`：使用 `restoreToCount(nState)` 恢复，并同步
  `m_saveCount = getSaveCount() - 1`，与 `SaveClip/ClipRect` 的压栈计数一致；
- `Render_Skia::ClearRect`：整行覆盖时把多行合并为一次连续填充，与逐行填充结果等价；
  非覆盖整行时按 `bytesPerRow` 定位每行起点，`nLeft/nRight` 已裁剪到有效范围；
- `ReadPixels/WritePixels`：矩形完全位于画布内才执行，`ReadPixels` 要求
  `dstPixelsLen >= 宽 × 高 × 4`，`WritePixels` 要求 `srcPixelsLen == 宽 × 高 × 4`，
  与接口约定一致；越界矩形在多线程/多窗口场景下返回 `false` 而不是部分读取；
- `MakeImageSnapshot`：可访问像素时零拷贝返回位图（底层像素来自画布），
  GPU 后端时通过 `SkSurface::readPixels` 回读；`MakeImageSnapshot(rc)` 先把区域
  裁剪到画布范围，空区域返回 `nullptr`；
- `RenderFactory_Skia`：位图、字体、离屏渲染对象的创建路径完整，
  `CreateBitmapRender` 注入渲染工厂指针，供字体回退等功能使用；
- `DrawSkiaText`：`BreakText/Linebreak/CountLines` 与 Skia 原生实现语义一致，
  断行时同时产出每行字节长度与宽度，`MeasureString` 复用该结果；
- `SkUTF`：`CountUTF8/16/32`、`NextUTF8/16/32`、`ToUTF8/ToUTF16`、
  `UTF8ToUTF16/UTF16ToUTF8` 的行为与 Skia 原始实现等价（含非法输入返回 -1/0、
  容量不足时只写入可容纳部分并返回完整长度）；
- `FastVector`：`push_back/resize/reserve` 在分配失败时放弃本次操作而不是写入空指针，
  `clear()` 保留容量、移动构造/赋值把被移动对象置空，语义与注释一致；
- `TextGlyphRunList`：`BeginRun` 自动结束未完成的 run，空 run 在 `EndRun` 时被丢弃，
  positions 模式只保存偏移、不保存步进宽度。

## 五、建议后续处理（本轮未修改）

以下问题不影响当前正确性，或属于低频/不可达场景，本轮仅记录，未做修改：

1. `SkiaTextData::EnumChars` 在 UTF-16 输入以“孤立高代理”结尾时返回 `true`，
   而 UTF-8 输入遇到被截断的序列时返回 `false`，两种编码对“截断”的处理不一致。
   建议后续统一为返回 `false`（或统一为容忍截断），需要同步评估调用方的错误处理；
2. `TextGlyphRunList::GetTotalWidth` 在 positions 模式下返回 0（该模式不保存步进宽度）。
   当前没有调用点在 positions 模式下使用总宽度，若后续需要，应按
   `positions[n].fX + 末字形宽度` 或单独累计；
3. `DrawRichText`：
   - 省略号的字体/颜色取自最后一个文本片段，建议改为取被截断位置的片段；
   - TAB 的绘制宽度与布局阶段计算的宽度口径不一致；
   - `UpdateDrawRichTextCache` 存在无符号减法下溢风险，且失败时无法回滚到旧缓存。
   该增量路径当前没有调用点，建议在启用该路径前先补齐这些边界处理；
4. `DrawSkiaImage` 的缩放缓存在图片内容被外部修改时不会失效（键为源图标识 +
   目标尺寸）。当前图片对象在使用期内视为只读，如需支持可变的图片，应增加
   版本号或内容哈希参与缓存键。

## 六、单元测试与验证结果

### 1. 新增测试

新增 [test_render_skia.cpp](file:///e:/develop/nim_duilib/tests/test_render_skia.cpp)，
并在 [tests/CMakeLists.txt](file:///e:/develop/nim_duilib/tests/CMakeLists.txt#L1342-L1378)
注册为 `render_skia` 测试目标。覆盖内容：

| 分组 | 用例 | 覆盖点 |
| --- | --- | --- |
| `TestSkUTF` | SK-01 ~ SK-11 | UTF-8/16/32 计数（合法、截断、非法字节、字节数非整倍数）、逐个解码与指针推进、编码与长度查询、编码转换的往返/容量不足/非法输入、字节类型与代理判断 |
| `TestFastVector` | SK-41 ~ SK-51 | 首次分配、连续增长不丢数据、`clear` 保留容量、`reserve` 不收缩、`resize` 默认值初始化、迭代器、`swap`、移动构造/赋值、空容器操作 |
| `TestSkiaTextData` | SK-61 ~ SK-68 | 空文本、UTF-8 字符枚举（码点 + 字节数）、回调提前停止、`ToUTF32String`、非法 UTF-8、UTF-16 代理对按 4 字节、UTF-32、`GetCharByteLength` |
| `TestTextGlyphRunList` | SK-81 ~ SK-87 | 游标构建 run、空 run 丢弃、未 `EndRun` 时自动结束、positions 模式保存偏移且总宽度为 0、rotate/起点/基线、`AddRun` 批量添加与空参数处理、`Clear` |
| `TestRenderSkiaBitmap` | SK-101 ~ SK-105 | `CreateBitmap` + `CreateBitmapRender`、绘制结果与位图像素一致、`ReadPixels` 越界返回 `false`、整图/局部快照尺寸与内容、`Clone` |
| `TestMeasureStringSpacing` | SK-121 ~ SK-124 | 默认行距总高 = 字体高 × 行数、倍数 2.0 时总高 = 字体高 × 5、附加量按“每行间距”累加、单行模式不受行间距影响 |

说明：
- `Clear/ClearRect/裁剪/BitBlt/StretchBlt/AlphaBlend/ClearAlpha/RestoreAlpha/快照/矩阵/Pen/Brush/Path`
  等绘制语义已由既有 `test_render_api.cpp` 覆盖，本测试不重复；
- `SkiaTextData` 的字符枚举与 `TextGlyphRunList` 的基础构建已有 `test_text_draw.cpp`
  覆盖，本测试补充的是编码边界、positions 模式与容量/移动语义；
- 测试只在 Windows 静态库（非 DLL）模式下注册，与库内未导出符号的可见性一致。

### 2. 构建与测试结果

```text
python build\build.py build --config Debug --arch x64 --offline --no-deps --no-examples --target test_render_skia
ctest --test-dir out\x64\Debug -C Debug -R "^render_skia$" --output-on-failure
```

结果：`render_skia` 通过（275 项检查，0 失败）。

回归验证：

```text
ctest --test-dir out\x64\Debug -C Debug -R "^(render_skia|render_api|render_ext|render_core|render_shape|text_draw|text_drawer|offscreen_render)$" --output-on-failure
```

结果：8 个测试全部通过，说明本轮对 `DrawSkiaImage`、`MeasureString`、`FontMgr_Skia`
的修改未引入回归。
