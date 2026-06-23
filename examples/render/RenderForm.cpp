#include "RenderForm.h"
#include "RenderTest1.h"
#include "RenderTest2.h"

RenderForm::RenderForm()
{
}

RenderForm::~RenderForm()
{
}

DString RenderForm::GetSkinFolder()
{
    return _T("render");
}

DString RenderForm::GetSkinFile()
{
    return _T("render.xml");
}

void RenderForm::OnInitWindow()
{
    TestPropertyGrid();

    //MenuBar测试
    ui::MenuBar* pMenuBar = dynamic_cast<ui::MenuBar*>(FindControl(_T("menu_bar_test")));
    if (pMenuBar != nullptr) {
        pMenuBar->AddTopMenu(_T("1"), _T("File"),      _T(""), DString(DUILIB_PUBLIC_RES_DIR) + _T("/menu/rich_edit_menu.xml"));
        pMenuBar->AddTopMenu(_T("2"), _T("Edit"),      _T(""), _T("controls\\menu\\settings_menu.xml"));
        pMenuBar->AddTopMenu(_T("3"), _T("Selection"), _T(""), DString(DUILIB_PUBLIC_RES_DIR) + _T("/menu/rich_edit_menu.xml"));
        pMenuBar->AddTopMenu(_T("4"), _T("View1"),      _T(""), _T("controls/menu/settings_menu.xml"));
        pMenuBar->AddTopMenu(_T("5"), _T("View2"),       _T(""), DString(DUILIB_PUBLIC_RES_DIR) + _T("/menu/rich_edit_menu.xml"));
        pMenuBar->AddTopMenu(_T("6"), _T("  ...  "),   _T(""), DString(DUILIB_PUBLIC_RES_DIR) + _T("/menu/rich_edit_menu.xml"), _T(""), _T("text_padding='8,1,8,7'"));
    }

    //注册菜单命令激活的响应函数
    ui::MenuBarItemActivatedEvent callback = [](const DString& menuItemId,
                                                const DString& menuName, int32_t nMenuLevel,
                                                const DString& itemName, size_t nItemIndex) {
            (void)menuItemId;
            (void)menuName;
            (void)nMenuLevel;
            (void)itemName;
            (void)nItemIndex;
            int ii = 0;
        };
    pMenuBar->AttachMenuBarItemActivated(callback);
}

void RenderForm::OnCloseWindow()
{
    CheckPropertyGridResult();
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMsg(0L);
}

ui::Control* RenderForm::CreateControl(const DString& strClass)
{
    if (strClass == _T("RenderTest1")) {
        return new ui::RenderTest1(this);
    }
    else if (strClass == _T("RenderTest2")) {
        return new ui::RenderTest2(this);
    }
    return nullptr;
}
void RenderForm::TestPropertyGrid()
{
    ui::PropertyGrid* pPropertyGrid = dynamic_cast<ui::PropertyGrid*>(FindControl(_T("property_grid_test")));
    if (pPropertyGrid == nullptr) {
        return;
    }
    // 表头文本
    pPropertyGrid->SetEnableHeaderCtrl(true,
        _T("STRID_RENDER_PROPGRID_PROPERTY"),  // 属性
        _T("STRID_RENDER_PROPGRID_VALUE"),     // 值
        true);    
    pPropertyGrid->SetEnableDescriptionArea(true);

    ui::PropertyGridGroup* pGroup = nullptr;

    // 分组一
    pGroup = pPropertyGrid->AddGroupById(
        _T("STRID_RENDER_PROPGRID_GROUP_1"),   // 分组一
        _T("STRID_RENDER_PROPGRID_DESC_GROUP_1"),  // 描述信息一
        (size_t)this);
    ASSERT(pGroup->GetGroupData() == (size_t)this);
    auto p = pPropertyGrid->AddTextPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_1"),  // 属性1
        _T("STRID_RENDER_PROPGRID_VALUE_1"),     // 值1
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_1"),  // 属性1的描述信息
        (size_t)this);
    ASSERT(p->GetPropertyData() == (size_t)this);
    auto p00 = pPropertyGrid->AddTextPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_2"),  // 属性2
        _T("STRID_RENDER_PROPGRID_VALUE_2"),     // 值2
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_2_DISABLE"));  // 属性2的描述信息：Disable
    p00->SetEnabled(false);

    // 分组二
    pGroup = pPropertyGrid->AddGroupById(
        _T("STRID_RENDER_PROPGRID_GROUP_2"),   // 分组二
        _T("STRID_RENDER_PROPGRID_DESC_GROUP_2"));  // 描述信息二
    pPropertyGrid->AddTextPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_2"),  // 属性2
        _T("STRID_RENDER_PROPGRID_VALUE_2"),     // 值2
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_2"));  // 属性2的描述信息

    // 分组三
    pGroup = pPropertyGrid->AddGroupById(
        _T("STRID_RENDER_PROPGRID_GROUP_3"),   // 分组三
        _T("STRID_RENDER_PROPGRID_DESC_GROUP_3"));  // 描述信息三
    auto p0 = pPropertyGrid->AddTextPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_3_0_TEXT"),  // 属性3-0(文本)
        _T("STRID_RENDER_PROPGRID_VALUE_3_0"),     // 文本值3-0
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_3"));  // 属性3的描述信息
    auto p1 = pPropertyGrid->AddTextPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_3_1_NUMBER"),  // 属性3-1(数字)
        _T("STRID_RENDER_PROPGRID_VALUE_3_1"),     // 3
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_3_SPIN"));  // 属性3的描述信息，带有Spin控件的数字
    p1->SetEnableSpin(true, -10, 10);

    auto p2 = pPropertyGrid->AddTextPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_3_2_READONLY"),  // 属性3-2(只读)
        _T("STRID_RENDER_PROPGRID_VALUE_3_2"),     // 值3-2
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_3"));  // 属性3的描述信息
    p2->SetReadOnly(true);

    auto p3 = pPropertyGrid->AddTextPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_3_3_PASSWORD"),  // 属性3-3(密码)
        _T("STRID_RENDER_PROPGRID_VALUE_3_3"),     // 值3-3
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_3"));  // 属性3的描述信息
    p3->SetPasswordMode(true);

    // 分组四
    pGroup = pPropertyGrid->AddGroupById(
        _T("STRID_RENDER_PROPGRID_GROUP_4"),   // 分组四
        _T("STRID_RENDER_PROPGRID_DESC_GROUP_4"));  // 描述信息四
    auto p10 = pPropertyGrid->AddComboPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_4_1_COMBO_LIST"),  // 属性4-1(下拉表)
        _T("STRID_RENDER_PROPGRID_OPTION_1"),     // 取值1
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_4"));  // 属性4的描述信息
    p10->AddOptionById(_T("STRID_RENDER_PROPGRID_OPTION_1"));  // 取值1
    p10->AddOptionById(_T("STRID_RENDER_PROPGRID_OPTION_2"));  // 取值2
    p10->AddOptionById(_T("STRID_RENDER_PROPGRID_OPTION_3"));  // 取值3
    p10->SetComboListMode(true);
    p10->SetCurSel(0);

    auto p11 = pPropertyGrid->AddComboPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_PROPERTY_4_1_COMBO_BOX"),  // 属性4-1(下拉框)
        _T("STRID_RENDER_PROPGRID_OPTION_1"),     // 取值1
        _T("STRID_RENDER_PROPGRID_DESC_PROPERTY_4"));  // 属性4的描述信息
    p11->AddOptionById(_T("STRID_RENDER_PROPGRID_OPTION_1"));  // 取值1
    p11->AddOptionById(_T("STRID_RENDER_PROPGRID_OPTION_2"));  // 取值2
    p11->AddOptionById(_T("STRID_RENDER_PROPGRID_OPTION_3"));  // 取值3
    p11->SetComboListMode(false);//默认
    p11->SetCurSel(0);

    // 分组五（字体）
    DString fontName = ui::GlobalManager::GetTextById(_T("STRID_RENDER_PROPGRID_FONT_ARIAL"));
    pGroup = pPropertyGrid->AddGroupById(
        _T("STRID_RENDER_PROPGRID_GROUP_5"),   // 分组五
        _T("STRID_RENDER_PROPGRID_DESC_GROUP_5_FONT"));  // 描述信息五：字体
    auto p20 = pPropertyGrid->AddFontPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_FONT"),  // 字体
        fontName,  // Arial(不使用多语言ID)
        _T("STRID_RENDER_PROPGRID_DESC_SET_FONT_NAME"), // 描述信息：设置字体名称
        0, false);  
    auto p21 = pPropertyGrid->AddFontSizePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_FONT_SIZE"),  // 字号
        _T("STRID_RENDER_PROPGRID_FONT_SIZE_5"),  // 五号
        _T("STRID_RENDER_PROPGRID_DESC_SET_FONT_SIZE"));  // 描述信息：设置字体大小

    auto s000 = p21->GetPropertyNewValue();
    auto s001 = p21->GetFontSize();
    auto s002 = p21->GetDpiFontSize();

    auto s003 = p21->GetFontSizeById(_T("STRID_RENDER_PROPGRID_FONT_SIZE_6"));  // 六号
    auto s004 = p21->GetDpiFontSizeById(_T("STRID_RENDER_PROPGRID_FONT_SIZE_6"));  // 六号

    auto p22 = pPropertyGrid->AddColorPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_COLOR"),  // 颜色
        _T("STRID_RENDER_PROPGRID_COLOR_BLUE"),  // Blue
        _T("STRID_RENDER_PROPGRID_DESC_SET_FONT_COLOR"));  // 描述信息：设置字体颜色

    // 分组六（日期时间）
    pGroup = pPropertyGrid->AddGroupById(
        _T("STRID_RENDER_PROPGRID_GROUP_6"),   // 分组六
        _T("STRID_RENDER_PROPGRID_DESC_GROUP_6_DATETIME"));  // 描述信息六：日期时间
    pPropertyGrid->AddDateTimePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_DATE"),  // 日期
        _T("2023-12-07"),
        _T("STRID_RENDER_PROPGRID_DESC_SET_DATE"),
        0, false, ui::DateTime::EditFormat::kDateCalendar);  // 描述信息：设置日期
    pPropertyGrid->AddDateTimePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_DATE"),  // 日期
        _T("2023/12/07"),
        _T("STRID_RENDER_PROPGRID_DESC_SET_DATE"),
        0, false, ui::DateTime::EditFormat::kDateCalendar);  // 描述信息：设置日期
    pPropertyGrid->AddDateTimePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_DATE"),  // 日期
        _T("2023-12-07"),
        _T("STRID_RENDER_PROPGRID_DESC_SET_DATE"),  // 描述信息：设置日期
        0, false, ui::DateTime::EditFormat::kDateUpDown);
    pPropertyGrid->AddDateTimePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_DATE_TIME"),  // 日期时间
        _T("2023-12-07 17:30:02"),
        _T("STRID_RENDER_PROPGRID_DESC_SET_DATE_TIME"),  // 描述信息：设置日期时间
        0, false, ui::DateTime::EditFormat::kDateTimeUpDown);
    pPropertyGrid->AddDateTimePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_DATE_TIME"),  // 日期时间
        _T("2023-12-07 17:30"),
        _T("STRID_RENDER_PROPGRID_DESC_SET_DATE_TIME"),  // 描述信息：设置日期时间
        0, false, ui::DateTime::EditFormat::kDateMinuteUpDown);
    pPropertyGrid->AddDateTimePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_TIME"),  // 时间
        _T("17:30:02"),
        _T("STRID_RENDER_PROPGRID_DESC_SET_TIME"),  // 描述信息：设置时间
        0, false, ui::DateTime::EditFormat::kTimeUpDown);
    pPropertyGrid->AddDateTimePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_TIME"),  // 时间
        _T("17:30"),
        _T("STRID_RENDER_PROPGRID_DESC_SET_TIME"),  // 描述信息：设置时间
        0, false, ui::DateTime::EditFormat::kMinuteUpDown);

    // 分组七（IP/热键）
    pGroup = pPropertyGrid->AddGroupById(
        _T("STRID_RENDER_PROPGRID_GROUP_7"),   // 分组七
        _T("STRID_RENDER_PROPGRID_DESC_GROUP_7"));  // 描述信息七
    pPropertyGrid->AddIPAddressPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_IP_ADDRESS"),  // IP地址
        _T("192.168.0.1"),
        _T("STRID_RENDER_PROPGRID_DESC_IP_ADDRESS"),// 描述信息：IP地址
        0, false);  
    pPropertyGrid->AddHotKeyPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_HOTKEY_1"),  // 热键1
        _T("Ctrl+C"),
        _T("STRID_RENDER_PROPGRID_DESC_HOTKEY_1"), // 描述信息：热键 HotKey控件1
        0, false);  
    pPropertyGrid->AddHotKeyPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_HOTKEY_2"),  // 热键2
        _T("Ctrl+Shift+C"),
        _T("STRID_RENDER_PROPGRID_DESC_HOTKEY_2"), // 描述信息：热键 HotKey控件2
        0, false);  
    pPropertyGrid->AddHotKeyPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_HOTKEY_3"),  // 热键3
        _T("Ctrl+Shift+Alt+C"),
        _T("STRID_RENDER_PROPGRID_DESC_HOTKEY_3"), // 描述信息：热键 HotKey控件3
        0, false);  
    pPropertyGrid->AddHotKeyPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_HOTKEY_4"),  // 热键4
        _T("Ctrl+Shift"),
        _T("STRID_RENDER_PROPGRID_DESC_HOTKEY_4"), // 描述信息：热键 HotKey控件4
        0, false);  

    // 分组八（文件/文件夹）
    const std::vector<ui::FileDialog::FileType> fileTypes = {
            {_T(""), _T("*.txt"), _T("STRID_RENDER_TXT_FILES")},  // 文本文件
            {_T(""), _T("*.csv"), _T("STRID_RENDER_CSV_FILES")},  // CSV文件
            {_T(""), _T("*.ini"), _T("STRID_RENDER_INI_FILES")},  // INI文件
            {_T(""), _T("*.*"), _T("STRID_RENDER_ALL_FILES")}     // 所有文件
    };

    pGroup = pPropertyGrid->AddGroupById(
        _T("STRID_RENDER_PROPGRID_GROUP_8"),   // 分组八
        _T("STRID_RENDER_PROPGRID_DESC_GROUP_8"));  // 描述信息八
    auto p80 = pPropertyGrid->AddFilePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_FILE_PATH"),  // 文件路径
        _T("C:\\Test-Save.txt"),
        _T("STRID_RENDER_PROPGRID_DESC_FILE_PATH"),  // 描述信息：文件路径
        0, false, false, fileTypes, 0, _T("txt"));
    auto p81 = pPropertyGrid->AddFilePropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_FILE_PATH"),  // 文件路径
        _T("C:\\Test-Open.txt"),
        _T("STRID_RENDER_PROPGRID_DESC_FILE_PATH"),  // 描述信息：文件路径
        0, false, true, fileTypes, 0, _T("txt"));

    auto p82 = pPropertyGrid->AddDirectoryPropertyById(pGroup,
        _T("STRID_RENDER_PROPGRID_FOLDER"),  // 文件夹
        _T("C:\\Test\\"),
        _T("STRID_RENDER_PROPGRID_DESC_FOLDER"), 0, false);  // 描述信息：文件夹
}

void RenderForm::CheckPropertyGridResult()
{
    ui::PropertyGrid* pPropertyGrid = dynamic_cast<ui::PropertyGrid*>(FindControl(_T("property_grid_test")));
    if (pPropertyGrid == nullptr) {
        return;
    }

    struct PropertyData
    {
        DString name;
        DString oldValue;
        DString newValue;
    };
    typedef std::vector<PropertyData> PropertyDataList;
    std::map<DString, PropertyDataList> propertyMap;

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
