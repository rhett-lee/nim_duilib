#include "SkRasterWindowContext_SDL.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/PerformanceUtil.h"
#ifdef DUILIB_SDL_SWAP_BUFFER_DIAGNOSTICS
#include "duilib/Utils/LogUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#endif

#ifdef DUILIB_BUILD_FOR_SDL

#include <SDL3/SDL.h>

/** 快路径诊断日志的调用宏：未定义 DUILIB_SDL_SWAP_BUFFER_DIAGNOSTICS 时不产生任何代码
*/
#ifdef DUILIB_SDL_SWAP_BUFFER_DIAGNOSTICS
#define SDL_SWAP_STATS_LOG(nReasonCode, pSdlSurface) LogSwapBufferDiagnostics(nReasonCode, pSdlSurface)
#else
#define SDL_SWAP_STATS_LOG(nReasonCode, pSdlSurface) ((void)0)
#endif

namespace ui {

#ifdef DUILIB_SDL_SWAP_BUFFER_DIAGNOSTICS

/** 将SDL返回的字符串转换为DString（SDL返回的字符串都是UTF-8编码）
*/
static DString SdlTextToDString(const char* sdlText)
{
    if (sdlText == nullptr) {
        return DString();
    }
    return StringConvert::WStringToT(StringConvert::UTF8ToWString(std::string(sdlText)));
}

/** 将窗口标志转换为便于阅读的字符串（用于诊断，确认窗口是否被创建为OpenGL/Vulkan/Metal窗口）
*/
static DString SdlWindowFlagsToDString(SDL_WindowFlags nFlags)
{
    DString sFlags;
    if ((nFlags & SDL_WINDOW_OPENGL) != 0) {
        sFlags += _T("OPENGL ");
    }
    if ((nFlags & SDL_WINDOW_VULKAN) != 0) {
        sFlags += _T("VULKAN ");
    }
    if ((nFlags & SDL_WINDOW_METAL) != 0) {
        sFlags += _T("METAL ");
    }
    if ((nFlags & SDL_WINDOW_HIGH_PIXEL_DENSITY) != 0) {
        sFlags += _T("HIGH_DPI ");
    }
    if ((nFlags & SDL_WINDOW_BORDERLESS) != 0) {
        sFlags += _T("BORDERLESS ");
    }
    if ((nFlags & SDL_WINDOW_HIDDEN) != 0) {
        sFlags += _T("HIDDEN ");
    }
    if (sFlags.empty()) {
        sFlags = _T("<无>");
    }
    return sFlags;
}

#endif //DUILIB_SDL_SWAP_BUFFER_DIAGNOSTICS

SkRasterWindowContext_SDL::SkRasterWindowContext_SDL(SDL_Window* sdlWindow, std::unique_ptr<const skwindow::DisplayParams> params):
    skwindow::internal::RasterWindowContext(std::move(params)),
    m_sdlWindow(sdlWindow),
    m_sdlTextrue(nullptr),
    m_pSDLWindowSurface(nullptr),
    m_bDirectSurface(false),
    m_nDirectSurfacePitch(0),
    m_pDirectSurfacePixels(nullptr),
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
    m_bDirectSurface = false;
    m_pSDLWindowSurface = nullptr;
    m_nDirectSurfacePitch = 0;
    m_pDirectSurfacePixels = nullptr;
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
    //Clear已经释放了缓冲，必须重置尺寸，否则resize会因"尺寸未变化"而提前返回，导致缓冲不再重建
    fWidth = 0;
    fHeight = 0;
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

    //准备后备缓冲：尽可能直接包装SDL窗口表面的像素（省去绘制结果到窗口表面的memcpy）
    //注意：必须在判断"只重新提交后备缓冲"之前执行，因为SDL窗口表面可能在窗口暴露、
    //遮挡恢复、最小化还原等场景下失效，此时需要让后备缓冲失效并重新完整绘制
    EnsureBackbufferSurface(nLayeredWindowAlpha);
    ASSERT(m_fBackbufferSurface != nullptr);
    if (m_fBackbufferSurface == nullptr) {
        return false;
    }

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

    //后备缓冲的内容无效时（刚创建、调整大小或者刚重建），必须整窗完整绘制一次：
    //此时缓冲中除了本次脏区外的其它区域内容未定义（可能是其它窗口的旧图像），
    //如果只做局部绘制并提交，窗口上就会显示残留内容
    if (!m_bBackBufferValid && !rcClient.IsEmpty()) {
        rcPaintList.clear();
        rcPaintList.push_back(rcClient);
        rcPaint = rcClient;
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

    bool bSwapped = false;
    if (bRet) {
        //绘制完成后，更新到窗口
        bSwapped = SwapPaintBuffers(rcPaintList, nLayeredWindowAlpha);
        //完整绘制成功后，后备缓冲保存了客户区的完整图像；局部绘制不会破坏其它区域的内容
        if (bSwapped && bFullPaint) {
            m_bBackBufferValid = true;
        }
    }

    //绘制并提交成功后，将已经绘制的区域标记为有效区域：
    //提交失败时不能标记有效，否则该区域不会再被重绘，界面会停留在旧内容上
    if (bSwapped && bUpdateRect) {
        ValidateRect(rcPaintList);
    }
    return bRet && bSwapped;
}

#ifdef DUILIB_SDL_SWAP_BUFFER_DIAGNOSTICS
void SkRasterWindowContext_SDL::LogSwapBufferDiagnostics(int32_t nReasonCode, const SDL_Surface* pSdlSurface)
{
    if ((nReasonCode < 1) || (nReasonCode > 31)) {
        return;
    }
    const int32_t nReasonFlag = (int32_t)(1u << (nReasonCode - 1));
    if ((m_nLoggedDiagnosticFlags & nReasonFlag) != 0) {
        //同一个原因只输出一次，避免每帧输出导致日志刷屏
        return;
    }
    m_nLoggedDiagnosticFlags |= nReasonFlag;

    DString sReason;
    switch (nReasonCode) {
    case 1:  sReason = _T("窗口透明度不是255，需要逐像素处理Alpha"); break;
    case 2:  sReason = _T("SDL_GetWindowSurface返回空"); break;
    case 3:  sReason = _T("SDL窗口表面的尺寸或像素缓冲无效"); break;
    case 4:  sReason = _T("SDL窗口表面的行字节数(pitch)不满足要求"); break;
    case 5:  sReason = _T("Skia的颜色字节顺序未知"); break;
    case 6:  sReason = _T("SDL窗口表面的颜色字节顺序未知"); break;
    case 7:  sReason = _T("Skia与SDL的颜色字节顺序不一致"); break;
    case 8:  sReason = _T("窗口没有关联的SDL_Renderer"); break;
    case 9:  sReason = _T("直绘模式下SDL窗口表面已失效或被重建"); break;
    case 10: sReason = _T("快路径提交成功（内存缓冲复制到窗口表面）"); break;
    case 11: sReason = _T("直绘路径提交成功（Skia直接绘制到窗口表面）"); break;
    case 12: sReason = _T("创建SDL纹理失败（慢路径不可用）"); break;
    case 13: sReason = _T("SDL_UpdateWindowSurface提交失败"); break;
    default: sReason = _T("未知原因"); break;
    }

    int32_t nWindowWidth = 0;
    int32_t nWindowHeight = 0;
    int32_t nPixelWidth = 0;
    int32_t nPixelHeight = 0;
    SDL_WindowFlags nWindowFlags = 0;
    if (m_sdlWindow != nullptr) {
        SDL_GetWindowSize(m_sdlWindow, &nWindowWidth, &nWindowHeight);
        SDL_GetWindowSizeInPixels(m_sdlWindow, &nPixelWidth, &nPixelHeight);
        nWindowFlags = SDL_GetWindowFlags(m_sdlWindow);
    }

    DString sRendererName = _T("<无>");
    SDL_Renderer* sdlRenderer = (m_sdlWindow != nullptr) ? SDL_GetRenderer(m_sdlWindow) : nullptr;
    if (sdlRenderer != nullptr) {
        sRendererName = SdlTextToDString(SDL_GetRendererName(sdlRenderer));
        if (sRendererName.empty()) {
            sRendererName = _T("<未知>");
        }
    }

    if ((pSdlSurface == nullptr) && (m_sdlWindow != nullptr)) {
        pSdlSurface = SDL_GetWindowSurface(m_sdlWindow);
    }
    DString sSurface;
    if (pSdlSurface == nullptr) {
        sSurface = _T("<空>");
    }
    else {
        sSurface = StringUtil::Printf(_T("w=%d, h=%d, pitch=%d, format=%s, pixels=%s"),
                                      pSdlSurface->w, pSdlSurface->h, pSdlSurface->pitch,
                                      SdlTextToDString(SDL_GetPixelFormatName(pSdlSurface->format)).c_str(),
                                      (pSdlSurface->pixels != nullptr) ? _T("有效") : _T("空"));
    }

    const int32_t nSdlVersion = SDL_GetVersion();
    DString sLog = StringUtil::Printf(
        _T("[SwapPaintBuffers诊断] 原因(%d): %s\n")
        _T("    绘制缓冲尺寸: %dx%d, 直绘模式: %s, 直绘pitch: %d\n")
        _T("    窗口尺寸: 逻辑 %dx%d, 像素 %dx%d, 窗口标志: 0x%llX(%s)\n")
        _T("    SDL_Renderer: %s, 视频驱动: %s, SDL版本: %d.%d.%d, 窗口表面加速: %s\n")
        _T("    SDL窗口表面: %s\n")
        _T("    SDL错误信息: %s"),
        nReasonCode, sReason.c_str(),
        width(), height(), m_bDirectSurface ? _T("true") : _T("false"), m_nDirectSurfacePitch,
        nWindowWidth, nWindowHeight, nPixelWidth, nPixelHeight,
        (unsigned long long)nWindowFlags, SdlWindowFlagsToDString(nWindowFlags).c_str(),
        sRendererName.c_str(), SdlTextToDString(SDL_GetCurrentVideoDriver()).c_str(),
        SDL_VERSIONNUM_MAJOR(nSdlVersion), SDL_VERSIONNUM_MINOR(nSdlVersion), SDL_VERSIONNUM_MICRO(nSdlVersion),
        SdlTextToDString(SDL_GetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION)).c_str(),
        sSurface.c_str(),
        SdlTextToDString(SDL_GetError()).c_str());

    LogUtil::OutputLine(sLog);
}
#endif //DUILIB_SDL_SWAP_BUFFER_DIAGNOSTICS

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

    if (m_bDirectSurface) {
        //直绘模式：绘制数据已经直接写入SDL窗口表面的像素缓冲，只需提交到窗口设备
        return SwapPaintBuffersDirect(rcPaintList, nLayeredWindowAlpha);
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
        SDL_SWAP_STATS_LOG(8, nullptr);
        return false;
    }

    //纹理的尺寸必须与后备缓冲一致，否则重建纹理（SDL创建纹理时，其内容未定义）
    if (m_sdlTextrue != nullptr) {
        float fTextureWidth = 0;
        float fTextureHeight = 0;
        if (SDL_GetTextureSize(m_sdlTextrue, &fTextureWidth, &fTextureHeight)) {
            if ((fTextureWidth != (float)width()) || (fTextureHeight != (float)height())) {
                SDL_DestroyTexture(m_sdlTextrue);
                m_sdlTextrue = nullptr;
            }
        }
    }
    bool bTextureCreated = false;
    if (m_sdlTextrue == nullptr) {
        // 渲染到窗口（IRender -> 绘制到 SDL Render -> 更新到 SDL 窗口）
#ifdef DUILIB_BUILD_FOR_WIN
        SDL_PixelFormat format = SDL_PIXELFORMAT_BGRA32;
#else
        SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA32;
#endif
        m_sdlTextrue = SDL_CreateTexture(sdlRenderer, format, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, width(), height());
        ASSERT(m_sdlTextrue != nullptr);
        //新建纹理的内容是未定义的（可能残留其它窗口的旧图像），必须上传一次完整内容
        bTextureCreated = (m_sdlTextrue != nullptr);
    }
    
    if (m_sdlTextrue == nullptr) {
        SDL_SWAP_STATS_LOG(12, nullptr);
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
    if (!bDrawOk || bTextureCreated) {
        //完整绘制（新建纹理时也必须完整上传，否则未上传的区域会显示残留内容）
        SDL_UpdateTexture(m_sdlTextrue, nullptr, m_fSurfaceMemory.get(), m_fBackbufferSurface->width() * sizeof(uint32_t));
    }

    //设置纹理的透明度（必须每次都设置：窗口透明度恢复为255时，需要清除上次设置的透明度
    //值，否则在无法使用窗口表面的环境下，窗口会一直保持半透明）
    SDL_SetTextureAlphaMod(m_sdlTextrue, nLayeredWindowAlpha);
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
        SDL_SWAP_STATS_LOG(2, nullptr);
        return false;
    }

    if ((sdlSurface->pixels == nullptr) || (sdlSurface->w != width()) || (sdlSurface->h != height())) {
        //大小不匹配
        SDL_SWAP_STATS_LOG(3, sdlSurface);
        return false;
    }
    if (sdlSurface->pitch != width() * (int32_t)sizeof(uint32_t)) {
        //行字节数不匹配（本路径要求逐行数据完全对齐，pitch 必须严格等于 宽度*4）
        SDL_SWAP_STATS_LOG(4, sdlSurface);
        return false;
    }
    
    SkColorType backSurfaceColorType = m_fBackbufferSurface->imageInfo().colorInfo().colorType();
    int32_t backR = -1;
    int32_t backG = -1;
    int32_t backB = -1;
    int32_t backA = -1;
    if (!GetSkiaColorByteOrder(backSurfaceColorType, backR, backG, backB, backA)) {
        SDL_SWAP_STATS_LOG(5, sdlSurface);
        return false;
    }

    SDL_PixelFormat sdlFormat = sdlSurface->format;
    int32_t sdlR = -1;
    int32_t sdlG = -1;
    int32_t sdlB = -1;
    int32_t sdlA = -1;
    if (!GetSDLColorByteOrder(sdlFormat, sdlR, sdlG, sdlB, sdlA)) {
        SDL_SWAP_STATS_LOG(6, sdlSurface);
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
    SDL_SWAP_STATS_LOG(10, sdlSurface);
    return true;
}

void SkRasterWindowContext_SDL::EnsureBackbufferSurface(uint8_t nLayeredWindowAlpha)
{
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return;
    }
    if (nLayeredWindowAlpha != 255) {
        //窗口透明度不是255时，需要逐像素处理Alpha，不能直绘
        SDL_SWAP_STATS_LOG(1, nullptr);
        SwitchToMemoryBackbuffer();
        return;
    }
    SDL_Surface* sdlSurface = SDL_GetWindowSurface(m_sdlWindow);
    if (m_bDirectSurface && (sdlSurface != nullptr) && (sdlSurface == m_pSDLWindowSurface) &&
        (sdlSurface->w == width()) && (sdlSurface->h == height()) &&
        (sdlSurface->pitch == m_nDirectSurfacePitch) && (sdlSurface->pixels == m_pDirectSurfacePixels) &&
        (sdlSurface->pixels != nullptr)) {
        //已经是直绘模式，且SDL窗口表面的尺寸、行字节数和像素缓冲均未发生变化，可以直接复用
        return;
    }
    if (!CanUseDirectSurface(sdlSurface, nLayeredWindowAlpha)) {
        //SDL窗口表面不可用或者颜色格式不兼容：回退到内存缓冲的双缓冲路径
        SwitchToMemoryBackbuffer();
        return;
    }
    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    ASSERT(pDisplayParams != nullptr);
    if (pDisplayParams == nullptr) {
        SwitchToMemoryBackbuffer();
        return;
    }
    //Skia表面直接包装SDL窗口表面的像素：绘制结果直接写入窗口表面，不需要memcpy
    SkImageInfo info = SkImageInfo::Make(width(), height(), pDisplayParams->colorType(),
                                         SkAlphaType::kPremul_SkAlphaType, pDisplayParams->colorSpace());
    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(info, sdlSurface->pixels, (size_t)sdlSurface->pitch);
    ASSERT(surface != nullptr);
    if (surface == nullptr) {
        SwitchToMemoryBackbuffer();
        return;
    }
    m_fBackbufferSurface = std::move(surface);
    m_pSDLWindowSurface = sdlSurface;
    m_nDirectSurfacePitch = sdlSurface->pitch;
    m_pDirectSurfacePixels = sdlSurface->pixels;
    m_bDirectSurface = true;
    //SDL窗口表面的像素内容未知（新创建或者重建），后备缓冲需要完整绘制一次
    m_bBackBufferValid = false;
}

bool SkRasterWindowContext_SDL::CanUseDirectSurface(SDL_Surface* sdlSurface, uint8_t nLayeredWindowAlpha)
{
    if ((sdlSurface == nullptr) || (sdlSurface->pixels == nullptr)) {
        SDL_SWAP_STATS_LOG(2, sdlSurface);
        return false;
    }
    if (nLayeredWindowAlpha != 255) {
        //窗口透明度不是255时，需要逐像素处理Alpha，不能直绘
        SDL_SWAP_STATS_LOG(1, sdlSurface);
        return false;
    }
    if ((sdlSurface->w != width()) || (sdlSurface->h != height())) {
        //窗口表面的尺寸与绘制表面不一致（如窗口尺寸变化后SDL尚未重建表面）
        SDL_SWAP_STATS_LOG(3, sdlSurface);
        return false;
    }
    if (sdlSurface->pitch < (width() * (int32_t)sizeof(uint32_t))) {
        SDL_SWAP_STATS_LOG(4, sdlSurface);
        return false;
    }
    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    ASSERT(pDisplayParams != nullptr);
    if (pDisplayParams == nullptr) {
        return false;
    }

    int32_t backR = -1;
    int32_t backG = -1;
    int32_t backB = -1;
    int32_t backA = -1;
    if (!GetSkiaColorByteOrder(pDisplayParams->colorType(), backR, backG, backB, backA)) {
        SDL_SWAP_STATS_LOG(5, sdlSurface);
        return false;
    }

    int32_t sdlR = -1;
    int32_t sdlG = -1;
    int32_t sdlB = -1;
    int32_t sdlA = -1;
    if (!GetSDLColorByteOrder(sdlSurface->format, sdlR, sdlG, sdlB, sdlA)) {
        SDL_SWAP_STATS_LOG(6, sdlSurface);
        return false;
    }

    //只有颜色字节顺序完全一致时才能直接绘制（否则需要转换颜色字节顺序）
    const bool bSameColorOrder = (sdlR == backR) && (sdlG == backG) && (sdlB == backB) && (sdlA == backA);
    if (!bSameColorOrder) {
        SDL_SWAP_STATS_LOG(7, sdlSurface);
    }
    return bSameColorOrder;
}

void SkRasterWindowContext_SDL::SwitchToMemoryBackbuffer()
{
    if (!m_bDirectSurface) {
        return;
    }
    m_bDirectSurface = false;
    m_pSDLWindowSurface = nullptr;
    m_nDirectSurfacePitch = 0;
    m_pDirectSurfacePixels = nullptr;
    //直绘模式下窗口表面的内容与内存缓冲不一致，需要重新完整绘制一次
    m_bBackBufferValid = false;

    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    ASSERT(pDisplayParams != nullptr);
    if ((pDisplayParams == nullptr) || (m_fSurfaceMemory.get() == nullptr) || (width() <= 0) || (height() <= 0)) {
        m_fBackbufferSurface.reset();
        return;
    }
    SkImageInfo info = SkImageInfo::Make(width(), height(), pDisplayParams->colorType(),
                                         SkAlphaType::kPremul_SkAlphaType, pDisplayParams->colorSpace());
    m_fBackbufferSurface = SkSurfaces::WrapPixels(info, m_fSurfaceMemory.get(), (size_t)sizeof(uint32_t) * width());
}

bool SkRasterWindowContext_SDL::SwapPaintBuffersDirect(const std::vector<UiRect>& rcPaintList, uint8_t nLayeredWindowAlpha)
{
    PerformanceUtil statPerformance(_T("PaintWindow 9, SkRasterWindowContext_SDL::SwapPaintBuffersDirect"));
    ASSERT(!rcPaintList.empty());
    if (rcPaintList.empty()) {
        return false;
    }
    ASSERT(m_sdlWindow != nullptr);
    if (m_sdlWindow == nullptr) {
        return false;
    }
    ASSERT(m_pSDLWindowSurface != nullptr);
    if (m_pSDLWindowSurface == nullptr) {
        return false;
    }
    ASSERT(nLayeredWindowAlpha == 255);
    if (nLayeredWindowAlpha != 255) {
        return false;
    }
    //确认SDL窗口表面未失效（窗口尺寸变化后，SDL会重新创建该表面）
    SDL_Surface* sdlSurface = SDL_GetWindowSurface(m_sdlWindow);
    if ((sdlSurface == nullptr) || (sdlSurface != m_pSDLWindowSurface)) {
        m_bBackBufferValid = false;
        SDL_SWAP_STATS_LOG(9, sdlSurface);
        return false;
    }

    //说明：本函数只负责提交，不再校验颜色字节顺序，其正确性由直绘模式的进入条件保证：
    //1. 进入直绘模式前，EnsureBackbufferSurface()->CanUseDirectSurface() 已逐项校验
    //   Skia后备缓冲的颜色类型与SDL窗口表面的像素格式，只有颜色字节顺序完全一致时才直绘；
    //2. Skia后备缓冲始终使用 getDisplayParams()->colorType()（即平台默认的N32颜色类型），
    //   与 CanUseDirectSurface() 校验时用的是同一个颜色类型，setDisplayParams() 会先退出直绘模式；
    //3. SDL窗口表面的像素格式在窗口创建后固定不变，表面被重建时指针会变化，下面的检查会退出直绘模式，
    //   下次绘制时重新走进入条件的校验。

    //直绘模式下，绘制数据已经在SDL窗口表面的像素缓冲中，直接提交到窗口设备
    const bool bFullPaint = (rcPaintList.size() == 1) && (rcPaintList[0].Width() == width()) && (rcPaintList[0].Height() == height());
    if (bFullPaint) {
        const bool bOk = SDL_UpdateWindowSurface(m_sdlWindow);
        SDL_SWAP_STATS_LOG(bOk ? 11 : 13, sdlSurface);
        return bOk;
    }

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
    }
    if (sdlRects.empty()) {
        return true;
    }
    const bool bOk = SDL_UpdateWindowSurfaceRects(m_sdlWindow, sdlRects.data(), (int)sdlRects.size());
    SDL_SWAP_STATS_LOG(bOk ? 11 : 13, sdlSurface);
    return bOk;
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
