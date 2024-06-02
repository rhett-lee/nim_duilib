#include "layouts_form.h"

LayoutsForm::LayoutsForm(const DString& class_name, const DString& theme_directory, const DString& layout_xml)
    : class_name_(class_name)
    , theme_directory_(theme_directory)
    , layout_xml_(layout_xml)
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

DString LayoutsForm::GetWindowClassName() const
{
    return class_name_;
}

void LayoutsForm::OnInitWindow()
{

}

void LayoutsForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMessage(0L);
}

void LayoutsForm::ShowCustomWindow(const DString& class_name, const DString& theme_directory, const DString& layout_xml)
{
    LayoutsForm* window = new LayoutsForm(class_name, theme_directory, layout_xml);
    window->CreateWnd(nullptr, class_name, UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
    window->CenterWindow();
    window->ShowWindow();
}
