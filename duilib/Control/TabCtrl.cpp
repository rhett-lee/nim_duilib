#include "TabCtrl.h"
#include "duilib/Box/TabBox.h"
#include "duilib/Layout/VLayout.h"
#include "duilib/Layout/HLayout.h"
#include <algorithm>
#include <vector>

namespace ui
{

TabCtrl::TabCtrl(Window* pWindow):
    ListBox(pWindow, new HLayout),
    m_nDefaultSelectedIndex(Box::InvalidIndex),
    m_pTabBox(nullptr),
    m_bEnableDragOrder(true),
    m_orientation(TabCtrlOrientation::kHorizontal),
    m_bEnableOverflowScroll(true),
    m_bSyncingTabBox(false),
    m_bEnableWheelSwitch(false),
    m_bEnableMiddleClickClose(true),
    m_bEnableOverflowButtons(true),
    m_bTabItemJustify(false),
    m_bEnableNewTabButton(false),
    m_fSelectedTabItemOutlineWidth(0.0f),
    m_fTabCtrlBottomLineHeight(0.0f),
    m_fSelectedTabItemIndicatorWidth(0.0f),
    m_bSelectedTabItemIndicatorAutoWidth(false),
    m_fSelectedTabItemIndicatorHeight(3.0f),
    m_bSelectedTabItemShowBg(true),
    m_fBottomLinePenHeight(0.0f),
    m_indicatorTextLength(0),
    m_bIndicatorTextLengthValid(false)
{
}

TabCtrl::~TabCtrl()
{
    if (m_pTabBox != nullptr) {
        m_pTabBox->RemoveTabSelectListener(this);
    }
}

void TabCtrl::ClearPaintCache()
{
    m_spBottomLinePen.reset();
    m_bottomLinePenColor.clear();
    m_fBottomLinePenHeight = 0.0f;
    m_indicatorColorKey.clear();
    m_indicatorColorValue = UiColor();
    m_bIndicatorTextLengthValid = false;
}

void TabCtrl::OnThemeChanged(bool bRedraw)
{
    //主题变化后，颜色字符串解析结果、画刷和画笔都需重新生成
    ClearPaintCache();
    BaseClass::OnThemeChanged(bRedraw);
}

void TabCtrl::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
    //DPI变化后字体尺寸变化，缓存的文字测量结果失效
    m_bIndicatorTextLengthValid = false;
}

void TabCtrl::EnsureLayout()
{
    const bool bVertical = (m_orientation == TabCtrlOrientation::kVertical);
    Layout* pLayout = GetLayout();
    const bool bLayoutMatched = bVertical ? (dynamic_cast<VLayout*>(pLayout) != nullptr)
                                          : (dynamic_cast<HLayout*>(pLayout) != nullptr);
    if (bLayoutMatched) {
        return;
    }
    //布局方向不匹配时重建：水平用 HLayout，垂直用 VLayout
    Layout* pNewLayout = bVertical ? static_cast<Layout*>(new VLayout())
                                   : static_cast<Layout*>(new HLayout());
    Layout* pOldLayout = ResetLayout(pNewLayout);
    if (pOldLayout != nullptr) {
        FreeLayout(pOldLayout);
    }
}

void TabCtrl::UpdateScrollBarConfig()
{
    //注意：滚动位置(SetScrollPos)依赖滚动条对象，因此即使启用了溢出箭头，
    //滚动条对象也需要保留，只是不显示（见 UpdateScrollBarVisibility）
    if (m_bEnableOverflowScroll && !m_bTabItemJustify) {
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
    UpdateScrollBarVisibility();
}

void TabCtrl::UpdateScrollBarVisibility()
{
    //溢出箭头与滚动条二选一：箭头模式下隐藏滚动条，避免两者叠加
    ScrollBar* pScrollBar = nullptr;
    if (m_orientation == TabCtrlOrientation::kVertical) {
        pScrollBar = GetVScrollBar();
    }
    else {
        pScrollBar = GetHScrollBar();
    }
    if (pScrollBar == nullptr) {
        return;
    }
    if (m_bEnableOverflowButtons) {
        pScrollBar->SetVisible(false);
        pScrollBar->SetMouseEnabled(false);
    }
    else {
        pScrollBar->SetMouseEnabled(true);
    }
}

DString TabCtrl::GetType() const { return DUI_CTR_TAB_CTRL; }

void TabCtrl::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("selected_id")) {
        int32_t nValue = StringUtil::StringToInt32(strValue);
        if (nValue >= 0) {
            m_nDefaultSelectedIndex = nValue;
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
    else if (strName == _T("selected_tab_item_indicator_width")) {
        if (strValue == _T("auto")) {
            //指示条宽度自动跟随标签标题文字宽度
            SetSelectedTabItemIndicatorAutoWidth(true);
        }
        else {
            SetSelectedTabItemIndicatorAutoWidth(false);
            SetSelectedTabItemIndicatorWidth(StringUtil::StringToFloat(strValue.c_str()));
        }
    }
    else if (strName == _T("selected_tab_item_indicator_height")) {
        SetSelectedTabItemIndicatorHeight(StringUtil::StringToFloat(strValue.c_str()));
    }
    else if (strName == _T("selected_tab_item_indicator_color")) {
        SetSelectedTabItemIndicatorColor(strValue);
    }
    else if (strName == _T("selected_tab_item_show_bg")) {
        SetSelectedTabItemShowBg(StringUtil::IsValueTrue(strValue));
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
    else if (strName == _T("wheel_switch")) {
        //是否启用鼠标滚轮切换标签，默认关闭（滚轮用于滚动标签栏）
        SetEnableWheelSwitch(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("middle_click_close")) {
        //是否启用鼠标中键关闭标签，默认启用
        SetEnableMiddleClickClose(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("drag_insert_color")) {
        //拖动排序时插入位置提示线的颜色
        SetDragInsertColor(strValue);
    }
    else if (strName == _T("overflow_buttons")) {
        //标签溢出时是否显示滚动箭头按钮，默认启用
        SetEnableOverflowButtons(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("tab_item_justify")) {
        //标签是否等宽填充，默认不启用
        SetTabItemJustify(StringUtil::IsValueTrue(strValue));
    }
    else if (strName == _T("new_tab_button")) {
        //是否显示"新建标签"按钮，默认不显示
        SetEnableNewTabButton(StringUtil::IsValueTrue(strValue));
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

    //批量重排：直接按目标顺序生成新的子项列表，只触发一次布局
    std::vector<Control*> newItems;
    newItems.reserve(nCount);
    for (Control* pItem : vecFixed) {
        if (pItem != nullptr) {
            newItems.push_back(pItem);
        }
    }
    //防御：保留未被遍历到的子控件（正常不会出现），避免遗漏
    for (Control* pItem : m_items) {
        if (std::find(newItems.begin(), newItems.end(), pItem) == newItems.end()) {
            newItems.push_back(pItem);
        }
    }
    if (newItems == m_items) {
        return;
    }

    //记录当前选中项，重排后按新的索引恢复（不触发选择事件）
    Control* pSelectedItem = nullptr;
    const size_t nOldSel = GetCurSel();
    if (nOldSel != Box::InvalidIndex) {
        pSelectedItem = GetItemAt(nOldSel);
    }
    m_items.swap(newItems);
    for (size_t i = 0; i < m_items.size(); ++i) {
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(m_items[i]);
        if (pListItem != nullptr) {
            pListItem->SetListBoxIndex(i);
        }
    }
    if (pSelectedItem != nullptr) {
        SetCurSel(GetItemIndex(pSelectedItem));
    }
    Arrange();
    AdjustItemLineStatus();
}

void TabCtrl::SetTabCtrlOrientation(TabCtrlOrientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }
    m_orientation = orientation;
    //切换布局方向：仅在当前布局与目标方向不一致时重建
    EnsureLayout();
    //按新的布局方向调整标签项宽度与分割线方向
    UpdateItemsLayoutForOrientation();
    if (IsInited()) {
        //重新配置滚动条并触发重排
        UpdateScrollBarConfig();
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
        UpdateScrollBarConfig();
        if (GetWindow() != nullptr) {
            Invalidate();
        }
    }
}

bool TabCtrl::IsEnableOverflowScroll() const
{
    return m_bEnableOverflowScroll;
}

void TabCtrl::SetEnableWheelSwitch(bool bEnable)
{
    m_bEnableWheelSwitch = bEnable;
}

bool TabCtrl::IsEnableWheelSwitch() const
{
    return m_bEnableWheelSwitch;
}

void TabCtrl::SetEnableMiddleClickClose(bool bEnable)
{
    m_bEnableMiddleClickClose = bEnable;
}

bool TabCtrl::IsEnableMiddleClickClose() const
{
    return m_bEnableMiddleClickClose;
}

void TabCtrl::SetDragInsertColor(const DString& insertColor)
{
    m_dragInsertColor = insertColor;
}

const DString& TabCtrl::GetDragInsertColor() const
{
    return m_dragInsertColor;
}

void TabCtrl::SetEnableOverflowButtons(bool bEnable)
{
    if (m_bEnableOverflowButtons == bEnable) {
        return;
    }
    m_bEnableOverflowButtons = bEnable;
    if (IsInited()) {
        UpdateScrollBarVisibility();
        Arrange();
    }
    if (GetWindow() != nullptr) {
        Invalidate();
    }
}

bool TabCtrl::IsEnableOverflowButtons() const
{
    return m_bEnableOverflowButtons;
}

void TabCtrl::SetTabItemJustify(bool bJustify)
{
    if (m_bTabItemJustify == bJustify) {
        return;
    }
    m_bTabItemJustify = bJustify;
    if (IsInited()) {
        //等宽填充时总宽度等于可视宽度，不需要滚动
        UpdateScrollBarConfig();
        Arrange();
        if (GetWindow() != nullptr) {
            Invalidate();
        }
    }
}

bool TabCtrl::IsTabItemJustify() const
{
    return m_bTabItemJustify;
}

void TabCtrl::SetEnableNewTabButton(bool bEnable)
{
    if (m_bEnableNewTabButton == bEnable) {
        return;
    }
    m_bEnableNewTabButton = bEnable;
    if (GetWindow() != nullptr) {
        Invalidate();
    }
}

bool TabCtrl::IsEnableNewTabButton() const
{
    return m_bEnableNewTabButton;
}

TabCtrlItem* TabCtrl::GetSelectedTabItem() const
{
    std::vector<size_t> selectedIndexs;
    GetSelectedItems(selectedIndexs);
    if (selectedIndexs.empty()) {
        return nullptr;
    }
    return dynamic_cast<TabCtrlItem*>(GetItemAt(selectedIndexs[0]));
}

bool TabCtrl::SetSelectedTabItem(TabCtrlItem* pItem, bool bTriggerEvent)
{
    if (pItem == nullptr) {
        return false;
    }
    const size_t nIndex = GetItemIndex(pItem);
    if (nIndex == Box::InvalidIndex) {
        return false;
    }
    return SelectItem(nIndex, false, bTriggerEvent);
}

TabCtrlItem* TabCtrl::GetTabItemAt(size_t iIndex) const
{
    if (iIndex == Box::InvalidIndex) {
        return nullptr;
    }
    return dynamic_cast<TabCtrlItem*>(GetItemAt(iIndex));
}

bool TabCtrl::AddTabItemToEnd(Control* pItem)
{
    if (pItem == nullptr) {
        return false;
    }
    return AddItem(pItem);
}

bool TabCtrl::SelectNextTab(bool bTakeFocus, bool bTriggerEvent)
{
    const size_t nCount = GetItemCount();
    if (nCount == 0) {
        return false;
    }
    size_t nIndex = 0;
    const size_t nCurSel = GetCurSel();
    if (nCurSel != Box::InvalidIndex) {
        nIndex = (nCurSel + 1) % nCount;
    }
    return SelectItem(nIndex, bTakeFocus, bTriggerEvent);
}

bool TabCtrl::SelectPrevTab(bool bTakeFocus, bool bTriggerEvent)
{
    const size_t nCount = GetItemCount();
    if (nCount == 0) {
        return false;
    }
    size_t nIndex = nCount - 1;
    const size_t nCurSel = GetCurSel();
    if (nCurSel != Box::InvalidIndex) {
        nIndex = (nCurSel == 0) ? (nCount - 1) : (nCurSel - 1);
    }
    return SelectItem(nIndex, bTakeFocus, bTriggerEvent);
}

void TabCtrl::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    if (m_bTabItemJustify) {
        LayoutJustify();
    }
    //重排过程中滚动条可能被重新显示，这里再隐藏一次（箭头模式下不显示滚动条）
    UpdateScrollBarVisibility();
}

bool TabCtrl::MouseEnter(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseEnter(msg);
    //基类会在鼠标进入时淡入悬浮滚动条，箭头模式下需要保持隐藏
    UpdateScrollBarVisibility();
    return bRet;
}

void TabCtrl::LayoutJustify()
{
    //收集参与等宽布局的标签项（可见、非浮动）
    std::vector<TabCtrlItem*> items;
    const size_t nCount = GetItemCount();
    items.reserve(nCount);
    for (size_t i = 0; i < nCount; ++i) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(i));
        if ((pItem == nullptr) || !pItem->IsVisible() || pItem->IsFloat()) {
            continue;
        }
        items.push_back(pItem);
    }
    if (items.empty()) {
        return;
    }
    UiRect rcClient = GetRect();
    rcClient.Deflate(GetPadding());
    if (rcClient.IsEmpty()) {
        return;
    }
    const int32_t nItemCount = static_cast<int32_t>(items.size());
    if (m_orientation == TabCtrlOrientation::kVertical) {
        int32_t nEach = rcClient.Height() / nItemCount;
        if (nEach <= 0) {
            return;
        }
        int32_t nTop = rcClient.top;
        for (int32_t i = 0; i < nItemCount; ++i) {
            int32_t nBottom = (i + 1 == nItemCount) ? rcClient.bottom : (nTop + nEach);
            TabCtrlItem* pItem = items[(size_t)i];
            //应用最小/最大高度约束
            const int32_t nMinHeight = pItem->GetMinHeight();
            if ((nMinHeight > 0) && ((nBottom - nTop) < nMinHeight)) {
                nBottom = std::min(rcClient.bottom, nTop + nMinHeight);
            }
            const int32_t nMaxHeight = pItem->GetMaxHeight();
            if ((nMaxHeight > 0) && ((nBottom - nTop) > nMaxHeight)) {
                nBottom = nTop + nMaxHeight;
            }
            pItem->SetPos(UiRect(rcClient.left, nTop, rcClient.right, nBottom));
            nTop = nBottom;
        }
    }
    else {
        int32_t nEach = rcClient.Width() / nItemCount;
        if (nEach <= 0) {
            return;
        }
        int32_t nLeft = rcClient.left;
        for (int32_t i = 0; i < nItemCount; ++i) {
            int32_t nRight = (i + 1 == nItemCount) ? rcClient.right : (nLeft + nEach);
            TabCtrlItem* pItem = items[(size_t)i];
            //应用最小/最大宽度约束
            const int32_t nMinWidth = pItem->GetMinWidth();
            if ((nMinWidth > 0) && ((nRight - nLeft) < nMinWidth)) {
                nRight = std::min(rcClient.right, nLeft + nMinWidth);
            }
            const int32_t nMaxWidth = pItem->GetMaxWidth();
            if ((nMaxWidth > 0) && ((nRight - nLeft) > nMaxWidth)) {
                nRight = nLeft + nMaxWidth;
            }
            pItem->SetPos(UiRect(nLeft, rcClient.top, nRight, rcClient.bottom));
            nLeft = nRight;
        }
    }
}

void TabCtrl::UpdateItemsLayoutForOrientation()
{
    const bool bVertical = (m_orientation == TabCtrlOrientation::kVertical);
    const size_t nCount = GetItemCount();
    for (size_t i = 0; i < nCount; ++i) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(i));
        if (pItem == nullptr) {
            continue;
        }
        if (bVertical) {
            //垂直标签栏：标签铺满标签栏宽度
            if (pItem->GetFixedWidth().IsAuto()) {
                pItem->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
            }
        }
        else {
            //水平标签栏：恢复为按内容自适应宽度
            if (pItem->GetFixedWidth().IsStretch()) {
                pItem->SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
            }
        }
        pItem->UpdateLineOrientation(bVertical);
    }
}

bool TabCtrl::ButtonDown(const EventArgs& msg)
{
    if ((msg.eventType == kEventMouseButtonDown) && HandleTabStripClick(msg.ptMouse)) {
        return true;
    }
    return BaseClass::ButtonDown(msg);
}

bool TabCtrl::OnListBoxKeyDown(const EventArgs& msg)
{
    if (msg.eventType != kEventKeyDown) {
        return BaseClass::OnListBoxKeyDown(msg);
    }
    if (IsKeyDown(msg, ModifierKey::kControl)) {
        if (msg.vkCode == kVK_TAB) {
            //Ctrl+Tab 选择下一个标签，Ctrl+Shift+Tab 选择上一个标签
            if (IsKeyDown(msg, ModifierKey::kShift)) {
                return SelectPrevTab(true, true);
            }
            return SelectNextTab(true, true);
        }
        if ((msg.vkCode >= kVK_1) && (msg.vkCode <= kVK_9)) {
            //Ctrl+1..9 选择对应索引的标签
            const size_t nIndex = static_cast<size_t>(msg.vkCode - kVK_1);
            if (nIndex < GetItemCount()) {
                return SelectItem(nIndex, true, true);
            }
            return false;
        }
    }
    return BaseClass::OnListBoxKeyDown(msg);
}

bool TabCtrl::IsTabOverflow() const
{
    const UiSize64 szRange = GetScrollRange();
    if (m_orientation == TabCtrlOrientation::kVertical) {
        return szRange.cy > 0;
    }
    return szRange.cx > 0;
}

bool TabCtrl::CalcOverflowButtonRects(UiRect& rcPrev, UiRect& rcNext) const
{
    rcPrev.Clear();
    rcNext.Clear();
    if (!m_bEnableOverflowButtons || !IsTabOverflow()) {
        return false;
    }
    const UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return false;
    }
    const int32_t nSize = Dpi().GetScaleInt(20);
    if (m_orientation == TabCtrlOrientation::kVertical) {
        const int32_t nHeight = std::min(nSize, rc.Height() / 2);
        if (nHeight <= 0) {
            return false;
        }
        rcPrev = UiRect(rc.left, rc.top, rc.right, rc.top + nHeight);
        rcNext = UiRect(rc.left, rc.bottom - nHeight, rc.right, rc.bottom);
    }
    else {
        const int32_t nWidth = std::min(nSize, rc.Width() / 2);
        if (nWidth <= 0) {
            return false;
        }
        rcPrev = UiRect(rc.left, rc.top, rc.left + nWidth, rc.bottom);
        rcNext = UiRect(rc.right - nWidth, rc.top, rc.right, rc.bottom);
    }
    return true;
}

bool TabCtrl::CalcNewTabButtonRect(UiRect& rcButton) const
{
    rcButton.Clear();
    if (!m_bEnableNewTabButton) {
        return false;
    }
    const UiRect rc = GetRect();
    if (rc.IsEmpty()) {
        return false;
    }
    const int32_t nSize = Dpi().GetScaleInt(24);
    const UiSize szScroll = GetScrollOffset();

    //找到最后一个可见标签项的末端位置
    TabCtrlItem* pLastItem = nullptr;
    const size_t nCount = GetItemCount();
    for (size_t i = nCount; i > 0; --i) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(i - 1));
        if ((pItem != nullptr) && pItem->IsVisible()) {
            pLastItem = pItem;
            break;
        }
    }

    UiRect rcPrev;
    UiRect rcNext;
    const bool bOverflow = CalcOverflowButtonRects(rcPrev, rcNext);
    if (m_orientation == TabCtrlOrientation::kVertical) {
        int32_t nTop = rc.top + 2;
        if (pLastItem != nullptr) {
            nTop = pLastItem->GetRect().bottom - szScroll.cy;
        }
        if (bOverflow) {
            //避免与底部"向后"箭头重叠
            nTop = std::min(nTop, rcNext.top - nSize - 2);
        }
        if (nTop + nSize > rc.bottom) {
            return false;
        }
        rcButton = UiRect(rc.left + 2, nTop + 2, rc.right - 2, nTop + 2 + nSize);
    }
    else {
        int32_t nLeft = rc.left + 2;
        if (pLastItem != nullptr) {
            nLeft = pLastItem->GetRect().right - szScroll.cx;
        }
        if (bOverflow) {
            //避免与右侧"向后"箭头重叠
            nLeft = std::min(nLeft, rcNext.left - nSize - 2);
        }
        if (nLeft + nSize > rc.right) {
            return false;
        }
        rcButton = UiRect(nLeft + 2, rc.top + 2, nLeft + 2 + nSize, rc.bottom - 2);
    }
    return true;
}

void TabCtrl::NotifyTabCloseRequest(TabCtrlItem* pItem)
{
    if (pItem == nullptr) {
        return;
    }
    const size_t nIndex = GetItemIndex(pItem);
    if (nIndex == Box::InvalidIndex) {
        return;
    }
    //只发送请求事件，不删除标签（删除操作由业务层完成）
    SendEvent(kEventTabClose, static_cast<WPARAM>(nIndex));
}

bool TabCtrl::HandleTabStripClick(const UiPoint& pt)
{
    UiRect rcPrev;
    UiRect rcNext;
    if (CalcOverflowButtonRects(rcPrev, rcNext)) {
        if (rcPrev.ContainsPt(pt)) {
            //按可视范围翻页滚动（ScrollBox 的 LineXxx 默认步长为 0，不会产生滚动）
            if (m_orientation == TabCtrlOrientation::kVertical) {
                PageUp();
            }
            else {
                PageLeft();
            }
            return true;
        }
        if (rcNext.ContainsPt(pt)) {
            if (m_orientation == TabCtrlOrientation::kVertical) {
                PageDown();
            }
            else {
                PageRight();
            }
            return true;
        }
    }
    UiRect rcNewTab;
    if (CalcNewTabButtonRect(rcNewTab) && rcNewTab.ContainsPt(pt)) {
        //仅发送事件，不自动创建标签（由业务层处理）
        SendEvent(kEventTabNew);
        return true;
    }
    return false;
}

void TabCtrl::PaintOverflowButtons(IRender* pRender)
{
    UiRect rcPrev;
    UiRect rcNext;
    if ((pRender == nullptr) || !CalcOverflowButtonRects(rcPrev, rcNext)) {
        return;
    }
    const UiColor bkColor = GetUiColor(_T("#66000000"));
    const UiColor lineColor = GetUiColor(_T("#FFFFFFFF"));
    const float fLineWidth = (float)Dpi().GetScaleInt(2);
    pRender->FillRoundRect(UiRectF::MakeFromRect(rcPrev), 3.0f, 3.0f, bkColor);
    pRender->FillRoundRect(UiRectF::MakeFromRect(rcNext), 3.0f, 3.0f, bkColor);

    const float fPrevX = (float)rcPrev.left + rcPrev.Width() / 2.0f;
    const float fPrevY = (float)rcPrev.top + rcPrev.Height() / 2.0f;
    const float fNextX = (float)rcNext.left + rcNext.Width() / 2.0f;
    const float fNextY = (float)rcNext.top + rcNext.Height() / 2.0f;
    const float fSize = (float)std::min(rcPrev.Width(), rcPrev.Height()) / 4.0f;
    if (m_orientation == TabCtrlOrientation::kVertical) {
        //向上、向下箭头
        pRender->DrawLine(UiPointF(fPrevX - fSize, fPrevY + fSize / 2), UiPointF(fPrevX, fPrevY - fSize / 2), lineColor, fLineWidth);
        pRender->DrawLine(UiPointF(fPrevX, fPrevY - fSize / 2), UiPointF(fPrevX + fSize, fPrevY + fSize / 2), lineColor, fLineWidth);
        pRender->DrawLine(UiPointF(fNextX - fSize, fNextY - fSize / 2), UiPointF(fNextX, fNextY + fSize / 2), lineColor, fLineWidth);
        pRender->DrawLine(UiPointF(fNextX, fNextY + fSize / 2), UiPointF(fNextX + fSize, fNextY - fSize / 2), lineColor, fLineWidth);
    }
    else {
        //向左、向右箭头
        pRender->DrawLine(UiPointF(fPrevX + fSize / 2, fPrevY - fSize), UiPointF(fPrevX - fSize / 2, fPrevY), lineColor, fLineWidth);
        pRender->DrawLine(UiPointF(fPrevX - fSize / 2, fPrevY), UiPointF(fPrevX + fSize / 2, fPrevY + fSize), lineColor, fLineWidth);
        pRender->DrawLine(UiPointF(fNextX - fSize / 2, fNextY - fSize), UiPointF(fNextX + fSize / 2, fNextY), lineColor, fLineWidth);
        pRender->DrawLine(UiPointF(fNextX + fSize / 2, fNextY), UiPointF(fNextX - fSize / 2, fNextY + fSize), lineColor, fLineWidth);
    }
}

void TabCtrl::PaintNewTabButton(IRender* pRender)
{
    UiRect rcButton;
    if ((pRender == nullptr) || !CalcNewTabButtonRect(rcButton)) {
        return;
    }
    const UiColor bkColor = GetUiColor(_T("#33000000"));
    const UiColor lineColor = GetUiColor(_T("#FF333333"));
    const float fLineWidth = (float)Dpi().GetScaleInt(2);
    pRender->FillRoundRect(UiRectF::MakeFromRect(rcButton), 3.0f, 3.0f, bkColor);
    const float fCenterX = (float)rcButton.left + rcButton.Width() / 2.0f;
    const float fCenterY = (float)rcButton.top + rcButton.Height() / 2.0f;
    const float fSize = (float)std::min(rcButton.Width(), rcButton.Height()) / 4.0f;
    pRender->DrawLine(UiPointF(fCenterX - fSize, fCenterY), UiPointF(fCenterX + fSize, fCenterY), lineColor, fLineWidth);
    pRender->DrawLine(UiPointF(fCenterX, fCenterY - fSize), UiPointF(fCenterX, fCenterY + fSize), lineColor, fLineWidth);
}

void TabCtrl::PaintDragInsertIndicator(IRender* pRender)
{
    if ((pRender == nullptr) || m_dragInsertColor.empty()) {
        return;
    }
    const size_t nCount = GetItemCount();
    for (size_t i = 0; i < nCount; ++i) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(i));
        if ((pItem == nullptr) || !pItem->IsInDraggingOrder() || !pItem->IsVisible()) {
            continue;
        }
        UiRect rcItem = pItem->GetRect();
        if (rcItem.IsEmpty()) {
            continue;
        }
        //子控件绘制时按滚动量偏移，这里换算到当前绘制坐标系
        const UiSize szScroll = GetScrollOffset();
        rcItem.Offset(-szScroll.cx, -szScroll.cy);
        const UiColor color = GetUiColor(m_dragInsertColor);
        const float fLineWidth = (float)Dpi().GetScaleInt(2);
        if (m_orientation == TabCtrlOrientation::kVertical) {
            //垂直布局：在标签上方画横线
            pRender->DrawLine(UiPointF((float)rcItem.left, (float)rcItem.top),
                              UiPointF((float)rcItem.right, (float)rcItem.top),
                              color, fLineWidth);
        }
        else {
            //水平布局：在标签左侧画竖线
            pRender->DrawLine(UiPointF((float)rcItem.left, (float)rcItem.top),
                              UiPointF((float)rcItem.left, (float)rcItem.bottom),
                              color, fLineWidth);
        }
    }
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

void TabCtrl::SetSelectedTabItemIndicatorWidth(float fIndicatorWidth)
{
    if (fIndicatorWidth < 0.0f) {
        fIndicatorWidth = 0.0f;
    }
    m_fSelectedTabItemIndicatorWidth = fIndicatorWidth;
}

float TabCtrl::GetSelectedTabItemIndicatorWidth() const
{
    return m_fSelectedTabItemIndicatorWidth;
}

void TabCtrl::SetSelectedTabItemIndicatorAutoWidth(bool bAuto)
{
    m_bSelectedTabItemIndicatorAutoWidth = bAuto;
}

bool TabCtrl::IsSelectedTabItemIndicatorAutoWidth() const
{
    return m_bSelectedTabItemIndicatorAutoWidth;
}

void TabCtrl::SetSelectedTabItemIndicatorHeight(float fIndicatorHeight)
{
    if (fIndicatorHeight < 0.0f) {
        fIndicatorHeight = 0.0f;
    }
    m_fSelectedTabItemIndicatorHeight = fIndicatorHeight;
}

float TabCtrl::GetSelectedTabItemIndicatorHeight() const
{
    return m_fSelectedTabItemIndicatorHeight;
}

void TabCtrl::SetSelectedTabItemIndicatorColor(const DString& indicatorColor)
{
    m_selectedTabItemIndicatorColor = indicatorColor;
}

const DString& TabCtrl::GetSelectedTabItemIndicatorColor() const
{
    return m_selectedTabItemIndicatorColor;
}

void TabCtrl::SetSelectedTabItemShowBg(bool bShow)
{
    m_bSelectedTabItemShowBg = bShow;
}

bool TabCtrl::IsSelectedTabItemShowBg() const
{
    return m_bSelectedTabItemShowBg;
}

void TabCtrl::OnInit()
{
    if (IsInited()) {
        return;
    }
    //设置布局方向（需在布局计算前完成）：仅在当前布局与目标方向不一致时重建
    EnsureLayout();

    BaseClass::OnInit();

    //固定标签归位：保证固定标签始终位于标签栏前端（XML 中未按顺序书写时也能自愈）
    NormalizeFixedItems();

    //按布局方向调整标签项与分割线（垂直方向标签铺满宽度、分割线改为底部横线）
    UpdateItemsLayoutForOrientation();

    //设置为单选
    SetMultiSelect(false);

    //绑定TabBox：XML 解析时TabBox可能尚未创建，此时控件树已完整，可稳定绑定
    if ((m_pTabBox == nullptr) && !m_tabBoxName.empty()) {
        SetTabBoxName(m_tabBoxName.c_str());
    }

    //默认选择的子项
    if (m_nDefaultSelectedIndex != Box::InvalidIndex) {
        SelectItem(m_nDefaultSelectedIndex);
    }

    //调整标签分割线的显示状态
    AdjustItemLineStatus();

    //溢出滚动：按当前配置准备滚动条（悬浮在子控件之上，不占用布局空间）
    UpdateScrollBarConfig();
}

void TabCtrl::HandleEvent(const EventArgs& msg)
{
    if ((msg.eventType == kEventMouseWheel) && m_bEnableWheelSwitch) {
        //滚轮切换标签（可通过 wheel_switch 关闭，关闭后滚轮仍用于滚动标签栏）
        const int32_t nDelta = msg.eventData;
        if ((nDelta != 0) && (GetItemCount() > 1)) {
            if (nDelta > 0) {
                SelectPrevTab(false, true);
            }
            else {
                SelectNextTab(false, true);
            }
            return;
        }
    }
    if ((msg.GetSender() == this) && (msg.eventType == kEventSelect)) {
        //尝试设置关联的TabBox（首次选择时可能尚未绑定）
        if ((m_pTabBox == nullptr) && !m_tabBoxName.empty()) {
            SetTabBoxName(m_tabBoxName.c_str());
        }
        //把当前选择项同步到TabBox
        SyncTabBoxSelection();
    }
    BaseClass::HandleEvent(msg);

    if ((msg.GetSender() == this) && ((msg.eventType == kEventSelect) || (msg.eventType == kEventUnSelect))) {
        //基类处理时可能触发用户回调（存在增删标签的可能），这里重新获取标签项
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(msg.wParam));
        if (pItem != nullptr) {
            pItem->AdjustItemLineStatus();
        }
        if (msg.eventType == kEventSelect) {
            //选中项自动滚入可视区域（保证键盘/程序化选择不会落在可视区外）
            const size_t nCurSel = GetCurSel();
            if (nCurSel != Box::InvalidIndex) {
                SelectEnsureVisible(nCurSel, false);
            }
        }
    }
}

void TabCtrl::PaintStateColors(IRender* pRender)
{
    BaseClass::PaintStateColors(pRender);
    //在标签底部画线（但排除当前选择标签的区域，不覆盖选择标签）
    const float fBottomLineHeight = GetTabCtrlBottomLineHeight();
    const DString& strBottomLineColor = GetTabCtrlBottomLineColor();
    //画笔缓存：仅在颜色或高度变化时重建
    IPen* pBottomLinePen = nullptr;
    if ((fBottomLineHeight > 0.01f) && !strBottomLineColor.empty()) {
        if ((m_spBottomLinePen == nullptr) ||
            (m_fBottomLinePenHeight != fBottomLineHeight) ||
            (m_bottomLinePenColor != strBottomLineColor)) {
            IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
            if (pRenderFactory != nullptr) {
                m_spBottomLinePen.reset(pRenderFactory->CreatePen(GetUiColor(strBottomLineColor), fBottomLineHeight));
                m_fBottomLinePenHeight = fBottomLineHeight;
                m_bottomLinePenColor = strBottomLineColor;
            }
        }
        pBottomLinePen = m_spBottomLinePen.get();
    }
    if (pBottomLinePen != nullptr) {
        UiRectF rcTabCtrl = UiRectF::MakeFromRect(GetRect());
        if (m_orientation == TabCtrlOrientation::kVertical) {
            //垂直标签栏：在右侧画线，分隔标签列与内容区
            rcTabCtrl.right -= (fBottomLineHeight / 2);
            pRender->DrawLine(UiPointF(rcTabCtrl.right, rcTabCtrl.top),
                              UiPointF(rcTabCtrl.right, rcTabCtrl.bottom),
                              pBottomLinePen);
        }
        else {
            //水平标签栏：在底部画线
            rcTabCtrl.bottom -= (fBottomLineHeight / 2);
            bool bDrawFullLine = true;
            if (m_bSelectedTabItemShowBg) {
                //绘制选中标签背景时，按选中标签的左右边界分段绘制，选择标签的区域不绘制
                std::vector<size_t> selectedIndexs;
                GetSelectedItems(selectedIndexs);
                if (!selectedIndexs.empty()) {
                    Control* pSelectedItem = GetItemAt(selectedIndexs[0]);
                    if (pSelectedItem != nullptr) {
                        const UiRect rcSelected = pSelectedItem->GetRect();
                        if (!rcSelected.IsEmpty()) {
                            bDrawFullLine = false;
                            if (rcSelected.left > rcTabCtrl.left) {
                                pRender->DrawLine(UiPointF(rcTabCtrl.left, rcTabCtrl.bottom),
                                                  UiPointF(std::min(rcTabCtrl.right, (float)rcSelected.left), rcTabCtrl.bottom),
                                                  pBottomLinePen);
                            }
                            if (rcSelected.right < rcTabCtrl.right) {
                                pRender->DrawLine(UiPointF(std::max(rcTabCtrl.left, (float)rcSelected.right), rcTabCtrl.bottom),
                                                  UiPointF(rcTabCtrl.right, rcTabCtrl.bottom),
                                                  pBottomLinePen);
                            }
                        }
                    }
                }
            }
            if (bDrawFullLine) {
                pRender->DrawLine(UiPointF(rcTabCtrl.left, rcTabCtrl.bottom),
                                  UiPointF(rcTabCtrl.right, rcTabCtrl.bottom),
                                  pBottomLinePen);
            }
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
    if (m_pTabBox == pTabBox) {
        if ((pTabBox != nullptr) && IsInited()) {
            SyncTabBoxSelection();
        }
        return;
    }
    //先解绑旧的TabBox，避免继续接收其选择事件
    if (m_pTabBox != nullptr) {
        m_pTabBox->RemoveTabSelectListener(this);
    }
    m_pTabBox = pTabBox;
    if (m_pTabBox != nullptr) {
        //注册监听，实现 TabBox -> TabCtrl 的反向同步（不会覆盖用户注册的选择事件）
        m_pTabBox->AddTabSelectListener(this);
        if (IsInited()) {
            //绑定后同步一次当前选择项，避免绑定前的选择状态与TabBox不一致
            SyncTabBoxSelection();
        }
    }
}

void TabCtrl::OnBoundTabBoxSelectChanged(size_t nTabBoxIndex)
{
    if (m_bSyncingTabBox) {
        //该变化由 TabCtrl -> TabBox 的同步触发，直接忽略，避免形成死循环
        return;
    }
    TabCtrlItem* pItem = FindTabItemByTabBoxIndex(nTabBoxIndex);
    if (pItem == nullptr) {
        return;
    }
    const size_t nIndex = GetItemIndex(pItem);
    if (nIndex == Box::InvalidIndex) {
        return;
    }
    m_bSyncingTabBox = true;
    SelectItem(nIndex, false, true);
    m_bSyncingTabBox = false;
}

TabCtrlItem* TabCtrl::FindTabItemByTabBoxIndex(size_t nTabBoxIndex) const
{
    if (nTabBoxIndex == Box::InvalidIndex) {
        return nullptr;
    }
    //优先使用显式配置的绑定索引
    const size_t nCount = GetItemCount();
    for (size_t i = 0; i < nCount; ++i) {
        TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(i));
        if ((pItem != nullptr) && (pItem->GetTabBoxItemIndex() == nTabBoxIndex)) {
            return pItem;
        }
    }
    //未配置绑定索引时，回退为相同位置的标签项
    if (nTabBoxIndex < nCount) {
        return dynamic_cast<TabCtrlItem*>(GetItemAt(nTabBoxIndex));
    }
    return nullptr;
}

void TabCtrl::SyncTabBoxSelection()
{
    if (m_bSyncingTabBox || (m_pTabBox == nullptr)) {
        return;
    }
    std::vector<size_t> selectedIndexs;
    GetSelectedItems(selectedIndexs);
    if (selectedIndexs.empty()) {
        return;
    }
    TabCtrlItem* pItem = dynamic_cast<TabCtrlItem*>(GetItemAt(selectedIndexs[0]));
    if (pItem == nullptr) {
        return;
    }
    size_t nItemIndex = pItem->GetTabBoxItemIndex();
    if (nItemIndex == Box::InvalidIndex) {
        //未配置绑定索引时，回退为标签索引
        nItemIndex = GetItemIndex(pItem);
    }
    if ((nItemIndex != Box::InvalidIndex) && (nItemIndex < m_pTabBox->GetItemCount())) {
        m_bSyncingTabBox = true;
        m_pTabBox->SelectItem(nItemIndex);
        m_bSyncingTabBox = false;
    }
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
            if (iIndex >= nCount) {
                //移到末尾时索引不能等于元素个数（Box::SetItemIndex 要求索引小于元素个数）
                iIndex = (nCount > 0) ? (nCount - 1) : 0;
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
        if (pItem != nullptr) {
            //按当前布局方向调整新标签项的宽度与分割线方向
            if (m_orientation == TabCtrlOrientation::kVertical) {
                if (pItem->GetFixedWidth().IsAuto()) {
                    pItem->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
                }
            }
            pItem->UpdateLineOrientation(m_orientation == TabCtrlOrientation::kVertical);
        }
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

void TabCtrl::AdjustItemLineStatus()
{
    TabCtrlItem* pLastItem = nullptr;
    TabCtrlItem* pLastVisibleItem = nullptr;
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

        //记录上一个标签
        pLastItem = pItem;
        pLastVisibleItem = pItem;
    }
    if (pLastVisibleItem != nullptr) {
        //最后一个可见标签，不显示分割线
        pLastVisibleItem->SetItemLineVisible(false);
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
    m_bClosable(true),
    m_bLineVertical(false),
    m_bLineOrientationValid(false),
    m_bUserDragOrder(true),
    m_bFixed(false),
    m_bLineVisible(true),
    m_fSelectedOutlinePenWidth(0.0f),
    m_bSelectedPathValid(false),
    m_nTabBoxItemIndex(Box::InvalidIndex)
{
    m_rcSelected.cx = -1;
    m_rcSelected.cy = -1;

    m_rcHovered.cx = -1;
    m_rcHovered.cy = -1;

    m_hoveredPadding.top = 0;
    m_hoveredPadding.left = -1;
    m_hoveredPadding.right = 0;
    m_hoveredPadding.bottom = 0;
}

TabCtrlItem::~TabCtrlItem() = default;

void TabCtrlItem::ClearPaintCache()
{
    m_spSelectedBgBrush.reset();
    m_selectedBgBrushColor.clear();
    m_spSelectedOutlinePen.reset();
    m_selectedOutlinePenColor.clear();
    m_fSelectedOutlinePenWidth = 0.0f;
    m_spSelectedPath.reset();
    m_bSelectedPathValid = false;
    m_hoveredColorKey.clear();
    m_hoveredColorValue = UiColor();
}

void TabCtrlItem::OnThemeChanged(bool bRedraw)
{
    //主题变化后，颜色字符串解析结果、画刷和画笔都需重新生成
    ClearPaintCache();
    BaseClass::OnThemeChanged(bRedraw);
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
    else if (strName == _T("title_color")) {
        SetTitleColor(strValue);
    }
    else if (strName == _T("hovered_title_color")) {
        SetHoveredTitleColor(strValue);
    }
    else if (strName == _T("selected_title_color")) {
        SetSelectedTitleColor(strValue);
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
    else if (strName == _T("closable")) {
        //标签是否可关闭，默认可关闭
        SetClosable(StringUtil::IsValueTrue(strValue));
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
    else if (strName == _T("drag_order")) {
        //记录用户配置的拖动排序开关：固定标签期间会临时关闭，取消固定后按此值恢复
        m_bUserDragOrder = StringUtil::IsValueTrue(strValue);
        SetEnableDragOrder(m_bUserDragOrder && !m_bFixed);
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
    //按标签栏方向初始化分割线布局
    TabCtrl* pTabCtrl = GetTabCtrl();
    if (pTabCtrl != nullptr) {
        UpdateLineOrientation(pTabCtrl->GetTabCtrlOrientation() == TabCtrlOrientation::kVertical);
    }
}

void TabCtrlItem::HandleEvent(const EventArgs& msg)
{
    BaseClass::HandleEvent(msg);
    if ((msg.GetSender() == this) && (msg.eventType == kEventStateChanged)) {
        //处理分割线的状态（内部对未变化的项直接跳过，不会再重复设置）
        AdjustItemLineStatus();
        //更新标题文字的颜色
        UpdateTitleTextColor();
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
            //保留图标资源字符串，重新创建图标控件后可恢复
            if (m_iconImageString.empty()) {
                m_iconImageString = m_pIcon->GetBkImage();
            }
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
        bool bCreatedOrChanged = false;
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
            bCreatedOrChanged = true;
        }
        else if (bChanged) {
            m_pLabel->SetClass(titleClass);
            bCreatedOrChanged = true;
        }
        if (bCreatedOrChanged) {
            //记录原始的文字颜色（未配置状态颜色时用于恢复），并应用配置的状态颜色
            m_defaultTitleColor = m_pLabel->GetStateTextColor(kControlStateNormal).c_str();
            UpdateTitleTextColor();
        }
    }
    else {
        if (m_pLabel != nullptr) {
            //保留文字内容和多语ID，重新创建文本控件后可恢复
            if (m_title.empty()) {
                m_title = m_pLabel->GetText();
            }
            if (m_titleId.empty()) {
                m_titleId = m_pLabel->GetTextId();
            }
            RemoveItem(m_pLabel);
            m_pLabel = nullptr;
        }
        m_defaultTitleColor.clear();
    }
}

DString TabCtrlItem::GetTitleClass() const
{
    return m_titleClass.c_str();
}

void TabCtrlItem::SetTitleColor(const DString& titleColor)
{
    if (m_titleColor != titleColor) {
        m_titleColor = titleColor;
        if (IsInited()) {
            UpdateTitleTextColor();
        }
    }
}

DString TabCtrlItem::GetTitleColor() const
{
    return m_titleColor.c_str();
}

void TabCtrlItem::SetHoveredTitleColor(const DString& hoveredTitleColor)
{
    if (m_hoveredTitleColor != hoveredTitleColor) {
        m_hoveredTitleColor = hoveredTitleColor;
        if (IsInited()) {
            UpdateTitleTextColor();
        }
    }
}

DString TabCtrlItem::GetHoveredTitleColor() const
{
    return m_hoveredTitleColor.c_str();
}

void TabCtrlItem::SetSelectedTitleColor(const DString& selectedTitleColor)
{
    if (m_selectedTitleColor != selectedTitleColor) {
        m_selectedTitleColor = selectedTitleColor;
        if (IsInited()) {
            UpdateTitleTextColor();
        }
    }
}

DString TabCtrlItem::GetSelectedTitleColor() const
{
    return m_selectedTitleColor.c_str();
}

void TabCtrlItem::UpdateTitleTextColor()
{
    if (m_pLabel == nullptr) {
        return;
    }
    if (m_titleColor.empty() && m_hoveredTitleColor.empty() && m_selectedTitleColor.empty()) {
        //未配置标题状态颜色：不干预，保持原有颜色
        return;
    }
    DString titleColor;
    if (IsSelected()) {
        titleColor = m_selectedTitleColor.c_str();
    }
    if (titleColor.empty() &&
        ((GetState() == kControlStateHovered) || (GetState() == kControlStatePressed))) {
        titleColor = m_hoveredTitleColor.c_str();
    }
    if (titleColor.empty()) {
        titleColor = m_titleColor.c_str();
    }
    if (titleColor.empty()) {
        titleColor = m_defaultTitleColor.c_str();
    }
    if (titleColor.empty()) {
        return;
    }
    //标题控件的状态恒为Normal，把当前应当显示的颜色写入Normal状态
    m_pLabel->SetStateTextColor(kControlStateNormal, titleColor);
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
            //点击关闭按钮：只发送关闭请求事件，不删除标签（删除由业务层完成）
            m_pCloseBtn->AttachClick([this](const EventArgs&) -> bool {
                if (m_bClosable && !IsFixed()) {
                    TabCtrl* pTabCtrl = GetTabCtrl();
                    if (pTabCtrl != nullptr) {
                        pTabCtrl->NotifyTabCloseRequest(this);
                    }
                }
                return true;
            });
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
            //应用缓存的可见状态，避免重建后分割线闪烁
            m_pLine->SetAlpha(m_bLineVisible ? 255 : 0);
            AddItem(m_pLine);
            AdjustSubItemIndex();
        }
        else if (bChanged) {
            m_pLine->SetClass(lineClass);
            //SetClass会重置控件属性，这里重新应用缓存的可见状态
            m_pLine->SetAlpha(m_bLineVisible ? 255 : 0);
        }
    }
    else {
        if (m_pLine != nullptr) {
            RemoveItem(m_pLine);
            m_pLine = nullptr;
        }
        //分割线控件已移除，可见状态恢复默认值
        m_bLineVisible = true;
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
    //批量重排：直接生成目标顺序的子控件列表，只触发一次布局
    if (subItems.size() > m_items.size()) {
        return;
    }
    for (Control* pControl : subItems) {
        if (std::find(m_items.begin(), m_items.end(), pControl) == m_items.end()) {
            //存在不属于当前容器的控件，放弃调整
            return;
        }
    }
    std::vector<Control*> newItems;
    newItems.reserve(m_items.size());
    newItems.insert(newItems.end(), subItems.begin(), subItems.end());
    for (Control* pControl : m_items) {
        if (std::find(subItems.begin(), subItems.end(), pControl) == subItems.end()) {
            newItems.push_back(pControl);
        }
    }
    if (newItems.size() != m_items.size()) {
        return;
    }
    if (newItems == m_items) {
        return;
    }
    m_items.swap(newItems);
    Arrange();
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
    return (m_pLine != nullptr) && m_bLineVisible && m_pLine->IsVisible() && (m_pLine->GetAlpha() != 0);
}

void TabCtrlItem::SetItemLineVisible(bool bVisible)
{
    if (m_pLine == nullptr) {
        m_bLineVisible = bVisible;
        return;
    }
    if (m_bLineVisible == bVisible) {
        //状态未变化，避免重复设置Alpha
        return;
    }
    m_bLineVisible = bVisible;
    //通过控件的Alpha控制显示与隐藏，不修改背景色，避免把主题颜色变量解析成固定颜色
    m_pLine->SetAlpha(bVisible ? 255 : 0);
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
    m_rcSelected.cx = szCorner.cx;
    m_rcSelected.cy = szCorner.cy;
}

UiSize TabCtrlItem::GetSelectedRoundCorner() const
{
    UiSize szCorner;
    szCorner.cx = m_rcSelected.cx;
    szCorner.cy = m_rcSelected.cy;
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
    m_rcHovered.cx = szCorner.cx;
    m_rcHovered.cy = szCorner.cy;
}

UiSize TabCtrlItem::GetHoveredRoundCorner() const
{
    UiSize szCorner;
    szCorner.cx = m_rcHovered.cx;
    szCorner.cy = m_rcHovered.cy;
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
    m_hoveredPadding.left = rcPadding.left;
    m_hoveredPadding.top = rcPadding.top;
    m_hoveredPadding.right = rcPadding.right;
    m_hoveredPadding.bottom = rcPadding.bottom;
}

UiPadding TabCtrlItem::GetHoveredPadding() const
{
    UiPadding rcPadding;
    rcPadding.left = m_hoveredPadding.left;
    rcPadding.top = m_hoveredPadding.top;
    rcPadding.right = m_hoveredPadding.right;
    rcPadding.bottom = m_hoveredPadding.bottom;
    if (rcPadding.left < 0) {
        rcPadding.top = 0;
        rcPadding.left = Dpi().GetScaleInt(3);
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
            if (IsAutoHideCloseButton()) {
                //自动隐藏：未选中时隐藏；选中时保持显示
                m_pCloseBtn->SetFadeVisible(IsSelected());
            }
            else {
                m_pCloseBtn->SetVisible(true);
            }
        }
    }
    return bRet;
}

void TabCtrlItem::OnPrivateSetSelected()
{
    UpdateCloseButtonVisible();
    //处理分割线的状态
    AdjustItemLineStatus();
    //更新标题文字的颜色
    UpdateTitleTextColor();
}

bool TabCtrlItem::ButtonDown(const EventArgs& msg)
{
    TabCtrl* pTabCtrl = GetTabCtrl();
    if ((pTabCtrl != nullptr) && (msg.eventType == kEventMouseButtonDown)) {
        //标签项覆盖了标签栏的整个区域，这里把落在溢出箭头/新建标签按钮上的点击转发给TabCtrl
        if (pTabCtrl->HandleTabStripClick(msg.ptMouse)) {
            return true;
        }
    }
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

bool TabCtrlItem::MButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::MButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return bRet;
    }
    TabCtrl* pTabCtrl = GetTabCtrl();
    if ((pTabCtrl != nullptr) && pTabCtrl->IsEnableMiddleClickClose() &&
        m_bClosable && !IsFixed()) {
        //鼠标中键：发送关闭请求事件，不删除标签（删除由业务层完成）
        pTabCtrl->NotifyTabCloseRequest(this);
        return true;
    }
    return bRet;
}

bool TabCtrlItem::MouseWheel(const EventArgs& msg)
{
    TabCtrl* pTabCtrl = GetTabCtrl();
    if (pTabCtrl != nullptr) {
        //标签项覆盖了标签栏的整个区域，滚轮消息需要转发给TabCtrl处理
        EventArgs newMsg = msg;
        pTabCtrl->SendEvent(kEventMouseWheel, newMsg);
        return true;
    }
    return BaseClass::MouseWheel(msg);
}

void TabCtrlItem::PaintStateColors(IRender* pRender)
{
    //绘制标签页的形状（通过颜色体现）
    if (IsSelected()) {
        TabCtrl* pTabCtrl = GetTabCtrl();
        if ((pTabCtrl == nullptr) || pTabCtrl->IsSelectedTabItemShowBg()) {
            PaintTabItemSelected(pRender);
        }
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
    if (rc.Height() < roundSize.cy * 2) {
        roundSize.cy = rc.Height() / 3;
    }

    DString color = GetSelectStateColor(ControlStateType::kControlStateNormal);
    if (color.empty()) {
        return;
    }

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    if (pRenderFactory != nullptr) {
        //选中背景的画刷缓存：仅在颜色变化时重建
        if ((m_spSelectedBgBrush == nullptr) || (m_selectedBgBrushColor != color)) {
            m_selectedBgBrushColor = color;
            m_spSelectedBgBrush.reset(pRenderFactory->CreateBrush(GetUiColor(color)));
        }
        if (m_spSelectedBgBrush != nullptr) {
            //轮廓边线相关参数
            float fOutlineWidth = 0.0f;
            UiColor outlineColor;
            DString strOutlineColor;
            TabCtrl* pTabCtrl = GetTabCtrl();
            if (pTabCtrl != nullptr) {
                fOutlineWidth = pTabCtrl->GetSelectedTabItemOutlineWidth();
                strOutlineColor = pTabCtrl->GetSelectedTabItemOutlineColor();

                fOutlineWidth = pTabCtrl->Dpi().GetScaleFloat(fOutlineWidth);
                if (!strOutlineColor.empty()) {
                    outlineColor = GetUiColor(strOutlineColor);
                }
            }
            //轮廓边线的画笔缓存：仅在颜色或宽度变化时重建
            IPen* pOutlinePen = nullptr;
            if ((fOutlineWidth > 0.01f) && !outlineColor.IsEmpty()) {
                if ((m_spSelectedOutlinePen == nullptr) ||
                    (m_fSelectedOutlinePenWidth != fOutlineWidth) ||
                    (m_selectedOutlinePenColor != strOutlineColor)) {
                    m_fSelectedOutlinePenWidth = fOutlineWidth;
                    m_selectedOutlinePenColor = strOutlineColor;
                    m_spSelectedOutlinePen.reset(pRenderFactory->CreatePen(outlineColor, fOutlineWidth));
                }
                pOutlinePen = m_spSelectedOutlinePen.get();
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
            //选中标签的背景路径缓存：仅在矩形或圆角变化时重建
            if ((m_spSelectedPath == nullptr) ||
                !m_bSelectedPathValid ||
                !m_rcSelectedPath.Equals(rc) ||
                (m_szSelectedPathRound.cx != roundSize.cx) ||
                (m_szSelectedPathRound.cy != roundSize.cy)) {
                if (m_spSelectedPath == nullptr) {
                    m_spSelectedPath.reset(pRenderFactory->CreatePath());
                }
                if (m_spSelectedPath != nullptr) {
                    m_spSelectedPath->Reset();
                    AddTabItemPath(m_spSelectedPath.get(), rc, roundSize);
                    m_rcSelectedPath = rc;
                    m_szSelectedPathRound = roundSize;
                    m_bSelectedPathValid = true;
                }
            }
            if (m_spSelectedPath == nullptr) {
                return;
            }
            IPath* pPath = m_spSelectedPath.get();
            if (pOutlinePen != nullptr) {
                //绘制选择标签的轮廓边线
                pRender->DrawPath(pPath, pOutlinePen);
                UiRect rcSelectItem = rc;
                if (pTabCtrl != nullptr) {
                    UiRect rcTabCtrl = pTabCtrl->GetRect();
                    if (bVertical) {
                        //垂直标签栏：在右侧（面向内容区的一侧）绘制竖线，连接被选择标签的上下边界
                        rcSelectItem.top = std::max(rcSelectItem.top, rcTabCtrl.top);
                        rcSelectItem.bottom = std::min(rcSelectItem.bottom, rcTabCtrl.bottom);
                        pRender->DrawLine(UiPointF(rcSelectItem.right, rcSelectItem.top),
                                          UiPointF(rcSelectItem.right, rcSelectItem.bottom),
                                          pOutlinePen);
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
                                          pOutlinePen);
                    }
                }
                else {
                    //未关联到 TabCtrl 时（理论上不会发生），按水平处理
                    pRender->DrawLine(UiPointF(rcSelectItem.left, rcSelectItem.bottom),
                                      UiPointF(rcSelectItem.right, rcSelectItem.bottom),
                                      pOutlinePen);
                }
            }
            //绘制选择标签的样式（从路径中绘制）
            pRender->FillPath(pPath, m_spSelectedBgBrush.get());
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
    //悬停背景颜色缓存：仅在颜色字符串变化时重新解析
    if (m_hoveredColorKey != color) {
        m_hoveredColorKey = color;
        m_hoveredColorValue = GetUiColor(color);
    }
    uint8_t uFade = 255;
    if (IsAnimationPlayerPlaying(AnimationType::kAnimationHovered)) {
        uFade = GetHoveredAlpha();
    }
    pRender->FillRoundRect(UiRectF::MakeFromRect(rc), (float)roundSize.cx, (float)roundSize.cy, m_hoveredColorValue, uFade);
}

void TabCtrl::PaintForeColor(IRender* pRender)
{
    BaseClass::PaintForeColor(pRender);
    //在子控件绘制完成后绘制指示条，避免被标签项自身的绘制区域裁剪
    PaintSelectedTabIndicator(pRender);
    //拖动排序时的插入位置提示
    PaintDragInsertIndicator(pRender);
    //标签溢出时的滚动箭头按钮
    PaintOverflowButtons(pRender);
    //"新建标签"按钮
    PaintNewTabButton(pRender);
}

void TabCtrl::PaintSelectedTabIndicator(IRender* pRender)
{
    if (pRender == nullptr) {
        return;
    }
    const DString& indicatorColor = GetSelectedTabItemIndicatorColor();
    const float fIndicatorWidth = GetSelectedTabItemIndicatorWidth();
    const float fIndicatorHeight = GetSelectedTabItemIndicatorHeight();
    const bool bAutoWidth = IsSelectedTabItemIndicatorAutoWidth();
    if (indicatorColor.empty() || (!bAutoWidth && (fIndicatorWidth <= 0.01f)) || (fIndicatorHeight <= 0.01f)) {
        //未配置指示条，不绘制（保持原有选中效果）
        return;
    }
    //颜色解析结果缓存：仅在颜色字符串变化时重新解析
    if (m_indicatorColorKey != indicatorColor) {
        m_indicatorColorKey = indicatorColor;
        m_indicatorColorValue = GetUiColor(indicatorColor);
    }
    if (m_indicatorColorValue.IsEmpty()) {
        return;
    }

    //获取当前选中标签项的矩形
    std::vector<size_t> selectedIndexs;
    GetSelectedItems(selectedIndexs);
    if (selectedIndexs.empty()) {
        return;
    }
    Control* pSelectedItem = GetItemAt(selectedIndexs[0]);
    if (pSelectedItem == nullptr) {
        return;
    }
    //优先以标签文字区域为基准居中，保证指示条与文字对齐
    UiRect rcItem = pSelectedItem->GetRect();
    TabCtrlItem* pTabItem = dynamic_cast<TabCtrlItem*>(pSelectedItem);
    if (pTabItem != nullptr) {
        Control* pTitleLabel = pTabItem->GetTextLabel();
        if ((pTitleLabel != nullptr) && !pTitleLabel->GetRect().IsEmpty()) {
            rcItem = pTitleLabel->GetRect();
        }
    }
    const UiRect rcTabCtrl = GetRect();
    if (rcItem.IsEmpty() || rcTabCtrl.IsEmpty()) {
        return;
    }

    //按DPI缩放指示条的宽度和高度
    const DpiManager& dpi = Dpi();
    int32_t nIndicatorWidth = 0;
    if (bAutoWidth) {
        //宽度自动跟随标签标题文字
        nIndicatorWidth = CalcAutoIndicatorLength(pRender, pSelectedItem, rcItem);
    }
    else {
        nIndicatorWidth = (int32_t)std::round(dpi.GetScaleFloat(fIndicatorWidth));
    }
    int32_t nIndicatorHeight = (int32_t)std::round(dpi.GetScaleFloat(fIndicatorHeight));
    if ((nIndicatorWidth <= 0) || (nIndicatorHeight <= 0)) {
        return;
    }

    UiRect rcIndicator;
    if (GetTabCtrlOrientation() == TabCtrlOrientation::kVertical) {
        //垂直标签栏：指示条位于标签栏面向内容区的一侧（右侧），沿垂直方向居中于当前标签
        int32_t nCenterY = (rcItem.top + rcItem.bottom) / 2;
        rcIndicator.left = rcTabCtrl.right - nIndicatorHeight;
        rcIndicator.right = rcTabCtrl.right;
        rcIndicator.top = nCenterY - nIndicatorWidth / 2;
        rcIndicator.bottom = rcIndicator.top + nIndicatorWidth;
        //限制在标签栏的上下边界内
        if (rcIndicator.top < rcTabCtrl.top) {
            rcIndicator.top = rcTabCtrl.top;
            rcIndicator.bottom = rcIndicator.top + nIndicatorWidth;
        }
        if (rcIndicator.bottom > rcTabCtrl.bottom) {
            rcIndicator.bottom = rcTabCtrl.bottom;
            rcIndicator.top = rcIndicator.bottom - nIndicatorWidth;
            if (rcIndicator.top < rcTabCtrl.top) {
                rcIndicator.top = rcTabCtrl.top;
            }
        }
    }
    else {
        //水平标签栏：指示条位于标签栏的底部，沿水平方向居中于当前标签
        int32_t nCenterX = (rcItem.left + rcItem.right) / 2;
        rcIndicator.left = nCenterX - nIndicatorWidth / 2;
        rcIndicator.right = rcIndicator.left + nIndicatorWidth;
        rcIndicator.bottom = rcTabCtrl.bottom;
        rcIndicator.top = rcIndicator.bottom - nIndicatorHeight;
        //限制在标签栏的左右边界内
        if (rcIndicator.left < rcTabCtrl.left) {
            rcIndicator.left = rcTabCtrl.left;
            rcIndicator.right = rcIndicator.left + nIndicatorWidth;
        }
        if (rcIndicator.right > rcTabCtrl.right) {
            rcIndicator.right = rcTabCtrl.right;
            rcIndicator.left = rcIndicator.right - nIndicatorWidth;
            if (rcIndicator.left < rcTabCtrl.left) {
                rcIndicator.left = rcTabCtrl.left;
            }
        }
    }
    if ((rcIndicator.Width() <= 0) || (rcIndicator.Height() <= 0)) {
        return;
    }

    //圆角取短边的一半，形成两端为半圆的指示条
    const float fRound = (float)std::min(rcIndicator.Width(), rcIndicator.Height()) / 2.0f;
    pRender->FillRoundRect(UiRectF::MakeFromRect(rcIndicator), fRound, fRound, m_indicatorColorValue, 255);
}

int32_t TabCtrl::CalcAutoIndicatorLength(IRender* pRender, Control* pSelectedItem, const UiRect& rcItem)
{
    int32_t nLength = 0;
    if (GetTabCtrlOrientation() == TabCtrlOrientation::kVertical) {
        //垂直标签栏：指示条沿垂直方向延伸，长度取标题区域的高度
        nLength = rcItem.Height();
    }
    else {
        //水平标签栏：指示条沿水平方向延伸，长度取标题文字的绘制宽度
        TabCtrlItem* pTabItem = dynamic_cast<TabCtrlItem*>(pSelectedItem);
        Label* pLabel = (pTabItem != nullptr) ? pTabItem->GetTextLabel() : nullptr;
        if ((pLabel != nullptr) && (pRender != nullptr)) {
            const DString text = pLabel->GetText();
            const DString fontId = pLabel->GetFontId();
            IFont* pFont = GetIFontById(fontId);
            if (!text.empty() && (pFont != nullptr)) {
                //测量结果缓存：仅在文本或字体变化时重新测量
                if (!m_bIndicatorTextLengthValid ||
                    (m_indicatorTextCache != text) ||
                    (m_indicatorFontIdCache != fontId)) {
                    MeasureStringParam param;
                    param.pFont = pFont;
                    param.uFormat = TEXT_SINGLELINE;
                    m_indicatorTextLength = pRender->MeasureString(text, param).Width();
                    m_indicatorTextCache = text;
                    m_indicatorFontIdCache = fontId;
                    m_bIndicatorTextLengthValid = true;
                }
                nLength = m_indicatorTextLength;
            }
        }
        if (nLength <= 0) {
            //无法测量文本时，使用标题区域的宽度
            nLength = rcItem.Width();
        }
    }
    //限制在标签项自身的范围内
    if (pSelectedItem != nullptr) {
        const UiRect rcSelectedItem = pSelectedItem->GetRect();
        const int32_t nMaxLength = (GetTabCtrlOrientation() == TabCtrlOrientation::kVertical) ?
                                   rcSelectedItem.Height() : rcSelectedItem.Width();
        if ((nMaxLength > 0) && (nLength > nMaxLength)) {
            nLength = nMaxLength;
        }
    }
    if (nLength < 0) {
        nLength = 0;
    }
    return nLength;
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
        //固定标签不可拖动，也不可作为拖放目标；取消固定后恢复用户配置的开关
        SetEnableDragOrder(bFixed ? false : m_bUserDragOrder);
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

void TabCtrlItem::SetClosable(bool bClosable)
{
    if (m_bClosable != bClosable) {
        m_bClosable = bClosable;
        UpdateCloseButtonVisible();
    }
}

bool TabCtrlItem::IsClosable() const
{
    return m_bClosable;
}

void TabCtrlItem::UpdateLineOrientation(bool bVertical)
{
    if (m_pLine == nullptr) {
        //分割线控件尚未创建：记录方向，创建后由 OnInit 应用
        m_bLineVertical = bVertical;
        m_bLineOrientationValid = false;
        return;
    }
    if (m_bLineOrientationValid && (m_bLineVertical == bVertical)) {
        return;
    }
    const bool bWasVertical = m_bLineVertical;
    m_bLineVertical = bVertical;
    m_bLineOrientationValid = true;
    if (!GetLineClass().empty()) {
        //先恢复类中配置的默认布局，再按方向覆盖
        m_pLine->SetClass(GetLineClass());
    }
    if (bVertical) {
        //垂直标签栏：分隔线改为标签底部的横线
        m_pLine->SetAttribute(_T("width"), _T("stretch"));
        m_pLine->SetAttribute(_T("height"), _T("1"));
        m_pLine->SetAttribute(_T("valign"), _T("bottom"));
        m_pLine->SetAttribute(_T("margin"), _T("4,0,4,0"));
    }
    else if (bWasVertical) {
        //从垂直方向恢复为水平方向时，清除垂直模式设置的边距
        m_pLine->SetAttribute(_T("margin"), _T("0,0,0,0"));
    }
    //恢复分割线的显示/隐藏状态
    m_pLine->SetAlpha(m_bLineVisible ? 255 : 0);
}

void TabCtrlItem::UpdateCloseButtonVisible()
{
    if (m_pCloseBtn == nullptr) {
        return;
    }
    if (IsFixed() || !m_bClosable) {
        //固定标签或不可关闭的标签，不显示关闭按钮
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

