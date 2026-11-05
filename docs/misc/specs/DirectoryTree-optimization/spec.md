# DirectoryTree 跨平台整体优化 · 规格说明

## 1. 背景

`DirectoryTree` 是 `TreeView` 示例左侧的目录树控件，提供目录枚举、树节点展开、路径定位、刷新同步和“计算机”磁盘视图。当前实现由三部分组成：

| 层 | 文件 | 职责 |
|---|---|---|
| 控件层 | `duilib/Control/DirectoryTree.h/.cpp` | 树节点管理、异步枚举调度、刷新与选择 |
| 平台接口层 | `duilib/Control/DirectoryTreeImpl.h` | 平台无关的枚举接口声明 |
| 平台实现层 | `duilib/Control/DirectoryTreeImpl_Windows.cpp` | Windows（Shell API） |
| 平台实现层 | `duilib/Control/DirectoryTreeImpl_Linux.cpp` / `_MacOS.cpp` | POSIX（两份近乎重复） |

已确认的问题包括：Windows 文件图标共享标志写反导致图标永不释放、隐藏/系统文件过滤条件对调、`kUserHome` 取到 `C:\Users` 而非当前用户目录、HICON/pidl 泄漏、工作线程直接访问 `m_folderMap` 与 `TreeNode` 造成数据竞争、macOS 读取不存在的 `/proc/mounts`、Unix 隐藏目录判断失效、NVMe 设备识别错误、挂载表解析截断、异步失败路径不触发完成回调、磁盘移除不同步、节点可能重复插入、路径比较规则不一致，以及树枚举时对每个子目录做一次多余的 `SHGetFileInfoW`。

## 2. 目标与非目标

### 目标

1. 修复全部正确性、崩溃与资源泄漏问题。
2. 在不改变 XML 属性名、控件类型名的前提下，把 `DiskInfo` 收敛为跨平台统一结构。
3. 合并 Mac/Linux 实现为一份 POSIX 实现，平台差异只保留在挂载表与设备类型识别处。
4. 工作线程不再访问任何 UI 对象或 UI 数据结构。
5. 用路径索引替代线性扫描，用共享图标/扩展名缓存降低图标开销。
6. 保持 TreeView 示例在 Windows 上的现有交互与视觉，并让 macOS 具备可用的“计算机”视图逻辑。

### 非目标

- 不把 `ShowAllDiskNodes` / `ShowVirtualDirectoryNode` 改为异步（会改变公开 API）。
- 不新增排序 UI、权限展示、缩略图、符号链接跟随开关。
- 不修改 `DirectoryWatcher`、地址栏、文件列表视图的实现。
- 不调整 XML 资源、主题、图标 SVG 文件。

## 3. 公开 API 设计（`DirectoryTree.h`）

### 3.1 DiskInfo 统一

把当前 `#ifdef DUILIB_BUILD_FOR_WIN` 分支下的 `DeviceType` 枚举移到公共区域（所有平台可见），取值不变：

```cpp
enum class DeviceType {
    UNKNOWN, HDD, SSD, NVME, USB, SD_CARD, CDROM,
    LOOP, VIRT_DISK, RAMDISK, NFS, SHARE
};
```

`DiskInfo` 收敛为单一结构（字段为各平台现有字段的超集）：

| 字段 | 类型 | Windows | 其他平台 |
|---|---|---|---|
| `m_displayName` | DString | 填充 | 填充 |
| `m_nIconID` / `m_bIconShared` | uint32_t / bool | 填充 | 填充 |
| `m_filePath` | FilePath | 驱动器根路径 | 挂载点路径 |
| `m_volumeName` | DString | 卷标 | 设备名（如 `/dev/sda2`） |
| `m_volumeType` | DString | “本地磁盘”等文案 | 空 |
| `m_deviceType` | DeviceType | 按驱动类型映射 | 按设备识别 |
| `m_fileSystem` | DString | NTFS/FAT32 等 | ext3/apfs 等 |
| `m_mountOn` | DString | 驱动器根路径 | 挂载点 |
| `m_totalBytes` / `m_freeBytes` | uint64_t | 填充 | 填充 |

Windows 的 `GetDriveType` 映射：`DRIVE_FIXED→HDD`、`DRIVE_REMOVABLE→USB`、`DRIVE_CDROM→CDROM`、`DRIVE_REMOTE→SHARE`，其余 `UNKNOWN`。

### 3.2 不变部分

- `PathInfo` 字段保持不变。
- `ShowFolderContentsEvent` / `ShowMyComputerContentsEvent` 签名不变。
- `ClearPathInfoList` / `ClearDiskInfoList`、所有公开方法保持不变。
- XML 属性 `small_icon_size` / `large_icon_size` / `show_hiden_files` / `show_system_files` 保持不变。

### 3.3 图标所有权规则（全平台统一）

- `m_bIconShared == false`：图标由**调用方**通过 `AddIcon` 获得并负责 `RemoveIcon`，随所在容器清理。
- `m_bIconShared == true`：图标由 `DirectoryTreeImpl` 的共享图标缓存持有，只在该 impl 析构时释放；调用方不得释放。
- `IconManager::AddIcon(HICON)` 只拷贝像素、不接管句柄，所有调用点必须配对 `DestroyIcon`。

## 4. Windows 实现改造（`DirectoryTreeImpl_Windows.cpp`）

### 4.1 `GetVirtualDirectoryInfo`

- `kUserHome` 改用 `FOLDERID_Profile`（回退 `CSIDL_PROFILE`），不再使用 `FOLDERID_UserProfiles`。
- 删除 PIDL 分支：统一先 `SHGetKnownFolderPath`，失败回退 `SHGetSpecialFolderPathW`；用 `std::wstring` 存放路径，去掉 `MAX_PATH` 缓冲与 `wcscpy_s`。
- 检查 `SHGetKnownFolderPath` 的 `HRESULT`。
- 显示名/图标改用 `SHGetFileInfoW(path, 0, ..., SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON)`；`AddIcon` 后立即 `DestroyIcon`。
- 删除无效语句 `m_pTree->GetWindow()->NativeWnd()->GetHWND();`。

### 4.2 `GetRootPathInfoList`

- 保留 `DRIVE_FIXED` / `DRIVE_REMOVABLE`，新增 `DRIVE_REMOTE`、`DRIVE_CDROM`（与资源管理器一致）；继续跳过 `A:`/`B:`。
- `GetDiskInfoList` 中对每个驱动器再次调用 `GetDriveType` 并按 3.1 映射填充 `m_deviceType`（不扩展 `PathInfo`，避免公开结构变更）。
- 所有 `AddIcon` 后 `DestroyIcon`。

### 4.3 图标缓存

`TImpl` 新增：

- `m_nSmallFolderIconID` / `m_nLargeFolderIconID`（已有，保留）。
- `m_nSmallMyComputerIconID`（新增，共享）。
- `std::map<DString, CachedFileIcon>`，键为小写扩展名，`CachedFileIcon{uint32_t iconID; DString typeName;}`（共享）。

规则：

- 文件夹图标：只取一次，之后全部复用，**不再为每个子目录调用 `SHGetFileInfoW`**。
- 文件图标：按扩展名取一次并复用；`typeName` 同时缓存。
- 树枚举（`fileList == nullptr`）不取 `typeName`。
- 特殊文件（`.exe`/`.ico`/`.lnk`/`.url`/`.msi`）仍按扩展名缓存（接受“不回显文件内嵌图标”的取舍，保证内存有界）。

### 4.4 `GetFolderContents`

- 修正隐藏/系统过滤：`show_hiden_files` 判断 `FILE_ATTRIBUTE_HIDDEN`，`show_system_files` 判断 `FILE_ATTRIBUTE_SYSTEM`。
- 参数新增 `bool bShowHiddenFiles, bool bShowSystemFiles`，不再从 `m_pTree` 读取；`bLargeIcon` 保留。
- 过滤 `FILE_ATTRIBUTE_REPARSE_POINT` 目录（避免 junction 递归）。
- 输出按名称排序（目录列表、文件列表各自排序，忽略大小写）。
- 检查 `weakFlag` 逻辑保持不变。
- 修正文件分支 `m_bIconShared`：共享缓存命中时为 `true`，未命中时同样由缓存持有。

### 4.5 `GetDiskInfoList` / `GetMyComputerIconID` / `NeedShowDirPath`

- `GetMyComputerIcon_Windows`：`LPITEMIDLIST pidl = nullptr` 初始化，仅在非空时 `ILFree`；`GetMyComputerIconID` 中 `AddIcon` 后 `DestroyIcon`；结果缓存并标记共享。
- `GetDiskInfoList`：修复 `GetLogicalDriveInfo` 失败分支的图标泄漏（失败时 `RemoveIcon` 或统一由 `pathInfoList` 的清理路径释放）；填充 `DeviceType`、`m_mountOn`、`m_volumeType`。
- `NeedShowDirPath`：修正盘符根目录判断为 `(s[0] >= 'C' && s[0] <= 'Z')` 并处理小写盘符；去掉重复的 `size()==3` 判断；新增 `bShowHiddenFiles/bShowSystemFiles` 参数。

## 5. POSIX 实现合并（新增 `DirectoryTreeImpl_Unix.cpp`，删除 `_Linux.cpp` / `_MacOS.cpp`）

编译条件：`DUILIB_BUILD_FOR_LINUX || DUILIB_BUILD_FOR_FREEBSD || DUILIB_BUILD_FOR_MACOS`。

共享部分：`GetVirtualDirectoryInfo`、`GetFolderContents`、`NeedShowDirPath`、`GetMyComputerIconID`、图标字符串生成、`DeviceType` 识别、设备基名解析。

### 5.1 挂载表

- macOS：`getmntinfo(&mounts, MNT_NOWAIT)`；容量用 `statfs` 的 `f_blocks * f_bsize`、`f_bavail * f_bsize`（`uint64_t`）；设备类型由 `f_fstypename` 判断（`nfs`/`smbfs`/`afpfs`/`webdav`→网络，`cd9660`/`udf`→CDROM）。
- Linux：读取 `/proc/mounts`，新增 `UnescapeMountPath()` 处理 `\040`/`\011`/`\134`；`sscanf` 检查返回值为 3；容量使用 `uint64_t` 运算。
- FreeBSD：使用 `getmntinfo`。
- 所有分支在循环内检查 `weakFlag`，支持取消。
- 跳过 `proc/sysfs/tmpfs/devtmpfs` 等虚拟文件系统（各平台保留现有规则）。

### 5.2 目录枚举

- 使用 `std::filesystem::directory_iterator(stdPath, errorCode)`，打开失败直接返回，不抛异常。
- 每个条目用 `error_code` 重载判断 `is_directory` / `is_regular_file` / `file_size` / `last_write_time`；单条失败跳过，不中断整个枚举。
- 继续跳过 symlink（保持现状）。
- `stat` 仅用于把 `last_write_time` 转成 `FileTime`，失败时该字段留空。
- 输出排序规则与 Windows 一致。

### 5.3 修复项

- `NeedShowDirPath`：改用 `path.GetFileName()` 判断首字符 `.`；macOS 额外读取 `UF_HIDDEN`。
- `get_base_device`：正确处理 `nvme0n1p1→nvme0n1`、`mmcblk0p1→mmcblk0`、`sda1→sda`；仅当设备名以字母开头且末尾是数字时剥离分区号。
- `GetVirtualDirectoryInfo` 的七类目录统一实现，避免 7 段重复代码；图标使用共享缓存。
- `GetRootPathInfoList`：`/` 与各挂载点图标改为共享缓存，不再复制 `DiskInfo` 的图标所有权。
- 所有 `AddIcon(DString)` 产生的共享图标在 impl 析构时释放。

## 6. 控件层改造（`DirectoryTree.cpp` / `DirectoryTree.h` 私有部分）

### 6.1 线程安全

- `RefreshNodeData` 新增 `bool m_bMyComputer`（UI 线程计算）。
- `RefreshNodeData` 继续在 UI 线程快照 `m_dirPath`、`m_bContentLoaded`、`m_childPaths`、`m_weakFlag`、`m_pTreeNode`。
- `RefreshPathInfo`（工作线程）**只使用快照数据**，禁止调用 `IsMyComputerNode` / `GetFolderData` / 任何 `TreeNode` 方法。
- `GetFolderContents` / `NeedShowDirPath` 调用处传入 UI 线程的快照参数 `bShowHiddenFiles`、`bShowSystemFiles`。
- `GetTreeNodeData` 内补充记录 `m_bMyComputer = IsMyComputerNode(pTreeNode)`。

### 6.2 路径索引

- 新增 `std::unordered_map<UiString, FolderStatus*> m_folderIndex`。
- 新增私有 `UiString NormalizePathKey(const FilePath& path) const`：`NormalizeDirectoryPath` + `ToString` +（Windows/macOS）转小写；Linux/FreeBSD 保留原样。
- `InsertTreeNode` 插入索引；`OnTreeNodeDestroy` 与析构删除索引项。
- `GetFolderData(FilePath)` 改为索引 O(1) 查找；`IsSamePath` 保留用于既有比较。
- `IsPathSame` 改为复用 `NormalizePathKey` 比较，与 `IsSamePath` 规则一致。

### 6.3 节点去重

- `InsertTreeNode` 在 `AddChildNode` 前检查父节点现有子节点：若存在同规范化路径的目录节点，则释放传入图标并返回既有节点，不新建。
- `UpdateTreeNodeData` 与 `OnShowSubFoldersEx` 依赖该去重保证幂等。

### 6.4 路径选择

- `SelectPath(FilePath, StdClosure)`：
  1. 校验路径存在；失败调用完成回调。
  2. 索引命中直接选中并回调。
  3. 构造 `chain = 父路径列表 + 目标自身`，全部 `FormatPathAsDirectory`。
  4. 用索引自顶向下顺序查找最深的已存在节点 `baseNode`。
  5. 剩余路径 = `chain` 中 `baseNode` 之后的项，交给 `OnSelectSubPath`。
- `SelectSubPath(TreeNode*, FilePath, StdClosure)`：
  1. 校验 `pTreeNode`、`subPath` 存在及父子关系；失败回调。
  2. 构造同样的 `chain`，但只在 `pTreeNode` 子树范围内选择最深已存在节点，基准节点优先使用 `pTreeNode`。
  3. 交给 `OnSelectSubPath`。
- `OnSelectSubPath`：
  - 过滤规则保留（已加载节点移除、非子目录移除），增加“必须保留目标节点”的保证。
  - 所有提前返回路径调用完成回调。
- 新增私有 `void NotifyFinish(StdClosure& finishCallback)`（或按值传参），统一结束通知。

### 6.5 刷新

- “计算机”节点刷新：使用 `m_childPaths` 与新磁盘列表求差集，**新增与移除同时处理**（移除项从 `m_newFolderList` 移除并标记删除）。
- 普通节点刷新：沿用现有逻辑，路径比较改用 `NormalizePathKey`。
- 新增目录插入前走 `InsertTreeNode` 去重。
- 刷新结果落地后，`m_newFolderList` 中未使用的图标统一释放。

### 6.6 加载状态与重复点击

- `OnTreeNodeExpand` 在发起异步枚举前标记 `m_bContentLoaded = true`；`OnShowSubFolders` 失败（父节点无效）时回滚为 `false`。
- `OnTreeNodeClick` 记录 `m_lastLoadedTreeNode` 与规范化路径；同一节点同路径重复点击时跳过重复枚举。
- `SetShowHidenFiles` / `SetShowSystemFiles` 触发的刷新做合并：若已有刷新请求在途，只置 `m_bNeedRefresh = true`，待刷新完成后补一次。

## 7. 示例适配（`examples/TreeView`）

- `ComputerView.cpp`：把 `#elif defined (DUILIB_BUILD_FOR_LINUX)` 改为 `#else`，覆盖 macOS/FreeBSD；平台无差异的分支合并。
- `ShowMyComputerContents_Win`：继续使用 `m_volumeType`（Windows 填入）。
- `ShowMyComputerContents_Linux` 重命名为 `ShowMyComputerContents_Unix`：按 `DeviceType` + `mountOn` 展示（macOS 复用同一逻辑）。
- `ComputerView.h` 同步调整函数声明与 `#if`。
- `InitComputerViewHeader_Linux` 合并为 `InitComputerViewHeader_Unix`，macOS 使用同一表头。
- 其他使用 `PathInfo` / `DiskInfo` 的文件不改（字段兼容）。
- 删除示例中已无用的 `m_volumeType` 分支判断（若存在 `#if` 包裹）。

## 8. 构建配置

- 删除 `duilib/Control/DirectoryTreeImpl_Linux.cpp`、`DirectoryTreeImpl_MacOS.cpp`。
- 更新 `duilib/duilib.vcxproj` 与 `duilib/duilib.vcxproj.filters`：移除两份旧文件，加入 `Control\DirectoryTreeImpl_Unix.cpp`。
- CMake 通过 `aux_source_directory` 自动收集，无需改动；确认 `duilib` 的 CMake 源文件收集方式覆盖 `Control` 目录。

## 9. 验证

### 构建

```
Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -DevCmdArguments "-arch=amd64 -host_arch=x64"
cmake --build out\x64\Release --target duilib TreeView
```

### Windows 实机回归（`bin\TreeView.exe`）

1. 首屏“计算机”视图显示全部驱动器（含可移动/光驱/网络盘），图标正常。
2. 左侧树逐级展开，子目录不重复、顺序稳定。
3. 点击目录节点，右侧列表正确刷新；重复点击同一节点不重复枚举。
4. 地址栏输入多级路径，左树正确选中**目标目录**（不是其父目录）。
5. 状态栏“正在加载”在成功后结束；无效路径也能结束。
6. 刷新按钮：新建/删除子目录后树同步；模拟移除 U 盘后对应节点消失。
7. 勾选“显示隐藏文件”/“显示系统文件”单独生效。
8. 反复浏览大目录 5 分钟，GDI/USER 对象数量稳定（任务管理器或 Process Explorer）。

### 静态检查

- 工程中不存在对 `FOLDERID_UserProfiles` 的引用。
- 所有 `AddIcon(HICON)` 调用点均有 `DestroyIcon`（可用 Grep 核对）。
- `RefreshPathInfo` 内不出现 `TreeNode` / `GetFolderData` / `IsMyComputerNode`。
- `DirectoryTreeImpl_Unix.cpp` 中不存在 `/proc/mounts` 被 macOS 分支调用。

## 10. 文件变更清单

| 文件 | 操作 |
|---|---|
| `duilib/Control/DirectoryTree.h` | 统一 `DeviceType` / `DiskInfo`；新增私有索引与辅助函数声明 |
| `duilib/Control/DirectoryTree.cpp` | 线程安全、路径索引、去重、选择逻辑、刷新同步、回调兜底 |
| `duilib/Control/DirectoryTreeImpl.h` | `GetFolderContents` / `NeedShowDirPath` 新增隐藏/系统参数 |
| `duilib/Control/DirectoryTreeImpl_Windows.cpp` | 按第 4 节重写 |
| `duilib/Control/DirectoryTreeImpl_Unix.cpp` | 新增，合并 Mac/Linux |
| `duilib/Control/DirectoryTreeImpl_Linux.cpp` | 删除 |
| `duilib/Control/DirectoryTreeImpl_MacOS.cpp` | 删除 |
| `duilib/duilib.vcxproj` / `.filters` | 更新源文件列表 |
| `examples/TreeView/ComputerView.cpp/.h` | 平台分支与统一字段适配 |

