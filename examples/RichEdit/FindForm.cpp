#include "FindForm.h"
#include "MainForm.h"

const LPCTSTR FindForm::kClassName = _T("FindForm");

FindForm::FindForm(MainForm* pMainForm):
    m_pMainForm(pMainForm),
    m_pDirectionOption(nullptr),
    m_pCaseSensitive(nullptr),
    m_pMatchWholeWord(nullptr),
    m_pFindText(nullptr)
{
}

FindForm::~FindForm()
{
}

DString FindForm::GetSkinFolder()
{
    return _T("rich_edit");
}

DString FindForm::GetSkinFile()
{
    return _T("find.xml");
}

DString FindForm::GetWindowClassName() const
{
    return kClassName;
}

void FindForm::OnInitWindow()
{
    m_pFindText = dynamic_cast<ui::RichEdit*>(FindControl(_T("btn_find_text")));
    m_pDirectionOption = dynamic_cast<ui::Option*>(FindControl(_T("option_direction_down")));
    m_pCaseSensitive = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_box_case_sensitive")));
    m_pMatchWholeWord = dynamic_cast<ui::CheckBox*>(FindControl(_T("check_box_match_whole_word")));
    ASSERT(m_pFindText != nullptr);
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
                if (args.pSender == pButton) {
                    CloseWnd();
                }
                return true;
            });
    }
    pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_find_next")));
    if (pButton != nullptr) {
        pButton->AttachClick([this, pButton](const ui::EventArgs& args) {
                if (args.pSender == pButton) {
                    OnFindNext();
                }
                return true;
            });
    }
}

void FindForm::OnFindNext()
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
        m_pMainForm->FindRichText(findText, bFindDown, bMatchCase, bMatchWholeWord, GetHWND());
    }
}
