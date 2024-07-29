#include "SkRasterWindowContext_SDL.h"
#include "duilib/Render/IRender.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

namespace ui {

SkRasterWindowContext_SDL::SkRasterWindowContext_SDL(SDL_Window* sdlWindow, const skwindow::DisplayParams& params):
    skwindow::internal::RasterWindowContext(params),
    m_sdlWindow(sdlWindow)
{
    fWidth = 0;
    fHeight = 0;

    if (m_sdlWindow != nullptr) {
        UiRect rect;
        GetClientRect(rect);
        this->resize(rect.Width(), rect.Height());
    }
}

SkRasterWindowContext_SDL::~SkRasterWindowContext_SDL()
{
}

void SkRasterWindowContext_SDL::setDisplayParams(const skwindow::DisplayParams& params)
{
    fDisplayParams = params;
    UiRect rect;
    GetClientRect(rect);
    this->resize(rect.Width(), rect.Height());
}

void SkRasterWindowContext_SDL::resize(int nWidth, int nHeight)
{
    if (nWidth < 0) {
        nWidth = 0;
    }
    if (nHeight < 0) {
        nHeight = 0;
    }
    if ((fWidth == nWidth) && (fHeight == nHeight)) {
        return;
    }
    fWidth = nWidth;
    fHeight = nHeight;

    m_fSurfaceMemory.reset();
    m_fBackbufferSurface.reset();
    
    if ((nWidth == 0) || (nHeight == 0)) {
        return;
    }

    m_fSurfaceMemory.reset(nWidth * nHeight * sizeof(uint32_t));
    void* pixels = m_fSurfaceMemory.get();
    ASSERT(pixels != nullptr);
    if (pixels == nullptr) {
        return;
    }

    SkImageInfo info = SkImageInfo::Make(nWidth, nHeight, fDisplayParams.fColorType, SkAlphaType::kPremul_SkAlphaType, fDisplayParams.fColorSpace);
    m_fBackbufferSurface = SkSurfaces::WrapPixels(info, pixels, sizeof(uint32_t) * nWidth);
}

sk_sp<SkSurface> SkRasterWindowContext_SDL::getBackbufferSurface()
{
    return m_fBackbufferSurface;
}

void SkRasterWindowContext_SDL::onSwapBuffers()
{
}

bool SkRasterWindowContext_SDL::PaintAndSwapBuffers(IRender* pRender, IRenderPaint* pRenderPaint)
{
    SkASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }
    SkASSERT(m_fBackbufferSurface != nullptr);
    if (m_fBackbufferSurface == nullptr) {
        return false;
    }
    SkASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    SkASSERT(pRenderPaint != nullptr);
    if (pRenderPaint == nullptr) {
        return false;
    }

    //获取需要绘制的区域(不支持局部绘制，每次都是需要重绘整个窗口的客户区域)
    UiRect rcPaint;
    GetClientRect(rcPaint);
    if (rcPaint.IsEmpty()) {
        //无需绘制
        return false;
    }

    //窗口透明度
    uint8_t nLayeredWindowAlpha = pRenderPaint->GetLayeredWindowAlpha();

    //执行绘制
    bool bRet = pRenderPaint->DoPaint(rcPaint);
    if (bRet) {
        //绘制完成后，更新到窗口
        SwapPaintBuffers(rcPaint, nLayeredWindowAlpha);
    }
    return bRet;
}

bool SkRasterWindowContext_SDL::SwapPaintBuffers(const UiRect& rcPaint, uint8_t nLayeredWindowAlpha) const
{
    ASSERT(!rcPaint.IsEmpty());
    if (rcPaint.IsEmpty()) {
        return false;
    }
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }

    SDL_Renderer* sdlRenderer = SDL_GetRenderer(m_sdlWindow);
    ASSERT(sdlRenderer != nullptr);
    if (sdlRenderer == nullptr) {
        return false;
    }

    // 渲染到窗口（IRender -> 绘制到 SDL Render -> 更新到 SDL 窗口）
    SDL_Surface* sdlSurface = SDL_CreateSurface(rcPaint.Width(), rcPaint.Height(), SDL_PIXELFORMAT_BGRA32);
    ASSERT(sdlSurface != nullptr);
    if (sdlSurface == nullptr) {
        return false;
    }

    //更新数据到Surface
    memcpy(sdlSurface->pixels, m_fSurfaceMemory.get(), sdlSurface->h * sdlSurface->pitch);

    //更新数据的流程：Surface -> Texture -> Renderer
    SDL_Texture* sdlTextrue = SDL_CreateTextureFromSurface(sdlRenderer, sdlSurface);
    ASSERT(sdlTextrue != nullptr);
    if (sdlTextrue != nullptr) {
        //对源SDL窗口清零，避免透明窗口的情况下，绘制到残留图像上，导致窗口阴影越来越浓
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0);
        SDL_RenderClear(sdlRenderer);

        //设置纹理的透明度
        if (nLayeredWindowAlpha != 255) {
            SDL_SetTextureAlphaMod(sdlTextrue, nLayeredWindowAlpha);
        }
        
        //将绘制的数据更新到SDL窗口
        SDL_RenderTexture(sdlRenderer, sdlTextrue, nullptr, nullptr);        
        SDL_RenderPresent(sdlRenderer);
    }

    //销毁资源
    SDL_DestroyTexture(sdlTextrue);
    SDL_DestroySurface(sdlSurface);
    sdlTextrue = nullptr;
    sdlSurface = nullptr;
    return true;
}

void SkRasterWindowContext_SDL::GetClientRect(UiRect& rcClient) const
{
    rcClient.Clear();
    ASSERT(m_sdlWindow != nullptr);
    int nWidth = 0;
    int nHeight = 0;
    int nRet = SDL_GetWindowSize(m_sdlWindow, &nWidth, &nHeight);
    ASSERT(nRet == 0);
    if (nRet == 0) {
        rcClient.left = 0;
        rcClient.top = 0;
        rcClient.right = rcClient.left + nWidth;
        rcClient.bottom = rcClient.top + nHeight;
    }
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
