#ifndef UI_CONTROL_LABEL_H_
#define UI_CONTROL_LABEL_H_

#pragma once

#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Image/Image.h"
#include "duilib/Utils/MultiLangSupport.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/DpiManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"

namespace ui
{

template<typename InheritType = Control>
class UILIB_API LabelTemplate : public InheritType
{
public:
	LabelTemplate();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
#if defined(ENABLE_UIAUTOMATION)
	virtual UIAControlProvider* GetUIAProvider() override;
#endif
	virtual std::wstring GetText() const;
	virtual std::string GetUTF8Text() const;
	virtual void SetText(const std::wstring& strText);
	virtual void SetUTF8Text(const std::string& strText);
	virtual void SetTextId(const std::wstring& strTextId);
	virtual void SetUTF8TextId(const std::string& strTextId);
	virtual bool HasHotState() override;
	virtual UiSize EstimateText(UiSize szAvailable, bool& bReEstimateSize) override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual void PaintText(IRender* pRender) override;
	virtual void SetPos(UiRect rc) override;
    virtual std::wstring GetToolTipText() const override;

    /**
     * @brief 设置文本样式
     * @param[in] uStyle 要设置的样式
     * @return 无
     */
	void SetTextStyle(UINT uStyle);

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
	std::wstring GetStateTextColor(ControlStateType stateType) const;

	/**
     * @brief 设置指定状态下的文本颜色
     * @param[in] stateType 要设置的状态标志
     * @param[in] dwTextColor 要设置的状态颜色字符串，该值必须在 global.xml 中存在
     * @return 无
     */
	void SetStateTextColor(ControlStateType stateType, const std::wstring& dwTextColor);

    /**
     * @brief 获取指定状态下的实际被渲染文本颜色
     * @param[in] buttonStateType 要获取何种状态下的颜色
     * @param[out] stateType 实际被渲染的状态
     * @return 返回颜色字符串，该值在 global.xml 中定义
     */
	std::wstring GetPaintStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType);

    /**
     * @brief 获取当前字体编号
     * @return 返回字体编号，该编号在 global.xml 中标识
     */
	std::wstring GetFont() const;

    /**
     * @brief 设置当前字体
     * @param[in] index 要设置的字体编号，该编号必须在 global.xml 中存在
     * @return 无
     */
	void SetFont(const std::wstring& strFontId);

    /**
     * @brief 获取文字边距
     * @return 返回文字的边距信息
     */
	UiRect GetTextPadding() const;

    /**
     * @brief 设置文字边距信息
     * @param[in] rc 边距信息
     * @return 无
     */
	void SetTextPadding(UiRect rc);

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
     * @brief 是否限制整行输出
     * @return 返回 true 为限制，false 为不限制
     */
	bool IsLineLimit() const;

    /**
     * @brief 限制整行输出
     * @param[in] bLineLimit 设置 true 为限制，false 为不限制
     * @return 无
     */
	void SetLineLimit(bool bLineLimit);

    /**
    * @brief 设置鼠标悬浮到控件显示的提示文本是否省略号出现时才显示
    * @param[in] bAutoShow true 省略号出现才显示 false 不做任何控制
    * @return 无
    */
	void SetAutoToolTip(bool bAutoShow);

    void SetDrawTextFillPath(bool bDrawTextFillPath);

protected:
	void CheckShowToolTip();

protected:
	std::wstring m_sFontId;
	std::wstring m_sAutoShowTooltipCache;
	UINT	m_uTextStyle;
	bool    m_bSingleLine;
	bool    m_bLineLimit;
	bool    m_bAutoShowToolTip;
    bool    m_bDrawTextFillPath;
	UiRect	m_rcTextPadding;
	std::wstring	m_sText;
	std::wstring	m_sTextId;
	StateColorMap	m_textColorMap;
};

template<typename InheritType>
LabelTemplate<InheritType>::LabelTemplate() :
    m_sFontId(),
    m_uTextStyle(DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOCLIP | DT_SINGLELINE),
    m_bSingleLine(true),
    m_bLineLimit(false),
    m_bAutoShowToolTip(false),
    m_bDrawTextFillPath(false),
    m_rcTextPadding(),
    m_sText(),
    m_sTextId(),
    m_textColorMap()
{
    if (dynamic_cast<Box*>(this)) {
        this->SetFixedWidth(DUI_LENGTH_STRETCH, false, false);
        this->SetFixedHeight(DUI_LENGTH_STRETCH, false);
    }
    else {
        this->SetFixedWidth(DUI_LENGTH_AUTO, false, false);
        this->SetFixedHeight(DUI_LENGTH_AUTO, false);
    }

    m_textColorMap.SetStateColor(kControlStateNormal, GlobalManager::GetDefaultTextColor());
    m_textColorMap.SetStateColor(kControlStateDisabled, GlobalManager::GetDefaultDisabledTextColor());
    m_textColorMap.SetControl(this);
}

template<typename InheritType>
inline std::wstring LabelTemplate<InheritType>::GetType() const { return DUI_CTR_LABEL; }

#if defined(ENABLE_UIAUTOMATION)
template<typename InheritType>
inline UIAControlProvider* LabelTemplate<InheritType>::GetUIAProvider()
{
	if (this->m_pUIAProvider == nullptr)
	{
		this->m_pUIAProvider = static_cast<UIAControlProvider*>(new (std::nothrow) UIALabelProvider(this));
	}
	return this->m_pUIAProvider;
}
#endif

template<typename InheritType>
std::wstring LabelTemplate<InheritType>::GetText() const
{
    std::wstring strText = m_sText;
    if (strText.empty() && !m_sTextId.empty()) {
        strText = MultiLangSupport::GetInstance()->GetStringViaID(m_sTextId);
    }

    return strText;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetAutoToolTip(bool bAutoShow)
{
	m_bAutoShowToolTip = bAutoShow;
	CheckShowToolTip();
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetDrawTextFillPath(bool bDrawTextFillPath) 
{
    m_bDrawTextFillPath = bDrawTextFillPath;
}

template<typename InheritType /*= Control*/>
void ui::LabelTemplate<InheritType>::SetPos(UiRect rc)
{
	__super::SetPos(rc);
	CheckShowToolTip();
}

template<typename InheritType>
std::wstring LabelTemplate<InheritType>::GetToolTipText() const
{
	std::wstring toolTip = __super::GetToolTipText();
    if (!toolTip.empty()) {
        return toolTip;
    }
    else if (m_bAutoShowToolTip) {
        toolTip = m_sAutoShowTooltipCache;
    }
	return toolTip;
}

template<typename InheritType>
void LabelTemplate<InheritType>::CheckShowToolTip()
{
    if (!m_bAutoShowToolTip || (this->GetWindow() == nullptr)) {
        return;
    }
    auto pRender = this->GetWindow()->GetRender();
    if (pRender == nullptr) {
        return;
    }
    m_sAutoShowTooltipCache.clear();
    std::wstring sText = this->GetText();
    if (sText.empty()) {
        return;
    }

    UiRect rc = this->GetRect();
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

    if (m_bSingleLine) {
        m_uTextStyle |= DT_SINGLELINE;
    }
    else {
        m_uTextStyle &= ~DT_SINGLELINE;
    }
    int width = this->GetFixedWidth();
    if (width < 0) {
        width = 0;
    }

    UiRect rcMessure = pRender->MeasureText(sText, m_sFontId, m_uTextStyle, width);
    if (rc.GetWidth() < rcMessure.GetWidth() || rc.GetHeight() < rcMessure.GetHeight()) {
        m_sAutoShowTooltipCache = sText;
    }
}

template<typename InheritType>
std::string LabelTemplate<InheritType>::GetUTF8Text() const
{
    std::wstring strIn = GetText();
    std::string strOut;
    StringHelper::UnicodeToMBCS(strIn, strOut, CP_UTF8);
    return strOut;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetText(const std::wstring& strText)
{
    if (m_sText == strText) {
        return;
    }
    m_sText = strText;

    if (this->GetFixedWidth() == DUI_LENGTH_AUTO || this->GetFixedHeight() == DUI_LENGTH_AUTO) {
        this->ArrangeAncestor();
    }
    else {
        this->Invalidate();
    }
	CheckShowToolTip();
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetUTF8Text(const std::string& strText)
{
    std::wstring strOut;
    StringHelper::MBCSToUnicode(strText, strOut, CP_UTF8);
    SetText(strOut);
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetTextId(const std::wstring& strTextId)
{
    if (m_sTextId == strTextId) {
        return;
    }
    m_sTextId = strTextId;

    if (this->GetFixedWidth() == DUI_LENGTH_AUTO || this->GetFixedHeight() == DUI_LENGTH_AUTO) {
        this->ArrangeAncestor();
    }
    else {
        this->Invalidate();
    }
    CheckShowToolTip();
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetUTF8TextId(const std::string& strTextId)
{
    std::wstring strOut;
    StringHelper::MBCSToUnicode(strTextId, strOut, CP_UTF8);
    SetTextId(strOut);
}

template<typename InheritType>
bool LabelTemplate<InheritType>::HasHotState()
{
    return m_textColorMap.HasHotColor() || __super::HasHotState();
}

template<typename InheritType>
UiSize LabelTemplate<InheritType>::EstimateText(UiSize szAvailable, bool& bReEstimateSize)
{
    if (m_bSingleLine) {
        m_uTextStyle |= DT_SINGLELINE;
    }
    else {
        m_uTextStyle &= ~DT_SINGLELINE;
    }

    int width = this->GetFixedWidth();
    if (width < 0) {
        width = 0;
    }
    UiSize fixedSize;
    std::wstring textValue = GetText();
    if (!textValue.empty() && (this->GetWindow() != nullptr)) {
        auto pRender = this->GetWindow()->GetRender();
        if (pRender != nullptr) {
            UiRect rect = pRender->MeasureText(textValue, m_sFontId, m_uTextStyle, width);
            if (this->GetFixedWidth() == DUI_LENGTH_AUTO) {
                fixedSize.cx = rect.right - rect.left + m_rcTextPadding.left + m_rcTextPadding.right;
            }
            if (this->GetFixedHeight() == DUI_LENGTH_AUTO) {
                int estimateWidth = rect.right - rect.left;
                int estimateHeight = rect.bottom - rect.top;

                if (!m_bSingleLine && this->GetFixedWidth() == DUI_LENGTH_AUTO && this->GetMaxWidth() == DUI_LENGTH_STRETCH) {
                    bReEstimateSize = true;
                    int maxWidth = szAvailable.cx - m_rcTextPadding.left - m_rcTextPadding.right;
                    if (estimateWidth > maxWidth) {
                        estimateWidth = maxWidth;
                        UiRect newRect = pRender->MeasureText(GetText(), m_sFontId, m_uTextStyle, estimateWidth);
                        estimateHeight = newRect.bottom - newRect.top;
                    }
                }
                fixedSize.cx = estimateWidth + m_rcTextPadding.left + m_rcTextPadding.right;
                fixedSize.cy = estimateHeight + m_rcTextPadding.top + m_rcTextPadding.bottom;
            }
        }        
    }

    return fixedSize;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"align") {
        if (strValue.find(L"left") != std::wstring::npos) {
            m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
            m_uTextStyle |= DT_LEFT;
        }
        if (strValue.find(L"center") != std::wstring::npos) {
            m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
            m_uTextStyle |= DT_CENTER;
        }
        if (strValue.find(L"right") != std::wstring::npos) {
            m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
            m_uTextStyle |= DT_RIGHT;
        }
        if (strValue.find(L"top") != std::wstring::npos) {
            m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER);
            m_uTextStyle |= DT_TOP;
        }
        if (strValue.find(L"vcenter") != std::wstring::npos) {
            m_uTextStyle &= ~(DT_TOP | DT_BOTTOM);
            m_uTextStyle |= DT_VCENTER;
        }
        if (strValue.find(L"bottom") != std::wstring::npos) {
            m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
            m_uTextStyle |= DT_BOTTOM;
        }        
    }
    else if (strName == L"endellipsis") {
        if (strValue == L"true") {
            m_uTextStyle |= DT_END_ELLIPSIS;
        }
        else {
            m_uTextStyle &= ~DT_END_ELLIPSIS;
        }
    }
    else if (strName == L"linelimit") SetLineLimit(strValue == L"true");
    else if (strName == L"singleline") SetSingleLine(strValue == L"true");
    else if (strName == L"text") SetText(strValue);
    else if (strName == L"textid") SetTextId(strValue);
    else if (strName == L"autotooltip") SetAutoToolTip(strValue == L"true");
    else if (strName == L"font") SetFont(strValue);
    else if (strName == L"normaltextcolor") SetStateTextColor(kControlStateNormal, strValue);
    else if (strName == L"hottextcolor")	SetStateTextColor(kControlStateHot, strValue);
    else if (strName == L"pushedtextcolor")	SetStateTextColor(kControlStatePushed, strValue);
    else if (strName == L"disabledtextcolor")	SetStateTextColor(kControlStateDisabled, strValue);
    else if (strName == L"textpadding") {
        UiRect rcTextPadding;
        LPTSTR pstr = nullptr;
        rcTextPadding.left = wcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
        rcTextPadding.top = wcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
        rcTextPadding.right = wcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
        rcTextPadding.bottom = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
        SetTextPadding(rcTextPadding);
    }
    else if (strName== L"drawtextfillpath") {
        SetDrawTextFillPath(strValue == L"true");
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void LabelTemplate<InheritType>::PaintText(IRender* pRender)
{
    std::wstring textValue = GetText();
    if (textValue.empty() || (pRender == nullptr)) {
        return;
    }
    UiRect rc = this->GetRect();
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

    auto stateType = this->GetState();
    UiColor dwClrColor = this->GetWindowColor(GetPaintStateTextColor(this->GetState(), stateType));

    if (m_bSingleLine) {
        m_uTextStyle |= DT_SINGLELINE;
    }
    else {
        m_uTextStyle &= ~DT_SINGLELINE;
    }

    if (this->GetAnimationManager().GetAnimationPlayer(kAnimationHot)) {
        if ((stateType == kControlStateNormal || stateType == kControlStateHot) && 
            !GetStateTextColor(kControlStateHot).empty()) {
            std::wstring clrColor = GetStateTextColor(kControlStateNormal);
            if (!clrColor.empty()) {
                UiColor dwTextColor = this->GetWindowColor(clrColor);
                pRender->DrawText(rc, textValue, dwTextColor, m_sFontId, m_uTextStyle, 255, m_bLineLimit, m_bDrawTextFillPath);
            }

            if (this->GetHotAlpha() > 0) {
                std::wstring textColor = GetStateTextColor(kControlStateHot);
                if (!textColor.empty()) {
                    UiColor dwTextColor = this->GetWindowColor(textColor);
                    pRender->DrawText(rc, textValue, dwTextColor, m_sFontId, m_uTextStyle, (BYTE)this->GetHotAlpha(), m_bLineLimit, m_bDrawTextFillPath);
                }
            }

            return;
        }
    }

    pRender->DrawText(rc, textValue, dwClrColor, m_sFontId, m_uTextStyle, 255, m_bLineLimit, m_bDrawTextFillPath);
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetTextStyle(UINT uStyle)
{
    m_uTextStyle = uStyle;
    this->Invalidate();
}

template<typename InheritType>
UINT LabelTemplate<InheritType>::GetTextStyle() const
{
    return m_uTextStyle;
}

template<typename InheritType>
std::wstring LabelTemplate<InheritType>::GetStateTextColor(ControlStateType stateType) const
{
    return m_textColorMap.GetStateColor(stateType);
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetStateTextColor(ControlStateType stateType, const std::wstring& dwTextColor)
{
    if (stateType == kControlStateHot) {
        this->GetAnimationManager().SetFadeHot(true);
    }
    m_textColorMap.SetStateColor(stateType, dwTextColor);
    this->Invalidate();
}

template<typename InheritType /*= Control*/>
std::wstring ui::LabelTemplate<InheritType>::GetPaintStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType)
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
std::wstring LabelTemplate<InheritType>::GetFont() const
{
    return m_sFontId;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetFont(const std::wstring& strFontId)
{
    m_sFontId = strFontId;
    this->Invalidate();
}

template<typename InheritType>
UiRect LabelTemplate<InheritType>::GetTextPadding() const
{
    return m_rcTextPadding;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetTextPadding(UiRect rc)
{
    DpiManager::GetInstance()->ScaleRect(rc);
    m_rcTextPadding = rc;
    if (this->GetFixedWidth() == DUI_LENGTH_AUTO || this->GetFixedHeight() == DUI_LENGTH_AUTO) {
        this->ArrangeAncestor();
    }
    else {
        this->Invalidate();
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

template<typename InheritType>
bool LabelTemplate<InheritType>::IsLineLimit() const
{
    return m_bLineLimit;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetLineLimit(bool bLineLimit)
{
    if (m_bLineLimit == bLineLimit) {
        return;
    }
    m_bLineLimit = bLineLimit;
    this->Invalidate();
}

typedef LabelTemplate<Control> Label;
typedef LabelTemplate<Box> LabelBox;

}

#endif // UI_CONTROL_LABEL_H_