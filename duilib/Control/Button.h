#ifndef UI_CONTROL_BUTTON_H_
#define UI_CONTROL_BUTTON_H_

#include "duilib/Control/Label.h"
#include "duilib/Core/Keyboard.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"

namespace ui {

template<typename InheritType = Control>
class UILIB_API ButtonTemplate : public LabelTemplate<InheritType>
{
    typedef LabelTemplate<InheritType> BaseClass;
public:
    explicit ButtonTemplate(Window* pWindow);

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void Activate(const EventArgs* pMsg) override;
    virtual void HandleEvent(const EventArgs& msg) override;
    virtual uint32_t GetControlFlags() const override;

    /** 该控件是否可以放置在标题栏上（以用于处理NC消息响应）
     * @return 返回 true 表示可以，false 表示不可以， 默认返回false
     */
    virtual bool CanPlaceCaptionBar() const override;
};

template<typename InheritType>
ButtonTemplate<InheritType>::ButtonTemplate(Window* pWindow):
    LabelTemplate<InheritType>(pWindow)
{
    this->SetTextStyle(TEXT_VCENTER | TEXT_CENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE, false);
}

template<typename InheritType>
inline DString ButtonTemplate<InheritType>::GetType() const { return DUI_CTR_BUTTON; }

template<>
inline DString ButtonTemplate<Box>::GetType() const { return DUI_CTR_BUTTONBOX; }

template<>
inline DString ButtonTemplate<HBox>::GetType() const { return DUI_CTR_BUTTONHBOX; }

template<>
inline DString ButtonTemplate<VBox>::GetType() const { return DUI_CTR_BUTTONVBOX; }

template<typename InheritType>
uint32_t ui::ButtonTemplate<InheritType>::GetControlFlags() const
{
    return this->IsKeyboardEnabled() && this->IsEnabled() && this->IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

template<typename InheritType>
void ButtonTemplate<InheritType>::HandleEvent(const EventArgs& msg)
{
    if (msg.eventType == kEventKeyDown) {
        if (this->IsEnabled() && this->IsKeyboardEnabled()) {
            //按下回车键或者空格键的时候，触发按钮响应动作
            if (msg.vkCode == kVK_SPACE || msg.vkCode == kVK_RETURN) {
                Activate(&msg);
                return;
            }
        }
    }
    BaseClass::HandleEvent(msg);
}

template<typename InheritType>
void ButtonTemplate<InheritType>::Activate(const EventArgs* pMsg)
{
    if (!this->IsActivatable()) {
        return;
    }
    if (pMsg != nullptr) {
        EventArgs newMsg = *pMsg;
        newMsg.eventData = pMsg->eventType;
        newMsg.eventType = kEventNone;
        this->SendEvent(kEventClick, newMsg);
    }
    else {
        this->SendEvent(kEventClick);
    }    
}

template<typename InheritType>
bool ButtonTemplate<InheritType>::CanPlaceCaptionBar() const
{
    return true;
}

typedef ButtonTemplate<Control> Button;
typedef ButtonTemplate<Box> ButtonBox;
typedef ButtonTemplate<HBox> ButtonHBox;
typedef ButtonTemplate<VBox> ButtonVBox;

}    // namespace ui

#endif // UI_CONTROL_BUTTON_H_
