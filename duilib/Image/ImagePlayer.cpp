#include "ImagePlayer.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Render/IRender.h"

namespace ui 
{
ImagePlayer::ImagePlayer():
    m_pControl(nullptr),
    m_pImage(nullptr),
    m_bAnimationPlaying(false),
    m_bAutoPlay(true),
    m_nCycledCount(0),
    m_nMaxPlayCount(-1),
    m_nVirtualEventStop(1)
{
}

ImagePlayer::~ImagePlayer()
{
}

void ImagePlayer::SetControl(Control* pControl)
{
    m_pControl = pControl;
}

void ImagePlayer::SetImage(Image* pImage)
{
    m_pImage = pImage;
    if (m_pImage != nullptr) {
        ASSERT(m_pImage->IsMultiFrameImage());
        //初始化播放循环次数        
        if (m_pImage->GetImageAttribute().m_bHasPlayCount) {
            m_nMaxPlayCount = m_pImage->GetImageAttribute().m_nPlayCount;
        }
        else {
            m_nMaxPlayCount = m_pImage->GetLoopCount();
        }
    }
}

void ImagePlayer::SetImageRect(const UiRect& rcImageRect)
{
    m_rcImageRect = rcImageRect;
}

bool ImagePlayer::StartImageAnimation()
{
    if (!m_bAutoPlay) {
        return false;
    }
    if (IsMultiFrameImage()) {
        if (IsAnimationPlaying()) {
            return true;
        }
        else {
            int32_t nPlayCount = m_pImage->GetImageAttribute().m_nPlayCount;
            bool bHasPlayCount = m_pImage->GetImageAttribute().m_bHasPlayCount;
            return StartImageAnimation(AnimationImagePos::kFrameCurrent, nPlayCount, bHasPlayCount);
        }
    }
    else {
        m_bAnimationPlaying = false;
        m_aniWeakFlag.Cancel();
        return false;
    }
}

bool ImagePlayer::StartImageAnimation(AnimationImagePos nStartFrame, int32_t nPlayCount, bool bHasPlayCount)
{
    m_aniWeakFlag.Cancel();
    if (!IsMultiFrameImage()) {
        m_bAnimationPlaying = false;
        return false;
    }
    if (bHasPlayCount) {
        if (nPlayCount <= 0) {
            nPlayCount = -1;//无限循环播放
        }
        m_nMaxPlayCount = nPlayCount;
    }

    //确定从哪一帧开始播放
    uint32_t nFrameIndex = GetImageFrameIndex(nStartFrame);
    m_pImage->SetCurrentFrameIndex(nFrameIndex);
    nFrameIndex = m_pImage->GetCurrentFrameIndex();

    std::shared_ptr<IAnimationImage::AnimationFrame> pAnimationFrame = m_pImage->GetImageCache()->GetFrame(nFrameIndex);
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        return false;
    }
    int32_t nTimerInterval = pAnimationFrame->GetDelayMs();
    ASSERT(nTimerInterval > 0);
    m_nCycledCount = 0;
    m_bAnimationPlaying = true;
    RedrawImage();
    auto animationPlayCallback = UiBind(&ImagePlayer::PlayAnimation, this);
    bool bRet = GlobalManager::Instance().Timer().AddTimer(m_aniWeakFlag.GetWeakFlag(),
                                                           animationPlayCallback,
                                                           nTimerInterval) != 0;
    return bRet;
}

void ImagePlayer::PlayAnimation()
{
    //定时器触发，播放下一帧
    if (!IsAnimationPlaying() || !IsMultiFrameImage()) {
        m_aniWeakFlag.Cancel();
        m_bAnimationPlaying = false;
        return;
    }

    uint32_t nFrameIndex = m_pImage->GetCurrentFrameIndex();
    std::shared_ptr<IAnimationImage::AnimationFrame> pAnimationFrame = m_pImage->GetImageCache()->GetFrame(nFrameIndex);
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        StopImageAnimation(true, AnimationImagePos::kFrameCurrent);
        return;
    }

    //检查当前帧和下一帧图片是否完成解码
    if (pAnimationFrame->m_bDataPending) {
        //该帧图片数据尚未完成解码
        return;
    }
    else {
        uint32_t nNextFrameIndex = nFrameIndex + 1;
        if (nNextFrameIndex >= m_pImage->GetImageCache()->GetFrameCount()) {
            nNextFrameIndex = 0;
        }
        std::shared_ptr<IAnimationImage::AnimationFrame> pNextAnimationFrame = m_pImage->GetImageCache()->GetFrame(nNextFrameIndex);
        ASSERT(pAnimationFrame != nullptr);
        if (pNextAnimationFrame == nullptr) {
            StopImageAnimation(true, AnimationImagePos::kFrameCurrent);
            return;
        }
        if (pNextAnimationFrame->m_bDataPending) {
            //下一帧图片数据尚未完成解码
            return;
        }
    }

    //播放下一帧
    int32_t nPreTimerInterval = pAnimationFrame->GetDelayMs();
    ASSERT(nPreTimerInterval > 0);
    nFrameIndex++;
    if (nFrameIndex >= m_pImage->GetImageCache()->GetFrameCount()) {
        nFrameIndex = 0;
        m_nCycledCount += 1;
        if ((m_nMaxPlayCount > 0) && (m_nCycledCount >= m_nMaxPlayCount)) {
            //达到最大播放次数，停止播放
            StopImageAnimation(true, AnimationImagePos::kFrameLast);
            return;
        }
    }
    pAnimationFrame = m_pImage->GetImageCache()->GetFrame(nFrameIndex);
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        StopImageAnimation(true, AnimationImagePos::kFrameCurrent);
        return;
    }
    int32_t nNowTimerInterval = pAnimationFrame->GetDelayMs();
    ASSERT(nNowTimerInterval > 0);
    bool bRet = true;
    if (nPreTimerInterval != nNowTimerInterval) {
        m_aniWeakFlag.Cancel();
        auto animationPlayCallback = UiBind(&ImagePlayer::PlayAnimation, this);
        bRet = GlobalManager::Instance().Timer().AddTimer(m_aniWeakFlag.GetWeakFlag(),
                                                          animationPlayCallback,
                                                          nNowTimerInterval) != 0;
    }
    if (bRet) {
        m_pImage->SetCurrentFrameIndex(nFrameIndex);
        RedrawImage();
    }
    else {
        //启动定时器失败
        StopImageAnimation(true, AnimationImagePos::kFrameCurrent);
    }
}

void ImagePlayer::StopImageAnimation()
{
    m_bAnimationPlaying = false;
    m_aniWeakFlag.Cancel();
}

void ImagePlayer::StopImageAnimation(bool bTriggerEvent, AnimationImagePos nStopFrame)
{
    m_bAnimationPlaying = false;
    m_aniWeakFlag.Cancel();
    if (IsMultiFrameImage()) {        
        uint32_t index = GetImageFrameIndex(nStopFrame);
        m_pImage->SetCurrentFrameIndex(index);
        RedrawImage();
    }
    //标记为手动停止，不自动播放动画
    m_bAutoPlay = false;
    if (bTriggerEvent) {
        BroadcastAnimationEvent(m_nVirtualEventStop);
    }
}

bool ImagePlayer::IsAnimationPlaying() const
{
    return m_bAnimationPlaying;
}

void ImagePlayer::BroadcastAnimationEvent(int32_t nVirtualEvent) const
{
    auto callback = m_OnAnimationEvent.find(nVirtualEvent);
    if (callback != m_OnAnimationEvent.end()) {
        EventArgs param;
        param.SetSender(m_pControl);
        callback->second(param);
    }
}

uint32_t ImagePlayer::GetImageFrameIndex(AnimationImagePos frame) const
{
    if (!IsMultiFrameImage()) {
        return 0;
    }
    uint32_t ret = 0;
    switch (frame)
    {
    case AnimationImagePos::kFrameCurrent:
        ret = m_pImage->GetCurrentFrameIndex();
        break;
    case AnimationImagePos::kFrameFirst:
        ret = 0;
        break;
    case AnimationImagePos::kFrameLast:
        {
            uint32_t nFrameCount = m_pImage->GetImageCache()->GetFrameCount();
            ret = nFrameCount > 0 ? nFrameCount - 1 : 0;
        }
        break;
    default:
        break;
    }
    return ret;
}

void ImagePlayer::RedrawImage()
{
    if (m_pControl != nullptr) {
        //重绘图片
        m_pControl->InvalidateRect(m_rcImageRect);
    }
}

bool ImagePlayer::IsMultiFrameImage() const
{
    if ((m_pControl != nullptr) && 
        (m_pImage != nullptr) &&
        (m_pImage->IsImagePaintEnabled()) &&
        (m_pImage->GetImageCache() != nullptr) &&
        (m_pImage->GetImageCache()->IsMultiFrameImage())) {
        return true;
    }
    return false;
}

}
