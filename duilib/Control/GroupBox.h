#ifndef UI_CONTROL_GROUPBOX_H_
#define UI_CONTROL_GROUPBOX_H_

#pragma once

#include "duilib/Control/Label.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"

namespace ui
{

/** 分组容器
*/
template<typename InheritType>
class UILIB_API GroupBoxTemplate : public LabelTemplate<InheritType>
{
public:
    GroupBoxTemplate();
	virtual ~GroupBoxTemplate();
		
	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
    virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
    virtual void PaintText(IRender* pRender) override;

    /** 设置圆角大小
     */
    void SetCornerSize(UiSize cxyRound, bool bNeedDpiScale);

    /** 设置线条宽度
    */
    void SetLineWidth(int32_t nLineWidth, bool bNeedDpiScale);

    /** 设置线条颜色
    */
    void SetLineColor(const std::wstring& lineColor);

private:
    /** 获取一定透明度的颜色
    */
    UiColor GetFadeColor(UiColor color, uint8_t nFade) const;

private:
    //线条宽度
    int32_t m_nLineWidth;

    //圆角大小（默认为直角, 无圆角）
    UiSize m_cornerSize;

    //线条颜色
    UiString m_lineColor;
};

template<typename InheritType>
GroupBoxTemplate<InheritType>::GroupBoxTemplate(): m_nLineWidth(0)
{
    SetAttribute(L"text_align", L"top,left");
    SetAttribute(L"text_padding", L"8,0,0,0");
}

template<typename InheritType>
GroupBoxTemplate<InheritType>::~GroupBoxTemplate()
{
}

template<typename InheritType>
inline std::wstring GroupBoxTemplate<InheritType>::GetType() const { return L"GroupBoxTemplate"; }

template<>
inline std::wstring GroupBoxTemplate<Box>::GetType() const { return DUI_CTR_GROUP_BOX; }

template<>
inline std::wstring GroupBoxTemplate<HBox>::GetType() const { return DUI_CTR_GROUP_HBOX; }

template<>
inline std::wstring GroupBoxTemplate<VBox>::GetType() const { return DUI_CTR_GROUP_VBOX; }

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetCornerSize(UiSize cxyRound, bool bNeedDpiScale)
{
    int32_t cx = cxyRound.cx;
    int32_t cy = cxyRound.cy;
    ASSERT(cx >= 0);
    ASSERT(cy >= 0);
    if ((cx < 0) || (cy < 0)) {
        return;
    }
    //两个参数要么同时等于0，要么同时大于0，否则参数无效
    ASSERT(((cx > 0) && (cy > 0)) || ((cx == 0) && (cy == 0)));
    if (cx == 0) {
        if (cy != 0) {
            return;
        }
    }
    else {
        if (cy == 0) {
            return;
        }
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleSize(cxyRound);
    }
    if (m_cornerSize != cxyRound) {
        m_cornerSize = cxyRound;
        this->Invalidate();
    }
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleInt(nLineWidth);
    }
    if (m_nLineWidth != nLineWidth) {
        m_nLineWidth = nLineWidth;
        this->Invalidate();
    }
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetLineColor(const std::wstring& lineColor)
{
    if (m_lineColor != lineColor) {
        m_lineColor = lineColor;
        this->Invalidate();
    }
}

template<typename InheritType>
UiColor GroupBoxTemplate<InheritType>::GetFadeColor(UiColor color, uint8_t nFade) const
{
    color = UiColor(nFade, color.GetR() * nFade / 255, color.GetG() * nFade / 255, color.GetB() * nFade / 255);
    return color;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"corner_size") {
        //圆角大小
        UiSize cxyRound;
        AttributeUtil::ParseSizeValue(strValue.c_str(), cxyRound);
        this->SetCornerSize(cxyRound, true);
    }
    else if (strName == L"line_width") {
        //线条宽度
        ASSERT(_wtoi(strValue.c_str()) >= 0);
        this->SetLineWidth(_wtoi(strValue.c_str()), true);
    }
    else if (strName == L"line_color") {
        //线条颜色
        this->SetLineColor(strValue);
    }
    else if (strName == L"text") {
        //设置文本内容
        __super::SetAttribute(strName, strValue);
    }
    else {
        __super::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::PaintText(IRender* pRender)
{
    //先绘制文字
    __super::PaintText(pRender);

    if (pRender == nullptr) {
        return;
    }

    UiPadding rcPadding = this->GetControlPadding();
    std::wstring textValue = this->GetText();//文本内容
    UiRect drawTextRect;//文本的绘制区域
    bool hasClip = false;
    if (!textValue.empty()) {
        UiRect textRect = pRender->MeasureString(textValue, this->GetFontId(), 0, 0);
        drawTextRect = this->GetRect();
        drawTextRect.Deflate(rcPadding);
        drawTextRect.Deflate(this->GetTextPadding());
        drawTextRect.right = drawTextRect.left + textRect.Width();
        drawTextRect.bottom = drawTextRect.top + textRect.Height();

        //设置剪辑区域，避免绘制文字区域
        pRender->SetClip(drawTextRect, false);
        hasClip = true;
    }

    //在文字底部绘制边框
    int32_t nLineWidth = m_nLineWidth;
    if (nLineWidth <= 0) {
        nLineWidth = GlobalManager::Instance().Dpi().GetScaleInt(1);
    }
    UiSize cornerSize = m_cornerSize;
    UiColor lineColor;
    if (!m_lineColor.empty()) {
        lineColor = this->GetUiColor(m_lineColor.c_str());
    }
    if (lineColor.GetARGB() == 0) {
        lineColor = GetFadeColor(UiColor(UiColors::Gray), 96);
    }

    int32_t nShadowOffset = 1;//阴影偏移
    UiRect rc = this->GetRect();
    rc.Deflate(rcPadding);
    rc.Deflate(nLineWidth / 2 + nShadowOffset, nLineWidth/2 + nShadowOffset);

    if (drawTextRect.Height() > 0) {
        //让线条在文本中间
        rc.top += (drawTextRect.Height() / 2 - nLineWidth / 2);
    }

    if ((cornerSize.cx > 0) && (cornerSize.cy > 0)) {
        //先绘制个阴影效果
        if (nShadowOffset > 0) {
            UiColor fadeColor = GetFadeColor(lineColor, 24);
            UiRect fadeRect = rc;
            fadeRect.Inflate(nShadowOffset, nShadowOffset);
            pRender->DrawRoundRect(fadeRect, cornerSize, fadeColor, nLineWidth);
        }
        //绘制圆角矩形边框
        pRender->DrawRoundRect(rc, cornerSize, lineColor, nLineWidth);
    }
    else {
        //先绘制个阴影效果
        if (nShadowOffset > 0) {
            UiColor fadeColor = GetFadeColor(lineColor, 24);
            UiRect fadeRect = rc;
            fadeRect.Inflate(nShadowOffset, nShadowOffset);
            pRender->DrawRect(fadeRect, fadeColor, nLineWidth);
        }
        //绘制矩形边框
        pRender->DrawRect(rc, lineColor, nLineWidth);
    }

    if (hasClip) {
        //恢复剪辑区域
        pRender->ClearClip();
    }    
}

/** 分组容器/垂直分组容器/水平分组容器
*/
typedef GroupBoxTemplate<Box>  GroupBox;
typedef GroupBoxTemplate<HBox> GroupHBox;
typedef GroupBoxTemplate<VBox> GroupVBox;

}

#endif // UI_CONTROL_GROUPBOX_H_
