#include "CefControlOffScreen.h"

#include "duilib/CEFControl/CefManager.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"
#include "duilib/CEFControl/internal/CefMemoryBlock.h"

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"

namespace ui {

#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))

CefControlOffScreen::CefControlOffScreen(Window* pWindow) :
    CefControlBase(pWindow),
    m_pDevToolView(nullptr)
{
    m_pCefMemData = std::make_unique<CefMemoryBlock>();
    m_pCefPopupMemData = std::make_unique<CefMemoryBlock>();
}

CefControlOffScreen::~CefControlOffScreen(void)
{
    if (m_pBrowserHandler.get()) {
        m_pBrowserHandler->SetHostWindow(nullptr);
        m_pBrowserHandler->SetHandlerDelegate(nullptr);

        if (m_pBrowserHandler->GetBrowser().get()) {
            // Request that the main browser close.
            m_pBrowserHandler->CloseAllBrowser();
        }
    }
    GetWindow()->RemoveMessageFilter(this);
}

void CefControlOffScreen::OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList& /*dirtyRects*/, const void* buffer, int width, int height)
{
    //只有离屏渲染才会走这个绘制接口
    //必须不使用缓存，否则绘制异常
    ASSERT(IsUseCache() == false);

    if ((buffer == nullptr) || (width <= 0) || (height <= 0)) {
        return;
    }

    if (type == PET_VIEW) {
        //页面的绘制数据
        m_pCefMemData->Init(buffer, width, height);
    }
    else if (type == PET_POPUP) {
        ////页面弹出窗口的绘制数据
        m_pCefPopupMemData->Init(buffer, width, height);
    }

    //在UI线程中调用Invalidate，触发绘制
    GlobalManager::Instance().Thread().PostTask(kThreadUI, UiBind(&CefControlOffScreen::Invalidate, this));
}

void CefControlOffScreen::ClientToControl(UiPoint& pt)
{
    auto offset = GetScrollOffsetInScrollBox();
    pt.x = pt.x + offset.x - GetRect().left;
    pt.y = pt.y + offset.y - GetRect().top;

    //传回的值，是未经DPI缩放的原始值(96 DPI)，然后CEF内部会进行DPI缩放
    pt.x = DpiManager::MulDiv(pt.x, 96, Dpi().GetDPI());
    pt.y = DpiManager::MulDiv(pt.y, 96, Dpi().GetDPI());
}

void CefControlOffScreen::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
    if (!show) {
        // 当popup窗口隐藏时，刷新popup区域
        m_rectPopup.Set(0, 0, 0, 0);
        if (browser->GetHost() != nullptr) {
            browser->GetHost()->Invalidate(PET_VIEW);
        }
    }
}

void CefControlOffScreen::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
    if ((rect.width <= 0) || (rect.height <= 0)) {
        return;
    }
    m_rectPopup = rect;
}

void CefControlOffScreen::Init()
{
    if (m_pBrowserHandler.get() == nullptr) {
        GetWindow()->AddMessageFilter(this);

        m_pBrowserHandler = new CefBrowserHandler;
        m_pBrowserHandler->SetHostWindow(GetWindow());
        m_pBrowserHandler->SetHandlerDelegate(this);
        ReCreateBrowser();
    }

    if (!m_jsBridge.get()) {
        m_jsBridge.reset(new CefJSBridge);
    }
    BaseClass::Init();
}

void CefControlOffScreen::ReCreateBrowser()
{
    if (m_pBrowserHandler->GetBrowser() == nullptr) {
        // 使用无窗模式，离屏渲染
        CefWindowInfo window_info;
#ifdef DUILIB_BUILD_FOR_WIN
        window_info.SetAsWindowless(GetWindow()->NativeWnd()->GetHWND());
#else

#endif
        CefBrowserSettings browser_settings;
        //背景色：设置为白色（如果不设置的话，是透明的，网页如果不设置背景色，则背景会被显示为黑色）
        browser_settings.background_color = CefColorSetARGB(255, 255, 255, 255);
        //browser_settings.file_access_from_file_urls = STATE_ENABLED;
        //browser_settings.universal_access_from_file_urls = STATE_ENABLED;
        CefBrowserHost::CreateBrowser(window_info, m_pBrowserHandler, _T(""), browser_settings, nullptr, nullptr);
    }
}

void CefControlOffScreen::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);

    if (m_pBrowserHandler.get()) {
        m_pBrowserHandler->SetViewRect({ rc.left, rc.top, rc.right, rc.bottom });
    }
}

void CefControlOffScreen::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            BaseClass::HandleEvent(msg);
        }
        return;
    }
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowser().get() == nullptr) {
        return BaseClass::HandleEvent(msg);
    }

    else if (msg.eventType == kEventSetFocus) {
        if (m_pBrowserHandler->GetBrowserHost().get()) {
            m_pBrowserHandler->GetBrowserHost()->SetFocus(true);
        }
    }
    else if (msg.eventType == kEventKillFocus) {
        if (m_pBrowserHandler->GetBrowserHost().get()) {
            m_pBrowserHandler->GetBrowserHost()->SetFocus(false);
        }
    }

    BaseClass::HandleEvent(msg);
}

void CefControlOffScreen::SetVisible(bool bVisible)
{
    BaseClass::SetVisible(bVisible);
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowserHost().get()) {
        m_pBrowserHandler->GetBrowserHost()->WasHidden(!bVisible);
    }
}

void CefControlOffScreen::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if ((pRender == nullptr) || (m_pBrowserHandler == nullptr) || (m_pBrowserHandler->GetBrowser() == nullptr)) {
        return;
    }

    if (m_pCefMemData->IsValid()) {
        // 绘制cef PET_VIEW类型的位图
        UiRect rect = GetRect();
        m_pCefMemData->PaintData(pRender, rcPaint, rect.left, rect.top);

        // 绘制cef PET_POPUP类型的位图
        if (!m_rectPopup.IsEmpty() && m_pCefPopupMemData->IsValid()) {
            // 假如popup窗口位置在控件的范围外，则修正到控件范围内，指绘制控件范围内的popup窗口
            UiRect dcPaint = GetRect();
            dcPaint.left += Dpi().GetScaleInt(m_rectPopup.x);
            dcPaint.top += Dpi().GetScaleInt(m_rectPopup.y);
            m_pCefPopupMemData->PaintData(pRender, rcPaint, dcPaint.left, dcPaint.top);
        }
    }
}

void CefControlOffScreen::SetWindow(Window* pWindow)
{
    if (!m_pBrowserHandler) {
        BaseClass::SetWindow(pWindow);
        return;
    }

    if (GetWindow()) {
        GetWindow()->RemoveMessageFilter(this);
    }
    BaseClass::SetWindow(pWindow);
    if (pWindow != nullptr) {
        pWindow->AddMessageFilter(this);
    }
    m_pBrowserHandler->SetHostWindow(pWindow);
}

LRESULT CefControlOffScreen::FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if (!IsVisible() || !IsEnabled()) {
        bHandled = false;
        return 0;
    }

    bHandled = false;
    if ((m_pBrowserHandler.get() == nullptr) || (m_pBrowserHandler->GetBrowser().get() == nullptr)) {
        return 0;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    switch (uMsg)
    {
    case WM_MOUSEMOVE:
    {
        return SendMouseMoveEvent(uMsg, wParam, lParam, bHandled);
    }
    break;

    case WM_SETCURSOR:
    {
        // 这里拦截WM_SETCURSOR消息，不让duilib处理（duilib会改变光标样式），否则会影响Cef中的鼠标光标
        UiPoint pt;
        GetWindow()->GetCursorPos(pt);
        GetWindow()->ScreenToClient(pt);
        if (!GetRect().ContainsPt(pt)) {
            return 0;
        }

        GetWindow()->NativeWnd()->CallDefaultWindowProc(uMsg, wParam, lParam);
        bHandled = true;
        return 0;
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        return SendButtonDownEvent(uMsg, wParam, lParam, bHandled);
    }
    break;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        return SendButtonUpEvent(uMsg, wParam, lParam, bHandled);
    }
    break;

    case WM_SYSCHAR:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_CHAR:
    {
        if (IsFocused())
        {
            return SendKeyEvent(uMsg, wParam, lParam, bHandled);
        }
    }
    break;

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    {
        return SendButtonDoubleDownEvent(uMsg, wParam, lParam, bHandled);
    }
    break;

    case WM_CAPTURECHANGED:
    case WM_CANCELMODE:
    {
        return SendCaptureLostEvent(uMsg, wParam, lParam, bHandled);
    }
    break;

    case WM_MOUSEWHEEL:
    {
        return SendMouseWheelEvent(uMsg, wParam, lParam, bHandled);
    }
    break;

    case WM_MOUSELEAVE:
    {
        return SendMouseLeaveEvent(uMsg, wParam, lParam, bHandled);
    }
    break;

    default:
        bHandled = false;
    }
#endif
    return 0;
}

bool CefControlOffScreen::AttachDevTools(Control* control)
{
    CefControlOffScreen* view = dynamic_cast<CefControlOffScreen*>(control);
    if (IsAttachedDevTools() || !view) {
        return true;
    }
    if ((m_pBrowserHandler == nullptr) || (view->m_pBrowserHandler == nullptr)) {
        return false;
    }
    auto browser = m_pBrowserHandler->GetBrowser();
    auto view_browser = view->m_pBrowserHandler->GetBrowser();
    if ((browser == nullptr) || (view_browser == nullptr)) {
        auto weak = view->GetWeakFlag();
        auto task = [this, weak, view]() {
            GlobalManager::Instance().Thread().PostTask(kThreadUI, ToWeakCallback([this, weak, view]() {
                    if (weak.expired()) {
                        return;
                    }
                    AttachDevTools(view);
                }));
            };
        view->m_pBrowserHandler->AddAfterCreateTask(task);
    }
    else {
        CefWindowInfo windowInfo;
#ifdef DUILIB_BUILD_FOR_WIN
        windowInfo.SetAsWindowless(GetWindow()->NativeWnd()->GetHWND());
#else

#endif

        CefBrowserSettings settings;
        if ((browser->GetHost() != nullptr) && (view_browser->GetHost() != nullptr)) {
            browser->GetHost()->ShowDevTools(windowInfo, view_browser->GetHost()->GetClient(), settings, CefPoint());
            SetAttachedDevTools(true);
            m_pDevToolView = view;
            OnDevToolsVisibleChanged();
        }
    }
    return IsAttachedDevTools();
}

void CefControlOffScreen::DettachDevTools()
{
    CefControlBase::DettachDevTools();
    m_pDevToolView = nullptr;
}

void CefControlOffScreen::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
    if (IsAttachedDevTools() && (m_pDevToolView != nullptr) && CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        //离屏渲染模式，开发者工具与页面位于相同的客户区位置
        int x = params->GetXCoord();
        int y = params->GetYCoord();
        //离屏渲染模式下，给到的参数是原始坐标，未经DPI自适应，所以需要做DPI自适应处理，否则页面的右键菜单位置显示不对
        uint32_t dpiScale = 100;
        if (GetWindow() != nullptr) {
            dpiScale = GetWindow()->Dpi().GetScale();
        }
        if (dpiScale > 100) {
            x = x * dpiScale / 100;
            y = y * dpiScale / 100;
        }

        UiPoint pt = { x + GetRect().left, y + GetRect().top };
        UiPoint offsetPt = GetScrollOffsetInScrollBox();
        pt.Offset(offsetPt);
        UiRect rect = GetRect();
        UiRect rectView = m_pDevToolView->GetRect();
        bool isPtInPageRect = GetRect().ContainsPt(pt);
        bool isPtInToolRect = m_pDevToolView->GetRect().ContainsPt(pt);
        if (isPtInToolRect && !isPtInPageRect) {
            //如果点击区域，位于开发工具区域，则不弹出页面的右键菜单
            if (model->GetCount() > 0) {
                // 禁止右键菜单
                model->Clear();
            }
            return;
        }
    }
    CefControlBase::OnBeforeContextMenu(browser, frame, params, model);
}

//////////////////////////////////////////////////////////////////////////////////
LRESULT CefControlOffScreen::SendButtonDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
#ifdef DUILIB_BUILD_FOR_WIN
    CefRefPtr<CefBrowserHost> host = m_pBrowserHandler->GetBrowserHost();

    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt)) {
        return 0;
    }

    this->SetFocus();
    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);

    CefBrowserHost::MouseButtonType btnType =
        (uMsg == WM_LBUTTONDOWN ? MBT_LEFT : (
            uMsg == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
    AdaptDpiScale(mouse_event);
    host->SendMouseClickEvent(mouse_event, btnType, false, 1);
#endif
    bHandled = false;
    return 0;
}

LRESULT CefControlOffScreen::SendButtonDoubleDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
#ifdef DUILIB_BUILD_FOR_WIN
    CefRefPtr<CefBrowserHost> host = m_pBrowserHandler->GetBrowserHost();

    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt)) {
        return 0;
    }

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);
    AdaptDpiScale(mouse_event);
    CefBrowserHost::MouseButtonType btnType =
        (uMsg == WM_LBUTTONDOWN ? MBT_LEFT : (
            uMsg == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));

    host->SendMouseClickEvent(mouse_event, btnType, false, 2);
#endif
    bHandled = true;
    return 0;
}

LRESULT CefControlOffScreen::SendButtonUpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
#ifdef DUILIB_BUILD_FOR_WIN
    CefRefPtr<CefBrowserHost> host = m_pBrowserHandler->GetBrowserHost();

    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt) && !GetWindow()->IsCaptured()) {
        return 0;
    }

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);
    AdaptDpiScale(mouse_event);
    CefBrowserHost::MouseButtonType btnType =
        (uMsg == WM_LBUTTONUP ? MBT_LEFT : (
            uMsg == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));

    host->SendMouseClickEvent(mouse_event, btnType, true, 1);
#endif
    bHandled = false;
    return 0;
}

LRESULT CefControlOffScreen::SendMouseMoveEvent(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
#ifdef DUILIB_BUILD_FOR_WIN
    CefRefPtr<CefBrowserHost> host = m_pBrowserHandler->GetBrowserHost();

    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt) && !GetWindow()->IsCaptured()) {
        return 0;
    }

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);
    AdaptDpiScale(mouse_event);
    host->SendMouseMoveEvent(mouse_event, false);
#endif
    bHandled = false;
    return 0;
}

LRESULT CefControlOffScreen::SendMouseWheelEvent(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
#ifdef DUILIB_BUILD_FOR_WIN
    CefRefPtr<CefBrowserHost> host = m_pBrowserHandler->GetBrowserHost();

    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    Window* pScrolledWnd = GetWindow()->WindowFromPoint(pt);
    if (pScrolledWnd != GetWindow()) {
        return 0;
    }

    GetWindow()->ScreenToClient(pt);
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt)) {
        return 0;
    }

    int delta = GET_WHEEL_DELTA_WPARAM(wParam);

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);
    AdaptDpiScale(mouse_event);
    host->SendMouseWheelEvent(mouse_event, IsKeyDown(VK_SHIFT) ? delta : 0, !IsKeyDown(VK_SHIFT) ? delta : 0);

#endif
    bHandled = true;
    return 0;
}

LRESULT CefControlOffScreen::SendMouseLeaveEvent(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = m_pBrowserHandler->GetBrowserHost();

    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt)) {
        return 0;
    }

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(wParam);
    AdaptDpiScale(mouse_event);
    host->SendMouseMoveEvent(mouse_event, true);

    bHandled = true;
    return 0;
}

LRESULT CefControlOffScreen::SendKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
#ifdef DUILIB_BUILD_FOR_WIN
    CefRefPtr<CefBrowserHost> host = m_pBrowserHandler->GetBrowserHost();

    CefKeyEvent event;
    event.windows_key_code = static_cast<int>(wParam);
    event.native_key_code = static_cast<int>(lParam);
    event.is_system_key = uMsg == WM_SYSCHAR || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP;

    if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
        event.type = KEYEVENT_RAWKEYDOWN;
    else if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP)
        event.type = KEYEVENT_KEYUP;
    else
        event.type = KEYEVENT_CHAR;
    event.modifiers = GetCefKeyboardModifiers(wParam, lParam);

    host->SendKeyEvent(event);
#endif

    bHandled = true;
    return 0;
}

LRESULT CefControlOffScreen::SendCaptureLostEvent(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host = m_pBrowserHandler->GetBrowserHost();

    host->SendCaptureLostEvent();
    bHandled = true;
    return 0;
}

bool CefControlOffScreen::IsKeyDown(WPARAM wparam)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return (::GetKeyState(static_cast<int>(wparam)) & 0x8000) != 0;
#else
    return false;
#endif
}

int CefControlOffScreen::GetCefMouseModifiers(WPARAM wparam)
{
    int modifiers = 0;
#ifdef DUILIB_BUILD_FOR_WIN
    if (wparam & MK_CONTROL)
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    if (wparam & MK_SHIFT)
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (IsKeyDown(VK_MENU))
        modifiers |= EVENTFLAG_ALT_DOWN;
    if (wparam & MK_LBUTTON)
        modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
    if (wparam & MK_MBUTTON)
        modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
    if (wparam & MK_RBUTTON)
        modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

    // Low bit set from GetKeyState indicates "toggled".
    if (::GetKeyState(VK_NUMLOCK) & 1)
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    if (::GetKeyState(VK_CAPITAL) & 1)
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;
#endif
    return modifiers;
}

int CefControlOffScreen::GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam)
{
    int modifiers = 0;

#ifdef DUILIB_BUILD_FOR_WIN
    if (IsKeyDown(VK_SHIFT))
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (IsKeyDown(VK_CONTROL))
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    if (IsKeyDown(VK_MENU))
        modifiers |= EVENTFLAG_ALT_DOWN;

    // Low bit set from GetKeyState indicates "toggled".
    if (::GetKeyState(VK_NUMLOCK) & 1)
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    if (::GetKeyState(VK_CAPITAL) & 1)
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;

    switch (wparam) {
    case VK_RETURN:
        if ((lparam >> 16) & KF_EXTENDED)
            modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
        if (!((lparam >> 16) & KF_EXTENDED))
            modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_NUMLOCK:
    case VK_NUMPAD0:
    case VK_NUMPAD1:
    case VK_NUMPAD2:
    case VK_NUMPAD3:
    case VK_NUMPAD4:
    case VK_NUMPAD5:
    case VK_NUMPAD6:
    case VK_NUMPAD7:
    case VK_NUMPAD8:
    case VK_NUMPAD9:
    case VK_DIVIDE:
    case VK_MULTIPLY:
    case VK_SUBTRACT:
    case VK_ADD:
    case VK_DECIMAL:
    case VK_CLEAR:
        modifiers |= EVENTFLAG_IS_KEY_PAD;
        break;
    case VK_SHIFT:
        if (IsKeyDown(VK_LSHIFT))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (IsKeyDown(VK_RSHIFT))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_CONTROL:
        if (IsKeyDown(VK_LCONTROL))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (IsKeyDown(VK_RCONTROL))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_MENU:
        if (IsKeyDown(VK_LMENU))
            modifiers |= EVENTFLAG_IS_LEFT;
        else if (IsKeyDown(VK_RMENU))
            modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    case VK_LWIN:
        modifiers |= EVENTFLAG_IS_LEFT;
        break;
    case VK_RWIN:
        modifiers |= EVENTFLAG_IS_RIGHT;
        break;
    }
#endif
    return modifiers;
}

void CefControlOffScreen::AdaptDpiScale(CefMouseEvent& mouse_event)
{
    if (CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        //离屏渲染模式，需要传给原始宽度和高度，因为CEF内部会进一步做DPI自适应
        uint32_t dpiScale = 100;
        if (GetWindow() != nullptr) {
            dpiScale = GetWindow()->Dpi().GetScale();
        }
        if (dpiScale > 100) {
            mouse_event.x = mouse_event.x * 100 / dpiScale;
            mouse_event.y = mouse_event.y * 100 / dpiScale;
        }
    }
}

} //namespace ui
