#ifndef UI_CORE_CLICK_THROUGH_H_
#define UI_CORE_CLICK_THROUGH_H_

#include "duilib/Core/UiPoint.h"

namespace ui
{
class Window;

/** 窗口点击穿透功能的实现封装
*/
class UILIB_API ClickThrough
{
public:
    ClickThrough();
    ~ClickThrough();

public:
    /** 鼠标点所在位置，进行点击穿透操作，激活鼠标点所在位置当前窗口后面的窗口
    * @param [in] pWindow 当前窗口
    * @param [in] ptMouse 鼠标点击的点（屏幕坐标）
    */
    bool ClickThroughWindow(Window* pWindow, const UiPoint& ptMouse);
};

}//namespace ui

#endif //UI_CORE_CLICK_THROUGH_H_
