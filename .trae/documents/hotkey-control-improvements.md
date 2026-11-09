# HotKey 控件功能完善与外观优化

## Context

`HotKey`（[HotKey.h](file:///e:/develop/nim_duilib/duilib/Control/HotKey.h)、[HotKey.cpp](file:///e:/develop/nim_duilib/duilib/Control/HotKey.cpp)）是继承 `HBox` 的热键录入控件，内部嵌入一个 `HotKeyRichEdit` 捕获按键。当前存在以下问题：

- **SDL 后端取键错误**：`OnKeyDown` 用 `msg.wParam` 作为虚拟键码、用 `(msg.lParam >> 16) & kKF_EXTENDED` 读扩展位，这两者都是 Windows 原生消息语义。SDL 后端下 `wParam = SDL_EVENT_KEY_DOWN`、`lParam = SDL_KeyboardEvent*` 指针，导致 SDL 下记录错误键值、扩展位随机。
- **修饰键状态不一致**：单独按 Ctrl/Shift/Alt 时 `OnKeyDown` 记录并显示 "Ctrl"，`OnKeyUp` 又把显示清空，但存储值未清 → `GetHotKey()` 与界面不一致。
- **空值处理缺陷**：`SetHotKey(0, 0)` 只在名称非空时更新文本，会残留旧热键文字；没有清空 API；`SetHotKeyName("")` 返回 false 且不改值（PropertyGrid 清空后再编辑会保留旧值）。
- **无变化通知**：值变化不发任何事件；`GetHotKeyName` 在无修饰键时直接返回空，裸键（如 F5）无法显示。
- **外观**：`hot_key` 主题类没有圆角与 hover/focus 边框；`default_text` 以普通文字颜色作为实际文本显示，没有灰色占位效果；Windows 版 RichEdit 的 `PaintPromptText` 忽略 `text_align` 与内边距（RichEdit2 已支持）。

目标：Windows 与 SDL 行为对齐（SDL 确实不支持的能力明确跳过并在文档说明），完善交互，改善外观，并同步演示与文档。

已与用户确认的两项行为：
1. 只按修饰键时**仅预览、松开后回退**，不产生"只有 Ctrl"的无效热键。
2. 未设置热键时，`default_text/default_text_id` 以**灰色占位提示**显示（prompt 模式）。

## 行为设计

1. **取键与修饰键**：虚拟键码一律用 `msg.vkCode`；修饰键用 `msg.modifierKey` 的 `kShift / kControl / kAlt` 位；扩展位仅在 `#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)` 下从 `lParam` 读取，SDL 下固定为 false。
2. **修饰键预览**：按下修饰键只刷新显示（"Ctrl" / "Ctrl + Shift"），**不修改存储值**；松开修饰键、失焦、按 Esc 时恢复为已提交的热键显示。
3. **提交**：按下非修饰键 → 存储 `(vk, modifiers)`（允许裸键）→ 更新显示 → 发送变化事件。
4. **清空**：Delete / Backspace → 清空值，显示占位提示，发送变化事件。
5. **屏蔽组合**：不记录 `kLWIN/kRWIN`，以及 `Alt + F4/Tab/Esc/Space`。注意不能整体过滤 `kIsSystemKey`（Windows 下 Alt+任意键都是系统键，会导致所有 Alt 组合无法录入）。
6. **Tab 限制**：Tab 在 `Window::OnKeyDownMsg` 中被焦点遍历提前截获，无法作为热键，文档注明。
7. **显示名**：`GetHotKeyName()` 在 `vk != 0` 时即使无修饰键也返回键名（F5）；`vk == 0` 且有修饰键时返回修饰键名（用于预览）。
8. **变化通知**：值真正变化时 `SendEvent(kEventTextChanged)`，按 `MAKEWORD(vk, modifiers)` 去重；`SetHotKey`、`SetHotKeyName`、`ClearHotKey` 与用户输入统一走该路径。
9. **空值归一化**：`SetHotKey(0, *)` 归一化为空（modifiers 也置 0）；`SetHotKeyName("")` 等价清空并返回 true；非空但解析失败返回 false 且不改值。

## 代码改动

### duilib/Control/HotKey.h / HotKey.cpp

- 新增公开接口：`void ClearHotKey();`、`virtual bool IsFocused() const override;`
- 新增私有方法：`void UpdateDisplayText();`、`void NotifyHotKeyChanged();`
- 新增成员：已提交值的快照（用于预览回退）、`uint16_t m_nLastHotKeyValue`（事件去重）。
- 内部类 `HotKeyRichEdit` 持有 `HotKey* m_pOwner`，按键逻辑通过 owner 的方法提交/预览/清空。
- `HotKeyRichEdit` 构造参数增加 owner；初始化 `prompt_mode=true`、`prompt_color=text_richedit_prompt`、`text_color=text_default`、`text_align=vcenter,hcenter`、`text_padding=0,0,0,0`、尽量占满父容器。
- 重写 `OnKeyDown`：按上述"行为设计"实现；删除首行 `SetHotKey(0, 0)`。
- 重写 `OnKeyUp`：删除旧的"清空显示"逻辑，仅处理预览结束回退。
- `SetAttribute`：`default_text` → `SetPromptText` + 清空实际文本；`default_text_id` → `SetPromptTextId`；删除默认文本缓存与 `OnLanguageChanged` 重载。
- `SetHotKey`：归一化 → 写值 → `UpdateDisplayText()` → `NotifyHotKeyChanged()`。
- `ClearHotKey()` / `SetHotKeyName("")`：清空并通知。
- `IsFocused()`：`BaseClass::IsFocused() || m_pRichEdit->IsFocused()`。
- `OnInit` 的 `AttachSetFocus/AttachKillFocus`：记录/恢复预览并 `Invalidate()`，保证焦点边框刷新。

### duilib/Control/RichEdit_Windows.cpp

- `PaintPromptText` 对齐 RichEdit2（[RichEdit2.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichEdit2.cpp#L2412-L2443)）：使用 `GetTextStyle()` 与 `GetRichTextDrawRect()`（含对齐与内边距），必要时补 `AllowPromptMode()` 判断，使 `text_align="vcenter,hcenter"` 在 Windows 后端生效。

### 主题与演示

- [global.xml](file:///e:/develop/nim_duilib/bin/resources/themes/default/global.xml#L797) 的 `hot_key` 类增加 `border_round="4,4"`、`padding="4,3,4,3"`、`hovered_border_color="border_richedit_hovered"`、`focused_border_color="border_focus_ring"`，与 `ip_address` 风格一致（仅用已定义的颜色 ID）。
- [property_grid.xml](file:///e:/develop/nim_duilib/bin/resources/themes/default/public/property_grid/property_grid.xml#L65) 的 `property_grid_hot_key` 同步圆角与 hover/focus 边框。
- 演示 XML（`controls/test.xml`、`controls/controls.xml`、`color_theme.xml`）无需改动，类升级后自动生效。

### 文档

- [Control.md](file:///e:/develop/nim_duilib/docs/Control.md#L682-L688) / [Control.en.md](file:///e:/develop/nim_duilib/docs/Control.en.md#L684-L690) HotKey 章节：属性改为"占位提示"语义；补充交互规则（组合键录制、修饰键预览回退、Esc 取消、Delete/Backspace 清空、裸键、屏蔽 Alt+F4/Win、Tab 限制、SDL 下无扩展键标志）；补充 C++ 接口（含 `ClearHotKey`、`SetHotKey` 也会发事件）；说明 `kEventTextChanged`。
- [Differences.md](file:///e:/develop/nim_duilib/docs/Differences.md#L65) / `Differences.en.md` 第 2 行同步。
- `CHANGELOG` 不改，由提交日志覆盖。

## 验证

1. Windows 构建：`cmake --build out\x64\Debug --target duilib controls --config Debug`（无警告）。
2. `python tests\test_controls_pages.py`（34 页 / 12121 项）与 `ctest --test-dir out\x64\Debug -C Debug --output-on-failure`（3/3）。
3. SDL 构建：按仓库 CMake 的 SDL 选项构建一次（若本机无 SDL 工具链，则至少确认新增代码无 Windows-only 依赖、扩展位逻辑已被宏隔离）。
4. UI 回归（[scripts/shot.ps1](file:///e:/develop/nim_duilib/scripts/shot.ps1)）：
   - 滚动到 HotKey 区截图：空态灰色占位、圆角边框。
   - `-MsgClicks` 点击后 `-Keys "^a"`：显示 `Ctrl + A`，焦点边框可见。
   - `-KeyCodes "8"`（Backspace）：恢复占位提示。
   - 悬停与禁用态（color_theme 页）截图核对。
5. 手工核对：修饰键按下/松开值不变；Ctrl+Shift 松开回退；Esc 恢复；Alt+字母可录入而 Alt+F4 不录入；语言切换占位文字跟随；PropertyGrid 清空后再编辑不残留旧值；IPAddress 的 prompt 显示无回归。

## 风险

- prompt 模式使 HotKey 内部文本为空：仓库内仅 PropertyGrid 通过 `GetHotKeyName()` 取值，无兼容问题，文档注明。
- `RichEdit_Windows::PaintPromptText` 属公共行为修改，需回归 IPAddress 等使用 prompt 的控件。
- Tab 无法作为热键（框架焦点遍历），文档注明为已知限制。
