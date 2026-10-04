#include "TestForm.h"
#include "MessageBoxForm.h"

TestForm::TestForm():
    m_nProgressValue(0.0)
{
}

TestForm::~TestForm()
{
}

DString TestForm::GetSkinFolder()
{
    return _T("controls");
}

DString TestForm::GetSkinFile()
{
    return _T("test.xml");
}

void TestForm::OnInitWindow()
{
    //启动一个定时器
    ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadUI,
        ui::UiBind(this, [this]() {
                OnTimer();
            }),
        100);

    //显示MessageBox的测试界面
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("message_box_btn")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& args) {
            MessageBoxForm* testForm = new MessageBoxForm();
            ui::WindowCreateParam createParam;
            createParam.m_dwStyle = ui::kWS_POPUP;
            createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
            createParam.m_windowTitle = _T("MessageBoxForm");
            createParam.m_bCenterWindow = true;
            testForm->CreateWnd(this, createParam);
            testForm->ShowModalFake();
            return true;
            });
    }
}

void TestForm::OnTimer()
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("progress_text")));
    std::vector<DString> controlList = {_T("progress11"), _T("progress12"), _T("progress13"), _T("progress14"),
                                        _T("progress21"), _T("progress22"), _T("progress23"), _T("progress24") };
    for (const DString& name : controlList) {
        ui::Progress* pProgress = dynamic_cast<ui::Progress*>(FindControl(name));
        if (pProgress != nullptr) {
            if (pLabel != nullptr) {
                pLabel->SetText(ui::StringUtil::Printf(_T("%d%%"), (int32_t)m_nProgressValue));                
            }
            pProgress->SetValue(m_nProgressValue);
        }
    }

    m_nProgressValue += 0.4;
    if (m_nProgressValue > 100.0) {
        m_nProgressValue = 0.0;
    }
}

