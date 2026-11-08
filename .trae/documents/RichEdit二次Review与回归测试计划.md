# RichEdit 示例二次 Review 与修复/回归测试计划

## 背景

`examples/RichEdit` 已完成 P2 代码质量拆分与上一轮 12 项问题修复。本轮做第二次 Review（含非 Windows / Windows+SDL 场景），发现 5 个问题，其中 3 个为确定的功能缺陷。修复后对 Windows 与 SDL 两个平台分别构建并做系统性功能测试，发现问题继续修复并回归，直到无问题。

## 修复清单

| 编号 | 问题 | 位置 | 修复方式 |
| --- | --- | --- | --- |
| F1 | 左侧分组折叠的标题区点击判定坐标错误：`args.ptMouse.y`（窗口客户区坐标）与 `pFirstChild->GetPos().top`（内容坐标）直接比较，左侧面板滚动后判定失效，点击标题无法折叠/展开 | [MainForm.cpp](file:///e:/develop/nim_duilib/examples/RichEdit/MainForm.cpp#L464-L472) | 用 `pGroup->MousePosToLayoutPos(args.ptMouse, pt)` 先换算为分组内相对坐标；标题高度取 `pFirstChild->GetPos().top - pGroup->GetPos().top`；折叠状态下整块区域都可点击展开 |
| F2 | 拖放文件加载失败时使用了未定义的文本 ID `STRID_RICHEDIT_OPEN_FILE_FAILED`，标题误用工具栏文案 `STRID_RICHEDIT_OPEN_FILE`，失败提示显示异常 | [MainForm.cpp](file:///e:/develop/nim_duilib/examples/RichEdit/MainForm.cpp#L440-L442) | 改为与“打开文件”一致的 `STRID_RICHEDIT_OPEN_FAILED_TEXT` / `STRID_RICHEDIT_OPEN_FAILED_TITLE` |
| F3 | 富文本示例文档的格式设置相互继承：每次在上一段仍被选中时调用 `GetFontInfo()`，导致快捷键标题/行距标题变成 16 号粗体，正文变成 16 号粗体斜体 | [MainForm.cpp](file:///e:/develop/nim_duilib/examples/RichEdit/MainForm.cpp#L1018-L1041) | 在任何格式设置之前一次性获取基础字体 `baseFont`，每段都从 `baseFont` 派生，只改需要的属性 |
| F4 | Windows“设置字体”对话框未遵守“应用到全文”复选框：`SetCharFormat` 在富文本模式下只作用于当前选区，与其他字体按钮行为不一致 | [MainFormFormat.cpp](file:///e:/develop/nim_duilib/examples/RichEdit/MainFormFormat.cpp#L633-L645) | 富文本模式下按 `IsApplyToAll()` 决定：临时全选→应用→恢复原选区 |
| F5 | 非 Windows 平台也会把 `rich_text=0` 写入共享的 `bin/user/RichEdit_settings.ini`，会覆盖 Windows 版本的富文本设置 | [MainFormSettings.cpp](file:///e:/develop/nim_duilib/examples/RichEdit/MainFormSettings.cpp) | 仅 `DUILIB_BUILD_FOR_WIN && !DUILIB_BUILD_FOR_SDL` 时保存 `rich_text`，其余平台不写该键 |

说明：Review 中还核对了 `kEventLinkClick` 的 WPARAM 类型、`ReplaceAll` 循环终止性、`CursorID 0`、`FindDialog` 的 `nFileTypeIndex`、文本 ID 完整性等，均无问题（Linux 下 RichEdit2 不发 link 事件，属框架限制，示例无需改动）。

## 实施步骤

1. 按上表逐项修改代码（仅 3 个文件：MainForm.cpp、MainFormFormat.cpp、MainFormSettings.cpp）。
2. 编译验证：
   - Windows：删除 `lib/x64/Debug/duilib.lib`（两套构建共用，避免互相覆盖），用 MSBuild 依次构建 `duilib.vcxproj`、`RichEdit.vcxproj`（Debug x64，需 `/p:SolutionDir=e:\develop\nim_duilib\build\`，`/m:1`）。
   - SDL：`python build\build.py build --config Debug --arch x64 --sdl --offline --jobs 2 --target RichEdit`（输出 `bin\RichEdit.exe`）。
3. 运行测试（两平台各一轮，使用键盘操作规避 175% DPI 下鼠标注入偏移）：
   - 编辑：输入、复制/剪切/粘贴/删除、全选、撤销/重做。
   - 查找：`Ctrl+F`，验证向下/向上查找、大小写、全字匹配、结果计数、“未找到”提示。
   - 替换：`Ctrl+H`，验证单次替换、全部替换、替换后提示与计数清空。
   - 左侧分组：点击标题折叠/展开；**先滚动左侧面板再点击标题**（F1 回归重点）。
   - 字体：名称/字号/粗体/斜体/下划线/删除线、颜色下拉、“更多颜色”对话框；勾选“应用到全文”后整篇生效（F4 回归重点）。
   - 行距：三个行距选项的切换效果及启用状态。
   - 视图：自动换行、缩放下拉、`Ctrl+滚轮`、`Ctrl+0`。
   - 文件：新建/打开(*.txt)/保存/另存为/拖放；构造加载失败场景验证提示文案（F2 回归）。
   - 状态栏：路径与 `*` 标记、行列号、字符数/选中数、缩放、模式与换行、编码、提示 3 秒自动消失。
   - Windows 额外：富文本模式开关（关闭时确认弹窗）、RTF 打开/保存、行距在富文本下可用、字体对话框“应用到全文”、链接点击；富文本示例文档各段格式是否正确（F3 回归重点）。
   - SDL 额外：富文本复选框置灰、行距选项始终可用。
4. 每次测试发现新问题立即修复，重新构建并回归；直至两平台无问题。
5. 清理测试截图与临时文件（`bin/user/RichEdit_settings.ini` 如产生则删除）。

## 验证标准

- 两平台均编译通过，无新增警告（Debug x64）。
- 上述功能项全部符合预期，无崩溃、无显示异常、无重复/遗漏提示。
- 代码修改仅限上述 5 项，不引入无关重构。
