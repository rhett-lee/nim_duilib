#ifndef UI_RENDER_SKIA_RASTER_WINDOW_CONTEXT_WINDOWS_H_
#define UI_RENDER_SKIA_RASTER_WINDOW_CONTEXT_WINDOWS_H_

#include "RenderWindowContext.h"

#pragma warning (push)
#pragma warning (disable: 4244 4201 4100)

#include "include/core/SkSurface.h"

#pragma warning (pop)

namespace ui 
{
class UiRect;

/** 该类的来源：skia\tools\window\win\RasterWindowContext_win.cpp，做了修改
*   函数和变量命名规则与Skia一致，以便于后续同步代码时方便比对代码修改。
*/
class SkRasterWindowContext_Windows: public RenderWindowContext
{
public:
    SkRasterWindowContext_Windows(HWND hWnd, const skwindow::DisplayParams& params);
    SkRasterWindowContext_Windows(const SkRasterWindowContext_Windows& r) = delete;
    SkRasterWindowContext_Windows& operator = (const SkRasterWindowContext_Windows& r) = delete;
    virtual ~SkRasterWindowContext_Windows() override;

public:
    virtual bool isGpuContext() override { return false; }
    virtual sk_sp<SkSurface> getBackbufferSurface() override;
    virtual bool isValid() override { return SkToBool(m_hWnd); }
    virtual void resize(int w, int h) override;
    virtual void setDisplayParams(const skwindow::DisplayParams& params) override;

public:
    /** 获取位图GDI句柄
    */
    HBITMAP GetHBitmap() const;

protected:
    virtual void onSwapBuffers() override;

    /** 绘制并刷新到屏幕（Render的实现已经与窗口关联）, 同步完成
    * @param [in] pRender 渲染引擎的接口
    * @param [in] pRenderPaint 界面绘制所需的回调接口
    */
    virtual bool PaintAndSwapBuffers(IRender* pRender, IRenderPaint* pRenderPaint);

    /** 绘制结束后，绘制数据从渲染引擎更新到窗口
    * @param [in] hPaintDC 当前绘制的DC
    * @param [in] rcPaint 绘制的区域
    * @param [in] pRender 绘制引擎接口，用于将绘制结果应用到窗口
    * @param [in] nWindowAlpha 窗口透明度
    * @return 成功返回true，失败则返回false
    */
    bool SwapPaintBuffers(HDC hPaintDC, const UiRect& rcPaint, IRender* pRender, uint8_t nWindowAlpha) const;

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
