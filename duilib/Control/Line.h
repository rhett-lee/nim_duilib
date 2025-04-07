#ifndef UI_CONTROL_LINE_H_
#define UI_CONTROL_LINE_H_

#include "duilib/Core/Control.h"

namespace ui
{
/** 画线控件
*/
class Line: public Control
{
    typedef Control BaseClass;
public:
    explicit Line(Window* pWindow);

    /** 获取控件类型
    */
    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

    /** DPI发生变化，更新控件大小和布局
    * @param [in] nOldDpiScale 旧的DPI缩放百分比
    * @param [in] nNewDpiScale 新的DPI缩放百分比，与Dpi().GetScale()的值一致
    */
    virtual void ChangeDpiScale(uint32_t nOldDpiScale, uint32_t nNewDpiScale) override;

    /** 设置水平还是垂直(true表示垂直)
    */
    void SetLineVertical(bool bVertical);

    /** 获取水平还是垂直
    */
    bool IsLineVertical() const;

    /** 设置线条的宽度
    * @param [in] fLineWidth 线条宽度
    * @param [in] bNeedDpiScale 是否支持DPI缩放
    */
    void SetLineWidth(float fLineWidth, bool bNeedDpiScale);

    /** 获取线条的宽度
    */
    float GetLineWidth() const;

    /** 设置线条颜色
    */
    void SetLineColor(const DString& lineColor);

    /** 获取线条颜色
    */
    DString GetLineColor() const;

    /** 设置线条类型
    */
    void SetLineDashStyle(const DString& dashStyle);

    /** 获取线条类型
    */
    DString GetLineDashStyle() const;

protected:
    /** 绘制控件的入口函数
    * @param [in] pRender 指定绘制区域
    * @param [in] rcPaint 指定绘制坐标
    */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

private:
    /** 水平还是垂直(true表示垂直)
    */
    bool m_bLineVertical;

    /** 线条颜色
    */
    UiString m_lineColor;

    /** 线条宽度
    */
    float m_fLineWidth;

    /** 线条类型
    */
    int8_t m_dashStyle;
};

}//namespace ui

#endif //UI_CONTROL_LINE_H_
