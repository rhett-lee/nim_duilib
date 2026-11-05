# AddressBar 第二轮 Review 与修复 Spec

## 1. 背景

上一轮 Review（`docs/misc/specs/AddressBar-improvement`）提出的 H1~H4、M1~M7、L1~L2 已全部实施并通过构建与 CTest。
本轮对当前工作区（含未提交改动）的 AddressBar 再做一次 Review，聚焦：编辑态生命周期、焦点归属、历史/自动补全一致性、错误路径健壮性。

盘点结论：核心功能完整，遗留问题集中在“编辑态退出后的焦点处理”和“Windows 路径大小写/枚举失败”两类边界。

## 2. 涉及文件

| 文件 | 作用 |
| --- | --- |
| `duilib/Control/AddressBar.cpp` | 主要修改点 |
| `duilib/Control/AddressBar.h` | `ResetEditText` 签名、新增私有方法声明 |
| `duilib/Control/AddressBarPathUtil.cpp` | 无需修改（继续复用 `NormalizeAddressBarHistoryPath`） |
| `tests/test_address_bar_path.cpp` | 回归验证（现有用例即可） |
| `docs/misc/AddressBar改进.md` | 可选：补充 XML 属性文档 |

## 3. 问题清单与修复方案

### P0-1 编辑框隐藏后仍处理导航键

- 位置：`AddressBar.cpp` `AddressBarEdit::OnKeyDown`（L58-65）、`AddressBar::HandleEditKeyDown`（L1844）
- 现象：`AddressBarEdit::OnKeyDown` 无条件调用 `HandleEditKeyDown`。编辑框被隐藏后若仍有键盘事件到达（消息队列延迟、焦点残留、IME），会执行 Tab 补全、上下键移动候选、回车提交、Esc 收起等逻辑。
- 修复：在 `HandleEditKeyDown` 首行增加
  ```cpp
  if (!IsEditing()) {
      return false;
  }
  ```
  保持 `AddressBarEdit::OnKeyDown` 不变。

### P0-2 回车/Esc/选中候选后键盘焦点丢失

- 位置：`AddressBar.cpp` `ShowAddressEdit`（L934-964）、`CommitAddressPath`（L1622-1624）、`OnAddressBarEsc`（L1577-1585）
- 现象：`ShowAddressEdit(false)` 隐藏当前持有焦点的 RichEdit，`Window::SetFocusControl(nullptr)` 后 `m_pFocus == nullptr`；`Window::OnKeyDownMsg`（`Window.cpp` L1651-1678）在 `m_pFocus == nullptr` 时不派发按键，Tab / 方向键 / F6 全部失效，必须再点一次鼠标才能恢复。
- 修复：
  1. `AddressBar.h` 增加私有方法 `void RestoreBreadcrumbFocus();`
  2. `AddressBar.cpp` 实现：仅当窗口仍激活且当前无焦点控件时才聚焦面包屑，绝不抢已落到其他控件的焦点：
     ```cpp
     Window* pWindow = GetWindow();
     if ((pWindow == nullptr) || !pWindow->IsWindowFocused() || (pWindow->GetFocusControl() != nullptr)) {
         return;
     }
     if (m_pBarBox == nullptr) {
         return;
     }
     for (size_t i = m_subPathBoxes.size(); i > 0; --i) {
         HBox* pBox = m_subPathBoxes[i - 1];
         if ((pBox == nullptr) || !pBox->IsVisible()) {
             continue;
         }
         Control* pBtn = pBox->GetItemAt(0);
         if ((pBtn != nullptr) && pBtn->IsVisible() && pBtn->IsEnabled()) {
             pBtn->SetFocus();
             return;
         }
     }
     ```
  3. 在 `ShowAddressEdit(false)` 的末尾（`UpdateHistoryButtonVisible()` 之后）调用 `RestoreBreadcrumbFocus();`。
     调用链已覆盖：回车提交、Esc 取消、选中候选提交、失焦提交。
     以下场景会自动跳过：窗口失活（`IsWindowFocused()==false`）、焦点已到其他控件（`GetFocusControl()!=nullptr`）、整体隐藏/无可见路径段。
  4. 确认不会重入编辑态：面包屑名称按钮已支持 `SetFocus()`（L1017-1021），不会触发 `m_pBarBox` 的 `AttachSetFocus` 重新进入编辑态。

### P0-3 失焦路径中 ResetEditText 抢走点击目标的焦点

- 位置：`AddressBar.cpp` `ResetEditText`（L982-989）、调用点 L1545 / L1604 / L820
- 现象：`ResetEditText()` 无条件 `m_pRichEdit->SetFocus()`。失焦提交被拒绝（L1604）或 `kill_focus_update_ui=false`（L1545）时，用户刚点击的控件已获得焦点，又被编辑框抢走，随后编辑框隐藏，导致被点击控件最终没有焦点。
- 修复：
  1. `AddressBar.h` L419 改为 `void ResetEditText(bool bSetFocus = true);`
  2. 实现中 `if (bSetFocus) { m_pRichEdit->SetFocus(); }`
  3. `AddressBar.cpp` L1545、L1604 两处失焦调用改为 `ResetEditText(false);`
  4. L820 “恢复当前路径”按钮保持 `ResetEditText()`（默认 true，焦点留在编辑框）。

### P1-4 “窗口失活”判定不准确且会丢弃输入

- 位置：`AddressBar.cpp` `OnAddressBarKillFocus`（L1528-1559）
- 现象 1：`const bool bWindowDeactivated = (pNewFocus == nullptr);` —— 控件自身被隐藏/禁用时同样会产生 `pNewFocus == nullptr`，被误判为窗口失活。
- 现象 2：`m_bKillFocusUpdateUI == false` 分支无条件 `ResetEditText()`，即使窗口失活也会丢弃用户输入，与 L1536-1537 注释“保留编辑态与输入内容”矛盾。
- 修复：
  ```cpp
  Window* pWindow = GetWindow();
  const bool bWindowDeactivated = (pWindow == nullptr) || !pWindow->IsWindowFocused();
  ...
  else if (!bWindowDeactivated) {
      ResetEditText(false);
      ShowAddressEdit(false);
  }
  ```
  窗口失活时既不重置文本，也不退出编辑态。

### P1-5 历史与候选去重对大小写敏感

- 位置：`AddressBar.cpp` `AddPathHistory`（L434-439）、`BuildAutoCompleteCandidates` 内 `AddCandidate`（L2007-2011）
- 现象：Windows 路径不区分大小写，`C:\Foo` 与 `c:\foo` 会被当作两条记录/候选；前缀匹配（L1999-2003）已不区分大小写，因此实际会出现重复候选。
- 修复：使用已有 API `StringUtil::IsEqualNoCase`（`duilib/Utils/StringUtil.h` L52-59）：
  ```cpp
  if (StringUtil::IsEqualNoCase(it->c_str(), historyPath.c_str())) { ... }        // AddPathHistory
  if (StringUtil::IsEqualNoCase(exist.c_str(), candidate.c_str())) { return; }   // AddCandidate
  ```

### P1-6 目录枚举部分失败时缓存不完整结果

- 位置：`AddressBar.cpp` `ScanDirectoryNames`（L2086-2117）
- 现象：迭代器出错（`ec` 置位，例如权限/设备异常）时循环提前结束，但函数仍 `return true`，把已扫描到的部分结果缓存下来；后续同目录输入会一直复用这份不完整缓存。
- 修复：循环结束后、排序之前增加
  ```cpp
  if (ec) {
      dirNames.clear();
      fileNames.clear();
      return false;
  }
  ```
  调用方（L2060-2063）在返回 false 时已清空 `m_autoCompleteCache.m_dirPath`，下次输入会重新扫描。

### P2-7 无候选时按 Tab 不应提交半截路径

- 位置：`AddressBar.cpp` `HandleEditKeyDown` Tab 分支（L1879-1882）
- 现象：没有候选时直接 `SetNextTabControl(true)`，焦点离开触发 `OnAddressBarKillFocus` 提交用户输入的半截路径；若校验失败会显示红色错误态。资源管理器的行为是放弃当前输入并离开编辑态。
- 修复：改为
  ```cpp
  else if (GetWindow() != nullptr) {
      ResetEditText();                       // 恢复为当前路径（focus 仍在编辑框）
      GetWindow()->SetNextTabControl(true);  // 移动焦点；失焦提交的是当前路径，不会触发导航
  }
  ```
- 若产品希望保留“Tab 即提交”的语义，可保持现状，但需在评审中明确选择。

### P2-8 运行时切换历史文件未清脏标记

- 位置：`AddressBar.cpp` `SetHistoryFile`（L475-488）
- 现象：切换文件后 `m_bHistoryDirty` 可能仍为 true，1 秒防抖定时器会把新文件内容再写一遍（无害但多余）。
- 修复：`SetHistoryFile` 内 `LoadPathHistory()` 之后执行 `m_bHistoryDirty = false;` 并 `SavePathHistory()` 仅按需（不强制写）。

### P2-9 文档未列出新增 XML 属性（可选）

- 位置：`docs/misc/AddressBar改进.md`
- 现状：未检索到 `history_enabled`、`max_history`、`history_file`、`drop_navigation`、`default_context_menu`、`path_tooltip` 等属性的说明。
- 处理：补充属性表（名称、类型、默认值、说明）。如本轮不做，记录为已知缺口。

## 4. 明确不做（设计取舍）

1. 自动补全不支持带引号路径、相对路径、仅 `/` 输入（L2031-2053）：这些输入在地址栏场景优先级低。
2. 目录扫描上限 `kMaxAutoCompleteScanCount = 2000`（L2097-2101）：为避免超大目录卡顿，保持。
3. 不引入 UIA 无障碍接口：项目内其他控件均未提供。

## 5. 验证方案

1. 编译（VS18 环境）：
   ```powershell
   Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
   Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -DevCmdArguments "-arch=amd64 -host_arch=amd64"
   cmake --build out\x64\Release --target duilib TreeView test_address_bar_path
   ```
2. 单元测试：`ctest --test-dir out\x64\Release -R test_address_bar_path --output-on-failure`
3. 人工回归（TreeView 示例）：
   - Ctrl+L 输入有效路径 → 回车：面包屑更新；随后直接按 Tab，焦点应移动到下一个控件（不再“键盘无响应”）。
   - Ctrl+L 输入 `~` 或带尾分隔符路径 → Esc：恢复面包屑，焦点落在当前路径段按钮，可按左/右方向键切换段。
   - 编辑态下点击目录树中的节点：点击后该节点保持焦点，编辑框退出且不抢回焦点。
   - 输入非法路径 → 回车 / 点击其他控件：红色错误态，焦点正常落到被点击控件。
   - 输入 `C:\Win` 观察候选；`c:\win` 与 `C:\Win` 不应出现两条候选/历史。
   - 无候选时输入半截路径按 Tab：不触发导航，焦点按 Tab 顺序离开地址栏。
   - 切换主题、切换语言、修改 DPI：工具提示/高亮/布局正常（沿用上一轮人工清单）。
   - 拖放文件夹到地址栏：正常导航；`drop_navigation=false` 时无高亮、不导航。

## 6. 风险

- `RestoreBreadcrumbFocus` 会改变键盘焦点归属，需确认没有其他控件依赖“提交后焦点为空”的既有行为（项目内仅 TreeView 示例使用地址栏）。
- `IsWindowFocused()` 在 SDL 平台的行为与 Windows 不同（见 `Window.cpp` L2195 注释）；该分支仅在窗口失活判定中使用，且仅在 Windows 目标验证。

## 7. 审批

以上 P0/P1 必须修复，P2 由评审确认；确认后按 `tasks.md` 执行。
