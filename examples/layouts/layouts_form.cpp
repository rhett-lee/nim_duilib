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
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("layouts"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
}
