#include "render_form.h"
#include "RenderTest1.h"
#include "RenderTest2.h"
#include "RenderTest3.h"

const std::wstring RenderForm::kClassName = L"render";

RenderForm::RenderForm()
{
}


RenderForm::~RenderForm()
{
}

std::wstring RenderForm::GetSkinFolder()
{
	return L"render";
}

std::wstring RenderForm::GetSkinFile()
{
	return L"render.xml";
}

std::wstring RenderForm::GetWindowClassName() const
{
	return kClassName;
}

void RenderForm::OnInitWindow()
{
	TestPropertyGrid();
}

LRESULT RenderForm::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	PostQuitMessage(0L);
	return __super::OnClose(uMsg, wParam, lParam, bHandled);
}

ui::Control* RenderForm::CreateControl(const std::wstring& strClass)
{
	if (strClass == L"RenderTest1") {
		return new ui::RenderTest1;
	}
	else if (strClass == L"RenderTest2") {
		return new ui::RenderTest2;
	}
	if (strClass == L"RenderTest3") {
		return new ui::RenderTest3;
	}
	return nullptr;
}

void RenderForm::TestPropertyGrid()
{
	ui::PropertyGrid* pPropertyGrid = dynamic_cast<ui::PropertyGrid*>(FindControl(L"property_grid_test"));
	if (pPropertyGrid == nullptr) {
		return;
	}
	pPropertyGrid->SetEnableHeaderCtrl(true, L"Property", L"Value");
	pPropertyGrid->SetEnableDescriptionArea(true);

	ui::PropertyGridGroup* pGroup = nullptr;
	
	pGroup = pPropertyGrid->AddGroup(L"分组一", L"<b>分组一</b><br/>描述信息");
	pPropertyGrid->AddProperty(pGroup, L"属性1", L"值1", L"<b>属性1</b><br/>属性1的描述信息");

	pGroup = pPropertyGrid->AddGroup(L"分组二", L"<b>分组二</b><br/>描述信息");
	pPropertyGrid->AddProperty(pGroup, L"属性2", L"值2", L"<b>属性2</b><br/>属性2的描述信息");

	pGroup = pPropertyGrid->AddGroup(L"分组三", L"<b>分组三</b><br/>描述信息");
	pPropertyGrid->AddProperty(pGroup, L"属性3", L"值3", L"<b>属性3</b><br/>属性3的描述信息");
}