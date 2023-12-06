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
	CheckPropertyGridResult();
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
	
	pGroup = pPropertyGrid->AddGroup(L"分组一", L"描述信息一", (size_t)this);
	ASSERT(pGroup->GetGroupData() == (size_t)this);
	auto p = pPropertyGrid->AddTextProperty(pGroup, L"属性1", L"值1", L"属性1的描述信息", (size_t)this);
	ASSERT(p->GetPropertyData() == (size_t)this);

	pGroup = pPropertyGrid->AddGroup(L"分组二", L"描述信息二");
	pPropertyGrid->AddTextProperty(pGroup, L"属性2", L"值2", L"属性2的描述信息");

	pGroup = pPropertyGrid->AddGroup(L"分组三", L"描述信息三");
	auto p0 = pPropertyGrid->AddTextProperty(pGroup, L"属性3-0(文本)", L"文本值3-0", L"属性3的描述信息");
	auto p1 = pPropertyGrid->AddTextProperty(pGroup, L"属性3-1(数字)", L"3", L"属性3的描述信息，带有Spin控件的数字");
	p1->SetEnableSpin(true, -10, 10);

	auto p2 = pPropertyGrid->AddTextProperty(pGroup, L"属性3-2(只读)", L"值3-2", L"属性3的描述信息");
	p2->SetReadOnly(true);

	auto p3 = pPropertyGrid->AddTextProperty(pGroup, L"属性3-3(密码)", L"值3-3", L"属性3的描述信息");
	p3->SetPassword(true);
}

void RenderForm::CheckPropertyGridResult()
{
	ui::PropertyGrid* pPropertyGrid = dynamic_cast<ui::PropertyGrid*>(FindControl(L"property_grid_test"));
	if (pPropertyGrid == nullptr) {
		return;
	}

	struct PropertyData
	{
		std::wstring name;
		std::wstring oldValue;
		std::wstring newValue;
	};
	typedef std::vector<PropertyData> PropertyDataList;
	std::map<std::wstring, PropertyDataList> propertyMap;

	std::vector<ui::PropertyGridProperty*> properties;
	std::vector<ui::PropertyGridGroup*> groups;
	pPropertyGrid->GetGroups(groups);
	for (auto pGroup : groups) {
		if (pGroup != nullptr) {
			PropertyDataList& dataList = propertyMap[pGroup->GetGroupName()];
			pGroup->GetProperties(properties);
			for (auto pProperty : properties) {
				if (pProperty != nullptr) {
					PropertyData data;
					data.name = pProperty->GetPropertyName();
					data.oldValue = pProperty->GetPropertyValue();
					data.newValue = pProperty->GetPropertyNewValue();
					dataList.push_back(data);
				}
			}
		}
	}
	propertyMap.clear();
}