#include "DpiManager.h"
#include "duilib/Utils/ApiWrapper.h"
#include <VersionHelpers.h>

namespace ui
{

/** 标准DPI值
*/
#define DPI_96	96

DpiManager::DpiManager():
	m_bDpiInited(false),
	m_uDpi(DPI_96),
	m_nScaleFactor(100)
{
	m_dpiAwarenessMode = GetDpiAwarenessMode();
}

DpiManager::~DpiManager()
{
}

bool DpiManager::InitDpiAwareness(const DpiInitParam& initParam)
{
	if (m_bDpiInited) {
		return false;
	}
	m_bDpiInited = true;
	bool bRet = true;
	if (initParam.m_dpiAwarenessFlag == DpiInitParam::DpiAwarenessFlag::kFromUserDefine) {
		//设置一次 Dpi Awareness
		SetDpiAwareness(initParam.m_dpiAwarenessMode);
		m_dpiAwarenessMode = GetDpiAwarenessMode();
		if (initParam.m_dpiAwarenessMode == DpiAwarenessMode::kDpiUnaware) {
			bRet = (m_dpiAwarenessMode == DpiAwarenessMode::kDpiUnaware) ? true : false;
		}
		else {
			bRet = (m_dpiAwarenessMode != DpiAwarenessMode::kDpiUnaware) ? true : false;
		}
	}
	else {
		m_dpiAwarenessMode = GetDpiAwarenessMode();
	}

	if (initParam.m_dpiFlag == DpiInitParam::DpiFlag::kFromSystem) {
		//从系统配置中读取默认的DPI值
		if (m_dpiAwarenessMode == DpiAwarenessMode::kDpiUnaware) {
			SetDPI(DPI_96);
		}
		else {
			SetDPI(DpiManager::GetMainMonitorDPI());
		}
	}
	else {
		//外部设置自定义的DPI值
		SetDPI(initParam.m_uDPI);
	}
	return bRet;
}

DpiAwarenessMode DpiManager::SetDpiAwareness(DpiAwarenessMode dpiAwarenessMode) const
{
	if (!::IsWindowsVistaOrGreater()) {
		//Vista以下版本系统，不支持DPI感知
		return DpiAwarenessMode::kDpiUnaware;
	}

	//说明：如果应用程序 (.exe) 清单设置 DPI 感知，则相关的设置API会调用失败
	//     如果此前调用或一次执行函数，则第二次调用的时候会失败（应该有限制，只允许设置一次）
	if (dpiAwarenessMode != DpiAwarenessMode::kDpiUnaware) {
		bool bSetOk = false;
		if (!bSetOk && ::IsWindows10OrGreater()) {
			//Windows10 及以上
			PROCESS_DPI_AWARENESS_CONTEXT newValueWin10 = PROCESS_DPI_AWARENESS_CONTEXT_UNAWARE;
			if (dpiAwarenessMode == DpiAwarenessMode::kPerMonitorDpiAware_V2) {
				newValueWin10 = PROCESS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;
			}
			else if (dpiAwarenessMode == DpiAwarenessMode::kPerMonitorDpiAware) {
				newValueWin10 = PROCESS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
			}
			else {
				newValueWin10 = PROCESS_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE;
			}
			PROCESS_DPI_AWARENESS_CONTEXT oldValueWin10 = PROCESS_DPI_AWARENESS_CONTEXT_UNAWARE;
			if (GetProcessDpiAwarenessContextWrapper(oldValueWin10)) {
				if (AreDpiAwarenessContextsEqualWrapper(oldValueWin10, newValueWin10)) {
					bSetOk = true;
				}
				if (!bSetOk && SetProcessDpiAwarenessContextWrapper(newValueWin10)) {
					bSetOk = true;
				}
				if (!bSetOk && (dpiAwarenessMode == DpiAwarenessMode::kPerMonitorDpiAware_V2)) {
					newValueWin10 = PROCESS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE;
					if (SetProcessDpiAwarenessContextWrapper(newValueWin10)) {
						bSetOk = true;
					}
				}
			}
		}

		if (!bSetOk && ::IsWindows8Point1OrGreater()) {
			//Win8.1 及以上
			PROCESS_DPI_AWARENESS newValueWin8 = PROCESS_DPI_UNAWARE;
			if ((dpiAwarenessMode == DpiAwarenessMode::kPerMonitorDpiAware) || 
				(dpiAwarenessMode == DpiAwarenessMode::kPerMonitorDpiAware_V2) ) {
				newValueWin8 = PROCESS_PER_MONITOR_DPI_AWARE;
			}
			else {
				newValueWin8 = PROCESS_SYSTEM_DPI_AWARE;
			}
			PROCESS_DPI_AWARENESS oldValueWin8 = PROCESS_DPI_UNAWARE;
			if (GetProcessDPIAwarenessWrapper(oldValueWin8)) {
				if (oldValueWin8 == newValueWin8) {
					bSetOk = true;
				}
				if (!bSetOk && SetProcessDPIAwarenessWrapper(newValueWin8)) {
					bSetOk = true;
				}
			}
		}

		if (!bSetOk) {
			bool bAware = false;
			if (IsProcessDPIAwareWrapper(bAware)) {
				if (bAware) {
					bSetOk = true;
				}
			}
			if (!bSetOk && SetProcessDPIAwareWrapper()) {
				bSetOk = true;
			}
		}
	}
	return GetDpiAwarenessMode();
}

DpiAwarenessMode DpiManager::GetDpiAwareness() const
{
	return m_dpiAwarenessMode;
}

DpiAwarenessMode DpiManager::GetDpiAwarenessMode() const
{
	DpiAwarenessMode dpiAwarenessMode = DpiAwarenessMode::kDpiUnaware;
	if (!::IsWindowsVistaOrGreater()) {
		//Vista以下版本系统，不支持DPI感知
		return dpiAwarenessMode;
	}
	bool bDpiInited = false;
	if (!bDpiInited && ::IsWindows10OrGreater()) {
		//Windows10 及以上
		PROCESS_DPI_AWARENESS_CONTEXT value = PROCESS_DPI_AWARENESS_CONTEXT_UNAWARE;
		if (GetProcessDpiAwarenessContextWrapper(value)) {
			bDpiInited = true;
			if (AreDpiAwarenessContextsEqualWrapper(value, PROCESS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
				dpiAwarenessMode = DpiAwarenessMode::kPerMonitorDpiAware_V2;
			}
			else if (AreDpiAwarenessContextsEqualWrapper(value, PROCESS_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {
				dpiAwarenessMode = DpiAwarenessMode::kPerMonitorDpiAware;
			}
			else if (AreDpiAwarenessContextsEqualWrapper(value, PROCESS_DPI_AWARENESS_CONTEXT_SYSTEM_AWARE)) {
				dpiAwarenessMode = DpiAwarenessMode::kSystemDpiAware;
			}
			else if (AreDpiAwarenessContextsEqualWrapper(value, PROCESS_DPI_AWARENESS_CONTEXT_UNAWARE)) {
				dpiAwarenessMode = DpiAwarenessMode::kDpiUnaware;
			}
			else {
				dpiAwarenessMode = DpiAwarenessMode::kDpiUnaware;
			}
		}
	}
	if (!bDpiInited && ::IsWindows8Point1OrGreater()) {
		//Win8.1 及以上
		PROCESS_DPI_AWARENESS value = PROCESS_DPI_UNAWARE;
		if (GetProcessDPIAwarenessWrapper(value)) {
			bDpiInited = true;
			if (value == PROCESS_PER_MONITOR_DPI_AWARE) {
				dpiAwarenessMode = DpiAwarenessMode::kPerMonitorDpiAware;
			}
			else if (value == PROCESS_SYSTEM_DPI_AWARE) {
				dpiAwarenessMode = DpiAwarenessMode::kSystemDpiAware;
			}
			else {
				dpiAwarenessMode = DpiAwarenessMode::kDpiUnaware;
			}
		}
	}
	if (!bDpiInited) {
		bool bAware = false;
		if (IsProcessDPIAwareWrapper(bAware)) {
			bDpiInited = true;
			if (bAware) {
				dpiAwarenessMode = DpiAwarenessMode::kSystemDpiAware;
			}
			else {
				dpiAwarenessMode = DpiAwarenessMode::kDpiUnaware;
			}
		}
	}
	return dpiAwarenessMode;
}

/** 获取某个显示器的DPI，开启DPI感知后有效
* @param[in] HMONITOR句柄
* @return 返回 DPI值
*/
static uint32_t GetMonitorDPI(HMONITOR hMonitor)
{
	uint32_t dpix = 96;
	uint32_t dpiy = 96;
	bool bOk = false;
	if (::IsWindows10OrGreater()) {
		if (GetDpiForSystemWrapper(dpix)) {
			bOk = true;
		}
	}
	if (!bOk && ::IsWindows8OrGreater()) {
		if (GetDpiForMonitorWrapper(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy)) {
			bOk = true;
		}
	}
	if (!bOk) {
		HDC desktopDc = ::GetDC(NULL);
		dpix = (uint32_t)::GetDeviceCaps(desktopDc, LOGPIXELSX);
		::ReleaseDC(0, desktopDc);
	}
	return dpix;
}

uint32_t DpiManager::GetMainMonitorDPI()
{
	POINT pt = { 1, 1 };
	HMONITOR hMonitor;
	hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
	return GetMonitorDPI(hMonitor);
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

int32_t DpiManager::ScaleInt(int32_t& iValue)
{
	if (m_nScaleFactor == 100) {
		return iValue;
	}
	iValue = MulDiv(iValue, m_nScaleFactor, 100);
	return iValue;
}

int32_t DpiManager::GetScaleInt(int32_t iValue)
{
	if (m_nScaleFactor == 100) {
		return iValue;
	}
	iValue = MulDiv(iValue, m_nScaleFactor, 100);
	return iValue;
}

uint32_t DpiManager::GetScaleInt(uint32_t iValue)
{
	if (m_nScaleFactor == 100) {
		return iValue;
	}
	iValue = (uint32_t)MulDiv((int)iValue, m_nScaleFactor, 100);
	return iValue;
}

void DpiManager::ScaleSize(SIZE &size)
{
	if (m_nScaleFactor == 100) {
		return;
	}
	size.cx = MulDiv(size.cx, m_nScaleFactor, 100);
	size.cy = MulDiv(size.cy, m_nScaleFactor, 100);
}

void DpiManager::ScaleSize(UiSize &size)
{
	if (m_nScaleFactor == 100) {
		return;
	}
	size.cx = MulDiv(size.cx, m_nScaleFactor, 100);
	size.cy = MulDiv(size.cy, m_nScaleFactor, 100);
}

void DpiManager::ScalePoint(POINT &point)
{
	if (m_nScaleFactor == 100) {
		return;
	}
	point.x = MulDiv(point.x, m_nScaleFactor, 100);
	point.y = MulDiv(point.y, m_nScaleFactor, 100);
}

void DpiManager::ScalePoint(UiPoint &point)
{
	if (m_nScaleFactor == 100) {
		return;
	}

	point.x = MulDiv(point.x, m_nScaleFactor, 100);
	point.y = MulDiv(point.y, m_nScaleFactor, 100);
}

void DpiManager::ScaleRect(RECT &rect)
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

void DpiManager::ScaleRect(UiRect &rect)
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

void DpiManager::ScalePadding(UiPadding& padding)
{
	if (m_nScaleFactor == 100) {
		return;
	}
	padding.left = MulDiv(padding.left, m_nScaleFactor, 100);
	padding.top = MulDiv(padding.top, m_nScaleFactor, 100);
	padding.right = MulDiv(padding.right, m_nScaleFactor, 100);
	padding.bottom = MulDiv(padding.bottom, m_nScaleFactor, 100);
}

void DpiManager::ScaleMargin(UiMargin& margin)
{
	if (m_nScaleFactor == 100) {
		return;
	}
	margin.left = MulDiv(margin.left, m_nScaleFactor, 100);
	margin.top = MulDiv(margin.top, m_nScaleFactor, 100);
	margin.right = MulDiv(margin.right, m_nScaleFactor, 100);
	margin.bottom = MulDiv(margin.bottom, m_nScaleFactor, 100);
}

int32_t DpiManager::MulDiv(int32_t nNumber, int32_t nNumerator, int32_t nDenominator) const
{
	return ::MulDiv(nNumber, nNumerator, nDenominator);
}

}