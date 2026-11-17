//
// GL vs Raster 渲染后端性能基准（Windows 平台）
//
// 设计：
//   同一窗口实例上，先运行 Raster 绘制 N 帧，再切换为 GL 绘制 N 帧，
//   每种后端先 warmup 30 帧，再记录 wall-clock 耗时、CPU 时间、以及每帧耗时分布。
//   窗口只创建一次（保持变量单一），退出前切回 Raster 再销毁。
//
// 用法：直接运行；或 benchmark.exe frames=200 size=1280x800 workload=heavy

#if defined(_WIN32)

#include <crtdbg.h>
#include <windows.h>
#include <Psapi.h>

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/WindowCreateParam.h"
#include "duilib/Render/IRender.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "psapi.lib")

using namespace ui;

namespace {

struct Config {
    int32_t frames = 200;
    int32_t width = 960;
    int32_t height = 600;
    enum Workload { kLight, kMedium, kHeavy, kHeavyImage } workload = kMedium;
};

Config g_cfg;

void ParseArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        const char* s = argv[i];
        if (!std::strncmp(s, "frames=", 7))
            g_cfg.frames = std::max(10, (int32_t)std::strtol(s + 7, nullptr, 10));
        else if (!std::strncmp(s, "size=", 5)) {
            const char* p = s + 5;
            char* end = nullptr;
            long w = std::strtol(p, &end, 10);
            if (end && *end == 'x') {
                long h = std::strtol(end + 1, nullptr, 10);
                if (w > 0 && h > 0) { g_cfg.width = (int32_t)w; g_cfg.height = (int32_t)h; }
            }
        }
        else if (!std::strncmp(s, "workload=", 9)) {
            const char* w = s + 9;
            if (!std::strcmp(w, "light")) g_cfg.workload = Config::kLight;
            else if (!std::strcmp(w, "medium")) g_cfg.workload = Config::kMedium;
            else if (!std::strcmp(w, "heavy")) g_cfg.workload = Config::kHeavy;
            else if (!std::strcmp(w, "heavy_image")) g_cfg.workload = Config::kHeavyImage;
        }
    }
}

// ---- 计时 ----
struct LTimer {
    LARGE_INTEGER freq = {};
    LARGE_INTEGER start = {};
    void begin() { QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&start); }
    double now_ms() const {
        LARGE_INTEGER t; QueryPerformanceCounter(&t);
        return (double)(t.QuadPart - start.QuadPart) * 1000.0 / (double)freq.QuadPart;
    }
};

struct FrameTimes {
    std::vector<double> frame_ms;
    double total_ms = 0;
    double user_ms = 0;
    double kernel_ms = 0;
    int32_t backend = 0; // 1 raster 2 GL
    int32_t actual_frames = 0;
    std::string label;

    void Compute() {
        if (frame_ms.empty()) return;
        std::sort(frame_ms.begin(), frame_ms.end());
        actual_frames = (int32_t)frame_ms.size();
        total_ms = std::accumulate(frame_ms.begin(), frame_ms.end(), 0.0);
    }
    double Avg() const { return actual_frames ? total_ms / actual_frames : 0; }
    double Median() const {
        if (!actual_frames) return 0;
        int n = actual_frames;
        return (n & 1) ? frame_ms[n / 2] : (frame_ms[n / 2 - 1] + frame_ms[n / 2]) * 0.5;
    }
    double Pct(double p) const {
        if (!actual_frames) return 0;
        int idx = (int)std::ceil(p * actual_frames / 100.0) - 1;
        if (idx < 0) idx = 0;
        if (idx >= actual_frames) idx = actual_frames - 1;
        return frame_ms[idx];
    }
};

void PrintStats(const FrameTimes& s) {
    std::cout << "---- " << s.label << " (" << (s.backend == 2 ? "GL" : "Raster") << ") ----" << std::endl;
    std::cout << "  frames       : " << s.actual_frames << std::endl;
    std::cout << "  total wall   : " << s.total_ms << " ms  (" << (s.total_ms / 1000.0) << " s)" << std::endl;
    std::cout << std::fixed << "  avg          : " << s.Avg() << " ms  (~"
              << (s.actual_frames * 1000.0 / (s.total_ms > 0 ? s.total_ms : 1))
              << " fps)" << std::endl;
    std::cout << std::fixed << "  median       : " << s.Median() << " ms" << std::endl;
    std::cout << std::fixed << "  p95 / p99    : " << s.Pct(95) << " / " << s.Pct(99) << " ms" << std::endl;
    std::cout << std::fixed << "  min / max    : " << (s.actual_frames ? s.frame_ms.front() : 0)
              << " / " << (s.actual_frames ? s.frame_ms.back() : 0) << " ms" << std::endl;
    std::cout << std::fixed << "  CPU user/krn : " << s.user_ms << " / " << s.kernel_ms << " ms  sum="
              << (s.user_ms + s.kernel_ms) << " ms" << std::endl;
    if (s.frame_ms.size() >= 5) {
        const int bins = 10;
        std::cout << "  histogram (" << bins << " bins over " << std::fixed << s.frame_ms.back() << " ms max):";
        double step = s.frame_ms.back() / bins;
        if (step < 0.01) step = 0.01;
        for (int i = 0; i < bins; ++i) {
            double lo = i * step, hi = (i + 1) * step;
            int cnt = 0;
            for (double v : s.frame_ms) if (v >= lo && v < hi) ++cnt;
            if (i == bins - 1) {
                cnt = 0; for (double v : s.frame_ms) if (v >= lo) ++cnt;
            }
            double bar = (double)cnt / s.actual_frames * 40.0;
            std::cout << "\n    [" << std::fixed << lo << " - " << hi << " ms] "
                      << std::string((int)bar, '#') << " (" << cnt << ")";
        }
        std::cout << std::endl;
    }
}

void PrintCompare(const FrameTimes& r, const FrameTimes& g) {
    std::cout << "\n========== SUMMARY ==========" << std::endl;
    std::cout << "  backend  avg(ms)  median(ms)  p95(ms)  total(ms)  cpu_sum(ms)" << std::endl;
    auto row = [](const FrameTimes& s, const char* name) {
        std::cout << "  " << name
                  << std::setw(7) << " " << std::setw(8) << std::setprecision(2) << s.Avg()
                  << std::setw(10) << std::setprecision(2) << s.Median()
                  << std::setw(8) << std::setprecision(2) << s.Pct(95)
                  << std::setw(9) << std::setprecision(1) << s.total_ms
                  << std::setw(12) << std::setprecision(1) << (s.user_ms + s.kernel_ms)
                  << std::endl;
    };
    row(r, "Raster ");
    row(g, "GL     ");
    if (r.Avg() > 0) {
        double speedup = r.Avg() / g.Avg();
        std::cout << std::fixed << "  GL vs Raster speedup (by avg): " << std::setprecision(2) << speedup << "x   "
                  << (speedup >= 1.0 ? "(GL faster)" : "(GL SLOWER)") << std::endl;
    }
}

// ---- 进程 CPU 时间 ----
void GetProcessTimes_ms(double& user_ms, double& kernel_ms) {
    FILETIME ftCr, ftExit, ftKr, ftUs;
    ::GetProcessTimes(::GetCurrentProcess(), &ftCr, &ftExit, &ftKr, &ftUs);
    auto ft2ms = [](const FILETIME& f) {
        ULARGE_INTEGER u; u.LowPart = f.dwLowDateTime; u.HighPart = f.dwHighDateTime;
        return (double)u.QuadPart / 10000.0;
    };
    user_ms = ft2ms(ftUs); kernel_ms = ft2ms(ftKr);
}

// ---- 绘制负载 ----
class BenchPaint : public IRenderPaint {
public:
    BenchPaint(IRender* pRender, Config::Workload workload):
        m_pRender(pRender), m_workload(workload) {
        // kHeavyImage 模式：预创建一批 IBitmap（48x48，带随机像素），后续每帧 DrawImage 复用
        // 这样 Bitmap_Skia 的 GetSkImage() 懒创建一次后跨帧复用，纹理缓存命中
        if (m_workload == Config::kHeavyImage && m_pRender != nullptr) {
            const int kBmpN = 40;
            constexpr uint32_t kBmpSize = 48;
            // 用 RenderFactory 创建 Bitmap（CreateBitmap 在 IRenderFactory 而非 IRender 上）
            IRenderFactory* pFactory = GlobalManager::Instance().GetRenderFactory();
            if (pFactory == nullptr) {
                return;  // 无法创建 bitmap，跳过
            }
            m_bitmaps.reserve(kBmpN);
            for (int i = 0; i < kBmpN; ++i) {
                std::unique_ptr<IBitmap> bmp(pFactory->CreateBitmap(kBmpSize, kBmpSize, AlphaType::kPremul));
                if (!bmp) break;
                // 填像素（ARGB），构造一个简单图案：底色 + 对角线渐变
                uint32_t* pBits = (uint32_t*)bmp->LockPixelBits();
                if (pBits) {
                    for (uint32_t y = 0; y < kBmpSize; ++y) {
                        for (uint32_t x = 0; x < kBmpSize; ++x) {
                            uint32_t r = (uint32_t)(x * 255 / kBmpSize);
                            uint32_t g = (uint32_t)(y * 255 / kBmpSize);
                            uint32_t b = (uint32_t)((x + y) * 255 / (kBmpSize * 2));
                            uint32_t a = 0xFF;
                            pBits[y * kBmpSize + x] = (a << 24) | (r << 16) | (g << 8) | b;
                        }
                    }
                    bmp->UnLockPixelBits();
                }
                m_bitmaps.push_back(std::move(bmp));
            }
        }
    }
    void SetRender(IRender* p) { m_pRender = p; }
    virtual bool DoPaint(const UiRect& rcPaint) override {
        if (m_pRender == nullptr) return false;
        m_pRender->Clear(UiColor(0xFF223344));

        int rectN = 0, textN = 0, imgN = 0;
        bool bRealImage = false;
        switch (m_workload) {
        case Config::kLight:      rectN = 12;  textN = 12;  break;
        case Config::kMedium:     rectN = 80;  textN = 60;  break;
        case Config::kHeavy:      rectN = 200; textN = 160; imgN = 40; break;
        case Config::kHeavyImage: rectN = 200; textN = 160; bRealImage = true; break;
        }
        float W = (float)rcPaint.Width();
        float H = (float)rcPaint.Height();

        for (int i = 0; i < rectN; ++i) {
            float x = (i * 53.0f + 7.0f) / (rectN + 1) * W;
            float y = (i * 37.0f + 11.0f) / (rectN + 1) * H;
            float w = 20.0f + ((i * 19) % 80);
            float h = 14.0f + ((i * 13) % 60);
            uint32_t c = 0xFF000000u | (((uint32_t)i * 2654435761u) & 0x00FFFFFFu);
            m_pRender->FillRect(UiRectF(x, y, x + w, y + h), UiColor(c));
        }
        for (int i = 0; i < textN; ++i) {
            float x = (i * 71.0f + 3.0f) / (textN + 1) * W;
            float y = (i * 41.0f + 17.0f) / (textN + 1) * H;
            std::wstringstream wss; wss << L"abcdefg-" << std::setw(3) << std::setfill(L'0') << i;
            DString s(wss.str().c_str());
            DrawStringParam dsp;
            dsp.textRect = UiRect((int32_t)x, (int32_t)(y + 2), (int32_t)x + 100, (int32_t)(y + 20));
            dsp.dwTextColor = UiColor(0xFFFFFFFF);
            m_pRender->DrawString(s, dsp);
        }
        if (bRealImage) {
            // 真实 DrawImage：复用预创建的 bitmap 数组，测试纹理缓存效果
            const int n = (int)m_bitmaps.size();
            for (int i = 0; i < n; ++i) {
                float x = (i * 97.0f) / (n + 1) * W;
                float y = (i * 59.0f) / (n + 1) * H;
                m_pRender->DrawImage(m_bitmaps[i].get(), UiRect((int32_t)x, (int32_t)y, (int32_t)x + 48, (int32_t)y + 48));
            }
        }
        else {
            for (int i = 0; i < imgN; ++i) {
                float x = (i * 97.0f) / (imgN + 1) * W;
                float y = (i * 59.0f) / (imgN + 1) * H;
                uint32_t c = 0x80000000u | (((uint32_t)i * 2246822519u) & 0x00FFFFFFu);
                m_pRender->FillRect(UiRectF(x, y, x + 48.0f, y + 48.0f), UiColor(c));
            }
        }
        return true;
    }
    virtual uint8_t GetLayeredWindowAlpha() override { return 255; }
    virtual bool GetUpdateRect(UiRect& /*rc*/) const override { return false; }
private:
    IRender* m_pRender;
    Config::Workload m_workload;
    std::vector<std::unique_ptr<IBitmap>> m_bitmaps;  // kHeavyImage 模式下预创建，跨帧复用
};

// ---- 用 duilib::Window 来创建窗口 ----
class BenchWindow : public Window {
public:
    BenchWindow() = default;
    virtual ~BenchWindow() = default;
};

void PumpMessages() {
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

// ---- 单次基准运行 ----
// 先创建窗口 → 切换到指定后端 → warmup → measure
FrameTimes RunOnce(Window* pWin, RenderBackendType backend, const Config& cfg, const char* label) {
    FrameTimes s;
    s.label = label;
    s.backend = (backend == RenderBackendType::kNativeGL_BackendType) ? 2 : 1;

    if (!pWin->IsWindow()) {
        // 创建窗口（屏幕内，否则 WGL 初始化在某些驱动上会失败）
        int32_t screenW = ::GetSystemMetrics(SM_CXSCREEN);
        int32_t screenH = ::GetSystemMetrics(SM_CYSCREEN);
        int32_t x = std::max(0, (screenW - cfg.width) / 2);
        int32_t y = std::max(0, (screenH - cfg.height) / 2);
        WindowCreateParam createParam(_T("Benchmark"));
        createParam.m_dwExStyle = 0;
        createParam.m_dwStyle = WindowStyle::kWS_POPUP | WindowStyle::kWS_VISIBLE;
        createParam.m_nX = x;
        createParam.m_nY = y;
        createParam.m_nWidth = cfg.width;
        createParam.m_nHeight = cfg.height;
        if (!pWin->CreateWnd(nullptr, createParam)) {
            std::cout << "  [ERROR] CreateWnd failed" << std::endl;
            return s;
        }
        PumpMessages();
    }

    HWND hWnd = (HWND)pWin->GetWindowHandle();
    if (hWnd == nullptr) {
        std::cout << "  [ERROR] GetWindowHandle returned null" << std::endl;
        return s;
    }

    // 设置渲染后端
    pWin->SetRenderBackendType(backend, 0, true);
    PumpMessages();

    // 验证后端是否已设置
    IRenderWindow* pRender = pWin->GetRenderWindow();
    if (pRender == nullptr) {
        std::cout << "  [SKIP] No render window" << std::endl;
        return s;
    }
    const int actual = (int)pRender->GetRenderBackendType();
    std::cout << "  [INFO] requested backend=" << (int)backend
              << ", got actual backend=" << actual << std::endl;
    if ((int)backend != actual) {
        if (backend == RenderBackendType::kNativeGL_BackendType) {
            std::cout << "  [SKIP] GL not available (possibly remote desktop / no OpenGL driver / ARM64)."
                      << std::endl;
        }
        return s;
    }

    BenchPaint painter(pRender, cfg.workload);  // IRenderWindow inherits from IRender

    // warmup 30 帧
    for (int i = 0; i < 30; ++i) {
        ::InvalidateRect(hWnd, nullptr, FALSE);
        PumpMessages();
        pRender->PaintAndSwapBuffers(&painter);
    }
    PumpMessages();

    // 测量
    double user0 = 0, krn0 = 0, user1 = 0, krn1 = 0;
    LTimer frameTimer;
    GetProcessTimes_ms(user0, krn0);
    for (int i = 0; i < cfg.frames; ++i) {
        ::InvalidateRect(hWnd, nullptr, FALSE);
        frameTimer.begin();
        pRender->PaintAndSwapBuffers(&painter);
        double ms = frameTimer.now_ms();
        s.frame_ms.push_back(ms);
        PumpMessages();
    }
    GetProcessTimes_ms(user1, krn1);
    s.user_ms = user1 - user0;
    s.kernel_ms = krn1 - krn0;
    s.Compute();
    return s;
}

} // namespace

int main(int argc, char* argv[]) {
    ::setvbuf(stdout, nullptr, _IONBF, 0);
#if defined(_DEBUG)
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
#endif

    ParseArgs(argc, argv);

    // 初始化 GlobalManager（使用 test_gl_window_smoke 的资源目录）
    const DString resourcePath = _T(DUILIB_TEST_RESOURCE_DIR);
    LocalFilesResParam resParam{FilePath(resourcePath)};
    if (!GlobalManager::Instance().Startup(resParam)) {
        std::cerr << "GlobalManager::Startup failed" << std::endl;
        return 1;
    }

    std::cout << "====================================================" << std::endl;
    std::cout << "  nim_duilib  GL vs Raster  Render Backend Benchmark" << std::endl;
    const char* wlName = "heavy";
    if (g_cfg.workload == Config::kLight) wlName = "light";
    else if (g_cfg.workload == Config::kMedium) wlName = "medium";
    else if (g_cfg.workload == Config::kHeavy) wlName = "heavy";
    else if (g_cfg.workload == Config::kHeavyImage) wlName = "heavy_image";
    std::cout << "====================================================" << std::endl;
    std::cout << "  config : frames=" << g_cfg.frames
              << "  size=" << g_cfg.width << "x" << g_cfg.height
              << "  workload=" << wlName << std::endl;
    std::cout << "====================================================" << std::endl;

    FrameTimes r1, g1;

    // Phase 1: 创建窗口，先测 Raster，再在同一个窗口上切 GL
    {
        std::cout << "\n--- Phase 1: Raster first, then switch to GL ---" << std::endl;
        BenchWindow win;
        r1 = RunOnce(&win, RenderBackendType::kRaster_BackendType, g_cfg, "Raster");
        PrintStats(r1);
        if (r1.frame_ms.size()) {
            g1 = RunOnce(&win, RenderBackendType::kNativeGL_BackendType, g_cfg, "GL");
            PrintStats(g1);
        }
        std::cout << "\n[INFO] Phase 1 done, releasing window..." << std::endl;
        // 销毁 render 前先切回 CPU（让析构更干净）
        win.SetRenderBackendType(RenderBackendType::kRaster_BackendType, 0, true);
        PumpMessages();
        ::Sleep(30);
    }
    std::cout << "[INFO] Window destroyed." << std::endl;

    FrameTimes bestR, bestG;
    bestR = r1;
    bestG = g1;

    if (!bestR.frame_ms.empty() && !bestG.frame_ms.empty()) {
        PrintCompare(bestR, bestG);
    } else {
        std::cout << "\nRaster 或 GL 阶段跳过，无法对比" << std::endl;
    }

    GlobalManager::Instance().Shutdown();
    return 0;
}

#else // _WIN32
#include <iostream>
int main() { std::cout << "benchmark only supports Windows build" << std::endl; return 0; }
#endif
