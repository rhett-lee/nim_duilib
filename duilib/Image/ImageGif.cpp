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
	m_nMaxPlayCount(-1)
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
			int32_t nPlayCount = m_pImage->GetImageAttribute().nPlayCount;
			return StartGifPlay(kGifFrameCurrent, nPlayCount);
		}
	}
	else {
		m_bPlayingGif = false;
		m_gifWeakFlag.Cancel();
		return false;
	}
}

bool ImageGif::StartGifPlay(GifFrameType nStartFrame, int32_t nPlayCount)
{
	m_gifWeakFlag.Cancel();
	if ((nPlayCount == 0) || !IsMultiFrameImage()) {
		m_bPlayingGif = false;
		return false;
	}
	m_nMaxPlayCount = nPlayCount;
	uint32_t nFrameIndex = GetGifFrameIndex(nStartFrame);
	m_pImage->SetCurrentFrame(nFrameIndex);
	nFrameIndex = m_pImage->GetCurrentFrame();
	int32_t nTimerInterval = m_pImage->GetImageCache()->GetFrameInterval(nFrameIndex);
	if (nTimerInterval <= 0) {
		m_bPlayingGif = false;
		return false;
	}

	m_nCycledCount = 0;
	m_bPlayingGif = true;
	RedrawImage();
	auto gifPlayCallback = nbase::Bind(&ImageGif::PlayGif, this);
	bool bRet = GlobalManager::Instance().Timer().AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(),
														             gifPlayCallback,
														             nTimerInterval,
														             TimerManager::REPEAT_FOREVER);
	return bRet;
}

bool ImageGif::PlayGif()
{
	//定时器触发，播放下一帧
	if (!IsPlayingGif() || !IsMultiFrameImage()) {
		m_gifWeakFlag.Cancel();
		m_bPlayingGif = false;
		return false;
	}

	uint32_t nFrameIndex = m_pImage->GetCurrentFrame();
	int32_t nPreTimerInterval = m_pImage->GetImageCache()->GetFrameInterval(nFrameIndex);
	nFrameIndex++;
	if (nFrameIndex >= m_pImage->GetImageCache()->GetFrameCount()) {
		nFrameIndex = 0;
		m_nCycledCount += 1;
		if ((m_nMaxPlayCount > 0) && (m_nCycledCount >= m_nMaxPlayCount)) {
			//达到最大播放次数，停止播放
			StopGifPlay(true, kGifFrameLast);
			return false;
		}
	}
	int32_t nNowTimerInterval = m_pImage->GetImageCache()->GetFrameInterval(nFrameIndex);
	if (nNowTimerInterval <= 0) {
		//播放间隔无效，停止播放
		StopGifPlay(true, kGifFrameCurrent);
		return false;
	}
	bool bRet = true;
	if (nPreTimerInterval != nNowTimerInterval) {
		m_gifWeakFlag.Cancel();
		auto gifPlayCallback = nbase::Bind(&ImageGif::PlayGif, this);
		bRet = GlobalManager::Instance().Timer().AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(),
															        gifPlayCallback,
															        nNowTimerInterval,
															        TimerManager::REPEAT_FOREVER);
	}
	if (bRet) {
		m_pImage->SetCurrentFrame(nFrameIndex);
		RedrawImage();
	}
	else {
		//启动定时器失败
		StopGifPlay(true, kGifFrameCurrent);
	}	
	return bRet;
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
		m_pImage->SetCurrentFrame(index);
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
		param.pSender = m_pControl;
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
		ret = m_pImage->GetCurrentFrame();
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
