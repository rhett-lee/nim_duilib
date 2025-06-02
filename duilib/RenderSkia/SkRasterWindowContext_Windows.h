#ifndef UI_RENDER_SKIA_RASTER_WINDOW_CONTEXT_WINDOWS_H_
#define UI_RENDER_SKIA_RASTER_WINDOW_CONTEXT_WINDOWS_H_

#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include "duilib/duilib_config_windows.h"

#include "SkiaHeaderBegin.h"
#include "include/core/SkSurface.h"
#include "tools/window/RasterWindowContext.h"
#include "SkiaHeaderEnd.h"

// DisplayParams.fGrContextOptions 类型为GrContextOptions:
// 在GR_TEST_UTILS宏定义和不定义的情况下，结构体大小会不同，如果不一致会导致程序崩溃，注意检查该宏定义的一致性
#ifndef SK_GL
    #error 请检查宏定义：SK_GL与Skia库的编译选项保持一致
#endif

namespace ui 
{
class UiRect;
class IRender;
class IRenderPaint;

/** 该类的来源：skia\tools\window\win\RasterWindowContext_win.cpp，做了修改
*   函数和变量命名规则与Skia一致，以便于后续同步代码时方便比对代码修改。
*/
class SkRasterWindowContext_Windows: public skwindow::internal::RasterWindowContext
{
public:
    SkRasterWindowContext_Windows(HWND hWnd, std::unique_ptr<const skwindow::DisplayParams> params);
    SkRasterWindowContext_Windows(const SkRasterWindowContext_Windows& r) = delete;
    SkRasterWindowContext_Windows& operator = (const SkRasterWindowContext_Windows& r) = delete;
    virtual ~SkRasterWindowContext_Windows() override;

public:
    virtual sk_sp<SkSurface> getBackbufferSurface() override;
    virtual bool isValid() override { return SkToBool(m_hWnd); }
    virtual void resize(int w, int h) override;
    virtual void setDisplayParams(std::unique_ptr<const skwindow::DisplayParams> params) override;

public:
    /** 绘制并刷新到屏幕（Render的实现已经与窗口关联）, 同步完成
    * @param [in] pRender 渲染引擎的接口
    * @param [in] pRenderPaint 界面绘制所需的回调接口
    */
    bool PaintAndSwapBuffers(IRender* pRender, IRenderPaint* pRenderPaint);

    /** 获取位图GDI句柄
    */
    HBITMAP GetHBitmap() const;

protected:
    virtual void onSwapBuffers() override;

    /** 绘制结束后，绘制数据从渲染引擎更新到窗口
    * @param [in] hPaintDC 当前绘制的DC
    * @param [in] rcPaint 绘制的区域
    * @param [in] pRender 绘制引擎接口，用于将绘制结果应用到窗口
    * @param [in] nLayeredWindowAlpha 窗口透明度，在UpdateLayeredWindow函数中作为参数使用
    * @return 成功返回true，失败则返回false
    */
    bool SwapPaintBuffers(HDC hPaintDC, const UiRect& rcPaint, IRender* pRender, uint8_t nLayeredWindowAlpha) const;

    /** 获取当前窗口的客户区矩形
    * @param [out] rcClient 返回窗口的客户区坐标
    */
    void GetClientRect(UiRect& rcClient) const;

    /** 获取当前窗口的窗口区矩形
    * @param [out] rcWindow 返回窗口左上角和右下角的屏幕坐标
    */
    void GetWindowRect(UiRect& rcWindow) const;

    /** 创建一个设备无关的位图
    *@return 返回位图句柄，由调用方释放位图资源
    */
    HBITMAP CreateHBitmap(int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits) const;

private:
    /** Surface接口
    */
    sk_sp<SkSurface> m_fBackbufferSurface;

    /** 窗口句柄
    */
    HWND m_hWnd;

    /** 位图GDI句柄
    */
    HBITMAP m_hBitmap;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RASTER_WINDOW_CONTEXT_WINDOWS_H_

#endif //DUILIB_BUILD_FOR_WIN
