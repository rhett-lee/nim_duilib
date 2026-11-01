#include "PopconfirmForm.h"

PopconfirmForm::PopconfirmForm()
{
}

PopconfirmForm::~PopconfirmForm()
{
}

DString PopconfirmForm::GetSkinFolder()
{
    return _T("controls");
}

DString PopconfirmForm::GetSkinFile()
{
    return _T("popconfirm.xml");
}

void PopconfirmForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    BindStateEcho();
    BindResultEcho();
    BindKeepOpen();
}

void PopconfirmForm::BindStateEcho()
{
    ui::Popconfirm* pPopconfirm = dynamic_cast<ui::Popconfirm*>(FindControl(_T("popconfirm_basic")));
    if (pPopconfirm == nullptr) {
        return;
    }
    //打开状态变化事件：wParam 为 1/0，lParam 为关闭原因（见 ui::PopoverCloseReason）
    pPopconfirm->AttachOpenChanged([this](const ui::EventArgs& args) {
        const bool bOpen = (static_cast<int32_t>(args.wParam) != 0);
        const int32_t nReason = (bOpen ? 0 : static_cast<int32_t>(args.lParam));
        UpdateStateText(bOpen, nReason);
        return true;
        });
}

void PopconfirmForm::BindResultEcho()
{
    //各演示锚点都只需要"回显最近一次操作结果"，统一遍历绑定
    const DString viewNames[] = {
        _T("popconfirm_basic"),
        _T("popconfirm_icon_warning"),
        _T("popconfirm_icon_info"),
        _T("popconfirm_icon_success"),
        _T("popconfirm_icon_error"),
        _T("popconfirm_icon_none"),
        _T("popconfirm_ok_primary"),
        _T("popconfirm_ok_default"),
        _T("popconfirm_ok_danger"),
        _T("popconfirm_no_cancel"),
        _T("popconfirm_long"),
    };
    for (const DString& strName : viewNames) {
        ui::Popconfirm* pPopconfirm = dynamic_cast<ui::Popconfirm*>(FindControl(strName));
        if (pPopconfirm == nullptr) {
            continue;
        }
        pPopconfirm->AttachConfirm([this](const ui::EventArgs& /*args*/) {
            UpdateResultText(true);
            return true;
            });
        pPopconfirm->AttachCancel([this](const ui::EventArgs& /*args*/) {
            UpdateResultText(false);
            return true;
            });
    }
}

void PopconfirmForm::BindKeepOpen()
{
    ui::Popconfirm* pKeepOpen = dynamic_cast<ui::Popconfirm*>(FindControl(_T("popconfirm_keep")));
    if (pKeepOpen == nullptr) {
        return;
    }
    m_pKeepOpen = pKeepOpen;

    //close_on_ok="false"：点击卡片内的"确定"不关闭，这里再用按钮演示受控调用
    pKeepOpen->AttachConfirm([this](const ui::EventArgs& /*args*/) {
        UpdateResultText(true);
        return true;
        });
    pKeepOpen->AttachCancel([this](const ui::EventArgs& /*args*/) {
        UpdateResultText(false);
        return true;
        });
    pKeepOpen->AttachOpenChanged([this](const ui::EventArgs& args) {
        const bool bOpen = (static_cast<int32_t>(args.wParam) != 0);
        const int32_t nReason = (bOpen ? 0 : static_cast<int32_t>(args.lParam));
        UpdateStateText(bOpen, nReason);
        return true;
        });

    ui::Button* pConfirmBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_popconfirm_confirm")));
    if (pConfirmBtn != nullptr) {
        pConfirmBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (m_pKeepOpen != nullptr) {
                m_pKeepOpen->Confirm();
            }
            return true;
            });
    }

    ui::Button* pCancelBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_popconfirm_cancel")));
    if (pCancelBtn != nullptr) {
        pCancelBtn->AttachClick([this](const ui::EventArgs& /*args*/) {
            if (m_pKeepOpen != nullptr) {
                m_pKeepOpen->Cancel();
            }
            return true;
            });
    }
}

void PopconfirmForm::UpdateStateText(bool bOpen, int32_t nReason)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("popconfirm_state")));
    if (pLabel == nullptr) {
        return;
    }

    //先拼出状态短语，再填入前缀模板的 {0}：语言文件会去掉首尾空白，
    //若把分隔空格写在英文前缀尾部会被裁掉（中文用全角冒号不需要空格）
    DString strValue;
    if (bOpen) {
        strValue = GetLangText(_T("STRID_POPCONFIRM_STATE_OPEN"));
    }
    else {
        DString strReason;
        switch (static_cast<ui::PopoverCloseReason>(static_cast<uint32_t>(nReason))) {
        case ui::PopoverCloseReason::kManual:       strReason = _T("STRID_POPCONFIRM_REASON_MANUAL"); break;
        case ui::PopoverCloseReason::kClickOutside: strReason = _T("STRID_POPCONFIRM_REASON_CLICK_OUTSIDE"); break;
        case ui::PopoverCloseReason::kEscape:       strReason = _T("STRID_POPCONFIRM_REASON_ESCAPE"); break;
        case ui::PopoverCloseReason::kAnchorLost:   strReason = _T("STRID_POPCONFIRM_REASON_ANCHOR_LOST"); break;
        case ui::PopoverCloseReason::kConfirm:      strReason = _T("STRID_POPCONFIRM_REASON_CONFIRM"); break;
        case ui::PopoverCloseReason::kCancel:       strReason = _T("STRID_POPCONFIRM_REASON_CANCEL"); break;
        default:                                    strReason = _T("STRID_POPCONFIRM_REASON_UNKNOWN"); break;
        }
        strValue = GetLangText(_T("STRID_POPCONFIRM_STATE_CLOSED"));
        strValue += GetLangText(strReason.c_str());
        strValue += _T(")");
    }
    DString strState = GetLangText(_T("STRID_POPCONFIRM_STATE_PREFIX"));
    ui::StringUtil::ReplaceAll(_T("{0}"), strValue, strState);
    pLabel->SetText(strState);
}

void PopconfirmForm::UpdateResultText(bool bConfirm)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("popconfirm_result")));
    if (pLabel == nullptr) {
        return;
    }
    const DString strValue = GetLangText(bConfirm ? _T("STRID_POPCONFIRM_RESULT_CONFIRM")
                                                  : _T("STRID_POPCONFIRM_RESULT_CANCEL"));
    DString strResult = GetLangText(_T("STRID_POPCONFIRM_RESULT_PREFIX"));
    ui::StringUtil::ReplaceAll(_T("{0}"), strValue, strResult);
    pLabel->SetText(strResult);
}

DString PopconfirmForm::GetLangText(const DString& strId) const
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}
