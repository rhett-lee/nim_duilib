#include "CefClientApp.h" 
#include "duilib/CEFControl/CefManager.h"

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_cookie.h"
#include "include/cef_process_message.h"
#include "include/cef_task.h"
#include "include/cef_v8.h"
#pragma warning (pop)

#include <string>

namespace ui
{

CefClientApp::CefClientApp()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
// CefApp methods.
void CefClientApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
    // Pass additional command-line flags to the browser process.
    if (process_type.empty()) {

        //同一个域下的使用同一个渲染进程
        command_line->AppendSwitch("process-per-site");
        command_line->AppendSwitch("disable-gpu");
        command_line->AppendSwitch("disable-gpu-compositing");
        //command_line->AppendSwitchWithValue("proxy-server", "SOCKS5://127.0.0.1:1080");    

        // 开启离屏渲染
        if (CefManager::GetInstance()->IsEnableOffScreenRendering()) {
            command_line->AppendSwitch("disable-surfaces");
            command_line->AppendSwitch("enable-begin-frame-scheduling");
        }
    }
}

void CefClientApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> /*registrar*/)
{
}

CefRefPtr<CefBrowserProcessHandler> CefClientApp::GetBrowserProcessHandler()
{
    return this;
}

CefRefPtr<CefRenderProcessHandler> CefClientApp::GetRenderProcessHandler()
{
    return this;
}

}
