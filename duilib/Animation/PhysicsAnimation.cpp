#include "duilib/Animation/PhysicsAnimation.h"
#include <algorithm>
#include <cmath>

namespace ui
{

namespace
{
    /** 物理动画的全局配置（只在 UI 线程访问，无需加锁）
    */
    struct PhysicsConfig
    {
        //物理动画总开关
        bool m_bEnabled = true;

        //默认弹簧参数：刚度、阻尼、质量（阻尼比约 0.88，接近临界阻尼，仅有轻微回弹）
        double m_springStiffness = 220.0;
        double m_springDamping = 26.0;
        double m_springMass = 1.0;

        //默认惯性参数：摩擦系数（1/秒）、最小速度阈值（像素/秒）
        double m_inertiaFriction = 5.0;
        double m_inertiaMinVelocity = 30.0;
    };

    PhysicsConfig& GetPhysicsConfig()
    {
        static PhysicsConfig s_physicsConfig;
        return s_physicsConfig;
    }

    /** 弹簧积分的最大步长（秒）：固定小步长积分，保证不同帧率下的结果一致
    */
    const double kSpringMaxStepSeconds = 1.0 / 240.0;

    /** 弹簧静止判定：位移阈值（像素）
    */
    const double kSpringRestDisplacement = 0.5;

    /** 弹簧静止判定：速度阈值（像素/秒）
    */
    const double kSpringRestVelocity = 8.0;
}

void PhysicsAnimation::SetEnabled(bool bEnabled)
{
    GetPhysicsConfig().m_bEnabled = bEnabled;
}

bool PhysicsAnimation::IsEnabled()
{
    return GetPhysicsConfig().m_bEnabled;
}

void PhysicsAnimation::SetDefaultSpringParameters(double stiffness, double damping, double mass)
{
    PhysicsConfig& config = GetPhysicsConfig();
    config.m_springStiffness = (stiffness > 0.0) ? stiffness : config.m_springStiffness;
    config.m_springDamping = (damping >= 0.0) ? damping : config.m_springDamping;
    config.m_springMass = (mass > 0.0) ? mass : config.m_springMass;
}

void PhysicsAnimation::GetDefaultSpringParameters(double& stiffness, double& damping, double& mass)
{
    const PhysicsConfig& config = GetPhysicsConfig();
    stiffness = config.m_springStiffness;
    damping = config.m_springDamping;
    mass = config.m_springMass;
}

void PhysicsAnimation::SetDefaultInertiaParameters(double friction, double minVelocity)
{
    PhysicsConfig& config = GetPhysicsConfig();
    config.m_inertiaFriction = (friction > 0.0) ? friction : config.m_inertiaFriction;
    config.m_inertiaMinVelocity = (minVelocity > 0.0) ? minVelocity : config.m_inertiaMinVelocity;
}

void PhysicsAnimation::GetDefaultInertiaParameters(double& friction, double& minVelocity)
{
    const PhysicsConfig& config = GetPhysicsConfig();
    friction = config.m_inertiaFriction;
    minVelocity = config.m_inertiaMinVelocity;
}

SpringSimulation::SpringSimulation():
    m_stiffness(220.0),
    m_damping(26.0),
    m_mass(1.0),
    m_value(0.0),
    m_targetValue(0.0),
    m_velocity(0.0),
    m_bFinished(true)
{
}

SpringSimulation::~SpringSimulation() = default;

void SpringSimulation::SetParameters(double stiffness, double damping, double mass)
{
    m_stiffness = (stiffness > 0.0) ? stiffness : 0.01;
    m_damping = (damping >= 0.0) ? damping : 0.0;
    m_mass = (mass > 0.0) ? mass : 0.01;
}

void SpringSimulation::SetParametersByDampingRatio(double stiffness, double dampingRatio, double mass)
{
    if (stiffness <= 0.0) {
        stiffness = 220.0;
    }
    if (mass <= 0.0) {
        mass = 1.0;
    }
    if (dampingRatio < 0.0) {
        dampingRatio = 0.0;
    }
    m_stiffness = stiffness;
    m_mass = mass;
    //临界阻尼系数为 2 * sqrt(k * m)，乘以阻尼比即为实际阻尼
    m_damping = 2.0 * dampingRatio * std::sqrt(stiffness * mass);
}

void SpringSimulation::Reset(double startValue, double targetValue, double startVelocity)
{
    m_value = startValue;
    m_targetValue = targetValue;
    m_velocity = startVelocity;
    m_bFinished = (std::abs(targetValue - startValue) < 0.01) && (std::abs(startVelocity) < 0.01);
}

void SpringSimulation::SetTarget(double targetValue)
{
    m_targetValue = targetValue;
    //目标值变化后重新开始积分（即使当前被判为静止，也需要继续运动）
    m_bFinished = false;
}

bool SpringSimulation::Advance(double deltaSeconds, double& outValue)
{
    if (m_bFinished) {
        outValue = m_value;
        return false;
    }
    if (deltaSeconds <= 0.0) {
        outValue = m_value;
        return true;
    }

    //按固定小步长积分，避免帧率波动导致结果不一致或数值发散
    double remainingSeconds = deltaSeconds;
    while (remainingSeconds > 0.0) {
        const double stepSeconds = std::min(remainingSeconds, kSpringMaxStepSeconds);
        const double displacement = m_value - m_targetValue;
        //弹簧力 + 阻尼力
        const double acceleration = (-m_stiffness * displacement - m_damping * m_velocity) / m_mass;
        m_velocity += acceleration * stepSeconds;
        m_value += m_velocity * stepSeconds;
        remainingSeconds -= stepSeconds;
    }

    //位移和速度都很小时判定为静止，并精确落到目标值
    if ((std::abs(m_value - m_targetValue) < kSpringRestDisplacement) &&
        (std::abs(m_velocity) < kSpringRestVelocity)) {
        m_value = m_targetValue;
        m_velocity = 0.0;
        m_bFinished = true;
    }
    outValue = m_value;
    return !m_bFinished;
}

InertiaSimulation::InertiaSimulation():
    m_friction(5.0),
    m_minVelocity(30.0),
    m_value(0.0),
    m_velocity(0.0),
    m_minValue(-1.0e18),
    m_maxValue(1.0e18),
    m_maxMillSeconds(0),
    m_elapsedMillSeconds(0),
    m_bFinished(true)
{
}

InertiaSimulation::~InertiaSimulation() = default;

void InertiaSimulation::SetParameters(double friction, double minVelocity, int32_t maxMillSeconds)
{
    m_friction = (friction > 0.001) ? friction : 0.001;
    m_minVelocity = (minVelocity > 0.0) ? minVelocity : 1.0;
    m_maxMillSeconds = (maxMillSeconds > 0) ? maxMillSeconds : 0;
}

void InertiaSimulation::SetValueRange(double minValue, double maxValue)
{
    if (minValue > maxValue) {
        std::swap(minValue, maxValue);
    }
    m_minValue = minValue;
    m_maxValue = maxValue;
}

void InertiaSimulation::Reset(double startValue, double startVelocity)
{
    m_value = startValue;
    m_velocity = startVelocity;
    if (m_value < m_minValue) {
        m_value = m_minValue;
    }
    else if (m_value > m_maxValue) {
        m_value = m_maxValue;
    }
    m_elapsedMillSeconds = 0;
    m_bFinished = (std::abs(startVelocity) <= m_minVelocity);
}

bool InertiaSimulation::Advance(double deltaSeconds, double& outValue)
{
    if (m_bFinished) {
        outValue = m_value;
        return false;
    }
    if (deltaSeconds <= 0.0) {
        outValue = m_value;
        return true;
    }

    //指数摩擦衰减的解析解：位移 = v0 * (1 - exp(-f * t)) / f
    const double decay = std::exp(-m_friction * deltaSeconds);
    m_value += m_velocity * (1.0 - decay) / m_friction;
    m_velocity *= decay;
    m_elapsedMillSeconds += static_cast<int32_t>(std::llround(deltaSeconds * 1000.0));

    if (m_value <= m_minValue) {
        //到达取值下限，停止
        m_value = m_minValue;
        m_velocity = 0.0;
        m_bFinished = true;
    }
    else if (m_value >= m_maxValue) {
        //到达取值上限，停止
        m_value = m_maxValue;
        m_velocity = 0.0;
        m_bFinished = true;
    }
    else if (std::abs(m_velocity) <= m_minVelocity) {
        //速度已经足够小，停止
        m_velocity = 0.0;
        m_bFinished = true;
    }
    else if ((m_maxMillSeconds > 0) && (m_elapsedMillSeconds >= m_maxMillSeconds)) {
        //超过最大持续时间，停止（兜底，避免异常情况下长时间占用定时器）
        m_bFinished = true;
    }
    outValue = m_value;
    return !m_bFinished;
}

} // namespace ui
