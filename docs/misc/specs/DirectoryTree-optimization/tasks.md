# DirectoryTree 跨平台整体优化 · 任务清单

> 所有任务均在 Windows 下构建并以 `bin\TreeView.exe` 回归；Unix 代码以编译期语法核对为主（无实机）。

## T1 公开结构统一（DirectoryTree.h）

- [x] 将 `DeviceType` 枚举移出 `#ifdef DUILIB_BUILD_FOR_WIN`，全平台可见。
- [x] 合并 `DiskInfo` 为单一结构，字段见 spec 3.1。
- [x] 新增私有成员 `std::unordered_map<DString, FolderStatus*> m_folderIndex`。
- [x] 新增私有函数声明：`NormalizePathKey`、`NotifyFinish`、`FindBaseNodeInChain`。

## T2 平台接口调整（DirectoryTreeImpl.h）

- [x] `GetFolderContents` 增加 `bool bShowHiddenFiles, bool bShowSystemFiles` 参数。
- [x] `NeedShowDirPath` 增加同名参数。
- [x] 注释说明“工作线程调用，禁止访问控件成员”。

## T3 Windows 实现修复与缓存（DirectoryTreeImpl_Windows.cpp）

- [x] `GetVirtualDirectoryInfo`：`FOLDERID_Profile`、动态路径、`SHGetFileInfoW`、配平 `DestroyIcon`。
- [x] `GetRootPathInfoList`：补充 `DRIVE_REMOTE`/`DRIVE_CDROM`，配平 `DestroyIcon`。
- [x] 新增扩展名图标缓存与共享文件夹图标缓存；树枚举不再逐目录调用 `SHGetFileInfoW`。
- [x] `GetFolderContents`：修正隐藏/系统过滤、参数化开关、过滤 reparse point、名称排序。
- [x] `GetMyComputerIconID`：`pidl` 初始化、`ILFree` 保护、`DestroyIcon`、结果缓存共享。
- [x] `GetDiskInfoList`：修复失败分支图标泄漏，填充 `DeviceType`/`m_volumeType`/`m_mountOn`。
- [x] `NeedShowDirPath`：修正盘符根目录判断、参数化开关。

## T4 POSIX 实现合并（DirectoryTreeImpl_Unix.cpp）

- [x] 新建文件，编译条件覆盖 LINUX/FREEBSD/MACOS，整合原 Linux 实现。
- [x] 挂载表：macOS/FreeBSD 用 `getmntinfo`，Linux 用 `/proc/mounts` + octal 转义 + `uint64_t` 容量。
- [x] 目录枚举：`directory_iterator(nativePath, ec)`、逐项 `error_code`、单条失败跳过、名称排序。
- [x] 修复 `NeedShowDirPath` 隐藏判断（`GetFileName()`）、`GetBaseDevice` NVMe、`GetDiskInfoList` 的 `weakFlag` 取消。
- [x] 虚拟目录七类目录合并为表驱动/循环实现，图标走共享缓存。
- [x] 删除 `DirectoryTreeImpl_Linux.cpp`、`DirectoryTreeImpl_MacOS.cpp`。
- [x] 更新 `duilib.vcxproj` 与 `.filters`。

## T5 控件层线程安全与索引（DirectoryTree.cpp）

- [x] `RefreshNodeData` 增加 `m_bMyComputer`/`m_bShowHiddenFiles`/`m_bShowSystemFiles`；`GetTreeNodeData` 填充。
- [x] `RefreshPathInfo` 去除所有 `TreeNode` / `GetFolderData` / `IsMyComputerNode` 调用，只使用快照。
- [x] 所有 `GetFolderContents` / `NeedShowDirPath` 调用传入 UI 线程快照的隐藏/系统开关。
- [x] 实现 `NormalizePathKey`，维护 `m_folderIndex`（插入、销毁、清空）。
- [x] `GetFolderData(FilePath)` 改索引查找；`IsPathSame` 与 `IsSamePath` 规则统一。

## T6 节点去重与选择逻辑

- [x] `InsertTreeNode` 增加同路径子节点去重；重复时释放图标并返回既有节点。
- [x] `SelectPath` / `SelectSubPath` 路径链补入目标自身，修复“选中父目录”的 off-by-one。
- [x] 用索引顺序查找最深已存在节点（`FindBaseNodeInChain`），替换 `m_folderMap` 任意顺序覆盖。
- [x] `OnSelectSubPath` 所有提前返回均调用完成回调；新增 `NotifyFinish`。

## T7 刷新同步与加载状态

- [x] “计算机”节点刷新同时处理磁盘新增与移除。
- [x] `OnTreeNodeExpand` 失败时回滚 `m_bContentLoaded`。
- [x] `OnTreeNodeClick` 对同一节点重复点击跳过重复枚举。
- [x] `SetShowHidenFiles` / `SetShowSystemFiles` 的刷新合并（`m_bRefreshing`/`m_bNeedRefresh`）。
- [x] 刷新结果未使用的图标统一释放。

## T8 TreeView 示例适配

- [x] `ComputerView.cpp/.h`：`#elif LINUX` 改为 `#else`，`_Linux` 重命名为 `_Unix`。
- [x] 使用统一后的 `DiskInfo` 字段（`IsDiskInfoListEqual` 统一比较 `m_volumeType`/`m_deviceType`/`m_mountOn`）。

## T9 构建与实机回归

- [x] 构建 `duilib` + `TreeView`（Windows x64，Release，无错误/警告）。
- [x] 按 spec 第 9 节执行 Windows 交互回归（详见 checklist“实机”章节，UI 自动化逐项验证）。
  - 已覆盖：首屏计算机视图、逐级展开无重复、节点点击刷新、地址栏多级/隐藏链导航、无效路径、新建/删除后刷新、隐藏/系统开关。
  - 未覆盖：驱动器移除（需物理卸载）、网络盘（本机无映射）、重复点击去重（外部不可观测）。
- [ ] 长时间浏览后检查 GDI/USER 对象数量稳定（已做 8 轮往返，GDI/USER 恒定；5 分钟连续观察未执行）。

## T10 收尾

- [x] Grep 核对：无 `FOLDERID_UserProfiles`、无遗留 `DirectoryTreeImpl_Linux/MacOS` 引用。
- [x] 更新 `docs/misc/Project-Audit.md` 中目录树实现表格（合并为 Windows / Unix）。
- [x] 汇总变更说明，等待用户确认提交。

## 依赖顺序

T1 → T2 → T3/T4（可并行）→ T5 → T6 → T7 → T8 → T9 → T10。

## 回滚点

- T3/T4 完成后先编译一次 `duilib`，确保平台层可用。
- T5~T7 每完成一项编译一次 `duilib`，避免问题累积。
- T8 完成后再编译 `TreeView`。
