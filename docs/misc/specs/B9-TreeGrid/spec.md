# 树表格 TreeGrid 控件 Spec（B9）

## Why
`docs/misc/Project-Audit.md` 的 **B9** 登记「树表格 TreeGrid / 列筛选——ListCtrl 规划中已明确取消（含内嵌树、分组折叠、L3 筛选），若出现企业报表需求需重新评估」，优先级「低（按需）」、状态「待办」。`docs/misc/Development-Plan.md` 第 526 行同步标注为「按需再评估」。

需要澄清的历史结论：ListCtrl 专项规划（[ListCtrl-Roadmap.md](../../ListCtrl-Roadmap.md) §四 / §六）已把「表格内嵌树（C5）」「行分组折叠（C5）」「多级表头（C6）」「单元格合并（C4）」「行拖拽换序（C7）」「行筛选 / 表头筛选器（L3）」**全部取消**，理由是"需 ListCtrl 的表头与行布局模型整体升级、改动面过大"。

因此本项**不回到被否决的路线**（不改动 `ListCtrl` 内部），而是**新增一个独立控件 `TreeGrid`**：把「层级树 + 多列表格」的语义收进一个自成一体的控件，复用框架既有的 `Box` / `ListBox` / `ListBoxItem` / `Label` / `CheckBox` 基础设施。**列筛选（L3 语义）不在本项范围内**（沿用「不实施」结论）。

用户决策（已确认）：
- 交付形态：**新增独立控件 `TreeGrid`**（节点 `TreeGrid`），不改动 `ListCtrl`。
- 行选择能力：**单选 / 多选 + 行首复选框（父子勾选级联，父节点三态）**。

## What Changes
### 1. 新增 `TreeGrid` 控件（继承 `VBox`，内部组合表头与列表体）
- **结构**：`TreeGrid : VBox`，内部由两个子控件组成——
  - `TreeGridHeader`（`HBox`）：列标题行，**固定不滚动**，每列一个标题单元格（`Label`），宽度与列宽一致；
  - `TreeGridBody`（`VListBox`，即纵向 `ListBox`）：数据行区，自带纵向滚动条（`ListBox` 继承自 `ScrollBox`）。
- **列**：列由列ID、标题、宽度、（可选）对齐方式定义；表头与数据行使用**同一套列宽**，因此天然对齐，无需坐标系 hack。
- **行**：每个数据行是一个 `TreeGridRow`（`ListBoxItemH`，即 HBox 版列表项），其子控件为 N 个等宽单元格（`Label`）；「树列」单元格内依次放置 `[缩进占位][展开开关][复选框][列文本]`。
- **层级**：节点树（`TreeGridNode`）由控件维护；展开/收起通过显示 / 隐藏该节点的子孙行实现（`Control::SetVisible`，`VLayout` 会自动跳过不可见子项，与 `TreeView` 同机制）。
- **展开开关**：复用 `CheckBox` 的两态图片（`normal_image` = 收起箭头，`selected_normal_image` = 展开箭头），以「勾选态 = 已展开」表达，点击即切换展开。
- **复选级联**：行首 `CheckBox` 父节点呈三态（全选 / 全不选 / 部分选），勾选/取消父节点级联其全部子孙；子节点状态变化回溯修正祖先。
- **选择**：复用 `ListBox` 的单选 / 多选选择模型（`multi_select`），并派发既有选择事件。
- **外观**：全部由主题类驱动（`tree_grid`、`tree_grid_header_cell`、`tree_grid_cell`、`tree_grid_row`、`tree_grid_expand`、`tree_grid_check`），展开/收起箭头走 SVG 图片 + `svg_replace_colors` 映射语义色，**C++ 侧不写死色值**。

### 2. 端到端接线与文档
- 类型常量（`DUI_CTR_TREE_GRID`）、工厂注册、构建工程、演示窗口、示例入口、多语言文案。
- 文档同步：`docs/Control.md`、`docs/Control.en.md`、两份 `api-reference.md`、`nim-duilib-llm-reference.md`。
- **非 BREAKING**：全部为新增，不改动既有控件行为（尤其不改动 `ListCtrl` / `TreeView`）。

## Impact
- Affected specs：`Project-Audit.md` B9；`Development-Plan.md` §附「TreeGrid …按需再评估」。
- Affected code：见下方「改动文件清单」。
- **不影响**：`ListCtrl`（含 Report / List / Icon 视图）、`TreeView` / `TreeNode` 行为一律不变。

## ADDED Requirements

### Requirement: TreeGrid 树表格控件
系统 SHALL 提供 `<TreeGrid>` 控件，在同一控件内呈现「可展开 / 收起的层级行 + 多列表格」，表头与数据行的列 SHALL 保持像素级对齐，行的选择 SHALL 支持单选 / 多选，行首 SHALL 支持带父子级联的复选框。

#### Scenario: 列定义与对齐
- **WHEN** 定义 3 列（标题 Name / Size / Time，宽度 200 / 100 / 160）
- **THEN** 表头显示 3 个标题单元格，任一行内的 3 个单元格左边界与表头逐列对齐。

#### Scenario: 展开 / 收起
- **WHEN** 某父节点有 2 个子节点，用户点击其展开开关
- **THEN** 2 个子节点行变为可见（含更深层子孙，按其自身展开状态），开关图片切换为"已展开"，并派发展开事件；再次点击则收起。

#### Scenario: 深层缩进
- **WHEN** 节点层级为 1 / 2 / 3
- **THEN** 树列单元格内的文本起始位置随层级递增缩进（缩进量由 `indent` 属性决定，DPI 缩放），其它列位置不受影响。

#### Scenario: 单选 / 多选
- **WHEN** `multi_select="false"`，用户点击某行
- **THEN** 仅该行处于选中态；**WHEN** `multi_select="true"`，配合 `Ctrl` / `Shift` 点击可多选，并派发选择变化事件。

#### Scenario: 复选框级联
- **WHEN** `check_mode="true"`，用户勾选某父节点
- **THEN** 其全部子孙节点变为勾选；父节点自身显示为全选态。
- **WHEN** 用户仅勾选某父节点的部分子节点
- **THEN** 父节点显示为"部分选"（三态），且其祖先同步为"部分选"。

#### Scenario: 列文本设置
- **WHEN** 调用 `SetCellText(node, columnId, text)`
- **THEN** 该行对应列的单元格立即显示新文本，不影响其它单元格与其它行。

#### Scenario: 动态增删节点
- **WHEN** 通过 `AddChildNode` / `RemoveChildNode` 增删节点
- **THEN** 行列表与其子孙行按当前展开状态正确刷新，选中态与勾选态保持一致。

#### Scenario: 主题切换
- **WHEN** 运行时切换主题
- **THEN** 表头 / 行背景 / 选中态 / 展开箭头颜色随主题类与 `svg_replace_colors` 自动变化，无需改动 C++ 代码。

## 设计与约定

### 结构分层
```
TreeGrid (VBox, 节点名 TreeGrid)
├── TreeGridHeader (HBox)           // 表头：N 个 Label 标题单元格，宽度=列宽；固定不滚动
└── TreeGridBody  (VListBox)        // 列表体：自带纵向滚动条；每项为一个 TreeGridRow
     └── TreeGridRow (ListBoxItemH) // 数据行：HBox，含 N 个等宽单元格
          ├── cell[0] (HBox, 宽=w0) // 树列单元格
          │    ├── 缩进占位 (Control, 宽 = depth * indent)
          │    ├── 展开开关 (CheckBox, class=tree_grid_expand) —— 仅"有子节点"时显示
          │    ├── 行复选框 (CheckBox, class=tree_grid_check) —— 仅 check_mode 开启时显示
          │    └── 列文本 (Label, 宽=stretch, class=tree_grid_cell)
          └── cell[k] (Label, 宽=wk, class=tree_grid_cell)  // 其余列
```
- 表头与行使用**同一列宽数组**，因此逐列对齐；`TreeGridBody` 的纵向滚动条会占用右侧宽度，表头右侧预留同等宽度（`TreeGrid` 内部按滚动条宽度做补偿，避免末列与滚动条重叠）。
- 行高由 `row_height`（默认 28，DPI 缩放）统一决定；表头高度由 `header_height`（默认 30）决定。

### TreeGrid：XML 属性
| 属性名称 | 默认值 | 类型 | 说明 |
| :--- | :--- | :--- | :--- |
| column_title |  | 字符串列表 | 列标题，`\|` 分隔，如 `Name\|Size\|Time` |
| column_width |  | 整数列表 | 列宽（像素，DPI 缩放），`\|` 分隔，与 `column_title` 一一对应；缺省项按 `default_column_width`（默认 120） |
| column_id |  | 整数列表 | 列ID（可选），`\|` 分隔；缺省时按列索引（0 起）作为列ID |
| column_align |  | 字符串列表 | 列对齐，`left`（默认）/ `center` / `right`，`\|` 分隔 |
| default_column_width | 120 | int | 未显式指定宽度时的列宽（DPI 缩放） |
| tree_column | 0 | int | 树列所在列索引（承载缩进 / 展开开关），默认第 0 列 |
| indent | 20 | int | 每层缩进量（像素，DPI 缩放） |
| header_height | 30 | int | 表头高度（像素，DPI 缩放） |
| row_height | 28 | int | 数据行高（像素，DPI 缩放） |
| show_header | true | bool | 是否显示表头 |
| multi_select | false | bool | 是否允许多选（行选择） |
| check_mode | false | bool | 是否显示行首复选框（并启用父子级联） |
| check_cascade | true | bool | 复选框是否启用父子级联（`check_mode` 开启时有效） |
| expand_image_class | tree_grid_expand | string | 展开 / 收起开关引用的皮肤 Class |
| check_image_class | tree_grid_check | string | 行复选框引用的皮肤 Class |
| header_cell_class | tree_grid_header_cell | string | 表头标题单元格引用的皮肤 Class |
| cell_class | tree_grid_cell | string | 数据单元格引用的皮肤 Class |
| row_class | tree_grid_row | string | 数据行引用的皮肤 Class |

`TreeGrid` 继承 `VBox` 的布局属性；表头 / 列表体为内部子控件，**用户不在 XML 里直接书写**。

### TreeGrid：C++ 接口
```cpp
/** 列定义 */
struct TreeGridColumn {
    size_t  nColumnId = 0;              // 列ID（默认取列索引）
    DString strTitle;                   // 列标题
    int32_t nWidth   = 120;             // 列宽（DPI 缩放后）
    DString strAlign = _T("left");      // 对齐：left / center / right
};

/** 节点勾选状态（三态） */
enum class TreeGridCheck { UnCheck, CheckedAll, CheckedPartly };

/** 复选框状态变化数据（kEventTreeGridCheckChanged 的 pEventData） */
struct TreeGridCheckValue {
    TreeGridNode* pNode = nullptr;      // 发生变化的节点
    bool          bChecked = false;     // 该节点当前是否勾选
};

/* ---- 列管理 ---- */
void    AddColumn(const TreeGridColumn& column);                 // 追加一列（内部重建表头与现有行）
size_t  GetColumnCount() const;
bool    GetColumn(size_t nColumnIndex, TreeGridColumn& column) const;
bool    SetColumnWidthById(size_t nColumnId, int32_t nWidth);     // 改列宽（同步表头与所有行）
int32_t GetColumnWidthById(size_t nColumnId) const;
bool    SetColumnTitleById(size_t nColumnId, const DString& title);
void    ClearColumns();

/* ---- 节点管理 ---- */
TreeGridNode* GetRootNode() const;                               // 虚拟根节点（不可见）
TreeGridNode* AddNode(TreeGridNode* pParent /*nullptr=根*/);     // 追加子节点并刷新行
bool    RemoveNode(TreeGridNode* pNode);                         // 移除节点（含子孙）
void    RemoveAllNodes();

/* ---- 数据设置 ---- */
bool    SetCellText(TreeGridNode* pNode, size_t nColumnId, const DString& text);
DString GetCellText(TreeGridNode* pNode, size_t nColumnId) const;
bool    SetNodeRowHeight(TreeGridNode* pNode, int32_t nHeight);  // 逐行行高（可选，0=用全局 row_height）

/* ---- 展开 / 收起 ---- */
bool    SetNodeExpand(TreeGridNode* pNode, bool bExpand, bool bTriggerEvent = false);
bool    IsNodeExpand(TreeGridNode* pNode) const;
bool    ExpandNode(TreeGridNode* pNode);                         // 级联展开祖先 + 自身
bool    EnsureNodeVisible(TreeGridNode* pNode);                  // 保证节点行可见（必要时滚动）

/* ---- 选择 ---- */
void    SetMultiSelect(bool bMultiSelect); bool IsMultiSelect() const;
size_t  GetSelectNodeCount() const;
TreeGridNode* GetSelectNodeAt(size_t nIndex) const;
std::vector<TreeGridNode*> GetSelectNodes() const;
bool    IsNodeSelected(TreeGridNode* pNode) const;
bool    SelectNode(TreeGridNode* pNode, bool bSelect = true, bool bTriggerEvent = true);
void    SelectAllNodes(bool bSelect = true);
void    ClearSelectNodes();

/* ---- 复选框（级联） ---- */
bool    IsCheckMode() const;      void SetCheckMode(bool bCheckMode);
bool    IsCheckCascade() const;   void SetCheckCascade(bool bCascade);
bool    IsNodeChecked(TreeGridNode* pNode) const;
bool    SetNodeChecked(TreeGridNode* pNode, bool bChecked, bool bTriggerEvent = true); // 含级联
TreeGridCheck GetNodeCheckState(TreeGridNode* pNode) const;       // 三态
void    SetAllNodesChecked(bool bChecked);
std::vector<TreeGridNode*> GetCheckedNodes() const;

/* ---- 事件 ---- */
void AttachNodeExpanded(const EventCallback& cb, EventCallbackID id = 0);     // kEventExpand，WPARAM 为节点指针
void AttachNodeCollapsed(const EventCallback& cb, EventCallbackID id = 0);    // kEventCollapse
void AttachNodeClick(const EventCallback& cb, EventCallbackID id = 0);         // kEventClick，pEventData 为 TreeGridNode*
void AttachNodeSelect(const EventCallback& cb, EventCallbackID id = 0);        // kEventSelect（选中）
void AttachNodeUnSelect(const EventCallback& cb, EventCallbackID id = 0);      // kEventUnSelect
void AttachSelectChanged(const EventCallback& cb, EventCallbackID id = 0);     // kEventSelChanged
void AttachCheckChanged(const EventCallback& cb, EventCallbackID id = 0);      // kEventTreeGridCheckChanged，pEventData 为 TreeGridCheckValue
```

### TreeGridNode：C++ 接口（数据节点）
```cpp
class DUILIB_API TreeGridNode {
public:
    TreeGrid*     GetTreeGrid() const;
    TreeGridNode* GetParentNode() const;
    size_t        GetDepth() const;                 // 根的一级子节点为 1

    TreeGridNode* AddChildNode(TreeGridNode* pNode);          // 追加子节点（转发到 TreeGrid）
    bool          RemoveChildNode(TreeGridNode* pNode);
    void          RemoveAllChildNodes();
    size_t        GetChildNodeCount() const;
    TreeGridNode* GetChildNode(size_t iIndex) const;
    size_t        GetChildNodeIndex(TreeGridNode* pNode) const;
    size_t        GetDescendantNodeCount() const;             // 递归子孙总数（不含自身）
    bool          IsChildOf(TreeGridNode* pNode) const;       // 是否为其子孙

    bool IsExpand() const;   bool SetExpand(bool bExpand);    // 转发 + 刷新
    bool IsChecked() const;  bool SetChecked(bool bChecked);  // 转发 + 级联
    TreeGridCheck GetCheckState() const;                      // 三态
    bool IsSelected() const; bool SetSelected(bool bSelected);// 转发到行选择

    bool          SetCellText(size_t nColumnId, const DString& text);
    DString       GetCellText(size_t nColumnId) const;
    void          SetUserData(void* pData);  void* GetUserData() const;   // 业务挂载数据
};
```
说明：`TreeGridNode` 是**数据节点**（不继承 `Control`），行 UI（`TreeGridRow`）由 `TreeGrid` 内部按节点维护；用户通过 `TreeGridNode` 操作数据。

### 绘制与配色约定（主题驱动）
- 表头标题、数据单元格文本统一使用 `Label` 渲染，字体 / 颜色来自 `tree_grid_header_cell` / `tree_grid_cell` 类（`font` / `text_color` 等），**C++ 不写死**。
- 行背景（常态 / 悬停 / 选中）来自 `tree_grid_row` 类（`bkcolor` / `hot_bkcolor` / `selected_bkcolor` 或对应状态图片），复用 `ListBoxItem` 的既有选择态绘制。
- 展开 / 收起箭头为 SVG 图片（复用主题既有 `${PATH_TREE}/collapse_normal.svg` / `expand_normal.svg`），经 `svg_replace_colors` 映射语义色，随主题切换。
- 复选框图片复用主题既有的三态勾选框图片（`${PATH_CHECKBOX}` 下 unchecked / checked / mixed，`svg_replace_colors` 映射 `border_svg_image`）；**不新增图片资源**。

### 交互约定
- 点击「展开开关」→ 切换节点展开态（并派发 `kEventExpand` / `kEventCollapse`），不改变行选择。
- 点击「行复选框」→ 切换勾选态并级联（派发 `kEventTreeGridCheckChanged`），不影响展开态。
- 点击行其它区域 → 选中该行（单选 / 多选，派发既有选择事件）。
- 键盘：沿用 `ListBox` 的方向键导航；`Left` / `Right` 收起 / 展开当前节点（当有选中行时）。
- `IsEnabled()==false` 时整体降透明且不响应交互。

## 改动文件清单
1. `duilib/duilib_defs.h`：新增 `DUI_CTR_TREE_GRID (_T("TreeGrid"))` 与事件 `kEventTreeGridCheckChanged`。
2. `duilib/Control/TreeGrid.h` / `.cpp`（新增）：`TreeGrid`（VBox 派生）、`TreeGridNode`、`TreeGridColumn`、`TreeGridCheck`、`TreeGridCheckValue`；列定义、节点树、展开 / 收起、选择、复选级联、表头 / 行同步、事件派发。
3. `duilib/Core/WindowBuilder.cpp`：include 头文件并在 `createControlMap` 注册 `DUI_CTR_TREE_GRID`。
4. `duilib/duilib.h`：汇总头引入 `Control/TreeGrid.h`。
5. `duilib/duilib.vcxproj` / `.filters`：新增 `TreeGrid.cpp` / `TreeGrid.h` 条目。
6. `bin/resources/themes/default/global.xml` 的既有 `${PATH_TREE}` / `${PATH_CHECKBOX}` 图片资源：**直接复用**（展开 / 收起箭头 + 三态复选框），不新增 `public/tree_grid/` 目录与图片。
7. `bin/resources/themes/default/global.xml`：**不新增** `<Var name="PATH_TREE_GRID">`；新增 `<Class name="tree_grid">`（根）、`tree_grid_header_cell`、`tree_grid_cell`、`tree_grid_row`、`tree_grid_expand`、`tree_grid_check`（后两者引用既有图片 Var 与三态图）。
8. `bin/resources/themes/default/controls/tree_grid.xml`（新增）：演示窗口（含一个配置好列与初始数据的 `TreeGrid`）。
9. `bin/resources/themes/default/controls/test.xml`：新增 `tree_grid_btn` 入口按钮。
10. `examples/controls/TreeGridForm.h` / `.cpp`（新增）：示例逻辑（构建 3 层节点树、填充列文本、展开 / 收起与勾选事件回显、多选切换）。
11. `examples/controls/TestForm.h` / `.cpp`：新增 `ShowTreeGridWindow()` 与 `tree_grid_btn` 处理。
12. `examples/controls/controls.vcxproj` / `.filters`：新增 `TreeGridForm` 条目。
13. `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增 `STRID_TREE_GRID_*` 文案。
14. `docs/Control.md`、`docs/Control.en.md`：新增「TreeGrid 控件（树表格）」小节。
15. `.trae/skills/nim-duilib/references/api-reference.md`、`.workbuddy/skills/nim-duilib/references/api-reference.md`：同步 TreeGrid 条目。
16. `.claude/docs/nim-duilib-llm-reference.md`：速查行补充 TreeGrid。
17. `docs/misc/Project-Audit.md` B9、`docs/misc/Development-Plan.md`（按需再评估表）：标记完成。

## 验证方式
- **编译**：`build/examples_no_cef.sln` → Configuration `Debug` / Platform `x64` 编译通过（无错误）。
- **资源**：展开 / 收起与三态复选框复用既有 `${PATH_TREE}` / `${PATH_CHECKBOX}` 图片，`tree_grid*` 类在 `global.xml` 中可被引用，`svg_replace_colors` 映射语义色。
- **运行时**（人工手动测试 + 截图核对）：
  1. 演示窗口打开，表头 3 列与数据行逐列对齐，无错位；
  2. 点击展开开关，子行出现 / 收起，箭头图片切换，事件回显；
  3. 深层节点文本缩进递增，其它列位置不变；
  4. 单击选中行、`Ctrl` / `Shift` 多选生效，无选中态串行；
  5. 勾选父节点级联子孙，部分勾选时父节点呈三态，事件回显；
  6. 切换主题后表头 / 行 / 箭头配色跟随。
- **文档**：`python .trae/skills/nim-duilib/scripts/verify_docs.py --repo "C:/develop/nim_duilib"` 全量扫描无漂移。
- **回归**：`ListCtrl` 与 `TreeView` 行为不变，既有示例无异常。

## 非目标（Out of Scope）
- **列筛选 / 表头筛选器**（L3 语义）：沿用 ListCtrl 规划中的「不实施」结论，本项不做。
- **列宽拖拽调整 / 列拖拽换序 / 点击列头排序**：本项不做（可作为后续增强）。
- **单元格合并（span）/ 多级表头**：不做（同 ListCtrl C4 / C6 取消结论）。
- **虚拟化（大数据量）**：本项使用 `ListBox`（非虚表），按节点创建行控件；超大行数（十万级）场景不在本期范围，后续如需可基于 `VirtualListBox` 增强。
- **树单元格富内容（嵌入任意子控件 / 图标列）**：本期列单元格仅支持文本；图标 / 富内容留待后续。
- **列冻结 / 横向滚动**：列总宽超出控件宽度时按裁剪处理，不做横向滚动（后续可增强）。
