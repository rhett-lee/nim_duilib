#include "ListCtrlLabel.h"
#include "duilib/Control/ListCtrlView.h"
#include "Core/Window.h"

namespace ui
{
ListCtrlLabel::ListCtrlLabel(Window* pWindow) :
    CheckBoxTemplate<HBox>(pWindow),
    m_pListBoxItem(nullptr),
    m_bMouseDown(false),
    m_bEnableEdit(false)
{
    //默认不获取焦点，无键盘消息
    SetNoFocus();
    SetKeyboardEnabled(false);
}

DString ListCtrlLabel::GetType() const { return _T("ListCtrlLabel"); }

void ListCtrlLabel::HandleEvent(const EventArgs& msg)
{
    if ((msg.eventType > kEventMouseBegin) && (msg.eventType < kEventMouseEnd) &&
        (msg.eventType != EventType::kEventMouseEnter) && (msg.eventType != EventType::kEventMouseLeave)) {
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

bool ListCtrlLabel::OnMouseEvent(const EventArgs& msg)
{
    if (msg.IsSenderExpired()) {
        return false;
    }
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
    //当前控件禁止接收鼠标消息时，将鼠标相关消息转发给上层处理
    auto labelFlag = GetWeakFlag();
    auto windowFlag = pWindow->GetWeakFlag();
    bool bButtonUpEvent = false;
    if (IsEnableEdit()) {
        if (msg.eventType == kEventMouseButtonDown) {
            m_bMouseDown = false;
            if (pWindow->GetFocusControl() == m_pListBoxItem) { //确保当前列表项是焦点控件
                //避免每次点击都进入编辑模式
                m_bMouseDown = true;
            }
        }
        else if (msg.eventType == kEventMouseButtonUp) {
            if (m_bMouseDown) {
                m_bMouseDown = false;
                bButtonUpEvent = true;
            }
        }
    }
    else {
        m_bMouseDown = false;
    }

    //先转给视图的子项，进行业务处理
    pViewItem->OnChildItemMouseEvent(msg);
    if (labelFlag.expired() || windowFlag.expired()) {
        return true;
    }
    if ((msg.eventType != kEventMouseEnter) &&
        (msg.eventType != kEventMouseLeave) &&
        (msg.eventType != kEventMouseMove)  &&
        (msg.eventType != kEventMouseHover)) {
        //窗口焦点发生变化，不再传递该消息(比如弹出右键菜单等)
        if (bWindowFocused != pWindow->IsWindowFocused()) {
            return true;
        }
        if (pWindow->GetFocusControl() != m_pListBoxItem) {
            //确保当前列表项是焦点控件，否则不再处理
            return true;
        }

        if (bButtonUpEvent) {
            //进入编辑状态
            OnItemEnterEditMode();
        }
    }

    //转到基类处理
    BaseClass::HandleEvent(msg);

    if (labelFlag.expired() || windowFlag.expired() || msg.IsSenderExpired()) {
        return true;
    }

    //最后触发ListCtrl子项的消息派发
    pViewItem->FireChildItemMouseEvent(msg);
    return true;
}

void ListCtrlLabel::OnItemEnterEditMode()
{
    SendEvent(kEventEnterEdit, (WPARAM)this);
}

void ListCtrlLabel::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    if (!m_textRect.IsZero()) {
        m_textRect = Dpi().GetScaleRect(m_textRect, nOldDpiScale);
    }
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

void ListCtrlLabel::SetTextRect(const UiRect& rect)
{
    m_textRect = rect;
}

UiRect ListCtrlLabel::GetTextRect() const
{
    UiRect rect = m_textRect;
    if (rect.IsZero()) {
        rect = GetRect();
        rect.Deflate(GetControlPadding());
        rect.Deflate(GetTextPadding());
    }
    return rect;
}

void ListCtrlLabel::SetListBoxItem(Control* pListBoxItem)
{
    m_pListBoxItem = pListBoxItem;
}

void ListCtrlLabel::SetEnableEdit(bool bEnableEdit)
{
    m_bEnableEdit = bEnableEdit;
}

bool ListCtrlLabel::IsEnableEdit() const
{
    return m_bEnableEdit;
}

}//namespace ui
