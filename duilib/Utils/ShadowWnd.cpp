#include "ShadowWnd.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Shadow.h"

namespace ui {

/**@brief 一个附加到基础窗口周围的窗口，带有 WS_EX_LAYERED 属性来实现阴影
	* @copyright (c) 2016, NetEase Inc. All rights reserved
	* @date 2019-03-22
	*/
class ShadowWndBase : public ui::WindowImplBase, public ui::IUIMessageFilter
{
public:
	ShadowWndBase();

	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;

	virtual LRESULT FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
	bool Create(Window* window);
private:
	Window* m_pWindow;
	//标记被跟随的窗口是否完成首次绘制
	bool m_isFirstPainted;
};

ShadowWndBase::ShadowWndBase():
	m_pWindow(nullptr),
	m_isFirstPainted(false)
{
}

std::wstring ShadowWndBase::GetSkinFolder()
{
	return L"";
}

std::wstring ShadowWndBase::GetSkinFile()
{
	return L"public/shadow/shadow.xml";
}

std::wstring ShadowWndBase::GetWindowClassName() const
{
	return L"ShadowWnd";
}

bool ShadowWndBase::Create(Window* window)
{
	m_pWindow = window;
	return Window::CreateWnd(NULL, L"ShadowWnd", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
}

LRESULT ShadowWndBase::FilterMessage(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	if ((m_pWindow == nullptr) || (m_pWindow->GetHWND() == nullptr)) {
		return 0;
	}
	HWND hWnd = m_pWindow->GetHWND();
	switch (uMsg)
	{
		case WM_ERASEBKGND:
		case WM_PAINT:
		case WM_MOVE:
		case WM_SIZE:
		case WM_SIZING:
		case WM_WINDOWPOSCHANGING:
		case WM_WINDOWPOSCHANGED:
		case WM_ACTIVATE:
		case WM_NCACTIVATE:
			if (m_isFirstPainted || (uMsg == WM_PAINT)) {
				if (::IsWindowVisible(hWnd)) {
					RECT rc = { 0 };
					::GetWindowRect(hWnd, &rc);
					SetWindowPos(ui::UiRect(rc.left, rc.top, rc.right, rc.bottom), false, SWP_SHOWWINDOW | SWP_NOACTIVATE, hWnd);
					m_isFirstPainted = true;
				}
			}			
			break;
		case WM_CLOSE:
			ShowWindow(false, false);
			break;
		case WM_SHOWWINDOW:
			ShowWindow(wParam == 0 ? false : true, false);
			break;
		default:
			break;
	}
	return 0;
}

ShadowWnd::ShadowWnd():
	m_pShadowWnd(nullptr)
{
}

bool ShadowWnd::CreateWnd(HWND hwndParent, const wchar_t* windowName, uint32_t dwStyle, uint32_t dwExStyle, const ui::UiRect& rc )
{
	ASSERT((m_pShadowWnd == nullptr) && (GetHWND() == nullptr));
	if ((m_pShadowWnd != nullptr) || (GetHWND() != nullptr)){
		return false;
	}
	//取消层窗口属性
	dwExStyle &= ~WS_EX_LAYERED;
	__super::CreateWnd(hwndParent, windowName, dwStyle, dwExStyle, rc);

	bool needCreateShadowWnd = NeedCreateShadowWnd();
	if (!needCreateShadowWnd) {
		//配置文件，设置了层窗口，或者关闭阴影，不再附加透明阴影		
		return GetHWND() != nullptr;
	}
	//关闭默认的窗口阴影
	if (IsShadowAttached()) {
		if (IsUseDefaultShadowAttached()) {
			SetShadowAttached(false);
			SetUseDefaultShadowAttached(true);
		}
		else {
			SetShadowAttached(false);
		}
	}	
	 
	//通过XML配置文件<Window>如下配置时，开启非透明窗口的阴影：
	//   layered_window = "false" shadow_attached = "true";
	if (m_pShadowWnd != nullptr) {
		m_pShadowWnd->Create(this);
		::EnableWindow(m_pShadowWnd->GetHWND(), FALSE);
		if (::IsWindowVisible(GetHWND())) {
			m_pShadowWnd->ShowWindow();
		}
	}
	return GetHWND() != nullptr;
}

Box* ShadowWnd::AttachShadow(Box* pRoot)
{
	ASSERT(m_pShadowWnd == nullptr);
	if (m_pShadowWnd != nullptr) {
		return __super::AttachShadow(pRoot);
	}

	bool needCreateShadowWnd = NeedCreateShadowWnd();
	if (!needCreateShadowWnd) {
		//不满足附加透明窗口阴影条件，走原来逻辑
		return __super::AttachShadow(pRoot);
	}
	else {
		//不附加默认的阴影，而是创建自己的透明窗口，作为阴影
		m_pShadowWnd = new ShadowWndBase;
		AddMessageFilter(m_pShadowWnd);

		//外置的阴影窗口需要将原窗口设置为圆角，避免圆角处出现黑色背景
		UiSize borderRound = Shadow::GetChildBoxBorderRound();
		SetRoundCorner(borderRound.cx, borderRound.cy);
		pRoot->SetBorderRound(borderRound);
		return pRoot;
	}
}

bool ShadowWnd::NeedCreateShadowWnd() const
{
	if (IsLayeredWindow()) {
		//设置了层窗口属性，不创建
		return false;
	}
	if (IsUseDefaultShadowAttached()) {
		//配置文件中，或者外部接口中，未设置ShadowAttached属性，创建
		return true;
	}
	else {
		//配置文件中有设置，以配置文件中的属性为准
		return IsShadowAttached();
	}	
}

} // namespace ui
