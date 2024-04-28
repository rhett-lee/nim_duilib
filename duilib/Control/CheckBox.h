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
    CheckBoxTemplate(const CheckBoxTemplate& r) = delete;
    CheckBoxTemplate& operator=(const CheckBoxTemplate& r) = delete;
    virtual ~CheckBoxTemplate();

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual std::wstring GetType() const override;
    virtual void Activate() override;
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
    virtual void PaintStateColors(IRender* pRender) override;
    virtual void PaintStateImages(IRender* pRender) override;
    virtual void PaintText(IRender* pRender) override;
    virtual bool HasHotState() override;
    virtual std::wstring GetBorderColor(ControlStateType stateType) const override;

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
    virtual void SetSelected(bool bSelected);

    /** 设置控件是否选择状态
     * @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
     * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。默认为 false
     */
    virtual void Selected(bool bSelected, bool bTriggerEvent = false);

    /** 获取被选择时的图片
     * @param [in] stateType 要获取何种状态下的图片，参考 ControlStateType 枚举
     * @return 返回图片路径和属性
     */
    std::wstring GetSelectedStateImage(ControlStateType stateType) const;

    /** 设置被选择时的图片
     * @param [in] stateType 要设置哪中状态下的图片
     * @param [in] strImage 图片路径和属性
     */
    void SetSelectedStateImage(ControlStateType stateType, const std::wstring& strImage);

    /** 获取被选择时的前景图片
     * @param[in] stateType 要获取何种状态下的前景图片
     */
    std::wstring GetSelectedForeStateImage(ControlStateType stateType) const;

    /** 设置被选择时的前景图片
     * @param[in] stateType 要设置何种状态下的前景图片
     * @param[in] strImage 图片位置
     */
    void SetSelectedForeStateImage(ControlStateType stateType, const std::wstring& strImage);

    /** 获取被选择时的文本颜色
     */
    std::wstring GetSelectedTextColor() const;

    /** 设置被选择时的文本颜色
     * @param[in] dwTextColor 要设置的颜色字符串，该颜色必须在 global.xml 中存在
     */
    void SetSelectedTextColor(const std::wstring& dwTextColor);

    /** 获取被选择时指定状态下的文本颜色
     * @param[in] stateType 要获取何种状态下的颜色
     * @return 返回颜色字符串，该值在 global.xml 中定义
     */
    std::wstring GetSelectedStateTextColor(ControlStateType stateType) const;

    /** 设置被选择时指定状态下的文本颜色
     * @param[in] stateType 要设置何种状态下的颜色
     * @param[in] stateColor 要设置的颜色
     */
    void SetSelectedStateTextColor(ControlStateType stateType, const std::wstring& dwTextColor);

    /** 获取被选择时指定状态下的实际被渲染文本颜色
     * @param [in] buttonStateType 要获取何种状态下的颜色
     * @param [out] stateType 实际被渲染的状态
     * @return 返回颜色字符串，该值在 global.xml 中定义
     */
    std::wstring GetPaintSelectedStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType) const;

    /** 获取被选择时的控件颜色
     * @param [in] stateType 要获取何种状态下的颜色
     * @return 返回颜色字符串，该值在 global.xml 中定义
     */
    std::wstring GetSelectStateColor(ControlStateType stateType) const;

    /** 设置被选择时的控件颜色
     * @param[in] stateType 要设置何种状态下的颜色
     * @param[in] stateColor 要设置的颜色
     */
    void SetSelectedStateColor(ControlStateType stateType, const std::wstring& stateColor);

    /** 监听被选择时的事件
     * @param [in] callback 被选择时触发的回调函数
     */
    void AttachSelect(const EventCallback& callback) { this->AttachEvent(kEventSelect, callback); }

    /** 监听取消选择时的事件
     * @param [in] callback 取消选择时触发的回调函数
     */
    void AttachUnSelect(const EventCallback& callback) { this->AttachEvent(kEventUnSelect, callback); }

public: //（三态选择[全部选择、部分选择、未选择]/勾选模式两种功能的函数）

    /** 鼠标按键弹起事件，用于判断是否点击在CheckBox图片上
    */
    virtual bool ButtonUp(const EventArgs& msg) override;

    /** 是否绘制选择状态下的背景色，提供虚函数作为可选项
   （比如ListBox/TreeView节点在多选时，由于有勾选项，并不需要绘制选择状态的背景色）
    */
    virtual bool CanPaintSelectedColors() const { return true; }

    /** 是否支持勾选模式（目前是TreeView/ListCtrl在使用这个模式）
        勾选模式是指：
        （1）只有点击在CheckBox图片上的时候，勾选框图片才是选择状态（非勾选模式下，是点击在控件矩形内就选择）
        （2）勾选状态和选择状态分离，是两个不同的状态
    */
    virtual bool SupportCheckedMode() const { return false; }

    /** 设置是否自动勾选选择的数据项(作用于Header与每行)
    */
    void SetAutoCheckSelect(bool bAutoCheckSelect) { m_bAutoCheckSelect = bAutoCheckSelect; }

    /** 获取是否自动勾选选择的数据项
    */
    bool IsAutoCheckSelect() const { return m_bAutoCheckSelect; }

    /** 是否处于勾选状态, 仅当 SupportCheckedMode() 函数为true的时候，有意义
    */
    bool IsChecked() const { return m_bChecked; }

    /** 设置Check状态
    * @param [in] bChecked 是否设置为Check状态
    * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。默认为 false
    */
    void SetChecked(bool bChecked, bool bTriggerEvent = false);

    /** 设置部分选择标志（支持三态选择标志：全部选择/部分选择/未选择）
    */
    void SetPartSelected(bool bPartSelected);

    /** 当前是否为部分选择
    */
    bool IsPartSelected() const;

    /** 获取部分选择时的图片
     * @param [in] stateType 要获取何种状态下的图片，参考 ControlStateType 枚举
     * @return 返回图片路径和属性
     */
    std::wstring GetPartSelectedStateImage(ControlStateType stateType);

    /** 设置部分选择时的图片
     * @param [in] stateType 要设置哪中状态下的图片
     * @param [in] strImage 图片路径和属性
     */
    void SetPartSelectedStateImage(ControlStateType stateType, const std::wstring& strImage);

    /** 获取部分选择时的前景图片
     * @param[in] stateType 要获取何种状态下的前景图片
     */
    std::wstring GetPartSelectedForeStateImage(ControlStateType stateType);

    /** 设置部分选择时的前景图片
     * @param[in] stateType 要设置何种状态下的前景图片
     * @param[in] strImage 图片位置
     */
    void SetPartSelectedForeStateImage(ControlStateType stateType, const std::wstring& strImage);

    /** 监听被勾选时的事件（仅当 SupportCheckedMode() 函数为true的时候，会有这个事件）
     * @param [in] callback 被选择时触发的回调函数
     */
    void AttachChecked(const EventCallback& callback) { this->AttachEvent(kEventChecked, callback); }

    /** 监听取消勾选时的事件（仅当 SupportCheckedMode() 函数为true的时候，会有这个事件）
     * @param [in] callback 取消选择时触发的回调函数
     */
    void AttachUnCheck(const EventCallback& callback) { this->AttachEvent(kEventUnCheck, callback); }

protected:
    /** 内部设置选择状态
    */
    void PrivateSetSelected(bool bSelected);

    /** 内部设置勾选状态
    */
    void PrivateSetChecked(bool bChecked);

    /** 选择状态变化事件(m_bSelected变量发生变化)
    */
    virtual void OnPrivateSetSelected() {}

    /** 勾选状态变化事件(m_bChecked变量发生变化)
    */
    virtual void OnPrivateSetChecked() {}

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

private: //（三态选择[全部选择、部分选择、未选择]/勾选模式两种功能的变量）

    //是否为部分选择（只影响选择状态下绘制哪个图片，对业务无影响）
    bool m_bPartSelected;

    //是否已经处于Check状态（仅当 SupportCheckedMode() 函数为true的时候，有意义）
    bool m_bChecked;

    //CheckBox图标所在的矩形（仅当 SupportCheckedMode() 函数为true的时候，有意义）
    UiRect* m_pCheckBoxImageRect;

    /** 当选择的时候，是否自动勾选，就是当m_bSelected变化的时候，同步修改m_bChecked，保持值相同
    *   但当m_bChecked变化的时候，不会同步给m_bSelected
    */
    bool m_bAutoCheckSelect;
};

template<typename InheritType>
CheckBoxTemplate<InheritType>::CheckBoxTemplate() : 
    m_bSelected(false), 
    m_bPaintNormalFirst(false), 
    m_dwSelectedTextColor(), 
    m_pSelectedTextColorMap(nullptr),
    m_pSelectedColorMap(nullptr),
    m_bPartSelected(false),
    m_bChecked(false),
    m_pCheckBoxImageRect(nullptr),
    m_bAutoCheckSelect(false)
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
    if (m_pCheckBoxImageRect != nullptr) {
        delete m_pCheckBoxImageRect;
        m_pCheckBoxImageRect = nullptr;
    }
}

template<typename InheritType>
inline std::wstring CheckBoxTemplate<InheritType>::GetType() const { return DUI_CTR_CHECKBOX; }

template<>
inline std::wstring CheckBoxTemplate<Box>::GetType() const { return DUI_CTR_CHECKBOXBOX; }

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"selected") {
        Selected(strValue == L"true", true);
    }
    else if ((strName == L"normal_first") || (strName == L"normalfirst")) {
        SetPaintNormalFirst(strValue == L"true");
    }
    else if ((strName == L"selected_normal_image") || (strName == L"selectednormalimage")) {
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
    else if ((strName == L"selected_fore_normal_image") || (strName == L"selectedforenormalimage")) {
        SetSelectedForeStateImage(kControlStateNormal, strValue);
    }
    else if ((strName == L"selected_fore_hot_image") || (strName == L"selectedforehotimage")) {
        SetSelectedForeStateImage(kControlStateHot, strValue);
    }
    else if ((strName == L"selected_fore_pushed_image") || (strName == L"selectedforepushedimage")) {
        SetSelectedForeStateImage(kControlStatePushed, strValue);
    }
    else if ((strName == L"selected_fore_disabled_image") || (strName == L"selectedforedisabledimage")) {
        SetSelectedForeStateImage(kControlStateDisabled, strValue);
    }
    else if (strName == L"part_selected_normal_image") {
        SetPartSelectedStateImage(kControlStateNormal, strValue);
    }
    else if (strName == L"part_selected_hot_image") {
        SetPartSelectedStateImage(kControlStateHot, strValue);
    }
    else if (strName == L"part_selected_pushed_image") {
        SetPartSelectedStateImage(kControlStatePushed, strValue);
    }
    else if (strName == L"part_selected_disabled_image") {
        SetPartSelectedStateImage(kControlStateDisabled, strValue);
    }
    else if (strName == L"part_selected_fore_normal_image") {
        SetPartSelectedForeStateImage(kControlStateNormal, strValue);
    }
    else if (strName == L"part_selected_fore_hot_image") {
        SetPartSelectedForeStateImage(kControlStateHot, strValue);
    }
    else if (strName == L"part_selected_fore_pushed_image") {
        SetPartSelectedForeStateImage(kControlStatePushed, strValue);
    }
    else if (strName == L"part_selected_fore_disabled_image") {
        SetPartSelectedForeStateImage(kControlStateDisabled, strValue);
    }
    else if ((strName == L"selected_text_color") || (strName == L"selectedtextcolor")) {
        SetSelectedTextColor(strValue);
    }
    else if ((strName == L"selected_normal_text_color") || (strName == L"selectednormaltextcolor")) {
        SetSelectedStateTextColor(kControlStateNormal, strValue);
    }
    else if ((strName == L"selected_hot_text_color") || (strName == L"selectedhottextcolor")) {
        SetSelectedStateTextColor(kControlStateHot, strValue);
    }
    else if ((strName == L"selected_pushed_text_color") || (strName == L"selectedpushedtextcolor")) {
        SetSelectedStateTextColor(kControlStatePushed, strValue);
    }
    else if ((strName == L"selected_disabled_text_color") || (strName == L"selecteddisabledtextcolor")) {
        SetSelectedStateTextColor(kControlStateDisabled, strValue);
    }
    else if ((strName == L"selected_normal_color") || (strName == L"selectednormalcolor")) {
        SetSelectedStateColor(kControlStateNormal, strValue);
    }
    else if ((strName == L"selected_hot_color") || (strName == L"selectedhotcolor")) {
        SetSelectedStateColor(kControlStateHot, strValue);
    }
    else if ((strName == L"selected_pushed_color") || (strName == L"selectedpushedcolor")) {
        SetSelectedStateColor(kControlStatePushed, strValue);
    }
    else if ((strName == L"selected_disabled_color") || (strName == L"selecteddisabledcolor")) {
        SetSelectedStateColor(kControlStateDisabled, strValue);
    }
    else if ((strName == L"switch_select") || (strName == L"switchselect")) {
        Selected(!IsSelected());
    }
    else if (strName == L"auto_check_select") {
        SetAutoCheckSelect(strValue == L"true");
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
bool CheckBoxTemplate<InheritType>::ButtonUp(const EventArgs& msg)
{
    bool bRet = __super::ButtonUp(msg);
    bool bCheckedMode = SupportCheckedMode();
    if (bCheckedMode && (m_pCheckBoxImageRect != nullptr)) {
        if (!this->IsEnabled()) {
            return bRet;
        }
        UiRect pos = this->GetPos();
        UiPoint pt(msg.ptMouse);
        pt.Offset(this->GetScrollOffsetInScrollBox());
        if (!pos.ContainsPt(pt) || !m_pCheckBoxImageRect->ContainsPt(pt)) {
            return bRet;
        }

        //确认点击在CheckBox图标上面，改变勾选状态(开关属性)
        SetChecked(!IsChecked(), true);
    }
    return bRet;
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetChecked(bool bChecked, bool bTriggerEvent)
{
    if (m_bChecked == bChecked) {
        return;
    }
    PrivateSetChecked(bChecked);
    if (bTriggerEvent) {
        if (bChecked) {
            this->SendEvent(kEventChecked);
        }
        else {
            this->SendEvent(kEventUnCheck);
        }
    }
    this->Invalidate();    
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::Activate()
{
    if (!this->IsActivatable()) {
        return;
    }
    Selected(!m_bSelected, true);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PrivateSetSelected(bool bSelected)
{
    bool bChanged = m_bSelected != bSelected;
    if (bChanged) {
        m_bSelected = bSelected;
    }
    if (!bSelected && m_bPartSelected) {
        //非选择状态时，对部分选择标记复位
        m_bPartSelected = false;
    }
    if (bChanged) {
        OnPrivateSetSelected();
    }
    if (IsAutoCheckSelect()) {
        //自动同步给Check变量
        PrivateSetChecked(m_bSelected);
    }
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PrivateSetChecked(bool bChecked)
{
    bool bChanged = m_bChecked != bChecked;
    if (bChanged) {
        m_bChecked = bChecked;
        OnPrivateSetChecked();
    }
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelected(bool bSelected)
{ 
    PrivateSetSelected(bSelected);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::Selected(bool bSelected, bool bTriggerEvent)
{
    if (m_bSelected == bSelected) {
        return;
    }
    PrivateSetSelected(bSelected);
    if (bTriggerEvent) {
        if (bSelected) {
            this->SendEvent(kEventSelect);
        }
        else {
            this->SendEvent(kEventUnSelect);
        }
    }
    this->Invalidate();
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetPartSelected(bool bPartSelected)
{ 
    if (m_bPartSelected != bPartSelected) {
        m_bPartSelected = bPartSelected;
        this->Invalidate();
    }
}

template<typename InheritType>
bool CheckBoxTemplate<InheritType>::IsPartSelected() const
{
    return m_bPartSelected;
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
std::wstring CheckBoxTemplate<InheritType>::GetBorderColor(ControlStateType stateType) const
{
    if (this->IsSelected()) {
        std::wstring borderColor = __super::GetBorderColor(kControlStatePushed);
        if (!borderColor.empty()) {
            return borderColor;
        }
    }
    return __super::GetBorderColor(stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::ClearStateImages()
{
    if (m_pCheckBoxImageRect != nullptr) {
        delete m_pCheckBoxImageRect;
        m_pCheckBoxImageRect = nullptr;
    }
    __super::ClearStateImages();
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PaintStateColors(IRender* pRender)
{
    if (!IsSelected() || !CanPaintSelectedColors()) {
        __super::PaintStateColors(pRender);
        return;
    }

    if (m_pSelectedColorMap == nullptr) {
        if (IsPaintNormalFirst()) {
            this->PaintStateColor(pRender, this->GetPaintRect(), this->GetState());
        }
    }
    else {
        if (IsPaintNormalFirst() && !m_pSelectedColorMap->HasStateColors()) {
            this->PaintStateColor(pRender, this->GetPaintRect(), this->GetState());
        }
        else {
            m_pSelectedColorMap->PaintStateColor(pRender, this->GetPaintRect(), this->GetState());
        }
    }
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::PaintStateImages(IRender* pRender)
{
    bool bCheckedMode = SupportCheckedMode();
    if (bCheckedMode && (m_pCheckBoxImageRect == nullptr)) {
        m_pCheckBoxImageRect = new UiRect;
    }

    bool isSelectNone = false;
    if (bCheckedMode) {
        //如果SupportCheckedMode()为true，则按IsChecked()判断是否显示选择状态的图片
        if (!IsChecked()) {
            isSelectNone = true;
        }
    }
    else {
        //如果SupportCheckedMode()为false，则按IsSelected()判断是否显示选择状态的图片
        if (!IsSelected()) {
            isSelectNone = true;
        }
    }

    if (isSelectNone) {
        //未选择状态
        this->PaintStateImage(pRender, kStateImageBk, this->GetState(), L"", m_pCheckBoxImageRect);
        this->PaintStateImage(pRender, kStateImageFore, this->GetState(), L"", m_pCheckBoxImageRect);
        return;
    }

    if (this->IsPartSelected()) {
        //部分选择状态
        bool bPainted = false;
        if (this->HasStateImage(kStateImagePartSelectedBk)) {
            this->PaintStateImage(pRender, kStateImagePartSelectedBk, this->GetState(), L"", m_pCheckBoxImageRect);
            bPainted = true;
        }
        if (this->HasStateImage(kStateImagePartSelectedFore)) {
            this->PaintStateImage(pRender, kStateImagePartSelectedFore, this->GetState(), L"", m_pCheckBoxImageRect);
            bPainted = true;
        }
        if (bPainted) {
            //如果已经绘制了部分选择状态，返回
            return;
        }
    }

    //全部选择状态
    if (IsPaintNormalFirst() && !this->HasStateImage(kStateImageSelectedBk)) {
        this->PaintStateImage(pRender, kStateImageBk, this->GetState(), L"", m_pCheckBoxImageRect);
    }
    else {
        this->PaintStateImage(pRender, kStateImageSelectedBk, this->GetState(), L"", m_pCheckBoxImageRect);
    }

    if (IsPaintNormalFirst() && !this->HasStateImage(kStateImageSelectedFore)) {
        this->PaintStateImage(pRender, kStateImageFore, this->GetState(), L"", m_pCheckBoxImageRect);
    }
    else {
        this->PaintStateImage(pRender, kStateImageSelectedFore, this->GetState(), L"", m_pCheckBoxImageRect);
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

    if (this->GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot)) {
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
std::wstring CheckBoxTemplate<InheritType>::GetSelectedStateImage(ControlStateType stateType) const
{
    return this->GetStateImage(kStateImageSelectedBk, stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedStateImage(ControlStateType stateType, const std::wstring& strImage)
{
    this->SetStateImage(kStateImageSelectedBk, stateType, strImage);
    this->RelayoutOrRedraw();
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectedForeStateImage(ControlStateType stateType) const
{
    return this->GetStateImage(kStateImageSelectedFore, stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetSelectedForeStateImage(ControlStateType stateType, const std::wstring& strImage)
{
    this->SetStateImage(kStateImageSelectedFore, stateType, strImage);
    this->RelayoutOrRedraw();
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetPartSelectedStateImage(ControlStateType stateType)
{
    return this->GetStateImage(kStateImagePartSelectedBk, stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetPartSelectedStateImage(ControlStateType stateType, const std::wstring& strImage)
{
    this->SetStateImage(kStateImagePartSelectedBk, stateType, strImage);
    this->RelayoutOrRedraw();
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetPartSelectedForeStateImage(ControlStateType stateType)
{
    return this->GetStateImage(kStateImagePartSelectedFore, stateType);
}

template<typename InheritType>
void CheckBoxTemplate<InheritType>::SetPartSelectedForeStateImage(ControlStateType stateType, const std::wstring& strImage)
{
    this->SetStateImage(kStateImagePartSelectedFore, stateType, strImage);
    this->RelayoutOrRedraw();
}

template<typename InheritType>
std::wstring CheckBoxTemplate<InheritType>::GetSelectedTextColor() const
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
std::wstring ui::CheckBoxTemplate<InheritType>::GetSelectedStateTextColor(ControlStateType stateType) const
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
std::wstring ui::CheckBoxTemplate<InheritType>::GetPaintSelectedStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType) const
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
std::wstring CheckBoxTemplate<InheritType>::GetSelectStateColor(ControlStateType stateType) const
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

typedef CheckBoxTemplate<Control> CheckBox;
typedef CheckBoxTemplate<Box> CheckBoxBox;

} // namespace ui

#endif // UI_CONTROL_CHECKBOX_H_