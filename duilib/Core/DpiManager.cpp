#include "DpiManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/ApiWrapper.h"
#include <VersionHelpers.h>

namespace ui
{

/** 标准DPI值
*/
#define DPI_96    96

DpiManager::DpiManager():
    m_bDpiInited(false),
    m_dpiAwarenessMode(DpiAwarenessMode::kDpiUnaware),
    m_bUserDefineDpi(false),
    m_uDpi(DPI_96),
    m_nScaleFactor(100)
{
}

DpiManager::~DpiManager()
{
}

uint32_t DpiManager::GetMainMonitorDPI()
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

void DpiManager::InitDpiAwareness(const DpiInitParam& dpiInitParam)
{
    //如果已经初始化过，则不重复初始化
    if (m_bDpiInited) {
        return;
    }
    m_bDpiInited = true;

    //初始化DPI感知模式
    DpiAwareness dpiAwareness;
    dpiAwareness.InitDpiAwareness(dpiInitParam);
    m_dpiAwarenessMode = dpiAwareness.GetDpiAwareness();

    //初始化DPI值
    if (dpiInitParam.m_uDPI == 0) {
        m_bUserDefineDpi = false;
        //从系统配置中读取默认的DPI值
        SetDpiByWindow(nullptr);
    }
    else {
        //外部设置自定义的DPI值
        m_bUserDefineDpi = true;
        SetDPI(dpiInitParam.m_uDPI);
    }
}

bool DpiManager::IsUserDefineDpi() const
{
    return m_bUserDefineDpi;
}

DpiAwarenessMode DpiManager::GetDpiAwareness() const
{
    DpiAwarenessMode dpiAwarenessMode = m_dpiAwarenessMode;
    if (!m_bDpiInited) {
        DpiAwareness dpiAwareness;
        dpiAwarenessMode = dpiAwareness.GetDpiAwareness();
    }
    return dpiAwarenessMode;
}

bool DpiManager::IsPerMonitorDpiAware() const
{
    DpiAwarenessMode dpiAwarenessMode = GetDpiAwareness();
    if ((dpiAwarenessMode == DpiAwarenessMode::kPerMonitorDpiAware) ||
        (dpiAwarenessMode == DpiAwarenessMode::kPerMonitorDpiAware_V2)) {
        return true;
    }
    else {
        return false;
    }
}

void DpiManager::SetDpiByWindow(Window* pWindow)
{
    //读取窗口的DPI值
    uint32_t uDPI = 0;
    HWND hWnd = nullptr;
    if (pWindow != nullptr) {
        hWnd = pWindow->GetHWND();
    }
    if (hWnd != nullptr) {
        if (::IsWindows10OrGreater()) {
            if (!GetDpiForWindowWrapper(pWindow->GetHWND(), uDPI)) {
                uDPI = 0;
            }
        }
    }
    if ((uDPI == 0) && (hWnd != nullptr) && ::IsWindows8OrGreater()) {
        HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        if (hMonitor == nullptr) {
            hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
        }
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

    //从系统配置中读取默认的DPI值
    if (uDPI == 0) {
        DpiAwarenessMode dpiAwarenessMode = GetDpiAwareness();
        if (dpiAwarenessMode == DpiAwarenessMode::kDpiUnaware) {
            uDPI = DPI_96;
        }
        else {
            uDPI = DpiManager::GetMainMonitorDPI();
        }
    }
    if (uDPI == 0) {
        uDPI = DPI_96;
    }
    SetDPI(uDPI);
}

void DpiManager::SetDPI(uint32_t uDPI)
{
    if (uDPI == 0) {
        uDPI = DPI_96;
    }
    m_nScaleFactor = MulDiv(uDPI, 100, 96);
    m_uDpi = uDPI;
    if (m_nScaleFactor == 0) {
        m_nScaleFactor = 100;
        m_uDpi = DPI_96;
    }
}

uint32_t DpiManager::GetDPI() const
{
    return m_uDpi;
}

uint32_t DpiManager::GetScale() const
{
    return m_nScaleFactor;
}

bool DpiManager::IsScaled() const
{
    return m_nScaleFactor != 100;
}

int32_t DpiManager::ScaleInt(int32_t& iValue) const
{
    if (m_nScaleFactor == 100) {
        return iValue;
    }
    iValue = MulDiv(iValue, m_nScaleFactor, 100);
    return iValue;
}

int32_t DpiManager::GetScaleInt(int32_t iValue) const
{
    if (m_nScaleFactor == 100) {
        return iValue;
    }
    iValue = MulDiv(iValue, m_nScaleFactor, 100);
    return iValue;
}

uint32_t DpiManager::GetScaleInt(uint32_t iValue) const
{
    if (m_nScaleFactor == 100) {
        return iValue;
    }
    iValue = (uint32_t)MulDiv((int)iValue, m_nScaleFactor, 100);
    return iValue;
}

int32_t DpiManager::GetScaleInt(int32_t iValue, uint32_t nOldDpiScale) const
{
    if ((nOldDpiScale == 0) || (m_nScaleFactor == 0)) {
        return iValue;
    }
    iValue = (uint32_t)MulDiv((int)iValue, m_nScaleFactor, nOldDpiScale);
    return iValue;
}

uint32_t DpiManager::GetScaleInt(uint32_t iValue, uint32_t nOldDpiScale) const
{
    if ((nOldDpiScale == 0) || (m_nScaleFactor == 0)) {
        return iValue;
    }
    iValue = (uint32_t)MulDiv((int)iValue, m_nScaleFactor, nOldDpiScale);
    return iValue;
}

void DpiManager::ScaleSize(SIZE &size) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    size.cx = MulDiv(size.cx, m_nScaleFactor, 100);
    size.cy = MulDiv(size.cy, m_nScaleFactor, 100);
}

void DpiManager::ScaleSize(UiSize &size) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    size.cx = MulDiv(size.cx, m_nScaleFactor, 100);
    size.cy = MulDiv(size.cy, m_nScaleFactor, 100);
}

UiSize DpiManager::GetScaleSize(UiSize size) const
{
    ScaleSize(size);
    return size;
}

UiSize DpiManager::GetScaleSize(UiSize size, uint32_t nOldDpiScale) const
{
    if ((m_nScaleFactor == 0) || (nOldDpiScale == 0) ){
        return size;
    }
    size.cx = MulDiv(size.cx, m_nScaleFactor, nOldDpiScale);
    size.cy = MulDiv(size.cy, m_nScaleFactor, nOldDpiScale);
    return size;
}

void DpiManager::ScalePoint(POINT &point) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    point.x = MulDiv(point.x, m_nScaleFactor, 100);
    point.y = MulDiv(point.y, m_nScaleFactor, 100);
}

void DpiManager::ScalePoint(UiPoint &point) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    point.x = MulDiv(point.x, m_nScaleFactor, 100);
    point.y = MulDiv(point.y, m_nScaleFactor, 100);
}

UiPoint DpiManager::GetScalePoint(UiPoint point, uint32_t nOldDpiScale) const
{
    if ((m_nScaleFactor == 0) || (nOldDpiScale == 0)) {
        return point;
    }
    point.x = MulDiv(point.x, m_nScaleFactor, nOldDpiScale);
    point.y = MulDiv(point.y, m_nScaleFactor, nOldDpiScale);
    return point;
}

void DpiManager::ScaleRect(RECT &rect) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    int width = MulDiv(rect.right - rect.left, m_nScaleFactor, 100);
    int height = MulDiv(rect.bottom - rect.top, m_nScaleFactor, 100);
    rect.left = MulDiv(rect.left, m_nScaleFactor, 100);
    rect.top = MulDiv(rect.top, m_nScaleFactor, 100);
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;
}

void DpiManager::ScaleRect(UiRect &rect) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    int32_t width = MulDiv(rect.right - rect.left, m_nScaleFactor, 100);
    int32_t height = MulDiv(rect.bottom - rect.top, m_nScaleFactor, 100);
    rect.left = MulDiv(rect.left, m_nScaleFactor, 100);
    rect.top = MulDiv(rect.top, m_nScaleFactor, 100);
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;
}

UiRect DpiManager::GetScaleRect(UiRect rect, uint32_t nOldDpiScale) const
{
    if ((m_nScaleFactor == 0) || (nOldDpiScale == 0)) {
        return rect;
    }
    int32_t width = MulDiv(rect.right - rect.left, m_nScaleFactor, nOldDpiScale);
    int32_t height = MulDiv(rect.bottom - rect.top, m_nScaleFactor, nOldDpiScale);
    rect.left = MulDiv(rect.left, m_nScaleFactor, nOldDpiScale);
    rect.top = MulDiv(rect.top, m_nScaleFactor, nOldDpiScale);
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;
    return rect;
}

void DpiManager::ScalePadding(UiPadding& padding) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    padding.left = MulDiv(padding.left, m_nScaleFactor, 100);
    padding.top = MulDiv(padding.top, m_nScaleFactor, 100);
    padding.right = MulDiv(padding.right, m_nScaleFactor, 100);
    padding.bottom = MulDiv(padding.bottom, m_nScaleFactor, 100);
}

UiPadding DpiManager::GetScalePadding(UiPadding padding, uint32_t nOldDpiScale) const
{
    if ((m_nScaleFactor == 0) || (nOldDpiScale == 0)) {
        return padding;
    }
    padding.left = MulDiv(padding.left, m_nScaleFactor, nOldDpiScale);
    padding.top = MulDiv(padding.top, m_nScaleFactor, nOldDpiScale);
    padding.right = MulDiv(padding.right, m_nScaleFactor, nOldDpiScale);
    padding.bottom = MulDiv(padding.bottom, m_nScaleFactor, nOldDpiScale);
    return padding;
}

void DpiManager::ScaleMargin(UiMargin& margin) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    margin.left = MulDiv(margin.left, m_nScaleFactor, 100);
    margin.top = MulDiv(margin.top, m_nScaleFactor, 100);
    margin.right = MulDiv(margin.right, m_nScaleFactor, 100);
    margin.bottom = MulDiv(margin.bottom, m_nScaleFactor, 100);
}

UiMargin DpiManager::GetScaleMargin(UiMargin margin, uint32_t nOldDpiScale) const
{
    if ((m_nScaleFactor == 0) || (nOldDpiScale == 0)) {
        return margin;
    }
    margin.left = MulDiv(margin.left, m_nScaleFactor, nOldDpiScale);
    margin.top = MulDiv(margin.top, m_nScaleFactor, nOldDpiScale);
    margin.right = MulDiv(margin.right, m_nScaleFactor, nOldDpiScale);
    margin.bottom = MulDiv(margin.bottom, m_nScaleFactor, nOldDpiScale);
    return margin;
}

int32_t DpiManager::MulDiv(int32_t nNumber, int32_t nNumerator, int32_t nDenominator) const
{
    return ::MulDiv(nNumber, nNumerator, nDenominator);
}

}