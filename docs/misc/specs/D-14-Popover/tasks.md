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

> T8 的编译结果与文档校验记录见 `checklist.md` 第 6 节；验收剩下的手工运行项见第 7 节。
