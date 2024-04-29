#ifndef UI_CORE_STATE_COLOR_MAP_H_
#define UI_CORE_STATE_COLOR_MAP_H_

#pragma once

#include "duilib/Render/IRender.h"
#include "duilib/Core/UiTypes.h"
#include <map>

namespace ui 
{
/** 控件状态与颜色值的映射
*/
class Control;
class IRender;
class UILIB_API StateColorMap
{
public:
	StateColorMap();

	/** 设置管理的控件接口
	*/
	void SetControl(Control* control);

	/** 获取颜色值，如果不包含此颜色，则返回空
	*/
	std::wstring GetStateColor(ControlStateType stateType) const;

	/** 设置颜色值
	*/
	void SetStateColor(ControlStateType stateType, const std::wstring& color);

	/** 是否包含Hot状态的颜色
	*/
	bool HasHotColor() const;

	/** 是否含有指定颜色值
	*/
	bool HasStateColor(ControlStateType stateType) const;

	/** 是否含有颜色值
	*/
	bool HasStateColors() const;

	/** 绘制指定状态的颜色
	*/
	void PaintStateColor(IRender* pRender, UiRect rcPaint, ControlStateType stateType) const;

private:
	/** 关联的控件接口
	*/
	Control* m_pControl;

	/** 状态与颜色值的映射表
	*/
	std::map<ControlStateType, UiString> m_stateColorMap;
};

} // namespace ui

#endif // UI_CORE_STATE_COLOR_MAP_H_
