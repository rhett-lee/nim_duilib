#include "TabCtrl.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Layout/VLayout.h"
#include "duilib/Layout/HLayout.h"
#include <vector>

namespace ui
{

TabCtrl::TabCtrl(Window* pWindow):
    ListBox(pWindow, new HLayout),
    m_nSelectedId(Box::InvalidIndex),
    m_pTabBox(nullptr),
    m_bEnableDragOrder(true),
    m_orientation(TabCtrlOrientation::kHorizontal),
    m_bEnableOverflowScroll(true)
{
}

DString TabCtrl::GetType() const { return DUI_CTR_TAB_CTRL; }

void TabCtrl::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("selected_id")) {
        int32_t nValue = StringUtil::StringToInt32(strValue);
        if (nValue >= 0) {
            m_nSelectedId = nValue;
        }
    }
    else if (strName == _T("tab_box_name")) {
        //绑定的TabBox控件名称，绑定后TabCtrl的选择项变化时，TabBox的选择项会跟随变化
        SetTabBoxName(strValue);
    }
    else if (strName == _T("drag_order")) {
        //是否支持拖动调整顺序（在同一个标签内），默认是开启的
        SetEnableDragOrder(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("selected_tab_item_outline_width")) {
        SetSelectedTabItemOutlineWidth(StringUtil::StringToFloat(strValue.c_str()));
    }
    else if (strName == _T("selected_tab_item_outline_color")) {
        SetSelectedTabItemOutlineColor(strValue);
    }
    else if (strName == _T("tab_ctrl_bottom_line_height")) {
        SetTabCtrlBottomLineHeight(StringUtil::StringToFloat(strValue.c_str()));
    }
    else if (strName == _T("tab_ctrl_bottom_line_color")) {
        SetTabCtrlBottomLineColor(strValue);
    }
    else if (strName == _T("tab_ctrl_orientation")) {
        //标签栏的布局方向：horz(水平) / vert(垂直)，默认 horz
        TabCtrlOrientation orientation = TabCtrlOrientation::kHorizontal;
        if ((strValue == _T("vert")) || (strValue == _T("vertical"))) {
            orientation = TabCtrlOrientation::kVertical;
        }
        SetTabCtrlOrientation(orientation);
    }
    else if (strName == _T("overflow_scroll")) {
        //是否启用溢出滚动（标签过多时显示滚动条），默认启用
        SetEnableOverflowScroll(StringUtil::IsValueTrue(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void TabCtrl::SetEnableDragOrder(bool bEnable)
{
    m_bEnableDragOrder = bEnable;
}

bool TabCtrl::IsEnableDragOrder() const
{
    return m_bEnableDragOrder;
}

size_t TabCtrl::GetFixedItemCount() const
{
    size_t nFixed = 0;
    const size_t nCount = GetItemCount();
    for (size_t i = 0; i < nCount; ++i) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(i));
        if ((pItem != nullptr) && pItem->IsFixed()) {
            ++nFixed;
        }
    }
    return nFixed;
}

void TabCtrl::NormalizeFixedItems()
{
    //稳定划分：固定标签在前（保持原有相对顺序），非固定标签在后
    const size_t nCount = GetItemCount();
    std::vector<Control*> vecFixed;
    std::vector<Control*> vecNormal;
    vecFixed.reserve(nCount);
    vecNormal.reserve(nCount);
    for (size_t i = 0; i < nCount; ++i) {
        Control* pItem = GetItemAt(i);
        if (pItem == nullptr) {
            continue;
        }
        TabCtrlItem* pTabItem = dynamic_cast<TabCtrlItem*>(pItem);
        if ((pTabItem != nullptr) && pTabItem->IsFixed()) {
            vecFixed.push_back(pItem);
        }
        else {
            vecNormal.push_back(pItem);
        }
    }
    vecFixed.insert(vecFixed.end(), vecNormal.begin(), vecNormal.end());

    //顺序已正确时无需重排（避免无谓的 RemoveItem/AddItemAt 与重排开销）
    bool bNeedArrange = (vecFixed.size() != nCount);
    for (size_t i = 0; !bNeedArrange && (i < vecFixed.size()); ++i) {
        if (GetItemAt(i) != vecFixed[i]) {
            bNeedArrange = true;
        }
    }
    if (!bNeedArrange) {
        return;
    }

    //按目标顺序逐个移动（SetItemIndex 内部做移除+插入）
    for (size_t i = 0; i < vecFixed.size(); ++i) {
        SetItemIndex(vecFixed[i], i);
    }
    AdjustItemLineStatus();
}

void TabCtrl::SetTabCtrlOrientation(TabCtrlOrientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }
    m_orientation = orientation;
    //切换布局方向：水平用 HLayout，垂直用 VLayout
    Layout* pNewLayout = (m_orientation == TabCtrlOrientation::kVertical) ?
                         static_cast<Layout*>(new VLayout()) :
                         static_cast<Layout*>(new HLayout());
    Layout* pOldLayout = ResetLayout(pNewLayout);
    if (pOldLayout != nullptr) {
        FreeLayout(pOldLayout);
    }
    if (IsInited()) {
        //重新配置滚动条并触发重排
        if (m_bEnableOverflowScroll) {
            if (m_orientation == TabCtrlOrientation::kVertical) {
                EnableScrollBar(true, false);
            }
            else {
                EnableScrollBar(false, true);
            }
            SetScrollBarFloat(true);
        }
        else {
            EnableScrollBar(false, false);
        }
        if (GetWindow() != nullptr) {
            Invalidate();
        }
    }
}

TabCtrlOrientation TabCtrl::GetTabCtrlOrientation() const
{
    return m_orientation;
}

void TabCtrl::SetEnableOverflowScroll(bool bEnable)
{
    if (m_bEnableOverflowScroll == bEnable) {
        return;
    }
    m_bEnableOverflowScroll = bEnable;
    if (IsInited()) {
        if (bEnable) {
            if (m_orientation == TabCtrlOrientation::kVertical) {
                EnableScrollBar(true, false);
            }
            else {
                EnableScrollBar(false, true);
            }
            SetScrollBarFloat(true);
        }
        else {
            EnableScrollBar(false, false);
        }
        if (GetWindow() != nullptr) {
            Invalidate();
        }
    }
}

bool TabCtrl::IsEnableOverflowScroll() const
{
    return m_bEnableOverflowScroll;
}

void TabCtrl::SetSelectedTabItemOutlineWidth(float fOutlineWidth)
{
    if (fOutlineWidth < 0.0f) {
        fOutlineWidth = 0.0f;
    }
    m_fSelectedTabItemOutlineWidth = fOutlineWidth;
}

float TabCtrl::GetSelectedTabItemOutlineWidth() const
{
    return m_fSelectedTabItemOutlineWidth;
}

void TabCtrl::SetSelectedTabItemOutlineColor(const DString& outlineColor)
{
    m_selectedTabItemOutlineColor = outlineColor;
}

const DString& TabCtrl::GetSelectedTabItemOutlineColor() const
{
    return m_selectedTabItemOutlineColor;
}

void TabCtrl::SetTabCtrlBottomLineHeight(float fLineHeight)
{
    if (fLineHeight < 0.0f) {
        fLineHeight = 0.0f;
    }
    m_fTabCtrlBottomLineHeight = fLineHeight;
}

float TabCtrl::GetTabCtrlBottomLineHeight() const
{
    return m_fTabCtrlBottomLineHeight;
}

void TabCtrl::SetTabCtrlBottomLineColor(const DString& lineColor)
{
    m_tabCtrlBottomLineColor = lineColor;
}

const DString& TabCtrl::GetTabCtrlBottomLineColor() const
{
    return m_tabCtrlBottomLineColor;
}

void TabCtrl::OnInit()
{
    if (IsInited()) {
        return;
    }
    //设置布局方向（需在布局计算前完成）：水平用 HLayout，垂直用 VLayout
    Layout* pNewLayout = (m_orientation == TabCtrlOrientation::kVertical) ?
                         static_cast<Layout*>(new VLayout()) :
                         static_cast<Layout*>(new HLayout());
    Layout* pOldLayout = ResetLayout(pNewLayout);
    if (pOldLayout != nullptr) {
        FreeLayout(pOldLayout);
    }

    BaseClass::OnInit();

    //固定标签归位：保证固定标签始终位于标签栏前端（XML 中未按顺序书写时也能自愈）
    NormalizeFixedItems();

    //设置为单选
    SetMultiSelect(false);

    //默认选择的子项
    if (m_nSelectedId != Box::InvalidIndex) {
        SelectItem(m_nSelectedId);
    }

    //调整标签分割线的显示状态
    AdjustItemLineStatus();

    //溢出滚动：启用对应的滚动条（悬浮在子控件之上，不占用布局空间）
    if (m_bEnableOverflowScroll) {
        if (m_orientation == TabCtrlOrientation::kVertical) {
            EnableScrollBar(true, false);    //垂直滚动条
        }
        else {
            EnableScrollBar(false, true);    //水平滚动条
        }
        SetScrollBarFloat(true);
    }
}

void TabCtrl::HandleEvent(const EventArgs& msg)
{
    if ((msg.GetSender() == this) && (msg.eventType == kEventSelect)) {
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
    BaseClass::HandleEvent(msg);

    if ((msg.GetSender() == this) && ((msg.eventType == kEventSelect) || (msg.eventType == kEventUnSelect))) {
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
}

void TabCtrl::PaintStateColors(IRender* pRender)
{
    BaseClass::PaintStateColors(pRender);
    //在标签底部画线（但排除当前选择标签的区域，不覆盖选择标签）
    float fBottomLineHeight = GetTabCtrlBottomLineHeight();
    DString strBottomLineColor = GetTabCtrlBottomLineColor();
    UiColor bottomLineColor;
    if (!strBottomLineColor.empty()) {
        bottomLineColor = GetUiColor(strBottomLineColor);
    }
    std::unique_ptr<IPen> pBottomLinePen;
    if ((fBottomLineHeight > 0.01f) && !bottomLineColor.IsEmpty()) {
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        if (pRenderFactory != nullptr) {
            pBottomLinePen.reset(pRenderFactory->CreatePen(bottomLineColor, fBottomLineHeight));
        }
    }
    if (pBottomLinePen != nullptr) {
        UiRectF rcTabCtrl = UiRectF::MakeFromRect(GetRect());
        if (m_orientation == TabCtrlOrientation::kVertical) {
            //垂直标签栏：在右侧画线，分隔标签列与内容区
            rcTabCtrl.right -= (fBottomLineHeight / 2);
            pRender->DrawLine(UiPointF(rcTabCtrl.right, rcTabCtrl.top),
                              UiPointF(rcTabCtrl.right, rcTabCtrl.bottom),
                              pBottomLinePen.get());
        }
        else {
            //水平标签栏：在底部画线
            rcTabCtrl.bottom -= (fBottomLineHeight / 2);
            pRender->DrawLine(UiPointF(rcTabCtrl.left, rcTabCtrl.bottom),
                              UiPointF(rcTabCtrl.right, rcTabCtrl.bottom),
                              pBottomLinePen.get());
        }
    }
}

void TabCtrl::SetTabBoxName(const DString& tabBoxName)
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

DString TabCtrl::GetTabBoxName() const
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
    //固定标签只能在固定块内移动（[0, nFixedCount)），非固定标签只能在非固定块内移动（[nFixedCount, count)）
    TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(pControl);
    if (pItem != nullptr) {
        const size_t nFixedCount = GetFixedItemCount();
        const size_t nCount = GetItemCount();
        if (pItem->IsFixed()) {
            if (nFixedCount > 0) {
                if (iIndex >= nFixedCount) {
                    iIndex = nFixedCount - 1;
                }
            }
            else {
                iIndex = 0;
            }
        }
        else {
            if (iIndex < nFixedCount) {
                iIndex = nFixedCount;
            }
            if (iIndex > nCount) {
                iIndex = nCount;
            }
        }
    }
    bool bRet = BaseClass::SetItemIndex(pControl, iIndex);
    if (bRet) {
        AdjustItemLineStatus();
    }
    return bRet;
}

bool TabCtrl::AddItem(Control* pControl)
{
    //固定标签追加到固定块末尾，非固定标签追加到末尾
    size_t iIndex = GetItemCount();
    TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(pControl);
    if ((pItem != nullptr) && pItem->IsFixed()) {
        iIndex = GetFixedItemCount();
    }
    return AddItemAt(pControl, iIndex);
}

bool TabCtrl::AddItemAt(Control* pControl, size_t iIndex)
{
    //固定标签只能插入到固定块区域内（[0, nFixedCount]），非固定标签只能插入到固定块之后
    TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(pControl);
    if (pItem != nullptr) {
        const size_t nFixedCount = GetFixedItemCount();
        const size_t nCount = GetItemCount();
        if (pItem->IsFixed()) {
            if (iIndex > nFixedCount) {
                iIndex = nFixedCount;
            }
        }
        else {
            if (iIndex < nFixedCount) {
                iIndex = nFixedCount;
            }
            if (iIndex > nCount) {
                iIndex = nCount;
            }
        }
    }
    bool bRet = BaseClass::AddItemAt(pControl, iIndex);
    if (bRet) {
        AdjustItemLineStatus();
    }
    return bRet;
}

bool TabCtrl::RemoveItem(Control* pControl)
{
    bool bRet = BaseClass::RemoveItem(pControl);
    if (bRet) {
        AdjustItemLineStatus();
    }
    return bRet;
}

bool TabCtrl::RemoveItemAt(size_t iIndex)
{
    bool bRet = BaseClass::RemoveItemAt(iIndex);
    if (bRet) {
        AdjustItemLineStatus();
    }
    return bRet;
}

void TabCtrl::RemoveAllItems()
{
    BaseClass::RemoveAllItems();
}

void TabCtrl::AdjustItemLineStatus()
{
    TabCtrlItem* pLastItem = nullptr;
    const size_t nCount = GetItemCount();
    for (size_t nItem = 0; nItem < nCount; ++nItem) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(nItem));
        if ((pItem == nullptr) || !pItem->IsVisible()) {
            continue;
        }
        ControlStateType state = pItem->GetState();
        if ((state == kControlStateHovered) || (state == kControlStatePressed) || pItem->IsSelected()) {
            //活动标签，不显示分割线
            pItem->SetItemLineVisible(false);

            if (pLastItem != nullptr) {
                //活动标签的前一个标签，不显示分割线
                pLastItem->SetItemLineVisible(false);
            }
        }
        else {
            //其他状态的标签，显示分割线
            pItem->SetItemLineVisible(true);
        }
        
        if (nItem == (nCount - 1)) {
            //最后一个标签，不显示分割线
            pItem->SetItemLineVisible(false);
        }

        //记录上一个标签
        pLastItem = pItem;
    }
}

///////////////////////////////////////////////////////////////////
////
TabCtrlItem::TabCtrlItem(Window* pWindow):
    ControlDragableT<ListBoxItemH>(pWindow),
    m_pIcon(nullptr),
    m_pLabel(nullptr),
    m_pCloseBtn(nullptr),
    m_pLine(nullptr),
    m_pBadge(nullptr),
    m_nBadgeCount(0),
    m_nBadgeMaxCount(99),
    m_bBadgeDot(false),
    m_bAutoHideCloseBtn(false),
    m_bFixed(false),
    m_nTabBoxItemIndex(Box::InvalidIndex)
{
    m_rcSelected.cx = (uint8_t)-1;
    m_rcSelected.cy = (uint8_t)-1;

    m_rcHovered.cx = (uint8_t)-1;
    m_rcHovered.cy = (uint8_t)-1;

    m_hoveredPadding.top = 0;
    m_hoveredPadding.left = (uint8_t)-1;
    m_hoveredPadding.right = 0;
    m_hoveredPadding.bottom = 0;
}

DString TabCtrlItem::GetType() const { return DUI_CTR_TAB_CTRL_ITEM; }

void TabCtrlItem::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("tab_box_item_index")) {
        SetTabBoxItemIndex((size_t)StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("title")) {
        SetTitle(strValue);
    }
    else if (strName == _T("title_id")) {
        SetTitleId(strValue);
    }
    else if (strName == _T("icon")) {
        SetIcon(strValue);
    }
    else if (strName == _T("icon_class")) {
        SetIconClass(strValue);
    }
    else if (strName == _T("title_class")) {
        SetTitleClass(strValue);
    }
    else if (strName == _T("close_button_class")) {
        SetCloseButtonClass(strValue);
    }
    else if (strName == _T("line_class")) {
        SetLineClass(strValue);
    }
    else if (strName == _T("selected_round_corner")) {
        UiSize sz;
        AttributeUtil::ParseSizeValue(strValue.c_str(), sz);
        SetSelectedRoundCorner(sz, true);
    }
    else if ((strName == _T("hovered_round_corner")) || (strName == _T("hot_round_corner"))) {
        UiSize sz;
        AttributeUtil::ParseSizeValue(strValue.c_str(), sz);
        SetHoveredRoundCorner(sz, true);
    }
    else if ((strName == _T("hovered_padding")) || (strName == _T("hot_padding"))) {
        UiPadding rcPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcPadding);
        SetHoveredPadding(rcPadding, true);
    }
    else if (strName == _T("auto_hide_close_button")) {
        SetAutoHideCloseButton(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("badge_count")) {
        SetBadgeCount(StringUtil::StringToInt64(strValue));
    }
    else if (strName == _T("badge_max_count")) {
        SetBadgeMaxCount(StringUtil::StringToInt64(strValue));
    }
    else if (strName == _T("badge_dot")) {
        SetBadgeDot(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("badge_class")) {
        SetBadgeClass(strValue);
    }
    else if (strName == _T("fixed")) {
        //是否为固定标签（固定标签不可拖动、不可作为拖放目标，且不显示关闭按钮）
        SetFixed(StringUtil::IsValueTrue(strValue));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void TabCtrlItem::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();
    SetIconClass(GetIconClass());
    SetTitleClass(GetTitleClass());
    SetLineClass(GetLineClass());
    SetCloseButtonClass(GetCloseButtonClass());
    SetBadgeClass(GetBadgeClass());
    CheckIconVisible();
    UpdateCloseButtonVisible();
    AdjustItemLineStatus();
}

void TabCtrlItem::HandleEvent(const EventArgs& msg)
{
    BaseClass::HandleEvent(msg);
    if ((msg.GetSender() == this) && (m_pLine != nullptr) && (msg.eventType == kEventStateChanged)) {
        //处理分割线的状态
        AdjustItemLineStatus();
    }
}

void TabCtrlItem::OnSetVisible(bool bChanged)
{
    BaseClass::OnSetVisible(bChanged);
    CheckIconVisible();
    UpdateCloseButtonVisible();
    AdjustItemLineStatus();
}

DString TabCtrlItem::GetToolTipText() const
{
    DString tooltip = BaseClass::GetToolTipText();
    if (tooltip.empty()) {
        if (m_pLabel != nullptr) {
            tooltip = m_pLabel->GetToolTipText();
        }
    }
    return tooltip;
}

void TabCtrlItem::SetIconClass(const DString& iconClass)
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
            m_pIcon = new IconControl(GetWindow());
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

DString TabCtrlItem::GetIconClass() const
{
    return m_iconClass.c_str();
}

void TabCtrlItem::SetTitleClass(const DString& titleClass)
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
            m_pLabel = new Label(GetWindow());
            m_pLabel->SetClass(titleClass);
            AddItem(m_pLabel);
            if (!m_title.empty()) {
                m_pLabel->SetText(m_title.c_str());
                m_title.clear();
            }
            if (!m_titleId.empty()) {
                m_pLabel->SetTextId(m_titleId.c_str());
                m_titleId.clear();
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

DString TabCtrlItem::GetTitleClass() const
{
    return m_titleClass.c_str();
}

void TabCtrlItem::SetCloseButtonClass(const DString& closeButtonClass)
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
            m_pCloseBtn = new Button(GetWindow());
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

DString TabCtrlItem::GetCloseButtonClass() const
{
    return m_closeBtnClass.c_str();
}

void TabCtrlItem::SetLineClass(const DString& lineClass)
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
            m_pLine = new Control(GetWindow());
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

DString TabCtrlItem::GetLineClass() const
{
    return m_lineClass.c_str();
}

void TabCtrlItem::SetBadgeClass(const DString& badgeClass)
{
    bool bChanged = m_badgeClass != badgeClass;
    if (bChanged) {
        m_badgeClass = badgeClass;
    }
    if (!IsInited()) {
        return;
    }
    if (!badgeClass.empty()) {
        if (m_pBadge == nullptr) {
            m_pBadge = new Badge(GetWindow());
            m_pBadge->SetClass(badgeClass);
            AddItem(m_pBadge);
            //应用保存的角标数据
            m_pBadge->SetMaxCount(m_nBadgeMaxCount);
            m_pBadge->SetDotMode(m_bBadgeDot);
            m_pBadge->SetCount(m_nBadgeCount);
            AdjustSubItemIndex();
        }
        else if (bChanged) {
            m_pBadge->SetClass(badgeClass);
        }
    }
    else {
        if (m_pBadge != nullptr) {
            RemoveItem(m_pBadge);
            m_pBadge = nullptr;
        }
    }
}

DString TabCtrlItem::GetBadgeClass() const
{
    return m_badgeClass.c_str();
}

void TabCtrlItem::SetBadgeCount(int64_t nCount)
{
    m_nBadgeCount = nCount;
    if (m_pBadge != nullptr) {
        m_pBadge->SetCount(nCount);
    }
}

int64_t TabCtrlItem::GetBadgeCount() const
{
    return (m_pBadge != nullptr) ? m_pBadge->GetCount() : m_nBadgeCount;
}

void TabCtrlItem::SetBadgeMaxCount(int64_t nMaxCount)
{
    m_nBadgeMaxCount = nMaxCount;
    if (m_pBadge != nullptr) {
        m_pBadge->SetMaxCount(nMaxCount);
    }
}

int64_t TabCtrlItem::GetBadgeMaxCount() const
{
    return (m_pBadge != nullptr) ? m_pBadge->GetMaxCount() : m_nBadgeMaxCount;
}

void TabCtrlItem::SetBadgeDot(bool bDot)
{
    m_bBadgeDot = bDot;
    if (m_pBadge != nullptr) {
        m_pBadge->SetDotMode(bDot);
    }
}

bool TabCtrlItem::IsBadgeDot() const
{
    return (m_pBadge != nullptr) ? m_pBadge->IsDotMode() : m_bBadgeDot;
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
    if ((m_pBadge != nullptr) && !m_pBadge->IsFloat()) {
        subItems.push_back(m_pBadge);
    }
    if ((m_pCloseBtn != nullptr) && !m_pCloseBtn->IsFloat()) {
        subItems.push_back(m_pCloseBtn);
    }
    if (m_pLine != nullptr) {
        subItems.push_back(m_pLine);
    }
    if ((m_pBadge != nullptr) && m_pBadge->IsFloat()) {
        subItems.push_back(m_pBadge);
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
    TabCtrl* pTabCtrl = GetTabCtrl();
    if (pTabCtrl != nullptr) {
        pTabCtrl->AdjustItemLineStatus();
    }
}

bool TabCtrlItem::IsItemLineVisible() const
{
    if ((m_pLine == nullptr) || !m_pLine->IsVisible()) {
        return false;
    }
    DString bkColor = m_pLine->GetBkColor();
    if (bkColor.empty()) {
        return false;
    }
    UiColor color = GetUiColor(bkColor);
    return color.GetA() != 0;
}

void TabCtrlItem::SetItemLineVisible(bool bVisible)
{
    if (m_pLine == nullptr) {
        return;
    }
    DString bkColor = m_pLine->GetBkColor();
    if (bkColor.empty()) {
        return;
    }
    UiColor oldColor = GetUiColor(bkColor);
    uint8_t a = bVisible ? 255 : 0;
    UiColor newColor = UiColor(a, oldColor.GetR(), oldColor.GetG(), oldColor.GetB());
    if (newColor != oldColor) {
        m_pLine->SetBkColor(newColor);
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
        Dpi().ScaleSize(szCorner);
    }
    m_rcSelected.cx = ui::TruncateToUInt8(szCorner.cx);
    m_rcSelected.cy = ui::TruncateToUInt8(szCorner.cy);
}

UiSize TabCtrlItem::GetSelectedRoundCorner() const
{
    UiSize szCorner;
    szCorner.cx = (int8_t)m_rcSelected.cx;
    szCorner.cy = (int8_t)m_rcSelected.cy;
    if (szCorner.cx < 0) {
        szCorner.cx = Dpi().GetScaleInt(12);
    }
    if (szCorner.cy < 0) {
        szCorner.cy = Dpi().GetScaleInt(12);
    }
    return szCorner;
}

void TabCtrlItem::SetHoveredRoundCorner(UiSize szCorner, bool bNeedDpiScale)
{
    ASSERT((szCorner.cx >= 0) && (szCorner.cy >= 0));
    szCorner.Validate();
    if (bNeedDpiScale) {
        Dpi().ScaleSize(szCorner);
    }
    m_rcHovered.cx = ui::TruncateToUInt8(szCorner.cx);
    m_rcHovered.cy = ui::TruncateToUInt8(szCorner.cy);
}

UiSize TabCtrlItem::GetHoveredRoundCorner() const
{
    UiSize szCorner;
    szCorner.cx = (int8_t)m_rcHovered.cx;
    szCorner.cy = (int8_t)m_rcHovered.cy;
    if (szCorner.cx < 0) {
        szCorner.cx = Dpi().GetScaleInt(5);
    }
    if (szCorner.cy < 0) {
        szCorner.cy = Dpi().GetScaleInt(5);
    }
    return szCorner;
}

void TabCtrlItem::SetHoveredPadding(UiPadding rcPadding, bool bNeedDpiScale)
{
    ASSERT((rcPadding.left >= 0) && (rcPadding.top >= 0) && (rcPadding.right >= 0) && (rcPadding.bottom >= 0));
    rcPadding.Validate();
    if (bNeedDpiScale) {
        Dpi().ScalePadding(rcPadding);
    }
    m_hoveredPadding.left = TruncateToUInt8(rcPadding.left);
    m_hoveredPadding.top = TruncateToUInt8(rcPadding.top);
    m_hoveredPadding.right = TruncateToUInt8(rcPadding.right);
    m_hoveredPadding.bottom = TruncateToUInt8(rcPadding.bottom);
}

UiPadding TabCtrlItem::GetHoveredPadding() const
{
    UiPadding rcPadding;
    rcPadding.left = (int8_t)m_hoveredPadding.left;
    rcPadding.top = (int8_t)m_hoveredPadding.top;
    rcPadding.right = (int8_t)m_hoveredPadding.right;
    rcPadding.bottom = (int8_t)m_hoveredPadding.bottom;
    if (rcPadding.left < 0) {
        rcPadding.top = 0;
        rcPadding.left = (uint8_t)Dpi().GetScaleInt(3);
        rcPadding.right = rcPadding.left;
        rcPadding.bottom = rcPadding.left;
    }
    return rcPadding;
}

void TabCtrlItem::SetAutoHideCloseButton(bool bAutoHideCloseBtn)
{
    if (m_bAutoHideCloseBtn != bAutoHideCloseBtn) {
        m_bAutoHideCloseBtn = bAutoHideCloseBtn;
        UpdateCloseButtonVisible();
    }
}

bool TabCtrlItem::IsAutoHideCloseButton() const
{
    return m_bAutoHideCloseBtn;
}

bool TabCtrlItem::MouseEnter(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseEnter(msg);
    UpdateCloseButtonVisible();
    return bRet;
}

bool TabCtrlItem::MouseLeave(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseLeave(msg);
    UiPoint pt = msg.ptMouse;
    pt.Offset(GetScrollOffsetInScrollBox());
    if (IsFixed()) {
        if (m_pCloseBtn != nullptr) {
            m_pCloseBtn->SetVisible(false);
        }
    }
    else if (!GetRect().ContainsPt(pt)) {
        if (m_pCloseBtn != nullptr) {
            m_pCloseBtn->SetFadeVisible(IsSelected());
        }
    }
    return bRet;
}

void TabCtrlItem::OnPrivateSetSelected()
{
    UpdateCloseButtonVisible();
    //处理分割线的状态
    AdjustItemLineStatus();
}

bool TabCtrlItem::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (!IsSelected() && IsActivatable()) {
        //按鼠标左键的时候，选择
        uint64_t vkFlag = kVkLButton;
        if (this->IsKeyDown(msg, ModifierKey::kControl)) {
            vkFlag |= kVkControl;
        }
        if (this->IsKeyDown(msg, ModifierKey::kShift)) {
            vkFlag |= kVkShift;
        }
        Selected(true, true, vkFlag);
    }
    return bRet;
}

void TabCtrlItem::PaintStateColors(IRender* pRender)
{
    //绘制标签页的形状（通过颜色体现）
    if (IsSelected()) {
        PaintTabItemSelected(pRender);
    }
    else if ((GetState() == ControlStateType::kControlStateHovered)    ||
             (GetState() == ControlStateType::kControlStatePressed) ||
             IsAnimationPlayerPlaying(AnimationType::kAnimationHovered)) {
        //鼠标悬停状态
        PaintTabItemHovered(pRender);
    }
    else {
        BaseClass::PaintStateColors(pRender);
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

    UiSize roundSize = GetSelectedRoundCorner();
    if (rc.Width() < roundSize.cx * 2) {
        roundSize.cx = rc.Width() / 3;
    }

    DString color = GetSelectStateColor(ControlStateType::kControlStateNormal);
    if (color.empty()) {
        return;
    }

    UiColor dwColor = GetUiColor(color);
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(dwColor));        
        std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
        if ((brush != nullptr) && (path != nullptr)) {
            //轮廓边线相关参数
            float fOutlineWidth = 0.0f;
            UiColor outlineColor;
            TabCtrl* pTabCtrl = GetTabCtrl();
            if (pTabCtrl != nullptr) {
                fOutlineWidth = pTabCtrl->GetSelectedTabItemOutlineWidth();
                DString strOutlineColor = pTabCtrl->GetSelectedTabItemOutlineColor();

                fOutlineWidth = pTabCtrl->Dpi().GetScaleFloat(fOutlineWidth);
                if (!strOutlineColor.empty()) {
                    outlineColor = GetUiColor(strOutlineColor);
                }
            }
            std::unique_ptr<IPen> pOutlinePen;
            if ((fOutlineWidth > 0.01f) && !outlineColor.IsEmpty()) {
                pOutlinePen.reset(pRenderFactory->CreatePen(outlineColor, fOutlineWidth));
            }
            bool bVertical = (pTabCtrl != nullptr) &&
                             (pTabCtrl->GetTabCtrlOrientation() == TabCtrlOrientation::kVertical);
            if (pOutlinePen != nullptr) {
                //为边线留出空间
                if (bVertical) {
                    rc.right -= (int32_t)std::round(fOutlineWidth / 2);
                }
                else {
                    rc.top += (int32_t)std::round(fOutlineWidth / 2);
                }
            }
            //绘制选择标签的样式到路径
            AddTabItemPath(path.get(), rc, roundSize);
            if (pOutlinePen != nullptr) {
                //绘制选择标签的轮廓边线
                pRender->DrawPath(path.get(), pOutlinePen.get());
                UiRect rcSelectItem = rc;
                if (pTabCtrl != nullptr) {
                    UiRect rcTabCtrl = pTabCtrl->GetRect();
                    if (bVertical) {
                        //垂直标签栏：在右侧（面向内容区的一侧）绘制竖线，连接被选择标签的上下边界
                        rcSelectItem.top = std::max(rcSelectItem.top, rcTabCtrl.top);
                        rcSelectItem.bottom = std::min(rcSelectItem.bottom, rcTabCtrl.bottom);
                        pRender->DrawLine(UiPointF(rcSelectItem.right, rcSelectItem.top),
                                          UiPointF(rcSelectItem.right, rcSelectItem.bottom),
                                          pOutlinePen.get());
                    }
                    else {
                        float fRoundWidth = 0;
                        float fRoundHeight = 0;
                        pTabCtrl->GetBorderRound(fRoundWidth, fRoundHeight);
                        rcTabCtrl.left += (int32_t)fRoundWidth;
                        rcTabCtrl.right -= (int32_t)fRoundWidth;

                        rcSelectItem.left = std::max(rcSelectItem.left, rcTabCtrl.left);
                        rcSelectItem.right = std::min(rcSelectItem.right, rcTabCtrl.right);
                        pRender->DrawLine(UiPointF(rcSelectItem.left, rcSelectItem.bottom),
                                          UiPointF(rcSelectItem.right, rcSelectItem.bottom),
                                          pOutlinePen.get());
                    }
                }
                else {
                    //未关联到 TabCtrl 时（理论上不会发生），按水平处理
                    pRender->DrawLine(UiPointF(rcSelectItem.left, rcSelectItem.bottom),
                                      UiPointF(rcSelectItem.right, rcSelectItem.bottom),
                                      pOutlinePen.get());
                }
            }
            //绘制选择标签的样式（从路径中绘制）
            pRender->FillPath(path.get(), brush.get());
        }
    }
}

/*************************************************************************
 * 函数功能：
 * 为 Tab 标签项绘制【顶部圆角、底部圆角、两侧直线】的闭合图形路径
 * 用于绘制 TabCtrl 的标签背景形状
 *
 * 参数说明：
 * @path    ：输出的图形路径对象（由外部创建）
 * @rect    ：Tab 标签项的整体绘制区域
 * @roundSize：圆角大小（cx=圆角宽度，cy=圆角高度）
 *************************************************************************/
void TabCtrlItem::AddTabItemPath(IPath* path, const UiRect& rect, UiSize roundSize) const
{
    // 安全校验：路径对象不能为空
    ASSERT(path != nullptr);
    if (path == nullptr) {
        return;
    }

    // ==========================
    // 确保圆角宽高都是【偶数】
    // 避免绘制时出现半像素模糊
    // ==========================
    if ((roundSize.cx % 2) != 0) {
        roundSize.cx += 1;
    }
    if ((roundSize.cy % 2) != 0) {
        roundSize.cy += 1;
    }

    // 如果绘制区域宽度小于圆角宽度，无法正常绘制，直接返回
    if (rect.Width() < roundSize.cx) {
        return;
    }

    // 如果绘制区域高度小于圆角高度，无法正常绘制，直接返回
    if (rect.Height() < roundSize.cy) {
        return;
    }

    // ==========================
    // 计算实际绘制路径的内缩区域
    // 左右各缩进 半个圆角宽度，用于放置圆角
    // ==========================
    UiRect rc = rect;
    rc.left += roundSize.cx / 2;
    rc.right -= roundSize.cx / 2;

    // ==========================
    // 开始绘制路径（顺时针绘制）
    // ==========================

    // 1. 左侧竖直线：左下 → 左上（圆角起点）
    path->AddLine(rc.left, rc.bottom - roundSize.cy / 2, rc.left, rc.top + roundSize.cy);

    // 2. 左上角圆弧：180度起点，扫90度
    path->AddArc(UiRect(rc.left, rc.top, rc.left + roundSize.cx, rc.top + roundSize.cy), 180, 90);

    // 3. 顶部横线：左 → 右
    path->AddLine(rc.left + roundSize.cx / 2, rc.top, rc.right - roundSize.cx / 2, rc.top);

    // 4. 右上角圆弧：270度起点，扫90度
    path->AddArc(UiRect(rc.right - roundSize.cx, rc.top, rc.right, rc.top + roundSize.cy), 270, 90);

    // 5. 右侧竖直线：右上 → 右下
    path->AddLine(rc.right, rc.top + roundSize.cy / 2, rc.right, rc.bottom - roundSize.cy / 2);

    // 6. 左下角圆弧：0度起点，扫90度
    path->AddArc(UiRect(rc.left - roundSize.cx, rc.bottom - roundSize.cy, rc.left, rc.bottom), 0, 90);

    // 7. 右下角圆弧：90度起点，扫90度
    path->AddArc(UiRect(rc.right, rc.bottom - roundSize.cy, rc.right + roundSize.cx, rc.bottom), 90, 90);

    // 8. 底部横线：左 → 右
    path->AddLine(rc.left - roundSize.cx / 2, rc.bottom, rc.right + roundSize.cx / 2, rc.bottom);

    // 闭合路径，形成完整 Tab 标签形状
    path->Close();
}

void TabCtrlItem::PaintTabItemHovered(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    UiPadding hotPadding = GetHoveredPadding();
    rc.top += hotPadding.top;
    rc.left += hotPadding.left;
    rc.right -= hotPadding.right;
    rc.bottom -= hotPadding.bottom;
 
    UiSize roundSize = GetHoveredRoundCorner();
    DString color = GetStateColor(ControlStateType::kControlStateHovered);
    if (color.empty()) {
        return;
    }
    UiColor dwColor = GetUiColor(color);
    uint8_t uFade = 255;
    if (IsAnimationPlayerPlaying(AnimationType::kAnimationHovered)) {
        uFade = GetHoveredAlpha();
    }
    pRender->FillRoundRect(UiRectF::MakeFromRect(rc), (float)roundSize.cx, (float)roundSize.cy, dwColor, uFade);
}

void TabCtrlItem::SetIcon(const DString& iconImageString)
{
    Control* pIconControl = GetIconControl();
    if (pIconControl != nullptr) {
        pIconControl->SetBkImage(iconImageString);
        m_iconImageString.clear();
        CheckIconVisible();
    }
    else {
        m_iconImageString = iconImageString;
    }
}

DString TabCtrlItem::GetIcon() const
{
    DString iconString;
    Control* pIconControl = GetIconControl();
    if (pIconControl != nullptr) {
        iconString = pIconControl->GetBkImage();
    }
    else {
        iconString = m_iconImageString.c_str();
    }
    return iconString;
}

bool TabCtrlItem::SetIconData(int32_t nWidth, int32_t nHeight, const uint8_t* pPixelBits, int32_t nPixelBitsSize)
{
    bool bRet = false;
    IconControl* pIconControl = GetIconControl();
    if (pIconControl != nullptr) {
        bRet = pIconControl->SetIconData(nWidth, nHeight, pPixelBits, nPixelBitsSize);
        if (bRet) {
            CheckIconVisible();
        }
    }
    return bRet;
}

void TabCtrlItem::ClearIconData()
{
    IconControl* pIconControl = GetIconControl();
    if (pIconControl != nullptr) {
        pIconControl->ClearIconData();
    }
}

void TabCtrlItem::CheckIconVisible()
{
    IconControl* pIconControl = GetIconControl();
    if (pIconControl != nullptr) {
        bool bVisible = !pIconControl->GetBkImage().empty() || pIconControl->HasIconData();
        if (!IsVisible()) {
            bVisible = false;
        }
        if (pIconControl->IsVisible() != bVisible) {
            pIconControl->SetVisible(bVisible);
        }        
    }
}

void TabCtrlItem::SetTitle(const DString& title)
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

DString TabCtrlItem::GetTitle() const
{
    DString title;
    Label* pLabel = GetTextLabel();
    if (pLabel != nullptr) {
        title = pLabel->GetText();
    }
    else {
        title = m_title.c_str();
    }
    return title;
}

void TabCtrlItem::SetTitleId(const DString& titleId)
{
    Label* pLabel = GetTextLabel();
    if (pLabel != nullptr) {
        pLabel->SetTextId(titleId);
        m_titleId.clear();
    }
    else {
        m_titleId = titleId;
    }
}

DString TabCtrlItem::GetTitleId() const
{
    DString titleId;
    Label* pLabel = GetTextLabel();
    if (pLabel != nullptr) {
        titleId = pLabel->GetTextId();
    }
    else {
        titleId = m_titleId.c_str();
    }
    return titleId;
}

void TabCtrlItem::SetTabBoxItemIndex(size_t nTabBoxItemIndex)
{
    m_nTabBoxItemIndex = nTabBoxItemIndex;
}

size_t TabCtrlItem::GetTabBoxItemIndex() const
{
    return m_nTabBoxItemIndex;
}

bool TabCtrlItem::IsEnableDragOrder() const
{
    TabCtrl* pTabCtrl = GetTabCtrl();
    if (pTabCtrl != nullptr) {
        if (!pTabCtrl->IsEnableDragOrder()) {
            //TabCtrl的总开关，控制总体功能关闭
            return false;
        }
    }
    return BaseClass::IsEnableDragOrder();
}

void TabCtrlItem::SetFixed(bool bFixed)
{
    if (m_bFixed != bFixed) {
        m_bFixed = bFixed;
        //固定标签不可拖动，也不可作为拖放目标
        SetEnableDragOrder(!bFixed);
        UpdateCloseButtonVisible();

        //归位：保证"固定标签恒在标签栏前端"这一不变量
        //（否则固定块之后的索引计算会失准，新插入的标签会落进固定区）
        TabCtrl* pTabCtrl = GetTabCtrl();
        if ((pTabCtrl != nullptr) && pTabCtrl->IsInited()) {
            const size_t nIndex = pTabCtrl->GetItemIndex(this);
            if (nIndex != Box::InvalidIndex) {
                const size_t nFixedCount = pTabCtrl->GetFixedItemCount();
                //置为固定：移动到固定块末尾（计数已含自身，故 -1）
                //取消固定：移动到固定块之后的第一个位置
                const size_t nTarget = bFixed ? (nFixedCount - 1) : nFixedCount;
                if (nTarget != nIndex) {
                    pTabCtrl->SetItemIndex(this, nTarget);
                }
            }
        }
    }
}

bool TabCtrlItem::IsFixed() const
{
    return m_bFixed;
}

void TabCtrlItem::UpdateCloseButtonVisible()
{
    if (m_pCloseBtn == nullptr) {
        return;
    }
    if (IsFixed()) {
        //固定标签不显示关闭按钮
        m_pCloseBtn->SetVisible(false);
        return;
    }
    if (IsAutoHideCloseButton()) {
        //自动隐藏：选中或悬停时显示
        bool bShow = IsSelected() ||
                     (GetState() == ControlStateType::kControlStateHovered) ||
                     IsAnimationPlayerPlaying(AnimationType::kAnimationHovered);
        m_pCloseBtn->SetFadeVisible(bShow);
    }
    else {
        m_pCloseBtn->SetVisible(true);
    }
}

}//namespace ui

