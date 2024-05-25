//MainForm.cpp
#include "MainForm.h"

const std::wstring MainForm::kClassName = L"MainForm";

MainForm::MainForm()
{
}


MainForm::~MainForm()
{
}

std::wstring MainForm::GetSkinFolder()
{
    return L"dpi_aware";
}

std::wstring MainForm::GetSkinFile()
{
    return L"DpiAware.xml";
}

std::wstring MainForm::GetWindowClassName() const
{
    return kClassName;
}

void MainForm::OnInitWindow()
{
    //窗口初始化完成，可以进行本Form的初始化
    UpdateUI();

    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(L"NewWindow"));
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

    pButton = dynamic_cast<ui::Button*>(FindControl(L"SetDPI"));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            //修改DPI值
            ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(L"DPI"));
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
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(L"dpi_awareness"));
    if (pLabel != nullptr) {
        std::wstring text;
        ui::DpiAwarenessMode mode = ui::GlobalManager::Instance().Dpi().GetDpiAwareness();
        if (mode == ui::DpiAwarenessMode::kDpiUnaware) {
            text = L"kDpiUnaware";
        }
        else if (mode == ui::DpiAwarenessMode::kSystemDpiAware) {
            text = L"kSystemDpiAware";
        }
        else if (mode == ui::DpiAwarenessMode::kPerMonitorDpiAware) {
            text = L"kPerMonitorDpiAware";
        }
        else if (mode == ui::DpiAwarenessMode::kPerMonitorDpiAware_V2) {
            text = L"kPerMonitorDpiAware_V2";
        }
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(L"system_dpi"));
    if (pLabel != nullptr) {
        uint32_t nDPI = ui::GlobalManager::Instance().Dpi().GetDPI();
        std::wstring text = ui::StringHelper::UInt32ToString(nDPI);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(L"system_dpi_percent"));
    if (pLabel != nullptr) {
        uint32_t nScale = ui::GlobalManager::Instance().Dpi().GetScale();
        std::wstring text = ui::StringHelper::UInt32ToString(nScale);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(L"window_dpi"));
    if (pLabel != nullptr) {
        uint32_t nDPI = Dpi().GetDPI();
        std::wstring text = ui::StringHelper::UInt32ToString(nDPI);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(L"window_dpi_percent"));
    if (pLabel != nullptr) {
        uint32_t nScale = Dpi().GetScale();
        std::wstring text = ui::StringHelper::UInt32ToString(nScale);
        pLabel->SetText(text);
    }
}
