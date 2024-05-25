#include "stdafx.h"
#include "layouts_form.h"

LayoutsForm::LayoutsForm(const std::wstring& class_name, const std::wstring& theme_directory, const std::wstring& layout_xml)
	: class_name_(class_name)
	, theme_directory_(theme_directory)
	, layout_xml_(layout_xml)
{
}


LayoutsForm::~LayoutsForm()
{
}

std::wstring LayoutsForm::GetSkinFolder()
{
	return theme_directory_;
}

std::wstring LayoutsForm::GetSkinFile()
{
	return layout_xml_;
}

std::wstring LayoutsForm::GetWindowClassName() const
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

void LayoutsForm::ShowCustomWindow(const std::wstring& class_name, const std::wstring& theme_directory, const std::wstring& layout_xml)
{
	LayoutsForm* window = new LayoutsForm(class_name, theme_directory, layout_xml);
	window->CreateWnd(NULL, class_name.c_str(), UI_WNDSTYLE_FRAME, WS_EX_LAYERED);
	window->CenterWindow();
	window->ShowWindow();
}
