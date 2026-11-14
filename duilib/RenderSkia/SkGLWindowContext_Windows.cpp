#include "SkGLWindowContext_Windows.h"
#include "SkOffscreenGLContext.h"
#include "duilib/Render/IRender.h"

// Skia 上游没有 Windows ARM64 的 WGL 实现：SkWGL_win.cpp 在 _M_ARM64 下被整体
// 排除，GrGLMakeNativeInterface() 在 ARM64 下直接返回 nullptr，因此这一渲染后端
// 在 ARM64 上不可用（Render_Skia_Windows 会自动回退到 CPU 绘制）。
#if defined(DUILIB_BUILD_FOR_WIN) && !defined(_M_ARM64)

#include "SkiaHeaderBegin.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "tools/ganesh/gl/win/SkWGL.h"
#include "tools/window/GLWindowContext.h"

#include <GL/gl.h>

#include "SkiaHeaderEnd.h"

namespace ui {

SkGLWindowContext_Windows::SkGLWindowContext_Windows(HWND hWnd, std::unique_ptr<const skwindow::DisplayParams> params):
    skwindow::internal::GLWindowContext(std::move(params)),
    m_hWnd(hWnd),
    m_hDC(nullptr),
    m_fHGLRC(nullptr)
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
    return BaseClass::getBackbufferSurface();
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

    //设置需要绘制的区域（需要对整个客户区全部绘制，GL不支持局部绘制）
    RECT rectUpdate = { 0, };
    ::GetClientRect(m_hWnd, &rectUpdate);
    UiRect rcPaint;
    rcPaint.left = rectUpdate.left;
    rcPaint.top = rectUpdate.top;
    rcPaint.right = rectUpdate.right;
    rcPaint.bottom = rectUpdate.bottom;
    if (rcPaint.IsEmpty()) {
        //无需绘制
        return false;
    }

    sk_sp<SkSurface> backbuffer = getBackbufferSurface();
    ASSERT(backbuffer != nullptr);
    if (backbuffer == nullptr) {
        return false;
    }

    //开始绘制
    PAINTSTRUCT ps = { 0, };
    ::BeginPaint(hWnd, &ps);
    ::EndPaint(hWnd, &ps);

    //执行绘制
    pRenderPaint->DoPaint(rcPaint);

    //标记绘制区域为有效区域
    ::ValidateRect(hWnd, nullptr);

    //提交绘制结果，并刷新到设备DC
    //这里使用异步提交(kNo)：SwapBuffers本身会与显示同步，不需要每帧等待CPU完成绘制，
    //否则会阻塞UI线程、显著降低帧率；需要在GPU绘制完成后读回像素的场景，由调用方显式同步
    if (auto dContext = this->directContext()) {
        dContext->flushAndSubmit(backbuffer.get(), GrSyncCpu::kNo);
    }

    //绘制完成后，更新到窗口
    swapBuffers();
    return true;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN && !_M_ARM64
