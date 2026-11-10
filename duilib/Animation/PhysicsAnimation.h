#ifndef UI_ANIMATION_PHYSICS_ANIMATION_H_
#define UI_ANIMATION_PHYSICS_ANIMATION_H_

#include "duilib/duilib_defs.h"

namespace ui
{

/** 物理动画类型
*/
enum class AnimationPhysicsType: int8_t
{
    kNone    = 0,   //不使用物理动画（按缓动函数播放）
    kSpring  = 1,   //弹簧动画（用于面板展开/折叠等有明确目标值的场景）
    kInertia = 2    //惯性动画（用于列表惯性滚动等只有初速度、没有目标值的场景）
};

/** 物理动画的全局配置与总开关
*
*  说明：
*  1. 默认开启；关闭后所有物理动画自动回退为缓动函数动画，行为与开启前一致；
*  2. 每个控件仍需单独启用（如 Panel 的 collapse_physics、ScrollBox 的 inertia_scroll），
*     因此默认开启不会改变现有界面的表现。
*/
class DUILIB_API PhysicsAnimation final
{
public:
    PhysicsAnimation() = delete;

    /** 设置物理动画总开关
    */
    static void SetEnabled(bool bEnabled);

    /** 物理动画总开关是否开启
    */
    static bool IsEnabled();

    /** 设置默认的弹簧参数（刚度、阻尼、质量）
    * @param [in] stiffness 刚度，越大回弹越快
    * @param [in] damping 阻尼，越大越不容易回弹
    * @param [in] mass 质量，越大惯性越强
    */
    static void SetDefaultSpringParameters(double stiffness, double damping, double mass);

    /** 获取默认的弹簧参数
    */
    static void GetDefaultSpringParameters(double& stiffness, double& damping, double& mass);

    /** 设置默认的惯性参数
    * @param [in] friction 摩擦系数（1/秒），越大停得越快
    * @param [in] minVelocity 最小速度阈值（像素/秒），低于该速度即停止
    */
    static void SetDefaultInertiaParameters(double friction, double minVelocity);

    /** 获取默认的惯性参数
    */
    static void GetDefaultInertiaParameters(double& friction, double& minVelocity);
};

/** 弹簧积分器（阻尼谐振子，半隐式欧拉积分）
*
*  用于有明确目标值的动画（如面板高度从 A 平滑变化到 B）。
*  支持亚像素精度，内部按固定小步长积分，避免帧率波动导致的数值发散。
*/
class DUILIB_API SpringSimulation
{
public:
    SpringSimulation();
    ~SpringSimulation();

public:
    /** 设置弹簧参数
    * @param [in] stiffness 刚度
    * @param [in] damping 阻尼
    * @param [in] mass 质量
    */
    void SetParameters(double stiffness, double damping, double mass);

    /** 按阻尼比设置弹簧参数（便于调节手感）
    * @param [in] stiffness 刚度
    * @param [in] dampingRatio 阻尼比：1.0 为临界阻尼（无回弹），小于 1.0 会回弹
    * @param [in] mass 质量
    */
    void SetParametersByDampingRatio(double stiffness, double dampingRatio, double mass);

    /** 重置积分状态
    * @param [in] startValue 起始值
    * @param [in] targetValue 目标值
    * @param [in] startVelocity 起始速度（像素/秒）
    */
    void Reset(double startValue, double targetValue, double startVelocity = 0.0);

    /** 修改目标值（播放过程中修改目标，会从当前状态平滑过渡，不会回跳）
    */
    void SetTarget(double targetValue);

    /** 推进一段时间
    * @param [in] deltaSeconds 时间增量（秒）
    * @param [out] outValue 推进后的当前值
    * @return 返回 true 表示仍在运动，false 表示已经静止
    */
    bool Advance(double deltaSeconds, double& outValue);

    /** 是否已经静止
    */
    bool IsFinished() const { return m_bFinished; }

    /** 获取当前值
    */
    double GetValue() const { return m_value; }

    /** 获取当前速度（像素/秒）
    */
    double GetVelocity() const { return m_velocity; }

private:
    //刚度
    double m_stiffness;

    //阻尼
    double m_damping;

    //质量
    double m_mass;

    //当前值
    double m_value;

    //目标值
    double m_targetValue;

    //当前速度（像素/秒）
    double m_velocity;

    //是否已经静止
    bool m_bFinished;
};

/** 惯性积分器（指数摩擦衰减）
*
*  用于只有初速度、没有明确目标值的动画（如列表惯性滚动）：
*  速度按 v(t) = v0 * exp(-friction * t) 衰减，总滑行距离为 v0 / friction。
*/
class DUILIB_API InertiaSimulation
{
public:
    InertiaSimulation();
    ~InertiaSimulation();

public:
    /** 设置惯性参数
    * @param [in] friction 摩擦系数（1/秒），越大停得越快
    * @param [in] minVelocity 最小速度阈值（像素/秒），低于该速度即停止
    * @param [in] maxMillSeconds 最大持续时间（毫秒），用于兜底，<=0 表示不限制
    */
    void SetParameters(double friction, double minVelocity, int32_t maxMillSeconds);

    /** 设置取值范围（到达边界后立即停止，用于滚动到顶/底时自然停住）
    * @param [in] minValue 最小值
    * @param [in] maxValue 最大值
    */
    void SetValueRange(double minValue, double maxValue);

    /** 重置积分状态
    * @param [in] startValue 起始值
    * @param [in] startVelocity 起始速度（像素/秒，正负表示方向）
    */
    void Reset(double startValue, double startVelocity);

    /** 推进一段时间
    * @param [in] deltaSeconds 时间增量（秒）
    * @param [out] outValue 推进后的当前值
    * @return 返回 true 表示仍在运动，false 表示已经停止
    */
    bool Advance(double deltaSeconds, double& outValue);

    /** 是否已经停止
    */
    bool IsFinished() const { return m_bFinished; }

    /** 获取当前值
    */
    double GetValue() const { return m_value; }

    /** 获取当前速度（像素/秒）
    */
    double GetVelocity() const { return m_velocity; }

private:
    //摩擦系数（1/秒）
    double m_friction;

    //最小速度阈值（像素/秒）
    double m_minVelocity;

    //当前值
    double m_value;

    //当前速度（像素/秒）
    double m_velocity;

    //取值下限
    double m_minValue;

    //取值上限
    double m_maxValue;

    //最大持续时间（毫秒）
    int32_t m_maxMillSeconds;

    //已经持续的时间（毫秒）
    int32_t m_elapsedMillSeconds;

    //是否已经停止
    bool m_bFinished;
};

} // namespace ui

#endif // UI_ANIMATION_PHYSICS_ANIMATION_H_
