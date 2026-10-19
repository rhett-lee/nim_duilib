English | [简体中文](Menu.md)

> Last synced: 2026-10-12

## Menu
The implementation of the menu is based on the window ([WindowImplBase](../duilib/Utils/WinImplBase.h)), and mainly consists of two classes: [Menu](../duilib/Control/Menu.h) and [MenuItem](../duilib/Control/Menu.h).
1. Menu Preview
This preview is the menu in the `examples/controls` example program.
<img src="./Images/Menu.png"/>
2. The menu implementation includes the basic functions of a system menu: supports icons, checkboxes, multi-level menus, menu item separators, dynamic modification of menu items, inserting non-menu controls into the menu, etc.
3. Main Content of `settings_menu.xml`:
```xml
<?xml version="1.0" encoding="utf-8"?>
<Window shadow_type="menu_round">
    <MenuListBox class="menu" name="main_menu">
        <!-- First-level menu -->
        <MenuItem class="menu_element" name="first" width="180">
          <Button name="button_01" width="auto" height="auto" bkimage="menu_settings.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
          <Label class="menu_text" text="First-level Menu Item 1" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
    
        <MenuItem class="menu_element" name="second" width="180">
          <Button name="button_02" width="auto" height="auto" bkimage="menu_proxy.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
          <Label class="menu_text" text="First-level Menu Item 2" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        
        <!-- Insert a normal control in the menu to implement a specific function -->
        <HBox class="menu_split_box" height="36">
            <Label class="menu_text" text="Volume" textpadding="0,0,6,0" mouse_enabled="false" keyboard_enabled="false"/>
            <Control width="auto" height="auto" bkimage="menu_speaker.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
            <Slider class="slider_horizontal_green" value="70" tooltip_text="ui::Slider"/>
        </HBox>
        
        <!-- Separator line between menu items -->
        <Box class="menu_split_box">
            <Control class="menu_split_line" />
        </Box>
        
        <MenuItem class="menu_element" name="third" width="180">
            <Button name="button_03" width="auto" height="auto" bkimage="menu_logs.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
            <Label class="menu_text" text="First-level Menu Item 3" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        
        <MenuItem class="menu_element" name="fourth" width="180">
            <Button name="button_04" width="auto" height="auto" bkimage="menu_tree.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
            <Label class="menu_text" text="Second-level Menu" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
            <!-- Second-level menu: the first supported form (keeps compatibility with the old version) -->
            <MenuItem class="menu_element" name="sub_menu0" width="180">
                <Button name="button_44" width="auto" height="auto" bkimage="menu_tree.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
                <Label class="menu_text" text="Second-level Menu Item 0" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
            </MenuItem>
            <!-- Second-level menu: the second supported form (new format, convenient for adding common controls in the sub-menu) -->
            <SubMenu>
                <MenuItem class="menu_element" name="sub_menu1" width="180">
                    <Label class="menu_text" text="Second-level Menu Item 1" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
                </MenuItem>
                <MenuItem class="menu_element" name="sub_menu2" width="180">
                    <Label class="menu_text" text="Second-level Menu Item 2" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
                </MenuItem>
                <MenuItem class="menu_element" name="sub_menu3" width="180">
                    <Label class="menu_text" text="Second-level Menu Item 3" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
                </MenuItem>
                <MenuItem class="menu_element" name="sub_menu4" width="180">
                    <Button name="button_05" width="auto" height="auto" bkimage="menu_tree.png" valign="center" mouse_enabled="false" keyboard_enabled="false"/>
                    <Label class="menu_text" text="Third-level Menu" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
                    <!-- Third-level menu -->
                    <MenuItem class="menu_element" name="sub_sub_menu1" width="180">
                        <Label class="menu_text" text="Third-level Menu Item 1" mouse_enabled="false" keyboard_enabled="false"/>
                    </MenuItem>
                    <MenuItem class="menu_element" name="sub_sub_menu2" width="180">
                        <Label class="menu_text" text="Third-level Menu Item 2" mouse_enabled="false" keyboard_enabled="false"/>
                    </MenuItem>
                </MenuItem>
            </SubMenu>
        </MenuItem>
        
        <!-- Separator line between menu items -->
        <Box class="menu_split_box">
            <Control class="menu_split_line" />
        </Box>
        
        <!-- Menu item with a checkbox -->
        <MenuItem class="menu_element" name="menu_check_01" width="180">
            <CheckBox class="menu_checkbox" name="menu_checkbox_01" text="Sort Order: Ascending" margin="0,5,0,10" selected="true" tooltiptext="ui::CheckBox" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        <MenuItem class="menu_element" name="menu_check_02" width="180">
            <CheckBox class="menu_checkbox" name="menu_checkbox_02" text="Sort Order: Descending" margin="0,5,0,10" selected="false" tooltiptext="ui::CheckBox" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
        
        <!-- Separator line between menu items -->
        <Box class="menu_split_box">
            <Control class="menu_split_line" mouse_enabled="false" keyboard_enabled="false"/>
        </Box>
    
        <MenuItem class="menu_element" name="about" width="auto">
            <Button name="button_06" width="auto" height="auto" bkimage="menu_about.png" valign="center" mouse="false" mouse_enabled="false" keyboard_enabled="false"/>
            <Label class="menu_text" text="About" margin="30,0,0,0" mouse_enabled="false" keyboard_enabled="false"/>
        </MenuItem>
  </MenuListBox>
</Window>
```

4. Main Content of `submenu.xml`:
```xml
<?xml version="1.0" encoding="utf-8"?>
<Window shadow_type="menu_round">
  <MenuListBox class="menu" name="submenu">
   
  </MenuListBox>
</Window>
```
`submenu.xml` is the configuration file of the sub-menu, and can be modified through the `Menu::SetSubMenuXml` interface:
```cpp
/** Set the XML template file and attributes for the multi-level sub-menu
@param [in] submenuXml The XML template file name of the sub-menu; if not set, the internal default is "submenu.xml"
@param [in] submenuNodeName The node name of the sub-menu item insertion position in the sub-menu XML file; if not set, the internal default is "submenu"
*/
void SetSubMenuXml(const std::wstring& submenuXml, const std::wstring& submenuNodeName);
```
5. Code Snippet for Displaying the Menu in the `examples/controls` Example Program    
Display the menu, and add sub-menu items in the second-level menu:
```cpp
void ControlForm::ShowPopupMenu(const ui::UiPoint& point, ui::Control* pRelatedControl)
{
    ui::Menu* menu = new ui::Menu(this, pRelatedControl);//The parent window must be set, otherwise when the menu pops up, the program taskbar becomes inactive
    menu->SetSkinFolder(GetResourcePath().ToString());
    DString xml(_T("menu/settings_menu.xml"));
    menu->ShowMenu(xml, point);

    //Add a sub-menu item in the second-level menu
    ui::MenuItem* menu_fourth = static_cast<ui::MenuItem*>(menu->FindControl(_T("fourth")));
    if (menu_fourth != nullptr) {
        ui::MenuItem* menu_item = new ui::MenuItem(menu);
        menu_item->SetText(_T("Dynamically created"));
        menu_item->SetClass(_T("menu_element"));
        menu_item->SetFixedWidth(ui::UiFixedInt(180), true, true);
        menu_item->SetFontId(_T("system_14"));
        menu_item->SetTextPadding({ 20, 0, 20, 0 }, true);
        menu_fourth->AddSubMenuItemAt(menu_item, 1);//After adding, the resource is managed by the menu
    }
```

Add the Associated Response Function for the Menu Item:
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

6. Convenient Right-click Context Menu Binding (One Line of Code)    
In addition to the manual way above (create `Menu` + convert coordinates + bind callbacks), `Control` provides the convenience interface `AttachPopupMenu`: one line of code binds a context menu to any control. When the user right-clicks the control, the specified menu pops up automatically with the screen coordinates resolved, and the callback is triggered after a menu item is activated (mouse click or Enter key).
```cpp
// Bind during window initialization: right-clicking context_menu_btn pops up menu/settings_menu.xml
ui::Button* pButton = dynamic_cast<ui::Button*>(FindControl(_T("context_menu_btn")));
if (pButton != nullptr) {
    pButton->AttachPopupMenu(_T("menu/settings_menu.xml"),
        [this](const DString& itemName, size_t nItemIndex) {
            // itemName: the "name" attribute of the activated menu item; nItemIndex: item index (from 0)
            ui::ToastWnd::Show(this, ui::StringUtil::Printf(_T("Context menu item: %s"), itemName.c_str()),
                               ui::ToastWnd::kTypeInfo, 1500, ui::ToastWnd::kPosTop, false);
        });
}
```
Interface description:
```cpp
/** Conveniently binds a right-click context menu: when the user right-clicks this control,
*   the menu specified by menuXml pops up automatically; after a menu item is activated
*   (mouse click or Enter key), actionCallback is invoked.
*   Calling this function again overwrites the previous binding (menu and callbacks).
* @param [in] menuXml The menu XML resource file name (relative to the window resource path
*              GetResourcePath(), e.g. "menu/settings_menu.xml")
* @param [in] actionCallback The callback after a menu item is activated; may be null
* @param [in] creatingCallback The callback after the menu pops up, for adjusting the menu
*              contents dynamically; may be null
* @return Returns true on success, otherwise false (menuXml is empty)
*/
bool AttachPopupMenu(const DString& menuXml,
                     const ContextMenuActionEvent& actionCallback = nullptr,
                     const ContextMenuCreatingEvent& creatingCallback = nullptr);

/** Removes the right-click context menu bound by AttachPopupMenu (its counterpart).
*   After calling it, this control no longer responds to the context menu; calling it
*   when nothing is bound has no effect.
*/
void DetachPopupMenu();
```
The two callback types are defined as follows:
```cpp
//Callback prototype after a menu item is activated
typedef std::function<void (const DString& itemName, size_t nItemIndex)> ContextMenuActionEvent;

//Callback prototype after the menu pops up (add/remove menu items, set checked/enabled states, etc.)
typedef std::function<void (Menu* pMenu, Control* pSender)> ContextMenuCreatingEvent;
```
Notes:
1. `AttachPopupMenu` works on all platforms (Windows / Linux / macOS): the framework generates the context menu event uniformly, including the `Shift+F10` case.
2. Calling `AttachPopupMenu` again overwrites the previous binding (both the menu XML and the two callbacks take the last values); it does not stack. Call `DetachPopupMenu()` to unbind, after which right-clicking no longer pops up the menu. `DetachPopupMenu()` only unregisters the callback registered by this interface and does not affect a `kEventContextMenu` callback registered separately via `AttachContextMenu`.
3. To dynamically adjust the menu contents based on the right-click position (add/remove items, set checked/enabled states), use `creatingCallback`: at that moment the menu is already loaded, and menu items can be fetched via `pMenu->FindControl(name)`.
4. The `examples/controls` sample already contains a demo control `context_menu_btn` in `test.xml`; see [TestForm.cpp](../examples/controls/TestForm.cpp).
