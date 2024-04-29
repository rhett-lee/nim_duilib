#ifndef UI_CONTROL_COLORPICKER_CUSTOM_H_
#define UI_CONTROL_COLORPICKER_CUSTOM_H_

#pragma once

#include "duilib/Core/Box.h"
#include "duilib/Control/ColorControl.h"
#include "duilib/Control/ColorSlider.h"
#include "duilib/Control/RichEdit.h"

namespace ui
{
/** 颜色选择器：自定义颜色
*/
class ColorPickerRegular;
class ColorPickerCustom : public Box
{
public:
	ColorPickerCustom();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;

	/** 选择一个颜色
	*/
	void SelectColor(const UiColor& color);

	/** 设置控件位置
	*/
	virtual void SetPos(UiRect rc) override;

	/** 监听选择颜色的事件
	* @param[in] callback 选择颜色变化时的回调函数
	*            参数说明:
						wParam: 当前新选择的颜色值，可以用UiColor((uint32_t)wParam)生成颜色
						lParam: 原来旧选择的颜色值，可以用UiColor((uint32_t)lParam)生成颜色
	*/
	void AttachSelectColor(const EventCallback& callback) { AttachEvent(kEventSelectColor, callback); }

private:
	/** 颜色变化的原因
	*/
	enum class ChangeReason
	{
		ColorUpdate,	//颜色值从其他页面更新
		ColorSpectrum,	//颜色光谱图
		ColorRegular,	//标准颜色表
		NewColorEdit,	//新颜色的可编辑框

		ColorARGB_A,	//ARGB颜色变化
		ColorARGB_R,	//ARGB颜色变化
		ColorARGB_G,	//ARGB颜色变化
		ColorARGB_B,	//ARGB颜色变化

		ColorHSV_H,		//HSV颜色变化
		ColorHSV_S,		//HSV颜色变化
		ColorHSV_V,		//HSV颜色变化

		ColorHSL_H,		//HSL颜色变化
		ColorHSL_S,		//HSL颜色变化
		ColorHSL_L,		//HSL颜色变化
	};

	/** 初始化
	*/
	void InitPicker();

	/** 颜色发生变化
	*/
	void OnColorChanged(WPARAM wParam, LPARAM lParam, ChangeReason reason);

	/** 判断输入是否为合法的颜色字符串，格式如"#FF123456"
	*/
	bool IsValidColorString(const std::wstring& colorText) const;

private:
	/** 一个颜色通道的界面控件
	*/
	struct ColorUI
	{
		//颜色值显示控件
		RichEdit* m_pColorEdit = nullptr;

		//颜色值调整控件
		ColorSlider* m_pColorSlider = nullptr;
	};

	/** 初始化一个RGB通道
	*/
	void InitRGB(const ColorUI& colorUI, ChangeReason reason);

	/** 初始化一个HSV通道
	*/
	void InitHSV(const ColorUI& colorUI, int32_t maxValue, ChangeReason reason);

	/** 初始化一个HSL通道
	*/
	void InitHSL(const ColorUI& colorUI, int32_t maxValue, ChangeReason reason);

	/** 更新RGB
	* @param [in] flag: 0 - A, 1 - R, 2 - G, 3 - B
	*/
	void UpdateRGB(const ColorUI& colorUI, const UiColor& color, int32_t flag);

	/** 更新HSV
	*/
	void UpdateHSV(const ColorUI& colorUIH, const ColorUI& colorUIS, const ColorUI& colorUIV, const UiColor& color, ChangeReason reason);

	/** 更新HSL
	*/
	void UpdateHSL(const ColorUI& colorUIH, const ColorUI& colorUIS, const ColorUI& colorUIL, const UiColor& color, ChangeReason reason);

	/** RGB颜色发生变化
	*/
	void OnRGBChanged(ChangeReason reason);

	/** HSV颜色发生变化
	*/
	void OnHSVChanged(ChangeReason reason);

	/** HSL颜色发生变化
	*/
	void OnHSLChanged(ChangeReason reason);

private:
	/** 是否已经完成初始化
	*/
	bool m_bPickerInited;

	/** 旧的颜色
	*/
	UiColor m_oldColor;

	/** 标准颜色控件接口
	*/
	ColorPickerRegular* m_pRegularPicker;

	/** 色谱控件
	*/
	ColorControl* m_pSpectrumControl;

	/** 新选择的颜色控件文本框接口
	*/
	RichEdit* m_pNewColorEdit;

private:
	/** RGB颜色调整控件
	*/
	ColorUI m_rgbA;
	ColorUI m_rgbR;
	ColorUI m_rgbG;
	ColorUI m_rgbB;

	/** HSV颜色调整控件
	*/
	ColorUI m_hsvH;
	ColorUI m_hsvS;
	ColorUI m_hsvV;

	/** HSL颜色调整控件
	*/
	ColorUI m_hslH;
	ColorUI m_hslS;
	ColorUI m_hslL;
};

}//namespace ui

#endif //UI_CONTROL_COLORPICKER_CUSTOM_H_
