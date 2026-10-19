简体中文 | [English](Menu.en.md)

## 菜单
菜单的实现是基于窗口([WindowImplBase](../duilib/Utils/WinImplBase.h))实现的，主要由[Menu](../duilib/Control/Menu.h)和[MenuItem](../duilib/Control/Menu.h)两个类组成。
1. 菜单的效果预览
这个预览是`examples/controls`示例程序中的菜单。
<img src="./Images/Menu.png"/>
2. 菜单的实现包含了系统菜单中的基本功能：支持图标、勾选框、多级菜单、菜单项分割条、动态修改菜单项、菜单中插入非菜单控件等。
3. `settings_menu.xml`中的主要内容：
```xml
<?xml version="1.0" encoding="utf-8"?>
<Window shadow_type="menu_round">
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
            <Slider class="slider_horizontal_green" value="70" tooltip_text="ui::Slider"/>
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
            <CheckBox class="menu_checkbox" name="menu_checkbox_01" text="排序方式：递增" margin="0,5,0,10" selected="true" tooltiptext="ui::CheckBox" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        <MenuItem class="menu_element" name="menu_check_02" width="180">
            <CheckBox class="menu_checkbox" name="menu_checkbox_02" text="排序方式：递减" margin="0,5,0,10" selected="false" tooltiptext="ui::CheckBox" mouse_enabled="false" keyboard_enabled="false"/>
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
<Window shadow_type="menu_round">
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
    ui::Menu* menu = new ui::Menu(this, pRelatedControl);//需要设置父窗口，否则菜单弹出的时候，程序状态栏编程非激活状态
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

6. 便捷绑定右键菜单（一行代码）    
除上述"手动创建 `Menu` + 坐标换算 + 绑定回调"的方式外，`Control` 提供了便捷接口 `AttachPopupMenu`：一行代码即可把右键菜单绑定到任意控件。右键点击控件时，会自动弹出指定菜单，并自动完成屏幕坐标换算；菜单项被激活（鼠标点击或键盘回车）后触发回调。
```cpp
// 在窗口初始化时绑定：在 context_menu_btn 控件上点右键，弹出 menu/settings_menu.xml
ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("context_menu_btn")));
if (pButton != nullptr) {
    pButton->AttachPopupMenu(_T("menu/settings_menu.xml"),
        [this](const DString& itemName, size_t nItemIndex) {
            // itemName: 被激活菜单项的 name 属性; nItemIndex: 菜单项索引(从0开始)
            ui::ToastWnd::Show(this, ui::StringUtil::Printf(_T("Context menu item: %s"), itemName.c_str()),
                               ui::ToastWnd::kTypeInfo, 1500, ui::ToastWnd::kPosTop, false);
        });
}
```
接口说明：
```cpp
/** 便捷绑定右键上下文菜单：右键点击此控件时，自动弹出 menuXml 指定的菜单；
*   菜单项被激活(鼠标点击或者键盘回车)后，通过 actionCallback 回调。
*   重复调用本函数时，会直接覆盖之前的绑定配置(菜单及回调均以最后一次为准)。
* @param [in] menuXml 菜单XML资源文件名(相对于窗口的资源路径 GetResourcePath()，如 "menu/settings_menu.xml")
* @param [in] actionCallback 菜单项被激活后的回调函数；可以为空(为空时不回调)
* @param [in] creatingCallback 菜单弹出后的回调函数，用于动态调整菜单内容；可以为空
* @return 绑定成功返回true，否则返回false(menuXml为空)
*/
bool AttachPopupMenu(const DString& menuXml,
                     const ContextMenuActionEvent& actionCallback = nullptr,
                     const ContextMenuCreatingEvent& creatingCallback = nullptr);

/** 解除由 AttachPopupMenu 绑定的右键上下文菜单(与 AttachPopupMenu 配对使用)
*   调用后，本控件不再响应右键上下文菜单；未绑定过时调用无副作用。
*/
void DetachPopupMenu();
```
其中，两个回调类型定义如下：
```cpp
//菜单项被激活后的回调函数原型
typedef std::function<void (const DString& itemName, size_t nItemIndex)> ContextMenuActionEvent;

//菜单弹出后的回调函数原型(可动态增删菜单项、设置菜单项的勾选/禁用状态等)
typedef std::function<void (Menu* pMenu, Control* pSender)> ContextMenuCreatingEvent;
```
说明：
1. `AttachPopupMenu` 支持在 Windows / Linux / macOS 等所有平台使用，框架会统一产生上下文菜单事件（含按 `Shift+F10` 触发的场景）。
2. 重复调用 `AttachPopupMenu` 会直接覆盖上一次的绑定配置（菜单 XML 与两个回调均以最后一次为准），不会叠加；如需解除绑定，调用 `DetachPopupMenu()`，解除后右键不再弹菜单。`DetachPopupMenu()` 只注销本接口注册的回调，不影响通过 `AttachContextMenu` 单独注册的 `kEventContextMenu` 回调。
3. 需要根据右键位置动态调整菜单内容（如增删菜单项、设置勾选/禁用状态）时，使用 `creatingCallback`：此时菜单已加载完成，可通过 `pMenu->FindControl(name)` 获取菜单项。
4. `examples/controls` 示例程序的 `test.xml` 中，已内置演示控件 `context_menu_btn`，可参考 [TestForm.cpp](../examples/controls/TestForm.cpp) 的实现。
