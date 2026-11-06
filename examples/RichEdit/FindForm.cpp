#include "FindForm.h"
#include "MainForm.h"

FindReplaceFormBase::FindReplaceFormBase(MainForm* pMainForm):
    m_pMainForm(pMainForm),
    m_pDirectionDown(nullptr),
    m_pDirectionUp(nullptr),
    m_pCaseSensitive(nullptr),
    m_pMatchWholeWord(nullptr),
    m_pFindText(nullptr),
    m_pFindResult(nullptr)
{
}

FindReplaceFormBase::~FindReplaceFormBase()
{
}

DString FindReplaceFormBase::GetSkinFolder()
{
    return _T("rich_edit");
}

LRESULT FindReplaceFormBase::OnKeyDownMsg(ui::VirtualKeyCode vkCode, uint32_t modifierKey, const ui::NativeMsg& nativeMsg, bool& bHandled)
{
    if (vkCode == ui::kVK_ESCAPE) {
        CloseWnd();
        bHandled = true;
        return 0;
    }
    if (vkCode == ui::kVK_F3) {
        OnFindNext();
        bHandled = true;
        return 0;
    }
    return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

void FindReplaceFormBase::InitFindBase()
{
    m_pFindText = dynamic_cast<ui::RichEdit*>(FindControl(_T("btn_find_text")));
    m_pDirectionDown = dynamic_cast<ui::Option*>(FindControl(_T("option_direction_down")));
    m_pDirectionUp = dynamic_cast<ui::Option*>(FindControl(_T("option_direction_up")));
    m_pCaseSensitive = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_box_case_sensitive")));
    m_pMatchWholeWord = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_box_match_whole_word")));
    m_pFindResult = dynamic_cast<ui::Label*>(FindControl(_T("find_result_label")));
    if (m_pFindResult != nullptr) {
        m_pFindResult->SetText(DString());
    }
    ASSERT(m_pFindText != nullptr);
    ASSERT(m_pDirectionDown != nullptr);
    ASSERT(m_pDirectionUp != nullptr);
    ASSERT(m_pCaseSensitive != nullptr);
    ASSERT(m_pMatchWholeWord != nullptr);

    RichEditFindOptions options;
    if (m_pMainForm != nullptr) {
        options = m_pMainForm->GetFindOptions();
    }
    if (m_pDirectionDown != nullptr) {
        m_pDirectionDown->Selected(options.bFindDown, false);
    }
    if (m_pDirectionUp != nullptr) {
        m_pDirectionUp->Selected(!options.bFindDown, false);
    }
    if (m_pCaseSensitive != nullptr) {
        m_pCaseSensitive->Selected(options.bMatchCase, false);
    }
    if (m_pMatchWholeWord != nullptr) {
        m_pMatchWholeWord->Selected(options.bMatchWholeWord, false);
    }

    if (m_pFindText != nullptr) {
        m_pFindText->SetFocus();

        ui::RichEdit* pRichEdit = nullptr;
        if (m_pMainForm != nullptr) {
            pRichEdit = m_pMainForm->GetRichEdit();
        }
        DString findText;
        if (pRichEdit != nullptr) {
            findText = pRichEdit->GetSelText();
        }
        if (findText.empty()) {
            findText = options.findText;
        }
        if (!findText.empty()) {
            m_pFindText->SetText(findText);
            m_pFindText->SetSelAll();
        }
        m_pFindText->AttachReturn([this](const ui::EventArgs& /*args*/) {
            OnFindNext();
            return true;
            });
        m_pFindText->AttachEsc([this](const ui::EventArgs& /*args*/) {
            CloseWnd();
            return true;
            });
    }
    BindCancelButton();
}

RichEditFindOptions FindReplaceFormBase::GetFindOptionsFromControls() const
{
    RichEditFindOptions options;
    if (m_pMainForm != nullptr) {
        options = m_pMainForm->GetFindOptions();
    }
    if (m_pFindText != nullptr) {
        options.findText = m_pFindText->GetText();
    }
    if (m_pDirectionDown != nullptr) {
        options.bFindDown = m_pDirectionDown->IsSelected();
    }
    if (m_pCaseSensitive != nullptr) {
        options.bMatchCase = m_pCaseSensitive->IsSelected();
    }
    if (m_pMatchWholeWord != nullptr) {
        options.bMatchWholeWord = m_pMatchWholeWord->IsSelected();
    }
    return options;
}

void FindReplaceFormBase::SaveFindOptionsFromControls(bool bUpdateReplaceText, const DString& replaceText)
{
    if (m_pMainForm == nullptr) {
        return;
    }
    RichEditFindOptions options = GetFindOptionsFromControls();
    if (bUpdateReplaceText) {
        options.replaceText = replaceText;
    }
    m_pMainForm->SetFindOptions(options);
}

void FindReplaceFormBase::OnFindNext()
{
    if (m_pFindText == nullptr) {
        return;
    }
    DString findText = m_pFindText->GetText();
    if (findText.empty()) {
        return;
    }
    RichEditFindOptions options = GetFindOptionsFromControls();
    SaveFindOptionsFromControls(false, DString());
    bool bFound = false;
    if (m_pMainForm != nullptr) {
        bFound = m_pMainForm->FindRichText(options.findText, options.bFindDown, options.bMatchCase, options.bMatchWholeWord, this);
    }
    UpdateFindResultLabel(bFound);
}

void FindReplaceFormBase::UpdateFindResultLabel(bool bFound)
{
    if (m_pFindResult == nullptr) {
        return;
    }
    if (!bFound) {
        m_pFindResult->SetText(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_NOT_FOUND")));
        return;
    }
    int32_t nIndex = 0;
    int32_t nTotal = 0;
    if (m_pMainForm != nullptr) {
        m_pMainForm->GetFindMatchInfo(nIndex, nTotal);
    }
    if (nTotal <= 0) {
        m_pFindResult->SetText(ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_NOT_FOUND")));
    }
    else if (nIndex > 0) {
        //显示格式：第 2 处，共 8 处
        m_pFindResult->SetText(ui::StringUtil::Printf(
            ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_RESULT_INDEX")).c_str(), nIndex, nTotal));
    }
    else {
        //显示格式：共 8 处
        m_pFindResult->SetText(ui::StringUtil::Printf(
            ui::GlobalManager::GetTextById(_T("STRID_RICHEDIT_FIND_RESULT_TOTAL")).c_str(), nTotal));
    }
}

void FindReplaceFormBase::BindCancelButton()
{
    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_cancel")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                CloseWnd();
            }
            return true;
            });
    }
}

FindForm::FindForm(MainForm* pMainForm):
    BaseClass(pMainForm)
{
}

FindForm::~FindForm()
{
}

DString FindForm::GetSkinFile()
{
    return _T("find.xml");
}

void FindForm::OnInitWindow()
{
    InitFindBase();

    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_find_next")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
            if (args.GetSender() == pButton) {
                OnFindNext();
            }
            return true;
            });
    }
}
