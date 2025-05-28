#include "ListCtrlView.h" 
#include "duilib/Control/ListCtrl.h"
#include "duilib/Control/ListCtrlData.h"
#include "duilib/Core/Keyboard.h"

namespace ui
{
ListCtrlView::ListCtrlView(Window* pWindow, Layout* pLayout):
    VirtualListBox(pWindow, pLayout)
{
}

ListCtrlView::~ListCtrlView()
{
}

void ListCtrlView::SendEventMsg(const EventArgs& msg)
{
    BaseClass::SendEventMsg(msg);
    if ((msg.eventType == kEventSelect) || (msg.eventType == kEventUnSelect)) {
        SendEvent(kEventSelChange);
    }
}

size_t ListCtrlView::GetTopElementIndex() const
{
    std::vector<size_t> itemIndexList;
    GetDisplayElements(itemIndexList);
    if (itemIndexList.empty()) {
        return Box::InvalidIndex;
    }
    else {
        return itemIndexList.front();
    }
}

bool ListCtrlView::IsDataItemDisplay(size_t itemIndex) const
{
    std::vector<size_t> itemIndexList;
    GetDisplayElements(itemIndexList);
    return std::find(itemIndexList.begin(), itemIndexList.end(), itemIndex) != itemIndexList.end();
}

void ListCtrlView::GetDisplayDataItems(std::vector<size_t>& itemIndexList) const
{
    itemIndexList.clear();
    GetDisplayElements(itemIndexList);
}

bool ListCtrlView::EnsureDataItemVisible(size_t itemIndex, bool bToTop)
{
    EnsureVisible(itemIndex, bToTop);
    return true;
}

void ListCtrlView::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    //禁止随鼠标滚轮的滚动改变选中项
    SetScrollSelect(false);
}

int32_t ListCtrlView::CalcRows() const
{
    VirtualHTileLayout* pHLayout = dynamic_cast<VirtualHTileLayout*>(GetLayout());
    return CalcHTileRows(pHLayout);
}

int32_t ListCtrlView::CalcColumns() const
{
    VirtualVTileLayout* pVLayout = dynamic_cast<VirtualVTileLayout*>(GetLayout());
    return CalcVTileColumns(pVLayout);
}

void ListCtrlView::HandleEvent(const EventArgs& msg)
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
        bHandled = OnListCtrlKeyDown(msg);
    }
    if (!bHandled) {
        BaseClass::HandleEvent(msg);
    }
}

bool ListCtrlView::OnListCtrlKeyDown(const EventArgs& msg)
{
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
    const size_t nElementCount = GetElementCount();
    if (!bArrowKeyDown || !IsMultiSelect() || (nElementCount == 0)) {
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
    size_t nCurSel = GetCurSel();
    if (nCurSel < GetItemCount()) {
        //判断其是否可以选择：置顶项是不可选择的，应加以过滤
        Control* pControl = GetItemAt(nCurSel);
        if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsSelectableType()) {
            nCurSel = Box::InvalidIndex;
        }
    }
    if (nCurSel >= GetItemCount()) {
        //查找当前视图内第一个可选择的项目，作为起始点
        size_t nCount = GetItemCount();
        for (size_t index = 0; index < nCount; ++index) {
            Control* pControl = GetItemAt(index);
            if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsSelectableType()) {
                continue;
            }
            IListBoxItem* pItem = dynamic_cast<IListBoxItem*>(pControl);
            if ((pItem != nullptr) && pItem->IsSelected()) {
                nCurSel = index;
                break;
            }
        }
    }
    if (nCurSel >= GetItemCount()) {
        //尝试查找未在当前视图显示的已选择元素
        size_t nDestItemIndex = Box::InvalidIndex;
        if (OnFindSelectable(GetCurSel(), SelectableMode::kSelect, 1, nDestItemIndex)) {
            nCurSel = GetCurSel();
            ASSERT(nCurSel == nDestItemIndex);
        }
    }
    if (nCurSel >= GetItemCount()) {
        //全部尝试未成功后，以当前视图内的第一个元素为起始点
        size_t nCount = GetItemCount();
        for (size_t index = 0; index < nCount; ++index) {
            Control* pControl = GetItemAt(index);
            if ((pControl == nullptr) || !pControl->IsVisible()) {
                continue;
            }
            IListBoxItem* pItem = dynamic_cast<IListBoxItem*>(pControl);
            if ((pItem != nullptr) &&
                (pItem->GetElementIndex() < nElementCount) &&
                pControl->IsSelectableType()) {
                nCurSel = index;
                SetCurSel(nCurSel);
                break;
            }
        }
    }

    size_t nIndexCurSel = Box::InvalidIndex;
    if (nCurSel < GetItemCount()) {
        nIndexCurSel = GetDisplayItemElementIndex(nCurSel);
    }
    const bool bForward = (msg.vkCode == kVK_DOWN) || (msg.vkCode == kVK_RIGHT) || 
                          (msg.vkCode == kVK_NEXT) || (msg.vkCode == kVK_HOME);
    if (nIndexCurSel < nElementCount) {
        //匹配可选择项
        nIndexCurSel = FindSelectableElement(nIndexCurSel, bForward);
    }
    if (nIndexCurSel >= nElementCount) {
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
            if ((nIndexCurSel + 1) < nElementCount) {
                nIndexEnd = nIndexCurSel + 1;
            }
            else {
                nIndexEnsureVisible = nElementCount - 1;
            }
        }
        else {
            //纵向布局
            if ((nIndexCurSel + nColumns) < nElementCount) {
                nIndexEnd = nIndexCurSel + nColumns;
            }
            else {
                nIndexEnsureVisible = nElementCount - 1;
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
            if ((nIndexCurSel + nRows) < nElementCount) {
                nIndexEnd = nIndexCurSel + nRows;
            }
            else {
                nIndexEnsureVisible = nElementCount - 1;
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
                if ((nIndexCurSel + 1) < nElementCount) {
                    nIndexEnd = nIndexCurSel + 1;
                }
                else {
                    nIndexEnsureVisible = nElementCount - 1;
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
        if ((nIndexCurSel + nScrollCount) < nElementCount) {
            nIndexEnd = nIndexCurSel + nScrollCount;
        }
        else {
            if (IsHorizontalLayout()) {
                for (int32_t nColumn = (int32_t)nShowColumns - 1; nColumn >= 0; --nColumn) {
                    nScrollCount = (size_t)nColumn * nShowRows;
                    if ((nIndexCurSel + nScrollCount) < nElementCount) {
                        //跳转到最后一列，同行的位置
                        nIndexEnd = nIndexCurSel + nScrollCount;
                        nIndexEnsureVisible = nElementCount - 1;
                        break;
                    }
                }
            }
            else {
                for (int32_t nRow = (int32_t)nShowRows - 1; nRow >= 0; --nRow) {
                    nScrollCount = nShowColumns * (size_t)nRow;
                    if ((nIndexCurSel + nScrollCount) < nElementCount) {
                        //跳转到最后一行，同列的位置
                        nIndexEnd = nIndexCurSel + nScrollCount;
                        nIndexEnsureVisible = nElementCount - 1;
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
        nIndexEnd = nElementCount - 1;
        break;
    default:
        break;
    }

    if (nIndexEnd >= nElementCount) {
        if (nIndexEnsureVisible != Box::InvalidIndex) {
            EnsureVisible(nIndexEnsureVisible, false);
        }
        return bHandled;
    }

    //匹配可选择项
    nIndexEnd = FindSelectableElement(nIndexEnd, bForward);
    if (nIndexEnd >= nElementCount) {
        return bHandled;
    }

    bHandled = true;
    std::vector<size_t> selectedIndexs; //需要选择的列表
    if (bShiftDown) {
        //按住Shift键：选择范围内的所有数据
        size_t nLastNoShiftIndex = GetLastNoShiftIndex();//起始的元素索引号
        if (nLastNoShiftIndex >= nElementCount) {
            nLastNoShiftIndex = 0;
        }
        size_t nStartElementIndex = std::min(nLastNoShiftIndex, nIndexEnd);
        size_t nEndElementIndex = std::max(nLastNoShiftIndex, nIndexEnd);
        for (size_t i = nStartElementIndex; i <= nEndElementIndex; ++i) {
            if (IsSelectableElement(i)) {
                selectedIndexs.push_back(i);
            }
        }
    }
    else {
        //没有按住Shift键：只选择最后一个数据
        selectedIndexs.push_back(nIndexEnd);
    }

    //选择这个范围内的所有元素
    std::vector<size_t> refreshIndexs;
    SetSelectedElements(selectedIndexs, true, refreshIndexs);
    RefreshElements(refreshIndexs);
    if (nIndexEnsureVisible != Box::InvalidIndex) {
        EnsureVisible(nIndexEnsureVisible, false);
    }
    else {
        EnsureVisible(nIndexEnd, false);
    }    
    nCurSel = GetDisplayItemIndex(nIndexEnd);
    ASSERT(nCurSel < GetItemCount());
    bool bTriggerEvent = false;
    if (nCurSel < GetItemCount()) {
        SetCurSel(nCurSel);
        SelectItemSingle(nCurSel, true, false);
        bTriggerEvent = true;
        ASSERT(GetItemAt(nCurSel)->IsFocused());
        ASSERT(IsElementSelected(nIndexEnd));
#ifdef _DEBUG
        std::vector<size_t> selected;
        GetSelectedItems(selected);
        ASSERT(std::find(selected.begin(), selected.end(), nCurSel) != selected.end());
#endif
    }
    OnSelectStatusChanged();
    if (bTriggerEvent) {
        SendEvent(kEventSelect, nCurSel, Box::InvalidIndex);
    }
    return bHandled;
}

bool ListCtrlView::SelectItem(size_t iIndex, bool bTakeFocus, bool bTriggerEvent, uint64_t vkFlag)
{
    //不需要判断IsSelectLikeListCtrl()是否开启, 固定使用这种选择模式
    return ListCtrlSelectItem(iIndex, bTakeFocus, bTriggerEvent, vkFlag);
}

}//namespace ui

