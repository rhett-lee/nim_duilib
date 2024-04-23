#ifndef UI_CONTROL_FILTERCOMBO_H_
#define UI_CONTROL_FILTERCOMBO_H_

#pragma once

#include "duilib/Control/Combo.h"

namespace ui 
{

/** 带有过滤功能的组合框
*/
class UILIB_API FilterCombo : public Combo
{
public:
	FilterCombo();
	FilterCombo(const FilterCombo& r) = delete;
	Combo& operator=(const FilterCombo& r) = delete;
	virtual ~FilterCombo();

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

protected:
	virtual void OnInit() override;
	
protected:
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

	/** Edit的文本内容发生变化
	 * @param[in] args 参数列表
	 * @return 始终返回 true
	 */
	virtual bool OnEditTextChanged(const ui::EventArgs& args);

private:

	/** 对下拉框列表里面的内容进行过滤
	*/
	void FilterComboList(const std::wstring& filterText);

	/** 判断文本是否满足过滤条件
	*/
	bool IsFilterText(const std::wstring& filterText, const std::wstring& itemText) const;
};

} // namespace ui

#endif // UI_CONTROL_FILTERCOMBO_H_
