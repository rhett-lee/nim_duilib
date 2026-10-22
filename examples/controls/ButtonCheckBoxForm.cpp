#include "ButtonCheckBoxForm.h"
#include "duilib/Control/CheckBox.h"
#include "duilib/Control/Option.h"

ButtonCheckBoxForm::ButtonCheckBoxForm()
{
}

ButtonCheckBoxForm::~ButtonCheckBoxForm()
{
}

DString ButtonCheckBoxForm::GetSkinFolder()
{
    return _T("controls");
}

DString ButtonCheckBoxForm::GetSkinFile()
{
    return _T("button_checkbox.xml");
}

void ButtonCheckBoxForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //按钮点击次数统计
    ui::Button* pCounter = dynamic_cast<ui::Button*>(FindControl(_T("bcb_btn_counter")));
    if (pCounter != nullptr) {
        pCounter->AttachClick([this](const ui::EventArgs& /*args*/) {
            ++m_nClickCount;
            ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("bcb_click_label")));
            if (pLabel != nullptr) {
                pLabel->SetText(ui::GlobalManager::GetTextById(_T("STRID_BCB_EVENT_CLICK_COUNT")) + _T(" ") +
                                ui::StringUtil::Printf(_T("%d"), m_nClickCount));
            }
            return true;
            });
    }

    //CheckBox 勾选/取消勾选事件回显
    ui::CheckBox* pCheckEvent = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_event_demo")));
    if (pCheckEvent != nullptr) {
        pCheckEvent->AttachCheck([this, pCheckEvent](const ui::EventArgs& /*args*/) {
            UpdateCheckText(pCheckEvent, true);
            return true;
            });
        pCheckEvent->AttachUnCheck([this, pCheckEvent](const ui::EventArgs& /*args*/) {
            UpdateCheckText(pCheckEvent, false);
            return true;
            });
    }

    //三态复选框：用户点击时更新状态文字
    ui::CheckBox* pTristate = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_tristate_demo")));
    if (pTristate != nullptr) {
        pTristate->AttachCheck([this](const ui::EventArgs& /*args*/) {
            UpdateTristateText();
            return true;
            });
        pTristate->AttachUnCheck([this](const ui::EventArgs& /*args*/) {
            UpdateTristateText();
            return true;
            });
    }

    //三态：全选
    ui::Button* pCheckAll = dynamic_cast<ui::Button*>(FindControl(_T("btn_check_all")));
    if (pCheckAll != nullptr) {
        pCheckAll->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CheckBox* pCheck = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_tristate_demo")));
            if (pCheck != nullptr) {
                pCheck->SetPartChecked(false);
                pCheck->SetChecked(true, false);
            }
            UpdateTristateText();
            return true;
            });
    }

    //三态：半选
    ui::Button* pCheckPart = dynamic_cast<ui::Button*>(FindControl(_T("btn_check_part")));
    if (pCheckPart != nullptr) {
        pCheckPart->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CheckBox* pCheck = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_tristate_demo")));
            if (pCheck != nullptr) {
                //勾选模式下，只有 IsChecked() 为 true 时才会绘制图片；半选需要同时置位两个标志
                pCheck->SetChecked(true, false);
                pCheck->SetPartChecked(true);
            }
            UpdateTristateText();
            return true;
            });
    }

    //三态：取消
    ui::Button* pCheckNone = dynamic_cast<ui::Button*>(FindControl(_T("btn_check_none")));
    if (pCheckNone != nullptr) {
        pCheckNone->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::CheckBox* pCheck = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_tristate_demo")));
            if (pCheck != nullptr) {
                pCheck->SetPartChecked(false);
                pCheck->SetChecked(false, false);
            }
            UpdateTristateText();
            return true;
            });
    }

    //Option 选中事件回显（两组演示 + 综合表单的更新通道）
    const DString optionNames[] = {
        _T("option_a"), _T("option_b"), _T("option_c"),
        _T("option_x"), _T("option_y"), _T("option_z"),
        _T("form_channel_stable"), _T("form_channel_beta")
    };
    for (const DString& optionName : optionNames) {
        ui::Option* pOption = dynamic_cast<ui::Option*>(FindControl(optionName));
        if (pOption != nullptr) {
            pOption->AttachSelect([this, pOption](const ui::EventArgs& /*args*/) {
                UpdateOptionText(pOption);
                return true;
                });
        }
    }

    //综合表单：保存
    ui::Button* pSave = dynamic_cast<ui::Button*>(FindControl(_T("form_save")));
    if (pSave != nullptr) {
        pSave->AttachClick([this](const ui::EventArgs& /*args*/) {
            ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("bcb_form_label")));
            if (pLabel != nullptr) {
                pLabel->SetText(ui::GlobalManager::GetTextById(_T("STRID_BCB_FORM_SAVED")));
            }
            return true;
            });
    }

    //综合表单：取消（关闭演示窗口）
    ui::Button* pCancel = dynamic_cast<ui::Button*>(FindControl(_T("form_cancel")));
    if (pCancel != nullptr) {
        pCancel->AttachClick([this](const ui::EventArgs& /*args*/) {
            CloseWnd();
            return true;
            });
    }
}

void ButtonCheckBoxForm::UpdateTristateText()
{
    ui::CheckBox* pCheck = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_tristate_demo")));
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("bcb_tristate_label")));
    if ((pCheck == nullptr) || (pLabel == nullptr)) {
        return;
    }

    DString textId = _T("STRID_BCB_TRISTATE_NONE_TEXT");
    if (pCheck->IsChecked()) {
        textId = pCheck->IsPartChecked() ? _T("STRID_BCB_TRISTATE_PART_TEXT") : _T("STRID_BCB_TRISTATE_ALL_TEXT");
    }
    pLabel->SetText(ui::GlobalManager::GetTextById(textId));
}

void ButtonCheckBoxForm::UpdateCheckText(const ui::CheckBox* pCheckBox, bool bChecked)
{
    if (pCheckBox == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("bcb_check_label")));
    if (pLabel == nullptr) {
        return;
    }

    DString text = ui::GlobalManager::GetTextById(bChecked ? _T("STRID_BCB_EVENT_CHECKED") :
                                                              _T("STRID_BCB_EVENT_UNCHECKED"));
    text += _T(" ");
    text += pCheckBox->GetText();
    pLabel->SetText(text);
}

void ButtonCheckBoxForm::UpdateOptionText(const ui::Option* pOption)
{
    if (pOption == nullptr) {
        return;
    }
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("bcb_option_label")));
    if (pLabel == nullptr) {
        return;
    }

    DString text = ui::GlobalManager::GetTextById(_T("STRID_BCB_EVENT_OPTION"));
    text += _T(" ");
    text += pOption->GetText();
    pLabel->SetText(text);
}
