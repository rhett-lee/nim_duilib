#include "MainForm.h"
#include "FindForm.h"
#include "ReplaceForm.h"
#include "MainThread.h"
#include "duilib/Utils/MessageBoxWnd.h"
#include <fstream>
#include <list>
#include <map>

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#include <ShellApi.h>
#include <commdlg.h>
#endif

#ifndef LY_PER_INCH
    #define LY_PER_INCH 1440
#endif

/** 用户设置文件中的键值对
*/
typedef std::map<std::string, std::string> SettingsMap;

/** 解析用户设置文本中的一行：键=值（以'#'或者';'开头的行为注释行）
*/
void ParseSettingsText(const std::string& text, SettingsMap& settings)
{
    settings.clear();
    std::list<std::string> lines = ui::StringUtil::Split(text, "\n");
    for (std::string& line : lines) {
        ui::StringUtil::Trim(line);
        if (line.empty() || (line[0] == '#') || (line[0] == ';')) {
            continue;
        }
        const size_t nPos = line.find('=');
        if (nPos == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, nPos);
        std::string value = line.substr(nPos + 1);
        ui::StringUtil::Trim(key);
        ui::StringUtil::Trim(value);
        if (!key.empty()) {
            settings[key] = value;
        }
    }
}

/** 读取用户设置文件（文本格式：每行一个"键=值"，UTF-8编码）
*/
void ReadSettingsFile(const ui::FilePath& filePath, SettingsMap& settings)
{
    settings.clear();
    std::vector<uint8_t> fileData;
    if (!ui::FileUtil::ReadFileData(filePath, fileData) || fileData.empty()) {
        return;
    }
    std::string text(fileData.begin(), fileData.end());
    //去除UTF-8 BOM
    if ((text.size() >= 3) &&
        (static_cast<uint8_t>(text[0]) == 0xEF) &&
        (static_cast<uint8_t>(text[1]) == 0xBB) &&
        (static_cast<uint8_t>(text[2]) == 0xBF)) {
        text.erase(0, 3);
    }
    ParseSettingsText(text, settings);
}

/** 从用户设置中读取一个整数
*/
int32_t GetSettingInt(const SettingsMap& settings, const std::string& key, int32_t nDefault)
{
    auto iter = settings.find(key);
    if (iter == settings.end()) {
        return nDefault;
    }
    return ui::StringUtil::StringToInt32(iter->second.c_str());
}

/** 从用户设置中读取一个字符串
*/
std::string GetSettingString(const SettingsMap& settings, const std::string& key, const std::string& defaultText)
{
    auto iter = settings.find(key);
    if (iter == settings.end()) {
        return defaultText;
    }
    return iter->second;
}

/** 把整数写入用户设置文本
*/
void AppendSettingInt(std::string& settingsText, const std::string& key, int32_t nValue)
{
    settingsText += key;
    settingsText += '=';
    settingsText += std::to_string(nValue);
    settingsText += '\n';
}

/** 把字符串写入用户设置文本（需要先转换为UTF-8）
*/
void AppendSettingString(std::string& settingsText, const std::string& key, const std::string& value)
{
    settingsText += key;
    settingsText += '=';
    settingsText += value;
    settingsText += '\n';
}

MainForm::MainForm():
    m_pRichEdit(nullptr),
    m_bSampleDocument(true),
    m_bLoadingDocument(false),
    m_bUpdatingRichTextCheck(false),
    m_bZoomComboSelecting(false),
    m_encodingText(_T("UTF-8")),
    m_pFindForm(nullptr),
    m_pReplaceForm(nullptr)
{
}

MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("rich_edit");
}

DString MainForm::GetSkinFile()
{
    return _T("rich_edit.xml");
}

void MainForm::OnInitWindow()
{
    m_pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("rich_edit")));
    ASSERT(m_pRichEdit != nullptr);
    m_findReplace.SetRichEdit(m_pRichEdit);

    ui::Button* pSaveButton = dynamic_cast<ui::Button*>(FindControl(_T("save_file")));
    if (pSaveButton != nullptr) {
        m_saveBtnText = pSaveButton->GetText();
    }

    SetupTestControls();
    SetupFileTools();
    SetupEditTools();
    SetupFindTools();
    SetupFontTools();
    SetupViewTools();
    SetupTextEvents();
    SetupDropFile();

    LoadSettings();
    LoadRichEditData();

    UpdateSaveStatus();
    UpdateStatusBar();
    UpdateFontStatus();
    UpdateEditButtonStatus();
    UpdateRowSpacingEnabled();
    UpdateZoomValue();
}


void MainForm::BindClick(const DString& controlName, const std::function<void()>& onClick)
{
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(controlName));
    if (pButton != nullptr) {
        pButton->AttachClick([pButton, onClick](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                onClick();
            }
            return true;
            });
    }
}

void MainForm::BindCheck(const DString& controlName, const std::function<void(bool)>& onChanged)
{
    ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(controlName));
    if (pCheckBox != nullptr) {
        //CheckBox只触发select/unselect事件，不触发kEventClick事件
        auto handler = [pCheckBox, onChanged](const ui::EventArgs& args) {
            if (args.GetSender() == pCheckBox) {
                onChanged(pCheckBox->IsSelected());
            }
            return true;
            };
        pCheckBox->AttachSelect(handler);
        pCheckBox->AttachUnSelect(handler);
    }
}

void MainForm::SetupTestControls()
{
    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("test_url")));
    if (pRichEdit != nullptr) {
        pRichEdit->AttachLinkClick([this, pRichEdit](const ui::EventArgs& args) {
            //点击了超级链接
            if (args.GetSender() == pRichEdit) {
                const DString::value_type* pUrl = (const DString::value_type*)args.wParam;
                if (pUrl != nullptr) {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
                    ::ShellExecuteW(NativeWnd()->GetHWND(), L"open", ui::StringConvert::TToWString(pUrl).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
                }
            }
            return true;
            });
    }
}

void MainForm::SetupFileTools()
{
    BindClick(_T("new_file"), [this]() { OnNewFile(); });
    BindClick(_T("open_file"), [this]() { OnOpenFile(); });
    BindClick(_T("save_file"), [this]() { OnSaveFile(); });
    BindClick(_T("save_as_file"), [this]() { OnSaveAsFile(); });
}

void MainForm::SetupEditTools()
{
    BindClick(_T("btn_copy"), [this]() {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->Copy();
            UpdateSaveStatus();
            UpdateEditButtonStatus();
        }
        });
    BindClick(_T("btn_cut"), [this]() {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->Cut();
            UpdateSaveStatus();
            UpdateEditButtonStatus();
        }
        });
    BindClick(_T("btn_paste"), [this]() {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->Paste();
            UpdateSaveStatus();
            UpdateEditButtonStatus();
        }
        });
    BindClick(_T("btn_delete"), [this]() {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->ReplaceSel(DString(), false);
            UpdateSaveStatus();
            UpdateEditButtonStatus();
        }
        });
    BindClick(_T("btn_sel_all"), [this]() {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetSelAll();
            UpdateSaveStatus();
            UpdateEditButtonStatus();
        }
        });
    BindClick(_T("btn_sel_none"), [this]() {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetSelNone();
            UpdateSaveStatus();
            UpdateEditButtonStatus();
        }
        });
    BindClick(_T("btn_undo"), [this]() {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->Undo();
            UpdateSaveStatus();
            UpdateEditButtonStatus();
        }
        });
    BindClick(_T("btn_redo"), [this]() {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->Redo();
            UpdateSaveStatus();
            UpdateEditButtonStatus();
        }
        });
}

void MainForm::SetupFindTools()
{
    BindClick(_T("btn_find_text"), [this]() { OnFindText(); });
    BindClick(_T("btn_find_next"), [this]() { OnFindNext(); });
    BindClick(_T("btn_replace_text"), [this]() { OnReplaceText(); });
}

void MainForm::SetupFontTools()
{
    //设置字体
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("set_font")));
    if (pButton != nullptr) {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                OnSetFont();
            }
            return true;
            });
#else
        pButton->SetEnabled(false);
#endif
    }

    //初始化字体名称列表
    ui::Combo* pFontNameCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_name")));
    if (pFontNameCombo != nullptr) {
        m_fontList.clear();
        ui::GlobalManager::Instance().Font().GetFontNameList(m_fontList);
        for (size_t nIndex = 0; nIndex < m_fontList.size(); ++nIndex) {
            const DString& fontName = m_fontList[nIndex];
            size_t nItemIndex = pFontNameCombo->AddTextItem(fontName);
            if (ui::Box::IsValidItemIndex(nItemIndex)) {
                pFontNameCombo->SetItemData(nItemIndex, nIndex);
            }
        }
        pFontNameCombo->AttachSelect([this, pFontNameCombo](const ui::EventArgs& /*args*/) {
            DString fontName = pFontNameCombo->GetText();
            SetFontName(fontName);
            return true;
            });
        pFontNameCombo->AttachWindowClose([this, pFontNameCombo](const ui::EventArgs& /*args*/) {
            DString fontName = pFontNameCombo->GetText();
            SetFontName(fontName);
            return true;
            });
    }

    //初始化字体大小列表
    ui::Combo* pFontSizeCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_font_size")));
    if (pFontSizeCombo != nullptr) {
        pFontSizeCombo->AttachSelect([this, pFontSizeCombo](const ui::EventArgs& /*args*/) {
            DString fontSize = pFontSizeCombo->GetText();
            SetFontSize(fontSize);
            return true;
            });
        pFontSizeCombo->AttachWindowClose([this, pFontSizeCombo](const ui::EventArgs& /*args*/) {
            DString fontSize = pFontSizeCombo->GetText();
            SetFontSize(fontSize);
            return true;
            });
    }
    InitFontSizeCombo();

    //粗体、斜体、下划线、删除线
    BindCheck(_T("btn_font_bold"), [this](bool bSelected) { SetFontBold(bSelected); });
    BindCheck(_T("btn_font_italic"), [this](bool bSelected) { SetFontItalic(bSelected); });
    BindCheck(_T("btn_font_underline"), [this](bool bSelected) { SetFontUnderline(bSelected); });
    BindCheck(_T("btn_font_strikeout"), [this](bool bSelected) { SetFontStrikeOut(bSelected); });

    BindClick(_T("btn_font_size_increase"), [this]() { AdjustFontSize(true); });
    BindClick(_T("btn_font_size_decrease"), [this]() { AdjustFontSize(false); });

    //初始化颜色选择
    InitColorCombo();
}

void MainForm::SetupViewTools()
{
    InitZoomCombo();
    UpdateZoomValue();
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->AttachZoom([this](const ui::EventArgs& /*args*/) {
            UpdateZoomValue();
            return true;
            });
    }

    //自动换行
    BindCheck(_T("btn_word_wrap"), [this](bool bSelected) {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetWordWrap(bSelected);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
            m_pRichEdit->SetAttribute(_T("hscrollbar"), bSelected ? _T("false") : _T("true"));
#endif
            UpdateStatusBar();
        }
        });

    //富文本模式
    BindCheck(_T("btn_rich_text"), [this](bool bSelected) {
        if (m_pRichEdit != nullptr) {
            m_bUpdatingRichTextCheck = true;
            m_bLoadingDocument = true;
            m_pRichEdit->SetRichText(bSelected);
            m_bLoadingDocument = false;
            m_bUpdatingRichTextCheck = false;
            UpdateSaveStatus();
            UpdateStatusBar();
            UpdateFontStatus();
            UpdateEditButtonStatus();
            UpdateRowSpacingEnabled();
        }
        });

    //行距相关控件：富文本关闭时禁用
    m_rowSpacingControls.clear();
    const DString rowSpacingNames[] = {
        _T("line_spacing_label"),
        _T("row_spacing_1_0"),
        _T("row_spacing_1_15"),
        _T("row_spacing_1_5"),
        _T("row_spacing_2_0"),
        _T("row_spacing_2_5"),
        _T("row_spacing_3_0")
    };
    for (const DString& name : rowSpacingNames) {
        ui::Control* pControl = FindControl(name);
        if (pControl != nullptr) {
            m_rowSpacingControls.push_back(pControl);
        }
    }
}

void MainForm::SetupTextEvents()
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->AttachTextChanged([this](const ui::EventArgs& /*args*/) {
            if (!m_bLoadingDocument) {
                m_statusMessage.clear();
                UpdateSaveStatus();
                UpdateStatusBar();
                UpdateEditButtonStatus();
            }
            return true;
            });
        m_pRichEdit->AttachLinkClick([this](const ui::EventArgs& args) {
            //点击了超级链接
            const DString::value_type* pUrl = (const DString::value_type*)args.wParam;
            if (pUrl != nullptr) {
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
                ::ShellExecuteW(NativeWnd()->GetHWND(), L"open", ui::StringConvert::TToWString(pUrl).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
            }
            return true;
            });
        m_pRichEdit->AttachSelChanged([this](const ui::EventArgs& /*args*/) {
            if (m_pRichEdit != nullptr) {
                UpdateFontStatus();
                UpdateStatusBar();
                UpdateEditButtonStatus();
            }
            return true;
            });
    }
}

void MainForm::SetupDropFile()
{
    if (m_pRichEdit == nullptr) {
        return;
    }
    m_pRichEdit->SetEnableDragDrop(true);
    m_pRichEdit->SetEnableDropFile(true);
    m_pRichEdit->SetDropFileTypes(_T(".txt;.rtf"));
    m_pRichEdit->AttachDropData([this](const ui::EventArgs& args) {
        std::vector<DString> fileList;
        if (args.wParam == ui::kControlDropTypeWindows) {
            const ui::ControlDropData_Windows* pData = (const ui::ControlDropData_Windows*)args.lParam;
            if (pData != nullptr) {
                fileList = pData->m_fileList;
            }
        }
        else if (args.wParam == ui::kControlDropTypeSDL) {
            const ui::ControlDropData_SDL* pData = (const ui::ControlDropData_SDL*)args.lParam;
            if (pData != nullptr) {
                fileList = pData->m_fileList;
            }
        }
        if (fileList.empty()) {
            return true;
        }
        const ui::FilePath filePath(fileList[0]);
        const DString fileExt = filePath.GetFileExtension();
        if ((fileExt != _T(".txt")) && (fileExt != _T(".rtf"))) {
            return true;
        }
        if (!ConfirmDiscardChanges()) {
            return true;
        }
        if (LoadFile(filePath)) {
            m_filePath = filePath;
            m_bSampleDocument = false;
            m_lastFileDir = filePath.GetParentPath();
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetModify(false);
            }
            m_statusMessage.clear();
            UpdateSaveStatus();
            UpdateStatusBar();
            UpdateEditButtonStatus();
        }
        else {
            ShowErrorMessage(_T("STRID_RICHEDIT_OPEN_FILE_FAILED"), _T("STRID_RICHEDIT_OPEN_FILE"));
        }
        return true;
        });
}

void MainForm::UpdateEditButtonStatus()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }
    const bool bHasSel = !pRichEdit->GetSelText().empty();

    auto setEnabled = [this](const DString& controlName, bool bEnabled) {
        ui::Control* pControl = FindControl(controlName);
        if (pControl != nullptr) {
            pControl->SetEnabled(bEnabled);
        }
        };
    setEnabled(_T("btn_undo"), pRichEdit->CanUndo());
    setEnabled(_T("btn_redo"), pRichEdit->CanRedo());
    setEnabled(_T("btn_paste"), pRichEdit->CanPaste());
    setEnabled(_T("btn_copy"), bHasSel);
    setEnabled(_T("btn_cut"), bHasSel);
    setEnabled(_T("btn_delete"), bHasSel);
    setEnabled(_T("btn_sel_none"), bHasSel);
    setEnabled(_T("btn_find_next"), m_findReplace.HasFindText());
}

void MainForm::UpdateRowSpacingEnabled()
{
    const bool bEnabled = (m_pRichEdit != nullptr) && m_pRichEdit->IsRichText();
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

void MainForm::InitZoomCombo()
{
    static const uint32_t zoomPercentList[] = { 50, 75, 100, 125, 150, 200, 300, 400 };
    ui::Combo* pCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_zoom")));
    if (pCombo == nullptr) {
        return;
    }
    pCombo->DeleteAllItems();
    for (uint32_t nZoomPercent : zoomPercentList) {
        DString text = ui::StringUtil::Printf(_T("%u%%"), nZoomPercent);
        size_t nItemIndex = pCombo->AddTextItem(text);
        if (ui::Box::IsValidItemIndex(nItemIndex)) {
            pCombo->SetItemData(nItemIndex, (size_t)nZoomPercent);
        }
    }
    pCombo->AttachSelect([this, pCombo](const ui::EventArgs& /*args*/) {
        const size_t nItemIndex = pCombo->GetCurSel();
        const uint32_t nZoomPercent = (uint32_t)pCombo->GetItemData(nItemIndex);
        if (nZoomPercent > 0) {
            //由下拉框自身触发的选择，不再回写下拉框，否则会打断下拉列表的关闭与键盘选择流程
            m_bZoomComboSelecting = true;
            SetZoomPercent(nZoomPercent);
            m_bZoomComboSelecting = false;
        }
        return true;
        });
    SyncZoomCombo();
}

void MainForm::SyncZoomCombo()
{
    ui::Combo* pCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_zoom")));
    if ((pCombo == nullptr) || (m_pRichEdit == nullptr)) {
        return;
    }
    const uint32_t nZoomPercent = m_pRichEdit->GetZoomPercent();
    const DString zoomText = ui::StringUtil::Printf(_T("%u%%"), nZoomPercent);
    bool bFound = false;
    //注意：Combo的条目数需要使用GetCount()，GetItemCount()返回的是子控件数量
    const size_t nItemCount = pCombo->GetCount();
    for (size_t nIndex = 0; nIndex < nItemCount; ++nIndex) {
        if ((uint32_t)pCombo->GetItemData(nIndex) == nZoomPercent) {
            //同步显示时不需要触发选择事件
            pCombo->SetCurSel(nIndex, false);
            bFound = true;
            break;
        }
    }
    if (!bFound) {
        pCombo->SetCurSel(ui::Box::InvalidIndex, false);
    }
    //下拉框为可编辑模式，同步编辑框中的文本（非标准比例时也能正确显示）
    pCombo->SetText(zoomText);
}

void MainForm::SetZoomPercent(uint32_t nZoomPercent)
{
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetZoomPercent(nZoomPercent);
    }
    UpdateZoomValue();
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

void MainForm::SetFontName(const DString& fontName)
{
    m_lastFontName = fontName;
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_fontName = fontName;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetFontSize(const DString& fontSize)
{
    for (const ui::FontSizeInfo& fontSizeInfo : m_fontSizeList) {
        if (fontSize == fontSizeInfo.fontSizeName) {
            if (m_pRichEdit != nullptr) {
                ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
                fontInfo.m_fontSize = (int32_t)(std::roundf(fontSizeInfo.fDpiFontSize));
                m_pRichEdit->SetFontInfo(fontInfo);
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
                    if (m_pRichEdit->SetFontInfo(fontInfo)) {
                        UpdateFontSizeStatus();
                    }
                }
            }
            else {
                //减小字体
                if (index > 0) {
                    fontInfo.m_fontSize = Dpi().GetScaleInt(fontSizeList[index - 1]);
                    if (m_pRichEdit->SetFontInfo(fontInfo)) {
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
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetFontItalic(bool bItalic)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bItalic = bItalic;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetFontUnderline(bool bUnderline)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bUnderline = bUnderline;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetFontStrikeOut(bool bStrikeOut)
{
    if (m_pRichEdit != nullptr) {
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        fontInfo.m_bStrikeOut = bStrikeOut;
        m_pRichEdit->SetFontInfo(fontInfo);
    }
}

void MainForm::SetTextColor(const DString& newColor)
{
    if (m_pRichEdit != nullptr) {
        if (m_pRichEdit->IsRichText()) {
            m_pRichEdit->SetSelectionTextColor(newColor);
        }
        else {
            m_pRichEdit->SetTextColor(newColor);
        }        
    }
}

LRESULT MainForm::OnWindowCloseMsg(uint32_t wParam, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    //有未保存的修改时，先提示用户处理，避免直接退出导致数据丢失
    if ((m_pRichEdit != nullptr) && m_pRichEdit->GetModify()) {
        int32_t nRet = ui::MessageBoxWnd::Show(this,
                            _T("STRID_RICHEDIT_UNSAVED_TEXT"),
                            _T("STRID_RICHEDIT_UNSAVED_TITLE"),
                            ui::MessageBoxWnd::kButtonsYesNoCancel,
                            ui::MessageBoxWnd::kIconQuestion,
                            nullptr, true);
        if (nRet == ui::MessageBoxWnd::kResultCancel) {
            //取消关闭
            bHandled = true;
            return 0;
        }
        if (nRet == ui::MessageBoxWnd::kResultYes) {
            if (!SaveDocument()) {
                //保存失败或用户取消了另存为，则不关闭窗口
                bHandled = true;
                return 0;
            }
        }
    }
    bHandled = false;
    return BaseClass::OnWindowCloseMsg(wParam, nativeMsg, bHandled);
}

void MainForm::OnPreCloseWindow()
{
    SaveSettings();
    BaseClass::OnPreCloseWindow();
}

void MainForm::OnCloseWindow()
{
    //关闭查找/替换窗口，避免遗留窗口
    if (m_pFindForm != nullptr) {
        m_pFindForm->CloseWnd();
        m_pFindForm = nullptr;
    }
    if (m_pReplaceForm != nullptr) {
        m_pReplaceForm->CloseWnd();
        m_pReplaceForm = nullptr;
    }

    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMsg(0);
}

bool MainForm::OnLanguageChanged()
{
    bool bRet = BaseClass::OnLanguageChanged();

    //界面语言切换后，按钮文字的 text_id 会被框架重新设置，需要重新记录基准文字
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("save_file")));
    if (pButton != nullptr) {
        m_saveBtnText = pButton->GetText();
        DString::size_type nPos = m_saveBtnText.find(_T("*"));
        if (nPos != DString::npos) {
            m_saveBtnText = m_saveBtnText.substr(0, nPos);
        }
    }
    UpdateSaveStatus();
    UpdateZoomValue();
    UpdateStatusBar();
    UpdateEditButtonStatus();
    if (m_bSampleDocument && (m_pRichEdit != nullptr) && !m_pRichEdit->GetModify()) {
        //示例文档未修改时，重新加载以刷新示例中的语言文本
        LoadRichEditData();
        UpdateFontStatus();
        UpdateStatusBar();
    }
    return bRet;
}

LRESULT MainForm::OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
    bool bControlDown = ui::Keyboard::IsKeyDown(ui::kVK_CONTROL);
    bool bShiftDown = ui::Keyboard::IsKeyDown(ui::kVK_SHIFT);
    bool bShortcutHandled = false;
    if (bControlDown) {
        if ((vkCode == 'N') && !bShiftDown) {
            //新建
            bShortcutHandled = true;
            OnNewFile();
        }
        else if ((vkCode == 'O') && !bShiftDown){
            //打开
            bShortcutHandled = true;
            OnOpenFile();
        }
        else if (vkCode == 'S') {
            bShortcutHandled = true;
            if (bShiftDown) {
                //另存为
                OnSaveAsFile();                
            }
            else {
                //保存
                OnSaveFile();
            }
        }
        if (!bShiftDown) {
            if (vkCode == 'F') {
                //查找
                bShortcutHandled = true;
                OnFindText();
            }
            else if (vkCode == 'H') {
                //替换
                bShortcutHandled = true;
                OnReplaceText();
            }
            else if (vkCode == '0') {
                //缩放复位
                bShortcutHandled = true;
                SetZoomPercent(100);
            }
        }
    }
    if (!bControlDown && !bShiftDown && (vkCode == ui::kVK_F3)) {
        //查找下一个
        bShortcutHandled = true;
        OnFindNext();
    }
    if (bShortcutHandled) {
        bHandled = true;
        return 0;
    }
    return lResult;
}

void MainForm::OnFindNext()
{
    if (!m_findReplace.HasFindText()) {
        //尚未设置查找内容时，打开查找窗口
        OnFindText();
        return;
    }
    if (!m_findReplace.FindNext()) {
        ShowStatusMessage(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_NOT_FOUND")));
    }
    else {
        m_statusMessage.clear();
        UpdateStatusBar();
    }
}

LRESULT MainForm::OnKeyUpMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    LRESULT lResult = BaseClass::OnKeyUpMsg(vkCode, modifierKey, nativeMsg, bHandled);
    return lResult;
}

void MainForm::UpdateSaveStatus()
{
    if (m_pRichEdit != nullptr) {
        ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("save_file")));
        if (m_pRichEdit->GetModify()) {
            if (pButton != nullptr) {
                pButton->SetText(m_saveBtnText + _T("*"));
            }
        }
        else {
            if (pButton != nullptr) {
                pButton->SetText(m_saveBtnText);
            }
        }
    }
}

void MainForm::UpdateStatusBar()
{
    if (m_pRichEdit == nullptr) {
        return;
    }

    //文件路径与修改标记
    ui::Label* pPathLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_path")));
    if (pPathLabel != nullptr) {
        DString filePath;
        if (m_bSampleDocument || m_filePath.IsEmpty()) {
            filePath = ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_UNTITLED"));
        }
        else {
            filePath = m_filePath.NativePath();
        }
        if (m_pRichEdit->GetModify()) {
            filePath += _T(" *");
        }
        pPathLabel->SetText(filePath);
    }

    //提示信息
    ui::Label* pHintLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_hint")));
    if (pHintLabel != nullptr) {
        pHintLabel->SetText(m_statusMessage);
        pHintLabel->SetVisible(!m_statusMessage.empty());
    }

    //行号与列号
    int32_t nStartChar = 0;
    int32_t nEndChar = 0;
    m_pRichEdit->GetSel(nStartChar, nEndChar);
    int32_t nLine = m_pRichEdit->LineFromChar(nStartChar);
    int32_t nLineStart = m_pRichEdit->LineIndex(nLine);
    int32_t nColumn = nStartChar - nLineStart;
    ui::Label* pPositionLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_position")));
    if (pPositionLabel != nullptr) {
        pPositionLabel->SetText(ui::StringUtil::Printf(_T("%d:%d"), nLine + 1, nColumn + 1));
    }

    //字符数（含选中字符数）
    uint32_t nTextLength = m_pRichEdit->GetTextLength();
    uint32_t nSelLength = static_cast<uint32_t>(nEndChar - nStartChar);
    ui::Label* pCharsLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_chars")));
    if (pCharsLabel != nullptr) {
        if (nSelLength > 0) {
            pCharsLabel->SetText(ui::StringUtil::Printf(_T("%u (%u)"), nTextLength, nSelLength));
        }
        else {
            pCharsLabel->SetText(ui::StringUtil::Printf(_T("%u"), nTextLength));
        }
    }

    //缩放比例
    ui::Label* pZoomLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_zoom")));
    if (pZoomLabel != nullptr) {
        pZoomLabel->SetText(ui::StringUtil::Printf(_T("%u%%"), m_pRichEdit->GetZoomPercent()));
    }

    //文本模式与换行状态
    ui::Label* pModeLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_mode")));
    if (pModeLabel != nullptr) {
        DString modeText = ui::GlobalManager::GetTextById(m_pRichEdit->IsRichText() ? _T("STRID_RICHEDIT_MODE_RICH_TEXT") : _T("STRID_RICHEDIT_MODE_PLAIN_TEXT"));
        DString wrapText = ui::GlobalManager::GetTextById(m_pRichEdit->IsWordWrap() ? _T("STRID_RICHEDIT_WRAP_ON") : _T("STRID_RICHEDIT_WRAP_OFF"));
        pModeLabel->SetText(modeText + _T(" · ") + wrapText);
    }

    //编码
    ui::Label* pEncodingLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_encoding")));
    if (pEncodingLabel != nullptr) {
        pEncodingLabel->SetText(m_encodingText);
    }
}

void MainForm::ShowStatusMessage(const DString& message)
{
    m_statusMessage = message;
    UpdateStatusBar();
}

void MainForm::ShowErrorMessage(const DString& textId, const DString& titleId)
{
    ui::MessageBoxWnd::Show(this, textId, titleId,
                            ui::MessageBoxWnd::kButtonsOK,
                            ui::MessageBoxWnd::kIconError,
                            nullptr, true);
}

bool MainForm::ConfirmDiscardChanges()
{
    if ((m_pRichEdit == nullptr) || !m_pRichEdit->GetModify()) {
        return true;
    }
    int32_t nRet = ui::MessageBoxWnd::Show(this,
                        _T("STRID_RICHEDIT_UNSAVED_TEXT"),
                        _T("STRID_RICHEDIT_UNSAVED_TITLE"),
                        ui::MessageBoxWnd::kButtonsYesNoCancel,
                        ui::MessageBoxWnd::kIconQuestion,
                        nullptr, true);
    if (nRet == ui::MessageBoxWnd::kResultCancel) {
        return false;
    }
    if (nRet == ui::MessageBoxWnd::kResultYes) {
        return SaveDocument();
    }
    return true;
}

bool MainForm::SaveDocument()
{
    return OnSaveFile();
}

void MainForm::LoadRichEditData()
{
    if (m_pRichEdit == nullptr) {
        return;
    }

    const DString title = ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_SAMPLE_TITLE"));
    const DString body = ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_SAMPLE_BODY"));
    const DString shortcutsTitle = ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_SAMPLE_SHORTCUTS_TITLE"));
    const DString shortcuts = ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_SAMPLE_SHORTCUTS"));
    const DString spacingTitle = ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_SAMPLE_LINE_SPACING_TITLE"));
    const DString spacing = ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_SAMPLE_LINE_SPACING"));

    DString sampleText = title + _T("\n\n") + body + _T("\n\n") + shortcutsTitle + _T("\n") + shortcuts + _T("\n\n") + spacingTitle + _T("\n") + spacing;
    const size_t posShortcutsTitle = title.size() + 2 + body.size() + 2;
    const size_t posSpacingTitle = posShortcutsTitle + shortcutsTitle.size() + 1 + shortcuts.size() + 2;

    m_bLoadingDocument = true;
    m_pRichEdit->SetText(sampleText);
    m_bLoadingDocument = false;
    m_pRichEdit->SetFocus();
    m_pRichEdit->SetSel(0, 0);
    m_pRichEdit->HomeUp();

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (m_pRichEdit->IsRichText()) {
        //标题：加粗、增大字号、强调色
        ui::UiFont fontInfo = m_pRichEdit->GetFontInfo();
        m_pRichEdit->SetSel(0, (int32_t)title.size());
        fontInfo.m_bBold = true;
        fontInfo.m_fontSize = Dpi().GetScaleInt(16);
        m_pRichEdit->SetFontInfo(fontInfo);
        m_pRichEdit->SetSelectionTextColor(_T("blue"));

        //快捷键标题：加粗
        fontInfo = m_pRichEdit->GetFontInfo();
        m_pRichEdit->SetSel((int32_t)posShortcutsTitle, (int32_t)(posShortcutsTitle + shortcutsTitle.size()));
        fontInfo.m_bBold = true;
        m_pRichEdit->SetFontInfo(fontInfo);

        //行间距示例标题：加粗
        fontInfo = m_pRichEdit->GetFontInfo();
        m_pRichEdit->SetSel((int32_t)posSpacingTitle, (int32_t)(posSpacingTitle + spacingTitle.size()));
        fontInfo.m_bBold = true;
        m_pRichEdit->SetFontInfo(fontInfo);

        //行间距示例正文：斜体
        fontInfo = m_pRichEdit->GetFontInfo();
        m_pRichEdit->SetSel((int32_t)(posSpacingTitle + spacingTitle.size() + 1), (int32_t)sampleText.size());
        fontInfo.m_bItalic = true;
        m_pRichEdit->SetFontInfo(fontInfo);

        //整篇示例的行间距为 1.25 倍
        m_pRichEdit->SetRowSpacingMul(1.25f);
    }
#endif

    m_pRichEdit->SetSel(0, 0);
    m_pRichEdit->HomeUp();
    m_pRichEdit->SetModify(false);
    //示例内容不是来自文件，保存时应转为另存为
    m_filePath.Clear();
    m_bSampleDocument = true;
    m_encodingText = _T("UTF-8");
}

void MainForm::OnFindText()
{
    if (m_pFindForm == nullptr) {
        m_pFindForm = new FindForm(this);
        ui::WindowCreateParam createParam;
        createParam.m_dwStyle = ui::kWS_POPUP;
        createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
        createParam.m_windowTitle = _T("FindForm");
        createParam.m_bCenterWindow = true;
        m_pFindForm->CreateWnd(this, createParam);
        m_pFindForm->ShowWindow(ui::kSW_SHOW);
        m_pFindForm->AttachWindowCloseMsg([this](const ui::EventArgs& args) {
                m_pFindForm = nullptr;
                return true;
            });
    }
    else {
        if (m_pFindForm->IsWindowMinimized()) {
            m_pFindForm->ShowWindow(ui::kSW_RESTORE);
        }
        else {
            m_pFindForm->ShowWindow(ui::kSW_SHOW);
        }
    }
}

void MainForm::OnReplaceText()
{
    if (m_pReplaceForm == nullptr) {
        m_pReplaceForm = new ReplaceForm(this);
        ui::WindowCreateParam createParam;
        createParam.m_dwStyle = ui::kWS_POPUP;
        createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
        createParam.m_windowTitle = _T("ReplaceForm");
        createParam.m_bCenterWindow = true;
        m_pReplaceForm->CreateWnd(this, createParam);
        m_pReplaceForm->ShowWindow(ui::kSW_SHOW);
        m_pReplaceForm->AttachWindowCloseMsg([this](const ui::EventArgs& args) {
                m_pReplaceForm = nullptr;
                return true;
            });
    }
    else {
        if (m_pReplaceForm->IsWindowMinimized()) {
            m_pReplaceForm->ShowWindow(ui::kSW_RESTORE);
        }
        else {
            m_pReplaceForm->ShowWindow(ui::kSW_SHOW);
        }
    }
}

bool MainForm::FindRichText(const DString& findText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
{
    bool bFound = m_findReplace.FindRichText(findText, bFindDown, bMatchCase, bMatchWholeWord, pWndDialog);
    if (!bFound) {
        ShowStatusMessage(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_NOT_FOUND")));
    }
    else {
        m_statusMessage.clear();
        UpdateStatusBar();
    }
    UpdateEditButtonStatus();
    return bFound;
}

void MainForm::ReplaceRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
{
    if (m_findReplace.ReplaceRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, pWndDialog)) {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetModify(true);
            UpdateSaveStatus();
            UpdateStatusBar();
        }
        const int32_t nReplaceCount = m_findReplace.GetLastReplaceCount();
        ShowStatusMessage(ui::StringUtil::Printf(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_REPLACE_DONE")).c_str(), (int32_t)nReplaceCount));
    }
    else {
        ShowStatusMessage(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_NOT_FOUND")));
    }
    UpdateEditButtonStatus();
}

void MainForm::ReplaceAllRichText(const DString& findText, const DString& replaceText, bool bFindDown, bool bMatchCase, bool bMatchWholeWord, ui::Window* pWndDialog)
{
    if (m_findReplace.ReplaceAllRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, pWndDialog)) {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetModify(true);
            UpdateSaveStatus();
            UpdateStatusBar();
        }
        const int32_t nReplaceCount = m_findReplace.GetLastReplaceCount();
        ShowStatusMessage(ui::StringUtil::Printf(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_REPLACE_ALL_DONE")).c_str(), (int32_t)nReplaceCount));
    }
    else {
        ShowStatusMessage(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_NOT_FOUND")));
    }
    UpdateEditButtonStatus();
}

ui::RichEdit* MainForm::GetRichEdit() const
{
    return m_pRichEdit;
}

const RichEditFindOptions& MainForm::GetFindOptions() const
{
    return m_findOptions;
}

void MainForm::SetFindOptions(const RichEditFindOptions& options)
{
    m_findOptions = options;
}

ui::FilePath MainForm::GetSettingsFilePath() const
{
    //保存在程序目录下的user子目录中（跨平台，且不依赖系统的用户配置目录）
    ui::FilePath filePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    filePath.JoinFilePath(ui::FilePath(_T("user")));
    filePath.JoinFilePath(ui::FilePath(_T("RichEdit_settings.ini")));
    return filePath;
}

void MainForm::LoadSettings()
{
    SettingsMap settings;
    ReadSettingsFile(GetSettingsFilePath(), settings);

    //设置文件的版本号：版本号小于2时，使用新的默认界面布局与默认编辑模式
    const int32_t nSettingsVersion = GetSettingInt(settings, "settings_version", 0);

    //1. 窗口位置与大小（保存的是物理像素）
    const int32_t nLeft = GetSettingInt(settings, "window_left", 0);
    const int32_t nTop = GetSettingInt(settings, "window_top", 0);
    const int32_t nWidth = GetSettingInt(settings, "window_width", 0);
    const int32_t nHeight = GetSettingInt(settings, "window_height", 0);
    if ((nSettingsVersion >= 2) && (nWidth > 0) && (nHeight > 0)) {
        ui::UiRect rcWork;
        if (!GetMonitorWorkRect(ui::UiPoint(nLeft, nTop), rcWork)) {
            GetMonitorWorkRect(rcWork);
        }
        const int32_t nWindowWidth = std::min(nWidth, rcWork.Width());
        const int32_t nWindowHeight = std::min(nHeight, rcWork.Height());
        const int32_t nWindowLeft = std::max(rcWork.left, std::min(nLeft, rcWork.right - nWindowWidth));
        const int32_t nWindowTop = std::max(rcWork.top, std::min(nTop, rcWork.bottom - nWindowHeight));
        SetWindowPos(ui::InsertAfterWnd(), nWindowLeft, nWindowTop, nWindowWidth, nWindowHeight, ui::kSWP_NOZORDER);
    }

    //2. 左侧面板的宽度
    const int32_t nLeftPanelWidth = GetSettingInt(settings, "left_panel_width", -1);
    if ((nSettingsVersion >= 2) && (nLeftPanelWidth > 0)) {
        ui::Control* pLeftPanel = FindControl(_T("left_panel"));
        if (pLeftPanel != nullptr) {
            const int32_t nMinWidth = 60;
            const int32_t nMaxWidth = 2048;
            pLeftPanel->SetFixedWidth(ui::UiFixedInt(std::max(nMinWidth, std::min(nLeftPanelWidth, nMaxWidth))), true, true);
        }
    }

    if (m_pRichEdit != nullptr) {
        //3. 缩放比例
        const int32_t nZoomPercent = GetSettingInt(settings, "zoom_percent", 100);
        if ((nSettingsVersion >= 2) && (nZoomPercent > 0)) {
            m_pRichEdit->SetZoomPercent((uint32_t)nZoomPercent);
        }

        //4. 自动换行
        const int32_t nWordWrap = GetSettingInt(settings, "word_wrap", -1);
        if (nWordWrap >= 0) {
            m_pRichEdit->SetWordWrap(nWordWrap != 0);
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
            m_pRichEdit->SetAttribute(_T("hscrollbar"), (nWordWrap != 0) ? _T("false") : _T("true"));
#endif
        }

        //5. 富文本模式（该模式缺少跨平台兼容性，旧版本设置不恢复，默认关闭）
        const int32_t nRichText = GetSettingInt(settings, "rich_text", -1);
        if ((nSettingsVersion >= 2) && (nRichText >= 0)) {
            m_bUpdatingRichTextCheck = true;
            m_bLoadingDocument = true;
            m_pRichEdit->SetRichText(nRichText != 0);
            m_bLoadingDocument = false;
            m_bUpdatingRichTextCheck = false;
        }

        //同步复选框的选中状态
        ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_word_wrap")));
        if (pCheckBox != nullptr) {
            pCheckBox->SetSelected(m_pRichEdit->IsWordWrap());
        }
        pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_rich_text")));
        if (pCheckBox != nullptr) {
            pCheckBox->SetSelected(m_pRichEdit->IsRichText());
        }
    }

    //6. 字体名称
    const std::string fontNameUtf8 = GetSettingString(settings, "font_name", std::string());
    if (!fontNameUtf8.empty()) {
        SetFontName(ui::StringConvert::UTF8ToT(fontNameUtf8));
    }
}

void MainForm::SaveSettings() const
{
    std::string settingsText;
    settingsText += "# RichEdit 示例的用户设置（程序自动生成，删除该文件后恢复默认设置）\n";
    AppendSettingInt(settingsText, "settings_version", 2);

    //1. 窗口位置与大小
    ui::UiRect rcWindow;
    GetWindowRect(rcWindow);
    AppendSettingInt(settingsText, "window_left", rcWindow.left);
    AppendSettingInt(settingsText, "window_top", rcWindow.top);
    AppendSettingInt(settingsText, "window_width", rcWindow.Width());
    AppendSettingInt(settingsText, "window_height", rcWindow.Height());

    //2. 左侧面板的宽度（保存未缩放的值）
    int32_t nLeftPanelWidth = -1;
    ui::Control* pLeftPanel = FindControl(_T("left_panel"));
    if (pLeftPanel != nullptr) {
        nLeftPanelWidth = Dpi().GetUnscaleInt(pLeftPanel->GetWidth());
    }
    AppendSettingInt(settingsText, "left_panel_width", nLeftPanelWidth);

    if (m_pRichEdit != nullptr) {
        //3. 缩放比例
        AppendSettingInt(settingsText, "zoom_percent", (int32_t)m_pRichEdit->GetZoomPercent());

        //4. 自动换行
        AppendSettingInt(settingsText, "word_wrap", m_pRichEdit->IsWordWrap() ? 1 : 0);

        //5. 富文本模式
        AppendSettingInt(settingsText, "rich_text", m_pRichEdit->IsRichText() ? 1 : 0);
    }

    //6. 字体名称
    AppendSettingString(settingsText, "font_name", ui::StringConvert::TToUTF8(m_lastFontName));

    const ui::FilePath filePath = GetSettingsFilePath();
    //创建设置文件所在的目录（不存在时自动创建）
    ui::FilePathUtil::CreateDirectories(filePath.GetParentPath().ToString());
    const std::vector<uint8_t> fileData(settingsText.begin(), settingsText.end());
    ui::FileUtil::WriteFileData(filePath, fileData);
}

void MainForm::OnWindowDisplayScaleChanged(uint32_t nOldScaleFactor, uint32_t nNewScaleFactor)
{
    BaseClass::OnWindowDisplayScaleChanged(nOldScaleFactor, nNewScaleFactor);
    InitFontSizeCombo();
    UpdateFontStatus();
    UpdateZoomValue();
}

void MainForm::UpdateZoomValue()
{
    ui::RichEdit* pRichEdit = GetRichEdit();
    if (pRichEdit == nullptr) {
        return;
    }

    ui::Label* pZoomLabel = dynamic_cast<ui::Label*>(FindControl(_T("lavel_zoom_value")));
    if (pZoomLabel != nullptr) {
        uint32_t nZoomPercent = pRichEdit->GetZoomPercent();
        DString strZoom = ui::StringUtil::Printf(_T("%u%%"), nZoomPercent);
        pZoomLabel->SetText(strZoom);
    }
    if (!m_bZoomComboSelecting) {
        SyncZoomCombo();
    }
    UpdateStatusBar();
}

bool MainForm::OnNewFile()
{
    if (!ConfirmDiscardChanges()) {
        return false;
    }
    if (m_pRichEdit != nullptr) {
        m_bLoadingDocument = true;
        m_pRichEdit->SetText(DString());
        m_bLoadingDocument = false;
        m_pRichEdit->SetModify(false);
        m_pRichEdit->SetFocus();
        m_pRichEdit->SetSel(0, 0);
    }
    m_filePath.Clear();
    m_bSampleDocument = false;
    m_encodingText = _T("UTF-8");
    m_statusMessage.clear();
    UpdateSaveStatus();
    UpdateStatusBar();
    UpdateEditButtonStatus();
    return true;
}

bool MainForm::OnOpenFile()
{
    if (!ConfirmDiscardChanges()) {
        return false;
    }

    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_ALL_FILES")), _T("*.*")});
    fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_TXT_FILES")), _T("*.txt") });
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    //仅富文本模式支持RTF文件，避免选择后无法打开
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
        fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_RTF_FILES")), _T("*.rtf") });
    }
#endif

    DString defaultExt;
    int32_t nFileTypeIndex = 1;
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if ((m_pRichEdit != nullptr) && m_pRichEdit->IsRichText()) {
        nFileTypeIndex = 2;
    }
#endif
    DString fileName;
    if (!m_filePath.IsEmpty() && !m_bSampleDocument) {
        fileName = m_filePath.GetFileName();
    }

    ui::FilePath filePath;
    ui::FileDialog openFileDlg;
    //初始的目录：当前文件所在的目录，或者上一次使用的目录
    ui::FilePath defaultLocation = m_filePath.GetParentPath();
    if (defaultLocation.IsEmpty() || m_bSampleDocument) {
        defaultLocation = m_lastFileDir;
    }
    if (defaultLocation.IsEmpty() || !defaultLocation.IsExistsDirectory()) {
        //默认目录不存在时，使用程序所在目录（避免文件对话框使用无效目录）
        defaultLocation = ui::FilePathUtil::GetCurrentModuleDirectory();
    }
    if (openFileDlg.BrowseForFile(this, filePath, true, fileTypes, nFileTypeIndex, defaultExt, fileName, defaultLocation)) {
        if (LoadFile(filePath)) {
            m_filePath = filePath;
            m_bSampleDocument = false;
            m_lastFileDir = filePath.GetParentPath();
            if (m_pRichEdit != nullptr) {
                m_pRichEdit->SetModify(false);
            }
            m_statusMessage.clear();
            UpdateSaveStatus();
            UpdateStatusBar();
        }
        else {
            ShowErrorMessage(_T("STRID_RICHEDIT_OPEN_FAILED_TEXT"), _T("STRID_RICHEDIT_OPEN_FAILED_TITLE"));
        }
    }
    return true;
}

bool MainForm::OnSaveFile()
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    if (m_bSampleDocument || m_filePath.IsEmpty()) {
        //示例文档或未命名文档，转成另存为
        return OnSaveAsFile();
    }
    if (!m_pRichEdit->GetModify()) {
        return true;
    }
    if (!SaveFile(m_filePath)) {
        ShowErrorMessage(_T("STRID_RICHEDIT_SAVE_FAILED_TEXT"), _T("STRID_RICHEDIT_SAVE_FAILED_TITLE"));
        return false;
    }
    m_pRichEdit->SetModify(false);
    m_statusMessage.clear();
    UpdateSaveStatus();
    UpdateStatusBar();
    return true;
}

bool MainForm::OnSaveAsFile()
{
    bool bRichText = (m_pRichEdit != nullptr) && m_pRichEdit->IsRichText();

    std::vector<ui::FileDialog::FileType> fileTypes;
    fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_ALL_FILES")), _T("*.*") });
    fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_TXT_FILES")), _T("*.txt") });
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    //仅富文本模式支持保存为RTF文件，避免选择后保存失败
    if (bRichText) {
        fileTypes.push_back({ ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_RTF_FILES")), _T("*.rtf") });
    }
#endif

    DString defaultExt = bRichText ? _T("rtf") : _T("txt");
    int32_t nFileTypeIndex = 1;
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    if (bRichText) {
        nFileTypeIndex = 2;
    }
#endif
    DString fileName;
    if (!m_filePath.IsEmpty() && !m_bSampleDocument) {
        fileName = m_filePath.GetFileName();
    }
    else {
        fileName = defaultExt == _T("rtf") ? _T("untitled.rtf") : _T("untitled.txt");
    }

    ui::FilePath filePath;
    ui::FileDialog saveFileDlg;
    //初始的目录：当前文件所在的目录，或者上一次使用的目录
    ui::FilePath defaultLocation = m_filePath.GetParentPath();
    if (defaultLocation.IsEmpty() || m_bSampleDocument) {
        defaultLocation = m_lastFileDir;
    }
    if (defaultLocation.IsEmpty() || !defaultLocation.IsExistsDirectory()) {
        //默认目录不存在时，使用程序所在目录（避免文件对话框使用无效目录）
        defaultLocation = ui::FilePathUtil::GetCurrentModuleDirectory();
    }
    if (!saveFileDlg.BrowseForFile(this, filePath, false, fileTypes, nFileTypeIndex, defaultExt, fileName, defaultLocation)) {
        return false;
    }
    if (!SaveFile(filePath)) {
        ShowErrorMessage(_T("STRID_RICHEDIT_SAVE_FAILED_TEXT"), _T("STRID_RICHEDIT_SAVE_FAILED_TITLE"));
        return false;
    }
    m_filePath = filePath;
    m_bSampleDocument = false;
    m_lastFileDir = filePath.GetParentPath();
    if (m_pRichEdit != nullptr) {
        m_pRichEdit->SetModify(false);
    }
    m_statusMessage.clear();
    UpdateSaveStatus();
    UpdateStatusBar();
    return true;
}

//读取文本文件（自动识别 UTF-8/UTF-16/ANSI 编码），空文件也能正常打开
static bool ReadTextFileContent(const ui::FilePath& filePath, DStringW& text, DString& encodingText)
{
    std::vector<uint8_t> fileData;
    if (!ui::FileUtil::ReadFileData(filePath, fileData)) {
        return false;
    }
    text.clear();
    encodingText = _T("UTF-8");
    if (fileData.empty()) {
        return true;
    }
    ui::CharsetType charsetType = ui::CharsetType::UNKNOWN;
    uint32_t bomSize = 0;
    if (!ui::StringCharset::GetDataAsString((const char*)fileData.data(), (uint32_t)fileData.size(), text, charsetType, bomSize)) {
        return false;
    }
    switch (charsetType) {
    case ui::CharsetType::ANSI:
        encodingText = _T("ANSI");
        break;
    case ui::CharsetType::UTF16_LE:
        encodingText = _T("UTF-16LE");
        break;
    case ui::CharsetType::UTF16_BE:
        encodingText = _T("UTF-16BE");
        break;
    default:
        encodingText = _T("UTF-8");
        break;
    }
    return true;
}

//保存文本文件：原文件已存在时尽量保持原编码，新文件按 UTF-8 保存；空文本也允许保存
static bool WriteTextFileContent(const ui::FilePath& filePath, const DStringW& text, DString& encodingText)
{
    encodingText = _T("UTF-8");
    ui::CharsetType charsetType = ui::CharsetType::UNKNOWN;
    std::vector<uint8_t> oldFileData;
    if (ui::FileUtil::ReadFileData(filePath, oldFileData) && !oldFileData.empty()) {
        charsetType = ui::StringCharset::GetDataCharsetByBOM((const char*)oldFileData.data(), (uint32_t)oldFileData.size());
        if (charsetType == ui::CharsetType::UNKNOWN) {
            charsetType = ui::StringCharset::GetDataCharset((const char*)oldFileData.data(), (uint32_t)oldFileData.size());
        }
        oldFileData.clear();
    }
    if (charsetType == ui::CharsetType::UTF16_LE) {
        encodingText = _T("UTF-16LE");
        return ui::FileUtil::WriteFileData(filePath, text);
    }
    if (charsetType == ui::CharsetType::ANSI) {
#ifdef DUILIB_BUILD_FOR_WIN
        encodingText = _T("ANSI");
        DStringA textA = ui::StringConvert::UnicodeToMBCS(text);
#else
        DStringA textA = ui::StringConvert::WStringToUTF8(text);
#endif
        return ui::FileUtil::WriteFileData(filePath, textA);
    }
    DStringA textA = ui::StringConvert::WStringToUTF8(text);
    return ui::FileUtil::WriteFileData(filePath, textA);
}

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

bool MainForm::IsRtfFile(const DString& filePath) const
{
    DString fileExt = ui::FilePathUtil::GetFileExtension(filePath);
    return ui::StringUtil::IsEqualNoCase(fileExt, _T("rtf"));
}

bool MainForm::LoadFile(const ui::FilePath& filePath)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DString filePathLocal = filePath.NativePath();

    //非富文本模式不支持读取RTF文件，避免把RTF二进制当成文本读取
    if (!m_pRichEdit->IsRichText() && IsRtfFile(filePathLocal)) {
        return false;
    }

    //RTF文件使用RichEdit的流接口读取，可以保留格式
    if (m_pRichEdit->IsRichText() && IsRtfFile(filePathLocal)) {
        HANDLE hFile = ::CreateFile(filePathLocal.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        EDITSTREAM es;
        es.dwCookie = (DWORD_PTR)hFile;
        es.dwError = 0;
        es.pfnCallback = StreamReadCallback;
        m_pRichEdit->StreamIn(SF_RTF, es);
        ::CloseHandle(hFile);
        if ((BOOL)es.dwError) {
            return false;
        }
        m_encodingText = _T("RTF");
        return true;
    }

    //普通文本统一按自动识别的编码读取，避免UTF-8/Unicode文件乱码
    DStringW text;
    if (!ReadTextFileContent(filePath, text, m_encodingText)) {
        return false;
    }
    m_bLoadingDocument = true;
    m_pRichEdit->SetText(text);
    m_bLoadingDocument = false;
    return true;
}

bool MainForm::SaveFile(const ui::FilePath& filePath)
{
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DString filePathLocal = filePath.NativePath();

    //非富文本模式不支持保存为RTF文件
    if (!m_pRichEdit->IsRichText() && IsRtfFile(filePathLocal)) {
        return false;
    }

    //RTF文件使用RichEdit的流接口保存，可以保留格式
    if (m_pRichEdit->IsRichText() && IsRtfFile(filePathLocal)) {
        HANDLE hFile = ::CreateFile(filePathLocal.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            return false;
        }

        EDITSTREAM es;
        es.dwCookie = (DWORD_PTR)hFile;
        es.dwError = 0;
        es.pfnCallback = StreamWriteCallback;
        m_pRichEdit->StreamOut(SF_RTF, es);
        ::CloseHandle(hFile);
        if ((BOOL)es.dwError) {
            return false;
        }
        m_encodingText = _T("RTF");
        return true;
    }

    //普通文本统一保存，空文本也允许保存
    if (!WriteTextFileContent(filePath, m_pRichEdit->GetText(), m_encodingText)) {
        return false;
    }
    return true;
}

DWORD MainForm::StreamReadCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
    ASSERT(dwCookie != 0);
    ASSERT(pcb != nullptr);

    return !::ReadFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, nullptr);
}

DWORD MainForm::StreamWriteCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG FAR* pcb)
{
    ASSERT(dwCookie != 0);
    ASSERT(pcb != nullptr);

    return !::WriteFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, nullptr);
}

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
    if (pRichEdit != nullptr) {
        if (pRichEdit->IsRichText()) {
            pRichEdit->SetSelectionCharFormat(charFormat);
        }
        else {
            pRichEdit->SetDefaultCharFormat(charFormat);
        }
    }
}

#else //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

bool MainForm::LoadFile(const ui::FilePath& filePath)
{
    //打开文件
    if (m_pRichEdit == nullptr) {
        return false;
    }
    DStringW text;
    if (!ReadTextFileContent(filePath, text, m_encodingText)) {
        return false;
    }
    m_bLoadingDocument = true;
    m_pRichEdit->SetText(text);
    m_bLoadingDocument = false;
    return true;
}

bool MainForm::SaveFile(const ui::FilePath& filePath)
{
    //保存
    if (m_pRichEdit == nullptr) {
        return false;
    }
    return WriteTextFileContent(filePath, m_pRichEdit->GetTextW(), m_encodingText);
}

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
