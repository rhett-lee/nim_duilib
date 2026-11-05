# AddressBar 第二轮 Review 任务清单

## 待办

- [x] T1（P0-1）`HandleEditKeyDown` 增加 `IsEditing()` 前置判断（`AddressBar.cpp` L1844）
- [x] T2（P0-2）新增 `RestoreBreadcrumbFocus()`（`AddressBar.h` + `.cpp`），并在 `ShowAddressEdit(false)` 末尾调用
- [x] T3（P0-3）`ResetEditText(bool bSetFocus = true)` 改造，L1545 / L1604 传 `false`，L820 保持默认
- [x] T4（P1-4）`OnAddressBarKillFocus` 用 `GetWindow()->IsWindowFocused()` 判定失活；失活时不重置文本、不退出编辑态
- [x] T5（P1-5）`AddPathHistory`、`LoadPathHistory` 与 `AddCandidate` 改用 `StringUtil::IsEqualNoCase`
- [x] T6（P1-6）`ScanDirectoryNames` 在 `ec` 置位时清空结果并返回 false
- [x] T7（P2-7）无候选 Tab：先 `ResetEditText()` 再 `SetNextTabControl(true)`
- [x] T8（P2-8）`SetHistoryFile` 重新加载后清除 `m_bHistoryDirty`
- [ ] T9（P2-9，可选）在 `docs/misc/AddressBar改进.md` 补充 XML 属性表
- [x] T10 构建 duilib / TreeView / test_address_bar_path，运行 CTest
- [ ] T11 人工回归（焦点、Esc/Tab、大小写候选、主题/语言/DPI、拖放），并在本文件记录结果

## 验证命令

```powershell
Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -DevCmdArguments "-arch=amd64 -host_arch=amd64"
cmake --build out\x64\Release --target duilib TreeView test_address_bar_path
ctest --test-dir out\x64\Release -R address_bar_path --output-on-failure
```

## 完成记录

- 2026-11-05 构建通过：`duilib.lib`、`bin/TreeView.exe`、`tests/test_address_bar_path.exe` 全部链接成功。
- 2026-11-05 `ctest -R address_bar_path`：1/1 通过（0.12s）。
- 待人工回归（T11）：提交后 Tab 焦点、Esc 后面包屑焦点、点击其他控件不抢焦点、大小写候选去重、主题/语言/DPI、拖放。
