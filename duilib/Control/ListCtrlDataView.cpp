#include "ListCtrlDataView.h"
#include "ListCtrl.h"

//包含类：ListCtrlDataView / ListCtrlDataLayout

namespace ui
{
ListCtrlDataView::ListCtrlDataView() :
    VirtualListBox(new ListCtrlDataLayout),
    m_pListCtrl(nullptr),
    m_nTopElementIndex(0)
{
    VirtualLayout* pVirtualLayout = dynamic_cast<VirtualLayout*>(GetLayout());
    SetVirtualLayout(pVirtualLayout);
}

ListCtrlDataView::~ListCtrlDataView() 
{
}

void ListCtrlDataView::SetListCtrl(ListCtrl* pListCtrl)
{
    m_pListCtrl = pListCtrl;
}

int32_t ListCtrlDataView::GetListCtrlWidth() const
{
    int32_t nToltalWidth = 0;
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return nToltalWidth;
    }
    ListCtrlHeader* pHeaderCtrl = m_pListCtrl->GetListCtrlHeader();
    if (pHeaderCtrl == nullptr) {
        return nToltalWidth;
    }
    size_t nColumnCount = pHeaderCtrl->GetColumnCount();
    for (size_t index = 0; index < nColumnCount; ++index) {
        ListCtrlHeaderItem* pHeaderItem = pHeaderCtrl->GetColumn(index);
        if ((pHeaderItem != nullptr) && pHeaderItem->IsVisible()) {
            nToltalWidth += pHeaderItem->GetColumnWidth();
        }
    }
    return nToltalWidth;
}

void ListCtrlDataView::SetTopElementIndex(size_t nTopElementIndex)
{
    m_nTopElementIndex = nTopElementIndex;
}

size_t ListCtrlDataView::GetTopElementIndex() const
{
    return m_nTopElementIndex;
}

void ListCtrlDataView::OnRefresh()
{
}

void ListCtrlDataView::OnArrangeChild()
{
    if (m_pListCtrl != nullptr) {
        m_pListCtrl->UpdateControlCheckStatus(Box::InvalidIndex);
    }
}


UiSize64 ListCtrlDataLayout::ArrangeChild(const std::vector<ui::Control*>& /*items*/, ui::UiRect rc)
{
    ListCtrlDataView* pList = dynamic_cast<ListCtrlDataView*>(m_pOwner);
    if ((pList == nullptr) || !pList->HasDataProvider()) {
        ASSERT(FALSE);
        return UiSize64();
    }
    DeflatePadding(rc);
    int64_t nTotalHeight = GetElementsHeight(rc, Box::InvalidIndex);
    UiSize64 sz(rc.Width(), rc.Height());
    sz.cy = std::max(nTotalHeight, sz.cy);
    sz.cx = std::max(GetItemWidth(), rc.Width()); //允许出现横向滚动条
    LazyArrangeChild(rc);
    return sz;
}

UiSize ListCtrlDataLayout::EstimateSizeByChild(const std::vector<Control*>& /*items*/, ui::UiSize szAvailable)
{
    ListCtrlDataView* pList = dynamic_cast<ListCtrlDataView*>(m_pOwner);
    if ((pList == nullptr) || !pList->HasDataProvider()) {
        ASSERT(FALSE);
        return UiSize();
    }
    UiRect rc(0, 0, szAvailable.cx, szAvailable.cy);
    UiSize szItem = GetElementSize(rc, 0);
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return UiSize();
    }
    szAvailable.Validate();
    int32_t nColumns = CalcTileColumns(szAvailable.cx);
    UiEstSize estSize;
    if (m_pOwner != nullptr) {
        estSize = m_pOwner->Control::EstimateSize(szAvailable);
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

void ListCtrlDataLayout::LazyArrangeChild(UiRect rc) const
{
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return;
    }
    if (!pOwnerBox->HasDataProvider()) {
        return;
    }

    const size_t nItemCount = pOwnerBox->GetItemCount();
    if (nItemCount > 0) {
        //第一个元素是表头控件，设置其位置大小
        Control* pHeaderCtrl = pOwnerBox->GetItemAt(0);
        if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
            int32_t nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
            if (nHeaderHeight > 0) {
                int32_t nHeaderWidth = GetElementSize(rc, 0).cx;
                if (nHeaderWidth <= 0) {
                    nHeaderWidth = rc.Width();
                }
                ui::UiRect rcTile(rc.left, rc.top, rc.left + nHeaderWidth, rc.top + nHeaderHeight);
                pHeaderCtrl->SetPos(rcTile);
                rc.top += nHeaderHeight;
            }
        }
    }

    // 顶部元素的索引号
    const size_t nTopElementIndex = GetTopElementIndex(rc);

    //列数
    int32_t nColumns = CalcTileColumns(rc.Width());

    //子项的左边起始位置 
    int32_t iPosLeft = rc.left;

    //Y轴坐标的偏移，需要保持，避免滚动位置变动后，重新刷新界面出现偏差
    int32_t yOffset = 0;
    UiSize szTopItem = GetElementSize(rc, nTopElementIndex);
    if (szTopItem.cy > 0) {
        yOffset = TruncateToInt32(pOwnerBox->GetScrollPos().cy % szTopItem.cy);
    }

    //子项的顶部起始位置
    int32_t iPosTop = rc.top - yOffset;

    //设置虚拟偏移，否则当数据量较大时，rc这个32位的矩形的高度会越界，需要64位整型才能容纳
    pOwnerBox->SetScrollVirtualOffsetY(pOwnerBox->GetScrollPos().cy);

    //控件的左上角坐标值
    ui::UiPoint ptTile(iPosLeft, iPosTop);

    UiSize szItem;
    size_t iCount = 0;
    for (size_t index = 1; index < nItemCount; ++index) {
        //第一个元素是表头控件，跳过填充数据
        Control* pControl = pOwnerBox->GetItemAt(index);
        if (pControl == nullptr) {
            continue;
        }
        //当前数据元素的索引号
        size_t nElementIndex = nTopElementIndex + iCount;
        szItem = GetElementSize(rc, nElementIndex);

        //设置当前控件的大小和位置
        ui::UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + szItem.cx, ptTile.y + szItem.cy);
        pControl->SetPos(rcTile);

        // 填充数据        
        if (nElementIndex < pOwnerBox->GetElementCount()) {
            if (!pControl->IsVisible()) {
                pControl->SetVisible(true);
            }
            pOwnerBox->FillElement(pControl, nElementIndex);
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
    pOwnerBox->OnArrangeChild();
}

size_t ListCtrlDataLayout::AjustMaxItem(UiRect rc) const
{
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return 0;
    }
    int32_t nItemHeight = GetItemHeight();
    ASSERT(nItemHeight > 0);
    if (nItemHeight <= 0) {
        return 0;
    }
    if (rc.IsEmpty()) {
        return 0;
    }
    int32_t nColumns = CalcTileColumns(rc.Width());
    int32_t nRows = rc.Height() / (nItemHeight + GetChildMarginY() / 2);
    //验证并修正
    if (nRows > 1) {
        int32_t calcHeight = nRows * nItemHeight + (nRows - 1) * GetChildMarginY();
        if (calcHeight < rc.Height()) {
            nRows += 1;
        }
    }
    //额外增加1行，确保真实控件填充满整个可显示区域
    nRows += 1;
    return nRows * nColumns;
}

size_t ListCtrlDataLayout::GetTopElementIndex(UiRect rc) const
{
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
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
    int32_t nItemHeight = GetItemHeight();
    ASSERT(nItemHeight > 0);
    if (nItemHeight <= 0) {
        return 0;
    }
    int64_t iIndex = (nPos / nItemHeight) * nColumns;
    return static_cast<size_t>(iIndex);
}

bool ListCtrlDataLayout::IsElementDisplay(UiRect rc, size_t iIndex) const
{
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return false;
    }

    int64_t nPos = pOwnerBox->GetScrollPos().cy;
    int64_t nElementPos = GetElementsHeight(rc, iIndex);
    if (nElementPos >= nPos) {
        int64_t nHeight = pOwnerBox->GetHeight();
        if (nElementPos <= nPos + nHeight) {
            return true;
        }
    }
    return false;
}

bool ListCtrlDataLayout::NeedReArrange() const
{
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
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

    if (nScrollPosY >= nVirtualOffsetY) {
        //向下滚动
        ui::UiRect rcItem = pOwnerBox->GetItemAt(nCount - 1)->GetPos();
        if ((rcItem.bottom + nVirtualOffsetY) < (nScrollPosY + rcThis.bottom)) {
            return true;
        }
    }
    else {
        //向上滚动
        ui::UiRect rcItem = pOwnerBox->GetItemAt(0)->GetPos();
        if ((rcItem.top + nVirtualOffsetY) > (nScrollPosY + rcThis.top)) {
            return true;
        }
    }
    return false;
}

void ListCtrlDataLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const
{
    collection.clear();
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
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

void ListCtrlDataLayout::EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const
{
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
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
    if (elementHeight == 0) {
        return;
    }
    int32_t nColumns = CalcTileColumns(rc.Width());
    int64_t nTopIndex = 0;
    if (elementHeight > 0) {
        nTopIndex = (nPos / elementHeight) * nColumns;
    }
    int64_t nNewPos = 0;

    if (bToTop)
    {
        nNewPos = GetElementsHeight(rc, iIndex);
        if (nNewPos > elementHeight) {
            nNewPos -= elementHeight;
        }
    }
    else {
        if (IsElementDisplay(rc, iIndex)) {
            return;
        }

        if ((int64_t)iIndex > nTopIndex) {
            // 向下
            int64_t height = GetElementsHeight(rc, iIndex + 1);
            nNewPos = height - pOwnerBox->GetRect().Height();
        }
        else {
            // 向上
            nNewPos = GetElementsHeight(rc, iIndex);
            if (nNewPos > elementHeight) {
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

ListCtrlDataView* ListCtrlDataLayout::GetOwnerBox() const
{
    ListCtrlDataView* pList = dynamic_cast<ListCtrlDataView*>(m_pOwner);
    ASSERT(pList != nullptr);
    return pList;
}

int64_t ListCtrlDataLayout::GetElementsHeight(UiRect rc, size_t nCount) const
{
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return 0;
    }
    int32_t nItemHeight = GetItemHeight();
    ASSERT(nItemHeight > 0);
    if (nItemHeight <= 0) {
        return 0;
    }
    int32_t nColumns = CalcTileColumns(rc.Width());
    if (nCount <= nColumns && nCount != Box::InvalidIndex) {
        //不到1行，或者刚好1行
        return nItemHeight + GetChildMarginY();
    }
    if (!Box::IsValidItemIndex(nCount)) {
        nCount = pOwnerBox->GetElementCount();
    }
    if (!Box::IsValidItemIndex(nCount)) {
        ASSERT(FALSE);
        return nItemHeight + GetChildMarginY();
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
        return nItemHeight * (rows + 1) + childMarginTotal;
    }
    return 0;
}

UiSize ListCtrlDataLayout::GetElementSize(UiRect rc, size_t /*nElementIndex*/) const
{
    UiSize szElementSize;
    szElementSize.cx = std::max(GetItemWidth(), rc.Width());
    szElementSize.cy = GetItemHeight();
    return szElementSize;
}

int32_t ListCtrlDataLayout::CalcTileColumns(int32_t /*rcWidth*/) const
{
    //固定，只有一列
    return 1;
}

int32_t ListCtrlDataLayout::GetItemWidth() const
{
    int32_t nItemWidth = 0;
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
    if (pOwnerBox != nullptr) {
        nItemWidth = pOwnerBox->GetListCtrlWidth();
    }
    return nItemWidth;
}

int32_t ListCtrlDataLayout::GetItemHeight() const
{
    //////////////////////////////////////////
    //TODO:
    return 64;
}

int32_t ListCtrlDataLayout::GetHeaderHeight() const
{
    int32_t nHeaderHeight = 0;
    size_t nItemCount = 0;
    ListCtrlDataView* pOwnerBox = GetOwnerBox();
    if (pOwnerBox != nullptr) {
        nItemCount = pOwnerBox->GetItemCount();
    }
    if (nItemCount > 0) {
        //第一个元素是表头控件，设置其位置大小
        Control* pHeaderCtrl = pOwnerBox->GetItemAt(0);
        if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
            nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
        }
    }
    return nHeaderHeight;
}

}//namespace ui

