# Render 目录代码审查报告

## 一、审查范围

- 审查目录：`duilib/Render`
- 审查文件：
  - 裁剪守卫：`AutoClip.h` / `AutoClip.cpp`
  - 位图 Alpha 保存/恢复：`BitmapAlpha.h` / `BitmapAlpha.cpp`
  - 渲染接口与内联实现：`IRender.h`
    （含 `RichTextAttrTable`、`RichTextData`、`RichTextDataList`、`RichTextCharLayout`、
    `RichTextRowInfo`、`RichTextLineText`、`RichTextLineInfo`、`CanPresentBackBufferOnly` 等）
- 审查重点：
  1. `BitmapAlpha` 的区域裁剪、整行合并、SSE2 与标量尾部处理、Alpha 恢复规则；
  2. `AutoClip` 的 RAII 对称性、空指针/空矩形/嵌套作用域；
  3. 富文本排版辅助结构的索引约定、去重逻辑、定点数存储、超宽行切换、稀疏属性标志；
  4. 共享文本的偏移访问契约、接口默认实现与调用点的一致性。
- 审查原则：不做大的修改，只修复确认的必要缺陷；需要较大范围调整或改变既有行为的
  问题记录在“五、建议后续处理（本轮未修改）”。

## 二、总体结论

`duilib/Render` 目录代码量不大，但都是布局/绘制链路的基础设施，整体实现质量较好：

1. `BitmapAlpha` 的空区域快速路径与逐行裁剪路径判定口径一致，整行覆盖时会合并为一次
   `memset`；SSE2 路径处理 4 像素对齐部分、标量循环处理尾部，两条路径的语义与文档规则
   完全对应；`RestoreAlpha` 的阴影边距语义与调用点（`Window.cpp`、`Render_Skia.cpp`）
   保持一致；
2. `AutoClip` 构造时先把成员置为安全状态，`bClip == false` 或渲染对象不可用时为空操作，
   `ClipRect/ClipRoundRect` 返回 -1 时不会记录待恢复状态，析构仅恢复一次，嵌套作用域
   依赖后端的 save/restore 栈，行为正确；
3. `RichTextAttrTable` 的“索引从 1 开始、0 表示未设置”约定在 `Add* / Get* / Is*` 中一致，
   字体按属性值去重、颜色按颜色值去重、图片按“指针 + 显示宽高”去重，`Clear` 保留容量、
   `Release` 归还容量、`Swap` 交换所有权，三者语义清晰；
4. `RichTextCharLayout` 用 1/8 像素定点数存储 x 偏移（四舍五入，误差 ≤ 1/16 像素且不随
   字符数累积），超过可表示范围时自动切换为 32 位偏移；属性标志采用稀疏存储并用
   `lower_bound` 查找，`ToRawOffset` 对负数/NaN 统一按 0 处理，避免了浮点转无符号的
   未定义行为；
5. `RichTextLineText` / `RichTextLineInfo` 通过共享整篇文本 + 偏移量的方式表示每行，
   避免每行一次字符串分配；文档明确“行文本不保证以 `\0` 结尾、不提供 `c_str()`”；
6. `IRender` 的默认实现（`GetUpdateRects`、`IsPresentOnly`、`IsBackBufferValid`）均为
   保守返回值，与派生类实现兼容；`CanPresentBackBufferOnly` 的真值表与注释一致。

本轮未发现功能性缺陷，修正了 3 处小问题（1 处可移植性/告警隐患、2 处注释与实现不一致），
并新增了覆盖上述基础设施的单元测试。`BitmapAlpha` 的核心算法已由既有
`tests/test_bitmap_alpha.cpp` 覆盖，本轮不重复测试。

## 三、已修复的问题

### 1. `RichTextAttrTable::GetFont/GetColor/GetImage` 的符号比较问题

- 位置：[IRender.h](file:///e:/develop/nim_duilib/duilib/Render/IRender.h#L877-L910)
- 问题：`nIndex` 为 `uint16_t`，经整型提升为 `int` 后与 `std::vector::size()`（`size_t`）
  比较，属于有符号/无符号混合比较；在 GCC/Clang 的 `-Wextra`（本项目 CMake 已启用
  `-Wall -Wextra`）下会产生 `-Wsign-compare` 告警，属于可移植性隐患；
- 修复：三处比较统一显式转换为 `size_t` 后再比较：
  `((size_t)nIndex > m_fontList.size())` 等；
- 影响：无行为变化，仅消除告警隐患。

### 2. `AutoClip.cpp` 注释对 `ClipRect` 返回值的描述与实现不符

- 位置：[AutoClip.cpp](file:///e:/develop/nim_duilib/duilib/Render/AutoClip.cpp#L42-L49)
- 问题：原注释称 `ClipRect` 返回 -1 的场景包括“矩形为空”；但 `IRender::ClipRect` 的接口约定
  与 `Render_Skia::ClipRect` 的实现均表明：**空矩形是合法输入**，会正常 `save()` + `clipRect()`
  并返回有效状态，只是裁剪结果为空（`IsClipEmpty() == true`）；-1 仅表示“渲染对象不可用/
  未压入裁剪状态”。错误注释会误导后续维护者把 -1 与空矩形混为一谈；
- 修复：更正注释，补充“空矩形返回有效标志、析构时仍需正常恢复”的说明；
- 影响：仅注释，无代码行为变化。

### 3. `BitmapAlpha.h` 成员注释的像素格式与类注释不一致

- 位置：[BitmapAlpha.h](file:///e:/develop/nim_duilib/duilib/Render/BitmapAlpha.h#L77-L79)
- 问题：类注释说明像素内存布局为 `BGRA` 或 `RGBA`（Alpha 位于第 4 个字节），而
  `m_nChannels` 成员注释写为 `ARGB`，两处描述冲突；
- 修复：成员注释统一为“只支持 4（BGRA/RGBA，Alpha 位于第 4 个字节）”；
- 影响：仅注释。

## 四、已核实、无需修改的点

- `BitmapAlpha::ClearAlpha`：
  - 全区域快速路径要求 `left/top == 0` 且宽高与位图完全相等，否则走逐行裁剪路径；
  - 逐行路径中 `nBytes == rowStrideBytes` 时把多行合并为一次 `memset`，与逐行结果等价；
  - 矩形越界（左/上为负、右/下超出）由 `std::max/std::min/std::min` 裁剪，不会越界访问；
- `BitmapAlpha::RestoreAlpha`：
  - `alpha == 0` 与 `alpha != 0` 两种恢复规则与头文件文档一致，且只修改 Alpha 字节；
  - `alpha != 0` 时“Alpha 等于 alpha 的像素置 0、Alpha 为 0 的像素置 255”两个条件互斥，
    SSE2 掩码组合（`= 0` 与 `| 0xFF000000`）不会相互覆盖；
  - `rcShadowPadding` 采用“以位图左上角为基准的绝对边距”，与调用点
    `Window.cpp`（`rcRootPadding = 根控件 Padding`、`rcNewPaint = rcPaint ∩ 根控件去 Padding 区域`）
    以及 `Render_Skia::ClearAlpha/RestoreAlpha` 的调用方式一致，不是“相对 rcDirty 的内缩”；
- `AutoClip`：
  - 两个构造函数都先把 `m_pRender/m_bClip/m_nClipState` 置为安全值，`bClip == false`
    或 `pRender == nullptr` 时析构不做任何操作（Release 下不会空指针解引用）；
  - `ClipRect/ClipRoundRect` 返回 -1 时不设置 `m_bClip`，避免向 `RestoreClip` 传入非法标志；
  - 嵌套使用时，内外层的恢复顺序与后端 save/restore 栈语义匹配；
- `RichTextAttrTable`：
  - `AddFont(nullptr)`、`AddColor(空颜色)`、`AddImage(nullptr)` 均返回 0 且不占用索引；
  - `GetImage` 在索引无效时返回函数内静态空对象，避免返回悬垂引用；
  - `IsFontEqual/IsColorEqual/IsImageEqual` 先做指针相等短路，再按值比较，跨表不同索引
    的比较结果正确；
- `RichTextDataList`：`Clear/Release/Swap` 同时作用于文本片段与属性表，`Reserve` 不影响数据；
- `RichTextCharLayout`：
  - `ToRawOffset` 用 `!(xOffset > 0.0f)` 同时处理负数和 NaN，且对接近 `uint32` 上限的值做饱和，
    不存在浮点转无符号的未定义行为；
  - `ConvertToWideRow` 在切换前按常规行（1 个 `uint16`/字符）读取旧数据，切换后数据无损；
  - 属性标志按字符下标升序写入，`CharFlag` 的 `lower_bound` 比较器与存储格式匹配；
  - `charFlag == 0` 不写入标志表，查询未命中时返回 0，与“默认无标志”一致；
- `RichTextLineText / RichTextLineInfo`：共享文本 + 偏移量的访问方式与文档契约一致
  （长度由 `m_nLineTextLen` 记录，调用方保证在共享文本范围内）；
- `CanPresentBackBufferOnly`：仅在“后备缓冲有效、内容未变化、无脏区”时返回 `true`；
- `IRender` 默认实现：`GetUpdateRects` 在无脏区时返回 `false`、`IsPresentOnly` 默认为
  `false`、`IsBackBufferValid` 默认为 `false`，均为保守行为。

## 五、建议后续处理（本轮未修改）

以下问题不影响当前正确性，或属于低频/不可达场景，本轮仅记录，未做修改：

1. `RichTextAttrTable` 的索引空间为 `uint16_t`：当单个文档的不同字体/颜色/图片数量超过
   65535 时，`AddFont/AddColor/AddImage` 返回 `(uint16_t)size` 会回绕为 0（被视为“未设置”）。
   实际场景不可达（单文档不会出现 6 万多种字体），如需彻底防御，可在 `Add*` 中增加容量
   判断并返回 0（同时保留断言）；
2. `RichTextCharLayout` 的属性标志把字符下标压入高 24 位（`(index << 8) | flag`）：单行字符数
   超过 16,777,215 时标志会错位。实际不可达，建议在文档中注明该上限；
3. `RichTextLineText::data()` 与 `RichTextLineInfo::GetLineTextView()` 不校验偏移/长度是否超出
   共享文本：越界属于调用方违约（`UiStringW`（`UiStringT`）未提供 `size()`，逐行调用
   `StringLen` 会带来 O(n²) 开销）。若需加固，建议在 `RichTextLineInfo` 中额外保存共享文本
   长度（由数据提供方在 `SetShareText` 时传入），而不是在 `data()` 内计算；
4. `RichTextCharLayout::CharWidth(CharCount())` 返回 0，而 `CharX(CharCount())` 返回行宽，
   两者不对称；`CharX` 已注明该行为，建议 `CharWidth` 也补充注释；
5. `AutoClip` 以 `m_nClipState >= 0` 判定有效，而 `IRender::RestoreClip` 约定“`<= 0` 不处理”：
   当前仅有的后端（Render_Skia）有效状态均 `>= 1`、-1 仅作失败标记，二者不冲突；后续新增
   后端时应保持“失败返回 -1”的约定；
6. `BitmapAlpha` 假定 `pPixel` 按 32 位对齐并以 `uint32_t*` 访问：Skia 位图满足该要求；
   若将来用于非对齐的宿主内存，应改为 `memcpy` 或按字节访问。

## 六、单元测试

### 1. 新增测试

新增 `tests/test_render_core.cpp`，并在 `tests/CMakeLists.txt` 中注册为 `test_render_core`
可执行目标与 `render_core` 测试项（静态库模式构建，链接配置与 `test_render_api` 一致）。

测试分为纯逻辑部分与离屏渲染部分：前者不依赖渲染后端，后者通过
`GlobalManager::Startup` + `IRenderFactory::CreateOffscreenRender` 创建 80x80 离屏画布，
不创建窗口、不启动消息循环。

### 2. 覆盖内容

1. `RichTextAttrTable`（RT-01 ~ RT-20）：空表查询、空字体/空颜色/空图片返回 0、
   索引从 1 开始、字体与颜色去重、无效索引、跨表比较（索引不同但内容相同）、
   `Swap`/`Clear`/`Release`；
2. `RichTextAttrTable` 的内联图片（RT-21 ~ RT-26）：同一图片 + 相同显示大小去重、
   显示大小不同视为不同图片、不同图片新增索引、`IsImageEqual` 的指针与尺寸组合比较；
3. `RichTextDataList`（RT-31 ~ RT-36）：增删、`Reserve`、`Swap`（片段与属性表联动）、
   `Clear`/`Release`；
4. `RichTextCharLayout`（RT-41 ~ RT-53）：空对象、常规行的偏移/宽度/行宽/属性标志、
   `CharX(CharCount())` 返回行宽、拷贝比较、`Clear` 重置超宽行标志、负数与 NaN 按 0 处理、
   超宽行切换后历史偏移不变、继续追加、`kOffsetMaxValue/kOffsetUnit` 边界值仍为常规存储、
   `Reserve` 不影响数据；
5. `RichTextLineText` / `RichTextLineInfo`（RT-61 ~ RT-68）：默认空文本、共享文本 + 偏移访问、
   偏移等于文本长度、独立存储（复制内容）、拷贝构造共享存储、按长度获取行视图、长度为 0
   的空视图；
6. `AutoClip`（AC-01 ~ AC-08）：矩形裁剪与析构恢复、`bClip == false`、嵌套裁剪（内层析构后
   外层仍有效）、空矩形产生空裁剪且能恢复、圆角矩形裁剪、`nullptr + bClip == false`、
   同一渲染对象上多次顺序使用；
7. `CanPresentBackBufferOnly`（RT-91 ~ RT-94）：后备缓冲有效性与内容脏标记/脏区域的真值表。

### 3. 未纳入单测的部分

- `BitmapAlpha` 的 Alpha 保存/恢复、全区域快速路径、SSE2 尾部处理与阴影边距
  已由既有 `tests/test_bitmap_alpha.cpp` 完整覆盖，本轮不重复测试；
- `AutoClip` 绑定真实窗口 Render 的场景（窗口绘制链路）依赖窗口创建与消息循环，
  不在本单测范围内，由现有窗口相关的渲染测试间接覆盖。

### 4. 验证结果

- 构建：`python build\build.py build --config Debug --arch x64 --offline --no-deps --no-examples --target test_render_core`
  成功（静态库模式，未使用 DLL 模式）；
- 运行：`ctest --test-dir out\x64\Debug -C Debug -R "^render_core$" --output-on-failure`
  100% 通过（195 项检查）；
- 回归：`bitmap_alpha`、`text_draw`、`render_ext`、`render_api` 等与 `IRender.h` / 富文本
  相关的测试全部通过，未发现对既有功能的影响；
- 全量回归：`ctest -E "^richedit2_edit$"` 共 34 项测试全部通过；`richedit2_edit` 为交互式
  窗口测试，在无交互的环境下会阻塞等待窗口，本轮未纳入自动化验证（与本次修改无关）。
