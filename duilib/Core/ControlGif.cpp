#include "ControlGif.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Render/IRender.h"

namespace ui 
{
ControlGif::ControlGif(Control* pControl):
	m_pControl(pControl),
	m_bGifPlay(true)
{
}

ControlGif::~ControlGif()
{
}

void ControlGif::SetBkImage(const std::shared_ptr<Image>& pBkImage)
{
	m_pBkImage = pBkImage;
}

bool ControlGif::CanGifPlay() const
{
	return m_bGifPlay;
}

bool ControlGif::GifPlay()
{
	return StartGifPlay(true);
}

bool ControlGif::StartGifPlay(bool bRedraw)
{
	if (m_pBkImage == nullptr) {
		return false;
	}
	if (!m_pBkImage->GetImageCache() ||
		!m_pBkImage->GetImageCache()->IsMultiFrameImage() ||
		!m_pBkImage->ContinuePlay()) {
		m_pBkImage->SetPlaying(false);
		m_gifWeakFlag.Cancel();
		return false;
	}

	if (!m_pBkImage->IsPlaying()) {
		m_pBkImage->SetCurrentFrame(0);
		m_gifWeakFlag.Cancel();
		int32_t timerInterval = m_pBkImage->GetCurrentInterval();//≤•∑≈º‰∏Ù£∫∫¡√Î
		if (timerInterval <= 0) {
			if (bRedraw && (m_pControl != nullptr)) {
				m_pControl->Invalidate();
			}
			return false;
		}
		m_pBkImage->SetPlaying(true);
		auto gifPlayCallback = nbase::Bind(&ControlGif::GifPlay, this);
		GlobalManager::Instance().Timer().AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(),
			gifPlayCallback,
			timerInterval,
			TimerManager::REPEAT_FOREVER);
	}
	else {
		int32_t preInterval = m_pBkImage->GetCurrentInterval();
		m_pBkImage->IncrementCurrentFrame();
		int32_t nowInterval = m_pBkImage->GetCurrentInterval();
		if (!m_pBkImage->ContinuePlay()) {
			StopGifPlayForUI(true, kGifStopLast);
		}
		else
		{
			if ((preInterval <= 0) || (nowInterval <= 0)) {
				m_pBkImage->SetPlaying(false);
				m_gifWeakFlag.Cancel();
				if (bRedraw && (m_pControl != nullptr)) {
					m_pControl->Invalidate();
				}
				return false;
			}

			if (preInterval != nowInterval) {
				m_gifWeakFlag.Cancel();
				m_pBkImage->SetPlaying(true);
				auto gifPlayCallback = nbase::Bind(&ControlGif::GifPlay, this);
				GlobalManager::Instance().Timer().AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(),
					gifPlayCallback,
					nowInterval,
					TimerManager::REPEAT_FOREVER);
			}
		}
	}
	if (bRedraw && (m_pControl != nullptr)) {
		m_pControl->Invalidate();
	}
	return m_pBkImage->IsPlaying();
}

bool ControlGif::StopGifPlay(GifStopType frame)
{
	if ((m_pBkImage == nullptr) || (m_pBkImage->GetImageCache() == nullptr)) {
		return false;
	}
	bool isStopped = false;
	if (m_pBkImage->GetImageCache()->IsMultiFrameImage()) {
		m_pBkImage->SetPlaying(false);
		m_gifWeakFlag.Cancel();
		uint32_t index = GetGifFrameIndex(frame);
		m_pBkImage->SetCurrentFrame(index);
		isStopped = true;
	}
	return isStopped;
}

bool ControlGif::StartGifPlayForUI(GifStopType frame, int32_t playcount)
{
	if ((m_pControl == nullptr) || (m_pBkImage == nullptr)) {
		return false;
	}
	if ((playcount == 0) || 
		(m_pBkImage->GetImageCache() == nullptr) || 
		!m_pBkImage->GetImageCache()->IsMultiFrameImage()) {
		m_bGifPlay = false;
		m_pBkImage->SetPlaying(false);
		m_gifWeakFlag.Cancel();
		return false;
	}
	m_gifWeakFlag.Cancel();
	m_bGifPlay = true;
	m_pBkImage->SetCurrentFrame(GetGifFrameIndex(frame));
	int32_t timerInterval = m_pBkImage->GetCurrentInterval();
	if (timerInterval <= 0) {
		m_pBkImage->SetPlaying(false);
		m_bGifPlay = false;
		return false;
	}
	m_pBkImage->SetPlaying(true);
	m_pBkImage->SetImagePlayCount(playcount);
	m_pBkImage->ClearCycledCount();
	auto gifPlayCallback = nbase::Bind(&ControlGif::GifPlay, this);
	GlobalManager::Instance().Timer().AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(),
		gifPlayCallback,
		timerInterval,
		TimerManager::REPEAT_FOREVER);
	return true;
}

bool ControlGif::StopGifPlayForUI(bool transfer, GifStopType frame)
{
	m_bGifPlay = false;
	bool isStopped = StopGifPlay(frame);
	if (transfer) {
		BroadcastGifEvent(m_nVirtualEventGifStop);
	}
	return isStopped;
}

void ControlGif::BroadcastGifEvent(int32_t nVirtualEvent) const
{
	auto callback = m_OnGifEvent.find(nVirtualEvent);
	if (callback != m_OnGifEvent.end()) {
		EventArgs param;
		param.pSender = m_pControl;
		callback->second(param);
	}
}

uint32_t ControlGif::GetGifFrameIndex(GifStopType frame) const
{
	if ((m_pBkImage == nullptr) || (m_pBkImage->GetImageCache() == nullptr)) {
		return 0;
	}
	uint32_t ret = frame;
	switch (frame)
	{
	case kGifStopCurrent:
		ret = m_pBkImage->GetCurrentFrameIndex();
		break;
	case kGifStopFirst:
		ret = 0;
		break;
	case kGifStopLast:
	{
		uint32_t nFrameCount = m_pBkImage->GetImageCache()->GetFrameCount();
		ret = nFrameCount > 0 ? nFrameCount - 1 : 0;
	}
	break;
	}
	return ret;
}

}