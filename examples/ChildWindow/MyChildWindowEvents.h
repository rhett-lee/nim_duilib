#ifndef EXAMPLES_MY_CHILD_WINDOW_EVENTS_H_
#define EXAMPLES_MY_CHILD_WINDOW_EVENTS_H_

// duilib
#include "duilib/duilib.h"

#include <chrono>
#include <memory>

//绘制的FPS计算工具
class FPSCounter;

//窗口绘制的实现
class ChildWindowPaint;

//窗口绘制管理器
class ChildWindowPaintScheduler;

/** 子窗口的事件接口
 */
class MyChildWindowEvents : public ui::ChildWindowEvents
{
public:
    MyChildWindowEvents(ui::ChildWindow* pChildWindow, size_t nChildWindowIndex, ChildWindowPaintScheduler* pPaintScheduler);
    virtual ~MyChildWindowEvents() override = default;

    /** 获取关联的子窗口
    */
    ui::ChildWindow* GetChildWindow() const;

    /** 当前是否处于持续绘制的状态
    */
    bool IsPaintFps() const;

protected:
    /** 窗口绘制(SDL_EVENT_WINDOW_EXPOSED/WM_PAINT)
     * @param [in] rcPaint 本次绘制，需要更新的矩形区域
     * @param [in] nativeMsg 从系统接收到的原始消息内容
     *             SDL实现：nativeMsg.uMsg值为SDL_EVENT_WINDOW_EXPOSED，nativeMsg.wParam的值为SDL_Window*指针
     *             Windows实现：nativeMsg.uMsg值为WM_PAINT，nativeMsg.wParam的值为窗口的HWND句柄
     * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
     * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
     */
    virtual LRESULT OnPaintMsg(const ui::UiRect& rcPaint, const ui::NativeMsg& nativeMsg, bool& bHandled) override;

private:
    //关联的子窗口
    ui::ControlPtrT<ui::ChildWindow> m_pChildWindow;

    //关联的绘制实现对象
    std::unique_ptr<ChildWindowPaint> m_childWindowPaint;

    //窗口绘制管理器
    ChildWindowPaintScheduler* m_pPaintScheduler;

    //FPS计数器
    std::unique_ptr<FPSCounter> m_fps;

    //上次显示FPS的时间
    std::chrono::high_resolution_clock::time_point m_lastPaintTime;

    //FPS显示控件
    ui::ControlPtrT<ui::Label> m_pFpsLabel;

    //是否开始动态绘制
    bool m_bPaintFps;
};

#endif // EXAMPLES_MY_CHILD_WINDOW_EVENTS_H_
