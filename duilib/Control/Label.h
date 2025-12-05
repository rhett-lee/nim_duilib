#ifndef UI_CONTROL_LABEL_H_
#define UI_CONTROL_LABEL_H_

#include "duilib/Core/DpiManager.h"
#include "duilib/Control/LabelImpl.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"

namespace ui
{
/** 标签控件（模板），用于显示文本
*/
template<typename T = Control>
class UILIB_API LabelTemplate : public T
{
    typedef T BaseClass;
public:
    explicit LabelTemplate(Window* pWindow);
    virtual ~LabelTemplate() override;

    /// 重写父类方法，提供个性化功能，请参考父类声明
    virtual DString GetType() const override;
    virtual DString GetText() const;
    virtual std::string GetUTF8Text() const;
    virtual DString GetTextId() const;
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

    /** 语言发生变化，刷新界面文字显示相关的内容
    */
    virtual void OnLanguageChanged() override;

    /** 恢复默认的文本样式
    * @param [in] bRedraw true表示重绘，false表示不重绘
    */
    void SetDefaultTextStyle(bool bRedraw);

    /** 设置文本样式
     * @param [in] uStyle 要设置的样式
     * @param [in] bRedraw true表示重绘，false表示不重绘
     */
    void SetTextStyle(uint32_t uStyle, bool bRedraw);

    /** 获取文本样式
     * @return 返回文本样式
     */
    uint32_t GetTextStyle() const;

    /** 获取指定状态下的文本颜色
     * @param [in] stateType 要获取的状态标志
     * @return 返回指定状态下的文本颜色
     */
    DString GetStateTextColor(ControlStateType stateType) const;

    /** 设置指定状态下的文本颜色
     * @param [in] stateType 要设置的状态标志
     * @param [in] dwTextColor 要设置的状态颜色字符串，该值必须在 global.xml 中存在
     * @return 无
     */
    void SetStateTextColor(ControlStateType stateType, const DString& dwTextColor);

    /** 获取指定状态下的实际被渲染文本颜色
     * @param [in] buttonStateType 要获取何种状态下的颜色
     * @param [out] stateType 实际被渲染的状态
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

    /** 判断是否是单行模式
     * @return 返回 true 表示单行模式，否则为 false
     */
    bool IsSingleLine() const;

    /** 设置为单行输入模式
     * @param [in] bSingleLine 为 true 时为单行模式，否则为 false
     */
    void SetSingleLine(bool bSingleLine);

    /** 设置鼠标悬浮到控件显示的提示文本是否省略号出现时才显示
    * @param [in] bAutoShow true 省略号出现才显示 false 不做任何控制
    */
    void SetAutoToolTip(bool bAutoShow);

    /** 获取鼠标悬浮到控件显示的提示文本是否省略号出现时才显示
    */
    bool IsAutoToolTip() const;

    /** 设置是否替换换行符(将字符串"\\n"替换为换行符"\n"，这样可以在XML中使用括号中这两个字符(\n)来当作换行符，从而支持多行文本)
    * @param [in] bReplaceNewline true表示替换，false表示不替换
    */
    void SetReplaceNewline(bool bReplaceNewline);

    /** 获取是否替换换行符(将字符串"\\n"替换为换行符"\n"，这样可以在XML中使用括号中这两个字符(\n)来当作换行符，从而支持多行文本)
    * @return true表示替换，false表示不替换
    */
    bool IsReplaceNewline() const;

    /** 设置行间距(横向文本和纵向文本均有效)
     *  设置后，实际的行间距为：字体大小 * mul + add
     * @param [in] mul 行间距的倍数, 是字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距。
     * @param [in] add 行间距附加量, 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移(像素)
     * @param [in] bNeedDpiScale 是否支持DPI缩放
     */
    void SetLineSpacing(float mul, float add, bool bNeedDpiScale);

    /** 获取行间距(横向文本和纵向文本均有效)
     * @param [out] mul 返回行间距的倍数
     * @param [out] add 返回行间距附加量(像素, 已经完成DPI缩放)
     */
    void GetLineSpacing(float* mul, float* add) const;

    /** 设置两个相邻的字符之间的间隔（像素）
    */
    void SetWordSpacing(float fWordSpacing, bool bNeedDpiScale);

    /** 设置两个相邻的字符之间的间隔（像素, 已经完成DPI缩放）
    */
    float GetWordSpacing() const;

public:
    /** 设置文本方向：true为纵向文本，false为横向文本
     *    横向文本：从左到右，从上到下
     *    纵向文本：从上到下，从右到左
     */
    void SetVerticalText(bool bVerticalText);

    /** 获取文本方向
    @return true为纵向文本，false为横向文本
    */
    bool IsVerticalText() const;

    /** 当纵向绘制文本时，设置字间距使用该字体的默认高度，而不是每个字的实际高度（显示时所有字体等高）
    */
    void SetUseFontHeight(bool bUseFontHeight);

    /** 获取当纵向绘制文本时，字间距使用该字体的默认高度，而不是每个字的实际高度（显示时所有字体等高）
    */
    bool IsUseFontHeight() const;

    /** 设置当纵向绘制文本时，对于字母、数字等字符，顺时针旋转90度显示
    */
    void SetRotate90ForAscii(bool bRotate90ForAscii);

    /** 获取当纵向绘制文本时，对于字母、数字等字符，顺时针旋转90度显示
    */
    bool IsRotate90ForAscii() const;

public:
    /** 获取当前评估绘制文字的参数
    * @return 返回当前设置的参数，不含rectSize字段的值
    */
    MeasureStringParam GetMeasureParam() const;

    /** 获取当前绘制文字的参数
    * @return 返回当前设置的参数，不含textRect/dwTextColor/uFade字段的值
    */
    DrawStringParam GetDrawParam() const;

    /** 辅助函数，获取正确的文本风格，保留支持的风格，包括对齐方式等（详情参见DrawStringFormat定义）
    * @param [in] nTextFormat 文本风格
    * @return 返回支持的文本风格标志位
    */
    static uint32_t GetValidTextStyle(uint32_t nTextFormat);

protected:
    /** 绘制文字的实现函数
    * @param [in] rc 实际绘制区域，不包含内边距（需由调用方剪去内边距）
    * @param [in] pRender 渲染接口
    */
    void DoPaintText(const UiRect& rc, IRender* pRender);

private:
    //功能内部实现
    std::unique_ptr<LabelImpl> m_impl;
};

template<typename T>
LabelTemplate<T>::LabelTemplate(Window* pWindow) :
    T(pWindow)
{
    m_impl = std::make_unique<LabelImpl>(this);
}

template<typename T>
LabelTemplate<T>::~LabelTemplate()
{
    m_impl.reset();
}

template<typename T>
inline DString LabelTemplate<T>::GetType() const { return DUI_CTR_LABEL; }

template<>
inline DString LabelTemplate<Box>::GetType() const { return DUI_CTR_LABELBOX; }

template<>
inline DString LabelTemplate<HBox>::GetType() const { return DUI_CTR_LABELHBOX; }

template<>
inline DString LabelTemplate<VBox>::GetType() const { return DUI_CTR_LABELVBOX; }

template<typename T>
void LabelTemplate<T>::SetAttribute(const DString& strName, const DString& strValue)
{
    if (!m_impl->SetAttribute(strName, strValue)) {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename T>
void LabelTemplate<T>::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!this->Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    UiPadding rcTextPadding = m_impl->GetTextPadding();
    rcTextPadding = this->Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    m_impl->SetTextPadding(rcTextPadding, false);

    float mul = 1.0f;
    float add = 0;
    m_impl->GetLineSpacing(&mul, &add);
    add = this->Dpi().GetScaleFloat(add, nOldDpiScale);
    m_impl->SetLineSpacing(mul, add, false);

    float fWordSpacing = m_impl->GetWordSpacing();
    fWordSpacing = this->Dpi().GetScaleFloat(fWordSpacing, nOldDpiScale);
    m_impl->SetWordSpacing(fWordSpacing, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

template<typename T>
void LabelTemplate<T>::OnLanguageChanged()
{
    BaseClass::OnLanguageChanged();
    //语言发生变化，字符串长度可能发生了变化，需要重新计算布局，更新ToolTip数据
    this->RelayoutOrRedraw();
    m_impl->CheckShowToolTip();
}

template<typename T>
uint32_t LabelTemplate<T>::GetValidTextStyle(uint32_t nTextFormat)
{
    return LabelImpl::GetValidTextStyle(nTextFormat);
}

template<typename T>
DString LabelTemplate<T>::GetText() const
{
    return m_impl->GetText();
}

template<typename T>
DString LabelTemplate<T>::GetTextId() const
{
    return m_impl->GetTextId();
}

template<typename T>
void LabelTemplate<T>::SetAutoToolTip(bool bAutoShow)
{
    m_impl->SetAutoShowToolTipEnabled(bAutoShow);
}

template<typename T>
bool LabelTemplate<T>::IsAutoToolTip() const
{
    return m_impl->IsAutoShowToolTipEnabled();
}

template<typename T>
void LabelTemplate<T>::SetReplaceNewline(bool bReplaceNewline)
{
    m_impl->SetReplaceNewline(bReplaceNewline);
}

template<typename T>
bool LabelTemplate<T>::IsReplaceNewline() const
{
    return m_impl->IsReplaceNewline();
}

template<typename T>
void LabelTemplate<T>::SetVerticalText(bool bVerticalText)
{
    m_impl->SetVerticalText(bVerticalText);
}

template<typename T>
bool LabelTemplate<T>::IsVerticalText() const
{
    return m_impl->IsVerticalText();
}

template<typename T>
void LabelTemplate<T>::SetLineSpacing(float mul, float add, bool bNeedDpiScale)
{
    m_impl->SetLineSpacing(mul, add, bNeedDpiScale);
}

template<typename T>
void LabelTemplate<T>::GetLineSpacing(float* mul, float* add) const
{
    m_impl->GetLineSpacing(mul, add);
}

template<typename T>
void LabelTemplate<T>::SetWordSpacing(float fWordSpacing, bool bNeedDpiScale)
{
    m_impl->SetWordSpacing(fWordSpacing, bNeedDpiScale);
}

template<typename T>
float LabelTemplate<T>::GetWordSpacing() const
{
    return m_impl->GetWordSpacing();
}

template<typename T>
void LabelTemplate<T>::SetUseFontHeight(bool bUseFontHeight)
{
    m_impl->SetUseFontHeight(bUseFontHeight);
}

template<typename T>
bool LabelTemplate<T>::IsUseFontHeight() const
{
    return m_impl->IsUseFontHeight();
}

template<typename T>
void LabelTemplate<T>::SetRotate90ForAscii(bool bRotate90ForAscii)
{
    m_impl->SetRotate90ForAscii(bRotate90ForAscii);
}

template<typename T>
bool LabelTemplate<T>::IsRotate90ForAscii() const
{
    return m_impl->IsRotate90ForAscii();
}

template<typename T>
MeasureStringParam LabelTemplate<T>::GetMeasureParam() const
{
    return m_impl->GetMeasureParam();
}

template<typename T>
DrawStringParam LabelTemplate<T>::GetDrawParam() const
{
    return m_impl->GetDrawParam();
}

template<typename T /*= Control*/>
void ui::LabelTemplate<T>::SetPos(UiRect rc)
{
    BaseClass::SetPos(rc);
    m_impl->CheckShowToolTip();
}

template<typename T>
DString LabelTemplate<T>::GetToolTipText() const
{
    DString toolTip = BaseClass::GetToolTipText();
    if (toolTip.empty()) {
        toolTip = m_impl->GetAutoToolTipText();
    }
    return toolTip;
}

template<typename T>
std::string LabelTemplate<T>::GetUTF8Text() const
{
    return m_impl->GetUTF8Text();
}

template<typename T>
void LabelTemplate<T>::SetText(const DString& strText)
{
    m_impl->SetText(strText);
}

template<typename T>
void LabelTemplate<T>::SetUTF8Text(const std::string& strText)
{
    m_impl->SetUTF8Text(strText);
}

template<typename T>
void LabelTemplate<T>::SetTextId(const DString& strTextId)
{
    m_impl->SetTextId(strTextId);
}

template<typename T>
void LabelTemplate<T>::SetUTF8TextId(const std::string& strTextId)
{
    m_impl->SetUTF8TextId(strTextId);
}

template<typename T>
bool LabelTemplate<T>::HasHotState()
{
    if (BaseClass::HasHotState()) {
        return true;
    }
    return m_impl->HasHotColorState();
}

template<typename T>
UiSize LabelTemplate<T>::EstimateText(UiSize szAvailable)
{
    return m_impl->EstimateText(szAvailable);
}

template<typename T>
void LabelTemplate<T>::PaintText(IRender* pRender)
{
    m_impl->PaintText(pRender);
}

template<typename T>
void LabelTemplate<T>::SetTextStyle(uint32_t uStyle, bool bRedraw)
{
    m_impl->SetTextStyle(uStyle, bRedraw);
}

template<typename T>
void LabelTemplate<T>::SetDefaultTextStyle(bool bRedraw)
{
    m_impl->SetDefaultTextStyle(bRedraw);
}

template<typename T>
uint32_t LabelTemplate<T>::GetTextStyle() const
{
    return m_impl->GetTextStyle();
}

template<typename T>
DString LabelTemplate<T>::GetStateTextColor(ControlStateType stateType) const
{
    return m_impl->GetStateTextColor(stateType);
}

template<typename T>
void LabelTemplate<T>::SetStateTextColor(ControlStateType stateType, const DString& dwTextColor)
{
    m_impl->SetStateTextColor(stateType, dwTextColor);
}

template<typename T /*= Control*/>
DString ui::LabelTemplate<T>::GetPaintStateTextColor(ControlStateType buttonStateType, ControlStateType& stateType)
{
    return m_impl->GetPaintStateTextColor(buttonStateType, stateType);
}

template<typename T>
DString LabelTemplate<T>::GetFontId() const
{
    return m_impl->GetFontId();
}

template<typename T>
void LabelTemplate<T>::SetFontId(const DString& strFontId)
{
    m_impl->SetFontId(strFontId);
}

template<typename T>
UiPadding LabelTemplate<T>::GetTextPadding() const
{
    return m_impl->GetTextPadding();
}

template<typename T>
void LabelTemplate<T>::SetTextPadding(UiPadding padding, bool bNeedDpiScale)
{
    m_impl->SetTextPadding(padding, bNeedDpiScale);   
}

template<typename T>
bool LabelTemplate<T>::IsSingleLine() const
{
    return m_impl->IsSingleLine();
}

template<typename T>
void LabelTemplate<T>::SetSingleLine(bool bSingleLine)
{
    m_impl->SetSingleLine(bSingleLine);
}

template<typename T>
void LabelTemplate<T>::DoPaintText(const UiRect& rc, IRender* pRender)
{
    m_impl->DoPaintText(rc, pRender);
}

typedef LabelTemplate<Control> Label;
typedef LabelTemplate<Box> LabelBox;
typedef LabelTemplate<HBox> LabelHBox;
typedef LabelTemplate<VBox> LabelVBox;

}

#endif // UI_CONTROL_LABEL_H_
