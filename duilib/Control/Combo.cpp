#include "Combo.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Box/ListBox.h"

namespace ui
{

class CComboWnd : public Window
{
public:
    void InitComboWnd(Combo* pOwner);
    virtual std::wstring GetWindowClassName() const override;
	virtual void OnFinalMessage(HWND hWnd) override;
	virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	void OnSeleteItem();

private:
    Combo* m_pOwner = nullptr;
    size_t m_iOldSel = Box::InvalidIndex;
};


void CComboWnd::InitComboWnd(Combo* pOwner)
{
	ASSERT(pOwner != nullptr);
	if (pOwner == nullptr) {
		return;
	}
    m_pOwner = pOwner;
    m_iOldSel = m_pOwner->GetCurSel();

    // Position the popup window in absolute space
    UiSize szDrop = m_pOwner->GetDropBoxSize();
    UiRect rcOwner = pOwner->GetPos(true);
	UiPoint scrollBoxOffset = pOwner->GetScrollOffsetInScrollBox();
	rcOwner.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);

    UiRect rc = rcOwner;
    rc.top = rc.bottom + 1;		// 父窗口left、bottom位置作为弹出窗口起点
    rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
	if (szDrop.cx > 0) {
		rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度
	}

    UiSize szAvailable(rc.right - rc.left, rc.bottom - rc.top);
    int cyFixed = 0;
	const size_t itemCount = pOwner->GetListBox()->GetItemCount();
	for (size_t it = 0; it < itemCount; ++it) {
		Control* pControl = pOwner->GetListBox()->GetItemAt(it);
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
        UiEstSize estSize = pControl->EstimateSize(szAvailable);
		if (estSize.cy.IsInt32()) {
			cyFixed += estSize.cy.GetInt32();
		}
    }

	int padding = 2;
	auto listBox = m_pOwner->GetListBox();
	if (listBox) {
		padding = listBox->GetLayout()->GetPadding().top + listBox->GetLayout()->GetPadding().bottom;
	}

	cyFixed += padding; // VBox 默认的Padding 调整
    rc.bottom = rc.top + std::min(cyFixed, szDrop.cy);

    MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, rc);

	UiRect rcWork;
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
    // HACK: Don't deselect the parent's caption
    HWND hWndParent = GetHWND();
    while( ::GetParent(hWndParent) != NULL ) 
		hWndParent = ::GetParent(hWndParent);
    ::ShowWindow(GetHWND(), SW_SHOW);
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
}

std::wstring CComboWnd::GetWindowClassName() const
{
    return L"ComboWnd";
}

void CComboWnd::OnFinalMessage(HWND hWnd)
{
	if (m_pOwner)
	{
		m_pOwner->m_pWindow = nullptr;
		m_pOwner->SetState(kControlStateNormal);
		m_pOwner->Invalidate();
	}
	__super::OnFinalMessage(hWnd);
    delete this;
}

void CComboWnd::OnSeleteItem()
{
	PostMessage(WM_KILLFOCUS);
}

LRESULT CComboWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	bHandled = false;
    if( uMsg == WM_CREATE ) {
		this->InitWnd(GetHWND());
		Box* pRoot = new Box;
		pRoot->SetAutoDestroyChild(false);
		pRoot->AddItem(m_pOwner->GetListBox());
		this->AttachBox(pRoot);
		this->SetResourcePath(m_pOwner->GetWindow()->GetResourcePath());
		this->SetShadowAttached(false);
		this->SetRenderTransparent(true);
		bHandled = true;
    }
    else if( uMsg == WM_CLOSE ) {
        m_pOwner->SetWindow(m_pOwner->GetWindow(), m_pOwner->GetParent(), false);
        m_pOwner->SetPos(m_pOwner->GetPos());
        m_pOwner->SetFocus();
    }
    else if( uMsg == WM_KILLFOCUS ) {
		if (GetHWND() != (HWND)wParam)	{
			m_pOwner->SelectItemInternal(m_pOwner->GetListBox()->GetCurSel());
			((Box*)this->GetRoot())->RemoveItemAt(0);
			m_pOwner->GetListBox()->PlaceHolder::SetWindow(nullptr, nullptr, false);
			//采取延迟关闭，从而实现展开时再点击一次是关闭，而不是每次点击都展开。
			CloseWnd();
		}
    }
	else if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
		PostMessage(WM_CLOSE);
	}
	LRESULT lResult = 0;
	if (!bHandled)
	{
		lResult = __super::OnWindowMessage(uMsg, wParam, lParam, bHandled);
	}
	return lResult;
}

////////////////////////////////////////////////////////

Combo::Combo() :
    m_pWindow(nullptr),
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
	m_pLayout.reset(new ListBox(new VLayout));
	m_pLayout->GetLayout()->SetPadding(UiPadding(1, 1, 1, 1), true);
	m_pLayout->SetBkColor(L"bk_wnd_lightcolor");
	m_pLayout->SetBorderColor(L"combobox_border");
	m_pLayout->SetBorderSize(UiRect(1, 1, 1, 1));
	m_pLayout->SetBkColor(L"white");
	m_pLayout->SetBorderColor(L"gray");
	m_pLayout->EnableScrollBar();
	m_pLayout->ApplyAttributeList(GetDropBoxAttributeList());
	m_pLayout->AttachSelect(nbase::Bind(&Combo::OnSelectItem, this, std::placeholders::_1));
}

std::wstring Combo::GetType() const { return DUI_CTR_COMBO; }

bool Combo::AddItem(Control* pControl)
{
	ASSERT(pControl != nullptr);
	if (pControl == nullptr) {
		return false;
	}
	m_pLayout->AddItem(pControl);
	m_iCurSel = m_pLayout->GetCurSel();
	return true;
}

bool Combo::RemoveItem(Control * pControl)
{
	ASSERT(pControl != nullptr);
	if (pControl == nullptr){
		return false;
	}
	bool ret = m_pLayout->RemoveItem(pControl);
	m_iCurSel = m_pLayout->GetCurSel();
	return ret;
}

bool Combo::RemoveItemAt(size_t iIndex)
{
	bool ret = m_pLayout->RemoveItemAt((int)iIndex);
	m_iCurSel = m_pLayout->GetCurSel();
	return ret;
}

void Combo::RemoveAllItems()
{
	m_pLayout->RemoveAllItems();
	m_iCurSel = Box::InvalidIndex;
}

Control* Combo::GetItemAt(size_t iIndex) const
{
	return m_pLayout->GetItemAt(iIndex);
}

size_t Combo::GetItemCount() const
{
	return m_pLayout->GetItemCount();
}

void Combo::Activate()
{
	if (!IsActivatable()) {
		return;
	}
	if (m_pWindow != nullptr) {
		return;
	}

    m_pWindow = new CComboWnd();
    m_pWindow->InitComboWnd(this);
    m_pWindow->AttachWindowClose(ToWeakCallback([this](const ui::EventArgs& msg) {
        FireAllEvents(msg);
        return true;
	}));

    Invalidate();
}

void Combo::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"dropbox") {
		SetDropBoxAttributeList(strValue);
	}
	else if (strName == L"vscrollbar") {
	}
	else if ((strName == L"dropbox_size") || (strName == L"dropboxsize") ){
		UiSize szDropBoxSize;
		AttributeUtil::ParseSizeValue(strValue.c_str(), szDropBoxSize);
		SetDropBoxSize(szDropBoxSize);
	}
	else if ((strName == L"popup_top") || (strName == L"popuptop")){
		SetPopupTop(strValue == L"true");
	}
	else if ((strName == L"text_padding") || (strName == L"textpadding")){
		UiPadding rcTextPadding;
		AttributeUtil::ParsePaddingValue(strValue.c_str(), rcTextPadding);
		SetTextPadding(rcTextPadding);
	}
	else {
		Box::SetAttribute(strName, strValue);
	}
}

void Combo::PaintText(IRender* pRender)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (!Box::IsValidItemIndex(m_iCurSel)) {
		return;
	}
	UiRect rcText = GetRect();	
	Control* pControl = m_pLayout->GetItemAt(m_iCurSel);
	ListBoxElement* pElement = nullptr;
	if (pControl) {
		pElement = dynamic_cast<ListBoxElement*>(pControl);
	}
	ASSERT(pElement != nullptr);
	if (pElement == nullptr) {
		return;
	}			
	UiPadding rcPadding = m_rcTextPadding;
	if (GetText().empty()) {
		return;
	}

	if (pElement->GetOwner() == NULL) {
		return;
	}			

	if (rcPadding.left == 0 && rcPadding.top == 0 && rcPadding.right == 0 && rcPadding.bottom == 0) {
		rcPadding = pElement->GetTextPadding();
	}				
	rcText.left += rcPadding.left;
	rcText.right -= rcPadding.right;
	rcText.top += rcPadding.top;
	rcText.bottom -= rcPadding.bottom;

	UiColor dwTextColor(0xFF000000);
	dwTextColor = this->GetUiColor(pElement->GetStateTextColor(kControlStateNormal));
	pRender->DrawString(rcText, GetText(), dwTextColor, pElement->GetFontId(), TEXT_SINGLELINE | TEXT_VCENTER | TEXT_END_ELLIPSIS);
}

bool Combo::CanPlaceCaptionBar() const
{
	return true;
}

std::wstring Combo::GetText() const
{
	if (!Box::IsValidItemIndex(m_iCurSel)) {
		return std::wstring();
	}
	ListBoxElement* pControl = dynamic_cast<ListBoxElement*>(m_pLayout->GetItemAt(m_iCurSel));
	return pControl ? pControl->GetText() : std::wstring();
}

const UiPadding& Combo::GetTextPadding() const
{
	return m_rcTextPadding;
}

void Combo::SetTextPadding(UiPadding padding)
{
	GlobalManager::Instance().Dpi().ScalePadding(padding);
	if (!m_rcTextPadding.Equals(padding)) {
		m_rcTextPadding = padding;
		this->Invalidate();
	}	
}

std::wstring Combo::GetDropBoxAttributeList()
{
    return m_sDropBoxAttributes;
}

void Combo::SetDropBoxAttributeList(const std::wstring& pstrList)
{
    m_sDropBoxAttributes = pstrList;
	m_pLayout->ApplyAttributeList(pstrList);
}

UiSize Combo::GetDropBoxSize() const
{
    return m_szDropBox;
}

void Combo::SetDropBoxSize(UiSize szDropBox)
{
	GlobalManager::Instance().Dpi().ScaleSize(szDropBox);
    m_szDropBox = szDropBox;
}

bool Combo::SelectItemInternal(size_t iIndex)
{
	if (!Box::IsValidItemIndex(iIndex) || iIndex >= m_pLayout->GetItemCount()) {
		return false;
	}
	size_t iOldSel = m_iCurSel;
	m_iCurSel = iIndex;
	m_pLayout->SelectItem(m_iCurSel, false, false);

	//add by djj below
	SendEvent(kEventSelect, m_iCurSel, iOldSel);
	Invalidate();
	return true;
}

bool Combo::SelectItem(size_t iIndex, bool bTrigger)
{
    m_pLayout->SelectItem(iIndex, false, false);
	if (!SelectItemInternal(iIndex)) {
		return false;
	}
    Invalidate();
    if (bTrigger) {
        SendEvent(kEventSelect, m_iCurSel, -1);
    }
	return true;
}

bool Combo::OnSelectItem(const EventArgs& /*args*/)
{
	if (m_pWindow != nullptr) {
		m_pWindow->OnSeleteItem();
	}        
	size_t iOldSel = m_iCurSel;
	m_iCurSel = m_pLayout->GetCurSel();
	auto pControl = m_pLayout->GetItemAt(m_iCurSel);
	if (pControl != nullptr) {
		pControl->SetState(kControlStateNormal);
	}
	SendEvent(kEventSelect, m_iCurSel, iOldSel);
	return true;
}

} // namespace ui
