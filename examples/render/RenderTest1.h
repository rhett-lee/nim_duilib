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
    /** 绘制的图片
    */
    std::unique_ptr<Image> m_pImage;
};

} //end of namespace ui
#endif //EXAMPLES_RENDER_TEST1_H_
