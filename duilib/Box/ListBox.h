#ifndef UI_BOX_LISTBOX_H_
#define UI_BOX_LISTBOX_H_

#pragma once

#include "duilib/Box/ScrollBox.h"
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
	/** 触发事件
	*/
	virtual void SendEvent(const EventArgs& event) = 0;

	/** 获取当前选择的索引，(如果无有效索引，则返回Box::InvalidIndex)
	*/
	virtual size_t GetCurSel() const = 0;

	/** 选择子项
	*  @param [in] iIndex 子项目的ID
	*  @param [in] bTakeFocus 是否让子项控件成为焦点控件
	*  @param [in] bTrigger 是否触发选择事件, 如果为true，会触发一个kEventSelect事件
	*/
	virtual bool SelectItem(size_t iIndex, bool bTakeFocus, bool bTrigger) = 0;

	/** 确保矩形区域可见
	*/
	virtual void EnsureVisible(const UiRect& rcItem) = 0;

	/** 停止滚动条动画
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
	explicit ListBox(Layout* pLayout);
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

public:
	/**获取当前选择的索引，(如果无有效索引，则返回Box::InvalidIndex)
	*/
	virtual size_t GetCurSel() const override;

	/** 选择子项
	*  @param [in] iIndex 子项目的ID
	*  @param [in] bTakeFocus 是否让子项控件成为焦点控件
	*  @param [in] bTrigger 是否触发选择事件, 如果为true，会触发一个kEventSelect事件
	*/
	virtual bool SelectItem(size_t iIndex, bool bTakeFocus, bool bTrigger) override;
	
	/** 确保矩形区域可见
	*/
	virtual void EnsureVisible(const UiRect& rcItem) override;

	/** 停止滚动条动画
	*/
	virtual void StopScroll() override;

public:
	/** 滚动到指定子项位置
	 * @param [in] itemName 子项名称(即：Control::GetName())
	 */
	virtual bool ScrollItemToTop(const std::wstring& itemName);

	/** 获取当前矩形区域(Control::::GetPos())中的第一个子项
	 */
	virtual Control* GetTopItem();
	
	/** 设置子项的位置索引
	 * @param [in] pControl 子项指针
	 * @param [in] iIndex 索引号
	 */
	virtual bool SetItemIndex(Control* pControl, size_t iIndex) override;

	/** 追加一个子项到末尾
	 * @param [in] pControl 子项指针
	 */
	virtual bool AddItem(Control* pControl) override;

	/** 在指定位置之后插入一个子项
	 * @param [in] pControl 子项指针
	 * @param[in] iIndex 要插入的位置索引
	 */
    virtual bool AddItemAt(Control* pControl, size_t  iIndex) override;

	/** 根据子项指针
	 * @param [in] pControl 子项指针
	 */
    virtual bool RemoveItem(Control* pControl) override;

	/** 根据索引移除一个子项
	 * @param [in] iIndex 子项索引
	 */
    virtual bool RemoveItemAt(size_t iIndex) override;

	/** 移除所有子项
	 */
    virtual void RemoveAllItems() override;

public:
	/** 确保子项可见
	* @param [in] iIndex 子项索引
	*/
	void EnsureVisible(size_t iIndex);

	/** 选择子项，选中后让子项控件成为焦点控件，并触发一个kEventSelect事件
	*  @param [in] iIndex 子项目的ID
	*/
	bool SelectItem(size_t iIndex);

	/** 选中上一项
	*/
	void SelectPreviousItem();

	/** 选中下一项
	 */
	void SelectNextItem();

	/** 对子项排序
	 * @param [in] pfnCompare 自定义排序函数
	 * @param [in] pCompareContext 传递给比较函数的用户自定义数据
	 */
	bool SortItems(PFNCompareFunc pfnCompare, void* pCompareContext);

	/** 获取是否随滚动改变选中项设置
	 * @return 返回 true 表示跟随滚动条改变选择项，否则为 false
	 */
	bool GetScrollSelect() const;

	/** 设置是否随滚动改变选中项设置
	 * @param[in] bScrollSelect 为 true 是为跟随滚动条改变选中项，false 为不跟随
	 */
	void SetScrollSelect(bool bScrollSelect);

	/** 监听选择子项的事件
	 * @param[in] callback 选择子项时的回调函数
	 */
	void AttachSelect(const EventCallback& callback) { AttachEvent(kEventSelect, callback); }

	/** 在移除一个子项后自动选择下一项
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

/** 列表项，用于在列表中展示数据的子项
*/
class UILIB_API ListBoxItem: 
	public OptionTemplate<Box>
{
public:
	ListBoxItem();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void Selected(bool bSelect, bool trigger) override;
	virtual void HandleEvent(const EventArgs& event) override;

	/** 判断控件类型是否为可选择的
	 * @return 返回true
	 */
	virtual bool IsSelectableType() const override;

	/** 获取父容器
	 */
	IListBoxOwner* GetOwner();

	/** 设置父容器
	 * @param[in] pOwner 父容器指针
	 */
	void SetOwner(IListBoxOwner* pOwner);

	/**
	 * @brief 获取当前索引号
	 */
	size_t GetIndex() const;

	/** 设置索引号
	 * @param[in] iIndex 索引号
	 */
	void SetIndex(size_t iIndex);

	/** 监听控件双击事件
	 * @param[in] callback 收到双击消息时的回调函数
	 */
	void AttachDoubleClick(const EventCallback& callback) { AttachEvent(kEventMouseDoubleClick, callback); }

	/** 监听回车事件
	 * @param[in] callback 收到回车时的回调函数
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

/** 横向布局的ListBox
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

/** 纵向布局的ListBox
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

/** 瓦片布局的ListBox(横向布局)
*/
class UILIB_API HTileListBox : public ListBox
{
public:
	HTileListBox() :
		ListBox(new HTileLayout)
	{
	}

	virtual std::wstring GetType() const override { return DUI_CTR_HTILE_LISTBOX; }
};

/** 瓦片布局的ListBox(纵向布局)
*/
class UILIB_API VTileListBox : public ListBox
{
public:
	VTileListBox() :
		ListBox(new VTileLayout)
	{
	}

	virtual std::wstring GetType() const override { return DUI_CTR_VTILE_LISTBOX; }
};

} // namespace ui

#endif // UI_BOX_LISTBOX_H_
