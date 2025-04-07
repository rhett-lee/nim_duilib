#ifndef UI_RENDER_SKIA_RENDER_FACTORY_H_
#define UI_RENDER_SKIA_RENDER_FACTORY_H_

#include "duilib/Render/IRender.h"

class SkFontMgr;

namespace ui 
{

class UILIB_API RenderFactory_Skia : public IRenderFactory
{
public:
    RenderFactory_Skia();
    RenderFactory_Skia(const RenderFactory_Skia& r) = delete;
    RenderFactory_Skia& operator = (const RenderFactory_Skia& r) = delete;
    virtual ~RenderFactory_Skia() override;

    virtual IFont* CreateIFont() override;
    virtual IPen* CreatePen(UiColor color, float fWidth = 1.0f) override;
    virtual IBrush* CreateBrush(UiColor corlor) override;
    virtual IPath* CreatePath() override;
    virtual IMatrix* CreateMatrix() override;
    virtual IBitmap* CreateBitmap() override;

    /** 创建一个Render对象
    * @param [in] spRenderDpi 关联的DPI转换接口
    * @param [in] platformData 平台相关的数据，Windows平台该值是窗口句柄
    * @parma [in] backendType 后台绘制方式
    */
    virtual IRender* CreateRender(const IRenderDpiPtr& spRenderDpi,
                                  void* platformData = nullptr,
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
