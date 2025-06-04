#include "CefClientApp.h" 

#include "duilib/CEFControl/internal/CefJsHandler.h"
#include "duilib/CEFControl/internal/CefIPCStringDefs.h"
#include "duilib/CEFControl/internal/CefJSBridge.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{
//////////////////////////////////////////////////////////////////////////////////////////

void CefClientApp::OnWebKitInitialized() 
{
    /**
     * JavaScript 扩展代码，这里定义一个 NimCefWebFunction 对象提供 call 方法来让 Web 端触发 native 的 CefV8Handler 处理代码
     * param[in] functionName    要调用的 C++ 方法名称
     * param[in] params            调用该方法传递的参数，在前端指定的是一个 Object，但转到 Native 的时候转为了字符串
     * param[in] callback        执行该方法后的回调函数
     * 前端调用示例
     * NimCefWebHelper.call('showMessage', { message: 'Hello C++' }, (arguments) => {
     *    console.log(arguments)
     * })
     */
    std::string extensionCode = R"(
        var NimCefWebInstance = {};
        (() => {
            NimCefWebInstance.call = (functionName, arg1, arg2) => {
                if (typeof arg1 === 'function') {
                    native function call(functionName, arg1);
                    return call(functionName, arg1);
                } else {
                    const jsonString = JSON.stringify(arg1);
                    native function call(functionName, jsonString, arg2);
                    return call(functionName, jsonString, arg2);
                }
            };
            NimCefWebInstance.register = (functionName, callback) => {
                native function register(functionName, callback);
                return register(functionName, callback);
            };
        })();
    )";
    CefRefPtr<CefJSHandler> handler = new CefJSHandler();

    if (!m_renderJsBridge.get()) {
        m_renderJsBridge.reset(new CefJSBridge);
    }
    handler->AttachJSBridge(m_renderJsBridge);
     CefRegisterExtension("v8/extern", extensionCode, handler);
}

void CefClientApp::OnBrowserCreated(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefDictionaryValue> /*extra_info*/)
{
    if (!m_renderJsBridge.get()) {
        m_renderJsBridge.reset(new CefJSBridge);
    }
}

void CefClientApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> /*browser*/) 
{
}

CefRefPtr<CefLoadHandler> CefClientApp::GetLoadHandler()
{
    return nullptr;
}

void CefClientApp::OnContextCreated(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/, CefRefPtr<CefV8Context> /*context*/)
{

}

void CefClientApp::OnContextReleased(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> /*context*/)
{
    if (m_renderJsBridge != nullptr) {
        m_renderJsBridge->RemoveCallbackFuncWithFrame(frame);
        m_renderJsBridge->UnRegisterJSFuncWithFrame(frame);
    }
}

void CefClientApp::OnUncaughtException(CefRefPtr<CefBrowser> /*browser*/,
                                       CefRefPtr<CefFrame> /*frame*/,
                                       CefRefPtr<CefV8Context> /*context*/,
                                       CefRefPtr<CefV8Exception> /*exception*/,
                                       CefRefPtr<CefV8StackTrace> /*stackTrace*/)
{
}

void CefClientApp::OnFocusedNodeChanged(CefRefPtr<CefBrowser> /*browser*/,
                                        CefRefPtr<CefFrame> frame,
                                        CefRefPtr<CefDOMNode> node) 
{
    if (frame == nullptr){
        return;
    }
    CefDOMNode::Type type = (node != nullptr) ? node->GetType() : CefDOMNode::Type::DOM_NODE_TYPE_UNSUPPORTED;
    bool bText = (node != nullptr) ? node->IsText() : false;
    bool bEditable = (node != nullptr) ? node->IsEditable() : false;
    CefRect nodeRect = (node != nullptr) ? node->GetElementBounds() : CefRect();

    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kFocusedNodeChangedMessage);
    if (message != nullptr) {
        message->GetArgumentList()->SetInt(0, (int)type);
        message->GetArgumentList()->SetBool(1, bText);
        message->GetArgumentList()->SetBool(2, bEditable);

        message->GetArgumentList()->SetInt(3, nodeRect.x);
        message->GetArgumentList()->SetInt(4, nodeRect.y);
        message->GetArgumentList()->SetInt(5, nodeRect.width);
        message->GetArgumentList()->SetInt(6, nodeRect.height);

        frame->SendProcessMessage(PID_BROWSER, message);
    }
}

bool CefClientApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> /*frame*/,
                                            CefProcessId source_process,
                                            CefRefPtr<CefProcessMessage> message)
{
    (void)source_process;
    ASSERT(source_process == PID_BROWSER);
    ASSERT(message != nullptr);
    if (message == nullptr) {
        return false;
    }
    ASSERT(browser != nullptr);
    if (browser == nullptr) {
        return false;
    }

    // 收到 browser 的消息回复
    const CefString& message_name = message->GetName();
    if (message_name == kExecuteJsCallbackMessage) {
        int callback_id = message->GetArgumentList()->GetInt(0);
        bool has_error = message->GetArgumentList()->GetBool(1);
        CefString json_string = message->GetArgumentList()->GetString(2);

        // 将收到的参数通过管理器传递给调用时传递的回调函数
        m_renderJsBridge->ExecuteJSCallbackFunc(callback_id, has_error, json_string);
    }
    else if (message_name == kCallJsFunctionMessage) {
        CefString function_name = message->GetArgumentList()->GetString(0);
        CefString json_string = message->GetArgumentList()->GetString(1);
        int cpp_callback_id = message->GetArgumentList()->GetInt(2);
        CefString frame_id_string = message->GetArgumentList()->GetString(3);

        CefRefPtr<CefFrame> jsFrame;
        if (frame_id_string.empty()) {
            jsFrame = browser->GetMainFrame();
        }
        else {
#if CEF_VERSION_MAJOR <= 109
            //CEF 109版本
            jsFrame = browser->GetFrame(StringUtil::StringToInt64(frame_id_string.c_str()));
#else
            //CEF 高版本
            jsFrame = browser->GetFrameByIdentifier(frame_id_string);
#endif
        }

        // 通过 C++ 执行一个已经注册过的 JS 方法
        // frame_id 小于 0 则可能是 browser 进程的 browser 是无效的，所以这里为了避免出现错误就获取一个顶层 frame 执行代码
        m_renderJsBridge->ExecuteJSFunc(function_name, json_string, jsFrame, cpp_callback_id);
    }

    return false;
}
}
