#ifndef UI_IMAGE_IMAGE_H_
#define UI_IMAGE_IMAGE_H_

#pragma once

#include "duilib/Image/ImageInfo.h"
#include "duilib/Image/ImageAttribute.h"
#include "duilib/Image/ImageLoadAttribute.h"
#include "duilib/Image/StateImageMap.h"
#include <memory>

namespace ui 
{
class Control;

/** 图片相关封装，支持的文件格式：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO
*/
class UILIB_API Image
{
public:
	Image();
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	/** 初始化图片属性
	*/
	void InitImageAttribute();

	/** 设置并初始化图片属性
	*@param [in] strImageString 图片属性字符串
	*/
	void SetImageString(const std::wstring& strImageString);

	/** 获取图片属性（含文件名，和图片设置属性等）
	*/
	std::wstring GetImageString() const;

	/** 判断图片属性是否相等
	*/
	bool EqualToImageString(const std::wstring& imageString) const;

	/** 获取图片文件名（含相对路径，不含图片属性）
	*/
	std::wstring GetImagePath() const;

	/** 设置图片属性的内边距
	*/
	void SetImagePadding(const UiPadding& newPadding);

	/** 获取图片属性的内边距
	*/
	UiPadding GetImagePadding() const;

	/** 判断是否禁用图片绘制
	*/
	bool IsImagePaintEnabled() const;

	/** 设置是否禁止背景图片绘制
	*/
	void SetImagePaintEnabled(bool bEnable);

	/** 获取图片属性（只读）
	*/
	const ImageAttribute& GetImageAttribute() const;

	/** 获取图片加载属性
	*/
	ImageLoadAttribute GetImageLoadAttribute() const;

public:
	/** 获取图片信息接口
	*/
	const std::shared_ptr<ImageInfo>& GetImageCache() const;

	/** 设置图片信息接口
	*/
	void SetImageCache(const std::shared_ptr<ImageInfo>& imageInfo);

	/** 清除图片信息缓存数据, 释放资源
	*/
	void ClearImageCache();

public:
	/** 设置图片属性：播放次数（仅当多帧图片时）
	*/
	void SetImagePlayCount(int32_t nPlayCount);

	/** 设置图片属性：透明度（仅当多帧图片时）
	*/
	void SetImageFade(uint8_t nFade);

	/** 是否正在播放中（仅当多帧图片时）
	*/
	bool IsPlaying() const { return m_bPlaying; }

	/** 设置是否正在播放中（仅当多帧图片时）
	*/
	void SetPlaying(bool bPlaying) { m_bPlaying = bPlaying; }

	/** 跳到下一帧（仅当多帧图片时）
	*/
	bool IncrementCurrentFrame();

	/** 设置当前图片帧（仅当多帧图片时）
	*/
	void SetCurrentFrame(uint32_t nCurrentFrame);

	/** 获取当前图片帧索引（仅当多帧图片时）
	*/
	uint32_t GetCurrentFrameIndex() const;

	/** 获取当前图片帧的图片数据
	*/
	IBitmap* GetCurrentBitmap() const;

	/** 获取当前图片帧播放的时间间隔（单位: 毫秒，仅当多帧图片时）
	*/
	int32_t GetCurrentInterval() const;

	/** 获取当前已循环播放的次数（仅当多帧图片时）
	*/
	int32_t GetCycledCount() const;

	/** 清空当前已循环播放的次数（仅当多帧图片时）
	*/
	void ClearCycledCount();

	/** 判断是否应该继续播放（仅当多帧图片时）
	*/
	bool ContinuePlay() const;

private:

	//当前正在播放的图片帧（仅当多帧图片时）
	uint32_t m_nCurrentFrame;

	//是否正在播放（仅当多帧图片时）
	bool m_bPlaying;

	//已播放次数（仅当多帧图片时）
	int32_t m_nCycledCount;

	//图片属性
	ImageAttribute m_imageAttribute;

	//图片信息
	std::shared_ptr<ImageInfo> m_imageCache;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_H_
