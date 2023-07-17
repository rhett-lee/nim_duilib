#include "ShadowCombo.h"
#include "duilib/Utils/Macros.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/UiColor.h"

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
    ui::UiRect rcOwner = pOwner->GetPos(true);
    ui::UiPoint scrollBoxOffset = pOwner->GetScrollOffsetInScrollBox();
    rcOwner.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);

    int iItemHeight = ui::Box::IsValidItemIndex(m_iOldSel) ? pOwner->GetItemAt(m_iOldSel)->GetFixedHeight() : 0;
    int iOffset = iItemHeight * ((int)m_iOldSel + 1);
    if (!ui::Box::IsValidItemIndex(m_iOldSel)) {
        iOffset = iItemHeight;
    }
    iOffset = std::max(iOffset, 0);
    int64_t iScrollPos = pOwner->GetCustomLayout()->GetScrollPos().cy;
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
    rc.bottom = rc.top + std::min(cyFixed, szDrop.cy);
    ui::UiRect shadow_corner = m_pOwner->GetShadowCorner();
    rc.left = rc.left - shadow_corner.left;
    rc.right = rc.right + shadow_corner.right;
    rc.top = rc.top - shadow_corner.top;
    rc.bottom = rc.bottom + shadow_corner.bottom;

    ASSERT(pOwner->GetWindow() != nullptr);
    if (pOwner->GetWindow() == nullptr) {
        return;
    }
    MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, rc);
    ui::UiRect rcWork;
    GetMonitorWorkRect(rcWork);
    if (rc.bottom > rcWork.bottom || m_pOwner->IsPopupTop()) {
        rc.left = rcOwner.left;
        rc.right = rcOwner.right;
        rc.top = rcOwner.bottom - std::min(cyFixed, szDrop.cy);
        rc.bottom = rcOwner.bottom;

        rc.left = rc.left - shadow_corner.left;
        rc.right = rc.right + shadow_corner.right;
        rc.top = rc.top - shadow_corner.top;
        rc.bottom = rc.bottom + shadow_corner.bottom;
        MapWindowRect(pOwner->GetWindow()->GetHWND(), HWND_DESKTOP, rc);
    }

    CreateWnd(pOwner->GetWindow()->GetHWND(), L"", WS_POPUP, WS_EX_TOOLWINDOW, rc);
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
            ui::UiSize ArrrowSize = m_cArrow->EstimateSize(ui::UiSize(GetRect().Width(), GetRect().Height()));
            rect.top = GetRect().top + (GetRect().Height() - ArrrowSize.cy) / 2;
            rect.bottom = rect.top + ArrrowSize.cy;
            rect.left = GetRect().right - ArrrowSize.cx - m_iArrowOffset;
            rect.right = rect.left + ArrrowSize.cx;
            m_cArrow->SetPos(rect);
            return true;
            }));
    }
}

std::wstring ShadowCombo::GetType() const { return L"ShadowCombo"; }

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
    else if ((strName == L"text_padding") || (strName == L"textpadding")) {
        ui::UiRect rcTextPadding;
        ui::AttributeUtil::ParseRectValue(strValue.c_str(), rcTextPadding);
        SetTextPadding(rcTextPadding);
    }
    else if ((strName == L"shadow_corner") || (strName == L"shadowcorner")) {
        ui::UiRect rcShadowCorner;
        ui::AttributeUtil::ParseRectValue(strValue.c_str(), rcShadowCorner);
        SetShadowCorner(rcShadowCorner);
    }
    else if ((strName == L"arrow_normal_image") || (strName == L"arrownormalimage")) {
        m_cArrow->SetStateImage(ui::kControlStateNormal, strValue);
    }
    else if ((strName == L"arrow_hot_image") || (strName == L"arrowhotimage")) {
        m_cArrow->SetStateImage(ui::kControlStateHot, strValue);
    }
    else if ((strName == L"arrow_pushed_image") || (strName == L"arrowpushedimage")) {
        m_cArrow->SetStateImage(ui::kControlStatePushed, strValue);
    }
    else if ((strName == L"arrow_disabled_image") || (strName == L"arrowdisabledimage")) {
        m_cArrow->SetStateImage(ui::kControlStateDisabled, strValue);
    }
    else if ((strName == L"arrow_offset") || (strName == L"arrowoffset")) {
        SetArrowOffset(_wtoi(strValue.c_str()));
    }
    else {
        Box::SetAttribute(strName, strValue);
    }
}

void ShadowCombo::PaintText(ui::IRender* pRender)
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

    ui::UiColor dwTextColor = this->GetUiColor(pElement->GetStateTextColor(ui::kControlStateNormal));
    pRender->DrawString(rcText, GetText(), dwTextColor, pElement->GetFontId(), ui::TEXT_SINGLELINE | ui::TEXT_VCENTER | ui::TEXT_END_ELLIPSIS);
}

void ShadowCombo::PaintChild(ui::IRender* pRender, const ui::UiRect& rcPaint)
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
    ui::GlobalManager::Instance().Dpi().ScaleRect(rc);
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
    ui::GlobalManager::Instance().Dpi().ScaleSize(szDropBox);
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
        ui::GlobalManager::Instance().Dpi().ScaleRect(rc);
    }
    m_rcShadowCorner = rc;
}

void ShadowCombo::SetArrowOffset(int offset, bool bNeedDpiScale) 
{
    if (bNeedDpiScale) {
        ui::GlobalManager::Instance().Dpi().ScaleInt(offset);
    }
    m_iArrowOffset = offset;
}

}
