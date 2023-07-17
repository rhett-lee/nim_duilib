#ifndef UI_BOX_LISTBOX_H_
#define UI_BOX_LISTBOX_H_

#pragma once

#include "duilib/Box/ScrollBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Box/VBox.h"
#include "duilib/Box/TileBox.h"
#include "duilib/Control/Option.h"

namespace ui 
{

/** 用户自定义的排序函数
@param [in] pControl1 第一个控件接口
@param [in] pControl2 第二个控件接口
@param [in] pCompareContext 用户自定义上下文数据
@return < 0 控件1小于控件2
        = 0 控件1等于控件2
		> 0 控件1大于控件2
*/
typedef int (CALLBACK *PFNCompareFunc)(Control* pControl1, Control* pControl2, void* pCompareContext);

/** ListBox所有者接口
*/
class UILIB_API IListBoxOwner
{
public:
	/**@brief 触发事件
	*/
	virtual void SendEvent(const EventArgs& event) = 0;

	/**@brief 获取当前选择的索引，(如果无有效索引，则返回Box::InvalidIndex)
	*/
	virtual size_t GetCurSel() const = 0;

	/**@brief 选择子项
	*  @param [in] iIndex 子项目的ID
	*  @param [in] bTakeFocus 是否让子项控件成为焦点控件
	*  @param [in] bTrigger 是否触发选择事件 
	*/
	virtual bool SelectItem(size_t iIndex, bool bTakeFocus = false, bool bTrigger = true) = 0;

	/**@brief 确保区域可见
	*/
	virtual void EnsureVisible(const UiRect& rcItem) = 0;

	/**@brief 停止滚动条动画
	*/
	virtual void StopScroll() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////
//

/** 列表容器，用于展示一组数据使用
*   通过修改布局，形成 HListBox和VListBox和TileListBox三个子类
*/
class UILIB_API ListBox : public ScrollBox, public IListBoxOwner
{
public:
	/** 默认为垂直布局的ListBox
	*/
	explicit ListBox(Layout* pLayout = new VLayout);
	ListBox(const ListBox& r) = delete;
	ListBox& operator=(const ListBox& r) = delete;

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual void HandleEvent(const EventArgs& event) override;
	virtual bool ButtonDown(const EventArgs& msg) override;
	virtual void SendEvent(EventType eventType, WPARAM wParam = 0, LPARAM lParam = 0, TCHAR tChar = 0,
						   const UiPoint& mousePos = UiPoint()) override;
	virtual void SendEvent(const EventArgs& event) override;

	/**@brief 获取当前选择的索引，(如果无有效索引，则返回Box::InvalidIndex)
	*/
	virtual size_t GetCurSel() const override;
	virtual bool SelectItem(size_t iIndex, bool bTakeFocus = false, bool bTrigger = true) override;
	virtual void EnsureVisible(const UiRect& rcItem) override;
	virtual void StopScroll() override;
	
	/**@brief 滚动到指定子项位置
	 * @param[in] strItemName 子项名称
	 */
	virtual bool ScrollItemToTop(const std::wstring& strItemName);

	/**@brief 获取当前位置第一个子项
	 */
	virtual Control* GetTopItem();
	
	/**@brief 设置子项的位置索引
	 * @param[in] pControl 子项指针
	 * @param[in] iIndex 索引号
	 */
	virtual bool SetItemIndex(Control* pControl, size_t iIndex) override;

	/**@brief 追加一个子项到末尾
	 * @param[in] pControl 子项指针
	 */
	virtual bool AddItem(Control* pControl) override;

	/**@brief 在指定位置之后插入一个子项
	 * @param[in] pControl 子项指针
	 * @param[in] iIndex 要插入的位置索引
	 */
    virtual bool AddItemAt(Control* pControl, size_t  iIndex) override;

	/**@brief 根据子项指针
	 * @param[in] pControl 子项指针
	 */
    virtual bool RemoveItem(Control* pControl) override;

	/**@brief 根据索引移除一个子项
	 * @param[in] iIndex 子项索引
	 */
    virtual bool RemoveItemAt(size_t  iIndex) override;

	/**@brief 移除所有子项
	 */
    virtual void RemoveAllItems() override;

	/**@brief 选中上一项
	*/
	void SelectPreviousItem();

	/**@brief 选中下一项
	 */
	void SelectNextItem();

	/**@brief 排列子项
	 * @param [in] pfnCompare 自定义排序函数
	 * @param [in] pCompareContext 传递给比较函数的用户自定义数据
	 */
	bool SortItems(PFNCompareFunc pfnCompare, void* pCompareContext);

	/**@brief 获取是否随滚动改变选中项设置
	 * @return 返回 true 表示跟随滚动条改变选择项，否则为 false
	 */
	bool GetScrollSelect() const;

	/**@brief 设置是否随滚动改变选中项设置
	 * @param[in] bScrollSelect 为 true 是为跟随滚动条改变选中项，false 为不跟随
	 */
	void SetScrollSelect(bool bScrollSelect);

	/**@brief 监听选择子项的事件
	 * @param[in] callback 选择子项时的回调函数
	 */
	void AttachSelect(const EventCallback& callback) { AttachEvent(kEventSelect, callback); }

	/**@brief 在移除一个子项后自动选择下一项
	 * @param[in] bSelectNextItem 为 true 时自动选择下一项，false 为不自动选择
	 */
	void SelectNextWhenActiveRemoved(bool bSelectNextItem);

private:
	/**
	 * @brief 默认的子项对比方法
	 * @param[in] pvlocale 保存 List 指针
	 * @param[in] item1 子项1
	 * @param[in] item2 子项2
	 * @return 返回对比结果
	 */
	static int __cdecl ItemComareFunc(void* pvlocale, const void* item1, const void* item2);
	int __cdecl ItemComareFunc(const void* item1, const void* item2);

protected:
	//是否随滚动改变选中项
	bool m_bScrollSelect;

	//当前选择的子项ID
	size_t m_iCurSel;

	//在移除一个子项后自动选择下一项
	bool m_bSelNextWhenRemoveActive;

	//用户自定义的排序比较函数
	PFNCompareFunc m_pCompareFunc;

	//用户自定义的排序比较函数中的上下文数据
	void* m_pCompareContext;
};

/////////////////////////////////////////////////////////////////////////////////////
//

/// 列表项，用于在列表中展示数据的子项
class UILIB_API ListBoxElement: 
	public OptionTemplate<Box>
{
public:
	ListBoxElement();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void Selected(bool bSelect, bool trigger) override;
	virtual void HandleEvent(const EventArgs& event) override;

	/** 判断控件类型是否为可选择的
	 * @return 返回true
	 */
	virtual bool IsSelectableType() const override;

	/**
	 * @brief 获取父容器
	 * @return 返回父容器指针
	 */
	IListBoxOwner* GetOwner();

	/**
	 * @brief 设置父容器
	 * @param[in] pOwner 父容器指针
	 * @return 无
	 */
	void SetOwner(IListBoxOwner* pOwner);

	/**
	 * @brief 获取当前索引
	 * @return 返回当前索引
	 */
	size_t GetIndex() const;

	/**
	 * @brief 设置索引
	 * @param[in] iIndex 索引值
	 * @return 无
	 */
	void SetIndex(size_t iIndex);

	/**
	 * @brief 监听控件双击事件
	 * @param[in] callback 收到双击消息时的回调函数
	 * @return 无
	 */
	void AttachDoubleClick(const EventCallback& callback) { AttachEvent(kEventMouseDoubleClick, callback); }

	/**
	 * @brief 监听回车事件
	 * @param[in] callback 收到回车时的回调函数
	 * @return 无
	 */
	void AttachReturn(const EventCallback& callback) { AttachEvent(kEventReturn, callback); }

private:
	/** 在ListBox容器中的子项索引号
	*/
	size_t m_iIndex;

	/** 在ListBox容器接口
	*/
	IListBoxOwner* m_pOwner;
};

/** 瓦片布局的ListBox
*/
class UILIB_API TileListBox : public ListBox
{
public:
	TileListBox() :
		ListBox(new TileLayout)
	{
	}

	virtual std::wstring GetType() const override { return DUI_CTR_TILELISTBOX; }
};

/** 水平布局的ListBox
*/
class UILIB_API HListBox : public ListBox
{
public:
	HListBox() :
		ListBox(new HLayout)
	{
	}

	virtual std::wstring GetType() const override { return DUI_CTR_HLISTBOX; }
};

/** 垂直布局的ListBox
*/
class UILIB_API VListBox : public ListBox
{
public:
	VListBox() :
		ListBox(new VLayout)
	{
	}

	virtual std::wstring GetType() const override { return DUI_CTR_VLISTBOX; }
};

} // namespace ui

#endif // UI_BOX_LISTBOX_H_
