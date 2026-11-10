#include "AnimationDemoForm.h"
#include "duilib/Utils/StringUtil.h"
#include <cmath>

AnimationDemoForm::AnimationDemoForm():
    m_nTotalMillSeconds(480),
    m_easingType(ui::EasingFunctionType::EaseOutCubic)
{
}

AnimationDemoForm::~AnimationDemoForm()
{
}

DString AnimationDemoForm::GetSkinFolder()
{
    return _T("controls");
}

DString AnimationDemoForm::GetSkinFile()
{
    return _T("animation_demo.xml");
}

void AnimationDemoForm::OnInitWindow()
{
    InitGlobalSettings();
    InitPropertyDemo();
    InitSlideDemo();
    InitEasingDemo();
    InitRetargetDemo();
    InitPhysicsDemo();

    //把默认的全局参数应用到所有演示控件
    ApplyGlobalSettings();
}

void AnimationDemoForm::InitGlobalSettings()
{
    BindButtonClick(_T("btn_duration_200"), [this]() {
        m_nTotalMillSeconds = 200;
        ApplyGlobalSettings();
    });
    BindButtonClick(_T("btn_duration_480"), [this]() {
        m_nTotalMillSeconds = 480;
        ApplyGlobalSettings();
    });
    BindButtonClick(_T("btn_duration_900"), [this]() {
        m_nTotalMillSeconds = 900;
        ApplyGlobalSettings();
    });

    BindButtonClick(_T("btn_easing_out_cubic"), [this]() {
        m_easingType = ui::EasingFunctionType::EaseOutCubic;
        ApplyGlobalSettings();
    });
    BindButtonClick(_T("btn_easing_inout_cubic"), [this]() {
        m_easingType = ui::EasingFunctionType::EaseInOutCubic;
        ApplyGlobalSettings();
    });
    BindButtonClick(_T("btn_easing_out_back"), [this]() {
        m_easingType = ui::EasingFunctionType::EaseOutBack;
        ApplyGlobalSettings();
    });
    BindButtonClick(_T("btn_easing_out_elastic"), [this]() {
        m_easingType = ui::EasingFunctionType::EaseOutElastic;
        ApplyGlobalSettings();
    });
}

void AnimationDemoForm::InitPropertyDemo()
{
    //为一个控件配置多种属性动画，它们会由同一个定时器在同一帧内一起推进
    ui::Control* pControl = FindControl(_T("demo_alpha"));
    if (pControl != nullptr) {
        pControl->GetAnimationManager().SetFadeAlpha(true, 255);
    }

    pControl = FindControl(_T("demo_width"));
    if (pControl != nullptr) {
        pControl->GetAnimationManager().SetFadeWidth(true);
    }

    pControl = FindControl(_T("demo_height"));
    if (pControl != nullptr) {
        pControl->GetAnimationManager().SetFadeHeight(true);
    }

    pControl = FindControl(_T("demo_size"));
    if (pControl != nullptr) {
        pControl->GetAnimationManager().SetFadeSize(true);
    }

    BindButtonClick(_T("btn_alpha"), [this]() { ToggleControlVisible(_T("demo_alpha")); });
    BindButtonClick(_T("btn_width"), [this]() { ToggleControlVisible(_T("demo_width")); });
    BindButtonClick(_T("btn_height"), [this]() { ToggleControlVisible(_T("demo_height")); });
    BindButtonClick(_T("btn_size"), [this]() { ToggleControlVisible(_T("demo_size")); });
}

void AnimationDemoForm::InitSlideDemo()
{
    ui::Control* pControl = FindControl(_T("slide_left"));
    if (pControl != nullptr) {
        pControl->GetAnimationManager().SetFadeInOutX(true, false);
    }

    pControl = FindControl(_T("slide_right"));
    if (pControl != nullptr) {
        pControl->GetAnimationManager().SetFadeInOutX(true, true);
    }

    pControl = FindControl(_T("slide_top"));
    if (pControl != nullptr) {
        pControl->GetAnimationManager().SetFadeInOutY(true, false);
    }

    pControl = FindControl(_T("slide_bottom"));
    if (pControl != nullptr) {
        pControl->GetAnimationManager().SetFadeInOutY(true, true);
    }

    BindButtonClick(_T("btn_slide_in"), [this]() { SetSlideControlsVisible(true); });
    BindButtonClick(_T("btn_slide_out"), [this]() { SetSlideControlsVisible(false); });
}

void AnimationDemoForm::InitEasingDemo()
{
    const DString boxNames[] =
    {
        _T("ease_box_0"),
        _T("ease_box_1"),
        _T("ease_box_2"),
        _T("ease_box_3")
    };
    for (const DString& boxName : boxNames) {
        ui::Control* pControl = FindControl(boxName);
        if (pControl != nullptr) {
            pControl->GetAnimationManager().SetFadeWidth(true);
        }
    }

    BindButtonClick(_T("btn_easing_compare"), [this]() { PlayEasingCompare(); });
}

void AnimationDemoForm::InitRetargetDemo()
{
    ui::Control* pControl = FindControl(_T("retarget_box"));
    if (pControl != nullptr) {
        ui::AnimationPlayer* pAnimationPlayer = pControl->GetAnimationManager().SetFadeWidth(true);
        if (pAnimationPlayer != nullptr) {
            pAnimationPlayer->SetStartValue(0);
            pAnimationPlayer->SetEndValue(120);
        }
    }

    BindButtonClick(_T("btn_retarget_120"), [this]() { SetRetargetWidth(120); });
    BindButtonClick(_T("btn_retarget_240"), [this]() { SetRetargetWidth(240); });
    BindButtonClick(_T("btn_retarget_360"), [this]() { SetRetargetWidth(360); });
    BindButtonClick(_T("btn_retarget_restart"), [this]() {
        ui::Control* pBox = FindControl(_T("retarget_box"));
        if (pBox == nullptr) {
            return;
        }
        ui::AnimationPlayer* pAnimationPlayer = pBox->GetAnimationManager().GetAnimationPlayer(ui::AnimationType::kAnimationWidth);
        if (pAnimationPlayer == nullptr) {
            return;
        }
        pAnimationPlayer->Stop();
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(120);
        pAnimationPlayer->SetTotalMillSeconds(m_nTotalMillSeconds);
        pAnimationPlayer->Start();
    });
}

void AnimationDemoForm::InitPhysicsDemo()
{
    //物理动画总开关：默认开启；本页的 Panel 与列表还需各自启用物理动画
    ui::PhysicsAnimation::SetEnabled(true);

    //1) 弹簧动画：可折叠面板
    ui::PanelVBox* pPanel = dynamic_cast<ui::PanelVBox*>(FindControl(_T("physics_panel")));
    if (pPanel != nullptr) {
        pPanel->SetCollapsePhysicsEnabled(true);
        pPanel->SetCollapseAnimMillSeconds(600);
        //阻尼比约 0.74：带轻微回弹（约 5%），既能看到弹簧效果，又不会把面板"打穿"到 0 高度
        pPanel->SetCollapseSpringParameters(220.0, 22.0, 1.0);
    }

    //2) 惯性动画：列表滚动
    ui::VListBox* pList = dynamic_cast<ui::VListBox*>(FindControl(_T("physics_list")));
    if (pList != nullptr) {
        pList->SetInertiaScrollEnabled(true);
        pList->SetInertiaScrollParameters(5.0, 0.8);

        //创建列表项（数量足够多，才能滚动并观察惯性滑行）
        const DString strItemPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_ANIM_PHYSICS_ITEM"));
        for (int32_t nIndex = 0; nIndex < 30; ++nIndex) {
            ui::ListBoxItem* pItem = new ui::ListBoxItem(this);
            pItem->SetClass(_T("list_item"));
            pItem->SetText(ui::StringUtil::Printf(_T("%s %02d"), strItemPrefix.c_str(), nIndex + 1));
            pItem->SetFixedHeight(ui::UiFixedInt(32), true, true);
            pList->AddItem(pItem);
        }
    }

    //3) 物理动画总开关：关闭后 Panel 与列表回退为缓动动画
    BindButtonClick(_T("btn_physics_toggle"), [this]() {
        ui::PhysicsAnimation::SetEnabled(!ui::PhysicsAnimation::IsEnabled());
        UpdatePhysicsStateLabel();
    });

    //4) 物理动画自检：不依赖单元测试框架，直接驱动积分器校验数值行为
    BindButtonClick(_T("btn_physics_selftest"), [this]() {
        RunPhysicsSelfTest();
    });
    UpdatePhysicsStateLabel();
}

void AnimationDemoForm::UpdatePhysicsStateLabel()
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_physics_state")));
    if (pLabel == nullptr) {
        return;
    }
    const bool bEnabled = ui::PhysicsAnimation::IsEnabled();
    pLabel->SetText(ui::GlobalManager::Instance().Lang().GetStringByID(
        bEnabled ? _T("STRID_ANIM_PHYSICS_STATE_ON") : _T("STRID_ANIM_PHYSICS_STATE_OFF")));
    pLabel->SetStateTextColor(ui::ControlStateType::kControlStateNormal,
                              bEnabled ? _T("color_success") : _T("color_error"));
}

namespace
{
/** 物理动画自检的统计结果
*/
struct PhysicsSelfTestResult
{
    //通过的检查项数量
    int32_t nPassed = 0;

    //失败的检查项数量
    int32_t nFailed = 0;

    //失败的检查项名称（便于定位问题）
    DString strFailedNames;

    /** 记录一项检查结果
    * @param [in] bPass 检查是否通过
    * @param [in] strName 检查项名称
    */
    void Check(bool bPass, const DString& strName)
    {
        if (bPass) {
            ++nPassed;
            return;
        }
        ++nFailed;
        if (!strFailedNames.empty()) {
            strFailedNames += _T(", ");
        }
        strFailedNames += strName;
    }
};
}//namespace

void AnimationDemoForm::RunPhysicsSelfTest()
{
    PhysicsSelfTestResult result;

    //1) 弹簧：0 → 100 应在 2 秒内静止，并精确落到目标值；同时超调很小（不超过 3 像素）
    {
        ui::SpringSimulation spring;
        spring.SetParameters(220.0, 26.0, 1.0);
        spring.Reset(0.0, 100.0);
        double value = 0.0;
        double maxValue = 0.0;
        int32_t nFrames = 0;
        while (!spring.IsFinished() && (nFrames < 120)) {
            spring.Advance(1.0 / 60.0, value);
            maxValue = (value > maxValue) ? value : maxValue;
            ++nFrames;
        }
        result.Check(spring.IsFinished() && (std::abs(value - 100.0) < 0.01), _T("spring_converge"));
        result.Check((maxValue >= 100.0) && (maxValue <= 103.0), _T("spring_overshoot"));
    }

    //2) 弹簧：播放中修改目标值，应从当前状态继续过渡，不回跳到起点
    {
        ui::SpringSimulation spring;
        spring.SetParameters(220.0, 26.0, 1.0);
        spring.Reset(0.0, 100.0);
        double value = 0.0;
        for (int32_t nFrame = 0; nFrame < 10; ++nFrame) {
            spring.Advance(1.0 / 60.0, value);
        }
        const double valueBeforeRetarget = value;
        spring.SetTarget(300.0);
        spring.Advance(1.0 / 60.0, value);
        result.Check((valueBeforeRetarget > 0.0) && (value > valueBeforeRetarget) && !spring.IsFinished(),
                     _T("spring_retarget"));
    }

    //3) 惯性：初速度 1000 像素/秒、摩擦 5/秒 → 总滑行距离约 (1000 - 30) / 5 ≈ 194 像素
    {
        ui::InertiaSimulation inertia;
        inertia.SetParameters(5.0, 30.0, 3000);
        inertia.SetValueRange(0.0, 100000.0);
        inertia.Reset(0.0, 1000.0);
        double value = 0.0;
        int32_t nFrames = 0;
        while (!inertia.IsFinished() && (nFrames < 600)) {
            inertia.Advance(1.0 / 60.0, value);
            ++nFrames;
        }
        result.Check(inertia.IsFinished(), _T("inertia_stop"));
        result.Check((value > 185.0) && (value < 200.0), _T("inertia_distance"));
    }

    //4) 惯性：遇到取值边界立即停住，不越界
    {
        ui::InertiaSimulation inertia;
        inertia.SetParameters(5.0, 30.0, 3000);
        inertia.SetValueRange(0.0, 50.0);
        inertia.Reset(0.0, 1000.0);
        double value = 0.0;
        int32_t nFrames = 0;
        while (!inertia.IsFinished() && (nFrames < 600)) {
            inertia.Advance(1.0 / 60.0, value);
            ++nFrames;
        }
        result.Check(inertia.IsFinished() && (std::abs(value - 50.0) < 0.001), _T("inertia_clamp"));
    }

    //5) 总开关：关闭后 IsEnabled 返回 false，恢复后回到原来的状态
    {
        const bool bOldEnabled = ui::PhysicsAnimation::IsEnabled();
        ui::PhysicsAnimation::SetEnabled(false);
        const bool bDisabled = !ui::PhysicsAnimation::IsEnabled();
        ui::PhysicsAnimation::SetEnabled(bOldEnabled);
        result.Check(bDisabled && (ui::PhysicsAnimation::IsEnabled() == bOldEnabled), _T("global_switch"));
    }

    //把结果显示到页面：通过/失败数量，失败时附带失败项名称
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_physics_selftest")));
    if (pLabel == nullptr) {
        return;
    }
    const int32_t nTotal = result.nPassed + result.nFailed;
    const DString strFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_ANIM_PHYSICS_TEST_RESULT"));
    DString strText = ui::StringUtil::Printf(strFormat.c_str(), result.nPassed, nTotal);
    if (!result.strFailedNames.empty()) {
        const DString strFailFormat = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_ANIM_PHYSICS_TEST_FAILED"));
        strText += ui::StringUtil::Printf(strFailFormat.c_str(), result.strFailedNames.c_str());
    }
    pLabel->SetText(strText);
    pLabel->SetStateTextColor(ui::ControlStateType::kControlStateNormal,
                              (result.nFailed == 0) ? _T("color_success") : _T("color_error"));
}

void AnimationDemoForm::ApplyGlobalSettings()
{
    const DString controlNames[] =
    {
        _T("demo_alpha"),
        _T("demo_width"),
        _T("demo_height"),
        _T("demo_size"),
        _T("slide_left"),
        _T("slide_right"),
        _T("slide_top"),
        _T("slide_bottom"),
        _T("ease_box_0"),
        _T("ease_box_1"),
        _T("ease_box_2"),
        _T("ease_box_3"),
        _T("retarget_box")
    };
    for (const DString& controlName : controlNames) {
        ui::Control* pControl = FindControl(controlName);
        if (pControl == nullptr) {
            continue;
        }
        ui::AnimationManager& animationManager = pControl->GetAnimationManager();
        animationManager.SetTotalMillSeconds(m_nTotalMillSeconds);
        //进场、出场使用同一个缓动函数（默认分别是 EaseOutCubic 和 EaseInCubic）
        animationManager.SetEasingFunctionType(m_easingType, m_easingType);
    }

    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("label_duration")));
    if (pLabel != nullptr) {
        pLabel->SetText(ui::StringUtil::Printf(_T("%d ms"), m_nTotalMillSeconds));
    }
}

void AnimationDemoForm::ToggleControlVisible(const DString& controlName)
{
    ui::Control* pControl = FindControl(controlName);
    if (pControl == nullptr) {
        return;
    }
    //SetFadeVisible 内部按已配置的动画播放，不需要再调用 SetVisible
    pControl->SetFadeVisible(!pControl->IsVisible());
}

void AnimationDemoForm::SetSlideControlsVisible(bool bVisible)
{
    const DString controlNames[] =
    {
        _T("slide_left"),
        _T("slide_right"),
        _T("slide_top"),
        _T("slide_bottom")
    };
    for (const DString& controlName : controlNames) {
        ui::Control* pControl = FindControl(controlName);
        if (pControl != nullptr) {
            pControl->SetFadeVisible(bVisible);
        }
    }
}

void AnimationDemoForm::PlayEasingCompare()
{
    const DString boxNames[] =
    {
        _T("ease_box_0"),
        _T("ease_box_1"),
        _T("ease_box_2"),
        _T("ease_box_3")
    };
    const DString trackNames[] =
    {
        _T("ease_track_0"),
        _T("ease_track_1"),
        _T("ease_track_2"),
        _T("ease_track_3")
    };
    const ui::EasingFunctionType easingTypes[] =
    {
        ui::EasingFunctionType::EaseLinear,
        ui::EasingFunctionType::EaseOutCubic,
        ui::EasingFunctionType::EaseOutBack,
        ui::EasingFunctionType::EaseOutElastic
    };
    const int32_t nBoxWidth = 40;

    for (int32_t nIndex = 0; nIndex < 4; ++nIndex) {
        ui::Control* pBox = FindControl(boxNames[nIndex]);
        ui::Control* pTrack = FindControl(trackNames[nIndex]);
        if ((pBox == nullptr) || (pTrack == nullptr)) {
            continue;
        }
        int32_t nDistance = pTrack->GetRect().Width();
        if (nDistance < nBoxWidth) {
            nDistance = nBoxWidth;
        }
        ui::AnimationPlayer* pAnimationPlayer = pBox->GetAnimationManager().GetAnimationPlayer(ui::AnimationType::kAnimationWidth);
        if (pAnimationPlayer == nullptr) {
            pAnimationPlayer = pBox->GetAnimationManager().SetFadeWidth(true);
        }
        if (pAnimationPlayer == nullptr) {
            continue;
        }
        //所有动画在同一帧内启动，使用同一个时间基准，可以直观对比缓动曲线的差异
        pAnimationPlayer->Stop();
        pAnimationPlayer->SetStartValue(0);
        pAnimationPlayer->SetEndValue(nDistance);
        pAnimationPlayer->SetTotalMillSeconds(m_nTotalMillSeconds);
        pAnimationPlayer->SetEasingFunctionType(easingTypes[nIndex]);
        pAnimationPlayer->Start();
    }
}

void AnimationDemoForm::SetRetargetWidth(int32_t nNewWidth)
{
    ui::Control* pControl = FindControl(_T("retarget_box"));
    if (pControl == nullptr) {
        return;
    }
    ui::AnimationPlayer* pAnimationPlayer = pControl->GetAnimationManager().GetAnimationPlayer(ui::AnimationType::kAnimationWidth);
    if (pAnimationPlayer == nullptr) {
        return;
    }
    if (pAnimationPlayer->IsPlaying()) {
        //播放中修改结束值：以当前播放值作为新起点续播，不会回跳到起始值
        pAnimationPlayer->SetEndValue(nNewWidth);
        return;
    }
    //未在播放中：从控件当前的宽度开始播放到新的宽度
    pAnimationPlayer->SetStartValue(pControl->GetRect().Width());
    pAnimationPlayer->SetEndValue(nNewWidth);
    pAnimationPlayer->SetTotalMillSeconds(m_nTotalMillSeconds);
    pAnimationPlayer->Start();
}

void AnimationDemoForm::BindButtonClick(const DString& buttonName, const std::function<void()>& handler)
{
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(buttonName));
    if (pButton == nullptr) {
        return;
    }
    pButton->AttachClick([handler](const ui::EventArgs& /*args*/) {
        if (handler) {
            handler();
        }
        return true;
    });
}
