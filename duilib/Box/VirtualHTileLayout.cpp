#include "VirtualHTileLayout.h"
#include "duilib/Box/VirtualListBox.h"

namespace ui 
{

VirtualHTileLayout::VirtualHTileLayout()
{
    SetRows(0);
    SetAutoCalcRows(true);
}

VirtualListBox* VirtualHTileLayout::GetOwnerBox() const
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    ASSERT(pList != nullptr);
    return pList;
}

int32_t VirtualHTileLayout::CalcTileRows(int32_t rcHeight) const
{
    int32_t nRows = GetRows();
    if (nRows < 0) {
        nRows = 0;
    }
    if (IsAutoCalcRows()) {
        nRows = 0;
    }
    if (nRows > 0) {
        return nRows;
    }
    int32_t totalHeight = rcHeight;
    int32_t tileHeight = GetItemSize().cy;
    int32_t childMarginY = GetChildMarginY();
    if (childMarginY < 0) {
        childMarginY = 0;
    }
    if (tileHeight > 0) {
        while (totalHeight > 0) {
            totalHeight -= tileHeight;
            if (nRows != 0) {
                totalHeight -= childMarginY;
            }
            if (totalHeight >= 0) {
                ++nRows;
            }
        }
    }
    if (nRows < 1) {
        nRows = 1;
    }
    return nRows;
}

UiSize64 VirtualHTileLayout::ArrangeChild(const std::vector<ui::Control*>& items, ui::UiRect rc)
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    if ((pList == nullptr) || !pList->HasDataProvider()) {
        //如果未设置数据接口，则兼容基类的功能
        return __super::ArrangeChild(items, rc);
    }
    DeflatePadding(rc);
    int64_t nTotalWidth = GetElementsWidth(rc, Box::InvalidIndex);
    UiSize64 sz(rc.Width(), rc.Height());
    sz.cx = std::max(nTotalWidth, sz.cx);
    LazyArrangeChild(rc);
    return sz;
}

UiSize VirtualHTileLayout::EstimateSizeByChild(const std::vector<Control*>& items, ui::UiSize szAvailable)
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    if ((pList == nullptr) || !pList->HasDataProvider()) {
        //如果未设置数据接口，则兼容基类的功能
        return __super::EstimateSizeByChild(items, szAvailable);
    }
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return UiSize();
    }
    szAvailable.Validate();
    int32_t nRows = CalcTileRows(szAvailable.cy);
    UiEstSize estSize;
    if (GetOwner() != nullptr) {
        estSize = GetOwner()->Control::EstimateSize(szAvailable);
    }
    UiSize size(estSize.cx.GetInt32(), estSize.cy.GetInt32());
    if (estSize.cx.IsStretch()) {
        size.cx = CalcStretchValue(estSize.cx, szAvailable.cx);
    }
    if (estSize.cy.IsStretch()) {
        size.cy = CalcStretchValue(estSize.cy, szAvailable.cy);
    }
    if (size.cy == 0) {
        size.cy = szItem.cy * nRows + GetChildMarginY() * (nRows - 1);
    }
    size.Validate();
    return size;
}

int64_t VirtualHTileLayout::GetElementsWidth(UiRect rc, size_t nCount) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return 0;
    }
    int32_t nRows = CalcTileRows(rc.Height());
    if (nCount <= nRows && nCount != Box::InvalidIndex) {
        //不到1列，或者刚好1列
        return szItem.cx + GetChildMarginX();
    }
    if (!Box::IsValidItemIndex(nCount)) {
        VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
        ASSERT(pList != nullptr);
        if (pList != nullptr) {
            nCount = pList->GetElementCount();
        }
    }
    if (!Box::IsValidItemIndex(nCount)) {
        ASSERT(FALSE);
        return szItem.cx + GetChildMarginX();
    }

    int64_t cols = nCount / nRows;
    if (nCount % nRows != 0) {
        cols += 1;
    }
    int64_t iChildMargin = 0;
    if (GetChildMarginX() > 0) {
        iChildMargin = GetChildMarginX();
    }
    if (nCount > 0) {
        int64_t childMarginTotal = 0;
        if (nCount % nRows == 0) {
            childMarginTotal = ((int64_t)nCount / nRows - 1) * iChildMargin;
        }
        else {
            childMarginTotal = ((int64_t)nCount / nRows) * iChildMargin;
        }
        return szItem.cx * cols + childMarginTotal;
    }
    return 0;
}

void VirtualHTileLayout::LazyArrangeChild(UiRect rc) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return;
    }
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return;
    }
    if (!pOwnerBox->HasDataProvider()) {
        return;
    }

    //行数
    int32_t nRows = CalcTileRows(rc.Height());

    //子项的顶部起始位置 
    int32_t iPosTop = rc.top;

    //X轴坐标的偏移，需要保持，避免滚动位置变动后，重新刷新界面出现偏差
    int32_t xOffset = 0;
    int64_t itemWidth = GetElementsWidth(rc, 1);
    if (itemWidth > 0) {
        xOffset = TruncateToInt32(pOwnerBox->GetScrollPos().cx % itemWidth);
    }

    //子项的左侧起始位置
    int32_t iPosLeft = rc.left - xOffset;

    //设置虚拟偏移，否则当数据量较大时，rc这个32位的矩形的高度会越界，需要64位整型才能容纳
    pOwnerBox->SetScrollVirtualOffsetX(pOwnerBox->GetScrollPos().cx);

    //控件的左上角坐标值
    ui::UiPoint ptTile(iPosLeft, iPosTop);

    VirtualListBox::RefreshDataList refreshDataList;
    VirtualListBox::RefreshData refreshData;
    // 顶部index
    size_t nTopIndex = GetTopElementIndex(rc);
    size_t iCount = 0;
    size_t nItemCount = pOwnerBox->m_items.size();
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        Control* pControl = pOwnerBox->m_items[nItemIndex];
        if (pControl == nullptr) {
            continue;
        }
        // Determine size
        ui::UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + szItem.cx, ptTile.y + szItem.cy);
        pControl->SetPos(rcTile);

        // 填充数据
        size_t nElementIndex = nTopIndex + iCount;
        if (nElementIndex < pOwnerBox->GetElementCount()) {
            if (!pControl->IsVisible()) {
                pControl->SetVisible(true);
            }
            pOwnerBox->FillElement(pControl, nElementIndex);
            refreshData.nItemIndex = nItemIndex;
            refreshData.pControl = pControl;
            refreshData.nElementIndex = nElementIndex;
            refreshDataList.push_back(refreshData);
        }
        else {
            if (pControl->IsVisible()) {
                pControl->SetVisible(false);
            }
        }

        if ((++iCount % nRows) == 0) {
            ptTile.y = iPosTop;
            ptTile.x += szItem.cx + GetChildMarginX();
        }
        else {
            ptTile.y += rcTile.Height() + GetChildMarginY();
        }
    }
    if (!refreshDataList.empty()) {
        pOwnerBox->OnRefreshElements(refreshDataList);
    }
}

size_t VirtualHTileLayout::AjustMaxItem(UiRect rc) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return 0;
    }
    if (rc.IsEmpty()) {
        return 0;
    }
    int32_t nRows = CalcTileRows(rc.Height());
    int32_t nColumns = rc.Width() / (szItem.cx + GetChildMarginX() / 2);
    //验证并修正
    if (nColumns > 1) {
        int32_t calcWidth = nColumns * szItem.cx + (nColumns - 1) * GetChildMarginX();
        if (calcWidth < rc.Width()) {
            nColumns += 1;
        }
    }
    //额外增加1列，确保真实控件填充满整个可显示区域
    nColumns += 1;
    return nRows * nColumns;
}

size_t VirtualHTileLayout::GetTopElementIndex(UiRect rc) const
{
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return 0;
    }
    int64_t nPos = pOwnerBox->GetScrollPos().cx;
    if (nPos < 0) {
        nPos = 0;
    }

    int64_t nRows = (int64_t)CalcTileRows(rc.Height());
    if (nRows < 0) {
        nRows = 0;
    }
    int64_t nWidth = GetElementsWidth(rc, 1);
    ASSERT(nWidth >= 0);
    if (nWidth <= 0) {
        return 0;
    }
    int64_t iIndex = (nPos / nWidth) * nRows;
    return static_cast<size_t>(iIndex);
}

bool VirtualHTileLayout::IsElementDisplay(UiRect rc, size_t iIndex) const
{
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return false;
    }

    int64_t nScrollPos = pOwnerBox->GetScrollPos().cx;
    int64_t nElementPos = GetElementsWidth(rc, iIndex + 1);
    UiSize szItem = GetItemSize();
    if ((nElementPos - szItem.cx) > nScrollPos) { //矩形的left位置
        int64_t nBoxWidth = pOwnerBox->GetWidth();
        if (nElementPos <= (nScrollPos + nBoxWidth)) {//矩形的right位置
            //完整显示
            return true;
        }
    }
    return false;
}

bool VirtualHTileLayout::NeedReArrange() const
{
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return false;
    }
    if (!pOwnerBox->HasDataProvider()) {
        return false;
    }
    size_t nCount = pOwnerBox->GetItemCount();
    if (nCount == 0) {
        return false;
    }

    if (pOwnerBox->GetElementCount() <= nCount) {
        return false;
    }

    ui::UiRect rcThis = pOwnerBox->GetPos();
    if (rcThis.IsEmpty()) {
        return false;
    }

    int64_t nScrollPosX = pOwnerBox->GetScrollPos().cx;
    int64_t nVirtualOffsetX = pOwnerBox->GetScrollVirtualOffset().cx;
    return nVirtualOffsetX != nScrollPosX;
}

void VirtualHTileLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const
{
    collection.clear();
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return;
    }

    if (pOwnerBox->GetItemCount() == 0) {
        return;
    }

    size_t nEleWidth = GetElementsWidth(rc, 1);
    if (nEleWidth == 0) {
        return;
    }

    int32_t nRows = CalcTileRows(rc.Height());
    size_t min = ((size_t)pOwnerBox->GetScrollPos().cx / nEleWidth) * nRows;
    size_t max = min + ((size_t)rc.Width() / nEleWidth) * nRows;

    size_t nCount = pOwnerBox->GetElementCount();
    if (nCount > 0) {
        if (max >= nCount) {
            max = nCount - 1;
        }
    }
    else {
        return;
    }

    for (size_t i = min; i <= max; ++i) {
        collection.push_back(i);
    }
}

void VirtualHTileLayout::EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const
{
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return;
    }
    if (!Box::IsValidItemIndex(iIndex) || iIndex >= pOwnerBox->GetElementCount()) {
        return;
    }
    if (pOwnerBox->m_pVScrollBar == nullptr) {
        return;
    }
    int64_t nPos = pOwnerBox->GetScrollPos().cx;
    int64_t elementWidth = GetElementsWidth(rc, 1);
    if (elementWidth <= 0) {
        return;
    }
    const int32_t nRows = CalcTileRows(rc.Height());
    int64_t nTopIndex = 0;
    if (elementWidth > 0) {
        nTopIndex = (nPos / elementWidth) * nRows;
    }
    int64_t nNewPos = 0;

    if (bToTop) {
        nNewPos = GetElementsWidth(rc, iIndex);
        if (nNewPos >= elementWidth) {
            nNewPos -= elementWidth;
        }
    }
    else {
        if (IsElementDisplay(rc, iIndex)) {
            return;
        }

        int64_t nTopColumns = nTopIndex / nRows;
        int64_t nDestColumns = iIndex / nRows;
        if ((int64_t)nDestColumns > nTopColumns) {
            // 向右滚动：确保在最右侧
            int64_t width = GetElementsWidth(rc, iIndex + 1);
            nNewPos = width - pOwnerBox->GetRect().Width();
        }
        else {
            // 向左滚动：确保在最左侧
            nNewPos = GetElementsWidth(rc, iIndex + 1);
            if (nNewPos >= elementWidth) {
                nNewPos -= elementWidth;
            }
        }
    }
    if (nNewPos < 0) {
        nNewPos = 0;
    }
    if (nNewPos > pOwnerBox->m_pHScrollBar->GetScrollRange()) {
        nNewPos = pOwnerBox->m_pHScrollBar->GetScrollRange();
    }
    ui::UiSize64 sz(nNewPos, 0);
    pOwnerBox->SetScrollPos(sz);
}
} // namespace ui
