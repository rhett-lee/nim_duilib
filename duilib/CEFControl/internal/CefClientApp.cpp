#include "CefClientApp.h" 
#include "duilib/CEFControl/CefManager.h"
#include "duilib/Utils/StringUtil.h"

#if defined (DUILIB_BUILD_FOR_LINUX) && defined (DUILIB_BUILD_FOR_SDL)
    #include "duilib/Core/MessageLoop_SDL.h"
#endif

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_cookie.h"
#include "include/cef_process_message.h"
#include "include/cef_task.h"
#include "include/cef_v8.h"
#pragma warning (pop)

#if defined (DUILIB_BUILD_FOR_LINUX) && defined (DUILIB_BUILD_FOR_SDL)
    #include <cstdlib> // 用于 getenv
#endif

namespace ui
{

#if defined (DUILIB_BUILD_FOR_LINUX) && defined (DUILIB_BUILD_FOR_SDL)
    static bool IsWaylandEnvironment()
    {
        // Wayland环境下通常会设置以下环境变量
        const char* waylandDisplay = getenv("WAYLAND_DISPLAY");
        const char* xdgSessionType = getenv("XDG_SESSION_TYPE");

        return (waylandDisplay != nullptr && *waylandDisplay != '\0') ||
               (xdgSessionType != nullptr && strcmp(xdgSessionType, "wayland") == 0);
    }
#endif

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

#if defined (DUILIB_BUILD_FOR_LINUX) && defined (DUILIB_BUILD_FOR_SDL)
        //关闭GPU的sandbox，该选项开启时，Linux平台会导致GPU进程无法正常启动
        command_line->AppendSwitch("disable-gpu-sandbox");

        DString currentVideoDriver = StringUtil::MakeLowerString(MessageLoop_SDL::GetCurrentVideoDriverName());
        //CEF的后端渲染模式，需要与SDL保持一致，否则子窗口模式无法正常工作
        if (currentVideoDriver == _T("wayland")) {
            // 强制指定 Ozone 平台为 Wayland（禁用 X11）
            command_line->AppendSwitchWithValue("ozone-platform", "wayland");
        }
        else if (currentVideoDriver == _T("x11")) {
            // 强制指定 Ozone 平台为 X11（禁用 Wayland）
            command_line->AppendSwitchWithValue("ozone-platform", "x11");
        }
     
        if (IsWaylandEnvironment() && (currentVideoDriver == _T("x11"))) {
            //XWayland环境，修复CEF页面DPI自适应功能失效的问题
            DString dpiFactor;
            float scale = MessageLoop_SDL::GetPrimaryDisplayContentScale();
            if (scale > 0.001f) {
                dpiFactor = StringUtil::Printf(_T("%.02f"), scale);
            }
            if (!dpiFactor.empty()) {
                command_line->AppendSwitchWithValue("force-device-scale-factor", CefString(dpiFactor.c_str()));
            }
        }

#endif //DUILIB_BUILD_FOR_LINUX && DUILIB_BUILD_FOR_SDL

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
