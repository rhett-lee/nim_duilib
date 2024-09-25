#ifndef EXAMPLES_RENDER_TEST1_H_
#define EXAMPLES_RENDER_TEST1_H_

// duilib
#include "duilib/duilib.h"

namespace ui {

class RenderTest1 : public ui::Control
{
    typedef ui::Control BaseClass;
public:
    explicit RenderTest1(ui::Window* pWindow);
    virtual ~RenderTest1() override;

public:
    /**
    * @brief 待补充
    * @param[in] 待补充
    * @return 待补充
    */
    virtual void AlphaPaint(IRender* pRender, const UiRect& rcPaint) override;

    /**
    * @brief 绘制控件的入口函数
    * @param[in] pRender 指定绘制区域
    * @param[in] rcPaint 指定绘制坐标
    * @return 无
    */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

    /**
    * @brief 绘制控件子项入口函数
    * @param[in] pRender 指定绘制区域
    * @param[in] rcPaint 指定绘制坐标
    * @return 无
    */
    virtual void PaintChild(IRender* pRender, const UiRect& rcPaint) override;

};

} //end of namespace ui
#endif //EXAMPLES_RENDER_TEST1_H_
