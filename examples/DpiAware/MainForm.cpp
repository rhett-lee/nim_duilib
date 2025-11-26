//MainForm.cpp
#include "MainForm.h"

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

void MainForm::OnInitWindow()
{
    //窗口初始化完成，可以进行本Form的初始化
    UpdateUI();

    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("NewWindow")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            //弹出一个新窗口
            MainForm* window = new MainForm();
            ui::WindowCreateParam createParam;
            createParam.m_dwStyle = ui::kWS_POPUP | ui::kWS_VISIBLE;
            createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
            createParam.m_windowTitle = _T("DpiAware");
            createParam.m_bCenterWindow = true;
            window->CreateWnd(this, createParam);
            window->ShowWindow(ui::kSW_SHOW_NORMAL);
            return true;
            });
    }

    pButton = dynamic_cast<ui::Button*>(FindControl(_T("set_display_scale_factor")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            //修改界面显示比例
            ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("display_scale_factor")));
            if (pRichEdit != nullptr) {
                int32_t nNewDisplayScaleFactor = ui::StringUtil::StringToInt32(pRichEdit->GetText());
                if (nNewDisplayScaleFactor > 0) {
                    this->ChangeDisplayScale((uint32_t)nNewDisplayScaleFactor);
                    UpdateUI();
                }
            }
            return true;
            });
    }

    ui::Control* pGroupTest = FindControl(_T("group_box_test"));
    if (pGroupTest != nullptr) {
        pGroupTest->AttachPosChanged([this](const ui::EventArgs& /*args*/) {
            UpdateUI();
            return true;
            });
    }
}

void MainForm::OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)
{
    ui::WindowImplBase::OnWindowDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);
    UpdateUI();
}

LRESULT MainForm::OnSizeMsg(ui::WindowSizeType sizeType, const ui::UiSize& newWindowSize, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = ui::WindowImplBase::OnSizeMsg(sizeType, newWindowSize, nativeMsg, bHandled);
    UpdateUI();
    return lResult;
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

    const ui::Control* pGroupTest = FindControl(_T("group_box_test"));
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("group_box_pos")));
    if ((pLabel != nullptr) && (pGroupTest != nullptr)) {
        DString text = ui::StringUtil::Printf(_T("[left: %d, top: %d]"), pGroupTest->GetRect().left, pGroupTest->GetRect().top);
        pLabel->SetText(text);
    }
    
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("primary_monitor_display_scale")));
    if (pLabel != nullptr) {
        uint32_t nScaleFactor = ui::GlobalManager::Instance().Dpi().GetDisplayScaleFactor();
        float fScale = ui::GlobalManager::Instance().Dpi().GetDisplayScale();
        DString text = ui::StringUtil::Printf(_T("%d%% (DisplayScale: %.02f)"), nScaleFactor, fScale);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("window_display_scale")));
    if (pLabel != nullptr) {
        uint32_t nScaleFactor = Dpi().GetDisplayScaleFactor();
        float fScale = Dpi().GetDisplayScale();
        DString text = ui::StringUtil::Printf(_T("%d%% (DisplayScale: %.02f)"), nScaleFactor, fScale);
        pLabel->SetText(text);
    }
    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("display_scale_factor")));
    if (pRichEdit != nullptr) {
        uint32_t nScaleFactor = Dpi().GetDisplayScaleFactor();
        pRichEdit->SetTextNumber((int64_t)nScaleFactor);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("window_size")));
    if (pLabel != nullptr) {
        ui::UiRect rcWindow;
        GetWindowRect(rcWindow);
        DString text = ui::StringUtil::Printf(_T("W:%d, H:%d [Left:%d, Top:%d]"), rcWindow.Width(), rcWindow.Height(), rcWindow.left, rcWindow.top);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("window_client_size")));
    if (pLabel != nullptr) {
        ui::UiRect rcClient;
        GetClientRect(rcClient);
        DString text = ui::StringUtil::Printf(_T("W:%d, H:%d [Left:%d, Top:%d]"), rcClient.Width(), rcClient.Height(), rcClient.left, rcClient.top);
        pLabel->SetText(text);
    }
#ifdef DUILIB_BUILD_FOR_SDL
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("SDL_GetWindowSize")));
    if (pLabel != nullptr) {
        int32_t w = 0;
        int32_t h = 0;
        NativeWnd()->GetWindowSize(&w, &h);
        DString text = ui::StringUtil::Printf(_T("W:%d, H:%d"), w, h);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("SDL_GetWindowSizeInPixels")));
    if (pLabel != nullptr) {
        int32_t w = 0;
        int32_t h = 0;
        NativeWnd()->GetWindowSizeInPixels(&w, &h);
        DString text = ui::StringUtil::Printf(_T("W:%d, H:%d"), w, h);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("SDL_GetDisplayContentScale")));
    if (pLabel != nullptr) {
        float scale = NativeWnd()->GetDisplayContentScale();
        DString text = ui::StringUtil::Printf(_T("%.02f"), scale);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("SDL_GetWindowDisplayScale")));
    if (pLabel != nullptr) {
        float scale = NativeWnd()->GetWindowDisplayScale();
        DString text = ui::StringUtil::Printf(_T("%.02f"), scale);
        pLabel->SetText(text);
    }
    pLabel = dynamic_cast<ui::Label*>(FindControl(_T("SDL_GetWindowPixelDensity")));
    if (pLabel != nullptr) {
        float scale = NativeWnd()->GetWindowPixelDensity();
        DString text = ui::StringUtil::Printf(_T("%.02f"), scale);
        pLabel->SetText(text);
    }
#else
    ui::Control* pSDL = FindControl(_T("SDL"));
    if (pSDL != nullptr) {
        pSDL->SetVisible(false);
    }
#endif

    if (0) {
        //TODO: 最大化和还原时，界面的图片显示有异常，原因待查
        ui::Label* pTitle = dynamic_cast<ui::Label*>(FindControl(_T("title")));
        const ui::Control* pTest = FindControl(_T("group_box_test"));
        if (pTest && pTitle) {
            DString text = ui::StringUtil::Printf(_T("[W:%d, H:%d]"), pTest->GetWidth(), pTest->GetHeight());
            const ui::Box* pBox = dynamic_cast<const ui::Box*>(pTest);
            for (size_t i = 0; i < pBox->GetItemCount(); ++i) {
                ui::Control* p = pBox->GetItemAt(i);
                if (p->IsVisible()) {
                    text += ui::StringUtil::Printf(_T(" (%i)[T:%d, H:%d]"), (int)i, p->GetRect().top - pTest->GetRect().top, p->GetHeight());
                }
            }
            pTitle->SetText(text);
            //::OutputDebugString(text.c_str());
        }
    }
}
