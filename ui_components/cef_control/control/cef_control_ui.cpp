#include "cef_control.h"
#include "ui_components/cef_control/handler/browser_handler.h"
#include "ui_components/cef_control/manager/cef_manager.h"
#include "ui_components/cef_control/app/cef_js_bridge.h"
#include "ui_components/public_define.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"

#include "base/thread/thread_manager.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_runnable.h"
#pragma warning (pop)

namespace nim_comp {

namespace {
	#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
	#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
}

CefControl::CefControl(void):
	devtool_view_(nullptr)
{
}

CefControl::~CefControl(void)
{
	if (browser_handler_.get())
	{
		browser_handler_->SetHostWindow(NULL);
		browser_handler_->SetHandlerDelegate(NULL);

		if (browser_handler_->GetBrowser().get()) {
			// Request that the main browser close.
			browser_handler_->CloseAllBrowser();
		}
	}
	GetWindow()->RemoveMessageFilter(this);
}

void CefControl::Init()
{
	if (browser_handler_.get() == nullptr)
	{
		GetWindow()->AddMessageFilter(this);
		
		browser_handler_ = new nim_comp::BrowserHandler;
		browser_handler_->SetHostWindow(GetWindow()->GetHWND());
		browser_handler_->SetHandlerDelegate(this);
		ReCreateBrowser();
	}

	if (!js_bridge_.get())
	{
		js_bridge_.reset(new nim_comp::CefJSBridge);
	}

	__super::Init();
}

void CefControl::ReCreateBrowser()
{
	if (browser_handler_->GetBrowser() == nullptr)
	{
		// 使用无窗模式，离屏渲染
		CefWindowInfo window_info;
		window_info.SetAsWindowless(GetWindow()->GetHWND(), false);
		CefBrowserSettings browser_settings;
		//browser_settings.file_access_from_file_urls = STATE_ENABLED;
		//browser_settings.universal_access_from_file_urls = STATE_ENABLED;
		CefBrowserHost::CreateBrowser(window_info, browser_handler_, L"", browser_settings, NULL);
	}	
}

void CefControl::SetPos(ui::UiRect rc)
{
	__super::SetPos(rc);

	if (browser_handler_.get())
	{
		browser_handler_->SetViewRect({ rc.left, rc.top, rc.right, rc.bottom });
	}
}

void CefControl::HandleEvent(const ui::EventArgs& msg)
{
	if (IsDisabledEvents(msg)) {
		//如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
		ui::Box* pParent = GetParent();
		if (pParent != nullptr) {
			pParent->SendEvent(msg);
		}
		else {
			__super::HandleEvent(msg);
		}
		return;
	}
	if (browser_handler_.get() && browser_handler_->GetBrowser().get() == NULL) {
		return __super::HandleEvent(msg);
	}

	else if (msg.Type == ui::kEventSetFocus) {
		if (browser_handler_->GetBrowserHost().get()) {
			browser_handler_->GetBrowserHost()->SendFocusEvent(true);
		}
	}
	else if (msg.Type == ui::kEventKillFocus) {
		if (browser_handler_->GetBrowserHost().get()) {
			browser_handler_->GetBrowserHost()->SendFocusEvent(false);
		}
	}

	__super::HandleEvent(msg);
}

void CefControl::SetVisible(bool bVisible)
{
	__super::SetVisible(bVisible);
	if (browser_handler_.get() && browser_handler_->GetBrowserHost().get())
	{
		browser_handler_->GetBrowserHost()->WasHidden(!bVisible);
	}	
}

void CefControl::Paint(ui::IRender* pRender, const ui::UiRect& rcPaint)
{
	__super::Paint(pRender, rcPaint);

	if (dc_cef_.IsValid() && browser_handler_.get() && browser_handler_->GetBrowser().get())
	{
		// 绘制cef PET_VIEW类型的位图
		ui::UiRect rect = GetRect();

		std::unique_ptr<ui::IBitmap> bitmap;
		ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
		ASSERT(pRenderFactory != nullptr);
		if (pRenderFactory != nullptr) {
			bitmap.reset(pRenderFactory->CreateBitmap());
		}
		ASSERT(bitmap != nullptr);
		if (bitmap == nullptr) {
			return;
		}

		if (!bitmap->Init(dc_cef_.GetWidth(), dc_cef_.GetHeight(), true, dc_cef_.GetBits())) {
			return;
		}

		pRender->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), bitmap.get(), 0, 0, ui::RopMode::kSrcCopy);

		// 绘制cef PET_POPUP类型的位图
		if (!rect_popup_.IsEmpty() && dc_cef_popup_.IsValid())
		{
			// 假如popup窗口位置在控件的范围外，则修正到控件范围内，指绘制控件范围内的popup窗口
			int paint_x = rect_popup_.x;
			int paint_y = rect_popup_.y;
			int paint_buffer_x = 0;
			int paint_buffer_y = 0;
			if (rect_popup_.x < 0)
			{
				paint_x = 0;
				paint_buffer_x = -rect_popup_.x;
			}
			if (rect_popup_.y < 0)
			{
				paint_y = 0;
				paint_buffer_y = -rect_popup_.y;
			}			
			if (!bitmap->Init(dc_cef_popup_.GetWidth(), dc_cef_popup_.GetHeight(), true, dc_cef_popup_.GetBits())) {
				return;
			}
			rect = GetRect();
			pRender->BitBlt(rect.left + paint_x, rect.top + paint_y, rect_popup_.width, rect_popup_.height, bitmap.get(), paint_buffer_x, paint_buffer_y, ui::RopMode::kSrcCopy);
		}
	}
}

void CefControl::SetWindow(ui::Window* pManager)
{
	if (!browser_handler_) {
		__super::SetWindow(pManager);
		return;
	}

	if (GetWindow()) {
		GetWindow()->RemoveMessageFilter(this);
		__super::SetWindow(pManager);
		pManager->AddMessageFilter(this);
	}

	browser_handler_->SetHostWindow(pManager->GetHWND());
}

LRESULT CefControl::FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (!IsVisible() || !IsEnabled())
	{
		bHandled = false;
		return 0;
	}

	bHandled = false;
	if (browser_handler_.get() == nullptr || browser_handler_->GetBrowser().get() == nullptr)
		return 0;

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
		ui::UiPoint pt;
		GetWindow()->GetCursorPos(pt);
		GetWindow()->ScreenToClient(pt);
		if (!GetRect().ContainsPt(pt))
			return 0;

		GetWindow()->CallDefaultWindowProc(uMsg, wParam, lParam);
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

	return 0;
}

bool CefControl::AttachDevTools(Control* control)
{
	CefControl *view = dynamic_cast<CefControl*>(control);
	if (devtool_attached_ || !view) {
		return true;
	}
	auto browser = browser_handler_->GetBrowser();
	auto view_browser = view->browser_handler_->GetBrowser();
	if (browser == nullptr || view_browser == nullptr)
	{
		auto weak = view->GetWeakFlag();
		auto task = [this, weak, view](){
			nbase::ThreadManager::PostTask(kThreadUI, ToWeakCallback([this, weak, view](){
				if (weak.expired())
					return;
				AttachDevTools(view);
			}));
		};
		view->browser_handler_->AddAfterCreateTask(task);
	}
	else
	{
		CefWindowInfo windowInfo;
		windowInfo.SetAsWindowless(GetWindow()->GetHWND(), false);
		CefBrowserSettings settings;
		browser->GetHost()->ShowDevTools(windowInfo, view_browser->GetHost()->GetClient(), settings, CefPoint());
		devtool_attached_ = true;
		devtool_view_ = view;
		if (cb_devtool_visible_change_ != nullptr) {
			cb_devtool_visible_change_(devtool_attached_);
		}
	}
	return true;
}

void CefControl::DettachDevTools()
{
	CefControlBase::DettachDevTools();
	devtool_view_ = nullptr;
}

void CefControl::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
	if (devtool_attached_ && (devtool_view_ != nullptr) && CefManager::GetInstance()->IsEnableOffsetRender()) {
		//离屏渲染模式，开发者工具与页面位于相同的客户区位置
		int x = params->GetXCoord();
		int y = params->GetYCoord();
		//离屏渲染模式下，给到的参数是原始坐标，未经DPI自适应，所以需要做DPI自适应处理，否则页面的右键菜单位置显示不对
		uint32_t dpiScale = ui::GlobalManager::Instance().Dpi().GetScale();
		if (dpiScale > 100) {
			x = x * dpiScale / 100;
			y = y * dpiScale / 100;
		}

		ui::UiPoint pt = { x + GetRect().left, y + GetRect().top};
		ui::UiPoint offsetPt = GetScrollOffsetInScrollBox();
		pt.Offset(offsetPt);
		ui::UiRect rect = GetRect();
		ui::UiRect rectView = devtool_view_->GetRect();
		bool isPtInPageRect = GetRect().ContainsPt(pt);
		bool isPtInToolRect = devtool_view_->GetRect().ContainsPt(pt);
		if (isPtInToolRect && !isPtInPageRect) {
			//如果点击区域，位于开发工具区域，则不弹出页面的右键菜单
			if (model->GetCount() > 0)			{
				// 禁止右键菜单
				model->Clear();
			}
			return;
		}
	}
	CefControlBase::OnBeforeContextMenu(browser, frame, params, model);
}

//////////////////////////////////////////////////////////////////////////////////
LRESULT CefControl::SendButtonDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

	ui::UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	pt.Offset(GetScrollOffsetInScrollBox());
	if (!GetRect().ContainsPt(pt))
		return 0;

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

	bHandled = false;
	return 0;
}

LRESULT CefControl::SendButtonDoubleDownEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

	ui::UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	pt.Offset(GetScrollOffsetInScrollBox());
	if (!GetRect().ContainsPt(pt))
		return 0;

	CefMouseEvent mouse_event;
	mouse_event.x = pt.x - GetRect().left;
	mouse_event.y = pt.y - GetRect().top;
	mouse_event.modifiers = GetCefMouseModifiers(wParam);
	AdaptDpiScale(mouse_event);
	CefBrowserHost::MouseButtonType btnType =
		(uMsg == WM_LBUTTONDOWN ? MBT_LEFT : (
		uMsg == WM_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));

	host->SendMouseClickEvent(mouse_event, btnType, false, 2);

	bHandled = true;
	return 0;
}

LRESULT CefControl::SendButtonUpEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

	ui::UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	pt.Offset(GetScrollOffsetInScrollBox());
	if (!GetRect().ContainsPt(pt) && !GetWindow()->IsCaptured())
		return 0;

	CefMouseEvent mouse_event;
	mouse_event.x = pt.x - GetRect().left;
	mouse_event.y = pt.y - GetRect().top;
	mouse_event.modifiers = GetCefMouseModifiers(wParam);
	AdaptDpiScale(mouse_event);
	CefBrowserHost::MouseButtonType btnType =
		(uMsg == WM_LBUTTONUP ? MBT_LEFT : (
		uMsg == WM_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));

	host->SendMouseClickEvent(mouse_event, btnType, true, 1);

	bHandled = false;
	return 0;
}

LRESULT CefControl::SendMouseMoveEvent(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

	ui::UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	pt.Offset(GetScrollOffsetInScrollBox());
	if (!GetRect().ContainsPt(pt) && !GetWindow()->IsCaptured())
		return 0;

	CefMouseEvent mouse_event;
	mouse_event.x = pt.x - GetRect().left;
	mouse_event.y = pt.y - GetRect().top;
	mouse_event.modifiers = GetCefMouseModifiers(wParam);
	AdaptDpiScale(mouse_event);
	host->SendMouseMoveEvent(mouse_event, false);

	bHandled = false;
	return 0;
}

LRESULT CefControl::SendMouseWheelEvent(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

	ui::UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	ui::Window* pScrolledWnd = GetWindow()->WindowFromPoint(pt);
	if (pScrolledWnd != GetWindow()) {
		return 0;
	}

	GetWindow()->ScreenToClient(pt);
	pt.Offset(GetScrollOffsetInScrollBox());
	if (!GetRect().ContainsPt(pt))
		return 0;

	int delta = GET_WHEEL_DELTA_WPARAM(wParam);

	CefMouseEvent mouse_event;
	mouse_event.x = pt.x - GetRect().left;
	mouse_event.y = pt.y - GetRect().top;
	mouse_event.modifiers = GetCefMouseModifiers(wParam);
	AdaptDpiScale(mouse_event);
	host->SendMouseWheelEvent(mouse_event, IsKeyDown(VK_SHIFT) ? delta : 0, !IsKeyDown(VK_SHIFT) ? delta : 0);

	bHandled = true;
	return 0;
}

LRESULT CefControl::SendMouseLeaveEvent(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

	ui::UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	pt.Offset(GetScrollOffsetInScrollBox());
	if (!GetRect().ContainsPt(pt))
		return 0;

	CefMouseEvent mouse_event;
	mouse_event.x = pt.x - GetRect().left;
	mouse_event.y = pt.y - GetRect().top;
	mouse_event.modifiers = GetCefMouseModifiers(wParam);
	AdaptDpiScale(mouse_event);
	host->SendMouseMoveEvent(mouse_event, true);

	bHandled = true;
	return 0;
}

LRESULT CefControl::SendKeyEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();
	
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

	bHandled = true;
	return 0;
}

LRESULT CefControl::SendCaptureLostEvent(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	CefRefPtr<CefBrowserHost> host = browser_handler_->GetBrowserHost();

	host->SendCaptureLostEvent();
	bHandled = true;
	return 0;
}

bool CefControl::IsKeyDown(WPARAM wparam) 
{
	return (GetKeyState(static_cast<int>(wparam)) & 0x8000) != 0;
}

int CefControl::GetCefMouseModifiers(WPARAM wparam)
{
	int modifiers = 0;
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
	return modifiers;
}

int CefControl::GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam)
{
	int modifiers = 0;
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
	return modifiers;
}

void CefControl::AdaptDpiScale(CefMouseEvent& mouse_event)
{
	if (CefManager::GetInstance()->IsEnableOffsetRender()) {
		//离屏渲染模式，需要传给原始宽度和高度，因为CEF内部会进一步做DPI自适应
		uint32_t dpiScale = ui::GlobalManager::Instance().Dpi().GetScale();
		if (dpiScale > 100) {
			mouse_event.x = mouse_event.x * 100 / dpiScale;
			mouse_event.y = mouse_event.y * 100 / dpiScale;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////

}