#ifndef UI_RENDER_SKIA_RENDER_FACTORY_H_
#define UI_RENDER_SKIA_RENDER_FACTORY_H_

#include "duilib/Render/IRender.h"

class SkFontMgr;

namespace ui 
{

class RenderFactory_Skia : public IRenderFactory
{
public:
    RenderFactory_Skia();
    RenderFactory_Skia(const RenderFactory_Skia& r) = delete;
    RenderFactory_Skia& operator = (const RenderFactory_Skia& r) = delete;
    virtual ~RenderFactory_Skia() override;

    virtual IFont* CreateIFont() override;
    virtual IPen* CreatePen(UiColor color, float fWidth = 1.0f) override;
    virtual IBrush* CreateBrush(UiColor color) override;
    virtual IPath* CreatePath() override;
    virtual IMatrix* CreateMatrix() override;
    virtual IBitmap* CreateBitmap() override;
    virtual IBitmap* CreateBitmap(uint32_t nWidth, uint32_t nHeight, AlphaType alphaType) override;

    /** 查询指定后端是否支持离屏渲染
    * @param [in] backendType 待查询的后端类型
    */
    virtual bool IsOffscreenBackendSupported(RenderBackendType backendType) const override;

    /** 创建一个离屏渲染对象（不关联任何窗口）
    * @param [in] nWidth 画布宽度（物理像素），可以为0
    * @param [in] nHeight 画布高度（物理像素），可以为0
    * @param [in] spRenderDpi 关联的DPI转换接口
    * @param [in] backendType 离屏渲染后端
    * @param [in] alphaType 画布的Alpha类型
    */
    virtual std::unique_ptr<IRender> CreateOffscreenRender(uint32_t nWidth, uint32_t nHeight,
                                                           const IRenderDpiPtr& spRenderDpi = nullptr,
                                                           RenderBackendType backendType = RenderBackendType::kRaster_BackendType,
                                                           AlphaType alphaType = AlphaType::kPremul) override;

    /** 创建一个窗口Render对象
    * @param [in] spRenderDpi 关联的DPI转换接口
    * @param [in] platformData 平台相关的数据，Windows平台该值是窗口句柄
    * @param [in] backendType 后台绘制方式
    */
    virtual IRenderWindow* CreateWindowRender(const IRenderDpiPtr& spRenderDpi,
                                              void* platformData,
                                              RenderBackendType backendType = RenderBackendType::kRaster_BackendType) override;

    /** 获取字体管理器接口（每个factory共享一个对象）
    */
    virtual IFontMgr* GetFontMgr() const override;

private:
    /** 内部实现类
    */
    class TImpl;
    TImpl* m_impl;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RENDER_FACTORY_H_
