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

RichEditFindOptions ReplaceForm::CollectSearchOptions() const
{
    RichEditFindOptions options = BaseClass::CollectSearchOptions();
    if (m_pReplaceText != nullptr) {
        options.replaceText = m_pReplaceText->GetText();
    }
    return options;
}

void ReplaceForm::OnReplace()
{
    if (m_pReplaceText == nullptr) {
        return;
    }
    const RichEditFindOptions options = CollectSearchOptions();
    if (options.findText.empty()) {
        return;
    }
    SaveFindOptions(options);
    bool bReplaced = false;
    if (m_pMainForm != nullptr) {
        bReplaced = m_pMainForm->ReplaceRichText(options, this);
    }
    UpdateFindResultLabel(bReplaced);
}

void ReplaceForm::OnReplaceAll()
{
    if (m_pReplaceText == nullptr) {
        return;
    }
    const RichEditFindOptions options = CollectSearchOptions();
    if (options.findText.empty()) {
        return;
    }
    SaveFindOptions(options);
    bool bReplaced = false;
    if (m_pMainForm != nullptr) {
        bReplaced = m_pMainForm->ReplaceAllRichText(options);
    }
    if (bReplaced) {
        //全部替换后已无匹配，清空计数提示
        ClearFindResultLabel();
    }
    else {
        UpdateFindResultLabel(false);
    }
}
