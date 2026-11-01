# Tasks: D-14 Popover 气泡卡片

- [x] T1 扩展 `duilib/Control/Flyout.h/.cpp`：`Placement` 追加 `BottomCenter` / `TopCenter` / `RightCenter` / `LeftCenter`，抽出 `FlipPlacement()`，`PositionAroundAnchor()` 覆盖 12 方位。
- [x] T2 新增 `duilib/Control/Popover.h`：`PopoverTrigger` / `PopoverPlacement` / `PopoverCloseReason`、类声明（`Popover : public Label`）、public API 与 private 生命周期方法。
- [x] T3 新增 `duilib/Control/Popover.cpp`：属性解析、方位/触发解析、XML 转义与卡片生成、打开/关闭生命周期、悬停延迟任务、屏幕坐标判定。
- [x] T4 完成 5 处注册（`duilib_defs.h` 含 `kEventPopoverOpenChanged`、`WindowBuilder.cpp`、`duilib.vcxproj`、`.filters`、`duilib.h`）+ `EventArgs.cpp` 事件名映射 + `global.xml` 的 6 个 Class（`popover_trigger` / `popover_anchor` / `popover_anchor_primary` / `popover_card` / `popover_title` / `popover_content`）。
- [x] T5 新增示例窗口 `PopoverForm.h/.cpp`、`controls/popover.xml`、`test.xml` 入口、`TestForm` 绑定、工程登记、中英语言资源。
- [x] T6 同步中英双语文档（Control.md / Control.en.md / Summary / Summary.en.md）、两份 api-reference、`.claude` 速查（README 无控件清单条目，无需改动）。
- [x] T7 回写 `Development-Plan.md`（P2 汇总、D-14 表格行、任务行）与 `Project-Audit.md` B8（注明富内容提示由 Popover 覆盖）。
- [x] T8 Debug|x64 编译通过（`duilib` + `controls`），`verify_docs.py` 退出码 0。
- [x] T9 示例对齐 Ant Design 演示页：新增锚点按钮外观 Class（`popover_anchor` / `popover_anchor_primary`），`controls/popover.xml` 改为 5 张卡片
      （基本用法 / 三种触发方式 + 手动 / 12 方位环形布局 / 受控模式 / 复杂内容），`PopoverForm` 拆出 `BindBasicPopover()` /
      `BindTriggerPopovers()` / `BindControlledPopover()` / `BindRichPopover()`。
- [x] T10 新增卡片内控件访问 API `GetCardWindow()` / `FindCardControl(name)`，用于在 `kEventPopoverOpenChanged` 回调里绑定卡片内
      按钮，实现"从浮层内关闭"；同步 Control.md / Control.en.md / 两份 api-reference / `.claude` 速查 / 本 spec。
- [x] T11 修复实测 5 项问题：
      ① 阴影过大且拦截鼠标/干扰 Hover → 卡片窗口 `shadow_type` 改 `small_round`；`Flyout` 新增
      `SetShadowClickThrough` + `IUIMessageFilter`（`WM_NCHITTEST` 阴影区返回 `HTTRANSPARENT`）、
      `GetVisualRect` / `IsPointInVisualArea`（阴影区按卡片外判定），`Popover::IsMouseOverAnchorOrCard`
      与 `Flyout::OnDetectTick` 改用可视区域；
      ② 受控模式"切换"永远只显示 → `Flyout` 新增 `SetClickOutsideDismiss`，`Popover` 在 `trigger="manual"`
      且未显式设置 `auto_dismiss` 时关闭"点外部关闭"（Esc 仍受 `auto_dismiss` 控制）；
      ③ "切换宽度/方位"无效果随 ② 一并恢复；
      ④ "打开复杂内容"等长文案按钮改 `width="auto" padding="12,0,12,0"`；
      ⑤ 按钮间距改用 `child_margin_x`（三种触发行 8、12 方位行 6）。
      同步 Control.md / Control.en.md / 两份 api-reference / `.claude` 速查 / 本 spec / checklist。
- [x] T12 对齐 antd 的尖角箭头与更轻的阴影：
      ① 新增 4 个箭头 SVG（`public/popover/popover_arrow_up|down|left|right.svg`，只描两条斜边）与
      `public/shadow/shadow_popover.svg`（模糊 4、偏移 2、边距 12/14）；
      ② `global.xml` 新增 `PATH_POPOVER` 变量与 `popover_arrow_up/down/left/right` 四个 Class
      （`svg_replace_colors` 映射 `bg_window_card` / `border_window`）；
      ③ `Popover` 新增 `arrow` 属性 / `SetArrow` / `IsArrow` / `m_bArrow`，`BuildCardXml(placement)` 生成
      "卡片框 + 箭头条"结构（水平方位按 `margin` 定位、垂直方位用 stretch/固定高度占位）；
      ④ `Flyout` 新增 `GetActualPlacement()`，`Show()` 在方位翻转时以 `m_bRebuilding` 守卫关闭旧浮层并按
      实际方位重建一次，保证箭头始终指向锚点；
      ⑤ 卡片窗口阴影由 `small_round` 改为 `shadow_type="custom"` + `shadow_popover.svg`（更紧凑）。
      同步 Control.md / Control.en.md / 两份 api-reference / `.claude` 速查 / 本 spec / checklist。
- [x] T13 修复 T12 实测的三个视觉问题（箭头位置错误 / 卡片箭头侧的纯白实心带 / 卡片边线横穿箭头"线条不美观"）：
      ① `Popover::BuildCardXml()` 改为"卡片按方位留 8 像素箭头带（`popover_card` 的 `margin`）+ 浮动箭头
      （`float="true" keep_float_pos="true"`，排在卡片之后、水平 16×9 / 垂直 9×16）"，`shadow_border_round` 改 `0,0`；
      ② 新增 4 个方位专用阴影 `public/shadow/shadow_popover_arrow_{up,down,left,right}.svg`
      （阴影主体在箭头一侧内缩 8，`corner` 该侧取"阴影边距 + 箭头带 + 卡片圆角"），使阴影主体边缘与卡片边缘重合，
      箭头带内只保留阴影渐变；
      ③ 4 个箭头 SVG 重做为 16×9 / 9×16（base 位于卡片侧并压入 1 像素，覆盖该处卡片边线，斜边描边从卡片边线出发），
      `global.xml` 的 4 个 Class 尺寸同步；
      ④ `Flyout` 新增 `AttachLayouted()` + `OnInitLayout()` 覆写（首帧布局后、首帧绘制前触发一次），
      `Popover::Show()` 在 `ShowAt()` 前注册回调，新增 `PositionCardArrow()` 按锚点中心与卡片实际矩形定位箭头
      （`ClampArrowOffset` 钳位、避开卡片圆角 12 像素）。
      同步 Control.md / Control.en.md / 两份 api-reference / `.claude` 速查 / 本 spec / checklist。
- [x] T14 修复实测断言（点击 `RT`（right_top）等方位时弹出"Breakpoint Instr / ASSERT"）：
      ① 根因：`AttributeUtil::ParseRectValue(strValue, rect, bCheckSize)` 对 `corner` 也做
      "`rect.Width() >= 0 && rect.Height() >= 0`" 的校验，而 `corner` 的 4 个值是四条边各自的内缩量
      （互相独立，如 `28,20,22,22` 表示左 28 / 上 20 / 右 22 / 下 22），按普通矩形算出的宽度 `22 - 28 = -6` 为负 → 触发断言；
      ② 修复：`ImageAttribute::ModifyAttribute()` 解析 `corner` 时改为 `ParseRectValue(value, *m_rcCorner, false)`
      （随后仍对四边各自钳到 ≥0），`src` / `dest` / `size_box` / `caption` 等"矩形语义"的属性仍保留尺寸校验；
      ③ 影响面：`bkimage` / `shadow_image` 等所有经 `ImageAttribute::ModifyAttribute()` 解析的 `corner` 属性，
      支持非对称九宫格（窗口阴影在箭头一侧多留 8 像素）；原对称 corner 行为不变。
      同步本 spec / checklist（框架行为说明，其余文档无 `corner` 语义条目需改动）。

> T8 的编译结果与文档校验记录见 `checklist.md` 第 6 节；验收剩下的手工运行项见第 7 节。
