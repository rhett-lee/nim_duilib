#include "Progress.h"

namespace ui
{

Progress::Progress() :
	m_bHorizontal(true),
	m_bStretchForeImage(true),
	m_nMax(100),
	m_nMin(0),
	m_nValue(0),
	m_sProgressColor(),
	m_pProgressImage(nullptr),
	m_sProgressImageModify(),
	m_bMarquee(false),
	m_nMarqueeWidth(10),
	m_nMarqueeStep(4),
	m_nMarqueeElapsed(50), // for 1s 25fps,will use 20fps default
	m_nMarqueePos(0),
    m_bReverse(false)
{
	SetTextStyle(TEXT_SINGLELINE | TEXT_CENTER, false);
	SetFixedHeight(UiFixedInt(12), true, true);
}

Progress::~Progress()
{
	if (m_pProgressImage != nullptr) {
		delete m_pProgressImage;
		m_pProgressImage = nullptr;
	}
}

std::wstring Progress::GetType() const { return DUI_CTR_PROGRESS; }

bool Progress::IsHorizontal()
{
	return m_bHorizontal;
}

void Progress::SetHorizontal(bool bHorizontal)
{
	if (m_bHorizontal == bHorizontal) {
		return;
	}
	m_bHorizontal = bHorizontal;
	Invalidate();
}

int Progress::GetMinValue() const
{
	return m_nMin;
}

void Progress::SetMinValue(int nMin)
{
	m_nMin = nMin;
	Invalidate();
}

int Progress::GetMaxValue() const
{
	return m_nMax;
}

void Progress::SetMaxValue(int nMax)
{
	m_nMax = nMax;
	Invalidate();
}

double Progress::GetValue() const
{
	return m_nValue;
}

void Progress::SetValue(double nValue)
{
	if (nValue == m_nValue) {
		return;
	}
	m_nValue = nValue;
	if (m_nValue > m_nMax) {
		m_nValue = m_nMax;
	}
	if (m_nValue < m_nMin) {
		m_nValue = m_nMin;
	}
	Invalidate();
}

bool Progress::IsStretchForeImage()
{
	return m_bStretchForeImage;
}

void Progress::SetStretchForeImage(bool bStretchForeImage /*= true*/)
{
	if (m_bStretchForeImage == bStretchForeImage) {
		return;
	}
	m_bStretchForeImage = bStretchForeImage;
	Invalidate();
}

std::wstring Progress::GetProgressImage() const
{
	if (m_pProgressImage != nullptr) {
		return m_pProgressImage->GetImageString();
	}
	return std::wstring();
}

void Progress::SetProgressImage(const std::wstring& strImage)
{
	if (m_pProgressImage == nullptr) {
		m_pProgressImage = new Image;
	}
	m_pProgressImage->SetImageString(strImage);
	Invalidate();
}

std::wstring Progress::GetProgressColor() const
{
	return m_sProgressColor.c_str();
}

void Progress::SetProgressColor(const std::wstring& strProgressColor)
{
	ASSERT(strProgressColor.empty() || HasUiColor(strProgressColor));
	if (m_sProgressColor == strProgressColor) {
		return;
	}
	m_sProgressColor = strProgressColor;
	Invalidate();
}

void Progress::SetAttribute(const std::wstring& srName, const std::wstring& strValue)
{
	if ((srName == L"horizontal") || (srName == L"hor")){
		SetHorizontal(strValue == L"true");
	}
	else if (srName == L"min") {
		SetMinValue(_wtoi(strValue.c_str()));
	}
	else if (srName == L"max") {
		SetMaxValue(_wtoi(strValue.c_str()));
	}
	else if (srName == L"value") {
		SetValue(_wtoi(strValue.c_str()));
	}
	else if ((srName == L"progress_image") || (srName == L"progressimage")){
		SetProgressImage(strValue);
	}
	else if ((srName == L"stretch_fore_image") || (srName == L"is_stretch_fore") || (srName == L"isstretchfore")){
		SetStretchForeImage(strValue == L"true");
	}
    else if ((srName == L"progress_color") || (srName == L"progresscolor")) {
        SetProgressColor(strValue);
    }
	else if (srName == L"marquee") {
		SetMarquee(strValue == L"true");
	}
	else if ((srName == L"marquee_width") || (srName == L"marqueewidth")){
		SetMarqueeWidth(_wtoi(strValue.c_str()));
	}
	else if ((srName == L"marquee_step") || (srName == L"marqueestep")){
		SetMarqueeStep(_wtoi(strValue.c_str()));
	}
	else if (srName == L"reverse") {
		SetReverse(strValue == L"true");
	}
	else {
		Label::SetAttribute(srName, strValue);
	}
}

void Progress::PaintStateImages(IRender* pRender)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (m_bMarquee) {
		PaintMarquee(pRender);
		return;
	}

	if (m_nMax <= m_nMin) m_nMax = m_nMin + 1;
	if (m_nValue > m_nMax) m_nValue = m_nMax;
	if (m_nValue < m_nMin) m_nValue = m_nMin;

	UiRect rc = GetProgressPos();
	if (!m_sProgressColor.empty()) {
		UiColor dwProgressColor = this->GetUiColor(m_sProgressColor.c_str());
		if (dwProgressColor.GetARGB() != 0) {
			UiRect rcProgressColor = GetRect();
			if (m_bHorizontal) {
				rcProgressColor.right = rcProgressColor.left + rc.right;
			}
			else {
				rcProgressColor.top = rcProgressColor.top + rc.top;
			}
			pRender->FillRect(rcProgressColor, dwProgressColor);
		}
	}
	if ((m_pProgressImage == nullptr) || (m_pProgressImage->GetImageString().empty())) {
		return;
	}

    m_sProgressImageModify.clear();
    if (m_bStretchForeImage) {
        m_sProgressImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", rc.left, rc.top, rc.right, rc.bottom);
    }
    else {
        ui::UiRect m_rcSrc = rc;
        if (m_pProgressImage->GetImageCache()) {
            if (m_rcSrc.right > m_pProgressImage->GetImageCache()->GetWidth()) {
                m_rcSrc.right = m_pProgressImage->GetImageCache()->GetWidth();
            }
            if (m_rcSrc.bottom > m_pProgressImage->GetImageCache()->GetHeight()) {
                m_rcSrc.bottom = m_pProgressImage->GetImageCache()->GetHeight();
            }
        }
        m_sProgressImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"
            , rc.left, rc.top, rc.right, rc.bottom
            , m_rcSrc.left, m_rcSrc.top, m_rcSrc.right, m_rcSrc.bottom);
    }

    // 让corner的值不超过可绘制范围
    const UiRect corner = m_pProgressImage->GetImageAttribute().GetCorner();
    if (IsHorizontal()) {
        if (corner.left != 0 && corner.left >= rc.right) {
			std::wstring imageModify = m_sProgressImageModify.c_str();
			imageModify += StringHelper::Printf(L" corner='%d,%d,%d,%d'",
                rc.right,
                corner.top,
                0,
                corner.bottom);
			m_sProgressImageModify = imageModify;
        }
    }
    else {
        if (corner.top != 0 && corner.top >= rc.bottom) {
			std::wstring imageModify = m_sProgressImageModify.c_str();
			imageModify += StringHelper::Printf(L" corner='%d,%d,%d,%d'",
                corner.left,
                corner.bottom,
                corner.right,
                0);
			m_sProgressImageModify = imageModify;
        }
    }
	PaintImage(pRender, m_pProgressImage, m_sProgressImageModify.c_str());
}

UiRect Progress::GetProgressPos()
{
    UiRect rc;
    if (m_bHorizontal) {
        if (m_bReverse) {
            rc.right = GetRect().Width();
            rc.left = rc.right - static_cast<int>(std::floor(static_cast<double>((m_nValue - m_nMin) * (GetRect().right - GetRect().left)) / static_cast<double>(m_nMax - m_nMin)));
        }
        else {
            rc.right = static_cast<int>(std::ceil(static_cast<double>((m_nValue - m_nMin) * (GetRect().right - GetRect().left)) / static_cast<double>(m_nMax - m_nMin)));
        }

        rc.bottom = GetRect().bottom - GetRect().top;
    }
    else {
        if (m_bReverse) {
            rc.bottom = static_cast<int>(std::floor(static_cast<double>((m_nMax - m_nValue) * (GetRect().bottom - GetRect().top)) / static_cast<double>(m_nMax - m_nMin)));
        }
        else {
            rc.top = static_cast<int>(std::ceil(static_cast<double>((m_nMax - m_nValue) * (GetRect().bottom - GetRect().top)) / static_cast<double>(m_nMax - m_nMin)));
            rc.bottom = GetRect().bottom - GetRect().top;
        }

        rc.right = GetRect().right - GetRect().left;
    }

    return rc;
}

void Progress::ClearImageCache()
{
	__super::ClearImageCache();
	if (m_pProgressImage != nullptr) {
		m_pProgressImage->ClearImageCache();
	}	
}

void Progress::Play()
{
    if (!m_bMarquee) {
        m_timer.Cancel();
        return;
    }
    m_nMarqueePos = m_nMarqueePos + m_nMarqueeStep;

    ui::UiRect rc = GetRect();
    if (m_bHorizontal) {
        if (m_nMarqueePos > rc.right - rc.left) {
            m_nMarqueePos = (m_nMarqueePos - (rc.right - rc.left)) - m_nMarqueeWidth;
        }
    }
    else {
        if (m_nMarqueePos > rc.bottom - rc.top) {
            m_nMarqueePos = (m_nMarqueePos - (rc.bottom - rc.top) - m_nMarqueeWidth);
        }
    }

    Invalidate();
}

void Progress::PaintMarquee(IRender* pRender) 
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (!m_sProgressColor.empty()) {
		UiColor dwProgressColor = GlobalManager::Instance().Color().GetColor(m_sProgressColor.c_str());
		if (dwProgressColor.GetARGB() != 0) {
			UiRect rcProgressColor = GetRect();
			ui::UiRect rc = GetRect();
			if (m_bHorizontal) {
				rc.left = std::max(m_nMarqueePos, 0) + rc.left;
				rc.right = rc.left + (m_nMarqueePos >= 0 ? m_nMarqueeWidth : (m_nMarqueeWidth + m_nMarqueePos));
			}
			else {
				rc.top = std::max(m_nMarqueePos, 0) + rc.top;
				rc.bottom = rc.top + (m_nMarqueePos >= 0 ? m_nMarqueeWidth : (m_nMarqueeWidth + m_nMarqueePos));
			}
			GlobalManager::Instance().Dpi().ScaleRect(rc);
			pRender->FillRect(rc, dwProgressColor);
		}
	}
}

bool Progress::IsMarquee()
{
	return m_bMarquee;
}

void Progress::SetMarquee(bool bMarquee)
{
	if (m_bMarquee == bMarquee) {
		return;
	}

	m_bMarquee = bMarquee;
	m_nMarqueePos = 0;

	if (m_bMarquee) {
		auto playCallback = nbase::Bind(&Progress::Play, this);
		GlobalManager::Instance().Timer().AddCancelableTimer(m_timer.GetWeakFlag(), playCallback, m_nMarqueeElapsed, TimerManager::REPEAT_FOREVER);
	}
	else {
		m_timer.Cancel();
	}
	Invalidate();
}

int Progress::GetMarqueeWidth()
{
	return m_nMarqueeWidth;
}

void Progress::SetMarqueeWidth(int nMarqueeWidth)
{
	if (m_nMarqueeWidth == nMarqueeWidth) {
		return;
	}
	m_nMarqueeWidth = nMarqueeWidth;
	Invalidate();
}

int Progress::GetMarqueeStep()
{
	return m_nMarqueeStep;
}

void Progress::SetMarqueeStep(int nMarqueeStep)
{
	if (m_nMarqueeStep == nMarqueeStep) {
		return;
	}
	m_nMarqueeStep = nMarqueeStep;
	Invalidate();
}

int Progress::GetMarqueeElapsed()
{
	return m_nMarqueeElapsed;
}

void Progress::SetMarqueeElapsed(int nMarqueeElapsed)
{
	if (!IsMarquee() || nMarqueeElapsed == m_nMarqueeElapsed) {
		return;
	}

	m_nMarqueeElapsed = nMarqueeElapsed;
	m_timer.Cancel();

	auto playCallback = nbase::Bind(&Progress::Play, this);
	GlobalManager::Instance().Timer().AddCancelableTimer(m_timer.GetWeakFlag(), playCallback, m_nMarqueeElapsed, TimerManager::REPEAT_FOREVER);

	Invalidate();
}

void Progress::SetReverse(bool bReverse)
{
	m_bReverse = bReverse;
}

}
