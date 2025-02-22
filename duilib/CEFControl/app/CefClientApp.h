/** @brief 定义Browser进程的CefApp类，管理Cef模块的生命周期
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2016/7/19
*/
#ifndef UI_CEF_CONTROL_CLIENT_APP_H_
#define UI_CEF_CONTROL_CLIENT_APP_H_

#include "duilib/duilib_config.h"
#include "duilib/CEFControl/app/CefJSBridge.h"
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_app.h"

namespace ui
{
class CefClientApp:
    public CefApp,
    public CefBrowserProcessHandler,
    public CefRenderProcessHandler
{
public:
    CefClientApp();

private:
    // CefApp接口的实现
    virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;
    virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override;
    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override;
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

    // CefBrowserProcessHandler接口的实现
    virtual void OnRegisterCustomPreferences(cef_preferences_type_t type, CefRawPtr<CefPreferenceRegistrar> registrar) override;
    virtual void OnContextInitialized() override;
    virtual void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) override;
    virtual bool OnAlreadyRunningAppRelaunch(CefRefPtr<CefCommandLine> command_line, const CefString& current_directory) override;
    virtual void OnScheduleMessagePumpWork(int64_t delay_ms) override;
    virtual CefRefPtr<CefClient> GetDefaultClient() override;
    virtual CefRefPtr<CefRequestContextHandler> GetDefaultRequestContextHandler() override;

    // CefRenderProcessHandler接口的实现
    virtual void OnWebKitInitialized() override;
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDictionaryValue> extra_info) override;
    virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    virtual void OnUncaughtException(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> context,
                                     CefRefPtr<CefV8Exception> exception,
                                     CefRefPtr<CefV8StackTrace> stackTrace) override;
    virtual void OnFocusedNodeChanged(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefDOMNode> node) override;
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefRefPtr<CefFrame> frame,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;
private:
    std::shared_ptr<CefJSBridge>    render_js_bridge_;
    bool                            last_node_is_editable_;
    // both the browser and renderer process.
    IMPLEMENT_REFCOUNTING(CefClientApp);
};
}

#pragma warning (pop)

#endif //UI_CEF_CONTROL_CLIENT_APP_H_
