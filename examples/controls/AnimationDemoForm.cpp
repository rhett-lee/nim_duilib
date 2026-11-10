#include "AnimationDemoForm.h"
#include "duilib/Utils/StringUtil.h"

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
