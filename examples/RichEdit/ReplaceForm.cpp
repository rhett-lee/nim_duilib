#include "ReplaceForm.h"
#include "MainForm.h"

ReplaceForm::ReplaceForm(MainForm* pMainForm):
    m_pMainForm(pMainForm),
    m_pDirectionOption(nullptr),
    m_pCaseSensitive(nullptr),
    m_pMatchWholeWord(nullptr),
    m_pFindText(nullptr),
    m_pReplaceText(nullptr)
{
}


ReplaceForm::~ReplaceForm()
{
}

DString ReplaceForm::GetSkinFolder()
{
    return _T("rich_edit");
}

DString ReplaceForm::GetSkinFile()
{
    return _T("replace.xml");
}

void ReplaceForm::OnInitWindow()
{
    m_pFindText = dynamic_cast<ui::RichEdit*>(FindControl(_T("btn_find_text")));
    m_pReplaceText = dynamic_cast<ui::RichEdit*>(FindControl(_T("btn_replace_text")));
    m_pDirectionOption = dynamic_cast<ui::Option*>(FindControl(_T("option_direction_down")));
    m_pCaseSensitive = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_box_case_sensitive")));
    m_pMatchWholeWord = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_box_match_whole_word")));
    ASSERT(m_pFindText != nullptr);
    ASSERT(m_pReplaceText != nullptr);
    ASSERT(m_pDirectionOption != nullptr);
    ASSERT(m_pCaseSensitive != nullptr);
    ASSERT(m_pMatchWholeWord != nullptr);

    if (m_pFindText != nullptr) {
        m_pFindText->SetFocus();

        ui::RichEdit* pRichEdit = nullptr;
        if (m_pMainForm != nullptr) {
            pRichEdit = m_pMainForm->GetRichEdit();
        }
        DString selText;
        if (pRichEdit != nullptr) {
            selText = pRichEdit->GetSelText();
        }
        if (!selText.empty()) {
            m_pFindText->SetText(selText);
            m_pFindText->SetSelAll();
        }
    }

    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_cancel")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
                if (args.GetSender() == pButton) {
                    CloseWnd();
                }
                return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_find_next")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
                if (args.GetSender() == pButton) {
                    OnFindNext();
                }
                return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_replace")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
                if (args.GetSender() == pButton) {
                    OnReplace();
                }
                return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_replace_all")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
                if (args.GetSender() == pButton) {
                    OnReplaceAll();
                }
                return true;
            });
    }
}

void ReplaceForm::OnFindNext()
{
    if (m_pFindText == nullptr) {
        return;
    }
    DString findText = m_pFindText->GetText();
    if (findText.empty()) {
        return;
    }

    bool bFindDown = true;
    if (m_pDirectionOption != nullptr) {
        bFindDown = m_pDirectionOption->IsSelected();
    }

    bool bMatchCase = true;
    if (m_pCaseSensitive != nullptr) {
        bMatchCase = m_pCaseSensitive->IsSelected();
    }

    bool bMatchWholeWord = false;
    if (m_pMatchWholeWord != nullptr) {
        bMatchWholeWord = m_pMatchWholeWord->IsSelected();
    }
    if (m_pMainForm != nullptr) {
        m_pMainForm->FindRichText(findText, bFindDown, bMatchCase, bMatchWholeWord, this);
    }
}

void ReplaceForm::OnReplace()
{
    if (m_pFindText == nullptr) {
        return;
    }
    DString findText = m_pFindText->GetText();
    if (findText.empty()) {
        return;
    }
    if (m_pReplaceText == nullptr) {
        return;
    }
    DString replaceText = m_pReplaceText->GetText();
    if (replaceText.empty()) {
        return;
    }

    bool bFindDown = true;
    if (m_pDirectionOption != nullptr) {
        bFindDown = m_pDirectionOption->IsSelected();
    }

    bool bMatchCase = true;
    if (m_pCaseSensitive != nullptr) {
        bMatchCase = m_pCaseSensitive->IsSelected();
    }

    bool bMatchWholeWord = false;
    if (m_pMatchWholeWord != nullptr) {
        bMatchWholeWord = m_pMatchWholeWord->IsSelected();
    }
    if (m_pMainForm != nullptr) {
        m_pMainForm->ReplaceRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, this);
    }
}

void ReplaceForm::OnReplaceAll()
{
    if (m_pFindText == nullptr) {
        return;
    }
    DString findText = m_pFindText->GetText();
    if (findText.empty()) {
        return;
    }
    if (m_pReplaceText == nullptr) {
        return;
    }
    DString replaceText = m_pReplaceText->GetText();
    if (replaceText.empty()) {
        return;
    }

    bool bFindDown = true;
    if (m_pDirectionOption != nullptr) {
        bFindDown = m_pDirectionOption->IsSelected();
    }

    bool bMatchCase = true;
    if (m_pCaseSensitive != nullptr) {
        bMatchCase = m_pCaseSensitive->IsSelected();
    }

    bool bMatchWholeWord = false;
    if (m_pMatchWholeWord != nullptr) {
        bMatchWholeWord = m_pMatchWholeWord->IsSelected();
    }
    if (m_pMainForm != nullptr) {
        m_pMainForm->ReplaceAllRichText(findText, replaceText, bFindDown, bMatchCase, bMatchWholeWord, this);
    }
}
