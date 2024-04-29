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
    virtual void Activate() override;
    virtual void HandleEvent(const EventArgs& msg) override;
    virtual uint32_t GetControlFlags() const override;

    /** 该控件是否可以放置在标题栏上（以用于处理NC消息响应）
     * @return 返回 true 表示可以，false 表示不可以， 默认返回false
     */
    virtual bool CanPlaceCaptionBar() const override;
};

template<typename InheritType>
inline std::wstring ButtonTemplate<InheritType>::GetType() const { return DUI_CTR_BUTTON; }

template<>
inline std::wstring ButtonTemplate<Box>::GetType() const { return DUI_CTR_BUTTONBOX; }

template<typename InheritType>
uint32_t ui::ButtonTemplate<InheritType>::GetControlFlags() const
{
    return this->IsKeyboardEnabled() && this->IsEnabled() && this->IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

template<typename InheritType>
ButtonTemplate<InheritType>::ButtonTemplate()
{
    this->SetTextStyle(TEXT_VCENTER | TEXT_CENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE, false);
}

template<typename InheritType>
void ButtonTemplate<InheritType>::HandleEvent(const EventArgs& msg)
{
    if (msg.Type == kEventKeyDown) {
        if (this->IsEnabled() && this->IsKeyboardEnabled()) {
            //按下回车键或者空格键的时候，触发按钮响应动作
            if (msg.chKey == VK_SPACE || msg.chKey == VK_RETURN) {
                Activate();
                return;
            }
        }
    }
    __super::HandleEvent(msg);
}

template<typename InheritType>
void ButtonTemplate<InheritType>::Activate()
{
    if (!this->IsActivatable()) {
        return;
    }
    this->SendEvent(kEventClick);
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