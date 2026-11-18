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
    m_bWaitingFrameData(false),
    m_nCycledCount(0),
    m_nMaxPlayCount(-1)
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
    //重新初始化图片时，先复位播放状态并取消旧的定时器，避免定时器仍按旧图片的状态运行
    m_animWeakFlag.Cancel();
    m_bAnimationPlaying = false;
    m_bWaitingFrameData = false;
    m_nCycledCount = 0;

    m_pImage = pImage;
    if (m_pImage != nullptr) {
        ASSERT(m_pImage->IsMultiFrameImage());
        //初始化播放循环次数
        m_nMaxPlayCount = m_pImage->GetImageAttribute().m_nPlayCount;
        if (m_nMaxPlayCount == 0) {
            m_nMaxPlayCount = m_pImage->GetLoopCount();
        }
        if (m_nMaxPlayCount <= 0) {
            m_nMaxPlayCount = -1;
        }

        //初始化是否自动播放
        m_bAutoPlay = m_pImage->GetImageAttribute().m_bAutoPlay;
    }
}

void ImagePlayer::SetImageAnimationRect(const UiRect& rcImageRect)
{
    m_rcImageAnimationRect = rcImageRect;
}

void ImagePlayer::GetImageAnimationStatus(ImageAnimationStatus& animStatus)
{
    if ((m_pImage != nullptr) && (m_pControl != nullptr) && (m_pImage->GetImageInfo() != nullptr)) {
        animStatus.m_name = m_pImage->GetImageAttribute().m_sImageName.c_str();
        animStatus.m_bBkImage = m_pControl->GetBkImage() == m_pImage->GetImageString();
        animStatus.m_nFrameCount = m_pImage->GetFrameCount();
        animStatus.m_nFrameIndex = m_pImage->GetCurrentFrameIndex();
        animStatus.m_nFrameDelayMs = m_pImage->GetImageInfo()->GetFrameDelayMs(animStatus.m_nFrameIndex);
        animStatus.m_nLoopCount = m_pImage->GetImageInfo()->GetLoopCount();
    }
    else {
        animStatus.m_name.clear();
        animStatus.m_bBkImage = false;
        animStatus.m_nFrameCount = 0;
        animStatus.m_nFrameIndex = 0;
        animStatus.m_nFrameDelayMs = 0;
        animStatus.m_nLoopCount = 0;
    }
}

void ImagePlayer::CheckStartImageAnimation()
{
    if (!m_bAutoPlay) {
        return;
    }
    if (IsMultiFrameImage()) {
        if (IsAnimationPlaying()) {
            return;
        }
        else {
            int32_t nPlayCount = m_pImage->GetImageAttribute().m_nPlayCount;
            bool bRet = StartImageAnimation(AnimationImagePos::kFrameCurrent, nPlayCount);
            ASSERT_UNUSED_VARIABLE(bRet);
        }
    }
    else {
        m_bAnimationPlaying = false;
        m_animWeakFlag.Cancel();
        return;
    }
}

bool ImagePlayer::StartImageAnimation(AnimationImagePos nStartFrame, int32_t nPlayCount)
{
    m_animWeakFlag.Cancel();
    if (!IsMultiFrameImage()) {
        m_bAnimationPlaying = false;
        return false;
    }
    if (nPlayCount != 0) {
        m_nMaxPlayCount = nPlayCount;
    }
    if (m_nMaxPlayCount <= 0) {
        //无限循环播放
        m_nMaxPlayCount = -1;
    }
    ASSERT((m_pImage != nullptr) && (m_pControl != nullptr) && (m_pImage->GetImageInfo() != nullptr));
    if ((m_pImage == nullptr) || (m_pControl == nullptr) || (m_pImage->GetImageInfo() == nullptr)) {
        m_bAnimationPlaying = false;
        return false;
    }

    //确定从哪一帧开始播放
    uint32_t nFrameIndex = GetImageFrameIndex(nStartFrame);
    m_pImage->SetCurrentFrameIndex(nFrameIndex);
    nFrameIndex = m_pImage->GetCurrentFrameIndex();
    int32_t nTimerInterval = m_pImage->GetImageInfo()->GetFrameDelayMs(nFrameIndex);
    ASSERT(nTimerInterval > 0);
    if (nTimerInterval <= 0) {
        m_bAnimationPlaying = false;
        return false;
    }
    m_nCycledCount = 0;
    m_bAnimationPlaying = true;
    m_bWaitingFrameData = false;
    RedrawImage();
    auto animationPlayCallback = UiBind(&ImagePlayer::PlayingImageAnimation, this);
    bool bRet = GlobalManager::Instance().Timer().AddTimer(m_animWeakFlag.GetWeakFlag(),
                                                           animationPlayCallback,
                                                           nTimerInterval) != 0;
    if (!bRet) {
        //启动定时器失败，恢复为非播放状态，避免状态与实际不符
        m_bAnimationPlaying = false;
    }
    if (bRet && (m_pControl != nullptr) && (m_pImage != nullptr)) {
        if (m_pControl->HasEventCallback(kEventImageAnimationStart)) {
            //触发一次动画开始事件
            ImageAnimationStatus animStatus;
            GetImageAnimationStatus(animStatus);
            m_pControl->SendEvent(kEventImageAnimationStart, (WPARAM)&animStatus);
        }
    }
    return bRet;
}

void ImagePlayer::PlayingImageAnimation()
{
    //定时器触发，播放下一帧
    if (!IsAnimationPlaying() || !IsMultiFrameImage()) {
        m_animWeakFlag.Cancel();
        m_bAnimationPlaying = false;
        return;
    }
    if ((m_pImage == nullptr) || (m_pControl == nullptr)) {
        m_animWeakFlag.Cancel();
        m_bAnimationPlaying = false;
        return;
    }
    std::shared_ptr<ImageInfo> pImageInfo = m_pImage->GetImageInfo();
    if (pImageInfo == nullptr) {
        m_animWeakFlag.Cancel();
        m_bAnimationPlaying = false;
        return;
    }
    uint32_t nFrameIndex = m_pImage->GetCurrentFrameIndex();
    if (!pImageInfo->IsFrameDataReady(nFrameIndex)) {
        //当前帧的数据尚未完成解码，如果未完成解码，应该是还没绘制: 跳过一个帧的时间
        //说明：此处的定时器是周期性触发的，直接返回后会在下一个定时器周期重试，
        //      无需重新添加定时器
        //请求继续异步解码：多帧图片只保持播放位置前方的预解码窗口，
        //当窗口关闭后需要主动请求，才能继续解码后续的帧
        m_bWaitingFrameData = true;
        m_pControl->RequestAsyncDecodeImage(m_pImage);
        return;
    }    
    else {
        //检查下一帧图片是否完成解码：下一帧
        uint32_t nNextFrameIndex = nFrameIndex + 1;
        if (nNextFrameIndex >= pImageInfo->GetFrameCount()) {
            nNextFrameIndex = 0;
        }
        if (!pImageInfo->IsFrameDataReady(nNextFrameIndex)) {
            //下一帧图片数据尚未完成解码: 不切换到下一帧
            //请求继续异步解码，避免因预解码窗口关闭而停止解码
            m_bWaitingFrameData = true;
            m_pControl->RequestAsyncDecodeImage(m_pImage);
            return;
        }
    }
    //帧数据已就绪，可以推进播放
    m_bWaitingFrameData = false;

    //播放下一帧
    int32_t nPreTimerInterval = pImageInfo->GetFrameDelayMs(nFrameIndex);
    ASSERT(nPreTimerInterval > 0);
    nFrameIndex++;
    if (nFrameIndex >= pImageInfo->GetFrameCount()) {
        //完成一次播放
        nFrameIndex = 0;
        m_nCycledCount += 1;
        if ((m_nMaxPlayCount > 0) && (m_nCycledCount >= m_nMaxPlayCount)) {
            //达到最大播放次数，停止播放
            StopImageAnimation(AnimationImagePos::kFrameLast, true);
            return;
        }
    }
    int32_t nNowTimerInterval = pImageInfo->GetFrameDelayMs(nFrameIndex);
    ASSERT(nNowTimerInterval > 0);
    bool bRet = true;
    if (nPreTimerInterval != nNowTimerInterval) {
        //帧与帧之间的播放时间发生变化，重启定时器，按新的播放时间差启动
        m_animWeakFlag.Cancel();
        auto animationPlayCallback = UiBind(&ImagePlayer::PlayingImageAnimation, this);
        bRet = GlobalManager::Instance().Timer().AddTimer(m_animWeakFlag.GetWeakFlag(),
                                                          animationPlayCallback,
                                                          nNowTimerInterval) != 0;
        ASSERT(bRet);
    }
    if (bRet) {
        //切换到下一帧, 重绘图片
        m_pImage->SetCurrentFrameIndex(nFrameIndex);
        RedrawImage();

        //触发播放进度
        if (m_pControl->HasEventCallback(kEventImageAnimationPlayFrame)) {
            //触发一次动画开始事件
            ImageAnimationStatus animStatus;
            GetImageAnimationStatus(animStatus);
            m_pControl->SendEvent(kEventImageAnimationPlayFrame, (WPARAM)&animStatus);
        }
    }
    else {
        //启动定时器失败
        StopImageAnimation(AnimationImagePos::kFrameCurrent, true);
    }
}

void ImagePlayer::StopImageAnimation(AnimationImagePos nStopFrame, bool bTriggerEvent)
{
    m_bAnimationPlaying = false;
    m_bWaitingFrameData = false;
    m_animWeakFlag.Cancel();
    if (IsMultiFrameImage() && (m_pImage != nullptr)) {
        uint32_t index = GetImageFrameIndex(nStopFrame);
        m_pImage->SetCurrentFrameIndex(index);
        RedrawImage();
    }
    //停止后不再自动播放：控件每次绘制都会调用CheckStartImageAnimation，
    //若不关闭自动播放，已停止(包括播放次数已用尽)的动画会在下次重绘时被重新启动
    m_bAutoPlay = false;
    if (bTriggerEvent && (m_pControl != nullptr) && (m_pImage != nullptr)) {
        if (m_pControl->HasEventCallback(kEventImageAnimationStop)) {
            //触发一次动画停止事件
            ImageAnimationStatus animStatus;
            GetImageAnimationStatus(animStatus);
            m_pControl->SendEvent(kEventImageAnimationStop, (WPARAM)&animStatus);
        }
    }
}

bool ImagePlayer::IsAnimationPlaying() const
{
    return m_bAnimationPlaying;
}

void ImagePlayer::SetAutoPlay(bool bAutoPlay)
{
    m_bAutoPlay = bAutoPlay;
}

bool ImagePlayer::IsAutoPlay() const
{
    return m_bAutoPlay;
}

void ImagePlayer::OnImageFrameDataReady()
{
    if (!m_bWaitingFrameData) {
        //播放未因缺帧而暂停：保持现有定时器的播放节奏，避免提前推进
        return;
    }
    if (!IsAnimationPlaying() || !IsMultiFrameImage()) {
        m_bWaitingFrameData = false;
        return;
    }
    //帧数据已就绪：立即按播放逻辑推进到下一帧，无需等待下一个定时器周期
    PlayingImageAnimation();
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
        if (m_pImage->GetImageInfo() != nullptr) {
            uint32_t nFrameCount = m_pImage->GetImageInfo()->GetFrameCount();
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
        if (m_rcImageAnimationRect.IsEmpty()) {
            //首次播放，区域为空，重回整个控件
            m_pControl->Invalidate();
        }
        else {
            m_pControl->InvalidateRect(m_rcImageAnimationRect);
        }
    }
}

bool ImagePlayer::IsMultiFrameImage() const
{
    if ((m_pControl != nullptr) && 
        (m_pImage != nullptr) &&
        (m_pImage->IsImagePaintEnabled()) &&
        (m_pImage->GetImageInfo() != nullptr) &&
        (m_pImage->GetImageInfo()->IsMultiFrameImage())) {
        return true;
    }
    return false;
}

}
