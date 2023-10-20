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

size_t VirtualListBox::GetElementCount()
{
    ASSERT(m_pDataProvider != nullptr);
    size_t elementCount = 0;
    if (m_pDataProvider != nullptr) {
        elementCount = m_pDataProvider->GetElementCount();
    }
    return elementCount;
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

void VirtualListBox::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEvent(msg);
        }
        else {
            __super::HandleEvent(msg);
        }
        return;
    }
    if (!HasDataProvider()) {
        return __super::HandleEvent(msg);
    }

    switch (msg.Type) {
    case ui::kEventKeyDown: {
        switch (msg.chKey) {
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
        switch (msg.chKey) {
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

    __super::HandleEvent(msg);
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

bool VirtualListBox::OnSelectedItem(const ui::EventArgs& args)
{
    size_t nItemIndex = args.wParam;
    if (nItemIndex != Box::InvalidIndex) {
        Control* pControl = GetItemAt(nItemIndex);
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            //更新该元素的选择状态
            size_t iElementIndex = pListBoxItem->GetElementIndex();
            ASSERT(m_pDataProvider != nullptr);
            if (m_pDataProvider != nullptr) {
                m_pDataProvider->SetElementSelected(iElementIndex, true);
            }
        }
    }
    return true;
}

bool VirtualListBox::OnUnSelectedItem(const ui::EventArgs& args)
{
    size_t nItemIndex = args.wParam;
    if (nItemIndex != Box::InvalidIndex) {
        Control* pControl = GetItemAt(nItemIndex);
        IListBoxItem* pListBoxItem = dynamic_cast<IListBoxItem*>(pControl);
        if (pListBoxItem != nullptr) {
            //更新该元素的选择状态
            size_t iElementIndex = pListBoxItem->GetElementIndex();
            ASSERT(m_pDataProvider != nullptr);
            if (m_pDataProvider != nullptr) {
                m_pDataProvider->SetElementSelected(iElementIndex, false);
            }
        }
    }
    return true;
}

}
