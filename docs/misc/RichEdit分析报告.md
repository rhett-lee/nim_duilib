# RichEdit 代码分析报告

> 分析对象：nim_duilib 的 RichEdit / RichEdit2 实现
> 分析日期：2026-11-12

## 一、实现结构

| 层 | 文件 | 说明 |
|---|---|---|
| 原生实现 | `duilib/Control/RichEdit_Windows.cpp`、`RichEditHost_Windows.cpp` | 用 ITextServices 承载 Windows 原生 RichEdit，`class="RichEdit"` |
| 自绘实现 | `duilib/Control/RichEdit2.cpp`（约 4700 行） | 完全自绘 + 自维护文本模型，`class="RichEdit2"`，也是 SDL 下唯一实现 |
| 文本模型 | `duilib/Control/RichEditData.cpp`（约 2900 行） | 行拆分、布局缓存、选择矩形、撤销/重做、密码掩码 |
| 绘制引擎 | `duilib/RenderSkia/DrawRichText.cpp` | Skia 行排版、绘制缓存、截断 |

整体分层清晰、缓存校验完整，但两套实现的行为差异较大，且自绘实现存在若干功能性缺陷。

---

## 二、确定性缺陷（建议优先修复）

### 1. 密码模式下横向滚动失效（高）

`RichEdit2.cpp` L1923-L1951：密码分支直接 `DrawString(passwordText, drawParam)` 到 `GetRichTextDrawRect()`（未滚动坐标），**没有应用 `GetScrollOffset()`**。而光标、选择区都按滚动后的布局绘制 → 超宽密码输入时文字不动、光标向右跑出可视区，选择高亮与文字错位。

建议：密码文本也走 `DrawRichText` 缓存（掩码文本作为绘制数据），或至少对 `rcDrawRect` 应用 `szScrollOffset`，并支持裁剪。

### 2. 单行模式可以存入换行符，且显示时"吞字"（高）

- `RichEditData.cpp` L871：`m_bSingleLineMode` 时整段文本作为一行，**不拆分 `\r\n`**；
- `RichEdit2.cpp` L4757：回车插入 `L"\r\n"`；
- `DrawRichText.cpp` L549、L786：单行模式遇到 `\n` 时跳行但不推进，`\r` 宽度为 0。

结果：向单行 RichEdit2 粘贴 "ab\r\ncd" 会显示成 "abcd"（两个词直接粘连），`GetText()` 却返回带换行符的原文，光标会停在不可见位置。Windows 单行编辑框会把换行替换为空格或丢弃。

建议：`Paste`/`SetText` 在单行模式下统一做 `\r\n|\r|\n` → 空格（或删除）规整。

### 3. 粘贴绕过数字范围修正（高，行为不一致）

`RichEdit2.cpp` L1529-L1554 的 `Paste()` 只对密码模式剔字符，直接 `ReplaceText`；`CanPaste()` 只用 `IsPasteLimited`（L1782-L1825）校验字符白名单，**不做 `min_number/max_number` 夹取**。而 Windows 实现会在文本变化后统一夹取（`RichEdit_Windows.cpp` L3523-L3553）。

结果：`number_only` + 范围 [0,64] 的框，键入 999 会被纠正，粘贴 999 却直接留在框里。建议把数字夹取下沉到 `RichEdit2` 的文本变化回调（与 Windows 实现一致），而不是只写在 `OnInputChar`。

### 4. `OnKeyDown` 吞掉全部按键；Ctrl+左右在无滚动条时完全无效（高）

- `OnKeyDown`（`RichEdit2.cpp` L3615-L3686）结尾无条件 `return true`，未处理的键（F5、Ctrl+F、Alt+字母 等）不会冒泡，窗口级快捷键/默认按钮失效；`want_return=false` 时回车也不会触发默认按钮。
- `OnCtrlArrowKeyDownScrollView`（L3688-L3765）：Ctrl+方向键被解释为滚动，且**无论滚动条是否存在都返回 true**。没有滚动条的输入框里 Ctrl+←/→ 什么都不发生；同时也没有实现 Windows 的按词移动 / Ctrl+Shift 按词选择。

建议：未命中的键 `return false` 交给基类；Ctrl+←/→ 实现按词移动与按词选择，滚动改用 Alt/仅在有滚动条时处理。

### 5. 焦点底部边框在圆角下左右不对称（高，纯视觉）

`PaintBorder`（`RichEdit2.cpp` L2047-L2057）：

```cpp
rcBorder.right -= round;   // 右侧内缩（正确）
rcBorder.left  -= round;   // 左侧向外扩（错误，应为 +=）
```

左端被画到控件外（被裁剪），右端提前结束，焦点下划线呈"左贴边、右缺角"。建议 `left += round`，并让两端与圆角内切；同时该下划线在 `IsReadOnly()` 时被跳过（L2039），只读框聚焦无任何视觉反馈。

### 6. 当前行高亮与 `hide_selection` 耦合，`inactive_current_row_bkcolor` 实际失效（中高）

`PaintCurrentRowBkColor`（`RichEdit2.cpp` L2243-L2277）开头 `if (IsHideSelection() && !m_bActive) return;`：`hide_selection` 默认 true，因此非激活时**永远走不到** `GetInactiveCurrentRowBkColor()`（L2257），该配置项是死代码；另外 Paint 只在 `!IsReadOnly() && IsEnabled()` 时绘制（L1869-L1880），只读框没有当前行提示。

建议：当前行高亮只由颜色是否配置决定，不参与"选择内容隐藏"的逻辑；是否只读单独用颜色区分。

### 7. 滚动条预留空间在"测量"和"绘制"中不一致（中高）

- `GetTextDrawRect`（`RichEdit2.cpp` L3322-L3342）：只要滚动条对象存在就扣除宽/高，**不判断 `IsValid()`**；
- `CalcRequiredSize`（L3369-L3378）：只有 `IsValid()` 时才把滚动条宽加回。

结果：声明了 `vscrollbar` 但内容未溢出（滚动条不显示）时，文本区永远少一列滚动条宽度——`text_align="center"` 明显偏左，右侧留白；`auto` 宽度测量也偏小可能截断。

建议：两处统一判定；或在滚动条显隐变化时触发一次布局（ScrollBox 已有 `UpdateScrollBar` 路径），避免"估算与绘制不一致"。

### 8. `CalcRequiredSize` 中两组 `else if` 疑似逻辑错误（中）

`RichEdit2.cpp` L3344-L3366：宽超限时把 `cx` 归零，高度即使同时超限也不会处理；`imageSize` 同理。若"归零"表示"不限制该方向"，则应为两个独立 `if`。另外 L3372-L3377 的 `if/else` 两个分支代码完全相同（复制粘贴残留），可直接删除分支。

### 9. IME 组字/候选窗口定位缺少控件原点（中高，需运行验证）

`OnImeStartComposition`（`RichEdit2.cpp` L3508-L3553）：`pt = GetCaretPos() - 外层滚动偏移`，但没有加控件在窗口中的位置；对比 `ShowCaret`（L2102-L2115）与 CefControlOffScreen 的做法（`GetRect()` 再叠加内容偏移），控件不在窗口左上角时候选框会跑到左上角。`ShowCaret` 里算出的 `rc` 也几乎未被使用（死代码痕迹），说明这里未完成。

建议：统一封装"控件局部坐标 → 窗口客户区坐标"的换算。

### 10. `Undo/Redo` 失败时列表状态已改变（中）

`RichEditData.cpp` L2697-L2756：先把条目从 undo 移到 redo，再 `ReplaceText`；若文本限制等原因导致替换失败，迁移不会回滚，撤销链与文本不一致（后续 Ctrl+Z 会"跳步"）。建议先试算/成功后再生效，或失败时回滚两个列表。

---

## 三、其他不足与一致性

- **光标位置不一致**：RichEdit2 在 `xPos+1` 画线（L2238），Windows 版在 `x` 画（`RichEdit_Windows.cpp` L3068-L3084）；DPI 1.5 时 2px 宽光标还会向右多出 1px。建议统一为插入点坐标并按 DPI 取整。
- **粘贴重复读剪贴板**：`CanPaste()`（→`IsPasteLimited`）与 `Paste()` 各读一次，剪贴板内容/格式变化时行为不一致，也无谓复制。建议一次读取并复用。
- **超限静默丢弃**：`ReplaceText` 因 `limit_text` 失败直接 `return false`（`RichEditData.cpp` L1062-L1069），输入无事件、无提示；Windows 版有 `kEventTextLimit`。建议自绘版补齐该事件。
- **数字模式输入 "-" 被拒**：`GetTextLength() > 0` 时不接受 "-"，即使当前是全选状态（本应被替换）。建议改为"选中整个文本时允许"。
- **撤销粒度**：每次输入都生成一条 undo（`AddToUndoList` L2665），逐字符撤销，体验不如原生按输入串合并。
- **构造初始化顺序与声明顺序不一致**（`RichEdit2.cpp` L38-L110），如 `m_bAllowPrompt`、`m_bNoSelOnKillFocus`、`m_sSelectionBkColor` 等，MSVC `/W4` 会报 C5038，且后续若出现依赖初始化的成员会变成隐蔽 bug。建议按声明顺序排列。
- **`SetFocusedBottomBorderSize`** 先 `ASSERT(size > 0)` 后又允许 `== 0`（L1399-L1406），断言与实现矛盾。
- **密码掩码按 UTF-16 码元**：emoji 显示为两个 `*`，`limit_text` 也按码元计数；与 Windows 一致但需明确文档。
- **IME 组合文本无内联显示**：RichEdit2 组合期间仅隐藏光标（L3543），无带下划线的 preedit，拼音选词时看不到正在组合的内容。
- **三击选段缺失**：双击选词已实现，三击选整段未实现。

---

## 四、性能优化

1. **Windows 版每次重绘整块回读**：`PaintRichEdit`（`RichEdit_Windows.cpp` L2674）对整个控件区域 `ReadPixels`，而实际只绘制 `rcPaint` 脏区；光标 500ms 闪烁即触发一次全控件 GPU→CPU 回读，大文本时明显卡顿。建议只回读 `rcUpdate`，或缓存文本位图、仅在内容/选择/主题变化时重建。
2. **密码模式每帧全量拷贝**：`RichEdit2.cpp` L1925-L1927 每次 Paint 都 `GetText()` + `ReplacePasswordChar()`，O(n) 分配。建议缓存掩码字符串，仅在文本/显示开关变化时重建。
3. **缩放表每次重建**：`GetNextZoomPercent`（L4408）每次 Ctrl+滚轮都构造 `std::vector`（且 `insert(begin)` 是 O(n²)）。改成静态常量数组/一次性构建。
4. **Paint 中做布局**：L1866 `CheckCalcTextRects()` 在绘制路径上，若尺寸/字体变更未提前触发布局，会把排版开销算进首帧绘制。建议统一在 `OnTextRectsChanged`/尺寸变化时完成。
5. **调试版全量重排**：`CalcTextRectsPart` 调试分支每次都做全量布局校验（Debug 下编辑大文本明显变慢），可加开关或采样。
6. **密码绘制未用绘制缓存**：见缺陷 1，改为缓存后同时解决滚动与性能问题。

---

## 五、界面美观度改进

1. **选择高亮/当前行/光标忽略控件透明度**：文本用 `GetAlpha()` 绘制，但选择填充（L2279-L2340）、当前行（L2243-L2277）、光标（L2238）都是纯色；控件淡入淡出/半透明皮肤下会出现"文字淡了、底色不淡"。
2. **圆角裁剪缺失**：选择区、当前行只与文本矩形求交，没有按 `border_round` 路径裁剪，大圆角皮肤下高亮会溢出到圆角外。
3. **选中/非激活色硬编码**：默认 `CornflowerBlue` / `DarkGray`，深色主题下 `DarkGray` 对比度极低；建议改为主题色令牌，或用主色 + alpha 派生非激活色。
4. **占位文本**：无省略号处理（`PaintPromptText`，L2412-L2443 直接 `DrawString`），超长 placeholder 被硬切；且 `PaintStateImages` 在只读时提前返回（L2464），只读空框看不到提示。建议单行 truncated + ellipsis，颜色缺省回退到 disabled 文本色，只读不拦截 prompt。
5. **焦点下划线**：修缺陷 5；另外建议下划线宽度、颜色随 DPI/主题缩放，并支持只读聚焦反馈。
6. **滚动条预留**：修缺陷 7，否则居中文本永远偏一侧。
7. **光标**：统一位置与宽度、支持透明度；组合输入时可考虑绘制 1px 细光标替代完全隐藏。

---

## 六、功能增强建议

- Ctrl+←/→ 按词移动、Ctrl+Shift+←/→ 按词选择、Ctrl+Home/End 文档首尾（现为滚动）。
- 三击选段、Shift+单击扩展选择。
- 撤销合并（连续输入/连续删除合成一条）。
- 输入法 preedit 内联绘制（下划线样式）。
- 数字模式支持 `+`/小数点（若与 SpinBox 共用），或明确由 SpinBox 覆盖。
- 无障碍：把光标、选择、链接区域暴露为 `IAccessible` 文本接口（框架若无基础设施可先做只读文本角色）。

---

## 七、文档需补全/修正（中英文同步）

- `hide_selection` 描述为"是否隐藏选择内容"，实际语义是"**控件非激活时**隐藏选择/当前行"，需改准。
- `auto_vscroll`、`auto_hscroll`、`save_selection`、`allow_beep`、`rich_text`、`auto_detect_url` 标注"SDL 实现时无效"，实际是"**RichEdit2 实现无效**（Windows 上选择 `RichEdit2` 同样无效），仅原生 `RichEdit` 支持"。
- 增加"RichEdit vs RichEdit2 差异清单"：粘贴过滤、数字夹取、撤销粒度、行距、对齐、IME、焦点下划线、密码滚动等（本报告第二、三节可直接提炼）。
- `limit_text`/`limit_chars` 补充计数单位（UTF-16 码元，emoji 计 2）。
- `password_char` 文案中的多余空格；补充"仅短暂显示最后一个字符"的准确描述。
- 补充单行模式下粘贴换行的处理策略（替换为空格/丢弃）与 `text_align` 支持值/大小写。

---

## 八、建议实施顺序

1. 先修 **密码滚动、单行换行规整、粘贴数字夹取、OnKeyDown 冒泡**（用户可感知的功能错误）。
2. 再修 **IME 定位、undo 失败回滚、滚动条预留一致性**（正确性/布局）。
3. 视觉批次：**焦点下划线、当前行解耦、透明度与圆角裁剪、prompt 省略号**。
4. 性能批次：**Windows ReadPixels 脏区化、密码文本缓存、缩放静态表**。
5. 最后补测试：单行粘贴换行、数字模式粘贴越界、密码超宽滚动、只读 prompt、焦点下划线几何。
