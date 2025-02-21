/** @file client_app.h
  * @brief 定义Browser进程的CefApp类，管理Cef模块的生命周期
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2016/7/19
*/
#pragma once

#include "ui_components/cef_control/app/cef_js_bridge.h"
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#pragma warning (push)
#pragma warning (disable:4100)
#include "include/cef_app.h"

namespace nim_comp
{
class ClientApp : public CefApp,
                  public CefBrowserProcessHandler,
                  public CefRenderProcessHandler
{
public:
    ClientApp();

private:
    // CefApp methods.
    virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;
    virtual void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override {};
    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override{ return this; }
    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override{ return this; }

    // CefBrowserProcessHandler methods.
    virtual void OnRegisterCustomPreferences(cef_preferences_type_t type, CefRawPtr<CefPreferenceRegistrar> registrar) override {}
    virtual void OnContextInitialized() override;
    virtual void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) override {} 
    virtual bool OnAlreadyRunningAppRelaunch(CefRefPtr<CefCommandLine> command_line, const CefString& current_directory) override  { return false; }
    virtual void OnScheduleMessagePumpWork(int64_t delay_ms) override {}
    virtual CefRefPtr<CefClient> GetDefaultClient() override { return nullptr; }
    virtual CefRefPtr<CefRequestContextHandler> GetDefaultRequestContextHandler() override { return nullptr; }

    // CefRenderProcessHandler methods.
    virtual void OnWebKitInitialized() override;
    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDictionaryValue> extra_info) override;
    virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;
    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
    virtual void OnUncaughtException( CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> context,
                                      CefRefPtr<CefV8Exception> exception,
                                      CefRefPtr<CefV8StackTrace> stackTrace) override;
    virtual void OnFocusedNodeChanged(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefDOMNode> node) override;
    virtual bool OnProcessMessageReceived( CefRefPtr<CefBrowser> browser,
                                           CefRefPtr<CefFrame> frame,
                                           CefProcessId source_process,
                                           CefRefPtr<CefProcessMessage> message) override;
private:
    std::shared_ptr<CefJSBridge>    render_js_bridge_;
    bool                            last_node_is_editable_;
    // both the browser and renderer process.
    IMPLEMENT_REFCOUNTING(ClientApp);
};
}

#pragma warning (pop)
