#include "CefJSBridge.h"
#include "CefIPCStringDefs.h"

#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"

namespace ui {

CefJSBridge::CefJSBridge()
{
}

CefJSBridge::~CefJSBridge()
{
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
        for (auto it = m_renderCallbackMap.begin(); it != m_renderCallbackMap.end();)  {
            if (it->second.first->IsSame(frame->GetV8Context())) {
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
            CefRefPtr<CefV8Value> json_parse = context->GetGlobal()->GetValue("JSON")->GetValue("parse");
            CefRefPtr<CefV8Value> json_object = json_parse->ExecuteFunction(nullptr, json_parse_args);
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

    if (replace) {
        m_renderRegisteredFunction.emplace(std::make_pair(function_name, frameId), function);
        return true;
    }
    else {
        auto it = m_renderRegisteredFunction.find(std::make_pair(function_name, frameId));
        if (it == m_renderRegisteredFunction.cend()) {
            m_renderRegisteredFunction.emplace(std::make_pair(function_name, frameId), function);
            return true;
        }

        return false;
    }
    
    return false;
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
    m_renderRegisteredFunction.erase(std::make_pair(function_name, frameId));
}

void CefJSBridge::UnRegisterJSFuncWithFrame(CefRefPtr<CefFrame> frame)
{
    ASSERT(frame != nullptr);
    if (frame == nullptr) {
        return;
    }

    // 由于本类中每一个 render 和 browser 进程都独享一份实例，而不是单例模式
    // 所以这里获取的 browser 都是全局唯一的，可以根据这个 browser 获取所有 frame 和 context
    auto browser = frame->GetBrowser();
    ASSERT(browser != nullptr);
    if (browser == nullptr) {
        return;
    }

    if (!m_renderRegisteredFunction.empty()) {
        for (auto it = m_renderRegisteredFunction.begin(); it != m_renderRegisteredFunction.end();) {
#if CEF_VERSION_MAJOR <= 109
            //CEF 109版本
            int64 identifier = StringUtil::StringToInt64(it->first.second.c_str());
            auto child_frame = browser->GetFrame(identifier);
#else
            //CEF 高版本
            auto child_frame = browser->GetFrameByIdentifier(it->first.second);
#endif
            if (child_frame.get() && child_frame->GetV8Context()->IsSame(frame->GetV8Context())) {
                it = m_renderRegisteredFunction.erase(it);
            }
            else {
                ++it;
            }
        }
    }
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

    auto it = m_renderRegisteredFunction.find(std::make_pair(function_name, frameId));
    if (it != m_renderRegisteredFunction.cend()) {

        auto context = frame->GetV8Context();
        auto function = it->second;

        if (context.get() && function.get()) {
            context->Enter();

            CefV8ValueList arguments;

            // 将 C++ 传递过来的 JSON 转换成 Object
            CefV8ValueList json_parse_args;
            json_parse_args.push_back(CefV8Value::CreateString(json_params));
            CefRefPtr<CefV8Value> json_object = context->GetGlobal()->GetValue("JSON");
            CefRefPtr<CefV8Value> json_parse = json_object->GetValue("parse");
            CefRefPtr<CefV8Value> json_stringify = json_object->GetValue("stringify");
            CefRefPtr<CefV8Value> json_object_args = json_parse->ExecuteFunction(nullptr, json_parse_args);
            arguments.push_back(json_object_args);

            // 执行回调函数
            CefRefPtr<CefV8Value> retval = function->ExecuteFunction(nullptr, arguments);
            if (retval.get() && retval->IsObject()) {
                // 回复调用 JS 后的返回值
                CefV8ValueList json_stringify_args;
                json_stringify_args.push_back(retval);
                CefRefPtr<CefV8Value> json_string = json_stringify->ExecuteFunction(nullptr, json_stringify_args);
                CefString str = json_string->GetStringValue();

                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kExecuteCppCallbackMessage);
                CefRefPtr<CefListValue> args = message->GetArgumentList();
                args->SetString(0, json_string->GetStringValue());
                args->SetInt(1, cpp_callback_id);

                if (context->GetFrame() != nullptr) {
                    context->GetFrame()->SendProcessMessage(PID_BROWSER, message);
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
    if (replace) {
        m_browserRegisteredFunction.emplace(std::make_pair(function_name, browser ? browser->GetIdentifier() : -1), function);// = ;
        return true;
    }
    else {
        auto it = m_browserRegisteredFunction.find(std::make_pair(function_name, browser ? browser->GetIdentifier() : -1));
        if (it == m_browserRegisteredFunction.cend()) {
            m_browserRegisteredFunction.emplace(std::make_pair(function_name, browser ? browser->GetIdentifier() : -1), function);
            return true;
        }
        return false;
    }

    return false;
}

void CefJSBridge::UnRegisterCppFunc(const CefString& function_name, CefRefPtr<CefBrowser> browser)
{
    m_browserRegisteredFunction.erase(std::make_pair(function_name, browser ? browser->GetIdentifier() : -1));
}

bool CefJSBridge::ExecuteCppFunc(const CefString& function_name, const CefString& params, int js_callback_id, CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
    if ((frame == nullptr) && (browser != nullptr)) {
        frame = browser->GetMainFrame();
    }
    CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(kExecuteJsCallbackMessage);
    CefRefPtr<CefListValue> args = message->GetArgumentList();

    auto it = m_browserRegisteredFunction.find(std::make_pair(function_name, browser->GetIdentifier()));
    if (it != m_browserRegisteredFunction.cend()) {
        auto function = it->second;
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [=]() {
            function(params, [=](bool has_error, const std::string& json_result) {
                // 测试代码，需要封装到管理器中
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

    it = m_browserRegisteredFunction.find(std::make_pair(function_name, -1));
    if (it != m_browserRegisteredFunction.cend()) {
        auto function = it->second;
        ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI, [=]() {
            function(params, [=](bool has_error, const std::string& json_result) {
                // 测试代码，需要封装到管理器中
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
        args->SetBool(1, true);
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

}
