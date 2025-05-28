#ifndef UI_CONTROL_OPTION_H_
#define UI_CONTROL_OPTION_H_

#include "duilib/Control/CheckBox.h"
#include "duilib/duilib_defs.h"

namespace ui
{

/** 单选按钮控件
*/
template<typename InheritType = Control>
class UILIB_API OptionTemplate : public CheckBoxTemplate<InheritType>
{
    typedef CheckBoxTemplate<InheritType> BaseClass;
public:
    explicit OptionTemplate(Window* pWindow);
    virtual ~OptionTemplate() override;
        
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetWindow(Window* pWindow) override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void Selected(bool bSelected, bool bTriggerEvent = false, uint64_t vkFlag = 0) override;
    virtual void Activate(const EventArgs* pMsg) override;

    /**
     * @brief 获取所属组名称
     * @return 返回组名称
     */
    virtual DString GetGroup() const;

    /**
     * @brief 设置所属组
     * @param[in] strGroupName 组名称
     * @return 无
     */
    virtual void SetGroup(const DString& strGroupName);

private:

    //所属组名称(同一个组内的控件，进行单选状态控制)
    UiString m_sGroupName;
};

template<typename InheritType>
OptionTemplate<InheritType>::OptionTemplate(Window* pWindow) :
    CheckBoxTemplate<InheritType>(pWindow),
    m_sGroupName()
{
}

template<typename InheritType>
OptionTemplate<InheritType>::~OptionTemplate()
{
    if (!m_sGroupName.empty() && this->GetWindow()) {
        this->GetWindow()->RemoveOptionGroup(m_sGroupName.c_str(), this);
    }
}

template<typename InheritType>
inline DString OptionTemplate<InheritType>::GetType() const { return DUI_CTR_OPTION; }

template<>
inline DString OptionTemplate<Box>::GetType() const { return DUI_CTR_OPTIONBOX; }

template<>
inline DString OptionTemplate<HBox>::GetType() const { return DUI_CTR_OPTIONHBOX; }

template<>
inline DString OptionTemplate<VBox>::GetType() const { return DUI_CTR_OPTIONVBOX; }

template<typename InheritType>
void OptionTemplate<InheritType>::SetWindow(Window* pWindow)
{
    BaseClass::SetWindow(pWindow);
    if (!m_sGroupName.empty()) {
        if (this->GetWindow()) {
            this->GetWindow()->AddOptionGroup(m_sGroupName.c_str(), this);
        }
    }
}

template<typename InheritType>
void OptionTemplate<InheritType>::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("group")) {
        SetGroup(strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void OptionTemplate<InheritType>::Selected(bool bSelected, bool bTriggerEvent, uint64_t /*vkFlag*/)
{
    bool isChanged = this->IsSelected() != bSelected;
    this->SetSelected(bSelected);

    if (this->GetWindow() != nullptr) {
        if (this->IsSelected()) {
            if (!m_sGroupName.empty()) {
                std::vector<Control*>* aOptionGroup = this->GetWindow()->GetOptionGroup(m_sGroupName.c_str());
                ASSERT(aOptionGroup);
                if (aOptionGroup) {
                    for (auto it = aOptionGroup->begin(); it != aOptionGroup->end(); ++it) {
                        auto pControl = static_cast<OptionTemplate<InheritType>*>(*it);
                        if ((pControl != nullptr) && pControl != this) {
                            pControl->Selected(false, bTriggerEvent);
                        }
                    }
                }
            }

            if (isChanged && bTriggerEvent) {
                this->SendEvent(kEventSelect);
            }
        }
        else {
            if (isChanged && bTriggerEvent) {
                this->SendEvent(kEventUnSelect);
            }
        }
    }

    this->Invalidate();
}

template<typename InheritType>
void OptionTemplate<InheritType>::Activate(const EventArgs* pMsg)
{
    if (!this->IsActivatable()) {
        return;
    }
    uint64_t vkFlag = 0;
    if (pMsg != nullptr) {
        if ((pMsg->eventType == kEventMouseButtonDown) || (pMsg->eventType == kEventMouseButtonUp)) {
            vkFlag |= kVkLButton;
        }
        if ((pMsg->eventType == kEventMouseRButtonDown) || (pMsg->eventType == kEventMouseRButtonUp)) {
            vkFlag |= kVkRButton;
        }
        if (this->IsKeyDown(*pMsg, ModifierKey::kControl)) {
            vkFlag |= kVkControl;
        }
        if (this->IsKeyDown(*pMsg, ModifierKey::kShift)) {
            vkFlag |= kVkShift;
        }
    }    
    Selected(true, true, vkFlag);
    ButtonTemplate<InheritType>::Activate(pMsg);
}

template<typename InheritType>
DString OptionTemplate<InheritType>::GetGroup() const
{
    return m_sGroupName.c_str();
}

template<typename InheritType>
void OptionTemplate<InheritType>::SetGroup(const DString& strGroupName)
{
    if (strGroupName.empty()) {
        if (m_sGroupName.empty()) {
            return;
        }
        if (this->GetWindow()) {
            this->GetWindow()->RemoveOptionGroup(m_sGroupName.c_str(), this);
        }
        m_sGroupName.clear();
    }
    else {
        if (m_sGroupName == strGroupName) {
            return;
        }
        if (!m_sGroupName.empty() && this->GetWindow()) {
            this->GetWindow()->RemoveOptionGroup(m_sGroupName.c_str(), this);
        }
        m_sGroupName = strGroupName;
        if (this->GetWindow()) {
            this->GetWindow()->AddOptionGroup(m_sGroupName.c_str(), this);
        }
    }

    Selected(this->IsSelected(), true);
}

typedef OptionTemplate<Control> Option;
typedef OptionTemplate<Box> OptionBox;
typedef OptionTemplate<HBox> OptionHBox;
typedef OptionTemplate<VBox> OptionVBox;

}

#endif // UI_CONTROL_OPTION_H_
