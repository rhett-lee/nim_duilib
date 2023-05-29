#ifndef UI_CONTROL_CHECKBOX_H_
#define UI_CONTROL_CHECKBOX_H_

#pragma once

#include "duilib/Control/Button.h"

namespace ui {

template<typename InheritType = Control>
class UILIB_API CheckBoxTemplate : public ButtonTemplate<InheritType>
{
public:
    CheckBoxTemplate();

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual std::wstring GetType() const override;
#if defined(ENABLE_UIAUTOMATION)
    virtual UIAControlProvider* GetUIAProvider() override;
#endif
    virtual void Activate() override;
    virtual Image* GetEstimateImage() override;
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
    virtual void PaintStatusColor(IRender* pRender) override;
    virtual void PaintStatusImage(IRender* pRender) override;
    virtual void PaintText(IRender* pRender) override;
    virtual bool HasHotState() override;

    /**
     * @brief 选择状态下，没有设置背景色或背景图时，是否用非选择状态的对应属性来绘制
     * @return 返回 true 为选择状态，否则为 false
     */
    bool IsPaintNormalFirst() const { return m_bPaintNormalFirst; }

    /**
     * @brief 设置控件选择状态下，没有设置背景色或背景图时，用非选择状态的对应属性来绘制
     * @param[in] bFirst 为 true 绘制非选择状态属性，false 不绘制
     * @return 无
     */
    void SetPaintNormalFirst(bool bFirst) { m_bPaintNormalFirst = bFirst; }

    /**
     * @brief 判断当前是否是选择状态
     * @return 返回 true 为选择状态，否则为 false
     */
    bool IsSelected() const { return m_bSelected; }

    /**
     * @brief 设置控件是否选择状态
     * @param[in] bSelected 为 true 时为选择状态，false 时为取消选择状态
     * @param[in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。默认为 false
     * @return 无
     */
    virtual void Selected(bool bSelected, bool bTriggerEvent = false);

    /**
     * @brief 获取被选择时的图片
     * @param[in] stateType 要获取何种状态下的图片，参考 ControlStateType 枚举
     * @return 返回图片位置
     */
    std::wstring GetSelectedStateImage(ControlStateType stateType);

    /**
     * @brief 设置被选择时的图片
     * @param[in] stateType 要设置哪中状态下的图片
     * @param[in] strImage 图片地址
     * @return 无
     */
    void SetSelectedStateImage(ControlStateType stateType, const std::wstring& strImage);

    /**
     * @brief 获取被选择时的文本颜色
     * @return 返回被选择时的文本颜色
     */
    std::wstring GetSelectedTextColor();

    /**
     * @brief 设置被选择时的文本颜色
     * @param[in] dwTextColor 要设置的颜色字符串，该颜色必须在 global.xml 中存在
     * @return 无
     */
    void SetSelectedTextColor(const std::wstring& dwTextColor);

    /**
     * @brief 获取被选择时指定状态下的文本颜色
     * @param[in] stateType 要获取何种状态下的颜色
     * @return 返回颜色字符串，该值在 global.xml 中定义
     */
    std::wstring GetSelectedStateTextColor(ControlStateType stateType);

    /**
     * @brief 设置被选择时指定状态下的文本颜色
     * @param[in] stateType 要设置何种状态下的颜色
     * @param[in] stateColor 要设置的颜色
     * @return 无
     */
    void SetSelectedStateTextColor(ControlStateType stateType, const std::wstring& dwTextColor);

    /**
     * @brief 获取被选择时指定状态下的实际被渲染文本颜色
     * @param[in] buttonStateType 要获取何种状态下的颜色
     * @param[out] stateType 实际被渲染的状态
     * @return 返回颜色字符串，该值在 global.xml 中定义
     */
    std::wstring GetPaintSelectedStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType);

    /**
     * @brief 获取被选择时的控件颜色
     * @param[in] stateType 要获取何种状态下的颜色
     * @return 返回颜色字符串，该值在 global.xml 中定义
     */
    std::wstring GetSelectStateColor(ControlStateType stateType);

    /**
     * @brief 设置被选择时的控件颜色
     * @param[in] stateType 要设置何种状态下的颜色
     * @param[in] stateColor 要设置的颜色
     * @return 无
     */
    void SetSelectedStateColor(ControlStateType stateType, const std::wstring& stateColor);

    /**
     * @brief 获取被选择时的前景图片
     * @param[in] stateType 要获取何种状态下的前景图片
     * @return 返回图片位置
     */
    std::wstring GetSelectedForeStateImage(ControlStateType stateType);

    /**
     * @brief 设置被选择时的前景图片
     * @param[in] stateType 要设置何种状态下的前景图片
     * @param[in] pStrImage 图片位置
     * @return 无
     */
    void SetSelectedForeStateImage(ControlStateType stateType, const std::wstring& pStrImage);

    /**
     * @brief 监听被选择时的事件
     * @param[in] callback 被选择时触发的回调函数
     * @return 无
     */
    void AttachSelect(const EventCallback& callback) { this->AttachEvent(kEventSelect, callback); }

    /**
     * @brief 监听取消选择时的事件
     * @param[in] callback 取消选择时触发的回调函数
     * @return 无
     */
    void AttachUnSelect(const EventCallback& callback) { this->AttachEvent(kEventUnSelect, callback); }

protected:
    bool			m_bSelected;
    bool			m_bPaintNormalFirst;
    std::wstring	m_dwSelectedTextColor;
    StateColorMap	m_selectedTextColorMap;
    StateColorMap	m_selectedColorMap;
};

template<typename InheritType>
CheckBoxTemplate<InheritType>::CheckBoxTemplate() : m_bSelected(false), m_bPaintNormalFirst(false), m_dwSelectedTextColor(), m_selectedColorMap()
{
    m_selectedColorMap.SetControl(this);
    m_selectedTextColorMap.SetControl(this);
}

template<typename InheritType>
inline std::wstring CheckBoxTemplate<InheritType>::GetType() const { return DUI_CTR_CHECKBOX; }
#if defined(ENABLE_UIAUTOMATION)
template<typename InheritType>
inline UIAControlProvider* CheckBoxTemplate<InheritType>::GetUIAProvider()
{
    if (this->m_pUIAProvider == nullptr)
    {
        this->m_pUIAProvider = static_cast<UIAControlProvider*>(new (std::nothrow) UIACheckBoxProvider(this));
    }
    return this->m_pUIAProvider;
}
#endif

template<typename InheritType>
void CheckBoxTemplate<InheritType>::Activate()
{
    if (!this->IsActivatable()) {
        return;
    }
    Selected(!m_bSelected, true);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::Selected(bool bSelected, bool bTriggerEvent)
{
    if (m_bSelected == bSelected) {
        return;
    }
    m_bSelected = bSelected;

    if (bTriggerEvent) {
        if (m_bSelected) {
            this->SendEvent(kEventSelect);
        }
        else {
            this->SendEvent(kEventUnSelect);
        }
    }

#if defined(ENABLE_UIAUTOMATION)
    if (this->m_pUIAProvider != nullptr && UiaClientsAreListening()) {
        VARIANT vtOld = { 0 }, vtNew = { 0 };
        vtOld.vt = vtNew.vt = VT_I4;
        vtOld.lVal = m_bSelected ? ToggleState_Off : ToggleState_On;
        vtNew.lVal = m_bSelected ? ToggleState_On : ToggleState_Off;

        UiaRaiseAutomationPropertyChangedEvent(this->m_pUIAProvider, UIA_ToggleToggleStatePropertyId, vtOld, vtNew);

    }
#endif

    this->Invalidate();
}

template<typename InheritType>
bool CheckBoxTemplate<InheritType>::HasHotState()
{
    return m_selectedColorMap.HasHotColor() || m_selectedTextColorMap.HasHotColor() || __super::HasHotState();
}

template<typename InheritType>
Image* CheckBoxTemplate<InheritType>::GetEstimateImage()
{
    Image* estimateImage = __super::GetEstimateImage();
    if (estimateImage == nullptr) {
        estimateImage = this->GetImageMap()->GetEstimateImage(kStateImageSelectedBk);
    }

    return estimateImage;
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"selected") Selected(strValue == L"true", true);
    else if (strName == L"normalfirst") SetPaintNormalFirst(strValue == L"true");
    else if (strName == L"selectednormalimage") SetSelectedStateImage(kControlStateNormal, strValue);
    else if (strName == L"selectedhotimage") SetSelectedStateImage(kControlStateHot, strValue);
    else if (strName == L"selectedpushedimage") SetSelectedStateImage(kControlStatePushed, strValue);
    else if (strName == L"selecteddisabledimage") SetSelectedStateImage(kControlStateDisabled, strValue);
    else if (strName == L"selectedtextcolor") SetSelectedTextColor(strValue);
    else if (strName == L"selectednormaltextcolor") SetSelectedStateTextColor(kControlStateNormal, strValue);
    else if (strName == L"selectedhottextcolor")	SetSelectedStateTextColor(kControlStateHot, strValue);
    else if (strName == L"selectedpushedtextcolor") SetSelectedStateTextColor(kControlStatePushed, strValue);
    else if (strName == L"selecteddisabledtextcolor") SetSelectedStateTextColor(kControlStateDisabled, strValue);
    else if (strName == L"selectednormalcolor") SetSelectedStateColor(kControlStateNormal, strValue);
    else if (strName == L"selectedhotcolor") SetSelectedStateColor(kControlStateHot, strValue);
    else if (strName == L"selectedpushedcolor") SetSelectedStateColor(kControlStatePushed, strValue);
    else if (strName == L"selecteddisabledcolor") SetSelectedStateColor(kControlStateDisabled, strValue);
    else if (strName == L"selectedforenormalimage") SetSelectedForeStateImage(kControlStateNormal, strValue);
    else if (strName == L"selectedforehotimage") SetSelectedForeStateImage(kControlStateHot, strValue);
    else if (strName == L"selectedforepushedimage") SetSelectedForeStateImage(kControlStatePushed, strValue);
    else if (strName == L"selectedforedisabledimage") SetSelectedForeStateImage(kControlStateDisabled, strValue);
    else if (strName == L"switchselect") Selected(!IsSelected());
    else __super::SetAttribute(strName, strValue);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PaintStatusColor(IRender* pRender)
{
    if (!IsSelected()) {
        __super::PaintStatusColor(pRender);
        return;
    }

    if (IsPaintNormalFirst() && !m_selectedColorMap.HasStateColors()) {
        this->GetColorMap()->PaintStateColor(pRender, this->GetPaintRect(), this->GetState());
    }
    else {
        m_selectedColorMap.PaintStateColor(pRender, this->GetPaintRect(), this->GetState());
    }
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PaintStatusImage(IRender* pRender)
{
    if (!IsSelected()) {
        __super::PaintStatusImage(pRender);
        return;
    }

    if (IsPaintNormalFirst() && !this->GetImageMap()->HasImageType(kStateImageSelectedBk)) {
        this->GetImageMap()->PaintStateImage(pRender, kStateImageBk, this->GetState());
    }
    else {
        this->GetImageMap()->PaintStateImage(pRender, kStateImageSelectedBk, this->GetState());
    }

    if (IsPaintNormalFirst() && !this->GetImageMap()->HasImageType(kStateImageSelectedFore)) {
        this->GetImageMap()->PaintStateImage(pRender, kStateImageFore, this->GetState());
    }
    else {
        this->GetImageMap()->PaintStateImage(pRender, kStateImageSelectedFore, this->GetState());
    }
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PaintText(IRender* pRender)
{
    if (!IsSelected()) {
        __super::PaintText(pRender);
        return;
    }

    if (this->GetText().empty()) {
        return;
    }
    UiRect rc = this->GetRect();
    rc.left += this->m_rcTextPadding.left;
    rc.right -= this->m_rcTextPadding.right;
    rc.top += this->m_rcTextPadding.top;
    rc.bottom -= this->m_rcTextPadding.bottom;

    auto stateType = this->GetState();
    std::wstring clrColor = GetPaintSelectedStateTextColor(this->GetState(), stateType);
    if (clrColor.empty()) {
        clrColor = m_dwSelectedTextColor.empty() ? this->GetPaintStateTextColor(this->GetState(), stateType) : m_dwSelectedTextColor;
    }
    UiColor dwClrColor = this->GetWindowColor(clrColor);

    if (this->m_bSingleLine) {
        this->m_uTextStyle |= TEXT_SINGLELINE;
    }
    else {
        this->m_uTextStyle &= ~~TEXT_SINGLELINE;
    }

    if (this->GetAnimationManager().GetAnimationPlayer(kAnimationHot)) {
        if ((stateType == kControlStateNormal || stateType == kControlStateHot)
            && !GetSelectedStateTextColor(kControlStateHot).empty()) {
            std::wstring clrStateColor = GetSelectedStateTextColor(kControlStateNormal);
            if (!clrStateColor.empty()) {
                UiColor dwWinColor = this->GetWindowColor(clrStateColor);
                pRender->DrawString(rc, this->GetText(), dwWinColor, this->m_sFontId, this->m_uTextStyle);
            }

            if (this->GetHotAlpha() > 0) {
                std::wstring textColor = GetSelectedStateTextColor(kControlStateHot);
                if (!textColor.empty()) {
                    UiColor dwTextColor = this->GetWindowColor(textColor);
                    pRender->DrawString(rc, this->GetText(), dwTextColor, this->m_sFontId, this->m_uTextStyle, (BYTE)this->GetHotAlpha());
                }
            }

            return;
        }
    }

    pRender->DrawString(rc, this->GetText(), dwClrColor, this->m_sFontId, this->m_uTextStyle);
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectedStateImage(ControlStateType stateType)
{
    return this->GetImageMap()->GetImagePath(kStateImageSelectedBk, stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedStateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
    this->GetImageMap()->SetImageString(kStateImageSelectedBk, stateType, pStrImage);
    if (this->GetFixedWidth() == DUI_LENGTH_AUTO || this->GetFixedHeight() == DUI_LENGTH_AUTO) {
        this->ArrangeAncestor();
    }
    else {
        this->Invalidate();
    }
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectedTextColor()
{
    return m_dwSelectedTextColor;
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedTextColor(const std::wstring& dwTextColor)
{
    m_dwSelectedTextColor = dwTextColor;
    this->Invalidate();
}

template<typename InheritType /*= Control*/>
std::wstring ui::CheckBoxTemplate<InheritType>::GetSelectedStateTextColor(ControlStateType stateType)
{
    return m_selectedTextColorMap.GetStateColor(stateType);
}

template<typename InheritType /*= Control*/>
void ui::CheckBoxTemplate<InheritType>::SetSelectedStateTextColor(ControlStateType stateType, const std::wstring& dwTextColor)
{
    m_selectedTextColorMap.SetStateColor(stateType, dwTextColor);
    this->Invalidate();
}

template<typename InheritType /*= Control*/>
std::wstring ui::CheckBoxTemplate<InheritType>::GetPaintSelectedStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType)
{
    stateType = buttonStateType;
    if (stateType == kControlStatePushed && GetSelectedStateTextColor(kControlStatePushed).empty()) {
        stateType = kControlStateHot;
    }
    if (stateType == kControlStateHot && GetSelectedStateTextColor(kControlStateHot).empty()) {
        stateType = kControlStateNormal;
    }
    if (stateType == kControlStateDisabled && GetSelectedStateTextColor(kControlStateDisabled).empty()) {
        stateType = kControlStateNormal;
    }
    return GetSelectedStateTextColor(stateType);
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectStateColor(ControlStateType stateType)
{
    return m_selectedColorMap.GetStateColor(stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedStateColor(ControlStateType stateType, const std::wstring& stateColor)
{
    m_selectedColorMap.SetStateColor(stateType, stateColor);
    this->Invalidate();
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectedForeStateImage(ControlStateType stateType)
{
    return this->GetImageMap()->GetImagePath(kStateImageSelectedFore, stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedForeStateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
    this->GetImageMap()->SetImageString(kStateImageSelectedFore, stateType, pStrImage);
    if (this->GetFixedWidth() == DUI_LENGTH_AUTO || this->GetFixedHeight() == DUI_LENGTH_AUTO) {
        this->ArrangeAncestor();
    }
    else {
        this->Invalidate();
    }
}

typedef CheckBoxTemplate<Control> CheckBox;
typedef CheckBoxTemplate<Box> CheckBoxBox;

} // namespace ui

#endif // UI_CONTROL_CHECKBOX_H_