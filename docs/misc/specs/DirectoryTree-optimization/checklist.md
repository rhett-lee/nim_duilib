# DirectoryTree 跨平台整体优化 · 验收清单

## 代码

- [x] `DeviceType` 在所有平台可见，`DiskInfo` 为单一结构。
- [x] 不存在 `FOLDERID_UserProfiles` 引用。
- [x] 所有 `AddIcon(HICON)` 调用点都配对 `DestroyIcon`。
- [x] `GetLogicalDriveInfo` 失败分支不再泄漏图标。
- [x] Windows 隐藏/系统过滤条件与开关语义一致。
- [x] `NeedShowDirPath` 盘符根目录判断使用 `&&` 并覆盖小写盘符。
- [x] 树枚举不再逐子目录调用 `SHGetFileInfoW`。
- [x] 文件图标按扩展名缓存且内存有界。
- [x] `RefreshPathInfo` 内无 `TreeNode` / `GetFolderData` / `IsMyComputerNode` 调用。
- [x] `m_folderIndex` 在插入、销毁、析构三处同步维护。
- [x] `InsertTreeNode` 对同路径子节点去重。
- [x] `SelectPath` / `SelectSubPath` 选中目标目录本身。
- [x] 所有异步路径失败时都调用 `finishCallback`。
- [x] 磁盘新增与移除都会同步到“计算机”节点。
- [x] `DirectoryTreeImpl_Linux.cpp` / `_MacOS.cpp` 已删除，构建文件已更新。
- [x] macOS 挂载表使用 `getmntinfo`，不读 `/proc/mounts`。
- [x] Unix 隐藏目录判断基于文件名而非绝对路径首字符。
- [x] NVMe 设备基名解析正确（`nvme0n1p1 → nvme0n1`）。
- [x] 挂载表中的 `\040` 等转义可正确还原。
- [x] Unix 目录枚举单条失败不中断整个目录。

## 构建

- [x] `cmake --build out\x64\Release --target duilib` 成功。
- [x] `cmake --build out\x64\Release --target TreeView` 成功。
- [x] 无新增编译警告（构建输出无 `warning C`）。

## 实机（Windows，2026-11-05 交互验证）

- [x] 首屏“计算机”视图显示本地/可移动/光驱/网络盘。
  - 实测：本地磁盘(C:)、DVD 驱动器(D:)、新加卷(E:) 均显示且带图标；本机无网络映射盘，`DRIVE_REMOTE` 分支未覆盖。
- [x] 树逐级展开无重复节点，顺序稳定。
  - 实测：`C:\Users\lee\AppData\Local\Temp\dtree_test`、`E:\develop\nim_duilib\duilib`、`E:\develop`、`E:\develop\newchild6` 均无重复节点。
- [x] 点击目录节点右侧列表正确刷新。
  - 实测：点击 `Documents` 节点后地址栏、右侧列表、状态栏同步更新。
- [ ] 重复点击同一节点不重复枚举。
  - 仅代码层确认（`OnTreeNodeClick` 使用 `m_pLastLoadedTreeNode` 去重），枚举次数在 UI 外部不可观测。
- [x] 地址栏输入多级绝对路径后左树选中目标目录。
  - 实测：多级路径逐级展开并选中目标；隐藏目录链（`AppData`）同样可导航，右侧仅显示符合显示设置的项。
- [x] 无效路径也能结束“正在加载”状态。
  - 实测：输入 `C:\nonexist_xyz_123` 弹出“路径不存在”提示，确认后恢复原路径，状态栏无残留“正在加载”，界面未卡死。
- [x] 新建/删除子目录后刷新同步。
  - 实测：新建 `new_after` 后点击刷新出现；删除后刷新消失。
- [ ] 移除驱动器后对应节点消失。
  - 未验证：需要物理卸载/弹出介质，本机不具备条件。
- [x] “显示隐藏文件”“显示系统文件”单独切换均生效。
  - 实测：隐藏开关关闭时仅显示 `normal`，打开后出现 `hidden_dir`、`hidden_file.txt`；系统开关勾选后出现 `system_dir`，两者互不影响。
- [ ] 反复浏览大目录 5 分钟，GDI/USER 对象数量稳定。
  - 部分验证：8 轮往返导航（含隐藏链目录与大目录）后 `GDI=65~70 / USER=24~25` 保持恒定，未做 5 分钟连续观察。

## 实机验证发现的问题（已修复）

- [x] `OnSelectSubPath` 冗余目录过滤误删基准节点自身路径，导致“父节点已加载内容后新建的子目录”被挂到错误的父节点。
  - 复现：`E:\develop` 已加载后新建 `E:\develop\newchild6\normal`，从右侧列表进入 `newchild6`，`normal` 被错挂到 `develop` 同级。
  - 修复：保留路径链首项（基准节点路径）作为 `OnShowSubFoldersEx` 的挂载锚点。
  - 回归：`normal` 正确位于 `newchild6` 下一级，`develop` 同级不再出现多余节点。
- [x] `GetShellIconID` 缺少 `SHGFI_ICON` 标志，`SHGetFileInfoW` 不返回 `hIcon`，导致文件夹/文件图标全部缺失（盘符图标因另走一条带该标志的调用而正常）。
  - 修复：在 `GetShellIconID` 的 flags 中补上 `SHGFI_ICON`。
  - 回归：树节点的文件夹图标、右侧大图标视图的文件夹/文件图标均恢复显示。

## 文档

- [x] `docs/misc/Project-Audit.md` 目录树实现表格已更新为 Windows / Unix。
- [x] 变更说明已汇总，等待提交确认。
