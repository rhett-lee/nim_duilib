#include "DpiAwareness.h"

//仅限非Windows平台, 不支持DpiAwareness的各种模式，非Windows平台无此概念
#if defined (DUILIB_BUILD_FOR_SDL) && !defined (DUILIB_BUILD_FOR_WIN)

namespace ui
{
DpiAwareness::DpiAwareness():
    m_dpiAwarenessMode(DpiAwarenessMode::kPerMonitorDpiAware_V2)
{
}

DpiAwareness::~DpiAwareness()
{
}

bool DpiAwareness::InitDpiAwareness(DpiAwarenessMode dpiAwarenessMode)
{
    switch (dpiAwarenessMode) {
    case DpiAwarenessMode::kFromManifest:
    case DpiAwarenessMode::kPerMonitorDpiAware:
    case DpiAwarenessMode::kPerMonitorDpiAware_V2:
        m_dpiAwarenessMode = DpiAwarenessMode::kPerMonitorDpiAware_V2;
    default:
        m_dpiAwarenessMode = DpiAwarenessMode::kDpiUnaware;
    }
    return true;
}

DpiAwarenessMode DpiAwareness::SetDpiAwareness(DpiAwarenessMode /*dpiAwarenessMode*/)
{
    return m_dpiAwarenessMode;
}

DpiAwarenessMode DpiAwareness::GetDpiAwareness() const
{
    return m_dpiAwarenessMode;
}

}

#endif //DUILIB_BUILD_FOR_SDL
