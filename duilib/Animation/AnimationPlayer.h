#ifndef UI_ANIMATION_ANIMATIONPLAYER_H_
#define UI_ANIMATION_ANIMATIONPLAYER_H_

#include "duilib/Animation/EasingFunctions.h"
#include "duilib/Core/Callback.h"
#include <chrono>

namespace ui 
{

typedef std::function<void (int64_t)> AnimationPlayCallback;     //播放回调函数
typedef std::function<void (void)> AnimationCompleteCallback;    //播放完成回调函数

//缓动函数的实现
class EasingFunctions;

/** 控件动画播放器的基类接口
*/
class UILIB_API AnimationPlayer : public virtual SupportWeakCallback
{
public:
    AnimationPlayer();
    virtual ~AnimationPlayer() override;

    AnimationPlayer(const AnimationPlayer& r) = delete;
    AnimationPlayer& operator=(const AnimationPlayer& r) = delete;

public:
    /** 设置动画类型
    * @param [in] animationType 动画的类型
    */
    void SetAnimationType(AnimationType animationType) { m_animationType = animationType; }

    /** 获取动画类型
    */
    AnimationType GetAnimationType() const { return m_animationType; }

    /** 设置缓动函数类型
    */
    void SetEasingFunctionType(EasingFunctionType easingFunctionType) { m_easingFunctionType = easingFunctionType ; }

    /** 获取缓动函数类型
    */
    EasingFunctionType GetEasingFunctionType() const { return m_easingFunctionType; }
    
    /** 设置动画播放起始值
    * @param [in] startValue 动画播放的起始值
    */
    void SetStartValue(int64_t startValue) { m_startValue = startValue; }

    /** 获取动画播放起始值
    */
    int64_t GetStartValue() const { return m_startValue; }

    /** 设置动画播放结束值
    * @param [in] endValue 动画播放的结束值
    */
    void SetEndValue(int64_t endValue) { m_endValue = endValue; }

    /** 获取动画播放结束值
    */
    int64_t GetEndValue() const { return m_endValue; }

    /** 设置播放动画的定时器时间间隔（毫秒）
    * @param [in] frameIntervalMillSeconds 播放动画的定时器时间间隔（毫秒）
    */
    void SetFrameIntervalMillSeconds(int32_t frameIntervalMillSeconds) { m_frameIntervalMillSeconds = frameIntervalMillSeconds; }

    /** 获取动画播放的定时器时间间隔（毫秒）
    */
    int32_t GetFrameIntervalMillSeconds() const { return m_frameIntervalMillSeconds; }

    /** 设置动画总的播放时间（毫秒）
    * @param [in] totalMillSeconds 动画总的播放时间（毫秒）
    */
    void SetTotalMillSeconds(int32_t totalMillSeconds) { m_totalMillSeconds = totalMillSeconds; }

    /** 获取动画总的播放时间（毫秒）
    */
    int32_t GetTotalMillSeconds() const { return m_totalMillSeconds; }

public:
    /** 设置播放回调函数
    */
    void SetPlayCallback(const AnimationPlayCallback& playCallback) { m_playCallback = playCallback; }

    /** 设置播放完成回调函数
    */
    void SetCompleteCallback(const AnimationCompleteCallback& completeCallback) { m_completeCallback = completeCallback; }

    /** 动画开始
    */
    void Start();

    /** 是否正在播放中
    */
    bool IsPlaying() const { return m_bPlaying; }

    /** 动画结束
    */
    void Stop();

    /** 动画继续（从起始值 到 结束值）
    */
    void Continue();

    /** 动画反向继续（从结束值 到 起始值，反向动画）
    */
    void ReverseContinue();

    /** 获取动画当前值
    */
    int64_t GetCurrentValue() { return m_currentValue; }

    /** 停止并清理资源
    */
    void Clear();

    /** 重置状态
    */
    void Reset();

private:
    /** 初始化
    */
    void Init();

    /** 启动动画定时器
    */
    void StartTimer();

    /** 播放一次动画（在定时器中触发调用）
    */
    void Play();

    /** 交换起始值和结束值
    */
    void ReverseAllValue();

    /** 完成播放动画，并触发播放完成回调函数
    */
    void Complete();

private:
    /** 起始值
    */
    int64_t m_startValue;

    /** 结束值
    */
    int64_t m_endValue;

    /** 播放动画的定时器时间间隔（毫秒）
    */
    int32_t m_frameIntervalMillSeconds;

    /** 播放总的时间（毫秒）
    */
    int32_t m_totalMillSeconds;

    /** 播放回调函数
    */
    AnimationPlayCallback m_playCallback;

    /** 播放完成回调函数
    */
    AnimationCompleteCallback m_completeCallback;

private:
    /** 当前值
    */
    int64_t m_currentValue;

    /** 定时器终止标志
    */
    WeakCallbackFlag m_weakFlagOwner;

    /** 缓动函数的实现
    */
    std::unique_ptr<EasingFunctions> m_pEasingFunctions;

    /** 当前播放的帧序号
    */
    int32_t m_frameIndex;

    /** 动画类型
    */
    AnimationType m_animationType;

    /** 缓动函数类型
    */
    EasingFunctionType m_easingFunctionType;

    /** 是否正在播放中
    */
    bool m_bPlaying;

    /** 是否正在反向播放
    */
    bool m_reverseStart;
};

} // namespace ui

#endif // UI_ANIMATION_ANIMATIONPLAYER_H_
