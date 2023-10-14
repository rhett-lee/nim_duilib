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

	/** 设置IP地址
	*/
	void SetIPAddress(const std::wstring& ipAddress);

	/** 获取IP地址
	*/
	std::wstring GetIPAddress() const;

protected:
	virtual void DoInit() override;

private:
	//初始化标志
	bool m_bInited;

	/** 编辑框列表
	*/
	std::vector<RichEdit*> m_editList;

	/** IP地址
	*/
	UiString m_ipAddress;
};

}//namespace ui

#endif //UI_CONTROL_IPADDRESS_H_
