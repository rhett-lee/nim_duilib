#include "RichEditForm.h"

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
            UpdateEventText(ui::StringUtil::Printf(_T("密码框中输入了 %d 个字符"),
                                                   (int32_t)pPassword->GetText().length()));
            return true;
            });
    }

    //多行编辑：设置示例文本，并回显字符数
    ui::RichEdit* pRichEdit = dynamic_cast<ui::RichEdit*>(FindControl(_T("rich_edit_multi")));
    if (pRichEdit != nullptr) {
        pRichEdit->SetText(_T("nim_duilib RichEdit 示例文本。\r\n")
                           _T("支持多行编辑、滚动条、右键菜单与文本拖放。\r\n")
                           _T("可以在此输入任意内容，下方会实时显示字符数。"));
        pRichEdit->AttachTextChanged([this, pRichEdit](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(_T("RichEdit 当前字符数：%d"),
                                                   (int32_t)pRichEdit->GetText().length()));
            return true;
            });
    }

    //RichEdit2：DirectWrite 渲染，示例文本
    ui::RichEdit2* pRichEdit2 = dynamic_cast<ui::RichEdit2*>(FindControl(_T("rich_edit2")));
    if (pRichEdit2 != nullptr) {
        pRichEdit2->SetText(_T("RichEdit2 使用 DirectWrite 渲染文本。\r\n")
                            _T("适合需要更好字体排版效果的场景。"));
        pRichEdit2->AttachTextChanged([this, pRichEdit2](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(_T("RichEdit2 当前字符数：%d"),
                                                   (int32_t)pRichEdit2->GetText().length()));
            return true;
            });
    }

    //单行编辑框
    ui::RichEdit* pSingle = dynamic_cast<ui::RichEdit*>(FindControl(_T("rich_edit_single")));
    if (pSingle != nullptr) {
        pSingle->AttachTextChanged([this, pSingle](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(_T("单行输入：%s"), pSingle->GetText().c_str()));
            return true;
            });
    }
}

void RichEditForm::UpdateEventText(const DString& text)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("rich_edit_event_label")));
    if (pLabel != nullptr) {
        pLabel->SetText(text);
    }
}
