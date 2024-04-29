#include "VirtualVLayout.h"
#include "duilib/Box/VirtualListBox.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui 
{

VirtualVLayout::VirtualVLayout()
{
}

VirtualListBox* VirtualVLayout::GetOwnerBox() const
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    ASSERT(pList != nullptr);
    return pList;
}

UiSize64 VirtualVLayout::ArrangeChild(const std::vector<ui::Control*>& items, ui::UiRect rc)
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

UiSize VirtualVLayout::EstimateSizeByChild(const std::vector<Control*>& items, ui::UiSize szAvailable)
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    if ((pList == nullptr) || !pList->HasDataProvider()) {
        //如果未设置数据接口，则兼容基类的功能
        return __super::EstimateSizeByChild(items, szAvailable);
    }
    szAvailable.Validate();
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
    if ((size.cx == 0) || (size.cy == 0)) {
        UiPadding rcPadding;
        if (GetOwner() != nullptr) {
            rcPadding = GetOwner()->GetPadding();
        }
        UiSize szItem = GetItemSize();
        szItem.cx += (rcPadding.left + rcPadding.right);
        szItem.cy += (rcPadding.top + rcPadding.bottom);

        if (size.cx == 0) {
            size.cx = szItem.cx;
        }
        if (size.cy == 0) {
            size.cy = szItem.cy;
        }
    }
    size.Validate();
    return size;
}

bool VirtualVLayout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    bool hasAttribute = true;
    if ((strName == L"item_size") || (strName == L"itemsize")) {
        UiSize szItem;
        AttributeUtil::ParseSizeValue(strValue.c_str(), szItem);
        SetItemSize(szItem);
    }
    else {
        hasAttribute = VLayout::SetAttribute(strName, strValue);
    }
    return hasAttribute;
}

void VirtualVLayout::SetItemSize(UiSize szItem, bool bNeedDpiScale)
{
    szItem.cx = std::max(szItem.cx, 0);
    szItem.cy = std::max(szItem.cy, 0);
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleSize(szItem);
    }
    ASSERT((szItem.cx > 0) && (szItem.cy > 0));
    if ((m_szItem.cx != szItem.cx) || (m_szItem.cy != szItem.cy)) {
        m_szItem = szItem;
        if (GetOwner() != nullptr) {
            GetOwner()->Arrange();
        }       
    }
}

const UiSize& VirtualVLayout::GetItemSize() const
{
    return m_szItem;
}

int64_t VirtualVLayout::GetElementsHeight(UiRect /*rc*/, size_t nCount) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return 0;
    }
    if (nCount <= 1) {
        return szItem.cy + GetChildMarginY();
    }
    if (!Box::IsValidItemIndex(nCount)) {
        VirtualListBox* pList = GetOwnerBox();
        ASSERT(pList != nullptr);
        if (pList != nullptr) {
            nCount = pList->GetElementCount();
        }
    }
    if (!Box::IsValidItemIndex(nCount)) {
        ASSERT(FALSE);
        return szItem.cy + GetChildMarginY();
    }

    int64_t rows = nCount;
    int64_t iChildMargin = 0;
    if (GetChildMarginY() > 0) {
        iChildMargin = GetChildMarginY();
    }
    int64_t childMarginTotal = ((int64_t)nCount - 1) * iChildMargin;
    return szItem.cy * rows + childMarginTotal;
}

void VirtualVLayout::LazyArrangeChild(UiRect rc) const
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
        ++iCount;

        //换行
        ptTile.x = iPosLeft;
        ptTile.y += szItem.cy + GetChildMarginY();
    }
    if (!refreshDataList.empty()) {
        pOwnerBox->OnRefreshElements(refreshDataList);
    }
}

size_t VirtualVLayout::AjustMaxItem(UiRect rc) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return 0;
    }
    if (rc.IsEmpty()) {
        return 0;
    }
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
    return nRows;
}

size_t VirtualVLayout::GetTopElementIndex(UiRect rc) const
{
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return 0;
    }
    int64_t nPos = pOwnerBox->GetScrollPos().cy;
    if (nPos < 0) {
        nPos = 0;
    }
    int64_t nHeight = GetElementsHeight(rc, 1);
    ASSERT(nHeight >= 0);
    if (nHeight <= 0) {
        return 0;
    }
    int64_t iIndex = nPos / nHeight;
    return static_cast<size_t>(iIndex);
}

bool VirtualVLayout::IsElementDisplay(UiRect rc, size_t iIndex) const
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
    int64_t nElementHeight = GetElementsHeight(rc, 1);
    if ((nElementPos - nElementHeight) > nScrollPos) { //矩形的top位置
        int64_t nBoxHeight = pOwnerBox->GetHeight();
        if (nElementPos <= (nScrollPos + nBoxHeight)) {//矩形的bottom位置
            return true;
        }
    }
    return false;
}

bool VirtualVLayout::NeedReArrange() const
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

void VirtualVLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const
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

    size_t min = ((size_t)pOwnerBox->GetScrollPos().cy / nEleHeight);
    size_t max = min + ((size_t)rc.Height() / nEleHeight);

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

void VirtualVLayout::EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const
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
    int64_t nTopIndex = 0;
    if (elementHeight > 0) {
        nTopIndex = (nPos / elementHeight);
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

        if ((int64_t)iIndex > nTopIndex) {
            // 向下
            int64_t height = GetElementsHeight(rc, iIndex + 1);
            nNewPos = height - pOwnerBox->GetRect().Height();
        }
        else {
            // 向上
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
