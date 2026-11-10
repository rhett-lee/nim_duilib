#ifndef UI_ANIMATION_ANIMATION_SCHEDULER_H_
#define UI_ANIMATION_ANIMATION_SCHEDULER_H_

#include "duilib/Core/Callback.h"
#include "duilib/Core/ControlPtrT.h"
#include <vector>
#include <memory>

namespace ui 
{

class AnimationPlayer;

/** 动画调度器（全局唯一）
*
*  所有处于播放状态的 AnimationPlayer 统一注册到本调度器，由同一个定时器驱动：
*  1. 同一帧内所有动画使用相同的时间基准，保证各属性动画严格同步；
*  2. 定时器消息数量从 "动画数量 × 帧率" 降为 "帧率"，避免消息队列拥塞；
*  3. 同一帧内多次请求的控件重排，合并为每帧一次；
*  4. 无动画播放时定时器自动停止，不占用CPU；
*  5. 所有窗口都最小化或者不可见时，暂停推进时间基准，节省CPU。
*/
class DUILIB_API AnimationScheduler final
{
public:
    /** 获取全局唯一的动画调度器
    */
    static AnimationScheduler& Instance();

    AnimationScheduler(const AnimationScheduler&) = delete;
    AnimationScheduler& operator=(const AnimationScheduler&) = delete;

public:
    /** 注册一个正在播放的动画
    * @param [in] pPlayer 动画播放器
    * @param [in] weakFlag 动画播放器的存活标志，expired() 为 true 时自动注销
    */
    void Register(AnimationPlayer* pPlayer, const std::weak_ptr<WeakFlag>& weakFlag);

    /** 注销一个动画
    * @param [in] pPlayer 动画播放器
    */
    void Unregister(AnimationPlayer* pPlayer);

    /** 请求在本次动画刷新结束时统一重排控件（同一帧内对同一个控件只重排一次）
    *   如果当前不在动画刷新过程中，则立即重排。
    * @param [in] pControl 需要重排的控件（弱引用，控件销毁后自动忽略）
    */
    void RequestArrange(const ControlPtr& pControl);

    /** 获取当前时间（毫秒），作为所有动画播放进度的统一时间基准
    *   该时间只在有动画播放且窗口可见时推进，窗口最小化/不可见期间不推进（相当于动画暂停）
    */
    static int64_t GetTimeMillSeconds();

private:
    AnimationScheduler();
    ~AnimationScheduler();

    /** 获取当前的真实时间（毫秒），不受暂停影响
    */
    static int64_t GetRealTimeMillSeconds();

    /** 动画是否应该暂停推进（所有窗口都最小化或者不可见时返回true）
    */
    static bool IsAnimationPaused();

    /** 定时器回调函数，驱动所有正在播放的动画
    */
    void OnTimer();

    /** 执行一次动画刷新
    * @param [in] nNowMillSeconds 本帧的统一时间基准
    */
    void Tick(int64_t nNowMillSeconds);

    /** 执行本帧内累计的控件重排
    */
    void FlushArrange();

private:
    /** 已注册的动画信息
    */
    struct TPlayerItem
    {
        /** 动画播放器
        */
        AnimationPlayer* pPlayer = nullptr;

        /** 动画播放器的存活标志
        */
        std::weak_ptr<WeakFlag> weakFlag;
    };

    /** 处于播放状态的动画列表
    */
    std::vector<TPlayerItem> m_players;

    /** 本帧内需要重排的控件列表
    */
    std::vector<ControlPtr> m_arrangeList;

    /** 动画刷新定时器的取消标志
    */
    WeakCallbackFlag m_timerFlag;

    /** 动画的统一时间基准（毫秒）：只在播放且窗口可见时推进
    */
    int64_t m_nVirtualTimeMillSeconds;

    /** 上一次刷新时的真实时间（毫秒），用于计算时间基准的增量
    */
    int64_t m_nLastRealTimeMillSeconds;

    /** 定时器是否已经启动
    */
    bool m_bTimerStarted = false;

    /** 是否正在动画刷新过程中（用于合并控件重排）
    */
    bool m_bInTick = false;
};

} // namespace ui

#endif // UI_ANIMATION_ANIMATION_SCHEDULER_H_
