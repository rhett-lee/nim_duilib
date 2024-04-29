#include "VirtualVTileLayout.h"
#include "duilib/Box/VirtualListBox.h"

namespace ui 
{

VirtualVTileLayout::VirtualVTileLayout()
{
    SetColumns(0);
    SetAutoCalcColumns(true);
}

VirtualListBox* VirtualVTileLayout::GetOwnerBox() const
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    ASSERT(pList != nullptr);
    return pList;
}

int32_t VirtualVTileLayout::CalcTileColumns(int32_t rcWidth) const
{
    int32_t nColumns = GetColumns();
    if (nColumns < 0) {
        nColumns = 0;
    }
    if (IsAutoCalcColumns()) {
        nColumns = 0;
    }
    if (nColumns > 0) {
        return nColumns;
    }
    int32_t totalWidth = rcWidth;
    int32_t tileWidth = GetItemSize().cx;
    int32_t childMarginX = GetChildMarginX();
    if (childMarginX < 0) {
        childMarginX = 0;
    }
    if (tileWidth > 0) {
        while (totalWidth > 0) {
            totalWidth -= tileWidth;
            if (nColumns != 0) {
                totalWidth -= childMarginX;
            }
            if (totalWidth >= 0) {
                ++nColumns;
            }
        }
    }
    if (nColumns < 1) {
        nColumns = 1;
    }
    return nColumns;
}

UiSize64 VirtualVTileLayout::ArrangeChild(const std::vector<ui::Control*>& items, ui::UiRect rc)
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    if ((pList == nullptr) || !pList->HasDataProvider()) {
        //如果未设置数据接口，则兼容基类的功能
        return __super::ArrangeChild(items, rc);
    }
    DeflatePadding(rc);
    int64_t nTotalHeight = GetElementsHeight(rc, Box::InvalidIndex);
    UiSize64 sz(rc.Width(), rc.Height());
    sz.cy = std::max(nTotalHeight, sz.cy);
    LazyArrangeChild(rc);
    return sz;
}

UiSize VirtualVTileLayout::EstimateSizeByChild(const std::vector<Control*>& items, ui::UiSize szAvailable)
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
    int32_t nColumns = CalcTileColumns(szAvailable.cx);
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
    if (size.cx == 0) {
        size.cx = szItem.cx * nColumns + GetChildMarginX() * (nColumns - 1);
    }
    size.Validate();
    return size;
}

int64_t VirtualVTileLayout::GetElementsHeight(UiRect rc, size_t nCount) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return 0;
    }
    int32_t nColumns = CalcTileColumns(rc.Width());
    if (nCount <= nColumns && nCount != Box::InvalidIndex) {
        //不到1行，或者刚好1行
        return szItem.cy + GetChildMarginY();
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
        return szItem.cy + GetChildMarginY();
    }

    int64_t rows = nCount / nColumns;
    if (nCount % nColumns != 0) {
        rows += 1;
    }
    int64_t iChildMargin = 0;
    if (GetChildMarginY() > 0) {
        iChildMargin = GetChildMarginY();
    }
    if (nCount > 0) {
        int64_t childMarginTotal = 0;
        if (nCount % nColumns == 0) {
            childMarginTotal = ((int64_t)nCount / nColumns - 1) * iChildMargin;
        }
        else {
            childMarginTotal = ((int64_t)nCount / nColumns) * iChildMargin;
        }
        return szItem.cy * rows + childMarginTotal;
    }
    return 0;
}

void VirtualVTileLayout::LazyArrangeChild(UiRect rc) const
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

    //列数
    int32_t nColumns = CalcTileColumns(rc.Width());

    //子项的左边起始位置 
    int32_t iPosLeft = rc.left;

    //Y轴坐标的偏移，需要保持，避免滚动位置变动后，重新刷新界面出现偏差
    int32_t yOffset = 0;
    int64_t itemHeight = GetElementsHeight(rc, 1);
    if (itemHeight > 0) {
        yOffset = TruncateToInt32(pOwnerBox->GetScrollPos().cy % itemHeight);
    }

    //子项的顶部起始位置
    int32_t iPosTop = rc.top - yOffset;

    //设置虚拟偏移，否则当数据量较大时，rc这个32位的矩形的高度会越界，需要64位整型才能容纳
    pOwnerBox->SetScrollVirtualOffsetY(pOwnerBox->GetScrollPos().cy);

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

        if ((++iCount % nColumns) == 0) {
            ptTile.x = iPosLeft;
            ptTile.y += szItem.cy + GetChildMarginY();
        }
        else {
            ptTile.x += rcTile.Width() + GetChildMarginX();
        }
    }
    if (!refreshDataList.empty()) {
        pOwnerBox->OnRefreshElements(refreshDataList);
    }
}

size_t VirtualVTileLayout::AjustMaxItem(UiRect rc) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return 0;
    }
    if (rc.IsEmpty()) {
        return 0;
    }
    int32_t nColumns = CalcTileColumns(rc.Width());
    int32_t nRows = rc.Height() / (szItem.cy + GetChildMarginY() / 2);
    //验证并修正
    if (nRows > 1) {
        int32_t calcHeight = nRows * szItem.cy + (nRows - 1) * GetChildMarginY();
        if (calcHeight < rc.Height()) {
            nRows += 1;
        }
    }
    //额外增加1行，确保真实控件填充满整个可显示区域
    nRows += 1;
    return nRows * nColumns;
}

size_t VirtualVTileLayout::GetTopElementIndex(UiRect rc) const
{
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return 0;
    }
    int64_t nPos = pOwnerBox->GetScrollPos().cy;
    if (nPos < 0) {
        nPos = 0;
    }

    int64_t nColumns = (int64_t)CalcTileColumns(rc.Width());
    if (nColumns < 0) {
        nColumns = 0;
    }
    int64_t nHeight = GetElementsHeight(rc, 1);
    ASSERT(nHeight >= 0);
    if (nHeight <= 0) {
        return 0;
    }
    int64_t iIndex = (nPos / nHeight) * nColumns;
    return static_cast<size_t>(iIndex);
}

bool VirtualVTileLayout::IsElementDisplay(UiRect rc, size_t iIndex) const
{
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return false;
    }

    int64_t nScrollPos = pOwnerBox->GetScrollPos().cy;
    int64_t nElementPos = GetElementsHeight(rc, iIndex + 1);
    UiSize szItem = GetItemSize();
    if ((nElementPos - szItem.cy) > nScrollPos) { //矩形的top位置
        int64_t nBoxHeight = pOwnerBox->GetHeight();
        if (nElementPos <= (nScrollPos + nBoxHeight)) {//矩形的bottom位置
            //完整显示
            return true;
        }
    }
    return false;
}

bool VirtualVTileLayout::NeedReArrange() const
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

    int64_t nScrollPosY = pOwnerBox->GetScrollPos().cy;
    int64_t nVirtualOffsetY = pOwnerBox->GetScrollVirtualOffset().cy;
    return nVirtualOffsetY != nScrollPosY;
}

void VirtualVTileLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const
{
    collection.clear();
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return;
    }

    if (pOwnerBox->GetItemCount() == 0) {
        return;
    }

    size_t nEleHeight = GetElementsHeight(rc, 1);
    if (nEleHeight == 0) {
        return;
    }

    int32_t nColumns = CalcTileColumns(rc.Width());
    size_t min = ((size_t)pOwnerBox->GetScrollPos().cy / nEleHeight) * nColumns;
    size_t max = min + ((size_t)rc.Height() / nEleHeight) * nColumns;

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

void VirtualVTileLayout::EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const
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
    int64_t nPos = pOwnerBox->GetScrollPos().cy;
    int64_t elementHeight = GetElementsHeight(rc, 1);
    if (elementHeight <= 0) {
        return;
    }
    const int32_t nColumns = CalcTileColumns(rc.Width());
    int64_t nTopIndex = 0;
    if (elementHeight > 0) {
        nTopIndex = (nPos / elementHeight) * nColumns;
    }
    int64_t nNewPos = 0;

    if (bToTop) {
        nNewPos = GetElementsHeight(rc, iIndex);
        if (nNewPos >= elementHeight) {
            nNewPos -= elementHeight;
        }
    }
    else {
        if (IsElementDisplay(rc, iIndex)) {
            return;
        }
        int64_t nTopRows = nTopIndex / nColumns;
        int64_t nDestRows = iIndex / nColumns;
        if ((int64_t)nDestRows > nTopRows) {
            // 向下滚动：确保在底部
            int64_t height = GetElementsHeight(rc, iIndex + 1);
            nNewPos = height - pOwnerBox->GetRect().Height();
        }
        else {
            // 向上滚动：确保在顶部
            nNewPos = GetElementsHeight(rc, iIndex + 1);
            if (nNewPos >= elementHeight) {
                nNewPos -= elementHeight;
            }
        }
    }
    if (nNewPos < 0) {
        nNewPos = 0;
    }
    if (nNewPos > pOwnerBox->m_pVScrollBar->GetScrollRange()) {
        nNewPos = pOwnerBox->m_pVScrollBar->GetScrollRange();
    }
    ui::UiSize64 sz(0, nNewPos);
    pOwnerBox->SetScrollPos(sz);
}
} // namespace ui
