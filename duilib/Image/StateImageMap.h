#ifndef UI_IMAGE_STATE_IMAGE_MAP_H_
#define UI_IMAGE_STATE_IMAGE_MAP_H_

#pragma once

#include "duilib/Image/StateImage.h"

namespace ui 
{
/** 控件图片类型与状态图片的映射
*/
class UILIB_API StateImageMap
{
public:
	StateImageMap();

	/** 设置关联的控件接口
	*/
	void SetControl(Control* pControl);

	/** 设置图片属性
	*@param [in] stateImageType 图片类型，比如正常状态前景图片、背景图片；选择状态的前景图片、背景图片等
	*@param [in] stateType 图片状态，比如正常、焦点、按下、禁用状态等
	*@param [in] strImagePath 图片属性字符串
	*/
	void SetImageString(StateImageType stateImageType, ControlStateType stateType, const std::wstring& strImagePath);

	/** 获取图片属性
	*@param [in] stateImageType 图片类型，比如正常状态前景图片、背景图片；选择状态的前景图片、背景图片等
	*@param [in] stateType 图片状态，比如正常、焦点、按下、禁用状态等
	*/
	std::wstring GetImageString(StateImageType stateImageType, ControlStateType stateType) const;

	/** 是否含有Hot状态的图片
	*/
	bool HasHotImage() const;

	/** 是否状态图片
	*/
	bool HasStateImages(void) const;

	/** 是否含有指定类型的图片
	*/
	bool HasStateImage(StateImageType stateImageType) const;

	/** 绘制指定类型、指定状态的图片
	* @param [in] pRender 绘制接口
	* @param [in] stateImageType 图片类型
	* @param [in] stateType 控件状态，用于选择绘制哪个图片
	* @param [in] sImageModify 图片的附加属性
	* @param [out] pDestRect 返回图片绘制的最终目标矩形区域
	* @return 绘制成功返回true, 否则返回false
	*/
	bool PaintStateImage(IRender* pRender, 
						 StateImageType stateImageType, 
					     ControlStateType stateType, 
		                 const std::wstring& sImageModify = L"",
		                 UiRect* pDestRect = nullptr);
	
	/** 获取用于估算Control控件大小（宽和高）的图片接口
	*/
	Image* GetEstimateImage(StateImageType stateImageType) const;

	/** 获取指定图片类型和状态的图片接口
	*/
	Image* GetStateImage(StateImageType stateImageType, ControlStateType stateType) const;

	/** 获取所有图片接口
	*/
	void GetAllImages(std::vector<Image*>& allImages) const;

	/** 清除所有图片类型的缓存，释放资源
	*/
	void ClearImageCache();

	/** 停止所有图片的动画
	*/
	void StopGifPlay();

private:
	//关联的控件接口
	Control* m_pControl;

	//每个图片类型的状态图片(正常状态前景图片、背景图片；选择状态的前景图片、背景图片)
	std::map<StateImageType, StateImage> m_stateImageMap;
};

} // namespace ui

#endif // UI_IMAGE_STATE_IMAGE_MAP_H_
