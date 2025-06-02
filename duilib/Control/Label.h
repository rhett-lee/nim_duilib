#ifndef UI_CONTROL_LABEL_H_
#define UI_CONTROL_LABEL_H_

#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/StateColorMap.h"
#include "duilib/Image/Image.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Render/IRender.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"

namespace ui
{

/** 标签控件（模板），用于显示文本
*/
template<typename InheritType = Control>
class UILIB_API LabelTemplate : public InheritType
{
    typedef InheritType BaseClass;
public:
    explicit LabelTemplate(Window* pWindow);
    virtual ~LabelTemplate() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual DString GetText() const;
    virtual std::string GetUTF8Text() const;
    virtual void SetText(const DString& strText);
    virtual void SetUTF8Text(const std::string& strText);
    virtual void SetTextId(const DString& strTextId);
    virtual void SetUTF8TextId(const std::string& strTextId);
    virtual bool HasHotState() override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintText(IRender* pRender) override;
    virtual void SetPos(UiRect rc) override;
    virtual DString GetToolTipText() const override;

    /** 计算文本区域大小（宽和高）
     *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
     *  @return 控件的文本估算大小，包含内边距(Box)，不包含外边距
     */
    virtual UiSize EstimateText(UiSize szAvailable) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 恢复默认的文本样式
    */
    void SetDefaultTextStyle(bool bRedraw);

    /**
     * @brief 设置文本样式
     * @param[in] uStyle 要设置的样式
     * @return 无
     */
    void SetTextStyle(UINT uStyle, bool bRedraw);

    /**
     * @brief 获取文本样式
     * @return 返回文本样式
     */
    UINT GetTextStyle() const;

    /**
     * @brief 获取指定状态下的文本颜色
     * @param[in] stateType 要获取的状态标志
     * @return 返回指定状态下的文本颜色
     */
    DString GetStateTextColor(ControlStateType stateType) const;

    /**
     * @brief 设置指定状态下的文本颜色
     * @param[in] stateType 要设置的状态标志
     * @param[in] dwTextColor 要设置的状态颜色字符串，该值必须在 global.xml 中存在
     * @return 无
     */
    void SetStateTextColor(ControlStateType stateType, const DString& dwTextColor);

    /**
     * @brief 获取指定状态下的实际被渲染文本颜色
     * @param[in] buttonStateType 要获取何种状态下的颜色
     * @param[out] stateType 实际被渲染的状态
     * @return 返回颜色字符串，该值在 global.xml 中定义
     */
    DString GetPaintStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType);

    /** 获取当前字体ID
     * @return 返回字体ID，该字体ID在 global.xml 中标识
     */
    DString GetFontId() const;

    /** 设置当前字体ID
     * @param[in] strFontId 要设置的字体ID，该字体ID必须在 global.xml 中存在
     */
    void SetFontId(const DString& strFontId);

    /** 获取文字内边距
     * @return 返回文字的内边距信息
     */
    UiPadding GetTextPadding() const;

    /** 设置文字内边距信息
     * @param [in] padding 内边距信息
     * @param [in] bNeedDpiScale 是否支持DPI缩放
     */
    void SetTextPadding(UiPadding padding, bool bNeedDpiScale);

    /**
     * @brief 判断是否是单行模式
     * @return 返回 true 表示单行模式，否则为 false
     */
    bool IsSingleLine() const;

    /**
     * @brief 设置为单行输入模式
     * @param[in] bSingleLine 为 true 时为单行模式，否则为 false
     * @return 无
     */
    void SetSingleLine(bool bSingleLine);

    /**
    * @brief 设置鼠标悬浮到控件显示的提示文本是否省略号出现时才显示
    * @param[in] bAutoShow true 省略号出现才显示 false 不做任何控制
    * @return 无
    */
    void SetAutoToolTip(bool bAutoShow);

protected:
    /** 检查是否需要自动显示ToolTip
    */
    void CheckShowToolTip();

    /** 绘制文字的实现函数
    * @param [in] rc 实际绘制区域，不包含内边距（需由调用方剪去内边距）
    * @param [in] pRender 渲染接口
    */
    void DoPaintText(const UiRect& rc, IRender* pRender);

private:
    UiString m_sFontId;
    UiString m_sAutoShowTooltipCache;
    UINT    m_uTextStyle;
    bool    m_bSingleLine;
    bool    m_bAutoShowToolTip;
    UiPadding16    m_rcTextPadding;
    UiString m_sText;
    UiString m_sTextId;
    StateColorMap* m_pTextColorMap;
};

template<typename InheritType>
LabelTemplate<InheritType>::LabelTemplate(Window* pWindow) :
    InheritType(pWindow),
    m_sFontId(),
    m_uTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE),
    m_bSingleLine(true),
    m_bAutoShowToolTip(false),
    m_rcTextPadding(),
    m_sText(),
    m_sTextId(),
    m_pTextColorMap(nullptr)
{
    Box* pBox = dynamic_cast<Box*>(this);
    if (pBox != nullptr) {
        this->SetFixedWidth(UiFixedInt::MakeStretch(), false, false);
        this->SetFixedHeight(UiFixedInt::MakeStretch(), false, false);
    }
    else {
        this->SetFixedWidth(UiFixedInt::MakeAuto(), false, false);
        this->SetFixedHeight(UiFixedInt::MakeAuto(), false, false);
    }
}

template<typename InheritType>
LabelTemplate<InheritType>::~LabelTemplate()
{
    if (m_pTextColorMap != nullptr) {
        delete m_pTextColorMap;
        m_pTextColorMap = nullptr;
    }
}

template<typename InheritType>
inline DString LabelTemplate<InheritType>::GetType() const { return DUI_CTR_LABEL; }

template<>
inline DString LabelTemplate<Box>::GetType() const { return DUI_CTR_LABELBOX; }

template<>
inline DString LabelTemplate<HBox>::GetType() const { return DUI_CTR_LABELHBOX; }

template<>
inline DString LabelTemplate<VBox>::GetType() const { return DUI_CTR_LABELVBOX; }

template<typename InheritType>
void LabelTemplate<InheritType>::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("text_align")) {
        if (strValue.find(_T("left")) != DString::npos) {
            m_uTextStyle &= ~(TEXT_CENTER | TEXT_RIGHT);
            m_uTextStyle |= TEXT_LEFT;
        }
        //center这个属性有歧义，保留以保持兼容性，新的属性是"hcenter"
        size_t centerPos = strValue.find(_T("center"));
        if (centerPos != DString::npos) {
            bool isCenter = true;
            size_t vCenterPos = strValue.find(_T("vcenter"));
            if (vCenterPos != DString::npos) {
                if ((vCenterPos + 1) == centerPos) {
                    isCenter = false;
                }
            }
            if (isCenter) {
                m_uTextStyle &= ~(TEXT_LEFT | TEXT_RIGHT);
                m_uTextStyle |= TEXT_CENTER;
            }
        }
        if (strValue.find(_T("hcenter")) != DString::npos) {
            m_uTextStyle &= ~(TEXT_LEFT | TEXT_RIGHT);
            m_uTextStyle |= TEXT_CENTER;
        }
        if (strValue.find(_T("right")) != DString::npos) {
            m_uTextStyle &= ~(TEXT_LEFT | TEXT_CENTER);
            m_uTextStyle |= TEXT_RIGHT;
        }
        if (strValue.find(_T("top")) != DString::npos) {
            m_uTextStyle &= ~(TEXT_BOTTOM | TEXT_VCENTER);
            m_uTextStyle |= TEXT_TOP;
        }
        if (strValue.find(_T("vcenter")) != DString::npos) {
            m_uTextStyle &= ~(TEXT_TOP | TEXT_BOTTOM);
            m_uTextStyle |= TEXT_VCENTER;
        }
        if (strValue.find(_T("bottom")) != DString::npos) {
            m_uTextStyle &= ~(TEXT_TOP | TEXT_VCENTER);
            m_uTextStyle |= TEXT_BOTTOM;
        }
    }
    else if ((strName == _T("end_ellipsis")) || (strName == _T("endellipsis"))) {
        if (strValue == _T("true")) {
            m_uTextStyle |= TEXT_END_ELLIPSIS;
        }
        else {
            m_uTextStyle &= ~TEXT_END_ELLIPSIS;
        }
    }
    else if ((strName == _T("path_ellipsis")) || (strName == _T("pathellipsis"))) {
        if (strValue == _T("true")) {
            m_uTextStyle |= TEXT_PATH_ELLIPSIS;
        }
        else {
            m_uTextStyle &= ~TEXT_PATH_ELLIPSIS;
        }
    }
    else if ((strName == _T("single_line")) || (strName == _T("singleline"))) {
        SetSingleLine(strValue == _T("true"));
    }
    else if ((strName == _T("multi_line")) || (strName == _T("multiline"))) {
        SetSingleLine(strValue != _T("true"));
    }
    else if (strName == _T("text")) {
        SetText(strValue);
    }
    else if ((strName == _T("text_id")) || (strName == _T("textid"))){
        SetTextId(strValue);
    }
    else if ((strName == _T("auto_tooltip")) || (strName == _T("autotooltip"))) {
        SetAutoToolTip(strValue == _T("true"));
    }
    else if (strName == _T("font")) {
        SetFontId(strValue);
    }
    else if ((strName == _T("normal_text_color")) || (strName == _T("normaltextcolor"))) {
        SetStateTextColor(kControlStateNormal, strValue);
    }
    else if ((strName == _T("hot_text_color")) || (strName == _T("hottextcolor"))) {
        SetStateTextColor(kControlStateHot, strValue);
    }
    else if ((strName == _T("pushed_text_color")) || (strName == _T("pushedtextcolor"))) {
        SetStateTextColor(kControlStatePushed, strValue);
    }
    else if ((strName == _T("disabled_text_color")) || (strName == _T("disabledtextcolor"))) {
        SetStateTextColor(kControlStateDisabled, strValue);
    }
    else if ((strName == _T("text_padding")) || (strName == _T("textpadding"))) {
        UiPadding rcTextPadding;
        AttributeUtil::ParsePaddingValue(strValue.c_str(), rcTextPadding);
        SetTextPadding(rcTextPadding, true);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void LabelTemplate<InheritType>::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == this->Dpi().GetScale());
    if (nNewDpiScale != this->Dpi().GetScale()) {
        return;
    }
    UiPadding rcTextPadding = GetTextPadding();
    rcTextPadding = this->Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    this->SetTextPadding(rcTextPadding, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

template<typename InheritType>
DString LabelTemplate<InheritType>::GetText() const
{
    DString strText = m_sText.c_str();
    if (strText.empty() && !m_sTextId.empty()) {
        strText = GlobalManager::Instance().Lang().GetStringViaID(m_sTextId.c_str());
    }

    return strText;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetAutoToolTip(bool bAutoShow)
{
    m_bAutoShowToolTip = bAutoShow;
    CheckShowToolTip();
}

template<typename InheritType /*= Control*/>
void ui::LabelTemplate<InheritType>::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    CheckShowToolTip();
}

template<typename InheritType>
DString LabelTemplate<InheritType>::GetToolTipText() const
{
    DString toolTip = BaseClass::GetToolTipText();
    if (!toolTip.empty()) {
        return toolTip;
    }
    else if (m_bAutoShowToolTip) {
        toolTip = m_sAutoShowTooltipCache.c_str();
    }
    return toolTip;
}

template<typename InheritType>
void LabelTemplate<InheritType>::CheckShowToolTip()
{
    m_sAutoShowTooltipCache.clear();
    if (!m_bAutoShowToolTip || (this->GetWindow() == nullptr)) {
        return;
    }
    auto pRender = this->GetWindow()->GetRender();
    if (pRender == nullptr) {
        return;
    }    
    DString sText = this->GetText();
    if (sText.empty()) {
        return;
    }
    UiRect rc = this->GetRect();
    if (rc.IsEmpty()) {
        return;
    }
    UiPadding rcPadding = this->GetControlPadding();
    rc.Deflate(rcPadding);
    rc.Deflate(this->GetTextPadding());

    if (m_bSingleLine) {
        m_uTextStyle |= TEXT_SINGLELINE;
    }
    else {
        m_uTextStyle &= ~TEXT_SINGLELINE;
    }
    int width = this->GetFixedWidth().GetInt32();
    if (this->GetFixedWidth().IsStretch()) {
        width = 0;
    }
    if (width < 0) {
        width = 0;
    }
    if (!m_bSingleLine && (width == 0)) {
        //多行文本评估宽高的时候，必须指定宽度
        width = rc.Width();
    }

    UiRect rcMessure = pRender->MeasureString(sText, this->GetIFontById(this->GetFontId()), m_uTextStyle, width);
    if (rc.Width() < rcMessure.Width() || rc.Height() < rcMessure.Height()) {
        m_sAutoShowTooltipCache = sText;
    }
}

template<typename InheritType>
std::string LabelTemplate<InheritType>::GetUTF8Text() const
{
    DString strIn = GetText();
    std::string strOut = StringConvert::TToUTF8(strIn);
    return strOut;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetText(const DString& strText)
{
    if (m_sText == strText) {
        return;
    }
    m_sText = strText;
    this->RelayoutOrRedraw();
    CheckShowToolTip();
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetUTF8Text(const std::string& strText)
{
    DString strOut = StringConvert::UTF8ToT(strText);
    SetText(strOut);
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetTextId(const DString& strTextId)
{
    if (m_sTextId == strTextId) {
        return;
    }
    m_sTextId = strTextId;
    this->RelayoutOrRedraw();
    CheckShowToolTip();
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetUTF8TextId(const std::string& strTextId)
{
    DString strOut = StringConvert::UTF8ToT(strTextId);
    SetTextId(strOut);
}

template<typename InheritType>
bool LabelTemplate<InheritType>::HasHotState()
{
    if (BaseClass::HasHotState()) {
        return true;
    }
    if (m_pTextColorMap != nullptr) {
        return m_pTextColorMap->HasHotColor();
    }
    return false;
}

template<typename InheritType>
UiSize LabelTemplate<InheritType>::EstimateText(UiSize szAvailable)
{
    if (m_bSingleLine) {
        m_uTextStyle |= TEXT_SINGLELINE;
    }
    else {
        m_uTextStyle &= ~TEXT_SINGLELINE;
    }

    int32_t nWidth = szAvailable.cx;
    if (this->GetFixedWidth().IsStretch()) {
        //如果是拉伸类型，使用外部宽度
        nWidth = CalcStretchValue(this->GetFixedWidth(), szAvailable.cx);
    }
    else if (this->GetFixedWidth().IsInt32()) {
        nWidth = this->GetFixedWidth().GetInt32();
    }
    else if (this->GetFixedWidth().IsAuto()) {
        //宽度为自动时，不限制宽度
        nWidth = INT_MAX;
    }
    const UiPadding rcTextPadding = this->GetTextPadding();
    const UiPadding rcPadding = this->GetControlPadding();
    if (nWidth != INT_MAX) {        
        nWidth -= (rcPadding.left + rcPadding.right);
        nWidth -= (rcTextPadding.left + rcTextPadding.right);
    }
    if (nWidth < 0) {
        nWidth = 0;
    }
    UiSize fixedSize;
    DString textValue = GetText();
    if (!textValue.empty() && (this->GetWindow() != nullptr)) {
        auto pRender = this->GetWindow()->GetRender();
        if (pRender != nullptr) {
            UiRect rect = pRender->MeasureString(textValue, this->GetIFontById(GetFontId()), m_uTextStyle, nWidth);
            fixedSize.cx = rect.Width();
            if (fixedSize.cx > 0) {
                fixedSize.cx += (rcTextPadding.left + rcTextPadding.right);
                fixedSize.cx += (rcPadding.left + rcPadding.right);
            }

            fixedSize.cy = rect.Height();
            if (fixedSize.cy) {
                fixedSize.cy += (rcTextPadding.top + rcTextPadding.bottom);
                fixedSize.cy += (rcPadding.top + rcPadding.bottom);
            }
        }
    }
    return fixedSize;
}

template<typename InheritType>
void LabelTemplate<InheritType>::PaintText(IRender* pRender)
{
    UiRect rc = this->GetRect();
    rc.Deflate(this->GetControlPadding());
    rc.Deflate(this->GetTextPadding());
    DoPaintText(rc, pRender);
}

template<typename InheritType>
void LabelTemplate<InheritType>::DoPaintText(const UiRect & rc, IRender * pRender)
{
    DString textValue = this->GetText();
    if (textValue.empty() || (pRender == nullptr)) {
        return;
    }

    ControlStateType stateType = this->GetState();
    UiColor dwClrColor = this->GetUiColor(GetPaintStateTextColor(this->GetState(), stateType));

    if (m_bSingleLine) {
        m_uTextStyle |= TEXT_SINGLELINE;
    }
    else {
        m_uTextStyle &= ~TEXT_SINGLELINE;
    }
    DString fontId = GetFontId();
    if (this->GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot)) {
        if ((stateType == kControlStateNormal || stateType == kControlStateHot) && 
            !GetStateTextColor(kControlStateHot).empty()) {
            DString clrColor = GetStateTextColor(kControlStateNormal);
            if (!clrColor.empty()) {
                UiColor dwTextColor = this->GetUiColor(clrColor);
                pRender->DrawString(rc, textValue, dwTextColor, this->GetIFontById(fontId), m_uTextStyle);
            }

            if (this->GetHotAlpha() > 0) {
                DString textColor = GetStateTextColor(kControlStateHot);
                if (!textColor.empty()) {
                    UiColor dwTextColor = this->GetUiColor(textColor);
                    pRender->DrawString(rc, textValue, dwTextColor, this->GetIFontById(fontId), m_uTextStyle, (uint8_t)this->GetHotAlpha());
                }
            }

            return;
        }
    }

    pRender->DrawString(rc, textValue, dwClrColor, this->GetIFontById(fontId), m_uTextStyle);
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetTextStyle(UINT uStyle, bool bRedraw)
{
    m_uTextStyle = uStyle;
    if (m_uTextStyle & TEXT_SINGLELINE) {
        m_bSingleLine = true;
    }
    else {
        m_bSingleLine = false;
    }
    if (bRedraw) {
        this->Invalidate();
    }    
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetDefaultTextStyle(bool bRedraw)
{
    SetTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE, bRedraw);
}

template<typename InheritType>
UINT LabelTemplate<InheritType>::GetTextStyle() const
{
    return m_uTextStyle;
}

template<typename InheritType>
DString LabelTemplate<InheritType>::GetStateTextColor(ControlStateType stateType) const
{
    DString stateColor;
    if (m_pTextColorMap != nullptr) {
        stateColor = m_pTextColorMap->GetStateColor(stateType);
    }
    if (stateColor.empty() && (stateType == kControlStateNormal)) {
        stateColor = GlobalManager::Instance().Color().GetDefaultTextColor();
    }
    if (stateColor.empty() && (stateType == kControlStateDisabled)) {
        stateColor = GlobalManager::Instance().Color().GetDefaultDisabledTextColor();
    }
    return stateColor;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetStateTextColor(ControlStateType stateType, const DString& dwTextColor)
{
    if (stateType == kControlStateHot) {
        this->GetAnimationManager().SetFadeHot(true);
    }
    if (m_pTextColorMap == nullptr) {
        m_pTextColorMap = new StateColorMap;
        m_pTextColorMap->SetControl(this);
    }
    m_pTextColorMap->SetStateColor(stateType, dwTextColor);
    this->Invalidate();
}

template<typename InheritType /*= Control*/>
DString ui::LabelTemplate<InheritType>::GetPaintStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType)
{
    stateType = buttonStateType;
    if (stateType == kControlStatePushed && GetStateTextColor(kControlStatePushed).empty()) {
        stateType = kControlStateHot;
    }
    if (stateType == kControlStateHot && GetStateTextColor(kControlStateHot).empty()) {
        stateType = kControlStateNormal;
    }
    if (stateType == kControlStateDisabled && GetStateTextColor(kControlStateDisabled).empty()) {
        stateType = kControlStateNormal;
    }
    return GetStateTextColor(stateType);
}

template<typename InheritType>
DString LabelTemplate<InheritType>::GetFontId() const
{
    return m_sFontId.c_str();
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetFontId(const DString& strFontId)
{
    m_sFontId = strFontId;
    this->Invalidate();
}

template<typename InheritType>
UiPadding LabelTemplate<InheritType>::GetTextPadding() const
{
    return UiPadding(m_rcTextPadding.left, m_rcTextPadding.top, m_rcTextPadding.right, m_rcTextPadding.bottom);
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
{
    ASSERT((padding.left >= 0) && (padding.top >= 0) && (padding.right >= 0) && (padding.bottom >= 0));
    if ((padding.left < 0) || (padding.top < 0) ||
        (padding.right < 0) || (padding.bottom < 0)) {
        return;
    }
    if (bNeedDpiScale) {
        this->Dpi().ScalePadding(padding);
    }    
    if (!this->GetTextPadding().Equals(padding)) {
        m_rcTextPadding.left = TruncateToUInt16(padding.left);
        m_rcTextPadding.top = TruncateToUInt16(padding.top);
        m_rcTextPadding.right = TruncateToUInt16(padding.right);
        m_rcTextPadding.bottom = TruncateToUInt16(padding.bottom);
        this->RelayoutOrRedraw();
    }    
}

template<typename InheritType>
bool LabelTemplate<InheritType>::IsSingleLine() const
{
    return m_bSingleLine;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetSingleLine(bool bSingleLine)
{
    if (m_bSingleLine == bSingleLine) {
        return;
    }
    m_bSingleLine = bSingleLine;
    this->Invalidate();
}

typedef LabelTemplate<Control> Label;
typedef LabelTemplate<Box> LabelBox;
typedef LabelTemplate<HBox> LabelHBox;
typedef LabelTemplate<VBox> LabelVBox;

}

#endif // UI_CONTROL_LABEL_H_
