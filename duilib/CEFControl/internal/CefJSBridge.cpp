#include "CefJSBridge.h"

#ifdef DUILIB_BUILD_FOR_CEF

#include "CefIPCStringDefs.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"

namespace ui {

CefJSBridge::CefJSBridge()
{
}

CefJSBridge::~CefJSBridge()
{
    m_renderRegisteredFunctions.ClearAllJsFunctions();
    m_browserRegisteredFunctions.ClearAllCppFunctions();
}

bool CefJSBridge::CallCppFunction(const CefString& function_name, const CefString& params, CefRefPtr<CefV8Value> callback)
{
    auto it = m_renderCallbackMap.find(m_jsCallbackId);
    if (it == m_renderCallbackMap.cend()) {
        CefRefPtr<CefV8Context> context = CefV8Context::GetCurrentContext();
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kCallCppFunctionMessage);

        message->GetArgumentList()->SetString(0, function_name);
        message->GetArgumentList()->SetString(1, params);
        message->GetArgumentList()->SetInt(2, m_jsCallbackId);

        m_renderCallbackMap.emplace(m_jsCallbackId++, std::make_pair(context, callback));

        // 发送消息到 browser 进程
        CefRefPtr<CefFrame> frame = context->GetFrame();
        if (frame != nullptr) {
            frame->SendProcessMessage(PID_BROWSER, message);
        }
        return true;
    }

    return false;
}

void CefJSBridge::RemoveCallbackFuncWithFrame(CefRefPtr<CefFrame> frame)
{
    if (!m_renderCallbackMap.empty()) {
        CefRefPtr<CefV8Context> frameContext = (frame != nullptr) ? frame->GetV8Context() : nullptr;
        if (frameContext == nullptr) {
            return;
        }
        for (auto it = m_renderCallbackMap.begin(); it != m_renderCallbackMap.end();)  {
            if ((it->second.first != nullptr) && it->second.first->IsSame(frameContext)) {
                it = m_renderCallbackMap.erase(it);
            }
            else {
                ++it;
            }
        }
    }
}

bool CefJSBridge::ExecuteJSCallbackFunc(int js_callback_id, bool has_error, const CefString& json_result)
{
    auto it = m_renderCallbackMap.find(js_callback_id);
    if (it != m_renderCallbackMap.cend()) {
        auto context = it->second.first;
        auto callback = it->second.second;

        if (context.get() && callback.get()) {
            context->Enter();

            CefV8ValueList arguments;

            // 第一个参数标记函数执行结果是否成功
            arguments.push_back(CefV8Value::CreateBool(has_error));

            // 第二个参数携带函数执行后返回的数据
            CefV8ValueList json_parse_args;
            json_parse_args.push_back(CefV8Value::CreateString(json_result));
            CefRefPtr<CefV8Value> json_object;
            CefRefPtr<CefV8Value> json_global = context->GetGlobal();
            if (json_global != nullptr) {
                CefRefPtr<CefV8Value> json = json_global->GetValue("JSON");
                if (json != nullptr) {
                    CefRefPtr<CefV8Value> json_parse = json->GetValue("parse");
                    if (json_parse != nullptr) {
                        json_object = json_parse->ExecuteFunction(nullptr, json_parse_args);
                    }
                }
            }
            if (json_object == nullptr) {
                //JSON解析失败时，传递null，避免将空对象传入V8导致异常
                json_object = CefV8Value::CreateNull();
            }
            arguments.push_back(json_object);

            // 执行 JS 方法
            CefRefPtr<CefV8Value> retval = callback->ExecuteFunction(nullptr, arguments);
            if (retval.get()) {
                if (retval->IsBool()) {
                    retval->GetBoolValue();
                }
            }

            context->Exit();

            // 从列表中移除 callback 缓存
            m_renderCallbackMap.erase(js_callback_id);
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

bool CefJSBridge::RegisterJSFunc(const CefString& function_name, CefRefPtr<CefV8Value> function, bool replace/* = false*/)
{
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
#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    CefString frameId = CefJSBridge::Int64ToCefString(frame->GetIdentifier());
#else
    //CEF 高版本
    CefString frameId = frame->GetIdentifier();
#endif

    return m_renderRegisteredFunctions.AddJsFunction(function_name, frameId, function, replace);
}

void CefJSBridge::UnRegisterJSFunc(const CefString& function_name, CefRefPtr<CefFrame> frame)
{
    ASSERT(frame != nullptr);
    if (frame == nullptr) {
        return;
    }
#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    CefString frameId = CefJSBridge::Int64ToCefString(frame->GetIdentifier());
#else
    //CEF 高版本
    CefString frameId = frame->GetIdentifier();
#endif
    m_renderRegisteredFunctions.RemoveJsFunction(function_name, frameId);
}

void CefJSBridge::UnRegisterJSFuncWithFrame(CefRefPtr<CefFrame> frame)
{
    m_renderRegisteredFunctions.RemoveJsFunctionByFrame(frame);
}

bool CefJSBridge::ExecuteJSFunc(const CefString& function_name, const CefString& json_params, CefRefPtr<CefFrame> frame, int cpp_callback_id)
{
    ASSERT(frame != nullptr);
    if (frame == nullptr) {
        return false;
    }
#if CEF_VERSION_MAJOR <= 109
    //CEF 109版本
    CefString frameId = CefJSBridge::Int64ToCefString(frame->GetIdentifier());
#else
    //CEF 高版本
    CefString frameId = frame->GetIdentifier();
#endif
    CefRefPtr<CefV8Value> function = m_renderRegisteredFunctions.FindJsFunction(function_name, frameId);
    if (function != nullptr) {
        auto context = frame->GetV8Context();
        if (context.get() != nullptr) {
            context->Enter();

            CefV8ValueList arguments;

            // 将 C++ 传递过来的 JSON 转换成 Object
            CefRefPtr<CefV8Value> json_parse;
            CefRefPtr<CefV8Value> json_stringify;
            CefRefPtr<CefV8Value> json_global = context->GetGlobal();
            if (json_global != nullptr) {
                CefRefPtr<CefV8Value> json = json_global->GetValue("JSON");
                if (json != nullptr) {
                    json_parse = json->GetValue("parse");
                    json_stringify = json->GetValue("stringify");
                }
            }
            CefRefPtr<CefV8Value> json_object_args;
            if (json_parse != nullptr) {
                CefV8ValueList json_parse_args;
                json_parse_args.push_back(CefV8Value::CreateString(json_params));
                json_object_args = json_parse->ExecuteFunction(nullptr, json_parse_args);
            }
            if (json_object_args == nullptr) {
                //JSON解析失败时，传递null，避免将空对象传入V8导致异常
                json_object_args = CefV8Value::CreateNull();
            }
            arguments.push_back(json_object_args);

            // 执行回调函数
            CefRefPtr<CefV8Value> retval = function->ExecuteFunction(nullptr, arguments);
            if ((retval.get() != nullptr) && retval->IsObject() && (json_stringify != nullptr)) {
                // 回复调用 JS 后的返回值
                CefV8ValueList json_stringify_args;
                json_stringify_args.push_back(retval);
                CefRefPtr<CefV8Value> json_string = json_stringify->ExecuteFunction(nullptr, json_stringify_args);
                if (json_string.get() != nullptr) {
                    //注意：GetStringValue()涉及字符串转换，只调用一次
                    CefString str = json_string->GetStringValue();

                    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kExecuteCppCallbackMessage);
                    CefRefPtr<CefListValue> args = message->GetArgumentList();
                    args->SetString(0, str);
                    args->SetInt(1, cpp_callback_id);

                    CefRefPtr<CefFrame> curFrame = context->GetFrame();
                    if (curFrame != nullptr) {
                        curFrame->SendProcessMessage(PID_BROWSER, message);
                    }
                }
            }

            context->Exit();

            return true;
        }

        return false;
    }

    return false;
}

bool CefJSBridge::CallJSFunction(const CefString& js_function_name, const CefString& params, CefRefPtr<CefFrame> frame, CallJsFunctionCallback callback)
{
    if (!frame.get()) {
        return false;
    }

    auto it = m_browserCallbackMap.find(m_cppCallbackId);
    if (it == m_browserCallbackMap.cend()) {
        m_browserCallbackMap.emplace(m_cppCallbackId, callback);
        // 发送消息给 render 要求执行一个 js function
        CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kCallJsFunctionMessage);
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        args->SetString(0, js_function_name);
        args->SetString(1, params);
        args->SetInt(2, m_cppCallbackId++);
#if CEF_VERSION_MAJOR <= 109
        //CEF 109版本
        CefString frameId = CefJSBridge::Int64ToCefString(frame->GetIdentifier());
#else
        //CEF 高版本
        CefString frameId = frame->GetIdentifier();
#endif
        args->SetString(3, frameId);

        frame->SendProcessMessage(PID_RENDERER, message);

        return true;
    }

    return false;
}

bool CefJSBridge::ExecuteCppCallbackFunc(int cpp_callback_id, const CefString& json_string)
{
    auto it = m_browserCallbackMap.find(cpp_callback_id);
    if (it != m_browserCallbackMap.cend()) {
        auto callback = it->second;
        if (callback) {
            ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [=]() { callback(json_string); });
        }
        
        // 执行完成后从缓存中移除
        m_browserCallbackMap.erase(cpp_callback_id);
    }
    return false;
}

bool CefJSBridge::RegisterCppFunc(const CefString& function_name, CppFunction function, CefRefPtr<CefBrowser> browser, bool replace /*= false*/)
{
    int64_t browser_id = browser ? browser->GetIdentifier() : -1;
    return m_browserRegisteredFunctions.AddCppFunction(function_name, browser_id, function, replace);
}

void CefJSBridge::UnRegisterCppFunc(const CefString& function_name, CefRefPtr<CefBrowser> browser)
{
    int64_t browser_id = browser ? browser->GetIdentifier() : -1;
    m_browserRegisteredFunctions.RemoveCppFunction(function_name, browser_id);
}

bool CefJSBridge::ExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    if ((frame == nullptr) && (browser != nullptr)) {
        frame = browser->GetMainFrame();
    }
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kExecuteJsCallbackMessage);
    CefRefPtr<CefListValue> args = message->GetArgumentList();

    int64_t browser_id = browser ? browser->GetIdentifier() : -1;
    CppFunction function = m_browserRegisteredFunctions.FindCppFunction(function_name, browser_id);//Browser相关函数
    if ((function == nullptr) && (browser_id != -1)) {
        function = m_browserRegisteredFunctions.FindCppFunction(function_name, -1);//全局函数
    }
    if (function != nullptr) {
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [=]() {
            function(params, [=](bool has_error, const std::string& json_result) {
                args->SetInt(0, js_callback_id);
                args->SetBool(1, has_error);
                args->SetString(2, json_result);
                if (frame != nullptr) {
                    frame->SendProcessMessage(PID_RENDERER, message);
                }                
            });
        });
        return true;
    }
    else {
        args->SetInt(0, js_callback_id);
        args->SetBool(1, true);//true表示有错误
        args->SetString(2, R"({"message":"Function does not exist."})");
        if (frame != nullptr) {
            frame->SendProcessMessage(PID_RENDERER, message);
        }
        return false;
    }
}

CefString CefJSBridge::Int64ToCefString(int64_t nValue)
{
    DString str = StringUtil::Int64ToString(nValue);
    return CefString(str);
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_CEF
