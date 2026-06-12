#include "DwmUtil.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Utils/DllManager_Windows.h"

HRESULT DwmSetIconicThumbnail(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags)
{
    typedef HRESULT(WINAPI *LPSetProcessDPIAware)(HWND hwnd, HBITMAP hbmp, DWORD dwSITFlags);
    static LPSetProcessDPIAware pDwmSetIconicThumbnail = nullptr;
    if (pDwmSetIconicThumbnail) {
        return pDwmSetIconicThumbnail(hwnd, hbmp, dwSITFlags);
    }
    else {
        HMODULE hModule = ui::DllManager::Instance().LoadDll(_T("dwmapi.dll"));
        if (hModule != nullptr) {
            pDwmSetIconicThumbnail = (LPSetProcessDPIAware)GetProcAddress(hModule, "DwmSetIconicThumbnail");
            if (pDwmSetIconicThumbnail) {
                return pDwmSetIconicThumbnail(hwnd, hbmp, dwSITFlags);
            }
        }
    }

    return S_FALSE;
}

HRESULT DwmSetIconicLivePreviewBitmap(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags)
{
    typedef HRESULT(WINAPI *LPDwmSetIconicLivePreviewBitmap)(HWND hwnd, HBITMAP hbmp, POINT *pptClient, DWORD dwSITFlags);
    static LPDwmSetIconicLivePreviewBitmap pDwmSetIconicLivePreviewBitmap = nullptr;
    if (pDwmSetIconicLivePreviewBitmap) {
        return pDwmSetIconicLivePreviewBitmap(hwnd, hbmp, pptClient, dwSITFlags);
    }
    else {
        HMODULE hModule = ui::DllManager::Instance().LoadDll(_T("dwmapi.dll"));
        if (hModule != nullptr) {
            pDwmSetIconicLivePreviewBitmap = (LPDwmSetIconicLivePreviewBitmap)GetProcAddress(hModule, "DwmSetIconicLivePreviewBitmap");
            if (pDwmSetIconicLivePreviewBitmap) {
                return pDwmSetIconicLivePreviewBitmap(hwnd, hbmp, pptClient, dwSITFlags);
            }
        }
    }

    return S_FALSE;
}

HRESULT DwmInvalidateIconicBitmaps(HWND hwnd)
{
    typedef HRESULT(WINAPI *LPDwmInvalidateIconicBitmaps)(HWND hwnd);
    static LPDwmInvalidateIconicBitmaps pDwmInvalidateIconicBitmaps = nullptr;
    if (pDwmInvalidateIconicBitmaps) {
        return pDwmInvalidateIconicBitmaps(hwnd);
    }
    else {
        HMODULE hModule = ui::DllManager::Instance().LoadDll(_T("dwmapi.dll"));
        if (hModule != nullptr) {
            pDwmInvalidateIconicBitmaps = (LPDwmInvalidateIconicBitmaps)GetProcAddress(hModule, "DwmInvalidateIconicBitmaps");
            if (pDwmInvalidateIconicBitmaps) {
                return pDwmInvalidateIconicBitmaps(hwnd);
            }
        }
    }

    return S_FALSE;
}

HRESULT DwmSetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
{
    typedef HRESULT(WINAPI *LPDwmSetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
    static LPDwmSetWindowAttribute pDwmSetWindowAttribute = nullptr;
    if (pDwmSetWindowAttribute) {
        return pDwmSetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
    }
    else {
        HMODULE hModule = ui::DllManager::Instance().LoadDll(_T("dwmapi.dll"));
        if (hModule != nullptr) {
            pDwmSetWindowAttribute = (LPDwmSetWindowAttribute)GetProcAddress(hModule, "DwmSetWindowAttribute");
            if (pDwmSetWindowAttribute) {
                HRESULT hr = pDwmSetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
                return hr;
            }
        }
    }

    return S_FALSE;
}

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
