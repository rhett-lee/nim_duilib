#ifndef UI_CORE_CONTROL_LOADING_H_
#define UI_CORE_CONTROL_LOADING_H_

#pragma once

#include "duilib/Core/UiTypes.h"
#include "base/callback/callback.h"
#include <memory>

namespace ui 
{

/** 控件加载中状态的逻辑封装
*/
class Image;
class Control;
class IRender;
class ControlLoading: public nbase::SupportWeakCallback
{
public:
	explicit ControlLoading(Control* pControl);
	~ControlLoading();
	ControlLoading(const ControlLoading&) = delete;
	ControlLoading& operator = (const ControlLoading&) = delete;

public:
	/** 设置"加载中"图片的路径等配置信息
	* @return 如果变化返回true，否则返回false
	*/
	bool SetLoadingImage(const std::wstring& strImage);

	/** 设置loading背景色
	* @param[in] strColor 背景色
	* @return 如果变化返回true，否则返回false
	*/
	bool SetLoadingBkColor(const std::wstring& strColor);

	/** 绘制"加载中"图片
	*/
	void PaintLoading(IRender* pRender);

	/** 开启loading状态
	* @param[in] start_angle loading图片旋转的角度
	*/
	bool StartLoading(int32_t fStartAngle = -1);

	/** 关闭loading状态
	* @param[in] frame 播放完成停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifFrameType 枚举
	*/
	void StopLoading(GifFrameType frame = kGifFrameFirst);

	/** 是否正在loading
	* @return 在loading返回true, 反之返回false
	*/
	bool IsLoading() const;

private:
	/** 计算loading图片的旋转角度, 旋转图片
	*/
	void Loading();

private:
	//是否处于加载中的状态
	bool m_bIsLoading;

	//加载中图片旋转的角度（0-360）
	int32_t m_fCurrrentAngele;

	//加载中状态时显示的图片
	std::unique_ptr<Image> m_pLoadingImage;

	//加载中状态时的背景颜色
	UiString m_strLoadingBkColor;

	//加载中状态图片(m_pLoadingImage)的生命周期管理、取消机制
	nbase::WeakCallbackFlag m_loadingImageFlag;

	//关联的Control对象
	Control* m_pControl;
};

} // namespace ui

#endif // UI_CORE_CONTROL_LOADING_H_
