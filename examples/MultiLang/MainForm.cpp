#include "MainForm.h"

const DString MainForm::kClassName = _T("MultiLang");

MainForm::MainForm()
{
}


MainForm::~MainForm()
{
}

DString MainForm::GetSkinFolder()
{
    return _T("MultiLang");
}

DString MainForm::GetSkinFile()
{
    return _T("MultiLang.xml");
}

DString MainForm::GetWindowClassName() const
{
    return kClassName;
}

void MainForm::OnInitWindow()
{
    //窗口初始化完成，可以进行本Form的初始化

    /* Show select language menu */
    ui::Button* select = static_cast<ui::Button*>(FindControl(_T("language")));
    ASSERT(select != nullptr);
    if (select == nullptr) {
        return;
    }
    select->AttachClick([this](const ui::EventArgs& args) {
        ui::UiRect rect = args.pSender->GetPos();
        ui::UiPoint point;
        point.x = rect.left;
        point.y = rect.bottom;
        ClientToScreen(point);

        ShowPopupMenu(point);
        return true;
        });
}

void MainForm::OnCloseWindow()
{
    //关闭窗口后，退出主线程的消息循环，关闭程序
    PostQuitMessage(0L);
}

void MainForm::ShowPopupMenu(const ui::UiPoint& point)
{
    ui::Menu* menu = new ui::Menu(GetHWND());//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    //设置菜单xml所在的目录
    menu->SetSkinFolder(GetResourcePath());
    DString xml(_T("lang_menu.xml"));
    menu->ShowMenu(xml, point);

    //当前语言文件
    DString currentLangFileName = ui::GlobalManager::Instance().GetLanguageFileName();

    //可用语言文件列表和显示名称
    std::vector<std::pair<DString, DString>> languageList;
    ui::GlobalManager::Instance().GetLanguageList(languageList);
    if (languageList.empty()) {
        languageList.push_back({ currentLangFileName , _T("")});
    }

    //动态添加菜单项
    for (auto& lang : languageList) {
        const DString fileName = lang.first;
        DString& displayName = lang.second;

        ui::MenuItem* pMenuItem = new ui::MenuItem(this);
        pMenuItem->SetClass(_T("menu_element"));
        ui::CheckBox* pCheckBox = new ui::CheckBox(this);
        pCheckBox->SetClass(_T("menu_checkbox"));
        pCheckBox->SetAttribute(_T("margin"), _T("0,5,0,10"));
        pCheckBox->SetText(!displayName.empty() ? displayName : fileName);
        pMenuItem->AddItem(pCheckBox);
        menu->AddMenuItem(pMenuItem);

        if (ui::StringUtil::IsEqualNoCase(fileName, currentLangFileName)) {
            pCheckBox->Selected(true);
        }

        //挂载选择语言事件
        pMenuItem->AttachClick([fileName](const ui::EventArgs& args) {
            //切换语言
            ui::GlobalManager& globalManager = ui::GlobalManager::Instance();
            if (globalManager.GetLanguageFileName() != fileName) {
                globalManager.ReloadLanguage(_T(""), fileName, true);
            }            
            return true;
            });
    }
}