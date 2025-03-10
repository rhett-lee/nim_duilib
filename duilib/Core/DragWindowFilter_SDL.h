#ifndef UI_CORE_DRAG_WINDOW_FILTER_H_
#define UI_CORE_DRAG_WINDOW_FILTER_H_

#include "duilib/Core/Window.h"

#ifdef DUILIB_BUILD_FOR_SDL

namespace ui
{
/** 窗口消息过滤接口，用于转接鼠标点击消息
*/
class DragWindowFilter : public IUIMessageFilter
{
public:
    DragWindowFilter(Window* pOwner, Window* pWindow);

    /**  消息处理函数，处理优先级高于Window类的消息处理函数
    * @param [in] uMsg 消息内容
    * @param [in] wParam 消息附加参数
    * @param [in] lParam 消息附加参数
    * @param[out] bHandled 返回 false 则继续派发该消息，返回 true 表示不再派发该消息
    * @return 返回消息处理结果
    */
    virtual LRESULT FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

private:
    /** 当前窗口
    */
    Window* m_pWindow;

    /** 父窗口
    */
    Window* m_pOwner;
};
}

#endif //DUILIB_BUILD_FOR_SDL

#endif // UI_CORE_DRAG_WINDOW_FILTER_H_
