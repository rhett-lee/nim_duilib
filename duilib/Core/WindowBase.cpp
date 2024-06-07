#include "WindowBase.h"
#include "duilib/Utils/ApiWrapper.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/ApiWrapper.h"

namespace ui
{
WindowBase::WindowBase():
    m_hWnd(nullptr),
    m_pParentWindow(nullptr),
    m_OldWndProc(::DefWindowProc),
    m_bIsLayeredWindow(false),
    m_bFakeModal(false),
    m_bCloseing(false),
    m_hDcPaint(nullptr),
    m_bFullScreen(false),
    m_dwLastStyle(0),
    m_bUseSystemCaption(false),
    m_nWindowAlpha(255),
    m_bMouseCapture(false)
{
    m_rcLastWindowPlacement = { sizeof(WINDOWPLACEMENT), };
}

WindowBase::~WindowBase()
{
    ASSERT(m_hWnd == nullptr);
    ClearWindow();
}

bool WindowBase::CreateWnd(WindowBase* pParentWindow, const DString& windowName,
                           uint32_t dwStyle, uint32_t dwExStyle, const UiRect& rc)
{
    if (!GetSuperClassName().empty()) {
        if (!RegisterSuperClass()) {
            return false;
        }
    }
    else {
        if (!RegisterWindowClass()) {
            return false;
        }
    }
    DString className = GetWindowClassName();

    //初始化层窗口属性
    m_bIsLayeredWindow = false;
    if (dwExStyle & WS_EX_LAYERED) {
        m_bIsLayeredWindow = true;
    }
    m_pParentWindow = pParentWindow;
    m_parentFlag.reset();
    if (pParentWindow != nullptr) {
        m_parentFlag = pParentWindow->GetWeakFlag();
    }
    HWND hParentWnd = pParentWindow != nullptr ? pParentWindow->GetHWND() : nullptr;
    HWND hWnd = ::CreateWindowEx(dwExStyle,
                                 className.c_str(),
                                 windowName.c_str(),
                                 dwStyle,
                                 rc.left, rc.top, rc.Width(), rc.Height(),
                                 hParentWnd, NULL, GetResModuleHandle(), this);
    ASSERT(::IsWindow(hWnd));
    if (hWnd != m_hWnd) {
        m_hWnd = hWnd;
    }
    OnInitWindow();
    return hWnd != nullptr;
}

HMODULE WindowBase::GetResModuleHandle() const
{
    return ::GetModuleHandle(NULL);
}

DString WindowBase::GetWindowClassName() const
{
    ASSERT(FALSE);
    return DString();
}

DString WindowBase::GetSuperClassName() const
{
    return DString();
}

UINT WindowBase::GetClassStyle() const
{
    return CS_DBLCLKS;
}

uint32_t WindowBase::GetWindowStyle() const
{
    ASSERT(::IsWindow(GetHWND()));
    uint32_t styleValue = (uint32_t)::GetWindowLong(GetHWND(), GWL_STYLE);
    //使用自绘的标题栏：从原来窗口样式中，移除 WS_CAPTION 属性
    styleValue &= ~WS_CAPTION;
    return styleValue;
}

void WindowBase::SetUseSystemCaption(bool bUseSystemCaption)
{
    m_bUseSystemCaption = bUseSystemCaption;
    OnUseSystemCaptionBarChanged();
}

bool WindowBase::IsUseSystemCaption() const
{
    return m_bUseSystemCaption;
}

void WindowBase::SetWindowAlpha(int nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    m_nWindowAlpha = static_cast<uint8_t>(nAlpha);
    OnWindowAlphaChanged();
}

void WindowBase::SetLayeredWindow(bool bIsLayeredWindow)
{
    m_bIsLayeredWindow = bIsLayeredWindow;
    if (::IsWindow(m_hWnd)) {
        LONG dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        if (m_bIsLayeredWindow) {
            dwExStyle |= WS_EX_LAYERED;
        }
        else {
            dwExStyle &= ~WS_EX_LAYERED;
        }
        ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle);
    }
    OnLayeredWindowChanged();
}

bool WindowBase::IsLayeredWindow() const
{
    return m_bIsLayeredWindow;
}

uint8_t WindowBase::GetWindowAlpha() const
{
    return m_nWindowAlpha;
}

void WindowBase::CloseWnd(UINT nRet)
{
    m_bCloseing = true;
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::PostMessage(m_hWnd, WM_CLOSE, (WPARAM)nRet, 0L);
}

void WindowBase::Close()
{
    m_bCloseing = true;
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::SendMessage(m_hWnd, WM_CLOSE, 0L, 0L);
}

bool WindowBase::RegisterWindowClass()
{
    WNDCLASS wc = { 0 };
    wc.style = GetClassStyle();
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.lpfnWndProc = WindowBase::__WndProc;
    wc.hInstance = GetResModuleHandle();
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    DString className = GetWindowClassName();
    wc.lpszClassName = className.c_str();
    ATOM ret = ::RegisterClass(&wc);
    ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool WindowBase::RegisterSuperClass()
{
    // Get the class information from an existing
    // window so we can subclass it later on...
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    DString superClassName = GetSuperClassName();
    if (!::GetClassInfoEx(NULL, superClassName.c_str(), &wc)) {
        if (!::GetClassInfoEx(GetResModuleHandle(), superClassName.c_str(), &wc)) {
            ASSERT(!"Unable to locate window class");
            return false;
        }
    }
    m_OldWndProc = wc.lpfnWndProc;
    wc.lpfnWndProc = WindowBase::__ControlProc;
    wc.hInstance = GetResModuleHandle();
    DString className = GetWindowClassName();
    wc.lpszClassName = className.c_str();
    ATOM ret = ::RegisterClassEx(&wc);
    ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT CALLBACK WindowBase::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowBase* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<WindowBase*>(lpcs->lpCreateParams);
        if (pThis != nullptr) {
            pThis->m_hWnd = hWnd;
        }
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
    }
    else {
        pThis = reinterpret_cast<WindowBase*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (uMsg == WM_NCDESTROY && pThis != nullptr) {
            LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
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

static const wchar_t* sPropName = _T("DuiLibWndX"); // 属性名称

WindowBase* WindowBase::GetWindowObject(HWND hWnd)
{
    WindowBase* pThis = reinterpret_cast<WindowBase*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if ((pThis != nullptr) && (pThis->m_hWnd != hWnd)) {
        pThis = nullptr;
    }
    if (pThis == nullptr) {
        pThis = reinterpret_cast<WindowBase*>(::GetPropW(hWnd, sPropName));
        if ((pThis != nullptr) && (pThis->m_hWnd != hWnd)) {
            pThis = nullptr;
        }
    }
    return pThis;
}

LRESULT CALLBACK WindowBase::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowBase* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<WindowBase*>(lpcs->lpCreateParams);
        if (pThis != nullptr) {
            ::SetPropW(hWnd, sPropName, (HANDLE)pThis);
            pThis->m_hWnd = hWnd;
        }
    }
    else {
        pThis = reinterpret_cast<WindowBase*>(::GetPropW(hWnd, sPropName));
        if (uMsg == WM_NCDESTROY && pThis != nullptr) {
            LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
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

LRESULT WindowBase::CallDefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(m_OldWndProc != nullptr);
    return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
}

bool WindowBase::AddMessageFilter(IUIMessageFilter* pFilter)
{
    if (std::find(m_aMessageFilters.begin(), m_aMessageFilters.end(), pFilter) != m_aMessageFilters.end()) {
        ASSERT(false);
        return false;
    }
    if (pFilter != nullptr) {
        m_aMessageFilters.push_back(pFilter);
    }
    return true;
}
bool WindowBase::RemoveMessageFilter(IUIMessageFilter* pFilter)
{
    auto iter = std::find(m_aMessageFilters.begin(), m_aMessageFilters.end(), pFilter);
    if (iter != m_aMessageFilters.end()) {
        m_aMessageFilters.erase(iter);
        return true;
    }
    return false;
}

LRESULT WindowBase::SendMsg(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT WindowBase::PostMsg(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

HDC WindowBase::GetPaintDC() const
{
    return m_hDcPaint;
}

void WindowBase::Invalidate(const UiRect& rcItem)
{
    GlobalManager::Instance().AssertUIThread();
    RECT rc = { rcItem.left, rcItem.top, rcItem.right, rcItem.bottom };
    ::InvalidateRect(m_hWnd, &rc, FALSE);
    // Invalidating a layered window will not trigger a WM_PAINT message,
    // thus we have to post WM_PAINT by ourselves.
    if (m_bIsLayeredWindow) {
        ::PostMessage(m_hWnd, WM_PAINT, 0, 0);
    }
}

LRESULT WindowBase::WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hWnd = m_hWnd;
    if (uMsg == WM_CREATE) {
        //执行窗口的初始化工作        
        InitWindow();
    }

    //第一优先级：将消息发给过滤器进行过滤
    for (auto filter : m_aMessageFilters) {
        bool bHandled = false;
        LRESULT lResult = filter->FilterMessage(uMsg, wParam, lParam, bHandled);
        if (bHandled) {
            //过滤器处理后截获此消息，不再进行派发
            return lResult;
        }
    }

    //第二优先级：派发给子类回调函数
    bool bHandled = false;
    LRESULT lResult = OnWindowMessage(uMsg, wParam, lParam, bHandled);

    if (!bHandled && (uMsg == WM_CLOSE)) {
        //窗口即将关闭
        OnCloseWindow();
    }

    //第三优先级：内部处理函数，优先保证自身功能正常
    if (!bHandled) {
        lResult = HandleMessage(uMsg, wParam, lParam, bHandled);
    }

    //第四优先级：系统默认的窗口函数
    if (!bHandled && ::IsWindow(hWnd)) {
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
    }
    return lResult;
}

LRESULT WindowBase::OnWindowMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    bHandled = false;
    return 0;
}

WindowBase* WindowBase::WindowBaseFromPoint(const UiPoint& pt)
{
    WindowBase* pWindow = nullptr;
    HWND hWnd = ::WindowFromPoint({ pt.x, pt.y });
    if (::IsWindow(hWnd)) {
        if (hWnd == m_hWnd) {
            pWindow = this;
        }
        else {
            pWindow = GetWindowObject(hWnd);
        }
    }
    return pWindow;
}

HWND WindowBase::GetHWND() const
{
    return m_hWnd;
}

bool WindowBase::IsWindow() const
{
    return (m_hWnd != nullptr) && ::IsWindow(m_hWnd);
}

void WindowBase::InitWindow()
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

    //设置窗口风格
    HWND hWnd = GetHWND();
    uint32_t nStyle = GetWindowStyle();
    if (nStyle != 0) {
        ::SetWindowLong(hWnd, GWL_STYLE, nStyle);
    }

    //创建绘制设备上下文
    m_hDcPaint = ::GetDC(hWnd);
    ASSERT(m_hDcPaint != nullptr);

    //注册接受Touch消息
    RegisterTouchWindowWrapper(hWnd, 0);

    //初始化窗口自身的DPI管理器
    const DpiManager& dpiManager = GlobalManager::Instance().Dpi();
    if (!dpiManager.IsUserDefineDpi() && dpiManager.IsPerMonitorDpiAware()) {
        //每个显示器，有独立的DPI：初始化窗口自己的DPI管理器
        m_dpi = std::make_unique<DpiManager>();
        m_dpi->SetDpiByWindow(this);
    }
}

void WindowBase::ClearWindow()
{    
    //注销平板消息
    HWND hWnd = GetHWND();
    if (hWnd != nullptr) {
        UnregisterTouchWindowWrapper(hWnd);
    }

    if (m_hDcPaint != nullptr) {
        ::ReleaseDC(m_hWnd, m_hDcPaint);
        m_hDcPaint = nullptr;
    }
    m_pParentWindow = nullptr;
    m_parentFlag.reset();
    m_dpi.reset();
    m_hWnd = nullptr;
}

void WindowBase::OnInitWindow()
{
}

void WindowBase::OnCloseWindow()
{
}

void WindowBase::OnFinalMessage()
{
    ClearWindow();
    OnDeleteSelf();
}

void WindowBase::OnDeleteSelf()
{
    delete this;
}

void WindowBase::ScreenToClient(HWND hWnd, UiPoint& pt) const
{
    POINT ptClient = { pt.x, pt.y };
    ::ScreenToClient(hWnd, &ptClient);
    pt = UiPoint(ptClient.x, ptClient.y);
}

void WindowBase::ClientToScreen(HWND hWnd, UiPoint& pt) const
{
    POINT ptClient = { pt.x, pt.y };
    ::ClientToScreen(hWnd, &ptClient);
    pt = UiPoint(ptClient.x, ptClient.y);
}

void WindowBase::GetClientRect(HWND hWnd, UiRect& rcClient) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetClientRect(hWnd, &rc);
    rcClient = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void WindowBase::GetWindowRect(HWND hWnd, UiRect& rcWindow) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetWindowRect(hWnd, &rc);
    rcWindow = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

bool WindowBase::GetMonitorWorkRect(HWND hWnd, UiRect& rcWork) const
{
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
        return true;
    }
    else {
        ASSERT(!"WindowBase::GetMonitorWorkRect failed!");
        return false;
    }
}

bool WindowBase::GetMonitorRect(HWND hWnd, UiRect& rcMonitor, UiRect& rcWork) const
{
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
        ASSERT(!"WindowBase::GetMonitorWorkRect failed!");
        return false;
    }
}

void WindowBase::MapWindowRect(HWND hwndFrom, HWND hwndTo, UiRect& rc) const
{
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

HWND WindowBase::GetWindowOwner() const
{
    return ::GetWindow(m_hWnd, GW_OWNER);
}

void WindowBase::GetClientRect(UiRect& rcClient) const
{
    GetClientRect(m_hWnd, rcClient);
}

void WindowBase::GetWindowRect(UiRect& rcWindow) const
{
    GetWindowRect(m_hWnd, rcWindow);
}

void WindowBase::ScreenToClient(UiPoint& pt) const
{
    ScreenToClient(m_hWnd, pt);
}

void WindowBase::ClientToScreen(UiPoint& pt) const
{
    ClientToScreen(m_hWnd, pt);
}

void WindowBase::GetCursorPos(UiPoint& pt) const
{
    POINT ptPos;
    ::GetCursorPos(&ptPos);
    pt = { ptPos.x, ptPos.y };
}

void WindowBase::MapWindowDesktopRect(UiRect& rc) const
{
    ASSERT(IsWindow());
    MapWindowRect(GetHWND(), HWND_DESKTOP, rc);
}

bool WindowBase::GetMonitorRect(UiRect& rcMonitor, UiRect& rcWork) const
{
    return GetMonitorRect(GetHWND(), rcMonitor, rcWork);
}

bool WindowBase::GetMonitorWorkRect(UiRect& rcWork) const
{
    return GetMonitorWorkRect(m_hWnd, rcWork);
}

bool WindowBase::GetMonitorWorkRect(const UiPoint& pt, UiRect& rcWork) const
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
        ASSERT(!"WindowBase::GetMonitorWorkRect failed!");
        return false;
    }
}

void WindowBase::SetCapture()
{
    ::SetCapture(m_hWnd);
    m_bMouseCapture = true;
}

void WindowBase::ReleaseCapture()
{
    if (m_bMouseCapture) {
        ::ReleaseCapture();
        m_bMouseCapture = false;
    }
}

bool WindowBase::IsCaptured() const
{
    return m_bMouseCapture;
}

void WindowBase::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
    ASSERT(::IsWindow(m_hWnd));
    ::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
}

void WindowBase::ShowModalFake()
{
    ASSERT(::IsWindow(m_hWnd));
    WindowBase* pParentWindow = m_pParentWindow;
    ASSERT((pParentWindow != nullptr) && (pParentWindow->GetHWND() != nullptr));
    if (pParentWindow != nullptr) {
        auto hOwnerWnd = GetWindowOwner();
        ASSERT(::IsWindow(hOwnerWnd));
        ASSERT_UNUSED_VARIABLE(hOwnerWnd == pParentWindow->GetHWND());
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(false);
        }
    }
    ShowWindow();
    m_bFakeModal = true;
}

bool WindowBase::IsFakeModal() const
{
    return m_bFakeModal;
}

void WindowBase::CenterWindow()
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
    GetMonitorWorkRect(hWnd, rcArea);
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

void WindowBase::ToTopMost()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void WindowBase::BringToTop()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void WindowBase::ActiveWindow()
{
    ASSERT(::IsWindow(m_hWnd));
    if (IsWindowMinimized()) {
        ::ShowWindow(m_hWnd, SW_RESTORE);
    }
    else {
        if (!::IsWindowVisible(m_hWnd)) {
            ::ShowWindow(m_hWnd, SW_SHOW);
        }
        ::SetForegroundWindow(m_hWnd);
    }
}

bool WindowBase::Maximized()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    OnWindowMaximized();
    return true;
}

bool WindowBase::Restore()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    OnWindowRestored();
    return true;
}

bool WindowBase::Minimized()
{
    ASSERT(::IsWindow(m_hWnd));
    ::SendMessage(m_hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
    OnWindowMinimized();
    return true;
}

bool WindowBase::EnterFullScreen()
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
    //保存窗口风格
    m_dwLastStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    //保存窗口大小位置信息
    m_rcLastWindowPlacement.length = sizeof(WINDOWPLACEMENT);
    ::GetWindowPlacement(m_hWnd, &m_rcLastWindowPlacement);

    int32_t xScreen = GetSystemMetricsForDpiWrapper(SM_XVIRTUALSCREEN, Dpi().GetDPI());
    int32_t yScreen = GetSystemMetricsForDpiWrapper(SM_YVIRTUALSCREEN, Dpi().GetDPI());
    int32_t cxScreen = GetSystemMetricsForDpiWrapper(SM_CXVIRTUALSCREEN, Dpi().GetDPI());
    int32_t cyScreen = GetSystemMetricsForDpiWrapper(SM_CYVIRTUALSCREEN, Dpi().GetDPI());

    // 去掉标题栏、边框
    DWORD dwFullScreenStyle = (m_dwLastStyle | WS_VISIBLE | WS_POPUP | WS_MAXIMIZE) & ~WS_CAPTION & ~WS_BORDER & ~WS_THICKFRAME & ~WS_DLGFRAME;
    ::SetWindowLongPtr(m_hWnd, GWL_STYLE, dwFullScreenStyle);
    ::SetWindowPos(m_hWnd, NULL, xScreen, yScreen, cxScreen, cyScreen, SWP_FRAMECHANGED); // 设置位置和大小

    m_bFullScreen = true;
    OnWindowEnterFullScreen();
    return true;
}

bool WindowBase::ExitFullScreen()
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    if (!m_bFullScreen) {
        return false;
    }
    m_bFullScreen = false;

    //恢复窗口风格
    ::SetWindowLong(m_hWnd, GWL_STYLE, m_dwLastStyle);

    //恢复窗口位置/大小信息
    ::SetWindowPlacement(m_hWnd, &m_rcLastWindowPlacement);

    OnWindowExitFullScreen();

    if (IsWindowMaximized()) {
        Restore();
    }
    return true;
}

bool WindowBase::SetForeground()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetForegroundWindow() != m_hWnd) {
        ::SetForegroundWindow(m_hWnd);
    }
    return ::GetForegroundWindow() == m_hWnd;
}

bool WindowBase::SetWindowFocus()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
    return ::GetFocus() == m_hWnd;
}

bool WindowBase::KillWindowFocus()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetFocus() == m_hWnd) {
        ::SetFocus(nullptr);
    }
    return ::GetFocus() != m_hWnd;
}

bool WindowBase::IsWindowFocused() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetFocus());
}

bool WindowBase::SetOwnerWindowFocus()
{
    HWND hwndParent = GetWindowOwner();
    if (hwndParent != nullptr) {
        ::SetFocus(hwndParent);
        return ::GetFocus() == hwndParent;
    }
    return false;
}

void WindowBase::CheckSetWindowFocus()
{
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
}

bool WindowBase::IsWindowForeground() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetForegroundWindow());
}

bool WindowBase::IsWindowMaximized() const
{
    return ::IsWindow(m_hWnd) && ::IsZoomed(m_hWnd);
}

bool WindowBase::IsWindowMinimized() const
{
    return ::IsWindow(m_hWnd) && ::IsIconic(m_hWnd);
}

bool WindowBase::IsWindowFullScreen() const
{
    return m_bFullScreen;
}

bool WindowBase::UpdateWindow() const
{
    bool bRet = false;
    if ((m_hWnd != nullptr) && ::IsWindow(m_hWnd)) {
        bRet = ::UpdateWindow(m_hWnd) != FALSE;
    }
    return bRet;
}

bool WindowBase::IsZoomed() const
{
    return ::IsZoomed(GetHWND()) != FALSE;
}

bool WindowBase::IsIconic() const
{
    return ::IsIconic(GetHWND()) != FALSE;
}

bool WindowBase::EnableWindow(bool bEnable)
{
    return ::EnableWindow(GetHWND(), bEnable ? TRUE : false) != FALSE;
}

bool WindowBase::IsWindowEnabled() const
{
    return ::IsWindowEnabled(GetHWND()) != FALSE;
}

bool WindowBase::SetWindowPos(HWND hWndInsertAfter, int32_t X, int32_t Y, int32_t cx, int32_t cy, UINT uFlags)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::SetWindowPos(m_hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags) != FALSE;
}

bool WindowBase::SetWindowPos(const UiRect& rc, bool bNeedDpiScale, UINT uFlags, HWND hWndInsertAfter, bool bContainShadow)
{
    UiRect rcNewPos = rc;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(rcNewPos);
    }

    ASSERT(::IsWindow(m_hWnd));
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        rcNewPos.Inflate(rcShadow);
    }
    return SetWindowPos(hWndInsertAfter, rcNewPos.left, rcNewPos.top, rcNewPos.Width(), rcNewPos.Height(), uFlags);
}

UiRect WindowBase::GetWindowPos(bool bContainShadow) const
{
    ASSERT(IsWindow());
    UiRect rcPos;
    GetWindowRect(rcPos);
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        rcPos.Deflate(rcShadow);
    }
    return rcPos;
}

void WindowBase::Resize(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cy);
        Dpi().ScaleInt(cx);
    }

    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        cx += rcShadow.left + rcShadow.right;
        cy += rcShadow.top + rcShadow.bottom;
    }
    ASSERT(IsWindow());
    WindowBase::SetWindowPos(nullptr, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

void WindowBase::SetIcon(UINT nRes)
{
    ASSERT(::IsWindow(m_hWnd));
    HICON hIcon = (HICON)::LoadImage(GetResModuleHandle(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                                     GetSystemMetricsForDpiWrapper(SM_CXICON, Dpi().GetDPI()),
                                     GetSystemMetricsForDpiWrapper(SM_CYICON, Dpi().GetDPI()),
                                     LR_DEFAULTCOLOR | LR_SHARED);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
    hIcon = (HICON)::LoadImage(GetResModuleHandle(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
                               GetSystemMetricsForDpiWrapper(SM_CXSMICON, Dpi().GetDPI()),
                               GetSystemMetricsForDpiWrapper(SM_CYSMICON, Dpi().GetDPI()),
                               LR_DEFAULTCOLOR | LR_SHARED);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);
}

void WindowBase::SetText(const DString& strText)
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowText(m_hWnd, strText.c_str());
    m_text = strText;
}

DString WindowBase::GetText() const
{
    return m_text;
}

void WindowBase::SetTextId(const DString& strTextId)
{
    ASSERT(::IsWindow(m_hWnd));
    ::SetWindowText(m_hWnd, GlobalManager::Instance().Lang().GetStringViaID(strTextId).c_str());
    m_textId = strTextId;
}

DString WindowBase::GetTextId() const
{
    return m_textId;
}

const DpiManager& WindowBase::Dpi() const
{
    return (m_dpi != nullptr) ? *m_dpi : GlobalManager::Instance().Dpi();
}

bool WindowBase::ChangeDpi(uint32_t nNewDPI)
{
    ASSERT(m_hWnd != nullptr);
    if (m_hWnd == nullptr) {
        return false;
    }
    //DPI值限制在60到300之间(小于50的时候，会出问题，比如原来是1的，经过DPI转换后，会变成0，导致很多逻辑失效)
    ASSERT((nNewDPI >= 60) && (nNewDPI <= 300)) ;
    if ((nNewDPI < 60) || (nNewDPI > 300)) {
        return false;
    }

    uint32_t nOldDPI = Dpi().GetDPI();
    uint32_t nOldDpiScale = Dpi().GetScale();
    if (m_dpi == nullptr) {
        m_dpi = std::make_unique<DpiManager>();
    }
    //更新窗口的DPI值为新值
    m_dpi->SetDPI(nNewDPI);

    ASSERT(nNewDPI == m_dpi->GetDPI());
    uint32_t nNewDpiScale = m_dpi->GetScale();

    //按新的DPI更新窗口布局
    OnDpiScaleChanged(nOldDpiScale, nNewDpiScale);

    //更新窗口大小和位置
    UiRect rcOldWindow;
    GetWindowRect(rcOldWindow);
    UiRect rcNewWindow = Dpi().GetScaleRect(rcOldWindow, nOldDpiScale);
    ::SetWindowPos(m_hWnd,
                   NULL,
                   rcOldWindow.left,
                   rcOldWindow.top,
                   rcNewWindow.Width(),
                   rcNewWindow.Height(),
                   SWP_NOZORDER | SWP_NOACTIVATE);
    OnWindowDpiChanged(nOldDPI, nNewDPI);
    return true;
}

void WindowBase::ProcessDpiChangedMsg(uint32_t nNewDPI, const UiRect& rcNewWindow)
{
    //此消息必须处理，否则窗口大小与界面的比例将失调
    const DpiManager& dpiManager = GlobalManager::Instance().Dpi();
    if ((m_dpi != nullptr) && dpiManager.IsPerMonitorDpiAware()) {
        //调整DPI值
        uint32_t nOldDPI = m_dpi->GetDPI();
        uint32_t nOldDpiScale = m_dpi->GetScale();

        //更新窗口的DPI值为新值
        m_dpi->SetDPI(nNewDPI);
        ASSERT(nNewDPI == m_dpi->GetDPI());
        uint32_t nNewDpiScale = m_dpi->GetScale();

        //按新的DPI更新窗口布局
        OnDpiScaleChanged(nOldDpiScale, nNewDpiScale);

        //更新窗口的位置和大小
        if (!rcNewWindow.IsEmpty()) {
            ::SetWindowPos(m_hWnd, NULL,
                           rcNewWindow.left, rcNewWindow.top, rcNewWindow.Width(), rcNewWindow.Height(),
                           SWP_NOZORDER | SWP_NOACTIVATE);
        }
        OnWindowDpiChanged(nOldDPI, nNewDPI);
    }
}

void WindowBase::OnDpiScaleChanged(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if ((nOldDpiScale == nNewDpiScale) || (nNewDpiScale == 0)) {
        return;
    }
    ASSERT(nNewDpiScale == Dpi().GetScale());
    if (nNewDpiScale != Dpi().GetScale()) {
        return;
    }
    m_szMinWindow = Dpi().GetScaleSize(m_szMinWindow, nOldDpiScale);
    m_szMaxWindow = Dpi().GetScaleSize(m_szMaxWindow, nOldDpiScale);
    m_rcSizeBox = Dpi().GetScaleRect(m_rcSizeBox, nOldDpiScale);
    m_szRoundCorner = Dpi().GetScaleSize(m_szRoundCorner, nOldDpiScale);
    m_rcCaption = Dpi().GetScaleRect(m_rcCaption, nOldDpiScale);
}

void WindowBase::OnWindowDpiChanged(uint32_t /*nOldDPI*/, uint32_t /*nNewDPI*/)
{

}

bool WindowBase::SetWindowRoundRectRgn(const UiRect& rcWnd, const UiSize& szRoundCorner, bool bRedraw)
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

void WindowBase::ClearWindowRgn(bool bRedraw)
{
    ASSERT(IsWindow());
    ::SetWindowRgn(GetHWND(), nullptr, bRedraw ? TRUE : FALSE);
}

const UiRect& WindowBase::GetSizeBox() const
{
    return m_rcSizeBox;
}

void WindowBase::SetSizeBox(const UiRect& rcSizeBox, bool bNeedDpiScale)
{
    m_rcSizeBox = rcSizeBox;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(m_rcSizeBox);
    }
    if (m_rcSizeBox.left < 0) {
        m_rcSizeBox.left = 0;
    }
    if (m_rcSizeBox.top < 0) {
        m_rcSizeBox.top = 0;
    }
    if (m_rcSizeBox.right < 0) {
        m_rcSizeBox.right = 0;
    }
    if (m_rcSizeBox.bottom < 0) {
        m_rcSizeBox.bottom = 0;
    }
}

const UiRect& WindowBase::GetCaptionRect() const
{
    return m_rcCaption;
}

void WindowBase::SetCaptionRect(const UiRect& rcCaption, bool bNeedDpiScale)
{
    m_rcCaption = rcCaption;
    if (bNeedDpiScale) {
        Dpi().ScaleRect(m_rcCaption);
    }
}

const UiSize& WindowBase::GetRoundCorner() const
{
    return m_szRoundCorner;
}

void WindowBase::SetRoundCorner(int cx, int cy, bool bNeedDpiScale)
{
    ASSERT(cx >= 0);
    ASSERT(cy >= 0);
    if ((cx < 0) || (cy < 0)) {
        return;
    }
    //两个参数要么同时等于0，要么同时大于0，否则参数无效
    ASSERT(((cx > 0) && (cy > 0)) || ((cx == 0) && (cy == 0)));
    if (cx == 0) {
        if (cy != 0) {
            return;
        }
    }
    else {
        if (cy == 0) {
            return;
        }
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    m_szRoundCorner.cx = cx;
    m_szRoundCorner.cy = cy;
}

void WindowBase::OnUseSystemCaptionBarChanged()
{
    if (IsUseSystemCaption()) {
        //使用系统默认标题栏, 需要增加标题栏风格
        if (::IsWindow(GetHWND())) {
            UINT oldStyleValue = (UINT)::GetWindowLong(GetHWND(), GWL_STYLE);
            UINT newStyleValue = oldStyleValue | WS_CAPTION;
            if (newStyleValue != oldStyleValue) {
                ::SetWindowLong(GetHWND(), GWL_STYLE, newStyleValue);
            }
        }
        //关闭层窗口
        SetLayeredWindow(false);
    }
}

UiSize WindowBase::GetMinInfo(bool bContainShadow) const
{
    UiSize xy = m_szMinWindow;
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (xy.cx != 0) {
            xy.cx -= rcShadow.left + rcShadow.right;
        }
        if (xy.cy != 0) {
            xy.cy -= rcShadow.top + rcShadow.bottom;
        }
    }
    return xy;
}

void WindowBase::SetMinInfo(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (cx != 0) {
            cx += rcShadow.left + rcShadow.right;
        }
        if (cy != 0) {
            cy += rcShadow.top + rcShadow.bottom;
        }
    }
    m_szMinWindow.cx = cx;
    m_szMinWindow.cy = cy;
}

UiSize WindowBase::GetMaxInfo(bool bContainShadow) const
{
    UiSize xy = m_szMaxWindow;
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (xy.cx != 0) {
            xy.cx -= rcShadow.left + rcShadow.right;
        }
        if (xy.cy != 0) {
            xy.cy -= rcShadow.top + rcShadow.bottom;
        }
    }

    return xy;
}

void WindowBase::SetMaxInfo(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
    ASSERT(cx >= 0 && cy >= 0);
    if (cx < 0) {
        cx = 0;
    }
    if (cy < 0) {
        cy = 0;
    }
    if (bNeedDpiScale) {
        Dpi().ScaleInt(cx);
        Dpi().ScaleInt(cy);
    }
    if (!bContainShadow) {
        UiPadding rcShadow;
        GetShadowCorner(rcShadow);
        if (cx != 0) {
            cx += rcShadow.left + rcShadow.right;
        }
        if (cy != 0) {
            cy += rcShadow.top + rcShadow.bottom;
        }
    }
    m_szMaxWindow.cx = cx;
    m_szMaxWindow.cy = cy;
}

} // namespace ui
