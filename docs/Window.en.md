English | [简体中文](Window.md)

> Last synced: 2026-11-02

## Window Attributes
| Attribute name          | Attribute category| Default value  |Parameter type| Function in [Window.h](../duilib/Core/Window.h) | Purpose |
| :---              | :---    | :---    | :---   |:---                     |:--- |
| caption           | Title bar  | "0,0,0,0" | rect   | SetCaptionRect          | Margin of the draggable title bar area of the window; the last parameter is the distance from the top border, e.g. "0,0,0,36" |
| use_system_caption| Title bar  | false   | bool   | SetUseSystemCaption     | Set whether to use the system title bar |
| snap_layout_menu  | Title bar  | true    | bool   | SetEnableSnapLayoutMenu | Whether to support displaying the snap layout menu (new Windows 11 feature: by hovering the mouse over the window's maximize button or pressing Win + Z, aligned layouts can be easily accessed). This attribute is enabled by default only when the window is a layered window or uses the system shadow |
| sys_menu          | Title bar  | true    | bool   | SetEnableSysMenu        | Whether to display the system window menu when right-clicking the title bar (can adjust window state, close window, etc.)|
| sys_menu_rect     | Title bar  | "0,0,0,0" | rect   | SetSysMenuRect          | Window menu area, at the top-left corner of the window title bar (double-clicking this area closes the window, clicking displays the system window menu). If not set in XML, this feature is disabled by default |
| icon              | Title bar  |         | string | SetWindowIcon           | Set the window icon file path, supports ico format |
| text              | Title bar  |         | string | SetText                 | Window title string|
| text_id           | Title bar  |         | string | SetTextId               | ID of the window title string; the ID is specified in the multilingual file, e.g. "STRID_MIANWINDOW_TITLE" |
| drag_drop         | Drag and drop    | true    | bool   | SetEnableDragDrop       | Set whether the controls inside the window support drag-and-drop operations (drop in files/text): true = supported, false = not supported |
| shadow_attached   | Window shadow| true    | bool   | SetShadowAttached       | Whether the window has an attached shadow effect; "true" means enable the window shadow function, "false" means disable it<br>If the shadow_type attribute is set, it implicitly sets shadow_attached to "true"<br>So generally after setting shadow_type, there is no need to set shadow_attached separately|
| shadow_type       | Window shadow|         | string | SetShadowType           | Set the window shadow type:<br> "default": default self-drawn shadow <br> "big": self-drawn shadow (large), right angle, with border (suitable for normal windows)<br> "big_round": self-drawn shadow (large), rounded corner, with border (suitable for normal windows)<br> "small": self-drawn shadow (small), right angle, with border (suitable for normal windows)<br> "small_round": self-drawn shadow (small), rounded corner, with border (suitable for normal windows)<br> "menu": self-drawn shadow (small), right angle, with border (suitable for popup windows, such as menus)<br> "menu_round": self-drawn shadow (small), rounded corner, with border (suitable for popup windows, such as menus)<br> "none": no shadow, right angle, with border<br> "none_round": no shadow, rounded corner, with border (self-drawn)<br> "custom": custom self-drawn shadow; this type requires setting `shadow_image`, `shadow_corner`, `shadow_border_round` at the same time <br> "system_default": use the system shadow, following the OS default window shadow properties<br> "system_not_round": use the system shadow, right angle<br> "system_round": use the system shadow, rounded corner<br> "system_small_round": use the system shadow, small rounded corner<br>Compatibility notes:<br>(1) For Win7 and Win10 systems: only "system_default" is supported among the system shadows; the other three are not supported<br>(2) For Win11 systems, all four system shadow attributes are supported, and the default system shadow is rounded corner<br>(3) For macOS systems, all four system shadow attributes are supported, and the default system shadow is rounded corner<br>(4) For Linux and FreeBSD systems, system shadows are not supported<br>(5) If an unsupported system shadow is set, it will automatically switch to the corresponding supported system shadow or self-drawn shadow|
| shadow_image      | Window shadow|         | string | SetShadowImage          | Use a custom shadow image to replace the default shadow effect; this attribute is generally only set when shadow_type="custom"<br>Note the relative path and the nine-grid (9-slice) attributes for the set path<br>e.g. (file='public/shadow/shadow_big.svg' corner='64,64,68,70') |
| shadow_corner     | Window shadow| "0,0,0,0" | rect   | SetShadowCorner       | After setting the shadow_image attribute, set this attribute to specify the nine-grid description of the shadow material; this attribute is generally only set when shadow_type="custom" |
| shadow_border_round| Window shadow| "0,0"    | size  | SetShadowBorderRound| After setting the shadow_image attribute, set this attribute to specify the rounded-corner attribute of the shadow; this attribute is generally only set when shadow_type="custom" |
| shadow_border_color| Window shadow|          |string | SetShadowBorderColor  | Set the border color of the window shadow |
| shadow_border_size | Window shadow|2         |int    | SetShadowBorderSize   | Set the border pixel size of the window shadow; the actually displayed border width is half of this value (e.g. set to 2, actually displays 1 pixel) |
| shadow_snap        | Window shadow| true     | bool  | SetEnableShadowSnap   | Set whether the shadow supports the window snap-to-edge operation; if true, when the window is close to the screen edge, the shadow on that side is automatically hidden to increase the effective space within the view<br>This attribute is only effective when using self-drawn shadow, and is invalid when using system shadow|
| size              | Window size| "0,0"     | size   | SetInitSize             | Initial size of the window; supported formats: size="1200,800", or size="50%,50%", or size="1200,50%", size="50%,800"; the percentage refers to the percentage of the screen width or height |
| size_contain_shadow| Window size| false  | bool   |    | Whether the window's initial size (size attribute) includes the window shadow; by default it does not, and the actual window size is the configured size value + shadow size |
| min_size          | Window size| "0,0"     | size   | SetWindowMinimumSize    | Minimum window size, e.g. "320,240" |
| max_size          | Window size| "0,0"     | size   | SetWindowMaximumSize    | Maximum window size, e.g. "1600,1200" |
| size_box          | Window size| "0,0,0,0" | rect   | SetSizeBox              | Margin of the window area that can be dragged to resize, e.g. "4,4,4,4" |
| round_corner      | Window shape| "0,0"     | size   | SetRoundCorner          | Window corner radius, e.g. "4,4"; this attribute generally does not need to be set when using window shadow (i.e. when the shadow_type attribute is set) |
| layered_window    | Window rendering| false   | bool   | SetLayeredWindow        | Set whether it is a layered window. Usage notes:<br>(1) This attribute does not need to be set when using window shadow (i.e. when the shadow_type attribute is set); the window shadow function will manage this attribute as needed<br>(2) When using the system title bar (i.e. use_system_caption="true"), this attribute should not be set |
| alpha             | Window rendering| 255     | int    | SetLayeredWindowAlpha   | Set the transparency value [0, 255]; when alpha is 0, the window is completely transparent. When alpha is 255, the window is opaque.<br>Only effective when layered_window="true"<br>This parameter is used as an argument in the UpdateLayeredWindow function (BLENDFUNCTION.SourceConstantAlpha)|
| opacity           | Window rendering| 255     | int    | SetLayeredWindowOpacity | Set the opacity value [0, 255]; when opacity is 0, the window is completely transparent. When opacity is 255, the window is opaque.<br>Only effective when layered_window="true", so if it is not currently a layered window, it will be automatically set to a layered window internally<br>This parameter is used as an argument in the SetLayeredWindowAttributes function (bAlpha)|
| render_backend_type|Window rendering| "CPU"   | string |SetRenderBackendType     | "CPU": CPU rendering <br> "GL": use OpenGL rendering <br> Notes:<br>(1) Multiple windows in the same thread may use OpenGL rendering; the current context is switched before each draw. Note that each window creates its own GL context, so watch video memory usage<br>(2) A window rendered with OpenGL cannot be a layered window (i.e. a window with the WS_EX_LAYERED attribute), so when using GL rendering, the `layered_window`, `alpha`, `opacity` attributes are all invalid<br>(3) OpenGL rendering uses a persistent FBO for partial updates, so small animated regions do not redraw the whole window; performance is usually better than full-window redraw |
| vsync             | Window rendering| "true"  | bool   | SetRenderBackendType    | Whether to enable vertical sync; false disables it.<br>Only effective when render_backend_type="GL" |
| msaa              | Window rendering| "0"     | int    | SetRenderBackendType    | MSAA sample count; 0 means MSAA is disabled (1x sampling).<br>Only effective when render_backend_type="GL" |

Note: for content related to window shadows, please refer to the document: [WindowShadow.en.md](WindowShadow.en.md)    
Note: the parsing function for window attributes can be found in: [WindowBuilder::ParseWindowAttributes function](../duilib/Core/WindowBuilder.cpp)    
Note: the tag name of the window in XML is: "Window"    
Usage example:    
```xml
<Window size="75%,90%" min_size="80,50" size_box="4,4,4,4"
        caption="0,0,0,36" shadow_type="system_default">
</Window>
```

## Window C++ interfaces (common)

Besides XML attributes, the window class exposes C++ interfaces for creating/showing windows, finding
controls, adjusting appearance, handling events, and more. The declarations are in
[Window.h](../duilib/Core/Window.h) and [WindowBase.h](../duilib/Core/WindowBase.h). Windows in the
sample programs usually derive from `WindowImplBase` ([WinImplBase.h](../duilib/Utils/WinImplBase.h)),
which provides the common default implementation of the window creation flow.

### 1. Creating, showing and closing

| Interface | Description |
| :--- | :--- |
| `bool CreateWnd(WindowBase* pParentWindow, const WindowCreateParam& createParam)` | Creates a normal (non-modal) window; `WindowCreateParam` specifies the window title, whether to center it, window styles, and so on |
| `int32_t DoModal(WindowBase* pParentWindow, const WindowCreateParam& createParam, bool bCloseByEsc = true, bool bCloseByEnter = false)` | Creates and shows a modal window, blocking the parent window; returns the close parameter (`kWindowCloseNormal`/`kWindowCloseOK`/`kWindowCloseCancel`), or -1 if creation fails; the object is **not** deleted automatically when such a window is destroyed |
| `bool CreateChildWnd(WindowBase* pParentWindow, int32_t nX, int32_t nY, int32_t nWidth, int32_t nHeight)` | Creates a child window (a `WS_CHILD` style window on Windows); coordinates are relative to the parent window |
| `bool ShowWindow(ShowWindowCommands nCmdShow)` | Shows/hides/maximizes/minimizes/restores the window, e.g. `ui::kSW_SHOW_NORMAL`, `ui::kSW_SHOW_MAXIMIZED`, `ui::kSW_MINIMIZE`, `ui::kSW_RESTORE`, `ui::kSW_HIDE` |
| `void ShowModalFake()` | Shows the window in "fake modal" mode (disables the parent window) without blocking the calling thread; use `IsFakeModal()` to check |
| `void CloseWnd(int32_t nRet = kWindowCloseNormal)` | Closes the window asynchronously (posts a close message; `IsClosingWnd()` returns true after the call returns) |
| `void Close()` | Closes the window synchronously |
| `void CenterWindow()` | Centers the window (supports multiple monitors) |
| `bool SetWindowPos(const InsertAfterWnd& insertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, uint32_t uFlags)` | Sets the window position and size (a wrapper around `::SetWindowPos`; no DPI scaling is applied internally) |
| `bool MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint)` | Moves the window and changes its size |
| `void Resize(int32_t cx, int32_t cy, bool bContainShadow, bool bNeedDpiScale)` | Resizes the window; `bContainShadow` indicates whether the width/height includes the shadow |
| `bool EnterFullscreen()` / `bool ExitFullscreen()` / `bool IsWindowFullscreen() const` | Enters/exits/checks fullscreen; ESC exits fullscreen by default |
| `void SetWindowAlwaysOnTop(bool bOnTop)` / `bool IsWindowAlwaysOnTop() const` | Sets/checks whether the window is always on top |
| `bool SetWindowForeground()` / `bool IsWindowForeground() const` | Brings the window to the foreground / checks whether it is in the foreground |
| `bool SetWindowFocus()` / `bool KillWindowFocus()` / `bool IsWindowFocused() const` | Sets/clears/checks whether the window has keyboard focus |
| `bool EnableWindow(bool bEnable)` / `bool IsWindowEnabled() const` / `bool IsWindowVisible() const` | Enables/disables the window and checks its enabled/visible state |
| `bool IsWindowMaximized() const` / `bool IsWindowMinimized() const` | Checks whether the window is maximized/minimized |

Typical code for creating and showing a main window (from `examples/DpiAware/MainThread.cpp`):

```cpp
MainForm* pWindow = new MainForm();                 //MainForm derives from ui::WindowImplBase
pWindow->CreateWnd(nullptr, ui::WindowCreateParam(_T("DpiAware"), true)); //second parameter true = centered
pWindow->PostQuitMsgWhenClosed(true);               //quit the message loop when the window closes
pWindow->ShowWindow(ui::kSW_SHOW_NORMAL);
```

Typical code for showing a window modally (from `examples/controls/ControlForm.cpp`):

```cpp
ui::WindowCreateParam createParam;
createParam.m_dwStyle = ui::kWS_POPUP;
createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
createParam.m_bCenterWindow = true;

MyDialog dlg;                        //derives from ui::WindowImplBase (or use ui::Dialog)
dlg.DoModal(this, createParam);      //blocks until the window closes
//Using CreateWnd + ShowModalFake() instead does not block the calling thread
```

> The default implementation of `Window::OnFinalMessage()` executes `delete this` for windows that
> were not shown via `DoModal`, so a normal window needs no manual delete after it closes. Windows
> shown via `DoModal` must have their lifetime managed by the caller (see the usage of `ui::Dialog`).

### 2. Finding controls and focus

| Interface | Description |
| :--- | :--- |
| `Control* FindControl(const DString& strName) const` | Finds a control by its `name` attribute (case-sensitive), searching from the window root container |
| `Control* FindControl(const UiPoint& pt) const` | Finds a control by client-area coordinate (visible, hit-testable, topmost) |
| `Control* FindSubControlByName(Control* pParent, const DString& strName) const` | Finds a control by name inside the subtree of the given parent |
| `Control* FindSubControlByPoint(Control* pParent, const UiPoint& pt) const` | Finds a control by coordinate inside the subtree of the given parent |
| `Control* GetFocusControl() const` / `void SetFocusControl(Control* pControl)` / `void KillFocusControl()` | Gets/sets/clears the control focus |
| `bool SetNextTabControl(bool bForward = true)` | Moves focus to the next (or previous) control in TAB order |
| `void SetCheckSetWindowFocus(bool bCheckSetWindowFocus)` | Sets whether setting the control focus also makes the window the focused window |
| `Control* GetHoverControl() const` / `Control* GetEventClick() const` | Gets the control currently under the mouse / the control currently receiving the click event |

```cpp
ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_ok")));
if (pBtn != nullptr) {
    pBtn->AttachClick([](const ui::EventArgs&) { return true; });
}
```

### 3. Appearance, shadow and DPI

| Interface | Description |
| :--- | :--- |
| `void SetShadowAttached(bool bShadowAttached)` / `bool IsShadowAttached() const` | Whether the window has an attached shadow |
| `void SetShadowType(ShadowType nShadowType)` / `ShadowType GetShadowType() const` | Sets the shadow type, e.g. `ShadowType::kShadowBig`, `kShadowSmallRound`, `kShadowMenu`, `kShadowNone`, `kShadowCustom`, `kShadowSystemDefault` (corresponding to the `shadow_type` XML strings) |
| `void SetShadowImage(const DString& shadowImage)` | Sets the custom shadow image (used with `kShadowCustom`) |
| `void SetShadowCorner(const UiPadding& rcShadowCorner)` | Sets the nine-grid (9-slice) description of the shadow image (without DPI scaling) |
| `void SetShadowBorderRound(UiSize szBorderRound)` / `void SetShadowBorderSize(int32_t nShadowBorderSize)` / `void SetShadowBorderColor(const DString& shadowBorderColor)` | Sets the shadow corner radius, border width, and border color |
| `void SetEnableShadowSnap(bool bEnable)` | Whether the shadow on the side facing a screen edge is hidden automatically when the window snaps to that edge (self-drawn shadow only) |
| `void SetRoundCorner(int cx, int cy, bool bNeedDpiScale)` | Sets the window corner radius |
| `void SetSizeBox(const UiRect& rcSizeBox, bool bNeedDpiScale)` | Sets the resize borders of the window |
| `void SetCaptionRect(const UiRect& rcCaption, bool bNeedDpiScale)` | Sets the draggable title bar area |
| `void SetSysMenuRect(const UiRect& rcSysMenuRect, bool bNeedDpiScale)` | Sets the system menu area (click shows the system menu, double-click closes the window) |
| `void SetEnableSnapLayoutMenu(bool bEnable)` / `void SetEnableSysMenu(bool bEnable)` | Whether to show the snap layout menu / the system menu on right-clicking the title bar |
| `bool SetLayeredWindow(bool bIsLayeredWindow, bool bRedraw)` / `bool IsLayeredWindow() const` | Sets/checks whether the window is a layered window |
| `bool SetLayeredWindowAlpha(int32_t nAlpha)` / `bool SetLayeredWindowOpacity(int32_t nAlpha)` | Two ways to set window transparency (value in [0,255], layered windows only; `Opacity` switches the window to layered automatically) |
| `void SetWindowMaximumSize(const UiSize& szMaxWindow, bool bNeedDpiScale)` / `void SetWindowMinimumSize(const UiSize& szMinWindow, bool bNeedDpiScale)` | Sets the maximum/minimum window size (a value of 0 means no limit) |
| `bool SetWindowIcon(const DString& iconFilePath)` | Sets the window icon (path relative to the resource root; ico format is recommended) |
| `void SetText(const DString& strText)` / `void SetTextId(const DString& strTextId)` | Sets the window title text / sets the title from a multilingual ID |
| `bool ChangeDisplayScale(uint32_t nNewDisplayScaleFactor, bool bDisableDpiAware = true)` | Changes the window's UI display scale (e.g. 100 means 100%) |
| `const DpiManager& Dpi() const` | Gets the window's DPI manager for DPI conversions |

> For details about shadows, see [WindowShadow.en.md](WindowShadow.en.md).

### 4. Styles, theme colors and multilingual support

| Interface | Description |
| :--- | :--- |
| `void AddClass(const DString& strClassName, const DString& strControlAttrList)` / `DString GetClassAttributes(const DString& strClassName) const` | Adds/reads a common style (class) for the window |
| `bool RemoveClass(const DString& strClassName)` / `void RemoveAllClass()` | Removes the specified/all common styles |
| `void AddThemeColor(const DString& strName, const DString& strValue)` / `void AddThemeColor(const DString& strName, UiColor argb)` | Adds a fixed color to the window (not affected by theme switching) |
| `void RemoveThemeColor(const DString& strName)` / `UiColor GetThemeColor(const DString& strName) const` | Removes/reads a window color |
| `bool OpenColorTheme(const FilePath& themePath)` / `bool OpenColorThemeData(const std::string& themeXmlFileData)` | Opens a private color theme for the window (e.g. `color_light`, `color_dark`) instead of using the global color manager |
| `void CloseColorTheme()` / `bool IsColorThemeDarkMode() const` | Closes the private color theme / checks whether the current theme is dark |
| `void NotifyLanguageChanged()` | Notifies the window that the language changed: updates the title, re-applies language resources and re-layouts, then sends `kWindowLanguageChangedMsg` |
| `void NotifyThemeChanged()` | Notifies the window that the theme changed: re-applies the theme and re-layouts, sends `kWindowThemeChangedMsg` |

### 5. Coordinates, mouse capture and messages

| Interface | Description |
| :--- | :--- |
| `void GetClientRect(UiRect& rcClient) const` / `void GetWindowRect(UiRect& rcWindow) const` | Gets the client rectangle / window rectangle (screen coordinates) |
| `void ScreenToClient(UiPoint& pt) const` / `void ClientToScreen(UiPoint& pt) const` | Converts between screen and client coordinates (there are also `UiRect` overloads) |
| `void GetCursorPos(UiPoint& pt) const` | Gets the current mouse position (screen coordinates) |
| `bool GetMonitorRect(UiRect& rcMonitor) const` / `bool GetMonitorWorkRect(UiRect& rcWork) const` | Gets the rectangle / work area of the monitor containing the window |
| `void SetCapture()` / `void ReleaseCapture()` / `bool IsCaptured() const` | Captures/releases mouse input, and checks whether it is captured |
| `int32_t SetWindowHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)` | Sets a window activation hotkey (pressing it activates the window); return 0 = invalid window, 1 = success, 2 = already used by another window |
| `bool RegisterHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers, int32_t id)` / `bool UnregisterHotKey(int32_t id)` | Registers/unregisters a global hotkey; when triggered the window receives `WM_HOTKEY` |
| `bool AddMessageFilter(IUIMessageFilter* pFilter)` / `bool RemoveMessageFilter(IUIMessageFilter* pFilter)` | Adds/removes a message filter (takes precedence over the window's own message handling) |
| `LRESULT PostMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0)` | Posts a message to the window's message queue |

### 6. Event callbacks

Window events are registered through the `AttachWindowXxxMsg` functions; the callback type is
`EventCallback`, and an optional `callbackID` can be passed for later removal with
`DetachWindowEventCallbackByID`. Common events:

| Interface | Trigger / meaning of `wParam` |
| :--- | :--- |
| `AttachWindowCreateMsg` | Window creation completed; `wParam` is 1 for a window created via `DoModal`, 0 for a normal window |
| `AttachWindowCloseMsg` | Window closing; `wParam` is the close parameter (`kWindowCloseNormal`/`kWindowCloseOK`/`kWindowCloseCancel`) |
| `AttachWindowFirstShown` | The window is shown for the first time (layout is complete) |
| `AttachWindowShowWindowMsg` | Window shown/hidden; `wParam` is 1 for shown, 0 for hidden |
| `AttachWindowEnterFullscreenMsg` / `AttachWindowExitFullscreenMsg` | Entering/exiting fullscreen |
| `AttachWindowMaximizedMsg` / `AttachWindowMinimizedMsg` / `AttachWindowRestoredMsg` | Maximized/minimized/restored |
| `AttachWindowPosChangedMsg` / `AttachWindowSizeMsg` / `AttachWindowMoveMsg` | Position/size changed / size changed / moved; the `Size` event's `wParam` is a `WindowSizeType` |
| `AttachWindowSetFocusMsg` / `AttachWindowKillFocusMsg` | Focus gained/lost; `wParam` is the related window pointer |
| `AttachWindowKeyDownMsg` / `AttachWindowKeyUpMsg` | Key down/up |
| `AttachWindowMouseWheelMsg` / `AttachWindowMouseMoveMsg` | Mouse wheel / mouse move |
| `AttachWindowLButtonDownMsg` etc. | Left/right/middle mouse button down, up and double-click events |
| `AttachWindowCaptureChangedMsg` | Mouse capture lost |
| `AttachWindowDropEnterMsg` / `AttachWindowDropOverMsg` / `AttachWindowDropMsg` / `AttachWindowDropLeaveMsg` | Drag-and-drop enter/hover/drop/leave |
| `AttachWindowDisplayScaleChangedMsg` | Window DPI change; `wParam` is a `WindowDisplayScaleData*` |
| `AttachWindowDisplayResolutionChangedMsg` | Monitor resolution change; `wParam` is a `WindowDisplayResolutionData*` |
| `AttachWindowLanguageChangedMsg` / `AttachWindowThemeChangedMsg` | Language change / theme change |

`HasWindowEventCallback(EventType)`, `DetachWindowEventCallback(EventType)`, and
`DetachWindowEventCallbackByID(EventCallbackID)` query and remove callbacks. For event type constants
and control events, see [Events.en.md](Events.en.md); for event binding in XML, see
[XmlEvents.en.md](XmlEvents.en.md).

```cpp
AttachWindowCloseMsg([this](const ui::EventArgs&) {
    //handle the window closing
    return true;
    });
```

### 7. Lifetime virtual functions

`Window`/`WindowImplBase` calls the following virtual functions at different stages of window creation
and closing; subclasses may override them:

| Virtual function | Call timing / purpose |
| :--- | :--- |
| `virtual DString GetSkinFolder()` | Returns the skin folder (relative to the resource root) when the window is created |
| `virtual DString GetSkinFile()` | Returns the skin XML file path, or the XML content directly (starting with '<') |
| `virtual Control* CreateControl(const DString& strClass)` | Called when an unregistered control name appears in XML, to create a custom control |
| `virtual void PreInitWindow()` | Before window data initialization; overrides must call the base implementation |
| `virtual void OnInitWindow()` | After the window is created; commonly used to register events, find controls, and initialize data |
| `virtual void PostInitWindow()` | After window initialization is completed |
| `virtual void OnInitLayout()` | After the UI layout (control positions and sizes) is initialized |
| `virtual void OnPreCloseWindow()` | Right before the window closes; used to release resources and save state |
| `virtual void OnCloseWindow()` | After the window has closed |
| `virtual void OnFinalMessage()` | The last message when the window is destroyed; the default implementation (for windows not shown via `DoModal`) executes `delete this`; override it if automatic destruction is not wanted |
| `virtual bool OnLanguageChanged()` / `virtual bool OnThemeChanged()` | Language/theme change; returning false suppresses the corresponding `kWindowLanguageChangedMsg`/`kWindowThemeChangedMsg` |
| `virtual void OnWindowEnterFullscreen()` / `virtual void OnWindowExitFullscreen()` | Entering/exiting fullscreen |
| `virtual void OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)` | Window DPI scale changed; used to update control sizes and layout |

Typical override example (based on `examples/DpiAware/MainForm.cpp`):

```cpp
class MainForm : public ui::WindowImplBase
{
public:
    virtual DString GetSkinFolder() override { return _T("dpi_aware"); }
    virtual DString GetSkinFile() override { return _T("DpiAware.xml"); }

    virtual void OnInitWindow() override
    {
        ui::WindowImplBase::OnInitWindow();  //must call the base implementation
        ui::Button* pBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_ok")));
        if (pBtn != nullptr) {
            pBtn->AttachClick([](const ui::EventArgs&) { return true; });
        }
    }

    virtual void OnPreCloseWindow() override
    {
        //save state, unregister callbacks
        ui::WindowImplBase::OnPreCloseWindow();
    }
};
```
