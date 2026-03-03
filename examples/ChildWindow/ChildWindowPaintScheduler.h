#ifndef EXAMPLES_CHILD_WINDOW_PAINT_SCHEDULER_H_
#define EXAMPLES_CHILD_WINDOW_PAINT_SCHEDULER_H_

// duilib
#include "duilib/duilib.h"

/** 子窗口的绘制管理接口（用于确定绘制哪个子窗口）
*/
class ChildWindowPaintScheduler
{
public:
    virtual ~ChildWindowPaintScheduler() = default;

    /** 立即绘制该子窗口
    * @param [in] pChildWindow 子窗口的指针
    */
    virtual bool PaintChildWindow(ui::ChildWindow* pChildWindow) = 0;

    /** 立即绘制该子窗口的下一个子窗口（按顺序依次绘制每个子窗口）
    * @param [in] pChildWindow 子窗口的指针
    */
    virtual bool PaintNextChildWindow(ui::ChildWindow* pChildWindow) = 0;
};

#endif // EXAMPLES_CHILD_WINDOW_PAINT_SCHEDULER_H_
