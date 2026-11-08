# 四示例代码 Review 与 UI 自动化测试计划

## Context

`examples/cef`、`examples/CefBrowser`、`examples/WebView2`、`examples/WebView2Browser` 四个示例的界面与功能优化已完成（WebView2Browser/CefBrowser 的界面与快捷键改动尚未提交）。本阶段目标：
1. 对四个示例做一次代码 Review，修复发现的问题；
2. 对基本功能做 UI 自动化测试，逐项验证并修复问题。

Review 已完成静态阅读（四个示例的 Form/Box/Manager/DragDrop 与 XML），共发现 12 项问题；单页示例 `examples/cef`、`examples/WebView2` 未发现功能级缺陷，问题集中在两个多标签示例。

## Review 问题清单

### P1（空指针 / 越界，必须修复）

1. **WebView2Browser/BrowserForm.cpp `OnBeforeDragBoxCallback`**
   - `pTabItem` 可能为 `nullptr`，但未判空就调用 `pTabItem->GetListBoxIndex()`；
   - 单标签时 `index+1` 超出范围，直接 `GetItemAt(index)` 且用 `static_cast`。
   - 修复：对齐 CefBrowser 版本——判空、越界检查、`dynamic_cast`、失败时恢复盒子显示。

2. **两处 `GenerateWebPageBitmap`（CefBrowser 与 WebView2Browser 的 BrowserForm.cpp）**
   - `ASSERT(render != nullptr)` 后直接 `render->Resize(...)`，Release 下 `CreateRender` 失败会空指针。
   - 修复：`if ((render != nullptr) && render->Resize(...))`。

3. **CefBrowser/BrowserForm.cpp `NotifyFavicon`**
   - `image == nullptr` 时调用 `ClearIconData()` 后仍继续执行 `image->GetAsBitmap(...)`，必然崩溃。
   - 修复：清空图标后立即 `return`。

4. **CefBrowser/Windows/BrowserBox_Windows.cpp `ConvertCefImageToHICON`**
   - `CreateDIBSection` 失败时 `pBitsColor` 为 `nullptr`，随后 `memcpy` 崩溃。
   - 修复：校验 `hBmpColor`/`pBitsColor`，失败返回 `nullptr`。

5. **WebView2Browser/BrowserBox.cpp `InitBrowserBox`**
   - `static_cast<ui::WebView2Control*>`；`SetSourceChangedCallback` 中直接 `m_pBrowserForm->SetURL`，未判空。
   - 修复：改 `dynamic_cast`，回调内判空。

### P2（功能 / 健壮性）

6. **CefBrowser/BrowserForm.cpp `OnReturn`**
   - 输入 `www.baidu.com` 这类无协议地址时直接 `LoadURL`，CEF 不会自动补全协议，导航失败；单页 `CefForm::OnNavigate` 已有协议补全逻辑。
   - 修复：复用同样规则（无 `://`、非 `about:`、非 `file:` 时补 `https://`）。

7. **WebView2Browser/BrowserForm.cpp `OnTabItemSelected`**
   - `GetCurSel()` 可能为 `Box::InvalidIndex`，直接 `GetItemAt`。
   - 修复：对齐 CefBrowser，先判 `InvalidIndex`。

8. **WebView2Browser/BrowserForm.cpp `OnCloseWindow`**
   - 未判空 `m_pBorwserBoxTab` 就遍历（CefBrowser 已判空）。
   - 修复：补空指针检查。

9. **WebView2Browser/DragDropManager.cpp**
   - 拖出新窗口时 `WindowCreateParam(_T("CefBrowser"))`，WebView2 示例错误地使用 CefBrowser 窗口标题。
   - 修复：改为 `_T("WebView2Browser")`。

10. **WebView2Browser/BrowserForm.cpp `HandleShortcutKey` 的 Esc 分支**
    - 取消标签拖出后未设置 `bHandled`，Esc 会继续下传。
    - 修复：取消拖出后返回已处理。

11. **CefBrowser/BrowserBox.cpp `OnLoadEnd`**
    - `m_pCefControl->RegisterCppFunc` 未判空。
    - 修复：加空指针保护。

12. **CefBrowser/Windows/BrowserForm_Windows.cpp `OnWindowMessage`**
    - `WM_TASKBARBUTTONCREATED` 分支遍历 `m_pBorwserBoxTab` 未判空。
    - 修复：补空指针保护。

> 单页示例 `examples/cef`（CefForm）、`examples/WebView2`（MainForm）经比对：按钮状态、地址栏、前进/后退/刷新/停止、F11/F12、焦点处理均已正确，无代码改动。

## 实施步骤

1. 按上表逐项修改（仅上述文件，不做无关重构）。
2. 编译四个 target（x64 Release，VS 2026 DevShell）：
   - `cmake --build e:\develop\nim_duilib\out\x64\Release --target cef CefBrowser WebView2 WebView2Browser`
   - 链接报 LNK1104 时先结束同名进程。
3. UI 自动化测试（每个示例：启动 → 截图 → 操作 → 截图 → 关闭）：
   - **cef**：窗口/卡片布局、浅色与深色主题、地址栏 Enter（含无协议输入）、后退/前进、刷新、停止、F12 开发者工具、F11 全屏。
   - **WebView2**：同上，验证 WebView2 初始化、F12。
   - **CefBrowser**：新建/关闭/切换标签、Ctrl+T/W/L/Tab、Ctrl+R/F5、Alt+←/→、Esc 停止、窗口标题跟随网页标题、关闭最后一个标签退出。
   - **WebView2Browser**：同上；重点回归 Ctrl+T 新标签、拖拽标签离开/合并、Esc 取消拖出、拖出新窗口标题为 WebView2Browser。
   - 工具：`%TEMP%\ui_audit.ps1`（rect/shot/click）、`wv2keys.ps1`、`capture_popup.ps1`；GUI 启动必须 `dangerouslyDisableSandbox=true` + `Win32_Process.Create`。
4. 发现新问题立即修复并回归对应示例。
5. 清理临时截图/脚本，输出中文总结与 conventional commit 日志（不代用户提交）。

## 验证标准

- 四个 target 编译通过（0 error）。
- 每个示例实际启动并截图确认：界面无错位/空白，浅色与深色主题均正常。
- 导航、标签页、快捷键、拖拽功能逐项通过；无崩溃、无断言。
- 修改文件清单与 `git status` 一致，无临时文件残留。
