#include "ReplaceForm.h"
#include "MainForm.h"

ReplaceForm::ReplaceForm(MainForm* pMainForm):
    BaseClass(pMainForm),
    m_pReplaceText(nullptr)
{
}

ReplaceForm::~ReplaceForm()
{
}

DString ReplaceForm::GetSkinFile()
{
    return _T("replace.xml");
}

void ReplaceForm::OnInitWindow()
{
    InitFindBase();

    m_pReplaceText = dynamic_cast<ui::RichEdit*>(FindControl(_T("btn_replace_text")));
    ASSERT(m_pReplaceText != nullptr);
    if (m_pReplaceText != nullptr) {
        if (m_pMainForm != nullptr) {
            const RichEditFindOptions& options = m_pMainForm->GetFindOptions();
            if (!options.replaceText.empty()) {
                m_pReplaceText->SetText(options.replaceText);
                m_pReplaceText->SetSelAll();
            }
        }
        m_pReplaceText->AttachReturn([this](const ui::EventArgs& /*args*/) {
            OnReplace();
            return true;
            });
        m_pReplaceText->AttachEsc([this](const ui::EventArgs& /*args*/) {
            CloseWnd();
            return true;
            });
    }

    ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("btn_find_next")));
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

    RichEditFindOptions options = GetFindOptionsFromControls();
    SaveFindOptionsFromControls(true, replaceText);
    if (m_pMainForm != nullptr) {
        m_pMainForm->ReplaceRichText(findText, replaceText, options.bFindDown, options.bMatchCase, options.bMatchWholeWord, this);
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

    RichEditFindOptions options = GetFindOptionsFromControls();
    SaveFindOptionsFromControls(true, replaceText);
    if (m_pMainForm != nullptr) {
        m_pMainForm->ReplaceAllRichText(findText, replaceText, options.bFindDown, options.bMatchCase, options.bMatchWholeWord, this);
    }
}
