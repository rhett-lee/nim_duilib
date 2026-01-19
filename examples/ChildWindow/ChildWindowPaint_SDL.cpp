#include "ChildWindowPaint.h"

#ifdef DUILIB_BUILD_FOR_SDL

#include "SDL3/SDL.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

// 定义图形类型枚举
typedef enum {
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_LINE,
    SHAPE_COUNT
} ShapeType;

/** 绘制圆形
 * @param renderer SDL渲染器指针
 * @param center 圆心坐标
 * @param radius 圆半径
 * @param fill 是否填充（1=填充，0=描边）
 */
static void RenderCircle(SDL_Renderer* renderer, const SDL_FPoint* center, float radius, int fill)
{
    const int segments = 36; // 圆形分段数，越多越平滑
    SDL_FPoint vertices[segments + 1];

    // 生成圆形顶点
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159265f * (float)i / (float)segments;
        vertices[i].x = center->x + radius * cosf(angle);
        vertices[i].y = center->y + radius * sinf(angle);
    }

    if (fill) {
        // 填充圆形：绘制三角形扇
        for (int i = 0; i < segments; i++) {
            SDL_FPoint tri[3] = { *center, vertices[i], vertices[i + 1] };
            SDL_RenderLines(renderer, tri, 3);
        }
    }
    else {
        // 描边圆形：绘制线条
        SDL_RenderLines(renderer, vertices, segments);
    }
}

/** 随机图案绘制函数
 * @param renderer SDL渲染器指针
 * @param windowWidth 窗口宽度
 * @param windowHeight 窗口高度
 * @param shapeCount 要生成的图形数量
 */
static void DrawRandomPatterns(SDL_Renderer* renderer, int windowWidth, int windowHeight, int shapeCount) {
    // 初始化随机数生成器（确保每次运行结果不同）
    static int randomInitialized = 0;
    if (!randomInitialized) {
        srand((unsigned int)time(NULL));
        randomInitialized = 1;
    }

    // 设置渲染器绘制颜色为黑色清屏
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 循环绘制随机图形
    for (int i = 0; i < shapeCount; i++) {
        // 1. 生成随机颜色 (RGBA)
        Uint8 r = (Uint8)(rand() % 256);
        Uint8 g = (Uint8)(rand() % 256);
        Uint8 b = (Uint8)(rand() % 256);
        Uint8 a = (Uint8)(180 + rand() % 76); // 透明度 180-255，保证可见性
        SDL_SetRenderDrawColor(renderer, r, g, b, a);

        // 2. 随机选择图形类型
        ShapeType shape = (ShapeType)(rand() % SHAPE_COUNT);

        // 3. 随机线条宽度 (1-10像素) - SDL3.4接口调整
        int lineWidth = 1 + rand() % 10;

        // 4. 根据图形类型绘制
        switch (shape) {
        case SHAPE_RECTANGLE: {
            // 随机矩形位置和尺寸
            int x = rand() % windowWidth;
            int y = rand() % windowHeight;
            int w = 10 + rand() % 100;  // 宽度 10-109
            int h = 10 + rand() % 100;  // 高度 10-109

            // 确保图形在窗口内
            if (x + w > windowWidth) w = windowWidth - x;
            if (y + h > windowHeight) h = windowHeight - y;

            SDL_FRect rect = { (float)x, (float)y, (float)w, (float)h };

            // 随机选择填充或描边
            if (rand() % 2 == 0) {
                SDL_RenderFillRect(renderer, &rect);  // 填充矩形（SDL3.4仍支持）
            }
            else {
                SDL_RenderRect(renderer, &rect); // 描边矩形（SDL3.4仍支持）
            }
            break;
        }

        case SHAPE_CIRCLE: {
            // 随机圆形位置和半径
            int centerX = rand() % windowWidth;
            int centerY = rand() % windowHeight;
            int radius = 5 + rand() % 50;  // 半径 5-54

            // 确保圆形在窗口内
            if (centerX - radius < 0) centerX = radius;
            if (centerX + radius > windowWidth) centerX = windowWidth - radius;
            if (centerY - radius < 0) centerY = radius;
            if (centerY + radius > windowHeight) centerY = windowHeight - radius;

            SDL_FPoint center = { (float)centerX, (float)centerY };

            // 调用适配的圆形绘制函数
            RenderCircle(renderer, &center, (float)radius, rand() % 2);
            break;
        }

        case SHAPE_LINE: {
            // 随机线条起点和终点
            SDL_FPoint start = { (float)(rand() % windowWidth), (float)(rand() % windowHeight) };
            SDL_FPoint end = { (float)(rand() % windowWidth), (float)(rand() % windowHeight) };
            SDL_FPoint points[2] = { start , end };
            SDL_RenderLines(renderer, points, 2);  // 绘制线条
            break;
        }

        default:
            break;
        }
    }

    // 更新屏幕显示
    SDL_RenderPresent(renderer);
}

struct ChildWindowPaint::TImpl
{
};

ChildWindowPaint::ChildWindowPaint(ui::ChildWindow* pChildWindow) :
    m_pChildWindow(pChildWindow),
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

void ChildWindowPaint::PaintChildWindow(const ui::UiRect& rcPaint, const ui::NativeMsg& nativeMsg, bool bPaintFps)
{
    ASSERT(nativeMsg.uMsg == SDL_EVENT_WINDOW_EXPOSED);
    if (nativeMsg.uMsg != SDL_EVENT_WINDOW_EXPOSED) {
        return;
    }
    SDL_Window* sdlWindow = (SDL_Window*)nativeMsg.wParam;
    ASSERT(sdlWindow != nullptr);
    if (sdlWindow == nullptr) {
        return;
    }
    SDL_Renderer* renderer = SDL_GetRenderer(sdlWindow);
    if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(sdlWindow, nullptr);
    }
    ASSERT(renderer != nullptr);
    if (renderer != nullptr) {
        if (bPaintFps && (m_pChildWindow != nullptr)) {
            ui::UiRect childWndRect;
            m_pChildWindow->GetChildWindowRect(childWndRect);
            DrawRandomPatterns(renderer, childWndRect.Width(), childWndRect.Height(), 60);
        }
        else {
            //显示纯色
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 设置蓝色
            SDL_RenderClear(renderer);   // 清屏
            SDL_RenderPresent(renderer); // 显示
        }
    }
}

#endif
