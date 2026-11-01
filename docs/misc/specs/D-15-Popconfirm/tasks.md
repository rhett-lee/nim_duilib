# Tasks: D-15 Popconfirm 确认气泡卡片

- [x] T1 `duilib/Control/Popover.h` 钩子化（行为不变）：`PopoverCloseReason` 追加 `kConfirm = 4` / `kCancel = 5`；
      `BuildCardBodyXml()` / `InitializeCard()` / `OnFlyoutClosed()` 移入 `protected`（前两者 `virtual`）；
      新增 `HideWithReason(PopoverCloseReason)` / `static EscapeXmlText()` 与成员 `m_bCloseReasonPending` /
      `m_pendingCloseReason`。
- [x] T2 `duilib/Control/Popover.cpp` 钩子化实现：`EscapeXmlText` 成员化、`BuildCardXml()` 抽出
      `BuildCardBodyXml()`、`Hide()` → `HideWithReason(kManual)`、`OnFlyoutClosed()` 优先取待用关闭原因。
- [x] T3 新增 `duilib/Control/Popconfirm.h/.cpp`：`PopconfirmIcon` / `PopconfirmOkType` 枚举、属性解析、
      `BuildCardBodyXml()` 覆写（图标 + 标题 / 描述 / 右对齐按钮行）、`InitializeCard()` 覆写（绑定
      `popconfirm_ok` / `popconfirm_cancel` 的 `kEventClick`）、`Confirm()` / `Cancel()` / `AttachConfirm` / `AttachCancel`。
- [x] T4 注册 5 处 + 事件：`DUI_CTR_POPCONFIRM`（`duilib_defs.h`，紧随 `DUI_CTR_POPOVER`）、
      `kEventPopconfirmConfirm` / `kEventPopconfirmCancel`（`kEventLast` 之前）、`WindowBuilder.cpp`、
      `duilib.vcxproj`、`duilib.vcxproj.filters`、`duilib.h`、`EventArgs.cpp` 的 `InitEventStringMap`。
- [x] T5 主题与资源：`global.xml` 新增 `PATH_POPCONFIRM` 与 `popconfirm_icon_*` / `popconfirm_title` /
      `popconfirm_description` / `popconfirm_btn` / `popconfirm_cancel` / `popconfirm_ok_*` 共 11 个 Class；
      新增 4 个 16×16 SVG（`public/popconfirm/`）。
- [x] T6 示例：`examples/controls/PopconfirmForm.h/.cpp`、`controls/popconfirm.xml`、`test.xml` 入口、
      `TestForm` 绑定、`controls.vcxproj` / `.filters`、中英示例语言资源。
- [x] T7 文档：`Control.md` / `Control.en.md` 新章节、`Summary` 双语、`Differences` 双语（**序号 54**）、
      `Development-Plan.md`、两份 `api-reference.md`（哈希一致）、`.claude` 速查、本 spec / tasks / checklist。
- [x] T8 验证：`duilib` + `controls`（Debug|x64）编译通过、`verify_docs.py` 退出码 0、`git diff --check` 无空白错误。

> 编译结果与文档校验记录见 `checklist.md` 第 6 节；验收剩下的手工运行项见第 7 节。
