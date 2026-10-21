#ifndef UI_CONTROL_SPINBOX_H_
#define UI_CONTROL_SPINBOX_H_

#include "duilib/Control/RichEdit.h"

namespace ui
{

/** 数字输入框控件（SpinBox）：基于 RichEdit 的数字输入 + 步进按钮的包装
*
*   与直接使用 RichEdit 的 spin_class 相比，本控件提供了：
*   - 默认开启 number_only 模式，无需额外设置
*   - 支持步长（step）设置，步进按钮和上下方向键按步长调整数值
*   - 支持小数（decimals）、数值范围（min/max）、千分位分隔符与前后缀
*   - 提供 SetValue/GetValue/SetRange 等语义化接口
*
*   XML 属性（除 RichEdit 通用属性外）：
*     step              步长值，double，必须大于 0，默认 1
*     value             初始值，double，默认 0
*     decimals          小数位数（0~10），默认 0；为 0 时与普通整数 SpinBox 行为一致
*     min_number/max_number  数值范围，按 double 解析；别名 min_value/max_value
*     prefix            显示前缀（如 "$ "），默认空
*     suffix            显示后缀（如 " kg"），默认空
*     group_separator   千分位分隔符（如 ","），默认空表示不分组；别名 thousand_separator
*     spin_class        步进按钮样式（同 RichEdit，格式："spin容器class,上按钮class,下按钮class"，
*                       可参考 global.xml 中的 rich_edit_spin 设置）
*
*   显示格式：prefix + 千分位整数[.小数] + suffix，例如 "$ 1,234.50 kg"。
*   XML 属性按声明顺序生效；数值最终会按 decimals 四舍五入并钳位到 [min, max]。
*
*   数值变化时触发 kEventTextChanged 事件（与 RichEdit 一致）。
*   失焦时会重新格式化文本；无法解析的输入恢复为最近一次有效值。
*/
class DUILIB_API SpinBox : public RichEdit
{
    typedef RichEdit BaseClass;
public:
    explicit SpinBox(Window* pWindow);
    SpinBox(const SpinBox& r) = delete;
    SpinBox& operator=(const SpinBox& r) = delete;
    virtual ~SpinBox() override = default;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** 调整数值（步进按钮和自动重复定时器调用；按 nDelta 的方向使用当前步长）
    */
    virtual void AdjustTextNumber(int32_t nDelta) override;

    /** 设置步长（步进按钮和上下方向键每次调整的数值）
    * @param [in] nStep 步长值，必须大于0，默认值为1
    */
    void SetStep(int32_t nStep);

    /** 设置步长（支持小数）
    * @param [in] dStep 步长值，必须大于0，默认值为1
    */
    void SetStep(double dStep);

    /** 获取步长（取整后的值）
    */
    int32_t GetStep() const;

    /** 获取步长（精确值，可能带小数）
    */
    double GetStepDouble() const;

    /** 设置当前数值（超出min/max范围时会被修正到边界值，并四舍五入到 decimals 位小数）
    */
    void SetValue(int64_t nValue);

    /** 设置当前数值（支持小数；超出范围时会被修正到边界值）
    */
    void SetValue(double dValue);

    /** 获取当前数值（取整后的值；精确值请用 GetValueDouble）
    */
    int64_t GetValue() const;

    /** 获取当前数值（精确值；文本无法解析时返回最近一次有效值）
    */
    double GetValueDouble() const;

    /** 设置数值范围
    * @param [in] nMin 最小值
    * @param [in] nMax 最大值
    */
    void SetRange(int64_t nMin, int64_t nMax);

    /** 设置数值范围（支持小数）
    * @param [in] dMin 最小值
    * @param [in] dMax 最大值
    */
    void SetRange(double dMin, double dMax);

    /** 设置最小值
    */
    void SetMinValue(double dMin);

    /** 获取最小值
    */
    double GetMinValue() const;

    /** 设置最大值
    */
    void SetMaxValue(double dMax);

    /** 获取最大值
    */
    double GetMaxValue() const;

    /** 设置小数位数（0~10，超出范围会被修正）
    */
    void SetDecimals(int32_t nDecimals);

    /** 获取小数位数
    */
    int32_t GetDecimals() const;

    /** 设置显示前缀（如 "$ "）
    */
    void SetPrefix(const DString& prefix);

    /** 获取显示前缀
    */
    DString GetPrefix() const;

    /** 设置显示后缀（如 " kg"）
    */
    void SetSuffix(const DString& suffix);

    /** 获取显示后缀
    */
    DString GetSuffix() const;

    /** 设置千分位分隔符（如 ","；空字符串表示不分组）
    */
    void SetGroupSeparator(const DString& separator);

    /** 获取千分位分隔符
    */
    DString GetGroupSeparator() const;

protected:
    //消息处理函数
    virtual bool OnKeyDown(const EventArgs& msg) override;
    virtual bool OnKillFocus(const EventArgs& msg) override;

private:
    /** 根据 decimals/前后缀/千分位配置更新输入过滤方式
    */
    void UpdateInputFilter();

    /** 按当前步长调整数值（delta 的符号决定方向）
    */
    void AdjustValue(double delta);

    /** 按当前格式重新生成文本；无法解析时恢复最近一次有效值
    */
    void NormalizeText();

    /** 解析文本为数值（支持前后缀与千分位）
    */
    bool TryParseValue(const DString& text, double& dValue) const;

    /** 按当前格式生成显示文本
    */
    DString FormatValue(double dValue) const;

    /** 按 decimals 四舍五入
    */
    double RoundValue(double dValue) const;

    /** 钳位到 [min, max]
    */
    double ClampValue(double dValue) const;

private:
    int32_t m_decimals;          //小数位数
    double m_step;               //步长
    double m_value;              //最近一次有效值
    double m_minValue;           //最小值
    double m_maxValue;           //最大值
    DString m_prefix;            //显示前缀
    DString m_suffix;            //显示后缀
    DString m_groupSeparator;    //千分位分隔符
    bool m_bCustomLimitChars;    //调用方是否自行设置过 limit_chars
};

} // namespace ui

#endif // UI_CONTROL_SPINBOX_H_
