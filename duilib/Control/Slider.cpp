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
	m_uTextStyle = TEXT_SINGLELINE | TEXT_CENTER;
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

void Slider::HandleEvent(const EventArgs& event)
{
	if (!IsMouseEnabled() && 
		(event.Type > kEventMouseBegin) && 
		(event.Type < kEventMouseEnd)) {
		//当前控件禁止接收鼠标消息时，将鼠标相关消息转发给上层处理
		if (GetParent() != nullptr) {
			GetParent()->SendEvent(event);
		}
		else {
			Progress::HandleEvent(event);
		}
		return;
	}

	if (event.Type == kEventMouseButtonDown || event.Type == kEventMouseDoubleClick) {
		if( IsEnabled() ) {
			UiPoint newPtMouse(event.ptMouse);
			newPtMouse.Offset(GetScrollOffsetInScrollBox());
			UiRect rcThumb = GetThumbRect();
			if (rcThumb.ContainsPt(newPtMouse)) {
				SetMouseFocused(true);
			}
		}
		return;
	}
	if (event.Type == kEventMouseButtonUp) {
		if(IsMouseFocused()) {
			SetMouseFocused(false);
		}
		if (IsEnabled()) {
			if (m_bHorizontal) {
				if (event.ptMouse.x >= GetRect().right - m_szThumb.cx / 2) m_nValue = m_nMax;
				else if (event.ptMouse.x <= GetRect().left + m_szThumb.cx / 2) m_nValue = m_nMin;
				else m_nValue = m_nMin + double((m_nMax - m_nMin) * (event.ptMouse.x - GetRect().left - m_szThumb.cx / 2)) / (GetRect().right - GetRect().left - m_szThumb.cx);
			}
			else {
				if (event.ptMouse.y >= GetRect().bottom - m_szThumb.cy / 2) m_nValue = m_nMin;
				else if (event.ptMouse.y <= GetRect().top + m_szThumb.cy / 2) m_nValue = m_nMax;
				else m_nValue = m_nMin + double((m_nMax - m_nMin) * (GetRect().bottom - event.ptMouse.y - m_szThumb.cy / 2)) / (GetRect().bottom - GetRect().top - m_szThumb.cy);
			}
			SendEvent(kEventValueChange);
			Invalidate();
		}
		return;
	}
	if (event.Type == kEventMouseWheel) {
		int detaValue = static_cast<int>(event.wParam);
		if (detaValue > 0) {
			SetValue(GetValue() + GetChangeStep());
			SendEvent(kEventValueChange);
			return;
		}
		else {
			SetValue(GetValue() - GetChangeStep());
			SendEvent(kEventValueChange);
			return;
		}
	}
	if (event.Type == kEventMouseMove) {
		if (IsMouseFocused()) {
			if (m_bHorizontal) {
				if (event.ptMouse.x >= GetRect().right - m_szThumb.cx / 2) m_nValue = m_nMax;
				else if (event.ptMouse.x <= GetRect().left + m_szThumb.cx / 2) m_nValue = m_nMin;
				else m_nValue = m_nMin + double((m_nMax - m_nMin) * (event.ptMouse.x - GetRect().left - m_szThumb.cx / 2)) / (GetRect().right - GetRect().left - m_szThumb.cx);
			}
			else {
				if (event.ptMouse.y >= GetRect().bottom - m_szThumb.cy / 2) m_nValue = m_nMin;
				else if (event.ptMouse.y <= GetRect().top + m_szThumb.cy / 2) m_nValue = m_nMax;
				else m_nValue = m_nMin + double((m_nMax - m_nMin) * (GetRect().bottom - event.ptMouse.y - m_szThumb.cy / 2)) / (GetRect().bottom - GetRect().top - m_szThumb.cy);
			}
			SendEvent(kEventValueChange);
			Invalidate();
		}
		return;
	}

	Progress::HandleEvent(event);
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

void Slider::PaintStatusImage(IRender* pRender)
{
	UiRect rc = GetRect();
	const UiPadding& padding = m_rcProgressBarPadding;
	rc.Deflate(padding);
	SetRect(rc);

	Progress::PaintStatusImage(pRender);

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
		if (!PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStatePushed), m_sImageModify)) {

		}
		else return;
	}
	else if (GetState() == kControlStateHot) {
		m_sImageModify.clear();
		m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
		if (!PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStateHot), m_sImageModify)) {

		}
		else return;
	}

	m_sImageModify.clear();
	m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
	if (!PaintImage(pRender, m_thumbStateImage.GetStateImage(kControlStateNormal), m_sImageModify)) {

	}
	else return;
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
	m_thumbStateImage.GetStateImage(stateType).SetImageString(pStrImage);
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
