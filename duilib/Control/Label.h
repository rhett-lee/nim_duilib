#ifndef UI_CONTROL_LABEL_H_
#define UI_CONTROL_LABEL_H_

#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/StateColorMap.h"
#include "duilib/Core/DpiManager.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
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
    /** 检查是否需要自动显示ToolTip
    */
    void CheckShowToolTip();

    /** 绘制文字的实现函数
    * @param [in] rc 实际绘制区域，不包含内边距（需由调用方剪去内边距）
    * @param [in] pRender 渲染接口
    */
    void DoPaintText(const UiRect& rc, IRender* pRender);

private:
    //文本内容
    UiString m_sText;

    //文本ID，用于支持多语言
    UiString m_sTextId;

    //字体ID
    UiString m_sFontId;

    //自动显示Tooltip的缓存
    UiString m_sAutoShowTooltipCache;

    //各个状态（默认/悬停/按下/禁用）的文本颜色映射表
    std::unique_ptr<StateColorMap> m_pTextColorMap;

    //文本内边距
    UiPadding16 m_rcTextPadding;

    //文本对齐属性
    uint32_t m_uTextStyle;

    //实际行间距 = 字体大小 × m_fSpacingMul + m_fSpacingAdd
    //行间距倍数: 字体大小的倍数比例（默认值通常为 1.0，即 100% 字体大小），用于按比例调整行间距
    float m_fSpacingMul;

    //行间距附加量: 是固定的附加像素值（默认值通常为 0），用于在比例调整的基础上增加固定偏移（像素）
    float m_fSpacingAdd;

    //每个字符之间的的间隔（像素）
    float m_fWordSpacing;

    //是否单行文本: true表示单行文本，false表示多行文本
    bool m_bSingleLine;

    //是否自动显示Tooltip
    bool m_bAutoShowToolTip;

    //是否替换换行符(将字符串"\\n"替换为换行符"\n"，这样可以在XML中使用括号中这两个字符(\n)来当作换行符，从而支持多行文本)
    bool m_bReplaceNewline;

    //文本方向：true为纵向文本，false为横向文本
    //    横向文本：从左到右，从上到下
    //    纵向文本：从上到下，从右到左
    bool m_bVerticalText;

    //纵向绘制时，使用字体的默认高度，而不是每个字体的高度（显示时所有字体等高）
    bool m_bUseFontHeight;

    //纵向绘制时，对于字母数字等，顺时针旋转90度显示
    bool m_bRotate90ForAscii;
};

template<typename InheritType>
LabelTemplate<InheritType>::LabelTemplate(Window* pWindow) :
    InheritType(pWindow),
    m_sFontId(),
    m_uTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE),
    m_bSingleLine(true),
    m_bAutoShowToolTip(false),
    m_bReplaceNewline(false),
    m_fSpacingMul(1.0f),
    m_fSpacingAdd(0),
    m_fWordSpacing(0),
    m_bVerticalText(false),
    m_bUseFontHeight(true),
    m_bRotate90ForAscii(true),
    m_rcTextPadding(),
    m_sText(),
    m_sTextId()
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
        bool bHCenter = false;        
        size_t centerPos = strValue.find(_T("center"));
        if (centerPos != DString::npos) {
            //"center"这个属性有歧义，保留以保持兼容性，新的属性是"hcenter"
            bHCenter = true;
            size_t vCenterPos = strValue.find(_T("vcenter"));
            if (vCenterPos != DString::npos) {
                if ((vCenterPos + 1) == centerPos) {
                    bHCenter = false;
                }
            }
        }

        //水平对齐方式
        if (strValue.find(_T("hcenter")) != DString::npos) {            
            bHCenter = true;
        }
        if (bHCenter) {
            //水平对齐：居中
            m_uTextStyle &= ~TEXT_HALIGN_ALL;
            m_uTextStyle |= TEXT_HCENTER;
        }
        else if (strValue.find(_T("right")) != DString::npos) {
            //水平对齐：靠右
            m_uTextStyle &= ~TEXT_HALIGN_ALL;
            m_uTextStyle |= TEXT_RIGHT;
        }
        else if (strValue.find(_T("left")) != DString::npos) {
            //水平对齐：靠左
            m_uTextStyle &= ~TEXT_HALIGN_ALL;
            m_uTextStyle |= TEXT_LEFT;
        }
        else if (strValue.find(_T("hjustify")) != DString::npos) {
            //水平对齐：两端对齐
            m_uTextStyle &= ~TEXT_HALIGN_ALL;
            m_uTextStyle |= TEXT_HJUSTIFY;
        }

        //垂直对齐方式
        if (strValue.find(_T("top")) != DString::npos) {
            //垂直对齐：靠上
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_TOP;
        }
        else if (strValue.find(_T("vcenter")) != DString::npos) {
            //垂直对齐：居中
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_VCENTER;
        }
        else if (strValue.find(_T("bottom")) != DString::npos) {
            //垂直对齐：靠下
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_BOTTOM;
        }
        else if (strValue.find(_T("vjustify")) != DString::npos) {
            //垂直对齐：靠下
            m_uTextStyle &= ~TEXT_VALIGN_ALL;
            m_uTextStyle |= TEXT_VJUSTIFY;
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
    else if (strName == _T("replace_newline")) {
        // 设置是否替换换行符(将字符串"\\n"替换为换行符"\n"
        SetReplaceNewline(strValue == _T("true"));
    }
    else if (strName == _T("spacing_mul")) {
        // 设置行间距倍数
        float mul = 1.0f;
        float add = 0;
        GetLineSpacing(&mul, &add);
        mul = StringUtil::StringToFloat(strValue.c_str(), nullptr);
        SetLineSpacing(mul, add, false);
    }
    else if (strName == _T("spacing_add")) {
        // 设置行间距固定的附加像素值
        float mul = 1.0f;
        float add = 0;
        GetLineSpacing(&mul, &add);
        add = StringUtil::StringToFloat(strValue.c_str(), nullptr);
        SetLineSpacing(mul, add, true);
    }
    else if (strName == _T("vertical_text")) {
        // 设置是否为纵向文本
        SetVerticalText(strValue == _T("true"));
    }
    else if (strName == _T("word_spacing")) {
        // 设置两个相邻的字符之间的间隔（像素）
        SetWordSpacing(StringUtil::StringToFloat(strValue.c_str(), nullptr), true);
    }
    else if (strName == _T("use_font_height")) {
        // 设置当纵向绘制文本时，使用字体的默认高度，而不是每个字体的高度（显示时所有字体等高）
        SetUseFontHeight(strValue == _T("true"));
    }
    else if (strName == _T("ascii_rotate_90")) {
        // 设置当纵向绘制文本时，对于字母数字等，顺时针旋转90度显示
        SetRotate90ForAscii(strValue == _T("true"));
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

template<typename InheritType>
void LabelTemplate<InheritType>::ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale)
{
    if (!this->Dpi().CheckDisplayScaleFactor(nNewDpiScale)) {
        return;
    }
    UiPadding rcTextPadding = GetTextPadding();
    rcTextPadding = this->Dpi().GetScalePadding(rcTextPadding, nOldDpiScale);
    this->SetTextPadding(rcTextPadding, false);

    float mul = 1.0f;
    float add = 0;
    GetLineSpacing(&mul, &add);
    add = this->Dpi().GetScaleFloat(add, nOldDpiScale);
    SetLineSpacing(mul, add, false);

    float fWordSpacing = GetWordSpacing();
    fWordSpacing = this->Dpi().GetScaleFloat(fWordSpacing, nOldDpiScale);
    SetWordSpacing(fWordSpacing, false);

    BaseClass::ChangeDpiScale(nOldDpiScale, nNewDpiScale);
}

template<typename InheritType>
uint32_t LabelTemplate<InheritType>::GetValidTextStyle(uint32_t nTextFormat)
{
    uint32_t nValidTextFormat = 0;
    if (nTextFormat & TEXT_HCENTER) {
        nValidTextFormat |= TEXT_HCENTER;
    }
    else if (nTextFormat & TEXT_RIGHT) {
        nValidTextFormat |= TEXT_RIGHT;
    }
    else if (nTextFormat & TEXT_HJUSTIFY) {
        nValidTextFormat |= TEXT_HJUSTIFY;
    }
    else {
        nValidTextFormat |= TEXT_LEFT;
    }

    if (nTextFormat & TEXT_VCENTER) {
        nValidTextFormat |= TEXT_VCENTER;
    }
    else if (nTextFormat & TEXT_BOTTOM) {
        nValidTextFormat |= TEXT_BOTTOM;
    }
    else if (nTextFormat & TEXT_VJUSTIFY) {
        nValidTextFormat |= TEXT_VJUSTIFY;
    }
    else {
        nValidTextFormat |= TEXT_TOP;
    }

    if (nTextFormat & TEXT_SINGLELINE) {
        nValidTextFormat |= TEXT_SINGLELINE;
    }
    if (nTextFormat & TEXT_END_ELLIPSIS) {
        nValidTextFormat |= TEXT_END_ELLIPSIS;
    }
    if (nTextFormat & TEXT_PATH_ELLIPSIS) {
        nValidTextFormat |= TEXT_PATH_ELLIPSIS;
    }
    if (nTextFormat & TEXT_NOCLIP) {
        nValidTextFormat |= TEXT_NOCLIP;
    }

    if (nTextFormat & TEXT_WORD_WRAP) {
        nValidTextFormat |= TEXT_WORD_WRAP;
    }

    if (nTextFormat & TEXT_VERTICAL) {
        nValidTextFormat |= TEXT_VERTICAL;
    }

    return nValidTextFormat;
}

template<typename InheritType>
DString LabelTemplate<InheritType>::GetText() const
{
    DString strText = m_sText.c_str();
    if (strText.empty() && !m_sTextId.empty()) {
        strText = GlobalManager::Instance().Lang().GetStringViaID(m_sTextId.c_str());
    }

    if (IsReplaceNewline()) {
        //将反斜杠+n这两个字符替换成换行符
        StringUtil::ReplaceAll(_T("\\n"), _T("\n"), strText);
    }
    return strText;
}

template<typename InheritType>
DString LabelTemplate<InheritType>::GetTextId() const
{
    return m_sTextId.c_str();
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetAutoToolTip(bool bAutoShow)
{
    m_bAutoShowToolTip = bAutoShow;
    CheckShowToolTip();
}

template<typename InheritType>
bool LabelTemplate<InheritType>::IsAutoToolTip() const
{
    return m_bAutoShowToolTip;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetReplaceNewline(bool bReplaceNewline)
{
    m_bReplaceNewline = bReplaceNewline;
}

template<typename InheritType>
bool LabelTemplate<InheritType>::IsReplaceNewline() const
{
    return m_bReplaceNewline;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetVerticalText(bool bVerticalText)
{
    if (m_bVerticalText != bVerticalText) {
        m_bVerticalText = bVerticalText;
        if (m_bVerticalText) {
            m_uTextStyle |= TEXT_VERTICAL;
        }
        else {
            m_uTextStyle &= ~TEXT_VERTICAL;
        }
        this->Invalidate();
    }
}

template<typename InheritType>
bool LabelTemplate<InheritType>::IsVerticalText() const
{
    return m_bVerticalText;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetLineSpacing(float mul, float add, bool bNeedDpiScale)
{
    if (mul < 0) {
        mul = 1.0f;
    }
    if (add < 0) {
        add = 0;
    }
    if (bNeedDpiScale) {
        add = this->Dpi().GetScaleFloat(add);
    }
    if ((mul != m_fSpacingMul) || (add != m_fSpacingAdd)) {
        m_fSpacingMul = mul;
        m_fSpacingAdd = add;
        this->Invalidate();
    }
}

template<typename InheritType>
void LabelTemplate<InheritType>::GetLineSpacing(float* mul, float* add) const
{
    if (mul != nullptr) {
        *mul = m_fSpacingMul;
    }
    if (add != nullptr) {
        *add = m_fSpacingAdd;
    }
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetWordSpacing(float fWordSpacing, bool bNeedDpiScale)
{
    if (fWordSpacing < 0) {
        fWordSpacing = 0;
    }
    if (bNeedDpiScale) {
        fWordSpacing = this->Dpi().GetScaleFloat(fWordSpacing);
    }
    if (m_fWordSpacing != fWordSpacing) {
        m_fWordSpacing = fWordSpacing;
        if (IsVerticalText()) {
            this->Invalidate();
        }
    }
}

template<typename InheritType>
float LabelTemplate<InheritType>::GetWordSpacing() const
{
    return m_fWordSpacing;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetUseFontHeight(bool bUseFontHeight)
{
    if (m_bUseFontHeight != bUseFontHeight) {
        m_bUseFontHeight = bUseFontHeight;
        if (IsVerticalText()) {
            this->Invalidate();
        }
    }
}

template<typename InheritType>
bool LabelTemplate<InheritType>::IsUseFontHeight() const
{
    return m_bUseFontHeight;
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetRotate90ForAscii(bool bRotate90ForAscii)
{
    if (m_bRotate90ForAscii != bRotate90ForAscii) {
        m_bRotate90ForAscii = bRotate90ForAscii;
        if (IsVerticalText()) {
            this->Invalidate();
        }
    }
}

template<typename InheritType>
bool LabelTemplate<InheritType>::IsRotate90ForAscii() const
{
    return m_bRotate90ForAscii;
}

template<typename InheritType>
MeasureStringParam LabelTemplate<InheritType>::GetMeasureParam() const
{
    MeasureStringParam measureParam;
    measureParam.pFont = this->GetIFontById(this->GetFontId());
    measureParam.uFormat = m_uTextStyle;
    measureParam.fSpacingMul = m_fSpacingMul;
    measureParam.fSpacingAdd = m_fSpacingAdd;
    measureParam.fWordSpacing = m_fWordSpacing;
    measureParam.bUseFontHeight = m_bUseFontHeight;
    measureParam.bRotate90ForAscii = m_bRotate90ForAscii;
    return measureParam;
}

template<typename InheritType>
DrawStringParam LabelTemplate<InheritType>::GetDrawParam() const
{
    DrawStringParam drawParam;
    drawParam.pFont = this->GetIFontById(this->GetFontId());
    drawParam.uFormat = m_uTextStyle;
    drawParam.fSpacingMul = m_fSpacingMul;
    drawParam.fSpacingAdd = m_fSpacingAdd;
    drawParam.fWordSpacing = m_fWordSpacing;
    drawParam.bUseFontHeight = m_bUseFontHeight;
    drawParam.bRotate90ForAscii = m_bRotate90ForAscii;
    return drawParam;
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

    int32_t rectSize = 0;
    if (!IsVerticalText()) {
        int32_t width = this->GetFixedWidth().GetInt32();
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
        rectSize = width;
    }
    else {
        int32_t height = this->GetFixedHeight().GetInt32();
        if (this->GetFixedHeight().IsStretch()) {
            height = 0;
        }
        if (height < 0) {
            height = 0;
        }
        if (!m_bSingleLine && (height == 0)) {
            //多行文本评估宽高的时候，必须指定高度
            height = rc.Height();
        }
        rectSize = height;
    }

    MeasureStringParam measureParam = GetMeasureParam();
    measureParam.rectSize = rectSize;
    UiRect rcMessure = pRender->MeasureString(sText, measureParam);
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
    int32_t nWidth = szAvailable.cx;
    int32_t nHeight = szAvailable.cy;
    const UiPadding rcTextPadding = this->GetTextPadding();
    const UiPadding rcPadding = this->GetControlPadding();
    if (!m_bVerticalText) {
        //文本方向：横向
        if (this->GetFixedWidth().IsStretch()) {
            //如果是拉伸类型，使用外部宽度
            nWidth = CalcStretchValue(this->GetFixedWidth(), szAvailable.cx);
        }
        else if (this->GetFixedWidth().IsInt32()) {
            nWidth = this->GetFixedWidth().GetInt32();
        }
        else if (this->GetFixedWidth().IsAuto()) {
            //宽度为自动时，不限制宽度
            nWidth = this->GetMaxWidth();
            if (nWidth != INT32_MAX) {
                nWidth -= (rcPadding.left + rcPadding.right);
                nWidth -= (rcTextPadding.left + rcTextPadding.right);
            }
            if (nWidth <= 0) {
                nWidth = INT32_MAX;
            }
        }
        if (!this->GetFixedWidth().IsAuto()) {
            nWidth -= (rcPadding.left + rcPadding.right);
            nWidth -= (rcTextPadding.left + rcTextPadding.right);
        }
        if (nWidth < 0) {
            nWidth = 0;
        }
    }
    else {
        //文本方向：纵向
        if (this->GetFixedHeight().IsStretch()) {
            //如果是拉伸类型，使用外部高度
            nHeight = CalcStretchValue(this->GetFixedHeight(), szAvailable.cy);
        }
        else if (this->GetFixedHeight().IsInt32()) {
            nHeight = this->GetFixedHeight().GetInt32();
        }
        else if (this->GetFixedHeight().IsAuto()) {
            //宽度为自动时，不限制宽度
            nHeight = this->GetMaxHeight();
            if (nHeight != INT32_MAX) {
                nHeight -= (rcPadding.top + rcPadding.bottom);
                nHeight -= (rcTextPadding.top + rcTextPadding.bottom);
            }
            if (nHeight <= 0) {
                nHeight = INT32_MAX;
            }
        }
        if (!this->GetFixedHeight().IsAuto()) {
            nHeight -= (rcPadding.top + rcPadding.bottom);
            nHeight -= (rcTextPadding.top + rcTextPadding.bottom);
        }
        if (nHeight < 0) {
            nHeight = 0;
        }
    }
    UiSize fixedSize;
    DString textValue = GetText();
    if (!textValue.empty() && (this->GetWindow() != nullptr)) {
        auto pRender = this->GetWindow()->GetRender();
        if (pRender != nullptr) {
            MeasureStringParam measureParam = GetMeasureParam();
            measureParam.rectSize = !m_bVerticalText ? nWidth : nHeight;
            UiRect rect = pRender->MeasureString(textValue, measureParam);
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

    DrawStringParam drawParam = GetDrawParam();//绘制参数
    drawParam.textRect = rc;

    if (this->GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot)) {
        if ((stateType == kControlStateNormal || stateType == kControlStateHot) && 
            !GetStateTextColor(kControlStateHot).empty()) {
            DString clrColor = GetStateTextColor(kControlStateNormal);
            if (!clrColor.empty()) {                
                drawParam.dwTextColor = this->GetUiColor(clrColor);
                drawParam.uFade = 255;
                pRender->DrawString(textValue, drawParam);
            }

            if (this->GetHotAlpha() > 0) {
                DString textColor = GetStateTextColor(kControlStateHot);
                if (!textColor.empty()) {
                    drawParam.dwTextColor = this->GetUiColor(textColor);
                    drawParam.uFade = (uint8_t)this->GetHotAlpha();
                    pRender->DrawString(textValue, drawParam);
                }
            }
            return;
        }
    }

    drawParam.dwTextColor = dwClrColor;
    drawParam.uFade = 255;
    pRender->DrawString(textValue, drawParam);
}

template<typename InheritType>
void LabelTemplate<InheritType>::SetTextStyle(uint32_t uStyle, bool bRedraw)
{
    m_uTextStyle = GetValidTextStyle(uStyle);
    if (m_uTextStyle & TEXT_SINGLELINE) {
        m_bSingleLine = true;
    }
    else {
        m_bSingleLine = false;
    }
    if (m_uTextStyle & TEXT_VERTICAL) {
        m_bVerticalText = true;
    }
    else {
        m_bVerticalText = false;
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
uint32_t LabelTemplate<InheritType>::GetTextStyle() const
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
        m_pTextColorMap = std::make_unique<StateColorMap>();
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
    if (m_bSingleLine != bSingleLine) {
        m_bSingleLine = bSingleLine;
        if (m_bSingleLine) {
            m_uTextStyle |= TEXT_SINGLELINE;
        }
        else {
            m_uTextStyle &= ~TEXT_SINGLELINE;
        }
        this->Invalidate();
    }   
}

typedef LabelTemplate<Control> Label;
typedef LabelTemplate<Box> LabelBox;
typedef LabelTemplate<HBox> LabelHBox;
typedef LabelTemplate<VBox> LabelVBox;

}

#endif // UI_CONTROL_LABEL_H_
