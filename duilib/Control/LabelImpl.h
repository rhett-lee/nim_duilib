#ifndef UI_CONTROL_LABEL_IMPL_H_
#define UI_CONTROL_LABEL_IMPL_H_

#include "duilib/Core/Control.h"
#include "duilib/Render/IRender.h"

namespace ui
{
class TextDrawer;

/** 标签控件的内部实现，用于显示文本
*/
class UILIB_API LabelImpl
{
public:
    explicit LabelImpl(Control* pOwner);
    ~LabelImpl();

    /// 重写父类方法，提供个性化功能，请参考父类声明
    DString GetText() const;
    std::string GetUTF8Text() const;
    DString GetTextId() const;
    void SetText(const DString& strText);
    void SetUTF8Text(const std::string& strText);
    void SetTextId(const DString& strTextId);
    void SetUTF8TextId(const std::string& strTextId);
    bool HasHotColorState();
    bool SetAttribute(const DString& strName, const DString& strValue);
    void PaintText(IRender* pRender);

    /** 绑定的窗口发生了变化
    */
    void OnWindowChanged();

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale);
    
    /** 计算文本区域大小（宽和高）
     *  @param [in] szAvailable 可用大小，不包含内边距，不包含外边距
     *  @return 控件的文本估算大小，包含内边距(Box)，不包含外边距
     */
    UiSize EstimateText(UiSize szAvailable);

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
    void SetAutoShowToolTipEnabled(bool bAutoShow);

    /** 获取鼠标悬浮到控件显示的提示文本是否省略号出现时才显示
    */
    bool IsAutoShowToolTipEnabled() const;

    /** 获取自动显示的提示文本信息
    */
    DString GetAutoToolTipText() const;

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

    /** 设置文本内容是否为RichText
    */
    void SetRichText(bool bRichText);

    /** 获取文本内容是否为RichText
    */
    bool IsRichText() const;

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

    /** 检查是否需要自动显示ToolTip
    */
    void CheckShowToolTip();

    /** 绘制文字的实现函数
    * @param [in] rc 实际绘制区域，不包含内边距（需由调用方剪去内边距）
    * @param [in] pRender 渲染接口
    */
    void DoPaintText(const UiRect& rc, IRender* pRender);

private:
    /** 关联控件
    */
    Control* m_pOwner;

    /** 文本绘制的实现（支持RichText文本绘制）
    */
    std::unique_ptr<TextDrawer> m_pTextDrawer;

    //文本内容
    UiString m_sText;

    //文本ID，用于支持多语言
    UiString m_sTextId;

    //字体ID
    UiString m_sFontId;

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

    //是否开启自动显示Tooltip的功能
    bool m_bAutoShowToolTipEnabled;

    //当前是否应自动显示Tooltip
    bool m_bAutoShowTooltip;

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

    //文本内容是否为RichText
    bool m_bRichText;
};

}

#endif // UI_CONTROL_LABEL_IMPL_H_
