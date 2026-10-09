# NavigationView 侧边栏导航控件 实现计划

> 对应 Roadmap 第二梯队 #8：汉堡按钮 + 分组导航项 + 页头 + 内容区联动（参考 WinUI NavigationView）。

## 一、目标与范围（v1）

做一个**内聚**的侧边导航控件，避免业务侧手工组合 TreeView+TabBox：

1. 左侧导航窗格 Pane：顶部汉堡按钮（+窗格标题）、可滚动的导航项列表（支持分组标题/分隔线）、
   底部可选"设置"项。
2. 右侧内容区：内置 TabBox，点导航项切换页面；顶部可选页头（标题随选中项联动）。
3. 汉堡按钮在"展开（图标+文字，宽 pane\_width）/紧凑（仅图标，宽 compact\_pane\_width）"间切换。
4. 语义色自绘，深/浅色主题与 DPI 自适应；多语言（text\_id）。
5. 选中事件 `kEventTabSelect`（复用，不新增事件枚举）。

v1 明确**不做**：Top/Minimal 显示模式、自动折叠（按窗口宽度）、选中指示条动画、
窗格宽度拖拽、Badge 角标（后续可叠加 Badge 控件）、多级子菜单（用分组标题即可）。

## 二、新增控件与 XML 用法

新增两个控件类型：

* `NavigationView`（容器，根节点）

* `NavigationViewItem`（导航项，仅作为 NavigationView 的直接子节点）

业务 XML：

```xml
<NavigationView name="nav" pane_width="220" compact_pane_width="48"
                pane_title_id="STRID_NAV_DEMO_TITLE"
                show_header="true" settings_item="true" settings_page="page_settings"
                selected_id="page_home">
  <!-- 导航项：page 指向内容页控件 name -->
  <NavigationViewItem text_id="STRID_NAV_HOME"
                      icon="file='public/nav/home.svg' width='16' height='16'"
                      page="page_home"/>
  <NavigationViewItem text_id="STRID_NAV_LIBRARY" icon="..." page="page_library"/>
  <!-- 分组标题（不可点） -->
  <NavigationViewItem item_type="header" text_id="STRID_NAV_GROUP_ADVANCED"/>
  <!-- 分隔线 -->
  <NavigationViewItem item_type="separator"/>
  <NavigationViewItem text_id="STRID_NAV_TOOLS" icon="..." page="page_tools"/>

  <!-- 内容页：非 NavigationViewItem 的直接子节点自动进入内容 TabBox，按 name 关联 -->
  <VBox name="page_home"> ...首页内容... </VBox>
  <VBox name="page_library"> ... </VBox>
  <VBox name="page_tools"> ... </VBox>
  <VBox name="page_settings"> ... </VBox>
</NavigationView>
```

### NavigationView 属性

| 属性                            | 说明                            | 默认       |
| ----------------------------- | ----------------------------- | -------- |
| pane\_width                   | 展开态窗格宽度（px，DPI 缩放）            | 220      |
| compact\_pane\_width          | 紧凑态窗格宽度                       | 48       |
| pane\_title / pane\_title\_id | 汉堡按钮右侧的窗格标题（仅展开态显示）           | 空        |
| collapsed                     | 初始是否紧凑态                       | false    |
| show\_header                  | 是否显示内容区页头                     | true     |
| settings\_item                | 是否在窗格底部显示"设置"项                | false    |
| settings\_page                | 设置项关联的内容页 name                | settings |
| selected\_id                  | 初始选中项对应的 page name（或导航项 name） | 第一项      |

C++ API：`SelectItem(name/index)`、`GetSelectedPage()`、`SetCollapsed(bool)`、
`IsCollapsed()`、`AttachNavSelect(cb)`（映射 kEventTabSelect）、`AddNavItem(...)` 动态追加。

### NavigationViewItem 属性

| 属性              | 说明                                                      |
| --------------- | ------------------------------------------------------- |
| text / text\_id | 项文字（支持多语言）                                              |
| icon            | 图标图片属性串（`file='...' width='16' height='16'`，SVG/PNG 均可） |
| page            | 关联内容页控件 name                                            |
| item\_type      | `item`（默认，可点）/ `header`（分组标题）/ `separator`（分隔线）         |
| name            | 控件名（亦可用于 selected\_id）                                  |

## 三、内部结构与实现要点

NavigationView 以 `Box`（自定义横向布局）为基类，**内部组合**而非全自绘：

```
NavigationView (Box)
├── m_pPane (VBox, 固定宽度, bg_window_card, 右边框 1px)
│   ├── 顶部 HBox：m_pToggleBtn(Button, ☰ 字形) + m_pPaneTitle(Label)
│   ├── m_pItemHost (ScrollBox/VBox, stretch)  ← 导航项插入此处
│   └── m_pSettingsItem (NavigationViewItem, 底部, 可选)
└── m_pRight (VBox, stretch)
    ├── m_pHeader (Label, 页头, 可选)
    └── m_pContent (TabBox, stretch)          ← 内容页插入此处
```

* **AddItem 路由**（核心）：重写 `AddItem`，按子节点 `GetType()` 分流：

  * `DUI_CTR_NAVIGATION_VIEW_ITEM` → 加入 `m_pItemHost`，记入 `m_items`；

  * 其他控件 → 加入 `m_pContent`（TabBox）。内部 Pane/Right 等控件在首次布局前惰性创建。

* **NavigationViewItem 自绘**（参照 Calendar 单元格 + Panel 命中处理）：

  * 圆角热区；hover 背景 `bg_list_item_hovered`；选中：背景 `bg_list_item_selected`，
    文字/图标强调色 `color_accent`；header：`text_muted` 小号字、不可点；separator：1px `border_window`。

  * 图标用 `kStateImageFore`（`SetForeImage`/`PaintStateImage`），文字用 DrawString；
    紧凑态仅居中显示图标并启用 tooltip（ToolTipText）。

  * 点击（ButtonUp 且 sender==this）→ 通知 View 选中该项。

* **选择联动**：View 取消旧项选中→置新项→`m_pContent->SelectItem(pageName)`→
  更新页头文字为该项文本→发 kEventTabSelect。

* **展开/紧凑**：切换 m\_pPane 固定宽度，遍历 m\_items 设置 compact 标志（隐藏文字、居中图标、
  隐藏窗格标题）。v1 即时切换，不做宽度动画（后续可用 Panel 的 AnimationPlayer 模式增强）。

* **内部控件样式**：全部用显式 setter（SetFixedWidth/SetBkColor/字体/颜色）或已验证的
  inline 属性方式，**不依赖新增 global.xml Class**（规避此前 CalendarFlyout 的 class 失效问题）。

* 内部控件设 `tab_stop` 合理值；导航项支持键盘方向键留待后续，v1 鼠标交互为主。

## 四、文件改动清单

新增：

* `duilib/Control/NavigationView.h`（NavigationView + NavigationViewItem）

* `duilib/Control/NavigationView.cpp`

修改框架：

* `duilib/duilib_defs.h`：`DUI_CTR_NAVIGATIONVIEW`("NavigationView")、`DUI_CTR_NAVIGATION_VIEW_ITEM`("NavigationViewItem")

* `duilib/Core/WindowBuilder.cpp`：include + 工厂表两项

* `duilib/duilib.h`：include 新头

* `duilib/duilib.vcxproj` / `.filters`：新增 ClInclude/ClCompile（CMake 用 aux\_source\_directory 自动收集，无需改）

资源：

* `bin/resources/themes/default/public/nav/*.svg`：先检查可复用图标，缺的补简单 SVG

* 语言文件 `zh_CN-examples.txt` / `en_US-examples.txt`：示例与设置项 STRID

* **无需新增语义色**（复用 bg\_window\_card/main、bg\_list\_item\_hovered/selected、color\_accent、text\_muted、border\_window）

示例：

* controls 示例新增"导航视图"演示窗口（参照 TestForm 模式，从 TestForm 加按钮打开），
  含 3\~4 个页面、一个分组标题、设置项、页头；ColorTheme 示例验证深色主题。

收尾：

* `docs/Control-Roadmap.md`（+ .en）#8 状态改为已完成并简述

* 运行 `python .trae/skills/nim-duilib/scripts/verify_docs.py`（如涉及文档）

* 构建：改头文件后先 Rebuild duilib，再构建 controls；清理临时文件

* 代码不提交（用户手动 commit）

## 五、验收标准

1. 启动不崩溃，示例窗口显示左侧导航 + 右侧首页。
2. 点击各导航项：选中项高亮（灰底+强调色文字），右侧内容页正确切换，页头标题联动。
3. 分组标题不可点、分隔线正常；设置项固定在窗格底部并能切到设置页。
4. 汉堡按钮：展开↔紧凑；紧凑态仅图标、居中、tooltip 正常，仍可切换页面。
5. 深色主题（ColorTheme\_d）下颜色全部正确；高 DPI 不错位。
6. selected\_id 初始选中生效；中/英切换文字跟随。

## 六、风险

* AddItem 分流依赖子节点类型判断，需确认 WindowBuilder 对自定义容器子节点均经 AddItem（实现时先验证）。

* 内部控件创建时机：必须在首个用户子节点 AddItem 前惰性建好 Pane/TabBox，且加入自身 children 顺序正确。

* 改头文件 → 必须 Rebuild duilib.lib（ODR 布局问题，既有教训）。

