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
    m_nPanelTextRound(0),
    m_pProgress(nullptr),
    m_pStatusBar(nullptr)
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

    //分格索引接口演示：按索引直接设置面板文本，无需 FindControl 找子 Label
    ui::Button* pPanelBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_panel_text")));
    if (pPanelBtn != nullptr) {
        pPanelBtn->AttachClick([this](const ui::EventArgs&) {
            SetPanelTextByIndex();
            return true;
            });
    }

    //尺寸手柄拖拽开关演示
    ui::Button* pGripBtn = dynamic_cast<ui::Button*>(FindControl(_T("btn_grip_resize")));
    if (pGripBtn != nullptr) {
        pGripBtn->AttachClick([this](const ui::EventArgs&) {
            ToggleSizeGripResize();
            return true;
            });
    }

    //状态栏内的后台进度条
    m_pProgress = dynamic_cast<ui::Progress*>(FindControl(_T("sb_progress")));

    m_pStatusBar = dynamic_cast<ui::StatusBar*>(FindControl(_T("status_bar_main")));
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

void StatusBarForm::SetPanelTextByIndex()
{
    if (m_pStatusBar == nullptr) {
        return;
    }

    //面板索引按布局顺序编号（只统计可见面板），与视觉上看到的分格顺序一致
    const size_t nPanelCount = m_pStatusBar->GetPanelCount();
    if (nPanelCount == 0) {
        return;
    }

    //每次点击轮换第 0 格（"状态"面板）的文本，演示按索引读写
    ++m_nPanelTextRound;
    const bool bBusy = (m_nPanelTextRound % 2) != 0;
    const DString strText = StatusBarLang(bBusy ? _T("STRID_STATUSBAR_STATUS_BUSY")
                                                : _T("STRID_STATUSBAR_STATUS_READY"));
    if (!m_pStatusBar->SetPanelText(0, strText)) {
        //第 0 格不是文本类控件时返回 false
        return;
    }

    //回读确认：GetPanelText / GetPanel / GetPanelIndex 与 SetPanelText 自洽
    const DString strReadBack = m_pStatusBar->GetPanelText(0);
    ui::Control* pPanel = m_pStatusBar->GetPanel(0);
    const int32_t nIndex = (pPanel != nullptr) ? m_pStatusBar->GetPanelIndex(pPanel) : -1;
    if ((strReadBack != strText) || (pPanel == nullptr) || (nIndex != 0)) {
        //索引接口自洽性校验失败（正常不会发生）
        return;
    }
}

void StatusBarForm::ToggleSizeGripResize()
{
    if (m_pStatusBar == nullptr) {
        return;
    }
    //关闭后手柄仅作视觉提示，不再拖拽调整窗口大小
    const bool bResize = !m_pStatusBar->IsSizeGripResize();
    m_pStatusBar->SetSizeGripResize(bResize);
}
