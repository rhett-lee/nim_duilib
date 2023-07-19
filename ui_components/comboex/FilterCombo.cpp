#include "FilterCombo.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Utils/AttributeUtil.h"
#include "base/util/string_util.h"

namespace nim_comp
{

class CFilterComboWnd: 
	public ui::Window
{
public:
    void InitComboWnd(FilterCombo* pOwner);
    virtual std::wstring GetWindowClassName() const override;
	virtual void OnFinalMessage(HWND hWnd) override;
	virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	void OnSeleteItem();

private:
    FilterCombo *m_pOwner = nullptr;
    size_t m_iOldSel = ui::Box::InvalidIndex;
};

void CFilterComboWnd::InitComboWnd(FilterCombo* pOwner)
{
	ASSERT(pOwner != nullptr);
	if (pOwner == nullptr) {
		return;
	}
    m_pOwner = pOwner;
    m_iOldSel = m_pOwner->GetCurSel();

    // Position the popup window in absolute space
	ui::UiSize szDrop = m_pOwner->GetDropBoxSize();
	ui::UiRect rcOwner = pOwner->GetPos(true);
	ui::UiPoint scrollBoxOffset = pOwner->GetScrollOffsetInScrollBox();
	rcOwner.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);

	ui::UiRect rc = rcOwner;
    rc.top = rc.bottom + 1;		// 父窗口left、bottom位置作为弹出窗口起点
    rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
	if (szDrop.cx > 0) {
		rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度
	}

	ui::UiSize szAvailable(rc.right - rc.left, rc.bottom - rc.top);
    int cyFixed = 0;
	for (size_t it = 0; it < pOwner->GetListBox()->GetItemCount(); ++it) {
		ui::Control* pControl = pOwner->GetListBox()->GetItemAt(it);
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
		ui::UiSize sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
    }
    cyFixed += 2; // VBox 默认的Padding 调整
    rc.bottom = rc.top + std::min(cyFixed, szDrop.cy);

	ASSERT(pOwner->GetWindow() != nullptr);
    MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, rc);
	ui::UiRect rcWork;
	GetMonitorWorkRect(rcWork);
    if( rc.bottom > rcWork.bottom || m_pOwner->IsPopupTop()) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;
        rc.top = rcOwner.top - std::min(cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
        MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, rc);
    }
    
    CreateWnd(pOwner->GetWindow()->GetHWND(), L"", WS_POPUP, WS_EX_TOOLWINDOW, rc);
    ::ShowWindow(GetHWND(), SW_SHOW);
}

std::wstring CFilterComboWnd::GetWindowClassName() const
{
    return L"ComboWnd";
}

void CFilterComboWnd::OnFinalMessage(HWND /*hWnd*/)
{
	if (m_pOwner != nullptr) {
		m_pOwner->m_pComboWnd = nullptr;
		m_pOwner->SetState(ui::kControlStateNormal);
		m_pOwner->Invalidate();		
	}
	delete this;
}

void CFilterComboWnd::OnSeleteItem()
{
	PostMessage(WM_KILLFOCUS);
}

LRESULT CFilterComboWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	bHandled = false;
    if( uMsg == WM_CREATE ) {
		this->InitWnd(GetHWND());
		ui::Box* pRoot = new ui::Box;
		pRoot->SetAutoDestroyChild(false);
		pRoot->AddItem(m_pOwner->GetListBox());
		m_pOwner->GetListBox()->SetFilterComboWnd(this);
		this->AttachBox(pRoot);
		this->SetResourcePath(m_pOwner->GetWindow()->GetResourcePath());
		this->SetShadowAttached(false);
		bHandled = true;
    }
    else if( uMsg == WM_CLOSE ) {
        m_pOwner->SetWindow(m_pOwner->GetWindow(), m_pOwner->GetParent(), false);
        m_pOwner->SetPos(m_pOwner->GetPos());
        m_pOwner->SetFocus();
    }
    else if( uMsg == WM_KILLFOCUS ) {
		if (GetHWND() != (HWND)wParam)	{
			if ((m_pOwner != nullptr) && (m_pOwner->GetListBox() != nullptr)){
				m_pOwner->SelectItem(m_pOwner->GetListBox()->GetCurSel());
				((ui::Box*)this->GetRoot())->RemoveItemAt(0);
				m_pOwner->GetListBox()->PlaceHolder::SetWindow(nullptr, nullptr, false);
				m_pOwner->GetListBox()->SetFilterComboWnd(nullptr);
			}
			PostMessage(WM_CLOSE);
		}
    }
	else if (uMsg == WM_IME_STARTCOMPOSITION) {
		if (m_pOwner) {
			m_pOwner->SendEvent(ui::kEventImeStartComposition, wParam, lParam, static_cast<TCHAR>(wParam));
		}
	}
	else if (uMsg == WM_IME_ENDCOMPOSITION) {
		if (m_pOwner) {
			m_pOwner->SendEvent(ui::kEventImeEndComposition, wParam, lParam, static_cast<TCHAR>(wParam));
		}
	}
#if 1
	else if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
		PostMessage(WM_CLOSE);
	}
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
			m_pOwner->HandleEvent(args);
		}
	}
#endif
	LRESULT lResult = 0;
	if (!bHandled) {
		lResult = __super::OnWindowMessage(uMsg, wParam, lParam, bHandled);
	}
	return lResult;
}

/////////////////////FilterListBox//////////////////////////
/////////////////////FilterListBox//////////////////////////

bool ListElementMatch::StringMatch(const std::string& utf8str)
{
	std::string utf8text = nbase::MakeLowerString(GetUTF8Text());
	std::string utf8date = nbase::MakeLowerString(GetUTF8DataID());
	std::string searchkey = nbase::MakeLowerString(utf8str);

	if (utf8text.find(searchkey) != std::string::npos || utf8date.find(searchkey) != std::string::npos)	{
		return true;
	}
	return false;
}


bool FilterListBox::SelectItem(size_t iIndex, bool bTakeFocus, bool bTrigger)
{
	if (iIndex == m_iCurSel) {
		return true;
	}
	size_t iOldSel = m_iCurSel;
	// We should first unselect the currently selected item
	if (Box::IsValidItemIndex(m_iCurSel)) {
		Control* pControl = GetItemAt(m_iCurSel);
		if (pControl != NULL) {
			ui::ListBoxElement* pListItem = dynamic_cast<ui::ListBoxElement*>(pControl);
			if (pListItem != NULL) {
				pListItem->OptionTemplate<Box>::Selected(false, bTrigger);
			}
		}
		m_iCurSel = Box::InvalidIndex;
	}
	if (!Box::IsValidItemIndex(iIndex)) {
		if (bTrigger) {
			SendEvent(ui::kEventSelect, m_iCurSel, iOldSel);
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

	ui::ListBoxElement* pListItem = dynamic_cast<ui::ListBoxElement*>(pControl);
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
	if (bTrigger) {
		SendEvent(ui::kEventSelect, m_iCurSel, iOldSel);
	}
	return true;
}

void FilterListBox::Filter(const std::string& utf8_str)
{
	ListElementMatch *item = nullptr;
	const size_t itemCount = GetItemCount();
	for (size_t i = 0; i < itemCount; ++i) {
		item = dynamic_cast<ListElementMatch*>(GetItemAt(i));
		if (item != nullptr) {
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
	m_iCurSel(Box::InvalidIndex),
	m_szDropBox(0, 150),
	m_sDropBoxAttributes(),
	m_bPopupTop(false)
{
	//需要调用设置函数，内部有DPI自适应的逻辑调整大小
	SetDropBoxSize({ 0, 150 });
	// The trick is to add the items to the new container. Their owner gets
	// reassigned by this operation - which is why it is important to reassign
	// the items back to the righfull owner/manager when the window closes.
	m_pLayout.reset(new FilterListBox(new ui::VLayout));
	m_pLayout->GetLayout()->SetPadding(ui::UiPadding(1, 1, 1, 1), true);
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
	m_pRichEdit->SetFixedWidth(DUI_LENGTH_STRETCH, true, true);
	m_pRichEdit->SetFixedHeight(DUI_LENGTH_STRETCH, true);
	m_pRichEdit->SetMargin({ 1, 1, 1, 1 }, true);
	m_pRichEdit->SetAttribute(L"padding", L"2,3");
	m_pRichEdit->SetFontId(L"system_14");
	//m_pRichEdit->SetMouseChildEnabled(false);
	//m_pRichEdit->EnableScrollBar();
	Box::AddItem(m_pRichEdit);
}

void FilterCombo::HandleEvent(const ui::EventArgs& args)
{
	if ((args.Type == ui::kEventChar) || 
		(args.Type == ui::kEventKeyDown) || 
		(args.Type == ui::kEventImeStartComposition) ||
		(args.Type == ui::kEventImeEndComposition)) {
		m_pRichEdit->HandleEvent(args);
	}
	else {
		return __super::HandleEvent(args);
	}
}

bool FilterCombo::AddItem(Control* pControl)
{
	ListElementMatch* pListElementMatch = dynamic_cast<ListElementMatch*>(pControl);
	if (pListElementMatch == nullptr) {
		printf("CheckCombo::AddItem pControl is not CheckBox object\n");
		ASSERT(0);
		return true;
	}

	m_pLayout->AddItem(pControl);
	m_iCurSel = m_pLayout->GetCurSel();
	return true;
}

bool FilterCombo::RemoveItem(Control * pControl)
{
	bool ret = m_pLayout->RemoveItem(pControl);
	m_iCurSel = m_pLayout->GetCurSel();
	return ret;
}

bool FilterCombo::RemoveItemAt(size_t iIndex)
{
	bool ret = m_pLayout->RemoveItemAt(iIndex);
	m_iCurSel = m_pLayout->GetCurSel();
	return ret;
}

void FilterCombo::RemoveAllItems()
{
	m_pLayout->RemoveAllItems();
	m_iCurSel = Box::InvalidIndex;
}

void FilterCombo::Activate()
{
	if (!IsActivatable()) {
		return;
	}
	if (m_pComboWnd != nullptr) {
		return;
	}

	m_pComboWnd = new CFilterComboWnd();
	m_pComboWnd->InitComboWnd(this);
	m_pComboWnd->AttachWindowClose(ToWeakCallback([this](const ui::EventArgs& msg) {
		FireAllEvents(msg);
		return true;
	}));
    Invalidate();
}

void FilterCombo::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"dropbox") {
		SetDropBoxAttributeList(strValue);
	}
	else if (strName == L"vscrollbar") 
	{
	}
	else if ((strName == L"dropbox_size") || (strName == L"dropboxsize")) {
		ui::UiSize szDropBoxSize;
		ui::AttributeUtil::ParseSizeValue(strValue.c_str(), szDropBoxSize);
		SetDropBoxSize(szDropBoxSize);
	}
	else if ((strName == L"popup_top") || (strName == L"popuptop")) {
		SetPopupTop(strValue == L"true");
	}
	else {
		Box::SetAttribute(strName, strValue);
	}
}

std::wstring FilterCombo::GetText() const
{
#if 0
    if( m_iCurSel < 0 ) return _T("");
	ListBoxElement* pControl = static_cast<ListBoxElement*>(m_pLayout->GetItemAt(m_iCurSel));
    return pControl->GetText();
#else
	if (m_pRichEdit != nullptr) {
		return m_pRichEdit->GetText();
	}
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

ui::UiSize FilterCombo::GetDropBoxSize() const
{
    return m_szDropBox;
}

void FilterCombo::SetDropBoxSize(ui::UiSize szDropBox)
{
	ui::GlobalManager::Instance().Dpi().ScaleSize(szDropBox);
    m_szDropBox = szDropBox;
}

bool FilterCombo::SelectItem(size_t iIndex)
{
	if (!Box::IsValidItemIndex(iIndex) || iIndex >= m_pLayout->GetItemCount() || m_iCurSel == iIndex) {
		return false;
	}
	m_iCurSel = iIndex;
	m_pLayout->SelectItem(m_iCurSel);
	return true;
}

ui::Control* FilterCombo::GetItemAt(size_t iIndex) const
{
	return m_pLayout->GetItemAt(iIndex);
}

size_t FilterCombo::GetItemCount() const 
{
	return m_pLayout->GetItemCount(); 
}

void FilterCombo::AttachSelect(const ui::EventCallback& callback)
{ 
	m_pLayout->AttachSelect(callback); 
}

bool FilterCombo::OnSelectItem(const ui::EventArgs& /*args*/)
{
	if (m_pComboWnd != nullptr) {
		m_pComboWnd->OnSeleteItem();
	}	
	m_iCurSel = m_pLayout->GetCurSel();
	auto pControl = m_pLayout->GetItemAt(m_iCurSel);
	if (pControl != nullptr) {
		pControl->SetState(ui::kControlStateNormal);
	}
	SendEvent(ui::kEventSelect, m_iCurSel, -1);
	ui::ListBoxElement *ele = dynamic_cast<ui::ListBoxElement*>(pControl);
	if (m_pRichEdit && ele)	{
		m_pRichEdit->SetText(ele->GetText());
	}
	return true;
}

bool FilterCombo::OnRichEditTextChanged(const ui::EventArgs& /*args*/)
{
	m_pLayout->Filter(m_pRichEdit->GetUTF8Text());
	if (m_pComboWnd == nullptr)	{
		return false;
	}
	ui::UiSize szDrop = GetDropBoxSize();
	ui::UiRect rcOwner = GetPos(true);
	ui::UiPoint scrollBoxOffset = GetScrollOffsetInScrollBox();
	rcOwner.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);

	ui::UiRect rc = rcOwner;
	rc.top = rc.bottom + 1;		// 父窗口left、bottom位置作为弹出窗口起点
	rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
	if (szDrop.cx > 0) {
		rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度
	}

	ui::UiSize szAvailable(rc.right - rc.left, rc.bottom - rc.top);
	int cyFixed = 0;
	const size_t itemCount = GetListBox()->GetItemCount();
	for (size_t it = 0; it < itemCount; ++it) {
		Control* pControl = GetListBox()->GetItemAt(it);
		if (!pControl->IsVisible()) {
			continue;
		}
		ui::UiSize sz = pControl->EstimateSize(szAvailable);
		cyFixed += sz.cy;
	}
	cyFixed += 2; // VBox 默认的Padding 调整
	rc.bottom = rc.top + std::min(cyFixed, szDrop.cy);

	GetWindow()->MapWindowRect(GetWindow()->GetHWND(), HWND_DESKTOP, rc);
	ui::UiRect rcWork;
	m_pComboWnd->GetMonitorWorkRect(rcWork);
	if (rc.bottom > rcWork.bottom || IsPopupTop()) {
		rc.left = rcOwner.left;
		rc.right = rcOwner.right;
		if (szDrop.cx > 0) rc.right = rc.left + szDrop.cx;
		rc.top = rcOwner.top - std::min(cyFixed, szDrop.cy);
		rc.bottom = rcOwner.top;
		GetWindow()->MapWindowRect(GetWindow()->GetHWND(), HWND_DESKTOP, rc);
	}

	m_pComboWnd->SetPos(rc, true, SWP_NOACTIVATE);
	m_pLayout->SetPos(m_pLayout->GetPos());
	return true;
}

bool FilterCombo::OnRichEditButtonDown(const ui::EventArgs& /*args*/)
{
	Activate();
	return true;
}

} // namespace ui
