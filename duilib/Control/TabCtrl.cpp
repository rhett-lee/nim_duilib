#include "TabCtrl.h"
#include "duilib/Box/TabBox.h"

namespace ui
{

TabCtrl::TabCtrl():
    ListBox(new HLayout),
    m_nSelectedId(Box::InvalidIndex),
    m_pTabBox(nullptr)
{
}

std::wstring TabCtrl::GetType() const { return DUI_CTR_TAB_CTRL; }

void TabCtrl::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"selected_id") {
        int32_t nValue = _wtoi(strValue.c_str());
        if (nValue >= 0) {
            m_nSelectedId = nValue;
        }
    }
    else if (strName == L"tab_box_name") {
        //绑定的TabBox控件名称，绑定后TabCtrl的选择项变化时，TabBox的选择项会跟随变化
        SetTabBoxName(strValue);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void TabCtrl::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();

    //设置为单选
    SetMultiSelect(false);

    //默认选择的子项
    if (m_nSelectedId != Box::InvalidIndex) {
        SelectItem(m_nSelectedId);
    }
}

void TabCtrl::HandleEvent(const EventArgs& msg)
{
    if ((msg.pSender == this) && (msg.Type == kEventSelect)) {
        //尝试设置关联的TabBox
        if ((m_pTabBox == nullptr) && !m_tabBoxName.empty()) {
            SetTabBoxName(m_tabBoxName.c_str());
        }
        if (m_pTabBox != nullptr) {
            TabCtrlItem* pItem = nullptr;
            size_t nSelectIndex = msg.wParam;
            Control* pControl = GetItemAt(nSelectIndex);
            if (pControl != nullptr) {
                pItem = dynamic_cast<TabCtrlItem*>(pControl);
            }
            if (pItem != nullptr) {
                size_t nItemIndex = pItem->GetTabBoxItemIndex();
                if (nItemIndex != Box::InvalidIndex) {
                    m_pTabBox->SelectItem(nItemIndex);
                }
            }
        }
    }
    if ((msg.pSender == this) && ((msg.Type == kEventSelect) || (msg.Type == kEventUnSelect))) {
        TabCtrlItem* pItem = nullptr;
        size_t nSelectIndex = msg.wParam;
        Control* pControl = GetItemAt(nSelectIndex);
        if (pControl != nullptr) {
            pItem = dynamic_cast<TabCtrlItem*>(pControl);
        }
        if (pItem != nullptr) {
            pItem->AdjustItemLineStatus();
        }        
    }
    __super::HandleEvent(msg);
}

void TabCtrl::SetTabBoxName(const std::wstring& tabBoxName)
{
    if (m_tabBoxName != tabBoxName) {
        m_tabBoxName = tabBoxName;
    }    
    if (!tabBoxName.empty() && (GetWindow() != nullptr) ) {
        TabBox* pTabBox = nullptr;
        Control* pSubControl = nullptr;
        Box* pRoot = GetWindow()->GetRoot();
        if (pRoot != nullptr) {
            pSubControl = pRoot->FindSubControl(tabBoxName);
        }
        if (pSubControl != nullptr) {
            pTabBox = dynamic_cast<TabBox*>(pSubControl);            
        }
        if (pTabBox != nullptr) {
            SetTabBox(pTabBox);
        }
    }
}

std::wstring TabCtrl::GetTabBoxName() const
{
    return m_tabBoxName.c_str();
}

void TabCtrl::SetTabBox(TabBox* pTabBox)
{
    m_pTabBox = pTabBox;
}

TabBox* TabCtrl::GetTabBox() const
{
    return m_pTabBox;
}

bool TabCtrl::SetItemIndex(Control* pControl, size_t iIndex)
{
    size_t nOldIndex = GetItemIndex(pControl);
    bool bRet = __super::SetItemIndex(pControl, iIndex);
    if (bRet) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(nOldIndex));
        if (pItem != nullptr) {
            pItem->AdjustItemLineStatus();
        }
        pItem = dynamic_cast<TabCtrlItem*>(pControl);
        if (pItem != nullptr) {
            pItem->AdjustItemLineStatus();
        }
    }
    return bRet;
}

bool TabCtrl::AddItem(Control* pControl)
{
    bool bRet = __super::AddItem(pControl);
    if (bRet) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(pControl);
        if (pItem != nullptr) {
            pItem->AdjustItemLineStatus();
        }
    }
    return bRet;
}

bool TabCtrl::AddItemAt(Control* pControl, size_t iIndex)
{
    bool bRet = __super::AddItemAt(pControl, iIndex);
    if (bRet) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(pControl);
        if (pItem != nullptr) {
            pItem->AdjustItemLineStatus();
        }
    }
    return bRet;
}

bool TabCtrl::RemoveItem(Control* pControl)
{
    size_t iIndex = GetItemIndex(pControl);
    bool bRet = __super::RemoveItem(pControl);
    if (bRet) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(iIndex - 1));
        if (pItem != nullptr) {
            pItem->AdjustItemLineStatus();
        }
    }
    return bRet;
}

bool TabCtrl::RemoveItemAt(size_t iIndex)
{
    bool bRet = __super::RemoveItemAt(iIndex);
    if (bRet) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(iIndex - 1));
        if (pItem != nullptr) {
            pItem->AdjustItemLineStatus();
        }
    }
    return bRet;
}

void TabCtrl::RemoveAllItems()
{
    __super::RemoveAllItems();
}

///////////////////////////////////////////////////////////////////
////
TabCtrlItem::TabCtrlItem():
    m_pIcon(nullptr),
    m_pLabel(nullptr),
    m_pCloseBtn(nullptr),
    m_pLine(nullptr),
    m_bAutoHideCloseBtn(false),
    m_nTabBoxItemIndex(Box::InvalidIndex)
{
    m_rcSelected.cx = (uint8_t)GlobalManager::Instance().Dpi().GetScaleInt(12);
    m_rcSelected.cy = m_rcSelected.cx;

    m_rcHot.cx = (uint8_t)GlobalManager::Instance().Dpi().GetScaleInt(5);
    m_rcHot.cy = m_rcHot.cx;

    m_hotPadding.top = 0;
    m_hotPadding.left = (uint8_t)GlobalManager::Instance().Dpi().GetScaleInt(3);
    m_hotPadding.right = m_hotPadding.left;
    m_hotPadding.bottom = m_hotPadding.left;
}

std::wstring TabCtrlItem::GetType() const { return DUI_CTR_TAB_CTRL_ITEM; }

void TabCtrlItem::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"tab_box_item_index") {
        SetTabBoxItemIndex((size_t)_wtoi(strValue.c_str()));
    }
    else if (strName == L"title") {
        SetTitle(strValue);
    }
    else if (strName == L"icon") {
        SetIcon(strValue);
    }
    else if (strName == L"icon_class") {
        SetIconClass(strValue);
    }
    else if (strName == L"title_class") {
        SetTitleClass(strValue);
    }
    else if (strName == L"close_button_class") {
        SetCloseButtonClass(strValue);
    }
    else if (strName == L"line_class") {
        SetLineClass(strValue);
    }
    else if (strName == L"selected_round_corner") {
        UiSize sz;
        AttributeUtil::ParseSizeValue(strValue.c_str(), sz);
        SetSelectedRoundCorner(sz, true);
    }
    else if (strName == L"hot_round_corner") {
        UiSize sz;
        AttributeUtil::ParseSizeValue(strValue.c_str(), sz);
        SetHotRoundCorner(sz, true);
    }
    else if (strName == L"hot_padding") {
        UiPadding rcPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcPadding);
        SetHotPadding(rcPadding, true);
    }
    else if (strName == L"auto_hide_close_button") {
        SetAutoHideCloseButton(strValue == L"true");
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void TabCtrlItem::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();
    SetIconClass(GetIconClass());
    SetTitleClass(GetTitleClass());
    SetLineClass(GetLineClass());
    SetCloseButtonClass(GetCloseButtonClass());
    if (m_pIcon != nullptr) {
        m_pIcon->SetVisible(!m_pIcon->GetBkImage().empty());
    }
    if (m_pCloseBtn != nullptr) {
        m_pCloseBtn->SetVisible(!IsAutoHideCloseButton() || IsSelected());
    }
    AdjustItemLineStatus();
}

void TabCtrlItem::HandleEvent(const EventArgs& msg)
{
    if ((msg.pSender == this) && (msg.Type == kEventStateChange) && (m_pLine != nullptr)) {
        //处理分割线的状态
        AdjustItemLineStatus();
    }
    __super::HandleEvent(msg);
}

void TabCtrlItem::SetVisible(bool bVisible)
{
    __super::SetVisible(bVisible);
    if (m_pIcon != nullptr) {
        m_pIcon->SetVisible(!m_pIcon->GetBkImage().empty());
    }
    if (IsVisible() && (m_pCloseBtn != nullptr)) {
        m_pCloseBtn->SetVisible(!IsAutoHideCloseButton() || IsSelected());
    }
    AdjustItemLineStatus();
}

std::wstring TabCtrlItem::GetToolTipText() const
{
    std::wstring tooltip = __super::GetToolTipText();
    if (tooltip.empty()) {
        if (m_pLabel != nullptr) {
            tooltip = m_pLabel->GetToolTipText();
        }
    }
    return tooltip;
}

void TabCtrlItem::SetIconClass(const std::wstring& iconClass)
{
    bool bChanged = m_iconClass != iconClass;
    if (bChanged) {
        m_iconClass = iconClass;
    }    
    if (!IsInited()) {
        return;
    }
    if (!iconClass.empty()) {
        if (m_pIcon == nullptr) {
            m_pIcon = new Control;
            m_pIcon->SetWindow(GetWindow());
            m_pIcon->SetClass(iconClass);
            AddItem(m_pIcon);
            if (!m_iconImageString.empty()) {
                m_pIcon->SetBkImage(m_iconImageString.c_str());
            }
            AdjustSubItemIndex();
        }
        else if(bChanged) {
            m_pIcon->SetClass(iconClass);
        }
    }
    else {
        if (m_pIcon != nullptr) {
            RemoveItem(m_pIcon);
            m_pIcon = nullptr;
        }
    }
}

std::wstring TabCtrlItem::GetIconClass() const
{
    return m_iconClass.c_str();
}

void TabCtrlItem::SetTitleClass(const std::wstring& titleClass)
{
    bool bChanged = m_titleClass != titleClass;
    if (bChanged) {
        m_titleClass = titleClass;
    }    
    if (!IsInited()) {
        return;
    }
    if (!titleClass.empty()) {
        if (m_pLabel == nullptr) {
            m_pLabel = new Label;
            m_pLabel->SetWindow(GetWindow());
            m_pLabel->SetClass(titleClass);
            AddItem(m_pLabel);
            if (!m_title.empty()) {
                m_pLabel->SetText(m_title.c_str());
            }
            AdjustSubItemIndex();
        }
        else if (bChanged) {
            m_pLabel->SetClass(titleClass);
        }
    }
    else {
        if (m_pLabel != nullptr) {
            RemoveItem(m_pLabel);
            m_pLabel = nullptr;
        }
    }
}

std::wstring TabCtrlItem::GetTitleClass() const
{
    return m_titleClass.c_str();
}

void TabCtrlItem::SetCloseButtonClass(const std::wstring& closeButtonClass)
{
    bool bChanged = m_closeBtnClass != closeButtonClass;
    if (bChanged) {
        m_closeBtnClass = closeButtonClass;
    }
    if (!IsInited()) {
        return;
    }
    if (!closeButtonClass.empty()) {
        if (m_pCloseBtn == nullptr) {
            m_pCloseBtn = new Button;
            m_pCloseBtn->SetWindow(GetWindow());
            m_pCloseBtn->SetClass(closeButtonClass);
            AddItem(m_pCloseBtn);
            AdjustSubItemIndex();
        }
        else if (bChanged) {
            m_pCloseBtn->SetClass(closeButtonClass);
        }
    }
    else {
        if (m_pCloseBtn != nullptr) {
            RemoveItem(m_pCloseBtn);
            m_pCloseBtn = nullptr;
        }
    }
}

std::wstring TabCtrlItem::GetCloseButtonClass() const
{
    return m_closeBtnClass.c_str();
}

void TabCtrlItem::SetLineClass(const std::wstring& lineClass)
{
    bool bChanged = m_lineClass != lineClass;
    if (bChanged) {
        m_lineClass = lineClass;
    }
    if (!IsInited()) {
        return;
    }
    if (!lineClass.empty()) {
        if (m_pLine == nullptr) {
            m_pLine = new Control;
            m_pLine->SetWindow(GetWindow());
            m_pLine->SetClass(lineClass);
            AddItem(m_pLine);
            AdjustSubItemIndex();
        }
        else if (bChanged) {
            m_pLine->SetClass(lineClass);
        }
    }
    else {
        if (m_pLine != nullptr) {
            RemoveItem(m_pLine);
            m_pLine = nullptr;
        }
    }
}

std::wstring TabCtrlItem::GetLineClass() const
{
    return m_lineClass.c_str();
}

void TabCtrlItem::AdjustSubItemIndex()
{
    std::vector<Control*> subItems;
    if (m_pIcon != nullptr) {
        subItems.push_back(m_pIcon);
    }
    if (m_pLabel != nullptr) {
        subItems.push_back(m_pLabel);
    }
    if ((m_pCloseBtn != nullptr) && !m_pCloseBtn->IsFloat()) {
        subItems.push_back(m_pCloseBtn);
    }
    if (m_pLine != nullptr) {
        subItems.push_back(m_pLine);
    }
    if ((m_pCloseBtn != nullptr) && m_pCloseBtn->IsFloat()) {
        subItems.push_back(m_pCloseBtn);
    }
    for (size_t nIndex = 0; nIndex < subItems.size(); ++nIndex) {
        Control* pControl = subItems[nIndex];
        size_t nCurrentIndex = GetItemIndex(pControl);
        if (nIndex != nCurrentIndex) {
            SetItemIndex(pControl, nIndex);
        }
    }
}

void TabCtrlItem::AdjustItemLineStatus()
{
    if (m_pLine == nullptr) {
        return;
    }
    TabCtrl* pTabCtrl = GetTabCtrl();
    if (pTabCtrl == nullptr) {
        return;
    }
    const size_t nCount = pTabCtrl->GetItemCount();
    if (nCount == 0) {
        return;
    }
    size_t nItem = pTabCtrl->GetItemIndex(this);
    if (nItem >= nCount) {
        return;
    }
    bool bLineVisible = true;
    TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(pTabCtrl->GetItemAt(nItem));
    if ((pItem != nullptr) && (pItem->m_pLine != nullptr)) {        
        ControlStateType state = pItem->GetState();
        if ((state == kControlStateHot) || (state == kControlStatePushed) || pItem->IsSelected()) {
            bLineVisible = false;
        }
        if (pItem->m_pLine->IsVisible() != bLineVisible) {
            pItem->m_pLine->SetVisible(bLineVisible);
        }        
    }
    //当前标签前面一个标签的分割线
    if (nItem > 0) {
        if (!pItem->IsVisible()) {
            bLineVisible = true;
        }
        pItem = dynamic_cast<TabCtrlItem*>(pTabCtrl->GetItemAt(nItem - 1));
        if ((pItem != nullptr) && (pItem->m_pLine != nullptr)) {
            ControlStateType state = pItem->GetState();
            if ((state == kControlStateHot) || (state == kControlStatePushed) || pItem->IsSelected()) {
                bLineVisible = false;
            }
            if (pItem->m_pLine->IsVisible() != bLineVisible) {
                pItem->m_pLine->SetVisible(bLineVisible);
            }
        }
    }

    //最后一个标签，不显示分割线
    nItem = nCount - 1;
    TabCtrlItem* pLastItem = dynamic_cast<TabCtrlItem*>(pTabCtrl->GetItemAt(nItem));
    if ((pLastItem != nullptr) && (pLastItem->m_pLine != nullptr) && pLastItem->m_pLine->IsVisible()) {
        pLastItem->m_pLine->SetVisible(false);
    }
}

TabCtrl* TabCtrlItem::GetTabCtrl() const
{
    TabCtrl* pTabCtrl = dynamic_cast<TabCtrl*>(GetParent());
    return pTabCtrl;
}

void TabCtrlItem::SetSelectedRoundCorner(UiSize szCorner, bool bNeedDpiScale)
{
    ASSERT((szCorner.cx >= 0) && (szCorner.cy >= 0));
    szCorner.Validate();
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleSize(szCorner);
    }
    m_rcSelected.cx = ui::TruncateToUInt8(szCorner.cx);
    m_rcSelected.cy = ui::TruncateToUInt8(szCorner.cy);
}

UiSize TabCtrlItem::GetSelectedRoundCorner() const
{
    UiSize szCorner;
    szCorner.cx = m_rcSelected.cx;
    szCorner.cy = m_rcSelected.cy;
    return szCorner;
}

void TabCtrlItem::SetHotRoundCorner(UiSize szCorner, bool bNeedDpiScale)
{
    ASSERT((szCorner.cx >= 0) && (szCorner.cy >= 0));
    szCorner.Validate();
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleSize(szCorner);
    }
    m_rcHot.cx = ui::TruncateToUInt8(szCorner.cx);
    m_rcHot.cy = ui::TruncateToUInt8(szCorner.cy);
}

UiSize TabCtrlItem::GetHotRoundCorner() const
{
    UiSize szCorner;
    szCorner.cx = m_rcHot.cx;
    szCorner.cy = m_rcHot.cy;
    return szCorner;
}

void TabCtrlItem::SetHotPadding(UiPadding rcPadding, bool bNeedDpiScale)
{
    ASSERT((rcPadding.left >= 0) && (rcPadding.top >= 0) && (rcPadding.right >= 0) && (rcPadding.bottom >= 0));
    rcPadding.Validate();
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScalePadding(rcPadding);
    }
    m_hotPadding.left = TruncateToUInt8(rcPadding.left);
    m_hotPadding.top = TruncateToUInt8(rcPadding.top);
    m_hotPadding.right = TruncateToUInt8(rcPadding.right);
    m_hotPadding.bottom = TruncateToUInt8(rcPadding.bottom);
}

UiPadding TabCtrlItem::GetHotPadding() const
{
    UiPadding rcPadding;
    rcPadding.left = m_hotPadding.left;
    rcPadding.top = m_hotPadding.top;
    rcPadding.right = m_hotPadding.right;
    rcPadding.bottom = m_hotPadding.bottom;
    return rcPadding;
}

void TabCtrlItem::SetAutoHideCloseButton(bool bAutoHideCloseBtn)
{
    if (m_bAutoHideCloseBtn != bAutoHideCloseBtn) {
        m_bAutoHideCloseBtn = bAutoHideCloseBtn;
        if (m_pCloseBtn != nullptr) {
            m_pCloseBtn->SetVisible(!IsAutoHideCloseButton() || IsSelected());
        }
    }
}

bool TabCtrlItem::IsAutoHideCloseButton() const
{
    return m_bAutoHideCloseBtn;
}

bool TabCtrlItem::MouseEnter(const EventArgs& msg)
{
    bool bRet = __super::MouseEnter(msg);
    if (IsAutoHideCloseButton() && (m_pCloseBtn != nullptr)) {
        m_pCloseBtn->SetFadeVisible(true);
    }
    return bRet;
}

bool TabCtrlItem::MouseLeave(const EventArgs& msg)
{
    bool bRet = __super::MouseLeave(msg);
    if (!GetRect().ContainsPt(msg.ptMouse)) {
        if (IsAutoHideCloseButton() && (m_pCloseBtn != nullptr)) {
            m_pCloseBtn->SetFadeVisible(IsSelected());
        }
    }
    return bRet;
}

void TabCtrlItem::OnPrivateSetSelected()
{
    if (IsAutoHideCloseButton() && (m_pCloseBtn != nullptr)) {
        m_pCloseBtn->SetVisible(IsSelected());
    }
}

bool TabCtrlItem::ButtonDown(const EventArgs& msg)
{
    bool bRet = __super::ButtonDown(msg);
    if (!IsSelected() && IsActivatable()) {
        //按鼠标左键的时候，选择
        Selected(true, true);
    }
    return bRet;
}

void TabCtrlItem::PaintStateColors(IRender* pRender)
{
    //绘制标签页的形状（通过颜色体现）
    if (IsSelected()) {
        PaintTabItemSelected(pRender);
    }
    else if ((GetState() == ControlStateType::kControlStateHot) ||
             (GetState() == ControlStateType::kControlStatePushed)) {
        //鼠标悬停状态
        PaintTabItemHot(pRender);
    }
    else {
        __super::PaintStateColors(pRender);
    }
}

void TabCtrlItem::PaintTabItemSelected(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }

    UiSize roundSize(m_rcSelected.cx, m_rcSelected.cy);
    if (rc.Width() < roundSize.cx * 2) {
        roundSize.cx = rc.Width() / 3;
    }

    std::wstring color = GetSelectStateColor(ControlStateType::kControlStateNormal);
    if (color.empty()) {
        return;
    }

    UiColor dwColor = GetUiColor(color);
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(dwColor));
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
        if (brush && path) {
            AddTabItemPath(path.get(), rc, roundSize);
            pRender->FillPath(path.get(), brush.get());
        }
    }
}

void TabCtrlItem::AddTabItemPath(IPath* path, const UiRect& rect, UiSize roundSize) const
{
    ASSERT(path != nullptr);
    if (path == nullptr) {
        return;
    }
    //确保圆角宽度和高度都是偶数
    if ((roundSize.cx % 2) != 0) {
        roundSize.cx += 1;
    }
    if ((roundSize.cy % 2) != 0) {
        roundSize.cy += 1;
    }
    if (rect.Width() < roundSize.cx) {
        //无法绘制
        return;
    }

    UiRect rc = rect;
    rc.left += roundSize.cx / 2;
    rc.right -= roundSize.cx / 2;

    path->AddLine(rc.left, (INT)rc.bottom - roundSize.cy / 2, rc.left, rc.top + roundSize.cy);

    path->AddArc(UiRect((INT)rc.left, rc.top, rc.left + roundSize.cx, rc.top + roundSize.cy), 180, 90);
    path->AddLine(rc.left + roundSize.cx / 2, (INT)rc.top, rc.right - roundSize.cx / 2, rc.top);
    
    path->AddArc(UiRect(rc.right - roundSize.cx, (INT)rc.top, rc.right, rc.top + roundSize.cy), 270, 90);
    path->AddLine((INT)rc.right, rc.top + roundSize.cy / 2, rc.right, rc.bottom - roundSize.cy / 2);

    path->AddArc(UiRect(rc.left - roundSize.cx, rc.bottom - roundSize.cy, rc.left, rc.bottom), 0, 90);
    path->AddArc(UiRect(rc.right, rc.bottom - roundSize.cy, rc.right + roundSize.cx, rc.bottom), 90, 90);
    path->AddLine(rc.left - roundSize.cx / 2, rc.bottom, rc.right + roundSize.cx / 2, rc.bottom);

    path->Close();
}

void TabCtrlItem::PaintTabItemHot(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    rc.top += m_hotPadding.top;
    rc.left += m_hotPadding.left;
    rc.right -= m_hotPadding.right;
    rc.bottom -= m_hotPadding.bottom;
 
    UiSize roundSize(m_rcHot.cx, m_rcHot.cy);
    std::wstring color = GetStateColor(ControlStateType::kControlStateHot);
    if (color.empty()) {
        return;
    }
    UiColor dwColor = GetUiColor(color);
    pRender->FillRoundRect(rc, roundSize, dwColor);
}

void TabCtrlItem::SetIcon(const std::wstring& iconImageString)
{
    Control* pIconControl = GetIconControl();
    if (pIconControl != nullptr) {
        pIconControl->SetBkImage(iconImageString);
        pIconControl->SetVisible(!iconImageString.empty());
        m_iconImageString.clear();
    }
    else {
        m_iconImageString = iconImageString;
    }
}

std::wstring TabCtrlItem::GetIcon() const
{
    std::wstring iconString;
    Control* pIconControl = GetIconControl();
    if (pIconControl != nullptr) {
        iconString = pIconControl->GetBkImage();
    }
    else {
        iconString = m_iconImageString.c_str();
    }
    return iconString;
}

void TabCtrlItem::SetTitle(const std::wstring& title)
{
    Label* pLabel = GetTextLabel();
    if (pLabel != nullptr) {
        pLabel->SetText(title);
        m_title.clear();
    }
    else {
        m_title = title;
    }
}

std::wstring TabCtrlItem::GetTitle() const
{
    std::wstring title;
    Label* pLabel = GetTextLabel();
    if (pLabel != nullptr) {
        title = pLabel->GetText();
    }
    else {
        title = m_title.c_str();
    }
    return title;
}

void TabCtrlItem::SetTabBoxItemIndex(size_t nTabBoxItemIndex)
{
    m_nTabBoxItemIndex = nTabBoxItemIndex;
}

size_t TabCtrlItem::GetTabBoxItemIndex() const
{
    return m_nTabBoxItemIndex;
}

}//namespace ui

