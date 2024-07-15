#ifndef UI_RENDER_SKIA_RENDER_WINDOW_CONTEXT_H_
#define UI_RENDER_SKIA_RENDER_WINDOW_CONTEXT_H_

#include "duilib/duilib_config_windows.h"

#pragma warning (push)
#pragma warning (disable: 4244 4201 4100)

#include "tools/window/WindowContext.h"

#pragma warning (pop)

namespace ui 
{
class IRenderPaint;
class IRender;

/** 适配Render的WindowContext
*/
class RenderWindowContext : public skwindow::WindowContext {
public:
    RenderWindowContext(const skwindow::DisplayParams& params) : WindowContext(params) {}

public:
    /** 绘制并刷新到屏幕（Render的实现已经与窗口关联）, 同步完成
    * @param [in] pRender 渲染引擎的接口
    * @param [in] pRenderPaint 界面绘制所需的回调接口
    */
    virtual bool PaintAndSwapBuffers(IRender* pRender, IRenderPaint* pRenderPaint) = 0;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RENDER_WINDOW_CONTEXT_H_
