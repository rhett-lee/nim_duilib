#include "NativeWindow_Windows.h"
#include "duilib/Core/WindowBase.h"
#include "duilib/Utils/ApiWrapper.h"
#include "duilib/Render/IRender.h"
#include <CommCtrl.h>
#include <Olectl.h>

namespace ui {

NativeWindow::NativeWindow(INativeWindow* pOwner):
    m_pOwner(pOwner),
    m_hWnd(nullptr),
    m_hResModule(nullptr),
    m_hDcPaint(nullptr),
    m_bIsLayeredWindow(false),
    m_nWindowAlpha(255),
    m_bUseSystemCaption(false),
    m_bMouseCapture(false),
    m_bCloseing(false),
    m_bFakeModal(false),
    m_bFullScreen(false),
    m_dwLastStyle(0),
    m_ptLastMousePos(-1, -1),
    m_paintStruct({0, })
{
    ASSERT(m_pOwner != nullptr);
    m_rcLastWindowPlacement = { sizeof(WINDOWPLACEMENT), };
}

NativeWindow::~NativeWindow()
{
    ASSERT(m_hWnd == nullptr);
    ClearNativeWindow();
}

bool NativeWindow::CreateWnd(WindowBase* pParentWindow,
                             const WindowCreateParam& createParam, const UiRect& rc)
{
    ASSERT(!createParam.m_className.empty());
    if (createParam.m_className.empty()) {
        return false;
    }

    m_hResModule = createParam.m_hResModule;
    if (m_hResModule == nullptr) {
        m_hResModule = ::GetModuleHandle(nullptr);
    }

    //注册窗口类
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = createParam.m_dwClassStyle;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.lpfnWndProc = NativeWindow::__WndProc;
    wc.hInstance = GetResModuleHandle();
    wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = createParam.m_className.c_str();
    if (createParam.m_nClassLogoResId > 0) {
        wc.hIcon = LoadIcon(GetResModuleHandle(), (LPCTSTR)MAKEINTRESOURCE(createParam.m_nClassLogoResId));
        wc.hIconSm = LoadIcon(GetResModuleHandle(), (LPCTSTR)MAKEINTRESOURCE(createParam.m_nClassLogoResId));
    }
    else {
        wc.hIcon = nullptr;
        wc.hIconSm = nullptr;
    }
    ATOM ret = ::RegisterClassEx(&wc);
    bool bRet = (ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    ASSERT(bRet);
    if (!bRet) {
        return false;
    }

    //初始化层窗口属性
    m_bIsLayeredWindow = false;
    if (createParam.m_dwExStyle & WS_EX_LAYERED) {
        m_bIsLayeredWindow = true;
    }

    HWND hParentWnd = pParentWindow != nullptr ? pParentWindow->NativeWnd()->GetHWND() : nullptr;
    HWND hWnd = ::CreateWindowEx(createParam.m_dwExStyle,
                                 createParam.m_className.c_str(),
                                 createParam.m_windowTitle.c_str(),
                                 createParam.m_dwStyle,
                                 rc.left, rc.top, rc.Width(), rc.Height(),
                                 hParentWnd, NULL, GetResModuleHandle(), this);
    ASSERT(::IsWindow(hWnd));
    ASSERT(hWnd == m_hWnd);
    if (hWnd != m_hWnd) {
        m_hWnd = hWnd;
    }
    InitWindow();
    return (m_hWnd != nullptr);
}

void NativeWindow::InitWindow()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    ASSERT(m_hDcPaint == nullptr);
    if (m_hDcPaint != nullptr) {
        //避免重复初始化
        return;
    }

    //设置窗口风格（去除标题栏）
    HWND hWnd = GetHWND();
    uint32_t dwStyle = (uint32_t)::GetWindowLong(hWnd, GWL_STYLE);
    //使用自绘的标题栏：从原来窗口样式中，移除 WS_CAPTION 属性
    uint32_t dwNewStyle = dwStyle & ~WS_CAPTION;
    if (dwNewStyle != dwStyle) {
        ::SetWindowLong(hWnd, GWL_STYLE, dwNewStyle);
    }

    //创建绘制设备上下文
    m_hDcPaint = ::GetDC(hWnd);
    ASSERT(m_hDcPaint != nullptr);

    //注册接受Touch消息
    RegisterTouchWindowWrapper(hWnd, 0);
}

void NativeWindow::ClearNativeWindow()
{
    //注销平板消息
    HWND hWnd = GetHWND();
    if (hWnd != nullptr) {
        UnregisterTouchWindowWrapper(hWnd);
    }
    //注销快捷键
    std::vector<int32_t> hotKeyIds = m_hotKeyIds;
    for (int32_t id : hotKeyIds) {
        UnregisterHotKey(id);
    }
    if (m_hDcPaint != nullptr) {
        ::ReleaseDC(m_hWnd, m_hDcPaint);
        m_hDcPaint = nullptr;
    }
    m_hWnd = nullptr;
}

HWND NativeWindow::GetHWND() const
{
    return m_hWnd;
}

bool NativeWindow::IsWindow() const
{
    return (m_hWnd != nullptr) && ::IsWindow(m_hWnd);
}

HMODULE NativeWindow::GetResModuleHandle() const
{
    return (m_hResModule != nullptr) ? m_hResModule : (::GetModuleHandle(nullptr));
}

HDC NativeWindow::GetPaintDC() const
{
    return m_hDcPaint;
}

void NativeWindow::CloseWnd(UINT nRet)
{
    m_bCloseing = true;
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::PostMessage(m_hWnd, WM_CLOSE, (WPARAM)nRet, 0L);
}

void NativeWindow::Close()
{
    m_bCloseing = true;
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::SendMessage(m_hWnd, WM_CLOSE, 0L, 0L);
}

bool NativeWindow::IsClosingWnd() const
{
    return m_bCloseing;
}

bool NativeWindow::SetLayeredWindow(bool bIsLayeredWindow, bool bRedraw)
{
    m_bIsLayeredWindow = bIsLayeredWindow;
    bool bChanged = false;
    SetLayeredWindowStyle(bIsLayeredWindow, bChanged);
    if (bRedraw && bChanged && IsWindow()) {
        // 强制窗口重绘
        ::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    }
    return false;
}

bool NativeWindow::SetLayeredWindowStyle(bool bIsLayeredWindow, bool& bChanged) const
{
    bChanged = false;
    if (::IsWindow(m_hWnd)) {
        LONG dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        LONG dwOldExStyle = dwExStyle;
        if (bIsLayeredWindow) {
            dwExStyle |= WS_EX_LAYERED;
        }
        else {
            dwExStyle &= ~WS_EX_LAYERED;
        }
        if (dwOldExStyle != dwExStyle) {
            bChanged = true;
            ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle);
            dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        }
        if (m_bIsLayeredWindow) {
            return (dwExStyle & WS_EX_LAYERED) ? true : false;
        }
        else {
            return (dwExStyle & WS_EX_LAYERED) ? false : true;
        }
    }
    return false;
}

bool NativeWindow::IsLayeredWindow() const
{
#if _DEBUG
    if (::IsWindow(m_hWnd)) {
        LONG dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        bool bIsLayeredWindow = (dwExStyle & WS_EX_LAYERED) ? true : false;
        ASSERT(bIsLayeredWindow == m_bIsLayeredWindow);
    }
#endif // _DEBUG
    return m_bIsLayeredWindow;
}

void NativeWindow::SetWindowAlpha(int nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    m_nWindowAlpha = static_cast<uint8_t>(nAlpha);
}

uint8_t NativeWindow::GetWindowAlpha() const
{
    return m_nWindowAlpha;
}

void NativeWindow::SetUseSystemCaption(bool bUseSystemCaption)
{
    m_bUseSystemCaption = bUseSystemCaption;
    if (IsUseSystemCaption()) {
        //使用系统默认标题栏, 需要增加标题栏风格
        bool bChanged = false;
        if (IsWindow()) {
            UINT oldStyleValue = (UINT)::GetWindowLong(GetHWND(), GWL_STYLE);
            UINT newStyleValue = oldStyleValue | WS_CAPTION;
            if (newStyleValue != oldStyleValue) {
                ::SetWindowLong(GetHWND(), GWL_STYLE, newStyleValue);
                bChanged = true; 
            }
        }
        //关闭层窗口
        if (IsLayeredWindow()) {
            bChanged = true;
            SetLayeredWindow(false, false);
        }
        if (bChanged) {
            // 强制窗口重绘
            ::SetWindowPos(GetHWND(), NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
            //重新激活窗口的非客户区绘制
            if (IsWindowForeground()) {
                KeepParentActive();
            }            
        }        
    }
    m_pOwner->OnNativeUseSystemCaptionBarChanged();
}

bool NativeWindow::IsUseSystemCaption() const
{
    return m_bUseSystemCaption;
}

bool NativeWindow::ShowWindow(ShowWindowCommands nCmdShow)
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    bool bRet = false;
    int nWindowCmdShow = SW_SHOWNORMAL;
    switch(nCmdShow)
    {
    case kSW_HIDE:
        nWindowCmdShow = SW_HIDE;
        break;
    case kSW_SHOW_NORMAL:
        nWindowCmdShow = SW_SHOWNORMAL;
        break;
    case kSW_SHOW_MINIMIZED:
        nWindowCmdShow = SW_SHOWMINIMIZED;
        break;
    case kSW_SHOW_MAXIMIZED:
        nWindowCmdShow = SW_SHOWMAXIMIZED;
        break;
    case kSW_SHOW_NOACTIVATE:
        nWindowCmdShow = SW_SHOWNOACTIVATE;
        break;
    case kSW_SHOW:
        nWindowCmdShow = SW_SHOW;
        break;
    case kSW_MINIMIZE:
        nWindowCmdShow = SW_MINIMIZE;
        break;
    case kSW_SHOW_MIN_NOACTIVE:
        nWindowCmdShow = SW_SHOWMINNOACTIVE;
        break;
    case kSW_SHOW_NA:
        nWindowCmdShow = SW_SHOWNA;
        break;
    case kSW_RESTORE:
        nWindowCmdShow = SW_RESTORE;
        break;
    default:
        ASSERT(false);
        break;
    }
    bRet = ::ShowWindow(m_hWnd, nWindowCmdShow) != FALSE;
    return bRet;
}

void NativeWindow::ShowModalFake(WindowBase* pParentWindow)
{
    ASSERT(::IsWindow(m_hWnd));
    ASSERT((pParentWindow != nullptr) && (pParentWindow->NativeWnd()->GetHWND() != nullptr));
    if (pParentWindow != nullptr) {
        auto hOwnerWnd = GetWindowOwner();
        ASSERT(::IsWindow(hOwnerWnd));
        ASSERT_UNUSED_VARIABLE(hOwnerWnd == pParentWindow->NativeWnd()->GetHWND());
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(false);
        }
    }
    ShowWindow(kSW_SHOW_NORMAL);
    m_bFakeModal = true;
}

void NativeWindow::OnCloseModalFake(WindowBase* pParentWindow)
{
    if (IsFakeModal()) {
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(true);
            pParentWindow->SetWindowFocus();
        }
        m_bFakeModal = false;
    }
    if (IsWindowFocused()) {
        SetOwnerWindowFocus();
    }
}

bool NativeWindow::IsFakeModal() const
{
    return m_bFakeModal;
}

void NativeWindow::CenterWindow()
{
    ASSERT(::IsWindow(m_hWnd));
    ASSERT((::GetWindowLong(m_hWnd, GWL_STYLE) & WS_CHILD) == 0);
    UiRect rcDlg;
    GetWindowRect(rcDlg);
    UiRect rcArea;
    UiRect rcCenter;
    HWND hWnd = GetHWND();
    HWND hWndCenter = GetWindowOwner();
    if (hWndCenter != nullptr) {
        hWnd = hWndCenter;
    }

    // 处理多显示器模式下屏幕居中
    UiRect rcMonitor;
    GetMonitorRect(hWnd, rcMonitor, rcArea);
    if (hWndCenter == nullptr) {
        rcCenter = rcArea;
    }
    else if (::IsIconic(hWndCenter)) {
        rcCenter = rcArea;
    }
    else {
        GetWindowRect(hWndCenter, rcCenter);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // Find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // The dialog is outside the screen, move it inside
    if (xLeft < rcArea.left) {
        xLeft = rcArea.left;
    }
    else if (xLeft + DlgWidth > rcArea.right) {
        xLeft = rcArea.right - DlgWidth;
    }
    if (yTop < rcArea.top) {
        yTop = rcArea.top;
    }
    else if (yTop + DlgHeight > rcArea.bottom) {
        yTop = rcArea.bottom - DlgHeight;
    }
    ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void NativeWindow::ToTopMost()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void NativeWindow::BringToTop()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

bool NativeWindow::SetWindowForeground()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetForegroundWindow() != m_hWnd) {
        ::SetForegroundWindow(m_hWnd);
    }
    return ::GetForegroundWindow() == m_hWnd;
}

bool NativeWindow::IsWindowForeground() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetForegroundWindow());
}

bool NativeWindow::SetWindowFocus()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
    return ::GetFocus() == m_hWnd;
}

bool NativeWindow::KillWindowFocus()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetFocus() == m_hWnd) {
        ::SetFocus(nullptr);
    }
    return ::GetFocus() != m_hWnd;
}

bool NativeWindow::IsWindowFocused() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetFocus());
}

bool NativeWindow::SetOwnerWindowFocus()
{
    HWND hwndParent = GetWindowOwner();
    if (hwndParent != nullptr) {
        ::SetFocus(hwndParent);
        return ::GetFocus() == hwndParent;
    }
    return false;
}

void NativeWindow::CheckSetWindowFocus()
{
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
}

LRESULT NativeWindow::SendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT NativeWindow::PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void NativeWindow::PostQuitMsg(int32_t nExitCode)
{
    ::PostQuitMessage(nExitCode);
}

bool NativeWindow::EnterFullScreen()
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    if (IsWindowMinimized()) {
        //最小化的时候，不允许激活全屏
        return false;
    }
    if (m_bFullScreen) {
        return true;
    }
    m_bFullScreen = true;

    //保存窗口风格
    m_dwLastStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    //保存窗口大小位置信息
    m_rcLastWindowPlacement.length = sizeof(WINDOWPLACEMENT);
    ::GetWindowPlacement(m_hWnd, &m_rcLastWindowPlacement);

    int32_t xScreen = GetSystemMetricsForDpiWrapper(SM_XVIRTUALSCREEN, m_pOwner->OnNativeGetDpi().GetDPI());
    int32_t yScreen = GetSystemMetricsForDpiWrapper(SM_YVIRTUALSCREEN, m_pOwner->OnNativeGetDpi().GetDPI());
    int32_t cxScreen = GetSystemMetricsForDpiWrapper(SM_CXVIRTUALSCREEN, m_pOwner->OnNativeGetDpi().GetDPI());
    int32_t cyScreen = GetSystemMetricsForDpiWrapper(SM_CYVIRTUALSCREEN, m_pOwner->OnNativeGetDpi().GetDPI());

    // 去掉标题栏、边框
    DWORD dwFullScreenStyle = (m_dwLastStyle | WS_VISIBLE | WS_POPUP | WS_MAXIMIZE) & ~WS_CAPTION & ~WS_BORDER & ~WS_THICKFRAME & ~WS_DLGFRAME;
    ::SetWindowLongPtr(m_hWnd, GWL_STYLE, dwFullScreenStyle);
    ::SetWindowPos(m_hWnd, NULL, xScreen, yScreen, cxScreen, cyScreen, SWP_FRAMECHANGED); // 设置位置和大小
    
    m_pOwner->OnNativeWindowEnterFullScreen();
    return true;
}

bool NativeWindow::ExitFullScreen()
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    if (!m_bFullScreen) {
        return false;
    }
    
    //恢复窗口风格
    if (m_dwLastStyle != 0) {
        ::SetWindowLong(m_hWnd, GWL_STYLE, m_dwLastStyle);
        m_dwLastStyle = 0;
    }

    //恢复窗口位置/大小信息
    ::SetWindowPlacement(m_hWnd, &m_rcLastWindowPlacement);

    if (IsWindowMaximized()) {
        ::ShowWindow(m_hWnd, SW_RESTORE);
    }
    m_bFullScreen = false;
    m_pOwner->OnNativeWindowExitFullScreen();
    return true;
}

bool NativeWindow::IsWindowMaximized() const
{
    return ::IsWindow(m_hWnd) && ::IsZoomed(m_hWnd);
}

bool NativeWindow::IsWindowMinimized() const
{
    return ::IsWindow(m_hWnd) && ::IsIconic(m_hWnd);
}

bool NativeWindow::IsWindowFullScreen() const
{
    return m_bFullScreen;
}

bool NativeWindow::EnableWindow(bool bEnable)
{
    return ::EnableWindow(m_hWnd, bEnable ? TRUE : false) != FALSE;
}

bool NativeWindow::IsWindowEnabled() const
{
    return ::IsWindow(m_hWnd) && ::IsWindowEnabled(m_hWnd) != FALSE;
}

bool NativeWindow::IsWindowVisible() const
{
    return ::IsWindow(m_hWnd) && ::IsWindowVisible(m_hWnd) != FALSE;
}

bool NativeWindow::SetWindowPos(const InsertAfterWnd& insertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, uint32_t uFlags)
{
    ASSERT(::IsWindow(m_hWnd));
    HWND hWndInsertAfter = HWND_TOP;
    if (!(uFlags & kSWP_NOZORDER)) {
        if (insertAfter.m_pWindow != nullptr) {
            hWndInsertAfter = insertAfter.m_pWindow->NativeWnd()->GetHWND();
        }
        else {
            hWndInsertAfter = (HWND)insertAfter.m_hwndFlag;
        }
    }
    return ::SetWindowPos(m_hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags) != FALSE;
}

bool NativeWindow::MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::MoveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint ? TRUE : FALSE) != FALSE;
}

void NativeWindow::SetIcon(UINT nRes)
{
    ASSERT(::IsWindow(m_hWnd));
    HICON hIcon = (HICON)::LoadImage(GetResModuleHandle(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                                     GetSystemMetricsForDpiWrapper(SM_CXICON, m_pOwner->OnNativeGetDpi().GetDPI()),
                                     GetSystemMetricsForDpiWrapper(SM_CYICON, m_pOwner->OnNativeGetDpi().GetDPI()),
                                     LR_DEFAULTCOLOR | LR_SHARED);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
    hIcon = (HICON)::LoadImage(GetResModuleHandle(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                               GetSystemMetricsForDpiWrapper(SM_CXSMICON, m_pOwner->OnNativeGetDpi().GetDPI()),
                               GetSystemMetricsForDpiWrapper(SM_CYSMICON, m_pOwner->OnNativeGetDpi().GetDPI()),
                               LR_DEFAULTCOLOR | LR_SHARED);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);
}

void NativeWindow::SetText(const DString& strText)
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowText(m_hWnd, strText.c_str());
}

void NativeWindow::SetCapture()
{
    ::SetCapture(m_hWnd);
    m_bMouseCapture = true;
}

void NativeWindow::ReleaseCapture()
{
    if (m_bMouseCapture) {
        ::ReleaseCapture();
        m_bMouseCapture = false;
    }
}

bool NativeWindow::IsCaptured() const
{
    return m_bMouseCapture;
}

bool NativeWindow::SetWindowRoundRectRgn(const UiRect& rcWnd, const UiSize& szRoundCorner, bool bRedraw)
{
    ASSERT((szRoundCorner.cx > 0) && (szRoundCorner.cy > 0));
    if ((szRoundCorner.cx <= 0) || (szRoundCorner.cy <= 0)) {
        return false;
    }
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
    int nRet = ::SetWindowRgn(GetHWND(), hRgn, bRedraw ? TRUE : FALSE);
    ::DeleteObject(hRgn);//TODO: 检查是否需要删除，按MSDN说法，是不需要删除的。
    return nRet != 0;
}

void NativeWindow::ClearWindowRgn(bool bRedraw)
{
    ASSERT(IsWindow());
    ::SetWindowRgn(GetHWND(), nullptr, bRedraw ? TRUE : FALSE);
}

void NativeWindow::Invalidate(const UiRect& rcItem)
{
    RECT rc = { rcItem.left, rcItem.top, rcItem.right, rcItem.bottom };
    ::InvalidateRect(m_hWnd, &rc, FALSE);
    // Invalidating a layered window will not trigger a WM_PAINT message,
    // thus we have to post WM_PAINT by ourselves.
    if (IsLayeredWindow()) {
        ::PostMessage(m_hWnd, WM_PAINT, 0, 0);
    }
}

bool NativeWindow::UpdateWindow() const
{
    bool bRet = false;
    if ((m_hWnd != nullptr) && ::IsWindow(m_hWnd)) {
        bRet = ::UpdateWindow(m_hWnd) != FALSE;
    }
    return bRet;
}

bool NativeWindow::BeginPaint(UiRect& rcPaint)
{
    rcPaint.Clear();
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    if (m_paintStruct.hdc != nullptr) {
        ::EndPaint(m_hWnd, &m_paintStruct);
        m_paintStruct = {0, };
    }

    if (!GetUpdateRect(rcPaint)) {
        rcPaint.Clear();
    }
    HDC hDC = ::BeginPaint(m_hWnd, &m_paintStruct);
    ASSERT(hDC == m_paintStruct.hdc);
    if (hDC != nullptr) {
        //rcPaint需要使用GetUpdateRect的结果，否则层窗口的情况下，有绘制异常
        if (rcPaint.IsEmpty()) {
            rcPaint.left = m_paintStruct.rcPaint.left;
            rcPaint.top = m_paintStruct.rcPaint.top;
            rcPaint.right = m_paintStruct.rcPaint.right;
            rcPaint.bottom = m_paintStruct.rcPaint.bottom;
        }

        if (IsLayeredWindow()) {
            //使用层窗口时，窗口部分在屏幕外时，获取到的无效区域仅仅是屏幕内的部分，这里做修正处理
            UiRect rcWindow;
            GetWindowRect(rcWindow);
            UiRect rcClient;
            GetClientRect(rcClient);
            int32_t xScreen = GetSystemMetricsForDpiWrapper(SM_XVIRTUALSCREEN, m_pOwner->OnNativeGetDpi().GetDPI());
            int32_t yScreen = GetSystemMetricsForDpiWrapper(SM_YVIRTUALSCREEN, m_pOwner->OnNativeGetDpi().GetDPI());
            int32_t cxScreen = GetSystemMetricsForDpiWrapper(SM_CXVIRTUALSCREEN, m_pOwner->OnNativeGetDpi().GetDPI());
            int32_t cyScreen = GetSystemMetricsForDpiWrapper(SM_CYVIRTUALSCREEN, m_pOwner->OnNativeGetDpi().GetDPI());
            if (rcWindow.left < xScreen && rcWindow.left + rcPaint.left == xScreen) {
                rcPaint.left = rcClient.left;
            }
            if (rcWindow.top < yScreen && rcWindow.top + rcPaint.top == yScreen) {
                rcPaint.top = rcClient.top;
            }
            if (rcWindow.right > cxScreen && rcWindow.left + rcPaint.right == xScreen + cxScreen) {
                rcPaint.right = rcClient.right;
            }
            if (rcWindow.bottom > cyScreen && rcWindow.top + rcPaint.bottom == yScreen + cyScreen) {
                rcPaint.bottom = rcClient.bottom;
            }
        }
    }
    return (hDC != nullptr);
}

bool NativeWindow::EndPaint(const UiRect& rcPaint, IRender* pRender)
{
    ASSERT(m_paintStruct.hdc != nullptr);
    if (m_paintStruct.hdc == nullptr) {
        return false;
    }
    
    if (pRender != nullptr) {
        // 渲染到窗口
        if (IsLayeredWindow()) {
            UiRect rcWindow;
            GetWindowRect(rcWindow);
            UiRect rcClient;
            GetClientRect(rcClient);
            POINT pt = { rcWindow.left, rcWindow.top };
            SIZE szWindow = { rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
            POINT ptSrc = { 0, 0 };
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, static_cast<BYTE>(GetWindowAlpha()), AC_SRC_ALPHA };
            HDC hdc = pRender->GetDC();
            BOOL bUpdated = ::UpdateLayeredWindow(m_hWnd, NULL, &pt, &szWindow, hdc, &ptSrc, 0, &bf, ULW_ALPHA);
            ASSERT(bUpdated);
            pRender->ReleaseDC(hdc);
        }
        else {
            HDC hdc = pRender->GetDC();
            ::BitBlt(m_paintStruct.hdc, rcPaint.left, rcPaint.top, rcPaint.Width(), rcPaint.Height(),
                     hdc, rcPaint.left, rcPaint.top, SRCCOPY);
            pRender->ReleaseDC(hdc);
        }
    }

    bool bRet = ::EndPaint(m_hWnd, &m_paintStruct) != FALSE;
    m_paintStruct = { 0, };
    return bRet;
}

bool NativeWindow::GetUpdateRect(UiRect& rcPaint)
{
    RECT rectPaint = { 0, };
    if (!::GetUpdateRect(m_hWnd, &rectPaint, FALSE)) {
        return false;
    }
    else {
        rcPaint.left = rectPaint.left;
        rcPaint.top = rectPaint.top;
        rcPaint.right = rectPaint.right;
        rcPaint.bottom = rectPaint.bottom;
        return true;
    }
}

void NativeWindow::KeepParentActive()
{
    HWND hWndParent = GetHWND();
    if (::IsWindow(hWndParent)) {
        while (::GetParent(hWndParent) != NULL) {
            hWndParent = ::GetParent(hWndParent);
        }
    }
    if (::IsWindow(hWndParent)) {
        ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
    }
}

void NativeWindow::GetClientRect(UiRect& rcClient) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetClientRect(m_hWnd, &rc);
    rcClient = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void NativeWindow::GetWindowRect(UiRect& rcWindow) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetWindowRect(m_hWnd, &rc);
    rcWindow = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void NativeWindow::GetWindowRect(HWND hWnd, UiRect& rcWindow) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetWindowRect(hWnd, &rc);
    rcWindow = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void NativeWindow::ScreenToClient(UiPoint& pt) const
{
    POINT ptClient = { pt.x, pt.y };
    ::ScreenToClient(m_hWnd, &ptClient);
    pt = UiPoint(ptClient.x, ptClient.y);
}

void NativeWindow::ClientToScreen(UiPoint& pt) const
{
    POINT ptClient = { pt.x, pt.y };
    ::ClientToScreen(m_hWnd, &ptClient);
    pt = UiPoint(ptClient.x, ptClient.y);
}

void NativeWindow::GetCursorPos(UiPoint& pt) const
{
    POINT ptPos;
    ::GetCursorPos(&ptPos);
    pt = { ptPos.x, ptPos.y };
}

void NativeWindow::MapWindowDesktopRect(UiRect& rc) const
{
    ASSERT(IsWindow());
    HWND hwndFrom = GetHWND();
    HWND hwndTo = HWND_DESKTOP;
    POINT pts[2];
    pts[0].x = rc.left;
    pts[0].y = rc.top;
    pts[1].x = rc.right;
    pts[1].y = rc.bottom;
    ::MapWindowPoints((hwndFrom), (hwndTo), &pts[0], 2);
    rc.left = pts[0].x;
    rc.top = pts[0].y;
    rc.right = pts[1].x;
    rc.bottom = pts[1].y;
}

bool NativeWindow::GetMonitorRect(UiRect& rcMonitor, UiRect& rcWork) const
{
    return GetMonitorRect(m_hWnd, rcMonitor, rcWork);
}

bool NativeWindow::GetMonitorRect(HWND hWnd, UiRect& rcMonitor, UiRect& rcWork) const
{
    ASSERT(::IsWindow(hWnd));
    rcMonitor.Clear();
    rcWork.Clear();
    HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    ASSERT(hMonitor != nullptr);
    if (hMonitor == nullptr) {
        return false;
    }
    MONITORINFO oMonitor = { 0, };
    oMonitor.cbSize = sizeof(oMonitor);
    if (::GetMonitorInfo(hMonitor, &oMonitor)) {
        rcWork = UiRect(oMonitor.rcWork.left, oMonitor.rcWork.top,
                        oMonitor.rcWork.right, oMonitor.rcWork.bottom);
        rcMonitor = UiRect(oMonitor.rcMonitor.left, oMonitor.rcMonitor.top,
                           oMonitor.rcMonitor.right, oMonitor.rcMonitor.bottom);
        return true;
    }
    else {
        ASSERT(!"NativeWindow::GetMonitorWorkRect failed!");
        return false;
    }
}

bool NativeWindow::GetMonitorWorkRect(UiRect& rcWork) const
{
    UiRect rcMonitor;
    return GetMonitorRect(rcMonitor, rcWork);
}

bool NativeWindow::GetMonitorWorkRect(const UiPoint& pt, UiRect& rcWork) const
{
    rcWork.Clear();
    HMONITOR hMonitor = ::MonitorFromPoint({ pt.x, pt.y }, MONITOR_DEFAULTTONEAREST);
    ASSERT(hMonitor != nullptr);
    if (hMonitor == nullptr) {
        return false;
    }
    MONITORINFO oMonitor = { 0, };
    oMonitor.cbSize = sizeof(oMonitor);
    if (::GetMonitorInfo(hMonitor, &oMonitor)) {
        rcWork = UiRect(oMonitor.rcWork.left, oMonitor.rcWork.top,
                        oMonitor.rcWork.right, oMonitor.rcWork.bottom);
        return true;
    }
    else {
        ASSERT(!"NativeWindow::GetMonitorWorkRect failed!");
        return false;
    }
}

const UiPoint& NativeWindow::GetLastMousePos() const
{
    return m_ptLastMousePos;
}

void NativeWindow::SetLastMousePos(const UiPoint& pt)
{
    m_ptLastMousePos = pt;
}

bool NativeWindow::GetModifiers(UINT message, WPARAM wParam, LPARAM lParam, uint32_t& modifierKey) const
{
    bool bRet = true;
    modifierKey = ModifierKey::kNone;
    switch (message) {
    case WM_CHAR:
        if (0 == (lParam & (1 << 30))) {
            modifierKey |= ModifierKey::kFirstPress;
        }
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (0 == (lParam & (1 << 30))) {
            modifierKey |= ModifierKey::kFirstPress;
        }
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;

    case WM_MOUSEWHEEL:
    {
        WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
        if (fwKeys & MK_CONTROL) {
            modifierKey |= ModifierKey::kControl;
        }
        if (fwKeys & MK_SHIFT) {
            modifierKey |= ModifierKey::kShift;
        }
    }
    break;
    case WM_MOUSEHOVER:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        if (wParam & MK_CONTROL) {
            modifierKey |= ModifierKey::kControl;
        }
        if (wParam & MK_SHIFT) {
            modifierKey |= ModifierKey::kShift;
        }
        break;
    default:
        bRet = false;
        break;
    }
    ASSERT(bRet);
    return bRet;
}

INativeWindow* NativeWindow::WindowBaseFromPoint(const UiPoint& pt)
{
    NativeWindow* pWindow = nullptr;
    HWND hWnd = ::WindowFromPoint({ pt.x, pt.y });
    if (::IsWindow(hWnd)) {
        if (hWnd == m_hWnd) {
            pWindow = this;
        }
        else {
            pWindow = reinterpret_cast<NativeWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if ((pWindow != nullptr) && (pWindow->m_hWnd != hWnd)) {
                pWindow = nullptr;
            }
        }
    }
    INativeWindow* pNativeWindow = nullptr;
    if (pWindow != nullptr) {
        pNativeWindow = pWindow->m_pOwner;
    }
    return pNativeWindow;
}


int32_t NativeWindow::SetWindowHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    ASSERT(IsWindow());
    return (int32_t)::SendMessage(GetHWND(), WM_SETHOTKEY, MAKEWORD(wVirtualKeyCode, wModifiers), 0);
}

bool NativeWindow::GetWindowHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
{
    ASSERT(IsWindow());
    DWORD dw = (DWORD)::SendMessage(GetHWND(), HKM_GETHOTKEY, 0, 0L);
    wVirtualKeyCode = LOBYTE(LOWORD(dw));
    wModifiers = HIBYTE(LOWORD(dw));
    return dw != 0;
}

bool NativeWindow::RegisterHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers, int32_t id)
{
    ASSERT(IsWindow());
    if (wVirtualKeyCode != 0) {
        UINT fsModifiers = 0;
        if (wModifiers & HOTKEYF_ALT)     fsModifiers |= MOD_ALT;
        if (wModifiers & HOTKEYF_CONTROL) fsModifiers |= MOD_CONTROL;
        if (wModifiers & HOTKEYF_SHIFT)   fsModifiers |= MOD_SHIFT;
        if (wModifiers & HOTKEYF_EXT)     fsModifiers |= MOD_WIN;

#ifndef MOD_NOREPEAT
        if (::IsWindows7OrGreater()) {
            fsModifiers |= 0x4000;
        }
#else
        fsModifiers |= MOD_NOREPEAT;
#endif

        LRESULT lResult = ::RegisterHotKey(this->GetHWND(), id, fsModifiers, wVirtualKeyCode);
        ASSERT(lResult != 0);
        if (lResult != 0) {
            auto iter = std::find(m_hotKeyIds.begin(), m_hotKeyIds.end(), id);
            if (iter != m_hotKeyIds.end()) {
                m_hotKeyIds.erase(iter);
            }
            m_hotKeyIds.push_back(id);
            return true;
        }
    }
    return false;
}

bool NativeWindow::UnregisterHotKey(int32_t id)
{
    ASSERT(IsWindow());
    auto iter = std::find(m_hotKeyIds.begin(), m_hotKeyIds.end(), id);
    if (iter != m_hotKeyIds.end()) {
        m_hotKeyIds.erase(iter);
    }
    return ::UnregisterHotKey(GetHWND(), id);
}

/** 窗口句柄的属性名称
*/
static const wchar_t* sPropName = _T("DuiLibWindow"); // 属性名称

LRESULT CALLBACK NativeWindow::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NativeWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<NativeWindow*>(lpcs->lpCreateParams);
        if (pThis != nullptr) {
            pThis->m_hWnd = hWnd;
        }
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
        ::SetPropW(hWnd, sPropName, (HANDLE)pThis);
    }
    else {
        pThis = reinterpret_cast<NativeWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
#ifdef _DEBUG
        //校验是否一致
        ASSERT((NativeWindow*)::GetPropW(hWnd, sPropName) == pThis);
#endif
        if (uMsg == WM_NCDESTROY && pThis != nullptr) {
            LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
            ::SetPropW(hWnd, sPropName, NULL);
            ASSERT(hWnd == pThis->GetHWND());
            pThis->OnFinalMessage();
            return lRes;
        }
    }

    if (pThis != nullptr) {
        ASSERT(hWnd == pThis->GetHWND());
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    }
    else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

LRESULT NativeWindow::WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    HWND hWnd = m_hWnd;
    INativeWindow* pOwner = m_pOwner;
    if (pOwner == nullptr) {
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
        return lResult;
    }
    //接口的生命周期标志
    std::weak_ptr<WeakFlag> ownerFlag = pOwner->GetWeakFlag();

    //消息首先转给过滤器(全部消息)
    bool bHandled = false;
    if (!bHandled && !ownerFlag.expired()) {
        lResult = pOwner->OnNativeWindowMessage(uMsg, wParam, lParam, bHandled);
    }

    //第三优先级：内部处理的消息，处理后，不再派发
    if (!bHandled && !ownerFlag.expired()) {
        lResult = ProcessInternalMessage(uMsg, wParam, lParam, bHandled);
    }

    //第四优先级：内部处理函数，优先保证自身功能正常
    if (!bHandled && !ownerFlag.expired()) {
        lResult = ProcessWindowMessage(uMsg, wParam, lParam, bHandled);
    }

    //自定义窗口消息的派发函数，仅供内部实现使用
    if (!bHandled && !ownerFlag.expired() && (uMsg >= WM_USER)) {
        lResult = pOwner->OnNativeUserMessage(uMsg, wParam, lParam, bHandled);
    }

    if (!bHandled && !ownerFlag.expired()) {
        if ((uMsg == WM_CLOSE) || ((uMsg == WM_SYSCOMMAND) && (GET_SC_WPARAM(wParam) == SC_CLOSE))) {
            //窗口即将关闭（关闭前）
            pOwner->OnNativePreCloseWindow();
        }
    }

    //第五优先级：系统默认的窗口函数
    if (!bHandled && !ownerFlag.expired() && ::IsWindow(hWnd)) {
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
    }

    if (!bHandled && !ownerFlag.expired() && (uMsg == WM_DESTROY)) {
        //窗口已经关闭（关闭后）
        pOwner->OnNativePostCloseWindow();
    }

    return lResult;
}

LRESULT NativeWindow::CallDefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

LRESULT NativeWindow::ProcessInternalMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    bool bInternalMsg = true;
    switch (uMsg)
    {
    case WM_NCACTIVATE:         lResult = OnNcActivateMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCCALCSIZE:         lResult = OnNcCalcSizeMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCHITTEST:          lResult = OnNcHitTestMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCLBUTTONDBLCLK:    lResult = OnNcLButtonDbClickMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_GETMINMAXINFO:      lResult = OnGetMinMaxInfoMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_ERASEBKGND:         lResult = OnEraseBkGndMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_DPICHANGED:         lResult = OnDpiChangedMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_WINDOWPOSCHANGING:  lResult = OnWindowPosChangingMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_NOTIFY:             lResult = OnNotifyMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_COMMAND:            lResult = OnCommandMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLOREDIT:       lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLORSTATIC:     lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled); break;
    case WM_TOUCH:              lResult = OnTouchMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_POINTERDOWN:
    case WM_POINTERUP:
    case WM_POINTERUPDATE:
    case WM_POINTERLEAVE:
    case WM_POINTERWHEEL:
    case WM_POINTERCAPTURECHANGED:
        lResult = OnPointerMsgs(uMsg, wParam, lParam, bHandled);
        break;

    default:
        bInternalMsg = false;
        break;
    }//end of switch

    if (bInternalMsg && !bHandled) {
        //调用窗口函数，然后不再继续传递此消息
        bHandled = true;
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
    }
    return lResult;
}

LRESULT NativeWindow::OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCACTIVATE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    LRESULT lResult = 0;
    if (IsWindowMinimized()) {
        bHandled = false;
    }
    else {
        //MSDN: wParam 参数为 FALSE 时，应用程序应返回 TRUE 以指示系统应继续执行默认处理
        bHandled = true;
        lResult = (wParam == FALSE) ? TRUE : FALSE;
    }
    return lResult;
}

LRESULT NativeWindow::OnNcCalcSizeMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCCALCSIZE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    //截获，让系统不处理此消息
    bHandled = true;
    return 0;
}

LRESULT NativeWindow::OnNcHitTestMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCHITTEST);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    bHandled = true;
    UiPoint pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    ScreenToClient(pt);

    UiRect rcClient;
    GetClientRect(rcClient);

    //客户区域，排除掉阴影部分区域
    UiPadding rcCorner;
    m_pOwner->OnNativeGetShadowCorner(rcCorner);
    rcClient.Deflate(rcCorner);

    if (!IsWindowMaximized()) {
        //非最大化状态
        UiRect rcSizeBox = m_pOwner->OnNativeGetSizeBox();
        if (pt.y < rcClient.top + rcSizeBox.top) {
            if (pt.y >= rcClient.top) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTTOPLEFT;//在窗口边框的左上角。
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTTOPRIGHT;//在窗口边框的右上角
                }
                else {
                    return HTTOP;//在窗口的上水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
            if (pt.y <= rcClient.bottom) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTBOTTOMLEFT;//在窗口边框的左下角
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTBOTTOMRIGHT;//在窗口边框的右下角
                }
                else {
                    return HTBOTTOM;//在窗口的下水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }

        if (pt.x < rcClient.left + rcSizeBox.left) {
            if (pt.x >= rcClient.left) {
                return HTLEFT;//在窗口的左边框
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        if (pt.x > rcClient.right - rcSizeBox.right) {
            if (pt.x <= rcClient.right) {
                return HTRIGHT;//在窗口的右边框中
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
    }

    UiRect rcCaption = m_pOwner->OnNativeGetCaptionRect();
    if ((pt.x >= rcClient.left + rcCaption.left) && (pt.x < rcClient.right - rcCaption.right) &&
        (pt.y >= rcClient.top + rcCaption.top && pt.y < rcClient.top + rcCaption.bottom)) {
        if (m_pOwner->OnNativeIsPtInCaptionBarControl(pt)) {
            return HTCLIENT;//在工作区中（放在标题栏上的控件，视为工作区）
        }
        else {
            return HTCAPTION;//在标题栏中
        }
    }
    //其他，在工作区中
    return HTCLIENT;
}

LRESULT NativeWindow::OnNcLButtonDbClickMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCLBUTTONDBLCLK);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    //TODO：需要判断是否可以最大化
    bHandled = true;
    if (!IsWindowMaximized()) {
        //最大化
        ShowWindow(kSW_SHOW_MAXIMIZED);
    }
    else {
        //还原
        ShowWindow(kSW_RESTORE);
    }
    return 0;
}

LRESULT NativeWindow::OnGetMinMaxInfoMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_GETMINMAXINFO);
    bHandled = false;
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
    UiRect rcWork;
    UiRect rcMonitor;
    GetMonitorRect(rcMonitor, rcWork);
    rcWork.Offset(-rcMonitor.left, -rcMonitor.top);

    //最大化时，默认设置为当前屏幕的最大区域
    lpMMI->ptMaxPosition.x = rcWork.left;
    lpMMI->ptMaxPosition.y = rcWork.top;
    lpMMI->ptMaxSize.x = rcWork.Width();
    lpMMI->ptMaxSize.y = rcWork.Height();

    if (m_pOwner->OnNativeGetMaxInfo(true).cx != 0) {
        lpMMI->ptMaxTrackSize.x = m_pOwner->OnNativeGetMaxInfo(true).cx;
    }
    if (m_pOwner->OnNativeGetMaxInfo(true).cy != 0) {
        lpMMI->ptMaxTrackSize.y = m_pOwner->OnNativeGetMaxInfo(true).cy;
    }
    if (m_pOwner->OnNativeGetMinInfo(true).cx != 0) {
        lpMMI->ptMinTrackSize.x = m_pOwner->OnNativeGetMinInfo(true).cx;
    }
    if (m_pOwner->OnNativeGetMinInfo(true).cy != 0) {
        lpMMI->ptMinTrackSize.y = m_pOwner->OnNativeGetMinInfo(true).cy;
    }
    return 0;
}

LRESULT NativeWindow::OnEraseBkGndMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_ERASEBKGND);
    bHandled = true;
    return 1;
}

LRESULT NativeWindow::OnDpiChangedMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_DPICHANGED);
    bHandled = false;//需要重新测试

    uint32_t nNewDPI = HIWORD(wParam);
    UiRect rcNewWindow;
    const RECT* prcNewWindow = (RECT*)lParam;
    if (prcNewWindow != nullptr) {
        rcNewWindow.left = prcNewWindow->left;
        rcNewWindow.top = prcNewWindow->top;
        rcNewWindow.right = prcNewWindow->right;
        rcNewWindow.bottom = prcNewWindow->bottom;
    }
    uint32_t nOldDpiScale = m_pOwner->OnNativeGetDpi().GetScale();
    m_pOwner->OnNativeProcessDpiChangedMsg(nNewDPI, rcNewWindow);
    if (nOldDpiScale != m_pOwner->OnNativeGetDpi().GetScale()) {
        m_ptLastMousePos = m_pOwner->OnNativeGetDpi().GetScalePoint(m_ptLastMousePos, nOldDpiScale);
    }
    //更新窗口的位置和大小
    if (!rcNewWindow.IsEmpty()) {
        SetWindowPos(InsertAfterWnd(),
                     rcNewWindow.left, rcNewWindow.top, rcNewWindow.Width(), rcNewWindow.Height(),
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    return 0;
}

LRESULT NativeWindow::OnWindowPosChangingMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_WINDOWPOSCHANGING);
    bHandled = false;
    if (IsWindowMaximized()) {
        //最大化状态
        LPWINDOWPOS lpPos = (LPWINDOWPOS)lParam;
        if (lpPos->flags & SWP_FRAMECHANGED) // 第一次最大化，而不是最大化之后所触发的WINDOWPOSCHANGE
        {
            POINT pt = { 0, 0 };
            HMONITOR hMontorPrimary = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
            HMONITOR hMonitorTo = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);

            // 先把窗口最大化，再最小化，然后恢复，此时MonitorFromWindow拿到的HMONITOR不准确
            // 判断GetWindowRect的位置如果不正确（最小化时得到的位置信息是-38000），则改用normal状态下的位置，来获取HMONITOR
            UiRect rc;
            GetWindowRect(rc);
            if (rc.left < -10000 && rc.top < -10000 && rc.bottom < -10000 && rc.right < -10000) {
                WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
                ::GetWindowPlacement(m_hWnd, &wp);
                hMonitorTo = ::MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTOPRIMARY);
            }
            if (hMonitorTo != hMontorPrimary) {
                // 解决无边框窗口在双屏下面（副屏分辨率大于主屏）时，最大化不正确的问题
                MONITORINFO  miTo = { sizeof(miTo), 0 };
                ::GetMonitorInfo(hMonitorTo, &miTo);

                lpPos->x = miTo.rcWork.left;
                lpPos->y = miTo.rcWork.top;
                lpPos->cx = miTo.rcWork.right - miTo.rcWork.left;
                lpPos->cy = miTo.rcWork.bottom - miTo.rcWork.top;
            }
        }
    }
    return 0;
}

LRESULT NativeWindow::OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NOTIFY);
    bHandled = false;
    LPNMHDR lpNMHDR = (LPNMHDR)lParam;
    if (lpNMHDR != nullptr) {
        bHandled = true;
        return ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT NativeWindow::OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_COMMAND);
    bHandled = false;
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT NativeWindow::OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_CTLCOLOREDIT || uMsg == WM_CTLCOLORSTATIC);
    bHandled = false;
    // Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
    // Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT NativeWindow::OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_TOUCH);
    LRESULT lResult = 0;
    bHandled = false;
    unsigned int nNumInputs = LOWORD(wParam);
    if (nNumInputs < 1) {
        nNumInputs = 1;
    }
    TOUCHINPUT* pInputs = new TOUCHINPUT[nNumInputs];
    // 只关心第一个触摸位置
    if (!GetTouchInputInfoWrapper((HTOUCHINPUT)lParam, nNumInputs, pInputs, sizeof(TOUCHINPUT))) {
        delete[] pInputs;
        return lResult;
    }
    else {
        CloseTouchInputHandleWrapper((HTOUCHINPUT)lParam);
        if (pInputs[0].dwID == 0) {
            return lResult;
        }
    }
    //获取触摸点的坐标，并转换为窗口内的客户区坐标
    UiPoint pt = { TOUCH_COORD_TO_PIXEL(pInputs[0].x) , TOUCH_COORD_TO_PIXEL(pInputs[0].y) };
    ScreenToClient(pt);

    DWORD dwFlags = pInputs[0].dwFlags;
    delete[] pInputs;
    pInputs = nullptr;

    if (dwFlags & TOUCHEVENTF_DOWN) {        
        lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, 0, NativeMsg(WM_LBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
    }
    else if (dwFlags & TOUCHEVENTF_MOVE) {
        UiPoint lastMousePos = m_ptLastMousePos;
        lResult = m_pOwner->OnNativeMouseMoveMsg(pt, 0, NativeMsg(WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
        int wheelDelta = pt.y - lastMousePos.y;
        if (wheelDelta != 0) {
            //触发滚轮功能
            lResult = m_pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, 0, NativeMsg(WM_MOUSEWHEEL, MAKEWPARAM(0, wheelDelta), MAKELPARAM(pt.x, pt.y)), bHandled);
        }
    }
    else if (dwFlags & TOUCHEVENTF_UP) {
        lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, 0, NativeMsg(WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
    }
    return lResult;
}

LRESULT NativeWindow::OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE( uMsg == WM_POINTERDOWN ||
                            uMsg == WM_POINTERUP ||
                            uMsg == WM_POINTERUPDATE ||
                            uMsg == WM_POINTERLEAVE ||
                            uMsg == WM_POINTERCAPTURECHANGED ||
                            uMsg == WM_POINTERWHEEL);

    LRESULT lResult = 0;
    bHandled = false;
    // 只关心第一个触摸点
    if (!IS_POINTER_PRIMARY_WPARAM(wParam)) {
        bHandled = true;
        return lResult;
    }
    //获取指针位置，并且将屏幕坐标转换为窗口客户区坐标
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    ScreenToClient(pt);
    switch (uMsg)
    {
    case WM_POINTERDOWN:
        lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, 0, NativeMsg(WM_LBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
        bHandled = true;
        break;
    case WM_POINTERUPDATE:
        lResult = m_pOwner->OnNativeMouseMoveMsg(pt, 0, NativeMsg(WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
        bHandled = true;
        break;
    case WM_POINTERUP:
        lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, 0, NativeMsg(WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
        bHandled = true;
        break;
    case WM_POINTERWHEEL:
    {
        int32_t wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        lResult = m_pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, 0, NativeMsg(WM_MOUSEWHEEL, MAKEWPARAM(0, wheelDelta), MAKELPARAM(pt.x, pt.y)), bHandled);
        bHandled = true;
    }
    break;
    case WM_POINTERLEAVE:
        lResult = m_pOwner->OnNativeMouseLeaveMsg(NativeMsg(WM_MOUSELEAVE, 0, 0), bHandled);
        break;
    case WM_POINTERCAPTURECHANGED:
        lResult = m_pOwner->OnNativeCaptureChangedMsg(NativeMsg(WM_CAPTURECHANGED, 0, 0), bHandled);
        //如果不设置bHandled，程序会转换为WM_BUTTON类消息
        bHandled = true;
        break;
    default:
        break;
    }
    return 0;
}

LRESULT NativeWindow::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    switch (uMsg)
    {
    case WM_SIZE:
    {
        WindowSizeType sizeType = static_cast<WindowSizeType>(wParam);
        UiSize newWindowSize;
        newWindowSize.cx = (int)(short)LOWORD(lParam);
        newWindowSize.cy = (int)(short)HIWORD(lParam);
        lResult = m_pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOVE:
    {
        UiPoint ptTopLeft;
        ptTopLeft.x = (int)(short)LOWORD(lParam);   // horizontal position 
        ptTopLeft.y = (int)(short)HIWORD(lParam);   // vertical position 
        lResult = m_pOwner->OnNativeMoveMsg(ptTopLeft, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_PAINT:
    {
        ASSERT(m_paintStruct.hdc == nullptr);

        lResult = m_pOwner->OnNativePaintMsg(NativeMsg(uMsg, wParam, lParam), bHandled);

        ASSERT(m_paintStruct.hdc == nullptr);
        if (m_paintStruct.hdc != nullptr) {
            ::EndPaint(m_hWnd, &m_paintStruct);
            m_paintStruct = { 0, };
        }

        if (!bHandled) {
            PAINTSTRUCT ps = { 0, };
            ::BeginPaint(m_hWnd, &ps);
            //::FillRect(ps.hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            ::EndPaint(m_hWnd, &ps);
        }
        break;
    }
    case WM_SETFOCUS:
    {
        INativeWindow* pLostFocusWindow = nullptr;
        HWND hWnd = (HWND)wParam;
        if ((hWnd != nullptr) && ::IsWindow(hWnd)) {
            NativeWindow* pThis = reinterpret_cast<NativeWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            //校验是否一致
            if (pThis != nullptr) {
                if ((NativeWindow*)::GetPropW(hWnd, sPropName) == pThis) {
                    pLostFocusWindow = pThis->m_pOwner;
                }
            }
        }
        lResult = m_pOwner->OnNativeSetFocusMsg(pLostFocusWindow, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KILLFOCUS:
    {
        INativeWindow* pSetFocusWindow = nullptr;
        HWND hWnd = (HWND)wParam;
        if ((hWnd != nullptr) && ::IsWindow(hWnd)) {
            NativeWindow* pThis = reinterpret_cast<NativeWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            //校验是否一致
            if (pThis != nullptr) {
                if ((NativeWindow*)::GetPropW(hWnd, sPropName) == pThis) {
                    pSetFocusWindow = pThis->m_pOwner;
                }
            }
        }
        lResult = m_pOwner->OnNativeKillFocusMsg(pSetFocusWindow, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_IME_STARTCOMPOSITION:
    {
        lResult = m_pOwner->OnNativeImeStartCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_IME_ENDCOMPOSITION:
    {
        lResult = m_pOwner->OnNativeImeEndCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT) {
            //只处理设置客户区的光标
            lResult = m_pOwner->OnNativeSetCursorMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        break;
    }
    case WM_CONTEXTMENU:
    {
        UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if ((pt.x != -1) && (pt.y != -1)) {
            ScreenToClient(pt);
        }
        lResult = m_pOwner->OnNativeContextMenuMsg(pt, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeKeyDownMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeKeyUpMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_CHAR:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeCharMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_HOTKEY:
    {
        int32_t hotkeyId = (int32_t)wParam;
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>((int32_t)(int16_t)HIWORD(lParam));
        uint32_t modifierKey = (uint32_t)(int16_t)LOWORD(lParam);
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeHotKeyMsg(hotkeyId, vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEWHEEL:
    {
        int32_t wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(pt);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEMOVE:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseMoveMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEHOVER:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseHoverMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSELEAVE:
    {
        lResult = m_pOwner->OnNativeMouseLeaveMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONUP:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONDBLCLK:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonDbClickMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONDOWN:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONUP:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONDBLCLK:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonDbClickMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_CAPTURECHANGED:
    {
        lResult = m_pOwner->OnNativeCaptureChangedMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_CLOSE:
    {
        lResult = m_pOwner->OnNativeWindowCloseMsg((uint32_t)wParam, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    default:
        break;
    }//end of switch
    return lResult;
}

void NativeWindow::OnFinalMessage()
{
    if (m_pOwner) {
        m_pOwner->OnNativeFinalMessage();
    }
}

HWND NativeWindow::GetWindowOwner() const
{
    return ::GetWindow(m_hWnd, GW_OWNER);
}

} // namespace ui
