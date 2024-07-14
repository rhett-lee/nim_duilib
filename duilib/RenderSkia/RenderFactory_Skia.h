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
    virtual IPen* CreatePen(UiColor color, int width = 1) override;
    virtual IBrush* CreateBrush(UiColor corlor) override;
    virtual IPath* CreatePath() override;
    virtual IMatrix* CreateMatrix() override;
    virtual IBitmap* CreateBitmap() override;
    virtual IRender* CreateRender(const IRenderDpiPtr& spRenderDpi) override;

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
