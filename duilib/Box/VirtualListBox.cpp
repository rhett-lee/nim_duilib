#include "VirtualListBox.h"
#include "duilib/Core/ScrollBar.h"
#include <algorithm>
#include <set>

namespace ui {

VirtualListBoxElement::VirtualListBoxElement():
    m_pfnCountChangedNotify(),
    m_pfnDataChangedNotify()
{
}

void VirtualListBoxElement::RegNotifys(const DataChangedNotify& dcNotify, const CountChangedNotify& ccNotify)
{
    m_pfnDataChangedNotify = dcNotify;
    m_pfnCountChangedNotify = ccNotify;
}

void VirtualListBoxElement::EmitDataChanged(size_t nStartIndex, size_t nEndIndex)
{
    if (m_pfnDataChangedNotify) {
        m_pfnDataChangedNotify(nStartIndex, nEndIndex);
    }
}

void VirtualListBoxElement::EmitCountChanged()
{
    if (m_pfnCountChangedNotify) {
        m_pfnCountChangedNotify();
    }
}

/////////////////////////////////////////////////////////////////////////////
//
VirtualListBox::VirtualListBox(Window* pWindow, Layout* pLayout)
    : ListBox(pWindow, pLayout)
    , m_pDataProvider(nullptr)
    , m_pVirtualLayout(nullptr)
    , m_nLastNoShiftIndex(0)
    , m_bEnableUpdateProvider(true)
{
    ASSERT(pLayout != nullptr);
}

void VirtualListBox::SetVirtualLayout(VirtualLayout* pVirtualLayout)
{
    ASSERT(pVirtualLayout != nullptr);
    m_pVirtualLayout = pVirtualLayout;
}

VirtualLayout* VirtualListBox::GetVirtualLayout() const
{
    return m_pVirtualLayout;
}

void VirtualListBox::SetDataProvider(VirtualListBoxElement* pProvider)
{
    if ((m_pDataProvider != pProvider) && (m_pDataProvider != nullptr)) {
        //注销原来的关联关系
        m_pDataProvider->RegNotifys(nullptr, nullptr);
    }
    m_pDataProvider = pProvider;
    if (pProvider != nullptr) {
        //同步单选还是多选
        pProvider->SetMultiSelect(IsMultiSelect());

        //注册模型数据变动通知回调
        pProvider->RegNotifys(
            UiBind(&VirtualListBox::OnModelDataChanged, this, std::placeholders::_1, std::placeholders::_2),
            UiBind(&VirtualListBox::OnModelCountChanged, this));
    }
}

VirtualListBoxElement* VirtualListBox::GetDataProvider() const
{
    return m_pDataProvider;
}

bool VirtualListBox::HasDataProvider() const
{
    return (m_pDataProvider != nullptr) && (m_pVirtualLayout != nullptr);
}

bool VirtualListBox::IsMultiSelect() const
{
    return BaseClass::IsMultiSelect();
}

void VirtualListBox::SetMultiSelect(bool bMultiSelect)
{
    bool bOldValue = m_bEnableUpdateProvider;
    m_bEnableUpdateProvider = false;
    bool bChanged = BaseClass::IsMultiSelect() != bMultiSelect;
    BaseClass::SetMultiSelect(bMultiSelect);
    if (m_pDataProvider != nullptr) {
        if (!bChanged) {
            bChanged = m_pDataProvider->IsMultiSelect() != bMultiSelect;
        }
        m_pDataProvider->SetMultiSelect(bMultiSelect);
        if (!bMultiSelect) {
            //切换为单选时，同步单选项
            size_t nCurSel = GetCurSel();
            if (nCurSel < GetItemCount()) {
                size_t nCurSelItemIndex = GetDisplayItemElementIndex(nCurSel);
                if (nCurSelItemIndex != Box::InvalidIndex) {
                    if (!bChanged) {
                        bChanged = !m_pDataProvider->IsElementSelected(nCurSelItemIndex);
                    }
                    m_pDataProvider->SetElementSelected(nCurSelItemIndex, true);
                }
            }
        }
    }
    if (bChanged) {
        Refresh();
    }
    m_bEnableUpdateProvider = bOldValue;
}

Control* VirtualListBox::CreateElement()
{
    Control* pControl = nullptr;
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider != nullptr) {
        pControl = m_pDataProvider->CreateElement(this);
    }
    if (pControl != nullptr) {
        //挂载鼠标事件，转接给List Box本身
        pControl->AttachDoubleClick([this](const EventArgs& args) {
            VSendEvent(args, true);
            return true;
            });
        pControl->AttachClick([this](const EventArgs& args) {
            VSendEvent(args, true);
            return true;
            });
        pControl->AttachRClick([this](const EventArgs& args) {
            VSendEvent(args, true);
            return true;
            });
        pControl->AttachEvent(kEventReturn, [this](const EventArgs& args) {
            VSendEvent(args, true);
            return true;
            });
    }
    return pControl;
}

void VirtualListBox::FillElement(Control* pControl, size_t nElementIndex)
{
    ASSERT(pControl != nullptr);
    if (pControl == nullptr) {
        return;
    }
    IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
    ASSERT(pListBoxItem != nullptr);
    if (pListBoxItem == nullptr) {
        return;
    }
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider == nullptr) {
        return;
    }
    bool bOldValue = m_bEnableUpdateProvider;
    m_bEnableUpdateProvider = false;

    bool bSelected = m_pDataProvider->IsElementSelected(nElementIndex);
    //先更新选择状态，再填充数据，从而避免与Check状态冲突
    pListBoxItem->SetItemSelected(bSelected);
    bool bFilled = m_pDataProvider->FillElement(pControl, nElementIndex);    
    ASSERT_UNUSED_VARIABLE(bFilled);

    //更新元素索引号
    pListBoxItem->SetElementIndex(nElementIndex);
    ASSERT(GetItemIndex(pControl) == pListBoxItem->GetListBoxIndex());
    ASSERT(pListBoxItem->IsSelected() == bSelected);

    m_bEnableUpdateProvider = bOldValue;
}

void VirtualListBox::OnItemSelectedChanged(size_t /*iIndex*/, IListBoxItem* pListBoxItem)
{
    if (!m_bEnableUpdateProvider) {
        return;
    }
    ASSERT(pListBoxItem != nullptr);
    if ((pListBoxItem == nullptr) || (m_pDataProvider == nullptr)) {
        return;
    }
    //更新该元素的选择状态
    bool bSelected = pListBoxItem->IsSelected();
    size_t nElementIndex = pListBoxItem->GetElementIndex();
    if (nElementIndex != Box::InvalidIndex) {
        m_pDataProvider->SetElementSelected(nElementIndex, bSelected);
    }
}

size_t VirtualListBox::GetElementCount() const
{
    ASSERT(m_pDataProvider != nullptr);
    size_t elementCount = 0;
    if (m_pDataProvider != nullptr) {
        elementCount = m_pDataProvider->GetElementCount();
    }
    return elementCount;
}

void VirtualListBox::SetElementSelected(size_t nElementIndex, bool bSelected)
{
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider != nullptr) {
        bool bChanged = m_pDataProvider->IsElementSelected(nElementIndex) != bSelected;
        m_pDataProvider->SetElementSelected(nElementIndex, bSelected);
        if (bChanged) {
            RefreshElements(nElementIndex, nElementIndex);
        }
    }
}

bool VirtualListBox::SetSelectedElements(const std::vector<size_t>& selectedIndexs, bool bClearOthers)
{
    std::vector<size_t> refreshIndexs;
    SetSelectedElements(selectedIndexs, bClearOthers, refreshIndexs);
    if (!refreshIndexs.empty()) {
        RefreshElements(refreshIndexs);
    }
    return !refreshIndexs.empty();
}

void VirtualListBox::SetSelectedElements(const std::vector<size_t>& selectedIndexs,
                                         bool bClearOthers,
                                         std::vector<size_t>& refreshIndexs)
{
    refreshIndexs.clear();
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider == nullptr) {
        return;
    }
    if (!m_pDataProvider->IsMultiSelect()) {
        return;
    }
    std::set<size_t> selectSet;
    for (auto index : selectedIndexs) {
        selectSet.insert(index);
    }
    std::set<size_t> refreshSet;
    std::vector<size_t> oldSelectedIndexs;
    if (bClearOthers) {        
        m_pDataProvider->GetSelectedElements(oldSelectedIndexs);
        if (!oldSelectedIndexs.empty()) {
            for (size_t nElementIndex : oldSelectedIndexs) {
                if (selectSet.find(nElementIndex) != selectSet.end()) {
                    //过滤掉即将选择的
                    continue;
                }
                m_pDataProvider->SetElementSelected(nElementIndex, false);
                refreshSet.insert(nElementIndex);
            }
        }
    }
    for (size_t nElementIndex : selectedIndexs) {
        if (m_pDataProvider->IsElementSelected(nElementIndex)) {
            continue;
        }
        m_pDataProvider->SetElementSelected(nElementIndex, true);
        refreshSet.insert(nElementIndex);
    }
    for (auto index : refreshSet) {
        refreshIndexs.push_back(index);
    }
}

bool VirtualListBox::IsElementSelected(size_t nElementIndex) const
{
    bool bSelected = false;
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider != nullptr) {
        bSelected = m_pDataProvider->IsElementSelected(nElementIndex);
    }
    return bSelected;
}

void VirtualListBox::GetSelectedElements(std::vector<size_t>& selectedIndexs) const
{
    selectedIndexs.clear();
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider != nullptr) {
        m_pDataProvider->GetSelectedElements(selectedIndexs);
    }
}

bool VirtualListBox::SetSelectAll()
{
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider == nullptr) {
        return false;
    }
    if (!m_pDataProvider->IsMultiSelect()) {
        return false;
    }
    std::vector<size_t> selectedIndexs;
    size_t nCount = m_pDataProvider->GetElementCount();
    for (size_t nElementIndex = 0; nElementIndex < nCount; ++nElementIndex) {
        if (IsSelectableElement(nElementIndex) && 
            !m_pDataProvider->IsElementSelected(nElementIndex)) {
            m_pDataProvider->SetElementSelected(nElementIndex, true);
            selectedIndexs.push_back(nElementIndex);
        }
    }
    if (!selectedIndexs.empty()) {
        RefreshElements(selectedIndexs);
    }
    return !selectedIndexs.empty();
}

bool VirtualListBox::SetSelectNone()
{
    std::vector<size_t> refreshIndexs;
    SetSelectNone(refreshIndexs);
    if (!refreshIndexs.empty()) {
        RefreshElements(refreshIndexs);
    }
    return !refreshIndexs.empty();
}

void VirtualListBox::SetSelectNone(std::vector<size_t>& refreshIndexs)
{
    SetSelectNoneExclude(std::vector<size_t>(), refreshIndexs);
}

void VirtualListBox::SetSelectNoneExclude(const std::vector<size_t>& excludeIndexs,
                                          std::vector<size_t>& refreshIndexs)
{
    refreshIndexs.clear();
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider == nullptr) {
        return;
    }
    std::vector<size_t> selectedIndexs;
    m_pDataProvider->GetSelectedElements(selectedIndexs);
    if (!selectedIndexs.empty()) {
        std::set<size_t> indexSet;
        for (size_t nElementIndex : excludeIndexs) {
            indexSet.insert(nElementIndex);
        }
        for (size_t nElementIndex : selectedIndexs) {
            if (!indexSet.empty()) {
                if (indexSet.find(nElementIndex) != indexSet.end()) {
                    //排除
                    continue;
                }
            }
            m_pDataProvider->SetElementSelected(nElementIndex, false);
        }
        refreshIndexs.swap(selectedIndexs);
    }
}

void VirtualListBox::RefreshElements(size_t nStartElementIndex, size_t nEndElementIndex)
{
    OnModelDataChanged(nStartElementIndex, nEndElementIndex);
}

void VirtualListBox::RefreshElements(const std::vector<size_t>& elementIndexs)
{
    if (elementIndexs.empty()) {
        return;
    }
    std::set<size_t> indexSet;
    for (size_t nElementIndex : elementIndexs) {
        indexSet.insert(nElementIndex);
    }
    VirtualListBox::RefreshDataList refreshDataList;
    VirtualListBox::RefreshData refreshData;
    size_t nItemCount = m_items.size();
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        Control* pControl = m_items[nItemIndex];
        if ((pControl == nullptr) || !pControl->IsVisible()) {
            continue;
        }
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem == nullptr) {
            continue;
        }
        size_t nElementIndex = pListBoxItem->GetElementIndex();
        if (nElementIndex != Box::InvalidIndex) {
            if (indexSet.find(nElementIndex) != indexSet.end()) {                
                FillElement(pControl, nElementIndex);
                pControl->Invalidate();

                refreshData.nItemIndex = nItemIndex;
                refreshData.pControl = pControl;
                refreshData.nElementIndex = nElementIndex;
                refreshDataList.push_back(refreshData);
            }
        }
    }
    if (!refreshDataList.empty()) {
        OnRefreshElements(refreshDataList);
    }
}

void VirtualListBox::OnModelDataChanged(size_t nStartElementIndex, size_t nEndElementIndex)
{
    VirtualListBox::RefreshDataList refreshDataList;
    VirtualListBox::RefreshData refreshData;
    size_t nItemCount = m_items.size();
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        Control* pControl = m_items[nItemIndex];
        if ((pControl == nullptr) || !pControl->IsVisible()) {
            continue;
        }
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            size_t nElementIndex = pListBoxItem->GetElementIndex();
            if ((nElementIndex >= nStartElementIndex) &&
                (nElementIndex <= nEndElementIndex)) {
                FillElement(pControl, nElementIndex);
                pControl->Invalidate();

                refreshData.nItemIndex = nItemIndex;
                refreshData.pControl = pControl;
                refreshData.nElementIndex = nElementIndex;
                refreshDataList.push_back(refreshData);
            }
        }
    }
    if (!refreshDataList.empty()) {
        OnRefreshElements(refreshDataList);
    }
}

void VirtualListBox::OnModelCountChanged()
{
    //元素的个数发生变化（有添加或者删除）
    Refresh();
}

bool VirtualListBox::IsEnableUpdateProvider() const
{
    return m_bEnableUpdateProvider;
}

void VirtualListBox::Refresh()
{
    if (!HasDataProvider()) {
        return;
    }
    //最大子项数
    size_t nMaxItemCount = m_pVirtualLayout->AjustMaxItem(GetPosWithoutPadding());
    if (nMaxItemCount == 0) {
        return;
    }

    //当前数据总数
    size_t nElementCount = GetElementCount();

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
            if (itemCount > 0) {
                RemoveItemAt(itemCount - 1);
            }            
        }
    }
    else if (nItemCount < nNewItemCount) {
        //如果现有子项总数小于新计算的子项数，新增比数据总数少的子项
        size_t n = nNewItemCount - nItemCount;
        for (size_t i = 0; i < n; ++i) {
            Control* pControl = CreateElement();
            AddItem(pControl);
        }
    }
    if (nElementCount > 0) {
        ReArrangeChild(true);
        Arrange();
    }
}

void VirtualListBox::GetDisplayElements(std::vector<size_t>& collection) const
{
    collection.clear();
    ASSERT(m_pVirtualLayout != nullptr);
    if (m_pVirtualLayout != nullptr) {
        m_pVirtualLayout->GetDisplayElements(GetPosWithoutPadding(), collection);
    }

#ifdef _DEBUG
    std::vector<size_t> collection2;
    size_t nItemCount = GetItemCount();
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        Control* pControl = GetItemAt(nItemIndex);
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            size_t iElementIndex = pListBoxItem->GetElementIndex();
            ASSERT(iElementIndex != Box::InvalidIndex);
            collection2.push_back(iElementIndex);
        }
    }
    ASSERT(collection2 == collection);
#endif
}

size_t VirtualListBox::GetDisplayItemIndex(size_t nElementIndex) const
{
    size_t nFoundItemIndex = Box::InvalidIndex;
    const size_t nItemCount = GetItemCount();
    for (size_t nItemIndex = 0; nItemIndex < nItemCount; ++nItemIndex) {
        Control* pControl = GetItemAt(nItemIndex);
        if ((pControl == nullptr) || !pControl->IsVisible()) {
            continue;
        }
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            if (nElementIndex == pListBoxItem->GetElementIndex()) {
                nFoundItemIndex = nItemIndex;
                break;
            }
        }
    }
    return nFoundItemIndex;
}

size_t VirtualListBox::GetDisplayItemElementIndex(size_t nItemIndex) const
{
    size_t nElementIndex = Box::InvalidIndex;
    Control* pControl = GetItemAt(nItemIndex);
    if ((pControl != nullptr) && pControl->IsVisible()) {
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            nElementIndex = pListBoxItem->GetElementIndex();
        }
    }
    return nElementIndex;
}

size_t VirtualListBox::GetCurSelElement() const
{
    size_t nElementIndex = Box::InvalidIndex;
    size_t nCurSel = GetCurSel();
    if (nCurSel < GetItemCount()) {
        nElementIndex = GetDisplayItemElementIndex(nCurSel);
    }
    return nElementIndex;
}

void VirtualListBox::EnsureVisible(size_t nElementIndex, bool bToTop)
{
    ASSERT(m_pVirtualLayout != nullptr);
    if (m_pVirtualLayout != nullptr) {
        m_pVirtualLayout->EnsureVisible(GetPosWithoutPadding(), nElementIndex, bToTop);
    }
}

void VirtualListBox::EnsureVisible(const UiRect& rcItem,
                                   ListBoxVerVisible vVisibleType,
                                   ListBoxHorVisible hVisibleType)
{
    BaseClass::EnsureVisible(rcItem, vVisibleType, hVisibleType);
}

size_t VirtualListBox::EnsureVisible(size_t iIndex, ListBoxVerVisible vVisibleType, ListBoxHorVisible hVisibleType)
{
    size_t nNewIndex = iIndex;
    size_t nElementIndex = GetDisplayItemElementIndex(iIndex);
    if (nElementIndex >= GetElementCount()) {
        nNewIndex = BaseClass::EnsureVisible(iIndex, vVisibleType, hVisibleType);
    }
    else {
        EnsureVisible(nElementIndex, false);
        nNewIndex = GetDisplayItemIndex(nElementIndex);
    }
    ASSERT(nNewIndex < GetItemCount());
    return nNewIndex;
}

void VirtualListBox::SetScrollPos(UiSize64 szPos)
{
    bool isChanged = (GetScrollPos().cy != szPos.cy) || (GetScrollPos().cx != szPos.cx);
    ListBox::SetScrollPos(szPos);
    if (isChanged) {
        ReArrangeChild(false);
    }
}

void VirtualListBox::SetPos(ui::UiRect rc)
{
    bool bChange = false;
    if (!GetRect().Equals(rc)) {
        bChange = true;
    }
    ListBox::SetPos(rc);
    if (bChange) {
        Refresh();
    }
}

void VirtualListBox::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
    ReArrangeChild(false);
    BaseClass::PaintChild(pRender, rcPaint);
}

void VirtualListBox::SendEventMsg(const EventArgs& msg)
{
    VSendEvent(msg, false);
}

void VirtualListBox::VSendEvent(const EventArgs& msg, bool bFromItem)
{
    if (bFromItem) {
        EventArgs newMsg = msg;
        newMsg.SetSender(this);
        size_t nItemIndex = GetItemIndex(msg.GetSender());
        if (nItemIndex < GetItemCount()) {
            newMsg.wParam = nItemIndex;
            newMsg.lParam = GetDisplayItemElementIndex(nItemIndex);
        }
        else {
            newMsg.wParam = Box::InvalidIndex;
            newMsg.lParam = Box::InvalidIndex;
        }        
        BaseClass::SendEventMsg(newMsg);
    }
    else if ((msg.eventType == kEventMouseDoubleClick) ||
             (msg.eventType == kEventClick) ||
             (msg.eventType == kEventRClick)) {
        if (msg.GetSender() == this) {
            ASSERT(msg.wParam == 0);
            ASSERT(msg.lParam == 0);
            EventArgs newMsg = msg;
            newMsg.wParam = Box::InvalidIndex;
            newMsg.lParam = Box::InvalidIndex;
            BaseClass::SendEventMsg(newMsg);
        }
        else {
            BaseClass::SendEventMsg(msg);
        }
    }
    else {
        BaseClass::SendEventMsg(msg);
    }
}

bool VirtualListBox::RemoveItem(Control* pControl)
{
    return BaseClass::RemoveItem(pControl);
}

bool VirtualListBox::RemoveItemAt(size_t iIndex)
{
    return BaseClass::RemoveItemAt(iIndex);
}

void VirtualListBox::RemoveAllItems()
{
    return BaseClass::RemoveAllItems();
}

void VirtualListBox::ReArrangeChild(bool bForce)
{
    if (!HasDataProvider()) {
        return;
    }
    if (!bForce) {
        if (!m_pVirtualLayout->NeedReArrange()) {
            return;
        }
    }
    m_pVirtualLayout->LazyArrangeChild(GetPosWithoutPadding());
    ASSERT(!m_pVirtualLayout->NeedReArrange());
}

bool VirtualListBox::OnFindSelectable(size_t nCurSel, SelectableMode mode,
                                      size_t nCount, size_t& nDestItemIndex)
{
    nDestItemIndex = Box::InvalidIndex;
    const size_t nElementCount = GetElementCount();
    const size_t itemCount = GetItemCount();
    if ((nElementCount == 0) || (itemCount == 0)) {
        return false;
    }
    size_t nElementIndex = Box::InvalidIndex;
    size_t nIndex = nCurSel;
    bool bForward = (mode == SelectableMode::kForward) ||
                    (mode == SelectableMode::kHome);
    if (mode == SelectableMode::kSelect) {
        //定位到当前选择的数据
        bool bRet = false;
        std::vector<size_t> selectedIndexs;
        GetSelectedElements(selectedIndexs);
        for (size_t index = 0; index < selectedIndexs.size(); ++index) {
            nElementIndex = selectedIndexs[index];
            if (!IsSelectableElement(nElementIndex)) {
                continue;
            }
            EnsureVisible(nElementIndex, false);
            size_t nSelItemIndex = GetDisplayItemIndex(nElementIndex);
            ASSERT(nSelItemIndex < GetItemCount());            
            if (nSelItemIndex < GetItemCount()) {
                SetCurSel(nSelItemIndex);
                nDestItemIndex = nSelItemIndex;
                bRet = true;
                break;
            }
        }
        return bRet;
    }
    else if (mode == SelectableMode::kHome) {
        //定位到第一条数据
        nElementIndex = 0;
        nIndex = 0;
    }
    else if (mode == SelectableMode::kEnd) {
        //定位到最后一条数据
        nElementIndex = nElementCount - 1;        
        if (itemCount > 0) {
            nIndex = itemCount - 1;
        }
    }
    else if ((mode == SelectableMode::kForward) || 
             (mode == SelectableMode::kBackward)) {
        //向前或者向后定位到第nCount条数据
        if ((nCount == 0) || (nCount == Box::InvalidIndex)) {
            nCount = 1;
        }
        nElementIndex = GetDisplayItemElementIndex(nCurSel);
        if (nElementIndex >= GetElementCount()) {
            //无法确定当前选择的元素索引号
            return false;
        }
        if (bForward) {
            //向前
            nElementIndex += nCount;
            if (nElementIndex >= nElementCount) {
                nElementIndex = nElementCount - 1;
            }
            nIndex = nCurSel + nCount;
            if (nIndex >= itemCount) {
                nIndex = itemCount - 1;
            }
        }
        else {
            //向后
            if (nCount > nElementIndex) {
                //已经到达第1条
                nElementIndex = 0;
            }
            else {
                nElementIndex -= nCount;
            }
            if (nCount > nCurSel) {
                nIndex = 0;
            }
            else {
                nIndex = nCurSel - nCount;
            }
        }
    }
    else {
        //不存在
        ASSERT(0);
        return false;
    }
    if (nElementIndex >= GetElementCount()) {
        //无法确定当前选择的元素索引号
        return false;
    }
    if (!IsSelectableElement(nElementIndex)) {
        nElementIndex = FindSelectableElement(nElementIndex, bForward);
        if (nElementIndex >= GetElementCount()) {
            //无法确定当前选择的元素索引号
            return false;
        }
    }

    bool bLoaded = false;
    if (GetDisplayItemElementIndex(nIndex) != nElementIndex) {
        //加载这个元素, 使其处于可显示状态
        EnsureVisible(nElementIndex, false);
        bLoaded = true;

        //检查并且获取最新的目标控件索引号
        bool bCheckOk = false;
        size_t n = GetItemCount();
        for (size_t i = 0; i < n; ++i) {
            if (GetDisplayItemElementIndex(i) == nElementIndex) {
                nDestItemIndex = i;
                bCheckOk = true;
                break;
            }
        }
        ASSERT(bCheckOk);
        if (!bCheckOk) {
            bLoaded = false;
        }
    }
    return bLoaded;
}

size_t VirtualListBox::GetItemCountBefore(size_t nCurSel)
{
    size_t nElementIndex = GetDisplayItemElementIndex(nCurSel);
    if (nElementIndex == Box::InvalidIndex) {
        nElementIndex = 0;
    }
    return nElementIndex;
}

size_t VirtualListBox::GetItemCountAfter(size_t nCurSel)
{
    size_t nElementIndex = GetDisplayItemElementIndex(nCurSel);
    if (nElementIndex < GetElementCount()) {
        nElementIndex = GetElementCount() - nElementIndex - 1;
    }
    else {
        nElementIndex = 0;
    }
    return nElementIndex;
}

bool VirtualListBox::IsSelectableElement(size_t /*nElementIndex*/) const
{
    return true;
}

size_t VirtualListBox::FindSelectableElement(size_t nElementIndex, bool /*bForward*/) const
{
    return nElementIndex;
}

bool VirtualListBox::SortItems(PFNCompareFunc /*pfnCompare*/, void* /*pCompareContext*/)
{
    //不支持外部排序
    ASSERT(!"SortItems no impl!");
    return false;
}

void VirtualListBox::SetLastNoShiftIndex(size_t nLastNoShiftIndex)
{
    m_nLastNoShiftIndex = nLastNoShiftIndex;
}

size_t VirtualListBox::GetLastNoShiftIndex() const
{
    return m_nLastNoShiftIndex;
}

void VirtualListBox::CalcTileElementRectV(size_t nElemenetIndex, const UiSize& szItem,
                                          int32_t nColumns, int32_t childMarginX, int32_t childMarginY,
                                          int64_t& iLeft, int64_t& iTop,
                                          int64_t& iRight, int64_t& iBottom) const
{
    iLeft = 0;
    iRight = 0;
    iTop = 0;
    iBottom = 0;
    ASSERT(nColumns > 0);
    if (nColumns <= 0) {
        return;
    }
    if (childMarginX < 0) {
        childMarginX = 0;
    }
    if (childMarginY < 0) {
        childMarginY = 0;
    }
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return;
    }

    size_t nRowIndex = nElemenetIndex / nColumns;
    size_t nColumnIndex = nElemenetIndex % nColumns;

    iBottom = (nRowIndex + 1) * szItem.cy + nRowIndex * childMarginY;
    iTop = iBottom - szItem.cy;

    iRight = (nColumnIndex + 1) * szItem.cx + nColumnIndex * childMarginX;
    iLeft = iRight - szItem.cx;
}

void VirtualListBox::CalcTileElementRectH(size_t nElemenetIndex, const UiSize& szItem,
                                          int32_t nRows, int32_t childMarginX, int32_t childMarginY,
                                          int64_t& iLeft, int64_t& iTop,
                                          int64_t& iRight, int64_t& iBottom) const
{
    iLeft = 0;
    iRight = 0;
    iTop = 0;
    iBottom = 0;
    ASSERT(nRows > 0);
    if (nRows <= 0) {
        return;
    }
    if (childMarginX < 0) {
        childMarginX = 0;
    }
    if (childMarginY < 0) {
        childMarginY = 0;
    }
    ASSERT((szItem.cx > 0) || (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return;
    }

    size_t nColumnIndex = nElemenetIndex / nRows;
    size_t nRowIndex = nElemenetIndex % nRows;

    iRight = (nColumnIndex + 1) * szItem.cx + nColumnIndex * childMarginX;
    iLeft = iRight - szItem.cx;

    iBottom = (nRowIndex + 1) * szItem.cy + nRowIndex * childMarginY;
    iTop = iBottom - szItem.cy;
}

bool VirtualListBox::OnFrameSelection(int64_t left, int64_t right, int64_t top, int64_t bottom)
{
    ASSERT((top <= bottom) && (left <= right));
    if ((top > bottom) && (left > right)) {
        return false;
    }
    VirtualListBoxElement* pDataProvider = GetDataProvider();
    ASSERT(pDataProvider != nullptr);
    if (pDataProvider == nullptr) {
        return false;
    }
    const size_t dataItemCount = pDataProvider->GetElementCount();
    if (dataItemCount == 0) {
        return false;
    }
    Layout* pLayout = GetLayout();
    ASSERT(pLayout != nullptr);
    if (pLayout == nullptr) {
        return false;
    }
    VirtualHTileLayout* pHTileLayout = dynamic_cast<VirtualHTileLayout*>(pLayout);
    VirtualVTileLayout* pVTileLayout = dynamic_cast<VirtualVTileLayout*>(pLayout);

    VirtualHLayout* pHLayout = dynamic_cast<VirtualHLayout*>(pLayout);
    VirtualVLayout* pVLayout = dynamic_cast<VirtualVLayout*>(pLayout);

    bool bHLayout = (pHTileLayout != nullptr) || (pHLayout != nullptr);
    bool bVLayout = (pVTileLayout != nullptr) || (pVLayout != nullptr);
    ASSERT(bHLayout || bVLayout);
    if (!bHLayout && !bVLayout) {
        return false;
    }

    UiSize szItem;
    if (pHTileLayout != nullptr) {
        szItem = pHTileLayout->GetItemSize();
    }
    else if (pVTileLayout != nullptr) {
        szItem = pVTileLayout->GetItemSize();
    }
    else if (pHLayout != nullptr) {
        szItem = pHLayout->GetItemSize();
    }
    else if (pVLayout != nullptr) {
        szItem = pVLayout->GetItemSize();
    }

    ASSERT((szItem.cx > 0) && (szItem.cy > 0));
    if ((szItem.cx <= 0) || (szItem.cy <= 0)) {
        return false;
    }

    int64_t iLeft = 0;
    int64_t iTop = 0;
    int64_t iRight = 0;
    int64_t iBottom = 0;

    int64_t cLeft = 0;
    int64_t cTop = 0;
    int64_t cRight = 0;
    int64_t cBottom = 0;

    bool bRet = false;
    if (bHLayout) {
        //横向布局
        int32_t childMarginY = pLayout->GetChildMarginY();
        if (childMarginY < 0) {
            childMarginY = 0;
        }
        int32_t nRows = 1;//VirtualVLayout只有1行
        if (pHTileLayout != nullptr) {
            nRows = CalcHTileRows(pHTileLayout);
        }

        //不支持隐藏
        std::vector<size_t> itemIndexList;
        int32_t childMarginX = pLayout->GetChildMarginX();
        if (childMarginX < 0) {
            childMarginX = 0;
        }
        int64_t nStartIndex = ((int64_t)left / ((int64_t)szItem.cx + childMarginX)) * nRows;
        const size_t nCount = GetElementCount();
        for (size_t nElemenetIndex = (size_t)nStartIndex; nElemenetIndex < nCount; ++nElemenetIndex) {
            CalcTileElementRectH(nElemenetIndex, szItem, nRows, childMarginX, childMarginY,
                                 iLeft, iTop, iRight, iBottom);
            cLeft = std::max(left, iLeft);
            cTop = std::max(top, iTop);
            cRight = std::min(right, iRight);
            cBottom = std::min(bottom, iBottom);
            if ((cRight > cLeft) && (cBottom > cTop)) {
                itemIndexList.push_back(nElemenetIndex);
            }
            if (iLeft > right) {
                break;
            }
        }
        bRet = SetSelectedElements(itemIndexList, true);
    }
    else if (bVLayout) {
        //纵向布局
        int32_t childMarginX = pLayout->GetChildMarginX();
        if (childMarginX < 0) {
            childMarginX = 0;
        }
        int32_t nColumns = 1;//VirtualHLayout只有1列
        if (pVTileLayout != nullptr) {
            nColumns = CalcVTileColumns(pVTileLayout);
        }

        //不支持隐藏
        std::vector<size_t> itemIndexList;
        int32_t childMarginY = pLayout->GetChildMarginY();
        if (childMarginY < 0) {
            childMarginY = 0;
        }
        int64_t nStartIndex = ((int64_t)top / ((int64_t)szItem.cy + childMarginY)) * nColumns;
        const size_t nCount = GetElementCount();
        for (size_t nElemenetIndex = (size_t)nStartIndex; nElemenetIndex < nCount; ++nElemenetIndex) {
            CalcTileElementRectV(nElemenetIndex, szItem, nColumns, childMarginX, childMarginY,
                                 iLeft, iTop, iRight, iBottom);
            cLeft = std::max(left, iLeft);
            cTop = std::max(top, iTop);
            cRight = std::min(right, iRight);
            cBottom = std::min(bottom, iBottom);
            if ((cRight > cLeft) && (cBottom > cTop)) {
                itemIndexList.push_back(nElemenetIndex);
            }
            if (iTop > bottom) {
                break;
            }
        }
        bRet = SetSelectedElements(itemIndexList, true);
    }
    return bRet;
}

bool VirtualListBox::SelectItem(size_t iIndex, bool bTakeFocus, bool bTriggerEvent, uint64_t vkFlag)
{
    if (!IsSelectLikeListCtrl()) {
        //未开启该功能
        return BaseClass::SelectItem(iIndex, bTakeFocus, bTriggerEvent, vkFlag);
    }
    return ListCtrlSelectItem(iIndex, bTakeFocus, bTriggerEvent, vkFlag);
}

bool VirtualListBox::ListCtrlSelectItem(size_t iIndex, bool bTakeFocus,
                                        bool bTriggerEvent, uint64_t vkFlag)
{
    //事件触发，需要放在函数返回之前，不能放在代码中间
    bool bSelectStatusChanged = false;
    bool bRet = false;
    if (IsMultiSelect()) {
        //多选模式
        const size_t nCurElementIndex = GetDisplayItemElementIndex(iIndex);
        if ((nCurElementIndex >= GetElementCount()) || !IsSelectableElement(nCurElementIndex)) {
            //无有效选择，或者选择在置顶的数据项，按多选处理
            bRet = SelectItemMulti(iIndex, bTakeFocus, bTriggerEvent);
            return bRet;
        }

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
            size_t nElementIndex = GetDisplayItemElementIndex(iIndex);
            if (bRbuttonDown && IsElementSelected(nElementIndex)) {
                bRet = true;
            }
            else {
                std::vector<size_t> refreshDataIndexs;
                SetLastNoShiftIndex(nElementIndex);
                if (nElementIndex == Box::InvalidIndex) {
                    SetSelectNone(refreshDataIndexs);
                }
                else {
                    std::vector<size_t> excludeIndexs;
                    excludeIndexs.push_back(nElementIndex);
                    SetSelectNoneExclude(excludeIndexs, refreshDataIndexs);
                }
                SetCurSel(iIndex);
                bRet = SelectItemSingle(iIndex, bTakeFocus, false);
                RefreshElements(refreshDataIndexs);
                ASSERT(IsElementSelected(nElementIndex));
                ASSERT(nElementIndex == GetDisplayItemElementIndex(iIndex));
                bSelectStatusChanged = true;
                bRet = true;
            }
        }
        else {
            if (bShiftDown) {
                //按左键: 同时按下了Shift键
                size_t nIndexStart = GetLastNoShiftIndex();
                if (nIndexStart >= GetElementCount()) {
                    nIndexStart = 0;
                }
                size_t nElementIndex = GetDisplayItemElementIndex(iIndex);
                if (nElementIndex < GetElementCount()) {
                    std::vector<size_t> selectedIndexs;
                    size_t iStart = std::min(nIndexStart, nElementIndex);
                    size_t iEnd = std::max(nIndexStart, nElementIndex);
                    for (size_t i = iStart; i <= iEnd; ++i) {
                        if (IsSelectableElement(i)) {
                            selectedIndexs.push_back(i);
                        }
                    }
                    std::vector<size_t> refreshDataIndexs;
                    SetSelectedElements(selectedIndexs, true, refreshDataIndexs);
                    SetCurSel(iIndex);
                    bRet = SelectItemSingle(iIndex, bTakeFocus, false);
                    RefreshElements(refreshDataIndexs);
                    ASSERT(IsElementSelected(nElementIndex));
                    ASSERT(nElementIndex == GetDisplayItemElementIndex(iIndex));
                    bSelectStatusChanged = true;
                    bRet = true;
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
                    SetLastNoShiftIndex(GetDisplayItemElementIndex(iIndex));
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

} //namespace ui
