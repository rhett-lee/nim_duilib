#include "RenderFactory_Skia.h"
#include "duilib/RenderSkia/Font_Skia.h"
#include "duilib/RenderSkia/FontMgr_Skia.h"
#include "duilib/RenderSkia/Bitmap_Skia.h"
#include "duilib/RenderSkia/Brush_Skia.h"
#include "duilib/RenderSkia/Pen_Skia.h"
#include "duilib/RenderSkia/Path_Skia.h"
#include "duilib/RenderSkia/Matrix_Skia.h"
#include "duilib/RenderSkia/Render_Skia_Offscreen.h"
#include "duilib/RenderSkia/Render_Skia_Bitmap.h"
#include "duilib/RenderSkia/SkOffscreenGLContext.h"

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

IBitmap* RenderFactory_Skia::CreateBitmap(uint32_t nWidth, uint32_t nHeight, AlphaType alphaType)
{
    Bitmap_Skia* pBitmap = new Bitmap_Skia;
    if (!pBitmap->Init(nWidth, nHeight, nullptr, 1.0f, alphaType)) {
        delete pBitmap;
        pBitmap = nullptr;
    }
    return pBitmap;
}

std::unique_ptr<IRender> RenderFactory_Skia::CreateBitmapRender(IBitmap* pBitmap, const IRenderDpiPtr& spRenderDpi)
{
    if ((pBitmap == nullptr) || !pBitmap->IsValid()) {
        return nullptr;
    }
    auto pRender = std::make_unique<Render_Skia_Bitmap>(pBitmap);
    if (!pRender->IsValid()) {
        return nullptr;
    }
    pRender->SetRenderDpi(spRenderDpi);
    //注入渲染工厂：富文本等接口内部需要用它创建字体
    pRender->SetRenderFactory(this);
    return pRender;
}

bool RenderFactory_Skia::IsOffscreenBackendSupported(RenderBackendType backendType) const
{
    if (backendType == RenderBackendType::kRaster_BackendType) {
        //Raster离屏渲染在所有平台均支持
        return true;
    }
    if (backendType == RenderBackendType::kNativeGL_BackendType) {
        //GPU离屏渲染依赖线程内的GL上下文：可复用窗口上下文，或创建后台隐藏窗口上下文
        return OffscreenGLContext::IsSupported();
    }
    //其他后端暂不支持
    return false;
}

std::unique_ptr<IRender> RenderFactory_Skia::CreateOffscreenRender(uint32_t nWidth, uint32_t nHeight,
                                                                  const IRenderDpiPtr& spRenderDpi,
                                                                  RenderBackendType backendType,
                                                                  AlphaType alphaType)
{
    if (!IsOffscreenBackendSupported(backendType)) {
        //请求的后端不支持时，不静默回退
        return nullptr;
    }
    if ((nWidth > (uint32_t)Render_Skia_Offscreen::kMaxOffscreenSize) ||
        (nHeight > (uint32_t)Render_Skia_Offscreen::kMaxOffscreenSize)) {
        return nullptr;
    }
    if (((nWidth == 0) && (nHeight != 0)) || ((nWidth != 0) && (nHeight == 0))) {
        //宽度和高度必须同时为0（表示创建后再Resize）或同时大于0
        return nullptr;
    }
    if (((uint64_t)nWidth * (uint64_t)nHeight * 4) > ((uint64_t)0xFFFFFFFF)) {
        //像素数据长度超出uint32_t的范围，拒绝创建
        return nullptr;
    }
    auto pRender = std::make_unique<Render_Skia_Offscreen>(nWidth, nHeight, backendType, alphaType);
    if (((nWidth > 0) && (nHeight > 0)) && !pRender->IsValid()) {
        //画布创建失败（如内存不足）
        return nullptr;
    }
    pRender->SetRenderDpi(spRenderDpi);
    //注入渲染工厂：富文本等接口内部需要用它创建字体
    pRender->SetRenderFactory(this);
    return pRender;
}

IRenderWindow* RenderFactory_Skia::CreateWindowRender(const IRenderDpiPtr& spRenderDpi, void* platformData, RenderBackendType backendType,
                                                     int32_t nMsaaSampleCount, bool bDisableVsync)
{
#if defined (DUILIB_BUILD_FOR_SDL)
    SDL_Window* sdlWindow = (SDL_Window*)platformData;
    //SDL平台暂不支持MSAA和垂直同步的配置
    UNUSED_VARIABLE(nMsaaSampleCount);
    UNUSED_VARIABLE(bDisableVsync);
    IRenderWindow* pRender = new Render_Skia_SDL(sdlWindow, backendType);
#elif defined(DUILIB_BUILD_FOR_WIN)
    HWND hWnd = (HWND)platformData;
    IRenderWindow* pRender = new Render_Skia_Windows(hWnd, backendType, nMsaaSampleCount, bDisableVsync);
#else
    UNUSED_VARIABLE(platformData);
    UNUSED_VARIABLE(backendType);
    UNUSED_VARIABLE(nMsaaSampleCount);
    UNUSED_VARIABLE(bDisableVsync);
    IRenderWindow* pRender = nullptr;
#endif
    ASSERT(pRender != nullptr);
    if (pRender != nullptr) {
        pRender->SetRenderDpi(spRenderDpi);
        //注入渲染工厂：富文本等接口内部需要用它创建字体
        pRender->SetRenderFactory(this);
    }
    return pRender;
}

IFontMgr* RenderFactory_Skia::GetFontMgr() const
{
    ASSERT(m_impl->m_pFontMgr != nullptr);
    return m_impl->m_pFontMgr.get();
}

} // namespace ui
