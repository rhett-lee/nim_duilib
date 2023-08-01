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
}

void VirtualListBox::SetVirtualLayout(VirtualLayout* pVirtualLayout)
{
    ASSERT(pVirtualLayout != nullptr);
    m_pVirtualLayout = pVirtualLayout;
}

void VirtualListBox::SetDataProvider(VirtualListBoxElement* pProvider)
{
    m_pDataProvider = pProvider;
    if (pProvider != nullptr) {
        // 注册模型数据变动通知回调
        pProvider->RegNotifys(
            nbase::Bind(&VirtualListBox::OnModelDataChanged, this, std::placeholders::_1, std::placeholders::_2),
            nbase::Bind(&VirtualListBox::OnModelCountChanged, this));
    }
}

VirtualListBoxElement* VirtualListBox::GetDataProvider()
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

void VirtualListBox::FillElement(Control* pControl, size_t iIndex)
{
    ASSERT(m_pDataProvider != nullptr);
    if (m_pDataProvider != nullptr) {
        m_pDataProvider->FillElement(pControl, iIndex);
    }
}

size_t VirtualListBox::GetElementCount()
{
    ASSERT(m_pDataProvider != nullptr);
    size_t elementCount = 0;
    if (m_pDataProvider != nullptr) {
        elementCount = m_pDataProvider->GetElementCount();
    }
    return elementCount;
}

void VirtualListBox::OnModelDataChanged(size_t nStartIndex, size_t nEndIndex)
{
    for (size_t i = nStartIndex; i <= nEndIndex; ++i) {
        size_t nItemIndex = ElementIndexToItemIndex(nStartIndex);
        if (Box::IsValidItemIndex(nItemIndex) && nItemIndex < m_items.size()) {
            FillElement(m_items[nItemIndex], i);
        }
    }
}

void VirtualListBox::OnModelCountChanged()
{
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
        //如果现有子项总数大于新计算的子项数，移除比数据总数多出的子项
        size_t n = nItemCount - nNewItemCount;
        for (size_t i = 0; i < n; ++i) {
            this->RemoveItemAt(0);
        }
    }
    else if (nItemCount < nNewItemCount) {
        //如果现有子项总数小于新计算的子项数，新增比数据总数少的子项
        size_t n = nNewItemCount - nItemCount;
        for (size_t i = 0; i < n; ++i) {
            Control* pControl = CreateElement();
            this->AddItem(pControl);
        }
    }
    if (nElementCount > 0) {
        ReArrangeChild(true);
        Arrange();
    }
}

void VirtualListBox::GetDisplayElements(std::vector<size_t>& collection)
{
    collection.clear();
    ASSERT(m_pVirtualLayout != nullptr);
    if (m_pVirtualLayout != nullptr) {
        m_pVirtualLayout->GetDisplayElements(GetPosWithoutPadding(), collection);
    }
}

void VirtualListBox::EnsureVisible(size_t iIndex, bool bToTop)
{
    ASSERT(m_pVirtualLayout != nullptr);
    if (m_pVirtualLayout != nullptr) {
        m_pVirtualLayout->EnsureVisible(GetPosWithoutPadding(), iIndex, bToTop);
    }
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
}

size_t VirtualListBox::ElementIndexToItemIndex(size_t nElementIndex) const
{
    ASSERT(m_pVirtualLayout != nullptr);
    if (m_pVirtualLayout == nullptr) {
        return Box::InvalidIndex;
    }
    UiRect rc = GetPosWithoutPadding();
    if (m_pVirtualLayout->IsElementDisplay(rc, nElementIndex)) {
        size_t nTopItemIndex = m_pVirtualLayout->GetTopElementIndex(rc);
        ASSERT(nElementIndex >= nTopItemIndex);
        if (nElementIndex >= nTopItemIndex) {
            return nElementIndex - nTopItemIndex;
        }
    }
    return Box::InvalidIndex;
}

void VirtualListBox::HandleEvent(const EventArgs& event)
{
    if (!IsMouseEnabled() && (event.Type > ui::kEventMouseBegin) && (event.Type < ui::kEventMouseEnd)) {
        if (GetParent() != nullptr) {
            GetParent()->SendEvent(event);
        }
        else {
            __super::HandleEvent(event);
        }
        return;
    }
    if (!HasDataProvider()) {
        return __super::HandleEvent(event);
    }

    switch (event.Type) {
    case ui::kEventKeyDown: {
        switch (event.chKey) {
        case VK_UP: {
            OnKeyDown(VK_UP);
            return;
        }
        case VK_DOWN: {
            OnKeyDown(VK_DOWN);
            return;
        }
        case VK_HOME:
            SetScrollPos(UiSize64());
            return;
        case VK_END: {
            SetScrollPos(GetScrollRange());
            return;
        }
        default:
            break;
        }
    }
    case ui::kEventKeyUp: {
        switch (event.chKey) {
        case VK_UP: {
            OnKeyUp(VK_UP);
            return;
        }
        case VK_DOWN: {
            OnKeyUp(VK_DOWN);
            return;
        }
        default:
            break;
        }
    default:
        break;
    }
    }

    __super::HandleEvent(event);
}

void VirtualListBox::OnKeyDown(TCHAR ch)
{
    if (ch == VK_UP) {
        LineUp(-1, false);
    }
    else if (ch == VK_DOWN) {
        LineDown(-1, false);
    }
}

void VirtualListBox::OnKeyUp(TCHAR /*ch*/)
{
}

}
