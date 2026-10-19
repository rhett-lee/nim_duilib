# 树表格 TreeGrid 控件 Checklist（B9）

## 骨架与注册
- [x] `DUI_CTR_TREE_GRID` 常量存在，且 `<TreeGrid>` XML 标签可通过 `WindowBuilder` 工厂创建。
- [x] `kEventTreeGridCheckChanged` 事件枚举已定义（并在 `EventArgs.cpp` 注册字符串映射）。
- [x] `duilib.vcxproj` / `.filters` 正确包含 `TreeGrid.cpp` / `TreeGrid.h`，Debug|x64 可编译。
- [x] `examples/controls` 工程正确包含 `TreeGridForm.h` / `.cpp`。

## 结构分层
- [x] `TreeGrid` 内部含表头（固定）与列表体（可滚动）两个子控件，表头不随行滚动。
- [x] 列表体为 `VListBox`，纵向滚动条可用；滚动时表头保持不动。
- [x] 表头右侧预留滚动条宽度，末列不与滚动条重叠。

## 列与对齐
- [x] `column_title` / `column_width` / `column_id` / `column_align` 解析正确，列数与标题 / 宽度一致。
- [x] 表头标题单元格与数据行单元格**逐列左边界对齐**（含滚动后）。
- [x] `AddColumn` 在已有行时能正确重建表头与所有行的单元格。
- [x] `SetColumnWidthById` 同步修改表头与所有行的列宽。
- [x] `SetColumnTitleById` 立即刷新表头标题。
- [x] 列总宽不足 / 超出控件宽度时按设计裁剪，不出现错位。

## 节点模型
- [x] `AddNode` / `RemoveNode` / `RemoveAllNodes` 正确维护父子关系与深度。
- [x] `SetCellText` 仅刷新目标单元格，不影响其它单元格与行。
- [x] `TreeGridNode::GetDepth` 与缩进一致（一级子节点深度为 1）。
- [x] `RemoveNode` 移除含子孙的节点后，行列表无残留。

## 展开 / 收起
- [x] 点击展开开关切换该节点的展开态，子孙行按设计显示 / 隐藏。
- [x] 展开 / 收起派发 `kEventExpand` / `kEventCollapse`，参数为对应节点。
- [x] 深层节点文本缩进随 `indent` 递增，**其它列位置不受影响**。
- [x] `ExpandNode` 级联展开祖先使目标可见；`EnsureNodeVisible` 滚动到可见。
- [x] 无子节点的节点不显示展开开关。

## 行选择
- [x] `multi_select="false"` 时仅单行可选；`true` 时选中不互斥（沿用 `ListBox` 多选语义）。
- [x] `SelectNode` / `IsNodeSelected` / `GetSelectNodes` / `SelectAllNodes` / `ClearSelectNodes` 行为正确。
- [x] 选择变化派发 `kEventSelect` / `kEventUnSelect` / `kEventSelChanged`（`pEventData` 为节点）。
- [x] 展开 / 收起后选中态不串行、不错位。

## 复选框级联
- [x] `check_mode="true"` 时行首显示复选框；`false` 时不显示。
- [x] 勾选父节点级联其全部子孙为勾选；取消则级联取消。
- [x] 仅部分子节点勾选时，父节点（及祖先）显示为三态"部分选"。
- [x] `check_cascade="false"` 时勾选仅影响自身，不级联。
- [x] 勾选变化派发 `kEventTreeGridCheckChanged`，`pEventData` 为 `TreeGridCheckValue`。
- [x] `GetCheckedNodes` / `SetAllNodesChecked` 结果正确。

## 外观与主题
- [x] 表头 / 单元格字体与颜色来自 `tree_grid_header_cell` / `tree_grid_cell` 类，**C++ 无写死色值**。
- [x] 行常态 / 悬停 / 选中背景由 `tree_grid_row` 类驱动。
- [x] 展开 / 收起箭头走 SVG 图片 + `svg_replace_colors` 映射语义色（非 `IPath` 自绘）。
- [x] 展开 / 收起与三态复选框 SVG 复用既有 `${PATH_TREE}` / `${PATH_CHECKBOX}`，可被引用（不新增图片）。
- [x] 切换主题后配色自动跟随（全部走语义色 / `svg_replace_colors`）。

## 交互与禁用态
- [x] 点击展开开关不改变行选择；点击复选框不改变展开态。
- [x] 点击行其它区域选中该行并派发 `kEventClick`（`pEventData` = 节点）。
- [x] `Left` / `Right` 收起 / 展开当前选中节点。
- [x] `IsEnabled()==false` 时置灰且不响应交互。

## 演示与示例
- [x] `controls/tree_grid.xml` 演示窗口可正常打开，含配置好列与初始数据的 `TreeGrid`。
- [x] `test.xml` 的 `tree_grid_btn` 能打开 TreeGrid 演示窗口。
- [x] 示例提供多选 / 勾选开关与事件回显标签。
- [x] `zh_CN-examples.txt` / `en_US-examples.txt` 的 `STRID_TREE_GRID_*` 文案齐全，切换语言正常。

## 编译与运行时
- [x] Debug|x64 编译无错误、无新增警告（`duilib` 与 `examples/controls` 均已编译通过）。
- [ ] 运行时截图核对：列对齐 / 展开收起 / 深层缩进 / 单选多选 / 复选级联三态 / 主题色跟随（交互视觉核对由人工手动测试）。

## 文档与收尾
- [x] `docs/Control.md` / `docs/Control.en.md` 均有 TreeGrid 小节（属性表 + 说明 + XML 示例 + C++ 接口）。
- [x] 两份 `api-reference.md` 与 `nim-duilib-llm-reference.md` 已同步 TreeGrid。
- [x] `verify_docs.py` 全量扫描无漂移。
- [x] `Project-Audit.md` B9 与 `Development-Plan.md`（按需再评估表）状态已更新。
- [ ] `ListCtrl` / `TreeView` 与既有示例无回归（启动冒烟通过；由人工手动测试确认）
