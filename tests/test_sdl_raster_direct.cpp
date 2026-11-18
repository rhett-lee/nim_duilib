//
// SDL Raster 后端直绘路径测试（Windows/SDL3）
//
// 覆盖优化项4「直绘窗口表面，去掉 memcpy」：
//  RD-01 ~ RD-03  全幅绘制后，绘制结果直接写入SDL窗口表面的像素缓冲（直绘模式生效）
//  RD-04 ~ RD-06  只重新提交后备缓冲（present-only）时不做任何像素拷贝
//  RD-07 ~ RD-09  窗口尺寸变化后重建绘制表面，并重新完整绘制
//  RD-10 ~ RD-12  窗口透明度非255时回退内存缓冲的双缓冲路径，之后可恢复直绘模式
//
// 说明：本测试创建真实SDL窗口（位于屏幕外），仅在SDL配置的Windows构建中注册。
//      运行环境无法创建SDL窗口时打印 SKIP 并返回成功，保证可自动执行。

#include "duilib/duilib_config.h"

#if defined(DUILIB_BUILD_FOR_SDL) && defined(DUILIB_BUILD_FOR_WIN)

#include <SDL3/SDL.h>

#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ResourceParam.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils/FilePath.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

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

//颜色值：0xAARRGGBB
const uint32_t kBackColor   = 0xFF204060u;
const uint32_t kMarkerColor = 0xFFFF0000u;

struct RgbColor
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
};

/** 读取SDL窗口表面上单个像素的颜色（窗口表面必须是32位格式） */
bool ReadSurfacePixel(SDL_Surface* pSurface, int32_t x, int32_t y, RgbColor& color)
{
    if (pSurface == nullptr) {
        return false;
    }
    const SDL_PixelFormatDetails* pFormat = SDL_GetPixelFormatDetails(pSurface->format);
    if ((pFormat == nullptr) || (pFormat->bits_per_pixel != 32)) {
        return false;
    }
    if ((x < 0) || (y < 0) || (x >= pSurface->w) || (y >= pSurface->h)) {
        return false;
    }
    uint32_t value = 0;
    ::memcpy(&value, (const uint8_t*)pSurface->pixels + (size_t)y * pSurface->pitch + (size_t)x * sizeof(uint32_t), sizeof(uint32_t));
    SDL_GetRGBA(value, pFormat, nullptr, &color.r, &color.g, &color.b, &color.a);
    return true;
}

/** 向SDL窗口表面写入单个像素（用于验证直绘模式下没有发生像素拷贝） */
bool WriteSurfacePixel(SDL_Surface* pSurface, int32_t x, int32_t y, uint32_t argb)
{
    if (pSurface == nullptr) {
        return false;
    }
    const SDL_PixelFormatDetails* pFormat = SDL_GetPixelFormatDetails(pSurface->format);
    if ((pFormat == nullptr) || (pFormat->bits_per_pixel != 32)) {
        return false;
    }
    if ((x < 0) || (y < 0) || (x >= pSurface->w) || (y >= pSurface->h)) {
        return false;
    }
    const uint32_t value = SDL_MapRGBA(pFormat, nullptr,
                                       (uint8_t)((argb >> 16) & 0xFF),
                                       (uint8_t)((argb >> 8) & 0xFF),
                                       (uint8_t)(argb & 0xFF),
                                       (uint8_t)((argb >> 24) & 0xFF));
    ::memcpy((uint8_t*)pSurface->pixels + (size_t)y * pSurface->pitch + (size_t)x * sizeof(uint32_t), &value, sizeof(uint32_t));
    return true;
}

/** 判断颜色是否与指定值相同（允许每个通道有少量误差） */
bool ColorNear(const RgbColor& color, uint8_t r, uint8_t g, uint8_t b, int32_t tolerance)
{
    return (std::abs((int32_t)color.r - (int32_t)r) <= tolerance) &&
           (std::abs((int32_t)color.g - (int32_t)g) <= tolerance) &&
           (std::abs((int32_t)color.b - (int32_t)b) <= tolerance);
}

/** 绘制回调：可配置脏区列表、窗口透明度与present-only标志 */
class DirectPaint : public ui::IRenderPaint
{
public:
    explicit DirectPaint(ui::IRender* pRender): m_pRender(pRender)
    {
    }
    virtual ~DirectPaint() override = default;

    virtual bool DoPaint(const ui::UiRect& rcPaint) override
    {
        ++m_nPaintCount;
        m_rcLastPaint = rcPaint;
        if (m_pRender == nullptr) {
            return false;
        }
        m_pRender->ClearRect(rcPaint, ui::UiColor(kBackColor));
        const float fLeft = (float)rcPaint.left + 4.0f;
        const float fTop = (float)rcPaint.top + 4.0f;
        m_pRender->FillRect(ui::UiRectF(fLeft, fTop, fLeft + 20.0f, fTop + 20.0f), ui::UiColor(kMarkerColor));
        return true;
    }

    virtual uint8_t GetLayeredWindowAlpha() override
    {
        return m_nAlpha;
    }

    virtual bool GetUpdateRect(ui::UiRect& rcUpdate) const override
    {
        rcUpdate.Clear();
        for (const ui::UiRect& rcItem : m_updateRects) {
            rcUpdate.Union(rcItem);
        }
        return !rcUpdate.IsEmpty();
    }

    virtual bool GetUpdateRects(std::vector<ui::UiRect>& rcUpdateList) const override
    {
        if (m_updateRects.empty()) {
            return false;
        }
        rcUpdateList = m_updateRects;
        return true;
    }

    virtual bool IsPresentOnly() const override
    {
        return m_bPresentOnly;
    }

    void SetUpdateRects(const std::vector<ui::UiRect>& rcList) { m_updateRects = rcList; }
    void ClearUpdateRects() { m_updateRects.clear(); }
    void SetPresentOnly(bool bPresentOnly) { m_bPresentOnly = bPresentOnly; }
    void SetAlpha(uint8_t nAlpha) { m_nAlpha = nAlpha; }
    int32_t GetPaintCount() const { return m_nPaintCount; }
    const ui::UiRect& GetLastPaintRect() const { return m_rcLastPaint; }

private:
    ui::IRender* m_pRender;
    int32_t m_nPaintCount = 0;
    ui::UiRect m_rcLastPaint;
    std::vector<ui::UiRect> m_updateRects;
    bool m_bPresentOnly = false;
    uint8_t m_nAlpha = 255;
};

/** 进行一次绘制（不产生脏区时按整幅绘制处理） */
bool PaintOnce(ui::IRenderWindow* pRender, DirectPaint& painter)
{
    if (pRender == nullptr) {
        return false;
    }
    return pRender->PaintAndSwapBuffers(&painter);
}

void TestDirectSurface(ui::IRenderFactory* pRenderFactory)
{
    SDL_Window* pWindow = SDL_CreateWindow("nim_duilib_sdl_raster_direct", 160, 120, 0);
    if (pWindow == nullptr) {
        Skip("SDL_CreateWindow failed");
        return;
    }
    //放到屏幕外，避免干扰用户桌面
    SDL_SetWindowPosition(pWindow, -32000, -32000);
    SDL_ShowWindow(pWindow);

    int32_t nWidth = 0;
    int32_t nHeight = 0;
    if (!SDL_GetWindowSizeInPixels(pWindow, &nWidth, &nHeight) || (nWidth <= 0) || (nHeight <= 0)) {
        Skip("SDL_GetWindowSizeInPixels failed");
        SDL_DestroyWindow(pWindow);
        return;
    }

    std::unique_ptr<ui::IRenderWindow> pRender(pRenderFactory->CreateWindowRender(
        nullptr, pWindow, ui::RenderBackendType::kRaster_BackendType));
    if (pRender == nullptr) {
        Skip("CreateWindowRender(kRaster_BackendType) failed");
        SDL_DestroyWindow(pWindow);
        return;
    }
    CHECK(pRender->GetRenderBackendType() == ui::RenderBackendType::kRaster_BackendType);
    CHECK(pRender->Resize(nWidth, nHeight));

    SDL_Surface* pSurface = SDL_GetWindowSurface(pWindow);
    const SDL_PixelFormatDetails* pFormat = (pSurface != nullptr) ? SDL_GetPixelFormatDetails(pSurface->format) : nullptr;
    if ((pSurface == nullptr) || (pFormat == nullptr) || (pFormat->bits_per_pixel != 32) ||
        (pSurface->w != nWidth) || (pSurface->h != nHeight)) {
        //窗口表面尺寸/格式不满足直绘条件：正式环境中会回退到内存缓冲路径，本测试无需继续
        Skip("SDL window surface is not a 32bpp surface matching the window size");
        pRender.reset();
        SDL_DestroyWindow(pWindow);
        return;
    }

    DirectPaint painter(pRender.get());

    //RD-01 ~ RD-03：整幅绘制，绘制结果应直接写入SDL窗口表面的像素
    CHECK(!pRender->IsBackBufferValid());
    CHECK(PaintOnce(pRender.get(), painter));
    CHECK(painter.GetLastPaintRect().Width() == nWidth);
    CHECK(painter.GetLastPaintRect().Height() == nHeight);
    CHECK(pRender->IsBackBufferValid());
    {
        RgbColor background;
        RgbColor marker;
        CHECK(ReadSurfacePixel(pSurface, 1, 1, background));
        CHECK(ReadSurfacePixel(pSurface, 10, 10, marker));
        CHECK(ColorNear(background, 0x20, 0x40, 0x60, 1));
        CHECK(ColorNear(marker, 0xFF, 0x00, 0x00, 1));
    }

    //RD-04 ~ RD-06：只重新提交后备缓冲时，不能把内存缓冲的内容拷贝到窗口表面
    //  （直绘模式下，窗口表面的像素就是后备缓冲，present-only 不会修改像素）
    CHECK(WriteSurfacePixel(pSurface, 1, 1, 0xFF00FF00u));
    {
        const int32_t nPaintCount = painter.GetPaintCount();
        painter.SetPresentOnly(true);
        CHECK(PaintOnce(pRender.get(), painter));
        painter.SetPresentOnly(false);
        RgbColor color;
        CHECK(ReadSurfacePixel(pSurface, 1, 1, color));
        CHECK(ColorNear(color, 0x00, 0xFF, 0x00, 1)); //仍是测试写入的绿色，说明没有发生整幅拷贝
        CHECK(painter.GetPaintCount() == nPaintCount); //未重新绘制控件树
        CHECK(pRender->IsBackBufferValid());
    }

    //局部更新：只有脏区被绘制与提交，其它区域保持不变
    {
        std::vector<ui::UiRect> updateRects;
        updateRects.push_back(ui::UiRect(20, 20, 60, 40));
        painter.SetUpdateRects(updateRects);
        CHECK(PaintOnce(pRender.get(), painter));
        painter.ClearUpdateRects();
        RgbColor inDirty;
        RgbColor outDirty;
        CHECK(ReadSurfacePixel(pSurface, 55, 30, inDirty));
        CHECK(ColorNear(inDirty, 0x20, 0x40, 0x60, 1));
        CHECK(ReadSurfacePixel(pSurface, 1, 1, outDirty));
        CHECK(ColorNear(outDirty, 0x00, 0xFF, 0x00, 1));
        CHECK(pRender->IsBackBufferValid());
    }

    //RD-07 ~ RD-09：窗口尺寸变化后重建表面，需要重新完整绘制
    {
        CHECK(SDL_SetWindowSize(pWindow, 200, 140));
        int32_t nNewWidth = 0;
        int32_t nNewHeight = 0;
        CHECK(SDL_GetWindowSizeInPixels(pWindow, &nNewWidth, &nNewHeight));
        CHECK(pRender->Resize(nNewWidth, nNewHeight));
        CHECK(!pRender->IsBackBufferValid());

        pSurface = SDL_GetWindowSurface(pWindow);
        CHECK(pSurface != nullptr);
        if (pSurface != nullptr) {
            CHECK(PaintOnce(pRender.get(), painter));
            CHECK(pRender->IsBackBufferValid());
            RgbColor background;
            CHECK(ReadSurfacePixel(pSurface, 1, 1, background));
            CHECK(ColorNear(background, 0x20, 0x40, 0x60, 1));
        }
    }

    //RD-10 ~ RD-12：窗口透明度非255时回退内存缓冲路径；恢复255后可重新启用直绘
    if (pSurface != nullptr) {
        painter.SetAlpha(128);
        CHECK(PaintOnce(pRender.get(), painter));
        RgbColor blended;
        CHECK(ReadSurfacePixel(pSurface, 1, 1, blended));
        //内存缓冲路径会按透明度缩放RGB通道（±3的误差用于容忍取整）
        CHECK(ColorNear(blended, 0x10, 0x20, 0x30, 3));

        painter.SetAlpha(255);
        CHECK(PaintOnce(pRender.get(), painter));
        RgbColor restored;
        CHECK(ReadSurfacePixel(pSurface, 1, 1, restored));
        CHECK(ColorNear(restored, 0x20, 0x40, 0x60, 1));

        //再次验证已恢复直绘：写入的像素不会被 present-only 覆盖
        CHECK(WriteSurfacePixel(pSurface, 1, 1, 0xFF00FF00u));
        painter.SetPresentOnly(true);
        CHECK(PaintOnce(pRender.get(), painter));
        painter.SetPresentOnly(false);
        RgbColor green;
        CHECK(ReadSurfacePixel(pSurface, 1, 1, green));
        CHECK(ColorNear(green, 0x00, 0xFF, 0x00, 1));
    }

    //先销毁渲染对象，再销毁窗口
    pRender.reset();
    SDL_DestroyWindow(pWindow);
}

}//namespace

int main(int /*argc*/, char* /*argv*/[])
{
    ::setvbuf(stdout, nullptr, _IONBF, 0);

    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            Skip("SDL_Init(SDL_INIT_VIDEO) failed");
            std::cout << "sdl_raster_direct: 0 checks, 0 failed, " << g_skipCount << " skipped" << std::endl;
            return 0;
        }
    }

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
            TestDirectSurface(pRenderFactory);
        }
    }

    std::cout << "sdl_raster_direct: " << g_checkCount << " checks, "
              << g_failedCount << " failed, " << g_skipCount << " skipped" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}

#else

#include <iostream>

int main()
{
    std::cout << "[SKIP] sdl_raster_direct: only supported on Windows SDL build" << std::endl;
    return 0;
}

#endif
