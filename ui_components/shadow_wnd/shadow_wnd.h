#ifndef __SHADOW_WND_H__
#define __SHADOW_WND_H__

#pragma once

#include "ui_components/windows_manager/window_ex.h"
#include "duilib/Utils/WinImplBase.h"

namespace nim_comp {

/** @class ShadowWnd
 * @brief 一个附加到基础窗口周围的窗口，带有 WS_EX_LAYERED 属性来实现阴影
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @date 2019-03-22
 */
class ShadowWnd : public ui::WindowImplBase, public ui::IUIMessageFilter
{
public:
	ShadowWnd();

	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override;

	virtual LRESULT FilterMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
	
	virtual HWND Create(Window* window);
private:
	Window*	window_ = nullptr;
};

/** @class ShadowWnd
 * @brief 使用附加阴影窗口的基类，实现了创建窗口并附加的功能，提供没有 WS_EX_LAYERED 属性的窗口继承
 * @copyright (c) 2016, NetEase Inc. All rights reserved
 * @date 2019-03-22
 */
class ShadowWndBase : public ui::WindowImplBase
{
public:
	ShadowWndBase();

	virtual HWND CreateWnd(HWND hwndParent,
						   const wchar_t* windowName,
						   uint32_t dwStyle,
						   uint32_t dwExStyle,
						   bool isLayeredWindow = true,
						   const ui::UiRect& rc = ui::UiRect(0, 0, 0, 0)) override;

private:
	ShadowWnd* shadow_wnd_;
};
} // namespace ui

#endif // __SHADOW_WND_H__
