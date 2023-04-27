#include "Combo.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Control/List.h"

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
    int m_iOldSel = -1;
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
    CSize szDrop = m_pOwner->GetDropBoxSize();
    UiRect rcOwner = pOwner->GetPosWithScrollOffset();
    UiRect rc = rcOwner;
    rc.top = rc.bottom + 1;		// 父窗口left、bottom位置作为弹出窗口起点
    rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
    if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度

    CSize szAvailable(rc.right - rc.left, rc.bottom - rc.top);
    int cyFixed = 0;
	for (int it = 0; it < pOwner->GetListBox()->GetCount(); it++) {
		Control* pControl = pOwner->GetListBox()->GetItemAt(it);
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
        CSize sz = pControl->EstimateSize(szAvailable);
        cyFixed += sz.cy;
    }

	int padding = 2;
	auto listBox = m_pOwner->GetListBox();
	if (listBox) {
		padding = listBox->GetLayout()->GetPadding().top + listBox->GetLayout()->GetPadding().bottom;
	}

	cyFixed += padding; // VBox 默认的Padding 调整
    rc.bottom = rc.top + std::min((LONG)cyFixed, szDrop.cy);

    ::MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, &rc);

    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTOPRIMARY), &oMonitor);
    UiRect rcWork(oMonitor.rcWork);
    if( rc.bottom > rcWork.bottom || m_pOwner->IsPopupTop()) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        if( szDrop.cx > 0 ) rc.right = rc.left + szDrop.cx;
        rc.top = rcOwner.top - std::min((LONG)cyFixed, szDrop.cy);
        rc.bottom = rcOwner.top;
        ::MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, &rc);
    }
    
    CreateWnd(pOwner->GetWindow()->GetHWND(), L"", WS_POPUP, WS_EX_TOOLWINDOW, true, rc);
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
		m_pOwner->m_pWindow = NULL;
		m_pOwner->m_uButtonState = kControlStateNormal;
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
		pRoot->Add(m_pOwner->GetListBox());
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
			((Box*)this->GetRoot())->RemoveAt(0);
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
	m_iCurSel(-1),
	m_szDropBox(0, 150),
	m_uButtonState(kControlStateNormal),
	m_sDropBoxAttributes(),
	m_bPopupTop(false)
{
	// The trick is to add the items to the new container. Their owner gets
	// reassigned by this operation - which is why it is important to reassign
	// the items back to the righfull owner/manager when the window closes.
	m_pLayout.reset(new ListBox(new VLayout));
	m_pLayout->GetLayout()->SetPadding(UiRect(1, 1, 1, 1));
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

#if defined(ENABLE_UIAUTOMATION)
UIAControlProvider* Combo::GetUIAProvider()
{
	if (m_pUIAProvider == nullptr)
	{
		m_pUIAProvider = static_cast<UIAControlProvider*>(new (std::nothrow) UIAComboBoxProvider(this));
	}
	return m_pUIAProvider;
}
#endif

bool Combo::Add(Control* pControl)
{
	ASSERT(pControl != nullptr);
	if (pControl == nullptr) {
		return false;
	}
	m_pLayout->Add(pControl);
	pControl->SetReceivePointerMsg(true);
	m_iCurSel = m_pLayout->GetCurSel();
	return true;
}

bool Combo::Remove(Control * pControl)
{
	ASSERT(pControl != nullptr);
	if (pControl == nullptr){
		return false;
	}
	bool ret = m_pLayout->Remove(pControl);
	m_iCurSel = m_pLayout->GetCurSel();
	return ret;
}

bool Combo::RemoveAt(size_t iIndex)
{
	bool ret = m_pLayout->RemoveAt((int)iIndex);
	m_iCurSel = m_pLayout->GetCurSel();
	return ret;
}

void Combo::RemoveAll()
{
	m_pLayout->RemoveAll();
	m_iCurSel = -1;
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
	m_pWindow->AttachWindowClose(ToWeakCallback([this](ui::EventArgs* msg) {
		auto callback = m_OnEvent.find(msg->Type);
		if (callback != m_OnEvent.end()) {
			callback->second(msg);
		}
		return true;
	}));

	if (m_pWindow != NULL) m_pWindow->SendNotify(this, kEventClick);
    Invalidate();
}

void Combo::Deactivate()
{
	if (!IsActivatable()) {
		return;
	}
	if (m_pWindow != nullptr) {
		m_pWindow->CloseWnd();
	}	
	Invalidate();
}

bool Combo::IsActivated()
{
	return ((m_pWindow != nullptr) && !m_pWindow->IsClosingWnd());
}

void Combo::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"dropbox") SetDropBoxAttributeList(strValue);
	else if (strName == L"vscrollbar") {}
	else if (strName == L"dropboxsize")
	{
		CSize szDropBoxSize;
		LPTSTR pstr = NULL;
		szDropBoxSize.cx = wcstol(strValue.c_str(), &pstr, 10); ASSERT(pstr);
		szDropBoxSize.cy = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetDropBoxSize(szDropBoxSize);
	}
	else if (strName == L"popuptop") SetPopupTop(strValue == L"true");
	else if (strName == L"textpadding") {
		UiRect rcTextPadding;
		LPTSTR pstr = NULL;
		rcTextPadding.left = wcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
		rcTextPadding.top = wcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		rcTextPadding.right = wcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		rcTextPadding.bottom = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetTextPadding(rcTextPadding);
	}
	else Box::SetAttribute(strName, strValue);
}

void Combo::PaintText(IRenderContext* pRender)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	UiRect rcText = m_rcItem;

	if (m_iCurSel >= 0) {
		Control* pControl = m_pLayout->GetItemAt(m_iCurSel);
		ListContainerElement* pElement = nullptr;
		if (pControl) {
			pElement = dynamic_cast<ListContainerElement*>(pControl);
		}
		ASSERT(pElement != nullptr);
		if (pElement == nullptr) {
			return;
		}			
		UiRect rcPadding = m_rcTextPadding;
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

		DWORD dwTextColor = 0xFF000000;
		dwTextColor = this->GetWindowColor(pElement->GetStateTextColor(kControlStateNormal));
		pRender->DrawText(rcText, GetText(), dwTextColor, pElement->GetFont(), DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
	}
}

bool Combo::CanPlaceCaptionBar() const
{
	return true;
}

std::wstring Combo::GetText() const
{
	if (m_iCurSel < 0) {
		return std::wstring();
	}
	ListContainerElement* pControl = dynamic_cast<ListContainerElement*>(m_pLayout->GetItemAt(m_iCurSel));
	return pControl ? pControl->GetText() : std::wstring();
}

UiRect Combo::GetTextPadding() const
{
	return m_rcTextPadding;
}

void Combo::SetTextPadding(UiRect rc)
{
	DpiManager::GetInstance()->ScaleRect(rc);
	m_rcTextPadding = rc;
	this->Invalidate();
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

CSize Combo::GetDropBoxSize() const
{
    return m_szDropBox;
}

void Combo::SetDropBoxSize(CSize szDropBox)
{
	DpiManager::GetInstance()->ScaleSize(szDropBox);
    m_szDropBox = szDropBox;
}

bool Combo::SelectItemInternal(int iIndex)
{
	if (iIndex < 0 || iIndex >= m_pLayout->GetCount()) {
		return false;
	}
	int iOldSel = m_iCurSel;
	m_iCurSel = iIndex;
	m_pLayout->SelectItem(m_iCurSel, false, false);

	//add by djj below
	if (m_pWindow != nullptr) {
		m_pWindow->SendNotify(this, kEventSelect, m_iCurSel, iOldSel);
	}

#if defined(ENABLE_UIAUTOMATION)
	if (m_pUIAProvider != nullptr && UiaClientsAreListening()) {
		VARIANT vtOld = { 0 };
		VARIANT vtNew = { 0 };
		vtOld.vt = VT_BSTR;
		vtNew.vt = VT_BSTR;
		ListContainerElement* pControl = dynamic_cast<ListContainerElement*>(m_pLayout->GetItemAt(m_iCurSel));
		vtOld.bstrVal = SysAllocString(pControl ? pControl->GetText().c_str() : L"");
		vtNew.bstrVal = SysAllocString(GetText().c_str());

		UiaRaiseAutomationPropertyChangedEvent(m_pUIAProvider, UIA_ValueValuePropertyId, vtOld, vtNew);
	}
#endif

	Invalidate();	

	return true;
}

bool Combo::SelectItem(int iIndex, bool bTrigger)
{
    m_pLayout->SelectItem(iIndex, false, false);
	if (!SelectItemInternal(iIndex)) {
		return false;
	}
    Invalidate();
    if ((m_pWindow != nullptr) && bTrigger) {
        m_pWindow->SendNotify(this, kEventSelect, m_iCurSel, -1);
    }
	return true;
}

Control* Combo::GetItemAt(int iIndex)
{
	return m_pLayout->GetItemAt(iIndex);
}

int Combo::GetCount() const 
{ 
	return m_pLayout->GetCount(); 
}

bool Combo::OnSelectItem(EventArgs* /*args*/)
{
	if (m_pWindow != nullptr) {
		m_pWindow->OnSeleteItem();
	}        
	int iOldSel = m_iCurSel;
	m_iCurSel = m_pLayout->GetCurSel();
	auto pControl = m_pLayout->GetItemAt(m_iCurSel);
	if (pControl != nullptr) {
		pControl->SetState(kControlStateNormal);
	}
	if (m_pWindow != nullptr) {
		m_pWindow->SendNotify(this, kEventSelect, m_iCurSel, iOldSel);
	}
	return true;
}

} // namespace ui
