#ifndef UI_CONTROL_MENU_BAR_H_
#define UI_CONTROL_MENU_BAR_H_

#include "duilib/Control/Menu.h"
#include "duilib/Control/Button.h"
#include "duilib/Box/HBox.h"
#include "duilib/Core/ControlPtrT.h"

namespace ui
{
/** 顶级菜单项数据
*/
struct MenuBarItem
{
    DString m_menuItemId;               //本菜单项的ID，回调函数中会使用，以标识哪个菜单中的命令被激活
    DString m_menuText;                 //菜单文本
    DString m_menuTextId;               //菜单文本ID，用于支持多语言版
    DString m_menuXmlPath;              //菜单资源XML路径(在资源目录的相对路径)

    DString m_menuTextButtonClass;      //菜单文本显示控件Button的属性Class名称，若为空表示使用默认风格(global.xml中的"menu_bar_button"属性)
    DString m_menuTextButtonAttributes; //菜单文本显示控件Button的属性附加属性列表，可以为空    
};

/** 顶级菜单选择菜单项的回调函数原型: 在菜单消失后，用于获取用户点击了哪个菜单项(鼠标点击或者键盘回车激活)
* @param [in] menuItemId 本菜单项的ID，即MenuBarItem里面的m_menuItemId值
* @param [in] menuName 菜单名称(即XML里面的的name属性，这代表菜单项的ID)
* @param [in] nMenuLevel 菜单层级（0表示一级菜单，1表示二级菜单，...）
* @param [in] itemName 菜单项的名称，相当于命令ID(即XML里面的的name属性，这代表菜单项的ID)
* @param [in] nItemIndex 菜单项的索引序号（从0开始的序号）
*/
typedef std::function<void (const DString& menuItemId,
                            const DString& menuName, int32_t nMenuLevel,
                            const DString& itemName, size_t nItemIndex)> MenuBarItemActivatedEvent;

/** 菜单栏
*/
class MenuBar: public HBox
{
    typedef HBox BaseClass;
public:
    explicit MenuBar(Window* pWindow);

public:
    /// 重写父类接口，提供个性化功能。方法具体说明请查看 Control 控件
    virtual DString GetType() const override;
    virtual void OnInit() override;

public:
    /** 添加一个顶级菜单，并返回其索引号
    * @param [in] menuItemId 本菜单项的ID，回调函数中会使用，以标识哪个菜单中的命令被激活
    * @param [in] menuText 顶级菜单的文本
    * @param [in] menuTextId 顶级菜单的文本ID, 用于支持多语言版
    * @param [in] menuXmlPath 菜单XML资源文件路径
    * @param [in] menuTextButtonClass 顶级菜单文本显示控件Button的属性Class名称，传空表示使用默认值
    * @param [in] menuTextButtonAttributes 顶级菜单文本显示控件Button的属性附加属性列表
    * @return 成功返回索引号，失败返回-1
    */
    int32_t AddTopMenu(const DString& menuItemId,
                       const DString& menuText,
                       const DString& menuTextId,
                       const DString& menuXmlPath,
                       const DString& menuTextButtonClass = _T(""),
                       const DString& menuTextButtonAttributes = _T(""));

    /** 添加一个顶级菜单，并返回其索引号
    * @param [in] menuBarItem 顶级菜单的相关数据
    * @return 成功返回索引号，失败返回-1
    */
    int32_t AddTopMenu(const MenuBarItem& menuBarItem);

    /** 在指定位置添加一个顶级菜单，并返回其索引号
    * @param [in] nMenuIndex 指定的索引号
    * @param [in] menuText 顶级菜单的文本
    * @param [in] menuXmlPath 菜单XML资源文件路径
    * @param [in] menuTextButtonClass 顶级菜单文本显示控件Button的属性Class名称，传空表示使用默认值
    * @param [in] menuTextButtonAttributes 顶级菜单文本显示控件Button的属性附加属性列表
    * @return 成功返回索引号，失败返回-1
    */
    int32_t InsertTopMenu(int32_t nMenuIndex,
                          const DString& menuItemId,
                          const DString& menuText,
                          const DString& menuTextId,
                          const DString& menuXmlPath,
                          const DString& menuTextButtonClass = _T(""),
                          const DString& menuTextButtonAttributes = _T(""));

    /** 在指定位置添加一个顶级菜单，并返回其索引号
    * @param [in] menuBarItem 顶级菜单的相关数据
    * @return 成功返回索引号，失败返回-1
    */
    int32_t InsertTopMenu(int32_t nMenuIndex, const MenuBarItem& menuBarItem);

    /** 获取指定索引号的顶级菜单信息
    * @param [in] nMenuIndex 指定的索引号
    * @param [out] menuBarItem 顶级菜单的相关数据
    */
    bool GetTopMenu(int32_t nMenuIndex, MenuBarItem& menuBarItem);

    /** 根据顶级菜单索引号，移除一个顶级菜单
    */
    bool RemoveTopMenu(int32_t nMenuIndex);

    /** 设置当前激活的顶级菜单（展开其子菜单）
    */
    bool SetActiveTopMenuIndex(int32_t nMenuIndex);

    /** 获取当前激活的顶级菜单索引号
    */
    int32_t GetActiveTopMenuIndex() const;

    /** 注册菜单项激活的回调函数, 在菜单消失后，用于获取用户点击了哪个菜单项(鼠标点击或者键盘回车激活)
    * @param [in] callback 回调函数
    */
    void AttachMenuBarItemActivated(MenuBarItemActivatedEvent callback);

    /** 清空所有回调函数
    */
    void ClearMenuBarItemActivated();

private:
    //顶级菜单项数据
    struct TopMenuData
    {
        UiString m_menuItemId;               //本菜单项的ID，回调函数中会使用，以标识哪个菜单中的命令被激活
        UiString m_menuText;                 //菜单文本
        UiString m_menuTextId;               //菜单文本ID，用于支持多语言版
        UiString m_menuXmlPath;              //菜单资源XML路径

        UiString m_menuTextButtonClass;      //菜单文本显示控件Button的属性Class名称
        UiString m_menuTextButtonAttributes; //菜单文本显示控件Button的属性附加属性列表

        size_t m_nItemDataId = Box::InvalidIndex;    //在UI容器中的控件ID号
        bool m_bMouseEnter = false;                  //是否处于MouseEnter状态中（用于避免菜单重复弹出）
    };

private:
    //将顶级菜单添加到UI控件
    void AddTopMenuToUI(const TopMenuData& menuData, size_t nInsertItem);

    //从UI控件中移除顶级菜单
    void RemoveTopMenuFromUI(const TopMenuData& menuData);

private:
    //顶级菜单按钮的鼠标事件
    friend class MenuBarButton;
    friend class Menu;

    void OnMenuMouseEnter(Button* pButton, const EventArgs& msg);
    void OnMenuMouseLeave(Button* pButton, const EventArgs& msg);
    void OnMenuMouseButtonDown(Button* pButton, const EventArgs& msg);
    void OnMenuMouseButtonUp(Button* pButton, const EventArgs& msg);

    //检查并按需显示子菜单
    void CheckShowPopupMenu(Button* pButton);

    //在指定顶级菜单项中，弹出子菜单
    void ShowPopupMenu(Button* pButton);

    //收起子菜单
    void HidePopupMenu(Button* pButton);

    //当前是否存在激活的菜单
    bool HasActivePopupMenu() const;

    /** 在菜单上键盘按下
    * @param [in] pMenu 菜单接口
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    */
    void OnMenuKeyDownMsg(Menu* pMenu, VirtualKeyCode vkCode, uint32_t modifierKey);

private:
    //顶级菜单项数据
    std::vector<TopMenuData> m_topMenuList;

    //菜单项激活回调函数
    std::vector<MenuBarItemActivatedEvent> m_callbackList;

    //顶级菜单在UI中的ID值
    size_t m_nItemDataId;

    //当前激活的菜单接口
    ControlPtrT<Menu> m_pActiveMenu;

    //当前是否处于激活状态
    bool m_bActiveState;

    //当前是否允许按钮激活时显示菜单项
    bool m_bEnableBtnActive;
};

} // namespace ui

#endif // UI_CONTROL_MENU_BAR_H_
