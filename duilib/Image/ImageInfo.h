#ifndef UI_IMAGE_IMAGE_INFO_H_
#define UI_IMAGE_IMAGE_INFO_H_

#pragma once

#include "duilib/Render/IRender.h"
#include "duilib/Core/UiTypes.h"

namespace ui 
{
	class IRender;
	class Control;

/** 图片信息
*/
class UILIB_API ImageInfo
{
public:
	ImageInfo();
	~ImageInfo();

	ImageInfo(const ImageInfo&) = delete;
	ImageInfo& operator = (const ImageInfo&) = delete;

public:
	/** 设置图片路径
	*/
	void SetImageFullPath(const std::wstring& path);

	/** 获取图片路径
	*/
	std::wstring GetImageFullPath() const;

	/** 设置该图片的大小是否已经做过适应DPI处理
	*/
	void SetBitmapSizeDpiScaled(bool isDpiScaled) { m_bDpiScaled = isDpiScaled; }

	/** 判断该图片的大小是否已经做过适应DPI处理
	*/
	bool IsBitmapSizeDpiScaled() const { return m_bDpiScaled; }

	/** 设置图片的宽和高
	*/
	void SetImageSize(int32_t nWidth, int32_t nHeight);

	/** 获取图片宽度
	*/
	int32_t GetWidth() const { return m_nWidth; }

	/** 获取图片高度
	*/
	int32_t GetHeight() const { return m_nHeight; }

	/** 添加一个图片帧数据, 添加后该资源由该类内部托管
	*/
	void SetFrameBitmap(const std::vector<IBitmap*>& frameBitmaps);

	/** 获取一个图片帧数据
	*/
	IBitmap* GetBitmap(uint32_t nIndex) const;

	/** 设置图片的多帧播放事件间隔（毫秒为单位 ）
	*/
	void SetFrameInterval(const std::vector<int32_t>& frameIntervals);

	/** 获取图片帧对应的播放时间间隔（毫秒为单位 ）
	*/
	int32_t GetFrameInterval(uint32_t nIndex) const;

	/** 获取图片的帧数
	*/
	uint32_t GetFrameCount() const;

	/** 是否位多帧图片(比如GIF等)
	*/
	bool IsMultiFrameImage() const;

	/** 设置循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
	*/
	void SetPlayCount(int32_t nPlayCount);

	/** 获取循环播放次数
	*@return 返回值：-1 表示未设置
	*               0  表示动画是一致循环播放的
	*              > 0 表示动画循环播放的具体次数
	*/
	int32_t GetPlayCount() const;

	/** 设置图片的缓存KEY, 用于图片的生命周期管理
	*/
	void SetCacheKey(const std::wstring& cacheKey);

	/** 获取图片的缓存KEY
	*/
	std::wstring GetCacheKey() const;

private:
	//图片的完整路径
	UiString m_imageFullPath;

	//该图片的大小是否已经做过适应DPI处理
	bool m_bDpiScaled;

	//图片的宽度
	int32_t m_nWidth;
	
	//图片的高度
	int32_t m_nHeight;

	//图片帧对应的播放时间间隔（毫秒为单位 ）
	std::vector<int32_t>* m_pFrameIntervals;

	//图片帧数据
	IBitmap** m_pFrameBitmaps;

	//图片帧数量
	uint32_t m_nFrameCount;

	//循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
	int32_t m_nPlayCount;

	/** 图片的缓存KEY, 用于图片的生命周期管理
	*/
	UiString m_cacheKey;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_INFO_H_
