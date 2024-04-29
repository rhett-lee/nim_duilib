#ifndef UI_UTILS_SHADOW_WND_H__
#define UI_UTILS_SHADOW_WND_H__

#pragma once

#include "duilib/Utils/WinImplBase.h"

namespace ui {

/** 使用附加阴影窗口的基类，实现了创建窗口并附加的功能，提供没有 WS_EX_LAYERED 属性的窗口继承
 * 该类实现，封装了3种窗口模式：
 * （1）正常模式，不附加阴影的窗口：可以通过XML配置文件<Window>如下配置（举例）：
 *             shadow_attached="false" layered_window="false" ，不需要配置alpha属性，不支持alpha属性;
 * （2）附加阴影窗口模式，需要使用层窗口(带有WS_EX_LAYERED属性)，可以通过XML配置文件<Window>如下配置（举例）：
 *             shadow_attached="true" layered_window="true" ，alpha为可选配置，设置窗口透明度，如果不设置，默认为255。
 * （3）附加阴影窗口模式，不需要使用层窗口(不带有WS_EX_LAYERED属性)，可以通过XML配置文件<Window>如下配置（举例）：
 *             shadow_attached="true" layered_window="false" ，不需要配置alpha属性，不支持alpha属性;
 *             上述配置等同于：layered_window="false"，不设置shadow_attached属性。
 */
class ShadowWndBase;
class ShadowWnd: public ui::WindowImplBase
{
public:
	ShadowWnd();

	/**@brief 创建窗口
	 * @param[in] hwndParent 父窗口句柄
	 * @param[in] windowName 窗口名称
	 * @param[in] dwStyle 窗口样式
	 * @param[in] dwExStyle 窗口拓展样式, 可以设置层窗口（WS_EX_LAYERED）等属性
	 * @param[in] rc 窗口大小
	 */
	virtual bool CreateWnd(HWND hwndParent,
						   const wchar_t* windowName,
						   uint32_t dwStyle,
						   uint32_t dwExStyle,
						   const ui::UiRect& rc = ui::UiRect(0, 0, 0, 0)) override;

private:
	/** 附加窗口阴影
	*/
	virtual Box* AttachShadow(Box* pRoot) override;

private:
	/** 判断是否需要创建透明窗口的跟随阴影
	*/
	bool NeedCreateShadowWnd() const;

private:
	//阴影窗口，透明的
	ShadowWndBase* m_pShadowWnd;
};
} // namespace ui

#endif // UI_UTILS_SHADOW_WND_H__
