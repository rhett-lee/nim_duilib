#ifndef UI_CORE_TOOLTIP_H_
#define UI_CORE_TOOLTIP_H_

#pragma once

#include "duilib/Core/UiPoint.h"
#include "duilib/Core/UiRect.h"
#include <string>
#include <CommCtrl.h>

namespace ui 
{
/** ToolTip的实现
*/
class UILIB_API ToolTip
{
public:
    ToolTip();
	~ToolTip();
	
public:
	/**@brief 设置鼠标跟踪状态
	*/
	void SetMouseTracking(HWND hParentWnd, bool bTracking);

	/**@brief 显示ToolTip信息
	* @param [in] hParentWnd 主窗口句柄
	* @param [in] hModule 窗口关联的资源句柄
	* @param [in] rect Tooltip显示区域
	* @param [in] maxWidth Tooltip显示最大宽度
	* @param [in] trackPos 跟踪的位置
    * @param [in] text Tooltip显示内容
	*/
	void ShowToolTip(HWND hParentWnd, 
					 HMODULE hModule, 
					 const UiRect& rect, 
					 uint32_t maxWidth,
					 const UiPoint& trackPos,
					 const std::wstring& text);

	/**@brief 隐藏ToolTip信息
	*/
	void HideToolTip();

	/**@brief 清除鼠标跟踪状态
	*/
	void ClearMouseTracking();

private:
    //鼠标跟踪状态
	bool m_bMouseTracking;
	
	//Tooltip信息
	TOOLINFO m_ToolTip;
	
	//Tooltip窗口
	HWND m_hwndTooltip;
	
	//主窗口
	HWND m_hParentWnd;
};

} // namespace ui

#endif // UI_CORE_TOOLTIP_H_
