#include "Layout.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/Box.h"

namespace ui 
{
Layout::Layout() :
    m_pOwner(nullptr),
    m_nChildMarginX(0),
    m_nChildMarginY(0),
    m_hChildAlignType(HorAlignType::kAlignLeft),
    m_vChildAlignType(VerAlignType::kAlignTop)
{
}

bool Layout::SetAttribute(const DString& strName, const DString& strValue, const DpiManager& dpiManager)
{
    bool hasAttribute = true;
    if ((strName == _T("child_margin")) || (strName == _T("childmargin"))) {
        int32_t iMargin = StringUtil::StringToInt32(strValue);
        dpiManager.ScaleInt(iMargin);
        SetChildMargin(iMargin);
    }
    else if ((strName == _T("child_margin_x")) || (strName == _T("childmarginx"))) {
        int32_t iMargin = StringUtil::StringToInt32(strValue);
        dpiManager.ScaleInt(iMargin);
        SetChildMarginX(iMargin);
    }
    else if ((strName == _T("child_margin_y")) || (strName == _T("childmarginy"))) {
        int32_t iMargin = StringUtil::StringToInt32(strValue);
        dpiManager.ScaleInt(iMargin);
        SetChildMarginY(iMargin);
    }
    else if (strName == _T("child_valign")) {
        //垂直对齐方式
        if (strValue == _T("top")) {
            SetChildVAlignType(VerAlignType::kAlignTop);
        }
        else if (strValue == _T("center")) {
            SetChildVAlignType(VerAlignType::kAlignCenter);
        }
        else if (strValue == _T("bottom")) {
            SetChildVAlignType(VerAlignType::kAlignBottom);
        }
        else {
            ASSERT(0);
        }
    }
    else if (strName == _T("child_halign")) {
        //水平对齐方式
        if (strValue == _T("left")) {
            SetChildHAlignType(HorAlignType::kAlignLeft);
        }
        else if (strValue == _T("center")) {
            SetChildHAlignType(HorAlignType::kAlignCenter);
        }
        else if (strValue == _T("right")) {
            SetChildHAlignType(HorAlignType::kAlignRight);
        }
        else {
            ASSERT(0);
        }
    }
    else if (strName == _T("child_align")) {
        //水平对齐
        if (strValue.find(_T("left")) != DString::npos) {
            SetChildHAlignType(HorAlignType::kAlignLeft);
        }
        else if (strValue.find(_T("hcenter")) != DString::npos) {
            SetChildHAlignType(HorAlignType::kAlignCenter);
        }
        else if (strValue.find(_T("right")) != DString::npos) {
            SetChildHAlignType(HorAlignType::kAlignRight);
        }

        //垂直对齐
        if (strValue.find(_T("top")) != DString::npos) {
            SetChildVAlignType(VerAlignType::kAlignTop);
        }
        else if (strValue.find(_T("vcenter")) != DString::npos) {
            SetChildVAlignType(VerAlignType::kAlignCenter);
        }
        else if (strValue.find(_T("bottom")) != DString::npos) {
            SetChildVAlignType(VerAlignType::kAlignBottom);
        }
    }
    else {
        hasAttribute = false;
    }
    return hasAttribute;
}

void Layout::ChangeDpiScale(const DpiManager& dpiManager, uint32_t nOldDpiScale)
{
    int32_t iMargin = GetChildMarginX();
    iMargin = dpiManager.GetScaleInt(iMargin, nOldDpiScale);
    SetChildMarginX(iMargin);

    iMargin = GetChildMarginY();
    iMargin = dpiManager.GetScaleInt(iMargin, nOldDpiScale);
    SetChildMarginY(iMargin);
}

bool Layout::LayoutByActualAreaSize() const
{
    LayoutType layoutType = GetLayoutType();
    if ((layoutType == LayoutType::FloatLayout) ||
        (layoutType == LayoutType::HLayout) ||
        (layoutType == LayoutType::VLayout) ||
        (layoutType == LayoutType::HFlowLayout) ||
        (layoutType == LayoutType::VFlowLayout) ||
        (layoutType == LayoutType::GridLayout)) {
        return true;
    }
    return false;
}

void Layout::SetOwner(Box* pOwner)
{
    m_pOwner = pOwner;
}

UiSize64 Layout::SetFloatPos(Control* pControl, const UiRect& rcContainer)
{
    return SetFloatPosInternal(pControl, rcContainer, false);
}

UiSize64 Layout::SetFloatPosInternal(Control* pControl, const UiRect& rcContainer, bool bEstimateOnly)
{
    ASSERT(pControl != nullptr);
    if ((pControl == nullptr) || (!pControl->IsVisible())) {
        return UiSize64();
    }    
    UiRect rc = rcContainer;
    rc.Deflate(pControl->GetMargin());
    UiSize szAvailable(rc.Width(), rc.Height());
    szAvailable.Validate();

    UiEstSize estSize = pControl->EstimateSize(szAvailable);
    UiSize childSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
    if (estSize.cx.IsStretch()) {
        childSize.cx = CalcStretchValue(estSize.cx, szAvailable.cx);
    }
    if (estSize.cy.IsStretch()) {
        childSize.cy = CalcStretchValue(estSize.cy, szAvailable.cy);
    }

    childSize.cx = std::clamp(childSize.cx, pControl->GetMinWidth(), pControl->GetMaxWidth());
    childSize.cy = std::clamp(childSize.cy, pControl->GetMinHeight(), pControl->GetMaxHeight());

    childSize.cx = std::max(childSize.cx, 0);
    childSize.cy = std::max(childSize.cy, 0);

    UiRect childPos = GetFloatPos(pControl, rcContainer, childSize);
    if (!bEstimateOnly) {
        //调整控件的位置和大小
        if (pControl->IsFloat() && pControl->IsKeepFloatPos() && (pControl->GetParent() != nullptr)) {
            //浮动控件：如果外部调整了其位置，则保持原位置
            UiSize oldFloatPos = pControl->GetFloatPos();
            if ((oldFloatPos.cx != INT32_MIN) && (oldFloatPos.cy != INT32_MIN)) {
                UiRect rcParent = pControl->GetParent()->GetRect();
                UiSize newFloatPos;
                newFloatPos.cx = childPos.left - rcParent.left;
                newFloatPos.cy = childPos.top - rcParent.top;
                childPos.Offset(oldFloatPos.cx - newFloatPos.cx, oldFloatPos.cy - newFloatPos.cy);
            }
        }
        pControl->SetPos(childPos);
    }
    return UiSize64(childPos.Width(), childPos.Height());
}

UiRect Layout::GetFloatPos(const Control* pControl, UiRect rcContainer, UiSize childSize)
{
    rcContainer.Validate();
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return UiRect();
    }
    UiMargin rcMargin = pControl->GetMargin();
    int32_t iPosLeft = rcContainer.left + rcMargin.left;
    int32_t iPosRight = rcContainer.right - rcMargin.right;
    int32_t iPosTop = rcContainer.top + rcMargin.top;
    int32_t iPosBottom = rcContainer.bottom - rcMargin.bottom;
    //如果空间不足，则宽高设置为零（如果界面可用调整大小，这个情况会频繁出现）
    if (iPosRight < iPosLeft) {
        iPosRight = iPosLeft;
    }
    if (iPosBottom < iPosTop) {
        iPosBottom = iPosTop;
    }
    
    childSize.cx = std::max(childSize.cx, 0);
    childSize.cy = std::max(childSize.cy, 0);

    int32_t childWidth = childSize.cx;
    int32_t childHeight = childSize.cy;

    //按照子控件指定的横向对齐方式和纵向对齐方式来排列控件
    const HorAlignType horAlignType = pControl->GetHorAlignType();
    const VerAlignType verAlignType = pControl->GetVerAlignType();

    int32_t childLeft = 0;
    int32_t childRight = 0;
    int32_t childTop = 0;
    int32_t childBottm = 0;

    //水平对齐方式
    if (horAlignType == HorAlignType::kAlignRight) {
        //靠右
        childRight = iPosRight;
        childLeft = childRight - childWidth;
    }
    else if (horAlignType == HorAlignType::kAlignCenter) {
        //水平居中
        childLeft = iPosLeft + (iPosRight - iPosLeft - childWidth) / 2;
        childRight = childLeft + childWidth;
    }
    else {
        //靠左（默认）
        childLeft = iPosLeft;
        childRight = childLeft + childWidth;
    }

    //垂直方向对齐方式
    if (verAlignType == VerAlignType::kAlignBottom) {
        //靠下
        childBottm = iPosBottom;
        childTop = childBottm - childHeight;
    }
    else if (verAlignType == VerAlignType::kAlignCenter) {
        //垂直居中
        childTop = iPosTop + (iPosBottom - iPosTop - childHeight) / 2;
        childBottm = childTop + childHeight;
    }
    else {
        //靠上（默认）
        childTop = iPosTop;
        childBottm = childTop + childHeight;
    }

    UiRect childPos(childLeft, childTop, childRight, childBottm);
    return childPos;
}

UiSize64 Layout::ArrangeChildren(const std::vector<Control*>& items, UiRect rc, bool bEstimateOnly)
{    
    DeflatePadding(rc);
    UiSize64 size;
    for (Control* pControl : items) {
        if ((pControl == nullptr) || (!pControl->IsVisible())) {
            continue;
        }
        UiSize64 controlSize = SetFloatPosInternal(pControl, rc, bEstimateOnly);
        size.cx = std::max(size.cx, controlSize.cx);
        size.cy = std::max(size.cy, controlSize.cy);
    }
    UiPadding rcPadding;
    if (m_pOwner != nullptr) {
        rcPadding = m_pOwner->GetPadding();
    }
    if (size.cx > 0) {
        size.cx += ((int64_t)rcPadding.left + rcPadding.right);
    }
    if (size.cy > 0) {
        size.cy += ((int64_t)rcPadding.top + rcPadding.bottom);
    }
    return size;
}

UiSize64 Layout::EstimateLayoutSize(const std::vector<Control*>& items, UiSize szAvailable)
{
    //宽度：取所有子控件宽度的最大值，加上Margin、Padding等，不含拉伸类型的子控件
    //高度：取所有子控件高度的最大值，加上Margin、Padding等，不含拉伸类型的子控件
    UiPadding rcPadding;
    if (m_pOwner != nullptr) {
        rcPadding = m_pOwner->GetPadding();
    }
    szAvailable.cx -= (rcPadding.left + rcPadding.right);
    szAvailable.cy -= (rcPadding.top + rcPadding.bottom);
    szAvailable.Validate();
    UiSize maxSize;
    UiSize itemSize;
    for (Control* pControl : items) {
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }
        UiEstSize estSize = pControl->EstimateSize(szAvailable);
        int32_t minWidth = pControl->GetMinWidth();
        int32_t maxWidth = pControl->GetMaxWidth();
        int32_t minHeight = pControl->GetMinHeight();
        int32_t maxHeight = pControl->GetMaxHeight();

        itemSize = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
        if (estSize.cx.IsStretch()) {
            //拉伸类型的子控件，不计入， 如果指定最小值，则按最小值计算
            itemSize.cx = std::max(minWidth, 0);
        }
        else {
            // 非拉伸控件：用std::clamp限制在[minWidth, maxWidth]范围内
            itemSize.cx = std::clamp(itemSize.cx, minWidth, maxWidth);
        }
        if (estSize.cy.IsStretch()) {
            //拉伸类型的子控件，不计入， 如果指定最小值，则按最小值计算
            itemSize.cy = std::max(minHeight, 0);
        }
        else {
            itemSize.cy = std::clamp(itemSize.cy, minHeight, maxHeight);
        }
        UiMargin rcMargin = pControl->GetMargin();
        if (itemSize.cx > 0) {
            maxSize.cx = std::max(itemSize.cx + rcMargin.left + rcMargin.right, maxSize.cx);
        }
        if (itemSize.cy > 0) {
            maxSize.cy = std::max(itemSize.cy + rcMargin.top + rcMargin.bottom, maxSize.cy);
        }        
    }    
    if (maxSize.cx > 0) {
        maxSize.cx += rcPadding.left + rcPadding.right;
    }
    if (maxSize.cy > 0) {
        maxSize.cy += rcPadding.top + rcPadding.bottom;
    }
    if ((maxSize.cx == 0) || (maxSize.cy == 0)){
        CheckConfig(items);
    }
    return UiSize64(maxSize.cx, maxSize.cy);
}

void Layout::CheckConfig(const std::vector<Control*>& items)
{
    //如果m_pOwner的宽高都是auto，而且子控件的宽高都是stretch，那么得到的结果是零，增加个断言
    if (m_pOwner == nullptr) {
        return;
    }
    if (!m_pOwner->GetFixedWidth().IsAuto() && !m_pOwner->GetFixedHeight().IsAuto()) {
        return;
    }

    bool isAllWidthStretch = true;
    bool isAllHeightStretch = true;
    size_t childCount = 0;
    for (Control* pControl : items) {
        if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
            continue;
        }
        if (!pControl->GetFixedWidth().IsStretch()) {
            isAllWidthStretch = false;
        }
        if (!pControl->GetFixedHeight().IsStretch()) {
            isAllHeightStretch = false;
        }
        ++childCount;
    }
    if ((childCount > 0) && m_pOwner->GetFixedWidth().IsAuto() && isAllWidthStretch) {
        ASSERT(!"配置错误：当前容器的宽是auto，子控件的宽都是stretch，估算宽度为零！");
    }
    if ((childCount > 0) && m_pOwner->GetFixedHeight().IsAuto() && isAllHeightStretch) {
        ASSERT(!"配置错误：当前容器的高是auto，子控件的高都是stretch，估算高度为零！");
    }
}

void Layout::SetChildMargin(int32_t nMargin)
{
    ASSERT(nMargin >= 0);
    nMargin = std::max(nMargin, 0);
    bool isChanged = ((int32_t)m_nChildMarginX != nMargin) || ((int32_t)m_nChildMarginY != nMargin);
    m_nChildMarginX = TruncateToUInt16((uint32_t)nMargin);
    m_nChildMarginY = TruncateToUInt16((uint32_t)nMargin);
    ASSERT(m_pOwner != nullptr);
    if (isChanged && (m_pOwner != nullptr)) {
        m_pOwner->Arrange();
    }
}

void Layout::SetChildMarginX(int32_t nMarginX)
{
    ASSERT(nMarginX >= 0);
    nMarginX = std::max(nMarginX, 0);
    bool isChanged = ((int32_t)m_nChildMarginX != nMarginX);
    m_nChildMarginX = TruncateToUInt16((uint32_t)nMarginX);
    ASSERT(m_pOwner != nullptr);
    if (isChanged && (m_pOwner != nullptr)) {
        m_pOwner->Arrange();
    }
}

void Layout::SetChildMarginY(int32_t nMarginY)
{
    ASSERT(nMarginY >= 0);
    nMarginY = std::max(nMarginY, 0);
    bool isChanged = ((int32_t)m_nChildMarginY != nMarginY);
    m_nChildMarginY = TruncateToUInt16((uint32_t)nMarginY);
    ASSERT(m_pOwner != nullptr);
    if (isChanged && (m_pOwner != nullptr)) {
        m_pOwner->Arrange();
    }
}

void Layout::SetChildHAlignType(HorAlignType hAlignType)
{
    if (m_hChildAlignType != hAlignType) {
        m_hChildAlignType = hAlignType;
        if (m_pOwner != nullptr) {
            m_pOwner->Arrange();
        }
    }
}

void Layout::SetChildVAlignType(VerAlignType vAlignType)
{
    if (m_vChildAlignType != vAlignType) {
        m_vChildAlignType = vAlignType;
        if (m_pOwner != nullptr) {
            m_pOwner->Arrange();
        }
    }
}

void Layout::DeflatePadding(UiRect& rc) const
{
    ASSERT(m_pOwner != nullptr);
    if (m_pOwner != nullptr) {
        rc.Deflate(m_pOwner->GetPadding());
        rc.Validate();
    }
}

} // namespace ui
