#ifndef EXAMPLES_CHILD_WINDOW_PAINT_H_
#define EXAMPLES_CHILD_WINDOW_PAINT_H_

#include <chrono>

// duilib
#include "duilib/duilib.h"

/** 子窗口的绘制实现
*/
class ChildWindowPaint
{
public:
    ChildWindowPaint(ui::ChildWindow* pChildWindow, size_t nChildWindowIndex);
    ~ChildWindowPaint();

public:
    /** 窗口绘制(SDL_EVENT_WINDOW_EXPOSED/WM_PAINT)
     * @param [in] rcPaint 本次绘制，需要更新的矩形区域
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     *             SDL实现：nativeMsg.uMsg值为SDL_EVENT_WINDOW_EXPOSED，nativeMsg.wParam的值为SDL_Window*指针
     *             Windows实现：nativeMsg.uMsg值为WM_PAINT，nativeMsg.wParam的值为窗口的HWND句柄
     * @param [in] bPaintFps 当前是否处于动态绘制状态中(用于演示子窗口绘制功能)
     * @return 返回true表示本次真正完成了一帧绘制；返回false表示没有绘制（用于统计真实绘制帧率）
     */
    bool PaintChildWindow(const ui::UiRect& rcPaint, const ui::NativeMsg& nativeMsg, bool bPaintFps);

private:
    //关联的子窗口
    ui::ChildWindow* m_pChildWindow;

    //子窗口的序号（用于错开各窗口的动画相位）
    size_t m_nChildWindowIndex;

    //动画起始时间（用于计算动画相位）
    std::chrono::steady_clock::time_point m_startTime;

    //内部实现类
    struct TImpl;
    TImpl* m_impl;
};

#endif // EXAMPLES_CHILD_WINDOW_PAINT_H_
