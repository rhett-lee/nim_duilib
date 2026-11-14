//
// GL 多窗口冒烟测试（Windows 平台）
//
// 覆盖设计文档 docs/misc/kNativeGL渲染后端实现分析与改进方案.md 中阶段4的回归点：
//  GS-01 ~ GS-02  创建两个真实窗口的GL渲染对象（同一线程，两个OpenGL上下文）
//  GS-03 ~ GS-05  交替绘制（验证每次绘制前切换到本窗口的上下文）
//  GS-06 ~ GS-08  窗口缩放（重建持久化表面与窗口后台缓冲表面）
//  GS-09 ~ GS-12  运行过程中切换后端（GL -> CPU -> GL，模拟 Window::SetRenderBackendType）
//  GS-13 ~ GS-16  两个GL窗口存活时，离屏GPU渲染复用窗口上下文
//
// 说明：
//  1. 本测试会创建真实窗口（位于屏幕外、不激活），因此仅在 Windows 平台构建；
//  2. 运行环境不支持OpenGL（远程桌面、虚拟机、缺少驱动、ARM64等）时，
//     工厂会把GL渲染降级为CPU渲染，此时打印 SKIP 并返回成功，保证可自动执行。

#if defined(_WIN32)

#include <crtdbg.h>
#include <windows.h>

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"

#include <cstdio>
#include <iostream>
#include <memory>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;
int32_t g_skipCount = 0;

void Check(bool condition, const char* message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message << std::endl;
    }
}
#define CHECK(expr) Check((expr), #expr)

void Skip(const char* message)
{
    ++g_skipCount;
    std::cout << "[SKIP] " << message << std::endl;
}

const uint32_t kBackgroundColor = 0xFF204060u;
const uint32_t kForegroundColor = 0xFFFF0000u;
const uint32_t kGreenColor      = 0xFF00FF00u;
const uint32_t kBlueColor       = 0xFF0000FFu;

/** 读取IBitmap中单个像素的颜色值 */
bool ReadBitmapPixel(ui::IBitmap* pBitmap, int32_t x, int32_t y, uint32_t& color)
{
    color = 0;
    if ((pBitmap == nullptr) || !pBitmap->IsValid()) {
        return false;
    }
    if ((x < 0) || (y < 0) || (x >= (int32_t)pBitmap->GetWidth()) || (y >= (int32_t)pBitmap->GetHeight())) {
        return false;
    }
    const void* pBits = pBitmap->LockPixelBits();
    if (pBits == nullptr) {
        return false;
    }
    const uint32_t* pLine = (const uint32_t*)((const uint8_t*)pBits + (size_t)y * pBitmap->GetRowBytes());
    color = pLine[x];
    pBitmap->UnLockPixelBits();
    return true;
}

/** 创建测试窗口（无边框，客户区大小等于传入的宽高）
* @note 窗口必须显示（放置在屏幕外并激活显示）：隐藏窗口不会产生更新区域，
*       无法触发GL后端的局部重绘路径
*/
HWND CreateTestWindow(int32_t nWidth, int32_t nHeight)
{
    const TCHAR* kClassName = _T("NimDuilibGLSmokeTestWindow");
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
    //WS_POPUP没有非客户区，窗口大小即为客户区大小
    HWND hWnd = ::CreateWindowEx(0, kClassName, _T(""), WS_POPUP,
                                 -32000, -32000, nWidth, nHeight, nullptr, nullptr, hInstance, nullptr);
    if (hWnd != nullptr) {
        ::ShowWindow(hWnd, SW_SHOWNOACTIVATE);
    }
    return hWnd;
}

/** 绘制回调：在渲染对象上绘制简单内容，并统计绘制次数 */
class SmokePaint : public ui::IRenderPaint
{
public:
    explicit SmokePaint(ui::IRender* pRender): m_pRender(pRender), m_nPaintCount(0)
    {
    }
    virtual ~SmokePaint() override = default;

    /** 更新关联的渲染对象（渲染对象被重建后必须调用，否则会访问已销毁的对象）
    */
    void SetRender(ui::IRender* pRender)
    {
        m_pRender = pRender;
    }

    virtual bool DoPaint(const ui::UiRect& rcPaint) override
    {
        ++m_nPaintCount;
        m_rcLastPaint = rcPaint;
        if (m_pRender == nullptr) {
            return false;
        }
        m_pRender->Clear(ui::UiColor(kBackgroundColor));
        const float fLeft = (float)rcPaint.left + 4.0f;
        const float fTop = (float)rcPaint.top + 4.0f;
        m_pRender->FillRect(ui::UiRectF(fLeft, fTop, fLeft + 16.0f, fTop + 16.0f), ui::UiColor(kForegroundColor));
        return true;
    }

    virtual uint8_t GetLayeredWindowAlpha() override
    {
        return 255;
    }

    virtual bool GetUpdateRect(ui::UiRect& /*rcUpdate*/) const override
    {
        //本测试不使用窗口的局部绘制回调（由GL实现自己查询更新区域）
        return false;
    }

    int32_t GetPaintCount() const { return m_nPaintCount; }
    const ui::UiRect& GetLastPaintRect() const { return m_rcLastPaint; }

private:
    ui::IRender* m_pRender;
    int32_t m_nPaintCount;
    ui::UiRect m_rcLastPaint;
};

/** 请求一次绘制并检查结果 */
bool PaintOnce(ui::IRenderWindow* pRender, SmokePaint& painter, HWND hWnd)
{
    if ((pRender == nullptr) || (hWnd == nullptr)) {
        return false;
    }
    //GL实现的持久化表面只重绘更新区域，因此每次绘制前都要产生一个更新区域
    ::InvalidateRect(hWnd, nullptr, FALSE);
    return pRender->PaintAndSwapBuffers(&painter);
}

// ---------------------------------------------------------------------------
// GS-01 ~ GS-16：多窗口、交替绘制、缩放、运行中切换、离屏复用
// ---------------------------------------------------------------------------
void TestGLMultiWindow(ui::IRenderFactory* pRenderFactory)
{
    HWND hWndA = CreateTestWindow(160, 120);
    HWND hWndB = CreateTestWindow(200, 100);
    CHECK(hWndA != nullptr);
    CHECK(hWndB != nullptr);
    if ((hWndA == nullptr) || (hWndB == nullptr)) {
        if (hWndA != nullptr) {
            ::DestroyWindow(hWndA);
        }
        if (hWndB != nullptr) {
            ::DestroyWindow(hWndB);
        }
        return;
    }

    //GS-01/GS-02：同一线程内创建两个窗口的GL渲染对象
    std::unique_ptr<ui::IRenderWindow> pRenderA(pRenderFactory->CreateWindowRender(
        nullptr, hWndA, ui::RenderBackendType::kNativeGL_BackendType, 0, true));
    std::unique_ptr<ui::IRenderWindow> pRenderB(pRenderFactory->CreateWindowRender(
        nullptr, hWndB, ui::RenderBackendType::kNativeGL_BackendType, 0, true));
    CHECK(pRenderA != nullptr);
    CHECK(pRenderB != nullptr);

    if ((pRenderA == nullptr) || (pRenderB == nullptr) ||
        (pRenderA->GetRenderBackendType() != ui::RenderBackendType::kNativeGL_BackendType) ||
        (pRenderB->GetRenderBackendType() != ui::RenderBackendType::kNativeGL_BackendType)) {
        //GL不可用时工厂会降级为CPU渲染：打印SKIP，保证测试在无OpenGL环境可自动执行
        Skip("GL window render is not available (remote desktop / no OpenGL driver / ARM64)");
        pRenderA.reset();
        pRenderB.reset();
        ::DestroyWindow(hWndA);
        ::DestroyWindow(hWndB);
        return;
    }

    SmokePaint painterA(pRenderA.get());
    SmokePaint painterB(pRenderB.get());

    //GS-03 ~ GS-05：交替绘制，验证每次绘制前切换到本窗口的上下文
    for (int32_t i = 0; i < 3; ++i) {
        CHECK(PaintOnce(pRenderA.get(), painterA, hWndA));
        CHECK(PaintOnce(pRenderB.get(), painterB, hWndB));
    }
    CHECK(painterA.GetPaintCount() == 3);
    CHECK(painterB.GetPaintCount() == 3);
    CHECK(!painterA.GetLastPaintRect().IsEmpty());

    //GS-06 ~ GS-08：窗口缩放后重建绘制表面，并继续正常绘制
    ::SetWindowPos(hWndA, nullptr, 0, 0, 180, 140, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    CHECK(pRenderA->Resize(180, 140));
    CHECK(pRenderA->GetWidth() == 180);
    CHECK(pRenderA->GetHeight() == 140);
    CHECK(PaintOnce(pRenderA.get(), painterA, hWndA));
    //缩放后另一窗口仍能正常绘制（上下文切换未被破坏）
    CHECK(PaintOnce(pRenderB.get(), painterB, hWndB));

    //GS-09 ~ GS-12：运行过程中切换后端（GL -> CPU -> GL），模拟 Window::SetRenderBackendType
    pRenderA.reset();
    {
        std::unique_ptr<ui::IRenderWindow> pCpuRender(pRenderFactory->CreateWindowRender(
            nullptr, hWndA, ui::RenderBackendType::kRaster_BackendType));
        CHECK(pCpuRender != nullptr);
        if (pCpuRender != nullptr) {
            CHECK(pCpuRender->GetRenderBackendType() == ui::RenderBackendType::kRaster_BackendType);
            SmokePaint cpuPainter(pCpuRender.get());
            CHECK(PaintOnce(pCpuRender.get(), cpuPainter, hWndA));
            CHECK(cpuPainter.GetPaintCount() == 1);
        }
    }
    pRenderA.reset(pRenderFactory->CreateWindowRender(
        nullptr, hWndA, ui::RenderBackendType::kNativeGL_BackendType, 2, true));
    CHECK(pRenderA != nullptr);
    if (pRenderA != nullptr) {
        CHECK(pRenderA->GetRenderBackendType() == ui::RenderBackendType::kNativeGL_BackendType);
        //渲染对象已重建，必须更新绘制回调中保存的指针
        painterA.SetRender(pRenderA.get());
        CHECK(PaintOnce(pRenderA.get(), painterA, hWndA));
    }

    //GS-13 ~ GS-16：两个GL窗口存活时，离屏GPU渲染复用窗口的GL上下文
    {
        std::unique_ptr<ui::IRender> pOffscreen(pRenderFactory->CreateOffscreenRender(
            64, 64, nullptr, ui::RenderBackendType::kNativeGL_BackendType));
        CHECK(pOffscreen != nullptr);
        if (pOffscreen != nullptr) {
            CHECK(pOffscreen->GetRenderBackendType() == ui::RenderBackendType::kNativeGL_BackendType);
            pOffscreen->Clear(ui::UiColor(kGreenColor));
            pOffscreen->FillRect(ui::UiRectF(16.0f, 16.0f, 48.0f, 48.0f), ui::UiColor(kBlueColor));

            std::unique_ptr<ui::IBitmap> pBitmap(pOffscreen->MakeImageSnapshot());
            CHECK(pBitmap != nullptr);
            if (pBitmap != nullptr) {
                uint32_t nColor = 0;
                CHECK(ReadBitmapPixel(pBitmap.get(), 32, 32, nColor) && (nColor == kBlueColor));
                CHECK(ReadBitmapPixel(pBitmap.get(), 0, 0, nColor) && (nColor == kGreenColor));
            }
        }
        //离屏渲染结束后，窗口渲染仍可继续绘制
        CHECK(PaintOnce(pRenderA.get(), painterA, hWndA));
        CHECK(PaintOnce(pRenderB.get(), painterB, hWndB));
    }

    //按"先销毁渲染对象、后销毁窗口"的顺序清理（与窗口的生命周期约束一致）
    pRenderA.reset();
    pRenderB.reset();
    ::DestroyWindow(hWndA);
    ::DestroyWindow(hWndB);
}

}//namespace

int main(int /*argc*/, char* /*argv*/[])
{
    //关闭输出缓冲，便于程序异常时也能看到已产生的输出
    ::setvbuf(stdout, nullptr, _IONBF, 0);
#if defined(_DEBUG)
    //把断言输出重定向到标准错误，避免在无界面环境下弹出断言对话框导致进程挂起
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    const DString resourcePath = _T(DUILIB_TEST_RESOURCE_DIR);
    if (!ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(ui::FilePath(resourcePath)))) {
        std::cerr << "[FAIL] GlobalManager::Startup failed" << std::endl;
        ++g_failedCount;
    }
    else {
        struct GlobalShutdown
        {
            ~GlobalShutdown() { ui::GlobalManager::Instance().Shutdown(); }
        } shutdownGuard;

        ui::IRenderFactory* pRenderFactory = ui::GlobalManager::Instance().GetRenderFactory();
        Check(pRenderFactory != nullptr, "GetRenderFactory() is null");
        if (pRenderFactory != nullptr) {
            TestGLMultiWindow(pRenderFactory);
        }
    }

    std::cout << "gl_window_smoke: " << g_checkCount << " checks, "
              << g_failedCount << " failed, " << g_skipCount << " skipped" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}

#else

#include <iostream>

int main()
{
    std::cout << "[SKIP] gl_window_smoke: only supported on Windows" << std::endl;
    return 0;
}

#endif
