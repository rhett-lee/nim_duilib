#ifndef UI_IMAGE_STATE_IMAGE_H_
#define UI_IMAGE_STATE_IMAGE_H_

#pragma once

#include "duilib/Core/UiTypes.h"
#include <map>

namespace ui 
{
/** 控件状态与图片的映射
*/
class Control;
class Image;
class IRender;
class UILIB_API StateImage
{
public:
	StateImage();
	~StateImage();

	/** 设置关联的控件接口
	*/
	void SetControl(Control* pControl);

	/** 设置图片属性
	*@param [in] stateType 图片类型
	*@param [in] strImageString 图片属性字符串
	*/
	void SetImageString(ControlStateType stateType, const std::wstring& strImageString);

	/** 获取图片属性
	*@param [in] stateType 图片类型
	*/
	std::wstring GetImageString(ControlStateType stateType) const;

	/** 获取图片文件名
	*@param [in] stateType 图片类型
	*/
	std::wstring GetImagePath(ControlStateType stateType) const;

	/** 获取图片的源区域大小
	*@param [in] stateType 图片类型
	*/
	UiRect GetImageSourceRect(ControlStateType stateType) const;

	/** 获取图片的透明度
	*@param [in] stateType 图片类型
	*/
	int32_t GetImageFade(ControlStateType stateType) const;

	/** 获取图片接口(可读，可写)
	*/
	Image* GetStateImage(ControlStateType stateType) const;

public:
	/** 是否包含Hot状态的图片
	*/
	bool HasHotImage() const;

	/** 是否包含状态图片
	*/
	bool HasImage() const;

	/** 绘制指定状态的图片
	* @param [in] pRender 绘制接口
	* @param [in] stateType 控件状态，用于选择绘制哪个图片
	* @param [in] sImageModify 图片的附加属性
	* @param [out] pDestRect 返回图片绘制的最终目标矩形区域
	* @return 绘制成功返回true, 否则返回false
	*/
	bool PaintStateImage(IRender* pRender, ControlStateType stateType, 
					     const std::wstring& sImageModify = L"",
		                 UiRect* pDestRect = nullptr);

	/** 获取用于估算Control控件大小（宽和高）的图片接口
	*/
	Image* GetEstimateImage() const;

	/** 获取所有图片接口
	*/
	void GetAllImages(std::vector<Image*>& allImages) const;

	/** 清空图片缓存，释放资源
	*/
	void ClearImageCache();

	/** 停止所有图片的动画
	*/
	void StopGifPlay();

private:
	//关联的控件接口
	Control* m_pControl;

	//每个状态的图片接口
	std::map<ControlStateType, Image*> m_stateImageMap;
};

} // namespace ui

#endif // UI_IMAGE_STATE_IMAGE_H_
