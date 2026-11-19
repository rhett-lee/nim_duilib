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

/** 当前激活的顶级菜单变化的回调函数原型
* @param [in] nOldIndex 变化前的激活索引（-1表示没有激活的菜单）
* @param [in] nNewIndex 变化后的激活索引（-1表示没有激活的菜单）
*/
typedef std::function<void (int32_t nOldIndex, int32_t nNewIndex)> MenuBarActiveChangedEvent;

//MenuBar上的文字按钮
class MenuBarButton;

/** 菜单栏
*/
class DUILIB_API MenuBar: public HBox
{
    typedef HBox BaseClass;
public:
    explicit MenuBar(Window* pWindow);

public:
    /// 重写父类接口，提供个性化功能。方法具体说明请查看 Control 控件
    virtual DString GetType() const override;
    virtual void OnInit() override;
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

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
    * @param [in] menuItemId 本菜单项的ID，回调函数中会使用，以标识哪个菜单中的命令被激活
    * @param [in] menuText 顶级菜单的文本
    * @param [in] menuTextId 顶级菜单的文本ID, 用于支持多语言版
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
    bool GetTopMenu(int32_t nMenuIndex, MenuBarItem& menuBarItem) const;

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

    /** 设置指定顶级菜单的文本
    * @param [in] nMenuIndex 顶级菜单索引号
    * @param [in] menuText 菜单文本
    * @param [in] menuTextId 菜单文本ID，用于支持多语言版（非空时优先使用）
    * @return 成功返回true，失败返回false
    */
    bool SetTopMenuText(int32_t nMenuIndex, const DString& menuText, const DString& menuTextId = _T(""));

    /** 设置指定顶级菜单是否可用
    * @param [in] nMenuIndex 顶级菜单索引号
    * @param [in] bEnable true表示可用，false表示禁用
    * @return 成功返回true，失败返回false
    */
    bool SetTopMenuEnabled(int32_t nMenuIndex, bool bEnable);

    /** 设置指定顶级菜单是否可见
    * @param [in] nMenuIndex 顶级菜单索引号
    * @param [in] bVisible true表示可见，false表示隐藏
    * @return 成功返回true，失败返回false
    */
    bool SetTopMenuVisible(int32_t nMenuIndex, bool bVisible);

    /** 关闭当前已展开的菜单
    */
    void ClosePopupMenu();

    /** 注册当前激活顶级菜单变化的回调函数
    * @param [in] callback 回调函数，参数为变化前的索引和变化后的索引（-1表示没有激活的菜单）
    */
    void AttachMenuBarActiveChanged(MenuBarActiveChangedEvent callback);

    /** 清空当前激活顶级菜单变化的回调函数
    */
    void ClearMenuBarActiveChanged();

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

        ControlPtrT<MenuBarButton> m_pButton; //对应的顶级菜单按钮控件
    };

private:
    //将顶级菜单添加到UI控件（成功返回true）
    bool AddTopMenuToUI(TopMenuData& menuData, size_t nInsertItem);

    //获取插入顶级菜单时，对应的子控件插入位置
    size_t GetInsertChildIndex(size_t nMenuIndex) const;

    //从UI控件中移除顶级菜单
    void RemoveTopMenuFromUI(TopMenuData& menuData);

private:
    //顶级菜单按钮的鼠标事件
    friend class MenuBarButton;
    friend class Menu;

    void OnMenuMouseEnter(MenuBarButton* pButton, const EventArgs& msg);
    void OnMenuMouseLeave(MenuBarButton* pButton, const EventArgs& msg);

    //检查并按需显示子菜单
    void CheckShowPopupMenu(MenuBarButton* pButton);

    //在指定顶级菜单项中，弹出子菜单
    void ShowPopupMenu(MenuBarButton* pButton);

    //收起子菜单（bNotify为false时，不触发激活状态变化事件）
    void HidePopupMenu(MenuBarButton* pButton, bool bNotify = true);

    //获取指定按钮对应的顶级菜单索引号（未找到返回-1）
    int32_t GetTopMenuIndex(MenuBarButton* pButton) const;

    //通知当前激活的顶级菜单发生变化
    void NotifyActiveChanged(int32_t nOldIndex, int32_t nNewIndex);

    //将焦点设置到当前激活的菜单按钮（无激活项时设置到第一个可用按钮）
    void FocusTopMenuButton();

    //切换菜单栏的键盘焦点状态（F10/Alt按下时调用：未聚焦则聚焦，已聚焦则取消）
    void ToggleFocusTopMenuButton();

    /** 把键盘焦点移动到相邻的顶级菜单按钮
    * @param [in] pButton 当前有焦点的顶级菜单按钮
    * @param [in] bNext true表示向右移动，false表示向左移动
    * @return 成功移动返回true，到达边界或者无法移动时返回false
    */
    bool FocusAdjacentTopMenuButton(MenuBarButton* pButton, bool bNext);

    //取消等待中的悬停切换定时器
    void CancelHoverTimer();

    //当前弹层是否属于指定的顶级菜单按钮
    bool IsPopupMenuOf(MenuBarButton* pButton) const;

    //切换指定顶级菜单按钮的弹层（已展开则收起，未展开则弹出）
    void TogglePopupMenu(MenuBarButton* pButton);

    //当前是否存在激活的菜单
    bool HasActivePopupMenu() const;

    /** 在菜单上键盘按下
    * @param [in] pMenu 菜单接口
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    */
    void OnMenuKeyDownMsg(Menu* pMenu, VirtualKeyCode vkCode, uint32_t modifierKey);

protected:
    /** 设置可见状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetVisible(bool bChanged) override;

    /** 设置可用状态事件
    * @param [in] bChanged true表示状态发生变化，false表示状态未发生变化
    */
    virtual void OnSetEnabled(bool bChanged) override;

private:
    //顶级菜单项数据
    std::vector<TopMenuData> m_topMenuList;

    //菜单项激活回调函数
    std::vector<MenuBarItemActivatedEvent> m_callbackList;

    //当前激活菜单变化的回调函数
    std::vector<MenuBarActiveChangedEvent> m_activeChangedList;

    //当前激活的菜单接口
    ControlPtrT<Menu> m_pActiveMenu;

    //等待悬停切换的菜单按钮
    ControlPtrT<MenuBarButton> m_pHoverButton;

    //等待悬停切换的定时器ID（0表示没有定时器）
    size_t m_nHoverTimerId = 0;

    //Alt键是否处于按下状态（用于过滤键盘自动重复产生的消息）
    bool m_bAltKeyDown = false;
};

} // namespace ui

#endif // UI_CONTROL_MENU_BAR_H_
