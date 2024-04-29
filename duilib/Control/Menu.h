#ifndef UI_CONTROL_MENU_H_
#define UI_CONTROL_MENU_H_

#pragma once

#include "duilib/duilib_defs.h"
#include "duilib/Utils/WinImplBase.h"
#include "duilib/Box/ListBox.h"

namespace ui {

//菜单对齐方式
enum MenuAlignment
{
	eMenuAlignment_Left			= 1 << 1,
	eMenuAlignment_Top			= 1 << 2,
	eMenuAlignment_Right		= 1 << 3,
	eMenuAlignment_Bottom		= 1 << 4,
	eMenuAlignment_Intelligent	= 1 << 5    //智能的防止被遮蔽
};

//菜单关闭类型
enum class MenuCloseType
{
	eMenuCloseThis,  //适用于关闭当前级别的菜单窗口，如鼠标移入时
	eMenuCloseAll     //关闭所有菜单窗口，如失去焦点时
};

//菜单弹出位置的类型
enum class MenuPopupPosType
{					    //鼠标点击的point属于菜单的哪个位置    1.-----.2       1左上 2右上              
												   //     |     |
	//这里假定用户是喜欢智能的                                3.-----.4       3左下 4右下
	RIGHT_BOTTOM	= eMenuAlignment_Right | eMenuAlignment_Bottom | eMenuAlignment_Intelligent,
	RIGHT_TOP		= eMenuAlignment_Right | eMenuAlignment_Top    | eMenuAlignment_Intelligent,
	LEFT_BOTTOM		= eMenuAlignment_Left  | eMenuAlignment_Bottom | eMenuAlignment_Intelligent,
	LEFT_TOP		= eMenuAlignment_Left  | eMenuAlignment_Top    | eMenuAlignment_Intelligent,
	//这里是normal，非智能的
	RIGHT_BOTTOM_N	= eMenuAlignment_Right | eMenuAlignment_Bottom,
	RIGHT_TOP_N		= eMenuAlignment_Right | eMenuAlignment_Top,
	LEFT_BOTTOM_N	= eMenuAlignment_Left  | eMenuAlignment_Bottom,
	LEFT_TOP_N		= eMenuAlignment_Left  | eMenuAlignment_Top
};

#include "observer_impl_base.hpp"
struct ContextMenuParam
{
	MenuCloseType wParam;
	HWND hWnd;
};

typedef class ObserverImpl<BOOL, ContextMenuParam> ContextMenuObserver;
typedef class ReceiverImpl<BOOL, ContextMenuParam> ContextMenuReceiver;

/////////////////////////////////////////////////////////////////////////////////////
//

/** 菜单类
*/
class CMenuElementUI;
class CMenuWnd : public ui::WindowImplBase, public ContextMenuReceiver
{
public:
	/** 构造函数，初始化菜单的父窗口句柄
	*/
	explicit CMenuWnd(HWND hParent);

	/** 设置资源加载的文件夹名称，如果没设置，内部默认为 "menu"
	*   XML文件中的资源（图片、XML等），均在这个文件夹中查找
	*/
	void SetSkinFolder(const std::wstring& skinFolder);

	/** 设置多级子菜单的XML模板文件及属性
	@param [in] submenuXml 子菜单的XML模板文件名，如果没设置，内部默认为 "submenu.xml"
	@param [in] submenuNodeName 子菜单XML文件中，子菜单项插入位置的节点名称，如果没设置，内部默认为 "submenu"
	*/
	void SetSubMenuXml(const std::wstring& submenuXml, const std::wstring& submenuNodeName);

	/** 初始化菜单配置，并且显示菜单
	*   返回后，可以通过FindControl函数来找到菜单项，进行后续操作
	* @param [in] xml 菜单XML资源文件名，内部会与GetSkinFolder()拼接成完整路径
	* @param [in] point 菜单弹出位置
	* @param [in] popupPosType 菜单弹出位置类型
	* @param [in] noFocus 菜单弹出后，不激活窗口，避免抢焦点
	* @Param [in] pOwner 父菜单的接口，如果这个值不是nullptr，则这个菜单是多级菜单模式
	*/
	void ShowMenu(const std::wstring& xml, 
		          const UiPoint& point,
		          MenuPopupPosType popupPosType = MenuPopupPosType::LEFT_TOP, 
		          bool noFocus = false,
		          CMenuElementUI* pOwner = nullptr);

	/** 关闭菜单
	*/
	void CloseMenu();

public:
	//添加子菜单项
	bool AddMenuItem(CMenuElementUI* pMenuItem);
	bool AddMenuItemAt(CMenuElementUI* pMenuItem, size_t iIndex);

	//删除菜单项
	bool RemoveMenuItem(CMenuElementUI* pMenuItem);
	bool RemoveMenuItemAt(size_t iIndex);

	//获取菜单项个数
	size_t GetMenuItemCount() const;

	//获取菜单项接口
	CMenuElementUI* GetMenuItemAt(size_t iIndex) const;
	CMenuElementUI* GetMenuItemByName(const std::wstring& name) const;

private:
	friend CMenuElementUI; //需要访问部分私有成员函数

	//获取全局菜单Observer对象
	static ContextMenuObserver& GetMenuObserver();

	//与父菜单对象接触关联关系
	void DetachOwner();		//add by djj 20200506

private:
	// 重新调整菜单的大小
	void ResizeMenu();

	// 重新调整子菜单的大小
	void ResizeSubMenu();

private:

	virtual BOOL Receive(ContextMenuParam param) override;

	virtual ui::Control* CreateControl(const std::wstring& pstrClass) override;
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;
	virtual void OnInitWindow() override;
	virtual void OnFinalMessage(HWND hWnd) override;
	virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

private:
	//菜单父窗口句柄
	HWND m_hParent;

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
	CMenuElementUI* m_pOwner;

	//菜单的布局接口
	ui::ListBox* m_pLayout;
};

/** 菜单项
*/
class CMenuElementUI : public ui::ListBoxItem
{
public:
	CMenuElementUI();

	//添加子菜单项
	bool AddSubMenuItem(CMenuElementUI* pMenuItem);
	bool AddSubMenuItemAt(CMenuElementUI* pMenuItem, size_t iIndex);

	//删除子菜单项
	bool RemoveSubMenuItem(CMenuElementUI* pMenuItem);
	bool RemoveSubMenuItemAt(size_t iIndex);
	void RemoveAllSubMenuItem();

	//获取子菜单项个数
	size_t GetSubMenuItemCount() const;

	//获取子菜单项接口
	CMenuElementUI* GetSubMenuItemAt(size_t iIndex) const;
	CMenuElementUI* GetSubMenuItemByName(const std::wstring& name) const;


private:
	//获取一个菜单项下所有子菜单项的接口(仅包含菜单子项元素)
	static void GetAllSubMenuItem(const CMenuElementUI* pParentElementUI, 
						          std::vector<CMenuElementUI*>& submenuItems);

	//获取一个菜单项下所有子菜单控件的接口(包含菜单子项元素和其他控件)
	static void GetAllSubMenuControls(const CMenuElementUI* pParentElementUI,
						              std::vector<Control*>& submenuControls);

private:
	virtual bool ButtonUp(const ui::EventArgs& msg) override;
	virtual bool MouseEnter(const ui::EventArgs& msg) override;
	virtual void PaintChild(ui::IRender* pRender, const ui::UiRect& rcPaint) override;

private:
	friend CMenuWnd; //需要访问部分私有成员函数

	//检查子菜单，如果是下级菜单，则创建下级菜单窗口，并显示
	bool CheckSubMenuItem();

	//创建下级菜单窗口，并显示
	void CreateMenuWnd();

private:
	//下级菜单窗口接口
	CMenuWnd* m_pSubWindow;
};

} // namespace ui

#endif // UI_CONTROL_MENU_H_
