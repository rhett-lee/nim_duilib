#ifndef UI_RENDER_SKIA_SK_OFFSCREEN_GL_CONTEXT_H_
#define UI_RENDER_SKIA_SK_OFFSCREEN_GL_CONTEXT_H_

#include "duilib/duilib_config.h"

class GrDirectContext;

namespace skwindow {
    class WindowContext;
}

namespace ui
{

/** GPU离屏渲染的GL上下文管理（每个线程共享一份）
*
* 关键约束：
*  1. GL上下文是线程私有的，GPU离屏渲染对象必须在创建它的线程上使用和析构；
*  2. 同一线程内已存在窗口的GL上下文时，复用其中一个窗口的GrDirectContext（支持多窗口，后注册的窗口优先）；
*  3. 否则惰性创建一个隐藏窗口承载的后台GL上下文（1x1，不显示）；
*  4. Windows ARM64没有WGL实现，恒不支持GPU离屏渲染。
*/
class OffscreenGLContext
{
public:
    /** 当前线程是否支持GPU离屏渲染
    */
    static bool IsSupported();

    /** 获取当前线程共享的GrDirectContext
    * @return 返回GrDirectContext指针（生命周期由本类管理，调用方不应释放）；不可用时返回nullptr
    */
    static GrDirectContext* GetDirectContext();

    /** 注册当前线程的窗口GL上下文（窗口渲染对象创建成功后调用）
    */
    static void RegisterWindowContext(skwindow::WindowContext* pContext);

    /** 注销当前线程的窗口GL上下文（窗口渲染对象析构前调用）
    */
    static void UnregisterWindowContext(skwindow::WindowContext* pContext);
};

} // namespace ui

#endif // UI_RENDER_SKIA_SK_OFFSCREEN_GL_CONTEXT_H_
