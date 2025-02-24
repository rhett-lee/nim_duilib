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

bool CefClientApp::OnAlreadyRunningAppRelaunch(CefRefPtr<CefCommandLine> /*command_line*/, const CefString& /*current_directory*/)
{
    return false;
}

void CefClientApp::OnScheduleMessagePumpWork(int64_t /*delay_ms*/)
{
}

CefRefPtr<CefClient> CefClientApp::GetDefaultClient()
{
    return nullptr;
}

CefRefPtr<CefRequestContextHandler> CefClientApp::GetDefaultRequestContextHandler()
{
    return nullptr;
}

}
