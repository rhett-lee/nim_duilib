#ifndef UI_UTILS_DPIMANAGER_H_
#define UI_UTILS_DPIMANAGER_H_

#pragma once

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** @class DpiManager
  * @brief DPI适配管理类
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2016/10/10
  */
class UILIB_API DpiManager
{
public:
	DpiManager();
	~DpiManager();
	DpiManager(const DpiManager&) = delete;
	DpiManager& operator = (const DpiManager&) = delete;

public:
	/** 设置是否支持DPI自适应，并设置默认缩放比
	* @param [in] bAdaptDPI true表示支持DPI自适应，false表示不支持DPI自适应
	*/
	bool SetAdaptDPI(bool bAdaptDPI);

	/** 是否支持适配DPI缩放效果(默认不支持，需要调用SetAdaptDPI设置才能支持)
	*/
	bool IsAdaptDPI() const;

	/** 获取当前界面缩放比（100代表无缩放）
	* @return 缩放比
	*/
	uint32_t GetScale() const;

	/** 当前界面是否有DPI缩放
	* @return 如果当前界面缩放比为100返回false，否则返回true
	*/
	bool IsScaled() const;

	/** 根据DPI值设置界面缩放比（只有IsAdaptDPI()为true的时候有效）
	* @param[in] uDPI DPI值
	*/
	bool SetScale(uint32_t uDPI);

	/** 根据界面缩放比来缩放整数
	* @param[in] iValue 整数
	* @return int 缩放后的值
	*/
	int32_t ScaleInt(int32_t& iValue);
	int32_t GetScaleInt(int32_t iValue);
	uint32_t GetScaleInt(uint32_t iValue);

	/** 根据界面缩放比来缩放SIZE
	* @param[in] size 需要缩放的Size引用
	*/
	void ScaleSize(SIZE& size);
	void ScaleSize(UiSize& size);

	/** 根据界面缩放比来缩放POINT
	* @param[in] point 需要缩放的point引用
	*/
	void ScalePoint(POINT& point);
	void ScalePoint(UiPoint& point);

	/** 根据界面缩放比来缩放RECT
	* @param[in] rect 需要缩放的rect引用
	* @return void	无返回值
	*/
	void ScaleRect(RECT& rect);
	void ScaleRect(UiRect& rect);
	void ScalePadding(UiPadding& padding);
	void ScaleMargin(UiMargin& margin);

private:
	/** 获取某个显示器的DPI，开启DPI感知后有效
	* @param[in] HMONITOR句柄
	* @return 返回 DPI值
	*/
	uint32_t GetMonitorDPI(HMONITOR hMonitor);

	/** 获取主显示器DPI，开启DPI感知后有效
	* @return 返回 DPI值
	*/
	uint32_t GetMainMonitorDPI();

	/** MulDiv 函数封装
	*/
	int32_t MulDiv(int32_t nNumber, int32_t nNumerator, int32_t nDenominator);

private:
	//DPI缩放因子，100表示无缩放
	uint32_t m_nScaleFactor;

	//是否开启DPI自适应
	bool m_bAdaptDPI;
};
}
#endif //UI_UTILS_DPIMANAGER_H_