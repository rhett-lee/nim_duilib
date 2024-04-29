#ifndef UI_BOX_LISTBOX_ITEM_H_
#define UI_BOX_LISTBOX_ITEM_H_

#pragma once

#include "duilib/Box/VBox.h"
#include "duilib/Box/HBox.h"
#include "duilib/Control/Option.h"

namespace ui 
{
class IListBoxOwner;

/** ListBoxItem 接口
*/
class UILIB_API IListBoxItem
{
public:
	virtual ~IListBoxItem() = default;

	/** 调用Option类的选择函数, 只更新界面的选择状态
	* @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
	 * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。
	*/
	virtual void OptionSelected(bool bSelect, bool bTriggerEvent) = 0;

	/** 设置选择状态, 处理存储状态同步，但不触发任何事件(适用于同步状态)
	*/
	virtual void SetItemSelected(bool bSelected) = 0;

	/** 判断当前是否是选择状态
	 * @return 返回 true 为选择状态，否则为 false
	 */
	virtual bool IsSelected() const = 0;

	/** 获取父容器
	 */
	virtual IListBoxOwner* GetOwner() = 0;

	/** 设置父容器
	 * @param[in] pOwner 父容器指针
	 */
	virtual void SetOwner(IListBoxOwner* pOwner) = 0;

	/** 获取容器索引号，范围：[0, GetItemCount())
	 */
	virtual size_t GetListBoxIndex() const = 0;

	/** 设置容器子项索引号
	 * @param[in] iIndex 索引号, 范围：[0, GetItemCount())
	 */
	virtual void SetListBoxIndex(size_t iIndex) = 0;

	/** 获取虚表数据元素索引号，用于支持虚表，范围：[0, GetElementCount())
	 */
	virtual size_t GetElementIndex() const = 0;

	/** 设置虚表数据元素索引号
	 * @param[in] iIndex 索引号, 用于支持虚表，范围：[0, GetElementCount())
	 */
	virtual void SetElementIndex(size_t iIndex) = 0;
};

/** 列表项的数据子项，用于在列表中展示数据的子项
*/
template<typename InheritType = Box>
class UILIB_API ListBoxItemTemplate:
	public OptionTemplate<InheritType>,
	public IListBoxItem
{
public:
	ListBoxItemTemplate();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;	
	virtual void HandleEvent(const EventArgs& msg) override;

	/** 是否绘制选择状态下的背景色，提供虚函数作为可选项
	   （比如ListBox/TreeView节点在多选时，由于有勾选项，并不需要绘制选择状态的背景色）
	*/
	virtual bool CanPaintSelectedColors() const override;

	/** 判断控件类型是否为可选择的
	 * @return 返回true
	 */
	virtual bool IsSelectableType() const override;

	/** 设置控件是否选择状态
	 * @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
	 * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。
	 */
	virtual void Selected(bool bSelect, bool bTriggerEvent) override;

	/** 调用Option类的选择函数, 只更新界面的选择状态
	* @param [in] bSelected 为 true 时为选择状态，false 时为取消选择状态
	 * @param [in] bTriggerEvent 是否发送状态改变事件，true 为发送，否则为 false。
	*/
	virtual void OptionSelected(bool bSelect, bool bTriggerEvent) override;

	/** 设置选择状态, 处理存储状态同步，但不触发任何事件(适用于同步状态)
	*/
	virtual void SetItemSelected(bool bSelected) override;

	/** 判断当前是否是选择状态
	 * @return 返回 true 为选择状态，否则为 false
	 */
	virtual bool IsSelected() const override;

	/** 获取父容器
	 */
	virtual IListBoxOwner* GetOwner() override;

	/** 设置父容器
	 * @param[in] pOwner 父容器指针
	 */
	virtual void SetOwner(IListBoxOwner* pOwner) override;

	/** 获取容器索引号，范围：[0, GetItemCount())
	 */
	virtual size_t GetListBoxIndex() const override;

	/** 设置容器子项索引号
	 * @param[in] iIndex 索引号, 范围：[0, GetItemCount())
	 */
	virtual void SetListBoxIndex(size_t iIndex) override;

	/** 获取虚表数据元素索引号，用于支持虚表，范围：[0, GetElementCount())
	 */
	virtual size_t GetElementIndex() const override;

	/** 设置虚表数据元素索引号
	 * @param[in] iIndex 索引号, 用于支持虚表，范围：[0, GetElementCount())
	 */
	virtual void SetElementIndex(size_t iIndex) override;

public:
	/** 绑定鼠标点击处理函数
	* @param[in] callback 要绑定的回调函数
	*/
	void AttachClick(const EventCallback& callback) { this->AttachEvent(kEventClick, callback); }

	/** 绑定鼠标右键点击处理函数
	* @param[in] callback 要绑定的回调函数
	*/
	void AttachRClick(const EventCallback& callback) { this->AttachEvent(kEventRClick, callback); }

	/** 监听控件双击事件
	 * @param[in] callback 收到双击消息时的回调函数
	 */
	void AttachDoubleClick(const EventCallback& callback) { this->AttachEvent(kEventMouseDoubleClick, callback); }

	/** 监听回车事件
	 * @param[in] callback 收到回车时的回调函数
	 */
	void AttachReturn(const EventCallback& callback) { this->AttachEvent(kEventReturn, callback); }

protected:
	/** 选择状态变化事件(m_bSelected变量发生变化)
	*/
	virtual void OnPrivateSetSelected() override;

	/** 勾选状态变化事件(m_bChecked变量发生变化)
	*/
	virtual void OnPrivateSetChecked() override;

private:
	/** 在ListBox容器中的子项索引号，范围：[0, GetItemCount())
	*/
	size_t m_iListBoxIndex;

	/** 虚表数据元素子项索引号，用于支持虚表，范围：[0, GetElementCount())
	*/
	size_t m_iElementIndex;

	/** 在ListBox容器接口
	*/
	IListBoxOwner* m_pOwner;
};

/////////////////////////////////////////////////////////////////////////////////////

template<typename InheritType>
ListBoxItemTemplate<InheritType>::ListBoxItemTemplate():
	m_iListBoxIndex(Box::InvalidIndex),
	m_iElementIndex(Box::InvalidIndex),
	m_pOwner(nullptr)
{
	this->SetTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE, false);
}

template<typename InheritType>
std::wstring ListBoxItemTemplate<InheritType>::GetType() const { return DUI_CTR_LISTBOX_ITEM; }

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::SetItemSelected(bool bSelected)
{
	if (OptionTemplate<InheritType>::IsSelected() == bSelected) {
		return;
	}
	//直接修改内部状态
	OptionTemplate<InheritType>::SetSelected(bSelected);
	if (m_pOwner == nullptr) {
		return;
	}

	//同步ListBox的选择ID
	bool bChanged = false;
	if (bSelected) {
		m_pOwner->SetCurSel(m_iListBoxIndex);
		bChanged = true;
	}
	else {
		if (m_pOwner->GetCurSel() == m_iListBoxIndex) {
			m_pOwner->SetCurSel(Box::InvalidIndex);
			bChanged = true;
		}
	}

	if (bChanged && !m_pOwner->IsMultiSelect()) {
		//单选：需要调用选择函数, 保证只有一个选中项
		m_pOwner->EnsureSingleSelection();
	}
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::OptionSelected(bool bSelect, bool bTriggerEvent)
{
	return OptionTemplate<InheritType>::Selected(bSelect, bTriggerEvent);
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::Selected(bool bSelected, bool bTriggerEvent)
{
	//界面点击等操作触发选择操作
	if (!this->IsEnabled()) {
		return;
	}
	if (m_pOwner != nullptr) {
		if (bSelected) {
			m_pOwner->SelectItem(m_iListBoxIndex, false, bTriggerEvent);
		}
		else {
			m_pOwner->UnSelectItem(m_iListBoxIndex, bTriggerEvent);
		}
	}
}

template<typename InheritType>
bool ListBoxItemTemplate<InheritType>::IsSelected() const
{
	return OptionTemplate<InheritType>::IsSelected();
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::OnPrivateSetSelected()
{
	if (m_pOwner != nullptr) {
		m_pOwner->OnItemSelectedChanged(m_iListBoxIndex, this);
	}
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::OnPrivateSetChecked()
{
	if (m_pOwner != nullptr) {
		m_pOwner->OnItemCheckedChanged(m_iListBoxIndex, this);
	}
}

template<typename InheritType>
bool ListBoxItemTemplate<InheritType>::CanPaintSelectedColors() const
{
	bool bHasStateImages = this->HasStateImages();
	if (m_pOwner != nullptr) {
		return m_pOwner->CanPaintSelectedColors(bHasStateImages);
	}
	return OptionTemplate<InheritType>::CanPaintSelectedColors();
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::HandleEvent(const EventArgs& msg)
{
	if (this->IsDisabledEvents(msg)) {
		//如果是鼠标键盘消息，并且控件是Disabled的，转发给Owner控件
		if (m_pOwner != nullptr) {
			m_pOwner->SendEvent(msg);
		}
		else {
			OptionTemplate<InheritType>::HandleEvent(msg);
		}
		return;
	}
	if (msg.Type == kEventMouseDoubleClick) {
		if (!this->IsActivatable()) {
			return;
		}
	}
	else if (msg.Type == kEventKeyDown && this->IsEnabled()) {
		if (msg.chKey == VK_RETURN) {
			if (this->IsActivatable()) {
				this->SendEvent(kEventReturn);
			}
			return;
		}
	}
	OptionTemplate<InheritType>::HandleEvent(msg);

	// An important twist: The list-item will send the event not to its immediate
	// parent but to the "attached" list. A list may actually embed several components
	// in its path to the item, but key-presses etc. needs to go to the actual list.
	//if( m_pOwner != NULL ) m_pOwner->HandleMessage(event); else Control::HandleMessage(event);
}

template<typename InheritType>
IListBoxOwner* ListBoxItemTemplate<InheritType>::GetOwner()
{
	return m_pOwner;
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::SetOwner(IListBoxOwner* pOwner)
{
	m_pOwner = pOwner;
}

template<typename InheritType>
size_t ListBoxItemTemplate<InheritType>::GetListBoxIndex() const
{
	return m_iListBoxIndex;
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::SetListBoxIndex(size_t iIndex)
{
	m_iListBoxIndex = iIndex;
}

template<typename InheritType>
size_t ListBoxItemTemplate<InheritType>::GetElementIndex() const
{
	return m_iElementIndex;
}

template<typename InheritType>
void ListBoxItemTemplate<InheritType>::SetElementIndex(size_t iIndex)
{
	m_iElementIndex = iIndex;
}

template<typename InheritType>
bool ListBoxItemTemplate<InheritType>::IsSelectableType() const
{
	return true;
}

/** 类型定义
*/
typedef ListBoxItemTemplate<Box> ListBoxItem;
typedef ListBoxItemTemplate<HBox> ListBoxItemH;
typedef ListBoxItemTemplate<VBox> ListBoxItemV;

} // namespace ui

#endif // UI_BOX_LISTBOX_ITEM_H_
