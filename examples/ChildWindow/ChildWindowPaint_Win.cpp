#include "ChildWindowPaint.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <vector>

// 定义缓存位图的最大数量（可根据需求调整）
#define MAX_CACHE_BITMAP_COUNT 10

// 定义图形类型枚举
typedef enum {
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_LINE,
    SHAPE_COUNT
} ShapeType;

// 缓存位图结构体
typedef struct {
    HBITMAP hBitmap;      // 缓存位图句柄
    HDC hMemDC;           // 内存DC句柄
    HBITMAP hOldBitmap;   // 内存DC原来关联的位图句柄
    int width;            // 缓存对应的窗口宽度
    int height;           // 缓存对应的窗口高度
} CacheBitmap;

// 随机数初始化标记
static int g_randomInitialized = 0;

struct ChildWindowPaint::TImpl
{
public:
    TImpl();
    ~TImpl();

public:
    /** 绘制窗口内容，普通窗口（用于测试子窗口的绘制功能）
    */
    void PaintChildWindow(HWND hWnd, const ui::UiRect& rcPaint, bool bPaintFps);

    /** 绘制窗口内容，分层窗口（用于测试子窗口的绘制功能）
    */
    void PaintChildWindowLayered(HWND hWnd, const ui::UiRect& rcPaint, const ui::UiRect& rcChildWindow, bool bPaintFps);

private:
    /** 创建缓存位图并绘制随机图案
     * @param hdc 参考DC（用于创建兼容位图）
     * @param width 窗口宽度
     * @param height 窗口高度
     * @param shapeCount 图形数量
     * @return 成功返回true，失败返回false
     */
    bool CreateCacheBitmap(HDC hdc, int width, int height, int shapeCount);

    /** 销毁所有缓存位图，释放GDI资源
    */
    void DestroyCacheBitmaps();

    /** 从缓存绘制到窗口DC
     * @param hdc 窗口DC
     * @param width 窗口宽度
     * @param height 窗口高度
     * @return 成功返回true，失败返回false
     */
    bool DrawFromCache(HDC hdc, int width, int height);

private:
    /** 创建绘制DC和位图等(仅在分层窗口绘制时使用)
    */
    HDC GetPaintDC(HWND hWnd);

    /** 创建位图(仅在分层窗口绘制时使用)
    */
    HBITMAP CreateHBitmap(HWND hWnd, int32_t nWidth, int32_t nHeight, bool flipHeight) const;

private:
    //位图缓存数据
    std::vector<CacheBitmap> m_cacheBitmaps;

private:
    //绘制DC/位图等(仅在分层窗口绘制时使用)
    HDC m_hPaintDC;     //绘制DC
    HGDIOBJ m_hOldObj;  //DC原来关联的对象
    HBITMAP m_hBitmap;  //绘制位图
    int32_t m_nBitmapWidth;  //位图宽度
    int32_t m_nBitmapHeight; //位图高度
};

ChildWindowPaint::TImpl::TImpl():
    m_hPaintDC(nullptr),
    m_hOldObj(nullptr),
    m_hBitmap(nullptr),
    m_nBitmapWidth(0),
    m_nBitmapHeight(0)
{
}

ChildWindowPaint::TImpl::~TImpl()
{
    DestroyCacheBitmaps();

    if (m_hPaintDC != nullptr) {
        ::SelectObject(m_hPaintDC, m_hOldObj);
        ::DeleteDC(m_hPaintDC);
    }
    if (m_hBitmap != nullptr) {
        ::DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
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

void ChildWindowPaint::TImpl::DestroyCacheBitmaps()
{
    for (size_t i = 0; i < m_cacheBitmaps.size(); i++) {
        if (m_cacheBitmaps[i].hMemDC) {
            SelectObject(m_cacheBitmaps[i].hMemDC, m_cacheBitmaps[i].hOldBitmap);//恢复原来的关联位图
            DeleteDC(m_cacheBitmaps[i].hMemDC);
            m_cacheBitmaps[i].hMemDC = nullptr;
        }
        if (m_cacheBitmaps[i].hBitmap) {
            DeleteObject(m_cacheBitmaps[i].hBitmap);
            m_cacheBitmaps[i].hBitmap = nullptr;
        }
    }
    m_cacheBitmaps.clear();
}

bool ChildWindowPaint::TImpl::CreateCacheBitmap(HDC hdc, int width, int height, int shapeCount)
{
    // 先销毁旧缓存
    DestroyCacheBitmaps();

    if (width <= 0 || height <= 0 || shapeCount <= 0) {
        return false;
    }

    // 初始化随机数生成器（仅初始化一次）
    if (!g_randomInitialized) {
        srand((unsigned int)time(NULL));
        g_randomInitialized = 1;
    }

    for (int32_t nIndex = 0; nIndex < MAX_CACHE_BITMAP_COUNT; ++nIndex) {

        // 创建内存DC和兼容位图
        CacheBitmap cache = { 0 };
        cache.hMemDC = CreateCompatibleDC(hdc);
        if (!cache.hMemDC) {
            return false;
        }

        cache.hBitmap = CreateCompatibleBitmap(hdc, width, height);
        if (!cache.hBitmap) {
            DeleteDC(cache.hMemDC);
            return false;
        }

        // 将位图选入内存DC
        cache.hOldBitmap = (HBITMAP)SelectObject(cache.hMemDC, cache.hBitmap);
        cache.width = width;
        cache.height = height;

        // 清屏（黑色背景）
        RECT rectClient = { 0, 0, width, height };
        HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(cache.hMemDC, &rectClient, hBlackBrush);
        DeleteObject(hBlackBrush);

        // 循环绘制随机图形到内存位图
        for (int i = 0; i < shapeCount; i++) {
            // 生成随机颜色 (RGB)
            BYTE r = (BYTE)(rand() % 256);
            BYTE g = (BYTE)(rand() % 256);
            BYTE b = (BYTE)(rand() % 256);
            COLORREF color = RGB(r, g, b);

            // 随机线条宽度 (1-10像素)
            int lineWidth = 1 + rand() % 10;

            // 创建画笔（用于描边/线条）
            HPEN hPen = CreatePen(PS_SOLID, lineWidth, color);
            HPEN hOldPen = (HPEN)SelectObject(cache.hMemDC, hPen);

            // 创建画刷（用于填充）
            HBRUSH hBrush = CreateSolidBrush(color);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(cache.hMemDC, hBrush);

            // 随机选择图形类型
            ShapeType shape = (ShapeType)(rand() % SHAPE_COUNT);

            switch (shape) {
            case SHAPE_RECTANGLE: {
                // 随机矩形位置和尺寸
                int x = rand() % width;
                int y = rand() % height;
                int w = 10 + rand() % 100;  // 宽度 10-109
                int h = 10 + rand() % 100;  // 高度 10-109

                // 确保图形在窗口内
                if (x + w > width) w = width - x;
                if (y + h > height) h = height - y;

                RECT rect = { x, y, x + w, y + h };

                // 随机选择填充或描边
                if (rand() % 2 == 0) {
                    FillRect(cache.hMemDC, &rect, hBrush);  // 填充矩形
                }
                else {
                    Rectangle(cache.hMemDC, rect.left, rect.top, rect.right, rect.bottom); // 描边矩形
                }
                break;
            }

            case SHAPE_CIRCLE: {
                // 随机圆形位置和半径（Windows GDI用椭圆模拟圆形）
                int centerX = rand() % width;
                int centerY = rand() % height;
                int radius = 5 + rand() % 50;  // 半径 5-54

                // 确保圆形在窗口内
                if (centerX - radius < 0) centerX = radius;
                if (centerX + radius > width) centerX = width - radius;
                if (centerY - radius < 0) centerY = radius;
                if (centerY + radius > height) centerY = height - radius;

                int left = centerX - radius;
                int top = centerY - radius;
                int right = centerX + radius;
                int bottom = centerY + radius;

                // 随机选择填充或描边
                if (rand() % 2 == 0) {
                    Ellipse(cache.hMemDC, left, top, right, bottom); // 填充圆形（椭圆）
                }
                else {
                    // 描边圆形：先画填充圆再用背景色画内圈
                    Ellipse(cache.hMemDC, left, top, right, bottom);
                    // 创建背景色画笔
                    HPEN hBgPen = CreatePen(PS_SOLID, lineWidth, RGB(0, 0, 0));
                    SelectObject(cache.hMemDC, hBgPen);
                    // 画内圈实现描边效果
                    int innerRadius = radius - lineWidth;
                    if (innerRadius > 0) {
                        Ellipse(cache.hMemDC, centerX - innerRadius, centerY - innerRadius,
                            centerX + innerRadius, centerY + innerRadius);
                    }
                    DeleteObject(hBgPen);
                    // 恢复原画笔
                    SelectObject(cache.hMemDC, hPen);
                }
                break;
            }

            case SHAPE_LINE: {
                // 随机线条起点和终点
                int x1 = rand() % width;
                int y1 = rand() % height;
                int x2 = rand() % width;
                int y2 = rand() % height;

                MoveToEx(cache.hMemDC, x1, y1, NULL);
                LineTo(cache.hMemDC, x2, y2);  // 绘制线条
                break;
            }

            default:
                break;
            }

            // 恢复并释放GDI对象（避免资源泄漏）
            SelectObject(cache.hMemDC, hOldPen);
            DeleteObject(hPen);
            SelectObject(cache.hMemDC, hOldBrush);
            DeleteObject(hBrush);
        }

        // 添加到缓存列表
        m_cacheBitmaps.push_back(cache);
    }
    return true;
}

bool ChildWindowPaint::TImpl::DrawFromCache(HDC hdc, int width, int height)
{
    if (m_cacheBitmaps.empty() || 
        m_cacheBitmaps[0].width != width ||
        m_cacheBitmaps[0].height != height) {
        return false;
    }

    // 直接将缓存位图拷贝到窗口DC（BitBlt是GDI中最快的绘制方式）
    int32_t nIndex = rand() % (int32_t)m_cacheBitmaps.size();
    BitBlt(hdc, 
           0, 0, width, height, 
           m_cacheBitmaps[nIndex].hMemDC,
           0, 0, 
           SRCCOPY);

    return true;
}

void ChildWindowPaint::TImpl::PaintChildWindow(HWND hWnd, const ui::UiRect& /*rcPaint*/, bool bPaintFps)
{
    // 获取窗口客户区尺寸
    RECT rectClient;
    ::GetClientRect(hWnd, &rectClient);
    int width = rectClient.right - rectClient.left;
    int height = rectClient.bottom - rectClient.top;

    //开始绘制
    PAINTSTRUCT ps = { 0, };
    HDC hPaintDC = ::BeginPaint(hWnd, &ps);
    if (bPaintFps) {//动态绘制
        //优先使用缓存绘制，缓存失效则重建
        if (!DrawFromCache(hPaintDC, width, height)) {
            // 缓存不存在/尺寸不匹配，创建新缓存并绘制
            CreateCacheBitmap(hPaintDC, width, height, 60);
            // 再次尝试从缓存绘制
            DrawFromCache(hPaintDC, width, height);
        }
    }
    else if (hPaintDC != nullptr) {
        //静态绘制，画蓝色背景色
        HBRUSH hBlueBrush = ::CreateSolidBrush(RGB(0, 0, 255));
        if (hBlueBrush != nullptr) {
            ::FillRect(hPaintDC, &rectClient, hBlueBrush);
            ::DeleteObject(hBlueBrush);
        }
    }
    //普通窗口
    ::EndPaint(hWnd, &ps);
}

void ChildWindowPaint::TImpl::PaintChildWindowLayered(HWND hWnd, const ui::UiRect& /*rcPaint*/,
                                                      const ui::UiRect& rcChildWindow, bool bPaintFps)
{
    // 获取窗口客户区尺寸
    RECT rcClient;
    ::GetClientRect(hWnd, &rcClient);
    int width = rcClient.right - rcClient.left;
    int height = rcClient.bottom - rcClient.top;

    RECT rectUpdate = { 0, };
    ::GetUpdateRect(hWnd, &rectUpdate, FALSE);

    //开始绘制
    HDC hPaintDC = GetPaintDC(hWnd);
    if (hPaintDC == nullptr) {
        return;
    }

    if (bPaintFps) {//动态绘制
        //优先使用缓存绘制，缓存失效则重建
        if (!DrawFromCache(hPaintDC, width, height)) {
            // 缓存不存在/尺寸不匹配，创建新缓存并绘制
            CreateCacheBitmap(hPaintDC, width, height, 60);
            // 再次尝试从缓存绘制
            DrawFromCache(hPaintDC, width, height);
        }
    }
    else {
        //静态绘制，画蓝色背景色
        HBRUSH hBlueBrush = ::CreateSolidBrush(RGB(0, 0, 255));
        if (hBlueBrush != nullptr) {
            ::FillRect(hPaintDC, &rcClient, hBlueBrush);
            ::DeleteObject(hBlueBrush);
        }
    }

    //结束绘制
    BYTE nLayeredWindowAlpha = 255;
    POINT pt = { rcChildWindow.left, rcChildWindow.top };
    SIZE szWindow = { rcClient.right - rcClient.left, rcClient.bottom - rcClient.top };
    POINT ptSrc = { 0, 0 };
    //BLENDFUNCTION bf = { AC_SRC_OVER, 0, nLayeredWindowAlpha, AC_SRC_ALPHA };
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, nLayeredWindowAlpha, AC_SRC_OVER };
    bool bRet = ::UpdateLayeredWindow(hWnd, nullptr, &pt, &szWindow, hPaintDC, &ptSrc, 0, &bf, ULW_ALPHA) != FALSE;

    //标记绘制区域为有效区域
    ::ValidateRect(hWnd, &rectUpdate);
}

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
    ASSERT(nativeMsg.uMsg == WM_PAINT);
    if (nativeMsg.uMsg != WM_PAINT) {
        return;
    }

    HWND hWnd = (HWND)nativeMsg.wParam;
    ASSERT((hWnd != nullptr) && ::IsWindow(hWnd));
    if ((hWnd == nullptr) || !::IsWindow(hWnd)) {
        return;
    }

    RECT rectUpdate = { 0, };
    if (!::GetUpdateRect(hWnd, &rectUpdate, FALSE)) {
        //无需绘制
        return;
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
        m_impl->PaintChildWindowLayered(hWnd, rcPaint, rcChildWindow, bPaintFps);
    }
    else {
        //普通窗口
        m_impl->PaintChildWindow(hWnd, rcPaint, bPaintFps);
    }
}

#endif
