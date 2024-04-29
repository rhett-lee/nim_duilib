#ifndef UI_IMAGE_IMAGE_H_
#define UI_IMAGE_IMAGE_H_

#pragma once

#include "duilib/Image/ImageInfo.h"
#include "duilib/Image/ImageAttribute.h"
#include "duilib/Image/ImageLoadAttribute.h"
#include "duilib/Image/StateImageMap.h"
#include "duilib/Utils/Delegate.h"
#include <memory>

namespace ui 
{
class Control;
class ImageGif;

/** 图片相关封装，支持的文件格式：SVG/PNG/GIF/JPG/BMP/APNG/WEBP/ICO
*/
class UILIB_API Image
{
public:
	Image();
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

public:
	/** @name 图片属性
	* @{
	*/
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

	/** 设置图片属性：播放次数（仅当多帧图片时）
	*/
	void SetImagePlayCount(int32_t nPlayCount);

	/** 设置图片属性：透明度（仅当多帧图片时）
	*/
	void SetImageFade(uint8_t nFade);

	/** 获取图片属性（只读）
	*/
	const ImageAttribute& GetImageAttribute() const;

	/** 获取图片加载属性
	*/
	ImageLoadAttribute GetImageLoadAttribute() const;

	/** @} */

public:
	/** @name 图片数据（由外部加载图片数据）
	* @{
	*/
	/** 获取图片信息接口
	*/
	const std::shared_ptr<ImageInfo>& GetImageCache() const;

	/** 设置图片信息接口
	*/
	void SetImageCache(const std::shared_ptr<ImageInfo>& imageInfo);

	/** 清除图片信息缓存数据, 释放资源
	*/
	void ClearImageCache();

	/** 设置当前图片帧（仅当多帧图片时）
	*/
	void SetCurrentFrame(uint32_t nCurrentFrame);

	/** 获取当前图片帧索引（仅当多帧图片时）
	*/
	uint32_t GetCurrentFrame() const;

	/** 获取图片的帧数
	*/
	uint32_t GetFrameCount() const;

	/** 是否位多帧图片(比如GIF等)
	*/
	bool IsMultiFrameImage() const;

	/** 获取当前图片帧的图片数据
	*/
	IBitmap* GetCurrentBitmap() const;

	/** @} */

public:
	/** @name 图片的显示
	* @{
	*/
	/** 设置关联的控件接口
	*/
	void SetControl(Control* pControl);

	/** 播放 GIF/WebP/APNG 动画
	* @param [in] rcImageRect 动画图片的显示区域
	*/
	bool CheckStartGifPlay(const UiRect& rcImageRect);

	/** 停止播放 GIF/WebP/APNG 动画
	 */
	void CheckStopGifPlay();

	/** 播放 GIF/WebP/APNG 动画
	 * @param [in] nStartFrame 从哪一帧开始播放，可设置第一帧、当前帧和最后一帧。请参考 GifFrameType 枚举
	 * @param [in] nPlayCount 指定播放次数, 如果是-1表示一直播放
	 */
	bool StartGifPlay(GifFrameType nStartFrame = kGifFrameFirst, int32_t nPlayCount = -1);

	/** 停止播放 GIF/WebP/APNG 动画
	 * @param [in] bTriggerEvent 是否将停止事件通知给订阅者，参考 AttachGifPlayStop 方法
	 * @param [in] nStopFrame 播放结束停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifFrameType 枚举
	 */
	void StopGifPlay(bool bTriggerEvent = false, GifFrameType nStopFrame = kGifFrameCurrent);

	/** 监听 GIF 播放完成通知
	 * @param[in] callback 要监听 GIF 停止播放的回调函数
	 */
	void AttachGifPlayStop(const EventCallback& callback);

	/** @} */

private:

	/** 当前正在播放的图片帧（仅当多帧图片时）
	*/
	uint32_t m_nCurrentFrame;

	/** 关联的控件接口
	*/
	Control* m_pControl;

	/** 多帧图片播放实现接口
	*/
	ImageGif* m_pImageGif;

	/** 图片属性
	*/
	ImageAttribute m_imageAttribute;

	/** 图片信息
	*/
	std::shared_ptr<ImageInfo> m_imageCache;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_H_
