#include "CefJsHandler.h"
#include "duilib/CEFControl/internal/CefIPCStringDefs.h"
#include "duilib/CEFControl/internal/CefJSBridge.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

bool CefJSHandler::Execute(const CefString& name, CefRefPtr<CefV8Value> /*object*/, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& /*retval*/, CefString& exception)
{
    // 当Web中调用了"NimCefWebFunction"函数后，会触发到这里，然后把参数保存，转发到Broswer进程
    // Broswer进程的CefBrowserHandler类在OnProcessMessageReceived接口中处理kJsCallbackMessage消息，就可以收到这个消息

    if (arguments.size() < 2) {
        exception = "Invalid arguments.";
        return false;
    }

    CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
    ASSERT(context != nullptr);
    if (context == nullptr) {
        return false;
    }

    CefRefPtr<CefFrame> frame = context->GetFrame();
    ASSERT(frame != nullptr);
    if (frame == nullptr) {
        return false;
    }

    CefRefPtr<CefBrowser> browser = context->GetBrowser();
    ASSERT(browser != nullptr);
    if (browser == nullptr) {
        return false;
    }

    int64_t browserId = browser->GetIdentifier();
#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    CefString frameId = CefJSBridge::Int64ToCefString(frame->GetIdentifier());
#else
    //CEF 高版本
    CefString frameId = frame->GetIdentifier();
#endif
    (void)browserId;
    (void)frameId;

    if (name == "call") {
        // 允许没有参数列表的调用，第二个参数为回调
        // 如果传递了参数列表，那么回调是第三个参数
        CefString function_name = arguments[0]->GetStringValue();
        CefString params = "{}";
        CefRefPtr<CefV8Value> callback;
        if (arguments[0]->IsString() && arguments[1]->IsFunction()) {
            callback = arguments[1];
        }
        else if (arguments[0]->IsString() && arguments[1]->IsString() && arguments[2]->IsFunction()) {
            params = arguments[1]->GetStringValue();
            callback = arguments[2];
        }
        else {
            exception = "Invalid arguments.";
            return false;
        }

        // 执行 C++ 方法
        if (!m_jsBridge->CallCppFunction(function_name, params, callback)) {
            exception = ui::StringUtil::Printf("Failed to call function %s.", function_name.c_str()).c_str();
            return false;
        }
        return true;
    }
    else if (name == "register") {
        if (arguments[0]->IsString() && arguments[1]->IsFunction())
        {
            std::string function_name = arguments[0]->GetStringValue();
            CefRefPtr<CefV8Value> callback = arguments[1];
            if (!m_jsBridge->RegisterJSFunc(function_name, callback)) {
                exception = "Failed to register function.";
                return false;
            }
            return true;
        }
        else {
            exception = "Invalid arguments.";
            return false;
        }
    }

    return false;
}

}
