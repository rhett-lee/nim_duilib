# CefBrowser 示例优化计划

## Context

`examples/CefBrowser` 是仓库中的多标签 CEF 浏览器示例（离屏渲染模式）。其界面仍为早期风格：标题栏是 36px 图标 + 竖线，工具条是 60x30 的文字按钮（后退/前进/刷新/停止），地址栏圆角、高度与已完成的 WebView2Browser 不一致；代码中存在空指针解引用、事件吞噬、除零等稳定性问题，缺少 Chrome/Edge 常用快捷键，窗口标题也不随当前网页变化。

本次按已完成的 WebView2Browser 的同一思路优化，目标为：**P0 代码缺陷 + P1 界面统一（对齐 WebView2 示例与 Edge/Chrome 视觉）+ P2 Chrome/Edge 快捷键**，窗口标题跟随当前网页标题。

## 关键结论：CEF 的快捷键拦截点

CefBrowser 示例在 `TestApplication.cpp` 中设置 `bEnableOSR = true`（离屏渲染），网页控件 `CefControlOffScreen` 是 duilib 内部控件、不创建独立子窗口，键盘消息仍进入 duilib 主窗口。`BrowserForm::OnKeyDownMsg` 重写后位于 `Window::OnKeyDownMsg` 之前执行，处理快捷键并返回即可阻止按键继续转发给 CEF 页面。

因此**不需要**像 WebView2 那样新增框架级 `SetAcceleratorKeyPressedCallback`，只在示例层把快捷键逻辑集中到 `HandleShortcutKey` 即可。

## 文件清单

| 文件 | 改动 |
|---|---|
| `bin/resources/themes/default/cef_browser/cef_browser.xml` | 按 WebView2 示例重构标题栏、卡片式工具条、地址栏、网页区域 |
| `bin/resources/themes/default/cef_browser/tab_item.xml` | 标签最小宽度 20 → 56 |
| `bin/resources/themes/default/cef_browser/*.svg` | 从 `themes/default/cef/` 复制 5 个主题化图标 |
| `examples/CefBrowser/browser/BrowserForm.h` | 新增缓存按钮成员、快捷键/标题/按钮状态辅助方法声明 |
| `examples/CefBrowser/browser/BrowserForm.cpp` | 修复 P0 缺陷、动态标题栏布局、P2 快捷键、窗口标题同步 |
| `examples/CefBrowser/browser/BrowserBox.cpp` | 类型转换与回调中的空指针保护 |
| `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt` | 新增 `STRID_CEF_BROWSER_NEW_TAB_PAGE` |

## 具体改动

### 1. cef_browser.xml 重构（对齐 webview2_browser.xml）

- 窗口属性：`size="80%,80%" min_size="760,540"`，补 `snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"`，保留 `text_id="STRID_CEF_BROWSER_MULTI_TAB"`。
- 根容器：`VBox bkcolor="bg_window_main"`。
- 标题栏（36 高，`bkcolor="bg_titlebar"`）：
  - 去掉 36px 图标与 `tab_ctrl_left_line` 竖线，改为 18x18 `public/caption/logo.svg`，`margin="8,0,6,0"`；
  - `TabCtrl class="tab_ctrl" name="tab_ctrl"`；
  - 新建标签按钮 `btn_add`：28x28、`border_round="6,6"`、`margin="2,4,4,4"`；
  - 右侧 `window_title_bar` 保持现有六个 40 宽按钮。
- 内容区：`VBox bkcolor="bg_window_main" padding="10,8,10,10"`。
  - 工具条：`HBox class="browser_card" height="46" padding="8,0,8,0" child_margin="2"`；后退/前进/刷新/停止四个 32x26 图标按钮；竖分隔线；地址栏 `RichEdit class="simple simple_border prompt" height="28" border_round="6,6" text_padding="6,0,6,0"`（`prompt_text_id="STRID_CEF_BROWSER_INPUT_ADDRESS"`）；再一条分隔线；`btn_dev_tool`（`dev_tool.svg`）。
  - 网页区：`VBox class="browser_card" height="stretch" margin="0,8,0,0" padding="4,4,4,4"`，内部 `<TabBox name="browser_box_tab" bkcolor="white"/>`。
- 局部 Class（复制 WebView2 写法）：
  - `tab_item_new_add_btn`（保留现有定义）；
  - `browser_card`：`bg_window_card` + 1px `border_split_level1` + 圆角 8；
  - `browser_tool_btn`：32x26、圆角 6、hovered/pressed；
  - `browser_tool_back/forward/refresh/stop`：`normal_image` + `disabled_image`（fade=128），`svg_replace_colors='#1E1E1E|border_svg_image'`。
- 删除不再使用的 `tab_ctrl_left_line`、`btn_cef_browser` 类，以及“一条分割线”控件。

### 2. 图标资源

从 `bin/resources/themes/default/cef/` 复制到 `cef_browser/`：`arrow-left.svg`、`arrow-right.svg`、`refresh.svg`、`stop.svg`、`dev_tool.svg`（内容保持，`stroke="#1E1E1E"` 由 `border_svg_image` 主题化替换）。

### 3. tab_item.xml 与多语言

- `min_width="56"`，`max_width="240"` 不变。
- 两个语言文件新增：
  - 中文：`STRID_CEF_BROWSER_NEW_TAB_PAGE = 新标签页`
  - 英文：`STRID_CEF_BROWSER_NEW_TAB_PAGE = New tab`

### 4. BrowserForm.h

- 新增缓存成员：`m_pBtnBack`、`m_pBtnForward`、`m_pBtnRefresh`、`m_pBtnStop`（构造/析构初始化为 nullptr）。
- 新增私有方法：`bool HandleShortcutKey(ui::VirtualKeyCode vkCode);`、`void UpdateNavigationButtonState();`、`void UpdateWindowTitle();`。

### 5. BrowserForm.cpp

**P0 修复**

1. `TitleBarHLayout::ArrangeChildren`：去掉 `items.size() == 5` 与 `items[2]` 的硬编码，改为遍历查找 `TabCtrl`，要求其后至少有两个控件，并保留 `bEstimateOnly` 分支（与 WebView2 版本一致）。
2. `OnInitWindow`：
   - `m_pEditUrl` 判空后再 `AttachReturn`；
   - `HBox/TabCtrl/TabBox` 一律 `dynamic_cast`；
   - 缓存四个工具条按钮并初始化（后退/前进禁用、刷新显示、停止隐藏）。
3. `OnCloseWindow`：`m_pBorwserBoxTab` 判空，循环内用 `dynamic_cast`。
4. `OnLoadingStateChange`：使用缓存按钮并判空。
5. `OnKeyDownMsg`：委托 `HandleShortcutKey`；`m_pTabCtrl` 判空且 `GetItemCount() > 0`，避免除零。
6. `OnWindowCloseMsg`：`m_pTabCtrl` 判空。
7. `ChangeToBox`：`m_pActiveBrowserBox->GetCefControl()`、`m_pEditUrl` 判空，更新地址栏后调用 `OnLoadingStateChange` 与 `UpdateWindowTitle`。
8. `OnBeforeDragBoxCallback`：`pTabItem` 判空后再取索引，索引越界保护。
9. `OnReturn`：去首尾空白后退，导航完成后把焦点交回 `CefControl`。
10. `OnClicked`：未识别的点击返回 `false`，不再吞事件；新增 `btn_dev_tool` → `SwitchShowDevTools()`。

**P2 交互（HandleShortcutKey）**

沿用 `ui::Keyboard::IsKeyDown` 判断修饰键：
- `Ctrl+T` 新建标签（`BrowserManager::CreateBorwserBox`）；
- `Ctrl+W` 关闭当前标签；
- `Ctrl+L` 聚焦并全选地址栏；
- `Ctrl+Tab` / `Ctrl+Shift+Tab` 下一/上一标签（先判空与数量）；
- `Ctrl+R` / `F5` 刷新；
- `Alt+←` / `Alt+→` 后退/前进（`CefControl::GoBack/GoForward`）；
- `Esc`：拖拽中取消拖出；否则加载中调用 `StopLoad()`；
- 保留 `F11`（页面全屏）与 `F12`（开发者工具）原逻辑。

**窗口标题与标签初始标题**

- `UpdateWindowTitle()`：取当前 `BrowserBox::GetTitle()`，非空 `SetText(title)`，否则 `SetTextId(_T("STRID_CEF_BROWSER_MULTI_TAB"))`；在 `ChangeToBox`、`SetTabItemName`（当前标签时）、`CloseBox` 切换后调用。
- `CreateBox`：创建 `TabCtrlItem` 后设置初始标题——`url` 为空 → `STRID_CEF_BROWSER_NEW_TAB_PAGE`，否则用 `url`，等待网页标题回调覆盖。
- `AttachBox`：`GetTitle()` 为空时用“新标签页”兜底。

### 6. BrowserBox.cpp

- `InitBrowserBox` 中 `FindSubControl` 用 `dynamic_cast`。
- `OnTitleChange` / `OnUrlChange` / `OnFaviconURLChange` 等回调先判 `m_pBrowserForm != nullptr`，避免跨窗口附加/销毁过程中的空指针。

## 验证

1. 编译（VS 环境 + Ninja）：
   ```powershell
   Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
   Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -SkipAutomaticLocation -DevCmdArguments "-arch=amd64 -host_arch=amd64"
   cmake --build "e:\develop\nim_duilib\out\x64\Release" --target CefBrowser
   ```
   若 exe 正在运行先 `Get-Process -Name CefBrowser | Stop-Process -Force`。
2. 启动：使用 WMI `Win32_Process::Create` 启动 `bin\CefBrowser.exe`。
3. 截图验证（浅色/深色主题）：
   - 标题栏 logo、标签、+ 按钮、窗口按钮布局与 WebView2 示例一致；
   - 卡片式工具条、图标按钮、分隔线、圆角地址栏显示正常，SVG 图标颜色随主题变化；
   - 连续新建 3~5 个标签，验证标签宽度、选中态、关闭按钮与内容区切换；
   - 地址栏输入网址回车，验证加载中“刷新→停止”切换与按钮禁用状态；
   - 缩小到最小尺寸，标题栏与工具条不重叠。
4. 快捷键冒烟（网页有焦点时）：`Ctrl+T` / `Ctrl+W` / `Ctrl+L` / `Ctrl+Tab` / `Ctrl+Shift+Tab` / `Ctrl+R` / `F5` / `Alt+←→` / `Esc`。
5. 窗口标题随当前标签网页标题变化；关闭全部标签时窗口正常关闭；无崩溃与断言。
