#ifndef EXAMPLES_RENDER_TEST2_H_
#define EXAMPLES_RENDER_TEST2_H_

// duilib
#include "duilib/duilib.h"

namespace ui {

class RenderTest2 : public ui::Control
{
    typedef ui::Control BaseClass;
public:
    explicit RenderTest2(ui::Window* pWindow);
    virtual ~RenderTest2() override;

public:
    /** 绘制控件自身及子控件
     * @param[in] pRender 渲染接口
     * @param[in] rcPaint 指定绘制区域
     */
    virtual void AlphaPaint(IRender* pRender, const UiRect& rcPaint) override;

    /** 绘制控件自身
    * @param[in] pRender 渲染接口
    * @param[in] rcPaint 指定绘制区域
    */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

    /** 绘制控件的子控件
    * @param[in] pRender 渲染接口
    * @param[in] rcPaint 指定绘制区域
    */
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

private:
    /** 对整型值进行DPI自适应
    */
    int32_t DpiScaledInt(int32_t iValue);
    float DpiScaledFloat(int32_t iValue);

    /** 在一个矩形内绘制正六边形拼接的颜色地图, 尽量充满整个矩形
    */
    void DrawColorMap(IRender* pRender, const UiRect& rect);

    /** 绘制一个正六边形(采用多边形顶点的方式进行路径填充)
    * @param [in] pRender 渲染接口
    * @param [in] centerPt 正六边形的中心点坐标
    * @param [in] radius 正六边形的半径，中心点距离顶点的距离（同时也是边长）
    * @param [in] penColor 画笔的颜色，如果为0，则不绘制边框
    * @param [in] penWidth 画笔的宽度，如果为0，则不绘制边框
    * @param [in] brushColor 画刷的颜色，如果为0，则填充颜色
    */
    bool DrawRegularHexagon(IRender* pRender, const UiPointF& centerPt, int32_t radius, 
                            const UiColor& penColor, int32_t penWidth, const UiColor& brushColor);

    /** 绘制一个正六边形(采用三角形旋转的方式绘制)
    * @param [in] pRender 渲染接口
    * @param [in] centerPt 正六边形的中心点坐标
    * @param [in] radius 正六边形的半径，中心点距离顶点的距离（同时也是边长）
    * @param [in] penColor 画笔的颜色，如果为0，则不绘制边框
    * @param [in] penWidth 画笔的宽度，如果为0，则不绘制边框
    * @param [in] brushColor 画刷的颜色，如果为0，则填充颜色
    */
    bool DrawRegularHexagon3(IRender* pRender, const UiPoint& centerPt, int32_t radius, 
                             const UiColor& penColor, int32_t penWidth, const UiColor& brushColor);

};

} //end of namespace ui
#endif //EXAMPLES_RENDER_TEST2_H_
