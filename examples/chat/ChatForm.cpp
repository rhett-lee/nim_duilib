#include "ChatForm.h"

ChatForm::ChatForm(const DString& class_name, const DString& theme_directory, const DString& layout_xml):
    m_themeDirectory(theme_directory),
    m_layoutXml(layout_xml)
{
}


ChatForm::~ChatForm()
{
}

DString ChatForm::GetSkinFolder()
{
    return m_themeDirectory;
}

DString ChatForm::GetSkinFile()
{
    return m_layoutXml;
}

void ChatForm::OnInitWindow()
{

}

void ChatForm::ShowCustomWindow(const DString& class_name, const DString& theme_directory, const DString& layout_xml)
{
    ChatForm* window = new ChatForm(class_name, theme_directory, layout_xml);
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("chat"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}
