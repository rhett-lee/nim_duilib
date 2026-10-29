# Spec: D-9 CascadingSelect 级联选择器

## 1. 目标

新增独立控件 `CascadingSelect`（XML 节点名 `CascadingSelect`，头文件 `duilib/Control/CascadingSelect.h`），
解决规划项 D-9：现有 `Combo` 的 Tree 形态只能手工模拟两级，且没有"父项变更自动重置/过滤子项"语义。

完成后回写 `docs/misc/Development-Plan.md` 第 41 行（P2 汇总去掉 D-9）、D-9 表格行、D-9 任务行。

## 2. 需求要点

- 任意层级（2 级及以上）的级联数据；父列选中后，子列**自动过滤**为该父项的子集，深层的旧选择**自动重置**。
- 数据可通过 XML 静态配置，也可通过 C++ 动态增删。
- 多语言：路径每一段可以是语言 ID，语言切换后自动刷新文本且保持选择与滚动状态。
- 支持禁用路径（禁用节点及其子树不可选、置灰）。
- 交付完整的示例、双语文档、技能参考与主题 Class。

## 3. 设计决策

### 3.1 形态：内嵌多列（纯自绘 Control）

不实现下拉浮层。`CascadingSelect` 在自身矩形内绘制 N 列（N = 数据最大深度），
列随选择逐级"点亮"；未激活的列留空。理由：

- 语义与下拉级联完全一致，但无弹窗生命周期/焦点/跨平台问题；
- 可内嵌到任意 XML 面板，也可由业务侧放进 `Flyout` 得到下拉形态；
- 命中、滚动、DPI 均在一块画布内，验证成本低。

明确不做：下拉弹层、键盘导航、搜索过滤、异步数据。

### 3.2 数据模型

```cpp
struct Node {
    DString text;     // 显示文本
    DString textId;   // 语言 ID（items_id 模式；否则为空）
    std::vector<Node> children;
};
std::vector<Node> m_roots;
std::vector<int32_t> m_selection;    // 每级选中索引，-1 = 未选
std::vector<int32_t> m_columnScroll; // 每列首行偏移
```

XML 数据是"路径列表"：

- `items="华东/上海/浦东|华东/上海/徐汇|华南/深圳/南山"`：`|` 分隔路径，路径内用 `path_separator`（默认 `/`）分隔层级。
- `items_id`：同上，但每段是语言 ID，按 `GlobalManager::GetTextById` 解析；保留 ID 串用于语言切换重建。
- 相同前缀自动合并成同一节点，节点顺序按首次出现顺序。

### 3.3 状态派生与重置语义

- 第 `i` 列的可见项 = `selection[0..i-1]` 对应节点的 children；前缀未选满时该列未激活（空白）。
- 点击第 `i` 列某行：`selection[i] = 行索引`，并把 `selection` 截断为 `i+1` 项（**深层自动重置**）。
- 选择变化（任一列的选中项或截断发生变化）时派发一次事件；值未变化不派发。
- 禁用节点：`disabled_paths` 中路径等于该节点文本路径 / ID 路径，或作为其前缀（即禁用整棵子树）。
- 语言切换只重建 `text`，树的形状与索引不变，选择与列滚动原样保留。

### 3.4 绘制与交互

- 列宽：`column_width > 0` 用固定值（DPI 缩放），否则等分控件宽度（末列吃余量）。
- 行高：`row_height`（默认 28，DPI 缩放）；可见行数 = 内容高度 / 行高。
- 行背景：选中 `selected_bg_color`；悬停（未选中）`hover_bg_color`；均为圆角矩形。
- 文本：`font`，选中文本 `selected_color`，普通 `item_color`，禁用 `disabled_color`，`TEXT_END_ELLIPSIS` 截断。
- 有子项的节点在行右侧绘制矢量向右小三角（`chevron_color`），不依赖字体字形。
- 列之间 1px 分隔线 `divider_color`。
- 列内容超出可见行数：滚轮在该列上滚动（每格 3 行），右缘绘制细滚动条 `scrollbar_color`。
- 鼠标悬停可交互行时手型光标；控件禁用 / `mouse_enabled=false` 时全部不可交互。
- 颜色默认值全部为语义色名，无写死色值。

### 3.5 事件

- 复用 `kEventValueChanged`，`wParam = 4`（级联选择模式标记）。
- `pEventData` 指向栈上临时 `CascadingSelectValue`：

```cpp
struct CascadingSelectValue
{
    std::vector<int32_t> indexes; // 每级选中索引，-1 = 未选
    DString text;                 // value_separator 连接的显示文本
    DString path;                 // path_separator 连接的完整路径
};
```

仅本次回调有效，文档注明不可保存指针。

## 4. 精确改动清单

### 4.1 控件

1. 新增 `duilib/Control/CascadingSelect.h`
   - `struct CascadingSelectValue`；
   - `class DUILIB_API CascadingSelect : public Control`，`typedef Control BaseClass`；
   - public：`GetType` / `SetAttribute` / `PaintStateImages` / `OnLanguageChanged` /
     `ButtonDown` / `MouseMove` / `MouseLeave` / `MouseWheel`；
   - 数据：`SetItems` / `SetItemIds` / `AddPath` / `ClearItems` / `GetDepth` / `GetItemCount` / `GetItemText`；
   - 选择：`GetSelection` / `SetSelection(indexes, bTriggerEvent)` / `SetSelectionByText` /
     `ClearSelection(bTriggerEvent)` / `GetSelectionText` / `GetSelectionPath`；
   - 禁用：`SetDisabledPaths` / `GetDisabledPaths` / `IsItemDisabled`；
   - 外观：`SetPathSeparator/SetValueSeparator/SetColumnWidth/SetRowHeight/SetTextPadding/SetFont`
     及 6 个颜色 Set/Get；
   - 事件：`AttachSelectionChanged`。
2. 新增 `duilib/Control/CascadingSelect.cpp`
   - 属性解析（主名 + 无下划线别名）、路径建树、选择钳位、事件派发、布局/命中/绘制。

### 4.2 注册（5 处）

| 文件 | 改动 |
| :--- | :--- |
| `duilib/duilib_defs.h` | `DUI_CTR_CASCADING_SELECT (_T("CascadingSelect"))` |
| `duilib/Core/WindowBuilder.cpp` | include + `{DUI_CTR_CASCADING_SELECT, ...new CascadingSelect(pWindow)}` |
| `duilib/duilib.vcxproj` | `Control\CascadingSelect.cpp/.h` |
| `duilib/duilib.vcxproj.filters` | 归入 `Control` 过滤器 |
| `duilib/duilib.h` | `#include "Control/CascadingSelect.h"` |

### 4.3 主题

`bin/resources/themes/default/global.xml` 新增：

```xml
<Class name="cascading_select" font="system_regular_14" row_height="28" text_padding="10"
       item_color="text_default" selected_color="color_accent" disabled_color="text_disabled"
       hover_bg_color="bg_list_item_hovered" selected_bg_color="bg_list_item_selected"
       divider_color="border_split_level1" chevron_color="color_gray" scrollbar_color="color_gray"/>
```

### 4.4 示例

1. `examples/controls/CascadingSelectForm.h/.cpp`：两个控件（基础三级 + 自定义分隔符/禁用/固定列宽），
   事件回显 + 选中 / 清空 / 追加路径 / 禁用上海 / 恢复可用 / 读取选择 六个按钮。
2. `bin/resources/themes/default/controls/cascading_select.xml`：标准标题栏 + 提示 + 两张卡片。
3. `bin/resources/themes/default/controls/test.xml`：新增 `GroupHBox text="CascadingSelect"` 与入口按钮。
4. `examples/controls/TestForm.h/.cpp`：`ShowCascadingSelectWindow()` + 绑定入口。
5. `examples/controls/controls.vcxproj` / `.filters`：登记新文件。
6. `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增 `STRID_CONTROLS_CASCADING_SELECT_TEST`、
   窗口/提示/卡片、按钮、地区/系统层级文案，两份键集合一致。

### 4.5 文档

| 文件 | 改动 |
| :--- | :--- |
| `docs/Control.md` | Breadcrumb 之后新增 `## CascadingSelect 控件（级联选择器）` |
| `docs/Control.en.md` | 新增 `## Attributes of CascadingSelect (cascading selector)` |
| `docs/Summary.md` / `.en.md` | 控件表新增一行（锚点指向新章节） |
| `README.md` / `README.en.md` | 功能列表第 52 条 |
| `.trae/skills/nim-duilib/references/api-reference.md` | 速查行 + 属性小节 |
| `.workbuddy/skills/nim-duilib/references/api-reference.md` | 与 `.trae` 版本同文本覆盖 |
| `.claude/docs/nim-duilib-llm-reference.md` | 速查行 |
| `docs/misc/Development-Plan.md` | P2 汇总去掉 D-9；D-9 表格行 ✅；任务行 `- [x]` + 日期 |

## 5. 验证

1. Debug|x64 编译：

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  "c:\develop\nim_duilib\build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo
```

2. 文档校验：

```powershell
python3 .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang
```

退出码必须为 0。

## 6. 验收标准

- [ ] XML `<CascadingSelect class="cascading_select" items="..."/>` 可创建，`FindControl` 可取到。
- [ ] 父列换选后，子列内容立即过滤、深层选择被清空。
- [ ] 点击叶子后 `AttachSelectionChanged` 只派发一次，`pEventData` 的 indexes/text/path 正确。
- [ ] `items_id` 在中/英切换后文本刷新，选择与滚动保持不变。
- [ ] `disabled_paths` 命中节点及其子树置灰且不可点。
- [ ] 列溢出时滚轮可滚动，滚动条比例正确；列宽固定/等分两种模式均正确。
- [ ] 浅色/深色主题、100%/125%/150% DPI 下布局与配色正常。
- [ ] 空数据、单级数据、越界选择、重复设置均不崩溃。
