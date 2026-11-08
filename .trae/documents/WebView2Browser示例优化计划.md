# WebView2Browser 示例优化计划

## Context

WebView2Browser 是仓库中的多标签浏览器示例，当前界面仍是早期风格：标题栏用 36px 图标 + 竖线，工具条是 30 高的文字按钮（后退/前进/刷新/停止），地址栏圆角和高度与已完成的 WebView2、cef 示例不一致；代码里还存在空指针、事件吞噬、除零等稳定性问题。

本次按已完成的 WebView2 示例思路，并以 Edge/Chrome 的标签页布局和快捷键习惯为参照，统一界面、修复缺陷、补齐常用交互。范围已确认为 **P0+P1+P2**，窗口标题跟随当前网页标题。

## 文件清单

| 文件 | 改动 |
|---|---|
| `bin/resources/themes/default/webview2_browser/webview2_browser.xml` | 重构标题栏、工具条、内容区 |
| `bin/resources/themes/default/webview2_browser/tab_item.xml` | 标签最小宽度 20→56 |
| `bin/resources/themes/default/webview2_browser/*.svg` | 新增 5 个主题化图标（从 `webview2/` 复制） |
| `examples/WebView2Browser/BrowserForm.h` | 新增缓存控件指针与辅助方法声明 |
| `examples/WebView2Browser/BrowserForm.cpp` | 修复 P0 缺陷，加入 P2 快捷键与窗口标题同步 |
| `examples/WebView2Browser/BrowserBox.cpp` | 新标签初始标题兜底 |
| `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt` | 新增 `STRID_WEBVIEW2_BROWSER_NEW_TAB_PAGE` |

## 具体改动

### 1. webview2_browser.xml 重构

- 窗口属性：`size="80%,80%"`、`min_size="760,540"`，补 `snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"`，`textid` 改为 `text_id="STRID_WEBVIEW2_BROWSER_MULTI_TAB"`。
- 根容器：`VBox bkcolor="bg_window_main"`（替换现有 `bg_tab_ctrl_item_selected`）。
- 标题栏（36 高，`bkcolor="bg_titlebar"`）：
  - 左侧 18x18 `logo.svg`，`margin="8,0,6,0"`，去掉竖分隔线；
  - `TabCtrl class="tab_ctrl" name="tab_ctrl"` 占满剩余宽度；
  - 新建标签按钮 `btn_add`：28x28、`border_round="6,6"`、`margin="2,4,4,4"`，保留 `STRID_WEBVIEW2_BROWSER_NEW_TAB` 提示；
  - 右侧窗口控制区保持现有 40 宽按钮结构。
- 内容区：`VBox bkcolor="bg_window_main" padding="10,8,10,10"`。
  - 工具条：`HBox class="browser_card" height="46" padding="8,0,8,0" child_margin="2"`，包含后退/前进/刷新/停止四个 32x26 图标按钮、竖分隔线、地址栏（`width="stretch" height="28" border_round="6,6" text_padding="6,0,6,0"`）、竖分隔线、开发者工具按钮。去掉“转到”文字按钮。
  - 网页区：`VBox class="browser_card" height="stretch" margin="0,8,0,0" padding="4,4,4,4"`，内部保留 `<TabBox name="browser_box_tab" bkcolor="white"/>`。
- 新增局部样式类（复制 WebView2 示例的写法）：
  - `browser_card`：`bg_window_card` + 1px `border_split_level1` + 圆角 8；
  - `browser_tool_btn`：32x26、圆角 6、hovered/pressed 使用 `bg_btn_hovered`/`bg_btn_pressed`；
  - `browser_tool_back/forward/refresh/stop`：`normal_image` + `disabled_image`（fade=128），`svg_replace_colors='#1E1E1E|border_svg_image'`。

### 2. 图标资源

从 `bin/resources/themes/default/webview2/` 复制到 `webview2_browser/`：

- `arrow-left.svg`、`arrow-right.svg`、`refresh.svg`、`stop.svg`、`dev_tool.svg`

保持 SVG 内容不变，依靠 `border_svg_image` 主题色自动适配浅色/深色。

### 3. tab_item.xml 与多语言

- `tab_item.xml`：`min_width="56"`，`max_width="240"` 保持不变。
- 两个语言文件新增：
  - 中文：`STRID_WEBVIEW2_BROWSER_NEW_TAB_PAGE = 新标签页`
  - 英文：`STRID_WEBVIEW2_BROWSER_NEW_TAB_PAGE = New tab`

### 4. BrowserForm.h

- 新增缓存成员：`m_pBtnBack`、`m_pBtnForward`、`m_pBtnRefresh`、`m_pBtnStop`（构造/析构初始化为 nullptr）。
- 新增私有方法：
  - `void UpdateNavigationButtonState();`
  - `void UpdateWindowTitle();`

### 5. BrowserForm.cpp

**P0 修复**

1. `OnInitWindow`：
   - 全部改用 `dynamic_cast`；
   - `m_pEditUrl` 判空后再 `AttachReturn`/`SetSelAllOnFocus`；
   - 缓存四个工具条按钮并初始化状态：后退/前进禁用、刷新显示、停止隐藏。
2. `OnClicked`：
   - 未识别的点击返回 `false`，不再吞事件；
   - 保留 `btn_close`、`btn_add`，新增 `btn_dev_tool` → `OpenDevToolsWindow()`。
3. `ChangeToBox`：对 `m_pActiveBrowserBox`、`GetWebView2Control()`、`m_pEditUrl` 判空；地址栏更新后调用 `OnLoadingStateChange` 与 `UpdateWindowTitle`。
4. `OnKeyDownMsg`：
   - `Ctrl+Tab` 前判断 `m_pTabCtrl != nullptr` 且 `GetItemCount() > 0`，避免除零；
   - 保留 F11/F12 与“Esc 取消标签拖出”逻辑。
5. `OnReturn`：去除首尾空白（`find_first_not_of/find_last_not_of`），取消防空判断后调用 `Navigate`，随后把焦点交回 WebView2。

**P1 支撑**

- `TitleBarHLayout::ArrangeChildren` 改为动态查找 `TabCtrl`，不再强依赖 `items.size() == 5`：
  - 找到 TabCtrl 后，确保其后至少有两个控件（新建按钮、右侧窗口区）；
  - 标签总宽小于 TabCtrl 宽度时，缩短 TabCtrl、左移新建按钮、扩展最后一个控件（可拖拽空白区）；
  - 保留 `bEstimateOnly` 分支，不写位置。

**P2 交互**

- 快捷键（统一用 `ui::Keyboard::IsKeyDown` 判断修饰键，沿用现有写法）：
  - `Ctrl+T`：新建标签；
  - `Ctrl+W`：关闭当前标签；
  - `Ctrl+L`：聚焦并全选地址栏；
  - `Ctrl+Shift+Tab`：上一个标签；
  - `Ctrl+R` / `F5`：刷新当前页；
  - `Alt+←` / `Alt+→`：后退/前进；
  - `Esc`：非拖拽状态下，若正在加载则停止加载。
- 窗口标题：`UpdateWindowTitle()` 中取当前 `BrowserBox::GetTitle()`，非空时 `SetText(title)`，否则 `SetTextId(_T("STRID_WEBVIEW2_BROWSER_MULTI_TAB"))`；在切换标签、`SetTabItemName`、关闭标签后调用。
- 新标签初始标题：`CreateBox` 创建 `TabCtrlItem` 后设置：
  - `url` 为空 → `GlobalManager::GetTextById(_T("STRID_WEBVIEW2_BROWSER_NEW_TAB_PAGE"))`；
  - `url` 非空 → 直接使用 `url`，等待 document title 回调覆盖。
- `AttachBox` 中若 `GetTitle()` 为空，同样使用“新标签页”兜底。

### 6. BrowserBox.cpp

- 无需结构性修改；仅在 `InitBrowserBox` 的标题回调里确认 `m_pBrowserForm` 判空后再调用 `SetTabItemName`（与现有其他回调保持一致）。

## 验证

1. 编译（VS 环境 + Ninja）：
   ```powershell
   Import-Module "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
   Enter-VsDevShell -VsInstallPath "C:\Program Files\Microsoft Visual Studio\18\Community" -SkipAutomaticLocation -DevCmdArguments "-arch=amd64 -host_arch=amd64"
   cmake --build "e:\develop\nim_duilib\out\x64\Release" --target WebView2Browser
   ```
   若 exe 正在运行先 `Get-Process -Name WebView2Browser | Stop-Process -Force`。
2. 启动：使用 WMI `Win32_Process::Create` 启动 `bin\WebView2Browser.exe`。
3. 截图验证（`ui_audit.ps1`）：
   - 浅色主题默认窗口：标题栏 logo、标签、+ 按钮、窗口按钮、图标工具条、地址栏、圆角卡片；
   - 切换深色主题：卡片、分割线、SVG 图标颜色正确，无白块；
   - 连续点击 `+` 创建 3~5 个标签，验证标签宽度、选中态、关闭按钮悬停、内容区切换；
   - 地址栏输入网址回车，验证加载中“刷新→停止”切换、加载完成后恢复；
   - 缩小到最小尺寸，验证标题栏与工具条不重叠、标签可滚动。
4. 快捷键冒烟：Ctrl+T / Ctrl+W / Ctrl+L / Ctrl+Shift+Tab / Ctrl+R / Alt+←→ / Esc。
5. 确认无崩溃、无控制台断言，任务栏标题随当前标签网页标题变化。
