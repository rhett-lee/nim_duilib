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

    /** 将本窗口的OpenGL上下文设置为当前上下文
    * @return 成功返回true，失败返回false
    * @note 同一线程内可能存在多个使用OpenGL绘制的窗口，而每个线程只能有一个当前上下文，
    *       所有访问OpenGL的入口（绘制、读取后台缓冲、销毁等）都必须先调用该函数；
    *       离屏渲染复用窗口上下文时，也需要先调用该函数（因此为public）
    */
    bool MakeCurrent();

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
    /** 释放窗口的DC
    */
    void ReleaseWindowDC();

    /** 获取窗口后台缓冲对应的SkSurface（惰性创建并缓存，窗口缩放后重建）
    * @return 返回包装窗口默认帧缓冲的SkSurface；窗口尺寸无效或上下文不可用时返回nullptr
    * @note 该函数必须在本窗口的OpenGL上下文为当前上下文时调用
    */
    sk_sp<SkSurface> GetWindowSurface();

private:
    /** 窗口句柄
    */
    HWND m_hWnd;

    /** 创建OpenGL上下文时使用的窗口DC，与上下文生命周期一致
    */
    HDC m_hDC;

    /** OpenGL的渲染上下文
    */
    HGLRC m_fHGLRC;

    /** 持久化的GPU绘制表面（FBO）
    * 界面绘制到该表面，每帧只重绘脏区域，然后把整幅内容拷贝到窗口的后台缓冲，
    * 避免每帧重绘整个窗口（大窗口下的小动画可显著降低CPU/GPU占用）
    */
    sk_sp<SkSurface> m_persistentSurface;

    /** 窗口后台缓冲（OpenGL默认帧缓冲，即FBO 0）对应的SkSurface
    * 注：不能使用基类的getBackbufferSurface()，因为基类按"当前绑定的FBO"来包装，
    *     而持久化表面创建或使用后，当前绑定的FBO可能已经不是窗口的默认帧缓冲
    */
    sk_sp<SkSurface> m_windowSurface;

    /** 是否需要完整重绘（首次绘制、尺寸变化、持久化表面重建后为true）
    */
    bool m_bNeedFullRedraw;
};

} // namespace ui

#endif // UI_RENDER_SKIA_GL_WINDOW_CONTEXT_WINDOWS_H_

#endif //DUILIB_BUILD_FOR_WIN
