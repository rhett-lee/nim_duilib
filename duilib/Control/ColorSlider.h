
#include "duilib/Control/Slider.h"

#ifndef UI_CONTROL_COLOR_SLIDER_H_
#define UI_CONTROL_COLOR_SLIDER_H_

#pragma once

namespace ui
{
/** 颜色类型
*/
enum class ColorMode
{
	kMode_ARGB,	//ARGB	
	kMode_HSV,	//HSV
	kMode_HSL	//HSL
};

/** 颜色调整方式
*/
enum class ColorAdjustMode
{
	kMode_ARGB_A, //ARGB模式，调整A
	kMode_ARGB_R, //ARGB模式，调整R
	kMode_ARGB_G, //ARGB模式，调整G
	kMode_ARGB_B, //ARGB模式，调整B
	kMode_HSV_H,  //HSV模式，调整H
	kMode_HSV_S,  //HSV模式，调整S
	kMode_HSV_V,  //HSV模式，调整V
	kMode_HSL_H,  //HSL模式，调整H
	kMode_HSL_S,  //HSL模式，调整S
	kMode_HSL_L   //HSL模式，调整L
};

/** HSV颜色
*/
class ColorHSV
{
public:
	uint16_t H = 0; //范围: [0, 359)
	uint8_t  S = 0; //范围: [0, 100]
	uint8_t  V = 0; //范围: [0, 100]
};

/** HSL颜色
*/
class ColorHSL
{
public:
	uint16_t H = 0; //范围: [0, 359)
	uint8_t  S = 0; //范围: [0, 100]
	uint8_t  L = 0; //范围: [0, 100]
};

/** 自定义颜色背景的Slider控件
*/
class ColorSlider: public Slider
{
public:
	ColorSlider();

	/** 获取控件类型
	*/
	virtual std::wstring GetType() const override;

	/** 设置颜色信息(ARGB格式的颜色)
	*/
	void SetColorInfo(const UiColor& color, ColorAdjustMode adjustMode);

	/** 设置颜色信息(HSV格式的颜色)
	*/
	void SetColorInfo(const ColorHSV& color, ColorAdjustMode adjustMode);

	/** 设置颜色信息(HSL格式的颜色)
	*/
	void SetColorInfo(const ColorHSL& color, ColorAdjustMode adjustMode);

	/** 绘制背景图片的入口函数
	* @param[in] pRender 指定绘制区域
	*/
	virtual void PaintBkImage(IRender* pRender) override;

private:

	/** 获取绘制的颜色位图接口
	* @param [in] rect 显示区域大小信息
	*/
	IBitmap* GetColorBitmap(const UiRect& rect);

	/** 获取ARGB格式的渐变颜色
	*/
	void GetARGB(uint32_t* buffer, int32_t samples,
				 const UiColor& start,
				 const UiColor& end) const;

private:
	/** 颜色位图
	*/
	std::unique_ptr<IBitmap> m_spBitmap;

	/** 颜色模式
	*/
	ColorMode m_colorMode;

	/** ARGB颜色值
	*/
	UiColor m_argbColor;

	/** HSV颜色值
	*/
	ColorHSV m_hsvColor;

	/** HSL颜色值
	*/
	ColorHSL m_hslColor;

	/** 颜色调整模式
	*/
	ColorAdjustMode m_adjustMode;
};

}//namespace ui

#endif //UI_CONTROL_COLOR_SLIDER_H_
