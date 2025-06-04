#include "CefManager_Linux.h"

#ifdef DUILIB_BUILD_FOR_LINUX

#include "duilib/CEFControl/internal/CefClientApp.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"

namespace ui
{
CefManager_Linux::CefManager_Linux()
{
}

CefManager_Linux::~CefManager_Linux()
{
}

DString CefManager_Linux::GetCefMoudlePath() const
{
    DString cefMoudlePath = BaseClass::GetCefMoudlePath();
    if (cefMoudlePath.empty()) {
        //使用默认规则
        cefMoudlePath = _T("libcef_linux");
    }
    return cefMoudlePath;
}

bool CefManager_Linux::Initialize(bool bEnableOffScreenRendering,
                                  const DString& appName,
                                  int argc,
                                  char** argv,
                                  OnCefSettingsEvent callback)
{
    if (!BaseClass::Initialize(bEnableOffScreenRendering, appName, argc, argv, callback)) {
        return false;
    }

    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefClientApp> app(new CefClientApp);

    // 如果是在子进程中调用，会堵塞直到子进程退出，并且exit_code返回大于等于0
    // 如果在Browser进程中调用，则立即返回-1
    int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
    if (exit_code >= 0) {
        return false;
    }

    CefSettings settings;
    GetCefSetting(settings);

    bool bRet = CefInitialize(main_args, settings, app.get(), nullptr);
    if (!bRet) {
        return false;
    }
    return true;
}

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_LINUX)
