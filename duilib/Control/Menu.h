#ifndef UI_CONTROL_MENU_H_
#define UI_CONTROL_MENU_H_

#include "duilib/Utils/WinImplBase.h"
#include "duilib/Box/ListBox.h"
#include "duilib/Core/ControlPtrT.h"

namespace ui {

//菜单对齐方式
enum MenuAlignment
{
    eMenuAlignment_Left         = 1 << 1,
    eMenuAlignment_Top          = 1 << 2,
    eMenuAlignment_Right        = 1 << 3,
    eMenuAlignment_Bottom       = 1 << 4,
    eMenuAlignment_Intelligent  = 1 << 5    //智能的防止被遮蔽
};

//菜单关闭类型
enum class MenuCloseType
{
    eMenuCloseThis,  //适用于关闭当前级别的菜单窗口，如鼠标移入时
    eMenuCloseAll     //关闭所有菜单窗口，如失去焦点时
};

//菜单弹出位置的类型
enum class MenuPopupPosType
{   //鼠标点击的point属于菜单的哪个位置        1.-----.2       1左上 2右上              
    //                                     |     |
    //这里假定用户是喜欢智能的                3.-----.4       3左下 4右下
    RIGHT_BOTTOM    = eMenuAlignment_Right | eMenuAlignment_Bottom | eMenuAlignment_Intelligent,
    RIGHT_TOP       = eMenuAlignment_Right | eMenuAlignment_Top    | eMenuAlignment_Intelligent,
    LEFT_BOTTOM     = eMenuAlignment_Left  | eMenuAlignment_Bottom | eMenuAlignment_Intelligent,
    LEFT_TOP        = eMenuAlignment_Left  | eMenuAlignment_Top    | eMenuAlignment_Intelligent,
    //这里是normal，非智能的
    RIGHT_BOTTOM_N  = eMenuAlignment_Right | eMenuAlignment_Bottom,
    RIGHT_TOP_N     = eMenuAlignment_Right | eMenuAlignment_Top,
    LEFT_BOTTOM_N   = eMenuAlignment_Left  | eMenuAlignment_Bottom,
    LEFT_TOP_N      = eMenuAlignment_Left  | eMenuAlignment_Top
};

#include "observer_impl_base.hpp"
struct ContextMenuParam
{
    MenuCloseType wParam;
    WindowBase* pWindow;
};

typedef class ObserverImpl<bool, ContextMenuParam> ContextMenuObserver;
typedef class ReceiverImpl<bool, ContextMenuParam> ContextMenuReceiver;

/////////////////////////////////////////////////////////////////////////////////////
//


/** 选择菜单项的回调函数原型: 在菜单消失后，用于获取用户点击了哪个菜单项(鼠标点击或者键盘回车激活)
* @param [in] menuName 菜单名称(即XML里面的的name属性，这代表菜单项的ID)
* @param [in] nMenuLevel 菜单层级（0表示一级菜单，1表示二级菜单，...）
* @param [in] itemName 菜单项的名称(即XML里面的的name属性，这代表菜单项的ID)
* @param [in] nItemIndex 菜单项的索引序号（从0开始的序号）
*/
typedef std::function<void (const DString& menuName, int32_t nMenuLevel,
                            const DString& itemName, size_t nItemIndex)> MenuItemActivatedEvent;

/** 菜单类
*/
class MenuItem;
class Menu : public WindowImplBase, public ContextMenuReceiver
{
    typedef WindowImplBase BaseClass;
public:
    /** 构造函数，初始化菜单的父窗口句柄
    * @param [in] pParentWindow 菜单的父窗口
    * @param [in] pRelatedControl 菜单的关联控件，菜单弹出时，设置关联控件的状态为Pushed
    */
    explicit Menu(Window* pParentWindow, Control* pRelatedControl = nullptr);

    /** 设置资源加载的文件夹名称，如果没设置，内部默认为 "menu"
    *   XML文件中的资源（图片、XML等），均在这个文件夹中查找
    */
    void SetSkinFolder(const DString& skinFolder);

    /** 设置多级子菜单的XML模板文件及属性
    @param [in] submenuXml 子菜单的XML模板文件名，如果没设置，内部默认为 "submenu.xml"
    @param [in] submenuNodeName 子菜单XML文件中，子菜单项插入位置的节点名称，如果没设置，内部默认为 "submenu"
    */
    void SetSubMenuXml(const DString& submenuXml, const DString& submenuNodeName);

    /** 初始化菜单配置，并且显示菜单
    *   返回后，可以通过FindControl函数来找到菜单项，进行后续操作
    * @param [in] xml 菜单XML资源文件名，内部会与GetSkinFolder()拼接成完整路径
    * @param [in] point 菜单弹出位置
    * @param [in] popupPosType 菜单弹出位置类型
    * @param [in] noFocus 菜单弹出后，不激活窗口，避免抢焦点
    * @Param [in] pOwner 父菜单的接口，如果这个值不是nullptr，则这个菜单是多级菜单模式
    */
    void ShowMenu(const DString& xml, 
                  const UiPoint& point,
                  MenuPopupPosType popupPosType = MenuPopupPosType::LEFT_TOP, 
                  bool noFocus = false,
                  MenuItem* pOwner = nullptr);

    /** 关闭菜单
    */
    void CloseMenu();

    /** 注册菜单项激活的回调函数, 在菜单消失后，用于获取用户点击了哪个菜单项(鼠标点击或者键盘回车激活)
    * @param [in] callback 回调函数
    */
    void AttachMenuItemActivated(MenuItemActivatedEvent callback);

public:
    //添加子菜单项
    bool AddMenuItem(MenuItem* pMenuItem);
    bool AddMenuItemAt(MenuItem* pMenuItem, size_t iIndex);

    //删除菜单项
    bool RemoveMenuItem(MenuItem* pMenuItem);
    bool RemoveMenuItemAt(size_t iIndex);

    //获取菜单项个数
    size_t GetMenuItemCount() const;

    //获取菜单项接口
    MenuItem* GetMenuItemAt(size_t iIndex) const;
    MenuItem* GetMenuItemByName(const DString& name) const;

private:
    friend MenuItem; //需要访问部分私有成员函数

    //获取全局菜单Observer对象
    static ContextMenuObserver& GetMenuObserver();

    //与父菜单对象接触关联关系
    void DetachOwner();        //add by djj 20200506

private:
    // 重新调整菜单的大小
    bool ResizeMenu();

    // 重新调整子菜单的大小
    bool ResizeSubMenu();

    /** 获取布局管理的ListBox接口
    */
    ListBox* GetLayoutListBox() const;

    /** 菜单项激活(鼠标点击或者键盘回车激活)
    * @param [in] menuName 菜单名称(即XML里面的的name属性，这代表菜单项的ID)
    * @param [in] nMenuLevel 菜单层级（0表示一级菜单，1表示二级菜单，...）
    * @param [in] itemName 菜单项的名称(即XML里面的的name属性，这代表菜单项的ID)
    * @param [in] nItemIndex 菜单项的索引序号（从0开始的序号）
    */
    void OnMenuItemActivated(const DString& menuName, int32_t nMenuLevel,
                             const DString& itemName, size_t nItemIndex);

private:

    virtual bool Receive(ContextMenuParam param) override;

    virtual ui::Control* CreateControl(const DString& pstrClass) override;
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void PostInitWindow() override;
    virtual void OnCloseWindow() override;

    /** 在窗口销毁时会被调用，这是该窗口的最后一个消息（该类默认实现是清理资源，并销毁该窗口对象）
    */
    virtual void OnFinalMessage() override;

    /** 窗口失去焦点(WM_KILLFOCUS)
    * @param [in] pSetFocusWindow 接收键盘焦点的窗口（可以为nullptr）
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKillFocusMsg(WindowBase* pSetFocusWindow, const NativeMsg& nativeMsg, bool& bHandled) override;

    /** 键盘按下(WM_KEYDOWN 或者 WM_SYSKEYDOWN)
    * @param [in] vkCode 虚拟键盘代码
    * @param [in] modifierKey 按键标志位，有效值：ModifierKey::kFirstPress, ModifierKey::kAlt
    * @param [in] nativeMsg 从系统接收到的原始消息内容
    * @param [out] bHandled 消息是否已经处理，返回 true 表明已经成功处理消息，不需要再传递给窗口过程；返回 false 表示将消息继续传递给窗口过程处理
    * @return 返回消息的处理结果，如果应用程序处理此消息，应返回零
    */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

    //屏蔽的消息
    virtual LRESULT OnContextMenuMsg(const UiPoint& pt, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnMouseRButtonDownMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnMouseRButtonUpMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;
    virtual LRESULT OnMouseRButtonDbClickMsg(const UiPoint& pt, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

private:
    //菜单父窗口
    Window* m_pParentWindow;

    //菜单弹出位置
    UiPoint m_menuPoint;

    //菜单弹出位置的类型
    MenuPopupPosType m_popupPosType;

    //资源加载的文件夹名称
    UiString m_skinFolder;

    //子菜单的XML模板文件名
    UiString m_submenuXml;

    //子菜单XML文件中，子菜单项插入位置的节点名称
    UiString m_submenuNodeName;

    //菜单资源的xml文件名
    UiString m_xml;

    //菜单弹出时，是否为无聚焦模式
    bool m_noFocus;

    //菜单的父菜单接口
    MenuItem* m_pOwner;

    //菜单的布局接口
    ControlPtrT<ListBox> m_pListBox;

    //关联的控件
    ControlPtrT<Control> m_pRelatedControl;

private:
    //菜单项激活回调函数
    std::vector<MenuItemActivatedEvent> m_callbackList;

    //激活的菜单项信息
    struct ActiveMenuItem
    {
        DString m_menuName;
        int32_t m_menuLevel = 0;
        DString m_itemName;
        size_t m_itemIndex = Box::InvalidIndex;
    };
    std::unique_ptr<ActiveMenuItem> m_pActiveMenuItem;
};

/** 菜单项
*/
class MenuItem : public ListBoxItem
{
    typedef ListBoxItem BaseClass;
public:
    explicit MenuItem(Window* pWindow);

    //添加子菜单项
    bool AddSubMenuItem(MenuItem* pMenuItem);
    bool AddSubMenuItemAt(MenuItem* pMenuItem, size_t iIndex);

    //删除子菜单项
    bool RemoveSubMenuItem(MenuItem* pMenuItem);
    bool RemoveSubMenuItemAt(size_t iIndex);
    void RemoveAllSubMenuItem();

    //获取子菜单项个数
    size_t GetSubMenuItemCount() const;

    //获取子菜单项接口
    MenuItem* GetSubMenuItemAt(size_t iIndex) const;
    MenuItem* GetSubMenuItemByName(const DString& name) const;

    //菜单项激活（被点击获取通过回车激活）
    virtual void Activate(const EventArgs* pMsg) override;

private:
    //获取一个菜单项下所有子菜单项的接口(仅包含菜单子项元素)
    static void GetAllSubMenuItem(const MenuItem* pParentElementUI, 
                                  std::vector<MenuItem*>& submenuItems);

    //获取一个菜单项下所有子菜单控件的接口(包含菜单子项元素和其他控件)
    static void GetAllSubMenuControls(const MenuItem* pParentElementUI,
                                      std::vector<Control*>& submenuControls);

private:
    virtual bool ButtonUp(const ui::EventArgs& msg) override;
    virtual bool MouseEnter(const ui::EventArgs& msg) override;
    virtual void PaintChild(ui::IRender* pRender, const ui::UiRect& rcPaint) override;

private:
    friend Menu; //需要访问部分私有成员函数

    //检查子菜单，如果是下级菜单，则创建下级菜单窗口，并显示
    bool CheckSubMenuItem();

    //创建下级菜单窗口，并显示
    void CreateMenuWnd();

private:
    //下级菜单窗口接口
    Menu* m_pSubWindow;
};

} // namespace ui

#endif // UI_CONTROL_MENU_H_
