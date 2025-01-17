#include "Render_Skia_SDL.h"
#include "SkRasterWindowContext_SDL.h"

#ifdef DUILIB_BUILD_FOR_SDL

#pragma warning (push)
#pragma warning (disable: 4267 4244 4201 4100)

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"

#pragma warning (pop)

namespace ui {

/** 创建Raster实现的WindowContext
* @param [in] hWnd 关联的窗口句柄，可以为nullptr
* @param [in] params 显示相关的参数
*/
std::unique_ptr<skwindow::WindowContext> MakeRasterForSDL(SDL_Window* sdlWindow, std::unique_ptr<const skwindow::DisplayParams> params)
{
    std::unique_ptr<skwindow::WindowContext> ctx(new SkRasterWindowContext_SDL(sdlWindow, std::move(params)));
    return ctx;
}

/** 创建GPU实现的WindowContext
* @param [in] hWnd 关联的窗口句柄，可以为nullptr
* @param [in] params 显示相关的参数
*/
//std::unique_ptr<skwindow::WindowContext> MakeGLForWin(HWND hWnd, const skwindow::DisplayParams& params)
//{
//    std::unique_ptr<skwindow::WindowContext> ctx(new SkGLWindowContext_Windows(hWnd, params));
//    if (!ctx->isValid()) {
//        return nullptr;
//    }
//    return ctx;
//}

Render_Skia_SDL::Render_Skia_SDL(SDL_Window* sdlWindow, RenderBackendType backendType):
    m_sdlWindow(sdlWindow),
    m_backendType(backendType)
{
    if (backendType == RenderBackendType::kNativeGL_BackendType) {
        //GPU的绘制，必须绑定窗口
        ASSERT(m_sdlWindow != nullptr);
        if (m_sdlWindow == nullptr) {
            backendType = RenderBackendType::kRaster_BackendType;
        }
    }
    //创建WindowContext
    //if (backendType == RenderBackendType::kNativeGL_BackendType) {
    //    //GPU绘制
    //    m_pWindowContext = MakeGLForWin(hWnd, skwindow::DisplayParams());
    //    ASSERT(m_pWindowContext != nullptr);
    //    if (m_pWindowContext != nullptr) {
    //        m_backendType = RenderBackendType::kNativeGL_BackendType;
    //    }        
    //}
    //如果GL不成功，则创建CPU绘制的上下文
    if (m_pWindowContext == nullptr) {
        //CPU绘制
        m_pWindowContext = MakeRasterForSDL(m_sdlWindow, std::make_unique<skwindow::DisplayParams>());
        ASSERT(m_pWindowContext != nullptr);
        if (m_pWindowContext != nullptr) {
            m_backendType = RenderBackendType::kRaster_BackendType;
        }        
    }
}

Render_Skia_SDL::~Render_Skia_SDL()
{
}

RenderBackendType Render_Skia_SDL::GetRenderBackendType() const
{
    return m_backendType;
}

bool Render_Skia_SDL::Resize(int32_t width, int32_t height)
{
    ASSERT((width > 0) && (height > 0));
    if ((width <= 0) || (height <= 0)) {
        return false;
    }
    if ((GetWidth() == width) && (GetHeight() == height)) {
        return true;
    }

    ASSERT(m_pWindowContext != nullptr);
    if (m_pWindowContext == nullptr) {
        return false;
    }
    m_pWindowContext->resize(width, height);
    ASSERT(GetSkCanvas() != nullptr);
    return true;
}

int32_t Render_Skia_SDL::GetWidth() const
{
    if (m_pWindowContext != nullptr) {
        return m_pWindowContext->width();
    }
    return 0;
}

int32_t Render_Skia_SDL::GetHeight() const
{
    if (m_pWindowContext != nullptr) {
        return m_pWindowContext->height();
    }
    return 0;
}

std::unique_ptr<ui::IRender> Render_Skia_SDL::Clone()
{
    std::unique_ptr<ui::IRender> pClone = std::make_unique<ui::Render_Skia_SDL>(m_sdlWindow, m_backendType);
    pClone->Resize(GetWidth(), GetHeight());
    pClone->SetRenderDpi(GetRenderDpi());
    pClone->BitBlt(0, 0, GetWidth(), GetHeight(), this, 0, 0, RopMode::kSrcCopy);
    return pClone;
}

bool Render_Skia_SDL::PaintAndSwapBuffers(IRenderPaint* pRenderPaint)
{
    ASSERT(pRenderPaint != nullptr);
    ASSERT(m_pWindowContext != nullptr);
    if ((m_pWindowContext != nullptr) && (pRenderPaint != nullptr)) {
        if (m_backendType == RenderBackendType::kNativeGL_BackendType) {
            /*SkGLWindowContext_Windows* pWindowContext = dynamic_cast<SkGLWindowContext_Windows*>(m_pWindowContext.get());
            ASSERT(pWindowContext != nullptr);
            if (pWindowContext != nullptr) {
                return pWindowContext->PaintAndSwapBuffers(this, pRenderPaint);
            }*/
        }
        else if (m_backendType == RenderBackendType::kRaster_BackendType) {
            SkRasterWindowContext_SDL* pWindowContext = dynamic_cast<SkRasterWindowContext_SDL*>(m_pWindowContext.get());
            ASSERT(pWindowContext != nullptr);
            if (pWindowContext != nullptr) {
                return pWindowContext->PaintAndSwapBuffers(this, pRenderPaint);
            }
        }
        else {
            ASSERT(false);
        }
    }
    return false;
}

SkSurface* Render_Skia_SDL::GetSkSurface() const
{
    ASSERT(m_pWindowContext != nullptr);
    if (m_pWindowContext == nullptr) {
        return nullptr;
    }
    //由于m_pWindowContext内部保存了成员变量，返回SkSurface的裸指针是安全的
    sk_sp<SkSurface> backbuffer = m_pWindowContext->getBackbufferSurface();
    ASSERT(backbuffer != nullptr);
    return backbuffer.get();
}

SkCanvas* Render_Skia_SDL::GetSkCanvas() const
{
    ASSERT(m_pWindowContext != nullptr);
    if (m_pWindowContext == nullptr) {
        return nullptr;
    }
    sk_sp<SkSurface> backbuffer = m_pWindowContext->getBackbufferSurface();
    ASSERT(backbuffer != nullptr);
    if (backbuffer == nullptr) {
        return nullptr;
    }
    ASSERT(backbuffer->getCanvas() != nullptr);
    return backbuffer->getCanvas();
}

#ifdef DUILIB_BUILD_FOR_WIN

HDC Render_Skia_SDL::GetRenderDC(HWND /*hWnd*/)
{
    ASSERT(0);
    return nullptr;
}

void Render_Skia_SDL::ReleaseRenderDC(HDC /*hdc*/)
{
    ASSERT(0);
}

#endif

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
