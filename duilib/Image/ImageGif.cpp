#include "ImageGif.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Render/IRender.h"

namespace ui 
{
ImageGif::ImageGif():
    m_pControl(nullptr),
    m_pImage(nullptr),
    m_bPlayingGif(false),
    m_bAutoPlay(true),
    m_nCycledCount(0),
    m_nMaxPlayCount(-1),
    m_nVirtualEventGifStop(1)
{
}

ImageGif::~ImageGif()
{
}

void ImageGif::SetControl(Control* pControl)
{
    m_pControl = pControl;
}

void ImageGif::SetImage(Image* pImage)
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

void ImageGif::SetImageRect(const UiRect& rcImageRect)
{
    m_rcImageRect = rcImageRect;
}

bool ImageGif::StartGifPlay()
{
    if (!m_bAutoPlay) {
        return false;
    }
    if (IsMultiFrameImage()) {
        if (IsPlayingGif()) {
            return true;
        }
        else {
            int32_t nPlayCount = m_pImage->GetImageAttribute().m_nPlayCount;
            bool bHasPlayCount = m_pImage->GetImageAttribute().m_bHasPlayCount;
            return StartGifPlay(kGifFrameCurrent, nPlayCount, bHasPlayCount);
        }
    }
    else {
        m_bPlayingGif = false;
        m_gifWeakFlag.Cancel();
        return false;
    }
}

bool ImageGif::StartGifPlay(GifFrameType nStartFrame, int32_t nPlayCount, bool bHasPlayCount)
{
    m_gifWeakFlag.Cancel();
    if (!IsMultiFrameImage()) {
        m_bPlayingGif = false;
        return false;
    }
    if (bHasPlayCount) {
        if (nPlayCount <= 0) {
            nPlayCount = -1;//无限循环播放
        }
        m_nMaxPlayCount = nPlayCount;
    }

    //确定从哪一帧开始播放
    uint32_t nFrameIndex = GetGifFrameIndex(nStartFrame);
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
    m_bPlayingGif = true;
    RedrawImage();
    auto gifPlayCallback = UiBind(&ImageGif::PlayGif, this);
    bool bRet = GlobalManager::Instance().Timer().AddTimer(m_gifWeakFlag.GetWeakFlag(),
                                                           gifPlayCallback,
                                                           nTimerInterval) != 0;
    return bRet;
}

void ImageGif::PlayGif()
{
    //定时器触发，播放下一帧
    if (!IsPlayingGif() || !IsMultiFrameImage()) {
        m_gifWeakFlag.Cancel();
        m_bPlayingGif = false;
        return;
    }

    uint32_t nFrameIndex = m_pImage->GetCurrentFrameIndex();
    std::shared_ptr<IAnimationImage::AnimationFrame> pAnimationFrame = m_pImage->GetImageCache()->GetFrame(nFrameIndex);
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        StopGifPlay(true, kGifFrameCurrent);
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
            StopGifPlay(true, kGifFrameCurrent);
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
            StopGifPlay(true, kGifFrameLast);
            return;
        }
    }
    pAnimationFrame = m_pImage->GetImageCache()->GetFrame(nFrameIndex);
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        StopGifPlay(true, kGifFrameCurrent);
        return;
    }
    int32_t nNowTimerInterval = pAnimationFrame->GetDelayMs();
    ASSERT(nNowTimerInterval > 0);
    bool bRet = true;
    if (nPreTimerInterval != nNowTimerInterval) {
        m_gifWeakFlag.Cancel();
        auto gifPlayCallback = UiBind(&ImageGif::PlayGif, this);
        bRet = GlobalManager::Instance().Timer().AddTimer(m_gifWeakFlag.GetWeakFlag(),
                                                          gifPlayCallback,
                                                          nNowTimerInterval) != 0;
    }
    if (bRet) {
        m_pImage->SetCurrentFrameIndex(nFrameIndex);
        RedrawImage();
    }
    else {
        //启动定时器失败
        StopGifPlay(true, kGifFrameCurrent);
    }
}

void ImageGif::StopGifPlay()
{
    m_bPlayingGif = false;
    m_gifWeakFlag.Cancel();
}

void ImageGif::StopGifPlay(bool bTriggerEvent, GifFrameType nStopFrame)
{
    m_bPlayingGif = false;
    m_gifWeakFlag.Cancel();
    if (IsMultiFrameImage()) {        
        uint32_t index = GetGifFrameIndex(nStopFrame);
        m_pImage->SetCurrentFrameIndex(index);
        RedrawImage();
    }
    //标记为手动停止，不自动播放动画
    m_bAutoPlay = false;
    if (bTriggerEvent) {
        BroadcastGifEvent(m_nVirtualEventGifStop);
    }
}

bool ImageGif::IsPlayingGif() const
{
    return m_bPlayingGif;
}

void ImageGif::BroadcastGifEvent(int32_t nVirtualEvent) const
{
    auto callback = m_OnGifEvent.find(nVirtualEvent);
    if (callback != m_OnGifEvent.end()) {
        EventArgs param;
        param.SetSender(m_pControl);
        callback->second(param);
    }
}

uint32_t ImageGif::GetGifFrameIndex(GifFrameType frame) const
{
    if (!IsMultiFrameImage()) {
        return 0;
    }
    uint32_t ret = frame;
    switch (frame)
    {
    case kGifFrameCurrent:
        ret = m_pImage->GetCurrentFrameIndex();
        break;
    case kGifFrameFirst:
        ret = 0;
        break;
    case kGifFrameLast:
    {
        uint32_t nFrameCount = m_pImage->GetImageCache()->GetFrameCount();
        ret = nFrameCount > 0 ? nFrameCount - 1 : 0;
    }
    break;
    }
    return ret;
}

void ImageGif::RedrawImage()
{
    if (m_pControl != nullptr) {
        //重绘图片
        m_pControl->InvalidateRect(m_rcImageRect);
    }
}

bool ImageGif::IsMultiFrameImage() const
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
