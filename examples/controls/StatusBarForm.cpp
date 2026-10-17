#include "StatusBarForm.h"

using namespace ui;

namespace
{
    /** 从语言文件读取指定 STRID 的文本（支持中英双语）
    */
    DString StatusBarLang(const DString& strId)
    {
        return GlobalManager::Instance().Lang().GetStringByID(strId);
    }
}

StatusBarForm::StatusBarForm() :
    m_bBusy(false),
    m_bConnected(true),
    m_nProgress(0),
    m_pProgress(nullptr)
{
}

StatusBarForm::~StatusBarForm()
{
}

DString StatusBarForm::GetSkinFolder()
{
    return _T("controls");
}

DString StatusBarForm::GetSkinFile()
{
    return _T("statusbar.xml");
}

void StatusBarForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //状态面板：就绪 / 忙碌
    ui::Button* pStateBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_toggle_state")));
    if (pStateBtn != nullptr) {
        pStateBtn->AttachClick([this](const ui::EventArgs&) {
            ToggleState();
            return true;
            });
    }

    //连接状态面板：已连接 / 未连接
    ui::Button* pConnBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_toggle_conn")));
    if (pConnBtn != nullptr) {
        pConnBtn->AttachClick([this](const ui::EventArgs&) {
            ToggleConnected();
            return true;
            });
    }

    //后台进度面板：推进进度
    ui::Button* pAdvanceBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_advance")));
    if (pAdvanceBtn != nullptr) {
        pAdvanceBtn->AttachClick([this](const ui::EventArgs&) {
            AdvanceProgress();
            return true;
            });
    }

    //状态栏内的后台进度条
    m_pProgress = dynamic_cast<ui::Progress*>(FindControl(_T("sb_progress")));
}

void StatusBarForm::ToggleState()
{
    m_bBusy = !m_bBusy;
    ui::Label* pState = dynamic_cast<ui::Label*>(FindControl(_T("sb_state")));
    if (pState != nullptr) {
        pState->SetText(StatusBarLang(m_bBusy ? _T("STRID_STATUSBAR_STATUS_BUSY")
                                              : _T("STRID_STATUSBAR_STATUS_READY")));
    }
}

void StatusBarForm::ToggleConnected()
{
    m_bConnected = !m_bConnected;
    ui::Label* pConn = dynamic_cast<ui::Label*>(FindControl(_T("sb_conn")));
    if (pConn != nullptr) {
        pConn->SetText(StatusBarLang(m_bConnected ? _T("STRID_STATUSBAR_CONN_ON")
                                                  : _T("STRID_STATUSBAR_CONN_OFF")));
    }
}

void StatusBarForm::AdvanceProgress()
{
    m_nProgress += 20;
    if (m_nProgress > 100) {
        m_nProgress = 0;
    }
    if (m_pProgress != nullptr) {
        m_pProgress->SetValue(m_nProgress);
    }
}
