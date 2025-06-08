#include "CefManager_MacOS.h"

#ifdef DUILIB_BUILD_FOR_MACOS

#include "duilib/CEFControl/internal/CefClientApp.h"
#include "duilib/CEFControl/internal/CefBrowserHandler.h"

#include "include/wrapper/cef_library_loader.h"

namespace ui
{
CefManager_MacOS::CefManager_MacOS()
{
}

CefManager_MacOS::~CefManager_MacOS()
{
}

bool CefManager_MacOS::Initialize(bool bEnableOffScreenRendering,
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

    CefSettings settings;
    GetCefSetting(settings);

    bool bRet = CefInitialize(main_args, settings, app.get(), nullptr);
    if (!bRet) {
        return false;
    }
    return true;
}

bool CefManager_MacOS::IsMultiThreadedMessageLoop() const
{
    return false;
}

} //namespace ui

#endif //defined (DUILIB_BUILD_FOR_MACOS)
