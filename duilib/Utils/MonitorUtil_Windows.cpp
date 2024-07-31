#include "MonitorUtil.h"
#include "duilib/Core/WindowBase.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#include "duilib/Utils/ApiWrapper_Windows.h"
#include "duilib/duilib_config_windows.h"
#include <VersionHelpers.h>

namespace ui
{

uint32_t MonitorUtil::GetWindowDpi(const WindowBase* pWindowBase)
{
    //读取窗口的DPI值
    uint32_t uDPI = 0;
    HWND hWnd = nullptr;
    if (pWindowBase != nullptr) {
        hWnd = pWindowBase->NativeWnd()->GetHWND();
    }
    if (hWnd != nullptr) {
        if (::IsWindows10OrGreater()) {
            if (!GetDpiForWindowWrapper(hWnd, uDPI)) {
                uDPI = 0;
            }
        }
    }
    if ((uDPI == 0) && (hWnd != nullptr) && ::IsWindows8OrGreater()) {
        HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        if (hMonitor != nullptr) {
            uint32_t dpix = 96;
            uint32_t dpiy = 96;
            if (GetDpiForMonitorWrapper(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy)) {
                uDPI = dpix;
            }
        }
    }
    if ((uDPI == 0) && (hWnd != nullptr)) {
        HDC hDC = ::GetDC(hWnd);
        if (hDC != nullptr) {
            uDPI = (uint32_t)::GetDeviceCaps(hDC, LOGPIXELSX);
            ::ReleaseDC(hWnd, hDC);
        }
    }
    return uDPI;
}

uint32_t MonitorUtil::GetPrimaryMonitorDPI()
{
    bool bOk = false;
    uint32_t uDPI = 96;
    if (::IsWindows10OrGreater()) {
        if (GetDpiForSystemWrapper(uDPI)) {
            bOk = true;
        }
    }
    if (!bOk && ::IsWindows8OrGreater()) {
        POINT pt = { 1, 1 };
        HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
        if (hMonitor != nullptr) {
            uint32_t dpix = 96;
            uint32_t dpiy = 96;
            if (GetDpiForMonitorWrapper(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy)) {
                uDPI = dpix;
                bOk = true;
            }
        }
    }
    if (!bOk) {
        HDC desktopDc = ::GetDC(nullptr);
        uDPI = (uint32_t)::GetDeviceCaps(desktopDc, LOGPIXELSX);
        ::ReleaseDC(nullptr, desktopDc);
    }
    if (uDPI == 0) {
        uDPI = 96;
    }
    return uDPI;
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
