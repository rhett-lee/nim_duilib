#include "SkRasterWindowContext_SDL.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/PerformanceUtil.h"

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

    //获取需要绘制的区域
    UiRect rcPaint;
    bool bUpdateRect = GetUpdateRect(rcPaint); //返回true表示支持局部绘制，只绘制更新的部分区域，以提高效率
    if (rcPaint.IsEmpty()) {
        bUpdateRect = false;
    }
    if (!bUpdateRect) {
        //不支持局部绘制，每次都是需要重绘整个窗口的客户区域
        GetClientRect(rcPaint);
    }
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

    //绘制完成后，将已经绘制的区域标记为有效区域
    if (bUpdateRect) {
        ValidateRect(rcPaint);
    }
    return bRet;
}

bool SkRasterWindowContext_SDL::SwapPaintBuffers(const UiRect& rcPaint, uint8_t nLayeredWindowAlpha) const
{
    PerformanceStat statPerformance(_T("PaintWindow, SkRasterWindowContext_SDL::SwapPaintBuffers"));
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
#ifdef DUILIB_BUILD_FOR_WIN
    SDL_PixelFormat format = SDL_PIXELFORMAT_BGRA32;
#else
    SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA32;
#endif
    SDL_Surface* sdlSurface = SDL_CreateSurface(width(), height(), format);
    ASSERT(sdlSurface != nullptr);
    if (sdlSurface == nullptr) {
        return false;
    }

    //更新数据到Surface
    ::memcpy(sdlSurface->pixels, m_fSurfaceMemory.get(), sdlSurface->h * sdlSurface->pitch);

    //更新数据的流程：Surface -> Texture -> Renderer
    SDL_Texture* sdlTextrue = SDL_CreateTextureFromSurface(sdlRenderer, sdlSurface);
    ASSERT(sdlTextrue != nullptr);
    if (sdlTextrue != nullptr) {
        SDL_FRect dstrect;
        dstrect.x = (float)rcPaint.left;
        dstrect.y = (float)rcPaint.top;
        dstrect.w = (float)rcPaint.Width();
        dstrect.h = (float)rcPaint.Height();

        //对源SDL窗口清零，避免透明窗口的情况下，绘制到残留图像上，导致窗口阴影越来越浓
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0);
        if ((rcPaint.Width() == width()) && (rcPaint.Height() == height())) {
            SDL_RenderClear(sdlRenderer);
        }
        else {
            SDL_RenderFillRect(sdlRenderer, &dstrect);
        }

        //设置纹理的透明度
        if (nLayeredWindowAlpha != 255) {
            SDL_SetTextureAlphaMod(sdlTextrue, nLayeredWindowAlpha);
        }
        
        //将绘制的数据更新到SDL窗口        
        SDL_RenderTexture(sdlRenderer, sdlTextrue, &dstrect, &dstrect);
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
    bool nRet = SDL_GetWindowSize(m_sdlWindow, &nWidth, &nHeight);
    ASSERT(nRet);
    if (nRet) {
        rcClient.left = 0;
        rcClient.top = 0;
        rcClient.right = rcClient.left + nWidth;
        rcClient.bottom = rcClient.top + nHeight;
    }
}

bool SkRasterWindowContext_SDL::GetUpdateRect(UiRect& /*rcUpdate*/) const
{
    //局部绘制功能暂不开启；
    //此功能开启时，需要修改SDL源码中对于WM_PAINT消息的处理，需要将"ValidateRect(hwnd, NULL);"这行代码注释掉，否则可能导致部分绘制消息丢失，出现不绘制的情况
    return false;

//    if (m_sdlWindow == nullptr) {
//        return false;
//    }
//    rcUpdate.Clear();
//#ifdef DUILIB_BUILD_FOR_WIN
//    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
//    HWND hWnd = (HWND)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
//    RECT rectUpdate = { 0, };
//    if ((hWnd != nullptr) && ::IsWindow(hWnd) && ::GetUpdateRect(hWnd, &rectUpdate, FALSE)) {
//        rcUpdate.left = rectUpdate.left;
//        rcUpdate.top = rectUpdate.top;
//        rcUpdate.right = rectUpdate.right;
//        rcUpdate.bottom = rectUpdate.bottom;
//        return true;
//    }
//#endif
//    return false;
}

void SkRasterWindowContext_SDL::ValidateRect(UiRect& rcPaint) const
{
    if ((m_sdlWindow == nullptr) || rcPaint.IsEmpty()) {
        return;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
    HWND hWnd = (HWND)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if ((hWnd != nullptr) && ::IsWindow(hWnd)) {
        RECT rectPaint = { 0, };
        rectPaint.left = rcPaint.left;
        rectPaint.top = rcPaint.top;
        rectPaint.right = rcPaint.right;
        rectPaint.bottom = rcPaint.bottom;
        ::ValidateRect(hWnd, &rectPaint);
    }
#endif
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
