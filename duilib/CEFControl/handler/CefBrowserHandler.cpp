#include "CefBrowserHandler.h"
#include "duilib/CEFControl/manager/CefManager.h"
#include "duilib/CEFControl/util/CefUtil.h"
#include "duilib/CEFControl/app/CefIPCStringDefs.h"
#include "duilib/CEFControl/app/CefJSBridge.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_frame.h"
#include "include/base/cef_callback.h"
#include "include/base/cef_bind.h"

namespace ui
{
BrowserHandler::BrowserHandler()
{
    window_ = nullptr;
    handle_delegate_ = nullptr;
    is_focus_oneditable_field_ = false;
    ZeroMemory(&rect_cef_control_, sizeof(RECT));
}

void BrowserHandler::SetHostWindow(ui::Window* window)
{ 
    window_ = window;
    window_flag_.reset();
    if (window != nullptr) {
        window_flag_ = window->GetWeakFlag();
    }
}

void BrowserHandler::SetViewRect(RECT rc)
{
    if (!CefCurrentlyOn(TID_UI)) {
        // 把操作跳转到Cef线程执行
        CefPostTask(TID_UI, base::BindOnce(&BrowserHandler::SetViewRect, this, rc));
        return;
    }

    rect_cef_control_ = rc;
    // 调用WasResized接口，调用后，BrowserHandler会调用GetViewRect接口来获取浏览器对象新的位置
    if (browser_.get() && browser_->GetHost().get())
        browser_->GetHost()->WasResized();
}

CefRefPtr<CefBrowserHost> BrowserHandler::GetBrowserHost()
{
    if (browser_.get())
    {
        return browser_->GetHost();
    }
    return nullptr;
}

CefUnregisterCallback BrowserHandler::AddAfterCreateTask(const ui::StdClosure& cb)
{
    return task_list_after_created_.AddCallback(cb);
}

void BrowserHandler::CloseAllBrowser()
{
    class CloseAllBrowserTask : public CefTask
    {
        IMPLEMENT_REFCOUNTING(CloseAllBrowserTask);
    public:
        explicit CloseAllBrowserTask(const std::vector<CefRefPtr<CefBrowser>>& browser_list)
        {
            browser_list_.assign(browser_list.begin(), browser_list.end());
        }
    public:
        void Execute()
        {
            for (auto it : browser_list_) {
                if (it != nullptr)
                    it->GetHost()->CloseBrowser(true);
            }
        }
    private:
        std::vector<CefRefPtr<CefBrowser>> browser_list_;
    };
    CefPostTask(TID_UI, new CloseAllBrowserTask(browser_list_));
}

bool BrowserHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                             CefRefPtr<CefFrame> frame,
                                             CefProcessId /*source_process*/,
                                             CefRefPtr<CefProcessMessage> message)
{
    // 处理render进程发来的消息
    std::string message_name = message->GetName();
    if (message_name == kFocusedNodeChangedMessage)
    {
        is_focus_oneditable_field_ = message->GetArgumentList()->GetBool(0);
        return true;
    }
    else if (message_name == kCallCppFunctionMessage)
    {
        CefString fun_name    = message->GetArgumentList()->GetString(0);
        CefString param        = message->GetArgumentList()->GetString(1);
        int js_callback_id    = message->GetArgumentList()->GetInt(2);

        if (handle_delegate_)
            handle_delegate_->OnExecuteCppFunc(fun_name, param, js_callback_id, browser, frame);

        return true;
    }
    else if (message_name == kExecuteCppCallbackMessage)
    {
        CefString param = message->GetArgumentList()->GetString(0);
        int callback_id = message->GetArgumentList()->GetInt(1);

        if (handle_delegate_)
            handle_delegate_->OnExecuteCppCallbackFunc(callback_id, param);
    }

    return false;
}

#pragma region CefLifeSpanHandler
// CefLifeSpanHandler methods
bool BrowserHandler::OnBeforePopup( CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    int /*popup_id*/,
                                    const CefString& target_url,
                                    const CefString& target_frame_name,
                                    CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                    bool user_gesture,
                                    const CefPopupFeatures& popupFeatures,
                                    CefWindowInfo& windowInfo,
                                    CefRefPtr<CefClient>& client,
                                    CefBrowserSettings& settings,
                                    CefRefPtr<CefDictionaryValue>& /*extra_info*/,
                                    bool* no_javascript_access)
{
    // 让新的链接在原浏览器对象中打开
    if (browser_.get() && !target_url.empty())
    {
        if (handle_delegate_)
        {
            // 返回true则继续在控件内打开新链接，false则禁止访问
            bool bRet = handle_delegate_->OnBeforePopup(browser, frame, target_url, target_frame_name, target_disposition, user_gesture, popupFeatures, windowInfo, client, settings, no_javascript_access);
            if (bRet)
                browser_->GetMainFrame()->LoadURL(target_url);
        }
        else
            browser_->GetMainFrame()->LoadURL(target_url);
    }

    // 禁止弹出popup窗口
    return true;
}

void BrowserHandler::OnBeforePopupAborted(CefRefPtr<CefBrowser> /*browser*/, int /*popup_id*/)
{
}

void BrowserHandler::OnBeforeDevToolsPopup(CefRefPtr<CefBrowser> /*browser*/,
                                           CefWindowInfo& /*windowInfo*/,
                                           CefRefPtr<CefClient>& /*client*/,
                                           CefBrowserSettings& /*settings*/,
                                           CefRefPtr<CefDictionaryValue>& /*extra_info*/,
                                           bool* /*use_default_window*/)
{
}

void BrowserHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    REQUIRE_UI_THREAD();
    ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, browser](){
        browser_list_.emplace_back(browser);
        if (browser_ != nullptr)
            browser_->GetHost()->WasHidden(true);
        browser_ = browser;
        CefManager::GetInstance()->AddBrowserCount();

        if (handle_delegate_)
        {
            handle_delegate_->OnAfterCreated(browser);
        }

        // 有窗模式下，浏览器创建完毕后，让上层更新一下自己的位置；因为在异步状态下，上层更新位置时可能Cef窗口还没有创建出来
        if (!CefManager::GetInstance()->IsEnableOffsetRender() && handle_delegate_)
        {
            handle_delegate_->UpdateWindowPos();
        }

        task_list_after_created_();
        task_list_after_created_.Clear();
    }));

    // 必须在cef ui线程调用RegisterDragDrop
    // 执行::DoDragDrop时，会在调用RegisterDragDrop的线程触发的DragOver、DragLeave、Drop、Drop回调
    // 进而调用browser_->GetHost()->DragTargetDragEnter、DragTargetDragOver、DragTargetDragLeave、DragTargetDrop
    // 这几个cef接口内部发现不在cef ui线程触发，则会转发到cef ui线程
    // 导致DragSourceEndedAt接口被调用时有部分DragTarget*方法没有被调用
    // 最终拖拽效果就会有问题，详见DragSourceEndedAt接口描述
    // 所以在cef ui线程调用RegisterDragDrop，让后面一系列操作都在cef ui线程里同步执行，则没问题
    //
    // RegisterDragDrop内部会在调用这个API的线程里创建一个窗口，用过这个窗口来做消息循环模拟阻塞的过程
    // 所以哪个线程调用RegisterDragDrop，就会在哪个线程阻塞并触发IDragTarget回调
    // 见https://docs.microsoft.com/zh-cn/windows/win32/api/ole2/nf-ole2-registerdragdrop
    if ((window_ != nullptr) && !window_flag_.expired()) {
        drop_target_ = CefManager::GetInstance()->GetDropTarget(window_->NativeWnd()->GetHWND());
    }
}

bool BrowserHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
    return false;
}

void BrowserHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    REQUIRE_UI_THREAD();
    ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, ToWeakCallback([this, browser](){
        CefManager::GetInstance()->SubBrowserCount();
        auto it = std::find_if(browser_list_.begin(), browser_list_.end(), [&](const CefRefPtr<CefBrowser>& item){
            return item->IsSame(browser);
        });
        if (it != browser_list_.end())
        {
            auto closed_browser = *it;
            browser_list_.erase(it);
            if (closed_browser->IsSame(browser_))
            {
                browser_ = browser_list_.size() > 0 ? *browser_list_.rbegin() : nullptr;
                if (browser_ != nullptr)
                {
                    browser_->GetHost()->WasHidden(false);
                    browser_->Reload();
                }
            }
        }

        if (handle_delegate_)
        {
            handle_delegate_->OnBeforeClose(browser);
        }
    }));
}
#pragma endregion

#pragma region CefRenderHandler
// CefRenderHandler methods

CefRefPtr<CefAccessibilityHandler> BrowserHandler::GetAccessibilityHandler()
{
    return nullptr;
}

bool BrowserHandler::GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    if ((window_ == nullptr) || window_flag_.expired()) {
        return false;
    }
    RECT window_rect = { 0 };
    HWND root_window = GetAncestor(window_->NativeWnd()->GetHWND(), GA_ROOT);
    if (::GetWindowRect(root_window, &window_rect))
    {
        rect = CefRect(window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
        return true;
    }
    return false;
}

void BrowserHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    if ((window_ == nullptr) || window_flag_.expired()) {
        return;
    }
    if (handle_delegate_)
    {
        rect.x = 0;
        rect.y = 0;
        rect.width = rect_cef_control_.right - rect_cef_control_.left;
        rect.height = rect_cef_control_.bottom - rect_cef_control_.top;

        if (CefManager::GetInstance()->IsEnableOffsetRender()) {
            //离屏渲染模式，需要传给原始宽度和高度，因为CEF内部会进一步做DPI自适应
            uint32_t dpiScale = window_->Dpi().GetScale();
            if (dpiScale > 100) {
                rect.width = rect.width * 100 / dpiScale;
                rect.height = rect.height * 100 / dpiScale;
            }
        }        
        return;
    }
    else
    {
        ui::UiRect clientRect;
        window_->GetClientRect(clientRect);
        rect.x = rect.y = 0;
        rect.width = clientRect.right;
        rect.height = clientRect.bottom;
        return;
    }

}

bool BrowserHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY)
{
    if ((window_ == nullptr) || window_flag_.expired()) {
        return false;
    }
    if (!window_->IsWindow()) {
        return false;
    }

    // Convert the point from view coordinates to actual screen coordinates.
    ui::UiPoint screen_pt = { viewX, viewY};
    if (CefManager::GetInstance()->IsEnableOffsetRender()) {
        //离屏渲染模式下，给到的参数是原始坐标，未经DPI自适应，所以需要做DPI自适应处理，否则页面的右键菜单位置显示不对
        uint32_t dpiScale = window_->Dpi().GetScale();
        if (dpiScale > 100) {
            screen_pt.x = screen_pt.x * dpiScale / 100;
            screen_pt.y = screen_pt.y * dpiScale / 100;
        }
    }
    //将页面坐标转换为窗口客户区坐标，否则页面弹出的右键菜单位置不正确
    screen_pt.x = screen_pt.x + rect_cef_control_.left;
    screen_pt.y = screen_pt.y + rect_cef_control_.top;
    window_->ClientToScreen(screen_pt);
    screenX = screen_pt.x;
    screenY = screen_pt.y;
    return true;
}

bool BrowserHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
{
    //只有离屏渲染模式下，才处理DPI缩放因子
    if (!CefManager::GetInstance()->IsEnableOffsetRender()) {
        return false;
    }
    if ((window_ == nullptr) || window_flag_.expired()) {
        return false;
    }
    uint32_t dpiScale = window_->Dpi().GetScale();
    if (dpiScale == 100) {
        return false;
    }
    else {
        screen_info.device_scale_factor = dpiScale / 100.0f;
        return true;
    }    
}

void BrowserHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnPopupShow, handle_delegate_, browser, show));
}

void BrowserHandler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnPopupSize, handle_delegate_, browser, rect));
}

void BrowserHandler::OnPaint(CefRefPtr<CefBrowser> browser,
    PaintElementType type,
    const RectList& dirtyRects,
    const void* buffer,
    int width,
    int height)
{
    if (handle_delegate_)
    {
        // 多线程消息循环模式中，OnPaint在Cef的线程被触发，这时把数据保存到paint_buffer_中，跳转到UI线程执行渲染操作。
        // 这里不对paint_buffer_加锁，即使两个线程操作paint_buffer_发生竞争，也只是让某一次渲染效果有瑕疵，不会崩溃，这个瑕疵是可以接受的
        int buffer_length = width * height * 4;
        if (buffer_length > (int)paint_buffer_.size())
            paint_buffer_.resize(buffer_length + 1);
        memcpy(&paint_buffer_[0], (char*)buffer, width * height * 4);

        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnPaint, handle_delegate_, browser, type, dirtyRects, &paint_buffer_, width, height));
    }
}

void BrowserHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const CefAcceleratedPaintInfo& info)
{
}

void BrowserHandler::GetTouchHandleSize(CefRefPtr<CefBrowser> browser,
                                        cef_horizontal_alignment_t orientation,
                                        CefSize& size) {
}

void BrowserHandler::OnTouchHandleStateChanged(CefRefPtr<CefBrowser> browser, const CefTouchHandleState& state)
{
}

void BrowserHandler::OnScrollOffsetChanged(CefRefPtr<CefBrowser> browser, double x, double y)
{
}

void BrowserHandler::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser,
                                                  const CefRange& selected_range,
                                                  const RectList& character_bounds)
{
}

void BrowserHandler::OnTextSelectionChanged(CefRefPtr<CefBrowser> browser, const CefString& selected_text, const CefRange& selected_range)
{
}

void BrowserHandler::OnVirtualKeyboardRequested(CefRefPtr<CefBrowser> browser, TextInputMode input_mode)
{
}

bool BrowserHandler::StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y)
{
    REQUIRE_UI_THREAD();
    if (!handle_delegate_)
        return false;

    if (!drop_target_)
        return false;

    current_drag_op_ = DRAG_OPERATION_NONE;
    CefBrowserHost::DragOperationsMask result =
        drop_target_->StartDragging(this, drag_data, allowed_ops, x, y);
    current_drag_op_ = DRAG_OPERATION_NONE;
    POINT pt = {};
    if ((window_ != nullptr) && !window_flag_.expired()) {
        ui::UiPoint uiPt;
        window_->GetCursorPos(uiPt);
        window_->ScreenToClient(uiPt);
        pt.x = uiPt.x;
        pt.y = uiPt.y;
    }
    handle_delegate_->ClientToControl(pt);
    browser->GetHost()->DragSourceEndedAt(
        pt.x,
        pt.y,
        result);
    browser->GetHost()->DragSourceSystemDragEnded();

    return true;
}

void BrowserHandler::UpdateDragCursor(CefRefPtr<CefBrowser> browser, CefRenderHandler::DragOperation operation)
{
    REQUIRE_UI_THREAD();
    current_drag_op_ = operation;
}

CefBrowserHost::DragOperationsMask BrowserHandler::OnDragEnter(CefRefPtr<CefDragData> drag_data, CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect)
{
    REQUIRE_UI_THREAD();
    if (browser_) {
        POINT pt = { ev.x, ev.y };
        handle_delegate_->ClientToControl(pt);
        ev.x = pt.x;
        ev.y = pt.y;
        browser_->GetHost()->DragTargetDragEnter(drag_data, ev, effect);
        browser_->GetHost()->DragTargetDragOver(ev, effect);
    }
    return current_drag_op_;
}

CefBrowserHost::DragOperationsMask BrowserHandler::OnDragOver(CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect)
{
    REQUIRE_UI_THREAD();
    if (browser_) {
        POINT pt = { ev.x, ev.y };
        handle_delegate_->ClientToControl(pt);
        ev.x = pt.x;
        ev.y = pt.y;
        browser_->GetHost()->DragTargetDragOver(ev, effect);
    }
    return current_drag_op_;
}

void BrowserHandler::OnDragLeave()
{
    REQUIRE_UI_THREAD();
    if (browser_)
        browser_->GetHost()->DragTargetDragLeave();
}

CefBrowserHost::DragOperationsMask BrowserHandler::OnDrop(CefMouseEvent ev, CefBrowserHost::DragOperationsMask effect)
{
    if (browser_) {
        POINT pt = { ev.x, ev.y };
        handle_delegate_->ClientToControl(pt);
        ev.x = pt.x;
        ev.y = pt.y;
        browser_->GetHost()->DragTargetDragOver(ev, effect);
        browser_->GetHost()->DragTargetDrop(ev);
    }
    return current_drag_op_;
}

#pragma endregion

#pragma region CefContextMenuHandler
// CefContextMenuHandler methods
void BrowserHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
CefRefPtr<CefFrame> frame,
CefRefPtr<CefContextMenuParams> params,
CefRefPtr<CefMenuModel> model)
{
    REQUIRE_UI_THREAD();

    if (handle_delegate_)
    {
        handle_delegate_->OnBeforeContextMenu(browser, frame, params, model);
    }
    else
    {
        // Customize the context menu...
        if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0)
        {
            if (model->GetCount() > 0)
            {
                // 禁止右键菜单
                model->Clear();
            }
        }
    }
}

bool BrowserHandler::RunContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback)
{
    return false;
}

bool BrowserHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags)
{
    if (handle_delegate_)
        return handle_delegate_->OnContextMenuCommand(browser, frame, params, command_id, event_flags);
    else
        return false;
}

void BrowserHandler::OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{

}

bool BrowserHandler::RunQuickMenu(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefFrame> frame,
                                  const CefPoint& location,
                                  const CefSize& size,
                                  QuickMenuEditStateFlags edit_state_flags,
                                  CefRefPtr<CefRunQuickMenuCallback> callback)
{
    return false;
}

bool BrowserHandler::OnQuickMenuCommand(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        int command_id,
                                        EventFlags event_flags)
{
    return false;
}

void BrowserHandler::OnQuickMenuDismissed(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame)
{
}

#pragma endregion

#pragma region CefDisplayHandler

// CefDisplayHandler methods
void BrowserHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
    // Update the URL in the address bar...
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnAddressChange, handle_delegate_, browser, frame, url));
}

void BrowserHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    // Update the browser window title...
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnTitleChange, handle_delegate_, browser, title));
}

void BrowserHandler::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls)
{
}

void BrowserHandler::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen)
{
}

bool BrowserHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
{
    return false;
}

void BrowserHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
}

bool BrowserHandler::OnConsoleMessage(CefRefPtr<CefBrowser> /*browser*/,
                                      cef_log_severity_t /*level*/,
                                      const CefString& /*message*/,
                                      const CefString& /*source*/,
                                      int /*line*/)
{
    // Log a console message...
    return true;
}

bool BrowserHandler::OnAutoResize(CefRefPtr<CefBrowser> browser, const CefSize& new_size)
{
    return false;
}

void BrowserHandler::OnLoadingProgressChange(CefRefPtr<CefBrowser> browser, double progress)
{
}

bool BrowserHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                    CefCursorHandle cursor,
                                    cef_cursor_type_t type,
                                    const CefCursorInfo& custom_cursor_info)
{
    if ((window_ != nullptr) && !window_flag_.expired()) {
        SetClassLongPtr(window_->NativeWnd()->GetHWND(), GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
    }
    SetCursor(cursor);
    return true;
}

void BrowserHandler::OnMediaAccessChange(CefRefPtr<CefBrowser> browser,
                                         bool has_video_access,
                                         bool has_audio_access) {
}

#pragma endregion

bool BrowserHandler::OnDragEnter(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> dragData, CefDragHandler::DragOperationsMask mask)
{
    return false;
}

void BrowserHandler::OnDraggableRegionsChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const std::vector<CefDraggableRegion>& regions)
{
}

// CefLoadHandler methods
void BrowserHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
    // Update UI for browser state...
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnLoadingStateChange, handle_delegate_, browser, isLoading, canGoBack, canGoForward));
}

void BrowserHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
                                 CefRefPtr<CefFrame> frame,
                                 TransitionType /*transition_type*/)
{
    // A frame has started loading content...
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnLoadStart, handle_delegate_, browser, frame));
}

void BrowserHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
    // A frame has finished loading content...
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnLoadEnd, handle_delegate_, browser, frame, httpStatusCode));
}

void BrowserHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
    // A frame has failed to load content...
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnLoadError, handle_delegate_, browser, frame, errorCode, errorText, failedUrl));
}

bool BrowserHandler::OnJSDialog(CefRefPtr<CefBrowser> /*browser*/,
                                const CefString& /*origin_url*/,
                                JSDialogType /*dialog_type*/,
                                const CefString& /*message_text*/,
                                const CefString& /*default_prompt_text*/,
                                CefRefPtr<CefJSDialogCallback> callback,
                                bool& suppress_message)
{
    // release时阻止弹出js对话框
    (void)suppress_message;
#ifndef _DEBUG
    suppress_message = true;
#endif

    return false;
}

bool BrowserHandler::OnBeforeUnloadDialog(CefRefPtr<CefBrowser> browser,
                                          const CefString& message_text,
                                          bool is_reload,
                                          CefRefPtr<CefJSDialogCallback> callback)
{
    return false;
}

void BrowserHandler::OnResetDialogState(CefRefPtr<CefBrowser> browser)
{
}

void BrowserHandler::OnDialogClosed(CefRefPtr<CefBrowser> browser)
{
}

bool BrowserHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event, bool* is_keyboard_shortcut)
{
    return false;
}

bool BrowserHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
    return false;
}

// CefRequestHandler methods
bool BrowserHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                    CefRefPtr<CefFrame> frame,
                                    CefRefPtr<CefRequest> request,
                                    bool /*user_gesture*/,
                                    bool is_redirect)
{
    if (handle_delegate_)
        return handle_delegate_->OnBeforeBrowse(browser, frame, request, is_redirect);
    
    return false;
}

bool BrowserHandler::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      const CefString& target_url,
                                      CefRequestHandler::WindowOpenDisposition target_disposition,
                                      bool user_gesture)
{
    return false;
}

CefRefPtr<CefResourceRequestHandler> BrowserHandler::GetResourceRequestHandler(CefRefPtr<CefBrowser> browser,
                                                                               CefRefPtr<CefFrame> frame,
                                                                               CefRefPtr<CefRequest> request,
                                                                               bool is_navigation,
                                                                               bool is_download,
                                                                               const CefString& request_initiator,
                                                                               bool& disable_default_handling)
{
    return this;
}

bool BrowserHandler::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
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

bool BrowserHandler::OnCertificateError(CefRefPtr<CefBrowser> browser,
                                        cef_errorcode_t cert_error,
                                        const CefString& request_url,
                                        CefRefPtr<CefSSLInfo> ssl_info,
                                        CefRefPtr<CefCallback> callback)
{
    return false;
}

bool BrowserHandler::OnSelectClientCertificate(CefRefPtr<CefBrowser> browser,
                                               bool isProxy,
                                               const CefString& host,
                                               int port,
                                               const X509CertificateList& certificates,
                                               CefRefPtr<CefSelectClientCertificateCallback> callback)
{
    return false;
}

void BrowserHandler::OnRenderViewReady(CefRefPtr<CefBrowser> browser)
{
}

bool BrowserHandler::OnRenderProcessUnresponsive(CefRefPtr<CefBrowser> browser,
                                                 CefRefPtr<CefUnresponsiveProcessCallback> callback)
{
    return false;
}

void BrowserHandler::OnRenderProcessResponsive(CefRefPtr<CefBrowser> browser)
{
}

void BrowserHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                               TerminationStatus status,
                                               int error_code,
                                               const CefString& error_string)
{
    if (handle_delegate_)
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, UiBind(&HandlerDelegate::OnRenderProcessTerminated, handle_delegate_, browser, status));
}

void BrowserHandler::OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser)
{
}

CefRefPtr<CefCookieAccessFilter> BrowserHandler::GetCookieAccessFilter(CefRefPtr<CefBrowser> browser,
                                                                       CefRefPtr<CefFrame> frame,
                                                                       CefRefPtr<CefRequest> request)
{
    return this;
}

CefResourceRequestHandler::ReturnValue BrowserHandler::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                                            CefRefPtr<CefFrame> frame,
                                                                            CefRefPtr<CefRequest> request,
                                                                            CefRefPtr<CefCallback> callback)
{
     if (handle_delegate_)
        return handle_delegate_->OnBeforeResourceLoad(browser, frame, request, callback);

    return RV_CONTINUE;
}

CefRefPtr<CefResourceHandler> BrowserHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                                                 CefRefPtr<CefFrame> frame,
                                                                 CefRefPtr<CefRequest> request)
{
    return nullptr;
}

void BrowserHandler::OnResourceRedirect(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response,
                                        CefString& new_url)
{
}

bool BrowserHandler::OnResourceResponse(CefRefPtr<CefBrowser> browser,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefRequest> request,
                                        CefRefPtr<CefResponse> response)
{
    return false;
}

CefRefPtr<CefResponseFilter> BrowserHandler::GetResourceResponseFilter(CefRefPtr<CefBrowser> browser,
                                                                       CefRefPtr<CefFrame> frame,
                                                                       CefRefPtr<CefRequest> request,
                                                                       CefRefPtr<CefResponse> response)
{
    return nullptr;
}

void BrowserHandler::OnResourceLoadComplete(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefRequest> request,
                                            CefRefPtr<CefResponse> response,
                                            URLRequestStatus status,
                                            int64_t received_content_length)
{
}

void BrowserHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)
{
    if (handle_delegate_)
        handle_delegate_->OnProtocolExecution(browser, url, allow_os_execution);
}

bool BrowserHandler::CanSendCookie(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefRequest> request,
                                   const CefCookie& cookie)
{
    return true;
}

bool BrowserHandler::CanSaveCookie(CefRefPtr<CefBrowser> browser,
                                   CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefRequest> request,
                                   CefRefPtr<CefResponse> response,
                                   const CefCookie& cookie)
{
    return true;
}

bool BrowserHandler::CanDownload(CefRefPtr<CefBrowser> browser,
                                 const CefString& url,
                                 const CefString& request_method)
{
    return true;
}

bool BrowserHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefDownloadItem> download_item,
                                      const CefString& suggested_name,
                                      CefRefPtr<CefBeforeDownloadCallback> callback)
{
    if (handle_delegate_)
        handle_delegate_->OnBeforeDownload(browser, download_item, suggested_name, callback);
    return true;
}

void BrowserHandler::OnDownloadUpdated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    CefRefPtr<CefDownloadItemCallback> callback)
{
    if (handle_delegate_)
        handle_delegate_->OnDownloadUpdated(browser, download_item, callback);
}

bool BrowserHandler::OnFileDialog(  CefRefPtr<CefBrowser> browser,
                                    FileDialogMode mode,
                                    const CefString& title,
                                    const CefString& default_file_path,
                                    const std::vector<CefString>& accept_filters,
                                    const std::vector<CefString>& /*accept_extensions*/,
                                    const std::vector<CefString>& /*accept_descriptions*/,
                                    CefRefPtr<CefFileDialogCallback> callback)
{
    if (handle_delegate_)
        return handle_delegate_->OnFileDialog(browser, mode, title, default_file_path, accept_filters, 0, callback);
    else
        return false;
}

}

#pragma warning (pop)
