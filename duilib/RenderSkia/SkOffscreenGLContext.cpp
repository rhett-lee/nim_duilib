#include "SkOffscreenGLContext.h"

#if defined(DUILIB_BUILD_FOR_WIN) && !defined(_M_ARM64)

#include "duilib/duilib_config_windows.h"
#include "duilib/RenderSkia/SkGLWindowContext_Windows.h"

#include "SkiaHeaderBegin.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "tools/window/WindowContext.h"

#include "SkiaHeaderEnd.h"

#include <memory>

namespace ui {

/** 创建GL实现的WindowContext（定义在Render_Skia_Windows.cpp中）
*/
std::unique_ptr<skwindow::WindowContext> MakeGLForWin(HWND hWnd, std::unique_ptr<const skwindow::DisplayParams> params);

namespace {

/** 当前线程的窗口GL上下文（存在时优先复用）
*/
thread_local skwindow::WindowContext* g_pWindowContext = nullptr;

/** 后台GL上下文（隐藏窗口 + GL上下文）
*/
struct BackgroundGLContext
{
    /** 隐藏窗口句柄
    */
    HWND m_hWnd = nullptr;

    /** GL上下文
    */
    std::unique_ptr<skwindow::WindowContext> m_pWindowContext;

    ~BackgroundGLContext()
    {
        //先释放GL上下文，再销毁承载它的隐藏窗口
        m_pWindowContext.reset();
        if (m_hWnd != nullptr) {
            ::DestroyWindow(m_hWnd);
            m_hWnd = nullptr;
        }
    }
};

/** 当前线程的后台GL上下文
*/
thread_local std::unique_ptr<BackgroundGLContext> g_pBackgroundContext;

/** 是否已经尝试创建过后台GL上下文
*/
thread_local bool g_bBackgroundTried = false;

/** 创建用于承载GL上下文的隐藏窗口
*/
HWND CreateHiddenWindow()
{
    const TCHAR* kClassName = _T("NimDuilibOffscreenGLWindow");
    HINSTANCE hInstance = ::GetModuleHandle(nullptr);
    if (hInstance == nullptr) {
        return nullptr;
    }
    WNDCLASSEX wc = { 0, };
    if (!::GetClassInfoEx(hInstance, kClassName, &wc)) {
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = ::DefWindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = kClassName;
        if (!::RegisterClassEx(&wc)) {
            return nullptr;
        }
    }
    return ::CreateWindowEx(0, kClassName, _T(""), WS_POPUP,
                            0, 0, 1, 1, nullptr, nullptr, hInstance, nullptr);
}

/** 获取当前线程的后台GL上下文（惰性创建）
*/
skwindow::WindowContext* GetBackgroundContext()
{
    if (g_pBackgroundContext != nullptr) {
        return g_pBackgroundContext->m_pWindowContext.get();
    }
    if (g_bBackgroundTried) {
        return nullptr;
    }
    g_bBackgroundTried = true;

    HWND hWnd = CreateHiddenWindow();
    if (hWnd == nullptr) {
        return nullptr;
    }
    std::unique_ptr<skwindow::WindowContext> pWindowContext =
        MakeGLForWin(hWnd, std::make_unique<skwindow::DisplayParams>());
    if (pWindowContext == nullptr) {
        ::DestroyWindow(hWnd);
        return nullptr;
    }
    auto pBackground = std::make_unique<BackgroundGLContext>();
    pBackground->m_hWnd = hWnd;
    pBackground->m_pWindowContext = std::move(pWindowContext);
    g_pBackgroundContext = std::move(pBackground);
    return g_pBackgroundContext->m_pWindowContext.get();
}

} // namespace

bool OffscreenGLContext::IsSupported()
{
    return GetDirectContext() != nullptr;
}

GrDirectContext* OffscreenGLContext::GetDirectContext()
{
    if (g_pWindowContext != nullptr) {
        //复用当前线程已存在的窗口GL上下文
        return g_pWindowContext->directContext();
    }
    skwindow::WindowContext* pBackgroundContext = GetBackgroundContext();
    return (pBackgroundContext != nullptr) ? pBackgroundContext->directContext() : nullptr;
}

void OffscreenGLContext::RegisterWindowContext(skwindow::WindowContext* pContext)
{
    g_pWindowContext = pContext;
}

void OffscreenGLContext::UnregisterWindowContext(skwindow::WindowContext* pContext)
{
    if (g_pWindowContext == pContext) {
        g_pWindowContext = nullptr;
    }
}

} // namespace ui

#else

namespace ui {

bool OffscreenGLContext::IsSupported()
{
    return false;
}

GrDirectContext* OffscreenGLContext::GetDirectContext()
{
    return nullptr;
}

void OffscreenGLContext::RegisterWindowContext(skwindow::WindowContext* /*pContext*/)
{
}

void OffscreenGLContext::UnregisterWindowContext(skwindow::WindowContext* /*pContext*/)
{
}

} // namespace ui

#endif
