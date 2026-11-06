#include "ChildWindowPaint.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "SDL3/SDL.h"

#include <math.h>
#include <vector>

namespace
{

/** 颜色定义（RGB）*/
struct PaintColor
{
    Uint8 r;
    Uint8 g;
    Uint8 b;
};

/** 计算两个颜色之间的插值（t: 0~1）*/
PaintColor LerpColor(const PaintColor& from, const PaintColor& to, double t)
{
    t = (t < 0.0) ? 0.0 : ((t > 1.0) ? 1.0 : t);
    PaintColor color;
    color.r = (Uint8)(from.r + (to.r - from.r) * t);
    color.g = (Uint8)(from.g + (to.g - from.g) * t);
    color.b = (Uint8)(from.b + (to.b - from.b) * t);
    return color;
}

/** 颜色变暗（用于模拟辉光的外层线条）*/
PaintColor DarkenColor(const PaintColor& color, double ratio)
{
    ratio = (ratio < 0.0) ? 0.0 : ((ratio > 1.0) ? 1.0 : ratio);
    PaintColor result;
    result.r = (Uint8)(color.r * ratio);
    result.g = (Uint8)(color.g * ratio);
    result.b = (Uint8)(color.b * ratio);
    return result;
}

/** 波形的定义 */
struct WaveDesc
{
    PaintColor color;       //线条颜色
    double amplitudeRatio;  //振幅占窗口高度的比例
    double frequency;       //窗口宽度内的周期数
    double phaseOffset;     //相位偏移（弧度）
};

/** 演示画面中绘制的波形 */
const WaveDesc g_waveList[] =
{
    { { 0x00, 0x78, 0xD4 }, 0.18, 1.2, 0.0 },
    { { 0x40, 0x9C, 0xFF }, 0.26, 1.8, 1.1 },
    { { 0x00, 0xB7, 0xC3 }, 0.32, 2.4, 2.2 },
    { { 0x8B, 0x7C, 0xFF }, 0.38, 3.0, 3.3 }
};

/** 背景渐变的起止颜色 */
const PaintColor g_bgTopColor = { 0x0D, 0x12, 0x1A };
const PaintColor g_bgBottomColor = { 0x16, 0x1E, 0x2A };

/** 背景网格线的颜色 */
const PaintColor g_gridColor = { 0x1A, 0x24, 0x32 };

/** 绘制虚线函数：绘制一段线段 */
void DrawLine(SDL_Renderer* renderer, float x1, float y1, float x2, float y2)
{
    SDL_FPoint points[2] = { { x1, y1 }, { x2, y2 } };
    SDL_RenderLines(renderer, points, 2);
}

/** 绘制演示画面（深色渐变背景 + 网格 + 流动波形）
 * @param renderer SDL渲染器
 * @param width 绘制区域宽度
 * @param height 绘制区域高度
 * @param phase 动画相位（弧度），静态绘制时传入0
 */
void PaintScene(SDL_Renderer* renderer, int width, int height, double phase)
{
    if ((renderer == nullptr) || (width <= 0) || (height <= 0)) {
        return;
    }

    //波形使用半透明颜色叠加，需要开启混合模式
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    //1. 垂直渐变背景
    const int nGradientBands = 64;
    for (int32_t i = 0; i < nGradientBands; ++i) {
        double t = (double)i / (double)(nGradientBands - 1);
        PaintColor color = LerpColor(g_bgTopColor, g_bgBottomColor, t);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_FRect rcBand = { 0.0f, (float)((height * i) / nGradientBands),
                              (float)width, (float)((height * (i + 1)) / nGradientBands - (height * i) / nGradientBands) };
        SDL_RenderFillRect(renderer, &rcBand);
    }

    //2. 淡色网格线
    SDL_SetRenderDrawColor(renderer, g_gridColor.r, g_gridColor.g, g_gridColor.b, 255);
    const int32_t nGridSize = 32;
    for (int32_t x = nGridSize; x < width; x += nGridSize) {
        DrawLine(renderer, (float)x, 0.0f, (float)x, (float)height);
    }
    for (int32_t y = nGridSize; y < height; y += nGridSize) {
        DrawLine(renderer, 0.0f, (float)y, (float)width, (float)y);
    }

    //3. 流动波形
    const double centerY = height / 2.0;
    const double twoPi = 2.0 * 3.14159265358979323846;
    std::vector<SDL_FPoint> points;
    for (const WaveDesc& wave : g_waveList) {
        points.clear();
        points.reserve(static_cast<size_t>(width / 3) + 2);
        for (int32_t x = 0; x <= width; x += 3) {
            double angle = (wave.frequency * twoPi * (double)x / (double)width) + phase + wave.phaseOffset;
            float y = (float)(centerY + wave.amplitudeRatio * (double)height * sin(angle));
            SDL_FPoint pt = { (float)x, y };
            points.push_back(pt);
        }
        if (points.size() < 2) {
            continue;
        }

        //外层：暗色线条，模拟辉光
        PaintColor glowColor = DarkenColor(wave.color, 0.4);
        SDL_SetRenderDrawColor(renderer, glowColor.r, glowColor.g, glowColor.b, 220);
        std::vector<SDL_FPoint> glowPoints = points;
        for (SDL_FPoint& pt : glowPoints) {
            pt.y += 1.0f;
        }
        SDL_RenderLines(renderer, glowPoints.data(), (int)glowPoints.size());

        //内层：主色线条
        SDL_SetRenderDrawColor(renderer, wave.color.r, wave.color.g, wave.color.b, 235);
        SDL_RenderLines(renderer, points.data(), (int)points.size());
    }
}

}//namespace

struct ChildWindowPaint::TImpl
{
};

ChildWindowPaint::ChildWindowPaint(ui::ChildWindow* pChildWindow, size_t nChildWindowIndex) :
    m_pChildWindow(pChildWindow),
    m_nChildWindowIndex(nChildWindowIndex),
    m_startTime(std::chrono::steady_clock::now()),
    m_impl(nullptr)
{
}

ChildWindowPaint::~ChildWindowPaint()
{
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

bool ChildWindowPaint::PaintChildWindow(const ui::UiRect& rcPaint, const ui::NativeMsg& nativeMsg, bool bPaintFps)
{
    (void)rcPaint;
    ASSERT(nativeMsg.uMsg == SDL_EVENT_WINDOW_EXPOSED);
    if (nativeMsg.uMsg != SDL_EVENT_WINDOW_EXPOSED) {
        return false;
    }
    SDL_Window* sdlWindow = (SDL_Window*)nativeMsg.wParam;
    ASSERT(sdlWindow != nullptr);
    if (sdlWindow == nullptr) {
        return false;
    }
    SDL_Renderer* renderer = SDL_GetRenderer(sdlWindow);
    if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(sdlWindow, nullptr);
    }

#ifdef DUILIB_DLL
    //DLL模式下，子窗口SDL无法支持绘制, 因为SDL库不支持跨模块使用，其内使用了大量的static变量。
    if (renderer == nullptr) {
        return false;
    }
#endif

    ASSERT(renderer != nullptr);
    if (renderer == nullptr) {
        return false;
    }
    if (m_impl == nullptr) {
        m_impl = new TImpl;
    }

    //动态绘制时按时间计算动画相位，静态绘制时固定显示相位0的静止帧
    double phase = 0.0;
    if (bPaintFps) {
        std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - m_startTime;
        phase = elapsed.count() * 0.9 + (double)m_nChildWindowIndex * 0.6;
    }

    int32_t width = 0;
    int32_t height = 0;
    if (m_pChildWindow != nullptr) {
        ui::UiRect childWndRect;
        m_pChildWindow->GetChildWindowRect(childWndRect);
        width = childWndRect.Width();
        height = childWndRect.Height();
    }
    if ((width <= 0) || (height <= 0)) {
        SDL_GetWindowSizeInPixels(sdlWindow, &width, &height);
    }
    if ((width <= 0) || (height <= 0)) {
        return false;
    }
    PaintScene(renderer, width, height, phase);
    SDL_RenderPresent(renderer);
    return true;
}

#endif
