#include "VirtualTileBox.h"
#include "duilib/Core/ScrollBar.h"
#include <algorithm>

namespace ui {

VirtualTileBoxElement::VirtualTileBoxElement():
    m_CountChangedNotify(), 
    m_DataChangedNotify()
{

}

void VirtualTileBoxElement::RegNotifys(const DataChangedNotify& dcNotify, const CountChangedNotify& ccNotify)
{
    m_DataChangedNotify = dcNotify;
    m_CountChangedNotify = ccNotify;
}

void VirtualTileBoxElement::EmitDataChanged(size_t nStartIndex, size_t nEndIndex)
{
    if (m_DataChangedNotify) {
        m_DataChangedNotify(nStartIndex, nEndIndex);
    }
}

void VirtualTileBoxElement::EmitCountChanged()
{
    if (m_CountChangedNotify) {
        m_CountChangedNotify();
    }
}


VirtualTileLayout::VirtualTileLayout():
    m_bAutoCalcColumn(true)
{
    m_nColumns = -1;
}

ui::UiSize VirtualTileLayout::ArrangeChild(const std::vector<ui::Control*>& /*items*/, ui::UiRect rc)
{
#ifdef _DEBUG
    VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
    ASSERT(pList != nullptr);
#endif

    ui::UiSize sz(rc.GetWidth(), rc.GetHeight());
    size_t nTotalHeight = GetElementsHeight(Box::InvalidIndex);
    sz.cy = std::max(LONG(nTotalHeight), sz.cy);
    LazyArrangeChild();
    return sz;
}

ui::UiSize VirtualTileLayout::EstimateSizeByChild(const std::vector<ui::Control*>& /*items*/, ui::UiSize szAvailable)
{
#ifdef _DEBUG
    VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
    ASSERT(pList != nullptr);
#endif

    ASSERT(m_nColumns > 0);
    ui::UiSize size = m_pOwner->Control::EstimateSize(szAvailable);
    if (size.cx == DUI_LENGTH_AUTO || size.cx == 0) {
        size.cx = m_szItem.cx * m_nColumns + m_iChildMargin * (m_nColumns - 1);
    }
    return size;
}

bool VirtualTileLayout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"column") {
        int iValue = _wtoi(strValue.c_str());
        if (iValue > 0) {
            SetColumns(iValue);
            m_bAutoCalcColumn = false;
        }
        else {
            m_bAutoCalcColumn = true;
        }
        return true;
    }
    else {
        return __super::SetAttribute(strName, strValue);
    }
}

size_t VirtualTileLayout::GetElementsHeight(size_t nCount)
{
    ASSERT(m_nColumns > 0);
    if (m_nColumns < 1) {
        return static_cast<size_t>(m_szItem.cy + m_iChildMargin);
    }
    if (nCount <= m_nColumns && nCount != Box::InvalidIndex) {
        return static_cast<size_t>(m_szItem.cy + m_iChildMargin);
    }

    VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
    ASSERT(pList != nullptr);

    if (!Box::IsValidItemIndex(nCount)) {
        nCount = pList->GetElementCount();
    }

    size_t rows = nCount / m_nColumns;
    if (nCount % m_nColumns != 0) {
        rows += 1;
    }
    size_t iChildMargin = 0;
    if (m_iChildMargin > 0) {
        iChildMargin = (size_t)m_iChildMargin;
    }
    if (nCount > 0) {
        size_t childMarginTotal = 0;
        if (nCount % m_nColumns == 0) {
            childMarginTotal = (nCount / m_nColumns - 1) * iChildMargin;
        }
        else {
            childMarginTotal = (nCount / m_nColumns) * iChildMargin;
        }
        return m_szItem.cy * (rows + 1) + childMarginTotal;
    }
    return 0;
}

void VirtualTileLayout::LazyArrangeChild()
{
    VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
    ASSERT(pList != nullptr);

    // 列在SetPos时已经设置好
    ASSERT(m_nColumns > 0);
    if (m_nColumns < 1) {
        return;
    }

    // 获取VirtualTileBox的Rect
    ui::UiRect rc = pList->GetPaddingPos();

    // 子项的左边起始位置 
    int iPosLeft = rc.left;

    // 子项的顶部起始位置 
    int iPosTop = rc.top + pList->GetScrollPos().cy;

    ui::UiPoint ptTile(iPosLeft, iPosTop);

    // 顶部index
    size_t nTopBottom = 0;
    size_t nTopIndex = pList->GetTopElementIndex(nTopBottom);

    size_t iCount = 0;

    for (auto pControl : pList->m_items)
    {
        if (pControl == nullptr) {
            continue;
        }
        // Determine size
        ui::UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + m_szItem.cx, ptTile.y + m_szItem.cy);
        pControl->SetPos(rcTile);

        // 填充数据
        size_t nElementIndex = nTopIndex + iCount;
        if (nElementIndex < pList->GetElementCount()) {
            if (!pControl->IsVisible()) {
                pControl->SetFadeVisible(true);
            }
            pList->FillElement(pControl, nElementIndex);
        }
        else {
            if (pControl->IsVisible()) {
                pControl->SetFadeVisible(false);
            }
        }

        if ((++iCount % m_nColumns) == 0) {
            ptTile.x = iPosLeft;
            ptTile.y += m_szItem.cy + m_iChildMargin;
        }
        else {
            ptTile.x += rcTile.GetWidth() + m_iChildMargin;
        }
    }
}

size_t VirtualTileLayout::AjustMaxItem()
{
    ui::UiRect rc = m_pOwner->GetPaddingPos();
    if (m_bAutoCalcColumn || (m_nColumns == 0) || (m_nColumns == (size_t)-1)) {
        if (m_szItem.cx > 0) {
            m_nColumns = static_cast<size_t>((rc.right - rc.left) / (m_szItem.cx + m_iChildMargin / 2));
        }
        if (m_nColumns == 0) {
            m_nColumns = 1;
        }
    }
    ASSERT(m_nColumns != 0);
    ASSERT(m_nColumns != (size_t)-1);

    int nHeight = m_szItem.cy + m_iChildMargin;
    int nRow = (rc.bottom - rc.top) / nHeight + 1;
    return (size_t)nRow * m_nColumns;
}

VirtualTileBox::VirtualTileBox(ui::Layout* pLayout /*= new VirtualTileLayout*/)
    : ui::ListBox(pLayout)
    , m_pDataProvider(nullptr)
    , m_nMaxItemCount(0)
    , m_nOldYScrollPos(0)
    , m_bArrangedOnce(false)
    , m_bForceArrange(false)
{
}

void VirtualTileBox::SetDataProvider(VirtualTileBoxElement* pProvider)
{
    m_pDataProvider = pProvider;
    if (pProvider != nullptr) {
        // 注册模型数据变动通知回调
        pProvider->RegNotifys(
            nbase::Bind(&VirtualTileBox::OnModelDataChanged, this, std::placeholders::_1, std::placeholders::_2),
            nbase::Bind(&VirtualTileBox::OnModelCountChanged, this));
    }
}

VirtualTileBoxElement* VirtualTileBox::GetDataProvider()
{
    return m_pDataProvider;
}

void VirtualTileBox::Refresh()
{
    size_t nMaxItemCount = GetTileLayout()->AjustMaxItem();
    m_nMaxItemCount = nMaxItemCount;

    size_t nElementCount = GetElementCount();
    size_t nItemCount = GetItemCount();

    // 如果现有子项总数大于数据总数， 移出比数据总数多出的子项
    if (nItemCount > nElementCount) {
        size_t n = nItemCount - nElementCount;
        for (size_t i = 0; i < n; ++i) {
            this->RemoveItemAt(0);
        }
    }
    // 如果子项总数据小于数据总数
    else if (nItemCount < nElementCount) {
        size_t n = 0;
        if (nElementCount <= nMaxItemCount) {
            n = nElementCount - nItemCount;
        }
        else {
            n = nMaxItemCount - nItemCount;
        }

        for (size_t i = 0; i < n; ++i) {
            Control* pControl = CreateElement();
            this->AddItem(pControl);
        }
    }

    if (nElementCount == 0) {
        return;
    }
    ReArrangeChild(true);
    Arrange();

}

void VirtualTileBox::RemoveAllItems()
{
    __super::RemoveAllItems();

    if (m_pVerticalScrollBar) {
        m_pVerticalScrollBar->SetScrollPos(0);
    }
    m_nOldYScrollPos = 0;
    m_bArrangedOnce = false;
    m_bForceArrange = false;
}

void VirtualTileBox::SetForceArrange(bool bForce)
{
    m_bForceArrange = bForce;
}

void VirtualTileBox::GetDisplayCollection(std::vector<size_t>& collection)
{
    collection.clear();
    if (GetItemCount() == 0) {
        return;
    }

    // 获取Box的Rect
    ui::UiRect rcThis = this->GetPaddingPos();

    size_t nEleHeight = GetRealElementHeight();

    size_t min = ((size_t)GetScrollPos().cy / nEleHeight) * GetColumns();
    size_t max = min + ((size_t)rcThis.GetHeight() / nEleHeight) * GetColumns();

    size_t nCount = GetElementCount();
    if (nCount > 0) {
        if (max >= nCount) {
            max = nCount - 1;
        }
    }
    else {
        ASSERT(FALSE);
        max = 0;
    }

    for (size_t i = min; i <= max; ++i) {
        collection.push_back(i);
    }
}

void VirtualTileBox::EnsureVisible(size_t iIndex, bool bToTop /*= false*/)
{
    if (!Box::IsValidItemIndex(iIndex) || iIndex >= GetElementCount()) {
        return;
    }
    if (m_pVerticalScrollBar == nullptr) {
        return;
    }
    size_t nPos = (size_t)GetScrollPos().cy;
    size_t nTopIndex = (nPos / GetRealElementHeight()) * GetColumns();
    size_t nNewPos = 0;

    if (bToTop)
    {
        nNewPos = CalcElementsHeight(iIndex);
        if (nNewPos >= (size_t)m_pVerticalScrollBar->GetScrollRange()) {
            return;
        }
    }
    else {
        if (IsElementDisplay(iIndex)) {
            return;
        }

        if (iIndex > nTopIndex) {
            // 向下
            size_t height = CalcElementsHeight(iIndex + 1);
            nNewPos = height - (size_t)GetRect().GetHeight();
        }
        else {
            // 向上
            nNewPos = CalcElementsHeight(iIndex);
        }
    }
    ui::UiSize sz(0, (int)nNewPos);
    SetScrollPos(sz);
}

void VirtualTileBox::SetScrollPos(ui::UiSize szPos)
{
    ASSERT(GetScrollPos().cy >= 0);
    m_nOldYScrollPos = (size_t)GetScrollPos().cy;
    ListBox::SetScrollPos(szPos);
    ReArrangeChild(false);
}

void VirtualTileBox::HandleEvent(const ui::EventArgs& event)
{
    if (!IsMouseEnabled() && (event.Type > ui::kEventMouseBegin) && (event.Type < ui::kEventMouseEnd)) {
        if (GetParent() != nullptr) {
            GetParent()->SendEvent(event);
        }
        else {
            ui::ScrollBox::HandleEvent(event);
        }
        return;
    }

    switch (event.Type) {
    case ui::kEventKeyDown: {
        switch (event.chKey) {
        case VK_UP: {
            OnKeyDown(VK_UP);
            return;
        }
        case VK_DOWN: {
            OnKeyDown(VK_DOWN);
            return;
        }
        case VK_HOME:
            SetScrollPosY(0);
            return;
        case VK_END: {
            int range = GetScrollPos().cy;
            SetScrollPosY(range);
            return;
        }
        default:
            break;
        }
    }
    case ui::kEventKeyUp: {
        switch (event.chKey) {
        case VK_UP: {
            OnKeyUp(VK_UP);
            return;
        }
        case VK_DOWN: {
            OnKeyUp(VK_DOWN);
            return;
        }
        default:
            break;
        }
    default:
        break;
    }
    }

    __super::HandleEvent(event);
}

void VirtualTileBox::SetPos(ui::UiRect rc)
{
    bool bChange = false;
    if (!GetRect().Equal(rc)) {
        bChange = true;
    }
    ListBox::SetPos(rc);
    if (bChange) {
        Refresh();
    }
}

void VirtualTileBox::ReArrangeChild(bool bForce)
{
    ScrollDirection direction = ScrollDirection::kScrollUp;
    if (!bForce && !m_bForceArrange) {
        if (!NeedReArrange(direction)) {
            return;
        }
    }
    LazyArrangeChild();
}


ui::Control* VirtualTileBox::CreateElement()
{
    if (m_pDataProvider != nullptr) {
        return m_pDataProvider->CreateElement();
    }
    return nullptr;
}

void VirtualTileBox::FillElement(Control* pControl, size_t iIndex)
{
    if (m_pDataProvider != nullptr) {
        m_pDataProvider->FillElement(pControl, iIndex);
    }
}

size_t VirtualTileBox::GetElementCount()
{
    if (m_pDataProvider != nullptr) {
        return m_pDataProvider->GetElementtCount();
    }
    return 0;
}

size_t VirtualTileBox::CalcElementsHeight(size_t nCount)
{
    return GetTileLayout()->GetElementsHeight(nCount);
}

size_t VirtualTileBox::GetTopElementIndex(size_t& bottom)
{
    size_t nPos = GetScrollPos().cy;

    size_t nHeight = GetRealElementHeight();
    size_t iIndex = (nPos / nHeight) * GetColumns();
    bottom = iIndex * nHeight;

    return iIndex;
}

bool VirtualTileBox::IsElementDisplay(size_t iIndex)
{
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }

    size_t nPos = GetScrollPos().cy;
    size_t nElementPos = CalcElementsHeight(iIndex);
    if (nElementPos >= nPos) {
        size_t nHeight = this->GetHeight();
        if (nElementPos <= nPos + nHeight) {
            return true;
        }
    }
    return false;
}

bool VirtualTileBox::NeedReArrange(ScrollDirection& direction)
{
    direction = ScrollDirection::kScrollUp;
    if (!m_bArrangedOnce) {
        m_bArrangedOnce = true;
        return true;
    }

    size_t nCount = GetItemCount();
    if (nCount == 0) {
        return false;
    }

    if (GetElementCount() <= nCount) {
        return false;
    }

    ui::UiRect rcThis = this->GetPos();
    if (rcThis.GetWidth() <= 0) {
        return false;
    }

    int nPos = GetScrollPos().cy;
    ui::UiRect rcItem;

    rcItem = m_items[0]->GetPos();

    if (nPos >= m_nOldYScrollPos) {
        // 下
        rcItem = m_items[nCount - 1]->GetPos();
        int nSub = (rcItem.bottom - rcThis.top) - (nPos + rcThis.GetHeight());
        if (nSub < 0) {
            direction = ScrollDirection::kScrollDown;
            return true;
        }
    }
    else {
        // 上
        rcItem = m_items[0]->GetPos();
        if (nPos < (rcItem.top - rcThis.top)) {
            direction = ScrollDirection::kScrollUp;
            return true;
        }
    }
    return false;
}

VirtualTileLayout* VirtualTileBox::GetTileLayout()
{
    auto* pLayout = dynamic_cast<VirtualTileLayout*>(GetLayout());
    return pLayout;
}

size_t VirtualTileBox::GetRealElementHeight()
{
    return GetTileLayout()->GetElementsHeight(1);
}

size_t VirtualTileBox::GetColumns()
{
    return GetTileLayout()->GetColumns();
}

void VirtualTileBox::LazyArrangeChild()
{

    GetTileLayout()->LazyArrangeChild();
}

void VirtualTileBox::OnModelDataChanged(size_t nStartIndex, size_t nEndIndex)
{
    for (size_t i = nStartIndex; i <= nEndIndex; ++i) {
        size_t nItemIndex = ElementIndexToItemIndex(nStartIndex);
        if (Box::IsValidItemIndex(nItemIndex) && nItemIndex < m_items.size()) {
            FillElement(m_items[nItemIndex], i);
        }
    }
}

void VirtualTileBox::OnModelCountChanged()
{
    Refresh();
}

size_t VirtualTileBox::ElementIndexToItemIndex(size_t nElementIndex)
{
    if (IsElementDisplay(nElementIndex)) {
        size_t nTopItemHeight = 0;
        return nElementIndex - GetTopElementIndex(nTopItemHeight);
    }
    return Box::InvalidIndex;
}

size_t VirtualTileBox::ItemIndexToElementIndex(size_t nItemIndex)
{
    size_t nTopItemHeight = 0;
    return GetTopElementIndex(nTopItemHeight) + nItemIndex;
}

}
