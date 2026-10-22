#include "SwitchForm.h"
#include "duilib/Control/Switch.h"

SwitchForm::SwitchForm()
{
}

SwitchForm::~SwitchForm()
{
}

DString SwitchForm::GetSkinFolder()
{
    return _T("controls");
}

DString SwitchForm::GetSkinFile()
{
    return _T("switch.xml");
}

void SwitchForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //选中/取消选中事件回显
    ui::Switch* pEventSwitch = dynamic_cast<ui::Switch*>(FindControl(_T("switch_event_demo")));
    if (pEventSwitch != nullptr) {
        pEventSwitch->AttachSelect([this](const ui::EventArgs& /*args*/) {
            UpdateEventText(true);
            return true;
            });
        pEventSwitch->AttachUnSelect([this](const ui::EventArgs& /*args*/) {
            UpdateEventText(false);
            return true;
            });
    }

    //动态进入加载状态
    ui::Button* pLoadingOn = dynamic_cast<ui::Button*>(FindControl(_T("btn_loading_on")));
    if (pLoadingOn != nullptr) {
        pLoadingOn->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Switch* pSwitch = dynamic_cast<ui::Switch*>(FindControl(_T("switch_loading_demo")));
            if (pSwitch != nullptr) {
                pSwitch->SetLoading(true);
            }
            return true;
            });
    }

    //动态结束加载状态
    ui::Button* pLoadingOff = dynamic_cast<ui::Button*>(FindControl(_T("btn_loading_off")));
    if (pLoadingOff != nullptr) {
        pLoadingOff->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Switch* pSwitch = dynamic_cast<ui::Switch*>(FindControl(_T("switch_loading_demo")));
            if (pSwitch != nullptr) {
                pSwitch->SetLoading(false);
            }
            return true;
            });
    }
}

void SwitchForm::UpdateEventText(bool bSelected)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("switch_event_label")));
    if (pLabel != nullptr) {
        pLabel->SetText(ui::GlobalManager::GetTextById(bSelected ? _T("STRID_SWITCH_EVENT_SELECTED") :
                                                               _T("STRID_SWITCH_EVENT_UNSELECTED")));
    }
}
