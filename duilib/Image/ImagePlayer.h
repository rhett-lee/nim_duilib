#ifndef UI_IMAGE_IMAGE_PLAYER_H_
#define UI_IMAGE_IMAGE_PLAYER_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Utils/Delegate.h"
#include "duilib/Core/Callback.h"
#include "duilib/Core/ControlPtrT.h"
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

    /** 设置动画的显示区域（在绘制前调用）
    */
    void SetImageAnimationRect(const UiRect& rcImageRect);

    /** 播放动画（按需检测是否应该自动播放图片动画, 正常情况下，图片动画会自动播放）
     */
    void CheckStartImageAnimation();

    /** 播放动画
     * @param [in] nStartFrame 从哪一帧开始播放，可设置第一帧、当前帧和最后一帧。请参考 AnimationImagePos 枚举
     * @param [in] nPlayCount 指定播放次数
                   -1: 表示一直播放
                    0: 表示无有效的播放次数，使用图片的默认值(或者预设值)
                   >0: 具体的播放次数，达到播放次数后，停止播放
     */
    bool StartImageAnimation(AnimationImagePos nStartFrame, int32_t nPlayCount);

    /** 停止播放动画     
     * @param [in] nStopFrame 播放结束停止在哪一帧，可设置第一帧、当前帧和最后一帧。请参考 AnimationImagePos 枚举
     * @param [in] bTriggerEvent 是否将停止事件通知给订阅者，参考 AttachImageAnimationStop 方法
     */
    void StopImageAnimation(AnimationImagePos nStopFrame, bool bTriggerEvent);

    /** 是否正在播放动画
    */
    bool IsAnimationPlaying() const;

    /** 设置是否自动开始播放
    */
    void SetAutoPlay(bool bAutoPlay);

    /** 获取是否自动开始播放
    */
    bool IsAutoPlay() const;

private:
    /** 获取动画帧号
    */
    uint32_t GetImageFrameIndex(AnimationImagePos frame) const;

    /** 定时器播放动画回调函数
    */
    void PlayingImageAnimation();

    /** 重绘图片
    */
    void RedrawImage();

    /** 是否位多帧图片
    */
    bool IsMultiFrameImage() const;

    /** 获取动画图片的播放状态
    */
    void GetImageAnimationStatus(ImageAnimationStatus& animStatus);

private:
    /** 图片动画播放的取消机制
    */
    WeakCallbackFlag m_animWeakFlag;

    /** 关联的Control对象
    */
    ControlPtr m_pControl;

    /** 图片接口
    */
    Image* m_pImage;

    /** 动画的显示区域
    */
    UiRect m_rcImageAnimationRect;

    /** 已播放次数
    */
    int32_t m_nCycledCount;

    /** 最大播放次数
    */
    int32_t m_nMaxPlayCount;

    /** 是否正在播放动画图片
    */
    bool m_bAnimationPlaying;

    /** 是否自动开始播放
    */
    bool m_bAutoPlay;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_PLAYER_H_
