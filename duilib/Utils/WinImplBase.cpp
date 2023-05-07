#include "WinImplBase.h"
#include "duilib/Utils/Shadow.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/Box.h"
#include <tchar.h>

namespace ui
{

WindowImplBase::WindowImplBase()
{
}

WindowImplBase::~WindowImplBase()
{
}

void WindowImplBase::OnInitWindow()
{
}

Control* WindowImplBase::CreateControl(const std::wstring& /*strClass*/)
{
	return nullptr;
}

LRESULT WindowImplBase::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	LRESULT lRes = 0;
	bHandled = false;
	switch (uMsg)
	{
	case WM_CREATE:			lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;	
	case WM_NCLBUTTONDBLCLK:lRes = OnNcLButtonDbClick(uMsg, wParam, lParam, bHandled); break;
	case WM_SYSCOMMAND:		lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
	//以下消息，无具体实现
	case WM_CLOSE:			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
	case WM_DESTROY:		lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
	case WM_MOUSEMOVE:		lRes = OnMouseMove(uMsg, wParam, lParam, bHandled); break;
	case WM_MOUSEWHEEL:		lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled); break;
	case WM_MOUSEHOVER:		lRes = OnMouseHover(uMsg, wParam, lParam, bHandled); break;
	case WM_LBUTTONDOWN:	lRes = OnLButtonDown(uMsg, wParam, lParam, bHandled); break;
	case WM_LBUTTONUP:		lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled); break;
	case WM_LBUTTONDBLCLK:	lRes = OnLButtonDbClk(uMsg, wParam, lParam, bHandled); break;
	case WM_RBUTTONDOWN:	lRes = OnRButtonDown(uMsg, wParam, lParam, bHandled); break;
	case WM_RBUTTONUP:		lRes = OnRButtonUp(uMsg, wParam, lParam, bHandled); break;
	case WM_RBUTTONDBLCLK:	lRes = OnRButtonDbClk(uMsg, wParam, lParam, bHandled); break;
	case WM_CHAR:			lRes = OnChar(uMsg, wParam, lParam, bHandled); break;
	case WM_KEYDOWN:		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
	case WM_KEYUP:			lRes = OnKeyUp(uMsg, wParam, lParam, bHandled); break;
	default:
		bHandled = false; 
		break;
	}
	return lRes;
}

LRESULT WindowImplBase::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	::SetWindowLong(this->GetHWND(), GWL_STYLE, GetStyle());

	InitWnd(GetHWND());
	SetResourcePath(GetSkinFolder());

	WindowBuilder builder;
	std::wstring strSkinFile;
	std::wstring xmlFile = GetSkinFile();
	if (!xmlFile.empty() && xmlFile.front() == L'<') {
		//返回的内容是XML文件内容，而不是文件路径
		strSkinFile = std::move(xmlFile);
	}
	else {
		strSkinFile = GetResourcePath() + xmlFile;
	}

	auto callback = nbase::Bind(&WindowImplBase::CreateControl, this, std::placeholders::_1);
	Box* pRoot = builder.Create(strSkinFile, callback, this);

	ASSERT(pRoot && L"Faield to load xml file.");
	if (pRoot == NULL) {
		TCHAR szErrMsg[MAX_PATH] = { 0 };
		_stprintf_s(szErrMsg, L"Failed to load xml file %s", strSkinFile.c_str());
		MessageBox(NULL, szErrMsg, _T("Duilib"), MB_OK | MB_ICONERROR);
		return -1;
	}
	//关联边框阴影
	pRoot = AttachShadow(pRoot);
	AttachBox(pRoot);
	
	if (pRoot->GetFixedWidth() == DUI_LENGTH_AUTO || pRoot->GetFixedHeight() == DUI_LENGTH_AUTO) {
		UiSize maxSize(99999, 99999);
		UiSize needSize = pRoot->EstimateSize(maxSize);
		if (needSize.cx < pRoot->GetMinWidth()) needSize.cx = pRoot->GetMinWidth();
		if (pRoot->GetMaxWidth() >= 0 && needSize.cx > pRoot->GetMaxWidth()) needSize.cx = pRoot->GetMaxWidth();
		if (needSize.cy < pRoot->GetMinHeight()) needSize.cy = pRoot->GetMinHeight();
		if (needSize.cy > pRoot->GetMaxHeight()) needSize.cy = pRoot->GetMaxHeight();

		::MoveWindow(GetHWND(), 0, 0, needSize.cx, needSize.cy, FALSE);
	}

	Control* pControl = (Control*)FindControl(DUI_CTR_BUTTON_CLOSE);
	if (pControl) {
		ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
		pControl->AttachClick(nbase::Bind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
	}

	pControl = (Control*)FindControl(DUI_CTR_BUTTON_MIN);
	if (pControl) {
		ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
		pControl->AttachClick(nbase::Bind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
	}

	pControl = (Control*)FindControl(DUI_CTR_BUTTON_MAX);
	if (pControl) {
		ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
		pControl->AttachClick(nbase::Bind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
	}

	pControl = (Control*)FindControl(DUI_CTR_BUTTON_RESTORE);
	if (pControl) {
		ASSERT(pControl->GetType() == DUI_CTR_BUTTON);
		pControl->AttachClick(nbase::Bind(&WindowImplBase::OnButtonClick, this, std::placeholders::_1));
	}

	OnInitWindow();
	return 0;
}

void WindowImplBase::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

LRESULT WindowImplBase::OnNcLButtonDbClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = true;
	Control* pBtnMax = FindControl(DUI_CTR_BUTTON_MAX);
	if (pBtnMax != nullptr) {
		ASSERT(pBtnMax->GetType() == DUI_CTR_BUTTON);
	}	
	Control* pBtnRestore = FindControl(DUI_CTR_BUTTON_RESTORE);
	if (pBtnRestore != nullptr) {
		ASSERT(pBtnRestore->GetType() == DUI_CTR_BUTTON);
	}

	if (!::IsZoomed(GetHWND()))
	{
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		if (pBtnMax && pBtnRestore)
		{
			pBtnMax->SetVisible(false);
			pBtnRestore->SetVisible(true);
		}
	}
	else
	{
		SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
		if (pBtnMax && pBtnRestore)
		{
			pBtnMax->SetVisible(true);
			pBtnRestore->SetVisible(false);
		}
	}

	return 0;
}

LRESULT WindowImplBase::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	bHandled = true;
	if (wParam == SC_CLOSE)	{		
		SendMessage(WM_CLOSE);
		return 0;
	}
	//首先调用默认的窗口函数，使得命令生效
	BOOL bZoomed = ::IsZoomed(GetHWND());
	LRESULT lRes = this->CallDefaultWindowProc(uMsg, wParam, lParam);
	if( ::IsZoomed(GetHWND()) != bZoomed) {
		if (wParam == 0xF012) {
			//修复窗口最大化和还原按钮的状态（当在最大化时，向下拖动标题栏，窗口会改变为非最大化状态）
			Control* pBtnMax = FindControl(DUI_CTR_BUTTON_MAX);
			if (pBtnMax != nullptr) {
				ASSERT(pBtnMax->GetType() == DUI_CTR_BUTTON);
			}
			Control* pBtnRestore = FindControl(DUI_CTR_BUTTON_RESTORE);
			if (pBtnRestore != nullptr) {
				ASSERT(pBtnRestore->GetType() == DUI_CTR_BUTTON);
			}

			if (pBtnMax && pBtnRestore) {
				if (!::IsZoomed(GetHWND())) {
					//非最大化
					pBtnMax->SetVisible(true);
					pBtnRestore->SetVisible(false);
				}
				else {
					//最大化
					pBtnMax->SetVisible(false);
					pBtnRestore->SetVisible(true);
				}
			}			
		}
	}

	return lRes;
}

bool WindowImplBase::OnButtonClick(const EventArgs& msg)
{
	ASSERT(msg.pSender != nullptr);
	if (msg.pSender == nullptr) {
		return false;
	}
	std::wstring sCtrlName = msg.pSender->GetName();
	if( sCtrlName == DUI_CTR_BUTTON_CLOSE) {
		CloseWnd();
	}
	else if( sCtrlName == DUI_CTR_BUTTON_MIN) {
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
	}
	else if( sCtrlName == DUI_CTR_BUTTON_MAX)	{
		Control* pMaxButton = (Control*)FindControl(DUI_CTR_BUTTON_MAX);
		Control* pRestoreButton = (Control*)FindControl(DUI_CTR_BUTTON_RESTORE);
		if (pMaxButton && pRestoreButton) {
			pMaxButton->SetFadeVisible(false);
			pRestoreButton->SetFadeVisible(true);
		}
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	}
	else if( sCtrlName == DUI_CTR_BUTTON_RESTORE)	{
		Control* pMaxButton = (Control*)FindControl(DUI_CTR_BUTTON_MAX);
		Control* pRestoreButton = (Control*)FindControl(DUI_CTR_BUTTON_RESTORE);
		if (pMaxButton && pRestoreButton) {
			pMaxButton->SetFadeVisible(true);
			pRestoreButton->SetFadeVisible(false);
		}
		SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 
	}

	return true;
}

LRESULT WindowImplBase::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnMouseHover(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnLButtonDbClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnRButtonDbClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

LRESULT WindowImplBase::OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
	bHandled = false;
	return 0;
}

}