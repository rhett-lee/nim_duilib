#include "CefControlOffScreen.h"

#include "duilib/CEFControl/CefManager.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"
#include "duilib/CEFControl/internal/CefMemoryBlock.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    #include "duilib/CEFControl/internal/Windows/util_win.h"
    #include "duilib/CEFControl/internal/Windows/osr_ime_handler_win.h"
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"

#if defined (DUILIB_BUILD_FOR_SDL)
    #include <SDL3/SDL.h>
#endif

namespace ui {

CefControlOffScreen::CefControlOffScreen(Window* pWindow) :
    CefControl(pWindow),
    m_bHasFocusNode(false),
    m_bFocusNodeEditable(false)
{
    m_pCefMemData = std::make_unique<CefMemoryBlock>();
    m_pCefPopupMemData = std::make_unique<CefMemoryBlock>();
}

CefControlOffScreen::~CefControlOffScreen(void)
{
    DoCloseAllBrowsers(true);
    if (m_pBrowserHandler.get()) {
        m_pBrowserHandler->SetHostWindow(nullptr);
        m_pBrowserHandler->SetHandlerDelegate(nullptr);
    }
}

void CefControlOffScreen::OnPaint(CefRefPtr<CefBrowser> /*browser*/, CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList& dirtyRects, const void* buffer, int width, int height)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    //只有离屏渲染才会走这个绘制接口
    //必须不使用缓存，否则绘制异常
    ASSERT(IsUseCache() == false);

    if ((buffer == nullptr) || (width <= 0) || (height <= 0)) {
        return;
    }

    std::vector<UiRect> dirtyRectList;
    for (const CefRect& rect : dirtyRects) {
        dirtyRectList.push_back(UiRect(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height));
    }
    if (type == PET_VIEW) {
        //页面的绘制数据        
        m_pCefMemData->Init(buffer, dirtyRectList, width, height);
    }
    else if (type == PET_POPUP) {
        ////页面弹出窗口的绘制数据
        m_pCefPopupMemData->Init(buffer, dirtyRectList, width, height);
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
    ASSERT(CefCurrentlyOn(TID_UI));
    if (!show) {
        // 当popup窗口隐藏时，刷新popup区域
        m_rectPopup.Set(0, 0, 0, 0);
        if (browser->GetHost() != nullptr) {
            browser->GetHost()->Invalidate(PET_VIEW);
        }
    }
}

void CefControlOffScreen::OnPopupSize(CefRefPtr<CefBrowser> /*browser*/, const CefRect& rect)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if ((rect.width <= 0) || (rect.height <= 0)) {
        return;
    }
    m_rectPopup = rect;
}

void CefControlOffScreen::Init()
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pBrowserHandler.get() == nullptr) {
        m_pBrowserHandler = new CefBrowserHandler;
        m_pBrowserHandler->SetHostWindow(GetWindow());
        m_pBrowserHandler->SetHandlerDelegate(this);

        //异步创建Browser对象, 避免阻塞主界面的解析和显示速度
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefControlOffScreen::ReCreateBrowser, this));
    }

    if (!m_jsBridge.get()) {
        m_jsBridge.reset(new CefJSBridge);
    }
    BaseClass::Init();
}

void CefControlOffScreen::ReCreateBrowser()
{
    GlobalManager::Instance().AssertUIThread();
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return;
    }
    ASSERT(pWindow->IsWindow());
    ASSERT(m_pBrowserHandler != nullptr);
    if (m_pBrowserHandler == nullptr) {
        return;
    }

    if (m_pBrowserHandler->GetBrowser() != nullptr) {
        //已经完成创建，不再重复创建Browser对象
        return;
    }

    // 使用无窗模式，离屏渲染
    CefWindowInfo window_info;
#if CEF_VERSION_MAJOR > 109
    //该参数必须显示初始化，不能使用默认值
    window_info.runtime_style = CEF_RUNTIME_STYLE_ALLOY;
#endif

#ifdef DUILIB_BUILD_FOR_WIN
    HWND hWnd = pWindow->NativeWnd()->GetHWND();
    window_info.SetAsWindowless(hWnd);
    if (::GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_NOACTIVATE) {
        // Don't activate the browser window on creation.
        window_info.ex_style |= WS_EX_NOACTIVATE;
    }
#elif defined (DUILIB_BUILD_FOR_LINUX) || defined (DUILIB_BUILD_FOR_FREEBSD)
    window_info.SetAsWindowless(pWindow->NativeWnd()->GetX11WindowNumber());
#elif defined DUILIB_BUILD_FOR_MACOS
    window_info.SetAsWindowless(pWindow->NativeWnd()->GetNSView());
#endif
    CefBrowserSettings browser_settings;
    //背景色：设置为白色（如果不设置的话，是透明的，网页如果不设置背景色，则背景会被显示为黑色）
    browser_settings.background_color = CefColorSetARGB(255, 255, 255, 255);
    //browser_settings.file_access_from_file_urls = STATE_ENABLED;
    //browser_settings.universal_access_from_file_urls = STATE_ENABLED;
    CefString url = GetInitURL();//创建成功后，立即加载的URL
    CefBrowserHost::CreateBrowser(window_info, m_pBrowserHandler, url, browser_settings, nullptr, nullptr);
}

void CefControlOffScreen::SetPos(UiRect rc)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::SetPos(rc);

    if (m_pBrowserHandler.get()) {
        m_pBrowserHandler->SetViewRect({ rc.left, rc.top, rc.right, rc.bottom });
    }
}

void CefControlOffScreen::OnSetVisible(bool bChanged)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::OnSetVisible(bChanged);
    if (m_pBrowserHandler.get() && m_pBrowserHandler->GetBrowserHost().get()) {
        m_pBrowserHandler->GetBrowserHost()->WasHidden(!IsVisible());
    }
}

void CefControlOffScreen::Paint(IRender* pRender, const UiRect& rcPaint)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::Paint(pRender, rcPaint);
    if ((pRender == nullptr) || (m_pBrowserHandler == nullptr) || (m_pBrowserHandler->GetBrowser() == nullptr)) {
        return;
    }

    if (m_pCefMemData->IsValid()) {
        // 绘制cef PET_VIEW类型的位图
        const UiRect rect = GetRect();
        bool bRectValid = true;
        if ((m_pCefMemData->GetWidth() != rect.Width()) || (m_pCefMemData->GetHeight() != rect.Height())) {            
            bRectValid = false;
        }

        if (!rcPaint.IsEmpty()) {
            if (bRectValid) {
                m_pCefMemData->PaintData(pRender, rect);
            }
            else {
                //如果区域不匹配，不绘制，再次触发一次绘制事件（避免绘制超出控件边界，覆盖其他控件）
                m_pBrowserHandler->SetViewRect(rect);
            }
        }

        // 绘制cef PET_POPUP类型的位图
        if (!m_rectPopup.IsEmpty() && m_pCefPopupMemData->IsValid()) {
            // 假如popup窗口位置在控件的范围外，则修正到控件范围内，指绘制控件范围内的popup窗口
            UiRect dcPaint = GetRect();
            dcPaint.left += Dpi().GetScaleInt(m_rectPopup.x);
            dcPaint.top += Dpi().GetScaleInt(m_rectPopup.y);
            dcPaint.right = dcPaint.left + m_pCefPopupMemData->GetWidth();
            dcPaint.bottom = dcPaint.top + m_pCefPopupMemData->GetHeight();
            if (!rcPaint.IsEmpty()) {
                m_pCefPopupMemData->PaintData(pRender, dcPaint);
            }
        }
    }
}

void CefControlOffScreen::SetWindow(Window* pWindow)
{
    GlobalManager::Instance().AssertUIThread();
    BaseClass::SetWindow(pWindow);
    if (m_pBrowserHandler) {
        m_pBrowserHandler->SetHostWindow(pWindow);
        m_pBrowserHandler->SetHandlerDelegate(this);
    }    
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

#ifdef DUILIB_BUILD_FOR_SDL

// 将 CEF 光标类型转换为 duilib 标准光标类型(指支持部分光标类型)
static CursorType CefCursorTypeToUiCursor(cef_cursor_type_t cefCursor)
{
    switch (cefCursor) {
    case CT_POINTER:           return CursorType::kCursorArrow;          // 指针 -> 标准箭头
    case CT_CROSS:             return CursorType::kCursorCross;          // 十字光标 -> 十字线
    case CT_HAND:              return CursorType::kCursorHand;           // 手型光标 -> 手型
    case CT_IBEAM:             return CursorType::kCursorIBeam;          // 文本光标 -> I型光标
    case CT_WAIT:              return CursorType::kCursorWait;           // 等待光标 -> 沙漏
    //case CT_HELP:              return IDC_HELP;           // 帮助光标 -> 帮助箭头

        // 方向调整光标
    case CT_EASTRESIZE:        return CursorType::kCursorSizeWE;         // 东向调整 -> 水平调整
    case CT_NORTHRESIZE:       return CursorType::kCursorSizeNS;         // 北向调整 -> 垂直调整
    case CT_NORTHEASTRESIZE:   return CursorType::kCursorSizeNESW;       // 东北向调整
    case CT_NORTHWESTRESIZE:   return CursorType::kCursorSizeNWSE;       // 西北向调整
    case CT_SOUTHRESIZE:       return CursorType::kCursorSizeNS;         // 南向调整 -> 垂直调整
    case CT_SOUTHEASTRESIZE:   return CursorType::kCursorSizeNWSE;       // 东南向调整
    case CT_SOUTHWESTRESIZE:   return CursorType::kCursorSizeNESW;       // 西南向调整
    case CT_WESTRESIZE:        return CursorType::kCursorSizeWE;         // 西向调整 -> 水平调整

        // 双向调整光标
    case CT_NORTHSOUTHRESIZE:  return CursorType::kCursorSizeNS;          // 南北调整 -> 垂直调整
    case CT_EASTWESTRESIZE:    return CursorType::kCursorSizeWE;          // 东西调整 -> 水平调整
    case CT_NORTHEASTSOUTHWESTRESIZE: return CursorType::kCursorSizeNESW; // 东北-西南调整
    case CT_NORTHWESTSOUTHEASTRESIZE: return CursorType::kCursorSizeNWSE; // 西北-东南调整

        // 其他可映射类型
    case CT_COLUMNRESIZE:      return CursorType::kCursorSizeWE;         // 列调整 -> 水平调整
    case CT_ROWRESIZE:         return CursorType::kCursorSizeNS;         // 行调整 -> 垂直调整
    case CT_MOVE:              return CursorType::kCursorSizeAll;        // 移动 -> 四向调整
    case CT_PROGRESS:          return CursorType::kCursorProgress;       // 进度 -> 应用启动光标
    case CT_NODROP:            return CursorType::kCursorNo;             // 禁止放置 -> 禁止光标
    case CT_NOTALLOWED:        return CursorType::kCursorNo;             // 不允许 -> 禁止光标
    case CT_COPY:              return CursorType::kCursorArrow;          // 复制 -> 标准箭头（可自定义）

        // 以下类型无直接对应Windows标准光标，返回空
    case CT_MIDDLEPANNING:
    case CT_EASTPANNING:
    case CT_NORTHPANNING:
    case CT_NORTHEASTPANNING:
    case CT_NORTHWESTPANNING:
    case CT_SOUTHPANNING:
    case CT_SOUTHEASTPANNING:
    case CT_SOUTHWESTPANNING:
    case CT_WESTPANNING:
    case CT_VERTICALTEXT:
    case CT_CELL:
    case CT_CONTEXTMENU:
    case CT_ALIAS:
    case CT_NONE:
    case CT_ZOOMIN:
    case CT_ZOOMOUT:
    case CT_GRAB:
    case CT_GRABBING:
    case CT_MIDDLE_PANNING_VERTICAL:
    case CT_MIDDLE_PANNING_HORIZONTAL:
    case CT_CUSTOM:
    case CT_DND_NONE:
    case CT_DND_MOVE:
    case CT_DND_COPY:
    case CT_DND_LINK:
    //case CT_NUM_VALUES:
    default:
        break;
    }
    return CursorType::kCursorArrow;
}

#endif //DUILIB_BUILD_FOR_SDL

void CefControlOffScreen::OnCursorChange(cef_cursor_type_t type)
{
#ifdef DUILIB_BUILD_FOR_SDL
    CursorType uiCursorType = CefCursorTypeToUiCursor(type);
    SetCursorType(uiCursorType);
#else
    (void)type;
#endif
}

bool CefControlOffScreen::OnSetCursor(const EventArgs& msg)
{
#ifdef DUILIB_BUILD_FOR_SDL
    //使用SDL时，需要设置光标
    return BaseClass::OnSetCursor(msg);
#else
    //离屏渲染时，控件本身不处理光标，由CEF模块内部处理光标，否则会影响Cef中的鼠标光标
    (void)msg;
    return true;
#endif
}

bool CefControlOffScreen::OnCaptureChanged(const EventArgs& /*msg*/)
{
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    if (host != nullptr) {
        host->SendCaptureLostEvent();
    }
    return false;
}

int32_t CefControlOffScreen::GetCefMouseModifiers(const EventArgs& /*msg*/) const
{
    int32_t modifiers = 0;
    if (Keyboard::IsKeyDown(kVK_CONTROL)) {
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    }
    if (Keyboard::IsKeyDown(kVK_SHIFT)) {
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if (Keyboard::IsKeyDown(kVK_MENU)) {
        modifiers |= EVENTFLAG_ALT_DOWN;
    }
    if (Keyboard::IsKeyDown(kVK_LBUTTON)) {
        modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
    }
    if (Keyboard::IsKeyDown(kVK_MBUTTON)) {
        modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
    }
    if (Keyboard::IsKeyDown(kVK_RBUTTON)) {
        modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
    }

    // Low bit set from GetKeyState indicates "toggled".
    if (Keyboard::IsKeyDown(kVK_NUMLOCK)) {
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    }
    if (Keyboard::IsKeyDown(kVK_CAPITAL)) {
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;
    }
    return modifiers;
}

bool CefControlOffScreen::MouseMove(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseMove(msg);
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    Window* pWindow = GetWindow();
    if ((pWindow != nullptr) && (host != nullptr)) {
        UiPoint pt = msg.ptMouse;
        pt.Offset(GetScrollOffsetInScrollBox());
        if (!GetRect().ContainsPt(pt) && !pWindow->IsCaptured()) {
            return false;
        }

        CefMouseEvent mouse_event;
        mouse_event.x = pt.x - GetRect().left;
        mouse_event.y = pt.y - GetRect().top;
        mouse_event.modifiers = GetCefMouseModifiers(msg);
        AdaptDpiScale(mouse_event);
        host->SendMouseMoveEvent(mouse_event, false);
    }
    return bRet;
}

bool CefControlOffScreen::MouseLeave(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseLeave(msg);
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    if (host == nullptr) {
        return bRet;
    }

    UiPoint pt = msg.ptMouse;
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt)) {
        return bRet;
    }

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(msg);
    AdaptDpiScale(mouse_event);
    host->SendMouseMoveEvent(mouse_event, true);
    return bRet;
}

bool CefControlOffScreen::MouseWheel(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseWheel(msg);
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    if (host == nullptr) {
        return bRet;
    }
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return bRet;
    }

    UiPoint pt = msg.ptMouse;
    Window* pScrolledWnd = pWindow->WindowFromPoint(pt);
    if (pScrolledWnd != pWindow) {
        return bRet;
    }

    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt)) {
        return bRet;
    }

    int delta = msg.eventData;
    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(msg);
    AdaptDpiScale(mouse_event);
    bool bShiftDown = Keyboard::IsKeyDown(kVK_SHIFT);
    host->SendMouseWheelEvent(mouse_event, bShiftDown ? delta : 0, !bShiftDown ? delta : 0);
    return bRet;
}

bool CefControlOffScreen::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    SendButtonDownEvent(msg);
    return bRet;
}

bool CefControlOffScreen::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    SendButtonUpEvent(msg);
    return bRet;
}

bool CefControlOffScreen::ButtonDoubleClick(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDoubleClick(msg);
    SendButtonDoubleClickEvent(msg);
    return bRet;
}

bool CefControlOffScreen::RButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::RButtonDown(msg);
    SendButtonDownEvent(msg);
    return bRet;
}

bool CefControlOffScreen::RButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::RButtonUp(msg);
    SendButtonUpEvent(msg);
    return bRet;
}

bool CefControlOffScreen::RButtonDoubleClick(const EventArgs& msg)
{
    bool bRet = BaseClass::RButtonDoubleClick(msg);
    SendButtonDoubleClickEvent(msg);
    return bRet;
}

bool CefControlOffScreen::MButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::MButtonDown(msg);
    SendButtonDownEvent(msg);
    return bRet;
}

bool CefControlOffScreen::MButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::MButtonUp(msg);
    SendButtonUpEvent(msg);
    return bRet;
}

bool CefControlOffScreen::MButtonDoubleClick(const EventArgs& msg)
{
    bool bRet = BaseClass::MButtonDoubleClick(msg);
    SendButtonDoubleClickEvent(msg);
    return bRet;
}

void CefControlOffScreen::SendButtonDownEvent(const EventArgs& msg)
{
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    if (host == nullptr) {
        return;
    }

    UiPoint pt = msg.ptMouse;
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt)) {
        return;
    }

    SetFocus();
    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(msg);

    CefBrowserHost::MouseButtonType btnType =
        (msg.eventType == kEventMouseButtonDown ? MBT_LEFT : (
            msg.eventType == kEventMouseRButtonDown ? MBT_RIGHT : MBT_MIDDLE));
    AdaptDpiScale(mouse_event);
    host->SendMouseClickEvent(mouse_event, btnType, false, 1);
}

void CefControlOffScreen::SendButtonUpEvent(const EventArgs& msg)
{
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    if (host == nullptr) {
        return;
    }

    UiPoint pt = msg.ptMouse;
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt) && !GetWindow()->IsCaptured()) {
        return;
    }

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(msg);
    AdaptDpiScale(mouse_event);
    CefBrowserHost::MouseButtonType btnType =
        (msg.eventType == kEventMouseButtonUp ? MBT_LEFT : (
            msg.eventType == kEventMouseRButtonUp ? MBT_RIGHT : MBT_MIDDLE));
    host->SendMouseClickEvent(mouse_event, btnType, true, 1);
}

void CefControlOffScreen::SendButtonDoubleClickEvent(const EventArgs& msg)
{
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    if (host == nullptr) {
        return;
    }

    UiPoint pt = msg.ptMouse;
    pt.Offset(GetScrollOffsetInScrollBox());
    if (!GetRect().ContainsPt(pt)) {
        return;
    }

    CefMouseEvent mouse_event;
    mouse_event.x = pt.x - GetRect().left;
    mouse_event.y = pt.y - GetRect().top;
    mouse_event.modifiers = GetCefMouseModifiers(msg);
    AdaptDpiScale(mouse_event);
    CefBrowserHost::MouseButtonType btnType =
        (msg.eventType == kEventMouseDoubleClick ? MBT_LEFT : (
            msg.eventType == kEventMouseRDoubleClick ? MBT_RIGHT : MBT_MIDDLE));
    host->SendMouseClickEvent(mouse_event, btnType, true, 2);
}

bool CefControlOffScreen::OnSetFocus(const EventArgs& /*msg*/)
{
    CefRefPtr<CefBrowserHost> browserHost = GetCefBrowserHost();
    if (browserHost != nullptr) {
        browserHost->SetFocus(true);
    }

    //不调用基类的方法(基类的方法会关闭输入法)
    if (GetState() == kControlStateNormal) {
        SetState(kControlStateHot);
    }

    //设置输入法相关属性
    if (m_bHasFocusNode) {
        OnFocusedNodeChanged(m_bFocusNodeEditable, m_focusNodeRect);
    }
    else {
        Window* pWindow = GetWindow();
        if (pWindow != nullptr) {
            pWindow->NativeWnd()->SetTextInputArea(nullptr, 0);
            pWindow->NativeWnd()->SetImeOpenStatus(false);
        }
    }
    Invalidate();
    return true;
}

bool CefControlOffScreen::OnKillFocus(const EventArgs& msg)
{
    CefRefPtr<CefBrowserHost> browserHost = GetCefBrowserHost();
    if (browserHost != nullptr) {
        browserHost->SetFocus(false);
    }
    return BaseClass::OnKillFocus(msg);
}

bool CefControlOffScreen::OnChar(const EventArgs& msg)
{
    bool bRet = BaseClass::OnChar(msg);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    bool bHandled = false;
    if (msg.modifierKey & ModifierKey::kIsSystemKey) {
        SendKeyEvent(WM_SYSCHAR, msg.wParam, msg.lParam, bHandled);
    }
    else {
        SendKeyEvent(WM_CHAR, msg.wParam, msg.lParam, bHandled);
    }
    return bRet || bHandled;
#else
    SendKeyEvent(msg, KEYEVENT_CHAR);
    return bRet;
#endif
}

bool CefControlOffScreen::OnKeyDown(const EventArgs& msg)
{
    bool bRet = BaseClass::OnKeyDown(msg);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    bool bHandled = false;
    if (msg.modifierKey & ModifierKey::kIsSystemKey) {
        SendKeyEvent(WM_SYSKEYDOWN, msg.wParam, msg.lParam, bHandled);
    }
    else {
        SendKeyEvent(WM_KEYDOWN, msg.wParam, msg.lParam, bHandled);
    }
    return bRet || bHandled;
#else
    SendKeyEvent(msg, KEYEVENT_KEYDOWN);
    return bRet;
#endif
}

bool CefControlOffScreen::OnKeyUp(const EventArgs& msg)
{
    bool bRet = BaseClass::OnKeyUp(msg);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    bool bHandled = false;
    if (msg.modifierKey & ModifierKey::kIsSystemKey) {
        SendKeyEvent(WM_SYSKEYUP, msg.wParam, msg.lParam, bHandled);
    }
    else {
        SendKeyEvent(WM_KEYUP, msg.wParam, msg.lParam, bHandled);
    }
    return bRet || bHandled;
#else
    SendKeyEvent(msg, KEYEVENT_KEYUP);
    return bRet;
#endif
}

bool CefControlOffScreen::IsCefOSR() const
{
    return true;
}

bool CefControlOffScreen::IsCefOsrImeMode() const
{
    //109版本的64位版本离屏渲染模式，输入法输入时，libcef.dll内部会崩溃，原因未知，现禁止输入法功能（副作用：中文输入法的候选框位置不正确）
#if defined (_WIN64)
    return (CEF_VERSION_MAJOR > 109) ? true : false;
#else
    return true;
#endif    
}

bool CefControlOffScreen::OnImeSetContext(const EventArgs& msg)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (IsCefOsrImeMode()) {
        OnIMESetContext(WM_IME_SETCONTEXT, msg.wParam, msg.lParam);
    }
#else
    UNUSED_VARIABLE(msg);
#endif
    return false;
}

bool CefControlOffScreen::OnImeStartComposition(const EventArgs& /*msg*/)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (IsCefOsrImeMode()) {
        OnIMEStartComposition();
    }
#endif
    return false;
}

bool CefControlOffScreen::OnImeComposition(const EventArgs& msg)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (IsCefOsrImeMode()) {
        OnIMEComposition(WM_IME_COMPOSITION, msg.wParam, msg.lParam);
    }
#else
    UNUSED_VARIABLE(msg);
#endif
    return false;
}

bool CefControlOffScreen::OnImeEndComposition(const EventArgs& /*msg*/)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (IsCefOsrImeMode()) {
        OnIMECancelCompositionEvent();
    }
#endif
    return false;
}

#if defined (DUILIB_BUILD_FOR_SDL)

/** 获取按键标志
*/
static uint32_t GetCefModifiers(SDL_Keymod mod)
{
    uint32_t modifiers = 0;
    if (mod & SDL_KMOD_CTRL) {
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    }
    if (mod & SDL_KMOD_SHIFT) {
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if (mod & SDL_KMOD_ALT) {
        modifiers |= EVENTFLAG_ALT_DOWN;
    }

    if (mod & SDL_KMOD_CAPS) {
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;
    }
    if (mod & SDL_KMOD_NUM) {
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    }
    return modifiers;
}

//Linux平台
void CefControlOffScreen::SendKeyEvent(const EventArgs& msg, cef_key_event_type_t type)
{
    if (!IsVisible() || !IsEnabled()) {
        return;
    }
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    if (host == nullptr) {
        return;
    }

    SDL_EventType eventType = (SDL_EventType)msg.wParam;
    if (type == KEYEVENT_KEYDOWN) {
        ASSERT(eventType == SDL_EVENT_KEY_DOWN);
        ASSERT(msg.lParam != 0);
        if ((eventType != SDL_EVENT_KEY_DOWN) || (msg.lParam == 0)) {
            return;
        }
    }
    else if (type == KEYEVENT_KEYUP) {
        ASSERT(eventType == SDL_EVENT_KEY_UP);
        ASSERT(msg.lParam != 0);
        if ((eventType != SDL_EVENT_KEY_UP) || (msg.lParam == 0)) {
            return;
        }
    }
    if ((type == KEYEVENT_KEYDOWN) || (type == KEYEVENT_KEYUP)) {
        SDL_KeyboardEvent* key = (SDL_KeyboardEvent*)msg.lParam;
        CefKeyEvent event;
        event.type = (type == KEYEVENT_KEYDOWN) ? KEYEVENT_KEYDOWN : KEYEVENT_KEYUP;
        event.windows_key_code = msg.vkCode;
        event.native_key_code = key->scancode;
        event.is_system_key = false;
        event.modifiers = GetCefModifiers(key->mod);

        host->SendKeyEvent(event);
    }
    else if (type == KEYEVENT_CHAR) {
        ASSERT(msg.eventData == SDL_EVENT_TEXT_INPUT);
        ASSERT(msg.vkCode == kVK_None);
        if ((msg.eventData == SDL_EVENT_TEXT_INPUT) && (msg.wParam != 0) && (msg.lParam > 0)) {
            //当前输入的字符或者字符串（比如中文输入时，候选词是一次输入，而不像Windows SDK那样按字符逐次输入）
            DStringW text = (DStringW::value_type*)msg.wParam;
            CefKeyEvent event;
            event.type = KEYEVENT_CHAR;
            event.modifiers = GetCefModifiers(SDL_GetModState());
            size_t nCharCount = text.size();
            for (size_t nCharIndex = 0; nCharIndex < nCharCount; ++nCharIndex) {
                event.character = text[nCharIndex];

                //下列值在Windows平台需要设置
                event.unmodified_character = event.character;
                event.windows_key_code = event.character;
                host->SendKeyEvent(event);
            }
        }
    }
}
#endif

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
static int LogicalToDevice(int value, float device_scale_factor)
{
    float scaled_val = static_cast<float>(value) * device_scale_factor;
    return static_cast<int>(std::floor(scaled_val));
}

static CefRect LogicalToDevice(const CefRect& value, float device_scale_factor)
{
    return CefRect(LogicalToDevice(value.x, device_scale_factor),
                   LogicalToDevice(value.y, device_scale_factor),
                   LogicalToDevice(value.width, device_scale_factor),
                   LogicalToDevice(value.height, device_scale_factor));
}
#endif

void CefControlOffScreen::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> /*browser*/, const CefRange& selected_range, const std::vector<CefRect>& character_bounds)
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    CefCurrentlyOn(TID_UI);
    if (m_imeHandler != nullptr) {
        float device_scale_factor = Dpi().GetDPI() / 96.0f;
        // Convert from view coordinates to device coordinates.
        CefRenderHandler::RectList device_bounds;
        CefRenderHandler::RectList::const_iterator it = character_bounds.begin();
        for (; it != character_bounds.end(); ++it) {
            CefRect value = LogicalToDevice(*it, device_scale_factor);
            value.x += GetRect().left;
            value.y += GetRect().top;
            device_bounds.push_back(value);
        }
        m_imeHandler->ChangeCompositionRange(selected_range, device_bounds);
    }
#else    
    UNUSED_VARIABLE(selected_range);
    UNUSED_VARIABLE(character_bounds);
#endif
}

void CefControlOffScreen::OnFocusedNodeChanged(CefRefPtr<CefBrowser> /*browser*/,
                                               CefRefPtr<CefFrame> /*frame*/,
                                               CefDOMNode::Type /*type*/,
                                               bool /*bText*/,
                                               bool bEditable,
                                               const CefRect& nodeRect)
{
    OnFocusedNodeChanged(bEditable, nodeRect);
}

void CefControlOffScreen::OnFocusedNodeChanged(bool bEditable, const CefRect& nodeRect)
{
    m_bHasFocusNode = true;
    m_bFocusNodeEditable = bEditable;
    m_focusNodeRect = nodeRect;

    if (!IsVisible() || !IsEnabled() || !IsFocused()) {
        return;
    }
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return;
    }

    //如果在文本编辑框内，打开输入法，否则关闭输入法
    bool bEnableIME = bEditable;
    pWindow->NativeWnd()->SetImeOpenStatus(bEnableIME);
    if (bEnableIME) {
        UiRect rc = GetRect();
        UiPoint scrollOffset = GetScrollOffsetInScrollBox();
        rc.Offset(-scrollOffset.x, -scrollOffset.y);

        UiRect inputRect;
        inputRect.left = rc.left + nodeRect.x;
        inputRect.right = inputRect.left + nodeRect.width;
        inputRect.top = rc.top + nodeRect.y;
        inputRect.bottom = inputRect.top + nodeRect.height;

        pWindow->NativeWnd()->SetTextInputArea(&inputRect, 0);
    }
    else {
        pWindow->NativeWnd()->SetTextInputArea(nullptr, 0);
    }
}

std::shared_ptr<IBitmap> CefControlOffScreen::MakeImageSnapshot()
{
    if ((m_pCefMemData == nullptr) || (GetWindow() == nullptr)){
        return nullptr;
    }
    std::unique_ptr<IRender> render;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        render.reset(pRenderFactory->CreateRender(GetWindow()->GetRenderDpi()));
    }
    if ((render != nullptr) && m_pCefMemData->MakeImageSnapshot(render.get())) {
        return std::shared_ptr<IBitmap>(render->MakeImageSnapshot());
    }
    return nullptr;
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

LRESULT CefControlOffScreen::SendKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    CefRefPtr<CefBrowserHost> host;
    if (m_pBrowserHandler != nullptr) {
        host = m_pBrowserHandler->GetBrowserHost();
    }
    if (host == nullptr) {
        return 0;
    }

    CefKeyEvent event;
    event.windows_key_code = static_cast<int>(wParam);
    event.native_key_code = static_cast<int>(lParam);
    event.is_system_key = uMsg == WM_SYSCHAR || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP;

    if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
        event.type = KEYEVENT_RAWKEYDOWN;
    }
    else if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP) {
        event.type = KEYEVENT_KEYUP;
    }
    else {
        event.type = KEYEVENT_CHAR;
    }
    event.modifiers = client::GetCefKeyboardModifiers(wParam, lParam);

    host->SendKeyEvent(event);
    bHandled = true;
    return 0;
}

void CefControlOffScreen::OnIMEStartComposition()
{
    HWND hWnd = nullptr;
    if (GetWindow() != nullptr) {
        ASSERT(GetWindow()->IsWindow());
        hWnd = (HWND)GetWindow()->GetWindowHandle();
    }
    ASSERT(hWnd != nullptr);
    if (hWnd == nullptr) {
        return;
    }
    if ((m_imeHandler == nullptr) || (m_imeHandler->GetHandlerHWND() != hWnd)) {
        //创建IME管理器        
        m_imeHandler = std::make_unique<client::OsrImeHandlerWin>(hWnd);        
    }
    if (m_imeHandler) {
        m_imeHandler->SetInputLanguage();
        m_imeHandler->CreateImeWindow();
        m_imeHandler->MoveImeWindow();
        m_imeHandler->ResetComposition();
    }
}

void CefControlOffScreen::OnIMESetContext(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (GetWindow() == nullptr) {
        return;
    }
    HWND hWnd = (HWND)GetWindow()->GetWindowHandle();
    // We handle the IME Composition Window ourselves (but let the IME Candidates
    // Window be handled by IME through DefWindowProc()), so clear the
    // ISC_SHOWUICOMPOSITIONWINDOW flag:
    lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
    ::DefWindowProc(hWnd, message, wParam, lParam);

    // Create Caret Window if required
    if ((m_imeHandler == nullptr) || (m_imeHandler->GetHandlerHWND() != hWnd)) {
        //创建IME管理器        
        m_imeHandler = std::make_unique<client::OsrImeHandlerWin>(hWnd);
        m_imeHandler->SetInputLanguage();
    }
    if (m_imeHandler) {
        m_imeHandler->CreateImeWindow();
        m_imeHandler->MoveImeWindow();
    }
}

void CefControlOffScreen::OnIMEComposition(UINT /*message*/, WPARAM /*wParam*/, LPARAM lParam)
{
    ASSERT(m_imeHandler != nullptr);
    CefRefPtr<CefBrowser> browser;
    if (m_pBrowserHandler != nullptr) {
        browser = m_pBrowserHandler->GetBrowser();
    }
    if ((browser != nullptr) && (browser->GetHost() != nullptr) && (m_imeHandler != nullptr)) {
        CefString cTextStr;
        if (m_imeHandler->GetResult(lParam, cTextStr)) {
            // Send the text to the browser. The |replacement_range| and
            // |relative_cursor_pos| params are not used on Windows, so provide
            // default invalid values.
            browser->GetHost()->ImeCommitText(cTextStr, CefRange((std::numeric_limits<uint32_t>::max)(), (std::numeric_limits<uint32_t>::max)()), 0);
            m_imeHandler->ResetComposition();
            // Continue reading the composition string - Japanese IMEs send both
            // GCS_RESULTSTR and GCS_COMPSTR.
        }

        std::vector<CefCompositionUnderline> underlines;
        int composition_start = 0;

        if (m_imeHandler->GetComposition(lParam, cTextStr, underlines,
            composition_start)) {
            // Send the composition string to the browser. The |replacement_range|
            // param is not used on Windows, so provide a default invalid value.
            browser->GetHost()->ImeSetComposition(
                cTextStr, underlines, CefRange((std::numeric_limits<uint32_t>::max)(), (std::numeric_limits<uint32_t>::max)()),
                CefRange(composition_start,
                    static_cast<int>(composition_start + cTextStr.length())));

            // Update the Candidate Window position. The cursor is at the end so
            // subtract 1. This is safe because IMM32 does not support non-zero-width
            // in a composition. Also,  negative values are safely ignored in
            // MoveImeWindow
            m_imeHandler->UpdateCaretPosition(composition_start - 1);
        }
        else {
            OnIMECancelCompositionEvent();
        }
    }
}

void CefControlOffScreen::OnIMECancelCompositionEvent()
{
    CefRefPtr<CefBrowser> browser;
    if (m_pBrowserHandler != nullptr) {
        browser = m_pBrowserHandler->GetBrowser();
    }
    if ((browser != nullptr) && (browser->GetHost() != nullptr)) {
        browser->GetHost()->ImeCancelComposition();
    }
    if (m_imeHandler != nullptr) {
        m_imeHandler->ResetComposition();
        m_imeHandler->DestroyImeWindow();
    }
}


#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

} //namespace ui
