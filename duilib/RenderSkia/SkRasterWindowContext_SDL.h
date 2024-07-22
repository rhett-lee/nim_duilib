#ifndef UI_RENDER_SKIA_RASTER_WINDOW_CONTEXT_SDL_H_
#define UI_RENDER_SKIA_RASTER_WINDOW_CONTEXT_SDL_H_

#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_SDL

#pragma warning (push)
#pragma warning (disable: 4244 4201 4100)

#include "include/core/SkSurface.h"
#include "src/base/SkAutoMalloc.h"
#include "tools/window/RasterWindowContext.h"

// DisplayParams.fGrContextOptions 类型为GrContextOptions:
// 在GR_TEST_UTILS宏定义和不定义的情况下，结构体大小会不同，如果不一致会导致程序崩溃
// 所以在此检测宏定义的一致性
#ifndef SK_GL
    #error 请检查宏定义：SK_GL与Skia库的编译选项保持一致
#endif

#ifdef _DEBUG
    #ifndef GR_TEST_UTILS
        #error 请检查宏定义：GR_TEST_UTILS与Skia库的编译选项保持一致（DisplayParams/GrContextOptions）
    #endif
#else
    #ifdef GR_TEST_UTILS
        #error 请检查宏定义：GR_TEST_UTILS与Skia库的编译选项保持一致（DisplayParams/GrContextOptions）
    #endif
#endif

#pragma warning (pop)

//SDL的类型，提前声明
struct SDL_Window;

namespace ui 
{
class UiRect;
class IRender;
class IRenderPaint;

/** 该类的来源：skia\tools\window\win\RasterWindowContext_win.cpp，做了修改
*   函数和变量命名规则与Skia一致，以便于后续同步代码时方便比对代码修改。
*/
class SkRasterWindowContext_SDL: public skwindow::internal::RasterWindowContext
{
public:
    SkRasterWindowContext_SDL(SDL_Window* sdlWindow, const skwindow::DisplayParams& params);
    SkRasterWindowContext_SDL(const SkRasterWindowContext_SDL& r) = delete;
    SkRasterWindowContext_SDL& operator = (const SkRasterWindowContext_SDL& r) = delete;
    virtual ~SkRasterWindowContext_SDL() override;

public:
    virtual sk_sp<SkSurface> getBackbufferSurface() override;
    virtual bool isValid() override { return m_sdlWindow != nullptr; }
    virtual void resize(int w, int h) override;
    virtual void setDisplayParams(const skwindow::DisplayParams& params) override;

public:
    /** 绘制并刷新到屏幕（Render的实现已经与窗口关联）, 同步完成
    * @param [in] pRender 渲染引擎的接口
    * @param [in] pRenderPaint 界面绘制所需的回调接口
    */
    bool PaintAndSwapBuffers(IRender* pRender, IRenderPaint* pRenderPaint);

protected:
    virtual void onSwapBuffers() override;

    /** 绘制结束后，绘制数据从渲染引擎更新到窗口
    * @param [in] hPaintDC 当前绘制的DC
    * @param [in] rcPaint 绘制的区域
    * @param [in] pRender 绘制引擎接口，用于将绘制结果应用到窗口
    * @param [in] nLayeredWindowAlpha 窗口透明度，在UpdateLayeredWindow函数中作为参数使用
    * @return 成功返回true，失败则返回false
    */
    bool SwapPaintBuffers(const UiRect& rcPaint, uint8_t nLayeredWindowAlpha) const;

    /** 获取当前窗口的客户区矩形
    * @param [out] rcClient 返回窗口的客户区坐标
    */
    void GetClientRect(UiRect& rcClient) const;

private:
    /** Surface数据
    */
    SkAutoMalloc m_fSurfaceMemory;

    /** Surface接口
    */
    sk_sp<SkSurface> m_fBackbufferSurface;

    /** 关联的窗口
    */
    SDL_Window* m_sdlWindow;
};

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL

#endif // UI_RENDER_SKIA_RASTER_WINDOW_CONTEXT_SDL_H_
