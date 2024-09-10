#include "DpiAwareness.h"

//该文件的实现仅限非Windows平台，不支持Awareness相关的逻辑
#ifndef DUILIB_BUILD_FOR_WIN

namespace ui
{
DpiInitParam::DpiInitParam() :
    m_dpiAwarenessFlag(DpiInitParam::DpiAwarenessFlag::kFromUserDefine),
    m_dpiAwarenessMode(DpiAwarenessMode::kSystemDpiAware),
    m_uDPI(0)
{
}

DpiAwareness::DpiAwareness()
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

DpiAwarenessMode DpiAwareness::SetDpiAwareness(DpiAwarenessMode /*dpiAwarenessMode*/)
{
    return DpiAwarenessMode::kSystemDpiAware;
}

DpiAwarenessMode DpiAwareness::GetDpiAwareness() const
{
    return DpiAwarenessMode::kSystemDpiAware;
}

}

#endif //DUILIB_BUILD_FOR_WIN
