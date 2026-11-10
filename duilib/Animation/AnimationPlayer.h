#ifndef UI_ANIMATION_ANIMATIONPLAYER_H_
#define UI_ANIMATION_ANIMATIONPLAYER_H_

#include "duilib/Animation/EasingFunctions.h"
#include "duilib/Animation/PhysicsAnimation.h"
#include "duilib/Core/Callback.h"

namespace ui 
{

typedef std::function<void (int32_t)> AnimationPlayCallback;     //播放回调函数
typedef std::function<void (void)> AnimationCompleteCallback;    //播放完成回调函数

//缓动函数的实现
class EasingFunctions;

//动画调度器
class AnimationScheduler;

/** 控件动画播放器的基类接口
*/
class DUILIB_API AnimationPlayer : public virtual SupportWeakCallback
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

    /** 设置物理动画类型
    *   物理动画受 PhysicsAnimation 的全局开关控制：开关关闭时按缓动函数播放；
    *   默认类型为 kNone（纯缓动函数动画），需要显式设置才会启用物理动画
    * @param [in] physicsType 物理动画类型（弹簧/惯性/不使用）
    */
    void SetPhysicsType(AnimationPhysicsType physicsType) { m_physicsType = physicsType; }

    /** 获取物理动画类型
    */
    AnimationPhysicsType GetPhysicsType() const { return m_physicsType; }

    /** 设置弹簧参数（物理类型为 kSpring 时生效）
    *   未设置时使用 PhysicsAnimation 的默认弹簧参数
    * @param [in] stiffness 刚度，越大回弹越快
    * @param [in] damping 阻尼，越大越不容易回弹
    * @param [in] mass 质量，越大惯性越强
    */
    void SetSpringParameters(double stiffness, double damping, double mass);

    /** 设置惯性动画的参数（物理类型为 kInertia 时生效）
    * @param [in] initialVelocity 初始速度（像素/秒，正负表示方向）
    * @param [in] friction 摩擦系数（1/秒），越大停得越快；未设置时使用全局默认值
    */
    void SetInertiaParameters(double initialVelocity, double friction = -1.0);

    /** 设置惯性动画的取值范围（到达边界后自动结束，DUI_NOSET_VALUE 的默认值表示不限制）
    * @param [in] minValue 取值下限
    * @param [in] maxValue 取值上限
    */
    void SetInertiaValueRange(int32_t minValue, int32_t maxValue);

    /** 物理动画当前是否生效（全局开关开启且本播放器设置了物理类型）
    */
    bool IsPhysicsEnabled() const;
    
    /** 设置动画播放起始值
    * @param [in] startValue 动画播放的起始值
    */
    void SetStartValue(int32_t startValue) { m_startValue = startValue; }

    /** 获取动画播放起始值
    */
    int32_t GetStartValue() const { return m_startValue; }

    /** 设置动画播放结束值
    *   如果动画正在播放中，则以当前播放值作为新的起点继续播放，避免数值回跳
    * @param [in] endValue 动画播放的结束值
    */
    void SetEndValue(int32_t endValue);

    /** 获取动画播放结束值
    */
    int32_t GetEndValue() const { return m_endValue; }

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

    /** 获取播放回调函数
    */
    AnimationPlayCallback GetPlayCallback() const { return m_playCallback; }

    /** 设置播放完成回调函数
    */
    void SetCompleteCallback(const AnimationCompleteCallback& completeCallback) { m_completeCallback = completeCallback; }

    /** 获取播放完成回调函数
    */
    AnimationCompleteCallback GetCompleteCallback() const { return m_completeCallback; }

    /** 动画开始
    */
    void Start();

    /** 动画继续（从起始值 到 结束值）
    */
    void Continue();

    /** 动画反向开始
    */
    void ReverseStart();

    /** 动画反向继续（从结束值 到 起始值，反向动画）
    */
    void ReverseContinue();

    /** 动画结束
    */
    void Stop();

    /** 停止并清理资源
    */
    void Clear();

public:
    /** 获取动画当前值
    */
    int32_t GetCurrentValue() { return m_currentValue; }

    /** 是否正在播放中
    */
    bool IsPlaying() const { return m_bPlaying; }

private:
    /** 初始化
    */
    void Init();

    /** 启动动画定时器
    * @param [in] bContinueMode true表示为继续模式（不重新开始，接着原来的进度继续），false表示重新开始
    * @param [in] bOldReversePlay 原来的播放模式是否为逆向播放, 该参数仅当bContinueMode为true的时候有效
    */
    void StartTimer(bool bContinueMode, bool bOldReversePlay);

    /** 播放一次动画（由动画调度器在定时器中触发调用）
    * @param [in] nNowMillSeconds 本帧的统一时间基准（毫秒）
    */
    void OnTick(int64_t nNowMillSeconds);

    /** 播放过程中修改结束值：以当前播放值作为新起点，用剩余的播放时间续播到新的结束值
    * @param [in] nNewEndValue 新的结束值
    */
    void Retarget(int32_t nNewEndValue);

    /** 交换起始值和结束值
    */
    void ReverseAllValue();

    /** 完成播放动画，并触发播放完成回调函数
    * @param [in] bSnapToEnd true表示把当前值对齐到结束值（弹簧动画），false表示保留当前值（惯性动画）
    */
    void Complete(bool bSnapToEnd = true);

    /** 是否正在使用物理积分器（弹簧或惯性）
    */
    bool HasPhysicsSimulation() const { return (m_pSpringSimulation != nullptr) || (m_pInertiaSimulation != nullptr); }

    /** 按物理模型启动动画（注册到动画调度器，由统一时间基准驱动）
    */
    void StartPhysics();

    /** 物理动画的一帧推进（由动画调度器在定时器中触发调用）
    * @param [in] nNowMillSeconds 本帧的统一时间基准（毫秒）
    */
    void OnTickPhysics(int64_t nNowMillSeconds);

private:
    friend class AnimationScheduler;

private:
    /** 起始值
    */
    int32_t m_startValue;

    /** 结束值
    */
    int32_t m_endValue;

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
    int32_t m_currentValue;

    /** 当前播放的帧序号
    */
    int32_t m_frameIndex;

    /** 当前播放进度（浮点数，范围[0.0, 1.0]，用于精确衔接反向/继续播放）
    */
    double m_playProgress;

    /** 本次播放实际使用的总时长（毫秒）
    */
    int32_t m_playTotalMillSeconds;

    /** 播放起点对应的时间（毫秒，使用动画调度器的统一时间基准）
    */
    int64_t m_startTimeMillSeconds;

    /** 定时器终止标志
    */
    WeakCallbackFlag m_weakFlagOwner;

    /** 缓动函数的实现
    */
    std::unique_ptr<EasingFunctions> m_pEasingFunctions;

    /** 动画类型
    */
    AnimationType m_animationType;

    /** 缓动函数类型
    */
    EasingFunctionType m_easingFunctionType;

    /** 物理动画类型
    */
    AnimationPhysicsType m_physicsType;

    /** 弹簧参数：刚度、阻尼、质量（均为 0 时使用全局默认值）
    */
    double m_springStiffness;
    double m_springDamping;
    double m_springMass;

    /** 惯性动画的初始速度（像素/秒）
    */
    double m_inertiaVelocity;

    /** 惯性动画的摩擦系数（<0 表示使用全局默认值）
    */
    double m_inertiaFriction;

    /** 惯性动画的取值下限
    */
    int32_t m_inertiaMinValue;

    /** 惯性动画的取值上限
    */
    int32_t m_inertiaMaxValue;

    /** 弹簧积分器（物理类型为 kSpring 时有效）
    */
    std::unique_ptr<SpringSimulation> m_pSpringSimulation;

    /** 惯性积分器（物理类型为 kInertia 时有效）
    */
    std::unique_ptr<InertiaSimulation> m_pInertiaSimulation;

    /** 物理动画上一帧对应的时间（毫秒），用于计算积分步长
    */
    int64_t m_physicsLastTimeMillSeconds;

    /** 是否正在播放中
    */
    bool m_bPlaying;

    /** 是否正在反向播放
    */
    bool m_bReversePlay;
};

} // namespace ui

#endif // UI_ANIMATION_ANIMATIONPLAYER_H_
