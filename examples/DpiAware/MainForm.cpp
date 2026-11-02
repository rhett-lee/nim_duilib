//MainForm.cpp
#include "MainForm.h"

namespace {

/** 生成"宽 × 高"形式的尺寸文本
*/
DString FormatSize(int32_t nWidth, int32_t nHeight)
{
    return ui::StringUtil::Printf(_T("%d \u00D7 %d"), nWidth, nHeight);
}

/** 生成"百分比 (DisplayScale: x.xx)"形式的缩放文本
*/
DString FormatScale(uint32_t nScaleFactor, float fScale)
{
    return ui::StringUtil::Printf(_T("%d%% (%.02f)"), (int32_t)nScaleFactor, fScale);
}

/** 获取 DPI 感知模式的显示文本（枚举名称 + 简要说明）
*/
DString FormatAwareness(ui::DpiAwarenessMode mode)
{
    DString strText;
    DString strTipId;
    switch (mode) {
    case ui::DpiAwarenessMode::kDpiUnaware:
        strText = _T("kDpiUnaware");
        strTipId = _T("STRID_DPIAWARE_AWARENESS_UNAWARE");
        break;
    case ui::DpiAwarenessMode::kSystemDpiAware:
        strText = _T("kSystemDpiAware");
        strTipId = _T("STRID_DPIAWARE_AWARENESS_SYSTEM");
        break;
    case ui::DpiAwarenessMode::kPerMonitorDpiAware:
        strText = _T("kPerMonitorDpiAware");
        strTipId = _T("STRID_DPIAWARE_AWARENESS_PER_MONITOR");
        break;
    case ui::DpiAwarenessMode::kPerMonitorDpiAware_V2:
        strText = _T("kPerMonitorDpiAware_V2");
        strTipId = _T("STRID_DPIAWARE_AWARENESS_PER_MONITOR_V2");
        break;
    default:
        break;
    }
    if (!strTipId.empty()) {
        strText += _T(" ");
        strText += ui::GlobalManager::Instance().Lang().GetStringByID(strTipId);
    }
    return strText;
}

}

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
            //弹出一个新窗口，用于对比不同窗口的 DPI 缩放结果
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

void MainForm::SetLabelText(const DString& strName, const DString& strText)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(strName));
    if (pLabel != nullptr) {
        pLabel->SetText(strText);
    }
}

void MainForm::UpdateUI()
{
    SetLabelText(_T("dpi_awareness"), FormatAwareness(ui::GlobalManager::Instance().Dpi().GetDpiAwareness()));
    SetLabelText(_T("primary_monitor_display_scale"),
                 FormatScale(ui::GlobalManager::Instance().Dpi().GetDisplayScaleFactor(),
                             ui::GlobalManager::Instance().Dpi().GetDisplayScale()));
    SetLabelText(_T("window_display_scale"),
                 FormatScale(Dpi().GetDisplayScaleFactor(), Dpi().GetDisplayScale()));

    //窗口与客户区的大小，单位是物理像素（受 DPI 缩放影响）
    ui::UiRect rcWindow;
    GetWindowRect(rcWindow);
    SetLabelText(_T("window_size"), FormatSize(rcWindow.Width(), rcWindow.Height()));

    ui::UiRect rcClient;
    GetClientRect(rcClient);
    SetLabelText(_T("window_client_size"), FormatSize(rcClient.Width(), rcClient.Height()));

    //内容区的大小，单位是逻辑像素（界面布局使用的坐标，不随 DPI 缩放变化）
    const ui::Control* pContentArea = FindControl(_T("content_area"));
    if (pContentArea != nullptr) {
        const ui::UiRect rcContentArea = pContentArea->GetRect();
        SetLabelText(_T("content_size"), FormatSize(rcContentArea.Width(), rcContentArea.Height()));
    }

    //缩放比例输入框：用户正在输入时不覆盖，避免打断输入
    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("display_scale_factor")));
    if ((pRichEdit != nullptr) && !pRichEdit->IsFocused()) {
        pRichEdit->SetTextNumber((int64_t)Dpi().GetDisplayScaleFactor());
    }

#ifdef DUILIB_BUILD_FOR_SDL
    int32_t nWidth = 0;
    int32_t nHeight = 0;
    NativeWnd()->GetWindowSize(&nWidth, &nHeight);
    SetLabelText(_T("SDL_GetWindowSize"), FormatSize(nWidth, nHeight));

    nWidth = 0;
    nHeight = 0;
    NativeWnd()->GetWindowSizeInPixels(&nWidth, &nHeight);
    SetLabelText(_T("SDL_GetWindowSizeInPixels"), FormatSize(nWidth, nHeight));

    SetLabelText(_T("SDL_GetDisplayContentScale"),
                 ui::StringUtil::Printf(_T("%.02f"), NativeWnd()->GetDisplayContentScale()));
    SetLabelText(_T("SDL_GetWindowDisplayScale"),
                 ui::StringUtil::Printf(_T("%.02f"), NativeWnd()->GetWindowDisplayScale()));
    SetLabelText(_T("SDL_GetWindowPixelDensity"),
                 ui::StringUtil::Printf(_T("%.02f"), NativeWnd()->GetWindowPixelDensity()));
#else
    ui::Control* pSDL = FindControl(_T("SDL"));
    if (pSDL != nullptr) {
        pSDL->SetVisible(false);
    }
#endif
}
