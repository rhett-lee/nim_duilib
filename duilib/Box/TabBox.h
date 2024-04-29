#ifndef UI_CORE_TABBOX_H_
#define UI_CORE_TABBOX_H_

#pragma once

#include "duilib/Core/Box.h"

namespace ui
{

class UILIB_API TabBox : public Box
{
public:
	explicit TabBox(Layout* pLayout = new Layout());

	// 用于初始化xml属性
	virtual void OnInit() override;

	/// 重写父类方法，提供个性化功能，请参考父类声明
	virtual std::wstring GetType() const override;
	virtual bool AddItem(Control* pControl) override;
	virtual bool AddItemAt(Control* pControl, size_t iIndex) override;
	virtual bool RemoveItem(Control* pControl) override;
	virtual bool RemoveItemAt(size_t iIndex) override;
	virtual void RemoveAllItems() override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

	/**
	 * @brief 获取当前选择项
	 * @return 返回当前选择项索引(如果无有效索引，则返回Box::InvalidIndex)
	 */
	size_t GetCurSel() const;

	/**
	 * @brief 根据控件索引选择一个子项
	 * @param[in] iIndex 要选择的子项索引
	 * @return 成功返回 true，否则返回 false
	 */
	bool SelectItem(size_t iIndex);

	/**
	 * @brief 根据控件指针选择一个子项
	 * @param[in] pControl 要选择的子项指针
	 * @return 成功返回 true，否则返回 false
	 */
	bool SelectItem(Control* pControl);

	/**
	 * @brief 根据控件名称选择一个子项
	 * @param[in] pControlName 要选择的控件名称
	 * @return 成功返回 true，否则返回 false
	 */
	bool SelectItem(const std::wstring& pControlName);

	/**
	 * @brief 设置是否显示动画效果
	 * @param[in] bFadeSwitch 设置为 true 则显示，false 为不显示动画
	 * @return 无
	 */
	void SetFadeSwitch(bool bFadeSwitch);

	/**
	 * @brief 判断是否需要显示动画效果
	 * @return 返回 true 显示动画，false 为不显示动画效果
	 */
	bool IsFadeSwitch() const { return m_bFadeSwith; }
	
	/** @brief 监听Tab页面选择事件
	  * @param[in] callback 事件处理的回调函数，请参考 EventCallback 声明
	  * @return 无
	  */
	void AttachTabSelect(const EventCallback& callback) { AttachEvent(kEventTabSelect, callback); }

protected:
	/**
	 * @brief 显示一个 TAB 项时，处理一些属性
	 * @param[in] it TAB 项索引
	 * @return 无
	 */
	void OnShowTabItem(size_t index);

	/**
	 * @brief 隐藏一个 TAB 项时，处理一些属性
	 * @param[in] it TAB 项索引
	 * @return 无
	 */
	void OnHideTabItem(size_t index);

	/** 动画播放结束
	*@param[in] it TAB 项索引
	*/
	void OnAnimationComplete(size_t index);

private:
	//当前选择的Item下标
	size_t m_iCurSel;

	//初始化时需要选择的Item下表
	size_t m_iInitSel;

	//是否需要切换动画
	bool m_bFadeSwith;
};

}
#endif // UI_CORE_TABBOX_H_
