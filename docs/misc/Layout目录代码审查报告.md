# Layout 目录代码审查报告

## 一、审查范围

- 审查目录：`duilib/Layout`
- 审查文件：
  - 布局基类：`Layout.h` / `Layout.cpp`
  - 线性布局：`HLayout.h` / `HLayout.cpp`、`VLayout.h` / `VLayout.cpp`
  - 网格布局：`GridLayout.h` / `GridLayout.cpp`
  - 瓦片布局：`HTileLayout.h` / `HTileLayout.cpp`、`VTileLayout.h` / `VTileLayout.cpp`
  - 流式布局：`HFlowLayout.h` / `HFlowLayout.cpp`、`VFlowLayout.h` / `VFlowLayout.cpp`
  - 虚拟布局：`VirtualLayout.h`、`VirtualVLayout.h` / `.cpp`、`VirtualHLayout.h` / `.cpp`、
    `VirtualVTileLayout.h` / `.cpp`、`VirtualHTileLayout.h` / `.cpp`
- 审查重点：拉伸空间计算、间距/边距统计口径、对齐与偏移、边界条件（0 尺寸控件、
  空列表、合并单元格冲突）、虚拟布局的滚动定位。
- 审查原则：不做大的修改，只修复确认的必要缺陷；需要较大范围调整或改变既有行为的
  问题记录在“五、建议后续处理（本轮未修改）”。

## 二、总体结论

整体实现质量较好，布局算法的主体逻辑（固定控件 + 拉伸控件的空间分配、min/max 钳制、
空间不足时的降级、超出时的裁剪）是完整且自洽的：

1. `Layout` 基类的属性解析（`child_margin`、`child_margin_x/y`、`child_halign`、
   `child_valign`、`child_align`）覆盖了别名与组合写法，未知属性正确返回 `false`；
   负值边距被钳制为 0，`ChangeDpiScale` 按旧缩放比例换算；
2. `HLayout` / `VLayout` 的三种分配场景（可用空间充足、可用空间为 0、可用空间不足）
   与文档中的规则 1~9 对应，拉伸比例基数、余量逐像素分配、超出缩减均正确；
3. `GridLayout` 的行列自动计算、单元格等分与余数分配、合并单元格占用检测与冲突返回、
   `scale_down` 等比缩放均实现正确；
4. `HTileLayout` / `VTileLayout` 的两趟布局（先估算行高/列宽，再定位）、自由布局、
   最后空列回收、`auto_calc_item_size` 行高分配均正确；
5. `HFlowLayout` / `VFlowLayout` 的换行/换列判断、`justify_content` 的六种分布、
   对 100% 拉伸子控件的显式断言均正确；
6. 虚拟布局的数据项尺寸累加、可见区间计算、滚动条范围钳位均正确。

本轮确认并修复 2 类必要缺陷（4 个虚拟布局的滚动定位差一错误、2 个线性布局的零尺寸
控件统计口径不一致），并补充了覆盖布局行为的单元测试。

## 三、已修复的问题

### 1. 四个虚拟布局 `EnsureVisible(bToTop = true)` 存在差一错误

- 位置：
  - `VirtualVLayout.cpp`，`VirtualVLayout::EnsureVisible`
  - `VirtualHLayout.cpp`，`VirtualHLayout::EnsureVisible`
  - `VirtualVTileLayout.cpp`，`VirtualVTileLayout::EnsureVisible`
  - `VirtualHTileLayout.cpp`，`VirtualHTileLayout::EnsureVisible`
- 问题：
  - `bToTop == true` 表示“把目标项滚动到最上方/最左侧”，实现应为目标项的起始边界
    减去一个项步长；
  - 原实现使用 `GetElementsHeight/Width(rc, iIndex)`（即目标项之前所有项的总高度/宽度），
    再减去一个步长，得到的是**目标项前一项**的起始边界；
  - 因此调用后目标项不一定进入可视区，若目标项恰好是当前可视区的第一项，还会额外
    向上/向左回滚一项；
  - 同一函数中“向上/向左”分支使用的是 `GetElementsXxx(rc, iIndex + 1)` 再减步长，
    两处口径不一致，属于明确的缺陷而非设计取舍。
- 修复：四个实现的 `bToTop` 分支统一改为与“向上/向左”分支一致：
  `nNewPos = GetElementsXxx(rc, iIndex + 1); if (nNewPos >= step) { nNewPos -= step; }`。
- 影响：仅影响滚动定位结果，不改变任何布局尺寸计算；`bToTop == false` 的原有逻辑不变。

### 2. `HLayout` / `VLayout` 中零尺寸固定控件的边距仍参与拉伸空间计算

- 位置：
  - `HLayout.cpp`，`HLayout::ArrangeChildren`
  - `VLayout.cpp`，`VLayout::ArrangeChildren`
- 问题：
  - 布局规则 9 规定：控件宽度（高度）为 0 时忽略布局，其外边距不计入布局；
  - 实际实现中，`totalAllControlsCount++` 与 `totalAllMargin += ...` 在循环开头无条件
    执行，而固定控件在宽度（高度）为 0 时并不会被放到布局链上；
  - 由此产生两处偏差：
    1. 总间距按“控件数 - 1”计算（`totalSpacing`），多算了被忽略控件贡献的间距；
    2. 总边距 `totalAllMargin` 多算了被忽略控件的边距；
  - 这两项都会从“总可分配空间”中扣除，导致拉伸控件分配到的宽度（高度）偏小，
    布局无法到达容器右边界（下边界）；
  - 同时 `cxFixedTotal` / `cyFixedTotal` 只在尺寸大于 0 时累加，统计口径不一致。
- 修复：
  - 将 `totalAllControlsCount++` 与 `totalAllMargin += ...` 移入分类分支；
  - 固定控件仅在 `sz.cx > 0`（`sz.cy > 0`）时累计计数、边距与 `cxFixedTotal`
    （`cyFixedTotal`）；
  - 拉伸控件分支内始终累计计数与边距。
- 影响：仅在存在“0 宽/0 高固定控件”时改变拉伸空间分配；其他场景结果不变。

## 四、已核实、无需修改的点

- `Layout::SetFloatPosInternal` / `GetFloatPos`：容器空间不足时宽高退化为 0，
  min/max 钳制、`Validate()` 处理、`keep_float_pos` 的位置保持均正确；
- `Layout::SetChildMargin` 的负值钳制为 0，`TruncateToUInt16` 上限断言属于防御性编码；
- `Layout::DeflatePadding` 在扣除内边距后保证矩形有效，各布局的 `rc.Width()/Height()`
  不会为负；
- `HLayout` / `VLayout` 的 `EstimateLayoutSize`：拉伸子控件按最小尺寸计入，
  内边距口径与 `ArrangeChildren` 一致；
- `GridLayout::CalcActualGridSize`：行列均为 0 时按 `sqrt(gridCount) + 1` 估算列数，
  再向上取整计算行数，结果不小于 1；
- `GridLayout::CalcColumnWidths` / `CalcRowHeights`：固定网格尺寸优先，否则按内容区域
  等分并将余数分配给前若干列/行；
- `GridLayout::CalcMergedCellRects`：合并单元格重叠时返回 `false`，调用方据此中止布局；
- `HTileLayout::CalcTileRows`：以最大子项高度和纵向间距循环计算行数，边界条件正确；
- `HTileLayout` / `VTileLayout` 的 `scale_down` 等比缩放比例计算正确，超界时靠左/靠上
  对齐的分支正确；
- `HFlowLayout` / `VFlowLayout`：`justify_content` 的六种取值偏移量计算正确，
  对 100% 拉伸子控件的断言避免出现未定义行为；
- 虚拟布局的 `GetElementsHeight/Width`、`IsElementDisplay`、滚动条范围钳位在正常
  数据下结果正确。

## 五、建议后续处理（本轮未修改）

以下问题不影响当前正确性，或属于设计取舍/低频场景，本轮仅记录，未做修改：

1. `VirtualVLayout` / `VirtualHLayout` 的 `GetElementsWidth/Height` 中，
   `nCount == 0` 分支位于 `nCount <= 1` 分支之后，属于不可达死代码；
   空列表时返回一个项步长，当前调用方未依赖该返回值，无可见影响；
2. 虚拟布局的 `IsElementDisplay` 使用 owner 的 `GetWidth()/GetHeight()`（含内边距），
   而 `EnsureVisible` 传入的是 `GetPosWithoutPadding()`，存在内边距级别的判断偏差；
3. `HTileLayout` / `VTileLayout` 的 `AjustMaxItem` 使用 `GetChildMarginX()/2` 估算
   行列数，偏保守（会多创建少量控件），无正确性问题；
4. `GridLayout::EstimateLayoutSize` 基于可用尺寸等分单元格，当容器宽度为 auto 时
   不会按内容收缩，属于既有设计取舍；
5. `HTileLayout` / `VTileLayout` 的 `ArrangeChildren` 在估算阶段可能通过
   `SetItemSize` 修改 `m_szItem.cy`（`auto_calc_item_size` 场景），属于既有行为，
   若后续要求“估算无副作用”需要单独调整；
6. `VFlowLayout` 中一处注释写为“纵向滚动条”，按上下文应为“横向滚动条”，属注释笔误；
7. `Layout::SetChildMargin` 对超过 65535 的边距会触发 `ASSERT`，属于极端配置下的
   防御性约束。

## 六、单元测试

### 1. 新增测试

新增 `tests/test_layout.cpp`，并在 `tests/CMakeLists.txt` 中注册为 `test_layout`
可执行目标与 `layout` 测试项（静态库模式构建，与 `test_box` 的链接配置一致）。

测试不创建窗口、不启动消息循环，通过 `Box::SetPos` 驱动布局计算，直接断言子控件的
`GetRect()`；对固定的 0 尺寸、边距、拉伸比例等构造最小场景。

### 2. 覆盖内容

1. `Layout` 基类：布局类型与 `LayoutByActualAreaSize()` 判断、负边距钳制、
   `child_margin` / `child_margin_x/y` / `child_halign` / `child_valign` / `child_align`
   属性解析、未知属性返回 `false`、DPI 比例不变时边距不变化；
2. `HLayout`：固定 + 拉伸子控件的宽度分配、`child_margin_x` 间距、
   水平居中与子控件垂直居中、auto 尺寸估算；
   回归用例：0 宽固定控件 + 10 边距不会挤占拉伸控件空间（对应修复 2）；
3. `VLayout`：固定 + 拉伸子控件的高度分配、`child_margin_y` 间距、
   垂直居中与子控件水平居中、auto 尺寸估算；
   回归用例：0 高固定控件 + 10 边距不会挤占拉伸控件空间（对应修复 2）；
4. `GridLayout`：固定 2x2 行列时的单元格位置，默认 `scale_down == false` 时
   子控件填满单元格；
5. `HTileLayout` / `VTileLayout`：按容器高度/宽度计算行列数后的排列位置
   （分别固定为靠上/靠左对齐，避免默认居中带来的偏移）；
6. `HFlowLayout` / `VFlowLayout`：自动换行/换列，以及 `justify_content`
   （`kSpaceBetween`）的剩余空间分配。

### 3. 验证结果

- 构建：`python build\build.py build --config Debug --arch x64 --offline --no-deps --no-examples --target test_layout` 成功；
- 运行：`ctest --test-dir out\x64\Debug -C Debug -R "^layout$" --output-on-failure`
  100% 通过（79 项检查）；
- 静态库模式下整体 `ctest` 通过，未发现对 `Box` 等其他目录测试的影响。

### 4. 未纳入单测的部分

虚拟布局（`VirtualVLayout` / `VirtualHLayout` / `VirtualVTileLayout` /
`VirtualHTileLayout`）的 `EnsureVisible` 依赖 `VirtualListBox` 的数据提供者、
滚动条范围与 `ReArrangeChild` 的完整流程，构造单测需要模拟较多窗口/滚动条状态，
本轮未纳入；对应修复通过代码审查确认，`GetElementsHeight/Width` 的累加逻辑正确性
由既有虚拟列表功能间接覆盖。
