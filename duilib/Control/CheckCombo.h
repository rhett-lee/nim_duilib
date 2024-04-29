#ifndef UI_CONTROL_CHECKCOMBO_H_
#define UI_CONTROL_CHECKCOMBO_H_

#pragma once

#include "duilib/Box/ScrollBox.h"

namespace ui
{

class CCheckComboWnd;
class UILIB_API CheckCombo : public ui::Box
{
	friend class CCheckComboWnd;
public:
	CheckCombo();
	CheckCombo(const CheckCombo& r) = delete;
	CheckCombo& operator=(const CheckCombo& r) = delete;
	virtual ~CheckCombo();

public:
	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual void Activate() override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

public:
	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual bool AddItem(Control* pControl) override;
	virtual bool AddItemAt(Control* pControl, size_t iIndex) override;
	virtual bool RemoveItem(Control* pControl) override;
	virtual bool RemoveItemAt(size_t iIndex) override;
	virtual void RemoveAllItems() override;
	virtual Control* GetItemAt(size_t iIndex) const override;
	virtual size_t GetItemIndex(Control* pControl) const override;
	virtual bool SetItemIndex(Control* pControl, size_t iIndex) override;
	virtual size_t GetItemCount() const override;

	/** 添加一个下拉框的列表项
	*/
	bool AddTextItem(const std::wstring& itemText);

	/** 获取选择的文本列表
	*/
	void GetSelectedText(std::vector<std::wstring>& selectedText) const;

	/** 清除所有列表项和选择项
	*/
	void ClearAll();

public:
	/** 获取当前所属的 List 对象
	*/
	ui::ScrollBox* GetListBox() { return m_pDropList.get(); }

	/** 设置下拉框的属性信息
	* @param [in] pstrList 转义后的 XML 格式属性列表
	*/
	void SetDropBoxAttributeList(const std::wstring& pstrList);

	/** 获取下拉框容器大小
	*/
	ui::UiSize GetDropBoxSize() const;

	/** 设置下拉框容器大小
	* @param[in] szDropBox 要设置的大小信息
	*/
	void SetDropBoxSize(ui::UiSize szDropBox);

	/** 设置 Combobox 是否向上弹出
	* @param [in] top 为 true 则向上弹出，false 为向下弹出
	*/
	void SetPopupTop(bool top) { m_bPopupTop = top; };

	/** 判断 Combobox 弹出模式是否是向上弹出
	*/
	bool IsPopupTop() const { return m_bPopupTop; };

	/** 设置下拉列表中每一个列表项的属性
	*/
	void SetDropboxItemClass(const std::wstring& classValue);

	/** 设置选择项中每一个子项的属性
	*/
	void SetSelectedItemClass(const std::wstring& classValue);

	/** 更新下拉列表窗口的位置
	*/
	void UpdateComboWndPos();

	/** 监听下拉窗关闭事件
	* @param[in] callback 下拉窗关闭后触发的回调函数
	*/
	void AttachWindowClose(const ui::EventCallback& callback) { AttachEvent(ui::kEventWindowClose, callback); }

private:
	/** 默认的子项被选择处理函数
	* @param[in] args 参数列表
	* @return 始终返回 true
	*/
	bool OnSelectItem(const ui::EventArgs& args);
	bool OnUnSelectItem(const ui::EventArgs& args);
	bool OnListButtonDown(const ui::EventArgs& args);

private:
	/** 解析属性列表
	*/
	void ParseAttributeList(const std::wstring& strList,
							std::vector<std::pair<std::wstring, std::wstring>>& attributeList) const;

	/** 设置控件的属性列表
	*/
	void SetAttributeList(Control* pControl, const std::wstring& classValue);

	/** 更新选择列表的高度
	*/
	void UpdateSelectedListHeight();

protected:
	/** 下拉框的窗口接口
	*/
	CCheckComboWnd* m_pCheckComboWnd;

	/** 下拉列表容器
	*/
	std::unique_ptr<ui::ScrollBox> m_pDropList;

	/** 选择的列表项容器
	*/
	std::unique_ptr<ui::ScrollBox> m_pList;
	
	/** 下拉框的宽度和高度
	*/
	ui::UiSize m_szDropBox;

	/** 下拉框是否向上弹出
	*/
	bool m_bPopupTop;

	/** 容器的高度
	*/
	int32_t m_iOrgHeight;

	/** 选择项的文本
	*/
	std::vector<std::string> m_vecDate;

	/** 下拉列表中每一个列表项的属性
	*/
	UiString m_dropboxItemClass;

	/** 选择项中每一个子项的属性
	*/
	UiString m_selectedItemClass;
};

} //namespace ui

#endif //UI_CONTROL_CHECKCOMBO_H_
