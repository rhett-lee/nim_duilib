#ifndef UI_CONTROL_BUTTON_H_
#define UI_CONTROL_BUTTON_H_

#pragma once

#include "duilib/Control/Label.h"

namespace ui {

template<typename InheritType = Control>
class UILIB_API ButtonTemplate : public LabelTemplate<InheritType>
{
public:
    ButtonTemplate();

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual std::wstring GetType() const override;
#if defined(ENABLE_UIAUTOMATION)
    virtual UIAControlProvider* GetUIAProvider() override;
#endif
    virtual void Activate() override;
    virtual void HandleEvent(const EventArgs& event) override;
    virtual UINT GetControlFlags() const override;

    /** 该控件是否可以放置在标题栏上（以用于处理NC消息响应）
     * @return 返回 true 表示可以，false 表示不可以， 默认返回false
     */
    virtual bool CanPlaceCaptionBar() const override;
};

template<typename InheritType>
UINT ui::ButtonTemplate<InheritType>::GetControlFlags() const
{
    return this->IsKeyboardEnabled() && this->IsEnabled() && this->IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

template<typename InheritType>
ButtonTemplate<InheritType>::ButtonTemplate()
{
    this->m_uTextStyle = DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS | DT_NOCLIP | DT_SINGLELINE;
}

template<typename InheritType>
void ButtonTemplate<InheritType>::HandleEvent(const EventArgs& event)
{
    if (!this->IsMouseEnabled() && 
        (event.Type > kEventMouseBegin) && 
        (event.Type < kEventMouseEnd)) {
        //当前控件禁止接收鼠标消息时，将鼠标相关消息转发给上层处理
        if (this->m_pParent != nullptr) {
            this->m_pParent->SendEvent(event);
        }
        else {
            __super::HandleEvent(event);
        }
        return;
    }
    if (event.Type == kEventKeyDown) {
        if (this->IsKeyboardEnabled()) {
            if (event.chKey == VK_SPACE || event.chKey == VK_RETURN) {
                Activate();
                return;
            }
        }
    }
    __super::HandleEvent(event);
}

template<typename InheritType>
inline std::wstring ButtonTemplate<InheritType>::GetType() const { return DUI_CTR_BUTTON; }

#if defined(ENABLE_UIAUTOMATION)
template<typename InheritType>
inline UIAControlProvider* ButtonTemplate<InheritType>::GetUIAProvider()
{
    if (this->m_pUIAProvider == nullptr)
    {
        this->m_pUIAProvider = (UIAControlProvider*)(new (std::nothrow) UIAButtonProvider(this));
    }
    return this->m_pUIAProvider;
}
#endif

template<typename InheritType>
void ButtonTemplate<InheritType>::Activate()
{
    if (!this->IsActivatable()) {
        return;
    }
    this->SendEvent(kEventClick);

#if defined(ENABLE_UIAUTOMATION)
    if (this->m_pUIAProvider != nullptr && UiaClientsAreListening())
        UiaRaiseAutomationEvent(this->m_pUIAProvider, UIA_Invoke_InvokedEventId);
#endif
}

template<typename InheritType>
bool ButtonTemplate<InheritType>::CanPlaceCaptionBar() const
{
    return true;
}

typedef ButtonTemplate<Control> Button;
typedef ButtonTemplate<Box> ButtonBox;

}	// namespace ui

#endif // UI_CONTROL_BUTTON_H_