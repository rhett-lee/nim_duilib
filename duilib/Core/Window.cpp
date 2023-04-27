#include "Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Utils/VersionHelpers.h"
#include "duilib/Utils/TimerManager.h"
#include "duilib/Utils/ApiWrapper.h"
#include "duilib/Utils/DpiManager.h"
#include "duilib/Utils/Shadow.h"
#include "duilib/Utils/MultiLangSupport.h"

#include <tchar.h>
#include <Olectl.h>

namespace ui 
{
Window::Window() :
	m_hWnd(nullptr),
	m_pRoot(nullptr),
	m_OnEvent(),
	m_OldWndProc(::DefWindowProc),
	m_bSubclassed(false),
	m_nAlpha(255),
	m_renderOffset(),
	m_hDcPaint(nullptr),
	m_hwndTooltip(nullptr),
	m_ToolTip(),
	m_pFocus(nullptr),
	m_pEventHover(nullptr),
	m_pEventClick(nullptr),
	m_pEventKey(nullptr),
	m_ptLastMousePos(-1, -1),
	m_pEventPointer(nullptr),
	m_bHandlePointerMsg(true),
	m_szMinWindow(),
	m_szMaxWindow(),
	m_szInitWindowSize(),
	m_rcMaximizeInfo(0, 0, 0, 0),
	m_rcSizeBox(),
	m_rcAlphaFix(0, 0, 0, 0),
	m_szRoundCorner(),
	m_rcCaption(),
	m_bFirstLayout(true),
	m_bIsArranged(false),
	m_bMouseTracking(false),
	m_bMouseCapture(false),
	m_bIsLayeredWindow(true),
	m_aMessageFilters(),
	m_aDelayedCleanup(),
	m_mOptionGroup(),
	m_defaultFontInfo(),
	m_defaultAttrHash(),
	m_strResourcePath(),
	m_closeFlag()
{
	m_shadow = std::make_unique<Shadow>();
	LOGFONT lf = { 0 };
	::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	lf.lfCharSet = DEFAULT_CHARSET;
	if (GlobalManager::GetDefaultFontName().length() > 0) {
		_tcscpy_s(lf.lfFaceName, LF_FACESIZE, GlobalManager::GetDefaultFontName().c_str());
	}

	HFONT hDefaultFont = ::CreateFontIndirect(&lf);
	m_defaultFontInfo.hFont = hDefaultFont;
	m_defaultFontInfo.sFontName = lf.lfFaceName;
	m_defaultFontInfo.iSize = -lf.lfHeight;
	m_defaultFontInfo.iWeight = FW_NORMAL;
	m_defaultFontInfo.bBold = (lf.lfWeight >= FW_BOLD);
	m_defaultFontInfo.bUnderline = (lf.lfUnderline == TRUE);
	m_defaultFontInfo.bStrikeout = (lf.lfStrikeOut == TRUE);
	m_defaultFontInfo.bItalic = (lf.lfItalic == TRUE);
	::ZeroMemory(&m_defaultFontInfo.tm, sizeof(m_defaultFontInfo.tm));
#if defined(ENABLE_UIAUTOMATION)
	m_pUIAProvider = nullptr;
#endif
}

Window::~Window()
{
	// Delete the control-tree structures
	for (auto it = m_aDelayedCleanup.begin(); it != m_aDelayedCleanup.end(); ++it) {
		delete* it;
	}
	m_aDelayedCleanup.clear();

	if (m_pRoot != nullptr) {
		delete m_pRoot;
		m_pRoot = nullptr;
	}
	
	if (m_defaultFontInfo.hFont != nullptr) {
		::DeleteObject(m_defaultFontInfo.hFont);
		m_defaultFontInfo.hFont = nullptr;
	}
	
	RemoveAllClass();
	RemoveAllOptionGroups();

	// Reset other parts...
	if (m_hwndTooltip != nullptr) {
		::DestroyWindow(m_hwndTooltip);
		m_hwndTooltip = nullptr;
	}
	if (m_hDcPaint != nullptr) {
		::ReleaseDC(m_hWnd, m_hDcPaint);
		m_hDcPaint = nullptr;
	}
}

HWND Window::GetHWND() const 
{ 
    return m_hWnd; 
}

bool Window::RegisterWindowClass()
{
	WNDCLASS wc = { 0 };
	wc.style = GetClassStyle();
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = NULL;
	wc.lpfnWndProc = Window::__WndProc;
	wc.hInstance = GetResModuleHandle();
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	std::wstring className = GetWindowClassName();
	wc.lpszClassName = className.c_str();
	ATOM ret = ::RegisterClass(&wc);
	ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool Window::RegisterSuperClass()
{
	// Get the class information from an existing
	// window so we can subclass it later on...
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	std::wstring superClassName = GetSuperClassName();
	if (!::GetClassInfoEx(NULL, superClassName.c_str(), &wc)) {
		if (!::GetClassInfoEx(GetResModuleHandle(), superClassName.c_str(), &wc)) {
			ASSERT(!"Unable to locate window class");
			return false;
		}
	}
	m_OldWndProc = wc.lpfnWndProc;
	wc.lpfnWndProc = Window::__ControlProc;
	wc.hInstance = GetResModuleHandle();
	std::wstring className = GetWindowClassName();
	wc.lpszClassName = className.c_str();
	ATOM ret = ::RegisterClassEx(&wc);
	ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}
std::wstring Window::GetWindowClassName() const
{
	ASSERT(FALSE);
	return std::wstring();
}

std::wstring Window::GetSuperClassName() const
{
    return std::wstring();
}

UINT Window::GetClassStyle() const
{
	return CS_DBLCLKS ;
}

UINT Window::GetStyle() const
{
	//从原来窗口样式中，移除 WS_CAPTION 属性
	ASSERT(::IsWindow(GetHWND()));
	UINT styleValue = (UINT)::GetWindowLong(GetHWND(), GWL_STYLE);
	styleValue &= ~WS_CAPTION;
	return styleValue;
}

HMODULE Window::GetResModuleHandle() const
{
	return ::GetModuleHandle(NULL);
}

HWND Window::Subclass(HWND hWnd)
{
	ASSERT(::IsWindow(hWnd));
	ASSERT(m_hWnd == nullptr);
	m_OldWndProc = SubclassWindow(hWnd, __WndProc);
	if (m_OldWndProc == nullptr){
	    return nullptr;
	}
	m_bSubclassed = true;
	m_hWnd = hWnd;
	::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
	return m_hWnd;
}

void Window::Unsubclass()
{
	ASSERT(::IsWindow(m_hWnd));
	if (!::IsWindow(m_hWnd)) {
		return;
	}
	if (m_bSubclassed) {
		SubclassWindow(m_hWnd, m_OldWndProc);
		m_OldWndProc = ::DefWindowProc;
		m_bSubclassed = false;
	}	
}

HWND Window::CreateWnd(HWND hwndParent, const wchar_t* windowName, uint32_t dwStyle, uint32_t dwExStyle, bool isLayeredWindow, const UiRect& rc)
{
	if (!GetSuperClassName().empty()){
		if (!RegisterSuperClass()) {
			return nullptr;
		}
	}
	else {
		if (!RegisterWindowClass()) {
			return nullptr;
		}		
	}
	std::wstring className = GetWindowClassName();
	m_bIsLayeredWindow = isLayeredWindow;
	if (m_bIsLayeredWindow) {
		dwExStyle |= WS_EX_LAYERED;
	}

    HWND hWnd = ::CreateWindowEx(dwExStyle, 
							     className.c_str(), 
							     windowName, 
							     dwStyle,
							     rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), 
							     hwndParent, NULL, GetResModuleHandle(), this);
	ASSERT(::IsWindow(hWnd));
	if (hWnd != m_hWnd) {
		m_hWnd = hWnd;
	}
    return hWnd;
}

void Window::CloseWnd(UINT nRet)
{
	if (m_bFakeModal)
	{
		auto parent_hwnd = GetWindowOwner(m_hWnd);
		ASSERT(::IsWindow(parent_hwnd));
		::EnableWindow(parent_hwnd, TRUE);
		::SetFocus(parent_hwnd);
		m_bFakeModal = false;
	}
	ASSERT(::IsWindow(m_hWnd));
	if (!::IsWindow(m_hWnd)) {
		return;
	}

	if (m_pRoot && IsWindowsVistaOrGreater()) {
		m_pRoot->SetFadeVisible(false);
		auto closeCallback = [this, nRet]() {
			this->PostMessage(WM_CLOSE, (WPARAM)nRet, 0L);
		};
		TimerManager::GetInstance()->AddCancelableTimer(m_closeFlag.GetWeakFlag(), closeCallback, 300, 1);
	}
	else {
		PostMessage(WM_CLOSE, (WPARAM)nRet, 0L);
	}
	m_bCloseing = true;
}

void Window::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
    ASSERT(::IsWindow(m_hWnd));
    ::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
}

void Window::ShowModalFake(HWND parent_hwnd)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(::IsWindow(parent_hwnd));
	auto p_hwnd = GetWindowOwner(m_hWnd);
	ASSERT(::IsWindow(p_hwnd));
	ASSERT_UNUSED_VARIABLE(p_hwnd == parent_hwnd);
	::EnableWindow(parent_hwnd, FALSE);
	ShowWindow();
	m_bFakeModal = true;
}

bool Window::IsFakeModal() const
{
	return m_bFakeModal;
}

void Window::CenterWindow()
{
    ASSERT(::IsWindow(m_hWnd));
    ASSERT((GetWindowStyle(m_hWnd) & WS_CHILD) == 0);
    UiRect rcDlg;
    ::GetWindowRect(m_hWnd, &rcDlg);
    UiRect rcArea;
    UiRect rcCenter;
	HWND hWnd = GetHWND();
    HWND hWndCenter = ::GetWindowOwner(m_hWnd);
	if (hWndCenter != nullptr) {
		hWnd = hWndCenter;
	}		

	// 处理多显示器模式下屏幕居中
	MONITORINFO oMonitor = {0,};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
	rcArea = UiRect(oMonitor.rcWork);

	if (hWndCenter == nullptr) {
		rcCenter = rcArea;
	}
	else if (::IsIconic(hWndCenter)) {
		rcCenter = rcArea;
	}
	else {
		::GetWindowRect(hWndCenter, &rcCenter);
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

void Window::ToTopMost()
{
	ASSERT(::IsWindow(m_hWnd));
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void Window::BringToTop()
{
	ASSERT(::IsWindow(m_hWnd));
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void Window::ActiveWindow()
{
	ASSERT(::IsWindow(m_hWnd));
	if (::IsIconic(m_hWnd)) {
		::ShowWindow(m_hWnd, SW_RESTORE);
	}
	else {
		if (!::IsWindowVisible(m_hWnd)) {
			::ShowWindow(m_hWnd, SW_SHOW);
		}
		::SetForegroundWindow(m_hWnd);
	}
}

void Window::SetIcon(UINT nRes)
{
	ASSERT(::IsWindow(m_hWnd));
    HICON hIcon = (HICON)::LoadImage(GetResModuleHandle(), MAKEINTRESOURCE(nRes), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR | LR_SHARED);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM) TRUE, (LPARAM) hIcon);
    hIcon = (HICON)::LoadImage(GetResModuleHandle(), MAKEINTRESOURCE(nRes), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR | LR_SHARED);
    ASSERT(hIcon);
    ::SendMessage(m_hWnd, WM_SETICON, (WPARAM) FALSE, (LPARAM) hIcon);
}

LRESULT Window::SendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
}

LRESULT Window::PostMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void Window::AttachWindowClose(const EventCallback& callback)
{
	m_OnEvent[kEventWindowClose] += callback;
}

void Window::OnFinalMessage(HWND hWnd)
{
	ASSERT(m_hWnd == hWnd);
	//取消异步关闭窗口回调，避免访问非法资源
	m_closeFlag.Cancel();

	UnregisterTouchWindowWrapper(hWnd);
	SendNotify(kEventWindowClose);

	//回收控件
	ReapObjects(GetRoot());
	m_hWnd = nullptr;

#if defined(ENABLE_UIAUTOMATION)
	if (nullptr != m_pUIAProvider) {
		// Coz UiaDisconnectProviderd require at least win8
		// UiaDisconnectProvider(m_pUIAProvider);

		m_pUIAProvider->ResetWindow();
		m_pUIAProvider->Release();

		m_pUIAProvider = nullptr;
	}
#endif
}

LRESULT CALLBACK Window::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<Window*>(lpcs->lpCreateParams);
        if (pThis != nullptr) {
            pThis->m_hWnd = hWnd;
        }
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
    }
    else {
        pThis = reinterpret_cast<Window*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if( uMsg == WM_NCDESTROY && pThis != nullptr ) {
            LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
            if (pThis->m_bSubclassed) {
                pThis->Unsubclass();
            }
			ASSERT(hWnd == pThis->GetHWND());
            pThis->OnFinalMessage(hWnd);
            return lRes;
        }
    }

    if( pThis != nullptr) {
		ASSERT(hWnd == pThis->GetHWND());
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    } 
    else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK Window::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window* pThis = nullptr;
    if( uMsg == WM_NCCREATE ) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<Window*>(lpcs->lpCreateParams);        
		if (pThis != nullptr) {
			::SetProp(hWnd, _T("WndX"), (HANDLE)pThis);
			pThis->m_hWnd = hWnd;
		}        
    } 
    else {
        pThis = reinterpret_cast<Window*>(::GetProp(hWnd, _T("WndX")));
        if( uMsg == WM_NCDESTROY && pThis != nullptr ) {
            LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
			if (pThis->m_bSubclassed) {
				pThis->Unsubclass();
			}
            ::SetProp(hWnd, _T("WndX"), NULL);
			ASSERT(hWnd == pThis->GetHWND());
            pThis->OnFinalMessage(hWnd);
            return lRes;
        }
    }
    if( pThis != nullptr ) {
		ASSERT(hWnd == pThis->GetHWND());
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    } 
    else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

#if defined(ENABLE_UIAUTOMATION)
UIAWindowProvider* Window::GetUIAProvider()
{
	if (m_pUIAProvider == nullptr)
	{
		m_pUIAProvider = new (std::nothrow) UIAWindowProvider(this);
	}
	return m_pUIAProvider;
}
#endif

void Window::InitWnd(HWND hWnd)
{
	ASSERT(::IsWindow(hWnd));
	if (!::IsWindow(hWnd)) {
		return;
	}
	ASSERT((m_hWnd == nullptr) || (m_hWnd == hWnd));
	if (m_hWnd != hWnd) {
		m_hWnd = hWnd;
	}

	// Remember the window context we came from
	ASSERT(m_hDcPaint == nullptr);
	m_hDcPaint = ::GetDC(hWnd);

	ASSERT(m_renderContext == nullptr);
	m_renderContext = GlobalManager::CreateRenderContext();

	RegisterTouchWindowWrapper(hWnd, 0);
}

Box* Window::AttachShadow(Box* pRoot)
{
	return m_shadow->AttachShadow(pRoot);
}

bool Window::AttachBox(Box* pRoot)
{
	ASSERT(::IsWindow(m_hWnd));	
	SetFocus(nullptr); //设置m_pFocus相关的状态
	m_pEventKey = nullptr;
	m_pEventHover = nullptr;
	m_pEventClick = nullptr;
	m_pEventPointer = nullptr;
	// Remove the existing control-tree. We might have gotten inside this function as
	// a result of an event fired or similar, so we cannot just delete the objects and
	// pull the internal memory of the calling code. We'll delay the cleanup.
	if ((m_pRoot != nullptr) && (pRoot != m_pRoot)) {
		AddDelayedCleanup(m_pRoot);
	}
	// Set the dialog root element
	m_pRoot = pRoot;
	m_controlFinder.SetRoot(pRoot);
	// Go ahead...
	m_bIsArranged = true;
	m_bFirstLayout = true;
	// Initiate all control
	return InitControls(m_pRoot);
}

bool Window::InitControls(Control* pControl, Box* pParent /*= NULL*/)
{
	ASSERT(pControl != nullptr);
	if (pControl == nullptr) {
		return false;
	}
	pControl->SetWindow(this, pParent != nullptr ? pParent : pControl->GetParent(), true);
	pControl->FindControl(ControlFinder::__FindControlFromNameHash, &m_controlFinder, UIFIND_ALL);
	return true;
}

void Window::ReapObjects(Control* pControl)
{
	if (pControl == nullptr) {
		return;
	}
	if (pControl == m_pEventKey) {
		m_pEventKey = nullptr;
	}
	if (pControl == m_pEventHover) {
		m_pEventHover = nullptr;
	}
	if (pControl == m_pEventClick) {
		m_pEventClick = nullptr;
	}
	if (pControl == m_pEventPointer) {
		m_pEventPointer = nullptr;
	}
	if (pControl == m_pFocus) {
		m_pFocus = nullptr;
	}
	m_controlFinder.ReapObjects(pControl);
}

const std::wstring& Window::GetResourcePath() const
{
	return m_strResourcePath;
}

void Window::SetResourcePath(const std::wstring& strPath)
{
	m_strResourcePath = strPath;
	if (!m_strResourcePath.empty()) {
		//确保路径最后字符是分割字符
		TCHAR cEnd = m_strResourcePath.back();
		if (cEnd != _T('\\') && cEnd != _T('/')) {
			m_strResourcePath += _T('\\');
		}
	}	
}

TFontInfo* Window::GetDefaultFontInfo()
{
	ASSERT(m_hDcPaint != nullptr);
	if (m_defaultFontInfo.tm.tmHeight == 0) {
		HFONT hOldFont = (HFONT) ::SelectObject(m_hDcPaint, m_defaultFontInfo.hFont);
		::GetTextMetrics(m_hDcPaint, &m_defaultFontInfo.tm);
		::SelectObject(m_hDcPaint, hOldFont);
	}
	return &m_defaultFontInfo;
}

void Window::AddClass(const std::wstring& strClassName, const std::wstring& strControlAttrList)
{
	ASSERT(!strClassName.empty());
	ASSERT(!strControlAttrList.empty());
	m_defaultAttrHash[strClassName] = strControlAttrList;
}

std::wstring Window::GetClassAttributes(const std::wstring& strClassName) const
{
	auto it = m_defaultAttrHash.find(strClassName);
	if (it != m_defaultAttrHash.end()) {
		return it->second;
	}
	return L"";
}

bool Window::RemoveClass(const std::wstring& strClassName)
{
	auto it = m_defaultAttrHash.find(strClassName);
	if (it != m_defaultAttrHash.end()) {
		m_defaultAttrHash.erase(it);
		return true;
	}
	return false;
}

void Window::RemoveAllClass()
{
	m_defaultAttrHash.clear();
}

void Window::AddTextColor(const std::wstring& strName, const std::wstring& strValue)
{
	std::wstring strColor;
	if (strValue.size() > 1) {
		strColor = strValue.substr(1);
	}
	LPTSTR pstr = nullptr;
	DWORD dwBackColor = _tcstoul(strColor.c_str(), &pstr, 16);

	m_mapTextColor[strName] = dwBackColor;
}

void Window::AddTextColor(const std::wstring& strName, DWORD argb)
{
	ASSERT(!strName.empty());
	m_mapTextColor[strName] = argb;
}

DWORD Window::GetTextColor(const std::wstring& strName) const
{
	auto it = m_mapTextColor.find(strName);
	if (it != m_mapTextColor.end()) {
		return it->second;
	}
	return 0;
}

bool Window::AddOptionGroup(const std::wstring& strGroupName, Control* pControl)
{
	ASSERT(!strGroupName.empty());
	ASSERT(pControl != nullptr);
	if ((pControl == nullptr) || strGroupName.empty()) {
		return false;
	}
	auto it = m_mOptionGroup.find(strGroupName);
	if (it != m_mOptionGroup.end()) {
		auto it2 = std::find(it->second.begin(), it->second.end(), pControl);
		if (it2 != it->second.end()){
			return false;
		}
		it->second.push_back(pControl);
	}
	else {
		m_mOptionGroup[strGroupName].push_back(pControl);
	}
	return true;
}

std::vector<Control*>* Window::GetOptionGroup(const std::wstring& strGroupName)
{
	auto it = m_mOptionGroup.find(strGroupName);
	if (it != m_mOptionGroup.end()) {
		return &(it->second);
	}
	return nullptr;
}

void Window::RemoveOptionGroup(const std::wstring& strGroupName, Control* pControl)
{
	ASSERT(!strGroupName.empty());
	ASSERT(pControl != nullptr);
	auto it = m_mOptionGroup.find(strGroupName);
	if (it != m_mOptionGroup.end()) {
		auto it2 = std::find(it->second.begin(), it->second.end(), pControl);
		if (it2 != it->second.end()) {
			it->second.erase(it2);
		}

		if (it->second.empty()) {
			m_mOptionGroup.erase(it);
		}
	}
}

void Window::RemoveAllOptionGroups()
{
	m_mOptionGroup.clear();
}

void Window::ClearImageCache()
{
	Control *pRoot = m_shadow->GetRoot();
	if (pRoot) {
		pRoot->ClearImageCache();
	}
	else if(m_pRoot != nullptr){
		m_pRoot->ClearImageCache();
	}
}

const POINT& Window::GetLastMousePos() const
{
	return m_ptLastMousePos;
}

const UiRect& Window::GetSizeBox() const
{
	return m_rcSizeBox;
}

void Window::SetSizeBox(const UiRect& rcSizeBox)
{
	m_rcSizeBox = rcSizeBox;
}

const UiRect& Window::GetCaptionRect() const
{
	return m_rcCaption;
}

void Window::SetCaptionRect(const UiRect& rcCaption)
{
	m_rcCaption = rcCaption;
	DpiManager::GetInstance()->ScaleRect(m_rcCaption);
}

const CSize& Window::GetRoundCorner() const
{
	return m_szRoundCorner;
}

void Window::SetRoundCorner(int cx, int cy)
{
	DpiManager::GetInstance()->ScaleInt(cx);
	DpiManager::GetInstance()->ScaleInt(cy);
	m_szRoundCorner.cx = cx;
	m_szRoundCorner.cy = cy;
}

const UiRect& Window::GetMaximizeInfo() const
{
	return m_rcMaximizeInfo;
}

void Window::SetMaximizeInfo(const UiRect& rcMaximize)
{
	m_rcMaximizeInfo = rcMaximize;
	DpiManager::GetInstance()->ScaleRect(m_rcMaximizeInfo);
}

const UiRect& Window::GetAlphaFixCorner() const
{
	return m_rcAlphaFix;
}

void Window::SetAlphaFixCorner(const UiRect& rc)
{
	m_rcAlphaFix = rc;
	DpiManager::GetInstance()->ScaleRect(m_rcAlphaFix);
}

void Window::SetText(const std::wstring& strText)
{
	ASSERT(::IsWindow(m_hWnd));
	::SetWindowText(m_hWnd, strText.c_str());
}

void Window::SetTextId(const std::wstring& strTextId)
{
	ASSERT(::IsWindow(m_hWnd));
	::SetWindowText(m_hWnd, MultiLangSupport::GetInstance()->GetStringViaID(strTextId).c_str());
}

void Window::SetShadowAttached(bool bShadowAttached)
{
	m_shadow->SetShadowAttached(bShadowAttached);
}

const std::wstring& Window::GetShadowImage() const
{
	return m_shadow->GetShadowImage();
}

void Window::SetShadowImage(const std::wstring &strImage)
{
	m_shadow->SetShadowImage(strImage);
}

UiRect Window::GetShadowCorner() const
{
	return m_shadow->GetShadowCorner();
}

bool Window::IsShadowAttached() const
{
    return m_shadow->IsShadowAttached();
}

void Window::SetShadowCorner(const UiRect& rect, bool bNeedDpiScale)
{
	m_shadow->SetShadowCorner(rect, bNeedDpiScale);
}

UiRect Window::GetPos(bool bContainShadow) const
{
	ASSERT(::IsWindow(m_hWnd));
	UiRect rcPos;
	::GetWindowRect(m_hWnd, &rcPos);

	if (!bContainShadow) {
		UiRect padding = m_shadow->GetShadowCorner();
		rcPos.left += padding.left;
		rcPos.right -= padding.right;
		rcPos.top += padding.top;
		rcPos.bottom -= padding.bottom;
	}
	return rcPos;
}

void Window::SetPos(const UiRect& rc, bool bNeedDpiScale, UINT uFlags, HWND hWndInsertAfter, bool bContainShadow)
{
	UiRect rcNewPos = rc;
	if (bNeedDpiScale) {
		DpiManager::GetInstance()->ScaleRect(rcNewPos);
	}

	ASSERT(::IsWindow(m_hWnd));
	if (!bContainShadow) {
		rcNewPos.Inflate(m_shadow->GetShadowCorner());
	}
	::SetWindowPos(m_hWnd, hWndInsertAfter, rcNewPos.left, rcNewPos.top, rcNewPos.GetWidth(), rcNewPos.GetHeight(), uFlags);
}

CSize Window::GetMinInfo(bool bContainShadow) const
{
	CSize xy = m_szMinWindow;
	if (!bContainShadow) {
		UiRect rcShadow = m_shadow->GetShadowCorner();
		if (xy.cx != 0) {
			xy.cx -= rcShadow.left + rcShadow.right;
		}
		if (xy.cy != 0) {
			xy.cy -= rcShadow.top + rcShadow.bottom;
		}
	}

	return xy;
}

void Window::SetMinInfo(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		DpiManager::GetInstance()->ScaleInt(cx);
		DpiManager::GetInstance()->ScaleInt(cy);
	}
	ASSERT(cx >= 0 && cy >= 0);

	if (!bContainShadow) {
		UiRect rcShadow = m_shadow->GetShadowCorner();
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

CSize Window::GetMaxInfo(bool bContainShadow) const
{
	CSize xy = m_szMaxWindow;
	if (!bContainShadow) {
		UiRect rcShadow = m_shadow->GetShadowCorner();
		if (xy.cx != 0) {
			xy.cx -= rcShadow.left + rcShadow.right;
		}
		if (xy.cy != 0) {
			xy.cy -= rcShadow.top + rcShadow.bottom;
		}
	}

	return xy;
}

void Window::SetMaxInfo(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		DpiManager::GetInstance()->ScaleInt(cx);
		DpiManager::GetInstance()->ScaleInt(cy);
	}
	ASSERT(cx >= 0 && cy >= 0);

	if (!bContainShadow) {
		UiRect rcShadow = m_shadow->GetShadowCorner();
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

CSize Window::GetInitSize(bool bContainShadow) const
{
	CSize xy = m_szInitWindowSize;
	if (!bContainShadow) {
		UiRect rcShadow = m_shadow->GetShadowCorner();
		if (xy.cx != 0) {
			xy.cx -= rcShadow.left + rcShadow.right;
		}
		if (xy.cy != 0) {
			xy.cy -= rcShadow.top + rcShadow.bottom;
		}
	}

	return xy;
}

void Window::Resize(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		DpiManager::GetInstance()->ScaleInt(cy);
		DpiManager::GetInstance()->ScaleInt(cx);
	}

	if (!bContainShadow) {
		UiRect rcShadow = m_shadow->GetShadowCorner();
		cx += rcShadow.left + rcShadow.right;
		cy += rcShadow.top + rcShadow.bottom;
	}
	m_szInitWindowSize.cx = cx;
	m_szInitWindowSize.cy = cy;
	ASSERT(::IsWindow(m_hWnd));
	::SetWindowPos(m_hWnd, NULL, 0, 0, m_szInitWindowSize.cx, m_szInitWindowSize.cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

void Window::SetInitSize(int cx, int cy, bool bContainShadow, bool bNeedDpiScale)
{
	if(m_pRoot == nullptr) {
		Resize(cx, cy, bContainShadow, bNeedDpiScale);
	}
}

bool Window::AddMessageFilter(IUIMessageFilter* pFilter)
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
bool Window::RemoveMessageFilter(IUIMessageFilter* pFilter)
{
	auto iter = std::find(m_aMessageFilters.begin(), m_aMessageFilters.end(), pFilter);
	if (iter != m_aMessageFilters.end()) {
		m_aMessageFilters.erase(iter);
		return true;
	}
	return false;
}

LRESULT Window::WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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

	//第三优先级：内部处理函数，优先保证自身功能正常
	if (!bHandled) {
		lResult = HandleMessage(uMsg, wParam, lParam, bHandled);
	}

	//第四优先级：系统默认的窗口函数
	if (!bHandled) {		
		lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
	}
	return lResult;
}

LRESULT Window::OnWindowMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT Window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	LRESULT lResult = 0;
	bHandled = false;
    // Custom handling of events
    switch (uMsg) 
	{
		case WM_APP + 1:
		{
			for (Control* pControl : m_aDelayedCleanup) {
				delete pControl;
			}
			m_aDelayedCleanup.clear();
			bHandled = true;
		}
		break;
		
		case WM_CLOSE:				lResult = OnCloseMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_NCACTIVATE:			lResult = OnNcActivateMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:			lResult = OnNcCalcSizeMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:			lResult = OnNcHitTestMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_GETMINMAXINFO:		lResult = OnGetMinMaxInfoMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_WINDOWPOSCHANGING:	lResult = OnWindowPosChangingMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:				lResult = OnSizeMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_ERASEBKGND:			lResult = OnEraseBkGndMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_PAINT:				lResult = OnPaintMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEHOVER:			lResult = OnMouseHoverMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSELEAVE:			lResult = OnMouseLeaveMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEMOVE:			lResult = OnMouseMoveMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEWHEEL:			lResult = OnMouseWheelMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONDOWN:		lResult = OnLButtonDownMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_RBUTTONDOWN:		lResult = OnRButtonDownMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONDBLCLK:		lResult = OnLButtonDoubleClickMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONUP:			lResult = OnLButtonUpMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_RBUTTONUP:			lResult = OnRButtonUpMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_IME_STARTCOMPOSITION: lResult = OnIMEStartCompositionMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_IME_ENDCOMPOSITION:	lResult = OnIMEEndCompositionMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_SETFOCUS:			lResult = OnSetFocusMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_KILLFOCUS:			lResult = OnKillFocusMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_CONTEXTMENU:		lResult = OnContextMenuMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_CHAR:				lResult = OnCharMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:			lResult = OnKeyDownMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYUP:				lResult = OnKeyUpMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_SETCURSOR:			lResult = OnSetCusorMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_NOTIFY:				lResult = OnNotifyMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_COMMAND:			lResult = OnCommandMsg(uMsg, wParam, lParam, bHandled); break;
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
			lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled);
			break;
		case WM_TOUCH:				
			lResult = OnTouchMsg(uMsg, wParam, lParam, bHandled); 
			break;		
		case WM_POINTERDOWN:
		case WM_POINTERUP:
		case WM_POINTERUPDATE:
		case WM_POINTERLEAVE:
		case WM_POINTERCAPTURECHANGED:
			lResult = OnPointerMsgs(uMsg, wParam, lParam, bHandled); 
			break;		
		
#if defined(ENABLE_UIAUTOMATION)
		case WM_GETOBJECT:
		{
			if (static_cast<long>(lParam) == static_cast<long>(UiaRootObjectId) && GlobalManager::IsAutomationEnabled()) {
				bHandled = TRUE;
				return UiaReturnRawElementProvider(m_hWnd, wParam, lParam, GetUIAProvider());
			}
		}
		break;
#endif

		default:
			break;
    }//end of switch
    return lResult;
}

LRESULT Window::CallDefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_OldWndProc != nullptr);
	return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
}

LRESULT Window::OnCloseMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_CLOSE);
	bHandled = false;
	// Make sure all matching "closing" events are sent
	if (m_pEventHover != nullptr) {
		m_pEventHover->HandleMessageTemplate(kEventMouseLeave);
	}
	if (m_pEventClick != nullptr) {
		m_pEventClick->HandleMessageTemplate(kEventMouseButtonUp);
	}
	if (m_pEventPointer != nullptr) {
		m_pEventPointer->HandleMessageTemplate(kEventMouseButtonUp);
	}
	SetFocus(nullptr);

	// Hmmph, the usual Windows tricks to avoid
	// focus loss...
	if (::GetFocus() == m_hWnd) {
		HWND hwndParent = ::GetWindowOwner(m_hWnd);
		if (hwndParent != nullptr) {
			::SetFocus(hwndParent);
		}
	}
	return 0;
}

LRESULT Window::OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_NCACTIVATE);
	LRESULT lResult = 0;
	if (::IsIconic(GetHWND())) {
		bHandled = false;
	}
	else {
		//MSDN: wParam 参数为 FALSE 时，应用程序应返回 TRUE 以指示系统应继续执行默认处理
		bHandled = true;
		lResult = (wParam == FALSE) ? TRUE : FALSE;
	}
	return lResult;
}

LRESULT Window::OnNcCalcSizeMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_NCCALCSIZE);
	//截获，让系统不处理此消息
	bHandled = true;
	return 0;
}

LRESULT Window::OnNcHitTestMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_NCHITTEST);
	bHandled = true;
	POINT pt = { 0 };
	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(GetHWND(), &pt);

	UiRect rcClient;
	::GetClientRect(GetHWND(), &rcClient);

	//客户区域，排除掉阴影部分区域
	rcClient.Deflate(m_shadow->GetShadowCorner());

	if (!::IsZoomed(GetHWND())) {
		//非最小化状态
		UiRect rcSizeBox = GetSizeBox();
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

	UiRect rcCaption = GetCaptionRect();
	if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
		&& pt.y >= rcClient.top + rcCaption.top && pt.y < rcClient.top + rcCaption.bottom) {
		Control* pControl = FindControl(pt);
		if (pControl) {
			if (pControl->CanPlaceCaptionBar()) {
				return HTCLIENT;//在工作区中（放在标题栏上的控件，视为工作区）
			}
			else {
				return HTCAPTION;//在标题栏中
			}
		}
	}
	//其他，在工作区中
	return HTCLIENT;
}

LRESULT Window::OnGetMinMaxInfoMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_GETMINMAXINFO);
	bHandled = false;
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTONEAREST), &oMonitor);
	UiRect rcWork(oMonitor.rcWork);
	UiRect rcMonitor(oMonitor.rcMonitor);
	rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);

	UiRect rcMaximize = GetMaximizeInfo();
	if (rcMaximize.GetWidth() > 0 && rcMaximize.GetHeight() > 0) {
		lpMMI->ptMaxPosition.x = rcWork.left + rcMaximize.left;
		lpMMI->ptMaxPosition.y = rcWork.top + rcMaximize.top;
		lpMMI->ptMaxSize.x = rcMaximize.GetWidth();
		lpMMI->ptMaxSize.y = rcMaximize.GetHeight();
	}
	else {
		// 计算最大化时，正确的原点坐标
		lpMMI->ptMaxPosition.x = rcWork.left;
		lpMMI->ptMaxPosition.y = rcWork.top;
		lpMMI->ptMaxSize.x = rcWork.GetWidth();
		lpMMI->ptMaxSize.y = rcWork.GetHeight();
	}

	if (GetMaxInfo().cx != 0) {
		lpMMI->ptMaxTrackSize.x = GetMaxInfo(true).cx;
	}
	if (GetMaxInfo().cy != 0) {
		lpMMI->ptMaxTrackSize.y = GetMaxInfo(true).cy;
	}
	if (GetMinInfo().cx != 0) {
		lpMMI->ptMinTrackSize.x = GetMinInfo(true).cx;
	}
	if (GetMinInfo().cy != 0) {
		lpMMI->ptMinTrackSize.y = GetMinInfo(true).cy;
	}
	return 0;
}

LRESULT Window::OnWindowPosChangingMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_WINDOWPOSCHANGING);
	bHandled = false;
	if (IsZoomed(m_hWnd)) {
		LPWINDOWPOS lpPos = (LPWINDOWPOS)lParam;
		if (lpPos->flags & SWP_FRAMECHANGED) // 第一次最大化，而不是最大化之后所触发的WINDOWPOSCHANGE
		{
			POINT pt = { 0, 0 };
			HMONITOR hMontorPrimary = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
			HMONITOR hMonitorTo = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);

			// 先把窗口最大化，再最小化，然后恢复，此时MonitorFromWindow拿到的HMONITOR不准确
			// 判断GetWindowRect的位置如果不正确（最小化时得到的位置信息是-38000），则改用normal状态下的位置，来获取HMONITOR
			RECT rc = { 0 };
			GetWindowRect(m_hWnd, &rc);
			if (rc.left < -10000 && rc.top < -10000 && rc.bottom < -10000 && rc.right < -10000) {
				WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
				GetWindowPlacement(m_hWnd, &wp);
				hMonitorTo = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTOPRIMARY);
			}
			if (hMonitorTo != hMontorPrimary) {
				// 解决无边框窗口在双屏下面（副屏分辨率大于主屏）时，最大化不正确的问题
				MONITORINFO  miTo = { sizeof(miTo), 0 };
				GetMonitorInfo(hMonitorTo, &miTo);

				lpPos->x = miTo.rcWork.left;
				lpPos->y = miTo.rcWork.top;
				lpPos->cx = miTo.rcWork.right - miTo.rcWork.left;
				lpPos->cy = miTo.rcWork.bottom - miTo.rcWork.top;
			}
		}
	}
	return 0;
}

LRESULT Window::OnSizeMsg(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_SIZE);
	bHandled = false;
	CSize szRoundCorner = GetRoundCorner();
	if (!::IsIconic(GetHWND()) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
		UiRect rcWnd;
		::GetWindowRect(GetHWND(), &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; 
		rcWnd.bottom++;
		HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(GetHWND(), hRgn, TRUE);
		::DeleteObject(hRgn);
	}
	if (m_pFocus != nullptr) {
		m_pFocus->HandleMessageTemplate(kEventWindowSize);
	}
	if (m_pRoot != nullptr) {
		m_pRoot->Arrange();
	}
	if (wParam == SIZE_MAXIMIZED) {
		m_shadow->MaximizedOrRestored(true);
	}
	else if (wParam == SIZE_RESTORED) {
		m_shadow->MaximizedOrRestored(false);
	}
	return 0;
}

LRESULT Window::OnEraseBkGndMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_ERASEBKGND);
	bHandled = true;
	return 1;
}

LRESULT Window::OnPaintMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_PAINT);
	bHandled = true;
	Paint();	
	return 0;
}

LRESULT Window::OnMouseHoverMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_MOUSEHOVER);
	bHandled = false;
	m_bMouseTracking = false;
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	Control* pHover = FindControl(pt);
	if (pHover == nullptr) {
		return 0;
	}
	// Generate mouse hover event
	if (m_pEventHover != nullptr) {
		m_pEventHover->HandleMessageTemplate(kEventMouseHover, 0, 0, 0, CPoint(pt));
	}
	// Create tooltip information
	std::wstring sToolTip = pHover->GetToolTipText();
	if (sToolTip.empty()) {
		return 0;
	}

	if (m_hwndTooltip != nullptr && IsWindowVisible(m_hwndTooltip)) {
		TOOLINFO toolTip = { 0 };
		toolTip.cbSize = sizeof(TOOLINFO);
		toolTip.hwnd = m_hWnd;
		toolTip.uId = (UINT_PTR)m_hWnd;
		std::wstring toolTipText;
		toolTipText.resize(MAX_PATH);
		toolTip.lpszText = const_cast<LPTSTR>((LPCTSTR)toolTipText.c_str());
		::SendMessage(m_hwndTooltip, TTM_GETTOOLINFO, 0, (LPARAM)&toolTip);
		if (pHover == m_pEventHover && sToolTip == std::wstring(toolTipText.c_str())) {
			return 0;
		}
	}

	::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
	m_ToolTip.cbSize = sizeof(TOOLINFO);
	m_ToolTip.uFlags = TTF_IDISHWND;
	m_ToolTip.hwnd = m_hWnd;
	m_ToolTip.uId = (UINT_PTR)m_hWnd;
	m_ToolTip.hinst = GetResModuleHandle();
	m_ToolTip.lpszText = const_cast<LPTSTR>((LPCTSTR)sToolTip.c_str());
	m_ToolTip.rect = pHover->GetPos();
	if (m_hwndTooltip == nullptr) {
		m_hwndTooltip = ::CreateWindowEx(0, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hWnd, NULL, GetResModuleHandle(), NULL);
		::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&m_ToolTip);
		::SetWindowPos(m_hwndTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	if (!::IsWindowVisible(m_hwndTooltip)) {
		::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, pHover->GetToolTipWidth());
		::SendMessage(m_hwndTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&m_ToolTip);
		::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ToolTip);
	}
	::SendMessage(m_hwndTooltip, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD)MAKELONG(pt.x, pt.y));
	return 0;
}

LRESULT Window::OnMouseLeaveMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_MOUSELEAVE);
	bHandled = false;
	if (m_hwndTooltip != nullptr) {
		::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ToolTip);
	}
	if (m_bMouseTracking) {
		::SendMessage(m_hWnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
	}
	m_bMouseTracking = false;
	return 0;
}

bool Window::HandleMouseEnterLeave(const POINT& pt, WPARAM wParam, LPARAM lParam)
{
	Control* pNewHover = FindControl(pt);
	if (pNewHover != nullptr && pNewHover->GetWindow() != this) {
		return false;
	}

	if (pNewHover != m_pEventHover && m_pEventHover != nullptr) {
		//Hover状态的控件发生变化，原来Hover控件的Tooltip应消失
		m_pEventHover->HandleMessageTemplate(kEventMouseLeave, 0, 0, 0, CPoint(pt));
		m_pEventHover = nullptr;
		if (m_hwndTooltip != nullptr) {
			::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ToolTip);
		}
	}
	if (pNewHover != m_pEventHover && pNewHover != nullptr) {
		m_pEventHover = pNewHover;
		m_pEventHover->HandleMessageTemplate(kEventMouseEnter, wParam, lParam, 0, CPoint(pt));
	}
	return true;
}

LRESULT Window::OnMouseMoveMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_MOUSEMOVE);
	bHandled = false;
	if (m_pEventPointer != nullptr) {
		return 0;
	}

	// Start tracking this entire window again...
	if (!m_bMouseTracking) {
		TRACKMOUSEEVENT tme = { 0 };
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = m_hwndTooltip == nullptr ? 400UL : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
		_TrackMouseEvent(&tme);//TODO: tooltip的流程，待梳理。
		m_bMouseTracking = true;
	}
	// Generate the appropriate mouse messages
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	m_ptLastMousePos = CPoint(pt);

	// Do not move the focus to the new control when the mouse is pressed
	if (!IsCaptured()) {
		if (!HandleMouseEnterLeave(pt, wParam, lParam)) {
			return 0;
		}
	}

	if (m_pEventClick != nullptr) {
		m_pEventClick->HandleMessageTemplate(kEventMouseMove, wParam, lParam, 0, CPoint(pt));
	}
	else if (m_pEventHover != nullptr) {
		m_pEventHover->HandleMessageTemplate(kEventMouseMove, wParam, lParam, 0, CPoint(pt));
	}
	return 0;
}

LRESULT Window::OnMouseWheelMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_MOUSEWHEEL);
	bHandled = false;
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	::ScreenToClient(m_hWnd, &pt);
	m_ptLastMousePos = CPoint(pt);
	Control* pControl = FindControl(pt);
	if ((pControl != nullptr) && (pControl->GetWindow() == this)) {
		int zDelta = (int)(short)HIWORD(wParam);
		pControl->HandleMessageTemplate(kEventMouseScrollWheel, zDelta, lParam);
	}	
	return 0;
}

LRESULT Window::OnLButtonDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_LBUTTONDOWN);
	bHandled = false;
	if (m_pEventPointer != nullptr) {
		return 0;
	}

	// We alway set focus back to our app (this helps
	// when Win32 child windows are placed on the dialog
	// and we need to remove them on focus change).
	::SetFocus(m_hWnd);
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	m_ptLastMousePos = CPoint(pt);
	Control* pControl = FindControl(pt);
	if ((pControl != nullptr) && (pControl->GetWindow() == this)) {
		m_pEventClick = pControl;
		pControl->SetFocus();
		SetCapture();
		pControl->HandleMessageTemplate(kEventMouseButtonDown, wParam, lParam, 0, CPoint(pt));
	}	
	return 0;
}

LRESULT Window::OnRButtonDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_RBUTTONDOWN);
	bHandled = false;
	if (m_pEventPointer != nullptr) {
		return 0;
	}

	::SetFocus(m_hWnd);
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	m_ptLastMousePos = CPoint(pt);
	Control* pControl = FindControl(pt);
	if ((pControl != nullptr) && (pControl->GetWindow() == this)) {
		m_pEventClick = pControl;
		pControl->SetFocus();
		SetCapture();
		pControl->HandleMessageTemplate(kEventMouseRightButtonDown, wParam, lParam, 0, CPoint(pt));
	}	
	return 0;
}

LRESULT Window::OnLButtonDoubleClickMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_LBUTTONDBLCLK);
	bHandled = false;
	if (m_pEventPointer != nullptr) {
		return 0;
	}

	::SetFocus(m_hWnd);
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	m_ptLastMousePos = CPoint(pt);
	Control* pControl = FindControl(pt);
	if ((pControl != nullptr) && (pControl->GetWindow() == this)) {
		m_pEventClick = pControl;
		SetCapture();
		pControl->HandleMessageTemplate(kEventInternalDoubleClick, wParam, lParam, 0, CPoint(pt));
	}	
	return 0;
}

LRESULT Window::OnLButtonUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_LBUTTONUP);
	bHandled = false;
	if (m_pEventPointer != nullptr) {
		m_pEventPointer->HandleMessageTemplate(kEventPointUp, 0, lParam, 0, m_ptLastMousePos);
		m_pEventPointer = nullptr;
	}
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	m_ptLastMousePos = CPoint(pt);
	ReleaseCapture();
	if (m_pEventClick != nullptr) {
		m_pEventClick->HandleMessageTemplate(kEventMouseButtonUp, wParam, lParam, 0, CPoint(pt));
		m_pEventClick = nullptr;
	}
	return 0;
}

LRESULT Window::OnRButtonUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_RBUTTONUP);
	bHandled = false;
	if (m_pEventPointer != nullptr) {
		m_pEventPointer->HandleMessageTemplate(kEventPointUp, 0, lParam, 0, m_ptLastMousePos);
		m_pEventPointer = nullptr;
	}
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	m_ptLastMousePos = CPoint(pt);
	ReleaseCapture();
	if (m_pEventClick != nullptr) {
		m_pEventClick->HandleMessageTemplate(kEventMouseRightButtonUp, wParam, lParam, 0, CPoint(pt));
		m_pEventClick = nullptr;
	}
	return 0;
}

void Window::ReleaseEventClick(bool bClickOrPointer, WPARAM wParam, LPARAM lParam)
{
	//TODO: 待删除
	if (bClickOrPointer) {
		if (m_pEventClick) {
			m_pEventClick->HandleMessageTemplate(kEventMouseButtonUp, wParam, lParam, 0, m_ptLastMousePos);
			m_pEventClick = nullptr;
		}
	}
	else {
		if (m_pEventPointer) {
			m_pEventPointer->HandleMessageTemplate(kEventPointUp, 0, lParam, 0, m_ptLastMousePos);
			m_pEventPointer = nullptr;
		}
	}
}

LRESULT Window::OnIMEStartCompositionMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_IME_STARTCOMPOSITION);
	bHandled = false;
	if (m_pFocus != nullptr) {
		m_pFocus->HandleMessageTemplate(kEventImeStartComposition, wParam, lParam, static_cast<TCHAR>(wParam));
	}
	return 0;
}

LRESULT Window::OnIMEEndCompositionMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_IME_ENDCOMPOSITION);
	bHandled = false;
	if (m_pFocus != nullptr) {
		m_pFocus->HandleMessageTemplate(kEventImeEndComposition, wParam, lParam, static_cast<TCHAR>(wParam));
	}
	return 0;
}

LRESULT Window::OnSetFocusMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_SETFOCUS);
	bHandled = false;
	return 0;
}

LRESULT Window::OnKillFocusMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_KILLFOCUS);
	bHandled = false;
	Control* pControl = m_pEventClick;
	if (m_pEventPointer != nullptr) {
		pControl = m_pEventPointer;
	}
	//TODO: 此消息的实现有问题，待纠正
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	m_ptLastMousePos = CPoint(pt);
	if (pControl == nullptr || !pControl->IsNeedButtonUpWhenKillFocus()) {
		return 0;
	}
	ReleaseCapture();

	pControl->HandleMessageTemplate(kEventMouseButtonUp, wParam, lParam, 0, CPoint(pt));
	m_pEventClick = nullptr;
	m_pEventPointer = nullptr;
	return 0;
}

LRESULT Window::OnContextMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_CONTEXTMENU);
	bHandled = false;
	if (m_pEventPointer) {
		//TODO: 待确认
		m_pEventPointer->HandleMessageTemplate(kEventPointUp, 0, lParam, 0, m_ptLastMousePos);
		m_pEventPointer = nullptr;
	}
	ReleaseCapture();

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if ((pt.x != -1) && (pt.y != -1)) {
		::ScreenToClient(m_hWnd, &pt);
		m_ptLastMousePos = CPoint(pt);
		Control* pControl = FindContextMenuControl(&pt);
		if ((pControl != nullptr) && (pControl->GetWindow() == this)) {
			Control* ptControl = FindControl(pt);//当前点击点所在的控件
			pControl->HandleMessageTemplate(kEventMouseMenu, wParam, (LPARAM)ptControl, 0, CPoint(pt));
		}
	}
	else {
		//如果用户键入 SHIFT+F10，则上下文菜单为 -1, -1，
		//应用程序应在当前所选内容的位置（而不是 (xPos、yPos) ）显示上下文菜单。
		Control* pControl = FindContextMenuControl(nullptr);
		if ((pControl != nullptr) && (pControl->GetWindow() == this)) {
			pControl->HandleMessageTemplate(kEventMouseMenu, wParam, 0, 0, CPoint(pt));
		}
	}
	return 0;
}

LRESULT Window::OnCharMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_CHAR);
	bHandled = false;
	if (m_pFocus != nullptr) {
		m_pFocus->HandleMessageTemplate(kEventChar, wParam, lParam, static_cast<TCHAR>(wParam));
	}
	return 0;
}

LRESULT Window::OnKeyDownMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_KEYDOWN);
	bHandled = false;
	if (m_pFocus == nullptr) {
		return 0;
	}
	if (wParam == VK_TAB) {
		if (m_pFocus->IsVisible() &&
			m_pFocus->IsEnabled() &&
			m_pFocus->IsWantTab()) {
			return 0;
		}
		else {
			//通过TAB键切换焦点控件
			SetNextTabControl(::GetKeyState(VK_SHIFT) >= 0);
		}
	}
	else {
		m_pEventKey = m_pFocus;
		m_pFocus->HandleMessageTemplate(kEventKeyDown, wParam, lParam, static_cast<TCHAR>(wParam));
	}	
	return 0;
}

LRESULT Window::OnKeyUpMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_KEYUP);
	bHandled = false;
	if (m_pEventKey != nullptr) {
		m_pEventKey->HandleMessageTemplate(kEventKeyUp, wParam, lParam, static_cast<TCHAR>(wParam));
		m_pEventKey = nullptr;
	}
	return 0;
}

LRESULT Window::OnSetCusorMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_SETCURSOR);
	bHandled = false;
	if (LOWORD(lParam) != HTCLIENT) {
		return 0;
	}
	if ((m_pEventClick != nullptr) || (m_pEventPointer != nullptr)) {
		bHandled = true;
		return 0;
	}

	POINT pt = { 0 };
	::GetCursorPos(&pt);
	::ScreenToClient(m_hWnd, &pt);
	m_ptLastMousePos = CPoint(pt);
	Control* pControl = FindControl(pt);
	if ((pControl != nullptr) && (pControl->GetWindow() == this)) {
		pControl->HandleMessageTemplate(kEventSetCursor, wParam, lParam, 0, CPoint(pt));
		bHandled = true;
	}	
	return 0;
}

LRESULT Window::OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
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

LRESULT Window::OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
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

LRESULT Window::OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
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

LRESULT Window::OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_TOUCH);
	bHandled = false;
    unsigned int nNumInputs = (int)LOWORD(wParam);
    if (nNumInputs < 1) {
        nNumInputs = 1;
    }
    TOUCHINPUT* pInputs = new TOUCHINPUT[nNumInputs];
    class ScopedReleaser
    {
    public:
		ScopedReleaser() :m_pInputs(nullptr), m_lParam(nullptr) {}
        ~ScopedReleaser() {
            if (m_lParam != nullptr) {
                CloseTouchInputHandleWrapper(m_lParam);
				m_lParam = nullptr;
            }
            if (m_pInputs != nullptr) {
                delete[] m_pInputs;
				m_pInputs = nullptr;
            }
        }
        TOUCHINPUT* m_pInputs;
        HTOUCHINPUT m_lParam;
    };
    ScopedReleaser scoped_releaser;
    scoped_releaser.m_pInputs = pInputs;

    // 只关心第一个触摸位置
	if ((nNumInputs < 1) || !GetTouchInputInfoWrapper((HTOUCHINPUT)lParam, nNumInputs, pInputs, sizeof(TOUCHINPUT)))
	{
		return 0;
	}
    scoped_releaser.m_lParam = (HTOUCHINPUT)lParam;
	if (pInputs[0].dwID == 0)
	{
		return 0;
	}
    POINT pt;
    pt.x = TOUCH_COORD_TO_PIXEL(pInputs[0].x);
    pt.y = TOUCH_COORD_TO_PIXEL(pInputs[0].y);
    ScreenToClient(m_hWnd, &pt);

    if (pInputs[0].dwFlags & TOUCHEVENTF_DOWN)
    {
        if (m_pEventClick != nullptr) {
			return 0;
        }

        ::SetFocus(m_hWnd);
        m_ptLastMousePos = CPoint(pt);
        Control* pControl = FindControl(pt);
        if (pControl == nullptr) {
			return 0;
        }
        if (pControl->GetWindow() != this) {
			return 0;
        }
        m_pEventPointer = pControl;
        pControl->SetFocus();
        SetCapture();

        pControl->HandleMessageTemplate(kEventTouchDown, 0, lParam, 0, CPoint(pt));
    }
    else if (pInputs[0].dwFlags & TOUCHEVENTF_MOVE)
    {
        if (m_pEventClick != nullptr) {
			return 0;
        }

        if (m_ptLastMousePos.x == pt.x && m_ptLastMousePos.y == pt.y) {
			return 0;
        }

        m_ptLastMousePos = CPoint(pt);
        if (m_pEventPointer == nullptr) {
            return 0;
        }

        if (!HandleMouseEnterLeave(pt, wParam, lParam)) {
			return 0;
        }

        m_pEventPointer->HandleMessageTemplate(kEventTouchMove, 0, 0, 0, CPoint(pt));
    }
    else if (pInputs[0].dwFlags & TOUCHEVENTF_UP)
    {
        ReleaseEventClick(true, wParam, lParam);
        m_ptLastMousePos = CPoint(pt);
        ReleaseCapture();
        if (m_pEventPointer != nullptr) {
			m_pEventPointer->HandleMessageTemplate(kEventTouchUp, 0, lParam, 0, CPoint(pt));
			m_pEventPointer = nullptr;
        }        
    }
	return 0;
}

LRESULT Window::OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	ASSERT_UNUSED_VARIABLE(uMsg == WM_POINTERDOWN || 
						   uMsg == WM_POINTERUP ||
						   uMsg == WM_POINTERUPDATE ||
						   uMsg == WM_POINTERLEAVE ||
						   uMsg == WM_POINTERCAPTURECHANGED);

    bHandled = false;
    if (!m_bHandlePointerMsg) {
        return 0;
    }

    // 只关心第一个触摸点
    if (!IS_POINTER_PRIMARY_WPARAM(wParam)) {
        bHandled = true;
		return 0;
    }

    UINT32 pointerId = GET_POINTERID_WPARAM(wParam);
    POINTER_INPUT_TYPE type;
    if (!GetPointerTypeWrapper(pointerId, &type)) {
        return 0;
    }

    POINT pt = { 0 };
    FLOAT pressure = 0.0f;

    switch (type)
    {
    case PT_TOUCH:
        POINTER_TOUCH_INFO touchInfo;
        if (!GetPointerTouchInfoWrapper(pointerId, &touchInfo)) {
            return 0;
        }

        pt = touchInfo.pointerInfo.ptPixelLocationRaw;
        pressure = (float)touchInfo.pressure / 1024;
        break;
    case PT_PEN:
        POINTER_PEN_INFO penInfo;
        if (!GetPointerPenInfoWrapper(pointerId, &penInfo)) {
            return 0;
        }

        pt = penInfo.pointerInfo.ptPixelLocationRaw;
        pressure = (float)penInfo.pressure / 1024;
        break;
    default:
        return 0;
    }
    ScreenToClient(m_hWnd, &pt);

    switch (uMsg)
    {
    case WM_POINTERDOWN:
    {
        if (m_pEventClick != nullptr) {
            bHandled = true;
            break;
        }

        ::SetFocus(m_hWnd);
        m_ptLastMousePos = CPoint(pt);
        Control* pControl = FindControl(pt);
        if (pControl == nullptr) {
            break;
        }
        if (pControl->GetWindow() != this) {
            break;
        }
        m_pEventPointer = pControl;
        pControl->SetFocus();
        SetCapture();

        pControl->HandleMessageTemplate(kEventPointDown, 0, lParam, 0, CPoint(pt), pressure);

        // 如果控件不支持处理WM_POINTERUPDATE消息，则不设置bHandled，程序会进入WM_BUTTON处理流程
        if (m_pEventPointer && m_pEventPointer->IsReceivePointerMsg()) {
            bHandled = true;
        }
    }
    break;
    case WM_POINTERUPDATE:
        if (m_pEventClick != nullptr) {
            bHandled = true;
            break;
        }

        if (m_ptLastMousePos.x == pt.x && m_ptLastMousePos.y == pt.y)
            break;

        m_ptLastMousePos = CPoint(pt);
        // 如果没有按下，则不设置bHandled，程序会转换为WM_BUTTON类消息
        if (m_pEventPointer == nullptr) {
            break;
        }

        if (!HandleMouseEnterLeave(pt, wParam, lParam)) break;

        m_pEventPointer->HandleMessageTemplate(kEventPointMove, 0, 0, 0, CPoint(pt), pressure);

        // 如果控件不支持处理WM_POINTERUPDATE消息，则不设置bHandled，程序会进入WM_MOUSEMOVE处理流程
        if (m_pEventPointer && m_pEventPointer->IsReceivePointerMsg()) {
            bHandled = true;
        }
        break;
    case WM_POINTERUP:
    case WM_POINTERLEAVE:
    case WM_POINTERCAPTURECHANGED:
        ReleaseEventClick(true, wParam, lParam);
        m_ptLastMousePos = CPoint(pt);
        // 如果没有按下，则不设置bHandled，程序会转换为WM_BUTTON类消息
        if (uMsg != WM_POINTERLEAVE) {
            // Refer to LBUTTONUP and MOUSELEAVE，LBUTTOUP ReleaseCapture while MOUSELEAVE DONOT ReleaseCapture
            ReleaseCapture();
        }
        if (m_pEventPointer == nullptr) {
            break;
        }

        m_pEventPointer->HandleMessageTemplate(kEventPointUp, 0, lParam, 0, CPoint(pt), pressure);

        // 如果控件不支持处理WM_POINTERUPDATE消息，则不设置bHandled，程序会进入WM_BUTTON处理流程
        if (m_pEventPointer && m_pEventPointer->IsReceivePointerMsg()) {
            bHandled = true;
        }
        m_pEventPointer = nullptr;
        break;
    default:
        break;
    }

    return 0;
}

Control* Window::GetFocus() const
{
	return m_pFocus;
}

void Window::SetFocus(Control* pControl)
{
	// Paint manager window has focus?
	HWND hFocusWnd = ::GetFocus();
	if ((hFocusWnd != m_hWnd) && (pControl != m_pFocus) && (pControl != nullptr)) {
		::SetFocus(m_hWnd);
	}
	// Already has focus?
	if (pControl == m_pFocus) {
		return;
	}
	// Remove focus from old control
	if (m_pFocus != nullptr) {
		m_pFocus->HandleMessageTemplate(kEventInternalKillFocus);
		Control* tmp = m_pFocus;
		m_pFocus = nullptr;
		SendNotify(tmp, kEventKillFocus);
	}
	// Set focus to new control
	if ((pControl != nullptr)			&& 
		(pControl->GetWindow() == this) && 
		 pControl->IsVisible()			&&
		 pControl->IsEnabled())
	{
		m_pFocus = pControl;
		ASSERT(::GetFocus() == m_hWnd);
		if (m_pFocus) {
			m_pFocus->HandleMessageTemplate(kEventInternalSetFocus);
		}
		if (m_pFocus) {
			SendNotify(m_pFocus, kEventSetFocus);
		}
	}
}

void Window::KillFocus()
{
	if (m_pFocus != nullptr) {
		m_pFocus->HandleMessageTemplate(kEventInternalKillFocus);
		SendNotify(m_pFocus, kEventKillFocus);
		m_pFocus = nullptr;
	}
}

void Window::SetCapture()
{
	::SetCapture(m_hWnd);
	m_bMouseCapture = true;
}

void Window::ReleaseCapture()
{
	if (m_bMouseCapture) {
		::ReleaseCapture();
		m_bMouseCapture = false;
	}
}

bool Window::IsCaptureControl(const ui::Control* pControl) const
{
	return m_pEventClick == pControl || m_pEventPointer == pControl;
}

bool Window::IsCaptured() const
{
	return m_bMouseCapture;
}

ui::Control* Window::GetHoverControl() const
{
	return m_pEventHover;
}

HWND Window::GetTooltipWindow() const
{
	return m_hwndTooltip;
}

bool Window::SetNextTabControl(bool bForward)
{
	// Find next/previous tabbable control
	FINDTABINFO info1 = { 0 };
	info1.pFocus = m_pFocus;
	info1.bForward = bForward;
	Control* pControl = m_pRoot->FindControl(ControlFinder::__FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
	if (pControl == nullptr) {
		if (bForward) {
			// Wrap around
			FINDTABINFO info2 = { 0 };
			info2.pFocus = bForward ? NULL : info1.pLast;
			info2.bForward = bForward;
			pControl = m_pRoot->FindControl(ControlFinder::__FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
		}
		else {
			pControl = info1.pLast;
		}
	}
	if (pControl != nullptr) {
		SetFocus(pControl);
	}
	return true;
}

Box* Window::GetRoot() const
{
	return m_pRoot;
}

void Window::SetArrange(bool bArrange)
{
	m_bIsArranged = bArrange;
}

void Window::AddDelayedCleanup(Control* pControl)
{
	if (pControl != nullptr) {
		pControl->SetWindow(this, NULL, false);
		m_aDelayedCleanup.push_back(pControl);
		::PostMessage(m_hWnd, WM_APP + 1, 0, 0L);
	}	
}

bool Window::SendNotify(EventType eventType, WPARAM wParam, LPARAM lParam)
{
	EventArgs msg;
	msg.pSender = nullptr;
	msg.Type = eventType;
	msg.ptMouse = GetLastMousePos();
	msg.dwTimestamp = ::GetTickCount();
	msg.wParam = wParam;
	msg.lParam = lParam;

	auto callback = m_OnEvent.find(msg.Type);
	if (callback != m_OnEvent.end()) {
		callback->second(&msg);
	}

	callback = m_OnEvent.find(kEventAll);
	if (callback != m_OnEvent.end()) {
		callback->second(&msg);
	}

	return true;
}

bool Window::SendNotify(Control* pControl, EventType msgType, WPARAM wParam, LPARAM lParam)
{
	EventArgs msg;
	msg.pSender = pControl;
	msg.Type = msgType;
	msg.ptMouse = GetLastMousePos();
	msg.dwTimestamp = ::GetTickCount();
	msg.wParam = wParam;
	msg.lParam = lParam;
	if (pControl != nullptr) {
		pControl->HandleMessageTemplate(msg);
	}
	return true;
}

HDC Window::GetPaintDC() const
{
	return m_hDcPaint;
}

ui::IRenderContext* Window::GetRenderContext() const
{
	return m_renderContext.get();
}

void Window::Invalidate(const UiRect& rcItem)
{
	GlobalManager::AssertUIThread();

	::InvalidateRect(m_hWnd, &rcItem, FALSE);
	// Invalidating a layered window will not trigger a WM_PAINT message,
	// thus we have to post WM_PAINT by ourselves.
	if (m_bIsLayeredWindow) {
		::PostMessage(m_hWnd, WM_PAINT, (LPARAM)&rcItem, (WPARAM)FALSE);
	}
}

void Window::Paint()
{
	GlobalManager::AssertUIThread();

	if (::IsIconic(m_hWnd) || (m_pRoot == nullptr)) {
		PAINTSTRUCT ps = { 0 };
		::BeginPaint(m_hWnd, &ps);
		::EndPaint(m_hWnd, &ps);
		return;
	}

	if (m_bIsArranged && m_pRoot->IsArranged() && (m_pRoot->GetFixedWidth() == DUI_LENGTH_AUTO || m_pRoot->GetFixedHeight() == DUI_LENGTH_AUTO)) {
		CSize maxSize(99999, 99999);
		CSize needSize = m_pRoot->EstimateSize(maxSize);
		if (needSize.cx < m_pRoot->GetMinWidth()) {
			needSize.cx = m_pRoot->GetMinWidth();
		}
		if (m_pRoot->GetMaxWidth() >= 0 && needSize.cx > m_pRoot->GetMaxWidth()) {
			needSize.cx = m_pRoot->GetMaxWidth();
		}
		if (needSize.cy < m_pRoot->GetMinHeight()) {
			needSize.cy = m_pRoot->GetMinHeight();
		}
		if (needSize.cy > m_pRoot->GetMaxHeight()) {
			needSize.cy = m_pRoot->GetMaxHeight();
		}
		UiRect rect;
		::GetWindowRect(m_hWnd, &rect);
		::MoveWindow(m_hWnd, rect.left, rect.top, needSize.cx, needSize.cy, TRUE);
	}

	// Should we paint?
	UiRect rcPaint;
	if (!::GetUpdateRect(m_hWnd, &rcPaint, FALSE) && !m_bFirstLayout) {
		return;
	}

	UiRect rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	UiRect rcWindow;
	::GetWindowRect(m_hWnd, &rcWindow);

	//使用层窗口时，窗口部分在屏幕外时，获取到的无效区域仅仅是屏幕内的部分，这里做修正处理
	if (m_bIsLayeredWindow) {
		int xScreen = GetSystemMetrics(SM_XVIRTUALSCREEN);
		int yScreen = GetSystemMetrics(SM_YVIRTUALSCREEN);
		int cxScreen = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int cyScreen = GetSystemMetrics(SM_CYVIRTUALSCREEN);
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

	PAINTSTRUCT ps = { 0 };
	::BeginPaint(m_hWnd, &ps);

	if (m_bIsArranged) {
		m_bIsArranged = false;
		if (!::IsRectEmpty(&rcClient)) {
			if (m_pRoot->IsArranged()) {
				m_pRoot->SetPos(rcClient);
			}
			else {
				Control* pControl = m_pRoot->FindControl(ControlFinder::__FindControlFromUpdate, nullptr, UIFIND_VISIBLE | UIFIND_ME_FIRST);
				while (pControl != nullptr) {
					pControl->SetPos(pControl->GetPos());
					//ASSERT(!pControl->IsArranged());
					pControl = m_pRoot->FindControl(ControlFinder::__FindControlFromUpdate, nullptr, UIFIND_VISIBLE | UIFIND_ME_FIRST);
				}
			}

			if (m_bFirstLayout) {
				m_bFirstLayout = false;
				OnInitLayout();
			}
		}
	}

	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;
	if (m_renderContext->Resize(width, height))	{
		rcPaint.left = 0;
		rcPaint.top = 0;
		rcPaint.right = width;
		rcPaint.bottom = height;
	}

	// 去掉alpha通道
	if (m_bIsLayeredWindow) {
		m_renderContext->ClearAlpha(rcPaint);
	}

	// 绘制
	AutoClip rectClip(m_renderContext.get(), rcPaint, true);
	CPoint ptOldWindOrg = m_renderContext->OffsetWindowOrg(m_renderOffset);
	m_pRoot->Paint(m_renderContext.get(), rcPaint);
	m_pRoot->PaintChild(m_renderContext.get(), rcPaint);
	m_renderContext->SetWindowOrg(ptOldWindOrg);

	// alpha修复
	if (m_bIsLayeredWindow) {
		if (m_shadow->IsShadowAttached() && m_renderOffset.x == 0 && m_renderOffset.y == 0) {
			//补救由于Gdi绘制造成的alpha通道为0
			UiRect rcNewPaint = rcPaint;
			rcNewPaint.Intersect(m_pRoot->GetPaddingPos());
			UiRect rcRootPadding = m_pRoot->GetLayout()->GetPadding();

			//考虑圆角
			rcRootPadding.left += 1;
			rcRootPadding.top += 1;
			rcRootPadding.right += 1;
			rcRootPadding.bottom += 1;

			m_renderContext->RestoreAlpha(rcNewPaint, rcRootPadding);
		}
		else {
			UiRect rcAlphaFixCorner = GetAlphaFixCorner();
			if (rcAlphaFixCorner.left > 0 || rcAlphaFixCorner.top > 0 || rcAlphaFixCorner.right > 0 || rcAlphaFixCorner.bottom > 0)
			{
				UiRect rcNewPaint = rcPaint;
				UiRect rcRootPaddingPos = m_pRoot->GetPaddingPos();
				rcRootPaddingPos.Deflate(rcAlphaFixCorner);
				rcNewPaint.Intersect(rcRootPaddingPos);

				UiRect rcRootPadding;
				m_renderContext->RestoreAlpha(rcNewPaint, rcRootPadding);
			}
		}
	}

	// 渲染到窗口
	if (m_bIsLayeredWindow) {
		CPoint pt(rcWindow.left, rcWindow.top);
		CSize szWindow(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
		CPoint ptSrc;
		BLENDFUNCTION bf = { AC_SRC_OVER, 0, static_cast<BYTE>(m_nAlpha), AC_SRC_ALPHA };
		::UpdateLayeredWindow(m_hWnd, NULL, &pt, &szWindow, m_renderContext->GetDC(), &ptSrc, 0, &bf, ULW_ALPHA);
	}
	else {
		::BitBlt(ps.hdc, rcPaint.left, rcPaint.top, rcPaint.GetWidth(),
			rcPaint.GetHeight(), m_renderContext->GetDC(), rcPaint.left, rcPaint.top, SRCCOPY);
	}

	::EndPaint(m_hWnd, &ps);
}

void Window::SetAlpha(int nAlpha)
{
	ASSERT(nAlpha >= 0 && nAlpha <= 255);
	if (m_pRoot != nullptr) {
		m_nAlpha = nAlpha;
		Invalidate(m_pRoot->GetPos());
	}
}

bool Window::IsRenderTransparent() const
{
	return m_renderContext->IsRenderTransparent();
}

bool Window::SetRenderTransparent(bool bCanvasTransparent)
{
	return m_renderContext->SetRenderTransparent(bCanvasTransparent);
}

bool Window::IsLayeredWindow() const
{
	return m_bIsLayeredWindow;
}

void Window::SetRenderOffset(CPoint renderOffset)
{
	if (m_pRoot != nullptr) {
		m_renderOffset = renderOffset;
		Invalidate(m_pRoot->GetPos());
	}
}

void Window::SetRenderOffsetX(int renderOffsetX)
{
	if (m_pRoot != nullptr) {
		m_renderOffset.x = renderOffsetX;
		Invalidate(m_pRoot->GetPos());
	}
}

void Window::SetRenderOffsetY(int renderOffsetY)
{
	if (m_pRoot != nullptr) {
		m_renderOffset.y = renderOffsetY;
		Invalidate(m_pRoot->GetPos());
	}
}

void Window::OnInitLayout()
{
	if ((m_pRoot != nullptr) && IsWindowsVistaOrGreater()) {
		AnimationPlayer* animationArgs = m_pRoot->GetAnimationManager().SetFadeAlpha(true);
		m_nAlpha = 0;
		std::function<void(int)> playCallback = nbase::Bind(&Window::SetAlpha, this, std::placeholders::_1);
		animationArgs->SetCallback(playCallback);
		m_pRoot->SetFadeVisible(true);
	}
}

Control* Window::FindControl(const POINT& pt) const
{
	return m_controlFinder.FindControl(pt);
}

Control* Window::FindContextMenuControl(const POINT* pt) const
{
	return m_controlFinder.FindContextMenuControl(pt);
}

Control* Window::FindControl(const std::wstring& strName) const
{
	return m_controlFinder.FindSubControlByName(m_pRoot, strName);
}

Control* Window::FindSubControlByPoint(Control* pParent, const POINT& pt) const
{
	return m_controlFinder.FindSubControlByPoint(pParent, pt);
}

Control* Window::FindSubControlByName(Control* pParent, const std::wstring& strName) const
{
	return m_controlFinder.FindSubControlByName(pParent, strName);
}

} // namespace ui
