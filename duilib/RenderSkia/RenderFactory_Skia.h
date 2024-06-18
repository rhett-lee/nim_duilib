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
    virtual IRender* CreateRender(Window* pWindow) override;

public:
    /** 获取Skia字体管理器
    */
    SkFontMgr* GetSkFontMgr();

private:
    /** Skia字体管理器
    */
    SkFontMgr* m_pSkFontMgr;
};

} // namespace ui

#endif // UI_RENDER_SKIA_RENDER_FACTORY_H_
