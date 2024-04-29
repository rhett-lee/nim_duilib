#include "Slider.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{

Slider::Slider() :
	m_szThumb(10, 10),
	m_nStep(1),
	m_sImageModify(),
	m_thumbStateImage(),
	m_rcProgressBarPadding()
{
	SetTextStyle(TEXT_SINGLELINE | TEXT_CENTER, false);
}

std::wstring Slider::GetType() const { return DUI_CTR_SLIDER; }

UiRect Slider::GetProgressPos()
{
	UiRect rc;
	if (m_bHorizontal) {
		rc.right = int((m_nValue - m_nMin) * (GetRect().right - GetRect().left - m_szThumb.cx) / (m_nMax - m_nMin) + m_szThumb.cx / 2 + 0.5);
		rc.bottom = GetRect().bottom - GetRect().top;
	}
	else {
		rc.top = int((m_nMax - m_nValue) * (GetRect().bottom - GetRect().top - m_szThumb.cy) / (m_nMax - m_nMin) + m_szThumb.cy / 2 + 0.5);
		rc.right = GetRect().right - GetRect().left;
		rc.bottom = GetRect().bottom - GetRect().top;
	}

	return rc;
}

void Slider::HandleEvent(const EventArgs& msg)
{
	if (IsDisabledEvents(msg)) {
		//如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
		Box* pParent = GetParent();
		if (pParent != nullptr) {
			pParent->SendEvent(msg);
		}
		else {
			__super::HandleEvent(msg);
		}
		return;
	}
	if (msg.Type == kEventMouseButtonDown || msg.Type == kEventMouseDoubleClick) {
		UiPoint newPtMouse(msg.ptMouse);
		newPtMouse.Offset(GetScrollOffsetInScrollBox());
		UiRect rcThumb = GetThumbRect();
		if (rcThumb.ContainsPt(newPtMouse)) {
			SetMouseFocused(true);
		}
		return;
	}
	if (msg.Type == kEventMouseButtonUp) {
		double oldValue = GetValue();
		if(IsMouseFocused()) {
			SetMouseFocused(false);
		}
		const int32_t nMin = GetMinValue();
		const int32_t nMax = GetMaxValue();
		if (m_bHorizontal) {			
			if (msg.ptMouse.x >= GetRect().right - m_szThumb.cx / 2) {
				SetValue(nMax);
			}
			else if (msg.ptMouse.x <= GetRect().left + m_szThumb.cx / 2) {
				SetValue(nMin);
			}
			else {
				double newValue = nMin + double((nMax - nMin) * (msg.ptMouse.x - GetRect().left - m_szThumb.cx / 2)) / (GetRect().right - GetRect().left - m_szThumb.cx);
				SetValue(newValue);
			}
		}
		else {
			if (msg.ptMouse.y >= GetRect().bottom - m_szThumb.cy / 2) {
				SetValue(nMax);
			}
			else if (msg.ptMouse.y <= GetRect().top + m_szThumb.cy / 2) {
				SetValue(nMin);
			}
			else {
				double newValue = nMin + double((nMax - nMin) * (GetRect().bottom - msg.ptMouse.y - m_szThumb.cy / 2)) / (GetRect().bottom - GetRect().top - m_szThumb.cy);
				SetValue(newValue);
			}
		}
		SendEvent(kEventValueChange, (WPARAM)GetValue(), (LPARAM)oldValue);
		Invalidate();
		return;
	}
	if (msg.Type == kEventMouseWheel) {
		double oldValue = GetValue();
		int detaValue = GET_WHEEL_DELTA_WPARAM(msg.wParam);
		if (detaValue > 0) {
			SetValue(GetValue() + GetChangeStep());
			SendEvent(kEventValueChange, (WPARAM)GetValue(), (LPARAM)oldValue);
			return;
		}
		else {
			SetValue(GetValue() - GetChangeStep());
			SendEvent(kEventValueChange, (WPARAM)GetValue(), (LPARAM)oldValue);
			return;
		}
	}
	if (msg.Type == kEventMouseMove) {
		const int32_t nMin = GetMinValue();
		const int32_t nMax = GetMaxValue();
		if (IsMouseFocused()) {
			double oldValue = GetValue();
			if (m_bHorizontal) {
				if (msg.ptMouse.x >= GetRect().right - m_szThumb.cx / 2) {
					SetValue(nMax);
				}
				else if (msg.ptMouse.x <= GetRect().left + m_szThumb.cx / 2) {
					SetValue(nMin);
				}
				else {
					double newValue = nMin + double((nMax - nMin) * (msg.ptMouse.x - GetRect().left - m_szThumb.cx / 2)) / (GetRect().right - GetRect().left - m_szThumb.cx);
					SetValue(newValue);
				}
			}
			else {
				if (msg.ptMouse.y >= GetRect().bottom - m_szThumb.cy / 2) {
					SetValue(nMax);
				}
				else if (msg.ptMouse.y <= GetRect().top + m_szThumb.cy / 2) {
					SetValue(nMin);
				}
				else {
					double newValue = nMin + double((nMax - nMin) * (GetRect().bottom - msg.ptMouse.y - m_szThumb.cy / 2)) / (GetRect().bottom - GetRect().top - m_szThumb.cy);
					SetValue(newValue);
				}
			}
			SendEvent(kEventValueChange, (WPARAM)GetValue(), (LPARAM)oldValue);
			Invalidate();
		}
		return;
	}

	Progress::HandleEvent(msg);
}

void Slider::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"step") {
		SetChangeStep(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"thumb_normal_image") || (strName == L"thumbnormalimage")) {
		SetThumbStateImage(kControlStateNormal, strValue);
	}
	else if ((strName == L"thumb_hot_image") || (strName == L"thumbhotimage")) {
		SetThumbStateImage(kControlStateHot, strValue);
	}
	else if ((strName == L"thumb_pushed_image") || (strName == L"thumbpushedimage")) {
		SetThumbStateImage(kControlStatePushed, strValue);
	}
	else if ((strName == L"thumb_disabled_image") || (strName == L"thumbdisabledimage")) {
		SetThumbStateImage(kControlStateDisabled, strValue);
	}
	else if ((strName == L"thumb_size") || (strName == L"thumbsize")) {
		UiSize szXY;
		AttributeUtil::ParseSizeValue(strValue.c_str(), szXY);
		SetThumbSize(szXY);
	}
	else if ((strName == L"progress_bar_padding") || (strName == L"progressbarpadding")) {
		UiPadding rcPadding;
		AttributeUtil::ParsePaddingValue(strValue.c_str(), rcPadding);
		SetProgressBarPadding(rcPadding);
	}
	else {
		Progress::SetAttribute(strName, strValue);
	}
}

void Slider::PaintBkColor(IRender* pRender)
{
	UiRect rc = GetRect();
	const UiPadding& padding = m_rcProgressBarPadding;
	rc.Deflate(padding);
	SetRect(rc);

	UiRect painttRect = GetPaintRect();
	painttRect.Deflate(padding);
	SetPaintRect(painttRect);

	Control::PaintBkColor(pRender);

	painttRect = GetPaintRect();
	painttRect.Inflate(padding);
	SetPaintRect(painttRect);

	rc = GetRect();
	rc.Inflate(padding);
	SetRect(rc);
}

void Slider::PaintStateImages(IRender* pRender)
{
	UiRect rc = GetRect();
	const UiPadding& padding = m_rcProgressBarPadding;
	rc.Deflate(padding);
	SetRect(rc);

	Progress::PaintStateImages(pRender);

	rc = GetRect();
	rc.Inflate(padding.left, padding.top, padding.right, padding.bottom);
	SetRect(rc);

	UiRect rcThumb = GetThumbRect();
	rcThumb.left -= GetRect().left;
	rcThumb.top -= GetRect().top;
	rcThumb.right -= GetRect().left;
	rcThumb.bottom -= GetRect().top;

	if (IsMouseFocused()) {
		m_sImageModify.clear();
		m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
		if (PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStatePushed), m_sImageModify.c_str())) {
			return;
		}
	}
	else if (GetState() == kControlStateHot) {
		m_sImageModify.clear();
		m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
		if (PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStateHot), m_sImageModify.c_str())) {
			return;
		}
	}

	m_sImageModify.clear();
	m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
	PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStateNormal), m_sImageModify.c_str());
}

void Slider::ClearImageCache()
{
	__super::ClearImageCache();
	m_thumbStateImage.ClearImageCache();
}

int Slider::GetChangeStep()
{
	return m_nStep;
}

void Slider::SetChangeStep(int step)
{
	m_nStep = step;
}

void Slider::SetThumbSize(UiSize szXY)
{
	GlobalManager::Instance().Dpi().ScaleSize(szXY);
	m_szThumb = szXY;
}

UiRect Slider::GetThumbRect() const
{
	if( m_bHorizontal ) {
		int left = int(GetRect().left + (GetRect().right - GetRect().left - m_szThumb.cx) * (m_nValue - m_nMin) / (m_nMax - m_nMin));
		int top = (GetRect().bottom + GetRect().top - m_szThumb.cy) / 2;
		return UiRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
	}
	else {
		int left = (GetRect().right + GetRect().left - m_szThumb.cx) / 2;
		int top = int(GetRect().bottom - m_szThumb.cy - (GetRect().bottom - GetRect().top - m_szThumb.cy) * (m_nValue - m_nMin) / (m_nMax - m_nMin));
		return UiRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
	}
}

std::wstring Slider::GetThumbStateImage(ControlStateType stateType) const
{
	return m_thumbStateImage.GetImageString(stateType);
}

void Slider::SetThumbStateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
	m_thumbStateImage.SetImageString(stateType, pStrImage);
	Invalidate();
}

const UiPadding& Slider::GetProgressBarPadding() const
{
	return m_rcProgressBarPadding;
}

void Slider::SetProgressBarPadding(UiPadding padding)
{
	GlobalManager::Instance().Dpi().ScalePadding(padding);
	if (!m_rcProgressBarPadding.Equals(padding)) {
		m_rcProgressBarPadding = padding;
		RelayoutOrRedraw();
	}	
}

}
