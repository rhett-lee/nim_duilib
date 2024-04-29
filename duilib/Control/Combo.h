#ifndef UI_CONTROL_COMBO_H_
#define UI_CONTROL_COMBO_H_

#pragma once

#include "duilib/Core/Box.h"
#include "duilib/Control/TreeView.h"
#include "duilib/Control/RichEdit.h"

namespace ui 
{
/** 组合框控件
*/
class UILIB_API Combo : public Box
{
    friend class CComboWnd;
public:
	Combo();
	Combo(const Combo& r) = delete;
	Combo& operator=(const Combo& r) = delete;
	virtual ~Combo();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual bool CanPlaceCaptionBar() const override;
	virtual std::wstring GetBorderColor(ControlStateType stateType) const override;

protected:
	virtual void OnInit() override;

public:
	/** Combo类型
	*/
	enum ComboType
	{
		//下拉列表, 不可编辑输入
		kCombo_DropList = 0,

		//编辑列表，可编辑输入
		kCombo_DropDown = 1
	};

	/** 设置Combo类型
	*/
	void SetComboType(ComboType comboType);

	/** 获取Combo类型
	*/
	ComboType GetComboType() const;

	/** 获取下拉框列表大小(宽度和高度)
	*/
	UiSize GetDropBoxSize() const;

	/** 设置下拉框列表大小(宽度和高度)
	 * @param [in] szDropBox 要设置的大小信息
	 * @param [in] bNeedScaleDpi 是否需要做DPI自适应
	 */
	void SetDropBoxSize(UiSize szDropBox, bool bNeedScaleDpi);

	/** 设置 Combobox 是否向上弹出
	 * @param[in] top 为 true 则向上弹出，false 为默认向下弹出
	 */
	void SetPopupTop(bool top) { m_bPopupTop = top; }

	/** 判断 Combobox 弹出模式是否是向上弹出
	 * @return 返回 true 表示向上弹出，否则为 false
	 */
	bool IsPopupTop() const { return m_bPopupTop; }

	/** 设置下拉表TreeView的Class属性
	*/
	void SetComboTreeClass(const std::wstring& classValue);

	/** 设置下拉表TreeView的节点Class属性
	*/
	void SetComboTreeNodeClass(const std::wstring& classValue);

	/** 设置显示图标的Class属性
	*/
	void SetIconControlClass(const std::wstring& classValue);

	/** 设置编辑控件的Class属性
	*/
	void SetEditControlClass(const std::wstring& classValue);

	/** 设置按钮控件的Class属性
	*/
	void SetButtonControlClass(const std::wstring& classValue);

public:
	/** 获取Combo列表中的子项个数
	*/
	size_t GetCount() const;

	/** 获取当前选择项索引
	 * @return 返回当前选择项索引, (如果无有效索引，则返回Box::InvalidIndex)
	 */
	size_t GetCurSel() const;

	/** 选择一个子项, 不触发选择事件
	 * @param[in] iIndex 要选择的子项索引
	 * @return 返回 true 表示成功，否则为 false
	 */
	bool SetCurSel(size_t iIndex);

	/** 获取子项关联的数据
	* @param [in] iIndex 子项索引号
	* @return 返回该索引号关联的数据
	*/
	size_t GetItemData(size_t iIndex) const;

	/** 设置子项关联的数据
	* @param [in] iIndex 子项索引号
	* @param [in] itemData 需要保存的数据
	*/
	bool SetItemData(size_t iIndex, size_t itemData);

	/** 获取子项的文本
	* @param [in] iIndex 子项索引号
	*/
	std::wstring GetItemText(size_t iIndex) const;

	/** 设置子项的文本
	* @param [in] iIndex 子项索引号
	* @param [in] itemText 子项的文本内容
	*/
	bool SetItemText(size_t iIndex, const std::wstring& itemText);

	/** 添加一个子项字符串
	* @param [in] itemText 子项的文本内容
	* @return 返回新添加的子项索引号
	*/
	size_t AddTextItem(const std::wstring& itemText);

	/** 在指定索引号位置, 插入一个子项字符串, 返回新添加的子项索引号
	* @param [in] iIndex 子项索引号
	* @param [in] itemText 子项的文本内容
	*/
	size_t InsertTextItem(size_t iIndex, const std::wstring& itemText);

	/** 删除一个子项
	* @param [in] iIndex 子项索引号
	*/
	bool DeleteItem(size_t iIndex);

	/** 移除所有子节点
	 */
	void DeleteAllItems();

	/** 选择匹配的文本项目
	* @param [in] itemText 子项的文本内容
	* @param [in] bTriggerEvent bTriggerEvent 是否触发选择事件, 如果为true，会触发一个kEventSelect事件
	* @return 返回该选中项的索引号，如果未能选中，则返回Box::InvalidIndex
	*/
	size_t SelectTextItem(const std::wstring& itemText, bool bTriggerEvent = true);

public:
	/** 获取当前编辑框内的文本
	 */
	std::wstring GetText() const;

	/** 设置编辑框内的文本
	*/
	void SetText(const std::wstring& text);

	/** 让控件获取焦点
	 */
	virtual void SetFocus();

public:
	/** 获取下拉列表的树接口
	*/
	TreeView* GetTreeView();

	/** 获取图标控件
	*/
	Control* GetIconControl() const;

	/** 编辑框控件
	*/
	RichEdit* GetEditControl() const;

	/** 按钮控件
	*/
	Button* GetButtonContrl() const;

	/** 更新下拉列表窗口的位置
	*/
	void UpdateComboWndPos();

public:
	/** 监听子项被选择事件
	 * @param[in] callback 子项被选择后触发的回调函数
	 */
	void AttachSelect(const EventCallback& callback) { AttachEvent(kEventSelect, callback);}	//mod by djj

	/** 监听下拉窗关闭事件
	 * @param[in] callback 下拉窗关闭后触发的回调函数
	 */
	void AttachWindowClose(const EventCallback& callback) { AttachEvent(kEventWindowClose, callback); }

protected:
	/** 显示下拉列表
	*/
	virtual void ShowComboList();

	/** 关闭下拉列表
	*/
	virtual void HideComboList();

	/** 更新下拉列表
	*/
	virtual void UpdateComboList();

	/** 默认的子项被选择处理函数
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnSelectItem(const EventArgs& args);

	/** 下拉框窗口关闭
	* @param [in] bCanceled true表示取消，否则表示正常关闭
	* @param [in] needUpdateSelItem true表示需要更新选择项，否则不需要更新选择项
	* @param [in] oldEditText 下拉框显示时，编辑框的文本内容
	*/
	virtual void OnComboWndClosed(bool bCanceled, 
		                          bool needUpdateSelItem,
								  const std::wstring& oldEditText);

	/** 鼠标按下按钮
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnButtonDown(const EventArgs& args);

	/** 点击按钮
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnButtonClicked(const EventArgs& args);

	/** 鼠标在Edit上面按下按钮
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnEditButtonDown(const EventArgs& args);

	/** 鼠标在Edit上面弹起按钮
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnEditButtonUp(const EventArgs& args);

	/** 在Edit上按键
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnEditKeyDown(const EventArgs& args);

	/** Edit控件获得焦点
	* @param[in] args 参数列表
	* @return 始终返回 true
	*/
	virtual bool OnEditSetFocus(const EventArgs& args);

	/** Edit控件失去焦点
	* @param[in] args 参数列表
	* @return 始终返回 true
	*/
	virtual bool OnEditKillFocus(const EventArgs& args);

	/** 窗口失去焦点
	* @param[in] args 参数列表
	* @return 始终返回 true
	*/
	virtual bool OnWindowKillFocus(const EventArgs& args);

	/** 窗口移动
	* @param[in] args 参数列表
	* @return 始终返回 true
	*/
	virtual bool OnWindowMove(const EventArgs& args);

	/** 选择项变化，同步Edit控件的文本
	*/
	virtual void OnSelectedItemChanged();

	/** Edit的文本内容发生变化
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnEditTextChanged(const ui::EventArgs& args);

private:
	/** 解析属性列表
	*/
	void ParseAttributeList(const std::wstring& strList,
							std::vector<std::pair<std::wstring, std::wstring>>& attributeList) const;

	/** 设置控件的属性列表
	*/
	void SetAttributeList(Control* pControl, const std::wstring& classValue);

	/** 移除控件
	*/
	void RemoveControl(Control* pControl);

	/** 创建一个新的树节点
	*/
	TreeNode* CreateTreeNode(const std::wstring& itemText);

	/** 挂载该控件的鼠标事件，设置焦点
	*/
	void AttachMouseEvents(Control* pControl);

private:
	/** Combo类型
	*/
	uint8_t m_comboType;

	/** 列表树的接口
	*/
	TreeView m_treeView;

	/** 下拉列表的窗口接口
	*/
	CComboWnd* m_pWindow;

	/** 下拉列表的大小（宽度和高度）
	*/
	UiSize m_szDropBox;

	/** 下拉列表是否向上弹出
	*/
	bool m_bPopupTop;

	/** 下拉表TreeView的节点Class属性
	*/
	UiString m_treeNodeClass;

private:
	/** 图标控件
	*/
	Control* m_pIconControl;

	/** 编辑框控件
	*/
	RichEdit* m_pEditControl;

	/** 按钮控件
	*/
	Button* m_pButtonControl;

	/** 当前选择项索引号
	*/
	size_t m_iCurSel;

	/** 鼠标按下的时候，是否正在显示下拉列表
	*/
	bool m_bDropListShown;
};

} // namespace ui

#endif // UI_CONTROL_COMBO_H_
