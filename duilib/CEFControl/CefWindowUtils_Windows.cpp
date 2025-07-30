#include "CefWindowUtils.h"
#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_WIN

namespace ui
{
void SetCefWindowPos(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    HWND hwnd = (HWND)cefWindow;
    if (::IsWindow(hwnd)) {
        UiRect rc = pCefControl->GetPos();
        ::SetWindowPos(hwnd, HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
    }
}

void SetCefWindowVisible(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    HWND hwnd = (HWND)cefWindow;
    if (hwnd) {
        if (pCefControl->IsVisible()) {
            ShowWindow(hwnd, SW_SHOW);
        }
        else {
            ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
        }
    }
}

void SetCefWindowParent(CefWindowHandle cefWindow, CefControl* pCefControl)
{
    if ((cefWindow == 0) || (pCefControl == nullptr)) {
        return;
    }
    if (!::IsWindow((HWND)cefWindow)) {
        return;
    }
    Window* pWindow = pCefControl->GetWindow();
    if (pWindow == nullptr) {
        return;
    }
    HWND hParent = pWindow->NativeWnd()->GetHWND();
    if (!::IsWindow(hParent)) {
        return;
    }
    ::SetParent((HWND)cefWindow, hParent);

    // 为新的主窗口重新设置WS_CLIPSIBLINGS、WS_CLIPCHILDREN样式，否则Cef窗口刷新会出问题
    LONG style = ::GetWindowLong(hParent, GWL_STYLE);
    ::SetWindowLong(hParent, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
}

bool CaptureCefWindowBitmap(CefWindowHandle cefWindow, std::vector<uint8_t>& bitmap, int32_t& width, int32_t& height)
{
    HWND hwnd = cefWindow;
    if (!::IsWindow(hwnd)) {
        return false;
    }
    // 获取窗口尺寸
    RECT rect = { 0, 0, 0, 0 };
    if (!GetClientRect(hwnd, &rect)) {
        return false;
    }

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    if (width <= 0 || height <= 0) {
        return false;
    }

    // 创建设备上下文
    HDC hdcScreen = ::GetDC(nullptr);
    if (hdcScreen == nullptr) {
        return false;
    }
    HDC hdcWindow = ::GetDC(hwnd);
    if (hdcWindow == nullptr) {
        ::ReleaseDC(nullptr, hdcScreen);
        return false;
    }

    HDC hdcMemDC = ::CreateCompatibleDC(hdcWindow);
    if (hdcMemDC == nullptr) {
        ::ReleaseDC(nullptr, hdcScreen);
        ::ReleaseDC(hwnd, hdcWindow);
        return false;
    }

    // 创建位图
    HBITMAP hBitmap = ::CreateCompatibleBitmap(hdcWindow, width, height);
    if (hBitmap == nullptr) {
        ::DeleteDC(hdcMemDC);
        ::ReleaseDC(nullptr, hdcScreen);
        ::ReleaseDC(hwnd, hdcWindow);
        return false;
    }

    HGDIOBJ hOldObj = ::SelectObject(hdcMemDC, hBitmap);

    // 拷贝屏幕内容到位图
    ::BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);

    // 获取位图信息
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  // 正数表示从下到上，负数表示从上到下
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // 分配内存并获取位图数据
    bitmap.resize(width * height * 4);
    ::GetDIBits(hdcMemDC, hBitmap, 0, height, bitmap.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // 清理资源
    ::SelectObject(hdcMemDC, hOldObj);
    ::DeleteObject(hBitmap);
    ::DeleteDC(hdcMemDC);
    ::ReleaseDC(nullptr, hdcScreen);
    ::ReleaseDC(hwnd, hdcWindow);

    return true;
}

void SetCefWindowCursor(CefWindowHandle cefWindow, CefCursorHandle cursor)
{
    if ((cefWindow == nullptr) || (cursor == nullptr)) {
        return;
    }
    ::SetClassLongPtr((HWND)cefWindow, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
    ::SetCursor(cursor);
}

void RemoveCefWindowFromParent(CefWindowHandle cefWindow)
{
    HWND hWnd = (HWND)cefWindow;
    if (::IsWindow(hWnd)) {
        ::SetParent(hWnd, nullptr);
    }
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
