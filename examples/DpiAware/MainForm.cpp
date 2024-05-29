//MainForm.cpp
#include "MainForm.h"

const DString MainForm::kClassName = _T("MainForm");

MainForm::MainForm()
{
}


MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("dpi_aware");
}

DString MainForm::GetSkinFile()
{
    return _T("DpiAware.xml");
}

DString MainForm::GetWindowClassName() const
{
    return kClassName;
}

void MainForm::OnInitWindow()
{
    //窗口初始化完成，可以进行本Form的初始化
    UpdateUI();

    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("NewWindow")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            //弹出一个新窗口
            class MainForm2 : public MainForm
            {
            public:
                virtual void OnCloseWindow() override {};
            };

            MainForm2* window = new MainForm2();
            window->CreateWnd(nullptr, MainForm2::kClassName.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
            window->CenterWindow();
            window->ShowWindow();
            return true;
            });
    }

    pButton = dynamic_cast<ui::Button*>(FindControl(_T("SetDPI")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            //修改DPI值
            ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("DPI")));
            if (pRichEdit != nullptr) {
                int32_t nNewDpi = _wtoi(pRichEdit->GetText().c_str());
                if (nNewDpi > 0) {
                    ChangeDpi((uint32_t)nNewDpi);
                    UpdateUI();
                }
            }
            return true;
            });
    }
}

void MainForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMessage(0L);
}

void MainForm::OnWindowDpiChanged(uint32_t /*nOldDPI*/, uint32_t /*nNewDPI*/)
{
    UpdateUI();
}

void MainForm::UpdateUI()
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("dpi_awareness")));
    if (pLabel != nullptr) {
        DString text;
        ui::DpiAwarenessMode mode = ui::GlobalManager::Instance().Dpi().GetDpiAwareness();
        if (mode == ui::DpiAwarenessMode::kDpiUnaware) {
            text = _T("kDpiUnaware");
        }
        else if (mode == ui::DpiAwarenessMode::kSystemDpiAware) {
            text = _T("kSystemDpiAware");
        }
        else if (mode == ui::DpiAwarenessMode::kPerMonitorDpiAware) {
            text = _T("kPerMonitorDpiAware");
        }
        else if (mode == ui::DpiAwarenessMode::kPerMonitorDpiAware_V2) {
            text = _T("kPerMonitorDpiAware_V2");
        }
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("system_dpi")));
    if (pLabel != nullptr) {
        uint32_t nDPI = ui::GlobalManager::Instance().Dpi().GetDPI();
        DString text = ui::StringUtil::UInt32ToString(nDPI);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("system_dpi_percent")));
    if (pLabel != nullptr) {
        uint32_t nScale = ui::GlobalManager::Instance().Dpi().GetScale();
        DString text = ui::StringUtil::UInt32ToString(nScale);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("window_dpi")));
    if (pLabel != nullptr) {
        uint32_t nDPI = Dpi().GetDPI();
        DString text = ui::StringUtil::UInt32ToString(nDPI);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("window_dpi_percent")));
    if (pLabel != nullptr) {
        uint32_t nScale = Dpi().GetScale();
        DString text = ui::StringUtil::UInt32ToString(nScale);
        pLabel->SetText(text);
    }
}
