//MainForm.cpp
#include "MainForm.h"

namespace {

/** 把颜色格式化为 #AARRGGBB
*/
DString FormatColor(const ui::UiColor& color)
{
    return ui::StringUtil::Printf(_T("#%08X"), (uint32_t)color.GetARGB());
}

/** 把颜色格式化为 R/G/B/A 分量文本
*/
DString FormatComponents(const ui::UiColor& color)
{
    return ui::StringUtil::Printf(_T("R %d   G %d   B %d   A %d"),
        (int32_t)color.GetR(), (int32_t)color.GetG(),
        (int32_t)color.GetB(), (int32_t)color.GetA());
}

}

MainForm::MainForm():
    m_pColorBlock(nullptr),
    m_pColorText(nullptr),
    m_pColorRgba(nullptr),
    m_pLogView(nullptr),
    m_nLogCount(0)
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("color_picker_demo");
}

DString MainForm::GetSkinFile()
{
    return _T("ColorPickerDemo.xml");
}

void MainForm::OnInitWindow()
{
    m_pColorBlock = FindControl(_T("color_block"));
    m_pColorText = dynamic_cast<ui::Label*>(FindControl(_T("color_text")));
    m_pColorRgba = dynamic_cast<ui::Label*>(FindControl(_T("color_rgba")));
    m_pLogView = dynamic_cast<ui::RichEdit*>(FindControl(_T("log_view")));

    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("open_picker")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            OpenColorPicker();
            return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("clear_log")));
    if (pButton != nullptr) {
        pButton->AttachClick([this](const ui::EventArgs& /*args*/) {
            ClearLog();
            return true;
            });
    }

    //与调色板的初始颜色保持一致：深色主题用黑色，浅色主题用白色
    ui::UiColor initColor(ui::UiColors::White);
    if (ui::GlobalManager::Instance().Theme().GetCurrentThemeStyle() == ui::ThemeStyle::kDark) {
        initColor = ui::UiColor(ui::UiColors::Black);
    }
    SetColor(initColor);
    PrintLog(ui::StringUtil::Printf(_T("%s %s"),
            ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_COLORPICKER_DEMO_CURRENT")).c_str(),
            FormatColor(initColor).c_str()));
}

bool MainForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();

    //日志行是代码拼接出来的文本（含语言文案），XML 的 text_id 不会刷新这部分；
    //直接清空并重新输出当前颜色，避免中英文混在一份日志里
    ClearLog();
    PrintLog(ui::StringUtil::Printf(_T("%s %s"),
            ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_COLORPICKER_DEMO_CURRENT")).c_str(),
            FormatColor(m_color).c_str()));
    return bRet;
}

void MainForm::OpenColorPicker()
{
    if (m_pColorPicker != nullptr) {
        //已经打开过一个，激活它即可
        m_pColorPicker->ShowWindow(ui::kSW_SHOW_NORMAL);
        return;
    }

    ui::ColorPicker* pColorPicker = new ui::ColorPicker;
    m_pColorPicker = pColorPicker;

    //记录打开前的颜色：点「取消」或按 Esc 时用它恢复
    m_colorBeforePick = m_color;

    //以当前显示的颜色作为调色板的初始颜色
    pColorPicker->SetSelectedColor(m_color);

    //开启"不遮挡父窗口"开关：调色板会自动贴到主窗口旁边显示，而不是居中盖住主窗口。
    //该开关默认关闭，仅在本示例程序中显式开启，避免影响 ColorPicker 的其他调用方。
    pColorPicker->SetAdjustWindowPos(true);

    //选色过程中实时回调：拖动滑动条或点色块时会连续触发，用于即时预览
    ui::ControlPtrT<MainForm> pThis(this);
    pColorPicker->AttachSelectColor([pThis](const ui::EventArgs& args) {
        if (pThis == nullptr) {
            return true;
        }
        ui::UiColor newColor((uint32_t)args.wParam);
        pThis->SetColor(newColor);
        pThis->PrintLog(ui::StringUtil::Printf(_T("%s %s"),
                ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_COLORPICKER_DEMO_SELECT")).c_str(),
                FormatColor(newColor).c_str()));
        return true;
        });

    //窗口关闭回调：只有点「确定」才采用最终颜色，其它情况恢复为原来的颜色
    pColorPicker->AttachWindowClose([pThis](const ui::EventArgs& args) {
        if (pThis == nullptr) {
            return true;
        }
        const DString resultText = pThis->GetCloseResultText((uint32_t)args.wParam);
        pThis->PrintLog(ui::StringUtil::Printf(_T("%s %s"),
                ui::GlobalManager::Instance().Lang().GetStringByID(_T("STRID_COLORPICKER_DEMO_CLOSED")).c_str(),
                resultText.c_str()));
        if ((args.wParam == ui::kWindowCloseOK) && (pThis->m_pColorPicker != nullptr)) {
            pThis->SetColor(pThis->m_pColorPicker->GetSelectedColor());
        }
        else {
            //取消或直接关闭：放弃选色过程中的实时预览效果，恢复为打开前的颜色
            pThis->SetColor(pThis->m_colorBeforePick);
        }
        pThis->m_pColorPicker = nullptr;
        return true;
        });

    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("ColorPicker");
    //不启用居中：居中会完全盖住主窗口；调色板在 OnInitWindow 中会自动贴到主窗口旁边显示
    createParam.m_bCenterWindow = false;
    pColorPicker->CreateWnd(this, createParam);
    pColorPicker->ShowModalFake();
}

void MainForm::SetColor(const ui::UiColor& color)
{
    m_color = color;
    if (m_pColorBlock != nullptr) {
        m_pColorBlock->SetBkColor(color);
    }
    if (m_pColorText != nullptr) {
        m_pColorText->SetText(FormatColor(color));
    }
    if (m_pColorRgba != nullptr) {
        m_pColorRgba->SetText(FormatComponents(color));
    }
}

void MainForm::PrintLog(const DString& text)
{
    if (m_pLogView == nullptr) {
        return;
    }
    if (m_nLogCount > 500) {
        ClearLog();
    }
    DString line = ui::StringUtil::Printf(_T("%04d  "), ++m_nLogCount);
    line += text;
    line += _T("\n");
    m_pLogView->AppendText(line);
}

void MainForm::ClearLog()
{
    m_nLogCount = 0;
    if (m_pLogView != nullptr) {
        m_pLogView->SetText(_T(""));
    }
}

DString MainForm::GetCloseResultText(uint32_t wParam)
{
    DString strId;
    switch (wParam) {
    case ui::kWindowCloseOK:
        strId = _T("STRID_COLORPICKER_DEMO_RESULT_OK");
        break;
    case ui::kWindowCloseCancel:
        strId = _T("STRID_COLORPICKER_DEMO_RESULT_CANCEL");
        break;
    default:
        strId = _T("STRID_COLORPICKER_DEMO_RESULT_CLOSE");
        break;
    }
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}
