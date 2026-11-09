#include "RichEditForm.h"

namespace
{
/** 读取语言资源（代码动态生成的文本必须走语言文件，才能支持中英文切换） */
DString RichEditLang(const DString& strId)
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
}

/** 读取多行语言资源，并把 \n 转换为编辑框使用的 \r\n */
DString RichEditLangLines(const DString& strId)
{
    DString text = RichEditLang(strId);
    ui::StringUtil::ReplaceAll(_T("\n"), _T("\r\n"), text);
    return text;
}
}//namespace

RichEditForm::RichEditForm()
{
}

RichEditForm::~RichEditForm()
{
}

DString RichEditForm::GetSkinFolder()
{
    return _T("controls");
}

DString RichEditForm::GetSkinFile()
{
    return _T("rich_edit.xml");
}

void RichEditForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    //密码模式
    ui::RichEdit* pPassword = dynamic_cast<ui::RichEdit*>(FindControl(_T("rich_edit_password")));
    if (pPassword != nullptr) {
        pPassword->SetPasswordMode(true);
        pPassword->AttachTextChanged([this, pPassword](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(RichEditLang(_T("STRID_RICHEDIT_EVENT_PASSWORD")).c_str(),
                                                   (int32_t)pPassword->GetText().length()));
            return true;
            });
    }

    //多行编辑：设置示例文本（内容超过可视高度，用于展示垂直滚动条），并回显字符数
    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("rich_edit_multi")));
    if (pRichEdit != nullptr) {
        m_strMultiSample = RichEditLangLines(_T("STRID_RICHEDIT_MULTI_SAMPLE"));
        pRichEdit->SetText(m_strMultiSample);
        pRichEdit->AttachTextChanged([this, pRichEdit](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(RichEditLang(_T("STRID_RICHEDIT_EVENT_CHARS")).c_str(),
                                                   (int32_t)pRichEdit->GetText().length()));
            return true;
            });
    }

    //RichEdit2：DirectWrite 渲染，示例文本（同样超出可视高度以展示滚动条）
    ui::RichEdit2* pRichEdit2 = dynamic_cast<ui::RichEdit2*>(FindControl(_T("rich_edit2")));
    if (pRichEdit2 != nullptr) {
        m_strRichEdit2Sample = RichEditLangLines(_T("STRID_RICHEDIT2_SAMPLE"));
        pRichEdit2->SetText(m_strRichEdit2Sample);
        pRichEdit2->AttachTextChanged([this, pRichEdit2](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(RichEditLang(_T("STRID_RICHEDIT_EVENT_CHARS2")).c_str(),
                                                   (int32_t)pRichEdit2->GetText().length()));
            return true;
            });
    }

    //单行编辑框
    ui::RichEdit* pSingle = dynamic_cast<ui::RichEdit*>(FindControl(_T("rich_edit_single")));
    if (pSingle != nullptr) {
        pSingle->AttachTextChanged([this, pSingle](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(RichEditLang(_T("STRID_RICHEDIT_EVENT_SINGLE")).c_str(), pSingle->GetText().c_str()));
            return true;
            });
    }
}

bool RichEditForm::OnLanguageChanged()
{
    const bool bRet = BaseClass::OnLanguageChanged();
    UpdateEventText(RichEditLang(_T("STRID_RICHEDIT_EVENT_DEFAULT")));

    //仅当示例文本未被用户修改时，才替换为新语言的示例文本
    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("rich_edit_multi")));
    if (pRichEdit != nullptr) {
        const DString strSample = RichEditLangLines(_T("STRID_RICHEDIT_MULTI_SAMPLE"));
        if (pRichEdit->GetText() == m_strMultiSample) {
            pRichEdit->SetText(strSample);
        }
        m_strMultiSample = strSample;
    }

    ui::RichEdit2* pRichEdit2 = dynamic_cast<ui::RichEdit2*>(FindControl(_T("rich_edit2")));
    if (pRichEdit2 != nullptr) {
        const DString strSample = RichEditLangLines(_T("STRID_RICHEDIT2_SAMPLE"));
        if (pRichEdit2->GetText() == m_strRichEdit2Sample) {
            pRichEdit2->SetText(strSample);
        }
        m_strRichEdit2Sample = strSample;
    }
    return bRet;
}

void RichEditForm::UpdateEventText(const DString& text)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("rich_edit_event_label")));
    if (pLabel != nullptr) {
        pLabel->SetText(text);
    }
}
