# 文件对话框参数补齐（SDL 后端）Tasks（C-6）

- [x] T1 `duilib/Utils/FileDialog_SDL.cpp`：新增文件内静态辅助函数
  - [x] T1.1 `struct DlgFileFilter`（`m_names` / `m_exts` / `m_filters`）+ `ConvertFileTypes`（LX101-L127）：合并 `BrowseForFile` 与 `BrowseForFiles` 中重复的筛选器转换代码（`"*.txt"` → `"txt"`，`szNameId` 走 `GlobalManager::GetTextById`），`m_filters[i].name/pattern` 指向 `m_names`/`m_exts` 的 `c_str()`，生命周期覆盖整个调用（局部对象，`DlgFileFilter` 在函数结束前不析构）
  - [x] T1.2 `GetDefaultExtension`（LX129-L154）：`"*.txt"`→`txt`；`"doc;docx"`→`doc`；`.txt`→`txt`；`"*.*"`→`""`（去前缀点号后为空 → 无效）
  - [x] T1.3 `GetDefaultLocation`（LX156-L179）：`defaultLocation` 不以 `/`、`\` 结尾时补齐 `FilePath::GetPathSeparatorStr()`，再拼接 `defaultFileName`；返回 UTF8 字符串
- [x] T2 `BrowseForFile`（LX256-L341）：三个参数生效
  - [x] T2.1 形参恢复命名（`nFileTypeIndex` / `defaultExt` / `defaultLocation`）
  - [x] T2.2 默认扩展名：`defaultExt` 为空且 `0 <= nFileTypeIndex < fileTypes.size()` 时取 `fileTypes[nFileTypeIndex].szExt`，再经 `GetDefaultExtension` 解析（LX276-L284）
  - [x] T2.3 `fileName` 无扩展名（`FilePath::GetFileExtension()` 为空）时追加 `"." + 默认扩展名`（LX286-L294）
  - [x] T2.4 `default_location = GetDefaultLocation(defaultLocation, 带扩展名的fileName)`；为空时传 `nullptr`（LX296-L305）
  - [x] T2.5 筛选器改用 `ConvertFileTypes`（LX309-L311，行为与改动前一致）
  - [x] T2.6 保存对话框返回值校正：`!bOpenFileDialog && 返回值无扩展名 && 默认扩展名非空` → 追加默认扩展名（LX334-L338）
  - [x] T2.7 中文注释已补（LX269-L275、LX296、LX314、LX334-L335），说明 SDL 的三项能力限制
- [x] T3 `BrowseForFiles`（LX343-L404）
  - [x] T3.1 形参恢复命名；`defaultLocation` 经 `GetDefaultLocation(defaultLocation, DStringA())` 生效（LX367）
  - [x] T3.2 `nFileTypeIndex` / `defaultExt` 以 `(void)xxx;` 标注未使用并写明原因（LX350-L354）
  - [x] T3.3 删除恒为空字符串的 `defaultFileName` 死代码；筛选器改用 `ConvertFileTypes`
  - [x] T3.4 `LX329` 的过期 TODO 替换为准确说明（现 LX394-L396：`IFileOpenDialog::GetResults`，NULL 结尾数组）
- [x] T4 目录对话框修正
  - [x] T4.1 `BrowseForFolders`：`allow_many` → `true`（LX239，对齐 Windows `FOS_ALLOWMULTISELECT`）
  - [x] T4.2 `BrowseForFolder` / `BrowseForFolders`：`default_location` 改用 `GetDefaultLocation(...)` 规范化（LX194 / LX231）
- [x] T5 调用侧启用
  - [x] T5.1 `duilib/Control/PropertyGrid.cpp` `PropertyGridFileProperty::OnBrowseButtonClicked`（LX3501-L3510）：由当前属性值推导 `fileName = GetFileName()`、`defaultLocation = GetParentPath()` 并传入
  - [x] T5.2 `examples/RichEdit/MainForm.cpp` `OnOpenFile`（LX1151-L1153）与 `OnSaveAsFile`（LX1196-L1198）：传入 `defaultLocation = m_filePath.GetParentPath()`
- [x] T6 编译验证
  - [x] T6.1 `MSBuild.exe build\examples.sln /p:Configuration=Debug /p:Platform=x64 /m` 增量编译：**0 error / 0 warning**，全部示例链接成功（含 `RichEdit_d.exe`、`render_d.exe`）
  - [x] T6.2 `build_temp\x64\duilib\Debug\FileDialog_SDL.obj`、`PropertyGrid.obj` 时间戳 2026/10/20 17:54，`bin\*_d.exe` 重新链接（17:54-17:55），改动已进入产物
- [ ] T7 运行时验证（**人工手动执行，助手不代跑 GUI**）
  - [ ] T7.1 `render_d.exe` 分组八「文件路径（保存）」：初始目录 `C:\`、文件名预填 `Test-Save.txt`
  - [ ] T7.2 `render_d.exe` 同上：把文件名改为 `abc` 后确认，返回值应为 `C:\abc.txt`
  - [ ] T7.3 `render_d.exe`「文件路径（打开）」：初始目录 `C:\`、文件名预填 `Test-Open.txt`
  - [ ] T7.4 `render_d.exe`「文件夹」：初始目录 `C:\Test`
  - [ ] T7.5 `RichEdit_d.exe` 打开/另存为：初始目录 = 当前文件所在目录
  - [ ] T7.6 多选核对（`BrowseForFiles` / `BrowseForFolders` 无现成调用点）：临时加入调用点运行后删除，核对"可选多个条目 + 返回全部路径"
- [ ] T8 回归核对（**人工手动执行**）
  - [ ] T8.1 若 `SDLEnabled=0` 编译：原生 Windows 后端路径不受影响（本次未改 `FileDialog_Windows.cpp`）
  - [ ] T8.2 取消对话框（Esc/取消按钮）返回 `false`，`filePath`/`folderPath` 保持空，无卡死、无重复回调
  - [ ] T8.3 无筛选器 / 空 `fileTypes` 时对话框正常打开（`filters=nullptr`）

# Task Dependencies
- T2 / T3 / T4 依赖 T1 ✅
- T5 独立（可与 T2-T4 并行）✅
- T6 依赖 T1-T5 ✅
- T7 依赖 T6（**待用户执行**）
- T8 依赖 T7（**待用户执行**）

# 状态说明（2026-10-20）
- **T1-T6 已完成并验证**：代码改动全部落地，Debug x64 增量编译 **0 error / 0 warning**，目标 obj/exe 均已重新生成。
- **T7 / T8 待用户人工验证**：涉及原生模态文件对话框的交互，无法自动化（duilib 为自绘 UI、不提供 UI Automation 提供程序，无法定位"浏览"按钮；助手亦无原生窗口截图能力），故按已批准的 spec 交由用户手动核对。
- 多选相关（T4.1、T7.6）同理需人工确认。

# 备注
- 代码提交由用户手动执行，助手不代为提交；`docs/misc/Development-Plan.md#L754` 的 C-6 勾选与"完成日期"由用户确认后填写。
- 只验证 Debug x64 编译，Release 由用户自行处理。
