# MenuBar 菜单栏控件分析报告

下面的分析基于 `duilib/Control/MenuBar.h`、`duilib/Control/MenuBar.cpp`，以及关联的 `duilib/Control/Menu.cpp`、`duilib/Core/WindowBuilder.cpp`、`bin/resources/themes/default/global.xml`。文中的行号对应分析时的代码版本；修复结果见开头的“处理状态”表。

## 处理状态

本次已按“六、建议实施顺序”完成 P0、P1、P2 修复，并处理 P3 中两个低风险缺陷（菜单栏隐藏/禁用、主窗口移动时关闭弹层）；随后又完成了一轮体验增强（见下方“体验增强实施状态”）。

| 编号 | 内容 | 状态 | 说明 |
| --- | --- | --- | --- |
| 1 | 键盘左右切换索引错位 | 已修复 | 改为在“可见且可用的按钮列表”中定位当前项，再环形取前后项 |
| 2 | `m_bActiveState` 与真实菜单状态不同步 | 已修复 | 删除该成员，改用 `IsPopupMenuOf()` / `TogglePopupMenu()` 以真实弹层为准 |
| 3 | 悬停切换后单击反而关闭 | 已修复 | 鼠标点击与键盘激活统一走 `TogglePopupMenu()` |
| 4 | 键盘 Enter/Space 无法打开 | 已修复 | `MenuBarButton` 处理鼠标按下与回车/空格按键，统一调用 `TogglePopupMenu()` |
| 5 | 移除激活中的顶级菜单未关闭弹层 | 已修复 | `RemoveTopMenuFromUI()` 在移除控件前先关闭关联弹层 |
| 6 | 悬停切换未校验目标按钮状态 | 已修复 | `ShowPopupMenu()` 统一校验可见、可用状态 |
| 7 | XML 的 `class`/`attributes` 被忽略 | 已修复 | `WindowBuilder` 读取并传递；文档与 XSD 已同步 |
| 8 | 列表索引被直接当作子控件索引 | 已修复 | `TopMenuData` 保存按钮指针，并新增 `GetInsertChildIndex()` 计算插入位置 |
| 9 | 同一菜单被关闭后立即重建 | 已修复 | `ShowPopupMenu()` 对当前已展开项直接返回 |
| 10 | 菜单 XML 加载失败无处理 | 已修复 | 检查 `IsWindow()` / `GetMenuItemCount()`，失败时清理状态并关闭窗口 |
| 11 | `AddItemAt` 失败时新按钮泄漏 | 已修复 | 失败时释放按钮并返回失败，`InsertTopMenu` 回滚列表 |
| 12 | 接口注释、`const`、死代码 | 已修复 | 补全 `InsertTopMenu` 注释、`GetTopMenu` 改为 `const`、删除空实现与重入标志 |
| 13 | 主窗口移动/隐藏时弹层不关闭 | 已修复 | 监听窗口位置变化；菜单栏隐藏或禁用时关闭弹层 |

### 体验增强实施状态

| 编号 | 内容（对应第三节） | 状态 | 说明 |
| --- | --- | --- | --- |
| E1 | 键盘可访问性（第三节 3） | 已实施 | `F10` / `Alt` 聚焦菜单栏，再按一次取消焦点（开关式）；`Alt` 不依赖 `kFirstPress` 标志，改为跟踪按键状态以过滤自动重复。聚焦后回车/空格切换、向下键展开、左右键在按钮间切换焦点；弹层内 `←/→` 切换顶级菜单、`Esc` 收起并把焦点还给顶级菜单按钮 |
| E2 | 键盘焦点视觉（第三节 3、第五节 5） | 已实施 | `menu_bar_button` 开启 `show_focused_rect`，但仅键盘操作（F10/Alt、回车等）获得焦点时才绘制；鼠标点击不显示。焦点框使用通用默认实现（强调色圆角描边，对齐 Windows 原生风格），仅在窗口激活时绘制 |
| E3 | 激活项强调色指示条（第五节 3） | 已实施 | `MenuBar::Paint()` 在激活按钮下方绘制 2px（DPI 缩放）强调色条，颜色取 `color_accent`，为空时回退 `text_menu_bar` |
| E4 | 悬停策略（第三节 4） | 已实施 | 已展开菜单时，悬停切换延迟 150ms，避免快速划过反复重建弹层；菜单栏隐藏/禁用时取消待执行的切换，控件销毁后回调因弱引用失效而不再执行 |
| E5 | 运行时维护接口（第三节 2） | 已实施 | 新增 `SetTopMenuText`、`SetTopMenuEnabled`、`SetTopMenuVisible`、`ClosePopupMenu`；禁用/隐藏时先关闭其弹层 |
| E6 | 激活状态事件（第三节 1） | 已实施 | 新增 `MenuBarActiveChangedEvent` 与 `AttachMenuBarActiveChanged` / `ClearMenuBarActiveChanged`，在展开、切换、收起、加载失败时通知 |
| E7 | 弹层右对齐（第三节 9、第五节 6） | 已具备，无需改动 | `Menu` 默认 `PopupPosType` 为右对齐并带屏幕边界智能翻转，`ResizeMenu()` 已处理贴边场景 |

仍未实施（及原因）：

- P2 浅色主题悬浮对比度提升：需要调整 `color_light` 主题色值，影响面较大，本轮不修改主题色文件。
- P2 键盘焦点描边（常驻边框）：会改变非焦点态外观，改用焦点矩形（E2）替代。
- P3 溢出折叠（“»”）：需要额外的折叠容器、弹出列表与中英文布局适配，复杂度与风险较高，暂不实施。
- P3 菜单窗口复用/缓存：每次弹出仍新建 `Menu` 窗口并解析 XML；复用需要处理子菜单层级、观察者注册与生命周期，暂不实施。已通过 E4 的悬停延迟降低频繁重建的观感影响。

## 一、总体结论

`MenuBar` 继承 `HBox`，用内部 `MenuBarButton`（`ButtonBox`）承载顶级菜单文本，点击/悬停时创建独立的 `Menu` 弹出窗口。结构简单、DPI 坐标处理基本正确，多语言通过 `text_id` 支持。

当前实现存在几个相互关联的薄弱点：

1. 弹出菜单状态用了一个全局 `bool m_bActiveState` 维护，与真实弹出菜单（`m_pActiveMenu`）可能脱节，导致点击行为不符合直觉；
2. 键盘左右切换的索引计算错误，存在隐藏/禁用项或非菜单子控件时会切错项；
3. 键盘聚焦顶级按钮后回车/空格无法打开菜单；
4. XML 中的 `class` 等样式属性被静默忽略，资源示例已经写了但没生效；
5. 顶级菜单项与子控件的索引/ID 隐式耦合，增删和插入容易错位；
6. 主题只有 `menu_bar_button`，缺少菜单栏容器样式与状态文字颜色，浅色主题的悬浮反馈偏弱。

## 二、错误与设计缺陷（按严重程度）

| 编号 | 级别 | 问题 | 位置 |
| --- | --- | --- | --- |
| 1 | 高 | 键盘左右切换索引错位，可能激活隐藏菜单 | `MenuBar.cpp#L433-L482` |
| 2 | 高 | `m_bActiveState` 与真实菜单状态不同步 | `MenuBar.cpp#L327-L338` |
| 3 | 中高 | 悬停切到另一菜单后，单击该菜单会反而关闭 | `MenuBar.cpp#L275-L313` |
| 4 | 中高 | 键盘 Enter/Space 无法打开顶级菜单 | `MenuBar.cpp#L108-L114` |
| 5 | 中高 | 移除激活中的顶级菜单时弹层未关闭 | `MenuBar.cpp#L212-L222` |
| 6 | 中 | 悬停切换到禁用/隐藏的顶级项不做校验 | `MenuBar.cpp#L292-L313` |
| 7 | 中 | XML 的 `class`/附加属性被忽略 | `WindowBuilder.cpp#L1439-L1474` |
| 8 | 中 | 列表索引被直接当作子控件索引使用 | `MenuBar.cpp#L190-L193` |
| 9 | 中 | 同一菜单可能被关闭后立即重建 | `MenuBar.cpp#L344-L415` |
| 10 | 中 | 菜单 XML 加载失败无任何处理 | `MenuBar.cpp#L378-L383` |
| 11 | 低 | `AddItemAt` 失败时新按钮泄漏 | `MenuBar.cpp#L92-L114` |
| 12 | 低 | 接口注释、`const`、死代码问题 | `MenuBar.h#L74-L100` |
| 13 | 低 | 主窗口移动/隐藏时弹层不跟随也不关闭 | MenuBar 未监听窗口位置或可见性变化 |

### 1. 键盘左右切换的索引计算错误（高）

`OnMenuKeyDownMsg`（`MenuBar.cpp#L433-L482`）里：

- `nStartItemIndex` 记录的是原始子控件索引（`GetItemCount()` 空间）；
- 随后却把它当作 `itemIndexList`（只包含可见且可用项）的下标参与 `+1/-1`；
- 最后又用 `GetItemAt(nNextMenuIndex)` 把它当原始子控件索引取控件。

两个索引空间混用，只要存在隐藏项、禁用项或非 `MenuBarButton` 子控件就会出错。例如：第 0 项隐藏、当前是第 1 项，按右方向键会执行 `GetItemAt(0)`，把隐藏菜单激活出来。

建议改写为“先在可见项列表里定位当前位置，再环形取下一项”：

```cpp
std::vector<size_t> items;      // 可见且可用的子控件索引
size_t currentPos = Box::InvalidIndex;
for (size_t i = 0; i < GetItemCount(); ++i) {
    auto* pBtn = dynamic_cast<MenuBarButton*>(GetItemAt(i));
    if ((pBtn != nullptr) && pBtn->IsVisible() && pBtn->IsEnabled()) {
        if (pMenu->GetRelatedControl() == pBtn) {
            currentPos = items.size();
        }
        items.push_back(i);
    }
}
if (items.empty() || (currentPos == Box::InvalidIndex)) {
    return;
}
size_t nextPos = (vkCode == kVK_RIGHT)
               ? (currentPos + 1) % items.size()
               : (currentPos + items.size() - 1) % items.size();
SwitchToMenuButton(items[nextPos]);
```

同时建议把“切换菜单”的动作抽成 `SwitchToMenuButton()`，内部直接调用 `ShowPopupMenu()`，避免再绕 `Activate()` -> `kEventClick` -> `m_bEnableBtnActive` 这条链路。

### 2. `m_bActiveState` 与实际菜单状态不同步（高）

`OnMenuMouseButtonDown`（`MenuBar.cpp#L327-L338`）用 `m_bActiveState = !m_bActiveState` 决定开关，但这个布尔值只有两处会改：

- 鼠标按下时取反；
- 菜单窗口关闭回调里置 `false`（且要求 `m_pActiveMenu == pMenu`）。

而 `SetActiveTopMenuIndex`（`MenuBar.cpp#L224-L242`）展开菜单时、`CheckShowPopupMenu`（`MenuBar.cpp#L275-L290`）悬停切换菜单时，都不会同步这个布尔值。

后果：

- 程序调用 `SetActiveTopMenuIndex(0)` 展开菜单后，再点击该顶级按钮，`m_bActiveState` 会从 `false` 变 `true`，走“显示菜单”分支，导致菜单被关闭后立刻重建（闪烁）；
- 悬停从 A 切到 B 后，点击 B 时 `m_bActiveState` 仍沿用之前的值，用户会感觉“点 B 却把菜单关掉了”。

建议删除 `m_bActiveState`，以真实状态为唯一来源：

```cpp
bool MenuBar::IsPopupMenuOf(const MenuBarButton* pButton) const {
    return HasActivePopupMenu() && (m_pActiveMenu->GetRelatedControl() == pButton);
}

void MenuBar::OnMenuMouseButtonDown(MenuBarButton* pButton, const EventArgs&) {
    if (IsPopupMenuOf(pButton)) {
        HidePopupMenu(pButton);   // 点击已展开的项 -> 收起
    }
    else {
        ShowPopupMenu(pButton);   // 点击其他项 -> 切换
    }
}
```

### 3. 悬停切换后单击会“关掉刚切换的菜单”（中高）

这是第 2 条的直接表现：鼠标从 A 移到 B 时，`CheckShowPopupMenu()` 已经把菜单切到 B；随后按下 B 的按钮，`m_bActiveState` 取反为 `false`，于是 `OnMenuMouseButtonDown` 执行 `HidePopupMenu()`。

按第 2 条改成“判断当前弹层是否属于该按钮”后即可自然修复：已属于 B 时点击才收起，否则切换。

### 4. 键盘 Enter/Space 无法打开菜单（中高）

`MenuBarButton` 继承 `ButtonBox`，`Button.h#L54-L67` 中 `kVK_SPACE/kVK_RETURN` 会走 `Activate()` -> 发送 `kEventClick`；而菜单栏的点击回调（`MenuBar.cpp#L108-L114`）又被 `m_bEnableBtnActive` 门控，该标志平时为 `false`，仅 `SetActiveTopMenuIndex` 和方向键切换时临时置 `true`。

因此 `MenuBarButton` 默认可参与 Tab 焦点（`m_bAllowTabstop` 默认为 `true`），但聚焦后按回车没有任何反应。

建议：让键盘激活与鼠标点击走同一入口（覆写 `MenuBarButton::Activate()` 直接通知 `MenuBar` 打开/切换菜单），不再依赖 `m_bEnableBtnActive` 这个重入标志；或保留标志但确保 `kEventClick` 的默认路径也置位。

### 5. 移除激活中的顶级菜单未关闭弹层（中高）

`RemoveTopMenu`（`MenuBar.cpp#L212-L222`）只删除列表项和 UI 控件，没有处理 `m_pActiveMenu`：

- 已展开的菜单窗口仍然显示；
- `m_pActiveMenu` 关联的按钮已被删除（弱指针失效），`GetActiveTopMenuIndex()` 返回 -1；
- `m_bActiveState` 可能残留，后续点击行为异常。

建议在删除前判断当前激活菜单是否关联该按钮，是则先 `HidePopupMenu()` 并复位状态。

### 6. 悬停切换未校验目标按钮状态（中）

`OnMenuMouseEnter`（`MenuBar.cpp#L292-L313`）和 `ShowPopupMenu`（`MenuBar.cpp#L344-L415`）都没有检查 `IsEnabled()` / `IsVisible()`。`SetActiveTopMenuIndex` 检查了，悬停路径没有，会出现“菜单被切到禁用项”的情况。

建议在 `ShowPopupMenu()` 开头统一做防御性检查，悬停和程序调用都受益。

### 7. XML 中 `MenuBarItem` 的样式属性被忽略（中）

`ParseMenuBarItemXmlNode`（`WindowBuilder.cpp#L1439-L1474`）只读取 `id/text/text_id/xml_path`，没有读取 `class` 或任何按钮附加属性，最终 `AddTopMenu()` 的 `menuTextButtonClass` 参数为空。

资源里已经出现了这种写法：`bin/resources/themes/default/color_theme/color_theme.xml#L872` 的 `class="menu_bar_button"`。因为正好与默认类同名，所以看起来正常；一旦改成自定义类就不会生效。

建议支持：

```xml
<MenuBarItem id="menu_file" text="File"
             class="menu_bar_button"
             attributes="text_padding='10,4,10,4'">
```

并在 `docs/schema/nim_duilib.xsd#L5579-L5601`、`docs/Control.md#L783-L808` 同步说明。

### 8. 顶级菜单索引与子控件索引耦合（中）

`AddTopMenuToUI`（`MenuBar.cpp#L83-L115`）用 `m_topMenuList` 的索引直接调用 `AddItemAt()`。如果 `MenuBar` 中还有 logo、分隔线、`stretch` 占位子控件，插入位置就会错位；`InsertTopMenu` 传入的 `nMenuIndex` 也会直接当子控件索引。

建议 `TopMenuData` 直接保存按钮控件指针（`ControlPtrT<MenuBarButton>`），增删和查找都基于控件本身；`m_nItemDataId` + `SetUserDataID` 的隐式约定可以同时删除。

### 9. 同一菜单可能被关闭后立即重建（中）

`ShowPopupMenu`（`MenuBar.cpp#L344-L415`）无条件先 `HidePopupMenu()` 再 `new Menu`。在状态不同步（第 2 条）或程序重复调用 `SetActiveTopMenuIndex` 时，会出现同一菜单“关掉又立刻重开”的闪烁，并重复解析 XML、创建窗口。

建议先比较 `m_pActiveMenu->GetRelatedControl()`，相同则直接返回或只同步状态。

### 10. 菜单 XML 加载失败无处理（中）

`ShowPopupMenu`（`MenuBar.cpp#L378-L383`）不检查 `ShowMenu()` 的结果。`xml_path` 写错时会显示一个空菜单窗口，按钮仍被置为按下状态。建议在 `ShowMenu` 后检查窗口是否有效/是否加载到菜单项，失败则关闭并恢复按钮状态。

### 11. 其他小问题（低）

- `AddTopMenuToUI`（`MenuBar.cpp#L89-L114`）：`AddItemAt` 失败时仍继续 `AttachClick`，新控件没有挂到控件树上，造成泄漏。
- `MenuBar.h#L74-L88`：`InsertTopMenu` 的注释缺少 `menuItemId`、`menuTextId` 两个参数。
- `MenuBar.h#L100`：`GetTopMenu()` 不修改成员，应声明为 `const`。
- `MenuBar.h#L135`：注释写“在UI容器中的控件ID号”，实际是内部自增 ID 计数器分配的值。
- `MenuBar.cpp#L340-L342`：`OnMenuMouseButtonUp()` 为空实现。
- `docs/Control.md#L784` 写“鼠标移入或点击顶级菜单后弹出”，实际首次打开必须点击，悬停只在已有菜单展开时才切换；文档与实现不一致。

## 三、功能不足（建议补齐）

1. 缺少激活状态事件：只能在菜单项被激活时收到回调，无法感知“顶级菜单展开/收起/激活索引变化”。建议增加 `AttachMenuBarActiveChanged` 或 `kEventMenuBarPopupOpened/Closed`。
2. 缺少运行时维护接口：没有 `UpdateTopMenuText()`、`SetTopMenuEnabled()`、`GetTopMenuButton()`、`RemoveAllTopMenus()`、`ClosePopupMenu()`（`HidePopupMenu` 目前是私有）；`GetTopMenu` 只能拿到配置数据，拿不到按钮控件，因而无法设置图标、Tooltip 或禁用状态。
3. 键盘可访问性不完整：除第 4 条外，还缺少 `Alt`/`F10` 激活菜单栏、聚焦后按 `Down` 展开、`Esc` 收起并回到菜单栏、方向键在菜单栏上的直接导航（目前必须在菜单已展开时由 `Menu` 转发）。同时没有键盘焦点视觉反馈。
4. 悬停策略不可配置：没有“悬停 N 毫秒后切换”“点击展开/悬停展开”之类的策略；对触屏或高灵敏度鼠标不友好。
5. XML 能力不足：每项不支持 `class`/`attributes`/`enabled`/弹层对齐/弹层偏移；菜单栏本身也没有统一的默认样式属性（如 `item_class`）。
6. 溢出处理缺失：顶级菜单总宽度超过菜单栏时没有折叠为“»”或横向滚动，英文/中文切换后长文本容易被裁剪。
7. 菜单加载无缓存：每次弹出都重新解析菜单 XML 并创建窗口，悬停切换频繁时开销和闪烁都更明显。
8. 窗口联动缺失：主窗口移动、缩放、最小化、菜单栏隐藏时，弹层不会跟随或关闭。
9. 弹层对齐策略单一：`ShowPopupMenu` 固定使用 `MenuPopupPosType::LEFT_TOP`，最右侧菜单靠近屏幕边缘时只能被“智能”平移，无法右对齐到按钮。

## 四、性能与健壮性

- 每次弹出都新建 `Menu` 窗口并解析 XML（`MenuBar.cpp#L378-L383` -> `Menu.cpp#L107-L190`）。悬停切换会反复触发；建议复用同一个 `Menu` 窗口（切换时只换 XML）或缓存解析结果。
- `RemoveTopMenuFromUI` / `ShowPopupMenu` 采用线性查找（`MenuBar.cpp#L117-L127`、`MenuBar.cpp#L355-L363`）。顶级菜单数量通常很少，问题不大；改为保存控件指针后可同时简化。
- 弱指针保护使用正确：菜单项激活回调和窗口关闭回调都检查了 `menuBarFlag`，`m_pActiveMenu` 使用 `ControlPtrT`，不会因控件销毁而野指针。
- 健壮性缺口主要集中在：菜单加载失败、`AddItemAt` 失败、移除激活项、窗口移动/隐藏时弹层未关闭。

## 五、界面美观度改进建议

当前样式只有 `menu_bar_button`（`bin/resources/themes/default/global.xml#L613-L618`），且只设置了 `normal_text_color`，没有 `menu_bar` 容器样式。

1. 补充 `menu_bar` 容器样式类（`global.xml#L613` 附近）：

```xml
<Class name="menu_bar" height="32" bkcolor="bg_menu_bar"
       padding="6,0,6,0" child_margin="2,0,2,0" valign="center"/>
```

这样 `RenderForm`、`page_control_test.xml` 中手写的 `height/bkcolor/margin/padding/border_round` 可以统一，浅色、深色及中英文布局一致。

2. 完善 `menu_bar_button` 的状态样式：目前悬浮/按下只改变背景色，文字颜色不变，且浅色主题的 `bg_menu_bar_hovered`（`#E5E5E5`，底色 `#F4F4F4`）对比偏弱。建议补充：

```xml
<Class name="menu_bar_button" height="26" width="auto" border_round="4,4" valign="center"
       text_align="vcenter" text_padding="10,4,10,4"
       normal_text_color="text_menu_bar"
       hovered_text_color="text_default"
       pressed_text_color="text_default"
       disabled_text_color="text_disabled"
       normal_color=""
       hovered_color="bg_menu_bar_hovered"
       pressed_color="bg_menu_bar_pressed"
       hovered_color_margin="2,0,2,0"
       pressed_color_margin="2,0,2,0"/>
```

`hovered_color_margin`/`pressed_color_margin` 可让悬浮块之间留出间隙，避免相邻项“连成一片”。

3. 区分“悬浮”和“菜单已展开”。现在展开状态复用 `kControlStatePressed`，浅色主题下 `#DBDBDB` 与悬浮 `#E5E5E5` 只差约 6% 亮度，视觉上难以区分。建议：
   - 提高 `bg_menu_bar_pressed` 的对比度，或
   - 给按下状态增加强调色（如 `pressed_border_color="color_accent"` + `border_size="1"`），或
   - 在激活项下方绘制 2px 强调色下划线（需要 `MenuBar` 侧增加激活指示控件/绘制逻辑）。

4. 统一命中区域和间距：目前 `menu_bar_button` 为 `width="auto" height="auto"`，悬浮背景高度只有“文字 + 上下 4px”，在 36px 高的菜单栏里偏小。建议给按钮设置 `height` 或 `min_height`（如 26~28），并给 `MenuBar` 设置 `child_margin`，中英文下命中区域一致。

5. 补充键盘焦点视觉：顶级按钮默认可 Tab 聚焦，但主题里没有任何焦点态样式，用户看不出焦点在哪个菜单上。建议增加焦点描边（可复用 `border_focus_ring` 之类的主题色）或至少改变文字颜色。

6. 弹层对齐和边缘处理：最右侧菜单靠近屏幕右边缘时，智能模式只是把弹层整体左移（`Menu.cpp#L477-L490`），不会把弹层右边与按钮右边对齐，视觉上会“偏移一截”。建议为菜单栏弹层提供“右对齐到按钮”的选项；靠近底部时也要避免弹层向上覆盖菜单栏本身。

7. 多语言布局：`text_id` 机制本身可用，但顶级项宽度为 `auto`，中英文长度差异会改变整条菜单栏宽度。建议：
   - 用 `child_margin` 保证项间最小间距；
   - 为菜单栏预留右侧 `stretch` 区域；
   - 对超长菜单栏设计“»”折叠（见第三节第 6 条），避免英文长文本被裁剪。

## 六、建议实施顺序

1. P0（行为正确性）：修正键盘左右索引；用真实弹层状态替代 `m_bActiveState`；修复 Enter/Space 打开；移除激活菜单时关闭弹层；`ShowPopupMenu` 增加可用性校验。
2. P1（结构健壮性）：`TopMenuData` 改为保存按钮控件指针；同一项不重复重建；菜单加载失败处理；`AddItemAt` 失败释放资源。
3. P2（XML/主题）：支持 `MenuBarItem` 的 `class`/`attributes`；新增 `menu_bar` 样式类；完善 `menu_bar_button` 状态颜色与焦点样式；调整浅色主题悬浮对比度。
4. P3（功能增强）：激活状态事件、运行时更新/禁用 API、Alt/F10/方向键导航、悬停延迟、溢出折叠、菜单窗口复用与缓存、窗口移动/隐藏时自动关闭弹层。

## 七、验证建议

若后续实施修改：使用非 DLL 的 Debug 配置编译验证；简体中文与英文界面分别检查菜单栏宽度、间距和悬浮/展开状态；重点回归“键盘左右切换（含隐藏/禁用项）”“悬停切换后单击”“移除激活项”“窗口移动时弹层行为”四类场景。
