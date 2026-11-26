/**@brief 定义一个C++与JS进行交互的JsHandler类
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/7/19
 */

#ifndef UI_CEF_CONTROL_JS_HANDLER_H_
#define UI_CEF_CONTROL_JS_HANDLER_H_

#include "duilib/duilib_config.h"

#ifdef DUILIB_BUILD_FOR_CEF

#include "duilib/CEFControl/internal/CefJSBridge.h"

namespace ui
{
    
/** C++ 与 JS 交互的 JS 函数处理器（对应 CefV8Handler 接口）
*   网页中调用注册的 C++ 函数时，会进入本类的 Execute 函数，
*   由本类把函数名与参数转发到 Browser 进程的 CefBrowserHandler 处理。
*/
class CefJSHandler : public CefV8Handler
{
public:
    CefJSHandler() {}

    /** 执行网页中调用的 C++ 函数
    * @param [in] name 被调用的函数名称
    * @param [in] object 函数所属的对象（当前实现未使用）
    * @param [in] arguments 调用时传入的参数列表
    * @param [out] retval 函数返回值（当前实现未使用）
    * @param [out] exception 执行失败时返回的异常信息
    * @return 返回 true 表示执行成功，false 表示执行失败
    */
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override;

    /** 关联 JS 桥接接口
    * @param [in] js_bridge JS 桥接接口
    */
    void AttachJSBridge(std::shared_ptr<CefJSBridge> js_bridge) { m_jsBridge = js_bridge; }

    // CEF 引用计数的宏实现
    IMPLEMENT_REFCOUNTING(CefJSHandler);

private:
    std::shared_ptr<CefJSBridge> m_jsBridge;
};
}

#endif //DUILIB_BUILD_FOR_CEF

#endif //UI_CEF_CONTROL_JS_HANDLER_H_
