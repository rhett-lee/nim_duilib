#include "DpiManager.h"
#include "duilib/Utils/VersionHelpers.h"
#include "duilib/Utils/ApiWrapper.h"

namespace ui
{

DpiManager::DpiManager():
	m_bAdaptDPI(false),
	m_nScaleFactor(100)
{
}

DpiManager::~DpiManager()
{
}

uint32_t DpiManager::GetMonitorDPI(HMONITOR hMonitor)
{
	uint32_t dpix = 96;
	uint32_t dpiy = 96;
	if (IsWindows8OrGreater()) {
		if (!GetDpiForMonitorWrapper(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy)) {
			dpix = 96;
		}
	}
	else {
		HDC desktopDc = GetDC(NULL);
		dpix = (uint32_t)GetDeviceCaps(desktopDc, LOGPIXELSX);
		ReleaseDC(0, desktopDc);
	}
	return dpix;
}

uint32_t DpiManager::GetMainMonitorDPI()
{
	POINT pt = { 1, 1 };
	HMONITOR hMonitor;
	hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	return GetMonitorDPI(hMonitor);
}

bool DpiManager::IsAdaptDPI() const
{
	return m_bAdaptDPI;
}

bool DpiManager::SetAdaptDPI(bool bAdaptDPI)
{
	m_bAdaptDPI = bAdaptDPI;
	if (!IsWindowsVistaOrGreater()) {		
		return true;
	}

	//说明：如果应用程序 (.exe) 清单设置 DPI 感知，则API会调用失败
	//     如果此前调用或一次执行函数，则第二次调用的时候会失败（应该有限制，只允许设置一次）
	bool isOk = false;
	PROCESS_DPI_AWARENESS_CONTEXT new_value_win10 = bAdaptDPI ? PROCESS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 : PROCESS_DPI_AWARENESS_CONTEXT_UNAWARE;
	PROCESS_DPI_AWARENESS_CONTEXT old_value_win10 = PROCESS_DPI_AWARENESS_CONTEXT_UNAWARE;		
	if (GetProcessDpiAwarenessContextWrapper(old_value_win10)) {
		if (AreDpiAwarenessContextsEqualWrapper(old_value_win10, new_value_win10)) {
			isOk = true;
		}
		if (!isOk && SetProcessDpiAwarenessContextWrapper(new_value_win10)) {
			isOk = true;
		}
	}

	if (!isOk) {
		PROCESS_DPI_AWARENESS new_value_win8 = bAdaptDPI ? PROCESS_PER_MONITOR_DPI_AWARE : PROCESS_DPI_UNAWARE;
		PROCESS_DPI_AWARENESS old_value_win8 = PROCESS_DPI_UNAWARE;
		if (GetProcessDPIAwarenessWrapper(old_value_win8)) {
			if (old_value_win8 == new_value_win8) {
				isOk = true;
			}
			if (!isOk && SetProcessDPIAwarenessWrapper(new_value_win8)) {
				isOk = true;
			}
		}
	}
	
	if (!isOk) {
		bool bAware = false;
		if (IsProcessDPIAwareWrapper(bAware)) {
			if (bAware == bAdaptDPI) {
				isOk = true;
			}
		}
		if (!isOk && SetProcessDPIAwareWrapper()) {
			isOk = true;
		}
	}
	//根据主屏幕的DPI设置默认缩放比
	SetScale(DpiManager::GetMainMonitorDPI());
	return isOk;
}

uint32_t DpiManager::GetScale() const
{
	if (!m_bAdaptDPI) {
		return 100;
	}
	return m_nScaleFactor;
}

bool DpiManager::IsScaled() const
{
	return m_nScaleFactor != 100;
}

bool DpiManager::SetScale(uint32_t uDPI)
{
	bool isSet = false;
	if (m_bAdaptDPI) {
		m_nScaleFactor = MulDiv(uDPI, 100, 96);
		isSet = true;
	}
	else {
		m_nScaleFactor = 100;
	}
	ASSERT(m_nScaleFactor >= 100);
	return isSet;
}

int32_t DpiManager::ScaleInt(int32_t& iValue)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return iValue;
	}
	iValue = MulDiv(iValue, m_nScaleFactor, 100);
	return iValue;
}

int32_t DpiManager::GetScaleInt(int32_t iValue)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return iValue;
	}
	iValue = MulDiv(iValue, m_nScaleFactor, 100);
	return iValue;
}

uint32_t DpiManager::GetScaleInt(uint32_t iValue)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return iValue;
	}
	iValue = (uint32_t)MulDiv((int)iValue, m_nScaleFactor, 100);
	return iValue;
}

void DpiManager::ScaleSize(SIZE &size)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return;
	}
	size.cx = MulDiv(size.cx, m_nScaleFactor, 100);
	size.cy = MulDiv(size.cy, m_nScaleFactor, 100);
}

void DpiManager::ScaleSize(UiSize &size)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return;
	}
	size.cx = MulDiv(size.cx, m_nScaleFactor, 100);
	size.cy = MulDiv(size.cy, m_nScaleFactor, 100);
}

void DpiManager::ScalePoint(POINT &point)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return;
	}
	point.x = MulDiv(point.x, m_nScaleFactor, 100);
	point.y = MulDiv(point.y, m_nScaleFactor, 100);
}

void DpiManager::ScalePoint(UiPoint &point)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return;
	}

	point.x = MulDiv(point.x, m_nScaleFactor, 100);
	point.y = MulDiv(point.y, m_nScaleFactor, 100);
}

void DpiManager::ScaleRect(RECT &rect)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return;
	}
	int width = MulDiv(rect.right - rect.left, m_nScaleFactor, 100);
	int height = MulDiv(rect.bottom - rect.top, m_nScaleFactor, 100);
	rect.left = MulDiv(rect.left, m_nScaleFactor, 100);
	rect.top = MulDiv(rect.top, m_nScaleFactor, 100);
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void DpiManager::ScaleRect(UiRect &rect)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return;
	}
	int32_t width = MulDiv(rect.right - rect.left, m_nScaleFactor, 100);
	int32_t height = MulDiv(rect.bottom - rect.top, m_nScaleFactor, 100);
	rect.left = MulDiv(rect.left, m_nScaleFactor, 100);
	rect.top = MulDiv(rect.top, m_nScaleFactor, 100);
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;
}

void DpiManager::ScalePadding(UiPadding& padding)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return;
	}
	padding.left = MulDiv(padding.left, m_nScaleFactor, 100);
	padding.top = MulDiv(padding.top, m_nScaleFactor, 100);
	padding.right = MulDiv(padding.right, m_nScaleFactor, 100);
	padding.bottom = MulDiv(padding.bottom, m_nScaleFactor, 100);
}

void DpiManager::ScaleMargin(UiMargin& margin)
{
	if (!m_bAdaptDPI || m_nScaleFactor == 100) {
		return;
	}
	margin.left = MulDiv(margin.left, m_nScaleFactor, 100);
	margin.top = MulDiv(margin.top, m_nScaleFactor, 100);
	margin.right = MulDiv(margin.right, m_nScaleFactor, 100);
	margin.bottom = MulDiv(margin.bottom, m_nScaleFactor, 100);
}

int32_t DpiManager::MulDiv(int32_t nNumber, int32_t nNumerator, int32_t nDenominator)
{
	return ::MulDiv(nNumber, nNumerator, nDenominator);
}

}