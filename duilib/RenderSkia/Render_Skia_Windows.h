#ifndef UI_RENDER_SKIA_RENDER_WINDOWS_H_
#define UI_RENDER_SKIA_RENDER_WINDOWS_H_

#include "duilib/RenderSkia/Render_Skia.h"

#ifdef DUILIB_BUILD_FOR_WIN
#include "duilib/duilib_config_windows.h"

namespace skwindow {
    class WindowContext;
}

namespace ui 
{
/** 渲染引擎接口的Windows实现
*/
class UILIB_API Render_Skia_Windows : public Render_Skia
{
public:
    /** 构造函数
    * @param [in] hWnd 关联的窗口句柄，可以为nullptr
    * @param [in] backendType 后台绘制方式
    */
    Render_Skia_Windows(HWND hWnd, RenderBackendType backendType);
    Render_Skia_Windows(const Render_Skia_Windows& r) = delete;
    Render_Skia_Windows& operator = (const Render_Skia_Windows& r) = delete;
    virtual ~Render_Skia_Windows() override;

public:
    /** 获取后台渲染的类型
    */
    virtual RenderBackendType GetRenderBackendType() const override;

    /** 大小发生变化
    */
    virtual bool Resize(int32_t width, int32_t height) override;

    /** 获取宽度
    */
    virtual int32_t GetWidth() const override;

    /** 获取高度
    */
    virtual int32_t GetHeight() const override;

    /** 复制一个新的渲染对象
    */
    virtual std::unique_ptr<IRender> Clone() override;

    /** 绘制并刷新到屏幕（Render的实现已经与窗口关联）, 同步完成
    * @param [in] pRenderPaint 界面绘制所需的回调接口
    */
    virtual bool PaintAndSwapBuffers(IRenderPaint* pRenderPaint) override;

    /** 获取SkSurface接口
    */
    virtual SkSurface* GetSkSurface() const override;

    /** 获取SkCanvas接口
    */
    virtual SkCanvas* GetSkCanvas() const override;

public:
    /** 获取DC句柄，当不使用后，需要调用ReleaseDC接口释放资源
    */
    virtual HDC GetRenderDC(HWND hWnd) override;

    /** 释放DC资源
    * @param [in] hdc 需要释放的DC句柄
    */
    virtual void ReleaseRenderDC(HDC hdc) override;
    
private:
    /** 删除DC
    */
    void DeleteDC();

private:
    /** WindowContext对象
    */
    std::unique_ptr<skwindow::WindowContext> m_pWindowContext;

    /** 后台绘制方式
    */
    RenderBackendType m_backendType;

    /** 关联的窗口句柄
    */
    HWND m_hWnd;

    /** 关联的DC句柄
    */
    HDC m_hDC;

    /** 该DC原来的位图
    */
    HGDIOBJ m_hOldObj;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RENDER_WINDOWS_H_

#endif //DUILIB_BUILD_FOR_WIN
