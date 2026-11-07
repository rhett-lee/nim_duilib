#include "MainForm.h"
#include <algorithm>
#include <cmath>
#include <map>

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#include <commdlg.h>
#endif

#ifndef LY_PER_INCH
    #define LY_PER_INCH 1440
#endif

void MainForm::UpdateRowSpacingEnabled()
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    //Windows平台：行距设置依赖RichEdit的富文本模式
    const bool bEnabled = (m_pRichEdit != nullptr) && m_pRichEdit->IsRichText();
#else
    //非Windows平台（含Windows的SDL版本）：RichEdit2支持行距设置，恒可用
    const bool bEnabled = (m_pRichEdit != nullptr);
#endif
    for (ui::Control* pControl : m_rowSpacingControls) {
        if (pControl != nullptr) {
            pControl->SetEnabled(bEnabled);
        }
    }
}

void MainForm::InitFontSizeCombo()
{
    ui::Combo* pFontSizeCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_size")));
    if (pFontSizeCombo == nullptr) {
        return;
    }
    pFontSizeCombo->DeleteAllItems();
    m_fontSizeList.clear();
    ui::GlobalManager::Instance().Font().GetDpiFontSizeList(Dpi(), m_fontSizeList);
    for (size_t nIndex = 0; nIndex < m_fontSizeList.size(); ++nIndex) {
        const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
        size_t nItemIndex = pFontSizeCombo->AddTextItem(fontSize.fontSizeName);
        if (ui::Box::IsValidItemIndex(nItemIndex)) {
            pFontSizeCombo->SetItemData(nItemIndex, nIndex);
        }
    }
}

void MainForm::InitColorCombo()
{
    ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(_T("color_combo_button")));
    if (pColorComboBtn == nullptr) {
        return;
    }
    ui::UiSize boxSize = pColorComboBtn->GetDropBoxSize();
    ui::Box* pComboBox = pColorComboBtn->GetComboBox();
    if (pComboBox == nullptr) {
        return;
    }
    pComboBox->SetWindow(this);
    ui::GlobalManager::Instance().FillBoxWithCache(pComboBox, ui::FilePath(_T("rich_edit/color_combox.xml")));
    ui::ColorPickerRegular* pColorPicker = dynamic_cast<ui::ColorPickerRegular*>(pComboBox->FindSubControl(_T("color_combo_picker")));
    if (pColorPicker != nullptr) {
        //响应选择颜色事件
        pColorPicker->AttachSelectColor([this, pColorComboBtn](const ui::EventArgs& args) {
            ui::UiColor newColor((uint32_t)args.wParam);
            //设置选择后的颜色
            ui::Label* pLeftColorLabel = pColorComboBtn->GetLabelBottom();
            if (pLeftColorLabel != nullptr) {
                pLeftColorLabel->SetBkColor(newColor);
                SetTextColor(pLeftColorLabel->GetBkColor());
            }
            return true;
            });
    }

    ui::Button* pMoreColorButton = dynamic_cast<ui::Button*>(pComboBox->FindSubControl(_T("color_combo_picker_more")));
    if (pMoreColorButton != nullptr) {
        pMoreColorButton->AttachClick([this](const ui::EventArgs& args) {
            ShowColorPicker();
            return true;
            });
    }
}

void MainForm::ShowColorPicker()
{
    ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(_T("color_combo_button")));
    if (pColorComboBtn == nullptr) {
        return;
    }
    ui::Label* pLeftColorLabel = pColorComboBtn->GetLabelBottom();
    if (pLeftColorLabel == nullptr) {
        return;
    }
    DString oldTextColor = pLeftColorLabel->GetBkColor();

    ui::ColorPicker* pColorPicker = new ui::ColorPicker;
    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("ColorPicker");
    createParam.m_bCenterWindow = true;
#ifdef DUILIB_BUILD_FOR_WIN
    pColorPicker->CreateWnd(nullptr, createParam);
    pColorPicker->ShowModalFake();
#else
    pColorPicker->CreateWnd(this, createParam);
    pColorPicker->ShowModalFake();
    pColorPicker->SetWindowForeground();
#endif

    ui::RichEdit* pEdit = m_pRichEdit;
    if (pEdit != nullptr) {
        if (!oldTextColor.empty()) {
            pColorPicker->SetSelectedColor(pEdit->GetUiColor(oldTextColor));
        }
        //如果在界面选择颜色，则临时更新RichEdit控件文本的颜色
        pColorPicker->AttachSelectColor([this, pEdit, pLeftColorLabel](const ui::EventArgs& args) {
            ui::UiColor newColor = ui::UiColor((uint32_t)args.wParam);
            pLeftColorLabel->SetBkColor(newColor);
            SetTextColor(pEdit->GetColorString(newColor));
            return true;
            });

        //窗口关闭事件
        pColorPicker->AttachWindowClose([this, pColorPicker, pEdit, oldTextColor, pLeftColorLabel](const ui::EventArgs& args) {
            ui::UiColor newColor = pColorPicker->GetSelectedColor();
            if ((args.wParam == ui::kWindowCloseOK) && !newColor.IsEmpty()) {
                //如果是"确认"，则设置RichEdit控件的文本颜色
                pLeftColorLabel->SetBkColor(newColor);
                SetTextColor(pEdit->GetColorString(newColor));
            }
            else {
                //如果是"取消"或者关闭窗口，则恢复原来的颜色
                pLeftColorLabel->SetBkColor(oldTextColor);
                SetTextColor(oldTextColor);
            }
            return true;
            });
    }
}

void MainForm::UpdateFontStatus()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    const ui::UiFont fontInfo = pRichEdit->GetFontInfo();

    //更新字体名称
    ui::Combo* pFontNameCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_name")));
    if (pFontNameCombo != nullptr) {
        size_t nItemIndex = pFontNameCombo->SelectTextItem(fontInfo.m_fontName.c_str(), false);
        if ((nItemIndex == ui::Box::InvalidIndex) && !fontInfo.m_fontName.empty()) {
            //字体列表中不存在时，将该字体名称添加到列表中并选中，避免下拉框中显示空白
            nItemIndex = pFontNameCombo->AddTextItem(fontInfo.m_fontName.c_str());
            if (ui::Box::IsValidItemIndex(nItemIndex)) {
                pFontNameCombo->SetCurSel(nItemIndex);
            }
        }
    }

    //更新字体大小
    UpdateFontSizeStatus();

    //更新是否粗体
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_bold")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(fontInfo.m_bBold);
    }

    //更新是否斜体
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_italic")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(fontInfo.m_bItalic);
    }

    //更新是否下划线
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_underline")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(fontInfo.m_bUnderline);
    }

    //更新是否删除线
    pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_font_strikeout")));
    if (pCheckBox != nullptr) {
        pCheckBox->SetSelected(fontInfo.m_bStrikeOut);
    }
}

void MainForm::UpdateFontSizeStatus()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    ui::Combo* pFontSizeCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_size")));
    if (pFontSizeCombo == nullptr) {
        return;
    }
    const ui::UiFont fontInfo = pRichEdit->GetFontInfo();
    if (fontInfo.m_fontSize == 0) {
        //不含有效字体大小信息
        pFontSizeCombo->SetCurSel(ui::Box::InvalidIndex);
        return;
    }
   
    size_t maxItemIndex = 0;
    for (size_t nIndex = 0; nIndex < m_fontSizeList.size(); ++nIndex) {
        if (nIndex == (m_fontSizeList.size() - 1)) {
            break;
        }
        if (m_fontSizeList[nIndex].fFontSize > m_fontSizeList[nIndex + 1].fFontSize) {
            maxItemIndex = nIndex;
            break;
        }
    }

    bool bSelected = false;
    for (size_t nIndex = maxItemIndex; nIndex < m_fontSizeList.size(); ++nIndex) {
        //优先选择汉字的字号
        const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
        if (fontInfo.m_fontSize == (int32_t)std::roundf(fontSize.fDpiFontSize)) {
            if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName, false) != ui::Box::InvalidIndex) {
                bSelected = true;
            }
            break;
        }
    }
    if (!bSelected) {
        for (size_t nIndex = 0; nIndex <= maxItemIndex; ++nIndex) {
            //选择数字的字号
            const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
            if ((int32_t)std::roundf(fontSize.fDpiFontSize) >= fontInfo.m_fontSize) {
                if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName, false) != ui::Box::InvalidIndex) {
                    bSelected = true;
                    break;
                }
            }
        }
    }
    if (!bSelected) {
        for (size_t nIndex = 0; nIndex <= maxItemIndex; ++nIndex) {
            //选择数字的字号
            const ui::FontSizeInfo& fontSize = m_fontSizeList[nIndex];
            int32_t nFontSize = Dpi().GetScaleInt((int32_t)std::roundf(fontSize.fFontSize));
            if (nFontSize >= fontInfo.m_fontSize) {
                if (pFontSizeCombo->SelectTextItem(fontSize.fontSizeName, false) != ui::Box::InvalidIndex) {
                    bSelected = true;
                    break;
                }
            }
        }
    }
}

bool MainForm::IsApplyToAll()
{
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_apply_to_all")));
    return (pCheckBox != nullptr) && pCheckBox->IsSelected();
}

bool MainForm::SetRichEditFontInfo(const ui::UiFont& fontInfo)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    if (!IsApplyToAll()) {
        return m_pRichEdit->SetFontInfo(fontInfo);
    }
    //应用到全文：临时全选，应用后再恢复原来的选区
    int32_t nStartChar = 0;
    int32_t nEndChar = 0;
    m_pRichEdit->GetSel(nStartChar, nEndChar);
    m_pRichEdit->SetSelAll();
    bool bRet = m_pRichEdit->SetFontInfo(fontInfo);
    m_pRichEdit->SetSel(nStartChar, nEndChar);
    return bRet;
}

void MainForm::SetFontName(const DString& fontName)
{
    m_lastFontName = fontName;
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_fontName = fontName;
        SetRichEditFontInfo(fontInfo);
    }
}

void MainForm::SetFontSize(const DString& fontSize)
{
    for (const ui::FontSizeInfo& fontSizeInfo : m_fontSizeList) {
        if (fontSize == fontSizeInfo.fontSizeName) {
            if (m_pRichEdit != nullptr) {
                ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
                fontInfo.m_fontSize = (int32_t)(std::roundf(fontSizeInfo.fDpiFontSize));
                SetRichEditFontInfo(fontInfo);
            }
            break;
        }
    }
}

void MainForm::AdjustFontSize(bool bIncreaseFontSize)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    std::map<int32_t, int32_t> fontSizeMap;
    for (const ui::FontSizeInfo& fontSizeInfo : m_fontSizeList) {
        fontSizeMap[(int32_t)(std::roundf(fontSizeInfo.fDpiFontSize))] = (int32_t)(std::roundf(fontSizeInfo.fFontSize));
    }
    std::vector<int32_t> fontSizeList;
    for (auto fontSize : fontSizeMap) {
        fontSizeList.push_back(fontSize.second);
    }
    auto pos = std::unique(fontSizeList.begin(), fontSizeList.end());
    if (pos != fontSizeList.end()) {
        fontSizeList.erase(pos);
    }

    ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
    //转换回原值
    auto iter = fontSizeMap.find(fontInfo.m_fontSize);
    if (iter != fontSizeMap.end()) {
        fontInfo.m_fontSize = iter->second;
    }
    else {
        bool bFound = false;
        if (bIncreaseFontSize) {
            for (iter = fontSizeMap.begin(); iter != fontSizeMap.end(); ++iter) {
                if (iter->first > fontInfo.m_fontSize) {
                    fontInfo.m_fontSize = iter->second;
                    bFound = true;
                    break;
                }
            }
        }
        else {
            auto pos = fontSizeMap.rbegin();
            for (; pos != fontSizeMap.rend(); ++pos) {
                if (pos->first < fontInfo.m_fontSize) {
                    fontInfo.m_fontSize = pos->second;
                    bFound = true;
                    break;
                }
            }
        }
        if (!bFound) {
            Dpi().UnscaleInt(fontInfo.m_fontSize);
        }
    }

    const size_t fontCount = fontSizeList.size();
    for (size_t index = 0; index < fontCount; ++index) {
        if (fontInfo.m_fontSize == fontSizeList[index]) {
            //匹配到当前字体大小
            if (bIncreaseFontSize) {
                //增加字体
                if (index < (fontCount - 1)) {                  
                    fontInfo.m_fontSize = Dpi().GetScaleInt(fontSizeList[index + 1]);
                    if (SetRichEditFontInfo(fontInfo)) {
                        UpdateFontSizeStatus();
                    }
                }
            }
            else {
                //减小字体
                if (index > 0) {
                    fontInfo.m_fontSize = Dpi().GetScaleInt(fontSizeList[index - 1]);
                    if (SetRichEditFontInfo(fontInfo)) {
                        UpdateFontSizeStatus();
                    }
                }
            }
            break;
        }
    }
}

void MainForm::SetFontBold(bool bBold)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bBold = bBold;
        SetRichEditFontInfo(fontInfo);
    }
}

void MainForm::SetFontItalic(bool bItalic)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bItalic = bItalic;
        SetRichEditFontInfo(fontInfo);
    }
}

void MainForm::SetFontUnderline(bool bUnderline)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bUnderline = bUnderline;
        SetRichEditFontInfo(fontInfo);
    }
}

void MainForm::SetFontStrikeOut(bool bStrikeOut)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bStrikeOut = bStrikeOut;
        SetRichEditFontInfo(fontInfo);
    }
}

void MainForm::SetTextColor(const DString& newColor)
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    if (!m_pRichEdit->IsRichText()) {
        //纯文本模式下，文字颜色本身作用于整个控件
        m_pRichEdit->SetTextColor(newColor);
        return;
    }
    if (!IsApplyToAll()) {
        m_pRichEdit->SetSelectionTextColor(newColor);
        return;
    }
    //应用到全文：临时全选，应用后再恢复原来的选区
    int32_t nStartChar = 0;
    int32_t nEndChar = 0;
    m_pRichEdit->GetSel(nStartChar, nEndChar);
    m_pRichEdit->SetSelAll();
    m_pRichEdit->SetSelectionTextColor(newColor);
    m_pRichEdit->SetSel(nStartChar, nEndChar);
}


#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

int32_t MainForm::ConvertToFontHeight(int32_t fontSize) const
{
    if (m_pRichEdit != nullptr) {
        return m_pRichEdit->ConvertToFontHeight(fontSize);
    }
    return fontSize;
}

bool MainForm::GetRichEditLogFont(LOGFONTW& lf) const
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return false;
    }

    CHARFORMAT2W cf = {};
    GetCharFormat(cf);

    if (cf.dwMask & CFM_SIZE) {
        HWND hWnd = NativeWnd()->GetHWND();
        HDC hDC = ::GetDC(hWnd);
        lf.lfHeight = -MulDiv(cf.yHeight, ::GetDeviceCaps(hDC, LOGPIXELSY), LY_PER_INCH);
        lf.lfWidth = 0;
        ::ReleaseDC(hWnd, hDC);
    }

    if (cf.dwMask & CFM_BOLD) {
        lf.lfWeight = (cf.dwEffects & CFE_BOLD) ? FW_BOLD : 0;
    }

    if (cf.dwMask & CFM_WEIGHT) {
        lf.lfWeight = cf.wWeight;
    }

    if (cf.dwMask & CFM_ITALIC) {
        lf.lfItalic = (cf.dwEffects & CFE_ITALIC) ? TRUE : FALSE;
    }

    if (cf.dwMask & CFM_UNDERLINE) {
        lf.lfUnderline = (cf.dwEffects & CFE_UNDERLINE) ? TRUE : FALSE;
    }

    if (cf.dwMask & CFM_STRIKEOUT) {
        lf.lfStrikeOut = (cf.dwEffects & CFE_STRIKEOUT) ? TRUE : FALSE;
    }

    if (cf.dwMask & CFM_CHARSET) {
        lf.lfCharSet = cf.bCharSet;
    }

    if (cf.dwMask & CFM_FACE) {
        lf.lfPitchAndFamily = cf.bPitchAndFamily;

        //替换为系统字体名称
        DStringW fontName = cf.szFaceName;
        ui::StringUtil::StringCopy(lf.lfFaceName, fontName.c_str());
    }
    return true;
}

void MainForm::InitCharFormat(const LOGFONTW& lf, CHARFORMAT2W& charFormat) const
{
    //字体字号需要转换, 否则字体大小显示异常
    LONG lfHeight = ConvertToFontHeight(lf.lfHeight);

    charFormat.cbSize = sizeof(CHARFORMAT2W);
    charFormat.dwMask = CFM_SIZE | CFM_OFFSET | CFM_FACE | CFM_CHARSET | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
    charFormat.yHeight = -lfHeight;
    if (lf.lfWeight >= FW_BOLD) {
        charFormat.dwEffects |= CFE_BOLD;
    }
    else {
        charFormat.dwEffects &= ~CFE_BOLD;
    }
    if (lf.lfItalic) {
        charFormat.dwEffects |= CFE_ITALIC;
    }
    else {
        charFormat.dwEffects &= ~CFE_ITALIC;
    }
    if (lf.lfUnderline) {
        charFormat.dwEffects |= CFE_UNDERLINE;
    }
    else {
        charFormat.dwEffects &= ~CFE_UNDERLINE;
    }
    if (lf.lfStrikeOut) {
        charFormat.dwEffects |= CFE_STRIKEOUT;
    }
    else {
        charFormat.dwEffects &= ~CFE_STRIKEOUT;
    }
    charFormat.bCharSet = lf.lfCharSet;
    charFormat.bPitchAndFamily = lf.lfPitchAndFamily;
    ui::StringUtil::StringCopy(charFormat.szFaceName, lf.lfFaceName);
}

void MainForm::OnSetFont()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    //文本颜色
    ui::UiColor textColor;
    if (pRichEdit->IsRichText()) {
        textColor = pRichEdit->GetUiColor(pRichEdit->GetSelectionTextColor());
    }
    else {
        textColor = pRichEdit->GetUiColor(pRichEdit->GetTextColor());
    }

    LOGFONTW logFont = {};
    GetRichEditLogFont(logFont);

    CHOOSEFONTW cf;
    WCHAR szStyleName[64];  // contains style name after return
    LOGFONTW lf;                // default LOGFONTW to store the info

    HWND hWndParent = NativeWnd()->GetHWND();
    LPLOGFONTW lplfInitial = &logFont;
    DWORD dwFlags = CF_EFFECTS | CF_SCREENFONTS;
    memset(&cf, 0, sizeof(cf));
    memset(&lf, 0, sizeof(lf));
    memset(&szStyleName, 0, sizeof(szStyleName));

    cf.lStructSize = sizeof(cf);
    cf.hwndOwner = hWndParent;
    cf.rgbColors = textColor.ToCOLORREF(IsColorThemeDarkMode());
    cf.lpszStyle = (LPWSTR)&szStyleName;
    cf.Flags = dwFlags;

    if (lplfInitial != nullptr) {
        cf.lpLogFont = lplfInitial;
        cf.Flags |= CF_INITTOLOGFONTSTRUCT;
        lf = *lplfInitial;
    }
    else {
        cf.lpLogFont = &lf;
    }

    BOOL bRet = ::ChooseFontW(&cf);
    if (bRet) {
        memcpy_s(&lf, sizeof(lf), cf.lpLogFont, sizeof(lf));

        //设置RichEdit字体
        CHARFORMAT2W charFormat = {};
        GetCharFormat(charFormat);
        InitCharFormat(lf, charFormat);

        //设置字体颜色
        charFormat.dwMask |= CFM_COLOR;
        charFormat.crTextColor = cf.rgbColors;
        charFormat.dwEffects &= ~CFE_AUTOCOLOR;

        SetCharFormat(charFormat);

        //记录最后一次设置的字体名称（与字体下拉框的选择保持一致），保存设置时使用
        m_lastFontName = lf.lfFaceName;

        //更新颜色
        ui::ComboButton* pColorComboBtn = dynamic_cast<ui::ComboButton*>(FindControl(_T("color_combo_button")));
        if (pColorComboBtn != nullptr) {
            if (pColorComboBtn->GetLabelBottom() != nullptr) {
                ui::UiColor textColor;
                textColor.SetFromCOLORREF(charFormat.crTextColor);
                pColorComboBtn->GetLabelBottom()->SetBkColor(textColor);
            }
        }
    }

    //更新字体按钮的状态
    UpdateFontStatus();
}

void MainForm::GetCharFormat(CHARFORMAT2W& charFormat) const
{
    charFormat = {};
    charFormat.cbSize = sizeof(CHARFORMAT2W);
    ui::RichEdit* pRichEdit = GetRichEdit();
    ASSERT(pRichEdit != nullptr);
    if (pRichEdit != nullptr) {
        if (pRichEdit->IsRichText()) {
            pRichEdit->GetSelectionCharFormat(charFormat);
        }
        else {
            pRichEdit->GetDefaultCharFormat(charFormat);
        }
    }
}

void MainForm::SetCharFormat(CHARFORMAT2W& charFormat)
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    ASSERT(pRichEdit != nullptr);
    if (pRichEdit == nullptr) {
        return;
    }
    if (!pRichEdit->IsRichText()) {
        //纯文本模式：字体格式本身作用于整个控件
        pRichEdit->SetDefaultCharFormat(charFormat);
        return;
    }
    if (!IsApplyToAll()) {
        pRichEdit->SetSelectionCharFormat(charFormat);
        return;
    }
    //应用到全文：临时全选，应用后再恢复原来的选区
    int32_t nStartChar = 0;
    int32_t nEndChar = 0;
    pRichEdit->GetSel(nStartChar, nEndChar);
    pRichEdit->SetSelAll();
    pRichEdit->SetSelectionCharFormat(charFormat);
    pRichEdit->SetSel(nStartChar, nEndChar);
}

#endif
