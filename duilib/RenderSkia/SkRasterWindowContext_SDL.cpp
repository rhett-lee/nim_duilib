#include "SkRasterWindowContext_SDL.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/PerformanceUtil.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

namespace ui {

SkRasterWindowContext_SDL::SkRasterWindowContext_SDL(SDL_Window* sdlWindow, std::unique_ptr<const skwindow::DisplayParams> params):
    skwindow::internal::RasterWindowContext(std::move(params)),
    m_sdlWindow(sdlWindow),
    m_sdlTextrue(nullptr),
    m_bBackBufferValid(false)
{
    fWidth = 0;
    fHeight = 0;
}

SkRasterWindowContext_SDL::~SkRasterWindowContext_SDL()
{
    Clear();
}

void SkRasterWindowContext_SDL::Clear()
{
    //缓冲内容失效，不能直接提交到窗口
    m_bBackBufferValid = false;
    if (m_sdlTextrue != nullptr) {
        SDL_DestroyTexture(m_sdlTextrue);
        m_sdlTextrue = nullptr;
    }
    m_fBackbufferSurface.reset();
    m_fSurfaceMemory.reset();
}

void SkRasterWindowContext_SDL::setDisplayParams(std::unique_ptr<const skwindow::DisplayParams> params)
{
    int32_t nWidth = width();
    int32_t nHeight = height();
    fDisplayParams = std::move(params);
    Clear();
    if ((nWidth > 0) && (nHeight > 0)) {
        this->resize(nWidth, nHeight);
    }   
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
            ASSERT(m_fSurfaceMemory.get() != nullptr);
            ASSERT(m_fBackbufferSurface != nullptr);
        }
        return;
    }
    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    ASSERT(pDisplayParams != nullptr);
    if (pDisplayParams == nullptr) {
        return;
    }

    fWidth = nWidth;
    fHeight = nHeight;

    //缓冲重建，内容失效
    m_bBackBufferValid = false;

    Clear();
    if ((nWidth == 0) || (nHeight == 0)) {
        return;
    }

    //注意：nWidth * nHeight * sizeof(uint32_t) 在 int32 范围内可能溢出（如 32768*32768*4 > INT32_MAX），必须先转为 size_t
    m_fSurfaceMemory.reset((size_t)nWidth * nHeight * sizeof(uint32_t));
    void* pixels = m_fSurfaceMemory.get();
    ASSERT(pixels != nullptr);
    if (pixels == nullptr) {
        m_fSurfaceMemory.reset();
        fWidth = 0;
        fHeight = 0;
        return;
    }

    SkImageInfo info = SkImageInfo::Make(nWidth, nHeight, pDisplayParams->colorType(), SkAlphaType::kPremul_SkAlphaType, pDisplayParams->colorSpace());
    m_fBackbufferSurface = SkSurfaces::WrapPixels(info, pixels, (size_t)sizeof(uint32_t) * nWidth);
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
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }
    ASSERT(m_fBackbufferSurface != nullptr);
    if (m_fBackbufferSurface == nullptr) {
        return false;
    }
    ASSERT(pRender != nullptr);
    if (pRender == nullptr) {
        return false;
    }
    ASSERT(pRenderPaint != nullptr);
    if (pRenderPaint == nullptr) {
        return false;
    }

    //窗口透明度
    uint8_t nLayeredWindowAlpha = pRenderPaint->GetLayeredWindowAlpha();

    //只重新提交后备缓冲（窗口暴露、遮挡恢复、最小化还原等场景）：
    //此时后备缓冲的内容仍然有效，直接把缓冲内容提交到窗口，跳过整棵控件树的重新绘制
    if (pRenderPaint->IsPresentOnly() && m_bBackBufferValid) {
        UiRect rcClient;
        GetClientRect(rcClient);
        if (!rcClient.IsEmpty() && (rcClient.Width() == width()) && (rcClient.Height() == height())) {
            std::vector<UiRect> rcPresentList;
            rcPresentList.push_back(rcClient);
            if (SwapPaintBuffers(rcPresentList, nLayeredWindowAlpha)) {
                //标记为有效区域，避免系统重复发送暴露事件
                ValidateRect(rcPresentList);
                return true;
            }
        }
        //后备缓冲不可用：退化为正常绘制流程
    }

    //获取需要绘制的区域列表（可能是多个不连续的脏矩形）
    std::vector<UiRect> rcPaintList;
    bool bUpdateRect = pRenderPaint->GetUpdateRects(rcPaintList); //返回true表示支持局部绘制，只绘制更新的部分区域，以提高效率
    UiRect rcClient;
    GetClientRect(rcClient);
    UiRect rcPaint; //所有脏矩形的并集边界
    if (bUpdateRect) {
        //确保区域的有效性
        std::vector<UiRect> rcValidList;
        for (UiRect& rcItem : rcPaintList) {
            rcItem.Intersect(rcClient);
            if (!rcItem.IsEmpty()) {
                rcValidList.push_back(rcItem);
                rcPaint.Union(rcItem);
            }
        }
        rcPaintList.swap(rcValidList);
    }
    if (rcPaint.IsEmpty()) {
        //不支持局部绘制，每次都是需要重绘整个窗口的客户区域
        rcPaintList.clear();
        rcPaintList.push_back(rcClient);
        rcPaint = rcClient;
    }
    if (rcPaint.IsEmpty()) {
        //无需绘制
        return false;
    }

    //是否为完全绘制
    const bool bFullPaint = (rcPaint.Width() == width()) && (rcPaint.Height() == height());
    if (bFullPaint) {
        //完全绘制时，使用单个矩形，走快速路径
        rcPaintList.clear();
        rcPaintList.push_back(rcClient);
    }

    //设置本次绘制的脏区列表及画布的裁剪区域（多个脏矩形的并集，脏区之外不会被重新光栅化）
    int32_t nClipState = pRender->BeginPaintRects(rcPaintList);

    //执行绘制（使用并集边界作为绘制范围，控件级使用脏区列表精确剔除）
    bool bRet = pRenderPaint->DoPaint(rcPaint);

    //恢复裁剪状态
    pRender->EndPaintRects(nClipState);

    if (bRet) {
        //绘制完成后，更新到窗口
        bool bSwapped = SwapPaintBuffers(rcPaintList, nLayeredWindowAlpha);
        //完整绘制成功后，后备缓冲保存了客户区的完整图像；局部绘制不会破坏其它区域的内容
        if (bSwapped && bFullPaint) {
            m_bBackBufferValid = true;
        }
    }

    //绘制完成后，将已经绘制的区域标记为有效区域
    if (bUpdateRect) {
        ValidateRect(rcPaintList);
    }
    return bRet;
}

bool SkRasterWindowContext_SDL::SwapPaintBuffers(const std::vector<UiRect>& rcPaintList, uint8_t nLayeredWindowAlpha)
{
    PerformanceUtil statPerformance(_T("PaintWindow 9, SkRasterWindowContext_SDL::SwapPaintBuffers"));
    ASSERT(!rcPaintList.empty());
    if (rcPaintList.empty()) {
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

    if (SwapPaintBuffersFast(rcPaintList, nLayeredWindowAlpha)) {
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
    if (!((rcPaintList.size() == 1) && (rcPaintList[0].Width() == width()) && (rcPaintList[0].Height() == height()))) {
        //局部绘制：只绘制更新的部分（按矩形列表逐个更新）
        bDrawOk = true;
        for (const UiRect& rcPaint : rcPaintList) {
            if (rcPaint.IsEmpty()) {
                continue;
            }
            SDL_Rect rect;
            rect.x = rcPaint.left;
            rect.y = rcPaint.top;
            rect.w = rcPaint.Width();
            rect.h = rcPaint.Height();
            SkIRect bounds = SkIRect::MakeLTRB(rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
            sk_sp<SkImage> snapshotImage = m_fBackbufferSurface->makeImageSnapshot(bounds);
            bool bRectOk = false;
            if (snapshotImage != nullptr) {
                SkPixmap pixmap;
                if (snapshotImage->peekPixels(&pixmap) && (pixmap.addr() != nullptr) && (pixmap.width() == rcPaint.Width()) && (pixmap.height() == rcPaint.Height())) {
                    SDL_UpdateTexture(m_sdlTextrue, &rect, pixmap.addr(), (int)pixmap.rowBytes());
                    bRectOk = true;
                }
            }
            bDrawOk = bDrawOk && bRectOk;
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

bool SkRasterWindowContext_SDL::SwapPaintBuffersFast(const std::vector<UiRect>& rcPaintList, uint8_t nLayeredWindowAlpha)
{
    ASSERT(!rcPaintList.empty());
    if (rcPaintList.empty()) {
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
    PerformanceUtil statPerformance(_T("PaintWindow 9, SkRasterWindowContext_SDL::SwapPaintBuffersFast"));

    bool bDrawOk = false;
    const bool bFullPaint = (rcPaintList.size() == 1) && (rcPaintList[0].Width() == width()) && (rcPaintList[0].Height() == height());
    if (!bFullPaint) {
        //局部绘制：只绘制更新的部分（按矩形列表逐个复制）
        std::vector<SDL_Rect> sdlRects;
        sdlRects.reserve(rcPaintList.size());
        for (const UiRect& rcPaint : rcPaintList) {
            if (rcPaint.IsEmpty()) {
                continue;
            }
            SDL_Rect rect;
            rect.x = rcPaint.left;
            rect.y = rcPaint.top;
            rect.w = rcPaint.Width();
            rect.h = rcPaint.Height();
            sdlRects.push_back(rect);

            //按行复制数据(每次复制1行数据)
            const int32_t nMaxRow = rcPaint.top + rcPaint.Height();
            const int32_t nWidth = rcPaint.Width();
            for (int32_t nRow = rcPaint.top; nRow < nMaxRow; ++nRow) {
                //注意：nRow * sdlSurface->w 是 int * int，溢出后转 size_t 错误；必须先转为 size_t
                const size_t nRowOffset = (size_t)nRow * sdlSurface->w;
                ::memcpy((uint32_t*)sdlSurface->pixels + nRowOffset + rcPaint.left,
                         (uint32_t*)m_fSurfaceMemory.get() + nRowOffset + rcPaint.left,
                         (size_t)nWidth * sizeof(uint32_t));
            }

            //处理颜色顺序
            UpdateColorByteOrder(sdlSurface->pixels, sdlSurface->w, rcPaint, backR, backG, backB, backA, sdlR, sdlG, sdlB, sdlA);
            UpdateColorAlpha(sdlSurface->pixels, sdlSurface->w, rcPaint, nLayeredWindowAlpha, sdlR, sdlG, sdlB, sdlA);
        }
        if (!sdlRects.empty()) {
            SDL_UpdateWindowSurfaceRects(m_sdlWindow, sdlRects.data(), (int)sdlRects.size());
            bDrawOk = true;
            ASSERT(bDrawOk);
        }
    }
    if (!bDrawOk) {
        //完整绘制
        ::memcpy(sdlSurface->pixels, m_fSurfaceMemory.get(), sdlSurface->h * sdlSurface->pitch);
        UiRect rcClient;
        GetClientRect(rcClient);
        UpdateColorByteOrder(sdlSurface->pixels, sdlSurface->w, rcClient, backR, backG, backB, backA, sdlR, sdlG, sdlB, sdlA);
        UpdateColorAlpha(sdlSurface->pixels, sdlSurface->w, rcClient, nLayeredWindowAlpha, sdlR, sdlG, sdlB, sdlA);
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
            //注意：nRow * nSurfaceWidth 是 int * int，溢出后转 size_t 错误；必须先转为 size_t
            uint32_t* pColorValue = (uint32_t*)surfacePixels + (size_t)nRow * nSurfaceWidth + rcPaint.left + nCol;
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
            //注意：nRow * nSurfaceWidth 是 int * int，溢出后转 size_t 错误；必须先转为 size_t
            uint32_t* pColorValue = (uint32_t*)surfacePixels + (size_t)nRow * nSurfaceWidth + rcPaint.left + nCol;
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
    ASSERT(m_sdlWindow != nullptr);
    ASSERT((width() > 0) && (height() > 0));
    rcClient.left = 0;
    rcClient.top = 0;
    rcClient.right = rcClient.left + width();
    rcClient.bottom = rcClient.top + height();
}

void SkRasterWindowContext_SDL::ValidateRect(const std::vector<UiRect>& rcPaintList) const
{
    if ((m_sdlWindow == nullptr) || rcPaintList.empty()) {
        return;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    SDL_PropertiesID propID = SDL_GetWindowProperties(m_sdlWindow);
    HWND hWnd = (HWND)SDL_GetPointerProperty(propID, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    if ((hWnd != nullptr) && ::IsWindow(hWnd)) {
        for (const UiRect& rcPaint : rcPaintList) {
            if (rcPaint.IsEmpty()) {
                continue;
            }
            RECT rectPaint = { 0, };
            rectPaint.left = rcPaint.left;
            rectPaint.top = rcPaint.top;
            rectPaint.right = rcPaint.right;
            rectPaint.bottom = rcPaint.bottom;
            ::ValidateRect(hWnd, &rectPaint);
        }
    }
#endif
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_SDL
