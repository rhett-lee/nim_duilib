#ifndef UI_IMAGE_IMAGE_GIF_H_
#define UI_IMAGE_IMAGE_GIF_H_

#pragma once

#include "duilib/Core/UiTypes.h"
#include "duilib/Utils/Delegate.h"
#include "base/callback/callback.h"
#include <memory>
#include <map>

namespace ui 
{
class Image;
class Control;
typedef std::map<int32_t, CEventSource> GifEventMap;

/** 控件背景动态播放动画的逻辑封装（支持GIF/WebP/APNG动画）
*/
class ImageGif: public nbase::SupportWeakCallback
{
public:
	ImageGif();
	virtual ~ImageGif();
	ImageGif(const ImageGif&) = delete;
	ImageGif& operator = (const ImageGif&) = delete;

public:
	/** 设置关联的控件接口
	*/
	void SetControl(Control* pControl);

	/** 设置关联的图片接口
	*/
	void SetImage(Image* pImage);

	/** 设置动画的显示区域
	*/
	void SetImageRect(const UiRect& rcImageRect);

	/** 播放 GIF/WebP/APNG 动画
	 */
	bool StartGifPlay();

	/** 停止播放 GIF/WebP/APNG 动画
	 */
	void StopGifPlay();

	/** 播放 GIF/WebP/APNG 动画
	 * @param [in] nStartFrame 从哪一帧开始播放，可设置第一帧、当前帧和最后一帧。请参考 GifFrameType 枚举
	 * @param [in] nPlayCount 指定播放次数, 如果是-1表示一直播放
	 */
	bool StartGifPlay(GifFrameType nStartFrame, int32_t nPlayCount);

	/** 停止播放 GIF/WebP/APNG 动画
	 * @param [in] bTriggerEvent 是否将停止事件通知给订阅者，参考 AttachGifPlayStop 方法
	 * @param [in] nStopFrame 播放结束停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifFrameType 枚举
	 */
	void StopGifPlay(bool bTriggerEvent, GifFrameType nStopFrame);

	/** 是否正在播放动画
	*/
	bool IsPlayingGif() const;

	/** 监听 GIF 播放完成通知
	 * @param[in] callback 要监听 GIF 停止播放的回调函数
	 */
	void AttachGifPlayStop(const EventCallback& callback) { m_OnGifEvent[m_nVirtualEventGifStop] += callback; };

private:
	/** 向监听者发送播放停止事件
	*/
	void BroadcastGifEvent(int32_t nVirtualEvent) const;

	/** 获取GIF帧号
	*/
	uint32_t GetGifFrameIndex(GifFrameType frame) const;

	/** 定时器播放GIF的回调函数
	*/
	bool PlayGif();

	/** 重绘图片
	*/
	void RedrawImage();

	/** 是否位多帧图片(比如GIF等)
	*/
	bool IsMultiFrameImage() const;

private:
	/** GIF背景图片播放的取消机制
	*/
	nbase::WeakCallbackFlag m_gifWeakFlag;

	/** GIF播放事件的回调注册管理容器(目前只有播放完成一个事件)
	*/
	GifEventMap m_OnGifEvent;

	/** 关联的Control对象
	*/
	Control* m_pControl;

	/** 图片接口
	*/
	Image* m_pImage;

	/** 动画的显示区域
	*/
	UiRect m_rcImageRect;

	/** 是否正在播放动画图片
	*/
	bool m_bPlayingGif;

	/** 是否自动开始播放
	*/
	bool m_bAutoPlay;

	/** 已播放次数
	*/
	int32_t m_nCycledCount;

	/** 最大播放次数
	*/
	int32_t m_nMaxPlayCount;

	/** GIF背景图片播放完成事件的ID
	*/
	static const int32_t m_nVirtualEventGifStop = 1;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_GIF_H_
