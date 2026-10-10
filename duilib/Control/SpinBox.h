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
*   - 提供 SetValue/GetValue/SetRange 等语义化接口
*
*   XML 属性（除 RichEdit 通用属性外）：
*     step        步长值，正整数，默认 1
*     value       初始值（整数）
*     spin_class  步进按钮样式（同 RichEdit，格式："spin容器class,上按钮class,下按钮class"，
*                 可参考 global.xml 中的 rich_edit_spin 设置）
*
*   数值变化时触发 kEventTextChanged 事件（与 RichEdit 一致）。
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

    /** 设置步长（步进按钮和上下方向键每次调整的数值）
    * @param [in] nStep 步长值，必须大于0，默认值为1
    */
    void SetStep(int32_t nStep);

    /** 获取步长
    */
    int32_t GetStep() const;

    /** 设置当前数值（超出min/max范围时会被修正到边界值）
    */
    void SetValue(int64_t nValue);

    /** 获取当前数值
    */
    int64_t GetValue() const;

    /** 设置数值范围
    * @param [in] nMin 最小值
    * @param [in] nMax 最大值
    */
    void SetRange(int64_t nMin, int64_t nMax);
};

} // namespace ui

#endif // UI_CONTROL_SPINBOX_H_
