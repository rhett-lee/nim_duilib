# Spec: D-11.1 Breadcrumb 面包屑导航控件

## 1. 目标

为 nim_duilib 新增面包屑导航控件 `Breadcrumb`（XML 节点名 `Breadcrumb`），用于文件/目录路径、站点层级、设置页层级导航等场景：用一行可点击的祖先路径 + 当前项，表达"我在哪一层、怎么回到上层"。

现状：全仓无 `Breadcrumb` / `breadcrumb` 命中（见 `Development-Plan.md` D-11）。`AddressBar` 是浏览器地址栏（输入 + 前进后退），`Stepper` 是流程状态机（步骤序号/对勾），`TreeView` / `ListBox` 是数据浏览容器，语义均不同，因此新增独立控件。

完成后回写：
- `docs/misc/Development-Plan.md` 第 41 行 P2 汇总、D-11 表格行、D-11.1 复选框。
- `docs/misc/Project-Audit.md` 无 Breadcrumb 条目，本次不新增审计行；`docs/misc/Control-Roadmap.md` 无对应行，不改。

用户约束：
- 代码提交由用户手动执行，助手不执行 `git commit`。
- 只保证 **Debug|x64** 编译通过（不编译 Release）。

## 2. 需求要点（来自 D-11 / D-11.1）

- 条目模型：每项一段文本，`|` 分隔；支持 `item_text` 与多语言 `item_text_id`。
- 当前项（current）：高亮显示；默认跟随最后一项；可由程序设置或鼠标点击祖先项切换。
- 分隔符可配置（默认 `/`），分隔符与条目之间的间距、条目可点击内边距均需 DPI 缩放。
- 点击祖先项可跳转（`click_to_jump`，默认开）；禁用项不可点击；提供 `NoEvent` 静默接口避免回调自递归。
- 值变化时派发事件，复用 `kEventValueChanged`，`pEventData` 携带新旧索引。
- 纯自绘、颜色走语义色名、字体可配置；不写死色值，浅色/深色主题均可用。
- 语言切换后 `item_text_id` 自动刷新。

## 3. 设计决策

### 3.1 继承与数据模型

- `class DUILIB_API Breadcrumb : public Control`，`typedef Control BaseClass;`（纯自绘，沿用 `Stepper` 的数据驱动 + 自绘范式，不创建子控件）。
- 条目结构：
  ```cpp
  struct Item
  {
      DString text; //条目文本
  };
  ```
- 当前索引用 `int32_t m_nCurrentIndex`，初值 `-1` 表示"自动跟随最后一项"：
  - `GetCurrentIndex()` 在空数据时返回 0；`m_nCurrentIndex < 0` 或越界时返回 `itemCount - 1`。
  - `SetAttribute("current")` / `SetCurrentIndex()` 写入显式索引，内部钳位到 `[0, itemCount-1]`。
- 禁用集合 `std::vector<size_t> m_disabledItems`，优先于其他状态。
- 只有文本字段，因此 `ParseItemsField()` 采用**整体替换**语义（与 Stepper 的双字段追加语义不同）：先清空 `m_items`，再按 `|` 逐段填充。

### 3.2 事件编码

- 事件类型：`kEventValueChanged`。
- 编码：`wParam = 3`（面包屑模式标记，与 RangeSlider `1`、Stepper `2`、Pagination `0` 区分），`lParam = 0`，
  `pEventData = &BreadcrumbValue{ currentIndex, oldIndex }`（栈上临时对象，**仅本次同步派发有效，勿保存指针**）。
  ```cpp
  struct BreadcrumbValue
  {
      size_t currentIndex = 0; //新的当前项索引
      size_t oldIndex = 0;     //变化前的当前项索引
  };
  ```
- 便捷接口：`void AttachCurrentChanged(const EventCallback& callback, EventCallbackID callbackID = 0);`
- 仅在索引**实际变化**时派发一次；`SetCurrentIndexNoEvent()` / `GoToNoEvent()` 通过 `m_bMuteEvent` 守卫抑制事件。

### 3.3 XML 属性表（默认值必须与构造函数一致）

| 属性 | 别名 | 默认值 | 类型 | 说明 |
| :--- | :--- | :--- | :--- | :--- |
| `item_text` | — | 空 | `|` 分隔文本 | 条目文本，如 `首页|产品|文档` |
| `item_text_id` | — | 空 | `|` 分隔 STRID | 多语言条目文本，语言切换自动刷新 |
| `current` | `current_index` | `-1` | int | 当前项索引（0 起）；`-1` = 自动跟随最后一项 |
| `separator` | `separator_text` | `/` | 文本 | 条目之间的分隔符 |
| `spacing` | — | `8` | int（像素） | 分隔符两侧的间距，DPI 缩放 |
| `item_padding` | — | `4` | int（像素） | 条目点击区域左右内边距，DPI 缩放 |
| `click_to_jump` | — | `true` | bool | 是否允许点击祖先项切换当前项 |
| `disabled_items` | — | 空 | 索引列表 | 禁用项索引，`|` 或 `,` 分隔，如 `1|2` |
| `item_color` | — | `text_link_normal` | 颜色 | 可点击祖先项颜色 |
| `current_color` | — | `text_default` | 颜色 | 当前项颜色 |
| `hovered_color` | — | `text_link_hovered` | 颜色 | 祖先项悬停颜色 |
| `pressed_color` | — | `text_link_pressed` | 颜色 | 祖先项按下颜色 |
| `disabled_color` | — | `text_disabled` | 颜色 | 禁用项颜色 |
| `separator_color` | — | `text_muted` | 颜色 | 分隔符颜色 |
| `font` | — | `system_regular_14` | 字体 ID | 条目与分隔符共用的字体 |

未知属性回退 `BaseClass::SetAttribute()`。颜色属性值经 `GetExpandVarStrings()` 展开后保存语义色名或颜色值。

### 3.4 布局与绘制

- 绘制入口：`PaintStateImages(IRender* pRender)`，先 `BaseClass::PaintStateImages(pRender)`，再对内容区 `rc = GetRect(); rc.Deflate(GetControlPadding());` 做 `AutoClip`。
- 条目宽度 = `MeasureString(text, font, TEXT_SINGLELINE).Width() + 2 * item_padding`；分隔符宽度同理（不加内边距）。
- 总宽 = Σ条目宽 + Σ分隔符宽 + `2 * spacing * 分隔符数量`。
- 起点：
  - 总宽 ≤ `rc.Width()`：`x = rc.left`（左对齐）。
  - 总宽 > `rc.Width()`：`x = rc.right - totalWidth`，即**右对齐**，左端超出部分由 `AutoClip` 裁剪，保证末项/当前项始终可见（v1 不做 `…` 收纳，见 3.6）。
- 每个条目矩形横向为 `[x, x + itemWidth]`，纵向为整个 `rc`；文本用 `TEXT_LEFT | TEXT_SINGLELINE | TEXT_VCENTER` 在其矩形内绘制。
- 分隔符绘制在相邻条目之间，水平居中于 `spacing` 间隙，颜色取 `separator_color`。
- 文本颜色优先级（写进代码注释）：**禁用 > 按下 > 悬停 > 当前 > 普通**。分隔符不参与交互，始终用 `separator_color`。
- 尺寸缩放统一用 `Dpi().GetScaleInt(...)`；文本测量与绘制用 `GetIFontById(m_strFont)`。

### 3.5 交互与命中

- **可交互项**定义：控件 `IsEnabled()`、`IsMouseEnabled()`、`m_bClickToJump` 为真，且该条目 `index != GetCurrentIndex()`、不在禁用集合中。
- 命中检测只针对条目文本矩形（含 `item_padding`），分隔符与空白区域不响应。
- `ButtonDown`：命中可交互项 → 记录 `m_nPressedIndex`、`SetMouseFocused(true)`、调用 `GoTo(index)`（内部派发事件）。
- `ButtonUp`：清除按下态。
- `MouseMove`：重算 `m_nHoveredIndex`（不可交互项记 -1），变化时 `Invalidate()`。
- `MouseLeave`：清除悬停/按下态。
- 光标：悬停可交互项时 `SetCursorType(CursorType::kCursorHand)`，否则恢复 `CursorType::kCursorArrow`；`MouseLeave` 恢复箭头。
- 当前项与禁用项不响应点击、不显示悬停/按下态。

### 3.6 明确不做（本次范围外）

- 宽度不足时的 `…` 收纳 / 折叠算法（v1 右对齐裁剪；条目过多由业务侧截短路径）。
- 条目图标 / 图片分隔符（只有文本）。
- 键盘导航（Home/End/Left/Right）、焦点框。
- 条目的右键菜单、拖拽、动态展开下拉。
- 逐项自定义字体/颜色（统一由控件级属性控制）。

### 3.7 边界行为

- 空数据：不绘制、不派发事件；`GetCurrentIndex()` 返回 0。
- 单条目：索引恒为 0，点击无效果。
- `SetCurrentIndex()`：越界钳位；与旧值相同则直接返回，不派发事件。
- `RemoveItemAt()` / `ClearItems()` / `SetItems()`：数据变化后把显式当前索引钳位到 `[0, count-1]`；`SetItems()` 保留禁用集合（索引越界项在绘制/命中时忽略），`ClearItems()` 同时清空禁用集合。
- 重复设置相同属性值：幂等，不派发事件。
- 语言切换：`OnLanguageChanged()` 中若保存了 `m_strItemTextIds`，重新按 STRID 解析并刷新文本。

## 4. 精确改动清单

### 4.1 新增控件源码

1. `duilib/Control/Breadcrumb.h`
   - `Item`、`BreadcrumbValue`、`Breadcrumb : public Control`。
   - public：构造/析构、`GetType()`、`SetAttribute()`、`PaintStateImages()`、`OnLanguageChanged()`、`ButtonDown/ButtonUp/MouseMove/MouseLeave`。
   - 数据 API：`AddItem(const DString&)`、`InsertItem(size_t, const DString&)`、`RemoveItemAt(size_t)`、`ClearItems()`、`SetItems(const std::vector<Item>&)`、`GetItemCount()`、`GetItemAt()`、`GetItems()`。
   - 当前项 API：`GetCurrentIndex()`、`SetCurrentIndex(size_t)`、`SetCurrentIndexNoEvent(size_t)`、`GoTo(size_t)`、`GoToNoEvent(size_t)`。
   - 禁用项 API：`SetItemEnabled(size_t, bool)`、`IsItemEnabled(size_t)`、`SetDisabledItems(...)`、`GetDisabledItems()`。
   - 外观 API：`SetSeparator/GetSeparator`、`SetSpacing/GetSpacing`、`SetItemPadding/GetItemPadding`、`SetClickToJump/IsClickToJump`、`SetItemColor/GetItemColor`、`SetCurrentColor/GetCurrentColor`、`SetHoveredColor/GetHoveredColor`、`SetPressedColor/GetPressedColor`、`SetDisabledColor/GetDisabledColor`、`SetSeparatorColor/GetSeparatorColor`、`SetFont/GetFont`。
   - 事件：`AttachCurrentChanged(const EventCallback&, EventCallbackID = 0)`。
   - private：`ParseItemsField(const DString&, bool bTranslate)`、`ParseDisabledItems(const DString&)`、`EmitCurrentChanged(size_t)`、`ClampCurrentIndex()`、`BuildLayout(IRender*, const UiRect&)`、`HitTestItem(...)`、`DrawBreadcrumbText(...)`、`UpdateCursor(...)`。
   - 成员均带中文注释；`GetType()` 返回 `DUI_CTR_BREADCRUMB`。
2. `duilib/Control/Breadcrumb.cpp`
   - 构造初始化全部成员（`m_nCurrentIndex(-1)`、`m_nSpacing(8)`、`m_nItemPadding(4)`、`m_bClickToJump(true)`、`m_strSeparator(_T("/"))`、颜色语义色名、`m_strFont(_T("system_regular_14"))`、`m_nHoveredIndex(-1)`、`m_nPressedIndex(-1)`、`m_bMuteEvent(false)`）。
   - 实现上述 API、布局、绘制、交互与事件；文本测量使用 `MeasureStringParam`，绘制使用 `DrawStringParam`。

### 4.2 注册（5 处，缺一不可）

| # | 文件 | 具体改动 |
| :--- | :--- | :--- |
| 1 | `duilib/duilib_defs.h` | 在 `DUI_CTR_PAGINATION` 之后新增 `#define DUI_CTR_BREADCRUMB (_T("Breadcrumb"))` |
| 2 | `duilib/Core/WindowBuilder.cpp` | 增加 `#include "duilib/Control/Breadcrumb.h"`；`CreateControlMap()` 增加 `{DUI_CTR_BREADCRUMB, [](Window* pWindow){ return new Breadcrumb(pWindow); }}` |
| 3 | `duilib/duilib.vcxproj` | 增加 `<ClCompile Include="Control\Breadcrumb.cpp" />` 与 `<ClInclude Include="Control\Breadcrumb.h" />` |
| 4 | `duilib/duilib.vcxproj.filters` | 上述两项归入 `Control` 过滤器（与 `Stepper`/`Pagination` 同组） |
| 5 | `duilib/duilib.h` | 增加 `#include "Control/Breadcrumb.h"` |

### 4.3 主题与默认样式

- `bin/resources/themes/default/global.xml` 新增：
  ```xml
  <!-- 面包屑导航（Breadcrumb）：纯文本 + 语义色 -->
  <Class name="breadcrumb" font="system_regular_14"
         item_color="text_link_normal" current_color="text_default"
         hovered_color="text_link_hovered" pressed_color="text_link_pressed"
         disabled_color="text_disabled" separator_color="text_muted"/>
  ```
- 无图片资源，因此不新增 `PATH_*` 变量；控件在无 Class 时也可正常工作（默认值写在构造函数）。

### 4.4 示例与入口

1. 新增 `examples/controls/BreadcrumbForm.h/.cpp`
   - `GetSkinFolder()` → `controls`；`GetSkinFile()` → `breadcrumb.xml`。
   - `OnInitWindow()` 绑定： `breadcrumb_demo` 的 `AttachCurrentChanged` 回显、`btn_bc_up`（上一级）、`btn_bc_root`（跳到首页）、`btn_bc_add`（追加一项）、`btn_bc_remove`（删除末项）。
2. 新增 `bin/resources/themes/default/controls/breadcrumb.xml`
   - 标题栏 + 提示 Label + 两张卡片：
     - 默认形态：`item_text_id` 4 项、`class` 写在最前、`current` 缺省跟随末项。
     - 自定义形态：`separator=">"`、`current="1"`、`disabled_items="2"`。
   - 操作按钮 4 个 + 事件回显 Label。
3. `bin/resources/themes/default/controls/test.xml`：新增 `GroupHBox text="Breadcrumb"` 与 `<Button class="btn_global" name="breadcrumb_btn" text_id="STRID_CONTROLS_BREADCRUMB_TEST" .../>`。
4. `examples/controls/TestForm.h`：新增 `void ShowBreadcrumbWindow();`。
5. `examples/controls/TestForm.cpp`：`#include "BreadcrumbForm.h"`、绑定 `breadcrumb_btn`、实现 `ShowBreadcrumbWindow()`（`kWS_POPUP + kWS_EX_LAYERED + m_bCenterWindow` + `ShowModalFake()`）。
6. `examples/controls/controls.vcxproj` 与 `.filters`：加入 `BreadcrumbForm.h/.cpp`。
7. `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt` 新增键：
   `STRID_CONTROLS_BREADCRUMB_TEST`、`STRID_BREADCRUMB_WINDOW_TITLE`、`STRID_BREADCRUMB_HINT`、
   `STRID_BREADCRUMB_BASIC_TITLE`、`STRID_BREADCRUMB_CUSTOM_TITLE`、
   `STRID_BREADCRUMB_ITEM_HOME`、`STRID_BREADCRUMB_ITEM_PRODUCTS`、`STRID_BREADCRUMB_ITEM_DOCS`、`STRID_BREADCRUMB_ITEM_CONTROLS`、
   `STRID_BREADCRUMB_ITEM_SYSTEM`、`STRID_BREADCRUMB_ITEM_NETWORK`、`STRID_BREADCRUMB_ITEM_PROXY`、`STRID_BREADCRUMB_ITEM_DETAIL`,
   `STRID_BREADCRUMB_UP`、`STRID_BREADCRUMB_ROOT`、`STRID_BREADCRUMB_ADD`、`STRID_BREADCRUMB_REMOVE`、
   `STRID_BREADCRUMB_EVENT_PREFIX`、`STRID_BREADCRUMB_EVENT_NONE`、`STRID_BREADCRUMB_NEW_ITEM`。

### 4.5 文档同步（中英成对）

| 文件 | 改动 |
| :--- | :--- |
| `docs/Control.md` | 末尾新增 `## Breadcrumb 控件（面包屑导航）`：属性表、XML 示例、C++ 接口、事件说明 |
| `docs/Control.en.md` | 末尾新增 `## Attributes of Breadcrumb (breadcrumb navigation)`，内容与中文对应 |
| `docs/Summary.md` / `docs/Summary.en.md` | 控件总表新增一行（基类 `Control`、头文件链接、能力摘要、锚点） |
| `README.md` / `README.en.md` | 功能列表新增第 48 条；首页控件计数同步 |
| `docs/misc/Development-Plan.md` | 第 41 行 P2 汇总去掉 D-11.1；D-11 表格行补 Breadcrumb 已完成；D-11.1 改为 `- [x]` 并填完成日期 |
| `.trae/skills/nim-duilib/references/api-reference.md` | 速查表新增 `Breadcrumb` 行 + 属性小节 + 事件 |
| `.workbuddy/skills/nim-duilib/references/api-reference.md` | 与上一份**同源同文本**覆盖 |
| `.claude/docs/nim-duilib-llm-reference.md` | 速查行/描述同步 |
| `docs/misc/Project-Audit.md` | 无 Breadcrumb 条目，不改 |
| `docs/misc/Control-Roadmap.md/.en.md` | 无对应行，不改 |

## 5. 验证步骤

1. 编译（只验证 Debug|x64）：
   ```bash
   MSBuild "c:\develop\nim_duilib\build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo
   ```
   无输出（无 error）即通过。
2. 运行 `bin\examples.exe`（或项目实际示例可执行文件）→ 打开 controls 测试页 → 点击 Breadcrumb 入口，按 `checklist.md` 手工验收。
3. 文档校验：
   ```bash
   python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang
   ```
   退出码 0 = 无漂移；若出现新标识符误报，按 `verify_allowlist.txt` 规则补充，不得在文档里写 HTML 注释规避。
4. 主题与语言：浅色/深色主题各看一次；中文 → 英文 → 中文切换后条目文案自动刷新、布局不串位。

## 6. 验收标准

- [ ] XML 中 `<Breadcrumb .../>` 能创建，`FindControl` 可取到；无 `class` 时正常绘制。
- [ ] 属性表中每个属性均实际生效；别名 `current_index` / `separator_text` 与主名等效；未知属性不崩。
- [ ] 默认值（`current=-1`、`separator="/"`、`spacing=8`、`item_padding=4`、`click_to_jump=true`、颜色/字体）与文档一致。
- [ ] 点击祖先项切换当前项并派发一次 `kEventValueChanged`（wParam=3，`pEventData` 含新旧索引）；`NoEvent` 接口不派发。
- [ ] 当前项、禁用项不可点击；`click_to_jump=false` 时全部不可点击；hover/pressed 只出现在可交互祖先项上。
- [ ] 空数据、单条目、越界索引、重复设置、删除条目后当前索引均正确（不崩、不越界）。
- [ ] `item_text_id` 中英切换自动刷新；`item_text` 与 `item_text_id` 后设置者生效。
- [ ] 浅色/深色主题下所有颜色均可读；100%/125%/150%/200% DPI 下间距与命中区域一致。
- [ ] 宽度不足时右对齐裁剪，末项/当前项仍可见；窗口拉伸不重叠、不异常。
- [ ] Debug|x64 编译通过；`verify_docs.py` 退出码 0；计划文档 D-11.1 已勾选。

## 7. 待用户确认

1. **实现形态**：纯自绘 `Control` 子类（同 Stepper），而不是 `HBox` + 动态子按钮（同 Pagination）。
2. **事件编码**：`kEventValueChanged` + `wParam = 3`。
3. **v1 范围**：不做 `…` 收纳、条目图标、键盘导航、右键菜单；宽度不足时右对齐裁剪。
4. **文件清单**：源码 2 个、注册 5 处、主题 Class 1 个、示例 2 个 C++ + 1 个 XML + 入口/工程/语言资源、双语文档 8 份 + 计划文档回写。

## 8. 增量需求（2026-10-22 视觉改版）

**背景**：首版默认语义色在浅色主题下 `text_link_normal` / `text_default` / `text_muted` 取值相同（均为 `#FF1A1A1A`），祖先项、当前项、分隔符无法区分层级；条目悬停/按下也无视觉反馈。用户反馈"效果不太好"，要求在**设计层面**改进。

**方向**：用户选择"链接式 + 胶囊悬停"。

**改动**：

1. 默认配色改为可区分层级、且深浅主题通用的语义色：
   - 祖先项 `color_blue`（浅 `#FF2966A3` / 深 `#FF337FCC`）
   - 悬停 `color_blue_light`、按下 `color_blue`、当前项 `text_default`
   - 禁用 `text_disabled`、分隔符 `color_gray`（`#FF808080`）
2. 新增属性与 API：
   - `current_font`（默认 `system_bold_14`）：当前项加粗强调；布局按对应字体测量宽度
   - `hover_bg_color`（默认 `bg_btn_hovered`）/ `pressed_bg_color`（默认 `bg_btn_pressed`）：悬停/按下时绘制圆角胶囊背景，留空可关闭
   - `item_round`（默认 4，DPI 缩放）：胶囊圆角半径
3. 默认值调整：`separator` 由 `/` 改为 `›`；`spacing` 8 → 4；`item_padding` 4 → 6（为胶囊留出内边距）。
4. `PaintStateImages` 绘制顺序：先胶囊背景，再文本；颜色优先级保持"禁用 > 按下 > 悬停 > 当前 > 普通"。
5. 示例 `breadcrumb.xml`：两张卡片由固定/拉伸高度改为 `height="auto"`，末尾加 `height="stretch"` 占位，消除卡片大面积留白。
6. 同步更新 `Control.md` / `Control.en.md`、`api-reference.md`（`.trae` + `.workbuddy`）、`.claude` LLM 参考。

**不变**：事件编码（`kEventValueChanged` + `wParam=3`）、数据模型、现有 API 语义与边界策略。
