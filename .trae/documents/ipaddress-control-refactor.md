# IPAddress 控件改造方案

## Context

当前 [IPAddress.cpp](file:///e:/develop/nim_duilib/duilib/Control/IPAddress.cpp) 由 4 个 `RichEdit` 与 3 个 `.` Label 拼接而成，只能靠鼠标点选 / 回车逐段切换，缺少整串粘贴、跨段退格、状态色与完整性判断；外观是一个直角单边框矩形，与主题中其它编辑框的圆角、focus 状态不一致。

目标：在**不改变对外已有接口语义**的前提下，提升输入效率（键盘流）、可编程性（完整性/合法性/清空）、视觉一致性（圆角 + hover/focus 边框 + 当前段高亮 + 占位符），并把散落的魔法数字收敛为常量与工具函数。

范围限定为已确认方案中的 A（功能）、B（交互）、C（视觉）与 D（常量/辅助函数重构），不做 XML 模板化（原 P2）。

## 改动文件

### 1. `duilib/Control/IPAddress.h`

新增公开接口：

- `bool IsComplete() const;` 四段是否均已填写（不校验范围）
- `bool IsValid() const;` 完整且每段为 0~255 的合法数字
- `void ClearIPAddress();` 清空四段
- `virtual bool IsFocused() const override;` 任一段聚焦即视为控件聚焦（驱动 `focused_border_color`）

新增受保护/私有实现：

- `void OnSegmentTextChanged(size_t index);`
- `bool OnSegmentKeyDown(size_t index, const EventArgs& msg);`（返回 true 表示已消费）
- `void OnSegmentSetFocus(size_t index);`
- `void FocusSegment(size_t index, bool bSelectAll);`
- `void UpdateSegmentBkColor();`
- `void NotifyValueChanged();`
- `int32_t GetSegmentNumber(size_t index) const;`
- `void SetSegmentNumber(size_t index, int32_t nValue);`

新增成员：

- `UiString m_lastIPAddress;` 上次对外通知的值（用于去重）
- `int32_t m_nFocusIndex;` 当前聚焦段，-1 表示无
- `bool m_bAllowEmpty;` `allow_empty` 属性：全空时 `GetIPAddress()` 返回空串

### 2. `duilib/Control/IPAddress.cpp`

- 匿名命名空间常量/工具：
  - `kSegmentCount = 4`、`kSegmentMaxValue = 255`、`kSegmentWidth = 28`、`kSeparatorWidth = 6`、`kHighlightColor = "bg_list_item_hovered"`
  - `bool IsAllDigits(const DString&)`
  - `bool TryParseIPAddress(const DString&, int32_t values[4])`（按 `.` 拆 4 段、逐段纯数字并 clamp 到 0~255）
- 构造函数初始化 `m_nFocusIndex(-1)`、`m_bAllowEmpty(false)`。
- `SetAttribute` 增加 `allow_empty` 解析。
- `OnInit()`：
  - 每段固定宽 `28`、`text_align=vcenter,hcenter`、`limit_text=3`、`number_only=true`、`min_number=0`、`max_number=255`、`prompt_mode=true`、`prompt_text="0"`、`prompt_color=text_richedit_prompt`、`text_padding=0`、`border_round=3,3`（供高亮圆角使用）；`want_return_msg/want_tab` 保持 false。
  - 分隔点改为 `width=6`、`text_align=vcenter,hcenter`，保持无焦点/不可交互。
  - 新增绑定：`AttachTextChanged`、`AttachEvent(kEventKeyDown)`、`AttachSetFocus`、`AttachKillFocus`、`AttachReturn`。
  - 末尾同步 `m_lastIPAddress = GetIPAddress().c_str()`。
- 交互：
  - 文本长度达到 3 时自动跳到下一段（最后一段不跳）。
  - `.` / 小键盘 `.`：跳下一段并消费按键。
  - Backspace：段内为空或光标在段首时，回到上一段并把光标放到末尾。
  - `←`：段首时跳到上一段末尾；`→`：段尾时跳到下一段开头。
  - `↑/↓`：当前段数值 +1/-1 并 clamp 到 0~255。
  - `Ctrl+V`：读取剪贴板，若能解析为完整 4 段 IP 则整体填入并消费按键；否则交给原始逻辑。
- 值变更通知：`NotifyValueChanged()` 仅在 `GetIPAddress()` 与 `m_lastIPAddress` 不同时 `SendEvent(kEventTextChanged)`。
- 焦点/高亮：`OnSegmentSetFocus`、`OnKillFocusEvent`、`UpdateSegmentBkColor` 维护当前段背景（`bg_list_item_hovered`）与 `kEventSetFocus/kEventKillFocus` 的对外转发；原有"段间移动不触发 KillFocus"的抑制逻辑保留。
- `SetIPAddress` / `GetIPAddress` 改用 `TryParseIPAddress` 与统一 clamp；`GetIPAddress` 在 `allow_empty` 且四段全空时返回空串。

### 3. `bin/resources/themes/default/global.xml`（第 791 行 `ip_address` 类）

```xml
<Class name="ip_address" width="140" height="28" bkcolor="bg_richedit"
       border_size="1" border_round="4,4" padding="4,3,4,3"
       border_color="border_richedit_normal"
       hovered_border_color="border_richedit_hovered"
       focused_border_color="border_focus_ring"/>
```

说明：`border_focus_ring` 是主题里语义为"焦点环/光晕"的颜色（light `#B4B4B4` / dark `#676767`），比 `border_richedit_focused` 的反馈更明显，用于圆角焦点环。

## 复用的既有能力

- `RichEdit`：`SetTextNoEvent`、`SetNumberOnly/SetMinNumber/SetMaxNumber`、`AttachReturn/AttachTextChanged/AttachSetFocus/AttachKillFocus/AttachEvent`、`GetSel/SetSel/SetSelAll`、`SetTextPadding/SetPromptText`。
- `Control`：`AttachEvent(kEventKeyDown)` 回调返回 `false` 即消费按键（`Control::SendEventMsg` 中 `FireAllEvents` 返回 false 后不再走 `HandleEvent`）；`IsFocused()` 为 `virtual const`，`PaintBorder` 据此取 `focused_border_color`；`SetBkColor` 支持空串且内部去重。
- `Clipboard::GetClipboardText(DStringW&)`（非 Unicode 构建用 `StringConvert::WStringToUTF8` 转换）。
- `StringUtil::Split/StringToInt32/Printf`；`ModifierKey::kControl`、`kVK_OEM_PERIOD/kVK_DECIMAL/kVK_BACK/kVK_LEFT/kVK_RIGHT/kVK_UP/kVK_DOWN/kVK_V`。

## 验证

1. 构建：`Launch-VsDevShell.ps1 -Arch amd64` 后 `cmake --build e:\develop\nim_duilib\out\x64\Debug --target duilib controls --config Debug`。
2. 运行 `controls` 示例，截图核对 IP 控件默认/悬浮/聚焦/当前段高亮外观；实际操作验证：输入 `192.168.0.1`（自动跳段）、`.` 跳段、退格跨段、上下键 ±1、`Ctrl+V` 整串粘贴。
3. 回归：`python tests/test_controls_pages.py` 与 `ctest --test-dir e:\develop\nim_duilib\out\x64\Debug -C Debug --output-on-failure`。
4. 不执行任何 git 提交。
