#include "ScreenCapture.h"
#include "duilib/Core/GlobalManager.h"

#if defined (DUILIB_BUILD_FOR_WIN)

namespace ui
{
/** 创建位图
*/
static HBITMAP CreateBitmap(const Window* pWindow, int32_t nWidth, int32_t nHeight, bool flipHeight, LPVOID* pBits)
{
    ASSERT((nWidth > 0) && (nHeight > 0));
    if (nWidth == 0 || nHeight == 0) {
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

    HWND hWnd = (pWindow != nullptr) ? pWindow->NativeWnd()->GetHWND() : nullptr;
    HBITMAP hBitmap = nullptr;
    HDC hdc = ::GetDC(hWnd);
    ASSERT(hdc != nullptr);
    if (hdc != nullptr) {
        hBitmap = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, pBits, nullptr, 0);
    }
    ::ReleaseDC(hWnd, hdc);
    return hBitmap;
}

std::shared_ptr<IBitmap> ScreenCapture::CaptureBitmap(const Window* pWindow)
{
    if (pWindow == nullptr) {
        return nullptr;
    }
    std::shared_ptr<IBitmap> spBitmap;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        spBitmap.reset(pRenderFactory->CreateBitmap());
    }
    if (spBitmap == nullptr) {
        return nullptr;
    }

    //抓取屏幕位图
    UiRect rcMonitor;
    pWindow->GetMonitorRect(rcMonitor);
    int32_t xScreen = rcMonitor.left;
    int32_t yScreen = rcMonitor.top;
    int32_t cxScreen = rcMonitor.Width();
    int32_t cyScreen = rcMonitor.Height();
    if ((cxScreen <= 0) || (cyScreen <= 0)) {
        return nullptr;
    }
    HWND hWnd = nullptr; //取桌面
    HDC hdcSrc = ::GetDC(hWnd); // 获取屏幕句柄
    if (hdcSrc == nullptr) {
        return nullptr;
    }
    HDC hdcDst = ::CreateCompatibleDC(hdcSrc); // 创建一个兼容屏幕的DC
    if (hdcDst == nullptr) {
        ::ReleaseDC(hWnd, hdcSrc);
        return nullptr;
    }

    LPVOID pBits = nullptr;
    HBITMAP hBitmap = CreateBitmap(pWindow, cxScreen, cyScreen, true, &pBits);
    if (hBitmap == nullptr) {
        ::ReleaseDC(hWnd, hdcSrc); // 释放句柄
        ::DeleteDC(hdcDst);
        return nullptr;
    }
    ::SelectObject(hdcDst, hBitmap);
    ::BitBlt(hdcDst, 0, 0, cxScreen, cyScreen, hdcSrc, xScreen, yScreen, SRCCOPY); // 复制屏幕内容到位图
    ::ReleaseDC(hWnd, hdcSrc); // 释放句柄
    ::DeleteDC(hdcDst);

    if (!spBitmap->Init(cxScreen, cyScreen, true, pBits)) {
        spBitmap.reset();
    }
    ::DeleteObject(hBitmap);
    return spBitmap;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
