# Spec: D-15 Popconfirm 确认气泡卡片

## 1. 目标

新增独立控件 `Popconfirm`（XML 节点名 `Popconfirm`，头文件 `duilib/Control/Popconfirm.h`），
对齐 Ant Design 的 [Popconfirm](https://ant-design.antgroup.com/components/popconfirm-cn) 语义：
**点击锚点弹出"图标 + 标题 + 描述 + 取消/确定"的确认卡片**，点击"确定"/"取消"后关闭卡片，
并通过事件把用户的选择告知外部。

现有能力缺口：

| 现有控件 | 缺什么 |
| :--- | :--- |
| `Popover`（D-14） | 只负责"展示信息"，没有"确认 / 取消"交互语义，也没有关闭原因之外的业务回调 |
| `MessageBoxWnd` | 模态对话框，居中显示，无法锚定在某一个控件旁边（不属于锚点型控件） |
| `Flyout` | 只有浮层地基，确认卡片的按钮、文案、关闭原因都要业务侧手写 |

`Popconfirm` 的定位：**"就地确认"**——浮层贴着触发控件弹出，不打断用户视线，不抢焦点；
用户点"确定"/"取消"后卡片关闭，外部通过事件拿到结果。

完成后回写 `docs/misc/Development-Plan.md` 的 P2 进展汇总、D 组表格（新增 D-15 行）、
P2 任务清单（新增 `- [x] **D-15**` 任务行）。

## 2. 需求要点

- 形态：`Popconfirm : public Popover`（控件层面独立、实现层面复用）。
- 卡片内容：`图标`（可选，默认警告色）+ `标题`（继承 `title` / `title_id`）+ `描述`
  （`description` / `description_id`，可选）+ 右对齐的"取消 / 确定"按钮行。
- 属性：`icon`、`description`、`ok_text` / `ok_text_id`、`cancel_text` / `cancel_text_id`、
  `show_cancel`、`ok_type`（primary / default / danger）、`close_on_ok`；
  其余（`trigger` / `placement` / `gap` / `card_width` / `arrow` / `auto_dismiss` 等）沿用 `Popover`。
- 事件：`kEventPopconfirmConfirm`（点击"确定"）、`kEventPopconfirmCancel`（点击"取消"）；
  打开/关闭状态仍通过继承来的 `kEventPopoverOpenChanged` 通知，关闭原因新增
  `PopoverCloseReason::kConfirm` / `kCancel`。
- **确认语义**：点击"确定"时关闭卡片（`close_on_ok` 可关闭该行为），点击"取消"时总是关闭卡片。
- 多语言：默认按钮文案直接使用框架已有的 `STRID_PUBLIC_STRING_OK` / `STRID_PUBLIC_STRING_CANCEL`。
- 交付完整的示例、双语文档、技能参考与主题 Class / 图标资源。

## 3. 设计决策

### 3.1 形态：独立控件 + 继承复用

对比三个方案：

| 方案 | 评价 |
| :--- | :--- |
| A. 在 `Popover` 上扩展属性（`show_buttons` / `ok_text` / `icon` …） | 卡片是"信息展示"还是"确认交互"是两种语义；`Popover` 每加一个属性都要在 `BuildCardBodyXml()` 里分支，属性表迅速膨胀，且 `Popover` 的使用者会被无关属性干扰。**不采用** |
| B. 复制 `Popover` 实现，另写一份 `Popconfirm` | `Popover.cpp` 约 974 行，其中 700+ 行（属性解析、方位映射、卡片框/箭头/阴影、跟随锚点、hover 时序、打开状态机）完全通用；复制后 D-14 的 T11~T16 这类修复要改两处，长期维护成本翻倍。**不采用** |
| C. 独立控件 `Popconfirm`，以 `class Popconfirm : public Popover` 继承 | 控件层面（XML 节点、类型名、属性表、事件）完全独立；实现层面（卡片窗口、方位、箭头、阴影、跟随、状态机）全部复用。**采用** |

### 3.2 `Popover` 钩子化（4 处，行为不变）

为了让子类只覆写"卡片内容"与"卡片内控件绑定"，`Popover` 增加 4 个受保护钩子：

| 钩子 | 作用 | `Popconfirm` 的用法 |
| :--- | :--- | :--- |
| `virtual DString BuildCardBodyXml() const` | 卡片根容器（`popover_card`）的子控件 XML；基类实现为"标题 + 正文/富内容" | 覆写为"图标 + 标题 / 描述 / 按钮行" |
| `virtual void InitializeCard(Flyout* pFlyout)` | 卡片创建完成、首次显示前的初始化；基类绑定卡片鼠标进入/离开（hover 时序） | 覆写后先绑定 `popconfirm_ok` / `popconfirm_cancel` 的 `kEventClick`，再调用基类 |
| `virtual void OnFlyoutClosed(int32_t nCloseReason)` | 浮层关闭回调（更新状态、派发事件） | 不覆写（继承基类即可） |
| `static DString EscapeXmlText(const DString&)` | 动态拼 XML 时的特殊字符转义 | 拼 `text` / `text_id` 时复用 |

`BuildCardXml()` 仍为 private，内部改为调用 `BuildCardBodyXml()`，因此卡片框、箭头带、阴影选择、
方位翻转逻辑对子类完全透明。

### 3.3 关闭原因如何让外部知道"是按钮触发的"

`kEventPopoverOpenChanged` 的 `lParam` 已经是 `PopoverCloseReason`，因此**不新增事件**，只扩展原因取值：

```cpp
enum class PopoverCloseReason : uint32_t
{
    kManual = 0, kClickOutside = 1, kEscape = 2, kAnchorLost = 3,  // 与 Flyout::CloseReason 一一对应
    kConfirm = 4,   // 点击卡片内的"确定"按钮
    kCancel  = 5    // 点击卡片内的"取消"按钮
};
```

`Flyout` 的关闭原因只有 4 个值（`kManual` / `kClickOutside` / `kEscape` / `kAnchorLost`），
且 `Flyout::Dismiss()` 会**同步**回调 `AttachClosed`，因此 `Popover` 用"待用原因"机制覆盖：

```cpp
protected void HideWithReason(PopoverCloseReason reason);   // 新增
```

- `Hide()` 等价于 `HideWithReason(kManual)`；
- `HideWithReason()` 把原因存进 `m_bCloseReasonPending` / `m_pendingCloseReason` 后再 `Dismiss()`；
- `OnFlyoutClosed()` 优先取待用原因（取用后立即清除），否则回退到 `Flyout` 的原因。

这样外部只需一个事件就能区分"用户点了确定 / 点了取消 / 点了外面 / 按了 Esc"。

### 3.4 卡片内容 XML

`Popconfirm::BuildCardBodyXml()` 生成：

```xml
<HBox width="stretch" height="auto">
  <Control class="popconfirm_icon_warning" width="16" height="16" margin="0,1,8,0"
           mouse_enabled="false" tab_stop="false"/>          <!-- icon="none" 时不生成 -->
  <Label class="popconfirm_title" text|text_id="..."/>
</HBox>
<!-- 可选：description -->
<Label class="popconfirm_description" text|text_id="..." margin="0,6,0,0"/>
<HBox width="auto" height="auto" halign="right" margin="0,12,0,0">
  <Button class="popconfirm_cancel" name="popconfirm_cancel" width="64" height="28" text|text_id="..."/>
  <Button class="popconfirm_ok"     name="popconfirm_ok"     width="64" height="28" margin="8,0,0,0" text|text_id="..."/>
</HBox>
```

- 标题与描述都走 `text` / `text_id` 双形态（`text_id` 优先），转义复用 `Popover::EscapeXmlText()`。
- 按钮行用 `halign="right"` 在卡片内右对齐（`halign` 是控件在父容器中的对齐方式，
  与 `duilib/Core/Control.cpp` 的 `SetHorAlignType` 一致）；两个按钮间距 8（`margin`，不用 `child_margin_x`，
  因为取消按钮可能被 `show_cancel="false"` 去掉）。
- 按钮的 class 按 `ok_type` 三选一：`popconfirm_ok_primary` / `popconfirm_ok_default` / `popconfirm_ok_danger`，
  控件名恒为 `popconfirm_ok`（`InitializeCard()` 按名字绑定，与外观解耦）。

### 3.5 属性与 API

| XML 属性 | API | 默认 | 说明 |
| :--- | :--- | :--- | :--- |
| `title` / `title_id` | `SetTitle` / `SetTitleId` | 空 | 主问句（继承自 `Popover`） |
| `description` / `description_id` | `SetDescription` / `SetDescriptionId` | 空 | 补充说明（同 `Popover` 的 `content`，但独立命名以免与继承属性混淆） |
| `icon` | `SetIcon(PopconfirmIcon)` | `warning` | `warning` / `info` / `success` / `error` / `none` |
| `ok_text` / `ok_text_id` | `SetOkText` / `SetOkTextId` | `STRID_PUBLIC_STRING_OK` | 确定按钮文案 |
| `cancel_text` / `cancel_text_id` | `SetCancelText` / `SetCancelTextId` | `STRID_PUBLIC_STRING_CANCEL` | 取消按钮文案 |
| `show_cancel` | `SetShowCancel` | `true` | 是否显示取消按钮 |
| `ok_type` | `SetOkType(PopconfirmOkType)` | `primary` | `primary` / `default` / `danger` |
| `close_on_ok` | `SetCloseOnOk` | `true` | 点击确定后是否关闭卡片 |
| 其余 | 继承 | — | `trigger` / `placement` / `gap` / `card_width` / `arrow` / `auto_dismiss` / `hover_delay` / `close_delay`，以及 `Popover::Show/Hide/Toggle/IsOpen/GetCardWindow/FindCardControl` |

额外 API：

```cpp
void Confirm();                 // 等同点击"确定"：派发 kEventPopconfirmConfirm，按 close_on_ok 关闭卡片
void Cancel();                  // 等同点击"取消"：派发 kEventPopconfirmCancel，并关闭卡片
void AttachConfirm(callback, id);   // 监听确定
void AttachCancel(callback, id);    // 监听取消
```

`Popconfirm` 不使用继承来的 `content` / `content_id` / `content_xml`（卡片内容由子类生成），
文档中明确提示改用 `description`。

### 3.6 事件

| 事件 | 派发时机 | 参数 |
| :--- | :--- | :--- |
| `kEventPopconfirmConfirm` | 点击"确定"按钮，或调用 `Confirm()` | 无 |
| `kEventPopconfirmCancel` | 点击"取消"按钮，或调用 `Cancel()` | 无 |
| `kEventPopoverOpenChanged`（继承） | 卡片打开/关闭 | `wParam` 1/0；`lParam` 关闭原因（新增 `kConfirm=4` / `kCancel=5`） |

事件顺序：`kEventPopconfirmConfirm` → `kEventPopoverOpenChanged(wParam=0, lParam=kConfirm)`；
即外部在 Confirm 回调里读到的 `IsOpen()` 仍是 `true`，可通过 `close_on_ok="false"` 让卡片保持打开。

### 3.7 明确不做

- 不做"确认结果的 Promise 式回调"（框架无异步回调设施，事件 + `AttachConfirm` 已足够）。
- 不做"弹出时自动聚焦确定按钮"（卡片窗口 `WS_EX_NOACTIVATE` 不抢焦点，聚焦会破坏"就地确认"体验）。
- 不做"确定/取消的间距、按钮宽度自适应文案"（固定 64×28，文案过长请自行改主题类）。
- 不改 `Flyout::CloseReason` 的取值（避免影响既有 4 个值的语义与外部已写的 switch）。

## 4. 精确改动清单

### 4.1 `Popover` 钩子化（行为不变）

- `duilib/Control/Popover.h`：
  - `PopoverCloseReason` 追加 `kConfirm = 4` / `kCancel = 5`；
  - `BuildCardBodyXml()` / `InitializeCard()` / `OnFlyoutClosed()` 移入 `protected`，前两者加 `virtual`；
  - 新增 `protected void HideWithReason(PopoverCloseReason reason)`、`protected static DString EscapeXmlText(...)`；
  - 新增成员 `m_bCloseReasonPending` / `m_pendingCloseReason`。
- `duilib/Control/Popover.cpp`：
  - `EscapeXmlText` 由匿名命名空间函数改为成员静态函数；
  - `BuildCardXml()` 中的"标题 + 正文"拼装抽为 `BuildCardBodyXml()`；
  - `Hide()` → `HideWithReason(kManual)`；新增 `HideWithReason()`；
  - `OnFlyoutClosed()` 优先使用待用关闭原因。

### 4.2 控件

- 新增 `duilib/Control/Popconfirm.h` / `.cpp`：
  - `PopconfirmIcon` / `PopconfirmOkType` 枚举与 `icon` / `ok_type` 解析（忽略大小写、支持无下划线写法）；
  - 属性解析、`BuildCardBodyXml()` 覆写、`InitializeCard()` 覆写、`Confirm()` / `Cancel()` / `AttachConfirm` / `AttachCancel`；
  - 默认文案走语言 ID（构造函数初始化 `m_strOkTextId` / `m_strCancelTextId`）。

### 4.3 注册（5 处 + 事件）

1. `duilib/duilib_defs.h`：`DUI_CTR_POPCONFIRM (_T("Popconfirm"))`（紧随 `DUI_CTR_POPOVER`）；
   `kEventPopconfirmConfirm` / `kEventPopconfirmCancel`（置于 `kEventLast` 之前）。
2. `duilib/Core/WindowBuilder.cpp`：`{DUI_CTR_POPCONFIRM, [](Window* pWindow) { return new Popconfirm(pWindow); }}`。
3. `duilib/duilib.vcxproj`：`ClCompile` + `ClInclude`。
4. `duilib/duilib.vcxproj.filters`：`ClCompile` + `ClInclude`（`Control` 过滤器）。
5. `duilib/duilib.h`：`#include "Control/Popconfirm.h"`。
6. `duilib/Core/EventArgs.cpp`：`InitEventStringMap` 追加
   `{kEventPopconfirmConfirm, _T("kEventPopconfirmConfirm"), _T("PopconfirmConfirm"), _T("popconfirm_confirm")}`、
   `{kEventPopconfirmCancel, _T("kEventPopconfirmCancel"), _T("PopconfirmCancel"), _T("popconfirm_cancel")}`。

### 4.4 主题与资源

- `bin/resources/themes/default/global.xml`：
  - `<Var name="PATH_POPCONFIRM" value="public/popconfirm"/>`；
  - Class：`popconfirm_icon_{warning,info,success,error}`、`popconfirm_title`、`popconfirm_description`、
    `popconfirm_btn`、`popconfirm_cancel`、`popconfirm_ok_default`、`popconfirm_ok_primary`、`popconfirm_ok_danger`。
- 新增 `bin/resources/themes/default/public/popconfirm/popconfirm_icon_{warning,info,success,error}.svg`
  （16×16：彩色圆 + 白色字形；占位色 `#0078D4` / `#FFFFFF` 由 `svg_replace_colors` 映射到
  `color_warning` / `color_accent` / `color_success` / `color_error` 与 `color_white`）。
- `danger` 按钮复用框架已有的 `btn_global_color_red`（固定红，与 `btn_global_color_red` 的既有约定一致——
  主题体系没有 danger 系列背景色）。

### 4.5 示例

- `examples/controls/PopconfirmForm.h/.cpp`：基本确认、危险操作、无取消按钮、纯图标 + 描述、
  自定义按钮文案、`close_on_ok="false"`（保持打开）、受控调用 `Confirm()`/`Cancel()`、事件回显。
- `bin/resources/themes/default/controls/popconfirm.xml`：演示窗口布局。
- `bin/resources/themes/default/test.xml`：新增入口按钮。
- `examples/controls/TestForm.cpp`：绑定入口按钮。
- `examples/controls/controls.vcxproj` / `.filters`：新增源码。
- `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：示例文案（含语言 ID）。

### 4.6 文档

- `docs/Control.md` / `docs/Control.en.md`：新增 `Popconfirm` 章节（属性表 / API / 事件 / 示例）。
- `docs/Summary.md` / `docs/Summary.en.md`：控件清单追加。
- `docs/Differences.md` / `docs/Differences.en.md`：第 3 节表格追加一行（**序号 54**）。
- `docs/misc/Development-Plan.md`：P2 汇总 + D 组表格 + 任务行。
- `.trae/skills/nim-duilib/references/api-reference.md` + `.workbuddy/skills/nim-duilib/references/api-reference.md`（两份哈希一致）
  与 `.claude/docs/nim-duilib-llm-reference.md`：控件速查追加 `Popconfirm`。
- `docs/misc/specs/D-15-Popconfirm/{spec,tasks,checklist}.md`。

## 5. 验证

1. `duilib`（Debug|x64）+ `examples/controls`（Debug|x64）编译通过。
2. `python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0（无漂移）。
3. `git diff --check` 无空白错误。
4. 手工运行示例程序，确认：四种图标外观、`ok_type` 三种外观、`show_cancel="false"`、
   `close_on_ok="false"`、确定/取消的事件与 `kEventPopoverOpenChanged` 的关闭原因、
   卡片内按钮可点击（不被浮层的"点击外部关闭"抢走）。

## 6. 验收标准

- [ ] XML 里写 `<Popconfirm class="..." name="..." title="..." description="..." ok_type="danger"/>`
  即可得到美观的确认卡片，无需业务侧拼 XML。
- [ ] 点击"确定"得到 `kEventPopconfirmConfirm`，且随后 `kEventPopoverOpenChanged(0, lParam=4)`；
  点击"取消"得到 `kEventPopconfirmCancel` 与 `lParam=5`。
- [ ] 点击卡片外/按 Esc 关闭时，`lParam` 仍为 1 / 2，语义不受本次扩展影响。
- [ ] 继承 `Popover` 的全部能力（12 方位、箭头、卡片宽度、hover/focus 触发、跟随锚点、滚动挂起恢复）照常工作。
- [ ] `Popover` 自身行为零变化（钩子化前后示例窗口表现一致）。
