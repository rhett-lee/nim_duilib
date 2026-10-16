#include "ListCtrlReportView.h" 
#include "ListCtrl.h"
#include "duilib/Render/AutoClip.h"

//包含类：ListCtrlReportView / ListCtrlReportLayout

namespace ui
{
ListCtrlReportView::ListCtrlReportView(Window* pWindow) :
    ListCtrlView(pWindow, new ListCtrlReportLayout),
    m_pListCtrl(nullptr),
    m_pData(nullptr),
    m_nTopElementIndex(0),
    m_fRowGridLineWidth(0),
    m_fColumnGridLineWidth(0),
    m_bFrozenHitTestPass(false)
{
    ListCtrlReportLayout* pDataLayout = dynamic_cast<ListCtrlReportLayout*>(GetLayout());
    ASSERT(pDataLayout != nullptr);

    VirtualLayout* pVirtualLayout = pDataLayout;
    ASSERT(pVirtualLayout != nullptr);
    SetVirtualLayout(pVirtualLayout);
    
    if (pDataLayout != nullptr) {
        pDataLayout->SetDataView(this);
    }
    SetRowGridLineWidth(1, true);
    SetColumnGridLineWidth(1, true);
}

ListCtrlReportView::~ListCtrlReportView() 
{
}

void ListCtrlReportView::HandleEvent(const EventArgs& msg)
{
    if ((msg.eventType == kEventKeyDown) && (m_pListCtrl != nullptr)) {
        //下拉候选浮层处于打开状态时（行选择模式与单元格选择模式通用）：键盘事件优先由浮层消费
        if (m_pListCtrl->OnComboKeyDown(msg)) {
            return;
        }
        //单元格选择模式下，键盘事件优先用于移动当前格 / 扩展选择区域
        if ((m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell) &&
            m_pListCtrl->OnCellKeyDown(msg)) {
            return;
        }
    }
    //鼠标移出控件时清除单元格悬停高亮，避免残留
    if ((msg.eventType == kEventMouseLeave) && (m_pListCtrl != nullptr) &&
        (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        m_pListCtrl->ClearHoverCell();
    }
    BaseClass::HandleEvent(msg);
    if (m_pListCtrl != nullptr) {
        if ((msg.eventType > kEventKeyBegin) && (msg.eventType < kEventKeyEnd)) {
            m_pListCtrl->OnViewKeyboardEvents(msg);
        }
        else if ((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd)) {
            m_pListCtrl->OnViewMouseEvents(msg);
        }
    }
}

bool ListCtrlReportView::OnListBoxItemMouseEvent(const EventArgs& msg)
{
    //单元格选择模式下，拦截鼠标事件用于单元格的选择与区域扩展
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        //行的进入/离开事件不做消费，必须继续向父控件（视图 ScrollBox）冒泡：
        //视图收到 MouseEnter/MouseLeave 才会进入/退出 Hovered 状态，进而触发自动隐藏滚动条的淡入/淡出；
        //行级 Hover/Pressed 高亮属于行选择模式专属视觉，由 ListCtrlItem 重写的 MouseEnter/MouseLeave
        //（kCell 下不进入 Hovered 状态、不启动悬停渐隐动画）屏蔽，绘制层 PaintStateColors 作为兜底
        if (msg.eventType == kEventMouseLeave) {
            //鼠标离开该行（比如移到视图的空白区域）：清除单元格悬停状态，避免悬停高亮残留在最后划过的单元格上
            m_pListCtrl->ClearHoverCell();
        }
        if ((msg.eventType == kEventMouseEnter) || (msg.eventType == kEventMouseLeave)) {
            return BaseClass::OnListBoxItemMouseEvent(msg);
        }
        const bool bLButton = (msg.eventType == kEventMouseButtonDown) || (msg.eventType == kEventMouseButtonUp);
        const bool bRButton = (msg.eventType == kEventMouseRButtonDown) || (msg.eventType == kEventMouseRButtonUp);
        if (bLButton || bRButton || (msg.eventType == kEventMouseMove) || (msg.eventType == kEventMouseHover)) {
            //先把鼠标事件转给基类：ListBoxHelper 借此记录框选状态并绘制框选蒙层（与行选择模式同一套实现）
            BaseClass::OnListBoxItemMouseEvent(msg);
            //鼠标落在"数据行/表头"自身的CheckBox图标上时，不消费该事件（返回false，交给Item自身处理）：
            //单元格选择模式下若消费该事件，ListBoxItemTemplate::HandleEvent 会提前返回，
            //Item 不会执行自身的 ButtonDown/ButtonUp，其中的CheckBox勾选逻辑就不生效，
            //导致行级CheckBox与表头CheckBox点击无响应；放行后由Item自行完成勾选/取消勾选，
            //整行选择逻辑已由 ListCtrlItem::ButtonDown 在单元格选择模式下跳过，不会误触发整行选择
            if (bLButton && IsMouseOnItemCheckBox(msg.GetSender(), msg.ptMouse)) {
                return false;
            }
            //因为下面会消费事件（阻止 Item 触发行选择以及行级 Hover/Pressed 视觉），Item 不会执行 Control::ButtonDown，
            //不会自动置位"鼠标焦点"状态，故这里显式维护，以满足 ListBoxHelper::OnMouseMove 对 pSender->IsMouseFocused() 的要求
            Control* pSender = msg.GetSender();
            if (pSender != nullptr) {
                if ((msg.eventType == kEventMouseButtonDown) || (msg.eventType == kEventMouseRButtonDown)) {
                    pSender->SetMouseFocused(true);
                }
                else if ((msg.eventType == kEventMouseButtonUp) || (msg.eventType == kEventMouseRButtonUp)) {
                    pSender->SetMouseFocused(false);
                }
            }
            //鼠标落在"表头/置顶行"区域内时，不做单元格的选择与悬停处理（按钮抬起除外，仅用于结束拖动状态）：
            //避免点击置顶行时"穿透"选中其下方被遮挡的单元格，也避免误清空已有的单元格选择
            const bool bButtonUp = (msg.eventType == kEventMouseButtonUp) || (msg.eventType == kEventMouseRButtonUp);
            if (!bButtonUp && IsPointOnAtTopItem(msg.ptMouse)) {
                if ((msg.eventType == kEventMouseMove) || (msg.eventType == kEventMouseHover)) {
                    //清除单元格悬停状态，避免悬停高亮残留在置顶行下方
                    m_pListCtrl->UpdateHoverCell(Box::InvalidIndex, Box::InvalidIndex);
                }
                else if ((msg.eventType == kEventMouseButtonDown) || (msg.eventType == kEventMouseRButtonDown)) {
                    //置顶行（不可选行）内的单元格CheckBox同样要求"当前列表项是焦点控件"才能勾选
                    //（见 ListCtrlLabel::OnMouseEvent 的焦点前置条件），故与普通行一致，按下时把命中的置顶行设为焦点控件
                    ListCtrlItem* pTopItem = HitTestTopItem(msg.ptMouse);
                    if (pTopItem != nullptr) {
                        pTopItem->SetFocus();
                    }
                }
                return true;
            }
            //再按单元格语义处理命中与选择
            size_t nRowIndex = Box::InvalidIndex;
            size_t nColumnId = Box::InvalidIndex;
            ListCtrlItem* pHitItem = nullptr;
            HitTestCell(msg.ptMouse, nRowIndex, nColumnId, &pHitItem);
            if ((pHitItem != nullptr) &&
                ((msg.eventType == kEventMouseButtonDown) || (msg.eventType == kEventMouseRButtonDown))) {
                //显式把命中的列表项设为键盘焦点控件：
                //1) 单元格内的CheckBox(ListCtrlLabel)在处理勾选/取消勾选前，要求"当前列表项是焦点控件"；
                //2) 焦点控件是键盘事件的接收入口，方向键/Ctrl+A等只有先到焦点控件才能冒泡到视图被 OnCellKeyDown 处理。
                //（行选择模式下由 ListBox::SelectItemSingle 完成同样的焦点设置）
                //注：SetFocus() 会经由 Control::OnSetFocus 把该行控件状态置为 Hovered，
                //因行级 Hovered 视觉已由 ListCtrlItem::PaintStateColors 在单元格选择模式下屏蔽，此处无需再做状态恢复。
                pHitItem->SetFocus();
            }
            m_pListCtrl->OnCellMouseEvent(msg, nRowIndex, nColumnId);
            //消费该事件：不触发行选择逻辑
            return true;
        }
        if (msg.eventType == kEventMouseRDoubleClick) {
            return true;
        }
    }
    return BaseClass::OnListBoxItemMouseEvent(msg);
}

bool ListCtrlReportView::OnListBoxItemKeyEvent(const EventArgs& msg)
{
    if (m_pListCtrl == nullptr) {
        return false;
    }
    //下拉候选浮层打开时，键盘事件优先由浮层消费（上下移动候选、回车提交、ESC取消）：
    //若不在子项层拦截，回车会被 ListBoxItem 消费并转为 kEventReturn，导致浮层收不到回车、无法提交
    return m_pListCtrl->OnComboKeyDown(msg);
}

bool ListCtrlReportView::HitTestCell(const UiPoint& ptMouse, size_t& nRowIndex, size_t& nColumnId, ListCtrlItem** ppItem) const
{
    nRowIndex = Box::InvalidIndex;
    nColumnId = Box::InvalidIndex;
    if (ppItem != nullptr) {
        *ppItem = nullptr;
    }
    const size_t nItemCount = GetItemCount();
    //首行是表头，从索引1开始遍历数据行
    for (size_t index = 1; index < nItemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
        if ((pItem == nullptr) || !pItem->IsVisible()) {
            continue;
        }
        if (!pItem->IsSelectableType()) {
            //置顶行等不可选的行不参与单元格命中
            continue;
        }
        //鼠标点换算为子项控件坐标（存在冻结列时，横向按冻结区口径换算）
        const UiPoint pt = MouseToCellSpace(ptMouse, pItem->GetScrollOffsetInScrollBox());
        //在子项控件中查找包含该点的单元格：此处不能复用 ListCtrlItem::GetSubItem，
        //因为它按统一滚动量换算，鼠标落在冻结区内时命中的列会与绘制错位
        ListCtrlSubItem* pSubItem = nullptr;
        const size_t nSubItemCount = pItem->GetItemCount();
        for (size_t nSubItemIndex = 0; nSubItemIndex < nSubItemCount; ++nSubItemIndex) {
            ListCtrlSubItem* pSubItemTemp = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nSubItemIndex));
            if ((pSubItemTemp != nullptr) && pSubItemTemp->IsVisible() && pSubItemTemp->GetRect().ContainsPt(pt)) {
                pSubItem = pSubItemTemp;
                break;
            }
        }
        if (pSubItem != nullptr) {
            nRowIndex = pItem->GetDataItemIndex();
            nColumnId = pSubItem->GetDataColumnId();
            if (ppItem != nullptr) {
                *ppItem = pItem;
            }
            return true;
        }
    }
    return false;
}

UiPoint ListCtrlReportView::MouseToCellSpace(const UiPoint& ptMouse, const UiPoint& scrollOffsetInBox) const
{
    int32_t nScrollX = scrollOffsetInBox.x;
    if ((nScrollX != 0) && (m_pListCtrl != nullptr)) {
        const ListCtrlCellGeometry cellGeometry = BuildCellGeometry();
        if (cellGeometry.bHasFrozen) {
            //鼠标落在冻结区内时，横向滚动量按 0 计：冻结列不随横向滚动移动
            //（绘制时横向的窗口原点偏移为 0，命中口径必须与绘制一致）
            if (cellGeometry.IsFrozenX(ptMouse.x)) {
                nScrollX = 0;
            }
        }
    }
    UiPoint pt(ptMouse);
    pt.Offset(nScrollX, scrollOffsetInBox.y);
    return pt;
}

bool ListCtrlReportView::IsPointOnAtTopItem(const UiPoint& ptMouse) const
{
    const size_t nItemCount = GetItemCount();
    //首行是表头，从索引1开始遍历数据行
    for (size_t index = 1; index < nItemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
        if ((pItem == nullptr) || !pItem->IsVisible() || pItem->IsSelectableType()) {
            continue;
        }
        //置顶行（不可选行）：判断鼠标点是否落在该行的范围内
        //（与 GetSubItem 使用同一个坐标口径，避免滚动后判断错误；存在冻结列时横向按冻结区口径换算）
        const UiPoint pt = MouseToCellSpace(ptMouse, pItem->GetScrollOffsetInScrollBox());
        if (pItem->GetRect().ContainsPt(pt)) {
            return true;
        }
    }
    return false;
}

ListCtrlItem* ListCtrlReportView::HitTestTopItem(const UiPoint& ptMouse) const
{
    const size_t nItemCount = GetItemCount();
    //首行是表头，从索引1开始遍历数据行
    for (size_t index = 1; index < nItemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
        if ((pItem == nullptr) || !pItem->IsVisible() || pItem->IsSelectableType()) {
            continue;
        }
        //置顶行（不可选行）：判断鼠标点是否落在该行的范围内（与 GetSubItem 使用同一个坐标口径）
        const UiPoint pt = MouseToCellSpace(ptMouse, pItem->GetScrollOffsetInScrollBox());
        if (pItem->GetRect().ContainsPt(pt)) {
            return pItem;
        }
    }
    return nullptr;
}

bool ListCtrlReportView::IsMouseOnItemCheckBox(const Control* pSender, const UiPoint& ptMouse) const
{
    if (pSender == nullptr) {
        return false;
    }
    //数据行（ListCtrlItem）自身的CheckBox
    const ListCtrlItem* pListItem = dynamic_cast<const ListCtrlItem*>(pSender);
    if (pListItem != nullptr) {
        return pListItem->IsShowCheckBox() && pListItem->IsMouseOnCheckBoxIcon(ptMouse);
    }
    //表头（ListCtrlHeader）自身的CheckBox
    const ListCtrlHeader* pListHeader = dynamic_cast<const ListCtrlHeader*>(pSender);
    if (pListHeader != nullptr) {
        return pListHeader->IsShowCheckBox() && pListHeader->IsMouseOnCheckBoxIcon(ptMouse);
    }
    return false;
}

bool ListCtrlReportView::GetTopFixedItemsRect(UiRect& rcTopFixed) const
{
    bool bHasTopFixed = false;
    const size_t nItemCount = GetItemCount();
    for (size_t index = 0; index < nItemCount; ++index) {
        Control* pControl = GetItemAt(index);
        if ((pControl == nullptr) || !pControl->IsVisible()) {
            continue;
        }
        if (index > 0) {
            //索引0为表头；其余元素中，只有置顶行（不可选行）是固定显示在顶部的
            ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(pControl);
            if ((pItem == nullptr) || pItem->IsSelectableType()) {
                continue;
            }
        }
        if (!bHasTopFixed) {
            rcTopFixed = pControl->GetRect();
            bHasTopFixed = true;
        }
        else {
            rcTopFixed.Union(pControl->GetRect());
        }
    }
    return bHasTopFixed;
}

void ListCtrlReportView::SetListCtrl(ListCtrl* pListCtrl)
{
    m_pListCtrl = pListCtrl;
}

void ListCtrlReportView::SetDataProvider(VirtualListBoxElement* pProvider)
{
    BaseClass::SetDataProvider(pProvider);
    if (GetDataProvider() != nullptr) {
        m_pData = dynamic_cast<ListCtrlData*>(GetDataProvider());
        ASSERT(m_pData != nullptr);
    }
    else {
        m_pData = nullptr;
    }
}

void ListCtrlReportView::InvalidateRowElementIndexes()
{
    //第0个子项是表头控件，从第1个开始（数据行控件）
    const size_t nItemCount = GetItemCount();
    for (size_t nItemIndex = 1; nItemIndex < nItemCount; ++nItemIndex) {
        Control* pControl = GetItemAt(nItemIndex);
        if (pControl == nullptr) {
            continue;
        }
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            pListBoxItem->SetElementIndex(Box::InvalidIndex);
        }
    }
}

void ListCtrlReportView::Refresh(bool bSync)
{
    if ((m_pListCtrl != nullptr) && !m_pListCtrl->IsEnableRefresh()) {
        //刷新功能已经禁止
        return;
    }
    if ((m_pListCtrl != nullptr) && !m_pListCtrl->IsInited()) {
        return;
    }
    if ((GetWindow() == nullptr) || !HasDataProvider()) {
        return;
    }
    //调整UI控件的个数，以满足显示使用
    AjustItemCount();

    //结构性刷新（排序/增删行/行高/置顶行/隐藏行等）后，数据元素与UI控件的绑定关系可能已经失效，
    //清除所有行的元素索引绑定，使下面重新布局时按需重新填充数据（绑定关系未失效的行不再重复填充）
    InvalidateRowElementIndexes();

    //刷新UI控件的显示内容
    if (GetElementCount() > 0) {
        ReArrangeChild(true);
        Arrange();
    }
    if (bSync) {
        //立即重新布局
        SetPos(GetPos());
    }
}

void ListCtrlReportView::AjustItemCount()
{
    VirtualLayout* pVirtualLayout = GetVirtualLayout();
    if (pVirtualLayout == nullptr) {
        return;
    }

    //最大子项数
    size_t nMaxItemCount = pVirtualLayout->AjustMaxItem(GetPosWithoutPadding());
    if (nMaxItemCount == 0) {
        return;
    }

    //当前数据总数: 数据总数 + Header控件
    size_t nElementCount = GetElementCount() + 1;

    //当前子项数
    size_t nItemCount = GetItemCount();

    //刷新后的子项数
    size_t nNewItemCount = nElementCount;
    if (nNewItemCount > nMaxItemCount) {
        nNewItemCount = nMaxItemCount;
    }

    if (nItemCount > nNewItemCount) {
        //如果现有子项总数大于新计算的子项数，移除比数据总数多出的子项（从后面删除）
        size_t n = nItemCount - nNewItemCount;
        for (size_t i = 0; i < n; ++i) {
            size_t itemCount = GetItemCount();
            if (itemCount > 1) {
                //至少保留一个Header控件
                RemoveItemAt(itemCount - 1);
            }
        }
    }
    else if (nItemCount < nNewItemCount) {
        //如果现有子项总数小于新计算的子项数，新增比数据总数少的子项
        size_t n = nNewItemCount - nItemCount;
        for (size_t i = 0; i < n; ++i) {
            Control* pControl = CreateElement();
            ASSERT(pControl != nullptr);
            if (pControl != nullptr) {
                AddItem(pControl);
            }
        }
    }
}

int32_t ListCtrlReportView::GetListCtrlWidth() const
{
    int32_t nToltalWidth = 0;
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return nToltalWidth;
    }
    ListCtrlHeader* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
    if (pHeaderCtrl == nullptr) {
        return nToltalWidth;
    }
    size_t nColumnCount = pHeaderCtrl->GetColumnCount();
    for (size_t index = 0; index < nColumnCount; ++index) {
        ListCtrlHeaderItem* pHeaderItem = pHeaderCtrl->GetColumn(index);
        if ((pHeaderItem != nullptr) && pHeaderItem->IsColumnVisible()) {
            nToltalWidth += pHeaderItem->GetColumnWidth();
        }
    }
    UiPadding rcReportViewPadding = GetPadding();
    nToltalWidth += rcReportViewPadding.left + rcReportViewPadding.right;
    nToltalWidth += pHeaderCtrl->GetPadding().left;
    return nToltalWidth;
}

void ListCtrlReportView::SetTopElementIndex(size_t nTopElementIndex)
{
    m_nTopElementIndex = nTopElementIndex;
}

size_t ListCtrlReportView::GetTopElementIndex() const
{
    return m_nTopElementIndex;
}

void ListCtrlReportView::SetDisplayDataItems(const std::vector<size_t>& itemIndexList)
{
    m_diplayItemIndexList = itemIndexList;
}

void ListCtrlReportView::GetDisplayDataItems(std::vector<size_t>& itemIndexList) const
{
    itemIndexList = m_diplayItemIndexList;
}

bool ListCtrlReportView::IsDataItemDisplay(size_t itemIndex) const
{
    auto iter = std::find(m_diplayItemIndexList.begin(), m_diplayItemIndexList.end(), itemIndex);
    return iter != m_diplayItemIndexList.end();
}

bool ListCtrlReportView::EnsureDataItemVisible(size_t itemIndex, bool bToTop)
{
    if (!Box::IsValidItemIndex(itemIndex) || (itemIndex >= GetElementCount())) {
        return false;
    }
    VirtualLayout* pVirtualLayout = GetVirtualLayout();
    if (pVirtualLayout != nullptr) {
        pVirtualLayout->EnsureVisible(GetRect(), itemIndex, bToTop);
        return true;
    }
    return false;
}

size_t ListCtrlReportView::GetTopDataItemIndex(int64_t nScrollPosY) const
{
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return 0;
    }
    size_t itemIndex = 0;
    if (nScrollPosY <= 0) {
        return itemIndex;
    }
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return itemIndex;
    }
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = pDataProvider->GetDataItemCount();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData* pRowData = pDataProvider->GetDisplayItemData(index);
        if (pRowData == nullptr) {
            continue;
        }
        const ListCtrlItemData& rowData = *pRowData;
        nItemHeight = rowData.nItemHeight;
        if (nItemHeight < 0) {
            //取默认行高
            nItemHeight = m_pListCtrl->GetDataItemHeight();
        }
        if (!rowData.bVisible || (nItemHeight  == 0)){
            continue;
        }
        totalItemHeight += nItemHeight;
        //如果每行高度都相同，相当于 nScrollPosY / ItemHeight
        if (totalItemHeight > nScrollPosY) {
            itemIndex = index;
            break;
        }
    }
    return itemIndex;
}

int32_t ListCtrlReportView::GetDataItemHeight(size_t itemIndex) const
{
    int32_t nItemHeight = 0;
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return 0;
    }
    nItemHeight = m_pListCtrl->GetDataItemHeight();
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return 0;
    }
    const ListCtrlItemData* pRowData = pDataProvider->GetDisplayItemData(itemIndex);
    ASSERT(pRowData != nullptr);
    if ((pRowData != nullptr) && (pRowData->nItemHeight >= 0)) {
        nItemHeight = pRowData->nItemHeight;
    }
    return nItemHeight;
}

void ListCtrlReportView::GetDataItemsToShow(int64_t nScrollPosY, size_t maxCount, 
                                            std::vector<ShowItemInfo>& itemIndexList,
                                            std::vector<ShowItemInfo>& atTopItemIndexList,
                                            int64_t& nPrevItemHeights) const
{
    nPrevItemHeights = 0;
    itemIndexList.clear();
    atTopItemIndexList.clear();
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return;
    }
    ASSERT(nScrollPosY >= 0);
    if (nScrollPosY < 0) {
        return;
    }
    //ASSERT(maxCount != 0);
    if (maxCount == 0) {
        return;
    }
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return;
    }
    size_t nTopDataItemIndex = Box::InvalidIndex; //顶部可见的第一个元素序号
    const int32_t nDefaultItemHeight = m_pListCtrl->GetDataItemHeight(); //默认行高
    //置顶的元素序号
    struct AlwaysAtTopData
    {
        int8_t nAlwaysAtTop;    //置顶优先级
        size_t nItemIndex;      //元素索引
        int32_t nItemHeight;    //元素的高度
    };
    std::vector<AlwaysAtTopData> alwaysAtTopItemList;
    
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = pDataProvider->GetDataItemCount();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData* pRowData = pDataProvider->GetDisplayItemData(index);
        if (pRowData == nullptr) {
            continue;
        }
        const ListCtrlItemData& rowData = *pRowData;
        nItemHeight = (rowData.nItemHeight < 0) ? nDefaultItemHeight : rowData.nItemHeight;
        if (!rowData.bVisible || (nItemHeight == 0)) {
            //不可见的，跳过
            continue;
        }

        if (rowData.nAlwaysAtTop >= 0) {
            //置顶的元素
            if (alwaysAtTopItemList.size() < maxCount) {
                alwaysAtTopItemList.push_back({ rowData.nAlwaysAtTop, index, nItemHeight });
            }
            continue;
        }

        if (nTopDataItemIndex == Box::InvalidIndex) {
            totalItemHeight += nItemHeight;
            //如果每行高度都相同，相当于 nScrollPosY / ItemHeight
            if (totalItemHeight > nScrollPosY) {
                nTopDataItemIndex = index;
                nPrevItemHeights = totalItemHeight - nItemHeight;
            }
        }

        if ((nTopDataItemIndex != Box::InvalidIndex) && (itemIndexList.size() < maxCount)){
            itemIndexList.push_back({ index, nItemHeight });
        }
    }

    //对置顶的排序
    if (!alwaysAtTopItemList.empty()) {
        std::stable_sort(alwaysAtTopItemList.begin(), alwaysAtTopItemList.end(),
            [](const AlwaysAtTopData& a, const AlwaysAtTopData& b) {
                //nAlwaysAtTop值大的，排在前面
                return a.nAlwaysAtTop > b.nAlwaysAtTop;
            });
    }
    for (const AlwaysAtTopData& item : alwaysAtTopItemList) {
        atTopItemIndexList.push_back({ item.nItemIndex, item.nItemHeight });
    }
    if (atTopItemIndexList.size() >= maxCount) {
        atTopItemIndexList.resize(maxCount);
        itemIndexList.clear();
    }
    else {
        size_t nLeftCount = maxCount - atTopItemIndexList.size();
        if (itemIndexList.size() > nLeftCount) {
            itemIndexList.resize(nLeftCount);
        }
    }
    ASSERT((itemIndexList.size() + atTopItemIndexList.size()) <= maxCount);
}

int32_t ListCtrlReportView::GetMaxDataItemsToShow(int64_t nScrollPosY, int32_t nRectHeight, 
                                                std::vector<size_t>* pItemIndexList,
                                                std::vector<size_t>* pAtTopItemIndexList) const
{
    if (pItemIndexList) {
        pItemIndexList->clear();
    }
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return 0;
    }
    ASSERT(nScrollPosY >= 0);
    if (nScrollPosY < 0) {
        return 0;
    }
    //ASSERT(nRectHeight > 0);
    if (nRectHeight <= 0) {
        return 0;
    }
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return 0;
    }
    size_t nTopDataItemIndex = Box::InvalidIndex; //顶部可见的第一个元素序号
    const int32_t nDefaultItemHeight = m_pListCtrl->GetDataItemHeight(); //默认行高
    //置顶的元素序号
    struct AlwaysAtTopData
    {
        int8_t nAlwaysAtTop;
        size_t index;
    };
    std::vector<AlwaysAtTopData> alwaysAtTopItemList;
    std::vector<size_t> itemIndexList;

    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = pDataProvider->GetDataItemCount();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData* pRowData = pDataProvider->GetDisplayItemData(index);
        if (pRowData == nullptr) {
            continue;
        }
        const ListCtrlItemData& rowData = *pRowData;
        nItemHeight = (rowData.nItemHeight < 0) ? nDefaultItemHeight : rowData.nItemHeight;
        if (!rowData.bVisible || (nItemHeight == 0)) {
            //不可见的，跳过
            continue;
        }

        if (rowData.nAlwaysAtTop >= 0) {
            //置顶的元素
            alwaysAtTopItemList.push_back({ rowData.nAlwaysAtTop, index });
            continue;
        }

        if (nTopDataItemIndex == Box::InvalidIndex) {
            totalItemHeight += nItemHeight;
            //如果每行高度都相同，相当于 nScrollPosY / ItemHeight
            if (totalItemHeight > nScrollPosY) {
                nTopDataItemIndex = index;
            }
        }

        if (nTopDataItemIndex != Box::InvalidIndex) {
            itemIndexList.push_back(index);
        }
    }

    //对置顶的排序
    if (!alwaysAtTopItemList.empty()) {
        std::stable_sort(alwaysAtTopItemList.begin(), alwaysAtTopItemList.end(),
            [](const AlwaysAtTopData& a, const AlwaysAtTopData& b) {
                //nAlwaysAtTop值大的，排在前面
                return a.nAlwaysAtTop > b.nAlwaysAtTop;
            });
    }
    std::vector<size_t> atTopIndexList;
    std::vector<size_t> tempItemIndexList;
    tempItemIndexList.swap(itemIndexList);
    for (const AlwaysAtTopData& item : alwaysAtTopItemList) {
        itemIndexList.push_back(item.index);
        atTopIndexList.push_back(item.index);
    }
    for (size_t index : tempItemIndexList) {
        itemIndexList.push_back(index);
    }
    int32_t nShowItemCount = 0;
    int64_t nTotalHeight = 0;
    for (size_t index : itemIndexList) {
        if (index >= dataItemCount) {
            continue;
        }
        const ListCtrlItemData* pRowData = pDataProvider->GetDisplayItemData(index);
        if (pRowData == nullptr) {
            continue;
        }
        nItemHeight = pRowData->nItemHeight;
        if (nItemHeight < 0) {
            //取默认行高
            nItemHeight = nDefaultItemHeight;
        }
        nTotalHeight += nItemHeight;
        if (nTotalHeight < nRectHeight) {
            if (pItemIndexList) {
                pItemIndexList->push_back(index);
            }
            if (pAtTopItemIndexList != nullptr) {
                if (std::find(atTopIndexList.begin(),
                              atTopIndexList.end(), index) != atTopIndexList.end()) {
                    pAtTopItemIndexList->push_back(index);
                }
            }
            ++nShowItemCount;
        }
        else {
            nShowItemCount += 2;
            break;
        }
    }
    return nShowItemCount;
}

int64_t ListCtrlReportView::GetDataItemTotalHeights(size_t itemIndex, bool bIncludeAtTops) const
{
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return 0;
    }
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return 0;
    }
    const int32_t nDefaultItemHeight = m_pListCtrl->GetDataItemHeight(); //默认行高
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = pDataProvider->GetDataItemCount();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData* pRowData = pDataProvider->GetDisplayItemData(index);
        if (pRowData == nullptr) {
            continue;
        }
        const ListCtrlItemData& rowData = *pRowData;
        nItemHeight = (rowData.nItemHeight < 0) ? nDefaultItemHeight : rowData.nItemHeight;
        if (!rowData.bVisible || (nItemHeight == 0)) {
            //不可见的，跳过
            continue;
        }

        if (rowData.nAlwaysAtTop >= 0) {
            //置顶的元素，需要统计在内
            if (bIncludeAtTops) {
                totalItemHeight += nItemHeight;
            }            
        }
        else if (index < itemIndex) {
            //符合要求的元素
            totalItemHeight += nItemHeight;
        }
        else if (!bIncludeAtTops) {
            //已经完成
            break;
        }
    }
    return totalItemHeight;
}

bool ListCtrlReportView::IsNormalMode() const
{
    bool bNormalMode = true;
    ListCtrlData* pDataProvider = m_pData;
    if (pDataProvider != nullptr) {
        bNormalMode = pDataProvider->IsNormalMode();
    }
    return bNormalMode;
}

void ListCtrlReportView::SetAtTopControlIndex(const std::vector<size_t>& atTopControlList)
{
    m_atTopControlList = atTopControlList;
}

void ListCtrlReportView::MoveTopItemsToLast(std::vector<Control*>& items, std::vector<Control*>& atTopItems) const
{
    atTopItems.clear();
    if (items.empty()) {
        return;
    }
    for (size_t index : m_atTopControlList) {
        if (index < items.size()) {
            atTopItems.push_back(items[index]);
        }
    }
    for (int32_t i = (int32_t)m_atTopControlList.size() - 1; i >= 0; --i) {
        size_t index = m_atTopControlList[i];
        if (index < items.size()) {
            items.erase(items.begin() + index);
        }
    }
    for (Control* pControl : atTopItems) {
        items.push_back(pControl);
    }
    ListCtrlHeader* pHeaderCtrl = dynamic_cast<ListCtrlHeader*>(GetItemAt(0));
    if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
        //最后绘制Header控件，避免被其他的覆盖
        Control* pHeader = items.front();
        items.erase(items.begin());
        items.push_back(pHeader);

        //将Header控件添加到列表
        atTopItems.push_back(pHeader);
    }    
    ASSERT(items.size() == m_items.size());
}

void ListCtrlReportView::OnThemeChanged(bool bRedraw)
{
    //主题变化后，单元格的文本样式/颜色需要重新从主题类解析，
    //清除所有行的元素索引绑定，使下次布局时重新填充单元格数据，避免沿用旧主题的样式
    InvalidateRowElementIndexes();
    BaseClass::OnThemeChanged(bRedraw);
}

void ListCtrlReportView::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    //重写VirtualListBox::PaintChild / ScrollBox::PaintChild函数，确保Header正常绘制
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }
    UiRect rcTemp;
    if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
        return;
    }

    const size_t nItemCount = GetItemCount();
    if (nItemCount <= 1) {
        //首行是表头，直接绘制
        BaseClass::PaintChild(pRender, rcPaint);
        return;
    }
    ListCtrlHeader* pHeaderCtrl = dynamic_cast<ListCtrlHeader*>(GetItemAt(0));
    ASSERT(pHeaderCtrl != nullptr);
    if (pHeaderCtrl == nullptr) {
        BaseClass::PaintChild(pRender, rcPaint);
        return;
    }

    //需要调整绘制策略
    ReArrangeChild(false);
    std::vector<Control*> items = m_items;
    //Header和置顶的元素，移动到最后
    std::vector<Control*> atTopItems;
    MoveTopItemsToLast(items, atTopItems);

    UiRect rcTopControls; //所有置顶控件的矩形区域
    for (size_t i = 0; i < atTopItems.size(); ++i) {
        const Control* pTopControl = atTopItems[i];
        if (pTopControl == nullptr) {
            continue;
        }
        if (i == 0) {
            rcTopControls = pTopControl->GetRect();
        }
        else {
            rcTopControls.Union(pTopControl->GetRect());
        }
    }

    //绘制列表项子控件
    //冻结列的处理：冻结列固定显示在列表左侧，不随横向滚动移动
    //绘制 / 命中测试 / 网格线统一复用同一套单元格几何层（BuildCellGeometry）
    const ListCtrlCellGeometry cellGeometry = BuildCellGeometry();

    //单元格选择模式：先绘制选择高亮（在数据项控件之前绘制，避免覆盖单元格文字）
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        if (cellGeometry.bHasFrozen) {
            PaintCellSelection(pRender, cellGeometry, false);
            PaintCellSelection(pRender, cellGeometry, true);
        }
        else {
            PaintCellSelection(pRender, cellGeometry, false);
        }
    }

    //绘制单个控件（bFrozenPass：是否绘制冻结列的部分）
    auto paintControl = [&](Control* pControl, bool bFrozenPass) {
        if ((pControl == nullptr) || !pControl->IsVisible()) {
            return;
        }
        //裁剪区（屏幕坐标）、绘制区（屏幕坐标）、窗口原点偏移，统一由坐标变换推导
        UiRect rcClip;
        UiRect rcNewPaint;
        UiPoint ptOffset;
        cellGeometry.GetPaintPass(bFrozenPass, rcClip, rcNewPaint, ptOffset);
        if (cellGeometry.bHasFrozen && (rcClip.left >= rcClip.right)) {
            return;
        }
        AutoClip alphaClip(pRender, rcClip, IsClip());

        rcNewPaint.Offset(GetRenderOffset().x, GetRenderOffset().y);

        int32_t nClipState = -1;
        if (!atTopItems.empty() &&
            (std::find(atTopItems.begin(), atTopItems.end(), pControl) == atTopItems.end())) {
            UiRect rcControlRect = pControl->GetRect();
            UiRect rUnion;
            if (UiRect::Intersect(rUnion, rcTopControls, rcControlRect)) {
                //有交集，需要设置裁剪，避免绘制置顶元素与其他元素重叠的区域
                nClipState = pRender->SetClip(rUnion, false);
            }
        }

        UiPoint ptOldOrg = pRender->OffsetWindowOrg(ptOffset);
        pControl->AlphaPaint(pRender, rcNewPaint);
        pRender->SetWindowOrg(ptOldOrg);
        if (nClipState >= 0) {
            pRender->ClearClip(nClipState);
        }
    };

    for (Control* pControl : items) {
        if (pControl == nullptr) {
            continue;
        }
        if (cellGeometry.bHasFrozen) {
            //先绘制可滚动区域（非冻结列）
            paintControl(pControl, false);
            //再绘制冻结列（覆盖在最上层）
            paintControl(pControl, true);
        }
        else {
            //没有冻结列：按原有逻辑单趟绘制
            paintControl(pControl, false);
        }
    }

    //网格线的绘制
    PaintGridLines(pRender);

    //绘制滚动条
    ScrollBar* pVScrollBar = GetVScrollBar();
    ScrollBar* pHScrollBar = GetHScrollBar();
    if ((pHScrollBar != nullptr) && pHScrollBar->IsVisible()) {
        pHScrollBar->AlphaPaint(pRender, rcPaint);
    }

    if ((pVScrollBar != nullptr) && pVScrollBar->IsVisible()) {
        pVScrollBar->AlphaPaint(pRender, rcPaint);
    }

    //鼠标框选功能的框选框绘制
    PaintFrameSelection(pRender);
}

ListCtrlReportView::ListCtrlColumnLayout ListCtrlReportView::BuildColumnLayout(int32_t nViewLeft) const
{
    ListCtrlColumnLayout columnLayout;
    const size_t nItemCount = GetItemCount();
    //优先从第一个可见的数据行推导各列的X区间（几何与列边界对齐，含行首CheckBox/内边距等占位宽度）
    for (size_t index = 1; index < nItemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
        if ((pItem == nullptr) || !pItem->IsVisible() || (pItem->GetWidth() <= 0)) {
            continue;
        }
        const size_t nSubItemCount = pItem->GetItemCount();
        for (size_t nSubItem = 0; nSubItem < nSubItemCount; ++nSubItem) {
            ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nSubItem));
            if ((pSubItem == nullptr) || !pSubItem->IsVisible()) {
                continue;
            }
            ListCtrlColumnLayout::Column column;
            column.nColumnId = pSubItem->GetDataColumnId();
            column.nColumnIndex = columnLayout.m_columns.size();
            column.nXBegin = pSubItem->GetRect().left - nViewLeft;
            column.nXEnd = pSubItem->GetRect().right - nViewLeft;
            columnLayout.m_columns.push_back(column);
        }
        //只处理第一个可见的数据行
        break;
    }
    if (!columnLayout.m_columns.empty()) {
        return columnLayout;
    }

    //没有可见的数据行时，从表头推导各列的X区间
    ListCtrlHeader* pHeaderCtrl = dynamic_cast<ListCtrlHeader*>(GetItemAt(0));
    if (pHeaderCtrl != nullptr) {
        const size_t nHeaderItemCount = pHeaderCtrl->GetItemCount();
        for (size_t i = 0; i < nHeaderItemCount; ++i) {
            ListCtrlHeaderItem* pHeaderItem = dynamic_cast<ListCtrlHeaderItem*>(pHeaderCtrl->GetItemAt(i));
            if (pHeaderItem == nullptr) {
                continue;
            }
            ListCtrlColumnLayout::Column column;
            column.nColumnId = pHeaderItem->GetColumnId();
            column.nColumnIndex = columnLayout.m_columns.size();
            column.nXBegin = pHeaderItem->GetRect().left - nViewLeft;
            column.nXEnd = pHeaderItem->GetRect().right - nViewLeft;
            columnLayout.m_columns.push_back(column);
        }
    }
    return columnLayout;
}

ListCtrlReportView::ListCtrlCellGeometry ListCtrlReportView::BuildCellGeometry() const
{
    ListCtrlCellGeometry cellGeometry;
    cellGeometry.rcViewport = GetPosWithoutPadding();
    cellGeometry.scrollPos = GetScrollOffset();
    cellGeometry.columnLayout = BuildColumnLayout(cellGeometry.rcViewport.left);
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetFrozenColumnCount() > 0)) {
        cellGeometry.nFrozenCount = m_pListCtrl->GetFrozenColumnCount();
        //冻结列宽度 = 前N列的总宽度（含行首的CheckBox/内边距等占位宽度，确保冻结区边界与列边界对齐）
        cellGeometry.nFrozenWidth = cellGeometry.columnLayout.GetColumnsWidth(cellGeometry.nFrozenCount);
        //冻结区宽度为0时视为没有冻结列（保持与历史行为一致）
        cellGeometry.bHasFrozen = cellGeometry.nFrozenWidth > 0;
    }
    return cellGeometry;
}

void ListCtrlReportView::PaintGridLines(IRender* pRender)
{
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return;
    }

    UiRect viewRect = GetRect();
    viewRect.Deflate(GetPadding());
    const bool bRectClip = IsClip();
    const bool bRoundClip = IsClip() && ShouldBeRoundRectFill();
    std::unique_ptr<AutoClip> rectClip = CreateRectClip(pRender, viewRect, bRectClip);
    std::unique_ptr<AutoClip> roundClip = CreateRoundClip(pRender, viewRect, bRoundClip);

    float fColumnLineWidth = GetColumnGridLineWidth();//纵向边线宽度        
    float fRowLineWidth = GetRowGridLineWidth();   //横向边线宽度
    UiColor columnLineColor;
    UiColor rowLineColor;
    DString color = GetColumnGridLineColor();
    if (!color.empty()) {
        columnLineColor = GetUiColor(color);
    }
    color = GetRowGridLineColor();
    if (!color.empty()) {
        rowLineColor = GetUiColor(color);
    }

    if ((fColumnLineWidth > 0.001f) && !columnLineColor.IsEmpty()) {
        //绘制纵向网格线        
        int32_t yTop = viewRect.top;
        std::vector<int32_t> xPosList;
        const size_t itemCount = GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            if (index == 0) {
                ListCtrlHeader* pHeader = dynamic_cast<ListCtrlHeader*>(GetItemAt(index));
                if (pHeader != nullptr) {
                    if (pHeader->IsVisible() && (pHeader->GetHeight() > 0)) {
                        //从Header的低端开始画线
                        yTop = pHeader->GetRect().bottom;

                        //以表头的拖动控件为纵向网格线的位置
                        std::vector<UiRect> rcSplitControls;
                        pHeader->GetHeaderSplitControlRect(rcSplitControls);
                        if (!rcSplitControls.empty()) {
                            for (const UiRect& rc : rcSplitControls) {
                                //以每列的列表项表头拖动控件的右侧为画线的X轴起点坐标
                                xPosList.push_back(rc.right);
                            }
                            break;
                        }
                    }
                    continue;
                }
            }
            ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
            if ((pItem == nullptr) || !pItem->IsVisible() || (pItem->GetWidth() <= 0)) {
                continue;
            }
            size_t nSubItemCount = pItem->GetItemCount();
            for (size_t nSubItem = 0; nSubItem < nSubItemCount; ++nSubItem) {
                ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nSubItem));
                if ((pSubItem == nullptr) || !pSubItem->IsVisible() || (pSubItem->GetWidth() <= 0)) {
                    continue;
                }
                UiPoint scrollBoxOffset = pSubItem->GetScrollOffsetInScrollBox();
                UiRect subItemRect = pSubItem->GetRect();
                subItemRect.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);
                //以每列的列表项最右侧为画线的X轴起点坐标
                xPosList.push_back(subItemRect.right);
            }
            break;
        }

        //冻结列的纵向网格线处理：冻结列不随横向滚动移动
        //与绘制复用同一套坐标变换（viewRect 与坐标变换的视口矩形同口径）
        const ListCtrlCellGeometry cellGeometry = BuildCellGeometry();
        const int32_t nFrozenRight = cellGeometry.FrozenRight();

        for (size_t i = 0; i < xPosList.size(); ++i) {
            int32_t xPos = xPosList[i];
            int32_t nClipState = -1;
            if (cellGeometry.bHasFrozen) {
                UiRect rcClip = viewRect;
                if (i < cellGeometry.nFrozenCount) {
                    //冻结列：恢复到不随横向滚动移动的屏幕位置
                    xPos = cellGeometry.GridLineScreenX(xPos, true);
                    rcClip.right = nFrozenRight;
                }
                else {
                    //非冻结列：绘制区域排除冻结列所占区域
                    rcClip.left = nFrozenRight;
                }
                if (rcClip.left >= rcClip.right) {
                    continue;
                }
                nClipState = pRender->SetClip(rcClip, true);
            }
            //横坐标位置放在每个子项控件的右侧部
            float fXPos = (float)xPos - (float)fColumnLineWidth / 2;
            UiPointF pt1(fXPos, (float)yTop);
            UiPointF pt2(fXPos, (float)viewRect.bottom);
            pRender->DrawLine(pt1, pt2, columnLineColor, fColumnLineWidth);
            if (nClipState >= 0) {
                pRender->ClearClip(nClipState);
            }
        }
    }
    if ((fRowLineWidth > 0.001f) && !rowLineColor.IsEmpty()) {
        //绘制横向网格线
        const size_t itemCount = GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
            if ((pItem == nullptr) || !pItem->IsVisible() || (pItem->GetHeight() <= 0)) {
                continue;
            }
            //纵坐标位置放在每个子项控件的底部（Header控件的底部不画线）
            int32_t yPos = pItem->GetRect().bottom;
            int32_t nChildMarginY = 0;
            Layout* pLayout = GetLayout();
            if (pLayout != nullptr) {
                nChildMarginY = pLayout->GetChildMarginY();
            }
            yPos += nChildMarginY / 2;

            UiPointF pt1(viewRect.left, yPos);
            UiPointF pt2(viewRect.right, yPos);
            pRender->DrawLine(pt1, pt2, rowLineColor, fRowLineWidth);
        }
    }
}

void ListCtrlReportView::PaintCellSelection(IRender* pRender, const ListCtrlCellGeometry& cellGeometry, bool bFrozenPass)
{
    if ((pRender == nullptr) || (m_pListCtrl == nullptr) || (m_pData == nullptr)) {
        return;
    }
    if (m_pListCtrl->GetSelectionMode() != ListCtrlSelectionMode::kCell) {
        return;
    }

    //与绘制数据项控件复用同一套坐标变换（裁剪区/绘制区/窗口原点偏移）
    UiRect rcClip;
    UiRect rcNewPaint;
    UiPoint ptOffset;
    cellGeometry.GetPaintPass(bFrozenPass, rcClip, rcNewPaint, ptOffset);
    if (cellGeometry.bHasFrozen && (rcClip.left >= rcClip.right)) {
        return;
    }
    AutoClip alphaClip(pRender, rcClip, IsClip());
    UiPoint ptOldOrg = pRender->OffsetWindowOrg(ptOffset);

    const UiColor fillColor = GetUiColor(_T("#66AACCEE"));
    const UiColor focusColor = GetUiColor(_T("#FF0078D7"));
    const UiColor hoverColor = GetUiColor(_T("#33AACCEE"));

    //逐格判定的当前格
    const ListCtrlCellId currentCell = m_pListCtrl->GetCurrentCell();

    //表头与置顶行固定显示在顶部：其遮挡的区域不绘制单元格高亮，避免高亮覆盖置顶行内容
    UiRect rcTopFixed;
    const bool bHasTopFixed = GetTopFixedItemsRect(rcTopFixed);

    const size_t nItemCount = GetItemCount();
    //首行是表头，从索引1开始遍历数据行
    for (size_t index = 1; index < nItemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
        if ((pItem == nullptr) || !pItem->IsVisible() || (pItem->GetWidth() <= 0)) {
            continue;
        }
        if (!pItem->IsSelectableType()) {
            //置顶行等不可选的行不参与单元格选择高亮
            continue;
        }
        const size_t nRowIndex = pItem->GetDataItemIndex();
        const size_t nSubItemCount = pItem->GetItemCount();
        for (size_t nSubItem = 0; nSubItem < nSubItemCount; ++nSubItem) {
            ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nSubItem));
            if ((pSubItem == nullptr) || !pSubItem->IsVisible() || (pSubItem->GetWidth() <= 0)) {
                continue;
            }
            const size_t nColumnId = pSubItem->GetDataColumnId();
            const size_t nColumnIndex = m_pListCtrl->GetColumnIndex(nColumnId);
            if (nColumnIndex == Box::InvalidIndex) {
                continue;
            }
            const bool bFrozenColumn = cellGeometry.bHasFrozen && (nColumnIndex < cellGeometry.nFrozenCount);
            if (bFrozenColumn != bFrozenPass) {
                continue;
            }
            UiRect rcCell = pSubItem->GetRect();
            if (bHasTopFixed && (rcCell.top < rcTopFixed.bottom)) {
                //单元格被表头/置顶行遮挡：只绘制其未被遮挡的部分，避免高亮覆盖置顶行内容
                rcCell.top = rcTopFixed.bottom;
                if (rcCell.bottom <= rcCell.top) {
                    continue;
                }
            }

            //按行身份判定该单元格是否被选中（O(1)，与显示顺序解耦）
            const bool bSelected = m_pData->IsCellSelected(nRowIndex, nColumnId);
            const bool bCurrent = currentCell.IsValid() &&
                                  (currentCell.nRowIndex == nRowIndex) &&
                                  (currentCell.nColumnId == nColumnId);
            const bool bHovered = m_pListCtrl->IsCellHovered(nRowIndex, nColumnId);
            if (!bSelected && !bCurrent && !bHovered) {
                continue;
            }
            if (bSelected) {
                pRender->FillRect(UiRectF::MakeFromRect(rcCell), fillColor);
            }
            else if (bHovered) {
                //悬停的单元格：浅色填充提示
                pRender->FillRect(UiRectF::MakeFromRect(rcCell), hoverColor);
            }
            if (bCurrent) {
                pRender->DrawRect(UiRectF::MakeFromRect(rcCell), focusColor, 1.0f, false);
            }
        }
    }
    pRender->SetWindowOrg(ptOldOrg);
}

Control* ListCtrlReportView::FindControl(FINDCONTROLPROC Proc, void* pProcData,
                                         uint32_t uFlags, const UiPoint& ptMouse,
                                         const UiPoint& scrollPos)
{
    //重写：ScrollBox::FindControl 函数，让Header/置顶的Item优先被查找到
    //同时处理冻结列：鼠标落在冻结列区域内时，命中测试需要按"不随横向滚动"的坐标计算
    std::vector<Control*> newItems = m_items;
    if ((uFlags & UIFIND_TOP_FIRST) != 0) {
        //Header和置顶的元素，移动到最后
        std::vector<Control*> atTopItems;
        MoveTopItemsToLast(newItems, atTopItems);
    }

    // Check if this guy is valid
    if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
        return nullptr;
    }
    if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
        return nullptr;
    }
    UiPoint boxPt(ptMouse);
    boxPt.Offset(scrollPos);
#ifdef _DEBUG
    if (((uFlags & UIFIND_HITTEST) != 0) && ((uFlags & UIFIND_DRAG_DROP) == 0) && (pProcData != nullptr)) {
        UiPoint ptOrg(*(UiPoint*)pProcData);
        ptOrg.Offset(this->GetScrollOffsetInScrollBox());
        ASSERT(ptOrg == boxPt);
    }
#endif // _DEBUG
    ScrollBar* pVScrollBar = GetVScrollBar();
    ScrollBar* pHScrollBar = GetHScrollBar();
    if ((uFlags & UIFIND_HITTEST) != 0) {
        if (!GetRect().ContainsPt(boxPt)) {
            return nullptr;
        }
        if (!IsMouseChildEnabled() && ((uFlags & UIFIND_TOOLTIP) == 0)) {
            Control* pResult = nullptr;            
            if (pVScrollBar != nullptr) {
                pResult = pVScrollBar->FindControl(Proc, pProcData, uFlags, boxPt);
            }            
            if ((pResult == nullptr) && (pHScrollBar != nullptr)) {
                pResult = pHScrollBar->FindControl(Proc, pProcData, uFlags, boxPt);
            }
            if (pResult == nullptr) {
                pResult = Control::FindControl(Proc, pProcData, uFlags, boxPt);
            }
            return pResult;
        }
    }

    Control* pResult = nullptr;
    if (pVScrollBar != nullptr) {
        pResult = pVScrollBar->FindControl(Proc, pProcData, uFlags, boxPt);
    }
    if ((pResult == nullptr) && (pHScrollBar != nullptr)) {
        pResult = pHScrollBar->FindControl(Proc, pProcData, uFlags, boxPt);
    }
    if (pResult != nullptr) {
        return pResult;
    }

    UiSize boxScrollOffset = GetScrollOffset();
    UiPoint boxScrollPos(boxScrollOffset.cx, boxScrollOffset.cy);
    bool bFrozenHitTestPass = false;
    if ((boxScrollPos.x != 0) && (m_pListCtrl != nullptr) && (m_pListCtrl->GetFrozenColumnCount() > 0)) {
        //存在冻结列且横向已滚动：判断鼠标是否落在冻结列区域内
        const ListCtrlCellGeometry cellGeometry = BuildCellGeometry();
        if (cellGeometry.IsFrozenX(boxPt.x)) {
            //鼠标落在冻结列区域内：冻结列不随横向滚动移动，命中遍历时的横向滚动量按0计
            //(1) 传给子控件的滚动量为(0, scrollY)，与冻结列的坐标空间一致；
            //(2) 同时让子控件的GetScrollOffsetInScrollBox()的横向分量为0，
            //    使框架的坐标约定(自定义数据 + 控件滚动量 == 传入坐标)成立，消除Debug断言；
            //(3) 查找函数按"自定义数据 + 控件滚动量"判定命中，上述两点一致后，
            //    命中结果与冻结列的显示位置一致（单元格tooltip、右键菜单同样受益）
            boxScrollPos.x = 0;
            bFrozenHitTestPass = true;
        }
    }
    m_bFrozenHitTestPass = bFrozenHitTestPass;
    pResult = FindControlInItems(newItems, Proc, pProcData, uFlags, boxPt, boxScrollPos);
    m_bFrozenHitTestPass = false;
    return pResult;
}

UiSize ListCtrlReportView::GetScrollOffset() const
{
    UiSize scrollOffset = BaseClass::GetScrollOffset();
    if (m_bFrozenHitTestPass) {
        //冻结列命中遍历：横向滚动量按0计，保证子控件的坐标系与冻结列的显示位置一致
        scrollOffset.cx = 0;
    }
    return scrollOffset;
}

UiSize ListCtrlReportView::GetScrollOffsetForChild(const PlaceHolder* pChild) const
{
    UiSize scrollOffset = BaseClass::GetScrollOffsetForChild(pChild);
    //冻结列区域内的子控件固定显示在视口左侧、不随横向滚动移动：横向偏移量按0计
    if ((scrollOffset.cx != 0) && IsChildInFrozenColumn(pChild)) {
        scrollOffset.cx = 0;
    }
    return scrollOffset;
}

bool ListCtrlReportView::IsChildInFrozenColumn(const PlaceHolder* pChild) const
{
    if ((pChild == nullptr) || (m_pListCtrl == nullptr)) {
        return false;
    }
    const size_t nFrozenCount = m_pListCtrl->GetFrozenColumnCount();
    if (nFrozenCount == 0) {
        return false;
    }
    //向上查找该控件所属的单元格（数据行的单元格为ListCtrlSubItem，表头单元格为ListCtrlHeaderItem）
    size_t nColumnId = Box::InvalidIndex;
    const PlaceHolder* pControl = pChild;
    while ((pControl != nullptr) && (pControl != this)) {
        const ListCtrlSubItem* pSubItem = dynamic_cast<const ListCtrlSubItem*>(pControl);
        if (pSubItem != nullptr) {
            nColumnId = pSubItem->GetDataColumnId();
            break;
        }
        const ListCtrlHeaderItem* pHeaderItem = dynamic_cast<const ListCtrlHeaderItem*>(pControl);
        if (pHeaderItem != nullptr) {
            nColumnId = pHeaderItem->GetColumnId();
            break;
        }
        pControl = pControl->GetParent();
    }
    if (nColumnId == Box::InvalidIndex) {
        //未找到所属的单元格（例如整行的ListCtrlItem），按默认口径处理
        return false;
    }
    const size_t nColumnIndex = m_pListCtrl->GetColumnIndex(nColumnId);
    return (nColumnIndex != Box::InvalidIndex) && (nColumnIndex < nFrozenCount);
}

Control* ListCtrlReportView::CreateDataItem()
{
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return nullptr;
    }
    ListCtrlItem* pItem = new ListCtrlItem(GetWindow());
    pItem->SetListCtrl(m_pListCtrl);
    pItem->SetClass(m_pListCtrl->GetDataItemClass());
    pItem->SetAutoCheckSelect(m_pListCtrl->IsAutoCheckSelect());
    return pItem;
}

bool ListCtrlReportView::FillDataItem(Control* pControl,
                                     size_t nElementIndex,
                                     const ListCtrlItemData& itemData,
                                     const std::vector<ListCtrlSubItemData2Pair>& subItemList)
{
    //该函数中控制数据元素数据与UI的展示，控制每列的显示宽度
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return false;
    }
    ListCtrlHeader* pHeaderCtrl = m_pListCtrl->GetHeaderCtrl();
    ASSERT(pHeaderCtrl != nullptr);
    if (pHeaderCtrl == nullptr) {
        return false;
    }

    ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(pControl);
    ASSERT(pItem != nullptr);
    if (pItem == nullptr) {
        return false;
    }

    //数据项是否显示CheckBox    
    bool bShowCheckBox = m_pListCtrl->IsDataItemShowCheckBox();
    pItem->SetShowCheckBox(bShowCheckBox);
    pItem->SetEnableControlPadding(false);

    ASSERT(itemData.bVisible);
    bool bItemChecked = itemData.bChecked;
    int32_t nImageId = itemData.nImageId;
    if (!bShowCheckBox) {
        bItemChecked = false;
    }
    pItem->SetChecked(bItemChecked, false);
    pItem->SetImageId(nImageId);

    //设置左侧内边距，避免CheckBox显示与文字显示重叠
    const int32_t nPaddingLeft = pItem->GetItemPaddingLeft();
    UiPadding rcPadding = pItem->GetPadding();
    if (nPaddingLeft != rcPadding.left) {
        rcPadding.left = nPaddingLeft;
        pItem->SetPadding(rcPadding, false);
    }

    //Header控件的内边距
    const UiPadding rcHeaderPadding = pHeaderCtrl->GetPadding();

    // 基本结构: <ListCtrlItem> <ListCtrlSubItem/> ... <ListCtrlSubItem/>  </ListCtrlItem>
    // 附加说明: 1. ListCtrlItem 是 HBox的子类;   
    //          2. 每一列，放置一个ListCtrlSubItem控件
    //          3. ListCtrlSubItem 是LabelBox的子类

    // 详细的结构说明，参见：ListCtrlItem.h

    //按列ID查找该行的单元格数据：列数通常很少（N <= ~10），线性查找比每行构造 std::map 更省，
    //且不产生堆分配（subItemList 已是连续的小容器）
    auto findSubItemData = [&subItemList](size_t nColumnId) -> const ListCtrlSubItemData2* {
        for (const ListCtrlSubItemData2Pair& dataPair : subItemList) {
            if (dataPair.nColumnId == nColumnId) {
                return dataPair.pSubItemData;
            }
        }
        return nullptr;
        };

    //获取需要显示的各个列的属性
    struct ElementData
    {
        size_t nColumnIndex = Box::InvalidIndex;
        size_t nColumnId = Box::InvalidIndex;
        int32_t nColumnWidth = 0;
        const ListCtrlSubItemData2* pStorage = nullptr;
    };
    std::vector<ElementData> elementDataList;
    const size_t nColumnCount = pHeaderCtrl->GetColumnCount();
    elementDataList.reserve(nColumnCount);
    for (size_t nColumnIndex = 0; nColumnIndex < nColumnCount; ++nColumnIndex) {
        ListCtrlHeaderItem* pHeaderItem = pHeaderCtrl->GetColumn(nColumnIndex);
        if ((pHeaderItem == nullptr) || !pHeaderItem->IsColumnVisible()) {
            continue;
        }
        int32_t nColumnWidth = pHeaderCtrl->GetColumnWidth(nColumnIndex);
        if (nColumnWidth < 0) {
            nColumnWidth = 0;
        }
        ElementData data;
        data.nColumnIndex = nColumnIndex;
        data.nColumnId = pHeaderCtrl->GetColumnId(nColumnIndex);        
        data.pStorage = findSubItemData(data.nColumnId);
        if (nColumnIndex == 0) {
            //第1列：确保表头与数据项右侧对齐
            data.nColumnWidth = nColumnWidth + rcHeaderPadding.left - nPaddingLeft;
            if (data.nColumnWidth < 0) {
                data.nColumnWidth = 0;
            }
        }
        else {
            //第2列开始的其他列
            data.nColumnWidth = nColumnWidth;
        }
        elementDataList.push_back(data);
    }

    ASSERT(!elementDataList.empty());
    if (elementDataList.empty()) {
        return false;
    }

    const size_t showColumnCount = elementDataList.size(); //显示的列数
    while (pItem->GetItemCount() > showColumnCount) {
        //移除多余的列
        if (!pItem->RemoveItemAt(pItem->GetItemCount() - 1)) {
            ASSERT(!"RemoveItemAt failed!");
            return false;
        }
    }

    //默认属性
    DString defaultSubItemClass = m_pListCtrl->GetDataSubItemClass();
    ListCtrlSubItem defaultSubItem(m_pListCtrl->GetWindow());
    defaultSubItem.SetClass(defaultSubItemClass);
    //缓存本行默认的状态文本颜色，供该行所有单元格复用，避免每个单元格重复取值
    DString defaultTextColors[kControlStateCount];
    for (int32_t nState = kControlStateNormal; nState < kControlStateCount; ++nState) {
        defaultTextColors[nState] = defaultSubItem.GetStateTextColor((ControlStateType)nState);
    }

    for (size_t nColumn = 0; nColumn < showColumnCount; ++nColumn) {
        const ElementData& elementData = elementDataList[nColumn];
        ListCtrlSubItem* pSubItem = nullptr;
        if (nColumn < pItem->GetItemCount()) {
            pSubItem = static_cast<ListCtrlSubItem*>(pItem->GetItemAt(nColumn));
            ASSERT(pSubItem != nullptr);
            if (pSubItem == nullptr) {
                return false;
            }
        }
        else {
            pSubItem = new ListCtrlSubItem(GetWindow());
            pSubItem->SetListCtrlItem(pItem);
            pSubItem->SetListBoxItem(pItem);
            pItem->AddItem(pSubItem);
            if (!defaultSubItemClass.empty()) {
                pSubItem->SetClass(defaultSubItemClass);
            }
            pSubItem->AttachMouseEnter([this, pItem, pSubItem](const EventArgs& args) {
                if (m_pListCtrl != nullptr) {
                    EventArgs msg = args;
                    msg.SetSender(m_pListCtrl);
                    msg.eventType = kEventSubItemMouseEnter;
                    msg.listCtrlType = (int32_t)ListCtrlType::Report;
                    msg.wParam = (WPARAM)pItem->GetListBoxIndex();
                    msg.lParam = (LPARAM)pSubItem->GetDataItemIndex();
                    msg.pEventData = pSubItem;
                    m_pListCtrl->FireAllEvents(msg);
                }
                return true;
                });
            pSubItem->AttachMouseLeave([this, pItem, pSubItem](const EventArgs& args) {
                if (m_pListCtrl != nullptr) {
                    EventArgs msg = args;
                    msg.SetSender(m_pListCtrl);
                    msg.eventType = kEventSubItemMouseLeave;
                    msg.listCtrlType = (int32_t)ListCtrlType::Report;
                    msg.wParam = (WPARAM)pItem->GetListBoxIndex();
                    msg.lParam = (LPARAM)pSubItem->GetDataItemIndex();
                    msg.pEventData = pSubItem;
                    m_pListCtrl->FireAllEvents(msg);
                }
                return true;
                });
        }
        //设置不获取焦点
        pSubItem->SetNoFocus();

        //设置关联的列ID
        const size_t nColumnId = elementData.nColumnId;
        pSubItem->SetDataColumnId(nColumnId);

        //设置可编辑属性
        const EventCallbackID callbackID = (EventCallbackID)this;
        bool bEditable = (elementData.pStorage != nullptr) ? elementData.pStorage->bEditable : false;
        if (bEditable && m_pListCtrl->IsEnableItemEdit()) {            
            pSubItem->SetEnableEdit(true);
            pSubItem->DetachEventByID(kEventEnterEdit, callbackID);
            pSubItem->AttachEvent(kEventEnterEdit, [this, nElementIndex, nColumnId, pItem, pSubItem](const EventArgs& /*args*/) {
                if (m_pListCtrl != nullptr) {
                    m_pListCtrl->OnItemEnterEditMode(nElementIndex, nColumnId, pItem, pSubItem);
                }
                return true;
                }, callbackID);
        }
        else {
            pSubItem->SetEnableEdit(false);
            pSubItem->DetachEventByID(kEventEnterEdit, callbackID);
        }

        //填充数据，设置属性        
        pSubItem->SetFixedWidth(UiFixedInt(elementData.nColumnWidth), true, false);
        FillSubItemData(pSubItem, elementData.pStorage, defaultSubItem, defaultTextColors, nColumnId, nElementIndex);
    }
    //给出当前行的数据填充回调
    SendEventReportViewItemFilled(pItem->GetListBoxIndex(), pItem->GetDataItemIndex(), pItem, nullptr);
    return true;
}

void ListCtrlReportView::FillSubItemData(ListCtrlSubItem* pSubItem,
                                         const ListCtrlSubItemData2* pStorage,
                                         const ListCtrlSubItem& defaultSubItem,
                                         const DString* pDefaultTextColors,
                                         size_t nColumnId,
                                         size_t nElementIndex)
{
    ASSERT(pSubItem != nullptr);
    if (pSubItem == nullptr) {
        return;
    }
    ASSERT(pDefaultTextColors != nullptr);
    if (pDefaultTextColors == nullptr) {
        return;
    }
    if (pStorage != nullptr) {
        //先设置富文本属性，再设置文本内容：文本内容需要按富文本方式解析
        pSubItem->SetRichText(pStorage->bRichText);
        pSubItem->SetText(pStorage->text.c_str());
        if (pStorage->nTextFormat != 0) {
            pSubItem->SetTextStyle(pStorage->nTextFormat, false);
        }
        else {
            pSubItem->SetTextStyle(defaultSubItem.GetTextStyle(), false);
        }
        pSubItem->SetTextPadding(defaultSubItem.GetTextPadding(), false);
        for (int32_t nState = kControlStateNormal; nState < kControlStateCount; ++nState) {
            ControlStateType stateType = (ControlStateType)nState;
            pSubItem->SetStateTextColor(stateType, pDefaultTextColors[nState]);
        }
        if (!pStorage->textColor.IsEmpty()) {
            pSubItem->SetStateTextColor(kControlStateNormal, pSubItem->GetColorString(pStorage->textColor));
        }
        if (!pStorage->bkColor.IsEmpty()) {
            pSubItem->SetBkColor(pStorage->bkColor);
        }
        else {
            pSubItem->SetBkColor(defaultSubItem.GetBkColor());
        }
        if (pStorage->bShowCheckBox) {
            //数据声明了CheckBox
            AddSubItemCheckBox(pSubItem, nColumnId, nElementIndex, pStorage->bChecked);
        }
        else {
            RemoveSubItemCheckBox(pSubItem);
        }
        pSubItem->SetImageId(pStorage->nImageId);
        pSubItem->SetProgress(pStorage->nProgress);
    }
    else {
        pSubItem->SetRichText(false);
        pSubItem->SetTextStyle(defaultSubItem.GetTextStyle(), false);
        pSubItem->SetText(defaultSubItem.GetText());
        pSubItem->SetTextPadding(defaultSubItem.GetTextPadding(), false);
        for (int32_t nState = kControlStateNormal; nState < kControlStateCount; ++nState) {
            ControlStateType stateType = (ControlStateType)nState;
            pSubItem->SetStateTextColor(stateType, pDefaultTextColors[nState]);
        }
        pSubItem->SetBkColor(defaultSubItem.GetBkColor());
        RemoveSubItemCheckBox(pSubItem);
        pSubItem->SetImageId(-1);
        pSubItem->SetProgress(-1);
    }
}

void ListCtrlReportView::SendEventReportViewItemFilled(size_t nListBoxIndex, size_t nElementIndex,
                                                       ListCtrlItem* pItem, ListCtrlSubItem* pSubItem)
{
    EventArgs msg;
    msg.SetSender(this);
    msg.eventType = kEventReportViewItemFilled;
    msg.vkCode = VirtualKeyCode::kVK_None;
    msg.wParam = (WPARAM)nListBoxIndex;
    msg.lParam = (LPARAM)nElementIndex;
    Window* pWindow = GetWindow();
    if (pWindow != nullptr) {
        msg.ptMouse = pWindow->GetLastMousePos();
    }
    msg.modifierKey = 0;
    msg.eventData = 0;
    msg.pEventData = pItem;
    msg.pExtraData = pSubItem;
    msg.listCtrlType = (int32_t)ListCtrlType::Report;

    //派发消息
    SendEventMsg(msg);
}

void ListCtrlReportView::AddSubItemCheckBox(ListCtrlSubItem* pSubItem, size_t nColumnId, size_t nElementIndex, bool bChecked)
{
    ASSERT(pSubItem != nullptr);
    if (pSubItem == nullptr) {
        return;
    }
    //添加CheckBox
    pSubItem->SetShowCheckBox(true);
    //挂载CheckBox的事件处理（先解绑，避免重复挂载）
    pSubItem->DetachEvent(kEventCheck);
    pSubItem->DetachEvent(kEventUnCheck);
    pSubItem->SetChecked(bChecked, false);
    pSubItem->AttachCheck([this, nColumnId, nElementIndex](const EventArgs& /*args*/) {
        OnSubItemColumnChecked(nElementIndex, nColumnId, true);
        return true;
        });
    pSubItem->AttachUnCheck([this, nColumnId, nElementIndex](const EventArgs& /*args*/) {
        OnSubItemColumnChecked(nElementIndex, nColumnId, false);
        return true;
        });
}

void ListCtrlReportView::RemoveSubItemCheckBox(ListCtrlSubItem* pSubItem)
{
    ASSERT(pSubItem != nullptr);
    if (pSubItem == nullptr) {
        return;
    }
    if (!pSubItem->IsShowCheckBox()) {
        //当前未显示CheckBox，无需移除（避免多余的ClearStateImages与事件解绑）
        return;
    }
    pSubItem->SetShowCheckBox(false);
    pSubItem->DetachEvent(kEventCheck);
    pSubItem->DetachEvent(kEventUnCheck);
    pSubItem->SetChecked(false, false);
}

bool ListCtrlReportView::RefreshSubItemData(size_t nElementIndex, size_t nColumnId)
{
    if ((m_pListCtrl == nullptr) || (m_pData == nullptr)) {
        return false;
    }
    if (!Box::IsValidItemIndex(nElementIndex) || (nElementIndex >= m_pData->GetDataItemCount())) {
        return false;
    }
    if (!m_pData->IsValidDataColumnId(nColumnId)) {
        return false;
    }
    //获取该单元格的数据
    const ListCtrlData::Storage* pStorage = m_pData->GetSubItemStorage(nElementIndex, nColumnId);

    //获取该数据元素当前对应的UI控件
    size_t nItemIndex = GetDisplayItemIndex(nElementIndex);
    if (!Box::IsValidItemIndex(nItemIndex)) {
        //该数据元素当前不在可见范围内，无需更新界面（数据本身已更新）
        return false;
    }
    ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(nItemIndex));
    if (pItem == nullptr) {
        return false;
    }

    //查找该列对应的子项控件
    ListCtrlSubItem* pSubItem = nullptr;
    const size_t nSubItemCount = pItem->GetItemCount();
    for (size_t nIndex = 0; nIndex < nSubItemCount; ++nIndex) {
        ListCtrlSubItem* pItemSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nIndex));
        if ((pItemSubItem != nullptr) && (pItemSubItem->GetDataColumnId() == nColumnId)) {
            pSubItem = pItemSubItem;
            break;
        }
    }
    if (pSubItem == nullptr) {
        //该列的控件不存在（比如该列被隐藏了），或者该单元格当前不在可见范围内
        return false;
    }

    //默认属性
    DString defaultSubItemClass = m_pListCtrl->GetDataSubItemClass();
    ListCtrlSubItem defaultSubItem(m_pListCtrl->GetWindow());
    defaultSubItem.SetClass(defaultSubItemClass);
    //缓存默认的状态文本颜色，供该单元格复用
    DString defaultTextColors[kControlStateCount];
    for (int32_t nState = kControlStateNormal; nState < kControlStateCount; ++nState) {
        defaultTextColors[nState] = defaultSubItem.GetStateTextColor((ControlStateType)nState);
    }

    //仅更新该单元格的数据，不改变其宽度和事件绑定
    FillSubItemData(pSubItem, pStorage, defaultSubItem, defaultTextColors, nColumnId, nElementIndex);
    pSubItem->Invalidate();
    //给出该单元格的数据填充回调（与整行填充保持一致，便于应用动态定制单元格的显示内容）
    SendEventReportViewItemFilled(pItem->GetListBoxIndex(), nElementIndex, pItem, pSubItem);
    return true;
}

int32_t ListCtrlReportView::GetMaxDataItemWidth(const std::vector<const ListCtrlSubItemData2*>& subItemList)
{
    int32_t nMaxWidth = -1;
    if (m_pListCtrl == nullptr) {
        return nMaxWidth;
    }

    //默认属性
    ListCtrlItem defaultItem(m_pListCtrl->GetWindow());
    defaultItem.SetListCtrl(m_pListCtrl);
    defaultItem.SetClass(m_pListCtrl->GetDataItemClass());

    DString defaultSubItemClass = m_pListCtrl->GetDataSubItemClass();
    ListCtrlSubItem defaultSubItem(m_pListCtrl->GetWindow());
    defaultSubItem.SetClass(defaultSubItemClass);
    defaultSubItem.SetListCtrlItem(&defaultItem);

    ListCtrlSubItem subItem(m_pListCtrl->GetWindow());
    subItem.SetClass(defaultSubItemClass);
    subItem.SetListCtrlItem(&defaultItem);

    for (const ListCtrlSubItemData2* pStorage : subItemList) {
        if (pStorage == nullptr) {
            continue;
        }
        if (pStorage->text.empty()) {
            continue;
        }

        subItem.SetText(pStorage->text.c_str());
        if (pStorage->nTextFormat != 0) {
            subItem.SetTextStyle(pStorage->nTextFormat, false);
        }
        else {
            subItem.SetTextStyle(defaultSubItem.GetTextStyle(), false);
        }
        subItem.SetTextPadding(defaultSubItem.GetTextPadding(), false);
        subItem.SetShowCheckBox(pStorage->bShowCheckBox);
        subItem.SetImageId(pStorage->nImageId);
        subItem.SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
        subItem.SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
        subItem.SetReEstimateSize(true);
        UiEstSize sz = subItem.EstimateSize(UiSize(0, 0));
        nMaxWidth = std::max(nMaxWidth, sz.cx.GetInt32());
    }

    if (nMaxWidth <= 0) {
        nMaxWidth = -1;
    }
    else {
        //增加一点余量
        nMaxWidth += Dpi().GetScaleInt(4);
    }
    return nMaxWidth;
}

void ListCtrlReportView::AdjustSubItemWidth(const std::map<size_t, int32_t>& subItemWidths)
{
    if (subItemWidths.empty()) {
        return;
    }
    size_t itemCount = GetItemCount();
    for (size_t index = 1; index < itemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
        if (pItem == nullptr) {
            continue;
        }
        size_t columnCount = pItem->GetItemCount();
        for (size_t nColumn = 0; nColumn < columnCount; ++nColumn) {
            auto iter = subItemWidths.find(nColumn);
            if (iter != subItemWidths.end()) {
                int32_t nColumnWidth = iter->second;
                if (nColumnWidth < 0) {
                    nColumnWidth = 0;
                }
                ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nColumn));
                if (pSubItem != nullptr) {
                    pSubItem->SetFixedWidth(UiFixedInt(nColumnWidth), true, false);
                }
            }
        }
    }
}

void ListCtrlReportView::OnSubItemColumnChecked(size_t nElementIndex, size_t nColumnId, bool bChecked)
{
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider != nullptr) {
        pDataProvider->SetSubItemCheck(nElementIndex, nColumnId, bChecked, false);
        //更新表头对应列的勾选项状态
        if (m_pListCtrl != nullptr) {
            m_pListCtrl->UpdateHeaderColumnCheckBox(nColumnId);
        }
    }
}

size_t ListCtrlReportView::GetDisplayItemCount(bool /*bIsHorizontal*/, size_t& nColumns, size_t& nRows) const
{
    nColumns = 1;
    size_t nDiplayItemCount = m_diplayItemIndexList.size();
    size_t nAtTopItemCount = m_atTopControlList.size();
    nRows = nDiplayItemCount;
    if (nRows > nAtTopItemCount) {
        nRows -= nAtTopItemCount;//减去置顶项
    }
    if (nRows > 1) {
        if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetHeaderHeight() > 0)) {
            nRows -= 1;//减去Header
        }
    }
    return nRows * nColumns;
}

bool ListCtrlReportView::IsSelectableElement(size_t nElementIndex) const
{
    bool bSelectable = true;
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider != nullptr) {
        bSelectable = pDataProvider->IsSelectableElement(nElementIndex);
    }
    return bSelectable;
}

size_t ListCtrlReportView::FindSelectableElement(size_t nElementIndex, bool bForward) const
{
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return nElementIndex;
    }
    const size_t nElementCount = pDataProvider->GetElementCount();
    if ((nElementCount == 0) || (nElementIndex >= nElementCount)) {
        return Box::InvalidIndex;
    }
    if (!pDataProvider->IsSelectableElement(nElementIndex)) {
        size_t nStartIndex = nElementIndex;
        nElementIndex = Box::InvalidIndex;
        if (bForward) {
            //向前查找下一个不是置顶的
            for (size_t i = nStartIndex + 1; i < nElementCount; ++i) {
                if (pDataProvider->IsSelectableElement(i)) {
                    nElementIndex = i;
                    break;
                }
            }
        }
        else {
            //向后查找下一个不是置顶的
            for (int32_t i = (int32_t)nStartIndex - 1; i >= 0; --i) {
                if (pDataProvider->IsSelectableElement(i)) {
                    nElementIndex = i;
                    break;
                }
            }
        }        
    }
    return nElementIndex;
}

void ListCtrlReportView::GetScrollDeltaValue(int32_t& nHScrollValue, int32_t& nVScrollValue) const
{
    nHScrollValue = DUI_NOSET_VALUE;
    nVScrollValue = DUI_NOSET_VALUE;
    int32_t deltaValue = DUI_NOSET_VALUE;
    if (m_pListCtrl != nullptr) {
        deltaValue = m_pListCtrl->GetDataItemHeight() * 2;
    }
    if (deltaValue > 0) {
        deltaValue = std::max(GetRect().Height() / 3, deltaValue);
        nVScrollValue = deltaValue;
    }
}

bool ListCtrlReportView::CanPaintSelectedColors(bool bHasStateImages) const
{
    //行选中背景色是行选择模式专属视觉；单元格选择模式下屏蔽，
    //使行选择状态（无论由鼠标/键盘/API 哪个入口改动）都不会显示到界面，看不出行可选状态
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        return false;
    }
    return BaseClass::CanPaintSelectedColors(bHasStateImages);
}

bool ListCtrlReportView::OnLButtonClickedBlank()
{
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        //单元格选择模式下：空白处点击清空单元格选择（不涉及行选择，故返回false避免误报行选择变化事件）
        m_pListCtrl->ClearCellSelection();
        return false;
    }
    return BaseClass::OnLButtonClickedBlank();
}

bool ListCtrlReportView::OnRButtonClickedBlank()
{
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        //单元格选择模式下：空白处点击清空单元格选择（不涉及行选择，故返回false避免误报行选择变化事件）
        m_pListCtrl->ClearCellSelection();
        return false;
    }
    return BaseClass::OnRButtonClickedBlank();
}

bool ListCtrlReportView::OnFrameSelection(int64_t left, int64_t right, int64_t top, int64_t bottom)
{
    if ((m_pListCtrl != nullptr) && (m_pListCtrl->GetSelectionMode() == ListCtrlSelectionMode::kCell)) {
        //单元格选择模式：按框选矩形命中单元格
        return OnFrameSelectionForCells(left, right, top, bottom);
    }
    if (left > GetListCtrlWidth()) {
        //在空白处，不做框选处理，只是取消所有选择项
        bool bRet = SetSelectNone();
        if (bRet) {
            OnSelectStatusChanged();
        }        
        return bRet;
    }
    ASSERT(top <= bottom);
    if (top > bottom) {
        return false;
    }
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return false;
    }
    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return false;
    }
    const size_t dataItemCount = pDataProvider->GetDataItemCount();
    if (dataItemCount == 0) {
        return false;
    }

    const int32_t nDefaultItemHeight = m_pListCtrl->GetDataItemHeight(); //默认行高
    int32_t nTopItemHeights = m_pListCtrl->GetHeaderHeight(); //Header与置顶元素所占有的高度

    std::vector<size_t> itemIndexList;

    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;    
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData* pRowData = pDataProvider->GetDisplayItemData(index);
        if (pRowData == nullptr) {
            continue;
        }
        const ListCtrlItemData& rowData = *pRowData;
        nItemHeight = (rowData.nItemHeight < 0) ? nDefaultItemHeight : rowData.nItemHeight;
        if (!rowData.bVisible || (nItemHeight == 0)) {
            //不可见的，跳过
            continue;
        }

        if (rowData.nAlwaysAtTop >= 0) {
            //置顶的元素
            nTopItemHeights += nItemHeight;
            continue;
        }
    }

    top -= nTopItemHeights;
    bottom -= nTopItemHeights;
    if (top < 0) {
        top = 0;
    }
    if (bottom < 0) {
        bottom = 0;
    }
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData* pRowData = pDataProvider->GetDisplayItemData(index);
        if (pRowData == nullptr) {
            continue;
        }
        const ListCtrlItemData& rowData = *pRowData;
        nItemHeight = (rowData.nItemHeight < 0) ? nDefaultItemHeight : rowData.nItemHeight;
        if (!rowData.bVisible || (nItemHeight == 0)) {
            //不可见的，跳过
            continue;
        }

        if (rowData.nAlwaysAtTop >= 0) {
            //置顶的元素，排除掉
            continue;
        }
        totalItemHeight += nItemHeight;
        if (totalItemHeight > top) {
            //开始
            itemIndexList.push_back(index);
        }
        if (totalItemHeight > bottom) {
            //结束
            break;
        }
    }

    //选择框选的数据
    bool bRet = SetSelectedElements(itemIndexList, true);
    if (bRet) {
        OnSelectStatusChanged();
    }    
    return bRet;
}

bool ListCtrlReportView::OnFrameSelectionForCells(int64_t left, int64_t right, int64_t top, int64_t bottom)
{
    if ((m_pListCtrl == nullptr) || (m_pData == nullptr)) {
        return false;
    }
    if (left > right) {
        const int64_t nTemp = left;
        left = right;
        right = nTemp;
    }
    if (top > bottom) {
        const int64_t nTemp = top;
        top = bottom;
        bottom = nTemp;
    }
    //框选矩形换算为与子项控件矩形一致的坐标系：
    //ListBoxHelper 传入的坐标是"内容坐标"（含 ListBox 自身的滚动量），而单元格子控件的矩形是"控件坐标"。
    //纵向采用虚拟滚动布局（控件被复用后排列在可视位置上），控件矩形的位置就是屏幕位置，不随纵向滚动移动，
    //所以纵向需要扣除滚动量后再做命中判定，否则滚动后框选的实际位置会与鼠标框选的位置相差一个滚动量。
    //（横向是真实的滚动条偏移，绘制时按 -偏移量 移动、命中时按 +偏移量 还原，两者口径已一致）
    const UiRect rcView = GetRect();
    const int64_t nScrollPosY = GetScrollPos().cy;
    const int64_t frameLeft = left + (int64_t)rcView.left;
    const int64_t frameRight = right + (int64_t)rcView.left;
    const int64_t frameTop = top + (int64_t)rcView.top - nScrollPosY;
    const int64_t frameBottom = bottom + (int64_t)rcView.top - nScrollPosY;

    //存在冻结列时的横向口径修正：ListBoxHelper 给出的横向坐标统一按"窗口坐标 + 横向滚动量"计算，
    //但冻结列（前 N 列）绘制时横向的窗口原点偏移为 0（即内容坐标等于窗口坐标），不随横向滚动移动。
    //因此对冻结列需把框选矩形左移一个滚动量；两个区域各自裁剪到所属范围，
    //避免框选区域跨冻结边界时误命中另一侧的列（无冻结列时两者与 frameLeft/frameRight 相同）
    const ListCtrlCellGeometry cellGeometry = BuildCellGeometry();
    int64_t frozenFrameLeft = frameLeft;
    int64_t frozenFrameRight = frameRight;
    int64_t scrollFrameLeft = frameLeft;
    int64_t scrollFrameRight = frameRight;
    if (cellGeometry.bHasFrozen) {
        const int64_t nScrollX = cellGeometry.scrollPos.cx;
        const int64_t nFrozenLeft = cellGeometry.rcViewport.left;
        const int64_t nFrozenRight = cellGeometry.FrozenRight();
        //冻结列：矩形左移滚动量，并裁剪到冻结区的横向内容范围
        frozenFrameLeft = std::max<int64_t>(frameLeft - nScrollX, nFrozenLeft);
        frozenFrameRight = std::min<int64_t>(frameRight - nScrollX, nFrozenRight);
        //非冻结列：矩形裁剪到"冻结区右侧 + 滚动量"之后的横向范围
        scrollFrameLeft = std::max<int64_t>(frameLeft, nFrozenRight + nScrollX);
        scrollFrameRight = frameRight;
    }

    //表头与置顶行固定显示在顶部，会遮挡其下方滚动的普通行：这些区域不参与单元格框选
    UiRect rcTopFixed;
    const bool bHasTopFixed = GetTopFixedItemsRect(rcTopFixed);
    if (bHasTopFixed &&
        (frameBottom <= (int64_t)rcTopFixed.bottom) &&
        (frameLeft < (int64_t)rcTopFixed.right) && (frameRight > (int64_t)rcTopFixed.left)) {
        //框选矩形完全落在表头/置顶行区域内：不改变任何单元格的选择状态
        return false;
    }

    //Ctrl+拖动：只追加（命中者置选、未命中者保持原状）；普通拖动：替换（命中者置选、可见未命中者取消）
    const bool bAdditive = m_pListCtrl->m_bCellDragAdditive;
    bool bChanged = false;
    const size_t nItemCount = GetItemCount();
    //首行是表头，从索引1开始遍历数据行
    for (size_t index = 1; index < nItemCount; ++index) {
        ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(GetItemAt(index));
        if ((pItem == nullptr) || !pItem->IsVisible()) {
            continue;
        }
        if (!pItem->IsSelectableType()) {
            //置顶行等不可选的行不参与单元格框选
            continue;
        }
        const size_t nRowIndex = pItem->GetDataItemIndex();
        const size_t nSubItemCount = pItem->GetItemCount();
        for (size_t nSubItem = 0; nSubItem < nSubItemCount; ++nSubItem) {
            ListCtrlSubItem* pSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nSubItem));
            if ((pSubItem == nullptr) || !pSubItem->IsVisible() || (pSubItem->GetWidth() <= 0)) {
                continue;
            }
            const size_t nColumnId = pSubItem->GetDataColumnId();
            const size_t nColumnIndex = m_pListCtrl->GetColumnIndex(nColumnId);
            if (nColumnIndex == Box::InvalidIndex) {
                continue;
            }
            UiRect rcCell = pSubItem->GetRect();
            if (bHasTopFixed && (rcCell.top < rcTopFixed.bottom)) {
                //单元格被表头/置顶行遮挡的部分不参与命中，避免选中置顶行下方被遮挡的区域
                rcCell.top = rcTopFixed.bottom;
                if (rcCell.bottom <= rcCell.top) {
                    continue;
                }
            }
            //按是否为冻结列选用对应区域的横向命中范围（无冻结列时两者相同）
            const bool bFrozenColumn = cellGeometry.bHasFrozen && (nColumnIndex < cellGeometry.nFrozenCount);
            const int64_t nCellFrameLeft = bFrozenColumn ? frozenFrameLeft : scrollFrameLeft;
            const int64_t nCellFrameRight = bFrozenColumn ? frozenFrameRight : scrollFrameRight;
            const bool bHit = (rcCell.right > nCellFrameLeft) && (rcCell.left < nCellFrameRight) &&
                              (rcCell.bottom > frameTop) && (rcCell.top < frameBottom);
            if (bAdditive && !bHit) {
                //追加语义：未命中的单元格保持原有选择状态
                continue;
            }
            if (m_pData->SetCellSelected(nRowIndex, nColumnId, bHit)) {
                bChanged = true;
            }
        }
    }
    if (bChanged) {
        m_pListCtrl->NotifyCellSelectionChanged();
        m_pListCtrl->RefreshCellSelectionView();
    }
    return bChanged;
}

void ListCtrlReportView::OnItemSelectedChanged(size_t iIndex, IListBoxItem* pListBoxItem)
{
    if (!IsEnableUpdateProvider()) {
        return;
    }
    BaseClass::OnItemSelectedChanged(iIndex, pListBoxItem);
    OnSelectStatusChanged();
}

void ListCtrlReportView::OnItemCheckedChanged(size_t /*iIndex*/, IListBoxItem* pListBoxItem)
{
    if (!IsEnableUpdateProvider()) {
        return;
    }
    ASSERT(pListBoxItem != nullptr);
    if (pListBoxItem == nullptr) {
        return;
    }
    ListCtrlItem* pItem = dynamic_cast<ListCtrlItem*>(pListBoxItem);
    if (pItem == nullptr) {
        return;
    }

    ListCtrlData* pDataProvider = m_pData;
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return;
    }
    size_t nElementIndex = pListBoxItem->GetElementIndex();
    bool bCheckChanged = false;
    bool bChecked = pItem->IsChecked();
    pDataProvider->SetDataItemChecked(nElementIndex, bChecked, bCheckChanged);
    if (bCheckChanged) {
        //更新表头的勾选项状态
        if (m_pListCtrl != nullptr) {
            m_pListCtrl->UpdateHeaderCheckBox();
        }
    }
}

void ListCtrlReportView::OnSelectStatusChanged()
{
    if ((m_pListCtrl != nullptr) && m_pListCtrl->IsAutoCheckSelect()) {
        //更新表头的勾选项状态
        m_pListCtrl->UpdateHeaderCheckBox();
    }
}

void ListCtrlReportView::SetRowGridLineWidth(float fLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        fLineWidth = Dpi().GetScaleFloat(fLineWidth);
    }
    if (fLineWidth < 0) {
        fLineWidth = 0;
    }
    if (m_fRowGridLineWidth != fLineWidth) {
        m_fRowGridLineWidth = fLineWidth;
        Invalidate();
    }
}

float ListCtrlReportView::GetRowGridLineWidth() const
{   
    return m_fRowGridLineWidth;
}

void ListCtrlReportView::SetRowGridLineColor(const DString& color)
{
    if (m_rowGridLineColor != color) {
        m_rowGridLineColor = color;
        Invalidate();
    }
}

DString ListCtrlReportView::GetRowGridLineColor() const
{
    return m_rowGridLineColor.c_str();
}

void ListCtrlReportView::SetColumnGridLineWidth(float fLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        fLineWidth = Dpi().GetScaleFloat(fLineWidth);
    }
    if (fLineWidth < 0) {
        fLineWidth = 0;
    }
    if (m_fColumnGridLineWidth != fLineWidth) {
        m_fColumnGridLineWidth = fLineWidth;
        Invalidate();
    }
}

float ListCtrlReportView::GetColumnGridLineWidth() const
{
    return m_fColumnGridLineWidth;
}

void ListCtrlReportView::SetColumnGridLineColor(const DString& color)
{
    if (m_columnGridLineColor != color) {
        m_columnGridLineColor = color;
        Invalidate();
    }
}

DString ListCtrlReportView::GetColumnGridLineColor() const
{
    return m_columnGridLineColor.c_str();
}

////////////////////////////////////////////////////////////////////////
/// ListCtrlReportLayout 的实现

ListCtrlReportLayout::ListCtrlReportLayout():
    m_pDataView(nullptr),
    m_bReserveSet(false)
{
}

void ListCtrlReportLayout::SetDataView(ListCtrlReportView* pDataView)
{
    ASSERT(pDataView != nullptr);
    m_pDataView = pDataView;
}

UiSize64 ListCtrlReportLayout::ArrangeChildren(const std::vector<ui::Control*>& /*items*/, ui::UiRect rc, bool bEstimateOnly)
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return UiSize64();
    }
    DeflatePadding(rc);
    const int32_t nHeaderHeight = GetHeaderHeight();
    int64_t nTotalHeight = GetElementsHeight(Box::InvalidIndex, true) + nHeaderHeight;
    UiSize64 sz(rc.Width(), rc.Height());
    sz.cy = std::max(nTotalHeight, sz.cy);
    m_bReserveSet = false;
    if (nTotalHeight > rc.Height()) {
        //需要出现滚动条，底部预留空间
        sz.cy += GetReserveHeight();
        if (pDataView->GetHScrollBar() != nullptr) {
            sz.cy += pDataView->GetHScrollBar()->GetHeight();
        }
        m_bReserveSet = true;
    }
    sz.cx = std::max(GetItemWidth(), rc.Width()); //允许出现横向滚动条
    if (!bEstimateOnly) {
        LazyArrangeChild(rc);
    }    
    return sz;
}

UiSize64 ListCtrlReportLayout::EstimateLayoutSize(const std::vector<Control*>& items, ui::UiSize szAvailable)
{
    //估算控件大小时（主要是用于宽高为"auto"类型的情况），只估算容器本身的大小，不包含列表数据的大小
    //因为列表实现为虚表，其数据规模较大，不适合用于估算"auto"控件的大小
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
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
        nTotalWidth = GetItemWidth();
        if (nTotalWidth > 0) {
            nTotalWidth += (rcPadding.left + rcPadding.right);
        }
    }
    if (nTotalHeight == 0) {
        nTotalHeight = szAvailable.cy;
    }
    return UiSize64(nTotalWidth, nTotalHeight);
}

void ListCtrlReportLayout::LazyArrangeChild(UiRect rc) const
{
    rc.Validate();
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return;
    }
    UiRect orgRect = rc;

    //清空缓存数据
    pDataView->SetAtTopControlIndex(std::vector<size_t>());
    pDataView->SetTopElementIndex(Box::InvalidIndex);
    pDataView->SetDisplayDataItems(std::vector<size_t>());
    pDataView->SetNormalItemTop(-1);

    if (pDataView->IsNormalMode()) {
        //常规模式
        LazyArrangeChildNormal(rc);
        return;
    }

    const size_t nItemCount = pDataView->GetItemCount();
    if (nItemCount > 0) {
        //第一个元素是表头控件，设置其位置大小
        ListCtrlHeader* pHeaderCtrl = dynamic_cast<ListCtrlHeader*>(pDataView->GetItemAt(0));
        if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
            int32_t nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
            if (nHeaderHeight < 0) {
                nHeaderHeight = 0;
            }
            //表头的宽度
            int32_t nHeaderWidth = std::max(GetItemWidth(), rc.Width());
            if (nHeaderWidth <= 0) {
                nHeaderWidth = rc.Width();
            }
            ui::UiRect rcTile(rc.left, rc.top, rc.left + nHeaderWidth, rc.top + nHeaderHeight);
            pHeaderCtrl->SetPos(rcTile);
            rc.top += nHeaderHeight;
        }
    }
    int32_t nNormalItemTop = rc.top; //普通列表项（非Header、非置顶）的top坐标

    //记录可见的元素索引号列表
    std::vector<size_t> diplayItemIndexList;
    diplayItemIndexList.reserve(nItemCount);

    //滚动条的Y坐标位置
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;

    //计算当前区域能够显示多少条数据
    int32_t nCalcItemCount = 0;
    if (!rc.IsEmpty()) {
        pDataView->GetMaxDataItemsToShow(nScrollPosY, rc.Height());
    }
    if (nCalcItemCount > (int32_t)(nItemCount - 1)) {
        //UI控件的个数不足，重新调整
        pDataView->AjustItemCount();
    }

    //取出需要显示的数据元素序号列表
    std::vector<ListCtrlReportView::ShowItemInfo> showItemIndexList;
    std::vector<ListCtrlReportView::ShowItemInfo> atTopItemIndexList;
    int64_t nPrevItemHeights = 0;
    pDataView->GetDataItemsToShow(nScrollPosY, nItemCount - 1, 
                                  showItemIndexList, atTopItemIndexList, nPrevItemHeights);
    if (showItemIndexList.empty() && atTopItemIndexList.empty()) {
        //没有需要显示的数据
        pDataView->SetScrollVirtualOffsetY(nScrollPosY);
        pDataView->SetTopElementIndex(0);
        pDataView->SetAtTopControlIndex(std::vector<size_t>());
        pDataView->SetDisplayDataItems(diplayItemIndexList);
        pDataView->SetNormalItemTop(nNormalItemTop);
        return;
    }

    // 顶部元素的索引号
    size_t nTopElementIndex = Box::InvalidIndex;
    if (!atTopItemIndexList.empty()) {
        nTopElementIndex = atTopItemIndexList.front().nItemIndex;
    }
    else if (!showItemIndexList.empty()) {
        nTopElementIndex = showItemIndexList.front().nItemIndex;
    }
    pDataView->SetTopElementIndex(nTopElementIndex); 

    //设置虚拟偏移，否则当数据量较大时，rc这个32位的矩形的高度会越界，需要64位整型才能容纳
    pDataView->SetScrollVirtualOffsetY(nScrollPosY);

    //第一条数据Y轴坐标的偏移，需要保持，避免滚动位置变动后，重新刷新界面出现偏差，导致最后一条数据显示不完整
    int32_t yOffset = 0;
    if ((nScrollPosY > 0) && !showItemIndexList.empty()) {
        int32_t nFirstHeight = showItemIndexList.front().nItemHeight;
        if (nFirstHeight > 0) {            
            yOffset = std::abs(nScrollPosY - nPrevItemHeights) % nFirstHeight;
        }
    }
    if ((nScrollPosY > 0) && (nScrollPosY == pDataView->GetScrollRange().cy)) {
        //滚动条已经到底
        if (!showItemIndexList.empty()) {
            size_t nLastItemIndex = showItemIndexList[showItemIndexList.size() - 1].nItemIndex;
            if (nLastItemIndex == pDataView->GetElementCount() - 1) {
                //已经是最后一条记录，确保底部显示完整
                int32_t rcHeights = rc.Height();
                if (m_bReserveSet && (rcHeights > GetReserveHeight())) {
                    rcHeights -= GetReserveHeight();
                    if (pDataView->GetHScrollBar() != nullptr) {
                        rcHeights -= pDataView->GetHScrollBar()->GetHeight();
                    }                    
                }
                for (const auto& info : atTopItemIndexList) {
                    rcHeights -= info.nItemHeight;
                }
                for (int32_t nIndex = (int32_t)showItemIndexList.size() - 1; nIndex >= 0; --nIndex) {
                    const auto& info = showItemIndexList[nIndex];
                    if ((rcHeights - info.nItemHeight) > 0) {
                        rcHeights -= info.nItemHeight;
                    }
                }
                if (rcHeights > 0) {
                    int32_t nFirstHeight = 0;
                    if (!showItemIndexList.empty()) {
                        nFirstHeight = showItemIndexList.front().nItemHeight;
                    }
                    if (nFirstHeight > 0) {
                        yOffset = nFirstHeight - (rcHeights % nFirstHeight);
                    }
                }
            }
        }
    }

    struct ShowItemIndex
    {
        size_t nElementIndex;   //元素索引 
        int32_t nItemHeight;    //元素的高度
        bool bAtTop;            //是否置顶
        int32_t yOffset;        //Y轴偏移量
    };
    std::vector<ShowItemIndex> itemIndexList;
    itemIndexList.reserve(atTopItemIndexList.size() + showItemIndexList.size());
    for (const auto& info : atTopItemIndexList) {
        itemIndexList.push_back({ info.nItemIndex, info.nItemHeight, true, 0});
    }
    for (const auto& info : showItemIndexList) {
        itemIndexList.push_back({ info.nItemIndex, info.nItemHeight, false, yOffset });
        yOffset = 0;    //只有第一个元素设置偏移
    }
    //元素的宽度：所有元素宽度都相同
    const int32_t cx = GetItemWidth(); 
    ASSERT(cx > 0);

    //控件的左上角坐标值
    ui::UiPoint ptTile(rc.left, rc.top);

    VirtualListBox::RefreshDataList refreshDataList;
    refreshDataList.reserve(nItemCount);
    VirtualListBox::RefreshData refreshData;

    UiSize szItem;
    size_t iCount = 0;
    std::vector<size_t> atTopUiItemIndexList;
    atTopUiItemIndexList.reserve(nItemCount);
    //第一个元素是表头控件，跳过填充数据，所以从1开始
    for (size_t index = 1; index < nItemCount; ++index) {
        Control* pControl = pDataView->GetItemAt(index);
        if (pControl == nullptr) {
            continue;
        }

        bool bAlwaysAtTop = false;
        size_t nElementIndex = Box::InvalidIndex;
        bool bFillElement = true;
        if (iCount < itemIndexList.size()) {
            //当前数据元素的索引号
            const ShowItemIndex& showItemIndex = itemIndexList[iCount];
            nElementIndex = showItemIndex.nElementIndex;
            bAlwaysAtTop = showItemIndex.bAtTop;
            szItem.cx = cx;
            szItem.cy = showItemIndex.nItemHeight;
            ASSERT(szItem.cy > 0);

            //设置当前控件的大小和位置
            if (showItemIndex.yOffset > 0) {
                ptTile.y = ptTile.y - showItemIndex.yOffset;
            }
            UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + szItem.cx, ptTile.y + szItem.cy);
            pControl->SetPos(rcTile);
            if (rcTile.top > orgRect.bottom) {
                //如果元素已经不可见，则停止填充数据
                bFillElement = false;                
            }
        }
        else {
            //数据已经展示完成
            bFillElement = false;
        }

        if (nElementIndex >= pDataView->GetElementCount()) {
            bFillElement = false;
        }
        
        if (bFillElement) {
            if (!pControl->IsVisible()) {
                pControl->SetVisible(true);
            }
            //按需填充数据：如果该控件已经绑定了同一个数据元素，则不需要重新填充数据
            //（避免在滚动/骨架屏、主题刷新、重复布局等场景下，对未变化的行做"整表重填"）
            //ListCtrlItem也是IListBoxItem的实现类，此处只做一次类型转换，避免每行重复dynamic_cast
            ListCtrlItem* pListCtrlItem = dynamic_cast<ListCtrlItem*>(pControl);
            const bool bNeedFillData = (pListCtrlItem == nullptr) || (pListCtrlItem->GetElementIndex() != nElementIndex);
            if (bNeedFillData) {
                pDataView->FillElementData(pControl, nElementIndex);

                refreshData.nItemIndex = index;
                refreshData.pControl = pControl;
                refreshData.nElementIndex = nElementIndex;
                refreshDataList.push_back(refreshData);
            }
            diplayItemIndexList.push_back(nElementIndex);

            if (pListCtrlItem != nullptr) {
                //置顶项不允许选择
                pListCtrlItem->SetSelectableType(bAlwaysAtTop ? false : true);
            }
            if (bAlwaysAtTop) {
                //记录置顶项
                atTopUiItemIndexList.push_back(index);
                //记录置顶项的底部坐标
                nNormalItemTop = pControl->GetRect().bottom;
            }
        }
        else {
            if (pControl->IsVisible()) {
                pControl->SetVisible(false);
            }
            ListCtrlItem* pListCtrlItem = dynamic_cast<ListCtrlItem*>(pControl);
            if (pListCtrlItem != nullptr) {
                pListCtrlItem->SetElementIndex(Box::InvalidIndex);
            }
        }

        //切换到下一行
        ptTile.y += szItem.cy + GetChildMarginY();
        ++iCount;
    }

    pDataView->SetAtTopControlIndex(atTopUiItemIndexList);
    pDataView->SetDisplayDataItems(diplayItemIndexList);    
    pDataView->SetNormalItemTop(nNormalItemTop);

    if (!refreshDataList.empty()) {
        pDataView->OnRefreshElements(refreshDataList);
        pDataView->OnFilledElements(refreshDataList);
    }
}

void ListCtrlReportLayout::LazyArrangeChildNormal(UiRect rc) const
{    
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return;
    }

    UiRect orgRect = rc;
    const size_t nItemCount = pDataView->GetItemCount();
    if (nItemCount > 0) {
        //第一个元素是表头控件，设置其位置大小
        Control* pHeaderCtrl = pDataView->GetItemAt(0);
        if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
            int32_t nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
            if (nHeaderHeight < 0) {
                nHeaderHeight = 0;
            }
            int32_t nHeaderWidth = GetElementSize(rc.Width(), 0).cx;
            if (nHeaderWidth <= 0) {
                nHeaderWidth = rc.Width();
            }
            ui::UiRect rcTile(rc.left, rc.top, rc.left + nHeaderWidth, rc.top + nHeaderHeight);
            pHeaderCtrl->SetPos(rcTile);
            rc.top += nHeaderHeight;
            //记录表头的bottom值
            pDataView->SetNormalItemTop(rc.top);
        }
    }

    //元素的宽度：所有元素宽度都相同
    const int32_t cx = GetItemWidth();
    if (cx <= 0) {
        return;
    }

    //元素的高度：所有元素高度都相同
    const int32_t cy = GetItemHeight();
    ASSERT(cy > 0);
    if (cy <= 0) {
        return;
    }

    //记录可见的元素索引号列表
    std::vector<size_t> diplayItemIndexList;
    diplayItemIndexList.reserve(nItemCount);

    // 顶部元素的索引号
    const size_t nTopElementIndex = GetTopElementIndex(orgRect);
    pDataView->SetTopElementIndex(nTopElementIndex);

    //滚动条的Y坐标位置
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;

    //Y轴坐标的偏移，需要保持，避免滚动位置变动后，重新刷新界面出现偏差
    int32_t yOffset = 0;
    if (cy > 0) {
        yOffset = TruncateToInt32(nScrollPosY % cy);
    }

    //设置虚拟偏移，否则当数据量较大时，rc这个32位的矩形的高度会越界，需要64位整型才能容纳
    pDataView->SetScrollVirtualOffsetY(nScrollPosY);

    //控件的左上角坐标值
    ui::UiPoint ptTile(rc.left, rc.top - yOffset);

    VirtualListBox::RefreshDataList refreshDataList;
    refreshDataList.reserve(nItemCount);
    VirtualListBox::RefreshData refreshData;

    size_t iCount = 0;
    //第一个元素是表头控件，跳过填充数据，所以从1开始
    for (size_t index = 1; index < nItemCount; ++index) {
        Control* pControl = pDataView->GetItemAt(index);
        if (pControl == nullptr) {
            continue;
        }
        //当前数据元素的索引号
        const size_t nElementIndex = nTopElementIndex + iCount;

        //设置当前控件的大小和位置
        ui::UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + cx, ptTile.y + cy);
        pControl->SetPos(rcTile);

        // 填充数据        
        if (nElementIndex < pDataView->GetElementCount()) {
            if (!pControl->IsVisible()) {
                pControl->SetVisible(true);
            }
            //按需填充数据：如果该控件已经绑定了同一个数据元素，则不需要重新填充数据
            //（避免在重复布局、主题刷新等场景下，对未变化的行做"整表重填"）
            //ListCtrlItem也是IListBoxItem的实现类，此处只做一次类型转换，避免每行重复dynamic_cast
            ListCtrlItem* pListCtrlItem = dynamic_cast<ListCtrlItem*>(pControl);
            const bool bNeedFillData = (pListCtrlItem == nullptr) || (pListCtrlItem->GetElementIndex() != nElementIndex);
            if (bNeedFillData) {
                pDataView->FillElementData(pControl, nElementIndex);

                refreshData.nItemIndex = index;
                refreshData.pControl = pControl;
                refreshData.nElementIndex = nElementIndex;
                refreshDataList.push_back(refreshData);
            }
            diplayItemIndexList.push_back(nElementIndex);
        }
        else {
            if (pControl->IsVisible()) {
                pControl->SetVisible(false);
            }
            ListCtrlItem* pListCtrlItem = dynamic_cast<ListCtrlItem*>(pControl);
            if (pListCtrlItem != nullptr) {
                pListCtrlItem->SetElementIndex(Box::InvalidIndex);
            }
        }

        ptTile.y += cy + GetChildMarginY();
        ++iCount;
    }
    pDataView->SetDisplayDataItems(diplayItemIndexList);
    if (!refreshDataList.empty()) {
        pDataView->OnRefreshElements(refreshDataList);
        pDataView->OnFilledElements(refreshDataList);
    }
}

size_t ListCtrlReportLayout::AjustMaxItem(UiRect rc) const
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return 1;
    }
    int32_t nItemHeight = GetItemHeight();
    ASSERT(nItemHeight > 0);
    if (nItemHeight <= 0) {
        return 1;
    }
    if (rc.IsEmpty()) {
        return 1;
    }
    int32_t nRows = 0;
    int32_t nHeaderHeight = GetHeaderHeight();
    if (nHeaderHeight > 0) {
        nRows += 1;
        rc.top += nHeaderHeight;
        rc.Validate();
    }

    nRows += rc.Height() / (nItemHeight + GetChildMarginY() / 2);
    //验证并修正
    if (nRows > 1) {
        int32_t calcHeight = nRows * nItemHeight + (nRows - 1) * GetChildMarginY();
        if (calcHeight < rc.Height()) {
            nRows += 1;
        }
    }
    
    if (!pDataView->IsNormalMode()) {
        //非标准模式，需要复杂计算得到结果
        int64_t nScrollPosY = pDataView->GetScrollPos().cy;
        int32_t nCalcRows = pDataView->GetMaxDataItemsToShow(nScrollPosY, rc.Height());
        if (nCalcRows > 0) {
            nCalcRows += 1;
            if (nCalcRows > nRows) {
                nRows = nCalcRows;
            }
        }
    }

    //额外增加1行，确保真实控件填充满整个可显示区域
    nRows += 1;
    return nRows;
}

size_t ListCtrlReportLayout::GetTopElementIndex(UiRect /*rc*/) const
{
    size_t nTopElementIndex = 0;
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return nTopElementIndex;
    }
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;
    if (!pDataView->IsNormalMode()) {
        //非标准模式
        nTopElementIndex = pDataView->GetTopDataItemIndex(nScrollPosY);
    }
    else {
        int32_t nItemHeight = GetItemHeight();
        ASSERT(nItemHeight > 0);
        if (nItemHeight > 0) {
            nTopElementIndex = static_cast<size_t>(nScrollPosY / nItemHeight);
        }
    }
    return nTopElementIndex;
}

bool ListCtrlReportLayout::IsElementDisplay(UiRect rc, size_t iIndex) const
{
    if (!Box::IsValidItemIndex(iIndex)) {
        return false;
    }
    std::vector<size_t> itemIndexList;
    GetDisplayElements(rc, itemIndexList);
    return std::find(itemIndexList.begin(), itemIndexList.end(), iIndex) != itemIndexList.end();
}

void ListCtrlReportLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection) const
{
    GetDisplayElements(rc, collection, nullptr);
}

void ListCtrlReportLayout::GetDisplayElements(UiRect rc, std::vector<size_t>& collection,
                                            std::vector<size_t>* pAtTopItemIndexList) const
{
    collection.clear();
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return;
    }
    if (pDataView->GetItemCount() <= 1) {
        return;
    }
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;
    if (!pDataView->IsNormalMode()) {
        //非标准模式
        pDataView->GetMaxDataItemsToShow(nScrollPosY, rc.Height(), 
                                         &collection, pAtTopItemIndexList);
        return;
    }

    int32_t nItemHeight = GetItemHeight();
    ASSERT(nItemHeight >= 0);
    if (nItemHeight <= 0) {
        return;
    }
    rc.top += GetHeaderHeight();
    rc.Validate();

    size_t min = (size_t)(nScrollPosY / nItemHeight);
    size_t max = min + (size_t)(rc.Height() / nItemHeight);
    size_t nCount = pDataView->GetElementCount();
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

bool ListCtrlReportLayout::NeedReArrange() const
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return false;
    }

    UiRect rcThis = pDataView->GetPos();
    if (rcThis.IsEmpty()) {
        return false;
    }

    int64_t nScrollPosY = pDataView->GetScrollPos().cy;//新滚动条位置
    int64_t nVirtualOffsetY = pDataView->GetScrollVirtualOffset().cy;//原滚动条位置
    //只要滚动位置发生变化，就需要重新布局
    return (nScrollPosY != nVirtualOffsetY);
}

void ListCtrlReportLayout::EnsureVisible(UiRect rc, size_t iIndex, bool bToTop) const
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return;
    }
    if (!Box::IsValidItemIndex(iIndex) || (iIndex >= pDataView->GetElementCount())) {
        return;
    }
    ScrollBar* pVScrollBar = pDataView->GetVScrollBar();
    if (pVScrollBar == nullptr) {
        return;
    }
    if (!bToTop) {
        std::vector<size_t> atTopItemIndexList;
        std::vector<size_t> itemIndexList;
        GetDisplayElements(rc, itemIndexList, &atTopItemIndexList);
        bool bDisplay = std::find(itemIndexList.begin(), itemIndexList.end(), iIndex) != itemIndexList.end();
        bool bFirst = false;
        bool bLast = false;
        if (!itemIndexList.empty()) {
            for (size_t i = 0; i < itemIndexList.size(); ++i) {
                if (std::find(atTopItemIndexList.begin(), 
                              atTopItemIndexList.end(), 
                              itemIndexList[i]) == atTopItemIndexList.end()) {
                    //第一个不置顶的索引
                    bFirst = itemIndexList[i] == iIndex;
                    break;
                }
            }            
            bLast = itemIndexList[itemIndexList.size() - 1] == iIndex;
        }
        if (bDisplay && !bLast && !bFirst) {
            //已经是显示状态
            return;
        }
    }
    const UiSize szElementSize = GetElementSize(0, iIndex); //目标元素的大小
    int64_t nNewTopPos = 0;     //顶部对齐时的位置
    int64_t nNewBottomPos = 0;  //底部对齐时的位置
    if (iIndex > 0) {
        nNewTopPos = GetElementsHeight(iIndex, false);
        if (!bToTop) {
            //底部对齐
            nNewBottomPos = GetElementsHeight(iIndex, false);
            int64_t nNewPosWithTop = GetElementsHeight(iIndex, true);
            int64_t nTopHeights = GetHeaderHeight();
            if (nNewPosWithTop > nNewBottomPos) {
                nTopHeights += (nNewPosWithTop - nNewBottomPos);
            }
            //扣除置顶项的高度、Header的高度和自身高度
            nNewBottomPos -= rc.Height();
            nNewBottomPos += nTopHeights;
            nNewBottomPos += szElementSize.cy;
        }
    }

    if (nNewTopPos < 0) {
        nNewTopPos = 0;
    }
    if (nNewTopPos > pVScrollBar->GetScrollRange()) {
        nNewTopPos = pVScrollBar->GetScrollRange();
    }
    if (nNewBottomPos < 0) {
        nNewBottomPos = 0;
    }
    if (nNewBottomPos > pVScrollBar->GetScrollRange()) {
        nNewBottomPos = pVScrollBar->GetScrollRange();
    }
    ui::UiSize64 scrollPos = pDataView->GetScrollPos();
    int64_t nScrollPosY = scrollPos.cy;
    int64_t nNewPos = nScrollPosY;
    if (bToTop) {
        //顶部对齐
        nNewPos = nNewTopPos;
    }
    else {
        //未指定对齐，智能判断
        int64_t diffTop = nNewTopPos - nScrollPosY;
        int64_t diffBottom = nNewBottomPos - nScrollPosY;
        bool bFullDisplay = false; //是否已经可以完全显示出来（纵向）
        if ((nScrollPosY >= nNewBottomPos) && (nScrollPosY <= nNewTopPos)) {
            if ((std::abs(diffTop) >= szElementSize.cy) && (std::abs(diffBottom) >= szElementSize.cy)) {
                //当前是完全显示的，不需要滚动
                bFullDisplay = true;
            }
        }
        if (!bFullDisplay) {
            if (std::abs(diffTop) > std::abs(diffBottom)) {
                //向上滚动：底部对齐
                nNewPos = nNewBottomPos;
            }
            else {
                //向下滚动：顶部对齐
                nNewPos = nNewTopPos;
            }
        }
    }
    if (scrollPos.cy != nNewPos) {
        scrollPos.cy = nNewPos;
        pDataView->SetScrollPos(scrollPos);
    }    
}

int64_t ListCtrlReportLayout::GetElementsHeight(size_t nCount, bool bIncludeAtTops) const
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return 0;
    }
    if (!Box::IsValidItemIndex(nCount)) {
        nCount = pDataView->GetElementCount();
    }
    if ((nCount == 0) || !Box::IsValidItemIndex(nCount)){
        return 0;
    }
    int64_t nTotalHeight = 0;
    if (!pDataView->IsNormalMode()) {
        //非标准模式
        nTotalHeight = pDataView->GetDataItemTotalHeights(nCount, bIncludeAtTops);
    }
    else {
        int32_t nItemHeight = GetItemHeight();
        ASSERT(nItemHeight > 0);
        if (nItemHeight <= 0) {
            return 0;
        }
        if (nCount <= 1) {
            //只有1行
            nTotalHeight = (int64_t)nItemHeight + GetChildMarginY();
        }
        else {
            int64_t iChildMargin = 0;
            if (GetChildMarginY() > 0) {
                iChildMargin = GetChildMarginY();
            }
            int64_t childMarginTotal = ((int64_t)nCount - 1) * iChildMargin;
            nTotalHeight = nItemHeight * nCount + childMarginTotal;
        }
    }
    return nTotalHeight;
}

UiSize ListCtrlReportLayout::GetElementSize(int32_t rcWidth, size_t nElementIndex) const
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(0);
        return UiSize();
    }
    UiSize szElementSize;
    szElementSize.cx = std::max(GetItemWidth(), rcWidth);
    if (!pDataView->IsNormalMode()) {
        //非标准模式，行高可能各不相同
        szElementSize.cy = pDataView->GetDataItemHeight(nElementIndex);
    }
    else {
        //标准模式，所有行的高度都相同的
        szElementSize.cy = GetItemHeight();
    }    
    return szElementSize;
}

int32_t ListCtrlReportLayout::GetItemWidth() const
{
    //宽度与表头的宽度相同
    int32_t nItemWidth = 0;
    ListCtrlReportView* pDataView = GetDataView();
    if (pDataView != nullptr) {
        nItemWidth = pDataView->GetListCtrlWidth();
    }
    return nItemWidth;
}

int32_t ListCtrlReportLayout::GetItemHeight() const
{
    //所有行的高度相同，并且从配置读取
    int32_t nItemHeight = 0;
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView != nullptr) && (pDataView->m_pListCtrl != nullptr)) {
        nItemHeight = pDataView->m_pListCtrl->GetDataItemHeight();
    }
    return nItemHeight;
}

int32_t ListCtrlReportLayout::GetHeaderHeight() const
{
    int32_t nHeaderHeight = 0;
    ListCtrlReportView* pDataView = GetDataView();
    if (pDataView != nullptr) {
        Control* pHeaderCtrl = nullptr;
        size_t nItemCount = pDataView->GetItemCount();
        if (nItemCount > 0) {
            //第一个元素是表头控件，设置其位置大小
            pHeaderCtrl = pDataView->GetItemAt(0);            
        }
        if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
            nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
        }
    }
    return nHeaderHeight;
}

int32_t ListCtrlReportLayout::GetReserveHeight() const
{
    int32_t nReserveHeight = 8;
    if (m_pDataView != nullptr) {
        m_pDataView->Dpi().ScaleInt(nReserveHeight);
    }
    return nReserveHeight;
}

}//namespace ui

