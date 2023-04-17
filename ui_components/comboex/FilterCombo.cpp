#include "FilterCombo.h"
#include "duilib/Core/Window.h"
#include "base/util/string_util.h"

namespace nim_comp
{

class CFilterComboWnd: 
	public ui::Window
{
public:
    void Init(FilterCombo* pOwner);
    virtual std::wstring GetWindowClassName() const override;
	virtual void OnFinalMessage(HWND hWnd) override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	void OnSeleteItem();

private:
    FilterCombo *m_pOwner = nullptr;
    int m_iOldSel = -1;
	bool m_bClosing = false;
};

void CFilterComboWnd::Init(FilterCombo* pOwner)
{
	ASSERT(pOwner != nullptr);
	if (pOwner == nullptr) {
		return;
	}
    m_pOwner = pOwner;
    m_iOldSel = m_pOwner->GetCurSel();

    // Position the popup window in absolute space
	ui::CSize szDrop = m_pOwner->GetDropBoxSize();
	ui::UiRect rcOwner = pOwner->GetPosWithScrollOffset();
	ui::UiRect rc = rcOwner;
    rc.top = rc.bottom + 1;		// 父窗口left、bottom位置作为弹出窗口起点
    rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
	if (szDrop.cx > 0) {
		rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度
	}

	ui::CSize szAvailable(rc.right - rc.left, rc.bottom - rc.top);
    int cyFixed = 0;
	for (int it = 0; it < pOwner->GetListBox()->GetCount(); it++) {
		ui::Control* pControl = pOwner->GetListBox()->GetItemAt(it);
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
		ui::CSize sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
    }
    cyFixed += 2; // VBox 默认的Padding 调整
    rc.bottom = rc.top + std::min((LONG)cyFixed, szDrop.cy);

	ASSERT(pOwner->GetWindow() != nullptr);
    ::MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, &rc);

    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	ui::UiRect rcWork(oMonitor.rcWork);
    if( rc.bottom > rcWork.bottom || m_pOwner->IsPopupTop()) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;
        rc.top = rcOwner.top - std::min((LONG)cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
        ::MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, &rc);
    }
    
    Create(pOwner->GetWindow()->GetHWND(), NULL, WS_POPUP, WS_EX_TOOLWINDOW, true, rc);
    // HACK: Don't deselect the parent's caption
    HWND hWndParent = m_hWnd;
	while (::GetParent(hWndParent) != NULL) {
		hWndParent = ::GetParent(hWndParent);
	}
    ::ShowWindow(m_hWnd, SW_SHOW);
    //::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
}

std::wstring CFilterComboWnd::GetWindowClassName() const
{
    return L"ComboWnd";
}

void CFilterComboWnd::OnFinalMessage(HWND /*hWnd*/)
{
	if (m_pOwner != nullptr) {
		m_pOwner->m_pComboWnd = NULL;
		m_pOwner->m_uButtonState = ui::kControlStateNormal;
		m_pOwner->Invalidate();		
	}
	delete this;
}

void CFilterComboWnd::OnSeleteItem()
{
	PostMessage(WM_KILLFOCUS);
}

LRESULT CFilterComboWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if( uMsg == WM_CREATE ) {
        this->Window::Init(m_hWnd);
		ui::Box* pRoot = new ui::Box;
		pRoot->SetAutoDestroyChild(false);
		pRoot->Add(m_pOwner->GetListBox());
		m_pOwner->GetListBox()->SetFilterComboWnd(this);
		this->AttachDialog(pRoot);
		this->SetWindowResourcePath(m_pOwner->GetWindow()->GetWindowResourcePath());
		this->SetShadowAttached(false);

        return 0;
    }
    else if( uMsg == WM_CLOSE ) {
        m_pOwner->SetWindow(m_pOwner->GetWindow(), m_pOwner->GetParent(), false);
        m_pOwner->SetPos(m_pOwner->GetPos());
        m_pOwner->SetFocus();
    }
    else if( uMsg == WM_KILLFOCUS ) {
		if (m_hWnd != (HWND)wParam)	{ 
			m_bClosing = true;
			PostMessage(WM_CLOSE);
			if ((m_pOwner != nullptr) && (m_pOwner->GetListBox() != nullptr)){
				m_pOwner->SelectItem(m_pOwner->GetListBox()->GetCurSel());
				((ui::Box*)this->GetRoot())->RemoveAt(0);
				m_pOwner->GetListBox()->PlaceHolder::SetWindow(nullptr, nullptr, false);
				m_pOwner->GetListBox()->SetFilterComboWnd(nullptr);
			}
		}
    }
#if 1
	else if (uMsg == WM_CHAR || uMsg == WM_KEYDOWN || uMsg == WM_KEYUP) {
		if (m_pOwner){
			ui::EventArgs args;
			args.pSender = m_pOwner->GetListBox();
			args.chKey = static_cast<TCHAR>(wParam);
			if (uMsg == WM_CHAR) {
				args.Type = ui::kEventChar;
			}
			else if (uMsg == WM_KEYDOWN) {
				args.Type = ui::kEventKeyDown;
			}
			else if (uMsg == WM_KEYUP) {
				args.Type = ui::kEventKeyUp;
			}
			args.wParam = wParam;
			args.lParam = lParam;
			args.dwTimestamp = ::GetTickCount();
			m_pOwner->HandleMessage(args);
		}
	}
	else if (uMsg == WM_SYSKEYDOWN){
		if (m_pOwner)
		{
			ui::EventArgs args;
			args.pSender = m_pOwner->GetListBox();
			args.chKey = static_cast<TCHAR>(wParam);
			args.Type = ui::kEventSystemKey;
			args.wParam = 0;
			args.lParam = 0;
			args.dwTimestamp = ::GetTickCount();
			m_pOwner->HandleMessage(args);
		}
	}
#endif
	if (m_bClosing)	{
		return CallWindowProc(uMsg, wParam, lParam);
	}
	else {
		bool handled = false;
		LRESULT ret = this->DoHandlMessage(uMsg, wParam, lParam, handled);
		if (handled) {
			return ret;
		}
		else {
			return CallWindowProc(uMsg, wParam, lParam);
		}
	}
}

/////////////////////FilterListBox//////////////////////////
/////////////////////FilterListBox//////////////////////////

bool ListElementMatch::StringMatch(const std::string& utf8str)
{
	std::string utf8text = nbase::MakeLowerString(GetUTF8Text());
	std::string utf8date = nbase::MakeLowerString(GetUTF8DataID());
	std::string searchkey = nbase::MakeLowerString(utf8str);

	if (utf8text.find(searchkey) != std::string::npos || utf8date.find(searchkey) != std::string::npos)
	{
		return true;
	}
	return false;
}


bool FilterListBox::SelectItem(int iIndex, bool bTakeFocus, bool bTrigger)
{
	if (iIndex == m_iCurSel) return true;
	int iOldSel = m_iCurSel;
	// We should first unselect the currently selected item
	if (m_iCurSel >= 0) {
		Control* pControl = GetItemAt(m_iCurSel);
		if (pControl != NULL) {
			ui::ListContainerElement* pListItem = dynamic_cast<ui::ListContainerElement*>(pControl);
			if (pListItem != NULL) {
				pListItem->OptionTemplate<Box>::Selected(false, bTrigger);
			}
		}
		m_iCurSel = -1;
	}
	if (iIndex < 0) {
		if ((m_pWindow != nullptr) && bTrigger) {
			m_pWindow->SendNotify(this, ui::kEventSelect, m_iCurSel, iOldSel);
		}
		return false;
	}
	Control* pControl = GetItemAt(iIndex);
	if (pControl == nullptr) {
		return false;
	}
	if (!pControl->IsVisible()) {
		return false;
	}
	if (!pControl->IsEnabled()) {
		return false;
	}

	ui::ListContainerElement* pListItem = dynamic_cast<ui::ListContainerElement*>(pControl);
	if (pListItem == nullptr) {
		return false;
	}
	m_iCurSel = iIndex;
	pListItem->OptionTemplate<Box>::Selected(true, bTrigger);

	Control* pSelItemControl = GetItemAt(m_iCurSel);
	if (pSelItemControl) {
		ui::UiRect rcItem = pSelItemControl->GetPos();
		EnsureVisible(rcItem);
	}

	if (bTakeFocus) pControl->SetFocus();
	if ((m_pWindow != nullptr) && bTrigger) {
		m_pWindow->SendNotify(this, ui::kEventSelect, m_iCurSel, iOldSel);
	}
	return true;
}

void FilterListBox::Filter(const std::string& utf8_str)
{
	ListElementMatch *item = nullptr;
	for (size_t i = 0; i < (size_t)GetCount(); ++i)
	{
		item = dynamic_cast<ListElementMatch*>(GetItemAt(i));
		if (item){
			if (item->StringMatch(utf8_str)) {
				item->SetFadeVisible(true);
			}
			else {
				item->SetFadeVisible(false);
			}
		}
	}
}

//////////////////////////FilterCombo////////////////////////////
//////////////////////////FilterCombo////////////////////////////

FilterCombo::FilterCombo() :
	m_pComboWnd(nullptr),
	m_iCurSel(-1),
	m_szDropBox(0, 150),
	m_uButtonState(ui::kControlStateNormal),
	m_sDropBoxAttributes(),
	m_bPopupTop(false)
{
	// The trick is to add the items to the new container. Their owner gets
	// reassigned by this operation - which is why it is important to reassign
	// the items back to the righfull owner/manager when the window closes.
	m_pLayout.reset(new FilterListBox(new ui::VLayout));
	m_pLayout->GetLayout()->SetPadding(ui::UiRect(1, 1, 1, 1));
	m_pLayout->SetBkColor(L"bk_wnd_lightcolor");
	m_pLayout->SetBorderColor(L"splitline_level1");
	m_pLayout->SetBorderSize(ui::UiRect(1, 1, 1, 1));
	m_pLayout->EnableScrollBar();
	m_pLayout->ApplyAttributeList(GetDropBoxAttributeList());
	m_pLayout->AttachSelect(nbase::Bind(&FilterCombo::OnSelectItem, this, std::placeholders::_1));

	m_pRichEdit = new ui::RichEdit;
	m_pRichEdit->AttachTextChange(nbase::Bind(&FilterCombo::OnRichEditTextChanged, this, std::placeholders::_1));
	m_pRichEdit->AttachButtonDown(nbase::Bind(&FilterCombo::OnRichEditButtonDown, this, std::placeholders::_1));
	m_pRichEdit->SetClass(L"simple");
	m_pRichEdit->SetFixedWidth(DUI_LENGTH_STRETCH);
	m_pRichEdit->SetFixedHeight(DUI_LENGTH_STRETCH);
	m_pRichEdit->SetMargin({ 1, 1, 1, 1 });
	m_pRichEdit->SetAttribute(L"padding", L"2,3");
	m_pRichEdit->SetFont(L"system_14");
	//m_pRichEdit->SetMouseChildEnabled(false);
	//m_pRichEdit->EnableScrollBar();
	Box::Add(m_pRichEdit);
}

void FilterCombo::HandleMessage(ui::EventArgs& args)
{
	if (args.Type == ui::kEventChar || args.Type == ui::kEventKeyDown)
	{
		m_pRichEdit->HandleMessage(args);
	}
	return __super::HandleMessage(args);
}

bool FilterCombo::Add(Control* pControl)
{
	ListElementMatch *pListElementMatch = dynamic_cast<ListElementMatch*>(pControl);
	if (pListElementMatch)
	{
		
	}
	else
	{
		printf("CheckCombo::Add pControl is not CheckBox object\n");
		ASSERT(0);
		return true;
	}

	m_pLayout->Add(pControl);
	pControl->SetReceivePointerMsg(true);
	m_iCurSel = m_pLayout->GetCurSel();
	return true;
}

bool FilterCombo::Remove(Control * pControl)
{
	bool ret = m_pLayout->Remove(pControl);
	m_iCurSel = m_pLayout->GetCurSel();
	return ret;
}

bool FilterCombo::RemoveAt(size_t iIndex)
{
	bool ret = m_pLayout->RemoveAt((int)iIndex);
	m_iCurSel = m_pLayout->GetCurSel();
	return ret;
}

void FilterCombo::RemoveAll()
{
	m_pLayout->RemoveAll();
	m_iCurSel = -1;
}

void FilterCombo::Activate()
{
	if (!IsActivatable()) {
		return;
	}
	if (m_pComboWnd) {
		return;
	}

	m_pComboWnd = new CFilterComboWnd();
	m_pComboWnd->Init(this);

	//if (m_pComboWnd != NULL) m_pComboWnd->SendNotify(this, kEventClick);
    Invalidate();
}

void FilterCombo::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"dropbox") SetDropBoxAttributeList(strValue);
	else if (strName == L"vscrollbar") {}
	else if (strName == L"dropboxsize")
	{
		ui::CSize szDropBoxSize;
		LPTSTR pstr = NULL;
		szDropBoxSize.cx = wcstol(strValue.c_str(), &pstr, 10); ASSERT(pstr);
		szDropBoxSize.cy = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetDropBoxSize(szDropBoxSize);
	}
	else if (strName == L"popuptop") SetPopupTop(strValue == L"true");
	else Box::SetAttribute(strName, strValue);
}

std::wstring FilterCombo::GetText() const
{
#if 0
    if( m_iCurSel < 0 ) return _T("");
	ListContainerElement* pControl = static_cast<ListContainerElement*>(m_pLayout->GetItemAt(m_iCurSel));
    return pControl->GetText();
#else
	if (m_pRichEdit)
		return m_pRichEdit->GetText();
	return L"";
#endif
}

FilterListBox* FilterCombo::GetListBox() 
{
	return m_pLayout.get(); 
}


std::wstring FilterCombo::GetDropBoxAttributeList()
{
    return m_sDropBoxAttributes;
}

void FilterCombo::SetDropBoxAttributeList(const std::wstring& pstrList)
{
    m_sDropBoxAttributes = pstrList;
}

ui::CSize FilterCombo::GetDropBoxSize() const
{
    return m_szDropBox;
}

void FilterCombo::SetDropBoxSize(ui::CSize szDropBox)
{
	ui::DpiManager::GetInstance()->ScaleSize(szDropBox);
    m_szDropBox = szDropBox;
}

bool FilterCombo::SelectItem(int iIndex)
{
	if (iIndex < 0 || iIndex >= m_pLayout->GetCount() || m_iCurSel == iIndex)
		return false;

	m_iCurSel = iIndex;
	m_pLayout->SelectItem(m_iCurSel);

	return true;
}

ui::Control* FilterCombo::GetItemAt(int iIndex)
{
	return m_pLayout->GetItemAt(iIndex);
}

int FilterCombo::GetCount() const 
{
	return m_pLayout->GetCount(); 
}

void FilterCombo::AttachSelect(const ui::EventCallback& callback)
{ 
	m_pLayout->AttachSelect(callback); 
}

bool FilterCombo::OnSelectItem(ui::EventArgs* /*args*/)
{
	if (m_pComboWnd != nullptr) {
		m_pComboWnd->OnSeleteItem();
	}	
	m_iCurSel = m_pLayout->GetCurSel();
	auto pControl = m_pLayout->GetItemAt(m_iCurSel);
	if (pControl != NULL) {
		pControl->SetState(ui::kControlStateNormal);
	}
	if (m_pComboWnd != nullptr) {
		m_pComboWnd->SendNotify(this, ui::kEventSelect, m_iCurSel, -1);
	}
	ui::ListContainerElement *ele = dynamic_cast<ui::ListContainerElement*>(pControl);
	if (m_pRichEdit && ele)
	{
		m_pRichEdit->SetText(ele->GetText());
	}
	return true;
}

bool FilterCombo::OnRichEditTextChanged(ui::EventArgs* /*args*/)
{
	m_pLayout->Filter(m_pRichEdit->GetUTF8Text());
	if (m_pComboWnd == nullptr)
	{
		return false;
	}
	ui::CSize szDrop = GetDropBoxSize();
	ui::UiRect rcOwner = GetPosWithScrollOffset();
	ui::UiRect rc = rcOwner;
	rc.top = rc.bottom + 1;		// 父窗口left、bottom位置作为弹出窗口起点
	rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
	if (szDrop.cx > 0) rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度

	ui::CSize szAvailable(rc.right - rc.left, rc.bottom - rc.top);
	int cyFixed = 0;
	for (int it = 0; it < GetListBox()->GetCount(); it++) {
		Control* pControl = GetListBox()->GetItemAt(it);
		if (!pControl->IsVisible()) continue;
		ui::CSize sz = pControl->EstimateSize(szAvailable);
		cyFixed += sz.cy;
	}
	cyFixed += 2; // VBox 默认的Padding 调整
	rc.bottom = rc.top + std::min((LONG)cyFixed, szDrop.cy);

	::MapWindowRect(GetWindow()->GetHWND(), HWND_DESKTOP, &rc);

	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(m_pComboWnd->GetHWND(), MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	ui::UiRect rcWork(oMonitor.rcWork);
	if (rc.bottom > rcWork.bottom || IsPopupTop()) {
		rc.left = rcOwner.left;
		rc.right = rcOwner.right;
		if (szDrop.cx > 0) rc.right = rc.left + szDrop.cx;
		rc.top = rcOwner.top - std::min((LONG)cyFixed, szDrop.cy);
		rc.bottom = rcOwner.top;
		::MapWindowRect(GetWindow()->GetHWND(), HWND_DESKTOP, &rc);
	}

	m_pComboWnd->SetPos(rc, true, SWP_NOACTIVATE);
	m_pLayout->SetPos(m_pLayout->GetPos());
	return true;
}

bool FilterCombo::OnRichEditButtonDown(ui::EventArgs* /*args*/)
{
	Activate();
	return true;
}

} // namespace ui
