#include "CefBrowserHandler.h"
#include "duilib/CEFControl/CefManager.h"
#include "duilib/CEFControl/CefWindowUtils.h"
#include "duilib/CEFControl/internal/CefIPCStringDefs.h"
#include "duilib/CEFControl/internal/CefJSBridge.h"
#include "duilib/CEFControl/internal/CefBrowserHandlerDelegate.h"

#ifdef DUILIB_BUILD_FOR_WIN
    #include "duilib/CEFControl/internal/Windows/osr_dragdrop_win.h"
    #include "duilib/CEFControl/internal/Windows/CefOsrDropTarget.h"
#endif

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Window.h"

#pragma warning (push)
#pragma warning (disable:4100 4324)

namespace ui
{
CefBrowserHandler::CefBrowserHandler():
    m_pHandlerDelegate(nullptr),
    m_hCefWindowHandle(0),
    m_bHostWindowClosed(false)
{
}

CefBrowserHandler::~CefBrowserHandler()
{
    ASSERT(m_browserList.empty());
}

void CefBrowserHandler::SetHostWindow(ui::Window* window)
{
    GlobalManager::Instance().AssertUIThread();
    m_spWindow = window;
}

void CefBrowserHandler::SetHandlerDelegate(CefBrowserHandlerDelegate* handler)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pHandlerDelegate != nullptr) {
        //注销DragDrop接口
        UnregisterDropTarget();
    }
    m_pHandlerDelegate = handler;
    if (m_pHandlerDelegate != nullptr) {
        //注册DragDrop接口
        RegisterDropTarget();
    }
}

void CefBrowserHandler::RegisterDropTarget()
{
    GlobalManager::Instance().AssertUIThread();
#ifdef DUILIB_BUILD_FOR_WIN
    if (CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        //离屏渲染模式
        if ((m_pHandlerDelegate != nullptr) && (m_pDropTarget == nullptr)) {
            HWND hWnd = nullptr;
            Control* pCefControl = m_pHandlerDelegate->GetCefControl();
            if ((pCefControl != nullptr) && (pCefControl->GetWindow() != nullptr)) {
                hWnd = pCefControl->GetWindow()->NativeWnd()->GetHWND();
                ASSERT(::IsWindow(hWnd));
                if (::IsWindow(hWnd)) {
                    std::shared_ptr<client::DropTargetWin> pDropTargetWin = std::make_shared<client::DropTargetWin>(hWnd, this);
                    m_pDropTarget = std::make_shared<CefOsrDropTarget>(pDropTargetWin);
                }
            }
        }
    }
#endif
}

void CefBrowserHandler::UnregisterDropTarget()
{
    GlobalManager::Instance().AssertUIThread();
#ifdef DUILIB_BUILD_FOR_WIN
    m_pDropTarget.reset();
#endif
}

ControlDropTarget_Windows* CefBrowserHandler::GetControlDropTarget()
{
    GlobalManager::Instance().AssertUIThread();
#ifdef DUILIB_BUILD_FOR_WIN
    return m_pDropTarget.get();
#else
    return nullptr;
#endif
}

void CefBrowserHandler::SetViewRect(const UiRect& rc)
{
    if (m_bHostWindowClosed) {
        return;
    }
    else {
        std::lock_guard<std::mutex> threadGuard(m_dataMutex);
        m_rcCefControl = rc;
    }
    // 调用WasResized接口，调用后，CefBrowserHandler会调用GetViewRect接口来获取浏览器对象新的位置
    CefRefPtr<CefBrowserHost> pCefBrowserHost = GetBrowserHost();
    if (pCefBrowserHost != nullptr) {
        pCefBrowserHost->WasResized();
    }
}

UiRect CefBrowserHandler::GetViewRect()
{
    std::lock_guard<std::mutex> threadGuard(m_dataMutex);
    UiRect rc = m_rcCefControl;
    return rc;
}

CefRefPtr<CefBrowserHost> CefBrowserHandler::GetBrowserHost()
{
    std::lock_guard<std::mutex> threadGuard(m_dataMutex);
    if (m_browser.get()) {
        return m_browser->GetHost();
    }
    return nullptr;
}

CefWindowHandle CefBrowserHandler::GetCefWindowHandle()
{
    std::lock_guard<std::mutex> threadGuard(m_dataMutex);
    CefWindowHandle hWindowHandle = 0;
    if (m_browser.get() && m_browser->GetHost().get()) {
        hWindowHandle = m_browser->GetHost()->GetWindowHandle();
        if (hWindowHandle == 0) {
            hWindowHandle = m_hCefWindowHandle;
        }
    }
    return hWindowHandle;
}

CefUnregisterCallback CefBrowserHandler::AddAfterCreateTask(const ui::StdClosure& cb)
{
    return m_taskListAfterCreated.AddCallback(cb);
}

void CefBrowserHandler::SetHostWindowClosed(bool bHostWindowClosed)
{
    m_bHostWindowClosed = bHostWindowClosed;
}

bool CefBrowserHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                 CefRefPtr<CefFrame> frame,
                                                 CefProcessId /*source_process*/,
                                                 CefRefPtr<CefProcessMessage> message)
{
    if (m_bHostWindowClosed) {
        return false;
    }
    // 处理render进程发来的消息
    std::string message_name = message->GetName();
    if (message->GetArgumentList() == nullptr) {
        return false;
    }
    if (message_name == kFocusedNodeChangedMessage) {
        CefDOMNode::Type type = (CefDOMNode::Type)message->GetArgumentList()->GetInt(0);
        bool bText = message->GetArgumentList()->GetBool(1);
        bool bEditable = message->GetArgumentList()->GetBool(2);

        CefRect nodeRect;
        nodeRect.x = message->GetArgumentList()->GetInt(3);
        nodeRect.y = message->GetArgumentList()->GetInt(4);
        nodeRect.width = message->GetArgumentList()->GetInt(5);
        nodeRect.height = message->GetArgumentList()->GetInt(6);

        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, browser, frame, type, bText, bEditable, nodeRect]() {
                if (m_pHandlerDelegate) {
                    m_pHandlerDelegate->OnFocusedNodeChanged(browser, frame, type, bText, bEditable, nodeRect);
                }
            }));
        return true;
    }
    else if (message_name == kCallCppFunctionMessage) {
        CefString fun_name = message->GetArgumentList()->GetString(0);
        CefString param = message->GetArgumentList()->GetString(1);
        int js_callback_id = message->GetArgumentList()->GetInt(2);

        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, fun_name, param, js_callback_id, browser, frame]() {
                if (m_pHandlerDelegate) {
                    m_pHandlerDelegate->OnExecuteCppFunc(fun_name, param, js_callback_id, browser, frame);
                }
            }));
        return true;
    }
    else if (message_name == kExecuteCppCallbackMessage) {
        CefString param = message->GetArgumentList()->GetString(0);
        int callback_id = message->GetArgumentList()->GetInt(1);

        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, callback_id, param]() {
            if (m_pHandlerDelegate) {
                m_pHandlerDelegate->OnExecuteCppCallbackFunc(callback_id, param);
            }
        }));
        return true;
    }
    return false;
}

bool CefBrowserHandler::DoOnBeforePopup(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        int popup_id,
                                        const CefString& target_url,
                                        const CefString& target_frame_name,
                                        CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                        bool user_gesture,
                                        const CefPopupFeatures& popupFeatures,
                                        CefWindowInfo& windowInfo,
                                        CefRefPtr<CefClient>& client,
                                        CefBrowserSettings& settings,
                                        CefRefPtr<CefDictionaryValue>& extra_info,
                                        bool* no_javascript_access)
{
    //让新的链接在原浏览器对象中打开
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        // 返回true则继续在控件内打开新链接，false则禁止访问
        return m_pHandlerDelegate->OnBeforePopup(browser, frame, popup_id, target_url, target_frame_name, target_disposition, user_gesture, popupFeatures, windowInfo, client, settings, extra_info, no_javascript_access);;
    }
    // 默认禁止弹出popup窗口
    return true;
}

#if CEF_VERSION_MAJOR > 109
//CEF 高版本
// CefLifeSpanHandler methods
bool CefBrowserHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      int popup_id,
                                      const CefString& target_url,
                                      const CefString& target_frame_name,
                                      CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                      bool user_gesture,
                                      const CefPopupFeatures& popupFeatures,
                                      CefWindowInfo& windowInfo,
                                      CefRefPtr<CefClient>& client,
                                      CefBrowserSettings& settings,
                                      CefRefPtr<CefDictionaryValue>& extra_info,
                                      bool* no_javascript_access)
{
    if (m_bHostWindowClosed) {
        return true;
    }
    return DoOnBeforePopup(browser, frame, popup_id,
                           target_url, target_frame_name, target_disposition,
                           user_gesture, popupFeatures, windowInfo,
                           client, settings, extra_info,
                           no_javascript_access);
}

void CefBrowserHandler::OnBeforePopupAborted(CefRefPtr<CefBrowser> browser, int popup_id)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnBeforePopupAborted)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnBeforePopupAborted, m_pHandlerDelegate, browser, popup_id));
    }
}

void CefBrowserHandler::OnBeforeDevToolsPopup(CefRefPtr<CefBrowser> /*browser*/,
                                              CefWindowInfo& /*windowInfo*/,
                                              CefRefPtr<CefClient>& /*client*/,
                                              CefBrowserSettings& /*settings*/,
                                              CefRefPtr<CefDictionaryValue>& /*extra_info*/,
                                              bool* /*use_default_window*/)
{
}
#else
bool CefBrowserHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      const CefString& target_url,
                                      const CefString& target_frame_name,
                                      CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                      bool user_gesture,
                                      const CefPopupFeatures& popupFeatures,
                                      CefWindowInfo& windowInfo,
                                      CefRefPtr<CefClient>& client,
                                      CefBrowserSettings& settings,
                                      CefRefPtr<CefDictionaryValue>& extra_info,
                                      bool* no_javascript_access)
{
    if (m_bHostWindowClosed) {
        return true;
    }
    return DoOnBeforePopup(browser, frame, 0,
                           target_url, target_frame_name, target_disposition,
                           user_gesture, popupFeatures, windowInfo,
                           client, settings, extra_info,
                           no_javascript_access);
}
#endif

void CefBrowserHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (browser == nullptr) {
        return;
    }
    else {
        std::lock_guard<std::mutex> threadGuard(m_dataMutex);
        //这个窗口句柄在TID_UI线程可以第一时间获取到，但转到主线程调用时，就获取不到了（有延迟）。
        auto hWndowHandle = browser->GetHost()->GetWindowHandle();//当browser为dev tools 时，此处返回nullptr，其他情况为非nullptr值
        if (m_browserList.empty()) {
            m_hCefWindowHandle = hWndowHandle;
            m_browser = browser;
        }
        else {
            ASSERT(browser->IsPopup());
        }
        auto it = std::find_if(m_browserList.begin(), m_browserList.end(), [browser](const CefRefPtr<CefBrowser>& item) {
            return item->IsSame(browser);
            });
        if (it == m_browserList.end()) {
            m_browserList.push_back(browser);
            CefManager::GetInstance()->AddBrowserCount();
        }
    }

    if (m_bHostWindowClosed) {
        return;
    }
    GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, browser](){
        if (m_pHandlerDelegate != nullptr) {
            // 有窗模式下，浏览器创建完毕后，让上层更新一下自己的位置；因为在异步状态下，上层更新位置时可能Cef窗口还没有创建出来
            m_pHandlerDelegate->UpdateCefWindowPos();
        }

        m_taskListAfterCreated();
        m_taskListAfterCreated.Clear();

        if (m_pHandlerDelegate) {
            m_pHandlerDelegate->OnAfterCreated(browser);
        }
    }));
}

void CefBrowserHandler::CloseAllBrowsers(bool bForceClose)
{
    std::vector<CefRefPtr<CefBrowserHost>> cefBrowserHostList;
    {
        std::lock_guard<std::mutex> threadGuard(m_dataMutex);
        for (auto pCefBrowser : m_browserList) {
            CefRefPtr<CefBrowserHost> spCefBrowserHost;
            if (pCefBrowser != nullptr) {
                spCefBrowserHost = pCefBrowser->GetHost();
            }
            if (spCefBrowserHost != nullptr) {
                cefBrowserHostList.push_back(spCefBrowserHost);
            }
        }
        //当有多个Browser时，按倒序关闭
        if (!cefBrowserHostList.empty()) {
            std::reverse(cefBrowserHostList.begin(), cefBrowserHostList.end());
        }
    }
    for (CefRefPtr<CefBrowserHost> spCefBrowserHost : cefBrowserHostList) {
        if (spCefBrowserHost != nullptr) {
            spCefBrowserHost->CloseBrowser(bForceClose);
        }
    }
}

bool CefBrowserHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
    return false;
}

void CefBrowserHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (browser == nullptr) {
        return;
    }
    else {
        std::lock_guard<std::mutex> threadGuard(m_dataMutex);
        auto it = std::find_if(m_browserList.begin(), m_browserList.end(), [&](const CefRefPtr<CefBrowser>& item) {
            return item->IsSame(browser);
            });
        if (it != m_browserList.end()) {
            auto closed_browser = *it;
            m_browserList.erase(it);
            if (closed_browser->IsSame(m_browser)) {
                ASSERT(m_browserList.empty());
                m_browser = nullptr;
                m_hCefWindowHandle = 0;
            }
            CefManager::GetInstance()->SubBrowserCount();
        }
    }

    if (m_bHostWindowClosed) {
        return;
    }
    GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, browser](){
        if (m_pHandlerDelegate) {
            m_pHandlerDelegate->OnBeforeClose(browser);
        }
    }));
}

// CefRenderHandler methods

CefRefPtr<CefAccessibilityHandler> CefBrowserHandler::GetAccessibilityHandler()
{
    return nullptr;
}

bool CefBrowserHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    rect = { 0, 0, 1, 1 }; //避免返回0，导致CEF内部触发DCHECK错误
    if (m_bHostWindowClosed) {        
        return false;
    }
    ControlPtrT<ui::Window> spWindow = m_spWindow;
    if ((spWindow == nullptr) || !spWindow->IsWindow()) {
        return false;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    //Windows平台
    RECT window_rect = { 0 };
    HWND root_window = GetAncestor(spWindow->NativeWnd()->GetHWND(), GA_ROOT);
    if (::GetWindowRect(root_window, &window_rect)) {
        rect = CefRect(window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
    }
    else {
        UiRect rcWindow;
        spWindow->GetWindowRect(rcWindow);
        rect = CefRect(rcWindow.left, rcWindow.top, rcWindow.Width(), rcWindow.Height());
    }
#else
    //其他平台
    UiRect rcWindow;
    spWindow->GetWindowRect(rcWindow);
    rect = CefRect(rcWindow.left, rcWindow.top, rcWindow.Width(), rcWindow.Height());
#endif
    if (rect.width <= 0) {
        rect.width = 1;
    }
    if (rect.height <= 0) {
        rect.height = 1;
    }
    return true;
}

void CefBrowserHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    rect = { 0, 0, 1, 1 }; //避免返回0，导致CEF内部触发DCHECK错误
    if (m_bHostWindowClosed) {
        return;
    }
    ControlPtrT<ui::Window> spWindow = m_spWindow;
    if ((spWindow == nullptr) || !spWindow->IsWindow()) {
        return;
    }
    if (m_pHandlerDelegate) {
        UiRect rect_cef_control;
        {
            std::lock_guard<std::mutex> threadGuard(m_dataMutex);
            rect_cef_control = m_rcCefControl;
        }
        rect.x = 0;
        rect.y = 0;
        rect.width = rect_cef_control.right - rect_cef_control.left;
        rect.height = rect_cef_control.bottom - rect_cef_control.top;
        if (CefManager::GetInstance()->IsEnableOffScreenRendering()) {
            //离屏渲染模式，需要传给原始宽度和高度，因为CEF内部会进一步做DPI自适应
            uint32_t dpiScale = spWindow->Dpi().GetScale();
            if (dpiScale > 100) {
                rect.width = rect.width * 100 / dpiScale;
                rect.height = rect.height * 100 / dpiScale;
            }
        }
    }
    if (rect.width <= 0) {
        rect.width = 1;
    }
    if (rect.height <= 0) {
        rect.height = 1;
    }
}

bool CefBrowserHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    if (m_bHostWindowClosed) {
        return false;
    }
    ControlPtrT<ui::Window> spWindow = m_spWindow;
    if ((spWindow == nullptr) || !spWindow->IsWindow()) {
        return false;
    }

    // Convert the point from view coordinates to actual screen coordinates.
    ui::UiPoint screen_pt = { viewX, viewY};
    if (CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        //离屏渲染模式下，给到的参数是原始坐标，未经DPI自适应，所以需要做DPI自适应处理，否则页面的右键菜单位置显示不对
        uint32_t dpiScale = spWindow->Dpi().GetScale();
        if (dpiScale > 100) {
            screen_pt.x = screen_pt.x * dpiScale / 100;
            screen_pt.y = screen_pt.y * dpiScale / 100;
        }
    }
    //将页面坐标转换为窗口客户区坐标，否则页面弹出的右键菜单位置不正确
    UiRect rect_cef_control;
    {
        std::lock_guard<std::mutex> threadGuard(m_dataMutex);
        rect_cef_control = m_rcCefControl;
    }
    screen_pt.x = screen_pt.x + rect_cef_control.left;
    screen_pt.y = screen_pt.y + rect_cef_control.top;
#if defined (DUILIB_BUILD_FOR_WIN)  || defined (DUILIB_BUILD_FOR_MACOS) 
    //Windows/MacOS：需要转换为屏幕坐标；Linux平台则不需要
    spWindow->ClientToScreen(screen_pt);
#endif
    screenX = screen_pt.x;
    screenY = screen_pt.y;

#if defined (DUILIB_BUILD_FOR_MACOS)
    //MacOS: 屏幕坐标是以屏幕左下角为原点的，需要进行转换
    UiRect rcMonitor;
    spWindow->GetMonitorRect(rcMonitor);
    screenY = rcMonitor.Height() - screenY;
#endif
    return true;
}

bool CefBrowserHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
{
    ASSERT(CefCurrentlyOn(TID_UI));
    //只有离屏渲染模式下，才处理DPI缩放因子
    if (!CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        return false;
    }
    if (m_bHostWindowClosed) {
        return false;
    }
    ControlPtrT<ui::Window> spWindow = m_spWindow;
    if ((spWindow == nullptr) || !spWindow->IsWindow()) {
        return false;
    }
    uint32_t dpiScale = spWindow->Dpi().GetScale();
    if (dpiScale == 100) {
        return false;
    }
    else {
        screen_info.device_scale_factor = dpiScale / 100.0f;
        return true;
    }    
}

void CefBrowserHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnPopupShow, m_pHandlerDelegate, browser, show));
    }
}

void CefBrowserHandler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnPopupSize, m_pHandlerDelegate, browser, rect));
    }
}

void CefBrowserHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                                PaintElementType type,
                                const RectList& dirtyRects,
                                const void* buffer,
                                int width,
                                int height)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnPaint(browser, type, dirtyRects, buffer, width, height);
    }
}
#if CEF_VERSION_MAJOR <= 109
//CEF 109版本
void CefBrowserHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, void* shared_handle)
{
}
#else
//CEF 高版本
void CefBrowserHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const CefAcceleratedPaintInfo& info)
{
}
#endif

void CefBrowserHandler::GetTouchHandleSize(CefRefPtr<CefBrowser> browser,
                                           cef_horizontal_alignment_t orientation,
                                           CefSize& size)
{
}

void CefBrowserHandler::OnTouchHandleStateChanged(CefRefPtr<CefBrowser> browser, const CefTouchHandleState& state)
{
}

void CefBrowserHandler::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x, double y)
{
}

void CefBrowserHandler::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
                                                     const CefRange& selected_range,
                                                     const RectList& character_bounds)
{
    //此函数只有OSR模式会有回调事件
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnImeCompositionRangeChanged(browser, selected_range, character_bounds);
    }
}

void CefBrowserHandler::OnTextSelectionChanged(CefRefPtr<CefBrowser> browser, const CefString& selected_text, const CefRange& selected_range)
{
}

void CefBrowserHandler::OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser, TextInputMode input_mode)
{
}

bool CefBrowserHandler::StartDragging(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefDragData> drag_data,
                                      CefRenderHandler::DragOperationsMask allowed_ops,
                                      int x,
                                      int y)
{
    if (m_bHostWindowClosed) {
        return false;
    }
    ASSERT(CefCurrentlyOn(TID_UI));
    //函数功能：网页内部的拖拽开始（只有离屏渲染模式才会调用该接口）
    if (!CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        return false;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    if (!m_pHandlerDelegate) {
        return false;
    }

    if ((browser == nullptr) || (browser->GetHost() == nullptr)) {
        return false;
    }
    
    //转到UI线程，执行DoDragDrop操作
    GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandler::DoDragDrop, this, browser, drag_data, allowed_ops, x, y));
    return true;
#else
    return false;
#endif
}

#ifdef DUILIB_BUILD_FOR_WIN

void CefBrowserHandler::DoDragDrop(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y)
{
    if (m_bHostWindowClosed) {
        return;
    }
    if (!m_pHandlerDelegate) {
        return;
    }

    if ((browser == nullptr) || (browser->GetHost() == nullptr)) {
        return;
    }
    ControlPtrT<ui::Window> spWindow = m_spWindow;
    if ((spWindow == nullptr) || !spWindow->IsWindow()) {
        return;
    }

    CefBrowserHost::DragOperationsMask result = client::OsrStartDragging(drag_data, allowed_ops, x, y);
    UiPoint pt = {};
    if (spWindow != nullptr) {
        spWindow->GetCursorPos(pt);
        spWindow->ScreenToClient(pt);
    }
    if (m_pHandlerDelegate != nullptr) {
        m_pHandlerDelegate->ClientToControl(pt);
    }
    if (browser->GetHost() != nullptr) {
        browser->GetHost()->DragSourceEndedAt(pt.x, pt.y, result);
        browser->GetHost()->DragSourceSystemDragEnded();
    }
}

#endif

void CefBrowserHandler::UpdateDragCursor(CefRefPtr<CefBrowser> browser, CefRenderHandler::DragOperation operation)
{
    if (m_bHostWindowClosed) {
        return;
    }
    ASSERT(CefCurrentlyOn(TID_UI));
    m_currentDragOperation = operation;
}

CefBrowserHost::DragOperationsMask CefBrowserHandler::OnDragEnter(CefRefPtr<CefDragData> drag_data, CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect)
{
    if (m_bHostWindowClosed) {
        return DRAG_OPERATION_NONE;
    }
    if ((m_browser != nullptr) && (m_browser->GetHost() != nullptr)) {
        UiPoint pt = { ev.x, ev.y };
        if (m_pHandlerDelegate) {
            m_pHandlerDelegate->ClientToControl(pt);
        }
        ev.x = pt.x;
        ev.y = pt.y;
        m_browser->GetHost()->DragTargetDragEnter(drag_data, ev, effect);
        m_browser->GetHost()->DragTargetDragOver(ev, effect);
    }
    return m_currentDragOperation;
}

CefBrowserHost::DragOperationsMask CefBrowserHandler::OnDragOver(CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect)
{
    if (m_bHostWindowClosed) {
        return DRAG_OPERATION_NONE;
    }
    if ((m_browser != nullptr) && (m_browser->GetHost() != nullptr)) {
        UiPoint pt = { ev.x, ev.y };
        if (m_pHandlerDelegate) {
            m_pHandlerDelegate->ClientToControl(pt);
        }
        ev.x = pt.x;
        ev.y = pt.y;
        m_browser->GetHost()->DragTargetDragOver(ev, effect);
    }
    return m_currentDragOperation;
}

void CefBrowserHandler::OnDragLeave()
{
    if (m_bHostWindowClosed) {
        return;
    }
    if ((m_browser != nullptr) && (m_browser->GetHost() != nullptr)) {
        m_browser->GetHost()->DragTargetDragLeave();
    }
}

CefBrowserHost::DragOperationsMask CefBrowserHandler::OnDrop(CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect)
{
    if (m_bHostWindowClosed) {
        return DRAG_OPERATION_NONE;
    }
    if ((m_browser != nullptr) && (m_browser->GetHost() != nullptr)) {
        UiPoint pt = { ev.x, ev.y };
        if (m_pHandlerDelegate != nullptr) {
            m_pHandlerDelegate->ClientToControl(pt);
        }
        ev.x = pt.x;
        ev.y = pt.y;
        m_browser->GetHost()->DragTargetDragOver(ev, effect);
        m_browser->GetHost()->DragTargetDrop(ev);
    }
    return m_currentDragOperation;
}

// CefContextMenuHandler methods
void CefBrowserHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefContextMenuParams> params,
                                            CefRefPtr<CefMenuModel> model)
{
    if (m_bHostWindowClosed) {
        return;
    }
    ASSERT(CefCurrentlyOn(TID_UI));

    if (m_pHandlerDelegate) {
        m_pHandlerDelegate->OnBeforeContextMenu(browser, frame, params, model);
    }
    else  if ((params != nullptr) && (model != nullptr)) {
        // Customize the context menu...
        if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0) {
            if (model->GetCount() > 0) {
                // 禁止右键菜单
                model->Clear();
            }
        }
    }
}

bool CefBrowserHandler::RunContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback)
{
    return false;
}

bool CefBrowserHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnContextMenuCommand(browser, frame, params, command_id, event_flags);
    }
    return false;        
}

void CefBrowserHandler::OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnContextMenuDismissed(browser, frame);
    }
}

bool CefBrowserHandler::RunQuickMenu(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     const CefPoint& location,
                                     const CefSize& size,
                                     QuickMenuEditStateFlags edit_state_flags,
                                     CefRefPtr<CefRunQuickMenuCallback> callback)
{
    return false;
}

bool CefBrowserHandler::OnQuickMenuCommand(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           int command_id,
                                           EventFlags event_flags)
{
    return false;
}

void CefBrowserHandler::OnQuickMenuDismissed(CefRefPtr<CefBrowser> browser,
                                             CefRefPtr<CefFrame> frame)
{
}

// CefDisplayHandler methods
void CefBrowserHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnAddressChange)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnAddressChange, m_pHandlerDelegate, browser, frame, url));
    }
}

void CefBrowserHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnTitleChange, m_pHandlerDelegate, browser, title));
    }
}

void CefBrowserHandler::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnFaviconURLChange)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnFaviconURLChange, m_pHandlerDelegate, browser, icon_urls));
    }
}

void CefBrowserHandler::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnFullscreenModeChange)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnFullscreenModeChange, m_pHandlerDelegate, browser, fullscreen));
    }
}

bool CefBrowserHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
{
    return false;
}

void CefBrowserHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnStatusMessage)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnStatusMessage, m_pHandlerDelegate, browser, value));
    }
}

bool CefBrowserHandler::OnConsoleMessage(CefRefPtr<CefBrowser> /*browser*/,
                                         cef_log_severity_t /*level*/,
                                         const CefString& /*message*/,
                                         const CefString& /*source*/,
                                         int /*line*/)
{
    // Log a console message...
    return true;
}

bool CefBrowserHandler::OnAutoResize(CefRefPtr<CefBrowser> browser, const CefSize& new_size)
{
    return false;
}

void CefBrowserHandler::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnLoadingProgressChange)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnLoadingProgressChange, m_pHandlerDelegate, browser, progress));
    }
}

bool CefBrowserHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                       CefCursorHandle cursor,
                                       cef_cursor_type_t type,
                                       const CefCursorInfo& custom_cursor_info)
{
    if (m_bHostWindowClosed) {
        return false;
    }
    if (!CefManager::GetInstance()->IsEnableOffScreenRendering()) {
        //子窗口模式，不需要设置光标，否则光标异常
        return false;
    }
    else {
        //离屏渲染模式：需要设置光标
        SetCefWindowCursor(GetCefWindowHandle(), cursor);
#ifdef DUILIB_BUILD_FOR_SDL
        //由于SDL内部，未开放设置光标事件，所以需要主动设置SDL的光标，才能保证光标正确（SDL内部会设置光标，覆盖此处的光标）
        if (m_pHandlerDelegate && !m_bHostWindowClosed) {
            GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnCursorChange, m_pHandlerDelegate, type));
        }
#endif
        return true;
    }
}

void CefBrowserHandler::OnMediaAccessChange(CefRefPtr<CefBrowser> browser,
                                            bool has_video_access,
                                            bool has_audio_access)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnMediaAccessChange)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnMediaAccessChange, m_pHandlerDelegate, browser, has_video_access, has_audio_access));
    }
}

bool CefBrowserHandler::OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnDragEnter(browser, dragData, mask);
    }
    return false;
}

void CefBrowserHandler::OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const std::vector<CefDraggableRegion>& regions)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnDraggableRegionsChanged)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnDraggableRegionsChanged, m_pHandlerDelegate, browser, frame, regions));
    }
}

// CefLoadHandler methods
void CefBrowserHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnLoadingStateChange)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnLoadingStateChange, m_pHandlerDelegate, browser, isLoading, canGoBack, canGoForward));
    }
}

void CefBrowserHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    TransitionType transition_type)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnLoadStart)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnLoadStart, m_pHandlerDelegate, browser, frame, transition_type));
    }
}

void CefBrowserHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnLoadEnd)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnLoadEnd, m_pHandlerDelegate, browser, frame, httpStatusCode));
    }
}

void CefBrowserHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnLoadError)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnLoadError, m_pHandlerDelegate, browser, frame, errorCode, errorText, failedUrl));
    }
}

bool CefBrowserHandler::OnJSDialog(CefRefPtr<CefBrowser> /*browser*/,
                                   const CefString& /*origin_url*/,
                                   JSDialogType /*dialog_type*/,
                                   const CefString& /*message_text*/,
                                   const CefString& /*default_prompt_text*/,
                                   CefRefPtr<CefJSDialogCallback> callback,
                                   bool& suppress_message)
{
    if (m_bHostWindowClosed) {
        suppress_message = true;
        return false;
    }
    // release时阻止弹出js对话框
    (void)suppress_message;
#ifndef _DEBUG
    suppress_message = true;
#endif

    return false;
}

bool CefBrowserHandler::OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
                                             const CefString& message_text,
                                             bool is_reload,
                                             CefRefPtr<CefJSDialogCallback> callback)
{
    return false;
}

void CefBrowserHandler::OnResetDialogState(CefRefPtr<CefBrowser> browser)
{
}

void CefBrowserHandler::OnDialogClosed(CefRefPtr<CefBrowser> browser)
{
}

bool CefBrowserHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event, bool* is_keyboard_shortcut)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnPreKeyEvent(browser, event, os_event, is_keyboard_shortcut);
    }
    return false;
}

bool CefBrowserHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnKeyEvent(browser, event, os_event);
    }
    return false;
}

// CefRequestHandler methods
bool CefBrowserHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       CefRefPtr<CefRequest> request,
                                       bool user_gesture,
                                       bool is_redirect)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnBeforeBrowse(browser, frame, request, user_gesture, is_redirect);
    }    
    return false;
}

bool CefBrowserHandler::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefFrame> frame,
                                         const CefString& target_url,
                                         CefRequestHandler::WindowOpenDisposition target_disposition,
                                         bool user_gesture)
{
    return false;
}

CefRefPtr<CefResourceRequestHandler> CefBrowserHandler::GetResourceRequestHandler(CefRefPtr<CefBrowser> browser,
                                                                                  CefRefPtr<CefFrame> frame,
                                                                                  CefRefPtr<CefRequest> request,
                                                                                  bool is_navigation,
                                                                                  bool is_download,
                                                                                  const CefString& request_initiator,
                                                                                  bool& disable_default_handling)
{
    return this;
}

bool CefBrowserHandler::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
                                           const CefString& origin_url,
                                           bool isProxy,
                                           const CefString& host,
                                           int port,
                                           const CefString& realm,
                                           const CefString& scheme,
                                           CefRefPtr<CefAuthCallback> callback)
{
    return false;
}

bool CefBrowserHandler::OnCertificateError(CefRefPtr<CefBrowser> browser,
                                           cef_errorcode_t cert_error,
                                           const CefString& request_url,
                                           CefRefPtr<CefSSLInfo> ssl_info,
                                           CefRefPtr<CefCallback> callback)
{
    return false;
}

bool CefBrowserHandler::OnSelectClientCertificate(CefRefPtr<CefBrowser> browser,
                                                  bool isProxy,
                                                  const CefString& host,
                                                  int port,
                                                  const X509CertificateList& certificates,
                                                  CefRefPtr<CefSelectClientCertificateCallback> callback)
{
    return false;
}

void CefBrowserHandler::OnRenderViewReady(CefRefPtr<CefBrowser> browser)
{
}

#if CEF_VERSION_MAJOR > 109
//CEF 高版本
bool CefBrowserHandler::OnRenderProcessUnresponsive(CefRefPtr<CefBrowser> browser,
                                                    CefRefPtr<CefUnresponsiveProcessCallback> callback)
{
    return false;
}

void CefBrowserHandler::OnRenderProcessResponsive(CefRefPtr<CefBrowser> browser)
{
}

void CefBrowserHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                                  TerminationStatus status,
                                                  int error_code,
                                                  const CefString& error_string)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnRenderProcessTerminated)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnRenderProcessTerminated, m_pHandlerDelegate, browser, status, error_code, error_string));
    }
}
#else
void CefBrowserHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnRenderProcessTerminated)) {
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnRenderProcessTerminated, m_pHandlerDelegate, browser, status, 0, ""));
    }
}
#endif

void CefBrowserHandler::OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && m_pHandlerDelegate->IsCallbackExists(CefCallbackID::OnDocumentAvailableInMainFrame)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnDocumentAvailableInMainFrame, m_pHandlerDelegate, browser));
    }
}

CefRefPtr<CefCookieAccessFilter> CefBrowserHandler::GetCookieAccessFilter(CefRefPtr<CefBrowser> browser,
                                                                          CefRefPtr<CefFrame> frame,
                                                                          CefRefPtr<CefRequest> request)
{
    if (m_bHostWindowClosed) {
        return nullptr;
    }
    return this;
}

CefResourceRequestHandler::ReturnValue CefBrowserHandler::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                                               CefRefPtr<CefFrame> frame,
                                                                               CefRefPtr<CefRequest> request,
                                                                               CefRefPtr<CefCallback> callback)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnBeforeResourceLoad(browser, frame, request, callback);
    }
    return RV_CONTINUE;
}

CefRefPtr<CefResourceHandler> CefBrowserHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                                                    CefRefPtr<CefFrame> frame,
                                                                    CefRefPtr<CefRequest> request)
{
    return nullptr;
}

void CefBrowserHandler::OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefRequest> request,
                                           CefRefPtr<CefResponse> response,
                                           CefString& new_url)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnResourceRedirect(browser, frame, request, response, new_url);
    }
}

bool CefBrowserHandler::OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefRequest> request,
                                           CefRefPtr<CefResponse> response)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnResourceResponse(browser, frame, request, response);
    }
    return false;
}

CefRefPtr<CefResponseFilter> CefBrowserHandler::GetResourceResponseFilter(CefRefPtr<CefBrowser> browser,
                                                                          CefRefPtr<CefFrame> frame,
                                                                          CefRefPtr<CefRequest> request,
                                                                          CefRefPtr<CefResponse> response)
{
    return nullptr;
}

void CefBrowserHandler::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                               CefRefPtr<CefFrame> frame,
                                               CefRefPtr<CefRequest> request,
                                               CefRefPtr<CefResponse> response,
                                               URLRequestStatus status,
                                               int64_t received_content_length)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnResourceLoadComplete(browser, frame, request, response, status, received_content_length);
    }
}

void CefBrowserHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefRequest> request,
                                            bool& allow_os_execution)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnProtocolExecution(browser, frame, request, allow_os_execution);
    }
}

bool CefBrowserHandler::CanSendCookie(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefRequest> request,
                                      const CefCookie& cookie)
{
    return true;
}

bool CefBrowserHandler::CanSaveCookie(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefRequest> request,
                                      CefRefPtr<CefResponse> response,
                                      const CefCookie& cookie)
{
    return true;
}

bool CefBrowserHandler::CanDownload(CefRefPtr<CefBrowser> browser,
                                    const CefString& url,
                                    const CefString& request_method)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnCanDownload(browser, url, request_method);
    }
    return true;
}

#if CEF_VERSION_MAJOR <= 109
//CEF 109版本
void CefBrowserHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefDownloadItem> download_item,
                                         const CefString& suggested_name,
                                         CefRefPtr<CefBeforeDownloadCallback> callback)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnBeforeDownload(browser, download_item, suggested_name, callback);
    }
}
#else
//CEF 高版本
bool CefBrowserHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                         CefRefPtr<CefDownloadItem> download_item,
                                         const CefString& suggested_name,
                                         CefRefPtr<CefBeforeDownloadCallback> callback)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnBeforeDownload(browser, download_item, suggested_name, callback);
    }
    return true;
}
#endif

void CefBrowserHandler::OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefDownloadItem> download_item,
                                          CefRefPtr<CefDownloadItemCallback> callback)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        m_pHandlerDelegate->OnDownloadUpdated(browser, download_item, callback);
    }
}

#if CEF_VERSION_MAJOR <= 109
//CEF 109版本
bool CefBrowserHandler::OnFileDialog(CefRefPtr<CefBrowser> browser,
                                     FileDialogMode mode,
                                     const CefString& title,
                                     const CefString& default_file_path,
                                     const std::vector<CefString>& accept_filters,
                                     CefRefPtr<CefFileDialogCallback> callback)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnFileDialog(browser, mode, title, default_file_path, accept_filters, std::vector<CefString>(), std::vector<CefString>(), callback);
    }
    return false;
}
#else
//CEF 高版本
bool CefBrowserHandler::OnFileDialog(CefRefPtr<CefBrowser> browser,
                                     FileDialogMode mode,
                                     const CefString& title,
                                     const CefString& default_file_path,
                                     const std::vector<CefString>& accept_filters,
                                     const std::vector<CefString>& accept_extensions,
                                     const std::vector<CefString>& accept_descriptions,
                                     CefRefPtr<CefFileDialogCallback> callback)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed) {
        return m_pHandlerDelegate->OnFileDialog(browser, mode, title, default_file_path, accept_filters, accept_extensions, accept_descriptions, callback);
    }
    return false;
}
#endif

void CefBrowserHandler::OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next)
{
}

bool CefBrowserHandler::OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source)
{
    return false;
}

void CefBrowserHandler::OnGotFocus(CefRefPtr<CefBrowser> browser)
{
    if (m_pHandlerDelegate && !m_bHostWindowClosed && (browser != nullptr) && (m_browser != nullptr) && browser->IsSame(m_browser)) {
        GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&CefBrowserHandlerDelegate::OnGotFocus, m_pHandlerDelegate));
    }
}


} //namespace ui

#pragma warning (pop)
