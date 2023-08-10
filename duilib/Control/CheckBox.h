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
    virtual ~CheckBoxTemplate();

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual std::wstring GetType() const override;
    virtual void Activate() override;
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
    virtual void PaintStateColors(IRender* pRender) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void PaintText(IRender* pRender) override;
    virtual bool HasHotState() override;

    /** 关闭CheckBox功能，清除CheckBox的所有图片属性(比如树节点，CheckBox功能是可用通过开关开启或者关闭的)
    */
    void ClearStateImages();

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

    /** 设置选择状态，但不触发事件，不更新界面
    * @param [in] bSelected true为选择状态，false为非选择状态
    */
    virtual void SetSelected(bool bSelected) { m_bSelected = bSelected; }

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

private:
    //选择状态
    bool m_bSelected;
    //是否优先绘制Normal状态
    bool m_bPaintNormalFirst;
    //选择状态的文本颜色
    UiString m_dwSelectedTextColor;
    //选择状态的文本颜色
    StateColorMap* m_pSelectedTextColorMap;
    //选择状态的背景颜色
    StateColorMap* m_pSelectedColorMap;
};

template<typename InheritType>
CheckBoxTemplate<InheritType>::CheckBoxTemplate() : 
    m_bSelected(false), 
    m_bPaintNormalFirst(false), 
    m_dwSelectedTextColor(), 
    m_pSelectedTextColorMap(nullptr),
    m_pSelectedColorMap(nullptr)
{
}

template<typename InheritType>
CheckBoxTemplate<InheritType>::~CheckBoxTemplate()
{
    if (m_pSelectedTextColorMap != nullptr) {
        delete m_pSelectedTextColorMap;
        m_pSelectedTextColorMap = nullptr;
    }
    if (m_pSelectedColorMap != nullptr) {
        delete m_pSelectedColorMap;
        m_pSelectedColorMap = nullptr;
    }
}

template<typename InheritType>
inline std::wstring CheckBoxTemplate<InheritType>::GetType() const { return DUI_CTR_CHECKBOX; }

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
    this->Invalidate();
}

template<typename InheritType>
bool CheckBoxTemplate<InheritType>::HasHotState()
{
    if (__super::HasHotState()) {
        return true;
    }
    if ((m_pSelectedColorMap != nullptr) && m_pSelectedColorMap->HasHotColor()) {
        return true;
    }
    if ((m_pSelectedTextColorMap != nullptr) && m_pSelectedTextColorMap->HasHotColor()) {
        return true;
    }
    return false;
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"selected") {
        Selected(strValue == L"true", true);
    }
    else if ((strName == L"normal_first") || (strName == L"normalfirst")) {
        SetPaintNormalFirst(strValue == L"true");
    }
    else if ((strName == L"selected_normal_image") || (strName == L"selectednormalimage")){
        SetSelectedStateImage(kControlStateNormal, strValue);
    }
    else if ((strName == L"selected_hot_image") || (strName == L"selectedhotimage")) {
        SetSelectedStateImage(kControlStateHot, strValue);
    }
    else if ((strName == L"selected_pushed_image") || (strName == L"selectedpushedimage")) {
        SetSelectedStateImage(kControlStatePushed, strValue);
    }
    else if ((strName == L"selected_disabled_image") || (strName == L"selecteddisabledimage")) {
        SetSelectedStateImage(kControlStateDisabled, strValue);
    }
    else if ((strName == L"selected_text_color") || (strName == L"selectedtextcolor") ){
        SetSelectedTextColor(strValue);
    }
    else if ((strName == L"selected_normal_text_color") || (strName == L"selectednormaltextcolor")) {
        SetSelectedStateTextColor(kControlStateNormal, strValue);
    }
    else if ((strName == L"selected_hot_text_color") || (strName == L"selectedhottextcolor") ){
        SetSelectedStateTextColor(kControlStateHot, strValue);
    }
    else if ((strName == L"selected_pushed_text_color") || (strName == L"selectedpushedtextcolor")) {
        SetSelectedStateTextColor(kControlStatePushed, strValue);
    }
    else if ((strName == L"selected_disabled_text_color") || (strName == L"selecteddisabledtextcolor") ){
        SetSelectedStateTextColor(kControlStateDisabled, strValue);
    }
    else if ((strName == L"selected_normal_color") || (strName == L"selectednormalcolor") ){
        SetSelectedStateColor(kControlStateNormal, strValue);
    }
    else if ((strName == L"selected_hot_color") || (strName == L"selectedhotcolor") ){
        SetSelectedStateColor(kControlStateHot, strValue);
    }
    else if ((strName == L"selected_pushed_color") || (strName == L"selectedpushedcolor") ){
        SetSelectedStateColor(kControlStatePushed, strValue);
    }
    else if ((strName == L"selected_disabled_color") || (strName == L"selecteddisabledcolor") ){
        SetSelectedStateColor(kControlStateDisabled, strValue);
    }
    else if ((strName == L"selected_fore_normal_image") || (strName == L"selectedforenormalimage")) {
        SetSelectedForeStateImage(kControlStateNormal, strValue);
    }
    else if ((strName == L"selected_fore_hot_image") || (strName == L"selectedforehotimage") ){
        SetSelectedForeStateImage(kControlStateHot, strValue);
    }
    else if ((strName == L"selected_fore_pushed_image") || (strName == L"selectedforepushedimage") ){
        SetSelectedForeStateImage(kControlStatePushed, strValue);
    }
    else if ((strName == L"selected_fore_disabled_image") || (strName == L"selectedforedisabledimage") ){
        SetSelectedForeStateImage(kControlStateDisabled, strValue);
    }
    else if ((strName == L"switch_select") || (strName == L"switchselect")) {
        Selected(!IsSelected());
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::ClearStateImages()
{
    __super::ClearStateImages();
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PaintStateColors(IRender* pRender)
{
    if (!IsSelected()) {
        __super::PaintStateColors(pRender);
        return;
    }

    if (IsPaintNormalFirst() || 
        (m_pSelectedColorMap == nullptr) || 
        !m_pSelectedColorMap->HasStateColors()) {
        this->PaintStateColor(pRender, this->GetPaintRect(), this->GetState());
    }
    else {
        m_pSelectedColorMap->PaintStateColor(pRender, this->GetPaintRect(), this->GetState());
    }
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PaintStateImages(IRender* pRender)
{
    if (!IsSelected()) {
        __super::PaintStateImages(pRender);
        return;
    }

    if (IsPaintNormalFirst() && !this->HasStateImage(kStateImageSelectedBk)) {
        this->PaintStateImage(pRender, kStateImageBk, this->GetState());
    }
    else {
        this->PaintStateImage(pRender, kStateImageSelectedBk, this->GetState());
    }

    if (IsPaintNormalFirst() && !this->HasStateImage(kStateImageSelectedFore)) {
        this->PaintStateImage(pRender, kStateImageFore, this->GetState());
    }
    else {
        this->PaintStateImage(pRender, kStateImageSelectedFore, this->GetState());
    }
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PaintText(IRender* pRender)
{
    if (!IsSelected()) {
        __super::PaintText(pRender);
        return;
    }
    std::wstring textValue = this->GetText();
    if (textValue.empty() || (pRender == nullptr)) {
        return;
    }
    UiRect rc = this->GetRect();
    UiPadding rcPadding = this->GetControlPadding();
    rc.Deflate(rcPadding);
    rc.Deflate(this->GetTextPadding());

    auto stateType = this->GetState();
    std::wstring clrColor = GetPaintSelectedStateTextColor(this->GetState(), stateType);
    if (clrColor.empty()) {
        clrColor = m_dwSelectedTextColor.empty() ? this->GetPaintStateTextColor(this->GetState(), stateType) : m_dwSelectedTextColor.c_str();
    }
    UiColor dwClrColor = this->GetUiColor(clrColor);

    uint32_t uTextStyle = this->GetTextStyle();
    if (this->IsSingleLine()) {        
        uTextStyle |= TEXT_SINGLELINE;        
    }
    else {
        uTextStyle &= ~TEXT_SINGLELINE;
    }
    this->SetTextStyle(uTextStyle, false);

    if (this->GetAnimationManager().GetAnimationPlayer(kAnimationHot)) {
        if ((stateType == kControlStateNormal || stateType == kControlStateHot)
            && !GetSelectedStateTextColor(kControlStateHot).empty()) {
            std::wstring clrStateColor = GetSelectedStateTextColor(kControlStateNormal);
            if (!clrStateColor.empty()) {
                UiColor dwWinColor = this->GetUiColor(clrStateColor);
                pRender->DrawString(rc, textValue, dwWinColor, this->GetFontId(), this->GetTextStyle());
            }

            if (this->GetHotAlpha() > 0) {
                std::wstring textColor = GetSelectedStateTextColor(kControlStateHot);
                if (!textColor.empty()) {
                    UiColor dwTextColor = this->GetUiColor(textColor);
                    pRender->DrawString(rc, textValue, dwTextColor, this->GetFontId(), this->GetTextStyle(), (BYTE)this->GetHotAlpha());
                }
            }

            return;
        }
    }

    pRender->DrawString(rc, textValue, dwClrColor, this->GetFontId(), this->GetTextStyle());
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectedStateImage(ControlStateType stateType)
{
    return this->GetStateImage(kStateImageSelectedBk, stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedStateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
    this->SetStateImage(kStateImageSelectedBk, stateType, pStrImage);
    this->RelayoutOrRedraw();
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectedTextColor()
{
    return m_dwSelectedTextColor.c_str();
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
    if (m_pSelectedTextColorMap != nullptr) {
        return m_pSelectedTextColorMap->GetStateColor(stateType);
    }
    return std::wstring();
}

template<typename InheritType /*= Control*/>
void ui::CheckBoxTemplate<InheritType>::SetSelectedStateTextColor(ControlStateType stateType, const std::wstring& dwTextColor)
{
    if (m_pSelectedTextColorMap == nullptr) {
        m_pSelectedTextColorMap = new StateColorMap;
        m_pSelectedTextColorMap->SetControl(this);
    }
    m_pSelectedTextColorMap->SetStateColor(stateType, dwTextColor);
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
    if (m_pSelectedColorMap != nullptr) {
        return m_pSelectedColorMap->GetStateColor(stateType);
    }
    return std::wstring();
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedStateColor(ControlStateType stateType, const std::wstring& stateColor)
{
    if (m_pSelectedColorMap == nullptr) {
        m_pSelectedColorMap = new StateColorMap;
        m_pSelectedColorMap->SetControl(this);
    }
    m_pSelectedColorMap->SetStateColor(stateType, stateColor);
    this->Invalidate();
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectedForeStateImage(ControlStateType stateType)
{
    return this->GetStateImage(kStateImageSelectedFore, stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedForeStateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
    this->SetStateImage(kStateImageSelectedFore, stateType, pStrImage);
    this->RelayoutOrRedraw();
}

typedef CheckBoxTemplate<Control> CheckBox;
typedef CheckBoxTemplate<Box> CheckBoxBox;

} // namespace ui

#endif // UI_CONTROL_CHECKBOX_H_