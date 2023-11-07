#include "VirtualListBox.h"
#include "duilib/Core/ScrollBar.h"
#include <algorithm>

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
VirtualListBox::VirtualListBox(Layout* pLayout)
    : ListBox(pLayout)
    , m_pDataProvider(nullptr)
    , m_pVirtualLayout(nullptr)
{
    ASSERT(pLayout != nullptr);
    AttachSelect(nbase::Bind(&VirtualListBox::OnSelectedItem, this, std::placeholders::_1));
    AttachUnSelect(nbase::Bind(&VirtualListBox::OnUnSelectedItem, this, std::placeholders::_1));
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
        //注册模型数据变动通知回调
        pProvider->RegNotifys(
            nbase::Bind(&VirtualListBox::OnModelDataChanged, this, std::placeholders::_1, std::placeholders::_2),
            nbase::Bind(&VirtualListBox::OnModelCountChanged, this));
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

Control* VirtualListBox::CreateElement()
{
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider != nullptr) {
        return m_pDataProvider->CreateElement();
    }
    return nullptr;
}

void VirtualListBox::FillElement(Control* pControl, size_t nElementIndex)
{
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider != nullptr) {
        bool bFilled = m_pDataProvider->FillElement(pControl, nElementIndex);
        bool bSelected = m_pDataProvider->IsElementSelected(nElementIndex);
        ASSERT_UNUSED_VARIABLE(bFilled);
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        ASSERT(pListBoxItem != nullptr);
        if (pListBoxItem != nullptr) {
            //更新元素索引号
            pListBoxItem->SetElementIndex(nElementIndex);
            ASSERT(GetItemIndex(pControl) == pListBoxItem->GetListBoxIndex());
            //更新选择状态
            pListBoxItem->SetItemSelected(bSelected);
        }
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

bool VirtualListBox::IsElementSelected(size_t nElementIndex) const
{
    bool bSelected = false;
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider != nullptr) {
        bSelected = m_pDataProvider->IsElementSelected(nElementIndex);
    }
    return bSelected;
}

void VirtualListBox::RefreshElements(size_t nStartElementIndex, size_t nEndElementIndex)
{
    OnModelDataChanged(nStartElementIndex, nEndElementIndex);
}

void VirtualListBox::OnModelDataChanged(size_t nStartElementIndex, size_t nEndElementIndex)
{
    for (Control* pControl : m_items) {
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            size_t iElementIndex = pListBoxItem->GetElementIndex();
            if ((iElementIndex >= nStartElementIndex) &&
                (iElementIndex <= nEndElementIndex)) {
                FillElement(pControl, iElementIndex);
            }
        }
    }
}

void VirtualListBox::OnModelCountChanged()
{
    //元素的个数发生变化（有添加或者删除）
    Refresh();
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
    OnRefresh();
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
    __super::EnsureVisible(rcItem, vVisibleType, hVisibleType);
}

size_t VirtualListBox::EnsureVisible(size_t iIndex, ListBoxVerVisible vVisibleType, ListBoxHorVisible hVisibleType)
{
    size_t nNewIndex = iIndex;
    size_t nElementIndex = GetDisplayItemElementIndex(iIndex);
    if (nElementIndex >= GetElementCount()) {
        nNewIndex = __super::EnsureVisible(iIndex, vVisibleType, hVisibleType);
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
    __super::PaintChild(pRender, rcPaint);
}

bool VirtualListBox::RemoveItem(Control* pControl)
{
    return __super::RemoveItem(pControl);
}

bool VirtualListBox::RemoveItemAt(size_t iIndex)
{
    return __super::RemoveItemAt(iIndex);
}

void VirtualListBox::RemoveAllItems()
{
    return __super::RemoveAllItems();
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

bool VirtualListBox::OnFindSelectable(size_t nCurSel, 
								      bool bForward, size_t nCount,
								      bool bHome, bool bEnd,
                                      size_t& nDestItemIndex)
{
    nDestItemIndex = Box::InvalidIndex;
    const size_t nElementCount = GetElementCount();
    const size_t itemCount = GetItemCount();
    if ((nElementCount == 0) || (itemCount == 0)) {
        return false;
    }
    size_t nElementIndex = Box::InvalidIndex;
    size_t nIndex = nCurSel;
    if (bHome) {
        nElementIndex = 0;
        nIndex = 0;
    }
    else if (bEnd) {
        nElementIndex = nElementCount - 1;        
        if (itemCount > 0) {
            nIndex = itemCount - 1;
        }
    }
    else {
        if ((nCount == 0) || (nCount == Box::InvalidIndex)) {
            return false;
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

bool VirtualListBox::OnSelectedItem(const ui::EventArgs& args)
{
    OnSetElementSelected(args.wParam, true);
    return true;
}

bool VirtualListBox::OnUnSelectedItem(const ui::EventArgs& args)
{
    OnSetElementSelected(args.wParam, false);
    return true;
}

void VirtualListBox::OnSetElementSelected(size_t nItemIndex, bool bSelected)
{
    if (nItemIndex != Box::InvalidIndex) {
        Control* pControl = GetItemAt(nItemIndex);
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            //更新该元素的选择状态
            size_t iElementIndex = pListBoxItem->GetElementIndex();
            ASSERT(m_pDataProvider != nullptr);
            if (m_pDataProvider != nullptr) {
                m_pDataProvider->SetElementSelected(iElementIndex, bSelected);
            }
        }
    }
}

}
