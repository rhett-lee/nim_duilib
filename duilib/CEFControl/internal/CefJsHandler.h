/**@brief 定义一个C++与JS进行交互的JsHandler类
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @author Redrain
 * @date 2016/7/19
 */

#ifndef UI_CEF_CONTROL_JS_HANDLER_H_
#define UI_CEF_CONTROL_JS_HANDLER_H_

#include "duilib/duilib_config.h"
#include "duilib/CEFControl/internal/CefJSBridge.h"

namespace ui
{
    
class CefJSHandler : public CefV8Handler
{
public:
    CefJSHandler() {}
    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override;
    void AttachJSBridge(std::shared_ptr<CefJSBridge> js_bridge) { m_jsBridge = js_bridge; }

    IMPLEMENT_REFCOUNTING(CefJSHandler);

private:
    std::shared_ptr<CefJSBridge> m_jsBridge;
};
}

#endif //UI_CEF_CONTROL_JS_HANDLER_H_
