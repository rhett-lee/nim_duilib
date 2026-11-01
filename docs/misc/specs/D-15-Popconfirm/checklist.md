# Checklist: D-15 Popconfirm 确认气泡卡片

> 全部勾选才视为完成（DoD）。第 7 节为运行时人工验收项，由用户在示例程序中确认。

## 1. Popover 钩子化（行为不变）

- [x] `PopoverCloseReason` 追加 `kConfirm = 4` / `kCancel = 5`，前 4 项与 `Flyout::CloseReason`
      保持 `static_cast` 对齐（既有取值不变，向后兼容）。
- [x] `BuildCardBodyXml()` 新增（`protected` + `virtual`）：`BuildCardXml()` 抽出"标题 + 正文/富内容"的拼装，
      `BuildCardXml()` 只保留 `<Window shadow_type="custom" ...>` 外壳、`popover_frame` 容器与浮动箭头。
- [x] `InitializeCard(Flyout*)` 新增（`protected` + `virtual`）：默认实现为空，供派生类绑定卡片内控件；
      `Popover::Show()` 在卡片创建后调用。
- [x] `OnFlyoutClosed(int32_t)` 移入 `protected`（`virtual`），并优先取"暂存关闭原因"
      （`m_bCloseReasonPending ? m_pendingCloseReason : static_cast<...>(nCloseReason)`），随后清标志、
      复位 `m_bOpen`、`CancelAllTasks()`、`Invalidate()`、`EmitOpenChanged(false, reason)`。
- [x] `Hide()` 改为 `HideWithReason(PopoverCloseReason::kManual)`；`HideWithReason()` 在调用
      `Flyout::Dismiss()` 前后置位/清理 `m_bCloseReasonPending`（`Dismiss()` 同步触发 `AttachClosed`，
      因此回调里能读到暂存原因；调用后仍做兜底清理）。
- [x] 卡片未打开且 `m_bOpen` 时 `HideWithReason()` 走原"本地复位"分支（不触碰浮层，不残留标志）。
- [x] `EscapeXmlText()` 由匿名命名空间函数改为 `static` 成员函数（`Popconfirm` 拼装卡片内容时复用）。
- [x] `AttachOpenChanged` 的注释补充"卡片内按钮触发的关闭同样派发本事件，原因为 `kConfirm` / `kCancel`"。
- [x] `Popover` 的 XML 属性、方位/箭头/阴影、触发方式、跟随锚点、挂起恢复逻辑**零改动**。

## 2. Popconfirm 控件源码

- [x] `duilib/Control/Popconfirm.h` / `.cpp` 已新增，命名空间 `ui`。
- [x] `GetType()` 返回 `DUI_CTR_POPCONFIRM`；基类为 `Popover`（`typedef Popover BaseClass`）。
- [x] 枚举 `PopconfirmIcon{kWarning,kInfo,kSuccess,kError,kNone}`（默认 `kWarning`）、
      `PopconfirmOkType{kPrimary,kDefault,kDanger}`（默认 `kPrimary`）为 `enum class : uint8_t`。
- [x] `SetAttribute()` 支持 `description` / `description_id` / `icon` / `ok_text` / `ok_text_id` /
      `cancel_text` / `cancel_text_id` / `show_cancel` / `ok_type` / `close_on_ok`，
      并支持无下划线别名；`content` / `content_id` 作为 `description` / `description_id` 的兼容别名。
- [x] 属性解析忽略大小写与 `_` / `-`（`ParseIcon()` / `ParseOkType()`）；未知属性回退 `BaseClass::SetAttribute()`。
- [x] `SetOkText("")` / `SetOkTextId("")`（及 cancel 同名接口）回退框架语言键
      （`STRID_PUBLIC_STRING_OK` / `STRID_PUBLIC_STRING_CANCEL`）。
- [x] 构造函数只设置默认语言键，**不修改** `placement` / `trigger`（与 `Popover` 默认一致：top + click）。
- [x] `BuildCardBodyXml()` 覆写：`<HBox width="stretch" height="auto">` + 图标
      `<Control class="popconfirm_icon_*" width="16" height="16" margin="0,1,8,0" mouse_enabled="false" tab_stop="false"/>`
      + `<Label class="popconfirm_title"/>`；可选 `<Label class="popconfirm_description"/>`；
      按钮行 `<HBox width="auto" height="auto" halign="right" margin="0,12,0,0">` +
      `<Button class="popconfirm_cancel" name="popconfirm_cancel">` + `<Button class="popconfirm_ok_*" name="popconfirm_ok">`。
- [x] `icon="none"` 时不生成图标控件，标题左边缘与卡片 padding 对齐。
- [x] `show_cancel="false"` 时不生成取消按钮；`ok_type` 经 `GetOkClassName()` 映射到
      `popconfirm_ok_primary` / `popconfirm_ok_default` / `popconfirm_ok_danger`。
- [x] 文本经 `EscapeXmlText()` 转义 `& < > " '`，多语言 ID 走 `text_id` 属性（ID 优先于字面文本）。
- [x] `InitializeCard()` 覆写：`FindControl("popconfirm_ok")` / `("popconfirm_cancel")` 后
      用 `UiBind` 绑定 `kEventClick` 到 `Confirm()` / `Cancel()`，最后调用 `BaseClass::InitializeCard(pFlyout)`。
- [x] `Confirm()`：先 `SendEvent(kEventPopconfirmConfirm)`，`m_bCloseOnOk` 为真时
      `HideWithReason(PopoverCloseReason::kConfirm)`；`close_on_ok="false"` 时卡片保持打开。
- [x] `Cancel()`：`SendEvent(kEventPopconfirmCancel)` 后 `HideWithReason(PopoverCloseReason::kCancel)`。
- [x] 事件顺序：按钮事件在前、`kEventPopoverOpenChanged(wParam=0)` 在后，因此按钮回调里
      `IsOpen()` 仍为 `true`（可用于关闭前二次校验）。
- [x] `Confirm()` / `Cancel()` 在卡片未打开时同样派发事件（受控调用场景），不触碰任何浮层。
- [x] 不使用继承来的 `content_xml`（卡片内容由 `BuildCardBodyXml()` 生成）；派生类可重写该方法。

## 3. 注册与事件映射

- [x] `duilib/duilib_defs.h` 新增 `DUI_CTR_POPCONFIRM`（紧随 `DUI_CTR_POPOVER`）。
- [x] `duilib/duilib_defs.h` 新增 `kEventPopconfirmConfirm` / `kEventPopconfirmCancel`（位于 `kEventLast` 之前）。
- [x] `duilib/Core/WindowBuilder.cpp` 新增 include 与工厂项 `{DUI_CTR_POPCONFIRM, ... new Popconfirm(pWindow)}`。
- [x] `duilib/duilib.vcxproj` / `.vcxproj.filters` 登记 `Control\Popconfirm.h/.cpp`（Filter = `Control`）。
- [x] `duilib/duilib.h` 新增 `#include "Control/Popconfirm.h"`。
- [x] `duilib/Core/EventArgs.cpp` 的 `InitEventStringMap` 追加 `kEventPopconfirmConfirm`（`popconfirm_confirm`）
      与 `kEventPopconfirmCancel`（`popconfirm_cancel`）。

## 4. 主题、资源与示例

- [x] `global.xml` 新增变量 `PATH_POPCONFIRM`（`public/popconfirm`）。
- [x] `global.xml` 新增 11 个 Class：`popconfirm_icon_warning` / `_info` / `_success` / `_error`、
      `popconfirm_title`（继承 `popover_title`，`multi_line` 让超长标题自动换行而不是截断成省略号）、
      `popconfirm_description`（继承 `popover_content`，12 号字）、
      `popconfirm_btn`（高 28 / 圆角 6 / 13 号字 / 手型光标 / `width="auto"` + `padding="10,0,10,0"` + `min_width="64"`）、
      `popconfirm_cancel`（继承 `btn_global` + 尺寸覆盖，宽度自适应）、
      `popconfirm_ok_default`（继承 `popconfirm_cancel`）、`popconfirm_ok_primary`（继承 `btn_primary_global` + 尺寸覆盖，宽度自适应）、
      `popconfirm_ok_danger`（红底固定配色，与 `btn_global_color_red` 的约定一致，`border_size="0"`）。
- [x] 图标颜色经 `svg_replace_colors` 映射语义色（圆形 → `color_warning` / `color_accent` /
      `color_success` / `color_error`，字形 → `color_white`），深浅色主题自动适配。
- [x] 新增 4 个 16×16 SVG：`public/popconfirm/popconfirm_icon_{warning,info,success,error}.svg`。
- [x] 卡片底框、阴影、尖角箭头全部复用 `popover_card` / `popover_arrow_*` 与 `public/shadow/` 的方位阴影。
- [x] `examples/controls/PopconfirmForm.h/.cpp` 存在：`GetSkinFolder()` = `controls`、
      `GetSkinFile()` = `popconfirm.xml`；`OnInitWindow()` 依次调用 `BindStateEcho()` / `BindResultEcho()` / `BindKeepOpen()`。
- [x] `BindResultEcho()` 遍历 11 个锚点绑定 `AttachConfirm` / `AttachCancel`；`UpdateStateText()` 覆盖
      `kConfirm` / `kCancel` 及既有原因；`UpdateResultText()` 用 `{0}` 模板替换锚点文案。
- [x] `bin/resources/themes/default/controls/popconfirm.xml` 存在，5 个分组：基本用法（+ 状态回显）、
      五种图标、确定按钮样式 + 无取消按钮、较长描述（`card_width="300"`）、保持打开 + 受控 `Confirm()`/`Cancel()`。
- [x] 示例中所有锚点与"调用 Confirm()/Cancel()"按钮均为 `width="auto"` + `padding`（配 `min_width`），
      切换中英文后锚点/按钮文案不会被截断。
- [x] `controls/test.xml` 有 Popconfirm 入口（`popconfirm_btn` + `STRID_CONTROLS_POPCONFIRM_TEST`）。
- [x] `examples/controls/TestForm.h/.cpp` 有 `ShowPopconfirmWindow()` 与入口绑定。
- [x] `controls.vcxproj` / `.filters` 登记 `PopconfirmForm.h/.cpp`。
- [x] 中英 examples 语言文件键集合一致（`STRID_POPCONFIRM_*` 段与 `STRID_CONTROLS_POPCONFIRM_TEST`），UTF-8 无 BOM、中文无乱码。

## 5. 文档与计划

- [x] `docs/Control.md` 的 `PopoverCloseReason` 枚举行补 `kConfirm`（4）/ `kCancel`（5），
      `### 事件` 补"卡片内按钮触发的关闭同样派发本事件"。
- [x] `docs/Control.md` 新增 `## Popconfirm 控件（确认气泡卡片）`（核心特性 / XML 属性表 / C++ 接口表 /
      枚举 / 事件与顺序 / XML 内联事件 / 11 个 Class 表 / 图标资源 / 使用示例 / 注意事项），并加 `<a name="popconfirm"></a>`。
- [x] `docs/Control.en.md` 对称更新（Popover 关闭原因与事件说明 + `## Popconfirm Control (confirmation popover card)` 章节）。
- [x] `docs/Summary.md` / `Summary.en.md` 控件清单追加 `Popconfirm` 行与有效锚点 `#popconfirm`。
- [x] `docs/Differences.md` / `Differences.en.md` 的"3. 新增控件/容器"表格追加 `Popconfirm` 行（**序号 54**，中英同步）。
- [x] `docs/misc/Development-Plan.md`：基线日期行、P2 进展汇总（D 组 0 项 + 已完成数量）、
      D 组表格 D-15 行、P2 任务清单 `- [x] **D-15**` 均已回写。
- [x] `.trae/skills/nim-duilib/references/api-reference.md` 与 `.workbuddy/skills/nim-duilib/references/api-reference.md`
      同步：控件清单 `Popconfirm` 行、`PopoverCloseReason` 枚举补两项、`### Popconfirm 属性(继承 Popover)` 速查段
      （两份 `Get-FileHash` 一致）。
- [x] `.claude/docs/nim-duilib-llm-reference.md` 控件速查表追加 `Popconfirm` 行。
- [x] `docs/misc/specs/D-15-Popconfirm/{spec,tasks,checklist}.md` 已创建并回写。

## 6. 验证命令

> 命令在 `e:\develop\nim_duilib` 下、PowerShell 中执行。

- [x] `duilib`（Debug|x64）编译退出码 0（Popover 钩子化 + 新增 Popconfirm 后）。
- [x] `examples\controls\controls.vcxproj`（Debug|x64，`/p:SolutionDir=<repo>\build\`）编译退出码 0。
- [x] `python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0。
- [x] `git diff --check` 无空白错误。
- [x] 两份 `api-reference.md` 的 `Get-FileHash` 相同。
- [x] 未执行 `git commit`（由用户手动提交）。

## 7. 功能验收（手工）

> 以下为运行时人工验收项，由用户在 controls 示例（"Popconfirm"分组）中确认。

### 示例演示（对齐 Ant Design 演示页）
- [ ] 基本用法：点击锚点弹出"警告图标 + 问句标题 + 描述 + 取消/确定"，点"取消"关闭、点"确定"关闭，状态回显区分原因。
- [ ] 五种图标：warning / info / success / error 四种圆形图标配色正确，`none` 不显示图标且标题左边缘与卡片内边距对齐。
- [ ] 确定按钮样式：primary（主色实心）、default（与取消同外观）、danger（红色实心）三种外观正确；`show_cancel="false"` 时只剩"确定"。
- [ ] 较长描述：`card_width="300"` 时描述多行自动换行，卡片高度随内容自适应。
- [ ] 保持打开：`close_on_ok="false"` 时点"确定"卡片不关闭，事件已派发；外部"确定"/"取消"按钮（`Confirm()` / `Cancel()`）行为一致。

### 文案完整显示（按钮/标题不被截断）
- [x] 卡片内"取消"/"确定"按钮宽度随文案自适应，短文案时为 64 的最小宽度，未出现文字被裁切。
- [x] `ok_text="删除"` 等较长文案时按钮自动变宽，文字完整显示。
- [x] 卡片标题较长（英文 "Are you sure you want to perform this operation?"）时自动换行显示完整，不出现 "…" 截断。
- [x] 示例中的锚点按钮（如英文 "Primary button"/"Default button"）与"调用 Confirm()"按钮文案完整显示。

### 创建与属性
- [ ] XML 创建成功，`FindControl("confirm_basic")` 可取到；继承的 `placement` / `trigger` / `gap` / `arrow` 等属性生效。
- [ ] `description` / `description_id` 生效（后者按 STRID 解析）；`content` / `content_id` 作为别名等价生效，且与 `description` 同时设置时以 `description` 为准。
- [ ] `ok_text` / `ok_text_id` / `cancel_text` / `cancel_text_id` 生效；默认（不设置）时显示语言的"确定"/"取消"。
- [ ] 文本含 XML 特殊字符（`<`、`&`、`"`）时卡片显示原字符且不破坏卡片 XML。
- [ ] 未知属性不崩；`icon` / `ok_type` 的大小写与下划线写法均可识别。

### 交互与事件
- [ ] 点击"确定"：先收到 `kEventPopconfirmConfirm`，再收到 `kEventPopoverOpenChanged(wParam=0, lParam=4)`。
- [ ] 点击"取消"：先收到 `kEventPopconfirmCancel`，再收到 `kEventPopoverOpenChanged(wParam=0, lParam=5)`。
- [ ] 在"确定"回调里读取 `IsOpen()` 仍为 `true`（关闭前二次校验场景可用）。
- [ ] 点击卡片外部 / Esc 关闭时 `lParam` 分别为 1 / 2，不被 4、5 覆盖（暂存关闭原因不会泄漏到其他关闭方式）。
- [ ] 卡片内按钮只响应鼠标点击（不抢焦点），不响应键盘 Enter；Tab 不会把焦点移入卡片。
- [ ] `arrow` / 12 方位 / 跟随锚点滚动 / 锚点滚出可视区域挂起与恢复等继承能力表现与 Popover 一致。

### 边界与生命周期
- [ ] 卡片打开状态下切换语言：卡片自动关闭，重新打开为新语言文案（含按钮默认文案）。
- [ ] 连续快速点击"确定"/"取消"不重复派发事件、不崩溃；反复打开/关闭无悬垂回调。
- [ ] `Confirm()` / `Cancel()` 在卡片未打开时调用仍派发事件但不崩溃、不影响其它卡片。
- [ ] 浅色 / 深色主题、100% ~ 200% DPI 下图标、按钮与文字尺寸正常，图标不被压缩变形。
