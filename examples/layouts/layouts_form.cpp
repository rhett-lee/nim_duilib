#include "layouts_form.h"

LayoutsForm::LayoutsForm(const DString& class_name, const DString& theme_directory, const DString& layout_xml):
    theme_directory_(theme_directory),
    layout_xml_(layout_xml)
{
}


LayoutsForm::~LayoutsForm()
{
}

DString LayoutsForm::GetSkinFolder()
{
    return theme_directory_;
}

DString LayoutsForm::GetSkinFile()
{
    return layout_xml_;
}

void LayoutsForm::OnInitWindow()
{

}

void LayoutsForm::ShowCustomWindow(const DString& class_name, const DString& theme_directory, const DString& layout_xml)
{
    LayoutsForm* window = new LayoutsForm(class_name, theme_directory, layout_xml);
    ui::WindowCreateParam createParam;
    createParam.m_dwExStyle = WS_EX_LAYERED;
    createParam.m_className = _T("layouts");
    createParam.m_windowTitle = createParam.m_className;
    window->CreateWnd(nullptr, createParam);
    window->PostQuitMsgWhenClosed(true);
    window->CenterWindow();
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}
