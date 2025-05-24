## 菜单
菜单的实现是基于窗口([WindowImplBase](../duilib/Utils/WinImplBase.h))实现的，主要由[Menu](../duilib/Control/Menu.h)和[MenuItem](../duilib/Control/Menu.h)两个类组成。
1. 菜单的效果预览
这个预览是`examples/controls`示例程序中的菜单。
<img src="./Images/Menu.png"/>
2. 菜单的实现包含了系统菜单中的基本功能：支持图标、勾选框、多级菜单、菜单项分割条、动态修改菜单项、菜单中插入非菜单控件等。
3. `settings_menu.xml`中的主要内容：
```xml
<?xml version="1.0" encoding="utf-8"?>
<Window shadow_type="menu_round" shadow_attached="true" layered_window="true" >
    <MenuListBox class="menu" name="main_menu">
        <!-- 一级菜单 -->
        <MenuItem class="menu_element" name="first" width="180">
          <Button name="button_01" width="auto" height="auto" bkimage="menu_settings.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
          <Label class="menu_text" text="一级菜单项 1" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
    
        <MenuItem class="menu_element" name="second" width="180">
          <Button name="button_02" width="auto" height="auto" bkimage="menu_proxy.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
          <Label class="menu_text" text="一级菜单项 2" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        
        <!-- 在菜单中插入普通控件，实现特定功能 -->
        <HBox class="menu_split_box" height="36">
            <Label class="menu_text" text="音量" textpadding="0,0,6,0" mouse_enabled="false" keyboard_enabled="false"/>
            <Control width="auto" height="auto" bkimage="menu_speaker.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
            <Slider class="slider_green" value="70" tooltip_text="ui::Slider"/>
        </HBox>
        
        <!-- 菜单项之间的分割线 -->
        <Box class="menu_split_box">
            <Control class="menu_split_line" />
        </Box>
        
        <MenuItem class="menu_element" name="third" width="180">
            <Button name="button_03" width="auto" height="auto" bkimage="menu_logs.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
            <Label class="menu_text" text="一级菜单项 3" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        
        <MenuItem class="menu_element" name="fourth" width="180">
            <Button name="button_04" width="auto" height="auto" bkimage="menu_tree.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
            <Label class="menu_text" text="二级菜单" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
            <!-- 二级菜单：第一种支持的形式（保持旧版本的兼容性） -->
            <MenuItem class="menu_element" name="sub_menu0" width="180">
                <Button name="button_44" width="auto" height="auto" bkimage="menu_tree.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
                <Label class="menu_text" text="二级菜单项 0" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
            </MenuItem>
            <!-- 二级菜单：第二种支持的形式（新增格式，方便在子菜单中添加通用控件） -->
            <SubMenu>
                <MenuItem class="menu_element" name="sub_menu1" width="180">
                    <Label class="menu_text" text="二级菜单项 1" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
                </MenuItem>
                <MenuItem class="menu_element" name="sub_menu2" width="180">
                    <Label class="menu_text" text="二级菜单项 2" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
                </MenuItem>
                <MenuItem class="menu_element" name="sub_menu3" width="180">
                    <Label class="menu_text" text="二级菜单项 3" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
                </MenuItem>
                <MenuItem class="menu_element" name="sub_menu4" width="180">
                    <Button name="button_05" width="auto" height="auto" bkimage="menu_tree.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
                    <Label class="menu_text" text="三级菜单" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
                    <!-- 三级菜单 -->
                    <MenuItem class="menu_element" name="sub_sub_menu1" width="180">
                        <Label class="menu_text" text="三级菜单项 1" mouse_enabled="false" keyboard_enabled="false"/>
                    </MenuItem>
                    <MenuItem class="menu_element" name="sub_sub_menu2" width="180">
                        <Label class="menu_text" text="三级菜单项 2" mouse_enabled="false" keyboard_enabled="false"/>
                    </MenuItem>
                </MenuItem>
            </SubMenu>
        </MenuItem>
        
        <!-- 菜单项之间的分割线 -->
        <Box class="menu_split_box">
            <Control class="menu_split_line" mouse_enabled="false" keyboard_enabled="false"/>
        </Box>
        
        <!-- 带勾选项的菜单项 -->
        <MenuItem class="menu_element" name="menu_check_01" width="180">
            <CheckBox class="menu_checkbox" name="menu_checkbox_01" text="排序方式：递增" margin="0,5,0,10" selected="true" tooltiptext="ui::Checkbox" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        <MenuItem class="menu_element" name="menu_check_02" width="180">
            <CheckBox class="menu_checkbox" name="menu_checkbox_02" text="排序方式：递减" margin="0,5,0,10" selected="false" tooltiptext="ui::Checkbox" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        
        <!-- 菜单项之间的分割线 -->
        <Box class="menu_split_box">
            <Control class="menu_split_line" mouse_enabled="false" keyboard_enabled="false"/>
        </Box>
    
        <MenuItem class="menu_element" name="about" width="auto">
            <Button name="button_06" width="auto" height="auto" bkimage="menu_about.png" valign="center" mouse="false" mouse_enabled="false" keyboard_enabled="false"/>
            <Label class="menu_text" text="关于" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
  </MenuListBox>
</Window>
```

4. `submenu.xml`中的主要内容：
```xml
<?xml version="1.0" encoding="utf-8"?>
<Window shadow_type="menu_round" shadow_attached="true" layered_window="true">
  <MenuListBox class="menu" name="submenu">
   
  </MenuListBox>
</Window>
```
`submenu.xml`是子菜单的配置文件，可以通过`Menu::SetSubMenuXml`接口修改：
```cpp
/** 设置多级子菜单的XML模板文件及属性
@param [in] submenuXml 子菜单的XML模板文件名，如果没设置，内部默认为 "submenu.xml"
@param [in] submenuNodeName 子菜单XML文件中，子菜单项插入位置的节点名称，如果没设置，内部默认为 "submenu"
*/
void SetSubMenuXml(const std::wstring& submenuXml, const std::wstring& submenuNodeName);
```
5. `examples/controls`示例程序中显示菜单的代码片段    
显示菜单，并且在二级菜单中，添加子菜单项：
```cpp
void ControlForm::ShowPopupMenu(const ui::UiPoint& point, ui::Control* pRelatedControl)
{
    ui::Menu* menu = new ui::Menu(this, pRelatedControl);//需要设置父窗口，否在菜单弹出的时候，程序状态栏编程非激活状态
    menu->SetSkinFolder(GetResourcePath().ToString());
    DString xml(_T("menu/settings_menu.xml"));
    menu->ShowMenu(xml, point);

    //在二级菜单中，添加子菜单项
    ui::MenuItem* menu_fourth = static_cast<ui::MenuItem*>(menu->FindControl(_T("fourth")));
    if (menu_fourth != nullptr) {
        ui::MenuItem* menu_item = new ui::MenuItem(menu);
        menu_item->SetText(_T("Dynamically created"));
        menu_item->SetClass(_T("menu_element"));
        menu_item->SetFixedWidth(ui::UiFixedInt(180), true, true);
        menu_item->SetFontId(_T("system_14"));
        menu_item->SetTextPadding({ 20, 0, 20, 0 }, true);
        menu_fourth->AddSubMenuItemAt(menu_item, 1);//添加后，资源由菜单统一管理
    }
```

添加菜单项的关联响应函数：
```cpp
    /* About menu */
    ui::MenuItem* menu_about = static_cast<ui::MenuItem*>(menu->FindControl(_T("about")));
    if (menu_about != nullptr) {
        menu_about->AttachClick([this](const ui::EventArgs& args) {
            AboutForm* about_form = new AboutForm();
            ui::WindowCreateParam createParam;
            createParam.m_dwStyle = ui::kWS_POPUP;
            createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
            createParam.m_windowTitle = _T("AboutForm");
            createParam.m_bCenterWindow = true;
            about_form->CreateWnd(this, createParam);
            about_form->ShowModalFake();
            return true;
            });
    }
}
```
