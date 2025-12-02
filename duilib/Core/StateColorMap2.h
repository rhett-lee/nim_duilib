#ifndef UI_CORE_STATE_COLOR_MAP2_H_
#define UI_CORE_STATE_COLOR_MAP2_H_

#include "duilib/Render/IRender.h"
#include "duilib/Core/UiTypes.h"

namespace ui 
{
/** 控件状态与颜色值的映射，支持更多属性的实现
*/
class Control;
class IRender;
class UILIB_API StateColorMap2
{
public:
    explicit StateColorMap2(Control* pControl);

    /** 获取颜色值，如果不包含此颜色，则返回空
    * @param [in] stateType 要获取何种状态下的颜色值，参考 ControlStateType 枚举
    * @return 指定状态下设定的颜色字符串
    */
    DString GetStateColor(ControlStateType stateType) const;

    /** 获取某个状态下的控件颜色矩形外边距
     * @param [in] stateType 要获取何种状态下的颜色值，参考 ControlStateType 枚举
     * @return 指定状态下设定的颜色矩形外边距
     */
    UiMargin GetStateColorMargin(ControlStateType stateType) const;

    /** 获取某个状态下的控件颜色矩形圆角大小
     * @param [in] stateType 要获取何种状态下的颜色值，参考 ControlStateType 枚举
     * @return 指定状态下设定的颜色矩形圆角大小
     */
    UiSize GetStateColorRound(ControlStateType stateType) const;

    /** 设置某个状态下的控件颜色、颜色外边距、颜色矩形的圆角大小
     * @param [in] stateType 要设置何种状态下的颜色值，参考 ControlStateType 枚举
     * @param [in] color 要设置的颜色值，该值必须在 global.xml 中存在
     * @param [in] colorMargin 要设置的颜色矩形外边距，如果不设置，则颜色矩形与控件矩形重合
     * @param [in] colorRound 要设置的颜色矩形圆角大小，如果不设置，则颜色矩形跟随控件矩形的形状
     */
    void SetStateColor(ControlStateType stateType, const DString& color);
    void SetStateColorMargin(ControlStateType stateType, const UiMargin& colorMargin);
    void SetStateColorRound(ControlStateType stateType, const UiSize& colorRound);

public:
    /** 是否包含Hot状态的颜色
    */
    bool HasHotColor() const;

    /** 是否含有指定颜色值
    */
    bool HasStateColor(ControlStateType stateType) const;

    /** 是否含有颜色值
    */
    bool HasStateColors() const;

public:
    /** 绘制指定状态的颜色
    */
    void PaintStateColor(IRender* pRender, const UiRect& rcPaint, ControlStateType stateType) const;

private:
    /** 绘制指定状态的颜色(使用预先获取的颜色值)
    */
    void DoPaintStateColor(IRender* pRender, UiRect rcPaint, ControlStateType stateType, UiColor colorValue, int32_t nAlpha = 255) const;

private:
    /** 每个颜色的基本属性
    */
    struct TColorProperty
    {
        //颜色字符串
        UiString m_colorStr;

        //该颜色的外边距
        UiMargin16 m_colorMargin;

        //该颜色的圆角大小
        UiSize16 m_colorRound;
    };

private:
    /** 关联的控件接口
    */
    Control* m_pControl;

    /** 状态与颜色值的映射表
    */
    std::vector<TColorProperty> m_stateColors;
};

} // namespace ui

#endif // UI_CORE_STATE_COLOR_MAP2_H_
