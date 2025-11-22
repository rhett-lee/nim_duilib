#include "DpiManager.h"
#include "duilib/Utils/MonitorUtil.h"
#include <cmath>

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
    m_nScaleFactor(100),
    m_fPixelDensity(1.0f)
{
}

DpiManager::~DpiManager()
{
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

void DpiManager::SetDpiByWindow(const WindowBase* pWindow)
{
    //读取窗口的DPI值
    uint32_t uDPI = MonitorUtil::GetWindowDpi(pWindow);
    
    //从系统配置中读取默认的DPI值
    if (uDPI == 0) {
        DpiAwarenessMode dpiAwarenessMode = GetDpiAwareness();
        if (dpiAwarenessMode == DpiAwarenessMode::kDpiUnaware) {
            uDPI = DPI_96;
        }
        else {
            uDPI = MonitorUtil::GetPrimaryMonitorDPI();
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
    m_nScaleFactor = MulDiv(uDPI, 100u, 96u);
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

uint32_t DpiManager::GetDisplayScaleFactor() const
{
    return m_nScaleFactor;
}

float DpiManager::GetDisplayScale() const
{
    return (float)m_nScaleFactor / 100.0f;
}

bool DpiManager::CheckDisplayScaleFactor(uint32_t nCheckScaleFactor) const
{
    ASSERT(nCheckScaleFactor == m_nScaleFactor);
    return (nCheckScaleFactor == m_nScaleFactor);
}

void DpiManager::SetWindowPixelDensity(float fPixelDensity)
{
    if (fPixelDensity < 1.0f) {
        fPixelDensity = 1.0f;
    }
    m_fPixelDensity = fPixelDensity;
}

float DpiManager::GetWindowPixelDensity() const
{
    return m_fPixelDensity;
}

bool DpiManager::IsDisplayScaled() const
{
    return m_nScaleFactor != 100;
}

void DpiManager::ScaleInt(int32_t& nValue) const
{
    if (m_nScaleFactor != 100) {
        nValue = MulDiv(nValue, (int32_t)m_nScaleFactor, 100);
    }    
}

void DpiManager::ScaleInt(uint32_t& nValue) const
{
    if (m_nScaleFactor != 100) {
        nValue = MulDiv(nValue, m_nScaleFactor, 100u);
    }
}

int32_t DpiManager::GetScaleInt(int32_t nValue) const
{
    if (m_nScaleFactor == 100) {
        return nValue;
    }
    nValue = MulDiv(nValue, (int32_t)m_nScaleFactor, 100);
    return nValue;
}

uint32_t DpiManager::GetScaleInt(uint32_t nValue) const
{
    if (m_nScaleFactor == 100) {
        return nValue;
    }
    nValue = (uint32_t)MulDiv(nValue, m_nScaleFactor, 100u);
    return nValue;
}

void DpiManager::UnscaleInt(int32_t& nValue) const
{
    if (m_nScaleFactor != 100) {
        nValue = MulDiv(nValue, 100, (int32_t)m_nScaleFactor);
    }
}

void DpiManager::UnscaleInt(uint32_t& nValue) const
{
    if (m_nScaleFactor != 100) {
        nValue = MulDiv(nValue, 100u, m_nScaleFactor);
    }
}

int32_t DpiManager::GetUnscaleInt(int32_t nValue) const
{
    if (m_nScaleFactor != 100) {
        nValue = MulDiv(nValue, 100, (int32_t)m_nScaleFactor);
    }
    return nValue;
}

uint32_t DpiManager::GetUnscaleInt(uint32_t nValue) const
{
    if (m_nScaleFactor != 100) {
        nValue = MulDiv(nValue, 100u, m_nScaleFactor);
    }
    return nValue;
}

float DpiManager::GetScaleFloat(int32_t nValue) const
{
    return std::round((nValue * m_nScaleFactor) / 100.0f);
}

float DpiManager::GetScaleFloat(uint32_t nValue) const
{
    return std::round((nValue * m_nScaleFactor) / 100.0f);
}

float DpiManager::GetScaleFloat(float fValue) const
{
    return std::round((fValue * m_nScaleFactor) / 100.0f);
}

float DpiManager::GetScaleFloat(float fValue, uint32_t nOldScaleFactor) const
{
    if ((nOldScaleFactor == 0) || (m_nScaleFactor == 0)) {
        return fValue;
    }
    return std::round((fValue * m_nScaleFactor) / (float)nOldScaleFactor);
}

int32_t DpiManager::GetScaleInt(int32_t nValue, uint32_t nOldScaleFactor) const
{
    if ((nOldScaleFactor == 0) || (m_nScaleFactor == 0)) {
        return nValue;
    }
    nValue = (uint32_t)MulDiv(nValue, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    return nValue;
}

uint32_t DpiManager::GetScaleInt(uint32_t nValue, uint32_t nOldScaleFactor) const
{
    if ((nOldScaleFactor == 0) || (m_nScaleFactor == 0)) {
        return nValue;
    }
    nValue = (uint32_t)MulDiv(nValue, m_nScaleFactor, nOldScaleFactor);
    return nValue;
}

void DpiManager::ScaleSize(UiSize& size) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    size.cx = MulDiv(size.cx, (int32_t)m_nScaleFactor, 100);
    size.cy = MulDiv(size.cy, (int32_t)m_nScaleFactor, 100);
}

UiSize DpiManager::GetScaleSize(UiSize size) const
{
    ScaleSize(size);
    return size;
}

UiSize DpiManager::GetScaleSize(UiSize size, uint32_t nOldScaleFactor) const
{
    if ((m_nScaleFactor == 0) || (nOldScaleFactor == 0) ){
        return size;
    }
    size.cx = MulDiv(size.cx, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    size.cy = MulDiv(size.cy, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    return size;
}

void DpiManager::ScalePoint(UiPoint &point) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    point.x = MulDiv(point.x, (int32_t)m_nScaleFactor, 100);
    point.y = MulDiv(point.y, (int32_t)m_nScaleFactor, 100);
}

UiPoint DpiManager::GetScalePoint(UiPoint point, uint32_t nOldScaleFactor) const
{
    if ((m_nScaleFactor == 0) || (nOldScaleFactor == 0)) {
        return point;
    }
    point.x = MulDiv(point.x, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    point.y = MulDiv(point.y, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    return point;
}

void DpiManager::ScaleRect(UiRect &rect) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    int32_t width = MulDiv(rect.right - rect.left, (int32_t)m_nScaleFactor, 100);
    int32_t height = MulDiv(rect.bottom - rect.top, (int32_t)m_nScaleFactor, 100);
    rect.left = MulDiv(rect.left, (int32_t)m_nScaleFactor, 100);
    rect.top = MulDiv(rect.top, (int32_t)m_nScaleFactor, 100);
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;
}

UiRect DpiManager::GetScaleRect(UiRect rect, uint32_t nOldScaleFactor) const
{
    if ((m_nScaleFactor == 0) || (nOldScaleFactor == 0)) {
        return rect;
    }
    int32_t width = MulDiv(rect.right - rect.left, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    int32_t height = MulDiv(rect.bottom - rect.top, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    rect.left = MulDiv(rect.left, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    rect.top = MulDiv(rect.top, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;
    return rect;
}

void DpiManager::ScalePadding(UiPadding& padding) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    padding.left = MulDiv(padding.left, (int32_t)m_nScaleFactor, 100);
    padding.top = MulDiv(padding.top, (int32_t)m_nScaleFactor, 100);
    padding.right = MulDiv(padding.right, (int32_t)m_nScaleFactor, 100);
    padding.bottom = MulDiv(padding.bottom, (int32_t)m_nScaleFactor, 100);
}

UiPadding DpiManager::GetScalePadding(UiPadding padding, uint32_t nOldScaleFactor) const
{
    if ((m_nScaleFactor == 0) || (nOldScaleFactor == 0)) {
        return padding;
    }
    padding.left = MulDiv(padding.left, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    padding.top = MulDiv(padding.top, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    padding.right = MulDiv(padding.right, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    padding.bottom = MulDiv(padding.bottom, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    return padding;
}

void DpiManager::ScaleMargin(UiMargin& margin) const
{
    if (m_nScaleFactor == 100) {
        return;
    }
    margin.left = MulDiv(margin.left, (int32_t)m_nScaleFactor, 100);
    margin.top = MulDiv(margin.top, (int32_t)m_nScaleFactor, 100);
    margin.right = MulDiv(margin.right, (int32_t)m_nScaleFactor, 100);
    margin.bottom = MulDiv(margin.bottom, (int32_t)m_nScaleFactor, 100);
}

UiMargin DpiManager::GetScaleMargin(UiMargin margin, uint32_t nOldScaleFactor) const
{
    if ((m_nScaleFactor == 0) || (nOldScaleFactor == 0)) {
        return margin;
    }
    margin.left = MulDiv(margin.left, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    margin.top = MulDiv(margin.top, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    margin.right = MulDiv(margin.right, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    margin.bottom = MulDiv(margin.bottom, (int32_t)m_nScaleFactor, (int32_t)nOldScaleFactor);
    return margin;
}

int32_t DpiManager::MulDiv(int32_t nNumber, int32_t nNumerator, int32_t nDenominator)
{
    if (nDenominator == 0) {
        return -1;
    }
    int64_t v = (int64_t)nNumber * (int64_t)nNumerator;
    return static_cast<int32_t>(std::round((double)v / nDenominator));
}

uint32_t DpiManager::MulDiv(uint32_t nNumber, uint32_t nNumerator, uint32_t nDenominator)
{
    if (nDenominator == 0) {
        return nNumber;
    }
    uint64_t v = (uint64_t)nNumber * (uint64_t)nNumerator;
    return static_cast<uint32_t>(std::round((double)v / nDenominator));
}

}
