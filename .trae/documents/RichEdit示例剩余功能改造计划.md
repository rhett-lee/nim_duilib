# RichEdit 示例剩余功能改造计划

## Context

`docs/misc/RichEdit示例程序改进.md` 中的 P0 缺陷与界面统一已完成（保存不再覆盖资源 XML、查找替换参数修正、编码统一、未保存保护、卡片布局、状态栏、语言刷新、光标归位、字体下拉补全等）。本次继续推进第三部分“功能增强建议”和第四部分“代码结构与文档”中尚未落地的内容。

约束：
- 不提交任何代码；`docs/misc/RichEdit示例程序改进.md` 不纳入提交。
- 只做 Debug 构建验证，不构建 Release。
- 保持 Windows 非 SDL 为主路径，同时不破坏 SDL 可编译性。

## 现状确认（不重做）

- `bin/resources/themes/default/rich_edit/rich_edit.xml` 已有页面内边距、`panel_card`、Split、工具栏自适应按钮、状态栏（路径/行列/字符数/缩放/模式/编码）。
- `MainForm` 已有：未保存确认、打开/保存失败提示、编码自动识别读写、语言切换刷新、颜色取消还原、快捷键 `bHandled`、示例文档保存转另存为、按模式推断默认扩展名。
- 查找/替换已有：允许空替换、未找到状态栏提示、“已替换 1 处/全部替换”提示。
- `RichEditFindReplace::OnTextNotFound` 与 `AdjustDialogPosition` 已具备基础能力。

---

## 实施步骤

### 第一步：工具栏状态联动（3.1）与行距联动（3.5）

文件：`examples/RichEdit/MainForm.h/.cpp`、`bin/resources/themes/default/rich_edit/rich_edit.xml`

1. 新增 `void UpdateEditButtonStatus()`：
   - `btn_undo` ← `CanUndo()`
   - `btn_redo` ← `CanRedo()`
   - `btn_paste` ← `CanPaste()`
   - `btn_copy/btn_cut/btn_delete/btn_sel_none` ← `!GetSelText().empty()`
   - `btn_find_next` ← `m_findReplace.HasFindText()`
2. `btn_delete` 行为改为删除当前选中内容（`ReplaceSel(_T(""))`），不再是 `Clear()` 全清。
3. 在 `AttachSelChanged`、`AttachTextChanged`、所有编辑命令执行后、文档加载/查找替换后调用。
4. XML 给行距标签与 6 个 `Option` 增加 `name`（如 `line_spacing_label`、`row_spacing_1_0`…）。
5. 新增 `void UpdateRowSpacingEnabled()`：富文本关闭时 `SetEnabled(false)`（保留布局，不隐藏）；在初始化、富文本复选框切换后调用。

### 第二步：新建文档（3.2）、查找替换键盘与选项记忆（3.3）、文件拖放（3.8）

文件：`examples/RichEdit/MainForm.h/.cpp`、`FindForm.h/.cpp`、`ReplaceForm.h/.cpp`、`RichEditFindReplace.h`

1. 新建文档：
   - XML 文件行增加 `Button name="new_file"`（`STRID_RICHEDIT_NEW_FILE`）。
   - `OnNewFile()`：`ConfirmDiscardChanges()` → 清空文本、`SetModify(false)`、`m_filePath.Clear()`、`m_bSampleDocument=false`、编码显示 UTF-8、刷新状态。
   - 快捷键 `Ctrl+N`。
2. 查找/替换对话框：
   - 重写 `OnKeyDownMsg`：`Esc` 关闭，`F3` 查找下一个。
   - 查找输入框 `AttachReturn` → 查找下一个；替换输入框回车 → 替换；两个输入框 `AttachEsc` → 关闭。
   - 在 `RichEditFindReplace.h` 增加 `RichEditFindOptions`（findText/replaceText/bFindDown/bMatchCase/bMatchWholeWord）；`MainForm` 保存上一轮选项并提供 getter/setter；两个对话框初始化时读取、执行前写回。
   - 打开对话框时优先用编辑区选中文本，否则用上次查找内容。
3. 拖放打开：
   - `m_pRichEdit->SetEnableDropFile(true); SetDropFileTypes(_T(".txt;.rtf"));`
   - `AttachDropData` 取 `m_fileList` 第一个文件，处理 `kControlDropTypeWindows/SDL`，走 `ConfirmDiscardChanges()` + `LoadFile()`，成功后同步 `m_filePath`/上次目录/修改标志/状态栏；失败弹打开失败提示。
   - `LoadFile` 中：非富文本模式遇到 `.rtf` 直接失败，避免把 RTF 二进制当文本读。

### 第三步：缩放标准下拉（3.4）与去除重复缩放表（4.3）

文件：`MainForm.h/.cpp`、`rich_edit.xml`

1. 工具栏缩放行去掉 `btn_zoom_in/btn_zoom_out/btn_zoom_off` 三个文字按钮，改为 `Combo name="combo_zoom"`（50/75/100/125/150/200/300/400），保留 `lavel_zoom_value` 显示实际比例。
2. 新增 `InitZoomCombo()` 与 `SyncZoomCombo()`：选择项设置缩放；`AttachZoom`/滚轮缩放后同步下拉（非标准值时清空选择，仅更新标签）。
3. 增加 `Ctrl+0` 复位 100%。
4. 删除 `MainForm::GetNextZoomPercent` 及其复制的 0.9/1.1 缩放表；示例只使用标准比例列表，控件内部表仅保留在 `RichEdit2`。`MainForm.h` 同步删除声明。

### 第四步：设置持久化（3.6）与关闭清理确认（4.4）

文件：`MainForm.h/.cpp`

1. 参照 `examples/TreeView/MainForm.cpp#L101-177` 与 `#L1960-2059`，在本文件匿名命名空间内加入 `SettingsMap/ParseSettingsText/ReadSettingsFile/GetSettingInt/AppendSettingInt`，并补充字符串键的读写。
2. `GetSettingsFilePath()` → `程序目录/user/RichEdit_settings.ini`。
3. `LoadSettings()`（`OnInitWindow` 中在加载示例前执行）：
   - 窗口位置/大小：`window_left/top/width/height`（物理像素，`SetWindowPos(ui::InsertAfterWnd(), …, ui::kSWP_NOZORDER)`；用 `GetMonitorWorkRect` 做可见性/尺寸钳制）。
   - 左侧面板宽度：`left_panel_width`（XML 左侧 `VScrollBox` 增加 `name="left_panel"`，用 `SetFixedWidth` 恢复）。
   - `zoom_percent`、`font_name`、`word_wrap`、`rich_text`。
4. `SaveSettings()`（`OnPreCloseWindow` 中，控件仍有效）：写回上述值；记录 `m_lastFontName`。
5. `OnCloseWindow` 已有的查找/替换窗口关闭与指针置空保持不变，补一次自检确认无悬空指针路径。

### 第五步：默认示例内容（3.7）

文件：`MainForm.cpp`、`rich_edit.xml`、两个语言文件

1. 删除 `LoadRichEditData()` 中读取 `rich_edit.xml` 源码的逻辑，改为内置样例文本（标题 + 功能说明 + 快捷键 + 行距示例），内容通过语言键获取，切换语言时可重载。
2. XML 默认 `rich_text="true"`；仅 Windows 非 SDL 下对样例应用格式：
   - 标题：加粗、加大字号、强调色；
   - 快捷键标题：加粗；
   - 部分正文：斜体/灰色；
   - `SetRowSpacingMul(1.25f)` 展示行距。
3. 应用格式后 `SetSel(0,0)`、`HomeUp()`、`SetModify(false)`，保持“示例文档”标记。

### 第六步：代码结构（4.1、4.2）

文件：`MainForm.h/.cpp`、`FindForm.h/.cpp`、`ReplaceForm.h/.cpp`

1. `OnInitWindow` 拆分为：
   - `BindClick(name, fn)` 辅助函数；
   - `SetupFileTools()`、`SetupEditTools()`、`SetupFindTools()`、`SetupFontTools()`、`SetupViewTools()`、`SetupTextEvents()`、`SetupDropFile()`。
   保留原行为，仅移动代码与去重 `dynamic_cast + GetSender` 模板。
2. `FindForm.h` 增加公共基类 `FindReplaceFormBase : public ui::WindowImplBase`，承载：
   - 公共成员（主窗口指针、方向/大小写/全字匹配、查找输入框）；
   - 选项收集、上次选项读写、查找输入框初始化；
   - `OnFindNext()`、`OnKeyDownMsg()`（Esc/F3）、取消按钮绑定。
   `FindForm` 与 `ReplaceForm` 继承它，只保留各自差异（替换输入框、替换/全部替换按钮、XML 文件）。不新增文件，避免改构建脚本。

### 第七步：资源清理（4.5）与文档（4.6）

1. 删除无引用的 `bin/resources/themes/default/rich_edit/logo_18x18.png`。
2. 删除中英文语言文件中未使用的 `STRID_RICHEDIT_MINIMIZE/MAXIMIZE/RESTORE/CLOSE`。
3. 更新新增语言键（中英文同步）：
   - `STRID_RICHEDIT_NEW_FILE`
   - `STRID_RICHEDIT_REPLACE_DONE/REPLACE_ALL_DONE`（改为 `%d` 计数格式）
   - `STRID_RICHEDIT_ZOOM_SELECT_TIPS`
   - `STRID_RICHEDIT_SAMPLE_*`（标题/正文/快捷键/行距示例）
4. 更新 `docs/Examples.md` 与 `docs/Examples.en.md` 的 RichEdit 条目，按 TreeView 风格列出能力、快捷键、资源与设置文件位置。

---

## 关键文件

- `examples/RichEdit/MainForm.h` / `MainForm.cpp`
- `examples/RichEdit/FindForm.h` / `FindForm.cpp`
- `examples/RichEdit/ReplaceForm.h` / `ReplaceForm.cpp`
- `examples/RichEdit/RichEditFindReplace.h` / `RichEditFindReplace.cpp`
- `bin/resources/themes/default/rich_edit/rich_edit.xml`、`find.xml`、`replace.xml`
- `bin/resources/lang/zh_CN-examples.txt`、`en_US-examples.txt`
- `docs/Examples.md`、`docs/Examples.en.md`
- 删除：`bin/resources/themes/default/rich_edit/logo_18x18.png`

## 验证

1. Debug 构建：仓库根执行 `py -3 build\build.py build --config Debug --arch x64 --offline`。
2. 启动截图（DPI 感知方式）：
   - PowerShell P/Invoke `SetProcessDpiAwarenessContext(-4)`；
   - `Start-Process` 后轮询 `MainWindowHandle`，`GetWindowRect`，`PrintWindow(h, hdc, 2)` 保存到 `$env:TEMP`；
   - 启动与截图在同一条命令内完成。
   检查新增按钮/缩放下拉/状态栏/示例富文本是否无裁切、无越界。
3. 手工功能核对：
   - 撤销/重做/粘贴/复制/剪切/删除/查找下一个的可用状态随选择与内容变化；
   - 新建、Ctrl+N、未保存确认；
   - 查找/替换：Enter、Esc、F3、空替换、选项记忆；
   - 拖入 `.txt`/`.rtf` 打开；
   - 缩放下拉、Ctrl+0、Ctrl+滚轮后状态栏同步；
   - 富文本关闭时行距选项禁用；
   - 关闭再打开，窗口位置/大小、分栏宽度、缩放、字体、换行、富文本状态恢复；
   - 中英文切换后新增文案正常。
4. 检查设置文件生成于 `user/RichEdit_settings.ini`（不提交）。

## 风险与取舍

- 窗口位置恢复使用物理像素并做工作区钳制，避免多显示器/缩放变化后窗口跑到屏幕外。
- 4.2 采用“公共基类 + 保留两个 XML”的方式，不改视觉与交互，只去重代码，降低回归风险。
- 删除 `logo_18x18.png` 前再次全仓库确认无引用（当前仅文档示例提及同名文件，不在 rich_edit 目录下）。
