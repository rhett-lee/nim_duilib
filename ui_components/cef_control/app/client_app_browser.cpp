#include "client_app.h" 

#include "ui_components/cef_control/util/util.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_cookie.h"
#include "include/cef_process_message.h"
#include "include/cef_task.h"
#include "include/cef_v8.h"
#pragma warning (pop)

namespace nim_comp
{
//////////////////////////////////////////////////////////////////////////////////////////
// CefBrowserProcessHandler methods.
void ClientApp::OnContextInitialized() 
{
	// Register cookieable schemes with the global cookie manager.
	CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
	ASSERT(manager.get());
	manager->SetSupportedSchemes(cookieable_schemes_, NULL);

	// 这里可以删除了保存的Cooies信息
	// manager->DeleteCookies(L"", L"", nullptr);
}

void ClientApp::OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line)
{
}

void ClientApp::OnRenderProcessThreadCreated(CefRefPtr<CefListValue> extra_info) 
{
}

}