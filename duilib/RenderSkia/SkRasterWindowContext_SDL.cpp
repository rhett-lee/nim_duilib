#include "SkRasterWindowContext_SDL.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/PerformanceUtil.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

namespace ui {

SkRasterWindowContext_SDL::SkRasterWindowContext_SDL(SDL_Window* sdlWindow, std::unique_ptr<const skwindow::DisplayParams> params):
    skwindow::internal::RasterWindowContext(std::move(params)),
    m_sdlWindow(sdlWindow),
    m_sdlTextrue(nullptr)
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
    if (m_sdlTextrue != nullptr) {
        SDL_DestroyTexture(m_sdlTextrue);
        m_sdlTextrue = nullptr;
    }
}

void SkRasterWindowContext_SDL::setDisplayParams(std::unique_ptr<const skwindow::DisplayParams> params)
{
    fDisplayParams = std::move(params);
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
        if ((fWidth > 0) && (fHeight > 0)) {
            SkASSERT(m_fSurfaceMemory != nullptr);
            SkASSERT(m_fBackbufferSurface != nullptr);
        }
        return;
    }
    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    SkASSERT(pDisplayParams != nullptr);
    if (pDisplayParams == nullptr) {
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
    SkASSERT(pixels != nullptr);
    if (pixels == nullptr) {
        m_fSurfaceMemory.reset();
        fWidth = 0;
        fHeight = 0;
        return;
    }

    SkImageInfo info = SkImageInfo::Make(nWidth, nHeight, pDisplayParams->colorType(), SkAlphaType::kPremul_SkAlphaType, pDisplayParams->colorSpace());
    m_fBackbufferSurface = SkSurfaces::WrapPixels(info, pixels, sizeof(uint32_t) * nWidth);

    if (m_sdlTextrue != nullptr) {
        SDL_DestroyTexture(m_sdlTextrue);
        m_sdlTextrue = nullptr;
    }

    if (m_fBackbufferSurface == nullptr) {
        m_fSurfaceMemory.reset();
        fWidth = 0;
        fHeight = 0;
        return;
    }
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
    bool bUpdateRect = pRenderPaint->GetUpdateRect(rcPaint); //返回true表示支持局部绘制，只绘制更新的部分区域，以提高效率
    if (bUpdateRect && !rcPaint.IsEmpty()) {
        //确保区域的有效性
        UiRect rcClient;
        GetClientRect(rcClient);
        rcPaint.Intersect(rcClient);
    }
    if (rcPaint.IsEmpty()) {
        //不支持局部绘制，每次都是需要重绘整个窗口的客户区域
        GetClientRect(rcPaint);
    }
    if (rcPaint.IsEmpty()) {
        //无需绘制
        return false;
    }

    //窗口透明度
    uint8_t nLayeredWindowAlpha = pRenderPaint->GetLayeredWindowAlpha();

    //是否为完全绘制
    const bool bFullPaint = (rcPaint.Width() == width()) && (rcPaint.Height() == height());
    SkCanvas* skCanvas = nullptr;
    if (!bFullPaint) {
        //使用裁剪区域，避免绘制其他无关区域的数据
        skCanvas = m_fBackbufferSurface->getCanvas();
        if (skCanvas != nullptr) {
            skCanvas->save();
            skCanvas->clipIRect(SkIRect::MakeLTRB(rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom));
        }
    }

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

    if (skCanvas != nullptr) {
        skCanvas->restore();
    }
    return bRet;
}

bool SkRasterWindowContext_SDL::SwapPaintBuffers(const UiRect& rcPaint, uint8_t nLayeredWindowAlpha)
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
    ASSERT(m_fSurfaceMemory.get() != nullptr);
    if (m_fSurfaceMemory.get() == nullptr) {
        return false;
    }
    ASSERT(m_fBackbufferSurface.get() != nullptr);
    if (m_fBackbufferSurface.get() == nullptr) {
        return false;
    }

    if (SwapPaintBuffersFast(rcPaint, nLayeredWindowAlpha)) {
        //直接通过窗口的Surface更新绘制数据到窗口设备(不使用GPU，速度更快)
        return true;
    }

    SDL_Renderer* sdlRenderer = SDL_GetRenderer(m_sdlWindow);
    ASSERT(sdlRenderer != nullptr);
    if (sdlRenderer == nullptr) {
        return false;
    }

    if (m_sdlTextrue == nullptr) {
        // 渲染到窗口（IRender -> 绘制到 SDL Render -> 更新到 SDL 窗口）
#ifdef DUILIB_BUILD_FOR_WIN
        SDL_PixelFormat format = SDL_PIXELFORMAT_BGRA32;
#else
        SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA32;
#endif
        m_sdlTextrue = SDL_CreateTexture(sdlRenderer, format, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, width(), height());
        ASSERT(m_sdlTextrue != nullptr);
    }
    
    if (m_sdlTextrue == nullptr) {
        return false;
    }

    //将界面数据复制到纹理
    bool bDrawOk = false;
    if ((rcPaint.Width() != width()) || (rcPaint.Height() != height())) {
        //局部绘制：只绘制更新的部分
        SDL_Rect rect;
        rect.x = rcPaint.left;
        rect.y = rcPaint.top;
        rect.w = rcPaint.Width();
        rect.h = rcPaint.Height();        
        SkIRect bounds = SkIRect::MakeLTRB(rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
        sk_sp<SkImage> snapshotImage = m_fBackbufferSurface->makeImageSnapshot(bounds);
        if (snapshotImage != nullptr) {
            SkPixmap pixmap;
            if (snapshotImage->peekPixels(&pixmap) && (pixmap.addr() != nullptr) && (pixmap.width() == rcPaint.Width()) && (pixmap.height() == rcPaint.Height())) {
                SDL_UpdateTexture(m_sdlTextrue, &rect, pixmap.addr(), (int)pixmap.rowBytes());
                bDrawOk = true;
            }
        }
        ASSERT(bDrawOk);
    }
    if (!bDrawOk) {
        //完整绘制
        SDL_UpdateTexture(m_sdlTextrue, nullptr, m_fSurfaceMemory.get(), m_fBackbufferSurface->width() * sizeof(uint32_t));
    }

    //设置纹理的透明度
    if (nLayeredWindowAlpha != 255) {
        SDL_SetTextureAlphaMod(m_sdlTextrue, nLayeredWindowAlpha);
    }
    //对源SDL窗口清零，避免透明窗口的情况下，绘制到残留图像上，导致窗口阴影越来越浓
    SDL_RenderClear(sdlRenderer);

    //绘制纹理
    SDL_RenderTexture(sdlRenderer, m_sdlTextrue, nullptr, nullptr);

    //提交绘制数据（这一步速度最慢：Windows平台很快，时间可以忽略； 但Linux平台X11环境（虚拟机中）下每次调用需要9毫秒左右）
    SDL_RenderPresent(sdlRenderer);
    return true;
}

bool SkRasterWindowContext_SDL::SwapPaintBuffersFast(const UiRect& rcPaint, uint8_t nLayeredWindowAlpha)
{
    ASSERT(!rcPaint.IsEmpty());
    if (rcPaint.IsEmpty()) {
        return false;
    }
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }
    ASSERT(m_fSurfaceMemory.get() != nullptr);
    if (m_fSurfaceMemory.get() == nullptr) {
        return false;
    }
    ASSERT(m_fBackbufferSurface.get() != nullptr);
    if (m_fBackbufferSurface.get() == nullptr) {
        return false;
    }

    SDL_Surface* sdlSurface = SDL_GetWindowSurface(m_sdlWindow);
    if (sdlSurface == nullptr) {
        return false;
    }

    if ((sdlSurface->pixels == nullptr) || (sdlSurface->w != width()) || (sdlSurface->h != height()) || (sdlSurface->pitch != width() * (int32_t)sizeof(uint32_t))) {
        //大小不匹配
        return false;
    }
    
    SkColorType backSurfaceColorType = m_fBackbufferSurface->imageInfo().colorInfo().colorType();
    int32_t backR = -1;
    int32_t backG = -1;
    int32_t backB = -1;
    int32_t backA = -1;
    if (!GetSkiaColorByteOrder(backSurfaceColorType, backR, backG, backB, backA)) {
        return false;
    }

    SDL_PixelFormat sdlFormat = sdlSurface->format;
    int32_t sdlR = -1;
    int32_t sdlG = -1;
    int32_t sdlB = -1;
    int32_t sdlA = -1;
    if (!GetSDLColorByteOrder(sdlFormat, sdlR, sdlG, sdlB, sdlA)) {
        return false;
    }

    //统计性能
    PerformanceStat statPerformance(_T("PaintWindow, SkRasterWindowContext_SDL::SwapPaintBuffersFast"));

    bool bDrawOk = false;
    if ((rcPaint.Width() != width()) || (rcPaint.Height() != height())) {
        //局部绘制：只绘制更新的部分
        SDL_Rect rect;
        rect.x = rcPaint.left;
        rect.y = rcPaint.top;
        rect.w = rcPaint.Width();
        rect.h = rcPaint.Height();
        //按行复制数据(每次复制1行数据)
        const int32_t nMaxRow = rcPaint.top + rcPaint.Height();
        const int32_t nWidth = rcPaint.Width();
        for (int32_t nRow = rcPaint.top; nRow < nMaxRow; ++nRow) {
            ::memcpy((uint32_t*)sdlSurface->pixels + nRow * sdlSurface->w + rcPaint.left,
                     (uint32_t*)m_fSurfaceMemory.get() + nRow * sdlSurface->w + rcPaint.left,
                     nWidth * sizeof(uint32_t));
        }

        //处理颜色顺序
        UpdateColorByteOrder(sdlSurface->pixels, sdlSurface->w, rcPaint, backR, backG, backB, backA, sdlR, sdlG, sdlB, sdlA);
        UpdateColorAlpha(sdlSurface->pixels, sdlSurface->w, rcPaint, nLayeredWindowAlpha, sdlR, sdlG, sdlB, sdlA);
        SDL_UpdateWindowSurfaceRects(m_sdlWindow, &rect, 1);
        bDrawOk = true;
        ASSERT(bDrawOk);
    }
    if (!bDrawOk) {
        //完整绘制
        ::memcpy(sdlSurface->pixels, m_fSurfaceMemory.get(), sdlSurface->h * sdlSurface->pitch);
        UpdateColorByteOrder(sdlSurface->pixels, sdlSurface->w, rcPaint, backR, backG, backB, backA, sdlR, sdlG, sdlB, sdlA);
        UpdateColorAlpha(sdlSurface->pixels, sdlSurface->w, rcPaint, nLayeredWindowAlpha, sdlR, sdlG, sdlB, sdlA);
        SDL_UpdateWindowSurface(m_sdlWindow);
    }
    return true;
}

bool SkRasterWindowContext_SDL::GetSkiaColorByteOrder(SkColorType backSurfaceColorType, int32_t& backR, int32_t& backG, int32_t& backB, int32_t& backA) const
{
    if (backSurfaceColorType == kBGRA_8888_SkColorType) {
        backB = 0;
        backG = 1;
        backR = 2;
        backA = 3;
        return true;
    }
    else if (backSurfaceColorType == kRGBA_8888_SkColorType) {
        backR = 0;
        backG = 1;
        backB = 2;
        backA = 3;
        return true;
    }
    return false;
}

bool SkRasterWindowContext_SDL::GetSDLColorByteOrder(int32_t sdlFormat, int32_t& sdlR, int32_t& sdlG, int32_t& sdlB, int32_t& sdlA) const
{
    int bpp = 0;
    Uint32 sdlRmask = 0;
    Uint32 sdlGmask = 0;
    Uint32 sdlBmask = 0;
    Uint32 sdlAmask = 0;
    SDL_GetMasksForPixelFormat((SDL_PixelFormat)sdlFormat, &bpp, &sdlRmask, &sdlGmask, &sdlBmask, &sdlAmask);
    if (bpp != 32) {
        return false;
    }
    sdlR = GetColorByteOrder(sdlRmask);
    sdlG = GetColorByteOrder(sdlGmask);
    sdlB = GetColorByteOrder(sdlBmask);
    if (sdlAmask != 0) {
        sdlA = GetColorByteOrder(sdlAmask);
    }
    else {
        sdlA = 3;
        if ((sdlR == 3) || (sdlG == 3) || (sdlB == 3)) {
            return false;
        }
    }
    return (sdlR >= 0) && (sdlG >= 0) && (sdlB >= 0) && (sdlA >= 0);
}

int32_t SkRasterWindowContext_SDL::GetColorByteOrder(uint32_t mask) const
{
    //Endian Order: LE
    int32_t colorOrder = -1;
    if (mask == 0x000000FF) {
        colorOrder = 0;
    }
    else if (mask == 0x0000FF00) {
        colorOrder = 1;
    }
    else if (mask == 0x00FF0000) {
        colorOrder = 2;
    }
    else if (mask == 0xFF000000) {
        colorOrder = 3;
    }
    return colorOrder;
}

void SkRasterWindowContext_SDL::UpdateColorByteOrder(void* surfacePixels, int32_t nSurfaceWidth, const UiRect& rcPaint,
                                                     int32_t backR, int32_t backG, int32_t backB, int32_t backA,
                                                     int32_t sdlR, int32_t sdlG, int32_t sdlB, int32_t sdlA) const
{
    if ((surfacePixels == nullptr) || (nSurfaceWidth < 1) || rcPaint.IsEmpty()) {
        return;
    }
    bool bDiffR = sdlR != backR;
    bool bDiffG = sdlG != backG;
    bool bDiffB = sdlB != backB;
    bool bDiffA = sdlA != backA;
    if (!bDiffR && !bDiffG && !bDiffB && !bDiffA) {
        //颜色格式相同，无需更新颜色数据
        return;
    }
    uint32_t colorValue = 0;
    const int32_t nMaxRow = rcPaint.top + rcPaint.Height();
    const int32_t nWidth = rcPaint.Width();
    for (int32_t nRow = rcPaint.top; nRow < nMaxRow; ++nRow) {
        for (int32_t nCol = 0; nCol < nWidth; ++nCol) {
            uint32_t* pColorValue = (uint32_t*)surfacePixels + nRow * nSurfaceWidth + rcPaint.left + nCol;
            colorValue = *pColorValue;
            if (bDiffR) {
                ((uint8_t*)pColorValue)[sdlR] = ((uint8_t*)&colorValue)[backR];
            }
            if (bDiffG) {
                ((uint8_t*)pColorValue)[sdlG] = ((uint8_t*)&colorValue)[backG];
            }
            if (bDiffB) {
                ((uint8_t*)pColorValue)[sdlB] = ((uint8_t*)&colorValue)[backB];
            }
            if (bDiffA) {
                ((uint8_t*)pColorValue)[sdlA] = ((uint8_t*)&colorValue)[backA];
            }
        }
    }
}

void SkRasterWindowContext_SDL::UpdateColorAlpha(void* surfacePixels, int32_t nSurfaceWidth, const UiRect& rcPaint, uint8_t nLayeredWindowAlpha,
                                                 int32_t sdlR, int32_t sdlG, int32_t sdlB, int32_t sdlA)
{
    if ((surfacePixels == nullptr) || (nSurfaceWidth < 1) || rcPaint.IsEmpty() || (nLayeredWindowAlpha == 255)) {
        return;
    }
    const int32_t nMaxRow = rcPaint.top + rcPaint.Height();
    const int32_t nWidth = rcPaint.Width();
    for (int32_t nRow = rcPaint.top; nRow < nMaxRow; ++nRow) {
        for (int32_t nCol = 0; nCol < nWidth; ++nCol) {
            uint32_t* pColorValue = (uint32_t*)surfacePixels + nRow * nSurfaceWidth + rcPaint.left + nCol;
            uint8_t& r = ((uint8_t*)pColorValue)[sdlR];
            uint8_t& g = ((uint8_t*)pColorValue)[sdlG];
            uint8_t& b = ((uint8_t*)pColorValue)[sdlB];
            uint8_t& a = ((uint8_t*)pColorValue)[sdlA];
            r = r * nLayeredWindowAlpha / 255;
            g = g * nLayeredWindowAlpha / 255;
            b = b * nLayeredWindowAlpha / 255;
            a = a * nLayeredWindowAlpha / 255;
        }
    }
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
