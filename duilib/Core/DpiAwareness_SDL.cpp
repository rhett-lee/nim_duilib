#include "DpiAwareness.h"

//仅限非Windows平台
#ifdef DUILIB_BUILD_FOR_SDL

namespace ui
{
DpiInitParam::DpiInitParam() :
    m_dpiAwarenessFlag(DpiInitParam::DpiAwarenessFlag::kFromUserDefine),
    m_dpiAwarenessMode(DpiAwarenessMode::kPerMonitorDpiAware_V2),
    m_uDPI(0)
{
}

DpiAwareness::DpiAwareness():
    m_dpiAwarenessMode(DpiAwarenessMode::kPerMonitorDpiAware_V2)
{
}

DpiAwareness::~DpiAwareness()
{
}

bool DpiAwareness::InitDpiAwareness(const DpiInitParam& initParam)
{
    bool bRet = true;
    if (initParam.m_dpiAwarenessFlag == DpiInitParam::DpiAwarenessFlag::kFromUserDefine) {
        //设置一次 Dpi Awareness
        SetDpiAwareness(initParam.m_dpiAwarenessMode);
        DpiAwarenessMode dpiAwarenessMode = GetDpiAwareness();
        if (initParam.m_dpiAwarenessMode == DpiAwarenessMode::kDpiUnaware) {
            bRet = (dpiAwarenessMode == DpiAwarenessMode::kDpiUnaware) ? true : false;
        }
        else {
            bRet = (dpiAwarenessMode != DpiAwarenessMode::kDpiUnaware) ? true : false;
        }
    }
    return bRet;
}

DpiAwarenessMode DpiAwareness::SetDpiAwareness(DpiAwarenessMode dpiAwarenessMode)
{
    m_dpiAwarenessMode = dpiAwarenessMode;
    return GetDpiAwareness();
}

DpiAwarenessMode DpiAwareness::GetDpiAwareness() const
{
    return m_dpiAwarenessMode;
}

}

#endif //DUILIB_BUILD_FOR_SDL
