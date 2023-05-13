#include "ShadowCombo.h"
#include "duilib/Utils/Macros.h"

namespace nim_comp
{
CShadowComboWnd::CShadowComboWnd() {}

CShadowComboWnd::~CShadowComboWnd() {}

void CShadowComboWnd::InitComboWnd(ShadowCombo* pOwner)
{
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return;
    }
    m_pOwner = pOwner;
    m_iOldSel = m_pOwner->GetCurSel();

    // Position the popup window in absolute space
    ui::UiSize szDrop = m_pOwner->GetDropBoxSize();
    ui::UiRect rcOwner = pOwner->GetPosWithScrollOffset(true);
    int iItemHeight = ui::Box::IsValidItemIndex(m_iOldSel) ? pOwner->GetItemAt(m_iOldSel)->GetFixedHeight() : 0;
    int iOffset = iItemHeight * ((int)m_iOldSel + 1);
    if (!ui::Box::IsValidItemIndex(m_iOldSel)) {
        iOffset = iItemHeight;
    }
    iOffset = std::max(iOffset, 0);
    int iScrollPos = pOwner->GetCustomLayout()->GetScrollPos().cy;
    if (iScrollPos > iItemHeight) {
        iOffset = iItemHeight;
    }

    ui::UiRect rc = rcOwner;
    rc.top = rc.bottom;		// 父窗口left、bottom位置作为弹出窗口起点
    rc.top = rc.top - iOffset;
    rc.bottom = rc.top + szDrop.cy;	// 计算弹出窗口高度
    if (szDrop.cx > 0) {
        rc.right = rc.left + szDrop.cx;	// 计算弹出窗口宽度
    }

    ui::UiSize szAvailable(rc.right - rc.left, rc.bottom - rc.top);
    int cyFixed = 0;
    const size_t itemCount = pOwner->GetListBox()->GetItemCount();
    for (int it = 0; it < itemCount; ++it) {
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

    int padding = 2;
    auto listBox = m_pOwner->GetListBox();
    if (listBox) {
        padding = listBox->GetLayout()->GetPadding().top + listBox->GetLayout()->GetPadding().bottom;
    }

    cyFixed += padding; // VBox 默认的Padding 调整
    rc.bottom = rc.top + std::min((LONG)cyFixed, szDrop.cy);
    ui::UiRect shadow_corner = m_pOwner->GetShadowCorner();
    rc.left = rc.left - shadow_corner.left;
    rc.right = rc.right + shadow_corner.right;
    rc.top = rc.top - shadow_corner.top;
    rc.bottom = rc.bottom + shadow_corner.bottom;

    ASSERT(pOwner->GetWindow() != nullptr);
    if (pOwner->GetWindow() == nullptr) {
        return;
    }
    ::MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, &rc);
    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(GetHWND(), MONITOR_DEFAULTTOPRIMARY), &oMonitor);
    ui::UiRect rcWork(oMonitor.rcWork);
    if (rc.bottom > rcWork.bottom || m_pOwner->IsPopupTop()) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        rc.top = rcOwner.bottom - std::min((LONG)cyFixed, szDrop.cy);
        rc.bottom = rcOwner.bottom;

        rc.left = rc.left - shadow_corner.left;
        rc.right = rc.right + shadow_corner.right;
        rc.top = rc.top - shadow_corner.top;
        rc.bottom = rc.bottom + shadow_corner.bottom;
        ::MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, &rc);
    }

    CreateWnd(pOwner->GetWindow()->GetHWND(), L"", WS_POPUP, WS_EX_TOOLWINDOW, true, rc);
    // HACK: Don't deselect the parent's caption
    HWND hWndParent = GetHWND();
    while (::GetParent(hWndParent) != NULL) {
        hWndParent = ::GetParent(hWndParent);
    }
    ::ShowWindow(GetHWND(), SW_SHOW);
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
}

std::wstring CShadowComboWnd::GetWindowClassName() const
{
    return L"CShadowComboWnd";
}

void CShadowComboWnd::OnFinalMessage(HWND hWnd)
{
    if (m_pOwner) {
        m_pOwner->SetCShadowComboWnd(nullptr);
        m_pOwner->Invalidate();
    }
    __super::OnFinalMessage(hWnd);
    delete this;
}

void CShadowComboWnd::OnSeleteItem()
{
    PostMessage(WM_KILLFOCUS);
}

LRESULT CShadowComboWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    bHandled = false;
    if (uMsg == WM_CREATE) {
        this->InitWnd(GetHWND());
        ui::Box* pRoot = new ui::Box;
        pRoot->SetAutoDestroyChild(false);
        pRoot->AddItem(m_pOwner->GetListBox());
        if (!m_pOwner->GetShadowImage().empty()) {
            pRoot->SetBkImage(m_pOwner->GetShadowImage());
        }
        pRoot->GetLayout()->SetPadding(m_pOwner->GetShadowCorner(), false);
        this->AttachBox(pRoot);
        this->SetResourcePath(m_pOwner->GetWindow()->GetResourcePath());
        this->SetShadowAttached(false);
        this->SetRenderTransparent(true);
        bHandled = true;
    }
    else if (uMsg == WM_CLOSE) {
        m_pOwner->SetWindow(m_pOwner->GetWindow(), m_pOwner->GetParent(), false);
        m_pOwner->SetPos(m_pOwner->GetPos());
        m_pOwner->SetFocus();
    }
    else if (uMsg == WM_KILLFOCUS) {
        if (GetHWND() != (HWND)wParam) {
            m_pOwner->SelectItemInternal(m_pOwner->GetListBox()->GetCurSel());
            ((ui::Box*)this->GetRoot())->RemoveItemAt(0);
            m_pOwner->GetListBox()->PlaceHolder::SetWindow(nullptr, nullptr, false);
            PostMessage(WM_CLOSE);
        }
    }
    else if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        PostMessage(WM_CLOSE);
    }
    LRESULT lResult = 0;
    if (!bHandled) {
        lResult = __super::OnWindowMessage(uMsg, wParam, lParam, bHandled);
    }
    return lResult;
}

/////////////////////////////////////////////////////////////////////////////
// ShadowCombo
ShadowCombo::ShadowCombo(): 
    m_pWindow(nullptr),
    m_iCurSel(ui::Box::InvalidIndex),
    m_sDropBoxAttributes(),
    m_bPopupTop(false),
    m_sShadowImage(L"file = '../public/bk/bk_combo_shadow.png' corner = '5,4,5,6'"),
    m_cArrow(nullptr),
    m_bInit(false) {
    SetDropBoxSize(ui::UiSize(0, 150));
    SetShadowCorner({ 5,4,5,6 });
    SetArrowOffset(10);

    m_pLayout.reset(new ui::ListBox(new ui::VLayout));
    m_pLayout->SetBkColor(L"white");
    m_pLayout->SetAutoDestroyChild(false);
    m_pLayout->EnableScrollBar();
    m_pLayout->ApplyAttributeList(GetDropBoxAttributeList());
    m_pLayout->AttachSelect(nbase::Bind(&ShadowCombo::OnSelectItem, this, std::placeholders::_1));
    SetStateImage(ui::kControlStateNormal, L"../public/combo/combo_normal.svg");
    SetStateImage(ui::kControlStateHot, L"../public/combo/combo_hot.svg");

    m_cArrow = new ui::Control;
    m_cArrow->SetStateImage(ui::kControlStateNormal, L"../public/combo/arrow_normal.svg");
    m_cArrow->SetStateImage(ui::kControlStateHot, L"../public/combo/arrow_hot.svg");
    m_cArrow->SetFixedWidth(DUI_LENGTH_AUTO, true, true);
    m_cArrow->SetFixedHeight(DUI_LENGTH_AUTO, true);
}

ShadowCombo::~ShadowCombo() 
{
    delete m_cArrow;
    m_cArrow = nullptr;
}

void ShadowCombo::DoInit() 
{
    __super::DoInit();

    if (!m_bInit) {
        m_bInit = true;

        m_cArrow->SetWindow(GetWindow());
        AttachResize(ToWeakCallback([this](const ui::EventArgs& /*args*/) {
            ui::UiRect rect = GetRect();
            ui::UiSize ArrrowSize = m_cArrow->EstimateSize(ui::UiSize(GetRect().GetWidth(), GetRect().GetHeight()));
            rect.top = GetRect().top + (GetRect().GetHeight() - ArrrowSize.cy) / 2;
            rect.bottom = rect.top + ArrrowSize.cy;
            rect.left = GetRect().right - ArrrowSize.cx - m_iArrowOffset;
            rect.right = rect.left + ArrrowSize.cx;
            m_cArrow->SetPos(rect);
            return true;
            }));
    }
}

std::wstring ShadowCombo::GetType() const { return L"ShadowCombo"; }

#if defined(ENABLE_UIAUTOMATION)
UIAControlProvider* ShadowCombo::GetUIAProvider()
{
    if (m_pUIAProvider == nullptr)
    {
        m_pUIAProvider = static_cast<UIAControlProvider*>(new (std::nothrow) UIAComboBoxProvider(this));
    }
    return m_pUIAProvider;
}
#endif

bool ShadowCombo::AddItem(Control* pControl)
{
    m_pLayout->AddItem(pControl);
    m_iCurSel = m_pLayout->GetCurSel();
    return true;
}

bool ShadowCombo::RemoveItem(Control* pControl)
{
    bool ret = m_pLayout->RemoveItem(pControl);
    m_iCurSel = m_pLayout->GetCurSel();
    return ret;
}

bool ShadowCombo::RemoveItemAt(size_t iIndex)
{
    bool ret = m_pLayout->RemoveItemAt((int)iIndex);
    m_iCurSel = m_pLayout->GetCurSel();
    return ret;
}

void ShadowCombo::RemoveAllItems()
{
    m_pLayout->RemoveAllItems();
    m_iCurSel = ui::Box::InvalidIndex;
}

void ShadowCombo::Activate()
{
    if (!IsActivatable()) {
        return;
    }
    if (m_pWindow != nullptr) {
        return;
    }

    m_pWindow = new CShadowComboWnd;
    m_pWindow->InitComboWnd(this);
    m_pWindow->AttachWindowClose(ToWeakCallback([this](const ui::EventArgs& msg) {
        FireAllEvents(msg);
        return true;
        }));
    Invalidate();
}

void ShadowCombo::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"dropbox") SetDropBoxAttributeList(strValue);
    else if (strName == L"vscrollbar") {}
    else if (strName == L"dropboxsize")
    {
        ui::UiSize szDropBoxSize;
        LPTSTR pstr = NULL;
        szDropBoxSize.cx = wcstol(strValue.c_str(), &pstr, 10); ASSERT(pstr);
        szDropBoxSize.cy = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
        SetDropBoxSize(szDropBoxSize);
    }
    else if (strName == L"popuptop") SetPopupTop(strValue == L"true");
    else if (strName == L"textpadding") {
        ui::UiRect rcTextPadding;
        LPTSTR pstr = NULL;
        rcTextPadding.left = wcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
        rcTextPadding.top = wcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
        rcTextPadding.right = wcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
        rcTextPadding.bottom = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
        SetTextPadding(rcTextPadding);
    }
    else if (strName == L"shadowcorner") {
        ui::UiRect rcShadowCorner;
        LPTSTR pstr = NULL;
        rcShadowCorner.left = wcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
        rcShadowCorner.top = wcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
        rcShadowCorner.right = wcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
        rcShadowCorner.bottom = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
        SetShadowCorner(rcShadowCorner);
    }
    else if (strName == L"arrownormalimage") {
        m_cArrow->SetStateImage(ui::kControlStateNormal, strValue);
    }
    else if (strName == L"arrowhotimage") {
        m_cArrow->SetStateImage(ui::kControlStateHot, strValue);
    }
    else if (strName == L"arrowpushedimage") {
        m_cArrow->SetStateImage(ui::kControlStatePushed, strValue);
    }
    else if (strName == L"arrowdisabledimage") {
        m_cArrow->SetStateImage(ui::kControlStateDisabled, strValue);
    }
    else if (strName == L"arrowoffset") {
        SetArrowOffset(_wtoi(strValue.c_str()));
    }

    else Box::SetAttribute(strName, strValue);
}

void ShadowCombo::PaintText(ui::IRenderContext* pRender)
{
    ui::UiRect rcText = GetRect();
    rcText.right = m_cArrow->GetPos().left;

    if (!ui::Box::IsValidItemIndex(m_iCurSel)) {
        return;
    }

    Control* pControl = static_cast<Control*>((m_pLayout->GetItemAt(m_iCurSel)));
    ui::ListBoxElement* pElement = dynamic_cast<ui::ListBoxElement*>(pControl);
    ASSERT(pElement);
    if (pElement == nullptr) {
        return;
    }
    ui::UiRect rcPadding = m_rcTextPadding;

    if (GetText().empty()) {
        return;
    }

    if (pElement->GetOwner() == nullptr) {
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
    dwTextColor = this->GetWindowColor(pElement->GetStateTextColor(ui::kControlStateNormal));
    pRender->DrawText(rcText, GetText(), dwTextColor, pElement->GetFont(), DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
}

void ShadowCombo::PaintChild(ui::IRenderContext* pRender, const ui::UiRect& rcPaint)
{
    __super::PaintChild(pRender, rcPaint);
    if (m_cArrow->GetState() != GetState()) {
        m_cArrow->SetState(GetState());
    }
    m_cArrow->Paint(pRender, rcPaint);
}

std::wstring ShadowCombo::GetText() const
{
    if (!ui::Box::IsValidItemIndex(m_iCurSel)) {
        return std::wstring();
    }
    ui::ListBoxElement* pControl = static_cast<ui::ListBoxElement*>(m_pLayout->GetItemAt(m_iCurSel));
    return pControl ? pControl->GetText() : std::wstring();
}

ui::UiRect ShadowCombo::GetTextPadding() const
{
    return m_rcTextPadding;
}

void ShadowCombo::SetTextPadding(ui::UiRect rc)
{
    ui::DpiManager::GetInstance()->ScaleRect(rc);
    m_rcTextPadding = rc;
    this->Invalidate();
}

std::wstring ShadowCombo::GetDropBoxAttributeList()
{
    return m_sDropBoxAttributes;
}

void ShadowCombo::SetDropBoxAttributeList(const std::wstring& pstrList)
{
    m_sDropBoxAttributes = pstrList;
    m_pLayout->ApplyAttributeList(pstrList);
}

ui::UiSize ShadowCombo::GetDropBoxSize() const
{
    return m_szDropBox;
}

void ShadowCombo::SetDropBoxSize(ui::UiSize szDropBox)
{
    ui::DpiManager::GetInstance()->ScaleSize(szDropBox);
    m_szDropBox = szDropBox;
}

bool ShadowCombo::SelectItemInternal(size_t iIndex)
{
    if (!ui::Box::IsValidItemIndex(iIndex) || iIndex >= m_pLayout->GetItemCount()) {
        return false;
    }

    size_t iOldSel = m_iCurSel;
    m_iCurSel = iIndex;
    m_pLayout->SelectItem(m_iCurSel, false, false);

    //add by djj below
    SendEvent(ui::kEventSelect, m_iCurSel, iOldSel);

#if defined(ENABLE_UIAUTOMATION)
    if (m_pUIAProvider != nullptr && UiaClientsAreListening()) {
        VARIANT vtOld = { 0 }, vtNew = { 0 };
        vtOld.vt = vtNew.vt = VT_BSTR;
        ListBoxElement* pControl = static_cast<ListBoxElement*>(m_pLayout->GetItemAt(m_iCurSel));
        vtOld.bstrVal = SysAllocString(pControl ? pControl->GetText().c_str() : L"");
        vtNew.bstrVal = SysAllocString(GetText().c_str());

        UiaRaiseAutomationPropertyChangedEvent(m_pUIAProvider, UIA_ValueValuePropertyId, vtOld, vtNew);
    }
#endif

    Invalidate();

    return true;
}
bool ShadowCombo::SelectItem(size_t iIndex, bool bTrigger)
{
    m_pLayout->SelectItem(iIndex, false, false);
    if (!SelectItemInternal(iIndex)) {
        return false;
    }
    Invalidate();
    if (bTrigger) {
        SendEvent(ui::kEventSelect, m_iCurSel, -1);
    }
    return true;
}

ui::Control* ShadowCombo::GetItemAt(size_t iIndex) const
{
    return m_pLayout->GetItemAt(iIndex);
}

bool ShadowCombo::OnSelectItem(const ui::EventArgs& /*args*/)
{
    if (m_pWindow) {
        m_pWindow->OnSeleteItem();
    }
    size_t iOldSel = m_iCurSel;
    m_iCurSel = m_pLayout->GetCurSel();
    auto pControl = m_pLayout->GetItemAt(m_iCurSel);
    if (pControl != nullptr) {
        pControl->SetState(ui::kControlStateNormal);
    }
    SendEvent(ui::kEventSelect, m_iCurSel, iOldSel);
    return true;
}

void ShadowCombo::SetShadowCorner(const ui::UiRect& rect, bool bNeedDpiScale)
{
    ui::UiRect rc = rect;
    if (bNeedDpiScale) {
        ui::DpiManager::GetInstance()->ScaleRect(rc);
    }
    m_rcShadowCorner = rc;
}

void ShadowCombo::SetArrowOffset(int offset, bool bNeedDpiScale) 
{
    if (bNeedDpiScale) {
        ui::DpiManager::GetInstance()->ScaleInt(offset);
    }
    m_iArrowOffset = offset;
}

}
