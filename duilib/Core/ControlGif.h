#ifndef UI_CORE_CONTROL_GIF_H_
#define UI_CORE_CONTROL_GIF_H_

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
class IRender;
typedef std::map<int32_t, CEventSource> GifEventMap;

/** 控件背景动态播放动画的逻辑封装（支持GIF/WebP/APNG动画）
*/
class ControlGif: public nbase::SupportWeakCallback
{
public:
	explicit ControlGif(Control* pControl);
	~ControlGif();
	ControlGif(const ControlGif&) = delete;
	ControlGif& operator = (const ControlGif&) = delete;

public:
	/** 设置关联的背景图片
	*/
	void SetBkImage(const std::shared_ptr<Image>& pBkImage);

	/** GIF动画播放标志
	*/
	bool CanGifPlay() const;

	/** 开始播放动画
	*@return 成功返回true，否则返回false
	*/
	bool StartGifPlay(bool bRedraw);

	/** 停止播放GIF动画
	* @return 成功返回true，否则返回false
	*/
	bool StopGifPlay(GifStopType frame = kGifStopCurrent);

	/** 停止播放 GIF
	 * @param [in] transfer 是否将停止事件通知给订阅者，参考 AttachGifPlayStop 方法
	 * @param [frame] frame 播放结束停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifStopType 枚举
	 */
	bool StopGifPlayForUI(bool transfer = false, GifStopType frame = kGifStopCurrent);

	/** 播放 GIF
	 * @param[in] 播放完成停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 GifStopType 枚举
	 */
	bool StartGifPlayForUI(GifStopType frame = kGifStopFirst, int32_t playcount = -1);

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
	uint32_t GetGifFrameIndex(GifStopType frame) const;

	/** 定时器播放GIF的回调函数
	*/
	bool GifPlay();

private:
	//是否为播放GIF的状态（当背景图片m_pBkImage是GIF文件时，触发此逻辑）
	bool m_bGifPlay;

	//GIF背景图片播放的取消机制
	nbase::WeakCallbackFlag m_gifWeakFlag;

	//GIF播放事件的回调注册管理容器(目前只有播放完成一个事件)
	GifEventMap m_OnGifEvent;

	//GIF背景图片播放完成事件的ID
	static const int32_t m_nVirtualEventGifStop = 1;

	//关联的Control对象
	Control* m_pControl;

	//控件的背景图片
	std::shared_ptr<Image> m_pBkImage;
};

} // namespace ui

#endif // UI_CORE_CONTROL_GIF_H_
