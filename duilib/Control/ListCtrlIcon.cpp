#include "ListCtrlIcon.h"
#include "duilib/Control/ListCtrlView.h"
#include "Core/Window.h"

namespace ui
{
ListCtrlIcon::ListCtrlIcon(Window* pWindow) :
    CheckBoxTemplate<HBox>(pWindow),
    m_pListBoxItem(nullptr)
{
    //默认不获取焦点，无键盘消息
    SetNoFocus();
    SetKeyboardEnabled(false);
}

DString ListCtrlIcon::GetType() const { return _T("ListCtrlIcon"); }

void ListCtrlIcon::HandleEvent(const EventArgs& msg)
{
    if ((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd)) {
        //鼠标消息
        if (!IsDisabledEvents(msg) && (m_pListBoxItem != nullptr)) {
            //可处理鼠标消息
            if (OnMouseEvent(msg)) {
                return;
            }
        }
    }
    //转给基类处理
    BaseClass::HandleEvent(msg);
}

bool ListCtrlIcon::OnMouseEvent(const EventArgs& msg)
{
    //业务处理依赖的基本条件校验
    ASSERT((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd));
    if ((msg.eventType <= kEventMouseBegin) || (msg.eventType >= kEventMouseEnd)) {
        return false;
    }
    Window* pWindow = GetWindow();
    ASSERT(pWindow != nullptr);
    if (pWindow == nullptr) {
        return false;
    }
    ASSERT(IsEnabled() && IsMouseEnabled());
    if (!IsEnabled() || !IsMouseEnabled()) {
        return false;
    }
    ASSERT(m_pListBoxItem != nullptr);
    if (m_pListBoxItem == nullptr) {
        return false;
    }
    IListCtrlViewItem* pViewItem = dynamic_cast<IListCtrlViewItem*>(m_pListBoxItem);
    ASSERT(pViewItem != nullptr);
    if (pViewItem == nullptr) {
        return false;
    }
    const bool bWindowFocused = pWindow->IsWindowFocused();
    auto iconFlag = GetWeakFlag();
    auto windowFlag = pWindow->GetWeakFlag();

    //先转给视图的子项，进行业务处理
    pViewItem->OnChildItemMouseEvent(msg);
    if (iconFlag.expired() || windowFlag.expired()) {
        return true;
    }
    //窗口焦点发生变化，不再传递该消息(比如弹出右键菜单等)
    if (bWindowFocused != pWindow->IsWindowFocused()) {
        return true;
    }
    if (pWindow->GetFocusControl() != m_pListBoxItem) {
        //确保当前列表项是焦点控件，否则不再处理
        return true;
    }

    //转到基类处理
    BaseClass::HandleEvent(msg);

    if (iconFlag.expired() || windowFlag.expired() || msg.IsSenderExpired()) {
        return true;
    }

    //最后触发ListCtrl子项的消息派发
    pViewItem->FireChildItemMouseEvent(msg);
    return true;
}

void ListCtrlIcon::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void ListCtrlIcon::SetListBoxItem(Control* pListBoxItem)
{
    m_pListBoxItem = pListBoxItem;
}

}//namespace ui
