#include "RichEditForm.h"

namespace
{
/** 读取语言资源（代码动态生成的文本必须走语言文件，才能支持中英文切换） */
DString RichEditLang(const DString& strId)
{
    return ui::GlobalManager::Instance().Lang().GetStringByID(strId);
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
        pRichEdit->SetText(_T("nim_duilib RichEdit 多行示例：\r\n")
                           _T("01. 支持多行编辑、垂直滚动条、右键菜单与文本拖放。\r\n")
                           _T("02. 当文本超过可视高度时，右侧会自动出现滚动条。\r\n")
                           _T("03. 可以拖动滚动条查看全部内容。\r\n")
                           _T("04. 支持 Ctrl+A 全选、Ctrl+C 复制、Ctrl+V 粘贴。\r\n")
                           _T("05. 支持 Ctrl+Z 撤销、Ctrl+Y 重做。\r\n")
                           _T("06. 鼠标滚轮可以上下滚动文本。\r\n")
                           _T("07. 可以在此输入任意内容。\r\n")
                           _T("08. 下方会实时显示字符数。\r\n")
                           _T("09. 滚动条支持自动隐藏与拖动。\r\n")
                           _T("10. 文本可以选择、删除和替换。\r\n")
                           _T("11. 支持将 txt/md 文件拖入编辑框。\r\n")
                           _T("12. 多行模式下按 Enter 会插入换行。\r\n")
                           _T("13. 按 Tab 可以插入制表符。\r\n")
                           _T("14. 字体、颜色和行距由主题统一控制。\r\n")
                           _T("15. 这是第 15 行示例文本。\r\n")
                           _T("16. 这是第 16 行示例文本。\r\n")
                           _T("17. 这是第 17 行示例文本。\r\n")
                           _T("18. 这是第 18 行示例文本。\r\n")
                           _T("19. 这是第 19 行示例文本。\r\n")
                           _T("20. 这是第 20 行示例文本。\r\n")
                           _T("21. 这是第 21 行示例文本。\r\n")
                           _T("22. 这是第 22 行示例文本。\r\n")
                           _T("23. 这是第 23 行示例文本。\r\n")
                           _T("24. 最后一行：滚动到底部后滚动条仍然可用。"));
        pRichEdit->AttachTextChanged([this, pRichEdit](const ui::EventArgs& /*args*/) {
            UpdateEventText(ui::StringUtil::Printf(RichEditLang(_T("STRID_RICHEDIT_EVENT_CHARS")).c_str(),
                                                   (int32_t)pRichEdit->GetText().length()));
            return true;
            });
    }

    //RichEdit2：DirectWrite 渲染，示例文本（同样超出可视高度以展示滚动条）
    ui::RichEdit2* pRichEdit2 = dynamic_cast<ui::RichEdit2*>(FindControl(_T("rich_edit2")));
    if (pRichEdit2 != nullptr) {
        pRichEdit2->SetText(_T("RichEdit2 使用 DirectWrite 渲染文本：\r\n")
                            _T("01. 适合需要更好字体排版效果的场景。\r\n")
                            _T("02. 文本超过可视高度时显示垂直滚动条。\r\n")
                            _T("03. 支持鼠标滚轮滚动。\r\n")
                            _T("04. 支持选择、复制、粘贴与撤销。\r\n")
                            _T("05. 支持右键菜单。\r\n")
                            _T("06. 支持多行编辑与换行。\r\n")
                            _T("07. 滚动条样式可通过 vscrollbar_class 定制。\r\n")
                            _T("08. 这是第 8 行示例文本。\r\n")
                            _T("09. 这是第 9 行示例文本。\r\n")
                            _T("10. 这是第 10 行示例文本。\r\n")
                            _T("11. 这是第 11 行示例文本。\r\n")
                            _T("12. 这是第 12 行示例文本。\r\n")
                            _T("13. 这是第 13 行示例文本。\r\n")
                            _T("14. 这是第 14 行示例文本。\r\n")
                            _T("15. 这是第 15 行示例文本。\r\n")
                            _T("16. 这是第 16 行示例文本。\r\n")
                            _T("17. 这是第 17 行示例文本。\r\n")
                            _T("18. 最后一行：向下拖动滚动条可查看全部内容。"));
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
    return bRet;
}

void RichEditForm::UpdateEventText(const DString& text)
{
    ui::Label* pLabel = dynamic_cast<ui::Label*>(FindControl(_T("rich_edit_event_label")));
    if (pLabel != nullptr) {
        pLabel->SetText(text);
    }
}
