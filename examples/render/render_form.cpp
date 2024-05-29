#include "render_form.h"
#include "RenderTest1.h"
#include "RenderTest2.h"
#include "RenderTest3.h"

const std::wstring RenderForm::kClassName = _T("render");

RenderForm::RenderForm()
{
}


RenderForm::~RenderForm()
{
}

std::wstring RenderForm::GetSkinFolder()
{
    return _T("render");
}

std::wstring RenderForm::GetSkinFile()
{
    return _T("render.xml");
}

std::wstring RenderForm::GetWindowClassName() const
{
    return kClassName;
}

void RenderForm::OnInitWindow()
{
    TestPropertyGrid();
}

void RenderForm::OnCloseWindow()
{
    CheckPropertyGridResult();
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMessage(0L);
}

ui::Control* RenderForm::CreateControl(const std::wstring& strClass)
{
    if (strClass == _T("RenderTest1")) {
        return new ui::RenderTest1(this);
    }
    else if (strClass == _T("RenderTest2")) {
        return new ui::RenderTest2(this);
    }
    if (strClass == _T("RenderTest3")) {
        return new ui::RenderTest3(this);
    }
    return nullptr;
}

void RenderForm::TestPropertyGrid()
{
    ui::PropertyGrid* pPropertyGrid = dynamic_cast<ui::PropertyGrid*>(FindControl(_T("property_grid_test")));
    if (pPropertyGrid == nullptr) {
        return;
    }
    pPropertyGrid->SetEnableHeaderCtrl(true, _T("Property"), _T("Value"));
    pPropertyGrid->SetEnableDescriptionArea(true);

    ui::PropertyGridGroup* pGroup = nullptr;
    
    pGroup = pPropertyGrid->AddGroup(_T("分组一"), _T("描述信息一"), (size_t)this);
    ASSERT(pGroup->GetGroupData() == (size_t)this);
    auto p = pPropertyGrid->AddTextProperty(pGroup, _T("属性1"), _T("值1"), _T("属性1的描述信息"), (size_t)this);
    ASSERT(p->GetPropertyData() == (size_t)this);
    auto p00 = pPropertyGrid->AddTextProperty(pGroup, _T("属性2"), _T("值2"), _T("属性2的描述信息：Disable"));
    p00->SetEnabled(false);

    pGroup = pPropertyGrid->AddGroup(_T("分组二"), _T("描述信息二"));
    pPropertyGrid->AddTextProperty(pGroup, _T("属性2"), _T("值2"), _T("属性2的描述信息"));

    pGroup = pPropertyGrid->AddGroup(_T("分组三"), _T("描述信息三"));
    auto p0 = pPropertyGrid->AddTextProperty(pGroup, _T("属性3-0(文本)"), _T("文本值3-0"), _T("属性3的描述信息"));
    auto p1 = pPropertyGrid->AddTextProperty(pGroup, _T("属性3-1(数字)"), _T("3"), _T("属性3的描述信息，带有Spin控件的数字"));
    p1->SetEnableSpin(true, -10, 10);

    auto p2 = pPropertyGrid->AddTextProperty(pGroup, _T("属性3-2(只读)"), _T("值3-2"), _T("属性3的描述信息"));
    p2->SetReadOnly(true);

    auto p3 = pPropertyGrid->AddTextProperty(pGroup, _T("属性3-3(密码)"), _T("值3-3"), _T("属性3的描述信息"));
    p3->SetPassword(true);

    pGroup = pPropertyGrid->AddGroup(_T("分组四"), _T("描述信息四"));
    auto p10 = pPropertyGrid->AddComboProperty(pGroup, _T("属性4-1(下拉表)"), _T("文本值4"), _T("属性4的描述信息"));
    p10->AddOption(_T("取值1"));
    p10->AddOption(_T("取值2"));
    p10->AddOption(_T("取值3"));
    p10->SetComboListMode(true);

    auto p11 = pPropertyGrid->AddComboProperty(pGroup, _T("属性4-1(下拉框)"), _T("文本值4"), _T("属性4的描述信息"));
    p11->AddOption(_T("取值1"));
    p11->AddOption(_T("取值2"));
    p11->AddOption(_T("取值3"));
    p11->SetComboListMode(false);//默认

    pGroup = pPropertyGrid->AddGroup(_T("分组五"), _T("描述信息五：字体"));
    auto p20 = pPropertyGrid->AddFontProperty(pGroup, _T("字体"), _T("宋体"), _T("描述信息：设置字体名称"));
    auto p21 = pPropertyGrid->AddFontSizeProperty(pGroup, _T("字号"), _T("五号"), _T("描述信息：设置字体大小"));

    auto s000 = p21->GetPropertyNewValue();
    auto s001 = p21->GetFontSize();
    auto s002 = p21->GetDpiFontSize();
    auto s003 = p21->GetFontSize(_T("六号"));
    auto s004 = p21->GetDpiFontSize(_T("六号"));

    auto p22 = pPropertyGrid->AddColorProperty(pGroup, _T("颜色"), _T("Blue"), _T("描述信息：设置字体颜色"));

    pGroup = pPropertyGrid->AddGroup(_T("分组六"), _T("描述信息六：日期时间"));
    pPropertyGrid->AddDateTimeProperty(pGroup, _T("日期"), _T("2023-12-07"), _T("描述信息：设置日期"));
    pPropertyGrid->AddDateTimeProperty(pGroup, _T("日期"), _T("2023/12/07"), _T("描述信息：设置日期"));
    pPropertyGrid->AddDateTimeProperty(pGroup, _T("日期"), _T("2023-12-07"), _T("描述信息：设置日期"), 0,
                                        ui::DateTime::EditFormat::kDateUpDown);
    pPropertyGrid->AddDateTimeProperty(pGroup, _T("日期时间"), _T("2023-12-07 17:30:02"), _T("描述信息：设置日期时间"), 0, 
                                        ui::DateTime::EditFormat::kDateTimeUpDown);
    pPropertyGrid->AddDateTimeProperty(pGroup, _T("日期时间"), _T("2023-12-07 17:30"), _T("描述信息：设置日期时间"), 0,
                                        ui::DateTime::EditFormat::kDateMinuteUpDown);
    pPropertyGrid->AddDateTimeProperty(pGroup, _T("时间"), _T("17:30:02"), _T("描述信息：设置时间"), 0,
                                        ui::DateTime::EditFormat::kTimeUpDown);
    pPropertyGrid->AddDateTimeProperty(pGroup, _T("时间"), _T("17:30"), _T("描述信息：设置时间"), 0,
                                        ui::DateTime::EditFormat::kMinuteUpDown);

    pGroup = pPropertyGrid->AddGroup(_T("分组七"), _T("描述信息七"));
    pPropertyGrid->AddIPAddressProperty(pGroup, _T("IP地址"), _T("192.168.0.1"), _T("描述信息：IP地址"));
    pPropertyGrid->AddHotKeyProperty(pGroup, _T("热键1"), _T("Ctrl+C"), _T("描述信息：热键 HotKey控件1"));
    pPropertyGrid->AddHotKeyProperty(pGroup, _T("热键2"), _T("Ctrl+Shift+C"), _T("描述信息：热键 HotKey控件2"));
    pPropertyGrid->AddHotKeyProperty(pGroup, _T("热键3"), _T("Ctrl+Shift+Alt+C"), _T("描述信息：热键 HotKey控件3"));
    pPropertyGrid->AddHotKeyProperty(pGroup, _T("热键4"), _T("Ctrl+Shift"), _T("描述信息：热键 HotKey控件4"));

    pGroup = pPropertyGrid->AddGroup(_T("分组八"), _T("描述信息八"));
    auto p80 = pPropertyGrid->AddFileProperty(pGroup, _T("文件路径"), _T("C:\\Test-Save.txt"), _T("描述信息：文件路径"), 0,
                                              false, 
                                              { 
                                                  {_T("Text文件"), _T("*.txt")},
                                                  {_T("CSV文件"), _T("*.csv")},
                                                  {_T("INI文件"), _T("*.ini")},
                                                  {_T("所有文件"), _T("*.*")}
                                              }, 
                                              0, _T("txt"));
    auto p81 = pPropertyGrid->AddFileProperty(pGroup, _T("文件路径"), _T("C:\\Test-Open.txt"), _T("描述信息：文件路径"), 0,
                                              true, 
                                              { 
                                                  {_T("Text文件"), _T("*.txt")},
                                                  {_T("CSV文件"), _T("*.csv")},
                                                  {_T("INI文件"), _T("*.ini")},
                                                  {_T("所有文件"), _T("*.*")}
                                              }, 
                                              0, _T("txt"));

    auto p82 = pPropertyGrid->AddDirectoryProperty(pGroup, _T("文件夹"), _T("C:\\Test\\"), _T("描述信息：文件夹"));

    return;
}

void RenderForm::CheckPropertyGridResult()
{
    ui::PropertyGrid* pPropertyGrid = dynamic_cast<ui::PropertyGrid*>(FindControl(_T("property_grid_test")));
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