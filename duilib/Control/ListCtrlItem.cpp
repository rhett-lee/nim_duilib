#include "ListCtrlItem.h"

namespace ui
{
ListCtrlItem::ListCtrlItem():
    m_bSelectable(true)
{
}

ListCtrlItem::~ListCtrlItem()
{
}

std::wstring ListCtrlItem::GetType() const { return L"ListCtrlItem"; }

bool ListCtrlItem::IsSelectableType() const
{
    return m_bSelectable;
}

void ListCtrlItem::SetSelectableType(bool bSelectable)
{
    m_bSelectable = bSelectable;
}

void ListCtrlItem::Selected(bool bSelect, bool bTriggerEvent)
{
    if (__super::IsSelected() != bSelect) {
        __super::Selected(bSelect, bTriggerEvent);
    }
}

void ListCtrlItem::Activate()
{
    //重写基类的实现逻辑，这里只发出一个Click事件
    if (IsActivatable()) {
        SendEvent(kEventClick);
    }
}

bool ListCtrlItem::ButtonDown(const EventArgs& msg)
{
    if (IsEnabled() && IsActivatable() && IsPointInWithScrollOffset(msg.ptMouse)) {
        uint64_t vkFlag = kVkLButton;
#ifdef UILIB_IMPL_WINSDK
        if (msg.wParam & MK_CONTROL) {
            vkFlag |= kVkControl;
        }
        if (msg.wParam & MK_SHIFT) {
            vkFlag |= kVkShift;
        }
#endif
        //左键按下的时候，选择
        SelectItem(vkFlag);
    }
    return __super::ButtonDown(msg);
}

bool ListCtrlItem::ButtonUp(const EventArgs& msg)
{
    return __super::ButtonUp(msg);
}

bool ListCtrlItem::ButtonDoubleClick(const EventArgs& msg)
{
    return __super::ButtonDoubleClick(msg);
}

bool ListCtrlItem::RButtonDown(const EventArgs& msg)
{
    uint64_t vkFlag = kVkRButton;
#ifdef UILIB_IMPL_WINSDK
    if (msg.wParam & MK_CONTROL) {
        vkFlag |= kVkControl;
    }
    if (msg.wParam & MK_SHIFT) {
        vkFlag |= kVkShift;
    }
#endif
    //左键按下的时候，选择
    SelectItem(vkFlag);
    return __super::RButtonDown(msg);
}

bool ListCtrlItem::RButtonUp(const EventArgs& msg)
{
    return __super::RButtonUp(msg);
}

bool ListCtrlItem::RButtonDoubleClick(const EventArgs& msg)
{
    return __super::RButtonDoubleClick(msg);
}

void ListCtrlItem::SelectItem(uint64_t vkFlag)
{
    IListBoxOwner* pOwner = GetOwner();
    ASSERT(pOwner != nullptr);
    if (pOwner != nullptr) {
        size_t nListBoxIndex = GetListBoxIndex();
        pOwner->SelectItem(nListBoxIndex, true, true, vkFlag);
    }
}

}//namespace ui

