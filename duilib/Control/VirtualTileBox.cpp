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

UiSize64 VirtualTileLayout::ArrangeChild(const std::vector<ui::Control*>& /*items*/, ui::UiRect rc)
{
    UiSize64 sz(rc.Width(), rc.Height());
    int64_t nTotalHeight = GetElementsHeight(Box::InvalidIndex);
    sz.cy = std::max(nTotalHeight, sz.cy);
    LazyArrangeChild();
    return sz;
}

ui::UiSize VirtualTileLayout::EstimateSizeByChild(const std::vector<ui::Control*>& /*items*/, ui::UiSize szAvailable)
{
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

int64_t VirtualTileLayout::GetElementsHeight(size_t nCount)
{
    ASSERT(m_nColumns > 0);
    if (m_nColumns < 1) {
        return m_szItem.cy + m_iChildMargin;
    }
    if (nCount <= m_nColumns && nCount != Box::InvalidIndex) {
        //不到1行，或者刚好1行
        return m_szItem.cy + m_iChildMargin;
    }
    if (!Box::IsValidItemIndex(nCount)) {
        VirtualTileBox* pList = dynamic_cast<VirtualTileBox*>(m_pOwner);
        ASSERT(pList != nullptr);
        if (pList != nullptr) {
            nCount = pList->GetElementCount();
        }        
    }
    if (!Box::IsValidItemIndex(nCount)) {
        ASSERT(FALSE);
        return m_szItem.cy + m_iChildMargin;
    }

    int64_t rows = nCount / m_nColumns;
    if (nCount % m_nColumns != 0) {
        rows += 1;
    }
    int64_t iChildMargin = 0;
    if (m_iChildMargin > 0) {
        iChildMargin = m_iChildMargin;
    }
    if (nCount > 0) {
        int64_t childMarginTotal = 0;
        if (nCount % m_nColumns == 0) {
            childMarginTotal = ((int64_t)nCount / m_nColumns - 1) * iChildMargin;
        }
        else {
            childMarginTotal = ((int64_t)nCount / m_nColumns) * iChildMargin;
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
    int32_t iPosTop = rc.top;
    pList->SetScrollVirtualOffsetY(pList->GetScrollPos().cy);

    ui::UiPoint ptTile(iPosLeft, iPosTop);

    // 顶部index
    size_t nTopIndex = pList->GetTopElementIndex(nullptr);
    size_t iCount = 0;
    for (auto pControl : pList->m_items) {
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
                pControl->SetVisible(true);
            }
            pList->FillElement(pControl, nElementIndex);
        }
        else {
            if (pControl->IsVisible()) {
                pControl->SetVisible(false);
            }
        }

        if ((++iCount % m_nColumns) == 0) {
            ptTile.x = iPosLeft;
            ptTile.y += m_szItem.cy + m_iChildMargin;
        }
        else {
            ptTile.x += rcTile.Width() + m_iChildMargin;
        }
    }
}

size_t VirtualTileLayout::AjustMaxItem()
{
    ASSERT(m_pOwner != nullptr);
    if (m_pOwner == nullptr) {
        return 0;
    }
    ui::UiRect rc = m_pOwner->GetPaddingPos();
    if (rc.IsEmpty()) {
        return 0;
    }
    if (m_bAutoCalcColumn || (m_nColumns <= 0)) {
        //计算需要几列
        if (m_szItem.cx > 0) {
            m_nColumns = rc.Width() / (m_szItem.cx + m_iChildMargin / 2);
            //验证并修正
            if (m_nColumns > 1) {
                int32_t calcWidth = m_nColumns * m_szItem.cx + (m_nColumns - 1) * m_iChildMargin;
                if (calcWidth > rc.Width()) {
                    m_nColumns -= 1;
                }
            }
        }        
    }
    if (m_nColumns <= 0) {
        m_nColumns = 1;
    }
    int32_t nRows = rc.Height() / (m_szItem.cy + m_iChildMargin / 2);
    //验证并修正
    if (nRows > 1) {
        int32_t calcHeight = nRows * m_szItem.cy + (nRows - 1) * m_iChildMargin;
        if (calcHeight < rc.Height()) {
            nRows += 1;
        }
    }
    //额外增加1行，确保真实控件填充满整个可显示区域
    nRows += 1;
    return nRows * m_nColumns;
}

VirtualTileBox::VirtualTileBox(Layout* pLayout /*= new VirtualTileLayout*/)
    : ListBox(pLayout)
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
    //最大子项数
    size_t nMaxItemCount = GetTileLayout()->AjustMaxItem();
    if (nMaxItemCount == 0) {
        return;
    }
    m_nMaxItemCount = nMaxItemCount;

    //当前数据总数
    size_t nElementCount = GetElementCount();

    //当前子项数
    size_t nItemCount = GetItemCount();

    //刷新后的子项数
    size_t nNewItemCount = nElementCount;
    if (nNewItemCount > nMaxItemCount) {
        nNewItemCount = nMaxItemCount;
    }
    
    if (nItemCount > nNewItemCount) {
        //如果现有子项总数大于新计算的子项数，移除比数据总数多出的子项
        size_t n = nItemCount - nNewItemCount;
        for (size_t i = 0; i < n; ++i) {
            this->RemoveItemAt(0);
        }
    }
    else if (nItemCount < nNewItemCount) {
        //如果现有子项总数小于新计算的子项数，新增比数据总数少的子项
        size_t n = nNewItemCount - nItemCount;
        for (size_t i = 0; i < n; ++i) {
            Control* pControl = CreateElement();
            this->AddItem(pControl);
        }
    }
    if (nElementCount > 0) {
        ReArrangeChild(true);
        Arrange();
    }    
}

void VirtualTileBox::RemoveAllItems()
{
    __super::RemoveAllItems();

    if (m_pVScrollBar) {
        m_pVScrollBar->SetScrollPos(0);
    }
    SetScrollVirtualOffset({ 0, 0 });
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
    size_t max = min + ((size_t)rcThis.Height() / nEleHeight) * GetColumns();

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
    if (m_pVScrollBar == nullptr) {
        return;
    }
    int64_t nPos = GetScrollPos().cy;
    int64_t elementHeight = GetRealElementHeight();
    int64_t nTopIndex = 0;
    if (elementHeight > 0) {
        nTopIndex = (nPos / elementHeight) * GetColumns();
    }
    int64_t nNewPos = 0;

    if (bToTop)
    {
        nNewPos = CalcElementsHeight(iIndex);
        if (nNewPos > elementHeight) {
            nNewPos -= elementHeight;
        }
    }
    else {
        if (IsElementDisplay(iIndex)) {
            return;
        }

        if ((int64_t)iIndex > nTopIndex) {
            // 向下
            int64_t height = CalcElementsHeight(iIndex + 1);
            nNewPos = height - GetRect().Height();
        }
        else {
            // 向上
            nNewPos = CalcElementsHeight(iIndex);
            if (nNewPos > elementHeight) {
                nNewPos -= elementHeight;
            }            
        }
    }
    if (nNewPos < 0) {
        nNewPos = 0;
    }
    if (nNewPos > m_pVScrollBar->GetScrollRange()) {
        nNewPos = m_pVScrollBar->GetScrollRange();
    }
    ui::UiSize64 sz(0, nNewPos);
    SetScrollPos(sz);
}

void VirtualTileBox::SetScrollPos(UiSize64 szPos)
{
    ASSERT(GetScrollPos().cy >= 0);
    bool isChanged = m_nOldYScrollPos != GetScrollPos().cy;
    m_nOldYScrollPos = GetScrollPos().cy;
    ListBox::SetScrollPos(szPos);
    if (isChanged) {
        ReArrangeChild(false);
    }
}

void VirtualTileBox::HandleEvent(const EventArgs& event)
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
            int64_t range = GetScrollRange().cy;
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

void VirtualTileBox::OnKeyDown(TCHAR ch)
{ 
    if (ch == VK_UP) {
        LineUp(-1, false);
    }
    else if (ch == VK_DOWN) {
        LineDown(-1, false);
    }
}

void VirtualTileBox::OnKeyUp(TCHAR /*ch*/)
{ 
}

void VirtualTileBox::SetPos(ui::UiRect rc)
{
    bool bChange = false;
    if (!GetRect().Equals(rc)) {
        bChange = true;
    }
    ListBox::SetPos(rc);
    if (bChange) {
        Refresh();
    }
}

void VirtualTileBox::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    ReArrangeChild(false);
    __super::PaintChild(pRender, rcPaint);
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
    size_t elementCount = 0;
    if (m_pDataProvider != nullptr) {
        elementCount = m_pDataProvider->GetElementCount();
    }
    return elementCount;
}

int64_t VirtualTileBox::CalcElementsHeight(size_t nCount)
{
    int64_t height = GetTileLayout()->GetElementsHeight(nCount);
    ASSERT(height >= 0);
    if (height < 0) {
        height = 0;
    }
    return height;
}

size_t VirtualTileBox::GetTopElementIndex(int64_t* bottom)
{
    int64_t nPos = GetScrollPos().cy;
    if (nPos < 0) {
        nPos = 0;
    }
    int64_t nColumns = (int64_t)GetColumns();
    if (nColumns < 0) {
        nColumns = 0;
    }
    int64_t nHeight = GetRealElementHeight();
    ASSERT(nHeight >= 0);
    if (nHeight <= 0) {
        if (bottom != nullptr) {
            *bottom = 0;
        }
        return 0;
    }
    int64_t iIndex = (nPos / nHeight) * nColumns;
    if (bottom != nullptr) {
        *bottom = iIndex * nHeight;
    }
    return static_cast<size_t>(iIndex);
}

bool VirtualTileBox::IsElementDisplay(size_t iIndex)
{
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }

    int64_t nPos = GetScrollPos().cy;
    int64_t nElementPos = CalcElementsHeight(iIndex);
    if (nElementPos >= nPos) {
        int64_t nHeight = this->GetHeight();
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
        //数据变化后，如果还没有完成一次布局，那么就强制做一次Arrange
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
    if (rcThis.Width() <= 0) {
        return false;
    }

    int64_t nPos = GetScrollPos().cy;
    int64_t nVirtualOffsetY = GetScrollVirtualOffset().cy;
    ui::UiRect rcItem;

    rcItem = m_items[0]->GetPos();

    if (nPos >= m_nOldYScrollPos) {
        // 下
        rcItem = m_items[nCount - 1]->GetPos();
        int64_t nSub = (rcItem.bottom + nVirtualOffsetY - rcThis.top) - (nPos + rcThis.Height());
        if (nSub < 0) {
            direction = ScrollDirection::kScrollDown;
            return true;
        }
    }
    else {
        // 上
        rcItem = m_items[0]->GetPos();
        if (nPos < (rcItem.top + nVirtualOffsetY - rcThis.top)) {
            direction = ScrollDirection::kScrollUp;
            return true;
        }
    }
    return false;
}

VirtualTileLayout* VirtualTileBox::GetTileLayout()
{
    auto* pLayout = dynamic_cast<VirtualTileLayout*>(GetLayout());
    ASSERT(pLayout != nullptr);
    return pLayout;
}

int64_t VirtualTileBox::GetRealElementHeight()
{
    int64_t height = GetTileLayout()->GetElementsHeight(1);
    ASSERT(height >= 0);
    if (height < 0) {
        height = 0;
    }
    return height;
}

size_t VirtualTileBox::GetColumns()
{
    int columns = GetTileLayout()->GetColumns();
    ASSERT(columns >= 0);
    if (columns < 0) {
        columns = 0;
    }
    return static_cast<size_t>(columns);
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
        size_t nTopItemIndex = GetTopElementIndex(nullptr);
        ASSERT(nElementIndex >= nTopItemIndex);
        if (nElementIndex >= nTopItemIndex)
        {
            return nElementIndex - nTopItemIndex;
        }
    }
    return Box::InvalidIndex;
}

size_t VirtualTileBox::ItemIndexToElementIndex(size_t nItemIndex)
{
    return GetTopElementIndex(nullptr) + nItemIndex;
}

}
