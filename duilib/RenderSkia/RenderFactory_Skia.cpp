#include "RenderFactory_Skia.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/FontMgr_Skia.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Brush_Skia.h"
#include "duilib/RenderSkia/Pen_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"

#if defined (DUILIB_BUILD_FOR_SDL)
    #include "duilib/RenderSkia/Render_Skia_SDL.h"
#elif defined (DUILIB_BUILD_FOR_WIN)
    #include "duilib/RenderSkia/Render_Skia_Windows.h"
#endif

namespace ui {

class RenderFactory_Skia::TImpl
{
public:
    /** Skia的字体管理器
    */
    std::shared_ptr<IFontMgr> m_pFontMgr;
};

RenderFactory_Skia::RenderFactory_Skia()
{
    m_impl = new TImpl;

    //创建Skia的字体管理器对象，进程内唯一
    m_impl->m_pFontMgr = std::make_shared<FontMgr_Skia>();
    ASSERT(m_impl->m_pFontMgr != nullptr);
}

RenderFactory_Skia::~RenderFactory_Skia()
{
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

IFont* RenderFactory_Skia::CreateIFont()
{
    return new Font_Skia(m_impl->m_pFontMgr);
}

IPen* RenderFactory_Skia::CreatePen(UiColor color, float fWidth)
{
    return new Pen_Skia(color, fWidth);
}

IBrush* RenderFactory_Skia::CreateBrush(UiColor color)
{
    return new Brush_Skia(color);
}

IPath* RenderFactory_Skia::CreatePath()
{
    return new Path_Skia();
}

IMatrix* RenderFactory_Skia::CreateMatrix()
{
    return new Matrix_Skia();
}

IBitmap* RenderFactory_Skia::CreateBitmap()
{
    return new Bitmap_Skia();
}

IRender* RenderFactory_Skia::CreateRender(const IRenderDpiPtr& spRenderDpi, void* platformData, RenderBackendType backendType)
{
#if defined (DUILIB_BUILD_FOR_SDL)
    SDL_Window* sdlWindow = (SDL_Window*)platformData;
    IRender* pRender = new Render_Skia_SDL(sdlWindow, backendType);
#elif defined(DUILIB_BUILD_FOR_WIN)
    HWND hWnd = (HWND)platformData;
    IRender* pRender = new Render_Skia_Windows(hWnd, backendType);
#else
    UNUSED_VARIABLE(platformData);
    UNUSED_VARIABLE(backendType);
    IRender* pRender = nullptr;
#endif
    ASSERT(pRender != nullptr);
    if (pRender != nullptr) {
        pRender->SetRenderDpi(spRenderDpi);
    }    
    return pRender;
}

IFontMgr* RenderFactory_Skia::GetFontMgr() const
{
    ASSERT(m_impl->m_pFontMgr != nullptr);
    return m_impl->m_pFontMgr.get();
}

} // namespace ui
