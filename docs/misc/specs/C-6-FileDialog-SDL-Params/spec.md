# 文件对话框参数补齐（SDL 后端）Spec（C-6）

## Why
`docs/misc/Development-Plan.md` 的 **C-6**（`Development-Plan.md#L441-L446`）：`FileDialog_SDL.cpp` 中 `BrowseForFile` / `BrowseForFiles` 的 `nFileTypeIndex` / `defaultExt` / `defaultLocation` 三个参数**被注释掉**（形参未命名），即完全未生效；`BrowseForFile` 还把 `fileName` 误当作 `default_location` 使用。同一份公开声明（`FileDialog.h`）在 Windows 原生后端（`FileDialog_Windows.cpp`）下**全部生效**，导致两个后端行为不一致。

**验收标准**（C-6 原文）：打开对话框时**初始目录**与**默认扩展名**正确。

**性质**：SDL 后端功能缺失（不是 SDL 的 Bug），需在 duilib 侧补齐；同时要处理 SDL3 的能力限制。

## 现状与 SDL3 能力边界（已逐条对照源码确认）
| 参数 | Windows 原生后端（`FileDialog_Windows.cpp`） | SDL 后端（改动前） | SDL3 是否有对应接口 |
|---|---|---|---|
| `defaultLocation`（初始目录） | `IFileDialog::SetDefaultFolder`（L28-L37 / L72-L81 / L164-L175 / L283-L...) | **未生效**（`BrowseForFile`/`BrowseForFiles` 中形参被注释） | **有**：`SDL_Show*Dialog` 的 `default_location` 参数 |
| `fileName`（初始文件名） | `SetFileName`（L179-L181） | 被错当成 `default_location`（`FileDialog_SDL.cpp#L179-L186`），仅"目录+文件名"中的文件名部分可用 | **间接支持**：SDL 的 `default_location` 会按最后一个 `/` 或 `\` 拆成目录+文件名（见下） |
| `defaultExt`（默认扩展名） | `SetDefaultExtension`（L184-L186） | **未生效** | **无**：SDL 的 Windows 实现未调用 `IFileDialog::SetDefaultExtension`（其自绘 vtable 内有该入口但从未使用） |
| `nFileTypeIndex`（预选筛选器） | `SetFileTypeIndex(nFileTypeIndex + 1)`（L170-L172，下标从 1 开始） | **未生效** | **无**：SDL 头文件与实现均无"预选筛选器索引"入口 |

**SDL3 `default_location` 的真实语义**（[SDL_windowsdialog.c#L538-L562](file:///c:/develop/SDL/src/dialog/windows/SDL_windowsdialog.c#L538-L562)）：
- 按**最后一个** `/` 或 `\` 拆分：分隔符前 → `SetFolder`（目录），分隔符后 → `SetFileName`（文件名）；
- 若字符串**不含**分隔符 → 整串作为**文件名**（`SetFileName`），目录不变；
- 若字符串**以**分隔符**结尾** → 只有目录，不设置文件名。
→ 结论：duilib 侧要把"默认目录 + 默认文件名"**拼成一个路径**传给 `default_location`；纯目录必须**以分隔符结尾**，否则会被 SDL 当成文件名。

**SDL 版本**：本机 `c:\develop\SDL` 为 **3.5.0**（`SDL_version.h#L47-L65`）。多选路径已改用 `IFileOpenDialog::GetResults`（[SDL_windowsdialog.c#L649-L676](file:///c:/develop/SDL/src/dialog/windows/SDL_windowsdialog.c#L649-L676)），返回 **NULL 结尾的完整路径数组**，即 `FileDialog_SDL.cpp#L329` 的 `//TODO: 多选的情况下，SDL 3.0目前返回的值不对` 所描述的缺陷在当前版本**已不存在**。

## What Changes

### 1) `duilib/Utils/FileDialog_SDL.cpp`——新增 3 个文件内静态辅助函数
在 `DialogFileCallback` 之后新增（仅供本文件使用）：

```cpp
/** 文件类型筛选器(转换为SDL格式后使用) */
struct DlgFileFilter
{
    std::vector<DStringA> m_names;   /** 筛选器显示名称(UTF8)，保持生命周期供 m_filters 引用 */
    std::vector<DStringA> m_exts;    /** 筛选器扩展名(UTF8)，保持生命周期供 m_filters 引用 */
    std::vector<SDL_DialogFileFilter> m_filters; /** SDL格式的筛选器列表 */
};

/** 将duilib的文件类型筛选器转换为SDL格式（原 BrowseForFile/BrowseForFiles 内重复的代码合并到此） */
static void ConvertFileTypes(const std::vector<FileDialog::FileType>& fileTypes, DlgFileFilter& dlgFileFilter);

/** 解析默认扩展名："*.txt"->"txt"；"doc;docx"->"doc"；".txt"->"txt"；"*.*"->""（无效，返回空） */
static DStringA GetDefaultExtension(const DString& defaultExt);

/** 组合SDL的 default_location：默认目录(确保以分隔符结尾) + 默认文件名(可为空) */
static DStringA GetDefaultLocation(const FilePath& defaultLocation, const DStringA& defaultFileName);
```

### 2) `BrowseForFile`（LX159-L247）——三个参数全部生效
- 形参恢复命名（去掉 `/*...*/`）；
- `defaultExt`：若为空且 `nFileTypeIndex` 合法，则取 `fileTypes[nFileTypeIndex].szExt` 作为默认扩展名（对齐 Windows"选中的文件类型决定默认扩展名"的语义，因为 SDL 无法预选筛选器）；
- `fileName`：若**没有扩展名**则追加 `"." + defaultExt`，使对话框文件名字段显示带默认扩展名的名称；
- `defaultLocation`：与 `fileName` 一起组成 `default_location`（目录部分补齐结尾分隔符）；
- **保存对话框返回值校正**：用户若输入了无扩展名的文件名，SDL 不会自动补扩展名（未调用 `SetDefaultExtension`），因此在返回结果上追加默认扩展名（与 Windows 行为等价）；
- 筛选器构造改用 `ConvertFileTypes`（行为不变）。

### 3) `BrowseForFiles`（LX249-L332）
- 形参恢复命名；`defaultLocation` 生效（仅目录，`default_location` 以分隔符结尾）；
- `nFileTypeIndex` / `defaultExt`：**SDL 无对应接口**（无法预选筛选器、无默认扩展名设置），明确以 `(void)` 标注并在注释中说明原因，保留参数以维持与 Windows 平台一致的接口；
- 删除恒为空字符串的 `defaultFileName` 死代码；筛选器构造改用 `ConvertFileTypes`；
- `L329` 的过期 TODO 替换为与实际 SDL 实现相符的注释。

### 4) `BrowseForFolders`（LX142）——修正明显的拷贝错误
`bool allow_many = false;` → `true`（Windows 原生后端使用 `FOS_ALLOWMULTISELECT`，见 [FileDialog_Windows.cpp#L68](file:///c:/develop/nim_duilib/duilib/Utils/FileDialog_Windows.cpp#L68)），否则"多选目录"只能选一个。

### 5) `BrowseForFolder` / `BrowseForFolders`——`default_location` 规范化
改用 `GetDefaultLocation(defaultLocation, DStringA())`，确保目录以分隔符结尾（否则 SDL 会把目录名当成文件名调用 `SetFileName`）；与 `BrowseForFile` 保持一致。

### 6) 调用侧启用（让参数在真实场景中可用，也是验收的观测点）
改动前**全仓库没有任何调用方传入 `defaultLocation`**，参数即便补齐也无人使用，验收无法观测：
- `duilib/Control/PropertyGrid.cpp` `PropertyGridFileProperty::OnBrowseButtonClicked`（LX3501-L3511）：以**当前属性值**为基准，传入 `fileName = 当前文件名`、`defaultLocation = 当前文件所在目录`；
- `examples/RichEdit/MainForm.cpp` `OnOpenFile`（LX1149-L1151）与 `OnSaveAsFile`（LX1192-L1194）：传入 `defaultLocation = m_filePath.GetParentPath()`（其余参数不变）。
> 两处改动对 Windows 原生后端同样是行为改进（此前 `SetFileName` / `SetDefaultFolder` 无调用方），非破坏性。

## ADDED Requirements

### Requirement: SDL 后端下文件对话框的初始目录与默认文件名/扩展名 SHALL 与传参一致
`FileDialog::BrowseForFile` / `BrowseForFiles` / `BrowseForFolder` / `BrowseForFolders` 在 SDL 后端下，SHALL 使用调用方传入的 `defaultLocation` 作为对话框的初始目录，并在有默认文件名/默认扩展名时**预填带默认扩展名的文件名**；保存对话框返回的路径若无扩展名，SHALL 追加默认扩展名。

#### Scenario: 指定初始目录（BrowseForFile）
- **WHEN** 调用 `BrowseForFile(..., defaultLocation = "C:\Test")`
- **THEN** 对话框在该目录打开（而非上次目录），文件名字段为空。

#### Scenario: 初始目录 + 默认文件名（BrowseForFile）
- **WHEN** 调用 `BrowseForFile(..., fileName = "Test.txt", defaultLocation = "C:\Test")`
- **THEN** 对话框在 `C:\Test` 打开，文件名字段预填 `Test.txt`。

#### Scenario: 默认扩展名（保存对话框）
- **WHEN** 调用 `BrowseForFile(pWindow, filePath, /*bOpenFileDialog*/false, fileTypes, 0, /*defaultExt*/_T("txt"), /*fileName*/_T("abc"))`
- **THEN** 文件名字段预填 `abc.txt`；若用户不改文件名直接保存，返回路径以 `.txt` 结尾。

#### Scenario: 保存对话框返回路径的扩展名校正
- **WHEN** 用户在保存对话框中把文件名改为无扩展名的 `abc` 并确认（`defaultExt = "txt"`）
- **THEN** 返回的 `filePath` 为 `...\abc.txt`。

#### Scenario: nFileTypeIndex 作为默认扩展名（SDL 无法预选筛选器）
- **WHEN** `defaultExt` 为空、`nFileTypeIndex = 1`、`fileTypes[1].szExt = "*.txt"`
- **THEN** 默认扩展名取 `txt`（保存对话框预填/校正 `.txt`）；筛选器下拉的**预选索引**在 SDL 后端无法实现（平台限制，见"非目标"）。

#### Scenario: 多选目录（BrowseForFolders）
- **WHEN** 调用 `BrowseForFolders` 并选择多个目录
- **THEN** 返回全部所选目录（SDL 不再因 `allow_many=false` 而退化为单选）。

#### Scenario: 非 Windows 的 SDL 平台行为不变
- **WHEN** 在 Linux / macOS / FreeBSD 的 SDL 后端下使用文件对话框
- **THEN** 仅"默认位置组合 + 筛选器转换"逻辑生效（均为跨平台 API），无平台相关分支变化。

#### Scenario: 原生 Windows 后端行为不回退
- **WHEN** `SDLEnabled=0` 编译运行
- **THEN** `FileDialog_Windows.cpp` 不参与本次改动；调用侧新增传参（PropertyGrid/RichEdit）使初始目录/文件名比改动前更准确，无回退。

## 平台能力差异（已知限制，写进代码注释）
- **无法预选筛选器索引**：SDL3 无该接口，`nFileTypeIndex` 仅用于推导默认扩展名。
- **无法直接设置默认扩展名**：SDL3 未调用 `IFileDialog::SetDefaultExtension`，duilib 通过"预填文件名 + 返回值校正"等价实现。
- **多选 TODO 已核对**：当前 SDL 3.5.0 用 `IFileOpenDialog::GetResults` 返回 NULL 结尾的完整列表，原 TODO 描述的问题已不存在（仍需人工核对一次运行结果）。
- **`defaultLocation` 指向不存在的目录时的上游行为**：SDL 的 Windows 实现用 `SHCreateItemFromParsingName` 创建目录项，失败即放弃"现代样式对话框"并**回退到旧版 COM32 对话框**（[SDL_windowsdialog.c#L601-L603](file:///c:/develop/SDL/src/dialog/windows/SDL_windowsdialog.c#L601-L603) + [L781-L783](file:///c:/develop/SDL/src/dialog/windows/SDL_windowsdialog.c#L781-L783)）；该回退路径同样会回调、**不会造成 duilib 消息循环卡死**（且 T2.6 的返回值扩展名校正对其同样有效）。Windows 原生后端在同类情况下是"忽略该目录、仍显示现代对话框"，属两后端的差异，本次不做额外防御代码。
- **旧版回退路径不支持多选目录**：SDL 的 `SHBrowseForFolderW` 回退分支忽略 `allow_many`（[L1088-L1114](file:///c:/develop/SDL/src/dialog/windows/SDL_windowsdialog.c#L1088-L1114)），仅在无法使用现代对话框的老系统上出现。

## Impact
- Affected specs：`docs/misc/Development-Plan.md` C-6（本项）、A-3（SDL 版本锁定——本项**不依赖**，无需等 A-3）。
- Affected code：`duilib/Utils/FileDialog_SDL.cpp`（主）、`duilib/Control/PropertyGrid.cpp`、`examples/RichEdit/MainForm.cpp`。
- **非 BREAKING**：公开 API 签名不变（仅恢复形参名，`FileDialog.h` 无需修改）、无资源/主题/XML 变化。
- **不影响**：`FileDialog_Windows.cpp`（Windows 原生后端）、SDL 源码（不改动）。

## 改动文件清单
1. `duilib/Utils/FileDialog_SDL.cpp`
   - 新增 `DlgFileFilter` / `ConvertFileTypes` / `GetDefaultExtension` / `GetDefaultLocation`；
   - `BrowseForFile`（LX159-L247）：恢复 3 个形参名 + 默认扩展名/默认位置/返回值校正；
   - `BrowseForFiles`（LX249-L332）：恢复形参名 + `defaultLocation` 生效 + 删死代码 + 更新 TODO 注释；
   - `BrowseForFolders`（LX142）：`allow_many` → `true`；
   - `BrowseForFolder`/`BrowseForFolders`：`default_location` 规范化（目录以分隔符结尾）。
2. `duilib/Control/PropertyGrid.cpp`：`PropertyGridFileProperty::OnBrowseButtonClicked`（LX3501-L3511）——以当前属性值推导 `fileName` / `defaultLocation`。
3. `examples/RichEdit/MainForm.cpp`：`OnOpenFile`（LX1151）、`OnSaveAsFile`（LX1194）——传入 `defaultLocation = m_filePath.GetParentPath()`。

## 验证方式
- **编译**：`C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe c:\develop\nim_duilib\build\examples.sln /p:Configuration=Debug /p:Platform=x64`（增量），要求 `duilib` **0 error / 0 warning**。**只验 Debug x64，不编译 Release**（用户约束）。
- **运行时（人工手动核对，用户执行；原生文件对话框无法自动化）**：
  1. `render_d.exe` → PropertyGrid 分组八「文件路径」：
     - 保存项（初值 `C:\Test-Save.txt`，`defaultExt="txt"`）：初始目录 = `C:\`✓（`defaultLocation` 生效）；文件名字段预填 `Test-Save.txt`✓；改名 `abc` 后确认，返回值应为 `C:\abc.txt`✓；
     - 打开项（初值 `C:\Test-Open.txt`）：初始目录 = `C:\`✓，文件名字段预填 `Test-Open.txt`✓；
     - 文件夹属性（初值 `C:\Test\`）：初始目录 = `C:\Test`✓。
  2. `RichEdit_d.exe` → 打开/另存为：初始目录 = 当前文件所在目录✓；文件名字段带扩展名✓。
  3. 多选（`BrowseForFiles` / `BrowseForFolders` 当前**无示例调用点**）：如需核对，由助手临时加入调用点运行后删除，或由用户自行调用验证；核对项：可选多个条目、返回全部路径。
- **不做的验证**：不编译 Release、不代提交代码、不代跑 GUI 交互。

## 非目标（Out of Scope）
- **不修改 SDL 源码**（不补 `SetDefaultExtension` / `SetFileTypeIndex`）；A-3（SDL 版本锁定）不启动。
- **不修改 `FileDialog.h` / `FileDialog_Windows.cpp`**（签名与 Windows 行为保持原样）。
- **不追求筛选器下拉的预选索引**（SDL 无接口，属平台能力差异，仅以注释与文档说明）。
- **不重构** `MessageLoop_SDL` / 回调线程同步机制（`DialogFileCallbackData` 的 AddRef/Release 逻辑不动）。
- **不做自动化 GUI 测试**（原生对话框无法自动化）。
