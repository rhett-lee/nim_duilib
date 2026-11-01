# Checklist: D-14 Popover 气泡卡片

> 全部勾选才视为完成（DoD）。

## 1. 源码

- [x] `duilib/Control/Popover.h` / `.cpp` 已新增，命名空间 `ui`。
- [x] `GetType()` 返回 `DUI_CTR_POPOVER`。
- [x] 基类为 `Label`（`typedef Label BaseClass`），锚点文本走 `Label` 的 `text` / `text_id` / `font` / `text_align`。
- [x] 构造函数初始化全部成员：`m_trigger = PopoverTrigger::kClick`、`m_placement = PopoverPlacement::kTop`、`m_nGap = 6`、`m_nCardWidth = 240`、`m_bArrow = true`、`m_bAutoDismiss = true`、`m_nHoverDelayMs = 100`、`m_nCloseDelayMs = 200`、`m_bRebuilding = false`。
- [x] XML 属性齐全：`title` / `title_id` / `content` / `content_id` / `content_xml` / `trigger` / `placement` / `gap` / `card_width` / `arrow` / `auto_dismiss` / `hover_delay` / `close_delay`，均支持无下划线别名，未知属性回退 `BaseClass::SetAttribute()`。
- [x] `placement` 解析忽略大小写与 `_` / `-`（`top_left`、`topLeft`、`top-left` 等价）。
- [x] `card_width` 钳到最小 120；`gap` / `hover_delay` / `close_delay` 负值归零。
- [x] 卡片 XML 由 `BuildCardXml()` 动态生成；标题/正文经 `EscapeXmlText()` 转义 `& < > " '`。
- [x] 卡片浮层复用 `Flyout`（非模态、不抢焦点、点外部/Esc 自动关闭、关闭后框架自动 `delete`）。
- [x] 12 方位映射到 `Flyout::Placement`（`ToFlyoutPlacement()`），越界翻转由 `Flyout::FlipPlacement()` 统一处理。
- [x] 不自持 `Flyout*`：用 `Flyout::GetActiveFlyout()` + `GetAnchor() == this` 查询，`AttachClosed` 回写状态。
- [x] 悬停延迟任务 `PostDelayedTask` + `CancelTask`，`m_nOpenTaskId` / `m_nCloseTaskId` 在析构、语言切换、浮层关闭时统一取消。
- [x] `m_bDestroying` 守卫：析构期间浮层关闭回调不回派事件。
- [x] `OnLanguageChanged()` 先走基类再 `Hide()`（卡片 XML 在打开时生成，切换语言后重新打开即新文案）。
- [x] `OnSetVisible(false)` / 锚点失效时关闭卡片，不残留浮层。
- [x] 跨窗口悬停判定用屏幕物理坐标（`GetPos()` + `GetScrollOffsetInScrollBox()` + `Dpi().ClientSizeToWindowSize()` + `ClientToScreen()`）。
- [x] 卡片内控件访问：`GetCardWindow()` 返回当前显示中的卡片窗口（未显示为 `nullptr`），`FindCardControl(name)` 在其上
      `FindControl`，卡片未显示时返回 `nullptr`；两个 API 均为 `const`，不引入额外状态。
- [x] 尖角箭头：`BuildCardXml(placement)` 生成"卡片按方位留 8 像素箭头带 + 浮动箭头"结构，箭头方向按方位选
      `popover_arrow_up/down/left/right`；箭头为 `float="true" keep_float_pos="true"`、排在卡片之后（绘制在卡片之上），
      尺寸含 1 像素压入卡片的重叠区（水平 16×9、垂直 9×16）。
- [x] 箭头定位：`Flyout` 新增 `AttachLayouted()` + `OnInitLayout()`（首帧布局后、首帧绘制前触发一次），
      `Show()` 在 `ShowAt()` 之前注册回调；`Popover::PositionCardArrow()` 用锚点物理屏幕矩形经 `ScreenToClient()`
      换算到浮层布局坐标，按卡片实际矩形与锚点中心定位箭头（`ClampArrowOffset()` 钳位、距卡片相邻边至少 12 像素）。
- [x] 方位翻转后的重建：`Show()` 最多尝试 2 次，`Flyout::GetActualPlacement()` 与 `GetPlacementSide()` 比较主方位，
      不同于请求方位时以 `m_bRebuilding` 守卫关闭旧浮层（`OnFlyoutClosed` 提前返回、不派发事件），按实际方位重建一次。

## 2. Flyout 地基扩展

- [x] `Placement` 追加 `BottomCenter` / `TopCenter` / `RightCenter` / `LeftCenter`，原 8 个枚举值不变（向后兼容）。
- [x] 翻转判定抽成匿名命名空间函数 `FlipPlacement()`，`PositionAroundAnchor()` 的翻转与目标方位统一走该函数。
- [x] `PositionAroundAnchor()` 覆盖 12 个方位（含新增 4 个居中方位）。
- [x] 阴影交互：`SetClickOutsideDismiss` / `IsClickOutsideDismiss`（默认 true）、`SetShadowClickThrough` / `IsShadowClickThrough`
      （默认 true）、`GetVisualRect` / `IsPointInVisualArea` 已实现；`GetVisualRect()` = 窗口矩形 `Deflate(GetCurrentShadowCorner())`。
- [x] `OnInitWindow()` 安装 `ShadowHitTestFilter`（`IUIMessageFilter`，Windows 非 SDL 宏保护），`WM_NCHITTEST` 落在可视区域外返回
      `HTTRANSPARENT`；`OnFinalMessage()` `RemoveMessageFilter()` 并释放。
- [x] `OnKillFocusMsg()` / `OnDetectTick()` 的"点击外部"判定受 `m_bClickOutsideDismiss` 控制且改用 `GetVisualRect()`；
      Esc 关闭仍受 `m_bAutoDismiss` 控制。
- [x] `Popover` 新增 `m_bAutoDismissDefined`：`trigger="manual"` 且未显式设置 `auto_dismiss` 时，`Show()` 传
      `SetClickOutsideDismiss(false)`（受控模式默认只由 API/Esc 关闭）；`IsMouseOverAnchorOrCard()` 改用 `GetVisualRect()`。
- [x] 新增 `GetActualPlacement()`：记录翻转后的实际方位（`m_actualPlacement` 由 `PositionAroundAnchor()` 写入），供 `Popover` 重建箭头方向。
- [x] 卡片窗口改用自定义阴影：`shadow_type="custom"` + `shadow_image`（按箭头方位选择
      `public/shadow/shadow_popover_arrow_{up,down,left,right}.svg`，无箭头时 `shadow_popover.svg`；
      `window_shadow_mode='true'`、`corner` 在箭头一侧为"阴影边距 + 8 箭头带 + 8 卡片圆角"）
      + `shadow_corner="12,12,14,14"` + `shadow_border_round="0,0"` + `shadow_border_size="0"` + `shadow_snap="false"`
      （比 `small_round` 更紧凑；阴影主体在箭头一侧内缩 8 以与卡片边缘对齐）。
- [x] 非对称 `corner` 不再误报断言：`ImageAttribute::ModifyAttribute()` 的 `corner` 解析改为
      `ParseRectValue(value, *m_rcCorner, false)`（`corner` 的 4 个值是四条边各自的内缩量，不是矩形），
      四边仍各自钳到 `>= 0`；`src` / `dest` / `size_box` / `caption` 等矩形语义属性保持尺寸校验。
- [x] 跟随锚点：`Flyout` 新增 `SetFollowAnchor(bool)` / `IsFollowAnchor()`（默认 `false`）与成员
      `m_bFollowAnchor` / `m_rcAnchorScreenLast`；`ShowAt()` 定位后记录锚点屏幕矩形（物理像素）。
- [x] 锚点可见性判定：`IsAnchorVisible()` 用"锚点布局矩形扣除 `GetScrollOffsetInScrollBox()` 累计滚动偏移"后的
      矩形，与窗口客户区、各级 `ScrollBox` 祖先的可视矩形依次求交，任一为空即视为不可见。
- [x] 重新定位：`UpdateAnchorPosition()` 在已打开状态下复用当前窗口调用 `PositionAroundAnchor()`，刷新
      `m_rcAnchorScreenLast` 并派发布局完成回调，返回实际方位是否变化（不重建窗口、不派发打开事件）。
- [x] 布局完成回调可重复触发：`AttachLayouted()` 的回调改由 `InvokeLayoutCallbacks()` 统一派发，
      `OnInitLayout()`（首帧）与 `UpdateAnchorPosition()`（重定位后）都会触发，`Popover::PositionCardArrow()`
      因此按新的锚点中心重新定位尖角。
- [x] `OnDetectTick()` 的跟随分支（约 50ms）：锚点不可见 → `DoClose(CloseReason::kAnchorLost)`；
      锚点屏幕矩形变化 → 临时 `m_bAllowFlip = false` 后 `UpdateAnchorPosition()`（保持打开时的方位，
      该侧空间不足时由定位逻辑夹持到显示器工作区），避免滚动时卡片突然翻转或消失。
- [x] `StartDetectTimer()` 的 `PostDelayedTask` 移出 Windows 宏（跨平台可用），启动条件放宽为
      `m_bNoFocus || m_bFollowAnchor`；`Popover::Show()` 打开卡片时调用 `SetFollowAnchor(true)`。

## 3. 注册

- [x] `duilib_defs.h` 新增 `DUI_CTR_POPOVER` 与 `kEventPopoverOpenChanged`（置于 `kEventLast` 之前）。
- [x] `WindowBuilder.cpp` 新增 include 与工厂 map 项。
- [x] `duilib.vcxproj` 登记 `.h/.cpp`。
- [x] `duilib.vcxproj.filters` 登记并归入 `Control`。
- [x] `duilib.h` 新增 include。
- [x] `EventArgs.cpp` 的 `InitEventStringMap` 追加 `kEventPopoverOpenChanged` / `PopoverOpenChanged` / `popover_open_changed`。

## 4. 主题与示例

- [x] `global.xml` 新增 10 个 Class：`popover_trigger` / `popover_anchor` / `popover_anchor_primary` / `popover_card` /
      `popover_arrow_up` / `popover_arrow_down` / `popover_arrow_left` / `popover_arrow_right` / `popover_title` / `popover_content`；
      新增变量 `PATH_POPOVER`（`public/popover`）。
- [x] 新增图片资源：`public/popover/popover_arrow_up|down|left|right.svg`（水平 16×9 / 垂直 9×16，含 1 像素压入卡片的重叠区；
      填充 `#FFFFFF|bg_window_card`、描边 `#D5D5D5|border_window` 经 `svg_replace_colors` 映射语义色）、
      `public/shadow/shadow_popover.svg` 与 4 个方位专用阴影 `public/shadow/shadow_popover_arrow_{up,down,left,right}.svg`
      （模糊 4、偏移 2、边距 12/14，箭头一侧的阴影主体再内缩 8）。
- [x] 全部颜色为语义色名（`text_link_*` / `text_disabled` / `bg_btn_*` / `border_btn_*` / `bg_primary_btn_*` / `border_primary_btn_*` /
      `text_btn_*` / `text_primary_btn_*` / `bg_window_card` / `border_window` / `text_default` / `text_muted`），无写死色值；
      字体为 `system_regular_14` / `system_bold_14`。
- [x] 锚点按钮 Class 的普通 / 悬停 / 按下 / 禁用四态配色分别来自 `bg_*` / `border_*` / `text_*` 语义色，`cursor_type="hand"`、`text_align="hcenter,vcenter"`。
- [x] `examples/controls/PopoverForm.h/.cpp` 存在，`OnInitWindow` 依次调用
      `BindBasicPopover()` / `BindTriggerPopovers()` / `BindControlledPopover()` / `BindRichPopover()`。
- [x] `BindBasicPopover()` 用 `content_xml` 拼两行正文；`BindTriggerPopovers()` 为 `manual` 锚点配"打开卡片"按钮；
      `BindControlledPopover()` 回显状态文本（含关闭原因映射）+ 显示/隐藏/切换/换样式 4 个按钮；
      `BindRichPopover()` 在 `AttachOpenChanged`（`wParam == 1`）里用 `FindCardControl()` 绑定卡片内"从浮层内关闭"按钮。
- [x] `bin/resources/themes/default/controls/popover.xml` 存在；`class` 属性写在最前；5 张卡片与 Ant Design 演示结构一致
      （基本用法 / 三种触发方式 + 手动 / 12 方位 / 受控模式 / 复杂内容）。
- [x] 12 方位用 5 行 `HBox` + `stretch` 占位组成环形布局（TL/Top/TR → LT/RT → Left/Right → LB/RB → BL/Bottom/BR），
      每个锚点的 `placement` 与文案一一对应。
- [x] `test.xml` 有 Popover 分组与 `popover_btn` 入口。
- [x] 按钮间距用 `child_margin_x`（三种触发行 8、12 方位行 6）：`child_margin` 只解析单个整数（会把 Y 方向一起设上），不采用。
- [x] 长文案按钮（`popover_manual` / `popover_controlled` / `popover_rich` / `btn_popover_style`）为
      `width="auto" padding="12,0,12,0"`，文字不被截断。
- [x] `TestForm.h/.cpp` 有 `ShowPopoverWindow()` 与入口绑定。
- [x] `controls.vcxproj` / `.filters` 登记新示例文件。
- [x] 中英 examples 语言文件键集合一致（`STRID_POPOVER_*` 段各 57 个键），UTF-8 无 BOM、中文无乱码。

## 5. 文档与计划

- [x] `docs/Control.md` 新增 `## Popover 控件（气泡卡片）`（属性表 / C++ 接口表 / 枚举 / 事件 / 皮肤 Class / 示例 / 注意事项）。
- [x] `docs/Control.md` 的 C++ 接口表含 `GetCardWindow()` / `FindCardControl(name)`，皮肤 Class 段为 6 个 Class 的表格（含 `popover_anchor` / `popover_anchor_primary`）。
- [x] `docs/Control.en.md` 新增对应英文章节（锚点 `#popover-control-bubble-card`），内容与中文版保持一致。
- [x] `docs/Control*.md` 的 `Flyout::Placement` 枚举行补全 4 个新值。
- [x] `docs/Summary.md` / `Summary.en.md` 新增 Popover 行与有效锚点。
- [x] `docs/misc/specs/D-14-Popover/{spec,tasks,checklist}.md` 已创建并随示例改造更新（T9 / T10、6 个 Class、示例结构验收项）。
- [x] `Development-Plan.md` P2 进展汇总、D 组表格 D-14 行、P2 任务行已回写。
- [x] `Project-Audit.md` 的 B8（富内容 Tooltip）已注明改由 `Popover` 覆盖。
- [x] `docs/Control*.md` 的 Flyout 章节补"阴影不拦截鼠标"说明与新增 API（`SetClickOutsideDismiss` / `SetShadowClickThrough` /
      `GetVisualRect` / `IsPointInVisualArea`）；Popover 章节说明 manual 默认不因外部点击关闭、hover 判定用可视区域。
- [x] `.trae` / `.workbuddy` 两份 `api-reference.md` 内容一致（`Get-FileHash` 相同，均含新增 2 个 API 与 3 个锚点 Class），`.claude/docs/nim-duilib-llm-reference.md` 速查行与 `Placement` 已同步。
- [x] `.trae` / `.workbuddy` 两份 `api-reference.md` 同步 T11 的 Flyout 新 API（`SetClickOutsideDismiss` / `SetShadowClickThrough` /
      `GetVisualRect` / `IsPointInVisualArea`）与 `shadow_type="small_round"` 说明。
- [x] T12 文档同步：`docs/Control.md` / `Control.en.md` 的 Popover 章节补 `arrow` 属性行、`SetArrow/IsArrow` 接口行、
      10 个皮肤 Class（含 4 个 `popover_arrow_*`）与箭头/阴影资源说明，`gap` 语义与"卡片不绘制箭头"的注意事项改写。
- [x] T12 文档同步：两份 `api-reference.md`（`Get-FileHash` 相同）与 `.claude/docs/nim-duilib-llm-reference.md` 速查行补
      `arrow`、4 个箭头 Class、`GetActualPlacement()` 与 `shadow_type="custom"` + `shadow_popover.svg`。
- [x] T13 文档同步：Control.md / Control.en.md 的 Popover 章节改为"箭头带 + 浮动箭头 + 方位专用阴影"，
      箭头尺寸与资源说明、阴影图按方位选择、`PositionCardArrow` 定位与 `AttachLayouted` 说明全部更新；
      两份 `api-reference.md`（`Get-FileHash` 相同）与 `.claude/docs/nim-duilib-llm-reference.md` 速查行同步。
- [x] 本 spec（3.9 尖角箭头与更紧凑的阴影）、`tasks.md`（T12/T13）、`checklist.md`（尖角验收项）已同步。
- [x] `README.md` / `README.en.md` 无逐控件清单条目（与最近新增控件一致），未改动。

## 6. 验证命令

- [x] `MSBuild "build\examples.sln" /t:duilib /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo` 退出码 0。
- [x] `MSBuild "examples\controls\controls.vcxproj" /p:Configuration=Debug /p:Platform=x64 /p:SolutionDir=<repo>\build\ /clp:ErrorsOnly /m:1 /nologo` 退出码 0（`bin\controls_d.exe` 已重新链接）。
- [x] `python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0。
- [x] T13 复查：`MSBuild "duilib\duilib.vcxproj" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo` 退出码 0，
      `MSBuild "examples\controls\controls.vcxproj" ... /p:SolutionDir=<repo>\build\` 退出码 0（`bin\controls_d.exe` 重新链接）。
- [x] T13 复查：`verify_docs.py` 退出码 0（234 个文档全部命中，未发现文档漂移）；`git diff --check` 退出码 0。
- [x] T14 复查：`duilib.vcxproj` 与 `controls.vcxproj`（Debug|x64）在修改 `ImageAttribute::ModifyAttribute()`
      后重新编译，退出码均为 0。
- [x] T15 复查：`duilib.vcxproj` 与 `controls.vcxproj`（Debug|x64）在新增跟随锚点能力后重新编译，退出码均为 0。
- [x] T15 复查：`verify_docs.py`（`--extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang`）
      退出码 0；`git diff --check` 退出码 0。
- [x] 未执行 `git commit`（由用户手动提交）。

## 7. 功能验收（手工）

> 以下为运行时人工验收项，由用户在示例程序中确认。

### 示例演示（对齐 Ant Design 演示页）
- [ ] 基本用法卡片：主色实心按钮"悬停我"，悬停弹出"标题 + 两行正文"卡片（对应 antd 的 Title + Content + Content）。
- [ ] 三种触发方式卡片：`Hover me` / `Focus me` / `Click me` 三个描边按钮行为各自正确；`Manual (API)` 锚点点击无反应，右侧"打开卡片"按钮可 Show/Toggle。
- [ ] 位置卡片：12 个按钮按环形排布（TL/Top/TR → LT/RT → Left/Right → LB/RB → BL/Bottom/BR），每个按钮的卡片相对锚点的方位与其文案一致。
- [ ] 受控模式卡片：显示 / 隐藏 / 切换 / 切换宽度方位 四个按钮可用；切换宽度方位后卡片按新配置重建；状态文本含关闭原因。
- [ ] 复杂内容卡片：卡片内多行文本 + 链接色文本 + "从浮层内关闭"按钮，按钮点击后卡片关闭（即 `FindCardControl()` 绑定生效）。
- [ ] 锚点按钮三态配色（普通 / 悬停 / 按下）与禁用态配色正确，配色来自语义色（`bg_btn_*` / `bg_primary_btn_*` 等），主题切换后同步变化。
- [ ] 语言切换：整页文案（含卡片标题、正文、状态文本）随语言切换更新；卡片若已打开会被关闭，重新打开为新语言文案。

### 创建与属性
- [ ] XML 创建成功，`FindControl("popover_click")` 可取到；无 `class` 时正常显示锚点文本。
- [ ] `title` / `title_id` 与 `content` / `content_id` 均生效（后者按 STRID 解析）。
- [ ] `content_xml` 富内容（多行 `Label`、按钮）渲染且按钮可点击。
- [ ] `gap` / `card_width` / `hover_delay` / `close_delay` 生效；`card_width` 小于 120 时按 120 渲染。
- [ ] 全部颜色属性与 `font` 实际生效；未知属性不崩。

### 交互与事件
- [ ] `trigger="click"`：点击锚点弹出，再点锚点关闭。
- [ ] `trigger="hover"`：移入 `hover_delay` 后弹出；移出后可从锚点移进卡片继续操作；离开两者 `close_delay` 后关闭。
- [ ] `trigger="focus"`：Tab 聚焦弹出，失焦关闭。
- [ ] `trigger="manual"`：点击锚点无反应，仅 `Show()` / `Toggle()` 生效。
- [ ] 12 方位均正确；靠近屏幕边缘时翻转到对侧且卡片完整可见。
- [ ] `AttachOpenChanged` 的 `wParam`（1/0）与 `lParam`（关闭原因）正确，状态文本回显一致。
- [ ] 点击卡片外部 / 按 Esc 关闭；`auto_dismiss="false"` 时点外部不关闭（仍可按 Esc 或 `Hide()` 关闭）。

### 阴影交互（T11 修复项）
- [ ] 卡片打开时，点在卡片阴影上的相邻按钮可正常点击（阴影区 `HTTRANSPARENT` 穿透，`WM_NCHITTEST` 过滤生效）。
- [ ] `trigger="hover"` 的锚点：卡片弹出后鼠标离开锚点，`close_delay` 后卡片消失（阴影覆盖不再让卡片常驻）。
- [ ] 卡片显示时鼠标移入卡片正文，卡片保持打开（可视区域内仍视为"在卡片内"）。
- [ ] 受控模式"切换"按钮：点一次打开、再点一次关闭；状态文本与 `IsOpen()` 一致。
- [ ] 受控模式"切换宽度/方位"按钮：卡片按新宽度与方位重建（关闭后重新打开可见变化）。
- [ ] `trigger="manual"` 且未显式配置 `auto_dismiss` 时，点击卡片外部不关闭（Esc 仍可关闭）；显式 `auto_dismiss="true"` 可恢复点外部关闭。

### 边界与生命周期
- [ ] 同一锚点重复 `Show()` 不产生第二张卡片；切换锚点会替换旧卡片。
- [ ] 锚点控件所在窗口关闭 / 滚动 / 移动 / DPI 变化时卡片位置正确且不残留。
- [ ] 卡片打开状态下切换语言：卡片关闭，重新点击得到新语言文案。
- [ ] 反复打开/关闭、销毁锚点控件均不崩溃、不出现悬垂回调。

### 主题与 DPI
- [ ] 浅色与深色主题下卡片描边、圆角、标题/正文配色正常。
- [ ] 100% / 125% / 150% / 200% DPI 下 `gap`、卡片圆角与文字内边距一致。

### 尖角箭头与阴影（T12/T13）
- [ ] 12 个方位下尖角箭头指向锚点，尖角落在卡片朝向锚点的那条边上，箭头底边与卡片边线连续（无 1px 断线）。
- [ ] 箭头沿卡片边**对准锚点中心**（水平方位对锚点中心 X、左右方位对锚点中心 Y），而不是固定在卡片起始端；
      卡片过窄时箭头居中，且距卡片相邻边至少 12 像素（不压到圆角）。
- [ ] 卡片箭头一侧**没有纯白实心带**：箭头带（8 像素）内只有阴影渐变，与窗口阴影带（12/14）连续过渡，
      在箭头带与阴影带之间看不到硬裁切的边界。
- [ ] 卡片 1 像素边线在箭头宽度范围内被箭头覆盖，斜边与卡片边线自然衔接（无横穿箭头的直线、无锯齿感）。
- [ ] 水平方位（`top*` / `bottom*`）与左右方位（`left*` / `right*`）的 4 种箭头朝向、4 张方位专用阴影图都正确对应。
- [ ] 靠近屏幕边缘触发翻转后，箭头仍指向锚点（卡片按实际方位重建）；过程中看不到"两张卡片"闪烁，也没有重复的打开/关闭事件。
- [ ] `arrow="false"` / `SetArrow(false)` 后卡片不再有箭头、不预留箭头带，阴影回退到 `shadow_popover.svg`，卡片位置、尺寸正常。
- [ ] 深浅色主题下箭头填充与卡片底色一致、描边与卡片边线一致（`svg_replace_colors` 生效）。
- [ ] 阴影明显比 T11 的 `small_round` 更紧凑（四周留白 12/14），且点在阴影上仍能命中相邻按钮。
- [ ] 点击箭头区域不会误触发卡片内控件（箭头 Class 的 `mouse_enabled="false"`）。
- [ ] 显示箭头时 `gap` 感觉为"尖角尖端到锚点"的距离（默认 6），箭头未插入锚点区域。
- [ ] 100% / 150% / 200% DPI 下箭头宽度、压入卡片的 1 像素重叠区与箭头带比例正确（无错位、无空隙）。

### 滚动跟随（T15）
- [ ] 卡片打开后滚动锚点所在的滚动容器（滚轮或拖动滚动条）：卡片在约 50ms 内跟随锚点重新定位，不出现明显脱节。
- [ ] 跟随过程中尖角箭头仍指向锚点中心（水平方位对锚点中心 X、左右方位对锚点中心 Y），阴影与卡片边线不错位。
- [ ] 跟随过程中卡片方位保持不变（不因锚点靠近边缘而翻转成另一侧），锚点滚到接近窗口边缘时卡片被夹持在显示器工作区内。
- [ ] 锚点被滚出滚动容器的可视区域（或滚出窗口客户区）后，卡片按"锚点失效"自动关闭，不残留浮层窗口。
- [ ] 状态文本 / `AttachOpenChanged` 在自动关闭时回显的关闭原因为 `kAnchorLost`（`wParam == 0`），后续点击可正常重新打开。
- [ ] 卡片不随锚点滚动时误触发重复的打开事件；反复滚动、滚出、重新打开均不崩溃、不出现悬垂回调。
