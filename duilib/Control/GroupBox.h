#ifndef UI_CONTROL_GROUPBOX_H_
#define UI_CONTROL_GROUPBOX_H_

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
    typedef LabelTemplate<InheritType> BaseClass;
public:
    explicit GroupBoxTemplate(Window* pWindow);
    virtual ~GroupBoxTemplate() override;
        
    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void PaintText(IRender* pRender) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 设置圆角大小
     */
    void SetCornerSize(UiSize cxyRound, bool bNeedDpiScale);

    /** 获取圆角大小
    */
    const UiSize& GetCornerSize() const;

    /** 设置线条宽度
    */
    void SetLineWidth(int32_t nLineWidth, bool bNeedDpiScale);

    /** 获取线条宽度
    */
    int32_t GetLineWidth() const;

    /** 设置线条颜色
    */
    void SetLineColor(const DString& lineColor);

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
GroupBoxTemplate<InheritType>::GroupBoxTemplate(Window* pWindow):
    LabelTemplate<InheritType>(pWindow),
    m_nLineWidth(0)
{
    SetAttribute(_T("text_align"), _T("top,left"));
    SetAttribute(_T("text_padding"), _T("8,0,0,0"));
}

template<typename InheritType>
GroupBoxTemplate<InheritType>::~GroupBoxTemplate()
{
}

template<typename InheritType>
inline DString GroupBoxTemplate<InheritType>::GetType() const { return _T("GroupBoxTemplate"); }

template<>
inline DString GroupBoxTemplate<Box>::GetType() const { return DUI_CTR_GROUP_BOX; }

template<>
inline DString GroupBoxTemplate<HBox>::GetType() const { return DUI_CTR_GROUP_HBOX; }

template<>
inline DString GroupBoxTemplate<VBox>::GetType() const { return DUI_CTR_GROUP_VBOX; }

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
        this->Dpi().ScaleSize(cxyRound);
    }
    if (m_cornerSize != cxyRound) {
        m_cornerSize = cxyRound;
        this->Invalidate();
    }
}

template<typename InheritType>
const UiSize& GroupBoxTemplate<InheritType>::GetCornerSize() const
{
    return m_cornerSize;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetLineWidth(int32_t nLineWidth, bool bNeedDpiScale)
{
    if (nLineWidth < 0) {
        nLineWidth = 0;
    }
    if (bNeedDpiScale) {
        this->Dpi().ScaleInt(nLineWidth);
    }
    if (m_nLineWidth != nLineWidth) {
        m_nLineWidth = nLineWidth;
        this->Invalidate();
    }
}

template<typename InheritType>
int32_t GroupBoxTemplate<InheritType>::GetLineWidth() const
{
    return m_nLineWidth;
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::SetLineColor(const DString& lineColor)
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
void GroupBoxTemplate<InheritType>::SetAttribute(const DString& strName, const DString& strValue)
{
    if (strName == _T("corner_size")) {
        //圆角大小
        UiSize cxyRound;
        AttributeUtil::ParseSizeValue(strValue.c_str(), cxyRound);
        this->SetCornerSize(cxyRound, true);
    }
    else if (strName == _T("line_width")) {
        //线条宽度
        ASSERT(StringUtil::StringToInt32(strValue) >= 0);
        this->SetLineWidth(StringUtil::StringToInt32(strValue), true);
    }
    else if (strName == _T("line_color")) {
        //线条颜色
        this->SetLineColor(strValue);
    }
    else if (strName == _T("text")) {
        //设置文本内容
        BaseClass::SetAttribute(strName, strValue);
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    ASSERT(nNewDpiScale == this->Dpi().GetScale());
    if (nNewDpiScale != this->Dpi().GetScale()) {
        return;
    }
    UiSize cxyRound = this->GetCornerSize();
    cxyRound = this->Dpi().GetScaleSize(cxyRound, nOldDpiScale);
    this->SetCornerSize(cxyRound, false);

    int32_t iValue = this->GetLineWidth();
    iValue = this->Dpi().GetScaleInt(iValue, nOldDpiScale);
    this->SetLineWidth(iValue, false);
    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

template<typename InheritType>
void GroupBoxTemplate<InheritType>::PaintText(IRender* pRender)
{
    //先绘制文字
    BaseClass::PaintText(pRender);

    if (pRender == nullptr) {
        return;
    }

    UiPadding rcPadding = this->GetControlPadding();
    DString textValue = this->GetText();//文本内容
    UiRect drawTextRect;//文本的绘制区域
    bool hasClip = false;
    if (!textValue.empty()) {
        UiRect textRect = pRender->MeasureString(textValue, this->GetIFontById(this->GetFontId()), 0, 0);
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
        nLineWidth = this->Dpi().GetScaleInt(1);
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
            pRender->DrawRoundRect(fadeRect, (float)cornerSize.cx, (float)cornerSize.cy, fadeColor, nLineWidth);
        }
        //绘制圆角矩形边框
        pRender->DrawRoundRect(rc, (float)cornerSize.cx, (float)cornerSize.cy, lineColor, nLineWidth);
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
