#ifndef UI_IMAGE_IMAGE_PLAYER_H_
#define UI_IMAGE_IMAGE_PLAYER_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Utils/Delegate.h"
#include "duilib/Core/Callback.h"
#include <map>

namespace ui 
{
class Image;
class Control;
typedef std::map<int32_t, CEventSource> AnimationEventMap;

/** 控件背景动态播放动画的逻辑封装（支持GIF/WebP/APNG动画）
*/
class ImagePlayer: public SupportWeakCallback
{
public:
    ImagePlayer();
    virtual ~ImagePlayer() override;
    ImagePlayer(const ImagePlayer&) = delete;
    ImagePlayer& operator = (const ImagePlayer&) = delete;

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

    /** 播放动画
     */
    bool StartImageAnimation();

    /** 停止播放动画
     */
    void StopImageAnimation();

    /** 播放动画
     * @param [in] nStartFrame 从哪一帧开始播放，可设置第一帧、当前帧和最后一帧。请参考 AnimationImagePos 枚举
     * @param [in] nPlayCount 指定播放次数, 如果是-1表示一直播放
     * @param [in] bHasPlayCount nPlayCount值是否有效
     */
    bool StartImageAnimation(AnimationImagePos nStartFrame, int32_t nPlayCount, bool bHasPlayCount = true);

    /** 停止播放动画
     * @param [in] bTriggerEvent 是否将停止事件通知给订阅者，参考 AttachImageAnimationStop 方法
     * @param [in] nStopFrame 播放结束停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 AnimationImagePos 枚举
     */
    void StopImageAnimation(bool bTriggerEvent, AnimationImagePos nStopFrame);

    /** 是否正在播放动画
    */
    bool IsAnimationPlaying() const;

    /** 监听动画播放完成通知
     * @param[in] callback 要监听动画停止播放的回调函数
     */
    void AttachImageAnimationStop(const EventCallback& callback) { m_OnAnimationEvent[m_nVirtualEventStop] += callback; };

private:
    /** 向监听者发送播放停止事件
    */
    void BroadcastAnimationEvent(int32_t nVirtualEvent) const;

    /** 获取动画帧号
    */
    uint32_t GetImageFrameIndex(AnimationImagePos frame) const;

    /** 定时器播放动画回调函数
    */
    void PlayAnimation();

    /** 重绘图片
    */
    void RedrawImage();

    /** 是否位多帧图片
    */
    bool IsMultiFrameImage() const;

private:
    /** 图片动画播放的取消机制
    */
    WeakCallbackFlag m_aniWeakFlag;

    /** 动画播放事件的回调注册管理容器(目前只有播放完成一个事件)
    */
    AnimationEventMap m_OnAnimationEvent;

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
    bool m_bAnimationPlaying;

    /** 是否自动开始播放
    */
    bool m_bAutoPlay;

    /** 已播放次数
    */
    int32_t m_nCycledCount;

    /** 最大播放次数
    */
    int32_t m_nMaxPlayCount;

    /** 动画图片播放完成事件的ID
    */
    const int32_t m_nVirtualEventStop;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_PLAYER_H_
