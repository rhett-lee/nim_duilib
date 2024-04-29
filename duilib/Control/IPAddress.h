#ifndef UI_CONTROL_IPADDRESS_H_
#define UI_CONTROL_IPADDRESS_H_

#pragma once

#include "duilib/Box/HBox.h"

namespace ui
{
/** IP地址控件
*/
class RichEdit;
class IPAddress: public HBox
{
public:
	IPAddress();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;
	virtual void SetAttribute(const std::wstring& strName, const std::wstring& strValue) override;

	/** 让控件获取焦点
	 */
	virtual void SetFocus() override;

	/** 设置IP地址
	*/
	void SetIPAddress(const std::wstring& ipAddress);

	/** 获取IP地址
	*/
	std::wstring GetIPAddress() const;

protected:

	/** 初始化接口
	*/
	virtual void OnInit() override;

	/** @brief 控件统一的消息处理入口，将传统 Windows 消息转换为自定义格式的消息
	 * @param[in] eventType	消息内容
	 * @param[in] wParam	消息附加内容
	 * @param[in] lParam	消息附加内容
	 * @param[in] tChar		按键信息
	 * @param[in] mousePos	鼠标信息
	 */
	virtual void SendEvent(EventType eventType,
			        	   WPARAM wParam = 0, 
						   LPARAM lParam = 0, 
						   TCHAR tChar = 0, 
						   const UiPoint& mousePos = UiPoint()) override;

	/** @brief 将转换后的消息派发到消息处理函数
	 * @param[in] msg 消息内容
	 */
    virtual void SendEvent(const EventArgs& msg) override;

	/** 编辑框的焦点转移时间
	*/
	void OnKillFocusEvent(RichEdit* pRichEdit, Control* pNewFocus);

private:
	/** 编辑框列表
	*/
	std::vector<RichEdit*> m_editList;

	/** 上次焦点在哪个控件上
	*/
	RichEdit* m_pLastFocus;

	/** IP地址
	*/
	UiString m_ipAddress;
};

}//namespace ui

#endif //UI_CONTROL_IPADDRESS_H_
