#include "ChildWindowPaint.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include <math.h>
#include <vector>

namespace
{

/** 颜色定义（RGB）*/
struct PaintColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

/** 计算两个颜色之间的插值（t: 0~1）*/
PaintColor LerpColor(const PaintColor& from, const PaintColor& to, double t)
{
    t = (t < 0.0) ? 0.0 : ((t > 1.0) ? 1.0 : t);
    PaintColor color;
    color.r = (uint8_t)(from.r + (to.r - from.r) * t);
    color.g = (uint8_t)(from.g + (to.g - from.g) * t);
    color.b = (uint8_t)(from.b + (to.b - from.b) * t);
    return color;
}

/** 颜色变暗（用于模拟辉光的外层线条）*/
PaintColor DarkenColor(const PaintColor& color, double ratio)
{
    ratio = (ratio < 0.0) ? 0.0 : ((ratio > 1.0) ? 1.0 : ratio);
    PaintColor result;
    result.r = (uint8_t)(color.r * ratio);
    result.g = (uint8_t)(color.g * ratio);
    result.b = (uint8_t)(color.b * ratio);
    return result;
}

COLORREF ToColorRef(const PaintColor& color)
{
    return RGB(color.r, color.g, color.b);
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

/** 波形的数量 */
constexpr int32_t kWaveCount = (int32_t)(sizeof(g_waveList) / sizeof(g_waveList[0]));

}//namespace

struct ChildWindowPaint::TImpl
{
public:
    TImpl();
    ~TImpl();

public:
    /** 绘制演示画面（深色渐变背景 + 网格 + 流动波形）
     * @param hdc 绘制DC
     * @param width 绘制区域宽度
     * @param height 绘制区域高度
     * @param phase 动画相位（弧度），静态绘制时传入0
     */
    void PaintScene(HDC hdc, int32_t width, int32_t height, double phase);

private:
    /** 创建（或复用）背景缓存位图：渐变背景 + 网格线
     */
    bool EnsureBackground(HDC hdc, int32_t width, int32_t height);

    /** 释放背景缓存位图
     */
    void ReleaseBackground();

    /** 创建（或复用）绘制波形用的画笔
     *  说明：GDI的宽画笔(Pen宽度大于1)绘制折线很慢，这里统一使用1像素宽的画笔，
     *  通过绘制相邻的像素线来形成较粗的线条，可以大幅提高绘制速度。
     */
    bool EnsurePens();

    /** 释放绘制波形用的画笔
     */
    void ReleasePens();

public:
    /** 创建绘制DC和位图等(仅在分层窗口绘制时使用)
    */
    HDC GetPaintDC(HWND hWnd);

    /** 创建位图(仅在分层窗口绘制时使用)
    */
    HBITMAP CreateHBitmap(HWND hWnd, int32_t nWidth, int32_t nHeight, bool flipHeight) const;

private:
    //背景缓存（渐变背景 + 网格线，仅在尺寸变化时重建）
    HDC m_bgDC;
    HBITMAP m_bgBitmap;
    HBITMAP m_bgOldBitmap;
    int32_t m_bgWidth;
    int32_t m_bgHeight;

private:
    //绘制DC/位图等(仅在分层窗口绘制时使用)
    HDC m_hPaintDC;     //绘制DC
    HGDIOBJ m_hOldObj;  //DC原来关联的对象
    HBITMAP m_hBitmap;  //绘制位图
    int32_t m_nBitmapWidth;  //位图宽度
    int32_t m_nBitmapHeight; //位图高度

private:
    //绘制波形使用的画笔（1像素宽，创建后复用，避免每帧创建/销毁GDI对象）
    HPEN m_wavePen[kWaveCount];     //波形主色画笔
    HPEN m_waveEdgePen[kWaveCount]; //波形边缘（暗色）画笔
};

ChildWindowPaint::TImpl::TImpl():
    m_bgDC(nullptr),
    m_bgBitmap(nullptr),
    m_bgOldBitmap(nullptr),
    m_bgWidth(0),
    m_bgHeight(0),
    m_hPaintDC(nullptr),
    m_hOldObj(nullptr),
    m_hBitmap(nullptr),
    m_nBitmapWidth(0),
    m_nBitmapHeight(0)
{
    for (int32_t nIndex = 0; nIndex < kWaveCount; ++nIndex) {
        m_wavePen[nIndex] = nullptr;
        m_waveEdgePen[nIndex] = nullptr;
    }
}

ChildWindowPaint::TImpl::~TImpl()
{
    ReleaseBackground();
    ReleasePens();

    if (m_hPaintDC != nullptr) {
        if (m_hOldObj != nullptr) {
            ::SelectObject(m_hPaintDC, m_hOldObj);
            m_hOldObj = nullptr;
        }
        ::DeleteDC(m_hPaintDC);
        m_hPaintDC = nullptr;
    }
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
}

void ChildWindowPaint::TImpl::ReleaseBackground()
{
    if (m_bgDC != nullptr) {
        if (m_bgOldBitmap != nullptr) {
            ::SelectObject(m_bgDC, m_bgOldBitmap);
            m_bgOldBitmap = nullptr;
        }
        ::DeleteDC(m_bgDC);
        m_bgDC = nullptr;
    }
    if (m_bgBitmap != nullptr) {
        ::DeleteObject(m_bgBitmap);
        m_bgBitmap = nullptr;
    }
    m_bgWidth = 0;
    m_bgHeight = 0;
}

bool ChildWindowPaint::TImpl::EnsurePens()
{
    bool bReady = true;
    for (int32_t nIndex = 0; nIndex < kWaveCount; ++nIndex) {
        if (m_wavePen[nIndex] == nullptr) {
            //1像素宽的实线画笔（GDI绘制宽画笔的折线很慢，这里通过相邻像素线实现加粗效果）
            m_wavePen[nIndex] = ::CreatePen(PS_SOLID, 1, ToColorRef(g_waveList[nIndex].color));
        }
        if (m_waveEdgePen[nIndex] == nullptr) {
            m_waveEdgePen[nIndex] = ::CreatePen(PS_SOLID, 1, ToColorRef(DarkenColor(g_waveList[nIndex].color, 0.45)));
        }
        if ((m_wavePen[nIndex] == nullptr) || (m_waveEdgePen[nIndex] == nullptr)) {
            bReady = false;
        }
    }
    return bReady;
}

void ChildWindowPaint::TImpl::ReleasePens()
{
    for (int32_t nIndex = 0; nIndex < kWaveCount; ++nIndex) {
        if (m_wavePen[nIndex] != nullptr) {
            ::DeleteObject(m_wavePen[nIndex]);
            m_wavePen[nIndex] = nullptr;
        }
        if (m_waveEdgePen[nIndex] != nullptr) {
            ::DeleteObject(m_waveEdgePen[nIndex]);
            m_waveEdgePen[nIndex] = nullptr;
        }
    }
}

bool ChildWindowPaint::TImpl::EnsureBackground(HDC hdc, int32_t width, int32_t height)
{
    if ((m_bgDC != nullptr) && (m_bgBitmap != nullptr) && (m_bgWidth == width) && (m_bgHeight == height)) {
        return true;
    }
    ReleaseBackground();

    m_bgDC = ::CreateCompatibleDC(hdc);
    if (m_bgDC == nullptr) {
        return false;
    }
    m_bgBitmap = ::CreateCompatibleBitmap(hdc, width, height);
    if (m_bgBitmap == nullptr) {
        ::DeleteDC(m_bgDC);
        m_bgDC = nullptr;
        return false;
    }
    m_bgOldBitmap = (HBITMAP)::SelectObject(m_bgDC, m_bgBitmap);

    //垂直渐变背景
    for (int32_t y = 0; y < height; y += 2) {
        double t = (height > 1) ? ((double)y / (double)(height - 1)) : 0.0;
        HBRUSH hBrush = ::CreateSolidBrush(ToColorRef(LerpColor(g_bgTopColor, g_bgBottomColor, t)));
        if (hBrush != nullptr) {
            RECT rcBand = { 0, y, width, y + 2 };
            ::FillRect(m_bgDC, &rcBand, hBrush);
            ::DeleteObject(hBrush);
        }
    }

    //淡色网格线
    HPEN hGridPen = ::CreatePen(PS_SOLID, 1, ToColorRef(g_gridColor));
    if (hGridPen != nullptr) {
        HGDIOBJ hOldPen = ::SelectObject(m_bgDC, hGridPen);
        const int32_t nGridSize = 32;
        for (int32_t x = nGridSize; x < width; x += nGridSize) {
            ::MoveToEx(m_bgDC, x, 0, nullptr);
            ::LineTo(m_bgDC, x, height);
        }
        for (int32_t y = nGridSize; y < height; y += nGridSize) {
            ::MoveToEx(m_bgDC, 0, y, nullptr);
            ::LineTo(m_bgDC, width, y);
        }
        ::SelectObject(m_bgDC, hOldPen);
        ::DeleteObject(hGridPen);
    }

    m_bgWidth = width;
    m_bgHeight = height;
    return true;
}

void ChildWindowPaint::TImpl::PaintScene(HDC hdc, int32_t width, int32_t height, double phase)
{
    if ((hdc == nullptr) || (width <= 0) || (height <= 0)) {
        return;
    }
    if (!EnsureBackground(hdc, width, height) || !EnsurePens()) {
        return;
    }
    //先绘制背景（渐变 + 网格）
    ::BitBlt(hdc, 0, 0, width, height, m_bgDC, 0, 0, SRCCOPY);

    //再绘制波形：每条波形由3条1像素线组成（上下为暗色边缘，中间为主色线）
    //说明：GDI绘制宽画笔的折线速度很慢，使用3条1像素线代替，速度可以提高数倍
    const double centerY = height / 2.0;
    const double twoPi = 2.0 * 3.14159265358979323846;
    const int32_t nStepX = 4;
    std::vector<POINT> upperPoints;
    std::vector<POINT> lowerPoints;
    std::vector<POINT> mainPoints;
    upperPoints.reserve(static_cast<size_t>(width / nStepX) + 2);
    lowerPoints.reserve(static_cast<size_t>(width / nStepX) + 2);
    mainPoints.reserve(static_cast<size_t>(width / nStepX) + 2);
    for (int32_t nIndex = 0; nIndex < kWaveCount; ++nIndex) {
        const WaveDesc& wave = g_waveList[nIndex];
        upperPoints.clear();
        lowerPoints.clear();
        mainPoints.clear();
        for (int32_t x = 0; x <= width; x += nStepX) {
            double angle = (wave.frequency * twoPi * (double)x / (double)width) + phase + wave.phaseOffset;
            int32_t y = (int32_t)(centerY + wave.amplitudeRatio * (double)height * sin(angle));
            if (y < 2) {
                y = 2;
            }
            else if (y > height - 3) {
                y = height - 3;
            }
            POINT ptMain = { x, y };
            mainPoints.push_back(ptMain);
            POINT ptUpper = { x, y - 1 };
            upperPoints.push_back(ptUpper);
            POINT ptLower = { x, y + 1 };
            lowerPoints.push_back(ptLower);
        }
        if (mainPoints.size() < 2) {
            continue;
        }
        const int32_t nPointCount = (int32_t)mainPoints.size();
        HGDIOBJ hOldPen = ::SelectObject(hdc, m_waveEdgePen[nIndex]);
        ::Polyline(hdc, upperPoints.data(), nPointCount);
        ::Polyline(hdc, lowerPoints.data(), nPointCount);
        ::SelectObject(hdc, m_wavePen[nIndex]);
        ::Polyline(hdc, mainPoints.data(), nPointCount);
        ::SelectObject(hdc, hOldPen);
    }
}

HDC ChildWindowPaint::TImpl::GetPaintDC(HWND hWnd)
{
    if (!::IsWindow(hWnd)) {
        return nullptr;
    }
    RECT rcClient;
    ::GetClientRect(hWnd, &rcClient);
    int32_t nBitmapWidth = rcClient.right - rcClient.left;
    int32_t nBitmapHeight = rcClient.bottom - rcClient.top;
    if ((nBitmapWidth <= 0) || (nBitmapHeight <= 0)) {
        return nullptr;
    }
    if ((nBitmapWidth != m_nBitmapWidth) || (nBitmapHeight != m_nBitmapHeight)) {
        //尺寸变化时，必须先把位图从DC中选回，再删除位图
        if ((m_hPaintDC != nullptr) && (m_hOldObj != nullptr)) {
            ::SelectObject(m_hPaintDC, m_hOldObj);
            m_hOldObj = nullptr;
        }
        if (m_hBitmap != nullptr) {
            ::DeleteObject(m_hBitmap);
            m_hBitmap = nullptr;
        }
    }
    if (m_hBitmap == nullptr) {
        m_hBitmap = CreateHBitmap(hWnd, nBitmapWidth, nBitmapHeight, true);
        if (m_hBitmap == nullptr) {
            return nullptr;
        }
        m_nBitmapWidth = nBitmapWidth;
        m_nBitmapHeight = nBitmapHeight;
    }
    if (m_hPaintDC == nullptr) {
        HDC hWndDC = ::GetDC(hWnd);
        m_hPaintDC = ::CreateCompatibleDC(hWndDC);
        ::ReleaseDC(hWnd, hWndDC);
    }
    if (m_hPaintDC == nullptr) {
        return nullptr;
    }
    if (m_hOldObj == nullptr) {
        m_hOldObj = ::SelectObject(m_hPaintDC, m_hBitmap);
    }
    else {
        ::SelectObject(m_hPaintDC, m_hBitmap);
    }
    return m_hPaintDC;
}

HBITMAP ChildWindowPaint::TImpl::CreateHBitmap(HWND hWnd, int32_t nWidth, int32_t nHeight, bool flipHeight) const
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if (nWidth <= 0 || nHeight <= 0) {
        return nullptr;
    }

    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = nWidth;
    if (flipHeight) {
        bmi.bmiHeader.biHeight = -nHeight;//负数表示位图方向：从上到下，左上角为圆点
    }
    else {
        bmi.bmiHeader.biHeight = nHeight; //正数表示位图方向：从下到上，左下角为圆点
    }
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = nWidth * nHeight * sizeof(DWORD);

    HBITMAP hBitmap = nullptr;
    HDC hdc = ::GetDC(hWnd);
    ASSERT(hdc != nullptr);
    if (hdc != nullptr) {
        void* pBits = nullptr;
        hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    }
    ::ReleaseDC(hWnd, hdc);
    return hBitmap;
}

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
    ASSERT(nativeMsg.uMsg == WM_PAINT);
    if (nativeMsg.uMsg != WM_PAINT) {
        return false;
    }

    HWND hWnd = (HWND)nativeMsg.wParam;
    ASSERT((hWnd != nullptr) && ::IsWindow(hWnd));
    if ((hWnd == nullptr) || !::IsWindow(hWnd)) {
        return false;
    }

    RECT rectUpdate = { 0, };
    if (!::GetUpdateRect(hWnd, &rectUpdate, FALSE)) {
        //无需绘制
        return false;
    }

    //动态绘制时按时间计算动画相位，静态绘制时固定显示相位0的静止帧
    double phase = 0.0;
    if (bPaintFps) {
        std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - m_startTime;
        phase = elapsed.count() * 0.9 + (double)m_nChildWindowIndex * 0.6;
    }

    bool bLayeredWindow = false;
    if (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) {
        //分层窗口
        COLORREF crKey = 0;
        BYTE bAlpha = 0;
        DWORD dwFlags = LWA_ALPHA | LWA_COLORKEY;
        //当返回true的时候，不能按分层窗口绘制，必须按普通的窗口模式绘制
        bool bAttributes = ::GetLayeredWindowAttributes(hWnd, &crKey, &bAlpha, &dwFlags) != FALSE;
        if (!bAttributes) {
            bLayeredWindow = true;
        }
    }
    if (m_impl == nullptr) {
        m_impl = new TImpl;
    }
    if (bLayeredWindow) {
        //分层窗口
        ui::UiRect rcChildWindow;
        if (m_pChildWindow != nullptr) {
            m_pChildWindow->GetChildWindowRect(rcChildWindow);
        }
        HDC hPaintDC = m_impl->GetPaintDC(hWnd);
        if (hPaintDC == nullptr) {
            return false;
        }
        RECT rcClient;
        ::GetClientRect(hWnd, &rcClient);
        m_impl->PaintScene(hPaintDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, phase);

        //结束绘制
        BYTE nLayeredWindowAlpha = 255;
        POINT pt = { rcChildWindow.left, rcChildWindow.top };
        SIZE szWindow = { rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
        POINT ptSrc = { 0, 0 };
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, nLayeredWindowAlpha, AC_SRC_OVER };
        BOOL bUpdated = ::UpdateLayeredWindow(hWnd, nullptr, &pt, &szWindow, hPaintDC, &ptSrc, 0, &bf, ULW_ALPHA);

        //标记绘制区域为有效区域
        RECT rectUpdate2 = { 0, };
        ::GetUpdateRect(hWnd, &rectUpdate2, FALSE);
        ::ValidateRect(hWnd, &rectUpdate2);
        return bUpdated != FALSE;
    }
    else {
        //普通窗口
        bool bPainted = false;
        PAINTSTRUCT ps = { 0, };
        HDC hPaintDC = ::BeginPaint(hWnd, &ps);
        if (hPaintDC != nullptr) {
            RECT rcClient;
            ::GetClientRect(hWnd, &rcClient);
            m_impl->PaintScene(hPaintDC, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, phase);
            bPainted = (rcClient.right > rcClient.left) && (rcClient.bottom > rcClient.top);
        }
        ::EndPaint(hWnd, &ps);
        return bPainted;
    }
}

#endif
