#ifndef DUILIB_WINDOW_ANCHOR_FOLLOWER_H_
#define DUILIB_WINDOW_ANCHOR_FOLLOWER_H_

#include <functional>
#include <cstddef>
#include <utility>
#include "duilib/Core/UiTypes.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{
/** 浮层窗口的锚点跟随器
*
*   用途：下拉框、浮层等窗口在打开期间，需要跟随宿主控件移动：
*       1. 宿主窗口移动时，宿主控件的屏幕位置发生变化；
*       2. 宿主控件所在的滚动容器滚动时，控件在屏幕上的位置发生变化。
*   以上两种情况都不会触发现有的事件通知，因此按固定周期检查锚点的屏幕矩形，
*   发现位置变化时调用回调函数重新布局浮层窗口。
*/
class WindowAnchorFollower
{
public:
    /** 获取锚点当前屏幕矩形的函数
    */
    using AnchorRectGetter = std::function<UiRect()>;

    /** 锚点位置变化时，更新浮层窗口位置的函数
    */
    using PositionUpdater = std::function<void()>;

public:
    WindowAnchorFollower() = default;
    ~WindowAnchorFollower() { Stop(); }

    WindowAnchorFollower(const WindowAnchorFollower&) = delete;
    WindowAnchorFollower& operator=(const WindowAnchorFollower&) = delete;

    /** 开始跟随锚点
    * @param [in] getAnchorRect 获取锚点屏幕矩形的函数
    * @param [in] updatePosition 锚点位置变化时，更新浮层窗口位置的函数
    */
    void Start(AnchorRectGetter getAnchorRect, PositionUpdater updatePosition)
    {
        Stop();
        m_getAnchorRect = std::move(getAnchorRect);
        m_updatePosition = std::move(updatePosition);
        m_rcAnchorLast = m_getAnchorRect();
        m_bRunning = true;
        ScheduleNext();
    }

    /** 停止跟随锚点
    */
    void Stop()
    {
        m_bRunning = false;
        if (m_nTaskId != 0) {
            GlobalManager::Instance().Thread().CancelTask(m_nTaskId);
            m_nTaskId = 0;
        }
        m_getAnchorRect = nullptr;
        m_updatePosition = nullptr;
    }

private:
    /** 注册下一次检查任务
    */
    void ScheduleNext()
    {
        m_nTaskId = GlobalManager::Instance().Thread().PostDelayedTask(
            kThreadUI,
            [this]() {
                m_nTaskId = 0;
                if (!m_bRunning || !m_getAnchorRect || !m_updatePosition) {
                    return;
                }
                UiRect rcAnchorNow = m_getAnchorRect();
                if (rcAnchorNow != m_rcAnchorLast) {
                    m_rcAnchorLast = rcAnchorNow;
                    m_updatePosition();
                }
                if (m_bRunning) {
                    ScheduleNext();
                }
            },
            50);
    }

private:
    //获取锚点屏幕矩形的函数
    AnchorRectGetter m_getAnchorRect;

    //更新浮层窗口位置的函数
    PositionUpdater m_updatePosition;

    //上一次记录的锚点屏幕矩形
    UiRect m_rcAnchorLast;

    //是否正在跟随
    bool m_bRunning = false;

    //定时检查任务的ID
    std::size_t m_nTaskId = 0;
};

} // namespace ui

#endif // DUILIB_WINDOW_ANCHOR_FOLLOWER_H_
