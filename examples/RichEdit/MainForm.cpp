#include "MainForm.h"
#include "FindForm.h"
#include "ReplaceForm.h"
#include "MainThread.h"
#include "duilib/Utils/MessageBoxWnd.h"

MainForm::MainForm():
    m_pRichEdit(nullptr),
    m_bSampleDocument(true),
    m_bLoadingDocument(false),
    m_bUpdatingRichTextCheck(false),
    m_bZoomComboSelecting(false),
    m_statusMessageTimerId(0),
    m_encodingText(_T("UTF-8")),
    m_pFindForm(nullptr),
    m_pReplaceForm(nullptr),
    m_pStatusPathLabel(nullptr),
    m_pStatusHintLabel(nullptr),
    m_pStatusPositionLabel(nullptr),
    m_pStatusCharsLabel(nullptr),
    m_pStatusZoomLabel(nullptr),
    m_pStatusModeLabel(nullptr),
    m_pStatusEncodingLabel(nullptr)
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
    InitStatusBarLabels();

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
                    ui::SystemUtil::OpenUrl(DString(pUrl));
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
        //条目文本就是字体名称（SetFontName 取的是 GetText()），不需要再维护额外的条目数据
        std::vector<DString> fontNameList;
        ui::GlobalManager::Instance().Font().GetFontNameList(fontNameList);
        for (const DString& fontName : fontNameList) {
            pFontNameCombo->AddTextItem(fontName);
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
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    BindCheck(_T("btn_rich_text"), [this](bool bSelected) {
        if ((m_pRichEdit == nullptr) || m_bUpdatingRichTextCheck) {
            return;
        }
        if (!bSelected && m_pRichEdit->IsRichText()) {
            //切换到纯文本模式会丢弃文档中的格式，必须先提示用户确认
            int32_t nRet = ui::MessageBoxWnd::Show(this,
                                _T("STRID_RICHEDIT_RICHTEXT_SWITCH_TEXT"),
                                _T("STRID_RICHEDIT_RICHTEXT_SWITCH_TITLE"),
                                ui::MessageBoxWnd::kButtonsYesNo,
                                ui::MessageBoxWnd::kIconQuestion,
                                nullptr, true);
            if (nRet != ui::MessageBoxWnd::kResultYes) {
                //用户取消切换：把复选框恢复为选中状态（设置标志位，避免再次触发本回调）
                m_bUpdatingRichTextCheck = true;
                ui::CheckBox* pCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_rich_text")));
                if (pCheckBox != nullptr) {
                    pCheckBox->SetSelected(true);
                }
                m_bUpdatingRichTextCheck = false;
                return;
            }
        }
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
        });
#else
    //非Windows平台（含Windows的SDL版本）：RichEdit不支持富文本模式，禁用该复选框
    ui::CheckBox* pRichTextCheckBox = dynamic_cast<ui::CheckBox*>(FindControl(_T("btn_rich_text")));
    if (pRichTextCheckBox != nullptr) {
        pRichTextCheckBox->SetSelected(false);
        pRichTextCheckBox->SetEnabled(false);
    }
#endif

    //缩放比例复位
    BindClick(_T("btn_zoom_reset"), [this]() { SetZoomPercent(100); });

    //行距与"应用到全文"：富文本关闭时禁用
    m_rowSpacingControls.clear();
    const DString rowSpacingNames[] = {
        _T("line_spacing_label"),
        _T("row_spacing_1_0"),
        _T("row_spacing_1_15"),
        _T("row_spacing_1_5"),
        _T("row_spacing_2_0"),
        _T("row_spacing_2_5"),
        _T("row_spacing_3_0"),
        _T("btn_apply_to_all")
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
                ui::SystemUtil::OpenUrl(DString(pUrl));
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
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    m_pRichEdit->SetDropFileTypes(_T(".txt;.rtf"));
#else
    //非Windows平台（含Windows的SDL版本）只支持纯文本文件
    m_pRichEdit->SetDropFileTypes(_T(".txt"));
#endif
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
        bool bSupportedFile = ui::StringUtil::IsEqualNoCase(fileExt, _T(".txt"));
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
        bSupportedFile = bSupportedFile || ui::StringUtil::IsEqualNoCase(fileExt, _T(".rtf"));
#endif
        if (!bSupportedFile) {
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
            UpdateFontStatus();
            UpdateRowSpacingEnabled();
        }
        else {
            ShowErrorMessage(_T("STRID_RICHEDIT_OPEN_FAILED_TEXT"), _T("STRID_RICHEDIT_OPEN_FAILED_TITLE"));
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
    //仅取选区的起止位置判断是否有选中内容，避免大文件全选时拷贝整段文本
    int32_t nStartChar = 0;
    int32_t nEndChar = 0;
    pRichEdit->GetSel(nStartChar, nEndChar);
    const bool bHasSel = (nEndChar > nStartChar);

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

    //下拉框为可编辑模式：支持直接输入自定义的缩放比例（回车或失去焦点时生效）
    ui::RichEdit* pEdit = pCombo->GetEditControl();
    if (pEdit != nullptr) {
        pEdit->AttachReturn([this](const ui::EventArgs& /*args*/) {
            ApplyZoomText();
            return true;
            });
        pEdit->AttachKillFocus([this](const ui::EventArgs& /*args*/) {
            ApplyZoomText();
            return true;
            });
    }
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

void MainForm::ApplyZoomText()
{
    if ((m_pRichEdit == nullptr) || m_bZoomComboSelecting) {
        return;
    }
    ui::Combo* pCombo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_zoom")));
    if (pCombo == nullptr) {
        return;
    }
    DString text = pCombo->GetText();
    ui::StringUtil::Trim(text);
    if (!text.empty() && (text[text.size() - 1] == _T('%'))) {
        text = text.substr(0, text.size() - 1);
        ui::StringUtil::Trim(text);
    }
    int32_t nZoomPercent = 0;
    if (!text.empty()) {
        nZoomPercent = ui::StringUtil::StringToInt32(text.c_str());
    }
    if ((nZoomPercent >= 50) && (nZoomPercent <= 400)) {
        if ((uint32_t)nZoomPercent != m_pRichEdit->GetZoomPercent()) {
            SetZoomPercent((uint32_t)nZoomPercent);
            return;
        }
    }
    //输入无效或与当前缩放比例相同时，把编辑框的文本恢复为规范格式（如"150%"）
    SyncZoomCombo();
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

void MainForm::InitStatusBarLabels()
{
    m_pStatusPathLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_path")));
    m_pStatusHintLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_hint")));
    m_pStatusPositionLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_position")));
    m_pStatusCharsLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_chars")));
    m_pStatusZoomLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_zoom")));
    m_pStatusModeLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_mode")));
    m_pStatusEncodingLabel = dynamic_cast<ui::Label*>(FindControl(_T("status_encoding")));
}

void MainForm::UpdateStatusBar()
{
    if (m_pRichEdit == nullptr) {
        return;
    }

    //文件路径与修改标记
    if (m_pStatusPathLabel != nullptr) {
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
        m_pStatusPathLabel->SetText(filePath);
    }

    //提示信息
    if (m_pStatusHintLabel != nullptr) {
        m_pStatusHintLabel->SetText(m_statusMessage);
        m_pStatusHintLabel->SetVisible(!m_statusMessage.empty());
    }

    //行号与列号
    int32_t nStartChar = 0;
    int32_t nEndChar = 0;
    m_pRichEdit->GetSel(nStartChar, nEndChar);
    int32_t nLine = m_pRichEdit->LineFromChar(nStartChar);
    //光标在文本末尾时，SDL版本的LineFromChar返回的行号可能超出有效范围，需要做保护
    const int32_t nLineCount = m_pRichEdit->GetLineCount();
    if ((nLineCount > 0) && (nLine >= nLineCount)) {
        nLine = nLineCount - 1;
    }
    if (nLine < 0) {
        nLine = 0;
    }
    int32_t nLineStart = m_pRichEdit->LineIndex(nLine);
    if (nLineStart < 0) {
        nLineStart = 0;
    }
    int32_t nColumn = nStartChar - nLineStart;
    if (m_pStatusPositionLabel != nullptr) {
        //显示格式：行:列 3:12
        m_pStatusPositionLabel->SetText(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_STATUS_POSITION_LABEL")) +
                                        ui::StringUtil::Printf(_T(" %d:%d"), nLine + 1, nColumn + 1));
    }

    //字符数（含选中字符数）
    uint32_t nTextLength = m_pRichEdit->GetTextLength();
    uint32_t nSelLength = static_cast<uint32_t>(nEndChar - nStartChar);
    if (m_pStatusCharsLabel != nullptr) {
        //显示格式：字符数 1289，有选中文本时为：字符数 1289 · 选中 12
        const DString charsLabel = ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_STATUS_CHARS_LABEL"));
        if (nSelLength > 0) {
            m_pStatusCharsLabel->SetText(charsLabel + ui::StringUtil::Printf(_T(" %u"), nTextLength) +
                                         _T(" · ") +
                                         ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_STATUS_SELECTED_LABEL")) +
                                         ui::StringUtil::Printf(_T(" %u"), nSelLength));
        }
        else {
            m_pStatusCharsLabel->SetText(charsLabel + ui::StringUtil::Printf(_T(" %u"), nTextLength));
        }
    }

    //缩放比例
    if (m_pStatusZoomLabel != nullptr) {
        //显示格式：缩放 100%
        m_pStatusZoomLabel->SetText(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_STATUS_ZOOM_LABEL")) +
                                    ui::StringUtil::Printf(_T(" %u%%"), m_pRichEdit->GetZoomPercent()));
    }

    //文本模式与换行状态
    if (m_pStatusModeLabel != nullptr) {
        DString modeText = ui::GlobalManager::GetTextById(m_pRichEdit->IsRichText() ? _T("STRID_RICHEDIT_MODE_RICH_TEXT") : _T("STRID_RICHEDIT_MODE_PLAIN_TEXT"));
        DString wrapText = ui::GlobalManager::GetTextById(m_pRichEdit->IsWordWrap() ? _T("STRID_RICHEDIT_WRAP_ON") : _T("STRID_RICHEDIT_WRAP_OFF"));
        m_pStatusModeLabel->SetText(modeText + _T(" · ") + wrapText);
    }

    //编码
    if (m_pStatusEncodingLabel != nullptr) {
        //显示格式：编码 UTF-8
        m_pStatusEncodingLabel->SetText(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_STATUS_ENCODING_LABEL")) +
                                        _T(" ") + m_encodingText);
    }
}

void MainForm::ShowStatusMessage(const DString& message)
{
    ui::TimerManager& timerManager = ui::GlobalManager::Instance().Timer();
    //先取消上一次的自动清除定时器
    if (m_statusMessageTimerId != 0) {
        timerManager.RemoveTimer(m_statusMessageTimerId);
        m_statusMessageTimerId = 0;
    }

    m_statusMessage = message;
    UpdateStatusBar();

    if (!message.empty()) {
        //提示信息显示3秒后自动清除
        m_statusMessageTimerId = timerManager.AddTimer(GetWeakFlag(), [this]() {
            m_statusMessageTimerId = 0;
            m_statusMessage.clear();
            UpdateStatusBar();
        }, 3000, 1);
    }
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
        //在设置任何格式之前先记录基础字体，避免各段格式相互继承
        const ui::UiFont baseFont = m_pRichEdit->GetFontInfo();
        ui::UiFont fontInfo = baseFont;

        //标题：加粗、增大字号、强调色
        m_pRichEdit->SetSel(0, (int32_t)title.size());
        fontInfo.m_bBold = true;
        fontInfo.m_fontSize = Dpi().GetScaleInt(16);
        m_pRichEdit->SetFontInfo(fontInfo);
        m_pRichEdit->SetSelectionTextColor(_T("blue"));

        //快捷键标题：加粗（其余属性保持基础字体）
        fontInfo = baseFont;
        m_pRichEdit->SetSel((int32_t)posShortcutsTitle, (int32_t)(posShortcutsTitle + shortcutsTitle.size()));
        fontInfo.m_bBold = true;
        m_pRichEdit->SetFontInfo(fontInfo);

        //行间距示例标题：加粗（其余属性保持基础字体）
        fontInfo = baseFont;
        m_pRichEdit->SetSel((int32_t)posSpacingTitle, (int32_t)(posSpacingTitle + spacingTitle.size()));
        fontInfo.m_bBold = true;
        m_pRichEdit->SetFontInfo(fontInfo);

        //行间距示例正文：斜体（其余属性保持基础字体）
        fontInfo = baseFont;
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
        //先注册关闭消息，避免窗口创建后立刻关闭时收不到通知
        m_pFindForm->AttachWindowCloseMsg([this](const ui::EventArgs& /*args*/) {
                m_pFindForm = nullptr;
                return true;
            });
        ui::WindowCreateParam createParam;
        createParam.m_dwStyle = ui::kWS_POPUP;
        createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
        createParam.m_windowTitle = _T("FindForm");
        createParam.m_bCenterWindow = true;
        m_pFindForm->CreateWnd(this, createParam);
        m_pFindForm->ShowWindow(ui::kSW_SHOW);
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
        //先注册关闭消息，避免窗口创建后立刻关闭时收不到通知
        m_pReplaceForm->AttachWindowCloseMsg([this](const ui::EventArgs& /*args*/) {
                m_pReplaceForm = nullptr;
                return true;
            });
        ui::WindowCreateParam createParam;
        createParam.m_dwStyle = ui::kWS_POPUP;
        createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
        createParam.m_windowTitle = _T("ReplaceForm");
        createParam.m_bCenterWindow = true;
        m_pReplaceForm->CreateWnd(this, createParam);
        m_pReplaceForm->ShowWindow(ui::kSW_SHOW);
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

bool MainForm::FindRichText(const RichEditFindOptions& options, ui::Window* pWndDialog)
{
    bool bFound = m_findReplace.FindRichText(options, pWndDialog);
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

bool MainForm::ReplaceRichText(const RichEditFindOptions& options, ui::Window* pWndDialog)
{
    const bool bHandled = m_findReplace.ReplaceRichText(options, pWndDialog);
    const int32_t nReplaceCount = m_findReplace.GetLastReplaceCount();
    if (nReplaceCount > 0) {
        if (m_pRichEdit != nullptr) {
            m_pRichEdit->SetModify(true);
            UpdateSaveStatus();
            UpdateStatusBar();
        }
        ShowStatusMessage(ui::StringUtil::Printf(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_REPLACE_DONE")).c_str(), (int32_t)nReplaceCount));
    }
    else if (bHandled) {
        //没有替换，但已找到并选中下一处匹配：只刷新界面，不提示未找到
        m_statusMessage.clear();
        UpdateStatusBar();
    }
    else {
        ShowStatusMessage(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_NOT_FOUND")));
    }
    UpdateEditButtonStatus();
    return bHandled;
}

bool MainForm::ReplaceAllRichText(const RichEditFindOptions& options)
{
    const bool bReplaced = m_findReplace.ReplaceAllRichText(options);
    if (bReplaced) {
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
    return bReplaced;
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

void MainForm::GetFindMatchInfo(int32_t& nIndex, int32_t& nTotal) const
{
    m_findReplace.GetMatchInfo(nIndex, nTotal);
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
