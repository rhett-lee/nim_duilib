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
	auto p00 = pPropertyGrid->AddTextProperty(pGroup, L"属性2", L"值2", L"属性2的描述信息：Disable");
	p00->SetEnabled(false);

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

	pGroup = pPropertyGrid->AddGroup(L"分组四", L"描述信息四");
	auto p10 = pPropertyGrid->AddComboProperty(pGroup, L"属性4-1(下拉表)", L"文本值4", L"属性4的描述信息");
	p10->AddOption(L"取值1");
	p10->AddOption(L"取值2");
	p10->AddOption(L"取值3");
	p10->SetComboListMode(true);

	auto p11 = pPropertyGrid->AddComboProperty(pGroup, L"属性4-1(下拉框)", L"文本值4", L"属性4的描述信息");
	p11->AddOption(L"取值1");
	p11->AddOption(L"取值2");
	p11->AddOption(L"取值3");
	p11->SetComboListMode(false);//默认

	pGroup = pPropertyGrid->AddGroup(L"分组五", L"描述信息五：字体");
	auto p20 = pPropertyGrid->AddFontProperty(pGroup, L"字体", L"宋体", L"描述信息：设置字体名称");
	auto p21 = pPropertyGrid->AddFontSizeProperty(pGroup, L"字号", L"五号", L"描述信息：设置字体大小");

	auto s000 = p21->GetPropertyNewValue();
	auto s001 = p21->GetFontSize();
	auto s002 = p21->GetDpiFontSize();
	auto s003 = p21->GetFontSize(L"六号");
	auto s004 = p21->GetDpiFontSize(L"六号");

	auto p22 = pPropertyGrid->AddColorProperty(pGroup, L"颜色", L"Blue", L"描述信息：设置字体颜色");

	pGroup = pPropertyGrid->AddGroup(L"分组六", L"描述信息六：日期时间");
	pPropertyGrid->AddDateTimeProperty(pGroup, L"日期", L"2023-12-07", L"描述信息：设置日期");
	pPropertyGrid->AddDateTimeProperty(pGroup, L"日期", L"2023/12/07", L"描述信息：设置日期");
	pPropertyGrid->AddDateTimeProperty(pGroup, L"日期", L"2023-12-07", L"描述信息：设置日期", 0,
										ui::DateTime::EditFormat::kDateUpDown);
	pPropertyGrid->AddDateTimeProperty(pGroup, L"日期时间", L"2023-12-07 17:30:02", L"描述信息：设置日期时间", 0, 
										ui::DateTime::EditFormat::kDateTimeUpDown);
	pPropertyGrid->AddDateTimeProperty(pGroup, L"日期时间", L"2023-12-07 17:30", L"描述信息：设置日期时间", 0,
										ui::DateTime::EditFormat::kDateMinuteUpDown);
	pPropertyGrid->AddDateTimeProperty(pGroup, L"时间", L"17:30:02", L"描述信息：设置时间", 0,
										ui::DateTime::EditFormat::kTimeUpDown);
	pPropertyGrid->AddDateTimeProperty(pGroup, L"时间", L"17:30", L"描述信息：设置时间", 0,
										ui::DateTime::EditFormat::kMinuteUpDown);

	pGroup = pPropertyGrid->AddGroup(L"分组七", L"描述信息七");
	pPropertyGrid->AddIPAddressProperty(pGroup, L"IP地址", L"192.168.0.1", L"描述信息：IP地址");
	pPropertyGrid->AddHotKeyProperty(pGroup, L"热键1", L"Ctrl+C", L"描述信息：热键 HotKey控件1");
	pPropertyGrid->AddHotKeyProperty(pGroup, L"热键2", L"Ctrl+Shift+C", L"描述信息：热键 HotKey控件2");
	pPropertyGrid->AddHotKeyProperty(pGroup, L"热键3", L"Ctrl+Shift+Alt+C", L"描述信息：热键 HotKey控件3");
	pPropertyGrid->AddHotKeyProperty(pGroup, L"热键4", L"Ctrl+Shift", L"描述信息：热键 HotKey控件4");

	pGroup = pPropertyGrid->AddGroup(L"分组八", L"描述信息八");
	auto p80 = pPropertyGrid->AddFileProperty(pGroup, L"文件路径", L"C:\\Test-Save.txt", L"描述信息：文件路径", 0,
		                                      false, 
											  { 
												  {L"Text文件", L"*.txt"},
												  {L"CSV文件", L"*.csv"},
												  {L"INI文件", L"*.ini"},
												  {L"所有文件", L"*.*"}
		                                      }, 
											  0, L"txt");
	auto p81 = pPropertyGrid->AddFileProperty(pGroup, L"文件路径", L"C:\\Test-Open.txt", L"描述信息：文件路径", 0,
											  true, 
											  { 
												  {L"Text文件", L"*.txt"},
												  {L"CSV文件", L"*.csv"},
												  {L"INI文件", L"*.ini"},
												  {L"所有文件", L"*.*"}
		                                      }, 
											  0, L"txt");

	auto p82 = pPropertyGrid->AddDirectoryProperty(pGroup, L"文件夹", L"C:\\Test\\", L"描述信息：文件夹");

	return;
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