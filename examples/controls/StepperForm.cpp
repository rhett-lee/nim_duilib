#include "StepperForm.h"

StepperForm::StepperForm()
{
}

StepperForm::~StepperForm()
{
}

DString StepperForm::GetSkinFolder()
{
    return _T("controls");
}

DString StepperForm::GetSkinFile()
{
    return _T("stepper.xml");
}

void StepperForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    ui::Stepper* pStepper = dynamic_cast<ui::Stepper*>(FindControl(_T("stepper_demo")));
    if (pStepper != nullptr) {
        //初始回显
        UpdateStepText(pStepper);

        //当前步骤变化事件：索引通过 pEventData 携带（StepValue）
        pStepper->AttachStepChanged([this](const ui::EventArgs& args) {
            ui::Stepper* p = dynamic_cast<ui::Stepper*>(args.GetSender());
            if (p != nullptr) {
                UpdateStepText(p);
            }
            return true;
            });
    }

    //上一步
    ui::Button* pPrevBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_step_prev")));
    if ((pPrevBtn != nullptr) && (pStepper != nullptr)) {
        pPrevBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Stepper* p = dynamic_cast<ui::Stepper*>(FindControl(_T("stepper_demo")));
            if (p != nullptr) {
                p->Back();
            }
            return true;
            });
    }

    //下一步
    ui::Button* pNextBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_step_next")));
    if ((pNextBtn != nullptr) && (pStepper != nullptr)) {
        pNextBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Stepper* p = dynamic_cast<ui::Stepper*>(FindControl(_T("stepper_demo")));
            if (p != nullptr) {
                p->Next();
            }
            return true;
            });
    }

    //跳到第3步
    ui::Button* pGotoBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_step_goto")));
    if ((pGotoBtn != nullptr) && (pStepper != nullptr)) {
        pGotoBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Stepper* p = dynamic_cast<ui::Stepper*>(FindControl(_T("stepper_demo")));
            if (p != nullptr) {
                p->GoTo(2);
            }
            return true;
            });
    }

    //重置到第1步
    ui::Button* pResetBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_step_reset")));
    if ((pResetBtn != nullptr) && (pStepper != nullptr)) {
        pResetBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Stepper* p = dynamic_cast<ui::Stepper*>(FindControl(_T("stepper_demo")));
            if (p != nullptr) {
                p->SetCurrentIndex(0);
            }
            return true;
            });
    }
}

void StepperForm::UpdateStepText(ui::Stepper* pStepper)
{
    if (pStepper == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("step_event")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strPrefix = ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_STEPPER_EVENT_PREFIX"));
    pLabel->SetText(strPrefix + ui::StringUtil::Printf(_T("%d / %d"),
                   static_cast<int>(pStepper->GetCurrentIndex() + 1),
                   static_cast<int>(pStepper->GetStepCount())));
}
