#include "CefClientApp.h" 
#include "duilib/CEFControl/CefManager.h"
#include "duilib/Core/GlobalManager.h"

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

void CefClientApp::OnScheduleMessagePumpWork(int64_t delay_ms)
{
    if (CefManager::GetInstance()->IsMultiThreadedMessageLoop()) {
        return;
    }

    if (GlobalManager::Instance().Thread().IsMainThreadExit()) {
        //主线程已经退出，正在退出
        if (CefManager::GetInstance()->IsCefInited()) {
            CefDoMessageLoopWork();
        }
    }
    else {
        // delay_ms 表示下次处理的建议延迟（毫秒）
        if (delay_ms <= 0) {
            // 立即触发消息处理
            GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, []() {
                    // 执行单次 CEF 消息处理
                    if (CefManager::GetInstance()->IsCefInited()) {
                        CefDoMessageLoopWork();
                    }
                });
        }
        else {
            // 设置延迟触发
            GlobalManager::Instance().Thread().PostDelayedTask(ui::kThreadUI, []() {
                    // 执行单次 CEF 消息处理
                    if (CefManager::GetInstance()->IsCefInited()) {
                        CefDoMessageLoopWork();
                    }
                }, (int32_t)delay_ms);
        }
    }
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
