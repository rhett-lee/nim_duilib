#ifndef UI_UTILS_DPIMANAGER_H_
#define UI_UTILS_DPIMANAGER_H_

#pragma once

#include "duilib/duilib_defs.h"

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
	/**
	* @brief 获取某个显示器的DPI，开启DPI感知后有效
	* @param[in] HMONITOR句柄
	* @return 返回 DPI值
	*/
	static uint32_t GetMonitorDPI(HMONITOR hMonitor);

	/**
	* @brief 获取主显示器DPI，开启DPI感知后有效
	* @return 返回 DPI值
    */
	static uint32_t GetMainMonitorDPI();

public:
	/** 获取单例对象
	*/
	static DpiManager* GetInstance();

	/**
	* @brief 是否程序自己适配DPI缩放效果
	* @return bool true 是，false 否
	*/
	bool IsAdaptDPI() const;

	/**
	* @brief 设置是否支持DPI自适应
	* @param [in] bAdaptDPI true表示支持DPI自适应，false表示不支持DPI自适应
	* @return bool true 设置成功，false 设置失败
	*/
	bool SetAdaptDPI(bool bAdaptDPI);

	/**
	* @brief 获取当前界面缩放比（100代表无缩放）
	* @return 缩放比
	*/
	uint32_t GetScale() const;

	/**
	* @brief 根据DPI值设置界面缩放比，只有程序自己处理DPI缩放时才有效
	* @param[in] uDPI DPI值
	* @return void	无返回值
	*/
	void SetScale(uint32_t uDPI);

	/**
	* @brief 根据界面缩放比来缩放整数，只有程序自己处理DPI缩放时才有效
	* @param[in] iValue 整数
	* @return int	缩放后的值
	*/
	int ScaleInt(int &iValue);

	/**
	* @brief 根据界面缩放比来缩放SIZE，只有程序自己处理DPI缩放时才有效
	* @param[in] pSize SIZE指针
	* @return void	无返回值
	*/
	void ScaleSize(SIZE &size);
	void ScaleSize(UiSize &size);

	/**
	* @brief 根据界面缩放比来缩放POINT，只有程序自己处理DPI缩放时才有效
	* @param[in] pSize SIZE指针
	* @return void	无返回值
	*/
	void ScalePoint(POINT &point);
	void ScalePoint(UiPoint &point);

	/**
	* @brief 根据界面缩放比来缩放RECT，只有程序自己处理DPI缩放时才有效
	* @param[in] pSize SIZE指针
	* @return void	无返回值
	*/
	void ScaleRect(RECT &rect);
	void ScaleRect(UiRect &rect);

private:
	DpiManager();
	~DpiManager() {};
	DpiManager(const DpiManager&) = delete;
	DpiManager& operator = (const DpiManager&) = delete;

private:
	//DPI缩放因子，100表示无缩放
	uint32_t m_nScaleFactor;

	//是否开启DPI自适应
	bool m_bAdaptDPI;
};
}
#endif //UI_UTILS_DPIMANAGER_H_