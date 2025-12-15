#include "VirtualHLayout.h"
#include "duilib/Box/VirtualListBox.h"

namespace ui 
{
VirtualHLayout::VirtualHLayout():
    m_bAutoCalcItemHeight(false)
{
    //默认居中对齐
    SetChildVAlignType(VerAlignType::kAlignCenter);
}

bool VirtualHLayout::SetAttribute(const DString& strName, const DString& strValue, const DpiManager& dpiManager)
{
    bool hasAttribute = true;
    if ((strName == _T("item_size")) || (strName == _T("itemsize"))) {
        UiSize szItem;
        AttributeUtil::ParseSizeValue(strValue.c_str(), szItem);
        dpiManager.ScaleSize(szItem);
        SetItemSize(szItem);
    }
    else if (strName == _T("auto_calc_item_size")) {
        SetAutoCalcItemHeight(strValue == _T("true"));
    }
    else {
        hasAttribute = BaseClass::SetAttribute(strName, strValue, dpiManager);
    }
    return hasAttribute;
}

VirtualListBox* VirtualHLayout::GetOwnerBox() const
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    ASSERT(pList != nullptr);
    return pList;
}

UiSize64 VirtualHLayout::ArrangeChildren(const std::vector<ui::Control*>& items, ui::UiRect rc, bool bEstimateOnly)
{
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    if ((pList == nullptr) || !pList->HasDataProvider()) {
        //如果未设置数据接口，则兼容基类的功能
        return BaseClass::ArrangeChildren(items, rc, bEstimateOnly);
    }
    DeflatePadding(rc);
    if (IsAutoCalcItemHeight()) {
        //设置了自动计算子项高度：占满整个高度
        UiSize szNewItemSize = GetItemSize();
        szNewItemSize.cy = rc.Height();
        SetItemSize(szNewItemSize, false);
    }
    int64_t nTotalWidth = GetElementsWidth(rc, Box::InvalidIndex);
    UiSize64 sz(rc.Width(), rc.Height());
    sz.cx = std::max(nTotalWidth, sz.cx);
    sz.cy = std::max((int64_t)GetItemSize().cy, sz.cy);//支持纵向滚动条
    if (!bEstimateOnly) {
        LazyArrangeChild(rc);
    }    
    return sz;
}

UiSize64 VirtualHLayout::EstimateLayoutSize(const std::vector<Control*>& items, ui::UiSize szAvailable)
{
    //估算控件大小时（主要是用于宽高为"auto"类型的情况），只估算容器本身的大小，不包含列表数据的大小
    //因为虚表数据规模较大，不适合用于估算"auto"控件的大小
    VirtualListBox* pList = dynamic_cast<VirtualListBox*>(GetOwner());
    if ((pList == nullptr) || !pList->HasDataProvider()) {
        //如果未设置数据接口，则兼容基类的功能
        return BaseClass::EstimateLayoutSize(items, szAvailable);
    }
    szAvailable.Validate();
    UiPadding rcPadding;
    if (GetOwner() != nullptr) {
        rcPadding = GetOwner()->GetPadding();
    }
    UiSize szAvailableLocal = szAvailable;
    szAvailableLocal.cx -= (rcPadding.left + rcPadding.right);
    szAvailableLocal.cy -= (rcPadding.top + rcPadding.bottom);
    szAvailableLocal.Validate();

    if (IsAutoCalcItemHeight()) {
        //设置了固定行，并且设置了自动计算子项高度
        UiSize szNewItemSize = GetItemSize();
        szNewItemSize.cy = szAvailableLocal.cy;
        SetItemSize(szNewItemSize, false);
    }

    UiEstSize estSize;
    if (GetOwner() != nullptr) {
        estSize = GetOwner()->Control::EstimateSize(szAvailableLocal);
    }
    int32_t nTotalWidth = estSize.cx.GetInt32();
    int32_t nTotalHeight = estSize.cy.GetInt32();
    if (estSize.cx.IsStretch()) {
        nTotalWidth = CalcStretchValue(estSize.cx, szAvailableLocal.cx);
        if (nTotalWidth > 0) {
            nTotalWidth += (rcPadding.left + rcPadding.right);
        }
    }
    if (estSize.cy.IsStretch()) {
        nTotalHeight = CalcStretchValue(estSize.cy, szAvailableLocal.cy);
        if (nTotalHeight > 0) {
            nTotalHeight += (rcPadding.top + rcPadding.bottom);
        }
    }
    if (nTotalWidth == 0) {
        UiSize szItem = GetItemSize();
        if (szItem.cx > 0) {
            nTotalWidth = szItem.cx + (rcPadding.left + rcPadding.right);
        }
    }
    if (nTotalHeight == 0) {
        UiSize szItem = GetItemSize();
        if (szItem.cy > 0) {
            nTotalHeight = szItem.cy + (rcPadding.top + rcPadding.bottom);
        }
    }
    nTotalWidth = std::max(nTotalWidth, 0);
    nTotalHeight = std::max(nTotalHeight, 0);
    return UiSize64(nTotalWidth, nTotalHeight);
}

void VirtualHLayout::ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale)
{
    UiSize szItem = GetItemSize();
    szItem = dpiManager.GetScaleSize(szItem, nOldDpiScale);
    SetItemSize(szItem);
    BaseClass::ChangeDpiScale(dpiManager, nOldDpiScale);
}

void VirtualHLayout::SetItemSize(UiSize szItem, bool bArrange)
{
    szItem.cx = std::max(szItem.cx, 0);
    szItem.cy = std::max(szItem.cy, 0);
    ASSERT((szItem.cx > 0) && (szItem.cy > 0));
    if ((m_szItem.cx != szItem.cx) || (m_szItem.cy != szItem.cy)) {
        m_szItem = szItem;
        if (bArrange && (GetOwner() != nullptr)) {
            GetOwner()->Arrange();
        }
    }
}

const UiSize& VirtualHLayout::GetItemSize() const
{
    return m_szItem;
}

void VirtualHLayout::SetAutoCalcItemHeight(bool bAutoCalcItemHeight)
{
    if (m_bAutoCalcItemHeight != bAutoCalcItemHeight) {
        m_bAutoCalcItemHeight = bAutoCalcItemHeight;
        if (GetOwner() != nullptr) {
            GetOwner()->Arrange();
        }
    }
}

bool VirtualHLayout::IsAutoCalcItemHeight() const
{
    return m_bAutoCalcItemHeight;
}

int64_t VirtualHLayout::GetElementsWidth(UiRect /*rc*/, size_t nCount) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) && (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return 0;
    }
    if (nCount <= 1) {
        return (int64_t)szItem.cx + GetChildMarginX();
    }
    if (!Box::IsValidItemIndex(nCount)) {
        VirtualListBox* pList = GetOwnerBox();
        ASSERT(pList != nullptr);
        if (pList != nullptr) {
            nCount = pList->GetElementCount();
        }
    }
    if (!Box::IsValidItemIndex(nCount)) {
        ASSERT(0);
        return (int64_t)szItem.cx + GetChildMarginX();
    }

    int64_t cols = nCount;
    int64_t iChildMargin = 0;
    if (GetChildMarginX() > 0) {
        iChildMargin = GetChildMarginX();
    }
    int64_t childMarginTotal = ((int64_t)nCount - 1) * iChildMargin;
    return szItem.cx * cols + childMarginTotal;
}

void VirtualHLayout::LazyArrangeChild(UiRect rc) const
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

    //X轴坐标的偏移，需要保持，避免滚动位置变动后，重新刷新界面出现偏差
    int32_t xOffset = 0;
    int64_t itemWidth = GetElementsWidth(rc, 1);
    if (itemWidth > 0) {
        xOffset = TruncateToInt32(pOwnerBox->GetScrollPos().cx % itemWidth);
    }

    //子项的左边起始位置 
    int32_t iPosLeft = rc.left - xOffset;

    //子项的顶部起始位置
    int32_t iPosTop = rc.top;

    //确定对齐方式
    if (szItem.cy < rc.Height()) {
        VerAlignType vAlign = GetChildVAlignType();
        if (vAlign == VerAlignType::kAlignCenter) {
            iPosTop = rc.CenterY() - szItem.cy / 2;
        }
        else if (vAlign == VerAlignType::kAlignBottom) {
            iPosTop = rc.bottom - szItem.cy;
        }
    }

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
            pOwnerBox->FillElementData(pControl, nElementIndex);
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
        ptTile.y = iPosTop;
        ptTile.x += szItem.cx + GetChildMarginX();
    }
    if (!refreshDataList.empty()) {
        pOwnerBox->OnRefreshElements(refreshDataList);
        pOwnerBox->OnFilledElements(refreshDataList);
    }
}

size_t VirtualHLayout::AjustMaxItem(UiRect rc) const
{
    UiSize szItem = GetItemSize();
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return 0;
    }
    if (rc.IsEmpty()) {
        return 0;
    }
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
    return nColumns;
}

size_t VirtualHLayout::GetTopElementIndex(UiRect rc) const
{
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return 0;
    }
    int64_t nPos = pOwnerBox->GetScrollPos().cx;
    if (nPos < 0) {
        nPos = 0;
    }
    int64_t nWidth = GetElementsWidth(rc, 1);
    ASSERT(nWidth >= 0);
    if (nWidth <= 0) {
        return 0;
    }
    int64_t iIndex = nPos / nWidth;
    return static_cast<size_t>(iIndex);
}

bool VirtualHLayout::IsElementDisplay(UiRect rc, size_t iIndex) const
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
    int64_t nElementWidth = GetElementsWidth(rc, 1);
    if ((nElementPos - nElementWidth) > nScrollPos) { //矩形的left位置
        int64_t nBoxWidth = pOwnerBox->GetWidth();
        if (nElementPos <= (nScrollPos + nBoxWidth)) {//矩形的right位置
            return true;
        }
    }
    return false;
}

bool VirtualHLayout::NeedReArrange() const
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

void VirtualHLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const
{
    collection.clear();
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return;
    }

    if (pOwnerBox->GetItemCount() == 0) {
        return;
    }

    int64_t nEleWidth = GetElementsWidth(rc, 1);
    if (nEleWidth <= 0) {
        return;
    }

    size_t min = (size_t)(pOwnerBox->GetScrollPos().cx / nEleWidth);
    size_t max = min + (size_t)(rc.Width() / nEleWidth);

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

void VirtualHLayout::EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const
{
    VirtualListBox* pOwnerBox = GetOwnerBox();
    if (pOwnerBox == nullptr) {
        return;
    }
    if (!Box::IsValidItemIndex(iIndex) || iIndex >= pOwnerBox->GetElementCount()) {
        return;
    }
    if (pOwnerBox->GetHScrollBar() == nullptr) {
        return;
    }
    int64_t nPos = pOwnerBox->GetScrollPos().cx;
    int64_t elementWidth = GetElementsWidth(rc, 1);
    if (elementWidth <= 0) {
        return;
    }
    int64_t nTopIndex = (nPos / elementWidth);
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

        if ((int64_t)iIndex > nTopIndex) {
            // 向下
            int64_t nWidth = GetElementsWidth(rc, iIndex + 1);
            nNewPos = nWidth - pOwnerBox->GetRect().Width();
        }
        else {
            // 向上
            nNewPos = GetElementsWidth(rc, iIndex + 1);
            if (nNewPos >= elementWidth) {
                nNewPos -= elementWidth;
            }
        }
    }
    if (nNewPos < 0) {
        nNewPos = 0;
    }
    if (nNewPos > pOwnerBox->GetHScrollBar()->GetScrollRange()) {
        nNewPos = pOwnerBox->GetHScrollBar()->GetScrollRange();
    }
    ui::UiSize64 sz(nNewPos, 0);
    pOwnerBox->SetScrollPos(sz);
}
} // namespace ui
