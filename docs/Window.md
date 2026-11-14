简体中文 | [English](Window.en.md)

## 窗口的属性
| 属性名称          | 属性分类| 默认值  |参数类型| 关联[Window.h](../duilib/Core/Window.h)中的函数| 用途 |
| :---              | :---    | :---    | :---   |:---                     |:--- |
| caption           | 标题栏  | "0,0,0,0" | rect   | SetCaptionRect          |窗口可拖动的标题栏大小的边距, 最后一个参数是指离上边框的距离,如"0,0,0,36" |
| use_system_caption| 标题栏  | false   | bool   | SetUseSystemCaption     |设置是否使用系统标题栏 |
| snap_layout_menu  | 标题栏  | true    | bool   | SetEnableSnapLayoutMenu |是否支持显示贴靠布局菜单（Windows 11新功能：通过将鼠标悬停在窗口的最大化按钮上或按 Win + Z，可以轻松访问对齐布局。），该属性仅当在窗口为分层窗口或者使用系统阴影时默认开启 |
| sys_menu          | 标题栏  | true    | bool   | SetEnableSysMenu        |在右键点击标题栏时，是否显示系统的窗口菜单（可进行调整窗口状态，关闭窗口等操作）|
| sys_menu_rect     | 标题栏  | "0,0,0,0" | rect   | SetSysMenuRect          | 窗口菜单区域，在窗口标题栏的左上角（双击该区域可关闭窗口，点击显示系统的窗口菜单），该功能若XML中无设置，默认是关闭的 |
| icon              | 标题栏  |         | string | SetWindowIcon           |设置窗口的图标文件路径，支持ico格式 |
| text              | 标题栏  |         | string | SetText                 |窗体标题字符串|
| text_id           | 标题栏  |         | string | SetTextId               |窗体标题字符串的ID, ID在多语言文件中指定, 如"STRID_MIANWINDOW_TITLE" |
| drag_drop         | 拖放    | true    | bool   | SetEnableDragDrop       |设置窗口内控件是否支持拖放操作（拖入文件/文本）：true=支持，false=不支持 |
| shadow_attached   | 窗口阴影| true    | bool   | SetShadowAttached       |窗口是否附加阴影效果，如"true"表示开启窗口阴影功能，"false"表示关闭窗口阴影功能<br>如果设置了shadow_type属性，相当于隐式将shadow_attached设置为"true"，<br>所以一般设置了shadow_type属性以后，不需要再单独设置shadow_attached属性|
| shadow_type       | 窗口阴影|         | string | SetShadowType           |设置窗口的阴影类型：<br> "default"：默认自绘阴影 <br> "big"：有自绘阴影（大），直角，有边框（适合普通窗口）<br> "big_round"：有自绘阴影（大），圆角，有边框（适合普通窗口）<br> "small"：有自绘阴影（小），直角，有边框（适合普通窗口）<br> "small_round"：有自绘阴影（小），圆角，有边框（适合普通窗口）<br> "menu"：有自绘阴影（小），直角，有边框（适合弹出式窗口，比如菜单等）<br> "menu_round"：有自绘阴影（小），圆角，有边框（适合弹出式窗口，比如菜单等）<br> "none"：无阴影，直角，有边框<br> "none_round"：无阴影，圆角，有边框（自绘）<br> "custom": 自定义自绘阴影，该类型需要同时设置`shadow_image`、`shadow_corner`、`shadow_border_round属性` <br> "system_default"：使用系统阴影，跟随操作系统默认窗口阴影属性<br> "system_not_round"：使用系统阴影，直角<br> "system_round"：使用系统阴影，圆角<br> "system_small_round"：使用系统阴影，小圆角<br>兼容性说明：<br>（1）对于Win7和Win10系统：系统阴影仅支持"system_default"这一个，其他三个不支持<br>（2）对于Win11系统，四个系统阴影属性均支持，系统默认阴影为圆角<br>（3）对于macOS系统，四个系统阴影属性均支持，系统默认阴影为圆角<br>（4）对于Linux系统和FreeBSD系统，不支持系统阴影<br>（5）若设置为不支持的系统阴影，会自动切换为相应支持的系统阴影或者自绘阴影|
| shadow_image      | 窗口阴影|         | string | SetShadowImage          |使用自定义的阴影图片去代替默认的阴影效果，该属性一般仅当shadow_type="custom"时设置<br>设置的路径要注意相对路径以及九宫格属性，<br>如(file='public/shadow/shadow_big.svg' corner='64,64,68,70') |
| shadow_corner     | 窗口阴影| "0,0,0,0" | rect   | SetShadowCorner       |设置了shadow_image属性后，设置此属性来指定阴影素材的九宫格描述，该属性一般仅当shadow_type="custom"时设置 |
| shadow_border_round| 窗口阴影| "0,0"    | size  | SetShadowBorderRound|设置了shadow_image属性后，设置此属性来指定阴影的圆角属性，该属性一般仅当shadow_type="custom"时设置 |
| shadow_border_color| 窗口阴影|          |string | SetShadowBorderColor  |设置窗口阴影的边框颜色 |
| shadow_border_size | 窗口阴影|2         |int    | SetShadowBorderSize   |设置窗口阴影的边框像素大小，实际视觉显示的边框宽度为此值的一半（例如设置为2，实际显示1像素） |
| shadow_snap        | 窗口阴影| true     | bool  | SetEnableShadowSnap   |设置阴影是否支持窗口贴边操作，如果为true，则在窗口贴近屏幕边缘时，这一侧的阴影自动隐藏，以增大视图内的有效空间<br>该属性仅当使用自绘阴影时有效，当使用系统阴影时无效|
| size              | 窗口大小| "0,0"     | size   | SetInitSize             |窗口的初始化大小, 支持的格式：size="1200,800", 或者size="50%,50%", 或者size="1200,50%", size="50%,800"，百分比是指屏幕宽度或者高度的百分比 |
| size_contain_shadow| 窗口大小| false  | bool   |    |窗口的初始化大小(size属性)，是否包含窗口的阴影，默认不包含，窗口的实际大小是配置的size值 + 阴影大小 |
| min_size          | 窗口大小| "0,0"     | size   | SetWindowMinimumSize    |窗口最小大小, 如"320,240" |
| max_size          | 窗口大小| "0,0"     | size   | SetWindowMaximumSize    |窗口最大大小, 如"1600,1200" |
| size_box          | 窗口大小| "0,0,0,0" | rect   | SetSizeBox              |窗口可拖动改变窗口大小的边距, 如"4,4,4,4" |
| round_corner      | 窗口形状| "0,0"     | size   | SetRoundCorner          |窗口圆角大小, 如"4,4"，该属性在使用窗口阴影（即设置了shadow_type属性）时，一般不需要设置 |
| layered_window    | 窗口绘制| false   | bool   | SetLayeredWindow        |设置是否为层窗口，使用注意事项：<br>（1）该属性在使用窗口阴影（即设置了shadow_type属性）时不需要设置，窗口阴影功能会根据需要来管理该属性<br>（2）当使用系统标题栏时（即use_system_caption="true"），不应设置该属性 |
| alpha             | 窗口绘制| 255     | int    | SetLayeredWindowAlpha   |设置透明度数值[0, 255]，当 alpha 为 0 时，窗口是完全透明的。 当 alpha 为 255 时，窗口是不透明的。<br>仅当layered_window="true"时有效，<br>该参数在UpdateLayeredWindow函数中作为参数使用(BLENDFUNCTION.SourceConstantAlpha)|
| opacity           | 窗口绘制| 255     | int    | SetLayeredWindowOpacity |设置不透明度数值[0, 255]，当 opacity 为 0 时，窗口是完全透明的。 当 opacity 为 255 时，窗口是不透明的。<br> 仅当layered_window="true"时有效，所以如果当前不是分层窗口，内部会自动设置为分层窗口 <br>该参数在SetLayeredWindowAttributes函数中作为参数使用(bAlpha)|
| render_backend_type|窗口绘制| "CPU"   | string |SetRenderBackendType     | "CPU": CPU绘制 <br> "GL": 使用OpenGL绘制 <br> 注意事项: <br> （1）同一线程内可以有多个窗口使用OpenGL绘制，绘制前会自动切换当前上下文；但每个窗口都会创建一个独立的 GL 上下文，注意显存占用 <br> （2）OpenGL绘制的窗口，不能是分层窗口（即带有WS_EX_LAYERED属性的窗口），因此使用GL渲染时，`layered_window`、`alpha`、`opacity` 属性均无效 <br> （3）OpenGL绘制使用持久 FBO 做局部刷新，小区域动画不会整窗重绘，性能通常优于整窗重绘 <br> （4）窗口运行过程中，可通过SetAttribute("render_backend_type", "GL"/"CPU")实时切换（内部会重建渲染对象）|
| vsync             | 窗口绘制| "true"  | bool   | SetRenderBackendType    |是否启用垂直同步，false 表示禁用；<br>仅当 render_backend_type="GL" 时有效<br>支持运行过程中通过SetAttribute实时切换|
| msaa              | 窗口绘制| "0"     | int    | SetRenderBackendType    |MSAA 采样数，0 表示不启用 MSAA（按 1 倍采样）；<br>仅当 render_backend_type="GL" 时有效<br>支持运行过程中通过SetAttribute实时切换|

备注：窗口阴影相关的内容，请参考文档：[WindowShadow.md](WindowShadow.md)    
备注：窗口属性的解析函数参见：[WindowBuilder::ParseWindowAttributes函数](../duilib/Core/WindowBuilder.cpp)    
备注：窗口在XML中的标签名称是："Window"     
使用示例：    
```xml
<Window size="75%,90%" min_size="80,50" size_box="4,4,4,4"
        caption="0,0,0,36" shadow_type="system_default">
</Window>
```

## 窗口的 C++ 接口（常用）

除 XML 属性外，窗口还提供了一组 C++ 接口，用于在代码中创建/显示窗口、查找控件、调整外观、处理事件等。
接口声明位于 [Window.h](../duilib/Core/Window.h) 与 [WindowBase.h](../duilib/Core/WindowBase.h)；
示例程序中的窗口一般继承 `WindowImplBase`（[WinImplBase.h](../duilib/Utils/WinImplBase.h)），它提供了窗口创建流程的常用默认实现。

### 1. 创建、显示与关闭

| 接口 | 说明 |
| :--- | :--- |
| `bool CreateWnd(WindowBase* pParentWindow, const WindowCreateParam& createParam)` | 创建普通窗口（非模态）；`WindowCreateParam` 可指定窗口标题、是否居中、窗口风格等 |
| `int32_t DoModal(WindowBase* pParentWindow, const WindowCreateParam& createParam, bool bCloseByEsc = true, bool bCloseByEnter = false)` | 创建并模态显示窗口，阻塞父窗口；返回关闭参数（`kWindowCloseNormal`/`kWindowCloseOK`/`kWindowCloseCancel`），创建失败返回 -1；该窗口销毁时不会自动 delete 对象 |
| `bool CreateChildWnd(WindowBase* pParentWindow, int32_t nX, int32_t nY, int32_t nWidth, int32_t nHeight)` | 创建子窗口（Windows 平台为 `WS_CHILD` 风格），坐标相对于父窗口 |
| `bool ShowWindow(ShowWindowCommands nCmdShow)` | 显示/隐藏/最大化/最小化/还原窗口，如 `ui::kSW_SHOW_NORMAL`、`ui::kSW_SHOW_MAXIMIZED`、`ui::kSW_MINIMIZE`、`ui::kSW_RESTORE`、`ui::kSW_HIDE` |
| `void ShowModalFake()` | 以“模拟模态”方式显示窗口（把父窗口设为 Disable），不阻塞调用线程；可用 `IsFakeModal()` 判断 |
| `void CloseWnd(int32_t nRet = kWindowCloseNormal)` | 异步关闭窗口（投递关闭消息，函数返回后 `IsClosingWnd()` 为 true） |
| `void Close()` | 同步关闭窗口 |
| `void CenterWindow()` | 居中显示（支持多显示器） |
| `bool SetWindowPos(const InsertAfterWnd& insertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, uint32_t uFlags)` | 设置窗口位置与大小（对 `::SetWindowPos` 的封装，内部不做 DPI 缩放） |
| `bool MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint)` | 移动并改变窗口大小 |
| `void Resize(int32_t cx, int32_t cy, bool bContainShadow, bool bNeedDpiScale)` | 调整窗口大小；`bContainShadow` 表示宽高是否包含阴影 |
| `bool EnterFullscreen()` / `bool ExitFullscreen()` / `bool IsWindowFullscreen() const` | 进入/退出/判断全屏；全屏状态下按 ESC 默认退出全屏 |
| `void SetWindowAlwaysOnTop(bool bOnTop)` / `bool IsWindowAlwaysOnTop() const` | 设置/判断是否置顶窗口 |
| `bool SetWindowForeground()` / `bool IsWindowForeground() const` | 将窗口设为前端窗口 / 判断是否为前端窗口 |
| `bool SetWindowFocus()` / `bool KillWindowFocus()` / `bool IsWindowFocused() const` | 设置/取消/判断窗口是否为焦点窗口 |
| `bool EnableWindow(bool bEnable)` / `bool IsWindowEnabled() const` / `bool IsWindowVisible() const` | 启用/禁用窗口，以及判断窗口的启用、可见状态 |
| `bool IsWindowMaximized() const` / `bool IsWindowMinimized() const` | 判断窗口是否最大化/最小化 |

创建并显示主窗口的典型代码（来自 `examples/DpiAware/MainThread.cpp`）：

```cpp
MainForm* pWindow = new MainForm();                 //MainForm 继承 ui::WindowImplBase
pWindow->CreateWnd(nullptr, ui::WindowCreateParam(_T("DpiAware"), true)); //第二个参数 true 表示居中
pWindow->PostQuitMsgWhenClosed(true);               //窗口关闭时退出消息循环
pWindow->ShowWindow(ui::kSW_SHOW_NORMAL);
```

模态显示窗口的典型代码（来自 `examples/controls/ControlForm.cpp`）：

```cpp
ui::WindowCreateParam createParam;
createParam.m_dwStyle = ui::kWS_POPUP;
createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
createParam.m_bCenterWindow = true;

MyDialog dlg;                        //继承 ui::WindowImplBase（或 ui::Dialog）
dlg.DoModal(this, createParam);      //阻塞，直到窗口关闭
//如果改用 CreateWnd + ShowModalFake()，窗口不会阻塞调用线程
```

> `Window::OnFinalMessage()` 的默认实现会在非 `DoModal` 窗口中执行 `delete this`，因此普通窗口关闭后不需要手动 delete；通过 `DoModal` 显示的窗口需由调用方自行管理对象生命周期（可参考 `ui::Dialog` 的用法）。

### 2. 查找控件与焦点

| 接口 | 说明 |
| :--- | :--- |
| `Control* FindControl(const DString& strName) const` | 按 `name` 属性查找控件（区分大小写），从窗口根容器开始搜索 |
| `Control* FindControl(const UiPoint& pt) const` | 按客户区坐标查找控件（可见、可命中、最上层） |
| `Control* FindSubControlByName(Control* pParent, const DString& strName) const` | 在指定父控件的子树中按名称查找 |
| `Control* FindSubControlByPoint(Control* pParent, const UiPoint& pt) const` | 在指定父控件的子树中按坐标查找 |
| `Control* GetFocusControl() const` / `void SetFocusControl(Control* pControl)` / `void KillFocusControl()` | 获取/设置/清除控件焦点 |
| `bool SetNextTabControl(bool bForward = true)` | 按 TAB 顺序切换焦点到下一个（或上一个）控件 |
| `void SetCheckSetWindowFocus(bool bCheckSetWindowFocus)` | 设置控件焦点时是否同时把窗口设为焦点窗口 |
| `Control* GetHoverControl() const` / `Control* GetEventClick() const` | 获取当前鼠标悬停控件 / 当前接收点击事件的控件 |

```cpp
ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_ok")));
if (pBtn != nullptr) {
    pBtn->AttachClick([](const ui::EventArgs&) { return true; });
}
```

### 3. 窗口外观、阴影与 DPI

| 接口 | 说明 |
| :--- | :--- |
| `void SetShadowAttached(bool bShadowAttached)` / `bool IsShadowAttached() const` | 是否附加窗口阴影 |
| `void SetShadowType(ShadowType nShadowType)` / `ShadowType GetShadowType() const` | 设置阴影类型，取值如 `ShadowType::kShadowBig`、`kShadowSmallRound`、`kShadowMenu`、`kShadowNone`、`kShadowCustom`、`kShadowSystemDefault`（与 XML 中的 `shadow_type` 字符串对应） |
| `void SetShadowImage(const DString& shadowImage)` | 设置自定义阴影图片（`kShadowCustom` 时使用） |
| `void SetShadowCorner(const UiPadding& rcShadowCorner)` | 设置阴影图片的九宫格（未做 DPI 缩放） |
| `void SetShadowBorderRound(UiSize szBorderRound)` / `void SetShadowBorderSize(int32_t nShadowBorderSize)` / `void SetShadowBorderColor(const DString& shadowBorderColor)` | 设置阴影圆角、边框粗细、边框颜色 |
| `void SetEnableShadowSnap(bool bEnable)` | 是否允许窗口贴边时自动隐藏该侧阴影（仅自绘阴影有效） |
| `void SetRoundCorner(int cx, int cy, bool bNeedDpiScale)` | 设置窗口圆角大小 |
| `void SetSizeBox(const UiRect& rcSizeBox, bool bNeedDpiScale)` | 设置窗口四边可拉伸区域 |
| `void SetCaptionRect(const UiRect& rcCaption, bool bNeedDpiScale)` | 设置标题栏可拖动区域 |
| `void SetSysMenuRect(const UiRect& rcSysMenuRect, bool bNeedDpiScale)` | 设置系统菜单区域（点击显示系统菜单，双击关闭窗口） |
| `void SetEnableSnapLayoutMenu(bool bEnable)` / `void SetEnableSysMenu(bool bEnable)` | 是否显示贴靠布局菜单 / 右键标题栏的系统菜单 |
| `bool SetLayeredWindow(bool bIsLayeredWindow, bool bRedraw)` / `bool IsLayeredWindow() const` | 设置/判断是否为分层窗口 |
| `bool SetLayeredWindowAlpha(int32_t nAlpha)` / `bool SetLayeredWindowOpacity(int32_t nAlpha)` | 设置窗口透明度的两种方式（取值 [0,255]，仅在分层窗口有效；`Opacity` 会自动切换为分层窗口） |
| `void SetWindowMaximumSize(const UiSize& szMaxWindow, bool bNeedDpiScale)` / `void SetWindowMinimumSize(const UiSize& szMinWindow, bool bNeedDpiScale)` | 设置窗口最大/最小尺寸（值为 0 表示不限制） |
| `bool SetWindowIcon(const DString& iconFilePath)` | 设置窗口图标（路径相对于资源根目录，推荐 ico 格式） |
| `void SetText(const DString& strText)` / `void SetTextId(const DString& strTextId)` | 设置窗口标题文字 / 按多语言 ID 设置标题 |
| `bool ChangeDisplayScale(uint32_t nNewDisplayScaleFactor, bool bDisableDpiAware = true)` | 主动修改窗口的界面显示比例（如 100 表示 100%） |
| `const DpiManager& Dpi() const` | 获取窗口的 DPI 管理器，用于做 DPI 换算 |

> 阴影相关的深入说明见 [WindowShadow.md](WindowShadow.md)。

### 4. 样式、主题颜色与多语言

| 接口 | 说明 |
| :--- | :--- |
| `void AddClass(const DString& strClassName, const DString& strControlAttrList)` / `DString GetClassAttributes(const DString& strClassName) const` | 为窗口添加/读取通用样式（class） |
| `bool RemoveClass(const DString& strClassName)` / `void RemoveAllClass()` | 删除指定/全部通用样式 |
| `void AddThemeColor(const DString& strName, const DString& strValue)` / `void AddThemeColor(const DString& strName, UiColor argb)` | 为窗口添加固定颜色（不随主题切换） |
| `void RemoveThemeColor(const DString& strName)` / `UiColor GetThemeColor(const DString& strName) const` | 删除/读取窗口颜色 |
| `bool OpenColorTheme(const FilePath& themePath)` / `bool OpenColorThemeData(const std::string& themeXmlFileData)` | 为窗口打开独立的颜色主题（如 `color_light`、`color_dark`），不跟随全局颜色管理器 |
| `void CloseColorTheme()` / `bool IsColorThemeDarkMode() const` | 关闭窗口私有颜色主题 / 判断当前是否为深色主题 |
| `void NotifyLanguageChanged()` | 通知窗口语言已切换：更新标题、重新应用语言资源并重新布局，然后发送 `kWindowLanguageChangedMsg` |
| `void NotifyThemeChanged()` | 通知窗口主题已切换：重新应用主题并重新布局，发送 `kWindowThemeChangedMsg` |

### 5. 坐标、鼠标捕获与消息

| 接口 | 说明 |
| :--- | :--- |
| `void GetClientRect(UiRect& rcClient) const` / `void GetWindowRect(UiRect& rcWindow) const` | 获取客户区矩形 / 窗口矩形（屏幕坐标） |
| `void ScreenToClient(UiPoint& pt) const` / `void ClientToScreen(UiPoint& pt) const` | 屏幕坐标与客户区坐标互转（同样提供 `UiRect` 重载） |
| `void GetCursorPos(UiPoint& pt) const` | 获取鼠标当前位置（屏幕坐标） |
| `bool GetMonitorRect(UiRect& rcMonitor) const` / `bool GetMonitorWorkRect(UiRect& rcWork) const` | 获取窗口所在显示器的矩形 / 工作区矩形 |
| `void SetCapture()` / `void ReleaseCapture()` / `bool IsCaptured() const` | 捕获/释放鼠标输入，以及判断是否已捕获 |
| `int32_t SetWindowHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)` | 设置窗口激活热键（注册后按热键可激活窗口）；返回值 0 表示窗口无效、1 表示成功、2 表示与其他窗口冲突 |
| `bool RegisterHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers, int32_t id)` / `bool UnregisterHotKey(int32_t id)` | 注册/注销系统全局热键，触发后窗口收到 `WM_HOTKEY` |
| `bool AddMessageFilter(IUIMessageFilter* pFilter)` / `bool RemoveMessageFilter(IUIMessageFilter* pFilter)` | 添加/移除消息过滤器（优先于窗口自身的消息处理） |
| `LRESULT PostMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0)` | 向窗口消息队列投递消息 |

### 6. 事件回调

窗口事件通过 `AttachWindowXxxMsg` 注册，回调类型为 `EventCallback`；可传入 `callbackID`，之后用 `DetachWindowEventCallbackByID` 注销。常用事件：

| 接口 | 触发时机 / `wParam` 含义 |
| :--- | :--- |
| `AttachWindowCreateMsg` | 窗口创建完成；`wParam` 为 1 表示通过 `DoModal` 创建，为 0 表示普通窗口 |
| `AttachWindowCloseMsg` | 窗口关闭；`wParam` 为关闭参数（`kWindowCloseNormal`/`kWindowCloseOK`/`kWindowCloseCancel`） |
| `AttachWindowFirstShown` | 窗口首次显示（界面布局已完成） |
| `AttachWindowShowWindowMsg` | 窗口显示/隐藏；`wParam` 为 1 显示、0 隐藏 |
| `AttachWindowEnterFullscreenMsg` / `AttachWindowExitFullscreenMsg` | 进入/退出全屏 |
| `AttachWindowMaximizedMsg` / `AttachWindowMinimizedMsg` / `AttachWindowRestoredMsg` | 最大化/最小化/还原 |
| `AttachWindowPosChangedMsg` / `AttachWindowSizeMsg` / `AttachWindowMoveMsg` | 位置大小变化 / 大小变化 / 移动；`Size` 事件的 `wParam` 为 `WindowSizeType` |
| `AttachWindowSetFocusMsg` / `AttachWindowKillFocusMsg` | 获得/失去焦点；`wParam` 为对应窗口指针 |
| `AttachWindowKeyDownMsg` / `AttachWindowKeyUpMsg` | 键盘按下/弹起 |
| `AttachWindowMouseWheelMsg` / `AttachWindowMouseMoveMsg` | 滚轮 / 鼠标移动 |
| `AttachWindowLButtonDownMsg` 等 | 鼠标左/右/中键的按下、弹起、双击事件 |
| `AttachWindowCaptureChangedMsg` | 丢失鼠标捕获 |
| `AttachWindowDropEnterMsg` / `AttachWindowDropOverMsg` / `AttachWindowDropMsg` / `AttachWindowDropLeaveMsg` | 拖放进入/悬停/放下/离开 |
| `AttachWindowDisplayScaleChangedMsg` | 窗口 DPI 变化；`wParam` 为 `WindowDisplayScaleData*` |
| `AttachWindowDisplayResolutionChangedMsg` | 显示器分辨率变化；`wParam` 为 `WindowDisplayResolutionData*` |
| `AttachWindowLanguageChangedMsg` / `AttachWindowThemeChangedMsg` | 语言切换 / 主题切换 |

`HasWindowEventCallback(EventType)`、`DetachWindowEventCallback(EventType)`、`DetachWindowEventCallbackByID(EventCallbackID)` 用于查询和注销回调。
事件类型常量与控件事件详见 [Events.md](Events.md)，XML 中的事件绑定详见 [XmlEvents.md](XmlEvents.md)。

```cpp
AttachWindowCloseMsg([this](const ui::EventArgs&) {
    //窗口关闭时的处理
    return true;
    });
```

### 7. 生命周期虚函数

`Window`/`WindowImplBase` 在窗口创建与关闭的不同阶段调用下列虚函数，子类可重写：

| 虚函数 | 调用时机 / 用途 |
| :--- | :--- |
| `virtual DString GetSkinFolder()` | 创建窗口时获取皮肤目录（相对资源根目录） |
| `virtual DString GetSkinFile()` | 创建窗口时获取皮肤 XML 文件路径，或直接返回 XML 内容（以 '<' 开头） |
| `virtual Control* CreateControl(const DString& strClass)` | XML 中出现未注册的控件名称时调用，用于创建自定义控件 |
| `virtual void PreInitWindow()` | 初始化窗口数据之前；若重写必须调用基类实现 |
| `virtual void OnInitWindow()` | 窗口创建完成后；常用于注册事件、查找控件、初始化数据 |
| `virtual void PostInitWindow()` | 窗口初始化完成之后 |
| `virtual void OnInitLayout()` | 界面布局（控件位置与大小）完成初始化后调用 |
| `virtual void OnPreCloseWindow()` | 窗口即将关闭时；用于注销资源、保存状态 |
| `virtual void OnCloseWindow()` | 窗口已经关闭后 |
| `virtual void OnFinalMessage()` | 窗口销毁时的最后一个消息；默认实现（非 `DoModal` 窗口）会 `delete this`，不需要自动销毁时可重写 |
| `virtual bool OnLanguageChanged()` / `virtual bool OnThemeChanged()` | 语言/主题切换；返回 false 表示不再发送对应的 `kWindowLanguageChangedMsg`/`kWindowThemeChangedMsg` |
| `virtual void OnWindowEnterFullscreen()` / `virtual void OnWindowExitFullscreen()` | 进入/退出全屏 |
| `virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)` | 窗口 DPI 缩放比变化，用于更新控件大小与布局 |

典型的重写示例（参考 `examples/DpiAware/MainForm.cpp`）：

```cpp
class MainForm : public ui::WindowImplBase
{
public:
    virtual DString GetSkinFolder() override { return _T("dpi_aware"); }
    virtual DString GetSkinFile() override { return _T("DpiAware.xml"); }

    virtual void OnInitWindow() override
    {
        ui::WindowImplBase::OnInitWindow();  //必须调用基类实现
        ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_ok")));
        if (pBtn != nullptr) {
            pBtn->AttachClick([](const ui::EventArgs&) { return true; });
        }
    }

    virtual void OnPreCloseWindow() override
    {
        //保存状态、注销回调
        ui::WindowImplBase::OnPreCloseWindow();
    }
};
```