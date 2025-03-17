#include "CefClientApp.h" 
#include "duilib/CEFControl/CefManager.h"

namespace ui
{
//////////////////////////////////////////////////////////////////////////////////////////
// CefBrowserProcessHandler methods.
void CefClientApp::OnRegisterCustomPreferences(cef_preferences_type_t /*type*/, CefRawPtr<CefPreferenceRegistrar> /*registrar*/)
{
}

void CefClientApp::OnContextInitialized()
{
}

void CefClientApp::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> /*command_line*/)
{
}

#if CEF_VERSION_MAJOR > 109
//CEF 高版本
bool CefClientApp::OnAlreadyRunningAppRelaunch(CefRefPtr<CefCommandLine> command_line, const CefString& /*current_directory*/)
{
    //又启动了一个Browser进程，需要保持进程单例模式
    OnAlreadyRunningAppRelaunchEvent pfnAlreadyRunningAppRelaunch = CefManager::GetInstance()->GetAlreadyRunningAppRelaunch();
    if (pfnAlreadyRunningAppRelaunch != nullptr) {
        std::vector<DString> argumentList;
        if (command_line != nullptr) {
            CefCommandLine::ArgumentList arguments;
            command_line->GetArguments(arguments);
            for (const CefString& arg : arguments) {
                argumentList.push_back(arg);
            }
        }
        pfnAlreadyRunningAppRelaunch(argumentList);
    }
    //返回true表示拦截
    return true;
}
#endif

void CefClientApp::OnScheduleMessagePumpWork(int64_t /*delay_ms*/)
{
}

CefRefPtr<CefClient> CefClientApp::GetDefaultClient()
{
    return nullptr;
}

#if CEF_VERSION_MAJOR > 109
//CEF 高版本
CefRefPtr<CefRequestContextHandler> CefClientApp::GetDefaultRequestContextHandler()
{
    return nullptr;
}
#endif

}
