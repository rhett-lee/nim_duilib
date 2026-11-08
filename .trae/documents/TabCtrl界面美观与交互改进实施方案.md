# TabCtrl 界面美观与交互改进实施方案

## 一、背景与目标

上一阶段已完成 TabCtrl C1–C10 能力（TabBox 同步、关闭事件、键盘导航、溢出箭头、等宽填充等）。本阶段针对「界面美观与交互」的 8 个问题做增强：

1. 指示条切换从瞬间跳变改为位置/宽度插值动画（Ant Design 质感）。
2. 选中态增加层次：支持选中标题字体（加粗）与可选淡底色。
3. 选中标签再 hover 时给出视觉反馈。
4. 修复垂直模式分隔线/间距错位。
5. 底边线断开策略可配置（卡片风格断开、线风格连续）。
6. 补齐键盘焦点框与禁用态。
7. 角标支持右上角 float 模式，避免长标题/固定宽度下被裁。
8. 关闭按钮在指示条风格下支持“仅 hover 显示”。

约束：新增公开属性必须同步 `docs/Control.md`、`docs/Control.en.md`、`docs/schema/nim_duilib.xsd`；中英文案一致；不执行 git 提交。

## 二、新增属性与 API 设计

### TabCtrl（标签栏级）

| 属性 | 默认值 | 说明 | 对应 API |
| --- | --- | --- | --- |
| `selected_tab_item_indicator_animation` | `true` | 选中指示条是否用位置/宽度插值动画 | `SetEnableIndicatorAnimation/IsEnableIndicatorAnimation` |
| `selected_tab_item_indicator_animation_duration` | `200` | 动画时长（毫秒，≤0 视为关闭） | `SetIndicatorAnimationDuration/GetIndicatorAnimationDuration` |
| `tab_ctrl_bottom_line_mode` | `auto` | 底边线断开策略：`auto`（选中项显示背景时断开）/`break`（始终断开）/`full`（始终连续） | `SetTabCtrlBottomLineMode/GetTabCtrlBottomLineMode` |

焦点态复用 Control 已有的通用属性：`show_focused_rect`（TabCtrl 构造时默认 `true`）与 `focused_rect_color`（为空时使用主题焦点色）。TabCtrl 重写 `PaintFocusRect` 为空，改为在 `PaintForeColor` 中围绕选中标签绘制点线焦点框，避免整条标签栏外框。

### TabCtrlItem（标签项级）

| 属性 | 默认值 | 说明 |
| --- | --- | --- |
| `selected_title_font` | 空 | 选中时的标题字体 ID（如 `system_bold_12`），为空保持原字体 |
| `selected_hovered_title_color` | 空 | 选中且 hover 时的标题颜色，为空沿用选中颜色 |
| `disabled_title_color` | 空 | 禁用态标题颜色，为空沿用原状态颜色 |
| `badge_float` | `false` | 角标改为右上角浮动模式，不占用标题布局宽度 |
| `close_button_show_mode` | `auto` | 关闭按钮显示策略：`auto`（沿用 `auto_hide_close_button` 现有逻辑）/`hover`（仅 hover 显示，选中也不常显）/`always`（始终显示） |

## 三、逐项实现

### 1. 指示条滑动动画
文件：`duilib/Control/TabCtrl.h/.cpp`

- 将 `PaintSelectedTabIndicator` 中的矩形计算抽取为 `bool CalcSelectedIndicatorRect(UiRect& rcIndicator)`，绘制与动画共用。
- 新增状态：`UiRectF m_rcIndicatorLast`、`m_rcIndicatorFrom`、`m_rcIndicatorTo`、`bool m_bIndicatorAnimating/m_bIndicatorAnimateNext/m_bIndicatorLastValid`、`uint64_t m_nIndicatorAnimStartMs`、`size_t m_nIndicatorTimerId`、`WeakCallbackFlag m_indicatorTimerFlag`。
- 在 `TabCtrl::OnItemSelectedChanged`（重写，基类为空）中，若子项变为选中则设置 `m_bIndicatorAnimateNext = true`，可覆盖 `bTriggerEvent=false` 的选择路径；同时在 `TabCtrlItem::OnPrivateSetSelected` 中补调 `BaseClass::OnPrivateSetSelected()` 以打通该回调。
- 绘制流程：先算目标矩形；若正在动画则按 `EaseOutCubic` 在 from/to 间插值；若动画中目标变化，则以当前绘制矩形为新起点重定向；首次绘制/不可见/时长为 0/方向切换/主题或 DPI 变化/选中清空时直接吸附，不播动画。
- 动画由约 15ms 的可取消定时器驱动，定时器回调只调用 `Invalidate()`；在 `PaintSelectedTabIndicator` 内计算进度并绘制。析构、禁用、隐藏时移除定时器。
- 目标矩形先在标签栏边界内钳制后再插值，避免边缘抖动；自动宽度测量仍只在绘制阶段进行，定时器不测量文字。

### 2. 选中态层次（选中字体 / 淡底色）
文件：`duilib/Control/TabCtrl.cpp`（TabCtrlItem 部分）

- 创建/切换 `title_class` 时记录标题标签默认字体 `m_defaultTitleFontId`。
- 新增 `UpdateTitleFont()`：选中且 `selected_title_font` 非空时 `m_pTitle->SetFontId(...)`，否则恢复默认字体；选择状态变化后调用。
- 淡底色不新增 API：沿用已有 `selected_tab_item_show_bg=true` + 标签项 `selected_normal_color`（可配圆角 `selected_round_corner`），文档给出用法示例。

### 3. 选中 + hover 反馈
文件：`duilib/Control/TabCtrl.cpp`（`TabCtrlItem::PaintStateColors`、`UpdateTitleTextColor`）

- 选中分支绘制选中背景后，若处于 hover/pressed/hover 动画，再调用 `PaintTabItemHovered` 叠加 hover 反馈；禁用态跳过。
- `UpdateTitleTextColor` 增加分支：选中且 hover 且 `selected_hovered_title_color` 非空时使用该颜色（指示条风格可据此获得文字反馈）。
- 指示条风格（`hovered_color` 为空）通过 `selected_hovered_title_color` 体现反馈。

### 4. 垂直模式分隔线/间距错位
文件：`duilib/Control/TabCtrl.cpp`（`TabCtrlItem::UpdateLineOrientation`）

- 根因：垂直分支只改 `width/height/valign/margin`，仍保留类里的 `float=true`、`halign=right`，浮动控件上的 `width=stretch` 无效，导致分隔线塌缩/错位；切回水平又只恢复 margin。
- 修复：进入垂直时显式设置 `float=false, width=stretch, height=1, halign=stretch, valign=bottom, margin=4,0,4,0`；切回水平时若配置了 `line_class` 则 `SetClass` 还原，否则使用首次覆盖前保存的 `float/width/height/halign/valign/margin` 快照还原，避免硬编码并保留用户运行期修改。
- 验证垂直模式下分隔线随标签项宽度铺满、文字与分隔线间距一致。

### 5. 底边线断开策略可配
文件：`duilib/Control/TabCtrl.h/.cpp`

- 新增枚举 `TabCtrlBottomLineMode { kAuto, kBreak, kFull }` 与属性解析（`auto/break/full`）。
- `PaintStateColors`：`kFull` 始终画整条线；`kBreak` 始终在选中项范围断开；`kAuto` 维持现有行为（仅当 `selected_tab_item_show_bg` 为 true 时断开）。
- 垂直模式同样支持断开：按选中项的上/下边界分段绘制右侧分隔线，保证卡片风格在垂直方向也一致。

### 6. 焦点框与禁用态
文件：`duilib/Control/TabCtrl.h/.cpp`

- TabCtrl 构造时 `SetShowFocusedRect(true)`；重写 `PaintFocusRect` 为空。
- 新增 `bool IsFocusWithin() const`：从 `GetWindow()->GetFocusControl()` 沿 parent 链向上查找是否为本控件（覆盖焦点在标签项/关闭按钮等子控件的情况）。
- `PaintForeColor` 中若 `IsShowFocusedRect() && IsFocusWithin()`，围绕选中标签绘制 2px 内缩的点线框，颜色取 `GetFocusedRectColor()`（为空时用 `border_focus_ring`）。
- `TabCtrlItem::ButtonDown` 选中后把焦点交给 TabCtrl；重写 `TabCtrlItem::OnSetFocus/OnKillFocus`，仅调用基类并 `Invalidate` 父 TabCtrl，保证焦点框及时刷新。
- TabCtrlItem 禁用：`UpdateTitleTextColor` 优先使用 `disabled_title_color`；`PaintStateColors` 在禁用时不绘制 hover 叠加。

### 7. 角标 float 模式
文件：`duilib/Control/TabCtrl.cpp`、`bin/resources/themes/default/global.xml`

- `TabCtrlItem` 新增 `m_bBadgeFloat`；`SetBadgeClass` 创建角标后调用 `UpdateBadgeLayoutMode()`。
- float 模式设置角标 `float=true, halign=right, valign=top, margin=0,2,6,0`，使其脱离标题流式布局，长标题/固定宽度下不被挤压。
- `global.xml` 新增类 `tab_ctrl_item_badge_float`（继承 `tab_ctrl_item_badge`，`width="auto" height="16" float="true" halign="right" valign="top" margin="0,2,6,0"`），供 XML 直接引用。
- `AdjustSubItemIndex` 现有 float 角标排序逻辑保持。

### 8. 关闭按钮仅 hover 显示
文件：`duilib/Control/TabCtrl.h/.cpp`

- 新增枚举 `TabCtrlCloseButtonShowMode { kAuto, kHover, kAlways }` 与属性 `close_button_show_mode`。
- `UpdateCloseButtonVisible`：固定/不可关闭项仍隐藏；`kAlways` 始终显示；`kHover` 仅 hover/pressed/hover 动画时显示；`kAuto` 沿用 `auto_hide_close_button` 逻辑。
- `MouseLeave` 统一调用 `UpdateCloseButtonVisible()`，避免与选中态逻辑重复。

## 四、测试界面（controls 示例）

文件：
- `bin/resources/themes/default/controls/tabctrl.xml`
- `examples/controls/TabCtrlForm.cpp/.h`
- `bin/resources/lang/zh_CN-examples.txt`、`en_US-examples.txt`

在现有卡片 ①–⑭ 后追加 ⑮–㉒，每张卡片对应一项改进，全部可手工验证：

| 卡片 | 内容 | 交互控件 |
| --- | --- | --- |
| ⑮ 指示条动画 | 开关动画、切换时长（0/200/600ms）、快速切换按钮 | `btn_anim_toggle`、`btn_anim_duration`、`btn_anim_next`、`label_anim_state` |
| ⑯ 选中态层次 | 开关 `selected_title_font`（regular/bold）、开关淡底色 | `btn_selected_font`、`btn_selected_bg`、`label_selected_state` |
| ⑰ 选中 + hover | 同一标签栏同时展示背景风格与指示条风格 | `label_hover_state`、`tab_ctrl_hover` |
| ⑱ 垂直分隔线 | 水平/垂直切换，观察分隔线与间距 | `btn_line_orient`、`label_line_state` |
| ⑲ 底边线策略 | 循环 auto/break/full | `btn_bottom_line_mode`、`label_bottom_line_state` |
| ⑳ 焦点/禁用 | 聚焦按钮 + 禁用开关，键盘 Tab/方向键观察焦点框 | `btn_focus_tabctrl`、`btn_disable_tabctrl`、`label_focus_state` |
| ㉑ 角标 float | 长标题 + `badge_float` 与流式角标对比 | `btn_badge_float`、`btn_badge_add`、`label_badge_state` |
| ㉒ 关闭按钮 hover | 指示条风格 + `close_button_show_mode="hover"` | `btn_close_mode`、`label_close_state` |

`TabCtrlForm` 增加：
- 新的 `FindTabCtrl` 目标与按钮事件绑定；
- 状态刷新函数（动画/字体/底色/方向/底线/焦点/角标/关闭按钮）；
- 纳入 `RefreshAllState()` 与 `RefreshToggleButtons()`，语言切换时同步刷新；
- 动态标签项继续使用 `CreateTabItem` 并做 DPI 宽度缩放。

所有新增文案使用 `STRID_TABCTRL_*` 键，中英文文件键完全一致。

## 五、文档与 XSD 同步

- `docs/Control.md`、`docs/Control.en.md`：在 TabCtrl / TabCtrlItem 属性表新增上述属性及默认值、C++ 接口，补充：
  - 指示条动画开关/时长；
  - `tab_ctrl_bottom_line_mode` 三种模式；
  - `show_focused_rect` 在 TabCtrl 中默认开启且焦点框绘制在选中标签上；
  - 选中字体/淡底色组合用法；
  - 选中 + hover 的文字颜色；
  - `badge_float` 与 `tab_ctrl_item_badge_float` 类；
  - `close_button_show_mode` 与 `auto_hide_close_button` 的优先级。
- `docs/schema/nim_duilib.xsd`：手工在 TabCtrl 与 TabCtrlItem 属性组中追加同名属性声明，保持现有 boolean/string/float 类型写法；不整权重生成，避免无关差异。

## 六、代码 Review 与 UI 自动化测试

1. 构建：Debug/Release 均通过（命令见下）。
2. Review 重点：
   - `OnPrivateSetSelected` 补调基类后的选择同步与 TabBox 联动是否重复触发；
   - 动画定时器生命周期、重定向、隐藏/删除标签/方向切换时的吸附；
   - 焦点框在 TabCtrl 整体聚焦与子控件聚焦两种路径下是否都能显示；
   - 垂直分隔线还原逻辑是否覆盖未配置 `line_class` 的情况；
   - 新增属性默认值是否保持既有界面外观不变。
3. UI 自动化：启动 controls 示例，进入 TabCtrl 测试页，逐张卡片操作 ⑮–㉒，覆盖：
   - 动画开关/时长/快速连续切换；
   - 选中字体与淡底色开关；
   - 选中后 hover、取消 hover；
   - 水平↔垂直来回切换两次；
   - auto/break/full 三种底线模式；
   - 键盘焦点进入/离开、禁用/启用；
   - float 角标长标题、添加多个角标；
   - 关闭按钮 auto/hover/always 与选中态组合。
   发现问题立即修复并复测，最后重新构建 Debug/Release。

## 七、验证命令

```powershell
$msvc='C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231'; $sdk='C:\Program Files (x86)\Windows Kits\10';
$env:INCLUDE="$msvc\include;$sdk\Include\10.0.26100.0\ucrt;$sdk\Include\10.0.26100.0\um;$sdk\Include\10.0.26100.0\shared;$sdk\Include\10.0.26100.0\winrt";
$env:LIB="$msvc\lib\x64;$sdk\Lib\10.0.26100.0\ucrt\x64;$sdk\Lib\10.0.26100.0\um\x64";
$env:PATH="$sdk\bin\10.0.26100.0\x64;$msvc\bin\Hostx64\x64;$env:PATH";
cmake --build --preset x64-debug --target duilib controls
cmake --build --preset x64-release --target duilib controls
```

运行 controls 示例进入 TabCtrl 页，按第六节清单逐项验证。

## 八、关键文件

- `duilib/Control/TabCtrl.h` / `TabCtrl.cpp`
- `bin/resources/themes/default/global.xml`
- `bin/resources/themes/default/controls/tabctrl.xml`
- `examples/controls/TabCtrlForm.h` / `.cpp`
- `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`
- `docs/Control.md` / `docs/Control.en.md`
- `docs/schema/nim_duilib.xsd`
