#ifndef UI_RENDER_SKIA_GL_WINDOW_CONTEXT_WINDOWS_H_
#define UI_RENDER_SKIA_GL_WINDOW_CONTEXT_WINDOWS_H_

#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include "duilib/duilib_config_windows.h"

#include "SkiaHeaderBegin.h"

#include "include/core/SkSurface.h"
#include "tools/window/GLWindowContext.h"

// DisplayParams.fGrContextOptions 类型为GrContextOptions:
// 在GR_TEST_UTILS宏定义和不定义的情况下，结构体大小会不同，如果不一致会导致程序崩溃，注意检查该宏定义的一致性
#ifndef SK_GL
    #error 请检查宏定义：SK_GL与Skia库的编译选项保持一致
#endif

#include "SkiaHeaderEnd.h"

namespace ui 
{
class UiRect;
class IRender;
class IRenderPaint;

/** 该类的来源：skia\tools\window\win\GLWindowContext_win.cpp，做了修改
*   函数和变量命名规则与Skia一致，以便于后续同步代码时方便比对代码修改。
*/
class SkGLWindowContext_Windows: public skwindow::internal::GLWindowContext
{
    typedef skwindow::internal::GLWindowContext BaseClass;
public:
    SkGLWindowContext_Windows(HWND hWnd, std::unique_ptr<const skwindow::DisplayParams> params);
    SkGLWindowContext_Windows(const SkGLWindowContext_Windows& r) = delete;
    SkGLWindowContext_Windows& operator = (const SkGLWindowContext_Windows& r) = delete;
    virtual ~SkGLWindowContext_Windows() override;

public:
    /** 绘制并刷新到屏幕（Render的实现已经与窗口关联）, 同步完成
    * @param [in] pRender 渲染引擎的接口
    * @param [in] pRenderPaint 界面绘制所需的回调接口
    */
    bool PaintAndSwapBuffers(IRender* pRender, IRenderPaint* pRenderPaint);

protected:
    virtual void resize(int w, int h) override;
    virtual void onSwapBuffers() override;
    virtual sk_sp<SkSurface> getBackbufferSurface() override;
    virtual sk_sp<const GrGLInterface> onInitializeContext() override;
    virtual void onDestroyContext() override;

    /** 绘制结束后，绘制数据从渲染引擎更新到窗口
    * @param [in] hPaintDC 当前绘制的DC
    * @param [in] rcPaint 绘制的区域
    * @param [in] pRender 绘制引擎接口，用于将绘制结果应用到窗口
    * @param [in] nWindowAlpha 窗口透明度
    * @return 成功返回true，失败则返回false
    */
    bool SwapPaintBuffers(HDC hPaintDC, const UiRect& rcPaint, IRender* pRender, uint8_t nWindowAlpha) const;

private:
    /** 窗口句柄
    */
    HWND m_hWnd;

    /** OpenGL的渲染上下文
    */
    HGLRC m_fHGLRC;
};

} // namespace ui

#endif // UI_RENDER_SKIA_GL_WINDOW_CONTEXT_WINDOWS_H_

#endif //DUILIB_BUILD_FOR_WIN
