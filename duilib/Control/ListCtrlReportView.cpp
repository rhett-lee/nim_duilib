#include "ListCtrlReportView.h" 
#include "ListCtrl.h"
#include "duilib/Render/AutoClip.h"

//包含类：ListCtrlReportView / ListCtrlReportLayout

namespace ui
{
ListCtrlReportView::ListCtrlReportView() :
    ListCtrlView(new ListCtrlReportLayout),
    m_pListCtrl(nullptr),
    m_pData(nullptr),
    m_nTopElementIndex(0)
{
    ListCtrlReportLayout* pDataLayout = dynamic_cast<ListCtrlReportLayout*>(GetLayout());
    ASSERT(pDataLayout != nullptr);

    VirtualLayout* pVirtualLayout = pDataLayout;
    ASSERT(pVirtualLayout != nullptr);
    SetVirtualLayout(pVirtualLayout);
    
    if (pDataLayout != nullptr) {
        pDataLayout->SetDataView(this);
    }
    m_nRowGridLineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);
    m_nColumnGridLineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);
}

ListCtrlReportView::~ListCtrlReportView() 
{
}

void ListCtrlReportView::HandleEvent(const EventArgs& msg)
{
    __super::HandleEvent(msg);
    if (m_pListCtrl != nullptr) {
        if ((msg.Type > kEventKeyBegin) && (msg.Type < kEventKeyEnd)) {
            m_pListCtrl->OnViewKeyboardEvents(msg);
        }
        else if ((msg.Type > kEventMouseBegin) && (msg.Type < kEventMouseEnd)) {
            m_pListCtrl->OnViewMouseEvents(msg);
        }
    }
}

void ListCtrlReportView::SetListCtrl(ListCtrl* pListCtrl)
{
    m_pListCtrl = pListCtrl;
}

void ListCtrlReportView::SetDataProvider(VirtualListBoxElement* pProvider)
{
    __super::SetDataProvider(pProvider);
    m_pData = dynamic_cast<ListCtrlData*>(GetDataProvider());
    ASSERT(m_pData != nullptr);
}

void ListCtrlReportView::Refresh()
{
    if ((m_pListCtrl != nullptr) && !m_pListCtrl->IsEnableRefresh()) {
        //刷新功能已经禁止
        return;
    }
    if ((GetWindow() == nullptr) || !HasDataProvider()) {
        return;
    }
    //调整UI控件的个数，以满足显示使用
    AjustItemCount();

    //刷新UI控件的显示内容
    if (GetElementCount() > 0) {
        ReArrangeChild(true);
        Arrange();
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

                //挂载鼠标事件
                AttachMouseEvents(pControl);
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
    const ListCtrlData::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = itemDataList.size();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData& rowData = itemDataList[index];
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
    const ListCtrlData::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    ASSERT(itemIndex < itemDataList.size());
    if (itemIndex < itemDataList.size()) {
        if (itemDataList[itemIndex].nItemHeight >= 0) {
            nItemHeight = itemDataList[itemIndex].nItemHeight;
        }        
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
    ASSERT(maxCount != 0);
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
    
    const ListCtrlData::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = itemDataList.size();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData& rowData = itemDataList[index];
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
    ASSERT(nRectHeight > 0);
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

    const ListCtrlData::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = itemDataList.size();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData& rowData = itemDataList[index];
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
        const ListCtrlItemData& rowData = itemDataList[index];
        nItemHeight = rowData.nItemHeight;
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
    const ListCtrlData::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;
    const size_t dataItemCount = itemDataList.size();
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData& rowData = itemDataList[index];
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
        __super::PaintChild(pRender, rcPaint);
        return;
    }
    ListCtrlHeader* pHeaderCtrl = dynamic_cast<ListCtrlHeader*>(GetItemAt(0));
    if ((pHeaderCtrl == nullptr) || !pHeaderCtrl->IsVisible()) {
        __super::PaintChild(pRender, rcPaint);
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
        if (i == 0) {
            rcTopControls = pTopControl->GetRect();
        }
        else {
            rcTopControls.Union(pTopControl->GetRect());
        }
    }

    //绘制列表项子控件
    for (Control* pControl : items) {
        if (pControl == nullptr) {
            continue;
        }
        if (!pControl->IsVisible()) {
            continue;
        }

        UiSize scrollPos = GetScrollOffset();
        UiRect rcNewPaint = GetPosWithoutPadding();
        AutoClip alphaClip(pRender, rcNewPaint, IsClip());
        rcNewPaint.Offset(scrollPos.cx, scrollPos.cy);
        rcNewPaint.Offset(GetRenderOffset().x, GetRenderOffset().y);

        bool bHasClip = false;
        if (!atTopItems.empty() &&
            (std::find(atTopItems.begin(), atTopItems.end(), pControl) == atTopItems.end())) {            
            UiRect rcControlRect = pControl->GetRect();
            UiRect rUnion;
            if (UiRect::Intersect(rUnion, rcTopControls, rcControlRect)) {
                //有交集，需要设置裁剪，避免绘制置顶元素与其他元素重叠的区域
                pRender->SetClip(rUnion, false);
                bHasClip = true;
            }
        }

        UiPoint ptOffset(scrollPos.cx, scrollPos.cy);
        UiPoint ptOldOrg = pRender->OffsetWindowOrg(ptOffset);
        pControl->AlphaPaint(pRender, rcNewPaint);
        pRender->SetWindowOrg(ptOldOrg);
        if (bHasClip) {
            pRender->ClearClip();
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

void ListCtrlReportView::PaintGridLines(IRender* pRender)
{
    int32_t nColumnLineWidth = GetColumnGridLineWidth();//纵向边线宽度        
    int32_t nRowLineWidth = GetRowGridLineWidth();   //横向边线宽度
    UiColor columnLineColor;
    UiColor rowLineColor;
    std::wstring color = GetColumnGridLineColor();
    if (!color.empty()) {
        columnLineColor = GetUiColor(color);
    }
    color = GetRowGridLineColor();
    if (!color.empty()) {
        rowLineColor = GetUiColor(color);
    }

    if ((nColumnLineWidth > 0) && !columnLineColor.IsEmpty()) {
        //绘制纵向网格线        
        UiRect viewRect = GetRect();
        int32_t yTop = viewRect.top;
        std::vector<int32_t> xPosList;
        const size_t itemCount = GetItemCount();
        for (size_t index = 0; index < itemCount; ++index) {
            if (index == 0) {
                ListCtrlHeader* pHeader = dynamic_cast<ListCtrlHeader*>(GetItemAt(index));
                if (pHeader != nullptr) {
                    if (pHeader->IsVisible() && (pHeader->GetHeight() > 0)) {
                        yTop = pHeader->GetRect().bottom;//从Header的低端开始画线
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
                xPosList.push_back(subItemRect.right);
            }
            break;
        }

        for (int32_t xPos : xPosList) {
            //横坐标位置放在每个子项控件的右侧部            
            UiPoint pt1(xPos, yTop);
            UiPoint pt2(xPos, viewRect.bottom);
            pRender->DrawLine(pt1, pt2, columnLineColor, nColumnLineWidth);
        }
    }
    if ((nRowLineWidth > 0) && !rowLineColor.IsEmpty()) {
        //绘制横向网格线
        UiRect viewRect = GetRect();
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

            UiPoint pt1(viewRect.left, yPos);
            UiPoint pt2(viewRect.right, yPos);
            pRender->DrawLine(pt1, pt2, rowLineColor, nRowLineWidth);
        }
    }
}

Control* ListCtrlReportView::FindControl(FINDCONTROLPROC Proc, LPVOID pProcData,
                                         uint32_t uFlags, const UiPoint& ptMouse,
                                         const UiPoint& scrollPos)
{
    //重写：ScrollBox::FindControl 函数，让Header优先被查找到，只处理含有UIFIND_TOP_FIRST标志的情况
    if ((uFlags & UIFIND_TOP_FIRST) == 0) {
        return __super::FindControl(Proc, pProcData, uFlags, ptMouse, scrollPos);
    }

    std::vector<Control*> newItems = m_items;
    //Header和置顶的元素，移动到最后
    std::vector<Control*> atTopItems;
    MoveTopItemsToLast(newItems, atTopItems);

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
        if (!IsMouseChildEnabled()) {
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
    return FindControlInItems(m_items, Proc, pProcData, uFlags, boxPt, boxScrollPos);
}

Control* ListCtrlReportView::CreateDataItem()
{
    ASSERT(m_pListCtrl != nullptr);
    if (m_pListCtrl == nullptr) {
        return nullptr;
    }
    ListCtrlItem* pItem = new ListCtrlItem;
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
    int32_t nPaddingLeft = pItem->GetItemPaddingLeft();
    pHeaderCtrl->SetPaddingLeftValue(nPaddingLeft);

    //Header控件的内边距, 需要同步给每个列表项控件，保持左侧对齐一致
    const UiPadding rcHeaderPadding = pHeaderCtrl->GetPadding();
    UiPadding rcPadding = pItem->GetPadding();
    if (rcHeaderPadding.left != rcPadding.left) {
        rcPadding.left = rcHeaderPadding.left;
        pItem->SetPadding(rcPadding, false);
    }

    // 基本结构: <ListCtrlItem> <ListCtrlSubItem/> ... <ListCtrlSubItem/>  </ListCtrlItem>
    // 附加说明: 1. ListCtrlItem 是 HBox的子类;   
    //          2. 每一列，放置一个ListCtrlSubItem控件
    //          3. ListCtrlSubItem 是LabelBox的子类

    std::map<size_t, ListCtrlSubItemData2Ptr> subItemDataMap;
    for (const ListCtrlSubItemData2Pair& dataPair : subItemList) {
        subItemDataMap[dataPair.nColumnId] = dataPair.pSubItemData;
    }

    //获取需要显示的各个列的属性
    struct ElementData
    {
        size_t nColumnId = Box::InvalidIndex;
        int32_t nColumnWidth = 0;
        ListCtrlSubItemData2Ptr pStorage;
    };
    std::vector<ElementData> elementDataList;
    const size_t nColumnCount = pHeaderCtrl->GetColumnCount();
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
        data.nColumnId = pHeaderCtrl->GetColumnId(nColumnIndex);
        data.nColumnWidth = nColumnWidth;
        data.pStorage = subItemDataMap[data.nColumnId];
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
    std::wstring defaultSubItemClass = m_pListCtrl->GetDataSubItemClass();
    ListCtrlSubItem defaultSubItem;
    defaultSubItem.SetWindow(m_pListCtrl->GetWindow());
    defaultSubItem.SetClass(defaultSubItemClass);

    for (size_t nColumn = 0; nColumn < showColumnCount; ++nColumn) {
        const ElementData& elementData = elementDataList[nColumn];
        ListCtrlSubItem* pSubItem = nullptr;
        if (nColumn < pItem->GetItemCount()) {
            pSubItem = dynamic_cast<ListCtrlSubItem*>(pItem->GetItemAt(nColumn));
            ASSERT(pSubItem != nullptr);
            if (pSubItem == nullptr) {
                return false;
            }
        }
        else {
            pSubItem = new ListCtrlSubItem;
            pSubItem->SetListCtrlItem(pItem);
            pSubItem->SetListBoxItem(pItem);
            pItem->AddItem(pSubItem);
            if (!defaultSubItemClass.empty()) {
                pSubItem->SetClass(defaultSubItemClass);
            }            
        }
        //设置不获取焦点
        pSubItem->SetNoFocus();

        //设置可编辑属性
        bool bEditable = (elementData.pStorage != nullptr) ? elementData.pStorage->bEditable : false;
        if (bEditable && m_pListCtrl->IsEnableItemEdit()) {
            size_t nColumnId = elementData.nColumnId;
            pSubItem->SetMouseEnabled(true);
            pSubItem->DetachEvent(kEventEnterEdit);
            pSubItem->AttachEvent(kEventEnterEdit, [this, nElementIndex, nColumnId, pItem, pSubItem](const EventArgs& /*args*/) {
                if (m_pListCtrl != nullptr) {
                    m_pListCtrl->OnItemEnterEditMode(nElementIndex, nColumnId, pItem, pSubItem);
                }
                return true;
                });
        }
        else {
            pSubItem->DetachEvent(kEventEnterEdit);
            pSubItem->SetMouseEnabled(false);
        }

        //填充数据，设置属性        
        pSubItem->SetFixedWidth(UiFixedInt(elementData.nColumnWidth), true, false);
        const ListCtrlSubItemData2Ptr& pStorage = elementData.pStorage;
        if (pStorage != nullptr) {
            pSubItem->SetText(pStorage->text.c_str());
            if (pStorage->nTextFormat != 0) {
                pSubItem->SetTextStyle(pStorage->nTextFormat, false);
            }
            else {
                pSubItem->SetTextStyle(defaultSubItem.GetTextStyle(), false);
            }
            pSubItem->SetTextPadding(defaultSubItem.GetTextPadding(), false);
            if (!pStorage->textColor.IsEmpty()) {
                pSubItem->SetStateTextColor(kControlStateNormal, pSubItem->GetColorString(pStorage->textColor));
            }
            else {
                pSubItem->SetStateTextColor(kControlStateNormal, defaultSubItem.GetStateTextColor(kControlStateNormal));
            }
            if (!pStorage->bkColor.IsEmpty()) {
                pSubItem->SetBkColor(pStorage->bkColor);
            }
            else {
                pSubItem->SetBkColor(defaultSubItem.GetBkColor());
            }
            if (pStorage->bShowCheckBox) {
                //添加CheckBox
                pSubItem->SetCheckBoxVisible(true);
                CheckBox* pCheckBox = pSubItem->GetCheckBox();
                ASSERT(pCheckBox != nullptr);

                //挂载CheckBox的事件处理
                if (pCheckBox != nullptr) {
                    pCheckBox->DetachEvent(kEventSelect);
                    pCheckBox->DetachEvent(kEventUnSelect);
                    pCheckBox->SetSelected(pStorage->bChecked);
                    size_t nColumnId = elementData.nColumnId;
                    pCheckBox->AttachSelect([this, nColumnId, nElementIndex](const EventArgs& /*args*/) {
                        OnSubItemColumnChecked(nElementIndex, nColumnId, true);
                        return true;
                        });
                    pCheckBox->AttachUnSelect([this, nColumnId, nElementIndex](const EventArgs& /*args*/) {
                        OnSubItemColumnChecked(nElementIndex, nColumnId, false);
                        return true;
                        });
                }
            }
            else {
                pSubItem->SetCheckBoxVisible(false);
            }
            pSubItem->SetImageId(pStorage->nImageId);
        }
        else {
            pSubItem->SetTextStyle(defaultSubItem.GetTextStyle(), false);
            pSubItem->SetText(defaultSubItem.GetText());
            pSubItem->SetTextPadding(defaultSubItem.GetTextPadding(), false);
            pSubItem->SetStateTextColor(kControlStateNormal, defaultSubItem.GetStateTextColor(kControlStateNormal));
            pSubItem->SetBkColor(defaultSubItem.GetBkColor());
            pSubItem->SetCheckBoxVisible(false);
            pSubItem->SetImageId(-1);
        }
    }
    return true;
}

int32_t ListCtrlReportView::GetMaxDataItemWidth(const std::vector<ListCtrlSubItemData2Ptr>& subItemList)
{
    int32_t nMaxWidth = -1;
    if (m_pListCtrl == nullptr) {
        return nMaxWidth;
    }

    //默认属性
    ListCtrlItem defaultItem;
    defaultItem.SetListCtrl(m_pListCtrl);
    defaultItem.SetWindow(m_pListCtrl->GetWindow());
    defaultItem.SetClass(m_pListCtrl->GetDataItemClass());

    std::wstring defaultSubItemClass = m_pListCtrl->GetDataSubItemClass();
    ListCtrlSubItem defaultSubItem;
    defaultSubItem.SetWindow(m_pListCtrl->GetWindow());
    defaultSubItem.SetClass(defaultSubItemClass);
    defaultSubItem.SetListCtrlItem(&defaultItem);

    ListCtrlSubItem subItem;
    subItem.SetWindow(m_pListCtrl->GetWindow());
    subItem.SetClass(defaultSubItemClass);
    subItem.SetListCtrlItem(&defaultItem);

    for (const ListCtrlSubItemData2Ptr& pStorage : subItemList) {
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
        subItem.SetCheckBoxVisible(pStorage->bShowCheckBox);
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
        nMaxWidth += GlobalManager::Instance().Dpi().GetScaleInt(4);
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

bool ListCtrlReportView::OnFrameSelection(int64_t left, int64_t /*right*/, int64_t top, int64_t bottom)
{
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
    const ListCtrlData::RowDataList& itemDataList = pDataProvider->GetItemDataList();
    const size_t dataItemCount = itemDataList.size();
    if (dataItemCount == 0) {
        return false;
    }

    const int32_t nDefaultItemHeight = m_pListCtrl->GetDataItemHeight(); //默认行高
    int32_t nTopItemHeights = m_pListCtrl->GetHeaderHeight(); //Header与置顶元素所占有的高度

    std::vector<size_t> itemIndexList;

    int64_t totalItemHeight = 0;
    int32_t nItemHeight = 0;    
    for (size_t index = 0; index < dataItemCount; ++index) {
        const ListCtrlItemData& rowData = itemDataList[index];
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
        const ListCtrlItemData& rowData = itemDataList[index];
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

void ListCtrlReportView::OnItemSelectedChanged(size_t iIndex, IListBoxItem* pListBoxItem)
{
    if (!IsEnableUpdateProvider()) {
        return;
    }
    __super::OnItemSelectedChanged(iIndex, pListBoxItem);
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

void ListCtrlReportView::SetRowGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    if (m_nRowGridLineWidth != nLineWidth) {
        m_nRowGridLineWidth = nLineWidth;
        Invalidate();
    }
}

int32_t ListCtrlReportView::GetRowGridLineWidth() const
{
    return m_nRowGridLineWidth;
}

void ListCtrlReportView::SetRowGridLineColor(const std::wstring& color)
{
    if (m_rowGridLineColor != color) {
        m_rowGridLineColor = color;
        Invalidate();
    }
}

std::wstring ListCtrlReportView::GetRowGridLineColor() const
{
    return m_rowGridLineColor.c_str();
}

void ListCtrlReportView::SetColumnGridLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    if (m_nColumnGridLineWidth != nLineWidth) {
        m_nColumnGridLineWidth = nLineWidth;
        Invalidate();
    }
}

int32_t ListCtrlReportView::GetColumnGridLineWidth() const
{
    return m_nColumnGridLineWidth;
}

void ListCtrlReportView::SetColumnGridLineColor(const std::wstring& color)
{
    if (m_columnGridLineColor != color) {
        m_columnGridLineColor = color;
        Invalidate();
    }
}

std::wstring ListCtrlReportView::GetColumnGridLineColor() const
{
    return m_columnGridLineColor.c_str();
}

////////////////////////////////////////////////////////////////////////
/// ListCtrlReportLayout 的实现

ListCtrlReportLayout::ListCtrlReportLayout():
    m_pDataView(nullptr),
    m_bReserveSet(false)
{
    m_nReserveHeight = GlobalManager::Instance().Dpi().GetScaleInt(8);
}

void ListCtrlReportLayout::SetDataView(ListCtrlReportView* pDataView)
{
    ASSERT(pDataView != nullptr);
    m_pDataView = pDataView;
}

UiSize64 ListCtrlReportLayout::ArrangeChild(const std::vector<ui::Control*>& /*items*/, ui::UiRect rc)
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
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
        sz.cy += m_nReserveHeight;
        if (pDataView->GetHScrollBar() != nullptr) {
            sz.cy += pDataView->GetHScrollBar()->GetHeight();
        }
        m_bReserveSet = true;
    }
    sz.cx = std::max(GetItemWidth(), rc.Width()); //允许出现横向滚动条
    LazyArrangeChild(rc);
    return sz;
}

UiSize ListCtrlReportLayout::EstimateSizeByChild(const std::vector<Control*>& /*items*/, ui::UiSize szAvailable)
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return UiSize();
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
    if (size.cx == 0) {
        size.cx = GetItemWidth();
    }
    if (size.cy == 0) {
        size.cy = szAvailable.cy;
    }
    size.Validate();
    return size;
}

void ListCtrlReportLayout::LazyArrangeChild(UiRect rc) const
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
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
    ASSERT(nItemCount > 0);
    if (nItemCount <= 1) {
        //第一个元素是表头        
        return;
    }

    //第一个元素是表头控件，设置其位置大小
    ListCtrlHeader* pHeaderCtrl = dynamic_cast<ListCtrlHeader*>(pDataView->GetItemAt(0));
    if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
        int32_t nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
        if (nHeaderHeight > 0) {
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

    //滚动条的Y坐标位置
    int64_t nScrollPosY = pDataView->GetScrollPos().cy;

    //计算当前区域能够显示多少条数据
    int32_t nCalcItemCount = pDataView->GetMaxDataItemsToShow(nScrollPosY, rc.Height());
    if (nCalcItemCount > (nItemCount - 1)) {
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
                if (m_bReserveSet && (rcHeights > m_nReserveHeight)) {
                    rcHeights -= m_nReserveHeight;
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
    VirtualListBox::RefreshData refreshData;

    UiSize szItem;
    size_t iCount = 0;
    std::vector<size_t> atTopUiItemIndexList;
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
            // 填充数据
            //TODO: 优化代码，避免每次刷新都Fill
            pDataView->FillElement(pControl, nElementIndex);
            diplayItemIndexList.push_back(nElementIndex);

            refreshData.nItemIndex = index;
            refreshData.pControl = pControl;
            refreshData.nElementIndex = nElementIndex;
            refreshDataList.push_back(refreshData);

            ListCtrlItem* pListCtrlItem = dynamic_cast<ListCtrlItem*>(pControl);
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
    }
}

void ListCtrlReportLayout::LazyArrangeChildNormal(UiRect rc) const
{    
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
        return;
    }

    UiRect orgRect = rc;
    const size_t nItemCount = pDataView->GetItemCount();
    if (nItemCount > 0) {
        //第一个元素是表头控件，设置其位置大小
        Control* pHeaderCtrl = pDataView->GetItemAt(0);
        if ((pHeaderCtrl != nullptr) && pHeaderCtrl->IsVisible()) {
            int32_t nHeaderHeight = pHeaderCtrl->GetFixedHeight().GetInt32();
            if (nHeaderHeight > 0) {
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
    }

    //元素的宽度：所有元素宽度都相同
    const int32_t cx = GetItemWidth();
    ASSERT(cx > 0);

    //元素的高度：所有元素高度都相同
    const int32_t cy = GetItemHeight();
    ASSERT(cy > 0);

    //记录可见的元素索引号列表
    std::vector<size_t> diplayItemIndexList;

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
            pDataView->FillElement(pControl, nElementIndex);
            diplayItemIndexList.push_back(nElementIndex);

            refreshData.nItemIndex = index;
            refreshData.pControl = pControl;
            refreshData.nElementIndex = nElementIndex;
            refreshDataList.push_back(refreshData);
        }
        else {
            if (pControl->IsVisible()) {
                pControl->SetVisible(false);
            }
        }

        ptTile.y += cy + GetChildMarginY();
        ++iCount;
    }
    pDataView->SetDisplayDataItems(diplayItemIndexList);
    if (!refreshDataList.empty()) {
        pDataView->OnRefreshElements(refreshDataList);
    }
}

size_t ListCtrlReportLayout::AjustMaxItem(UiRect rc) const
{
    ListCtrlReportView* pDataView = GetDataView();
    if ((pDataView == nullptr) || !pDataView->HasDataProvider()) {
        ASSERT(FALSE);
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
        ASSERT(FALSE);
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
        ASSERT(FALSE);
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
        ASSERT(FALSE);
        return false;
    }
    size_t nItemCount = pDataView->GetItemCount();//界面的显示控件个数（行）
    if (nItemCount == 0) {
        return false;
    }

    if (pDataView->GetElementCount() <= nItemCount) {
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
        ASSERT(FALSE);
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
        ASSERT(FALSE);
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
            nTotalHeight = nItemHeight + GetChildMarginY();
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
        ASSERT(FALSE);
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

}//namespace ui

