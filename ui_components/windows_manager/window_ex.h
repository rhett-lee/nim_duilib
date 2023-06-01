#pragma once

#include "duilib/Utils/ShadowWnd.h"

namespace nim_comp
{
/** @class WindowEx
 * @brief 所有窗体的基类
 * @copyright (c) 2015, NetEase Inc. All rights reserved
 * @date 2015/9/16
 */
class WindowEx : public ui::ShadowWnd
{
public:
	WindowEx();
	virtual ~WindowEx();

	/**
	 * 创建窗口
	 * @param[in] hwndParent 父窗口句柄
	 * @param[in] pstrName 窗口名称
	 * @param[in] dwStyle 窗口样式
	 * @param[in] dwExStyle 窗口扩展样式
	 * @param[in] isLayeredWindow 是否创建分层窗口
	 * @param[in] rc 窗口位置
	 * @return HWND 窗口句柄
	 */
	virtual HWND CreateWnd(HWND hwndParent,
						   const wchar_t* windowName,
						   uint32_t dwStyle,
						   uint32_t dwExStyle,
						   const ui::UiRect& rc = ui::UiRect(0, 0, 0, 0)) override;

	/**
	 * 处理窗口被销毁的消息
	 * @param[in] uMsg 消息
	 * @param[in] wParam 参数
	 * @param[in] lParam 参数
	 * @param[out] bHandled 消息是否被处理
	 * @return LRESULT 处理结果
	 */
	virtual LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

	/**
	 * 处理ESC键单击的消息
	 * @param[out] bHandled 消息是否被处理
	 * @return void 无返回值
	 */
	virtual void OnEsc(bool &bHandled);

	/**
	 * 获取窗口类名的接口
	 * @return wstring 窗口类名
	 */
	virtual std::wstring GetWindowClassName() const = 0;

	/**
	 * 获取窗口id的接口
	 * @return wstring 窗口id
	 */
	virtual std::wstring GetWindowId(void) const = 0;

	/**@brief 窗口消息的派发函数
	 * @param[in] uMsg 消息体
	 * @param[in] wParam 消息附加参数
	 * @param[in] lParam 消息附加参数
	 * @param[out] bHandled 消息是否已经处理，
				   返回 true  表明已经成功处理消息，不需要再传递给窗口过程；
				   返回 false 表示将消息继续传递给窗口过程处理
	 * @return 返回消息的处理结果
	 */
	virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

private:
	/**
	 * 从WindowManager中注册自己
	 * @return bool true 注册成功，false 注册失败
	 */
	bool RegisterWnd();

	/**
	 * 从WindowManager中反注册自己
	 * @return void 无返回值
	 */
	void UnRegisterWnd();
};

/**
* 获取弹出窗口可以显示在右下角位置的坐标
* @return POINT 窗口坐标
*/
POINT GetPopupWindowPos(WindowEx* window);

}