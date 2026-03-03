#ifndef UI_CEF_CONTROL_CEF_REGISTERED_FUNCTIONS_H_
#define UI_CEF_CONTROL_CEF_REGISTERED_FUNCTIONS_H_

#include "duilib/CEFControl/CefControlEvent.h"

#pragma warning (push)
#pragma warning (disable:4100)
    #include "include/cef_v8.h"
    #include "include/cef_version.h"
#pragma warning (pop)

#include <functional>
#include <map>

namespace ui
{
/** Render：注册一个JS函数
*/
class RenderRegisteredFunctions
{
public:
    /** 添加一个JS函数
     * @param [in] function_name 函数名称
     * @param [in] frame_id CefFrame对象的ID
     * @param [in] function 注册的JS函数
     * @param [in] enable_replace 是否允许覆盖
     */
    bool AddJsFunction(const CefString& function_name, const CefString& frame_id,
                       const CefRefPtr<CefV8Value>& function, bool enable_replace);

    /** 查找一个已经注册的JS函数
     * @param [in] function_name 函数名称
     * @param [in] frame_id CefFrame对象的ID
     */
    CefRefPtr<CefV8Value> FindJsFunction(const CefString& function_name, const CefString& frame_id);

    /** 删除已经注册JS函数
    * @param [in] function_name 函数名称
    */
    void RemoveJsFunction(const CefString& function_name);

    /** 删除已经注册JS函数
    * @param [in] function_name 函数名称
    * @param [in] frame_id CefFrame对象的ID
    */
    void RemoveJsFunction(const CefString& function_name, const CefString& frame_id);

    /** 删除已经注册JS函数
    * @param [in] frame_id CefFrame对象的ID
    */
    void RemoveJsFunctionByFrameId(const CefString& frame_id);

    /** 删除已经注册JS函数
    * @param [in] frame CefFrame对象
    */
    void RemoveJsFunctionByFrame(CefRefPtr<CefFrame> frame);

    /** 清除所有的已注册函数
    */
    void ClearAllJsFunctions();

private:
    typedef std::map<CefString/* frame_id*/, CefRefPtr<CefV8Value>/* function*/> FrameFunctionMap;
    typedef std::map<CefString/* function_name*/, FrameFunctionMap> RenderRegisteredFunctionMap;

    //函数映射表
    RenderRegisteredFunctionMap m_renderJsFunctionMap;
};

/** Browser：注册一个Cpp函数
*/
class BrowserRegisteredFunctions
{
public:
    /** 添加一个JS函数
     * @param [in] function_name 函数名称
     * @param [in] browser_id Browser对象的ID
     * @param [in] function 注册的Cpp函数
     * @param [in] enable_replace 是否允许覆盖
     */
    bool AddCppFunction(const CefString& function_name, int64_t browser_id,
                        CppFunction function, bool enable_replace);

    /** 查找一个已经注册的Cpp函数
     * @param [in] function_name 函数名称
     * @param [in] browser_id Browser对象的ID
     */
    CppFunction FindCppFunction(const CefString& function_name, int64_t browser_id);

    /** 删除已经注册Cpp函数
    * @param [in] function_name 函数名称
    */
    void RemoveCppFunction(const CefString& function_name);

    /** 删除已经注册Cpp函数
    * @param [in] function_name 函数名称
    * @param [in] browser_id Browser对象的ID
    */
    void RemoveCppFunction(const CefString& function_name, int64_t browser_id);

    /** 清除所有的已注册函数
    */
    void ClearAllCppFunctions();

private:
    typedef std::map<int64_t/*browser_id*/, CppFunction/* function*/> BrowserFunctionMap;
    typedef std::map<CefString/* function_name*/, BrowserFunctionMap> BrowserRegisteredFunctionMap;

    //函数映射表
    BrowserRegisteredFunctionMap m_browserCppFunctionMap;
};

}//namespace ui

#endif //UI_CEF_CONTROL_CEF_REGISTERED_FUNCTIONS_H_
