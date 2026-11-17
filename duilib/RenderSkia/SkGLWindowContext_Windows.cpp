#include "SkGLWindowContext_Windows.h"
#include "SkOffscreenGLContext.h"
#include "duilib/Render/IRender.h"

// Skia 上游没有 Windows ARM64 的 WGL 实现：SkWGL_win.cpp 在 _M_ARM64 下被整体
// 排除，GrGLMakeNativeInterface() 在 ARM64 下直接返回 nullptr，因此这一渲染后端
// 在 ARM64 上不可用（Render_Skia_Windows 会自动回退到 CPU 绘制）。
#if defined(DUILIB_BUILD_FOR_WIN) && !defined(_M_ARM64)

#include "SkiaHeaderBegin.h"

#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "tools/ganesh/gl/win/SkWGL.h"
#include "tools/window/GLWindowContext.h"

#include <GL/gl.h>

#include "SkiaHeaderEnd.h"

namespace ui {

SkGLWindowContext_Windows::SkGLWindowContext_Windows(HWND hWnd, std::unique_ptr<const skwindow::DisplayParams> params):
    skwindow::internal::GLWindowContext(std::move(params)),
    m_hWnd(hWnd),
    m_hDC(nullptr),
    m_fHGLRC(nullptr),
    m_bNeedFullRedraw(true),
    m_bRenderDirectToFBO0(false),
    m_bSingleSampleSurfaces(false)
{
    ASSERT(::IsWindow(hWnd));
    fWidth = 0;
    fHeight = 0;
    this->initializeContext();
    if (isValid()) {
        //注册到当前线程，供GPU离屏渲染复用该GL上下文
        OffscreenGLContext::RegisterWindowContext(this);
    }
}

SkGLWindowContext_Windows::~SkGLWindowContext_Windows()
{
    OffscreenGLContext::UnregisterWindowContext(this);
    //销毁GL上下文之前必须先释放表面对象，避免Skia访问已销毁的GL资源
    //释放GPU资源需要本窗口的上下文是当前上下文
    MakeCurrent();
    m_persistentSurface.reset();
    m_windowSurface.reset();
    //离屏渲染可能共享本窗口的GrDirectContext：销毁GL上下文前先将其标记为废弃状态，
    //使其后续的绘制调用变为安全的空操作，避免访问已经销毁的OpenGL上下文
    if (auto pDirectContext = this->directContext()) {
        pDirectContext->abandonContext();
    }
    this->destroyContext();
}

void SkGLWindowContext_Windows::resize(int nWidth, int nHeight)
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
    //窗口缩放时只重建SkSurface，保持WGL上下文与GrDirectContext不变：
    //这样拖动缩放时不会反复销毁/创建OpenGL上下文，避免卡顿和闪烁
    if (!MakeCurrent()) {
        return;
    }
    fWidth = nWidth;
    fHeight = nHeight;
    //下次getBackbufferSurface()时按新尺寸重建绘制表面
    fSurface.reset();
    m_persistentSurface.reset();
    m_windowSurface.reset();
    m_bNeedFullRedraw = true;
    glViewport(0, 0, nWidth, nHeight);
}

void SkGLWindowContext_Windows::onSwapBuffers()
{
    if (m_hDC == nullptr) {
        return;
    }
    ::SwapBuffers(m_hDC);
}

bool SkGLWindowContext_Windows::MakeCurrent()
{
    if (m_fHGLRC == nullptr) {
        return false;
    }
    //如果当前上下文已经是本窗口的上下文，则无需重复设置（该检查开销极小，可避免每帧多次调用wglMakeCurrent）
    if (::wglGetCurrentContext() == m_fHGLRC) {
        return true;
    }
    if (m_hDC == nullptr) {
        ASSERT(::IsWindow(m_hWnd));
        if (!::IsWindow(m_hWnd)) {
            return false;
        }
        m_hDC = ::GetDC(m_hWnd);
        if (m_hDC == nullptr) {
            return false;
        }
    }
    return ::wglMakeCurrent(m_hDC, m_fHGLRC) != FALSE;
}

void SkGLWindowContext_Windows::ReleaseWindowDC()
{
    if (m_hDC != nullptr) {
        if (::IsWindow(m_hWnd)) {
            ::ReleaseDC(m_hWnd, m_hDC);
        }
        m_hDC = nullptr;
    }
}

sk_sp<SkSurface> SkGLWindowContext_Windows::getBackbufferSurface()
{
    //同一线程内可能存在多个使用OpenGL绘制的窗口，读取后台缓冲前必须确保本窗口的上下文是当前上下文
    if (!MakeCurrent()) {
        return nullptr;
    }

    // ---- 渲染模式感知 ----
    // 当 m_bRenderDirectToFBO0 为 true 时，跳过 persistent surface 直接返回 FBO 0 的 surface
    // 这样整条链路（GetSkCanvas → IRender::DoPaint）都画到 FBO 0，避免中间拷贝
    if (m_bRenderDirectToFBO0) {
        return GetWindowSurface();
    }

    //尺寸变化后，持久化表面需要重建
    if ((m_persistentSurface != nullptr) &&
        ((m_persistentSurface->width() != fWidth) || (m_persistentSurface->height() != fHeight))) {
        m_persistentSurface.reset();
        m_bNeedFullRedraw = true;
    }
    if (m_persistentSurface != nullptr) {
        return m_persistentSurface;
    }

    if ((fWidth <= 0) || (fHeight <= 0)) {
        //窗口最小化等场景：没有可用的绘制尺寸
        return nullptr;
    }

    GrDirectContext* pDirectContext = this->directContext();
    if (pDirectContext == nullptr) {
        return nullptr;
    }

    //创建持久化的GPU渲染目标：界面绘制到该表面，每帧只在脏区域内重绘，内容在帧间保留
    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    SkImageInfo info = SkImageInfo::Make(fWidth, fHeight, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                         (pDisplayParams != nullptr) ? pDisplayParams->colorSpace() : nullptr);
    const SkSurfaceProps* pSurfaceProps = (pDisplayParams != nullptr) ? &pDisplayParams->surfaceProps() : nullptr;
    int nSampleCount = GetSurfaceSampleCount();
    m_persistentSurface = SkSurfaces::RenderTarget(pDirectContext,
                                                   skgpu::Budgeted::kNo,
                                                   info,
                                                   nSampleCount,
                                                   kTopLeft_GrSurfaceOrigin,
                                                   pSurfaceProps);
    if ((m_persistentSurface == nullptr) && (nSampleCount > 1)) {
        //部分GPU支持多重采样的窗口缓冲（默认帧缓冲），但不支持多重采样的渲染目标（渲染到纹理）：
        //此时按像素格式的采样数创建表面会失败，导致整个GL后端不可用，因此退化为单采样表面
        m_bSingleSampleSurfaces = true;
        nSampleCount = 1;
        m_persistentSurface = SkSurfaces::RenderTarget(pDirectContext,
                                                       skgpu::Budgeted::kNo,
                                                       info,
                                                       nSampleCount,
                                                       kTopLeft_GrSurfaceOrigin,
                                                       pSurfaceProps);
    }
    if (m_persistentSurface == nullptr) {
        //无法创建GPU渲染目标时，退化为直接绘制到窗口的后台缓冲
        return GetWindowSurface();
    }
    //新创建的表面内容未定义：清空并标记需要完整重绘
    m_persistentSurface->getCanvas()->clear(SK_ColorTRANSPARENT);
    m_bNeedFullRedraw = true;
    return m_persistentSurface;
}

sk_sp<SkSurface> SkGLWindowContext_Windows::GetWindowSurface()
{
    //尺寸变化后需要重建（窗口缩放时fWidth/fHeight已更新）
    if ((m_windowSurface != nullptr) &&
        ((m_windowSurface->width() != fWidth) || (m_windowSurface->height() != fHeight))) {
        m_windowSurface.reset();
    }
    if (m_windowSurface != nullptr) {
        return m_windowSurface;
    }
    GrDirectContext* pDirectContext = this->directContext();
    if ((pDirectContext == nullptr) || (fWidth <= 0) || (fHeight <= 0)) {
        return nullptr;
    }

    //显式包装窗口的默认帧缓冲(FBO 0)，不能使用基类的实现：
    //基类按"当前绑定的FBO"来包装，而持久化表面创建或使用之后，当前绑定的FBO可能已不是默认帧缓冲，
    //这会导致把持久化表面自身当成窗口后台缓冲，界面无法显示
    GrGLFramebufferInfo fbInfo;
    fbInfo.fFBOID = 0;
    fbInfo.fFormat = 0x8058; //GL_RGBA8
    fbInfo.fProtected = skgpu::Protected::kNo;

    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    sk_sp<SkColorSpace> colorSpace;
    const SkSurfaceProps* pSurfaceProps = nullptr;
    if (pDisplayParams != nullptr) {
        colorSpace = pDisplayParams->colorSpace();
        pSurfaceProps = &pDisplayParams->surfaceProps();
    }
    int nSampleCount = GetSurfaceSampleCount();
    GrBackendRenderTarget backendRenderTarget =
        GrBackendRenderTargets::MakeGL(fWidth, fHeight, nSampleCount, fStencilBits, fbInfo);
    m_windowSurface = SkSurfaces::WrapBackendRenderTarget(pDirectContext,
                                                         backendRenderTarget,
                                                         kBottomLeft_GrSurfaceOrigin,
                                                         kRGBA_8888_SkColorType,
                                                         colorSpace,
                                                         pSurfaceProps);
    if ((m_windowSurface == nullptr) && (nSampleCount > 1)) {
        //同getBackbufferSurface()：不支持多重采样渲染目标时，退化为单采样的窗口表面
        m_bSingleSampleSurfaces = true;
        nSampleCount = 1;
        backendRenderTarget = GrBackendRenderTargets::MakeGL(fWidth, fHeight, nSampleCount, fStencilBits, fbInfo);
        m_windowSurface = SkSurfaces::WrapBackendRenderTarget(pDirectContext,
                                                             backendRenderTarget,
                                                             kBottomLeft_GrSurfaceOrigin,
                                                             kRGBA_8888_SkColorType,
                                                             colorSpace,
                                                             pSurfaceProps);
    }
    return m_windowSurface;
}

int SkGLWindowContext_Windows::GetSurfaceSampleCount() const
{
    if (m_bSingleSampleSurfaces) {
        return 1;
    }
    return (fSampleCount > 0) ? fSampleCount : 1;
}

sk_sp<const GrGLInterface> SkGLWindowContext_Windows::onInitializeContext()
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return nullptr;
    }
    const skwindow::DisplayParams* pDisplayParams = getDisplayParams();
    ASSERT(pDisplayParams != nullptr);
    if (pDisplayParams == nullptr) {
        return nullptr;
    }

    if (m_fHGLRC != nullptr) {
        destroyContext();
    }

    if (m_hDC == nullptr) {
        m_hDC = ::GetDC(m_hWnd);
    }
    if (m_hDC == nullptr) {
        return nullptr;
    }

    m_fHGLRC = SkCreateWGLContext(m_hDC, pDisplayParams->msaaSampleCount(), false /* deepColor */,
                                  kGLPreferCompatibilityProfile_SkWGLContextRequest);
    if (nullptr == m_fHGLRC) {
        ReleaseWindowDC();
        return nullptr;
    }

    //创建成功后必须立即设置为当前上下文：后续的Vsync设置和状态初始化都依赖当前上下文
    if (!::wglMakeCurrent(m_hDC, m_fHGLRC)) {
        //上下文不可用时，必须清理已创建的上下文并返回失败，由上层回退到CPU绘制
        ::wglDeleteContext(m_fHGLRC);
        m_fHGLRC = nullptr;
        ReleaseWindowDC();
        return nullptr;
    }

    //Vsync必须在上下文成为当前上下文后设置，否则设置无效
    SkWGLExtensions extensions;
    if (extensions.hasExtension(m_hDC, "WGL_EXT_swap_control")) {
        extensions.swapInterval(pDisplayParams->disableVsync() ? 0 : 1);
    }

    //检测是否附加了RenderDoc，如果附加了，则使用Core Profile重新创建上下文
    {
        sk_sp<const GrGLInterface> glInterface = GrGLMakeNativeInterface();
        const bool renderDocAttached = (glInterface != nullptr) && glInterface->hasExtension("GL_EXT_debug_tool");
        glInterface.reset();
        if (renderDocAttached) {
            //删除当前上下文前，必须先取消当前上下文，否则删除会失败
            ::wglMakeCurrent(nullptr, nullptr);
            ::wglDeleteContext(m_fHGLRC);
            m_fHGLRC = SkCreateWGLContext(m_hDC, pDisplayParams->msaaSampleCount(), false /* deepColor */,
                                          kGLPreferCoreProfile_SkWGLContextRequest);
            if (nullptr == m_fHGLRC) {
                ReleaseWindowDC();
                return nullptr;
            }
            if (!::wglMakeCurrent(m_hDC, m_fHGLRC)) {
                ::wglDeleteContext(m_fHGLRC);
                m_fHGLRC = nullptr;
                ReleaseWindowDC();
                return nullptr;
            }
            //重新创建上下文后，需要重新设置Vsync
            SkWGLExtensions coreExtensions;
            if (coreExtensions.hasExtension(m_hDC, "WGL_EXT_swap_control")) {
                coreExtensions.swapInterval(pDisplayParams->disableVsync() ? 0 : 1);
            }
        }
    }

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // use DescribePixelFormat to get the stencil and color bit depth.
    int pixelFormat = ::GetPixelFormat(m_hDC);
    PIXELFORMATDESCRIPTOR pfd = {0, };
    DescribePixelFormat(m_hDC, pixelFormat, sizeof(pfd), &pfd);
    fStencilBits = pfd.cStencilBits;

    // Get sample count if the MSAA WGL extension is present
    if (extensions.hasExtension(m_hDC, "WGL_ARB_multisample")) {
        static const int kSampleCountAttr = SK_WGL_SAMPLES;
        extensions.getPixelFormatAttribiv(m_hDC,
            pixelFormat,
            0,
            1,
            &kSampleCountAttr,
            &fSampleCount);
        fSampleCount = std::max(fSampleCount, 1);
    }
    else {
        fSampleCount = 1;
    }

    RECT rect;
    ::GetClientRect(m_hWnd, &rect);
    fWidth = rect.right - rect.left;
    fHeight = rect.bottom - rect.top;
    glViewport(0, 0, fWidth, fHeight);
    return GrGLMakeNativeInterface();
}

void SkGLWindowContext_Windows::onDestroyContext()
{
    if (m_fHGLRC != nullptr) {
        //不再依赖"本上下文恰好是当前上下文"这一前提，先切换为本窗口的上下文，再取消当前上下文并删除
        MakeCurrent();
        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(m_fHGLRC);
        m_fHGLRC = nullptr;
    }
    ReleaseWindowDC();
}

bool SkGLWindowContext_Windows::PaintAndSwapBuffers(IRender* /*pRender*/, IRenderPaint* pRenderPaint)
{
    HWND hWnd = m_hWnd;
    ASSERT(::IsWindow(hWnd));
    if ((hWnd == nullptr) || !::IsWindow(hWnd)) {
        return false;
    }
    ASSERT(pRenderPaint != nullptr);
    if (pRenderPaint == nullptr) {
        return false;
    }

    //同一线程内可能存在多个使用OpenGL绘制的窗口，绘制前必须切换到本窗口的上下文
    if (!MakeCurrent()) {
        return false;
    }

    // ---- 查询更新区域 ----
    // 必须在BeginPaint之前查询：BeginPaint会验证（清空）窗口的更新区域，
    // 之后再调用GetUpdateRect会恒返回false，局部增量刷新将无法生效
    RECT rectClient = { 0, };
    ::GetClientRect(m_hWnd, &rectClient);
    const int32_t clientW = rectClient.right - rectClient.left;
    const int32_t clientH = rectClient.bottom - rectClient.top;
    RECT rectUpdate = { 0, };
    const bool bHasUpdateRect = (::GetUpdateRect(m_hWnd, &rectUpdate, FALSE) != FALSE);

    // ---- 判断本次是否全量重绘 ----
    bool bFullRedraw = m_bNeedFullRedraw;
    if (!bFullRedraw) {
        if (!bHasUpdateRect) {
            //没有更新区域（例如直接调用绘制、窗口尺寸为0等场景）：退化为全量重绘，
            //保证窗口内容始终正确（不能直接跳过绘制）
            bFullRedraw = true;
        }
        else {
            const int32_t updateW = rectUpdate.right - rectUpdate.left;
            const int32_t updateH = rectUpdate.bottom - rectUpdate.top;
            if ((updateW >= clientW) && (updateH >= clientH)) {
                //更新区域覆盖整个客户区：按全量重绘处理
                bFullRedraw = true;
            }
        }
    }

    // ---- 设置绘制目标，让整条 DoPaint 链路跟随 ----
    bool bUsePersistentCopy = false;
    m_bRenderDirectToFBO0 = false;
    if (bFullRedraw) {
        //全量重绘：跳过持久化表面，直接绘制到 FBO 0，避免中间拷贝
        m_bRenderDirectToFBO0 = true;
        //持久化表面的内容已过期（本次重绘覆盖窗口的全部内容），释放其占用的GPU内存
        if (m_persistentSurface != nullptr) {
            m_persistentSurface.reset();
        }
    }
    else {
        //局部重绘：绘制到持久化表面（保留帧间内容，只补脏区域），再整体拷贝到 FBO 0
        if (getBackbufferSurface() == nullptr) {
            return false;
        }
        if (m_persistentSurface == nullptr) {
            //持久化表面不可用（GPU不支持渲染到纹理）：后台缓冲交换后内容未定义，
            //无法保留上一帧的内容，只能退化为直接绘制到FBO 0的全量重绘
            m_bRenderDirectToFBO0 = true;
            bFullRedraw = true;
        }
        else {
            bUsePersistentCopy = true;
            if (m_bNeedFullRedraw) {
                //持久化表面刚创建（内容已清为透明）或因窗口尺寸变化重建：
                //本次必须完整绘制，否则把整幅表面拷贝到窗口后，脏区域以外的内容会被清空
                bFullRedraw = true;
            }
        }
    }

    // ---- 确定 rcPaint ----
    UiRect rcPaint;
    if (bFullRedraw) {
        rcPaint = UiRect(rectClient.left, rectClient.top, rectClient.right, rectClient.bottom);
    }
    else {
        //局部重绘：只绘制脏区域
        rcPaint = UiRect(rectUpdate.left, rectUpdate.top, rectUpdate.right, rectUpdate.bottom);
    }
    if (rcPaint.IsEmpty()) {
        m_bRenderDirectToFBO0 = false;
        return false;
    }
    m_bNeedFullRedraw = false;

    // ---- 按Win32的PAINT协议处理：BeginPaint验证窗口的更新区域 ----
    PAINTSTRUCT ps = { 0, };
    ::BeginPaint(hWnd, &ps);

    // ---- 执行绘制 ----
    // DoPaint 内部通过 IRender::GetSkCanvas() → Render_Skia_Windows::GetSkCanvas()
    //   → m_pWindowContext->getBackbufferSurface() → 我们设的模式标志自动生效
    pRenderPaint->DoPaint(rcPaint);

    // ---- 局部重绘：把持久化表面的整幅内容拷贝到 FBO 0 ----
    if (bUsePersistentCopy) {
        sk_sp<SkSurface> backbuffer = GetWindowSurface();
        if (backbuffer != nullptr) {
            SkCanvas* pCanvas = backbuffer->getCanvas();
            if ((pCanvas != nullptr) && (m_persistentSurface != nullptr)) {
                SkPaint paint;
                paint.setBlendMode(SkBlendMode::kSrc);
                sk_sp<SkImage> snapshot = m_persistentSurface->makeImageSnapshot();
                if (snapshot != nullptr) {
                    pCanvas->drawImage(snapshot, 0, 0, SkSamplingOptions(SkFilterMode::kNearest), &paint);
                }
            }
        }
    }

    // 清理渲染模式标志（必须在 EndPaint 之前：EndPaint 内部可能触发重绘消息的回调）
    m_bRenderDirectToFBO0 = false;

    ::EndPaint(hWnd, &ps);

    //异步提交(kNo)：SwapBuffers本身会与显示同步，不需要每帧等待CPU完成绘制
    if (auto dContext = this->directContext()) {
        dContext->flushAndSubmit(GrSyncCpu::kNo);
    }

    swapBuffers();
    return true;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN && !_M_ARM64
