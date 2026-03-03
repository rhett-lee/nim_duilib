#include "CefRegisteredFunctions.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{
bool RenderRegisteredFunctions::AddJsFunction(const CefString& function_name, const CefString& frame_id,
                                              const CefRefPtr<CefV8Value>& function, bool enable_replace)
{
    ASSERT(!function_name.empty() && !frame_id.empty() && (function != nullptr));
    if (function_name.empty() || frame_id.empty() || (function == nullptr)) {
        return false;
    }
    bool bRet = true;
    FrameFunctionMap& frameFunctionMap = m_renderJsFunctionMap[function_name];
    auto iter = frameFunctionMap.find(frame_id);
    if (iter != frameFunctionMap.end()) {
        //已经存在
        if (enable_replace) {
            frameFunctionMap[frame_id] = function; //允许覆盖
        }
        else {
            bRet = false;//不允许覆盖
        }
    }
    else {
        //不存在, 添加
        frameFunctionMap[frame_id] = function;
    }
    return bRet;
}

CefRefPtr<CefV8Value> RenderRegisteredFunctions::FindJsFunction(const CefString& function_name, const CefString& frame_id)
{
    CefRefPtr<CefV8Value> function = nullptr;
    auto iter = m_renderJsFunctionMap.find(function_name);
    if (iter != m_renderJsFunctionMap.end()) {
        const FrameFunctionMap& frameFunctionMap = iter->second;
        auto pos = frameFunctionMap.find(frame_id);
        if (pos != frameFunctionMap.end()) {
            function = pos->second;
        }
    }
    return function;
}

void RenderRegisteredFunctions::RemoveJsFunction(const CefString& function_name)
{
    auto iter = m_renderJsFunctionMap.find(function_name);
    if (iter != m_renderJsFunctionMap.end()) {
        m_renderJsFunctionMap.erase(iter);
    }
}

void RenderRegisteredFunctions::RemoveJsFunction(const CefString& function_name, const CefString& frame_id)
{
    auto iter = m_renderJsFunctionMap.find(function_name);
    if (iter != m_renderJsFunctionMap.end()) {
        FrameFunctionMap& frameFunctionMap = iter->second;
        auto pos = frameFunctionMap.find(frame_id);
        if (pos != frameFunctionMap.end()) {
            frameFunctionMap.erase(pos);
        }
        if (frameFunctionMap.empty()) {
            m_renderJsFunctionMap.erase(iter);
        }
    }
}

void RenderRegisteredFunctions::RemoveJsFunctionByFrameId(const CefString& frame_id)
{
    auto iter = m_renderJsFunctionMap.begin();
    while (iter != m_renderJsFunctionMap.end()) {
        FrameFunctionMap& frameFunctionMap = iter->second;
        auto pos = frameFunctionMap.find(frame_id);
        if (pos != frameFunctionMap.end()) {
            frameFunctionMap.erase(pos);
        }
        if (frameFunctionMap.empty()) {
            iter = m_renderJsFunctionMap.erase(iter);
        }
        else {
            ++iter;
        }
    }
}

void RenderRegisteredFunctions::RemoveJsFunctionByFrame(CefRefPtr<CefFrame> frame)
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

    if (!m_renderJsFunctionMap.empty()) {
        for (auto iter = m_renderJsFunctionMap.begin(); iter != m_renderJsFunctionMap.end();) {
            FrameFunctionMap& frameFunctionMap = iter->second;            
            for (auto pos = frameFunctionMap.begin(); pos != frameFunctionMap.end();) {
#if CEF_VERSION_MAJOR <= 109
                //CEF 109版本
                int64 identifier = StringUtil::StringToInt64(pos->first.c_str());
                auto child_frame = browser->GetFrame(identifier);
#else
                //CEF 高版本
                auto child_frame = browser->GetFrameByIdentifier(pos->first);
#endif
                if (child_frame.get() && child_frame->GetV8Context()->IsSame(frame->GetV8Context())) {
                    pos = frameFunctionMap.erase(pos);
                }
                else {
                    ++pos;
                }
            }

            if (frameFunctionMap.empty()) {
                iter = m_renderJsFunctionMap.erase(iter);
            }
            else {
                ++iter;
            }
        }
    }
}

void RenderRegisteredFunctions::ClearAllJsFunctions()
{
    m_renderJsFunctionMap.clear();
}

/////////////////////////////////////////////////////////////
bool BrowserRegisteredFunctions::AddCppFunction(const CefString& function_name, int64_t browser_id,
                                                CppFunction function, bool enable_replace)
{
    ASSERT(!function_name.empty() && (function != nullptr));
    if (function_name.empty() || (function == nullptr)) {
        return false;
    }
    bool bRet = true;
    BrowserFunctionMap& browserFunctionMap = m_browserCppFunctionMap[function_name];
    auto iter = browserFunctionMap.find(browser_id);
    if (iter != browserFunctionMap.end()) {
        //已经存在
        if (enable_replace) {
            browserFunctionMap[browser_id] = function; //允许覆盖
        }
        else {
            bRet = false;//不允许覆盖
        }
    }
    else {
        //不存在, 添加
        browserFunctionMap[browser_id] = function;
    }
    return bRet;
}

CppFunction BrowserRegisteredFunctions::FindCppFunction(const CefString& function_name, int64_t browser_id)
{
    CppFunction function = nullptr;
    auto iter = m_browserCppFunctionMap.find(function_name);
    if (iter != m_browserCppFunctionMap.end()) {
        const BrowserFunctionMap& browserFunctionMap = iter->second;
        auto pos = browserFunctionMap.find(browser_id);
        if (pos != browserFunctionMap.end()) {
            function = pos->second;
        }
    }
    return function;
}

void BrowserRegisteredFunctions::RemoveCppFunction(const CefString& function_name)
{
    auto iter = m_browserCppFunctionMap.find(function_name);
    if (iter != m_browserCppFunctionMap.end()) {
        m_browserCppFunctionMap.erase(iter);
    }
}

void BrowserRegisteredFunctions::RemoveCppFunction(const CefString& function_name, int64_t browser_id)
{
    auto iter = m_browserCppFunctionMap.find(function_name);
    if (iter != m_browserCppFunctionMap.end()) {
        BrowserFunctionMap& browserFunctionMap = iter->second;
        auto pos = browserFunctionMap.find(browser_id);
        if (pos != browserFunctionMap.end()) {
            browserFunctionMap.erase(pos);
        }
        if (browserFunctionMap.empty()) {
            m_browserCppFunctionMap.erase(iter);
        }
    }
}

void BrowserRegisteredFunctions::ClearAllCppFunctions()
{
    m_browserCppFunctionMap.clear();
}

} //namespace ui
