#include "CefClientApp.h" 

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
bool CefClientApp::OnAlreadyRunningAppRelaunch(CefRefPtr<CefCommandLine> /*command_line*/, const CefString& /*current_directory*/)
{
    return false;
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
