#ifndef UI_CONTROL_COMBO_BUTTON_H_
#define UI_CONTROL_COMBO_BUTTON_H_

#pragma once

#include "duilib/Core/Box.h"
#include "duilib/Box/VBox.h"
#include "duilib/Control/TreeView.h"
#include "duilib/Control/RichEdit.h"

namespace ui 
{
typedef ButtonTemplate<VBox> ButtonVBox;

/** 带有下拉组合框的按钮
*/
class UILIB_API ComboButton : public Box
{
    friend class ComboButtonWnd;
public:
	ComboButton();
	ComboButton(const ComboButton& r) = delete;
	ComboButton& operator=(const ComboButton& r) = delete;
	virtual ~ComboButton();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;
	virtual bool CanPlaceCaptionBar() const override;
	virtual std::wstring GetBorderColor(ControlStateType stateType) const override;

public:

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

	/** 设置左侧按钮控件的Class属性
	*/
	void SetLeftButtonClass(const std::wstring& classValue);

	/** 设置左侧按钮上侧的Label控件的Class属性
	*/
	void SetLeftButtonTopLabelClass(const std::wstring& classValue);

	/** 设置左侧按钮下侧的Label控件的Class属性
	*/
	void SetLeftButtonBottomLabelClass(const std::wstring& classValue);

	/** 设置右侧按钮控件的Class属性
	*/
	void SetRightButtonClass(const std::wstring& classValue);

	/** 设置下拉列表容器的Class属性
	*/
	void SetComboBoxClass(const std::wstring& classValue);

public:
	/** 获取下拉列表的容器接口
	*/
	Box* GetComboBox() const;

	/** 获取按钮控件
	*/
	ButtonVBox* GetLeftButtonBox() const;

	/** 获取按钮控件上侧的文本控件
	*/
	Label* GetLabelTop() const;

	/** 获取按钮控件下侧的文本控件
	*/
	Label* GetLabelBottom() const;

	/** 按钮控件
	*/
	Button* GetRightButton() const;

	/** 更新下拉列表窗口的位置
	*/
	void UpdateComboWndPos();

public:
	/** 监听子项按钮点击事件（左侧的按钮被点击）
	 * @param[in] callback 触发的回调函数
	 */
	void AttachClick(const EventCallback& callback) { AttachEvent(kEventClick, callback);}

	/** 监听下拉窗关闭事件
	 * @param[in] callback 下拉窗关闭后触发的回调函数，WPARAM 为1表示取消，为0表示正常关闭
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

	/** 下拉框窗口关闭
	* @param [in] bCanceled true表示取消，否则表示正常关闭
	*/
	virtual void OnComboWndClosed(bool bCanceled);

	/** 左侧按钮点击事件
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnLeftButtonClicked(const EventArgs& args);

	/** 右侧按钮鼠标按下事件
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnRightButtonDown(const EventArgs& args);

	/** 右侧按钮点击事件
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnRightButtonClicked(const EventArgs& args);

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

protected:
	/** 初始化函数
	*/
	virtual void OnInit() override;

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

	/** 按钮的状态发生变化，同步状态
	* @param[in] args 参数列表
	* @return 始终返回 true
	*/
	bool OnButtonStateChanged(const EventArgs& args);

private:
	/** 下拉列表的窗口接口
	*/
	ComboButtonWnd* m_pWindow;

	/** 下拉列表的大小（宽度和高度）
	*/
	UiSize m_szDropBox;

	/** 下拉列表是否向上弹出
	*/
	bool m_bPopupTop;

private:
	/** 下拉列表表容器
	*/
	Box* m_pComboBox;

	/** 按钮容器控件ButtonVBox
	*/
	ButtonVBox* m_pLeftButton;

	/** 按钮容器中上侧的文本控件(位于ButtonVBox里面)
	*/
	Label* m_pLabelTop;

	/** 按钮容器中下侧的文本控件(位于ButtonVBox里面)
	*/
	Label* m_pLabelBottom;

	/** 按钮控件（右侧下拉按钮）
	*/
	Button* m_pRightButton;

	/** 鼠标按下的时候，是否正在显示下拉列表
	*/
	bool m_bDropListShown;
};

} // namespace ui

#endif // UI_CONTROL_COMBO_BUTTON_H_
