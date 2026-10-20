# 文件对话框参数补齐（SDL 后端）Checklist（C-6）

## 代码
- [x] `FileDialog_SDL.cpp` 中不再存在 `/*nFileTypeIndex*/`、`/*defaultExt*/`、`/*defaultLocation*/` 形式的被注释形参（已 grep 核对，0 处）
- [x] `BrowseForFile` 的 `defaultLocation` 用于 `default_location`（不再用 `fileName` 顶替）（LX297）
- [x] `fileName` 与 `defaultLocation` 组合为 `default_location`，目录部分以分隔符结尾（`GetDefaultLocation`，LX165-L179）
- [x] `defaultExt` 为空时可用 `nFileTypeIndex` 推导（`fileTypes[nFileTypeIndex].szExt`）（LX278-L283）
- [x] 保存对话框返回路径无扩展名时追加默认扩展名（LX334-L338）
- [x] `BrowseForFolders` 的 `allow_many == true`（LX239）
- [x] `nFileTypeIndex` / `defaultExt` 在 `BrowseForFiles` 中显式标注未使用并说明原因（LX350-L354）
- [x] `FileDialog.h`、`FileDialog_Windows.cpp` 未改动（签名与 Windows 行为不变）
- [x] `L329` 的过期 TODO 已替换为准确说明（LX394-L396）
- [x] 筛选器转换逻辑合并到 `ConvertFileTypes`，`BrowseForFile` / `BrowseForFiles` 共用（无重复代码）

## 编译
- [x] Debug x64 增量编译通过，**0 error / 0 warning**（全部示例链接成功）
- [x] `FileDialog_SDL.obj` / `PropertyGrid.obj` 时间戳 2026/10/20 17:54，`bin\*_d.exe` 已重新链接（改动进入产物）

## 运行时（用户人工）
- [ ] 保存对话框：初始目录正确、文件名带默认扩展名、无扩展名输入被校正
- [ ] 打开对话框：初始目录正确、文件名预填正确
- [ ] 目录对话框：初始目录正确
- [ ] `BrowseForFolders` 可多选目录
- [ ] 取消对话框返回 `false` 且无卡死/重复回调
- [ ] 无筛选器时对话框正常打开

## 文档
- [x] `Development-Plan.md` 中 C-6 段落（标为 ✅ 已完成）与勾选项（现 `L776`）已更新；C-4 已按用户要求标注为「不处理，保持现状」（`L426`、`L740`、`L775`）；基线日期更新为 2026-10-20
