// Animation 目录的单元测试
//
// 测试对象：duilib/Animation 下的纯逻辑代码，不创建窗口、不启动全局资源
// （GlobalManager::Startup），也不依赖消息循环与真实定时器，因此可以在命令行
// （含 CI）直接运行。
//
// 覆盖内容：
//  1. EasingFunctions：缓动函数查找、名称解析、端点/钳位/对称性、已知取值；
//  2. EasingFunctions 类：整帧与亚帧取值、起止值、帧数、特殊输入（NaN/越界）；
//  3. PhysicsAnimation：全局开关与默认参数、SpringSimulation、InertiaSimulation；
//  4. AnimationPlayer：默认属性、起止值相同的同步完成、Stop/Clear 的状态复位；
//  5. AnimationManager：动画播放器的创建/查询/清除、参数下发；
//  6. PopupAnimation：全局配置项与空窗口调用的安全行为；
//  7. AnimationScheduler：统一时间基准在无动画推进时保持稳定。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Animation/AnimationScheduler.h"
#include "duilib/Animation/EasingFunctions.h"
#include "duilib/Animation/PhysicsAnimation.h"
#include "duilib/Animation/PopupAnimation.h"
#include "duilib/Control/Label.h"

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const std::function<std::string()>& message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message() << std::endl;
    }
}

/** 直接使用常量字符串作为失败信息 */
void Check(bool condition, const char* message)
{
    Check(condition, [message]() { return std::string(message); });
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 浮点数近似比较 */
bool IsNear(double value, double expected, double epsilon = 1e-9)
{
    return std::fabs(value - expected) <= epsilon;
}

/** 全部缓动函数类型（顺序与枚举定义一致） */
const ui::EasingFunctionType kAllEasingTypes[] =
{
    ui::EaseLinear,
    ui::EaseInSine,        ui::EaseOutSine,        ui::EaseInOutSine,
    ui::EaseInQuad,        ui::EaseOutQuad,        ui::EaseInOutQuad,
    ui::EaseInCubic,       ui::EaseOutCubic,       ui::EaseInOutCubic,
    ui::EaseInQuart,       ui::EaseOutQuart,       ui::EaseInOutQuart,
    ui::EaseInQuint,       ui::EaseOutQuint,       ui::EaseInOutQuint,
    ui::EaseInExpo,        ui::EaseOutExpo,        ui::EaseInOutExpo,
    ui::EaseInCirc,        ui::EaseOutCirc,        ui::EaseInOutCirc,
    ui::EaseInBack,        ui::EaseOutBack,        ui::EaseInOutBack,
    ui::EaseInElastic,     ui::EaseOutElastic,     ui::EaseInOutElastic,
    ui::EaseInBounce,      ui::EaseOutBounce,      ui::EaseInOutBounce
};

/** 是否为缓入缓出（InOut）类型的缓动函数 */
bool IsInOutType(ui::EasingFunctionType type)
{
    switch (type) {
    case ui::EaseInOutSine:
    case ui::EaseInOutQuad:
    case ui::EaseInOutCubic:
    case ui::EaseInOutQuart:
    case ui::EaseInOutQuint:
    case ui::EaseInOutExpo:
    case ui::EaseInOutCirc:
    case ui::EaseInOutBack:
    case ui::EaseInOutElastic:
    case ui::EaseInOutBounce:
        return true;
    default:
        return false;
    }
}

/** 缓动函数查找与名称解析 */
void TestEasingFunctionLookup()
{
    for (ui::EasingFunctionType type : kAllEasingTypes) {
        CHECK(ui::EasingFunctions::GetEasingFunction(type) != nullptr);
    }
    //超出枚举范围的类型返回 nullptr
    CHECK(ui::EasingFunctions::GetEasingFunction(static_cast<ui::EasingFunctionType>(200)) == nullptr);

    //名称与枚举值完全一致
    CHECK(ui::EasingFunctions::GetEasingFunctionType(_T("EaseOutCubic")) == ui::EaseOutCubic);
    CHECK(ui::EasingFunctions::GetEasingFunctionType(_T("EaseInOutCubic")) == ui::EaseInOutCubic);
    //允许首字母小写
    CHECK(ui::EasingFunctions::GetEasingFunctionType(_T("easeOutCubic")) == ui::EaseOutCubic);
    CHECK(ui::EasingFunctions::GetEasingFunctionType(_T("easeInOutQuad")) == ui::EaseInOutQuad);
    //未知名称与空字符串回退为线性缓动
    CHECK(ui::EasingFunctions::GetEasingFunctionType(_T("")) == ui::EaseLinear);
    CHECK(ui::EasingFunctions::GetEasingFunctionType(_T("UnknownName")) == ui::EaseLinear);
}

/** 缓动曲线的端点、钳位、对称性与已知取值 */
void TestEasingCurves()
{
    for (ui::EasingFunctionType type : kAllEasingTypes) {
        ui::EasingFunction fn = ui::EasingFunctions::GetEasingFunction(type);
        CHECK(fn != nullptr);
        if (fn == nullptr) {
            continue;
        }
        //所有曲线的端点固定为 0 和 1
        CHECK(IsNear(fn(0.0), 0.0));
        CHECK(IsNear(fn(1.0), 1.0));
        //输入超出 [0, 1] 时按端点钳位
        CHECK(IsNear(fn(-1.0), 0.0));
        CHECK(IsNear(fn(2.0), 1.0));

        if (IsInOutType(type)) {
            //InOut 曲线在 0.5 处对称，应返回 0.5（历史上 easeInOutQuad 曾在这里跳变）
            CHECK(IsNear(fn(0.5), 0.5, 1e-6));
            //0.5 左右连续，不应出现跳变（EaseInOutCirc 在 0.5 处斜率无穷大，
            //因此这里只判断是否存在“跳变”，不判断导数是否连续）
            CHECK(IsNear(fn(0.5 - 1e-6), fn(0.5 + 1e-6), 1e-2));
        }
    }

    //已知取值
    ui::EasingFunction linear = ui::EasingFunctions::GetEasingFunction(ui::EaseLinear);
    CHECK(IsNear(linear(0.25), 0.25));
    ui::EasingFunction quadIn = ui::EasingFunctions::GetEasingFunction(ui::EaseInQuad);
    CHECK(IsNear(quadIn(0.5), 0.25));
    ui::EasingFunction quadOut = ui::EasingFunctions::GetEasingFunction(ui::EaseOutQuad);
    CHECK(IsNear(quadOut(0.5), 0.75));

    //回退曲线会短暂越过 [0, 1] 取值范围
    CHECK(ui::EasingFunctions::GetEasingFunction(ui::EaseInBack)(0.1) < 0.0);
    CHECK(ui::EasingFunctions::GetEasingFunction(ui::EaseOutBack)(0.9) > 1.0);
}

/** EasingFunctions 类的取值逻辑 */
void TestEasingFunctionsClass()
{
    ui::EasingFunctions easing(0, 100, 10, ui::EaseLinear);
    CHECK(easing.GetStartValue() == 0);
    CHECK(easing.GetEndValue() == 100);
    CHECK(easing.GetFrameCount() == 10);

    //整帧取值：0 <= 帧序号 <= 帧数，超出范围按端点处理
    CHECK(easing.GetEasingValue(0) == 0);
    CHECK(easing.GetEasingValue(5) == 50);
    CHECK(easing.GetEasingValue(10) == 100);
    CHECK(easing.GetEasingValue(99) == 100);

    //亚帧精度
    CHECK(easing.GetEasingValue(0.5) == 50);
    CHECK(easing.GetEasingValue(-0.5) == 0);
    CHECK(easing.GetEasingValue(1.5) == 100);

    //非法输入（NaN）回退为结束值，避免产生未定义行为
    const double nanValue = std::numeric_limits<double>::quiet_NaN();
    CHECK(easing.GetEasingValue(nanValue) == 100);

    //起止值相同时，任意进度都返回同一个值
    ui::EasingFunctions sameValue(30, 30, 10, ui::EaseOutCubic);
    CHECK(sameValue.GetEasingValue(0) == 30);
    CHECK(sameValue.GetEasingValue(10) == 30);
    CHECK(sameValue.GetEasingValue(0.5) == 30);

    //结束值小于起始值时按线性插值递减
    ui::EasingFunctions reversed(100, 0, 4, ui::EaseLinear);
    CHECK(reversed.GetEasingValue(2) == 50);
    CHECK(reversed.GetEasingValue(4) == 0);
}

/** 物理动画的全局开关与默认参数 */
void TestPhysicsGlobalConfig()
{
    const bool bOldEnabled = ui::PhysicsAnimation::IsEnabled();
    ui::PhysicsAnimation::SetEnabled(false);
    CHECK(!ui::PhysicsAnimation::IsEnabled());
    ui::PhysicsAnimation::SetEnabled(true);
    CHECK(ui::PhysicsAnimation::IsEnabled());
    ui::PhysicsAnimation::SetEnabled(bOldEnabled);

    double stiffness = 0.0;
    double damping = 0.0;
    double mass = 0.0;
    ui::PhysicsAnimation::GetDefaultSpringParameters(stiffness, damping, mass);
    const double oldStiffness = stiffness;
    const double oldDamping = damping;
    const double oldMass = mass;
    CHECK(stiffness > 0.0);
    CHECK(mass > 0.0);

    //非法值（<=0）保持原有默认值不变
    ui::PhysicsAnimation::SetDefaultSpringParameters(-1.0, -1.0, -1.0);
    ui::PhysicsAnimation::GetDefaultSpringParameters(stiffness, damping, mass);
    CHECK(IsNear(stiffness, oldStiffness));
    CHECK(IsNear(damping, oldDamping));
    CHECK(IsNear(mass, oldMass));

    //合法值生效
    ui::PhysicsAnimation::SetDefaultSpringParameters(100.0, 10.0, 2.0);
    ui::PhysicsAnimation::GetDefaultSpringParameters(stiffness, damping, mass);
    CHECK(IsNear(stiffness, 100.0));
    CHECK(IsNear(damping, 10.0));
    CHECK(IsNear(mass, 2.0));
    //恢复默认值，避免影响其它测试
    ui::PhysicsAnimation::SetDefaultSpringParameters(oldStiffness, oldDamping, oldMass);
    ui::PhysicsAnimation::GetDefaultSpringParameters(stiffness, damping, mass);
    CHECK(IsNear(stiffness, oldStiffness));
    CHECK(IsNear(damping, oldDamping));
    CHECK(IsNear(mass, oldMass));

    double friction = 0.0;
    double minVelocity = 0.0;
    ui::PhysicsAnimation::GetDefaultInertiaParameters(friction, minVelocity);
    const double oldFriction = friction;
    const double oldMinVelocity = minVelocity;
    CHECK(friction > 0.0);
    CHECK(minVelocity > 0.0);

    //非法值保持原有默认值不变
    ui::PhysicsAnimation::SetDefaultInertiaParameters(-1.0, -1.0);
    ui::PhysicsAnimation::GetDefaultInertiaParameters(friction, minVelocity);
    CHECK(IsNear(friction, oldFriction));
    CHECK(IsNear(minVelocity, oldMinVelocity));

    ui::PhysicsAnimation::SetDefaultInertiaParameters(3.0, 10.0);
    ui::PhysicsAnimation::GetDefaultInertiaParameters(friction, minVelocity);
    CHECK(IsNear(friction, 3.0));
    CHECK(IsNear(minVelocity, 10.0));
    ui::PhysicsAnimation::SetDefaultInertiaParameters(oldFriction, oldMinVelocity);
}

/** 弹簧积分器 */
void TestSpringSimulation()
{
    ui::SpringSimulation spring;
    CHECK(spring.IsFinished());
    CHECK(spring.GetValue() == 0.0);
    CHECK(spring.GetVelocity() == 0.0);

    double value = -1.0;
    //已静止：返回 false，并输出当前值
    CHECK(!spring.Advance(1.0 / 60.0, value));
    CHECK(value == 0.0);

    spring.Reset(0.0, 100.0);
    CHECK(!spring.IsFinished());
    CHECK(spring.GetValue() == 0.0);

    //时间增量为 0：保持不动，但仍处于运动状态
    CHECK(spring.Advance(0.0, value));
    CHECK(value == 0.0);

    //开始向目标值靠近
    CHECK(spring.Advance(1.0 / 60.0, value));
    CHECK(value > 0.0);

    //持续推进直到静止（默认参数为轻微欠阻尼，允许极小的回弹）
    bool bMoving = true;
    int32_t nSteps = 0;
    double previousValue = value;
    while (bMoving && (nSteps < 600)) {
        bMoving = spring.Advance(1.0 / 60.0, value);
        CHECK(value >= previousValue - 1.0);
        previousValue = value;
        ++nSteps;
    }
    CHECK(!bMoving);
    CHECK(nSteps < 600);
    CHECK(spring.IsFinished());
    CHECK(IsNear(spring.GetValue(), 100.0));
    CHECK(spring.GetVelocity() == 0.0);

    //播放过程中修改目标值：应重新开始运动，且从当前状态平滑过渡
    spring.Reset(0.0, 0.0);
    CHECK(spring.IsFinished());
    spring.SetTarget(50.0);
    CHECK(!spring.IsFinished());
    CHECK(spring.Advance(1.0 / 60.0, value));
    CHECK(value > 0.0);

    //起始速度不为 0 时不应判定为静止
    spring.Reset(0.0, 0.0, 100.0);
    CHECK(!spring.IsFinished());
    CHECK(spring.Advance(1.0 / 60.0, value));
    CHECK(value > 0.0);

    //临界阻尼（阻尼比 1.0）时不应超过目标值
    ui::SpringSimulation criticalSpring;
    criticalSpring.SetParametersByDampingRatio(100.0, 1.0, 1.0);
    criticalSpring.Reset(0.0, 100.0);
    bMoving = true;
    nSteps = 0;
    while (bMoving && (nSteps < 600)) {
        bMoving = criticalSpring.Advance(1.0 / 60.0, value);
        CHECK(value <= 100.0 + 1e-6);
        ++nSteps;
    }
    CHECK(!bMoving);
    CHECK(IsNear(criticalSpring.GetValue(), 100.0));
}

/** 惯性积分器 */
void TestInertiaSimulation()
{
    ui::InertiaSimulation inertia;
    double value = -1.0;
    CHECK(inertia.IsFinished());
    CHECK(!inertia.Advance(1.0 / 60.0, value));
    CHECK(value == 0.0);

    //初始速度低于最小速度阈值：不产生运动
    inertia.Reset(0.0, 10.0);
    CHECK(inertia.IsFinished());

    //正常滑行：向速度方向移动，速度逐步衰减
    inertia.Reset(0.0, 1000.0);
    CHECK(!inertia.IsFinished());
    const double initialVelocity = inertia.GetVelocity();
    CHECK(IsNear(initialVelocity, 1000.0));
    CHECK(inertia.Advance(1.0 / 60.0, value));
    CHECK(value > 0.0);
    CHECK(inertia.GetVelocity() < initialVelocity);
    CHECK(inertia.GetVelocity() > 0.0);

    bool bMoving = true;
    int32_t nSteps = 0;
    while (bMoving && (nSteps < 600)) {
        bMoving = inertia.Advance(1.0 / 60.0, value);
        ++nSteps;
    }
    CHECK(!bMoving);
    CHECK(inertia.IsFinished());
    //总滑行距离约为 v0 / friction = 200，停止时略小于该值
    CHECK(value > 190.0);
    CHECK(value <= 200.0 + 1e-6);

    //速度为负时向相反方向滑行
    inertia.Reset(0.0, -1000.0);
    CHECK(!inertia.IsFinished());
    CHECK(inertia.Advance(1.0 / 60.0, value));
    CHECK(value < 0.0);

    //到达取值边界后立即停止，并把值钳位在边界上
    inertia.SetValueRange(0.0, 50.0);
    inertia.Reset(100.0, 2000.0);
    CHECK(IsNear(inertia.GetValue(), 50.0));
    CHECK(!inertia.Advance(1.0 / 60.0, value));
    CHECK(IsNear(value, 50.0));
    CHECK(inertia.IsFinished());
    CHECK(inertia.GetVelocity() == 0.0);

    //最大持续时间：超过后强制停止（兜底）
    inertia.SetValueRange(-1.0e18, 1.0e18);
    inertia.SetParameters(5.0, 1.0, 100);
    inertia.Reset(0.0, 100000.0);
    bMoving = true;
    nSteps = 0;
    while (bMoving && (nSteps < 100)) {
        bMoving = inertia.Advance(0.02, value);
        ++nSteps;
    }
    CHECK(!bMoving);
    CHECK(inertia.IsFinished());
    CHECK(nSteps <= 6);
    CHECK(value > 0.0);
    CHECK(value < 100000.0 / 5.0);
}

/** AnimationPlayer 的默认属性与简单赋值 */
void TestAnimationPlayerProperties()
{
    ui::AnimationPlayer player;
    CHECK(player.GetAnimationType() == ui::AnimationType::kAnimationNone);
    CHECK(player.GetEasingFunctionType() == ui::EaseInOutCubic);
    CHECK(player.GetPhysicsType() == ui::AnimationPhysicsType::kNone);
    CHECK(player.GetStartValue() == 0);
    CHECK(player.GetEndValue() == 0);
    CHECK(player.GetCurrentValue() == 0);
    CHECK(!player.IsPlaying());
    CHECK(player.GetFrameIntervalMillSeconds() == -1);
    CHECK(player.GetTotalMillSeconds() == -1);

    //未设置物理类型时，即使全局开关开启也不启用物理动画
    CHECK(!player.IsPhysicsEnabled());
    player.SetPhysicsType(ui::AnimationPhysicsType::kSpring);
    CHECK(player.IsPhysicsEnabled());
    player.SetPhysicsType(ui::AnimationPhysicsType::kInertia);
    CHECK(player.IsPhysicsEnabled());
    player.SetPhysicsType(ui::AnimationPhysicsType::kNone);
    CHECK(!player.IsPhysicsEnabled());

    player.SetAnimationType(ui::AnimationType::kAnimationAlpha);
    CHECK(player.GetAnimationType() == ui::AnimationType::kAnimationAlpha);

    player.SetEasingFunctionType(ui::EaseLinear);
    CHECK(player.GetEasingFunctionType() == ui::EaseLinear);

    player.SetFrameIntervalMillSeconds(16);
    CHECK(player.GetFrameIntervalMillSeconds() == 16);
    player.SetTotalMillSeconds(200);
    CHECK(player.GetTotalMillSeconds() == 200);

    //未播放时，设置结束值直接生效
    player.SetStartValue(10);
    player.SetEndValue(20);
    CHECK(player.GetStartValue() == 10);
    CHECK(player.GetEndValue() == 20);
    CHECK(player.GetCurrentValue() == 0);

    player.Clear();
    CHECK(!player.GetPlayCallback());
    CHECK(!player.GetCompleteCallback());
    CHECK(!player.IsPlaying());
}

/** 起止值相同时，Start/ReverseStart 应同步完成（不注册定时器） */
void TestAnimationPlayerCompleteImmediately()
{
    ui::AnimationPlayer player;
    int32_t nPlayCount = 0;
    int32_t nCompleteCount = 0;
    int32_t nLastValue = -1;
    player.SetPlayCallback([&](int32_t nValue) {
        ++nPlayCount;
        nLastValue = nValue;
    });
    player.SetCompleteCallback([&]() {
        ++nCompleteCount;
    });
    player.SetStartValue(10);
    player.SetEndValue(10);

    player.Start();
    CHECK(!player.IsPlaying());
    CHECK(nCompleteCount == 1);
    //起止值相同，不会触发播放回调
    CHECK(nPlayCount == 0);
    CHECK(nLastValue == -1);
    //完成时把当前值对齐到结束值
    CHECK(player.GetCurrentValue() == 10);

    //反向播放：起止值相同，同样直接完成
    player.ReverseStart();
    CHECK(!player.IsPlaying());
    CHECK(nCompleteCount == 2);

    //Stop 后当前值复位到起始值，播放状态结束
    player.Stop();
    CHECK(!player.IsPlaying());
    CHECK(player.GetCurrentValue() == player.GetStartValue());
}

/** AnimationManager 的播放器管理与参数下发 */
void TestAnimationManager()
{
    ui::Label label(nullptr);
    ui::AnimationManager manager(&label);
    CHECK(manager.GetFrameIntervalMillSeconds() == -1);
    CHECK(manager.GetTotalMillSeconds() == -1);
    CHECK(manager.GetEasingFunctionType() == ui::EaseInOutCubic);
    CHECK(!manager.HasAnimationPlayer(ui::AnimationType::kAnimationAlpha));
    CHECK(manager.GetAnimationPlayer(ui::AnimationType::kAnimationAlpha) == nullptr);

    ui::AnimationPlayer* pAlpha = manager.SetFadeAlpha(true, 200);
    CHECK(pAlpha != nullptr);
    CHECK(manager.HasAnimationPlayer(ui::AnimationType::kAnimationAlpha));
    CHECK(manager.GetAnimationPlayer(ui::AnimationType::kAnimationAlpha) == pAlpha);
    CHECK(pAlpha->GetAnimationType() == ui::AnimationType::kAnimationAlpha);
    CHECK(pAlpha->GetStartValue() == 0);
    CHECK(pAlpha->GetEndValue() == 200);

    //参数变更后同步到已创建（未播放）的播放器
    manager.SetFrameIntervalMillSeconds(16);
    manager.SetTotalMillSeconds(300);
    manager.SetEasingFunctionType(ui::EaseLinear);
    CHECK(manager.GetFrameIntervalMillSeconds() == 16);
    CHECK(manager.GetTotalMillSeconds() == 300);
    CHECK(manager.GetEasingFunctionType() == ui::EaseLinear);
    CHECK(pAlpha->GetFrameIntervalMillSeconds() == 16);
    CHECK(pAlpha->GetTotalMillSeconds() == 300);
    CHECK(pAlpha->GetEasingFunctionType() == ui::EaseLinear);

    //新建的播放器沿用管理器的参数
    ui::AnimationPlayer* pAlpha2 = manager.SetFadeAlpha(true, 255);
    CHECK(pAlpha2 != nullptr);
    CHECK(pAlpha2 != pAlpha);
    CHECK(pAlpha2->GetFrameIntervalMillSeconds() == 16);
    CHECK(pAlpha2->GetTotalMillSeconds() == 300);
    CHECK(pAlpha2->GetEasingFunctionType() == ui::EaseLinear);
    CHECK(pAlpha2->GetStartValue() == 0);
    CHECK(pAlpha2->GetEndValue() == 255);

    //清除动画
    CHECK(manager.SetFadeAlpha(false, 0) == nullptr);
    CHECK(!manager.HasAnimationPlayer(ui::AnimationType::kAnimationAlpha));
    CHECK(manager.GetAnimationPlayer(ui::AnimationType::kAnimationAlpha) == nullptr);

    //清理所有动画资源
    CHECK(manager.SetFadeAlpha(true, 128) != nullptr);
    manager.Clear(&label);
    CHECK(!manager.HasAnimationPlayer(ui::AnimationType::kAnimationAlpha));
}

/** PopupAnimation 的全局配置与空窗口调用的安全行为 */
void TestPopupAnimation()
{
    const bool bOldEnabled = ui::PopupAnimation::IsEnabled();
    const int32_t nOldTotalMillSeconds = ui::PopupAnimation::GetTotalMillSeconds();
    const ui::EasingFunctionType oldEasing = ui::PopupAnimation::GetEasingFunctionType();
    CHECK(bOldEnabled);
    CHECK(nOldTotalMillSeconds == 120);
    CHECK(oldEasing == ui::EaseOutCubic);

    ui::PopupAnimation::SetEnabled(false);
    CHECK(!ui::PopupAnimation::IsEnabled());
    ui::PopupAnimation::SetEnabled(true);
    CHECK(ui::PopupAnimation::IsEnabled());

    ui::PopupAnimation::SetTotalMillSeconds(0);
    CHECK(ui::PopupAnimation::GetTotalMillSeconds() == 0);
    ui::PopupAnimation::SetTotalMillSeconds(200);
    CHECK(ui::PopupAnimation::GetTotalMillSeconds() == 200);

    ui::PopupAnimation::SetEasingFunctionType(ui::EaseOutQuad);
    CHECK(ui::PopupAnimation::GetEasingFunctionType() == ui::EaseOutQuad);

    //窗口为空时应安全返回，并释放传入的动画对象
    std::unique_ptr<ui::AnimationPlayer> pPlayer(new ui::AnimationPlayer);
    ui::PopupAnimation::PlayFadeIn(nullptr, pPlayer);
    CHECK(pPlayer == nullptr);

    //恢复默认配置，避免影响其它测试
    ui::PopupAnimation::SetEnabled(bOldEnabled);
    ui::PopupAnimation::SetTotalMillSeconds(nOldTotalMillSeconds);
    ui::PopupAnimation::SetEasingFunctionType(oldEasing);
}

/** 动画调度器的统一时间基准：没有动画推进时应保持不变 */
void TestAnimationSchedulerTime()
{
    const int64_t nFirst = ui::AnimationScheduler::GetTimeMillSeconds();
    const int64_t nSecond = ui::AnimationScheduler::GetTimeMillSeconds();
    CHECK(nFirst > 0);
    CHECK(nFirst == nSecond);
}

}//namespace

int main()
{
#if defined(_WIN32) && defined(_DEBUG)
    // 本测试不启动全局资源（GlobalManager::Startup），部分内部断言仅用于提示
    // "未初始化/非UI线程"，这里让断言只输出到调试器，避免弹出对话框导致测试挂起
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

    std::cout << "[run] TestEasingFunctionLookup" << std::endl;
    TestEasingFunctionLookup();
    std::cout << "[run] TestEasingCurves" << std::endl;
    TestEasingCurves();
    std::cout << "[run] TestEasingFunctionsClass" << std::endl;
    TestEasingFunctionsClass();
    std::cout << "[run] TestPhysicsGlobalConfig" << std::endl;
    TestPhysicsGlobalConfig();
    std::cout << "[run] TestSpringSimulation" << std::endl;
    TestSpringSimulation();
    std::cout << "[run] TestInertiaSimulation" << std::endl;
    TestInertiaSimulation();
    std::cout << "[run] TestAnimationPlayerProperties" << std::endl;
    TestAnimationPlayerProperties();
    std::cout << "[run] TestAnimationPlayerCompleteImmediately" << std::endl;
    TestAnimationPlayerCompleteImmediately();
    std::cout << "[run] TestAnimationManager" << std::endl;
    TestAnimationManager();
    std::cout << "[run] TestPopupAnimation" << std::endl;
    TestPopupAnimation();
    std::cout << "[run] TestAnimationSchedulerTime" << std::endl;
    TestAnimationSchedulerTime();

    std::cout << "animation tests: "
              << g_checkCount << " checks, "
              << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
