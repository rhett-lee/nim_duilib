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

/** DPI感知模式
*/
enum class UILIB_API DpiAwarenessMode
{
	/** 无法感知
	 *    应用程序的 DPI 视图：所有显示器均为 96 DPI
	 *	  DPI 更改时的行为：位图拉伸（模糊）
	 */
	kDpiUnaware = 0,

	/** 系统
	 *  引入的Windows版本：Vista
	 *  应用程序的 DPI 视图：所有显示器均为 96 DPI
	 *	DPI 更改时的行为：位图拉伸（模糊））
	 */
	kSystemDpiAware = 1,

	/** 每显示器
	 *  引入的Windows版本：8.1
	 *  应用程序的 DPI 视图：应用程序窗口主要位于的显示器的 DPI
	 *	DPI 更改时的行为：通知顶级 HWND DPI 更改，没有任何 UI 元素的 DPI 缩放。
	 */
	kPerMonitorDpiAware = 2,

	/** 每显示器 V2
	 *  引入的Windows版本：Windows 10 创意者更新 (1703)
	 *  应用程序的 DPI 视图：应用程序窗口主要位于的显示器的 DPI
	 *	DPI 更改时的行为：通知顶级 HWND DPI 更改，自动 DPI 缩放：非工作区、常用控件中的主题绘制位图 (comctl32 V6)、对话框 (CreateDialog)。
	 */
	kPerMonitorDpiAware_V2 = 3
};

/** DPI感知功能初始化参数
*/
class UILIB_API DpiInitParam
{
public:
	enum class DpiAwarenessFlag
	{
		kFromManifest,  //从可执行程序的manifest配置中读取，不需要再次设置
		kFromUserDefine //通过参数设置（即：DpiInitParam::m_dpiAwarenessMode 成员变量）
	};
	enum class DpiFlag
	{
		kFromSystem,    //从操作系统的DPI配置中读取
		kFromUserDefine //通过参数设置（即：DpiInitParam::m_uDPI 成员变量）
	};

	/** DPI感知参数类型（默认是通过 m_dpiAwarenessMode 值设置的）
	*/
	DpiAwarenessFlag m_dpiAwarenessFlag = DpiInitParam::DpiAwarenessFlag::kFromUserDefine;

	/** 参数设置的DPI感知参数模式(仅当 m_dpiAwarenessFlag 为 kFromUserDefine时有效)
	*/
	DpiAwarenessMode m_dpiAwarenessMode = DpiAwarenessMode::kPerMonitorDpiAware_V2;

	/** DPI值的参数类型（默认读取系统设置的DPI值，不读取m_uDPI值）
	*/
	DpiFlag m_dpiFlag = DpiInitParam::DpiFlag::kFromSystem;

	/** 参数设置的DPI值(仅当 m_dpiFlag 为 kFromUserDefine时有效)
	*/
	uint32_t m_uDPI = 96;
};

/** DPI感知功能的接口
*/
class UILIB_API DpiManager
{
public:
	DpiManager();
	~DpiManager();
	DpiManager(const DpiManager&) = delete;
	DpiManager& operator = (const DpiManager&) = delete;

public:
	/** 初始化DPI感知模式和DPI值（该函数只可调用一次，后续调用被忽略）
	* @param [in] initParam 初始化参数，详见参数说明
	*/
	bool InitDpiAwareness(const DpiInitParam& initParam);
	
	/** 获取进程的DPI感知模式
	*/
	DpiAwarenessMode GetDpiAwareness() const;

public:
	/** 设置DPI值，并更新界面缩放比
	*   DPI值会影响界面缩放比，常用的值关系如下：
	*   DPI 值    缩放百分比(Scale值)
	* -------------------------------
	*   96        100%
	*   120       125%
	*   144       150%
	*   168       175%
	*   192       200%
	* @param [in] uDPI DPI值，如果设置为96，则代表界面无缩放
	*/
	void SetDPI(uint32_t uDPI);

	/** 获取DPI值
	*/
	uint32_t GetDPI() const;

	/** 获取当前界面缩放比（100代表无缩放）
	* @return 缩放比，比如：如果返回缩放比为125，则对应DPI值是120
	*/
	uint32_t GetScale() const;

	/** 当前界面是否有DPI缩放
	* @return 如果当前界面缩放比为100返回false，否则返回true
	*/
	bool IsScaled() const;

public:
	/** 根据界面缩放比来缩放整数
	* @param[in] iValue 整数
	* @return int 缩放后的值
	*/
	int32_t ScaleInt(int32_t& iValue) const;
	int32_t GetScaleInt(int32_t iValue) const;
	uint32_t GetScaleInt(uint32_t iValue) const;

	/** 根据界面缩放比来缩放SIZE
	* @param[in] size 需要缩放的Size引用
	*/
	void ScaleSize(SIZE& size) const;
	void ScaleSize(UiSize& size) const;

	/** 根据界面缩放比来缩放POINT
	* @param[in] point 需要缩放的point引用
	*/
	void ScalePoint(POINT& point) const;
	void ScalePoint(UiPoint& point) const;

	/** 根据界面缩放比来缩放RECT
	* @param[in] rect 需要缩放的rect引用
	* @return void	无返回值
	*/
	void ScaleRect(RECT& rect) const;
	void ScaleRect(UiRect& rect) const;
	void ScalePadding(UiPadding& padding) const;
	void ScaleMargin(UiMargin& margin) const;

private:

	/** 设置进程的DPI感知模式
	   （DPI感知模式只允许设置一次，设置成功后不允许修改；如果在可执行程序的manifest配置中设置了感知模式，也无法更改）
	* @param [in] dpiAwarenessMode 期望的DPI感知模式，总体策略是逐步降级设置
	*             依次为：kPerMonitorDpiAware_V2 -> kPerMonitorDpiAware -> kSystemDpiAware
	* @return 返回实际的进程DPI感知模式
	*/
	DpiAwarenessMode SetDpiAwareness(DpiAwarenessMode dpiAwarenessMode) const;

	/** 获取当前进程的DPI感知模式
	*/
	DpiAwarenessMode GetDpiAwarenessMode() const;

	/** 获取主显示器DPI，开启DPI感知后有效
	* @return 返回 DPI值
	*/
	uint32_t GetMainMonitorDPI();

	/** MulDiv 函数封装
	*/
	int32_t MulDiv(int32_t nNumber, int32_t nNumerator, int32_t nDenominator) const;

private:
	/** 是否已经初始化过
	*/
	bool m_bDpiInited;

	/** 当前进程的DPI感知模式
	*/
	DpiAwarenessMode m_dpiAwarenessMode;

	/** 当前的DPI值
	*/
	uint32_t m_uDpi;

	/** DPI缩放因子，100表示无缩放
	*/
	uint32_t m_nScaleFactor;
};
}
#endif //UI_UTILS_DPIMANAGER_H_