#include "ListBox.h"
#include "ListBoxHelper.h"
#include "duilib/Core/ScrollBar.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Box/VirtualHTileLayout.h"
#include "duilib/Box/VirtualVTileLayout.h"

namespace ui 
{

//多选的时候，是否显示选择背景色: 0 - 默认规则; 1 - 显示背景色; 2: 不显示背景色
enum ePaintSelectedColors
{
    PAINT_SELECTED_COLORS_DEFAULT = 0,
    PAINT_SELECTED_COLORS_YES = 1,
    PAINT_SELECTED_COLORS_NO = 2
};

ListBox::ListBox(Window* pWindow, Layout* pLayout) :
    ScrollBox(pWindow, pLayout),
    m_iCurSel(Box::InvalidIndex),
    m_nLastNoShiftItem(0),
    m_pCompareFunc(nullptr),
    m_pCompareContext(nullptr),
    m_uPaintSelectedColors(PAINT_SELECTED_COLORS_DEFAULT),
    m_bScrollSelect(false),
    m_bSelectNextWhenActiveRemoved(false),
    m_bMultiSelect(false),
    m_bSelectLikeListCtrl(false),
    m_bSelectNoneWhenClickBlank(true)
{
}

ListBox::~ListBox()
{
}

DString ListBox::GetType() const { return _T("ListBox"); }

void ListBox::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("multi_select")) {
        SetMultiSelect(strValue == _T("true"));
    }
    else if (strName == _T("paint_selected_colors")) {
        if (strValue == _T("true")) {
            m_uPaintSelectedColors = PAINT_SELECTED_COLORS_YES;
        }
        else {
            m_uPaintSelectedColors = PAINT_SELECTED_COLORS_NO;
        }
    }
    else if ((strName == _T("scroll_select")) || (strName == _T("scrollselect"))) {
        SetScrollSelect(strValue == _T("true"));
    }
    else if (strName == _T("select_next_when_active_removed")) {
        SetSelectNextWhenActiveRemoved(strValue == _T("true"));
    }
    else if (strName == _T("frame_selection")) {
        SetEnableFrameSelection(strValue == _T("true"));
    }
    else if (strName == _T("frame_selection_color")) {
        SetFrameSelectionColor(strValue);
    }
    else if (strName == _T("frame_selection_alpha")) {
        SetframeSelectionAlpha((uint8_t)StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("frame_selection_border_size")) {
        SetFrameSelectionBorderSize(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("frame_selection_border_color")) {
        SetFrameSelectionBorderColor(strValue);
    }
    else if (strName == _T("select_none_when_click_blank")) {
        SetSelectNoneWhenClickBlank(strValue == _T("true"));
    }
    else if (strName == _T("select_like_list_ctrl")) {
        SetSelectLikeListCtrl(strValue == _T("true"));
    }
    else {
        ScrollBox::SetAttribute(strName, strValue);
    }
}

void ListBox::SetEnableFrameSelection(bool bEnable)
{
    if (m_pHelper == nullptr) {
        m_pHelper = std::make_unique<ListBoxHelper>(this);
    }
    m_pHelper->SetEnableFrameSelection(bEnable);
}

bool ListBox::IsEnableFrameSelection() const
{
    if (m_pHelper != nullptr) {
        return m_pHelper->IsEnableFrameSelection();
    }
    return false;
}

void ListBox::SetFrameSelectionColor(const DString& frameSelectionColor)
{
    if (m_pHelper == nullptr) {
        m_pHelper = std::make_unique<ListBoxHelper>(this);
    }
    m_pHelper->SetFrameSelectionColor(frameSelectionColor);
}

DString ListBox::GetFrameSelectionColor() const
{
    if (m_pHelper != nullptr) {
        return m_pHelper->GetFrameSelectionColor();
    }
    return DString();
}

void ListBox::SetframeSelectionAlpha(uint8_t frameSelectionAlpha)
{
    if (m_pHelper == nullptr) {
        m_pHelper = std::make_unique<ListBoxHelper>(this);
    }
    m_pHelper->SetframeSelectionAlpha(frameSelectionAlpha);
}

uint8_t ListBox::GetFrameSelectionAlpha() const
{
    if (m_pHelper != nullptr) {
        return m_pHelper->GetFrameSelectionAlpha();
    }
    return 255;
}

void ListBox::SetFrameSelectionBorderColor(const DString& frameSelectionBorderColor)
{
    if (m_pHelper == nullptr) {
        m_pHelper = std::make_unique<ListBoxHelper>(this);
    }
    m_pHelper->SetFrameSelectionBorderColor(frameSelectionBorderColor);
}

DString ListBox::GetFrameSelectionBorderColor() const
{
    if (m_pHelper != nullptr) {
        return m_pHelper->GetFrameSelectionBorderColor();
    }
    return DString();
}

void ListBox::SetFrameSelectionBorderSize(int32_t nBorderSize)
{
    if (m_pHelper == nullptr) {
        m_pHelper = std::make_unique<ListBoxHelper>(this);
    }
    m_pHelper->SetFrameSelectionBorderSize(nBorderSize);
}

int32_t ListBox::GetFrameSelectionBorderSize() const
{
    if (m_pHelper != nullptr) {
        return m_pHelper->GetFrameSelectionBorderSize();
    }
    return 0;
}

void ListBox::SetNormalItemTop(int32_t nNormalItemTop)
{
    if (m_pHelper == nullptr) {
        m_pHelper = std::make_unique<ListBoxHelper>(this);
    }
    m_pHelper->SetNormalItemTop(nNormalItemTop);
}

int32_t ListBox::GetNormalItemTop() const
{
    if (m_pHelper != nullptr) {
        return m_pHelper->GetNormalItemTop();
    }
    return -1;
}

void ListBox::SetSelectNoneWhenClickBlank(bool bSelectNoneWhenClickBlank)
{
    m_bSelectNoneWhenClickBlank = bSelectNoneWhenClickBlank;
}

bool ListBox::IsSelectNoneWhenClickBlank() const
{
    return m_bSelectNoneWhenClickBlank;
}

void ListBox::SetSelectLikeListCtrl(bool bSelectLikeListCtrl)
{
    m_bSelectLikeListCtrl = bSelectLikeListCtrl;
}

bool ListBox::IsSelectLikeListCtrl() const
{
    return m_bSelectLikeListCtrl;
}

void ListBox::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            BaseClass::HandleEvent(msg);
        }
        return;
    }
    bool bHandled = false;
    if (msg.eventType == kEventKeyDown) {
        if (IsSelectLikeListCtrl()) {
            //优先使用ListCtrl风格的快捷逻辑
            bHandled = OnListCtrlKeyDown(msg);
        }        
        if (!bHandled) {
            bHandled = OnListBoxKeyDown(msg);
        }        
    }
    else if (msg.eventType == kEventMouseWheel) {
        bHandled = OnListBoxMouseWheel(msg);
    }    
    if(!bHandled) {
        BaseClass::HandleEvent(msg);
    }
}

bool ListBox::OnListBoxKeyDown(const EventArgs& msg)
{
    ASSERT(msg.eventType == kEventKeyDown);
    bool bHandled = false;
    bool bArrowKeyDown = (msg.eventType == kEventKeyDown) &&
                          ((msg.vkCode == kVK_UP)    || (msg.vkCode == kVK_DOWN) ||
                           (msg.vkCode == kVK_LEFT)  || (msg.vkCode == kVK_RIGHT) ||
                           (msg.vkCode == kVK_PRIOR) || (msg.vkCode == kVK_NEXT) ||
                           (msg.vkCode == kVK_HOME)  || (msg.vkCode == kVK_END));
    if (!bArrowKeyDown) {
        return bHandled;
    }

    bool bHasSelectItem = GetCurSel() < GetItemCount(); //是否有单选的选择项
    if (!IsMultiSelect() && (GetItemCount() > 0) && !bHasSelectItem) {
        //当前界面中无选中项，需要查询子类（虚表实现）中是否有选中项
        size_t nDestItemIndex = Box::InvalidIndex;
        if (OnFindSelectable(GetCurSel(), SelectableMode::kSelect, 1, nDestItemIndex)) {
            bHasSelectItem = true;
            ASSERT(GetCurSel() == nDestItemIndex);
        }
    }
    if (IsMultiSelect() || (GetItemCount() == 0) || !bHasSelectItem) {
        //在无数据、支持多选、无选中项的情况下，不支持单选快捷键逻辑，但支持HOME和END键的响应(滚动)
        if (msg.vkCode == kVK_HOME) {
            if (IsHorizontalScrollBar()) {
                HomeLeft();
            }
            else {
                HomeUp();
            }
            bHandled = true;
        }
        else if (msg.vkCode == kVK_END) {
            if (IsHorizontalScrollBar()) {
                EndRight();
            }
            else {
                EndDown(false, false);
            }
            bHandled = true;
        }
        return bHandled;
    }

    //单选、有数据、有选中项的情况
    bHandled = true;
    switch (msg.vkCode) {
    case kVK_UP:
        if (IsHorizontalScrollBar()) {
            //横向滚动条，向上1条
            SelectItemPrevious(true, true);
        }
        else {
            //不是横向滚动条，向上1行
            size_t nColumns = 0;
            size_t nRows = 0;
            GetDisplayItemCount(false, nColumns, nRows);
            if ((m_iCurSel < GetItemCount()) && IsSelectableItem(m_iCurSel) &&
                (GetItemCountBefore(m_iCurSel) >= nColumns)) {
                //可以向上滚动1行
                SelectItemCountN(true, true, false, nColumns);
            }
        }        
        break;
    case kVK_DOWN:
        if (IsHorizontalScrollBar()) {
            //横向滚动条，向下1条
            SelectItemNext(true, true);
        }
        else {
            //不是横向滚动条，向下1行
            size_t nColumns = 0;
            size_t nRows = 0;
            GetDisplayItemCount(false, nColumns, nRows);
            if ((m_iCurSel < GetItemCount()) && IsSelectableItem(m_iCurSel) &&
                (GetItemCountAfter(m_iCurSel) >= nColumns)) {
                SelectItemCountN(true, true, true, nColumns);
            }
            else {
                PageDown();
                SelectItem(m_iCurSel, true, false);
            }
        }
        break;
    case kVK_LEFT:
        if (IsHorizontalScrollBar()) {
            //横向滚动条，向上1列
            size_t nColumns = 0;
            size_t nRows = 0;
            GetDisplayItemCount(false, nColumns, nRows);
            if ((m_iCurSel < GetItemCount()) && IsSelectableItem(m_iCurSel) &&
                (GetItemCountBefore(m_iCurSel) >= nRows)) {
                SelectItemCountN(true, true, false, nRows);
            }
        }
        else {
            //不是横向滚动条，向上1条
            SelectItemPrevious(true, true);
        }
        break;
    case kVK_RIGHT:
        if (IsHorizontalScrollBar()) {
            //横向滚动条，向下1行
            size_t nColumns = 0;
            size_t nRows = 0;
            GetDisplayItemCount(false, nColumns, nRows);
            if ((m_iCurSel < GetItemCount()) && IsSelectableItem(m_iCurSel) &&
                (GetItemCountAfter(m_iCurSel) >= nRows)) {
                SelectItemCountN(true, true, true, nRows);
            }
            else {
                PageRight();
                SelectItem(m_iCurSel, true, false);
            }
        }
        else {
            //不是横向滚动条，向下1条
            SelectItemNext(true, true);
        }
        break;
    case kVK_PRIOR:
        SelectItemPage(true, true, false, 0);
        break;
    case kVK_NEXT:
        SelectItemPage(true, true, true, 0);
        break;
    case kVK_HOME:
        SelectItemHome(true, true);
        break;
    case kVK_END:
        SelectItemEnd(true, true);
        break;
    default:
        bHandled = false;
        break;
    }
    if (!IsKeyDown(msg, ModifierKey::kShift)) {
        SetLastNoShiftItem(GetCurSel());
    }
    return bHandled;
}

int32_t ListBox::CalcRows() const
{
    HLayout* pHLayout = dynamic_cast<HLayout*>(GetLayout());
    if (pHLayout != nullptr) {
        //此布局固定1行
        return 1;
    }
    HTileLayout* pHTileLayout = dynamic_cast<HTileLayout*>(GetLayout());
    return CalcHTileRows(pHTileLayout);
}

int32_t ListBox::CalcColumns() const
{
    VLayout* pVLayout = dynamic_cast<VLayout*>(GetLayout());
    if (pVLayout != nullptr) {
        //此布局固定1列
        return 1;
    }
    VTileLayout* pVTileLayout = dynamic_cast<VTileLayout*>(GetLayout());
    return CalcVTileColumns(pVTileLayout);
}

bool ListBox::OnListCtrlKeyDown(const EventArgs& msg)
{
    //该函数只实现非虚表情况下的快捷键逻辑，虚表模式下的相应逻辑在子类中实现
    ASSERT(msg.eventType == kEventKeyDown);
    bool bHandled = false;
    bool bCtrlADown = (msg.eventType == kEventKeyDown) && ((msg.vkCode == _T('A')) || (msg.vkCode == _T('a')));
    if (bCtrlADown) {
        //Ctrl + A 全选操作
        bHandled = true;
        bool bRet = SetSelectAll();
        if (bRet) {
            OnSelectStatusChanged();
            SendEvent(kEventSelChange);
        }
        return bHandled;
    }

    //方向键操作
    bool bArrowKeyDown = (msg.eventType == kEventKeyDown) &&
                         ((msg.vkCode == kVK_UP) || (msg.vkCode == kVK_DOWN) ||
                          (msg.vkCode == kVK_LEFT) || (msg.vkCode == kVK_RIGHT) ||
                          (msg.vkCode == kVK_PRIOR) || (msg.vkCode == kVK_NEXT) ||
                          (msg.vkCode == kVK_HOME) || (msg.vkCode == kVK_END));
    const size_t nItemCount = GetItemCount();
    if (!bArrowKeyDown || !IsMultiSelect() || (nItemCount == 0)) {
        //在方向键按下消息、无数据、不支持多选的情况下，走默认处理流程
        return bHandled;
    }

    bool bShiftDown = Keyboard::IsKeyDown(kVK_SHIFT);
    bool bControlDown = Keyboard::IsKeyDown(kVK_CONTROL);
    bool bAltDown = Keyboard::IsKeyDown(kVK_MENU);

    if (bAltDown || bControlDown) {
        //如果按住Ctrl键 或者 Alt键走默认流程
        return bHandled;
    }

    // 以下流程处理方向键操作
    // 处理多选情况下的方向键操作，基本与单选流程相似，多选的情况下GetCurSel()值不一定正确，需要校准
   // size_t nCurSel = GetCurSel();
    const bool bForward = (msg.vkCode == kVK_DOWN) || (msg.vkCode == kVK_RIGHT) ||
                          (msg.vkCode == kVK_NEXT) || (msg.vkCode == kVK_END);
    size_t nIndexCurSel = Box::InvalidIndex;
    if (bForward) {
        //查找当前视图内选择区域内的最后一个可选择的项目，作为起始点
        for (int32_t index = (int32_t)nItemCount - 1; index >= 0; --index) {
            Control* pControl = GetItemAt(index);
            if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsSelectableType()) {
                continue;
            }
            IListBoxItem* pItem = dynamic_cast<IListBoxItem*>(pControl);
            if ((pItem != nullptr) && pItem->IsSelected()) {
                nIndexCurSel = index;
                break;
            }
        }
    }
    else {
        //查找当前视图内选择区域内的第一个可选择的项目，作为起始点
        for (size_t index = 0; index < nItemCount; ++index) {
            Control* pControl = GetItemAt(index);
            if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsSelectableType()) {
                continue;
            }
            IListBoxItem* pItem = dynamic_cast<IListBoxItem*>(pControl);
            if ((pItem != nullptr) && pItem->IsSelected()) {
                nIndexCurSel = index;
                break;
            }
        }
    }
    if (nIndexCurSel >= nItemCount) {
        if (bForward) {
            nIndexCurSel = 0;
        }
        else {
            nIndexCurSel = nItemCount - 1;
        }
    }
    
    if (nIndexCurSel < nItemCount) {
        //匹配可选择项
        nIndexCurSel = FindSelectable(nIndexCurSel, bForward);
    }
    if (nIndexCurSel >= nItemCount) {
        //没有有效的数据选择项
        return bHandled;
    }

    const int32_t nRows = CalcRows();
    const int32_t nColumns = CalcColumns();

    size_t nIndexEnsureVisible = Box::InvalidIndex; //需要保证可见的元素
    size_t nIndexEnd = Box::InvalidIndex;
    //实现Shift键 + 方向键的选择逻辑
    switch (msg.vkCode) {
    case kVK_UP:
        if (IsHorizontalLayout()) {
            //横向布局
            if (nIndexCurSel >= 1) {
                nIndexEnd = nIndexCurSel - 1;
            }
            else {
                nIndexEnsureVisible = 0;
            }
        }
        else {
            //纵向布局
            if ((int32_t)nIndexCurSel >= nColumns) {
                nIndexEnd = nIndexCurSel - nColumns;
            }
            else {
                nIndexEnsureVisible = 0;
            }
        }
        break;
    case kVK_DOWN:
        if (IsHorizontalLayout()) {
            //横向布局
            if ((nIndexCurSel + 1) < nItemCount) {
                nIndexEnd = nIndexCurSel + 1;
            }
            else {
                nIndexEnsureVisible = nItemCount - 1;
            }
        }
        else {
            //纵向布局
            if ((nIndexCurSel + nColumns) < nItemCount) {
                nIndexEnd = nIndexCurSel + nColumns;
            }
            else {
                nIndexEnsureVisible = nItemCount - 1;
            }
        }
        break;
    case kVK_LEFT:
        if (IsHorizontalLayout()) {
            //横向布局
            if ((int32_t)nIndexCurSel >= nRows) {
                nIndexEnd = nIndexCurSel - nRows;
            }
            else {
                nIndexEnsureVisible = 0;
            }
        }
        else {
            //纵向布局
            if (nColumns <= 1) {
                //只有1列的时候，按滚动处理
                LineLeft();
                bHandled = true;
            }
            else {
                if (nIndexCurSel >= 1) {
                    nIndexEnd = nIndexCurSel - 1;
                }
                else {
                    nIndexEnsureVisible = 0;
                }
            }
        }
        break;
    case kVK_RIGHT:
        if (IsHorizontalLayout()) {
            //横向布局
            if ((nIndexCurSel + nRows) < nItemCount) {
                nIndexEnd = nIndexCurSel + nRows;
            }
            else {
                nIndexEnsureVisible = nItemCount - 1;
            }
        }
        else {
            if (nColumns <= 1) {
                //只有1列的时候，按滚动处理
                LineRight();
                bHandled = true;
            }
            else {
                //纵向布局
                if ((nIndexCurSel + 1) < nItemCount) {
                    nIndexEnd = nIndexCurSel + 1;
                }
                else {
                    nIndexEnsureVisible = nItemCount - 1;
                }
            }
        }
        break;
    case kVK_PRIOR:
    {
        size_t nShowColumns = 0;
        size_t nShowRows = 0;
        if (IsHorizontalLayout()) {
            //横向布局
            GetDisplayItemCount(true, nShowColumns, nShowRows);
        }
        else {
            //纵向布局            
            GetDisplayItemCount(false, nShowColumns, nShowRows);
        }
        size_t nScrollCount = nShowColumns * nShowRows;
        if (nIndexCurSel >= nScrollCount) {
            nIndexEnd = nIndexCurSel - nScrollCount;
        }
        else {
            if (IsHorizontalLayout()) {
                for (int32_t nColumn = (int32_t)nShowColumns - 1; nColumn >= 0; --nColumn) {
                    nScrollCount = (size_t)nColumn * nShowRows;
                    if (nIndexCurSel >= nScrollCount) {
                        //跳转到第一列，同行的位置
                        nIndexEnd = nIndexCurSel - nScrollCount;
                        break;
                    }
                }
            }
            else {
                for (int32_t nRow = (int32_t)nShowRows - 1; nRow >= 0; --nRow) {
                    nScrollCount = nShowColumns * (size_t)nRow;
                    if (nIndexCurSel >= nScrollCount) {
                        //跳转到第一行，同列的位置
                        nIndexEnd = nIndexCurSel - nScrollCount;
                        break;
                    }
                }
            }
        }
    }
    break;
    case kVK_NEXT:
    {
        size_t nShowColumns = 0;
        size_t nShowRows = 0;
        if (IsHorizontalLayout()) {
            //横向布局
            GetDisplayItemCount(true, nShowColumns, nShowRows);
        }
        else {
            //纵向布局            
            GetDisplayItemCount(false, nShowColumns, nShowRows);
        }
        size_t nScrollCount = nShowColumns * nShowRows;
        if ((nIndexCurSel + nScrollCount) < nItemCount) {
            nIndexEnd = nIndexCurSel + nScrollCount;
        }
        else {
            if (IsHorizontalLayout()) {
                for (int32_t nColumn = (int32_t)nShowColumns - 1; nColumn >= 0; --nColumn) {
                    nScrollCount = (size_t)nColumn * nShowRows;
                    if ((nIndexCurSel + nScrollCount) < nItemCount) {
                        //跳转到最后一列，同行的位置
                        nIndexEnd = nIndexCurSel + nScrollCount;
                        nIndexEnsureVisible = nItemCount - 1;
                        break;
                    }
                }
            }
            else {
                for (int32_t nRow = (int32_t)nShowRows - 1; nRow >= 0; --nRow) {
                    nScrollCount = nShowColumns * (size_t)nRow;
                    if ((nIndexCurSel + nScrollCount) < nItemCount) {
                        //跳转到最后一行，同列的位置
                        nIndexEnd = nIndexCurSel + nScrollCount;
                        nIndexEnsureVisible = nItemCount - 1;
                        break;
                    }
                }
            }
        }
    }
    break;
    case kVK_HOME:
        nIndexEnd = 0;
        break;
    case kVK_END:
        nIndexEnd = nItemCount - 1;
        break;
    default:
        break;
    }

    if (nIndexEnd >= nItemCount) {
        if (nIndexEnsureVisible != Box::InvalidIndex) {
            EnsureVisible(nIndexEnsureVisible);
        }
        return bHandled;
    }

    //匹配可选择项
    nIndexEnd = FindSelectable(nIndexEnd, bForward);
    if (nIndexEnd >= nItemCount) {
        return bHandled;
    }

    bHandled = true;
    std::set<size_t> selectedIndexs; //需要选择的列表
    if (bShiftDown) {
        //按住Shift键：选择范围内的所有数据
        size_t nLastNoShiftItem = GetLastNoShiftItem();//起始的元素索引号
        if (nLastNoShiftItem >= nItemCount) {
            nLastNoShiftItem = 0;
        }
        size_t nStartItemIndex = std::min(nLastNoShiftItem, nIndexEnd);
        size_t nEndItemIndex = std::max(nLastNoShiftItem, nIndexEnd);
        for (size_t i = nStartItemIndex; i <= nEndItemIndex; ++i) {
            if (IsSelectableItem(i)) {
                selectedIndexs.insert(i);
            }
        }
    }
    else {
        //没有按住Shift键：只选择最后一个数据
        selectedIndexs.insert(nIndexEnd);
    }

    //选择这个范围内的所有元素
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        Control* pControl = GetItemAt(nItemIndex);
        if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
            continue;
        }
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListItem == nullptr) {
            continue;
        }
        if (selectedIndexs.find(nItemIndex) != selectedIndexs.end()) {
            //选中集合
            if (!pListItem->IsSelected()) {
                //如果原来是非选择状态，更新为选择状态
                pListItem->OptionSelected(true, false);
            }
        }
        else {
            //取消其他
            if (pListItem->IsSelected()) {
                //如果原来是选择状态，更新为非选择状态
                pListItem->OptionSelected(false, false);
            }
        }
    }
    if (nIndexEnsureVisible != Box::InvalidIndex) {
        EnsureVisible(nIndexEnsureVisible);
    }
    else {
        EnsureVisible(nIndexEnd);
    }
    size_t nCurSel = nIndexEnd;
    ASSERT(nCurSel < GetItemCount());
    bool bTriggerEvent = false;
    if (nCurSel < GetItemCount()) {
        SetCurSel(nCurSel);
        SelectItemSingle(nCurSel, true, false);
        if (!bShiftDown) {
            SetLastNoShiftItem(nCurSel);
        }
        bTriggerEvent = true;
        ASSERT(GetItemAt(nCurSel)->IsFocused());
    }
    OnSelectStatusChanged();
    if (bTriggerEvent) {
        SendEvent(kEventSelect, nCurSel, Box::InvalidIndex);
    }
    return bHandled;
}

bool ListBox::OnListBoxMouseWheel(const EventArgs& msg)
{
    ASSERT(msg.eventType == kEventMouseWheel);
    bool bHandled = false;
    if (m_bScrollSelect && (msg.eventType == kEventMouseWheel)) {
        int32_t deltaValue = msg.eventData;
        if (deltaValue != 0) {
            bool bForward = deltaValue > 0 ? false : true;
            SelectItemPage(true, true, bForward, std::abs(deltaValue));
            bHandled = true;
        }
    }
    return bHandled;
}

bool ListBox::SelectItem(size_t iIndex)
{
    return SelectItem(iIndex, true, true);
}

size_t ListBox::SelectItemPrevious(bool bTakeFocus, bool bTriggerEvent)
{
    return SelectItemCountN(bTakeFocus, bTriggerEvent, false, 1);
}

size_t ListBox::SelectItemNext(bool bTakeFocus, bool bTriggerEvent)
{
    return SelectItemCountN(bTakeFocus, bTriggerEvent, true, 1);
}

size_t ListBox::SelectItemPageUp(bool bTakeFocus, bool bTriggerEvent)
{
    return SelectItemPage(bTakeFocus, bTriggerEvent, false, 0);
}

size_t ListBox::SelectItemPageDown(bool bTakeFocus, bool bTriggerEvent)
{
    return SelectItemPage(bTakeFocus, bTriggerEvent, true, 0);
}

size_t ListBox::SelectItemHome(bool bTakeFocus, bool bTriggerEvent)
{
    if (GetItemCount() == 0) {
        return Box::InvalidIndex;
    }
    size_t iIndex = 0;
    size_t nDestItemIndex = Box::InvalidIndex;
    if (OnFindSelectable(m_iCurSel, SelectableMode::kHome, 1, nDestItemIndex)) {
        iIndex = nDestItemIndex;
    }
    size_t itemIndex = FindSelectable(iIndex, true);
    if (Box::IsValidItemIndex(itemIndex)) {        
        SelectItem(itemIndex, false, bTriggerEvent);
        itemIndex = SelectEnsureVisible(itemIndex, bTakeFocus);
        if (bTriggerEvent) {
            SendEvent(kEventSelect, itemIndex, Box::InvalidIndex);
        }
    }
    return itemIndex;
}

size_t ListBox::SelectItemEnd(bool bTakeFocus, bool bTriggerEvent)
{
    if (GetItemCount() == 0) {
        return Box::InvalidIndex;
    }
    size_t iIndex = GetItemCount() - 1;
    size_t nDestItemIndex = Box::InvalidIndex;
    if (OnFindSelectable(m_iCurSel, SelectableMode::kEnd, 1, nDestItemIndex)) {
        iIndex = nDestItemIndex;
    }
    size_t itemIndex = FindSelectable(iIndex, false);
    if (Box::IsValidItemIndex(itemIndex)) {
        SelectItem(itemIndex, false, bTriggerEvent);
        itemIndex = SelectEnsureVisible(itemIndex, bTakeFocus);
        if (bTriggerEvent) {
            SendEvent(kEventSelect, itemIndex, Box::InvalidIndex);
        }
    }
    return itemIndex;
}

size_t ListBox::SelectItemPage(bool bTakeFocus, bool bTriggerEvent, bool bForward, int32_t nDeltaValue)
{
    //Page Up / Page Down 键的翻页逻辑
    size_t itemIndex = Box::InvalidIndex;
    const size_t itemCount = GetItemCount();
    if (itemCount == 0) {
        return itemIndex;
    }

    bool bIsHorizontal = IsHorizontalScrollBar(); //是否为横向滚动条
    if (nDeltaValue == 0) {
        //计算nDeltaValue值
        nDeltaValue = bIsHorizontal ? GetRect().Width() : GetRect().Height();
    }
    if (nDeltaValue == 0) {
        if (m_iCurSel < itemCount) {
            EnsureVisible(m_iCurSel);
        }
        return itemIndex;
    }
    size_t nCountPerPage = 1;
    size_t nColumns = 0;
    size_t nRows = 0;
    size_t nTotalDisplayCount = GetDisplayItemCount(bIsHorizontal, nColumns, nRows);
    if (nTotalDisplayCount < 1) {
        nTotalDisplayCount = 1;
    }
    if (bIsHorizontal) {
        //只有横向滚动条，按横向处理
        nCountPerPage = nTotalDisplayCount * std::abs(nDeltaValue) / GetRect().Width();
        if (nCountPerPage > nRows) {
            nCountPerPage -= nRows; //减掉1行
        }
        if ((m_iCurSel < GetItemCount()) && IsSelectableItem(m_iCurSel)) {
            //如果无法满足翻页，那么停留在最后一列
            while (nCountPerPage > nRows) {
                if (bForward) {
                    if (GetItemCountAfter(m_iCurSel) >= nCountPerPage) {
                        break;
                    }
                }
                else {
                    if (GetItemCountBefore(m_iCurSel) >= nCountPerPage) {
                        break;
                    }
                }
                nCountPerPage -= nRows;
            }
        }
    }
    else {
        //其他情况，按纵向滚动处理
        nCountPerPage = nTotalDisplayCount * std::abs(nDeltaValue) / GetRect().Height();
        if (nCountPerPage > nColumns) {
            nCountPerPage -= nColumns; //减掉1列
        }
        if ((m_iCurSel < GetItemCount()) && IsSelectableItem(m_iCurSel)) {
            //如果无法满足翻页，那么停留在最后一行
            while (nCountPerPage > nColumns) {
                if (bForward) {
                    if (GetItemCountAfter(m_iCurSel) >= nCountPerPage) {
                        break;
                    }
                }
                else {
                    if (GetItemCountBefore(m_iCurSel) >= nCountPerPage) {
                        break;
                    }
                }
                nCountPerPage -= nColumns;
            }
        }
    }
    if (nCountPerPage < 1) {
        nCountPerPage = 1;
    }
    
    itemIndex = Box::InvalidIndex;
    if ((m_iCurSel < GetItemCount()) && IsSelectableItem(m_iCurSel)) {
        if (bForward) {
            if (GetItemCountAfter(m_iCurSel) >= nCountPerPage) {
                itemIndex = SelectItemCountN(bTakeFocus, bTriggerEvent, bForward, nCountPerPage);
            }
        }
        else {
            if (GetItemCountBefore(m_iCurSel) >= nCountPerPage) {
                itemIndex = SelectItemCountN(bTakeFocus, bTriggerEvent, bForward, nCountPerPage);
            }
        }
    }
    if (itemIndex == Box::InvalidIndex) {
        if (IsHorizontalScrollBar()) {
            if (bForward) {
                PageRight();
            }
            else {
                PageLeft();
            }            
        }
        else {
            if (bForward) {
                PageDown();
            }
            else {
                PageUp();
            }
        }
        SelectItem(m_iCurSel, true, false);
    }
    return itemIndex;
}

size_t ListBox::SelectItemCountN(bool bTakeFocus, bool bTriggerEvent, bool bForward, size_t nCount)
{
    if (m_iCurSel >= GetItemCount()) {
        //当前无有效的选择项，无法操作
        return Box::InvalidIndex;
    }
    if (!IsSelectableItem(m_iCurSel)) {
        //如果当前选中项为不可选择项，无法操作
        return Box::InvalidIndex;
    }
    if ((nCount == 0) || (nCount == Box::InvalidIndex)){
        nCount = 1;
    }
    bool bExceedFirst = false; //已经到达第1条
    size_t iIndex = Box::InvalidIndex;
    if (!bForward) {
        //Page Up
        if (m_iCurSel > nCount) {
            iIndex = m_iCurSel - nCount;
        }
        else {
            bExceedFirst = true;
        }
    }
    else {
        //Page Down
        iIndex = m_iCurSel + nCount;
    }
    if (IsSelectableItem(iIndex)) {
        //目标子项存在，直接返回
        size_t itemIndex = iIndex;
        if (itemIndex < GetItemCount()) {
            SelectItem(itemIndex, false, bTriggerEvent);
            itemIndex = SelectEnsureVisible(itemIndex, bTakeFocus);
            if (bTriggerEvent) {
                SendEvent(kEventSelect, itemIndex, Box::InvalidIndex);
            }
        }
        return itemIndex;
    }

    //可能需要预加载数据，如果有预加载行为，m_iCurSel的值可能发生变化
    size_t nDestItemIndex = Box::InvalidIndex;
    SelectableMode mode = bForward ? SelectableMode::kForward : SelectableMode::kBackward;
    if (OnFindSelectable(m_iCurSel, mode, nCount, nDestItemIndex)) {
        iIndex = nDestItemIndex;
        ASSERT(iIndex < GetItemCount());
        if (iIndex >= GetItemCount()) {
            return Box::InvalidIndex;
        }
    }
    else {
        if (bExceedFirst) {
            iIndex = 0;
        }
    }
    const size_t itemCount = GetItemCount();
    if (iIndex >= itemCount) {
        iIndex = itemCount - 1;
    }    
    size_t itemIndex = FindSelectable(iIndex, bForward);
    if (itemIndex < itemCount) {        
        SelectItem(itemIndex, false, bTriggerEvent);
        itemIndex = SelectEnsureVisible(itemIndex, bTakeFocus);
        if (bTriggerEvent) {
            SendEvent(kEventSelect, itemIndex, Box::InvalidIndex);
        }
    }
    return itemIndex;
}

size_t ListBox::SelectEnsureVisible(size_t itemIndex, bool bTakeFocus)
{
    itemIndex = EnsureVisible(itemIndex);
    if (bTakeFocus) {
        Control* pSelectedControl = GetItemAt(itemIndex);
        if ((pSelectedControl != nullptr) && pSelectedControl->IsVisible()) {
            pSelectedControl->SetFocus();
        }
        ASSERT(pSelectedControl != nullptr);
        ASSERT(pSelectedControl->IsVisible());
        ASSERT(GetWindow()->GetFocusControl() == pSelectedControl);
    }
    return itemIndex;
}

bool ListBox::IsSelectableItem(size_t itemIndex) const
{
    bool bSelectable = false;
    Control* pControl = GetItemAt(itemIndex);
    if ((pControl != nullptr) &&
        pControl->IsSelectableType() &&
        pControl->IsVisible() &&
        pControl->IsEnabled()) {
        bSelectable = true;
    }
    return bSelectable;
}

bool ListBox::IsItemSelected(size_t nIndex) const
{
    bool bSelected = false;
    Control* pControl = GetItemAt(nIndex);
    if (pControl != nullptr) {
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
        if ((pListItem != nullptr) && pListItem->IsSelected()) {
            bSelected = true;
        }
    }    
    return bSelected;
}

size_t ListBox::GetDisplayItemCount(bool bIsHorizontal, size_t& nColumns, size_t& nRows) const
{
    nColumns = 1;
    nRows = 1;
    size_t nCount = 1;
    bool bRet = false;
    HTileLayout* pHTileLayout = dynamic_cast<HTileLayout*>(GetLayout());
    if ((pHTileLayout != nullptr) && pHTileLayout->IsFreeLayout()) {
        pHTileLayout = nullptr;
    }
    VTileLayout* pVTileLayout = dynamic_cast<VTileLayout*>(GetLayout());
    if ((pVTileLayout != nullptr) && pVTileLayout->IsFreeLayout()) {
        pVTileLayout = nullptr;
    }
    if (pHTileLayout != nullptr) {
        nRows = CalcHTileRows(pHTileLayout);
        nColumns = CalcHTileColumns(pHTileLayout);
        nCount = nColumns * nRows;
        bRet = true;
    }
    else if (pVTileLayout != nullptr) {        
        nRows = CalcVTileRows(pVTileLayout);
        nColumns = CalcVTileColumns(pVTileLayout);
        nCount = nColumns * nRows;
        bRet = true;
    }
    if(!bRet) {
        std::map<int32_t, int32_t> rows;
        std::map<int32_t, int32_t> columns;        
        UiRect boxRect = GetRect();
        const size_t nItemCount = GetItemCount();
        for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
            Control* pControl = GetItemAt(nItemIndex);
            if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
                continue;
            }

            bool bDisplayItem = false;
            const UiRect& rc = pControl->GetRect();
            if (bIsHorizontal) {
                if ((rc.left >= boxRect.left) &&
                    (rc.right <= boxRect.right)) {
                    if ((rc.top >= boxRect.top) && (rc.top < boxRect.bottom)) {
                        bDisplayItem = true;
                    }
                    else if ((rc.bottom >= boxRect.top) && (rc.top < boxRect.bottom)) {
                        bDisplayItem = true;
                    }
                }
            }
            else {
                if ((rc.top >= boxRect.top) &&
                    (rc.bottom <= boxRect.bottom)) {
                    if ((rc.left >= boxRect.left) && (rc.left < boxRect.right)) {
                        bDisplayItem = true;
                    }
                    else if ((rc.right >= boxRect.left) && (rc.right < boxRect.right)) {
                        bDisplayItem = true;
                    }
                }
            }
            if (bDisplayItem) {
                rows[pControl->GetRect().top] = 0;
                columns[pControl->GetRect().left] = 0;
                ++nCount;
            }
        }
        nColumns = columns.size();
        nRows = rows.size();
        if (nCount >= (nRows * nColumns)) {
            nCount = nRows * nColumns;
        }
    }
    return nCount;
}

int32_t ListBox::CalcHTileRows(HTileLayout* pHTileLayout) const
{
    int32_t nRows = 1;
    if (pHTileLayout == nullptr) {
        return nRows;
    }
    nRows = pHTileLayout->GetRows();
    bool bAutoRows = pHTileLayout->IsAutoCalcRows();
    if (bAutoRows) {
        nRows = 0;
    }
    if (nRows <= 0) {
        UiSize szItem = pHTileLayout->GetItemSize();
        if (szItem.cy <= 0) {
            return nRows;
        }
        int32_t childMarginY = pHTileLayout->GetChildMarginY();
        if (childMarginY < 0) {
            childMarginY = 0;
        }

        UiRect rc = GetRect();
        rc.Deflate(GetControlPadding());
        int32_t totalHeight = rc.Height();
        while (totalHeight > 0) {
            totalHeight -= szItem.cy;
            if (nRows != 0) {
                totalHeight -= childMarginY;
            }
            if (totalHeight >= 0) {
                ++nRows;
            }
        }
    }
    if (nRows <= 0) {
        nRows = 1;
    }
    return nRows;
}

int32_t ListBox::CalcHTileColumns(HTileLayout* pHTileLayout) const
{
    int32_t nColumns = 1;
    if (pHTileLayout == nullptr) {
        return nColumns;
    }
    UiSize szItem = pHTileLayout->GetItemSize();
    if (szItem.cx <= 0) {
        return nColumns;
    }
    int32_t childMarginX = pHTileLayout->GetChildMarginX();
    if (childMarginX < 0) {
        childMarginX = 0;
    }
    nColumns = 0;
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    int32_t totalWidth = rc.Width();
    while (totalWidth > 0) {
        totalWidth -= szItem.cx;
        if (nColumns != 0) {
            totalWidth -= childMarginX;
        }
        if (totalWidth >= 0) {
            ++nColumns;
        }
    }
    if (nColumns <= 0) {
        nColumns = 1;
    }
    return nColumns;
}

int32_t ListBox::CalcVTileColumns(VTileLayout* pVTileLayout) const
{
    int32_t nColumns = 1;
    if (pVTileLayout == nullptr) {
        return nColumns;
    }
    
    nColumns = pVTileLayout->GetColumns();
    bool bAutoColumns = pVTileLayout->IsAutoCalcColumns();
    if (bAutoColumns) {
        nColumns = 0;
    }
    if (nColumns <= 0) {
        UiSize szItem = pVTileLayout->GetItemSize();
        if (szItem.cx <= 0) {
            return nColumns;
        }
        int32_t childMarginX = pVTileLayout->GetChildMarginX();
        if (childMarginX < 0) {
            childMarginX = 0;
        }

        UiRect rc = GetRect();
        rc.Deflate(GetControlPadding());
        int32_t totalWidth = rc.Width();
        while (totalWidth > 0) {
            totalWidth -= szItem.cx;
            if (nColumns != 0) {
                totalWidth -= childMarginX;
            }
            if (totalWidth >= 0) {
                ++nColumns;
            }
        }
    }
    if (nColumns <= 0) {
        nColumns = 1;
    }
    return nColumns;
}

int32_t ListBox::CalcVTileRows(VTileLayout* pVTileLayout) const
{
    int32_t nRows = 1;
    if (pVTileLayout == nullptr) {
        return nRows;
    }
    UiSize szItem = pVTileLayout->GetItemSize();
    if (szItem.cy <= 0) {
        return nRows;
    }
    int32_t childMarginY = pVTileLayout->GetChildMarginY();
    if (childMarginY < 0) {
        childMarginY = 0;
    }
    nRows = 0;
    UiRect rc = GetRect();
    rc.Deflate(GetControlPadding());
    int32_t totalHeight = rc.Height();
    while (totalHeight > 0) {
        totalHeight -= szItem.cy;
        if (nRows != 0) {
            totalHeight -= childMarginY;
        }
        if (totalHeight >= 0) {
            ++nRows;
        }
    }
    if (nRows <= 0) {
        nRows = 1;
    }
    return nRows;
}

void ListBox::SendEventMsg(const EventArgs& msg)
{
    ScrollBox::SendEventMsg(msg);
}

size_t ListBox::GetCurSel() const
{
    return m_iCurSel;
}

void ListBox::SetCurSel(size_t iIndex)
{
    if (Box::IsValidItemIndex(iIndex)) {
        ASSERT(iIndex < GetItemCount());
        if (iIndex > GetItemCount()) {
            return;
        }
    }    
    m_iCurSel = iIndex;
}

bool ListBox::IsScrollSelect() const
{
    return m_bScrollSelect;
}

void ListBox::SetScrollSelect(bool bScrollSelect)
{
    m_bScrollSelect = bScrollSelect;
}

bool ListBox::IsSelectNextWhenActiveRemoved() const
{
    return m_bSelectNextWhenActiveRemoved;
}

void ListBox::SetSelectNextWhenActiveRemoved(bool bSelectNextItem)
{
    m_bSelectNextWhenActiveRemoved = bSelectNextItem;
}

void ListBox::GetSelectedItems(std::vector<size_t>& selectedIndexs) const
{
    selectedIndexs.clear();
    const size_t itemCount = GetItemCount();
    for (size_t iIndex = 0; iIndex < itemCount; ++iIndex) {
        Control* pControl = m_items[iIndex];
        if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
            continue;
        }
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListItem != nullptr) {
            if (pListItem->IsSelected()) {
                selectedIndexs.push_back(iIndex);
            }
        }
    }
}

size_t ListBox::FindSelectable(size_t iIndex, bool bForward) const
{
    return BaseClass::FindSelectable(iIndex, bForward);
}

bool ListBox::OnFindSelectable(size_t /*nCurSel*/, SelectableMode /*mode*/,
                               size_t /*nCount*/, size_t& /*nDestItemIndex*/)
{
    return false;
}

size_t ListBox::GetItemCountBefore(size_t nCurSel)
{
    if (nCurSel < GetItemCount()) {
        return nCurSel;
    }
    else {
        return 0;
    }
}

size_t ListBox::GetItemCountAfter(size_t nCurSel)
{
    size_t nCount = 0;
    if (nCurSel < GetItemCount()) {
        nCount = GetItemCount() - nCurSel - 1;
    }
    return nCount;
}

bool ListBox::SelectItem(size_t iIndex, bool bTakeFocus, bool bTriggerEvent, uint64_t vkFlag)
{
    bool bRet = false;
    if (!IsSelectLikeListCtrl()) {
        //未开启该功能
        if (IsMultiSelect()) {
            //多选
            bRet = SelectItemMulti(iIndex, bTakeFocus, bTriggerEvent);
        }
        else {
            //单选
            bRet = SelectItemSingle(iIndex, bTakeFocus, bTriggerEvent);
        }
        if (bRet) {
            OnSelectStatusChanged();
        }
    }
    else {
        return ListCtrlSelectItem(iIndex, bTakeFocus, bTriggerEvent, vkFlag);
    }
    return bRet;
}

bool ListBox::ListCtrlSelectItem(size_t iIndex, bool bTakeFocus,
                                 bool bTriggerEvent, uint64_t vkFlag)
{
    if (!IsSelectableItem(iIndex)) {
        //iIndex的值无效，或者当前子项不可选择
        return false;
    }

    //事件触发，需要放在函数返回之前，不能放在代码中间
    bool bSelectStatusChanged = false;
    bool bRet = false;
    if (IsMultiSelect()) {
        //多选模式        
        bool bRbuttonDown = vkFlag & kVkRButton;
        bool bShiftDown = vkFlag & kVkShift;
        bool bControlDown = vkFlag & kVkControl;
        if (bShiftDown && bControlDown) {
            //同时按下Shift和Ctrl键，忽略
            bShiftDown = false;
            bControlDown = false;
        }
        if (bRbuttonDown || (!bShiftDown && !bControlDown)) {
            //按右键的时候：如果当前项没选择，按单选逻辑实现，只保留一个选项；
            //            如果已经选择，则保持原选择，所有项选择状态不变（以提供右键菜单，对所选项操作的机会）
            //在没有按下Control键也没有按Shift键：按单选逻辑实现，只保留一个选项            
            if (bRbuttonDown && IsItemSelected(iIndex)) {
                bRet = true;
            }
            else {                             
                //取消其他选择项
                size_t nItemCount = GetItemCount();
                for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
                    if (nItemIndex == iIndex) {
                        continue;
                    }
                    Control* pControl = GetItemAt(nItemIndex);
                    if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
                        continue;
                    }
                    IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
                    if (pListItem == nullptr) {
                        continue;
                    }
                    if (pListItem->IsSelected()) {
                        //如果原来是选择状态，更新为非选择状态
                        pListItem->OptionSelected(false, false);
                    }
                }
                SetLastNoShiftItem(iIndex);
                SetCurSel(iIndex);
                bRet = SelectItemSingle(iIndex, bTakeFocus, false);
                bSelectStatusChanged = true;
            }
        }
        else {
            if (bShiftDown) {
                //按左键: 同时按下了Shift键
                size_t nIndexStart = GetLastNoShiftItem();
                if (nIndexStart >= GetItemCount()) {
                    nIndexStart = 0;
                }
                if (iIndex < GetItemCount()) {
                    std::set<size_t> selectedIndexs;
                    size_t iStart = std::min(nIndexStart, iIndex);
                    size_t iEnd = std::max(nIndexStart, iIndex);
                    for (size_t i = iStart; i <= iEnd; ++i) {
                        if (IsSelectableItem(i)) {
                            selectedIndexs.insert(i);
                        }
                    }
                    size_t nItemCount = GetItemCount();
                    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
                        Control* pControl = GetItemAt(nItemIndex);
                        if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
                            continue;
                        }
                        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
                        if (pListItem == nullptr) {
                            continue;
                        }
                        if (selectedIndexs.find(nItemIndex) != selectedIndexs.end()) {
                            //选中集合
                            if (!pListItem->IsSelected()) {
                                //如果原来是非选择状态，更新为选择状态
                                pListItem->OptionSelected(true, false);
                            }
                        }
                        else {
                            //取消其他
                            if (pListItem->IsSelected()) {
                                //如果原来是选择状态，更新为非选择状态
                                pListItem->OptionSelected(false, false);
                            }
                        }
                    }
                    SetCurSel(iIndex);
                    bRet = SelectItemSingle(iIndex, bTakeFocus, false);
                    bSelectStatusChanged = true;
                }
                else {
                    //未知情况，正常无法走到这里
                    bRet = SelectItemMulti(iIndex, bTakeFocus, false);
                }
            }
            else {
                //按左键: 同时按下了Control键，保持多选
                bRet = SelectItemMulti(iIndex, bTakeFocus, false);
                if (bRet) {
                    SetLastNoShiftItem(iIndex);
                }
            }
        }
    }
    else {
        //单选
        bRet = SelectItemSingle(iIndex, bTakeFocus, false);
    }
    if (bSelectStatusChanged) {
        OnSelectStatusChanged();
    }
    if (bTriggerEvent && bRet) {
        SendEvent(kEventSelect, iIndex, Box::InvalidIndex);
    }
    return bRet;
}

bool ListBox::UnSelectItem(size_t iIndex, bool bTriggerEvent)
{
    bool bHasEvent = false;
    Control* pControl = GetItemAt(iIndex);
    if (pControl != nullptr) {
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
        if ((pListItem != nullptr) && pListItem->IsSelected()) {
            pListItem->OptionSelected(false, bTriggerEvent);
            bHasEvent = true;            
            //仅在状态变化时重绘
            Invalidate();
        }
    }
    if (iIndex == m_iCurSel) {
        m_iCurSel = Box::InvalidIndex;
    }
    if (bTriggerEvent && bHasEvent) {
        //事件触发，需要放在函数返回之前，不能放在代码中间
        SendEvent(kEventUnSelect, iIndex, Box::InvalidIndex);
    }
    return bHasEvent;
}

void ListBox::OnItemSelectedChanged(size_t /*iIndex*/, IListBoxItem* /*pListBoxItem*/)
{
}

void ListBox::OnItemCheckedChanged(size_t /*iIndex*/, IListBoxItem* /*pListBoxItem*/)
{
}

bool ListBox::SelectItemSingle(size_t iIndex, bool bTakeFocus, bool bTriggerEvent)
{
    //单选
    if (iIndex == m_iCurSel) {
        Control* pControl = GetItemAt(iIndex);
        if (pControl == nullptr) {
            m_iCurSel = Box::InvalidIndex;
            return false;
        }        
        //确保可见，然后返回        
        if (bTakeFocus) {
            pControl->SetFocus();
        }
        bool bChanged = false;
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
        if ((pListItem != nullptr) && !pListItem->IsSelected()) {
            bChanged = true;
            pListItem->OptionSelected(true, bTriggerEvent);
        }
        Invalidate();
        if (bChanged && bTriggerEvent) {
            SendEvent(kEventSelect, m_iCurSel, Box::InvalidIndex);
        }
        return bChanged;
    }
    bool hasUnSelectEvent = false;
    const size_t iOldSel = m_iCurSel;
    if (Box::IsValidItemIndex(iOldSel)) {
        //取消旧选择项的选择状态
        Control* pControl = GetItemAt(iOldSel);
        if (pControl != nullptr) {
            IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
            if ((pListItem != nullptr) && pListItem->IsSelected()) {
                pListItem->OptionSelected(false, bTriggerEvent);
                hasUnSelectEvent = true;                
            }
        }
        m_iCurSel = Box::InvalidIndex;
    }
    if (!Box::IsValidItemIndex(iIndex)) {
        Invalidate();
        if (hasUnSelectEvent && bTriggerEvent) {
            SendEvent(kEventUnSelect, iOldSel, Box::InvalidIndex);
        }
        return hasUnSelectEvent;
    }

    Control* pControl = GetItemAt(iIndex);
    if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
        Invalidate();
        if (hasUnSelectEvent && bTriggerEvent) {
            SendEvent(kEventUnSelect, iOldSel, Box::InvalidIndex);
        }
        return hasUnSelectEvent;
    }
    IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
    if (pListItem == nullptr) {
        Invalidate();
        if (hasUnSelectEvent && bTriggerEvent) {
            SendEvent(kEventUnSelect, iOldSel, Box::InvalidIndex);
        }
        return hasUnSelectEvent;
    }
    m_iCurSel = iIndex;
    //设置选择状态
    pListItem->OptionSelected(true, bTriggerEvent);
    pControl = GetItemAt(m_iCurSel);
    if (pControl != nullptr) {        
        if (bTakeFocus) {
            pControl->SetFocus();
        }
    }

    Invalidate();
    if (hasUnSelectEvent && bTriggerEvent) {
        SendEvent(kEventUnSelect, iOldSel, Box::InvalidIndex);
    }
    if (bTriggerEvent) {
        SendEvent(kEventSelect, m_iCurSel, iOldSel);
    }    
    return true;
}

bool ListBox::SelectItemMulti(size_t iIndex, bool bTakeFocus, bool bTriggerEvent)
{
    //多选: m_iCurSel 始终执行最后一个选中项
    size_t iOldSel = m_iCurSel;
    m_iCurSel = Box::InvalidIndex;
    if (!Box::IsValidItemIndex(iIndex)) {
        Invalidate();
        return false;
    }
    Control* pControl = GetItemAt(iIndex);
    if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()){
        Invalidate();
        return false;
    }
    IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
    if (pListItem == nullptr) {
        Invalidate();
        return false;
    }
    
    if (pListItem->IsSelected()) {
        //多选时，再次选择时，按取消选择处理
        pListItem->OptionSelected(false, false);
        Invalidate();
        if (bTriggerEvent) {
            SendEvent(kEventUnSelect, iIndex, Box::InvalidIndex);
        }
    }
    else {
        //如果原来是非选择状态，更新为选择状态
        m_iCurSel = iIndex;
        pListItem->OptionSelected(true, false);        
        if (bTakeFocus) {            
            pControl->SetFocus();
        }
        Invalidate();
        if (bTriggerEvent) {
            SendEvent(kEventSelect, iIndex, iOldSel);
        }
    }    
    return true;
}

void ListBox::EnsureVisible(const UiRect& rcItem,
                            ListBoxVerVisible vVisibleType,
                            ListBoxHorVisible hVisibleType)
{
    UiRect rcNewItem = rcItem;
    UiSize scrollOffset = GetScrollOffset();
    rcNewItem.Offset(-scrollOffset.cx, -scrollOffset.cy);
    UiRect rcList = GetPos();
    rcList.Deflate(GetPadding());

    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar && pVScrollBar->IsValid()) {
        if (IsVScrollBarAtLeft()) {
            ASSERT(pVScrollBar->GetFixedWidth().GetInt32() > 0);
            rcList.left += pVScrollBar->GetFixedWidth().GetInt32();
        }
        else {
            ASSERT(pVScrollBar->GetFixedWidth().GetInt32() > 0);
            rcList.right -= pVScrollBar->GetFixedWidth().GetInt32();
        }
    }

    ScrollBar* pHScrollBar = GetHScrollBar();
    if (pHScrollBar && pHScrollBar->IsValid()) {
        ASSERT(pHScrollBar->GetFixedHeight().GetInt32() > 0);
        rcList.bottom -= pHScrollBar->GetFixedHeight().GetInt32();
    }

    if ((rcNewItem.left >= rcList.left) && (rcNewItem.top >= rcList.top) && 
        (rcNewItem.right <= rcList.right) && (rcNewItem.bottom <= rcList.bottom)) {
        IListBoxItem* listBoxElement = dynamic_cast<IListBoxItem*>(GetParent());
        IListBoxOwner* lisBoxOwner = nullptr;
        if (listBoxElement != nullptr) {
            lisBoxOwner = listBoxElement->GetOwner();
        }
        if (lisBoxOwner != nullptr) {
            lisBoxOwner->EnsureVisible(rcNewItem, vVisibleType, hVisibleType);
        }        
        return;
    }
    //水平滚动条
    int32_t dx = 0;
    if (hVisibleType == ListBoxHorVisible::kVisibleAtCenter) {
        //居中显示
        if (rcNewItem.left < rcList.CenterX()) {
            dx = rcNewItem.left - rcList.CenterX();
        }
        if (rcNewItem.right > rcList.CenterX()) {
            dx = rcNewItem.right - rcList.CenterX();
        }
    }
    else if (hVisibleType == ListBoxHorVisible::kVisibleAtLeft) {
        //靠左显示
        dx = rcNewItem.left - rcList.left;
    }
    else if (hVisibleType == ListBoxHorVisible::kVisibleAtRight) {
        //靠右显示
        dx = rcNewItem.right - rcList.right;
    }
    else {
        if (rcNewItem.left < rcList.left) {
            dx = rcNewItem.left - rcList.left;
        }
        if (rcNewItem.right > rcList.right) {
            dx = rcNewItem.right - rcList.right;
        }
        UiRect rcNewList = rcList;
        rcNewList.top = rcNewItem.top;
        rcNewList.bottom = rcNewItem.bottom;
        if (rcNewItem.ContainsRect(rcNewList)) {
            //子项的区域，已经在可见区域，不调整，避免鼠标点击后左右晃动
            dx = 0;
        }
    }
    //垂直滚动条
    int32_t dy = 0;
    if (vVisibleType == ListBoxVerVisible::kVisibleAtCenter) {
        //居中显示
        if (rcNewItem.top < rcList.CenterY()) {
            dy = rcNewItem.top - rcList.CenterY();
        }
        if (rcNewItem.bottom > rcList.CenterY()) {
            dy = rcNewItem.bottom - rcList.CenterY();
        }
    }
    else if (vVisibleType == ListBoxVerVisible::kVisibleAtTop) {
        //顶部对齐
        dy = rcNewItem.top - rcList.top;
    }
    else if (vVisibleType == ListBoxVerVisible::kVisibleAtBottom) {
        //底部对齐
        dy = rcNewItem.bottom - rcList.bottom;
    }
    else {
        //只要可见即可
        if (rcNewItem.top < rcList.top) {
            dy = rcNewItem.top - rcList.top;
        }
        if (rcNewItem.bottom > rcList.bottom) {
            dy = rcNewItem.bottom - rcList.bottom;
        }
        UiRect rcNewList = rcList;
        rcNewList.left = rcNewItem.left;
        rcNewList.right = rcNewItem.right;
        if (rcNewItem.ContainsRect(rcNewList)) {
            //子项的区域，已经在可见区域，不调整，避免鼠标点击后上下晃动
            dy = 0;
        }
    }
    if ((dx != 0) || (dy != 0)) {
        UiSize64 sz = GetScrollPos();
        SetScrollPos(UiSize64(sz.cx + dx, sz.cy + dy));
        Invalidate();
    }
}

void ListBox::StopScroll()
{
    StopScrollAnimation();
}

bool ListBox::CanPaintSelectedColors(bool bHasStateImages) const
{
    if (m_uPaintSelectedColors == PAINT_SELECTED_COLORS_YES) {
        return true;
    }
    else if (m_uPaintSelectedColors == PAINT_SELECTED_COLORS_NO) {
        return false;
    }
    if (bHasStateImages && IsMultiSelect()) {
        //如果有CheckBox，多选的时候，默认不显示选择背景色
        return false;
    }
    return true;
}

bool ListBox::IsHorizontalLayout() const
{
    LayoutType type = GetLayout()->GetLayoutType();
    bool bHorizontal = false;
    if ((type == LayoutType::HLayout) ||
        (type == LayoutType::HTileLayout) ||
        (type == LayoutType::VirtualHLayout) ||
        (type == LayoutType::VirtualHTileLayout)) {
        bHorizontal = true;
    }
    return bHorizontal;
}

bool ListBox::IsHorizontalScrollBar() const
{
    bool bHasVScrollBar = false;
    ScrollBar* pVScrollBar = GetVScrollBar();
    if (pVScrollBar && pVScrollBar->IsValid()) {
        bHasVScrollBar = true;
    }

    bool bHasHScrollBar = false;
    ScrollBar* pHScrollBar = GetHScrollBar();
    if (pHScrollBar && pHScrollBar->IsValid()) {
        bHasHScrollBar = true;
    }

    bool bIsHorizontal = (bHasHScrollBar && !bHasVScrollBar) ? true : false;
    if (GetLayout()->IsHLayout()) {
        //确定是横向布局
        bIsHorizontal = true;
    }
    else if (GetLayout()->IsVLayout()) {
        bIsHorizontal = false;
    }
    return bIsHorizontal;
}

bool ListBox::ScrollItemToTop(size_t iIndex)
{
    Control* pControl = GetItemAt(iIndex);
    if ((pControl == nullptr) || !pControl->IsVisible()) {
        return false;
    }
    if (IsHorizontalLayout()) {
        //横向布局
        if (GetScrollRange().cx != 0) {
            UiSize64 scrollPos = GetScrollPos();
            scrollPos.cx = (int64_t)pControl->GetPos().left - GetPosWithoutPadding().left;
            if (scrollPos.cx >= 0) {
                SetScrollPos(scrollPos);
                return true;
            }
        }
    }
    else {
        //纵向布局
        if (GetScrollRange().cy != 0) {
            UiSize64 scrollPos = GetScrollPos();
            scrollPos.cy = (int64_t)pControl->GetPos().top - GetPosWithoutPadding().top;
            if (scrollPos.cy >= 0) {
                SetScrollPos(scrollPos);
                return true;
            }
        }
    }
    return false;
}

bool ListBox::ScrollItemToTop(const DString& itemName)
{
    const size_t itemCount = m_items.size();
    for (size_t iIndex = 0; iIndex < itemCount; ++iIndex) {
        Control* pControl = m_items[iIndex];
        if ((pControl == nullptr) || !pControl->IsVisible()) {
            continue;
        }
        if (pControl->IsNameEquals(itemName)) {
            return ScrollItemToTop(iIndex);
        }
    }
    return false;
}

Control* ListBox::GetTopItem() const
{
    if (IsHorizontalLayout()) {
        //横向布局
        int32_t listLeft = GetPos().left + GetPadding().left + GetScrollOffset().cx;
        for (Control* pControl : m_items) {
            ASSERT(pControl != nullptr);
            if (pControl->IsVisible() && !pControl->IsFloat() && pControl->GetPos().right >= listLeft) {
                return pControl;
            }
        }
    }
    else {
        //纵向布局
        int32_t listTop = GetPos().top + GetPadding().top + GetScrollOffset().cy;
        for (Control* pControl : m_items) {
            ASSERT(pControl != nullptr);
            if (pControl->IsVisible() && !pControl->IsFloat() && pControl->GetPos().bottom >= listTop) {
                return pControl;
            }
        }
    }
    return nullptr;
}

bool ListBox::SetItemIndex(Control* pControl, size_t iIndex)
{
    size_t iOrginIndex = GetItemIndex(pControl);
    if (!Box::IsValidItemIndex(iOrginIndex)) {
        return false;
    }
    if (iOrginIndex == iIndex) {
        return true;
    }

    IListBoxItem* pSelectedListItem = nullptr;
    if (Box::IsValidItemIndex(m_iCurSel)) {
        pSelectedListItem = dynamic_cast<IListBoxItem*>(GetItemAt(m_iCurSel));
    }
    if (!ScrollBox::SetItemIndex(pControl, iIndex)) {
        return false;
    }
    size_t iMinIndex = std::min(iOrginIndex, iIndex);
    size_t iMaxIndex = std::max(iOrginIndex, iIndex);
    for(size_t i = iMinIndex; i < iMaxIndex + 1; ++i) {
        Control* pItemControl = GetItemAt(i);
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pItemControl);
        if( pListItem != nullptr ) {
            pListItem->SetListBoxIndex(i);
        }
    }
    if (Box::IsValidItemIndex(m_iCurSel) && pSelectedListItem != nullptr) {
        m_iCurSel = pSelectedListItem->GetListBoxIndex();
    }
    return true;
}

size_t ListBox::EnsureVisible(size_t iIndex, ListBoxVerVisible vVisibleType, ListBoxHorVisible hVisibleType)
{
    Control* pControl = GetItemAt(iIndex);
    ASSERT(pControl != nullptr);
    if (pControl != nullptr) {
        UiRect rcItem = pControl->GetPos();
        EnsureVisible(rcItem, vVisibleType, hVisibleType);
        ASSERT(GetItemAt(iIndex) == pControl);
    }
    return iIndex;
}

bool ListBox::AddItem(Control* pControl)
{
    IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
    if( pListItem != nullptr) {
        pListItem->SetOwner(this);
        pListItem->SetListBoxIndex(GetItemCount());
        if (!IsMultiSelect()) {
            pListItem->OptionSelected(false, false);
        }
    }
    return ScrollBox::AddItem(pControl);
}

bool ListBox::AddItemAt(Control* pControl, size_t iIndex)
{
    if (!ScrollBox::AddItemAt(pControl, iIndex)) {
        return false;
    }

    IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
    if( pListItem != nullptr ) {
        pListItem->SetOwner(this);
        pListItem->SetListBoxIndex(iIndex);
        if (!IsMultiSelect()) {
            pListItem->OptionSelected(false, false);
        }
    }

    const size_t itemCount = GetItemCount();
    for(size_t i = iIndex + 1; i < itemCount; ++i) {
        Control* p = GetItemAt(i);
        pListItem = dynamic_cast<IListBoxItem*>(p);
        if( pListItem != nullptr ) {
            pListItem->SetListBoxIndex(i);
        }
    }
    if (Box::IsValidItemIndex(m_iCurSel) && (m_iCurSel >= iIndex)) {
        m_iCurSel += 1;
    }
    return true;
}

bool ListBox::RemoveItem(Control* pControl)
{
    size_t iIndex = GetItemIndex(pControl);
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }
    return RemoveItemAt(iIndex);
}

bool ListBox::RemoveItemAt(size_t iIndex)
{
    if (!IsAutoDestroyChild()) {
        Control* p = GetItemAt(iIndex);
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(p);
        if (pListItem != nullptr) {
            pListItem->SetOwner(nullptr);
        }
    }
    if (!ScrollBox::RemoveItemAt(iIndex)) {
        return false;
    }
    const size_t itemCount = GetItemCount();
    for(size_t i = iIndex; i < itemCount; ++i) {
        Control* p = GetItemAt(i);
        IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(p);
        if (pListItem != nullptr) {
            pListItem->SetListBoxIndex(i);
        }
    }

    if (Box::IsValidItemIndex(m_iCurSel)) {
        if (iIndex == m_iCurSel) {
            if (!IsMultiSelect() && m_bSelectNextWhenActiveRemoved) {
                SelectItem(FindSelectable(m_iCurSel--, false));
            }
            else {
                m_iCurSel = Box::InvalidIndex;
            }
        }
        else if (iIndex < m_iCurSel) {
            m_iCurSel -= 1;
        }
    }
    return true;
}

void ListBox::RemoveAllItems()
{
    if (!IsAutoDestroyChild()) {
        const size_t itemCount = GetItemCount();
        for (size_t i = 0; i < itemCount; ++i) {
            Control* p = GetItemAt(i);
            IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(p);
            if (pListItem != nullptr) {
                pListItem->SetOwner(nullptr);
            }
        }
    }
    m_iCurSel = Box::InvalidIndex;
    ScrollBox::RemoveAllItems();
}

bool ListBox::SortItems(PFNCompareFunc pfnCompare, void* pCompareContext)
{
    if (pfnCompare == nullptr) {
        return false;
    }        
    if (m_items.empty()) {
        return true;
    }

    m_pCompareFunc = pfnCompare;
    m_pCompareContext = pCompareContext;
#if defined (_WIN32) || defined (_WIN64)
    //Windows系统
    qsort_s(&(*m_items.begin()), m_items.size(), sizeof(Control*), ListBox::ItemComareFuncWindows, this);
#elif defined(__APPLE__)
    // macOS
    qsort_r(&(*m_items.begin()), m_items.size(), sizeof(Control*), this, ListBox::ItemComareFuncMacOS);   
#else
    //Linux 系统
    qsort_r(&(*m_items.begin()), m_items.size(), sizeof(Control*), ListBox::ItemComareFuncLinux, this);
#endif    
    IListBoxItem* pItem = nullptr;
    const size_t itemCount = m_items.size();
    for (size_t i = 0; i < itemCount; ++i) {
        pItem = dynamic_cast<IListBoxItem*>(m_items[i]);
        if (pItem != nullptr) {
            pItem->SetListBoxIndex(i);
            //取消界面上所有的选择项
            pItem->OptionSelected(false, false);
        }
    }
    SelectItem(Box::InvalidIndex);
    SetPos(GetPos());
    Invalidate();
    return true;
}

int ListBox::ItemComareFuncWindows(void* pvlocale, const void* item1, const void* item2)
{
    ListBox* pThis = (ListBox*)pvlocale;
    if (!pThis || !item1 || !item2) {
        return 0;
    }
    return pThis->ItemComareFunc(item1, item2);
}

int ListBox::ItemComareFuncLinux(const void *item1, const void *item2, void* pvlocale)
{
    ListBox *pThis = (ListBox*)pvlocale;
    if (!pThis || !item1 || !item2) {
        return 0;
    }
    return pThis->ItemComareFunc(item1, item2);
}

int ListBox::ItemComareFuncMacOS(void* context, const void* item1, const void* item2)
{
    ListBox* pThis = (ListBox*)context;
    if (!pThis || !item1 || !item2) {
        return 0;
    }
    return pThis->ItemComareFunc(item1, item2);
}

int ListBox::ItemComareFunc(const void *item1, const void *item2)
{
    if (!item1 || !item2) {
        return 0;
    }
    Control *pControl1 = *(Control**)item1;
    Control *pControl2 = *(Control**)item2;
    return m_pCompareFunc(pControl1, pControl2, m_pCompareContext);
}

bool ListBox::IsMultiSelect() const
{
    return m_bMultiSelect;
}

void ListBox::SetMultiSelect(bool bMultiSelect)
{
    m_bMultiSelect = bMultiSelect;
    if (!bMultiSelect) {
        //只要bMultiSelect为false，就执行取消现有多选的逻辑
        //此处不能判断与原值是否变化来决定是否执行下面的代码，否则可能会影响子类（TreeView的逻辑）
        if (OnSwitchToSingleSelect()) {
            Invalidate();
        }
    }
}

void ListBox::EnsureSingleSelection()
{
    if (!IsMultiSelect()) {
        OnSwitchToSingleSelect();
    }    
}

bool ListBox::OnSwitchToSingleSelect()
{
    bool bChanged = false;
    IListBoxItem* pItem = nullptr;
    const size_t itemCount = m_items.size();
    if (m_iCurSel > itemCount) { 
        //如果单选状态不同步，使用第一个选择的作为最终单选的选择项
        for (size_t i = 0; i < itemCount; ++i) {
            pItem = dynamic_cast<IListBoxItem*>(m_items[i]);
            if ((pItem != nullptr) && pItem->IsSelected()) {
                m_iCurSel = i;
                break;
            }
        }
    }
    for (size_t i = 0; i < itemCount; ++i) {
        pItem = dynamic_cast<IListBoxItem*>(m_items[i]);
        if ((pItem != nullptr) && pItem->IsSelected()) {
            if (m_iCurSel != i) {
                pItem->OptionSelected(false, false); //不触发Select事件
                bChanged = true;
            }
        }
    }
    if (UpdateCurSelItemSelectStatus()) {
        bChanged = true;
    }
    return bChanged;
}

bool ListBox::UpdateCurSelItemSelectStatus()
{
    //同步当前选择项的状态
    bool bChanged = false;
    size_t curSelIndex = GetCurSel();
    if (Box::IsValidItemIndex(curSelIndex)) {
        bool bSelectItem = false;
        IListBoxItem* pItem = dynamic_cast<IListBoxItem*>(GetItemAt(curSelIndex));
        if (pItem != nullptr) {
            bSelectItem = pItem->IsSelected();
        }
        if (!bSelectItem) {
            SetCurSel(Box::InvalidIndex);
            bChanged = true;
        }
    }
    return bChanged;
}

bool ListBox::ButtonDown(const EventArgs& msg)
{
    bool ret = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    //停止滚动
    StopScroll();
    if (m_pHelper != nullptr) {
        m_pHelper->OnButtonDown(msg.ptMouse, msg.GetSender());
    }    
    return ret;
}

bool ListBox::ButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    Control* pSender = msg.GetSender();
    if (IsEnableFrameSelection()) {
        //按住Ctrl或者Shift的时候，不触发清空选择操作，避免误操作
        if (IsKeyDown(msg, ModifierKey::kControl)) {
            pSender = nullptr;
        }
        else if (this->IsKeyDown(msg, ModifierKey::kShift)) {
            pSender = nullptr;
        }
    }
    if (m_pHelper != nullptr) {
        m_pHelper->OnButtonUp(msg.ptMouse, pSender);
    }
    return bRet;
}

bool ListBox::RButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::RButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (m_pHelper != nullptr) {
        m_pHelper->OnRButtonDown(msg.ptMouse, msg.GetSender());
    }
    return bRet;
}

bool ListBox::RButtonUp(const EventArgs& msg)
{
    bool bRet = BaseClass::RButtonUp(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (m_pHelper != nullptr) {
        m_pHelper->OnRButtonUp(msg.ptMouse, msg.GetSender());
    }
    return bRet;
}

bool ListBox::MouseMove(const EventArgs& msg)
{
    bool bRet = BaseClass::MouseMove(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (m_pHelper != nullptr) {
        m_pHelper->OnMouseMove(msg.ptMouse, msg.GetSender());
    }
    return bRet;
}

bool ListBox::OnWindowKillFocus(const EventArgs& msg)
{
    bool bRet = BaseClass::OnWindowKillFocus(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }
    if (m_pHelper != nullptr) {
        m_pHelper->OnWindowKillFocus();
    }
    return bRet;
}

bool ListBox::OnListBoxItemMouseEvent(const EventArgs& msg)
{
    if (m_pHelper != nullptr) {
        if (msg.eventType == kEventMouseButtonDown) {
            m_pHelper->OnButtonDown(msg.ptMouse, msg.GetSender());
        }
        else if (msg.eventType == kEventMouseButtonUp) {
            m_pHelper->OnButtonUp(msg.ptMouse, msg.GetSender());
        }
        else if (msg.eventType == kEventMouseRButtonDown) {
            m_pHelper->OnRButtonDown(msg.ptMouse, msg.GetSender());
        }
        else if (msg.eventType == kEventMouseRButtonUp) {
            m_pHelper->OnButtonUp(msg.ptMouse, msg.GetSender());
        }
        else if (msg.eventType == kEventMouseMove) {
            m_pHelper->OnMouseMove(msg.ptMouse, msg.GetSender());
        }
    }
    //必须返回false, 否则会截获该消息，影响正常功能
    return false;
}

void ListBox::OnListBoxItemWindowKillFocus()
{
    if (m_pHelper != nullptr) {
        m_pHelper->OnWindowKillFocus();
    }
}

void ListBox::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::PaintChild(pRender, rcPaint);
    PaintFrameSelection(pRender);
}

void ListBox::PaintFrameSelection(IRender* pRender)
{
    if (m_pHelper != nullptr) {
        m_pHelper->PaintFrameSelection(pRender);
    }
}

void ListBox::GetScrollDeltaValue(int32_t& nHScrollValue, int32_t& nVScrollValue) const
{
    nHScrollValue = DUI_NOSET_VALUE;
    nVScrollValue = DUI_NOSET_VALUE;
    if (IsHorizontalLayout()) {
        //横向布局
        int32_t deltaValue = 0;
        VirtualHTileLayout* pVirtualHTileLayout = dynamic_cast<VirtualHTileLayout*>(GetLayout());
        if (pVirtualHTileLayout != nullptr) {
            deltaValue = pVirtualHTileLayout->GetItemSize().cx * 2;
        }
        HTileLayout* pHTileLayout = dynamic_cast<HTileLayout*>(GetLayout());
        if (pHTileLayout != nullptr) {
            deltaValue = pHTileLayout->GetItemSize().cx * 2;
        }
        if (deltaValue > 0) {
            deltaValue = std::max(GetRect().Width() / 3, deltaValue);
            nHScrollValue = deltaValue;
        }
    }
    else {
        //纵向布局
        int32_t deltaValue = 0;
        VirtualVTileLayout* pVirtualVTileLayout = dynamic_cast<VirtualVTileLayout*>(GetLayout());
        if (pVirtualVTileLayout != nullptr) {
            deltaValue = pVirtualVTileLayout->GetItemSize().cy * 2;            
        }
        VTileLayout* pVTileLayout = dynamic_cast<VirtualVTileLayout*>(GetLayout());
        if (pVTileLayout != nullptr) {
            deltaValue = pVTileLayout->GetItemSize().cy * 2;
        }
        if (deltaValue > 0) {
            deltaValue = std::max(GetRect().Height() / 3, deltaValue);
            nHScrollValue = deltaValue;
        }
    }
}

bool ListBox::OnLButtonClickedBlank()
{
    if (IsEnableFrameSelection() && IsSelectNoneWhenClickBlank()) {
        //在空白处点击鼠标左键，取消全部选择
        bool bRet = SetSelectNone();
        OnSelectStatusChanged();
        return bRet;
    }
    return false;
}

bool ListBox::OnRButtonClickedBlank()
{
    if (IsEnableFrameSelection() && IsSelectNoneWhenClickBlank()) {
        //在空白处点击鼠标右键，取消全部选择
        bool bRet = SetSelectNone();
        OnSelectStatusChanged();
        return bRet;
    }
    return false;
}

bool ListBox::SetSelectAll()
{
    bool bChanged = false;
    if (IsMultiSelect()) {
        size_t nItemCount = GetItemCount();
        for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
            Control* pControl = GetItemAt(nItemIndex);
            if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
                continue;
            }
            IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
            if (pListItem == nullptr) {
                continue;
            }
            if (!pListItem->IsSelected()) {
                //如果原来是非选择状态，更新为选择状态
                pListItem->OptionSelected(true, false);
                bChanged = true;
            }
        }
    }
    if (bChanged) {
        Invalidate();
    }
    return bChanged;
}

bool ListBox::SetSelectNone()
{
    bool bChanged = false;
    if (IsMultiSelect()) {
        size_t nItemCount = GetItemCount();
        for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
            Control* pControl = GetItemAt(nItemIndex);
            if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
                continue;
            }
            IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
            if (pListItem == nullptr) {
                continue;
            }
            if (pListItem->IsSelected()) {
                //如果原来是选择状态，更新为非选择状态
                pListItem->OptionSelected(false, false);
                bChanged = true;
            }
        }
    }
    if (bChanged) {
        SetCurSel(Box::InvalidIndex);
        Invalidate();
    }
    return bChanged;
}

void ListBox::OnSelectStatusChanged()
{
}

bool ListBox::OnFrameSelection(int64_t left, int64_t right, int64_t top, int64_t bottom)
{
    bool bChanged = false;
    if (IsEnableFrameSelection() && IsMultiSelect()) {
        UiRect rcListBox = GetRect();
        size_t nItemCount = GetItemCount();
        for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
            Control* pControl = GetItemAt(nItemIndex);
            if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
                continue;
            }
            IListBoxItem* pListItem = dynamic_cast<IListBoxItem*>(pControl);
            if (pListItem == nullptr) {
                continue;
            }
            UiRect rc = pControl->GetRect();
            rc.Offset(-rcListBox.left, -rcListBox.top);
            int64_t nNewLeft = (std::max)(left, (int64_t)rc.left);
            int64_t nNewTop = (std::max)(top, (int64_t)rc.top);
            int64_t nNewRight = (std::min)(right, (int64_t)rc.right);
            int64_t nNewBottom = (std::min)(bottom, (int64_t)rc.bottom);
            bool bContains = (nNewBottom > nNewTop) && (nNewRight > nNewLeft);
            if (bContains) {
                //如果原来是非选择状态，更新为选择状态
                if (!pListItem->IsSelected()) {
                    pListItem->OptionSelected(true, false);
                    bChanged = true;
                }
            }
            else {
                //如果原来是选择状态，更新为非选择状态
                if (pListItem->IsSelected()) {
                    pListItem->OptionSelected(false, false);
                    bChanged = true;
                }
            }
        }
    }
    return bChanged;
}

void ListBox::SetLastNoShiftItem(size_t nLastNoShiftItem)
{
    m_nLastNoShiftItem = nLastNoShiftItem;
}

size_t ListBox::GetLastNoShiftItem() const
{
    return m_nLastNoShiftItem;
}

} // namespace ui
