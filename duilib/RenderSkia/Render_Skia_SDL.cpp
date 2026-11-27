#include "Render_Skia_SDL.h"
#include "SkRasterWindowContext_SDL.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "SkiaHeaderBegin.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkRegion.h"

#include "SkiaHeaderEnd.h"

#ifdef DUILIB_BUILD_FOR_WIN
    #include "WindowRgn_Windows.h"
    #include "SDL3/SDL.h"
#endif

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

//注意：SDL构建模式暂未实现GPU（OpenGL）的WindowContext，因此没有对应的MakeGLForSDL函数，
//调用方请求kNativeGL_BackendType时，会回退为CPU绘制（见Render_Skia_SDL的构造函数）

Render_Skia_SDL::Render_Skia_SDL(SDL_Window* sdlWindow, RenderBackendType backendType):
    m_sdlWindow(sdlWindow),
    m_backendType(backendType)
{
    //SDL构建模式暂不支持kNativeGL_BackendType后端（没有基于SDL窗口的GL WindowContext实现），
    //无论请求哪种后端，最终都回退为CPU绘制；实际后端以GetRenderBackendType()的返回值为准
    if (backendType == RenderBackendType::kNativeGL_BackendType) {
        //GPU的绘制，必须绑定窗口
        ASSERT(m_sdlWindow != nullptr);
        if (m_sdlWindow == nullptr) {
            backendType = RenderBackendType::kRaster_BackendType;
        }
    }
    //SDL后端不支持GL绘制，直接创建CPU绘制的上下文
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

std::unique_ptr<ui::IRender> Render_Skia_SDL::Clone() const
{
    std::unique_ptr<ui::IRender> pClone = std::make_unique<ui::Render_Skia_SDL>(m_sdlWindow, m_backendType);
    pClone->Resize(GetWidth(), GetHeight());
    pClone->SetRenderDpi(GetRenderDpi());
    pClone->SetRenderFactory(GetRenderFactory());
    pClone->SetImageSamplingQuality(GetImageSamplingQuality());
    pClone->BitBlt(0, 0, GetWidth(), GetHeight(), this, 0, 0, RopMode::kSrcCopy);
    return pClone;
}

bool Render_Skia_SDL::PaintAndSwapBuffers(IRenderPaint* pRenderPaint)
{
    ASSERT(pRenderPaint != nullptr);
    ASSERT(m_pWindowContext != nullptr);
    if ((m_pWindowContext != nullptr) && (pRenderPaint != nullptr)) {
        if (m_backendType == RenderBackendType::kNativeGL_BackendType) {
            //SDL构建模式不支持GL后端：构造时已回退为CPU绘制，不会进入该分支
            ASSERT(false);
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

bool Render_Skia_SDL::IsBackBufferValid() const
{
    if (m_pWindowContext == nullptr) {
        return false;
    }
    if (m_backendType == RenderBackendType::kRaster_BackendType) {
        SkRasterWindowContext_SDL* pWindowContext = dynamic_cast<SkRasterWindowContext_SDL*>(m_pWindowContext.get());
        if (pWindowContext != nullptr) {
            return pWindowContext->IsBackBufferValid();
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

bool Render_Skia_SDL::SetWindowRoundRectRgn(const UiRect& rcWnd, float rx, float ry, bool bRedraw)
{
    if (m_sdlWindow != nullptr) {
        SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
        HWND hWnd = (HWND)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        if (::IsWindow(hWnd)) {
            return WindowRgn::SetWindowRoundRectRgn(hWnd, rcWnd, rx, ry, bRedraw);
        }
    }
    return false;
}

bool Render_Skia_SDL::SetWindowRectRgn(const UiRect& rcWnd, bool bRedraw)
{
    if (m_sdlWindow != nullptr) {
        SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
        HWND hWnd = (HWND)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        if (::IsWindow(hWnd)) {
            return WindowRgn::SetWindowRectRgn(hWnd, rcWnd, bRedraw);
        }
    }
    return false;
}

void Render_Skia_SDL::ClearWindowRgn(bool bRedraw)
{
    if (m_sdlWindow != nullptr) {
        SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
        HWND hWnd = (HWND)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
        if (::IsWindow(hWnd)) {
            WindowRgn::ClearWindowRgn(hWnd, bRedraw);
        }
    }
}

#endif // DUILIB_BUILD_FOR_WIN

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
