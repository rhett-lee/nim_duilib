#include "ScrollBar.h"
#include "duilib/Core/ScrollBox.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

ScrollBar::ScrollBar() :
	m_bHorizontal(false),
	m_bShowButton1(true),
	m_bShowButton2(true),
	m_bAutoHide(true),
	m_nRange(100),
	m_nScrollPos(0),
	m_nLineSize(8),
	m_nThumbMinLength(30),
	m_nLastScrollPos(0),
	m_nLastScrollOffset(0),
	m_nScrollRepeatDelay(0),
	m_pOwner(nullptr),
	m_ptLastMouse({ 0, 0 }),
	m_rcButton1(0, 0, 0, 0),
	m_rcButton2(0, 0, 0, 0),
	m_rcThumb(0, 0, 0, 0),
	m_uButton1State(kControlStateNormal),
	m_uButton2State(kControlStateNormal),
	m_uThumbState(kControlStateNormal),
	m_sImageModify(),
	m_bkStateImage(),
	m_button1StateImage(),
	m_button2StateImage(),
	m_thumbStateImage(),
	m_railStateImage(),
	m_weakFlagOwner()
{
	m_bkStateImage = std::make_unique<StateImage>();
	m_button1StateImage = std::make_unique<StateImage>();
	m_button2StateImage = std::make_unique<StateImage>();
	m_thumbStateImage = std::make_unique<StateImage>();
	m_railStateImage = std::make_unique<StateImage>();

	m_bkStateImage->SetControl(this);
	m_thumbStateImage->SetControl(this);

	SetFixedWidth(DEFAULT_SCROLLBAR_SIZE, false, false);
	SetFixedHeight(0, false);
	m_ptLastMouse.x = 0;
	m_ptLastMouse.y = 0;
	SetFloat(true);
}

ScrollBox* ScrollBar::GetOwner() const
{
	return m_pOwner;
}

void ScrollBar::SetOwner(ScrollBox* pOwner)
{
	m_pOwner = pOwner;
}

std::wstring ScrollBar::GetType() const { return DUI_CTR_SCROLLBAR; }

void ScrollBar::SetEnabled(bool bEnable)
{
	Control::SetEnabled(bEnable);
	if( bEnable ) {
		m_uButton1State = kControlStateNormal;
		m_uButton2State = kControlStateNormal;
		m_uThumbState = kControlStateNormal;
	}
	else {
		m_uButton1State = kControlStateDisabled;
		m_uButton2State = kControlStateDisabled;
		m_uThumbState = kControlStateDisabled;
	}
}

void ScrollBar::SetFocus()
{
	if (m_pOwner != nullptr) {
		m_pOwner->SetFocus();
	}
	else {
		Control::SetFocus();
	}
}

void ScrollBar::SetVisible(bool bVisible)
{
	if (IsVisible() == bVisible) {
		return;
	}
	bool v = IsVisible();
	__super::SetVisible(bVisible);
	if( IsVisible() != v) {
		ArrangeSelf();
	}
}

bool ScrollBar::ButtonUp(const EventArgs& msg)
{
	bool ret = false;
	if( IsMouseFocused() ) {
		SetMouseFocused(false);
		Invalidate();
		UiRect pos = GetPos();
		if (pos.ContainsPt(msg.ptMouse)) {
			SetState(kControlStateHot);
			ret = true;
		}
		else {
			SetState(kControlStateNormal);
		}
	}

	UiRect ownerPos = m_pOwner->GetPos();
	if (m_bAutoHide && !ownerPos.ContainsPt(msg.ptMouse)) {
		SetFadeVisible(false);
	}

	return ret;
}

bool ScrollBar::HasHotState()
{
	return true;
}

bool ScrollBar::MouseEnter(const EventArgs& msg)
{
	bool ret = __super::MouseEnter(msg);
	if (ret) {
		m_uButton1State = kControlStateHot;
		m_uButton2State = kControlStateHot;
		m_uThumbState = kControlStateHot;
	}

	return ret;
}

bool ScrollBar::MouseLeave(const EventArgs& msg)
{
	bool ret = __super::MouseLeave(msg);
	if (ret) {
		m_uButton1State = kControlStateNormal;
		m_uButton2State = kControlStateNormal;
		m_uThumbState = kControlStateNormal;
	}

	return ret;
}

void ScrollBar::SetPos(UiRect rc)
{
	Control::SetPos(rc);
	rc = GetRect();

	if (m_bHorizontal) {
		int cx = rc.right - rc.left;
		if (m_bShowButton1) {
			cx -= GetFixedHeight();
		}
		if (m_bShowButton2) {
			cx -= GetFixedHeight();
		}
		if (cx > GetFixedHeight()) {
			m_rcButton1.left = rc.left;
			m_rcButton1.top = rc.top;
			if (m_bShowButton1) {
				m_rcButton1.right = rc.left + GetFixedHeight();
				m_rcButton1.bottom = rc.top + GetFixedHeight();
			}
			else {
				m_rcButton1.right = m_rcButton1.left;
				m_rcButton1.bottom = m_rcButton1.top;
			}

			m_rcButton2.top = rc.top;
			m_rcButton2.right = rc.right;
			if (m_bShowButton2) {
				m_rcButton2.left = rc.right - GetFixedHeight();
				m_rcButton2.bottom = rc.top + GetFixedHeight();
			}
			else {
				m_rcButton2.left = m_rcButton2.right;
				m_rcButton2.bottom = m_rcButton2.top;
			}

			m_rcThumb.top = rc.top;
			m_rcThumb.bottom = rc.top + GetFixedHeight();
			if (m_nRange > 0) {
				int64_t cxThumb = (int64_t)cx * (rc.right - rc.left) / (m_nRange + rc.right - rc.left);
				if (cxThumb < m_nThumbMinLength) {
					cxThumb = m_nThumbMinLength;
				}

				m_rcThumb.left = static_cast<LONG>(m_nScrollPos * (cx - cxThumb) / m_nRange + m_rcButton1.right);
				m_rcThumb.right = static_cast<LONG>(m_rcThumb.left + cxThumb);
				if (m_rcThumb.right > m_rcButton2.left) {
					m_rcThumb.left = static_cast<LONG>(m_rcButton2.left - cxThumb);
					m_rcThumb.right = m_rcButton2.left;
				}
			}
			else {
				m_rcThumb.left = m_rcButton1.right;
				m_rcThumb.right = m_rcButton2.left;
			}
		}
		else {
			int cxButton = (rc.right - rc.left) / 2;
			if (cxButton > GetFixedHeight()) {
				cxButton = GetFixedHeight();
			}
			m_rcButton1.left = rc.left;
			m_rcButton1.top = rc.top;
			if (m_bShowButton1) {
				m_rcButton1.right = rc.left + cxButton;
				m_rcButton1.bottom = rc.top + GetFixedHeight();
			}
			else {
				m_rcButton1.right = m_rcButton1.left;
				m_rcButton1.bottom = m_rcButton1.top;
			}

			m_rcButton2.top = rc.top;
			m_rcButton2.right = rc.right;
			if (m_bShowButton2) {
				m_rcButton2.left = rc.right - cxButton;
				m_rcButton2.bottom = rc.top + GetFixedHeight();
			}
			else {
				m_rcButton2.left = m_rcButton2.right;
				m_rcButton2.bottom = m_rcButton2.top;
			}

			::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
		}
	}
	else {
		int cy = rc.bottom - rc.top;
		if (m_bShowButton1) {
			cy -= GetFixedWidth();
		}
		if (m_bShowButton2) {
			cy -= GetFixedWidth();
		}
		if (cy > GetFixedWidth()) {
			m_rcButton1.left = rc.left;
			m_rcButton1.top = rc.top;
			if (m_bShowButton1) {
				m_rcButton1.right = rc.left + GetFixedWidth();
				m_rcButton1.bottom = rc.top + GetFixedWidth();
			}
			else {
				m_rcButton1.right = m_rcButton1.left;
				m_rcButton1.bottom = m_rcButton1.top;
			}

			m_rcButton2.left = rc.left;
			m_rcButton2.bottom = rc.bottom;
			if (m_bShowButton2) {
				m_rcButton2.top = rc.bottom - GetFixedWidth();
				m_rcButton2.right = rc.left + GetFixedWidth();
			}
			else {
				m_rcButton2.top = m_rcButton2.bottom;
				m_rcButton2.right = m_rcButton2.left;
			}

			m_rcThumb.left = rc.left;
			m_rcThumb.right = rc.left + GetFixedWidth();
			if (m_nRange > 0) {
				int64_t cyThumb = (int64_t)cy * (rc.bottom - rc.top) / (m_nRange + rc.bottom - rc.top);
				if (cyThumb < m_nThumbMinLength) cyThumb = m_nThumbMinLength;

				m_rcThumb.top = static_cast<LONG>(m_nScrollPos * (cy - cyThumb) / m_nRange + m_rcButton1.bottom);
				m_rcThumb.bottom = static_cast<LONG>(m_rcThumb.top + cyThumb);
				if (m_rcThumb.bottom > m_rcButton2.top) {
					m_rcThumb.top = static_cast<LONG>(m_rcButton2.top - cyThumb);
					m_rcThumb.bottom = m_rcButton2.top;
				}
			}
			else {
				m_rcThumb.top = m_rcButton1.bottom;
				m_rcThumb.bottom = m_rcButton2.top;
			}
		}
		else {
			int cyButton = (rc.bottom - rc.top) / 2;
			if (cyButton > GetFixedWidth()) {
				cyButton = GetFixedWidth();
			}
			m_rcButton1.left = rc.left;
			m_rcButton1.top = rc.top;
			if (m_bShowButton1) {
				m_rcButton1.right = rc.left + GetFixedWidth();
				m_rcButton1.bottom = rc.top + cyButton;
			}
			else {
				m_rcButton1.right = m_rcButton1.left;
				m_rcButton1.bottom = m_rcButton1.top;
			}

			m_rcButton2.left = rc.left;
			m_rcButton2.bottom = rc.bottom;
			if (m_bShowButton2) {
				m_rcButton2.top = rc.bottom - cyButton;
				m_rcButton2.right = rc.left + GetFixedWidth();
			}
			else {
				m_rcButton2.top = m_rcButton2.bottom;
				m_rcButton2.right = m_rcButton2.left;
			}

			::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
		}
	}
}

void ScrollBar::HandleEvent(const EventArgs& event)
{
	if (!IsMouseEnabled() && 
		(event.Type > kEventMouseBegin) && 
		(event.Type < kEventMouseEnd)) {
		//当前控件禁止接收鼠标消息时，将鼠标相关消息转发给上层处理
		if (m_pOwner != nullptr) {
			m_pOwner->SendEvent(event);
		}
		return;
	}
	else if ((event.Type == kEventMouseButtonDown) || (event.Type == kEventMouseDoubleClick)) {
		if (!IsEnabled()) {
			return;
		}

		m_nLastScrollOffset = 0;
		m_nScrollRepeatDelay = 0;

		auto callback = nbase::Bind(&ScrollBar::ScrollTimeHandle, this);
		GlobalManager::Instance().Timer().AddCancelableTimer(m_weakFlagOwner.GetWeakFlag(), callback, 50, TimerManager::REPEAT_FOREVER);

		if (m_rcButton1.ContainsPt(event.ptMouse)) {
			m_uButton1State = kControlStatePushed;
			if (!m_bHorizontal) {
				if (m_pOwner != nullptr) {
					m_pOwner->LineUp();
				}
				else {
					SetScrollPos(m_nScrollPos - m_nLineSize);
				}
			}
			else {
				if (m_pOwner != nullptr) {
					m_pOwner->LineLeft();
				}
				else {
					SetScrollPos(m_nScrollPos - m_nLineSize);
				}
			}
		}
		else if (m_rcButton2.ContainsPt(event.ptMouse)) {
			m_uButton2State = kControlStatePushed;
			if (!m_bHorizontal) {
				if (m_pOwner != nullptr) {
					m_pOwner->LineDown();
				}
				else {
					SetScrollPos(m_nScrollPos + m_nLineSize);
				}
			}
			else {
				if (m_pOwner != nullptr) {
					m_pOwner->LineRight();
				}
				else {
					SetScrollPos(m_nScrollPos + m_nLineSize);
				}
			}
		}
		else if (m_rcThumb.ContainsPt(event.ptMouse)) {
			m_uThumbState = kControlStatePushed;
			SetMouseFocused(true);
			m_ptLastMouse = event.ptMouse;
			m_nLastScrollPos = m_nScrollPos;
		}
		else {
			if (!m_bHorizontal) {
				if (event.ptMouse.y < m_rcThumb.top) {
					if (m_pOwner != nullptr) {
						m_pOwner->PageUp();
					}
					else {
						SetScrollPos(m_nScrollPos + GetRect().top - GetRect().bottom);
					}
				}
				else if (event.ptMouse.y > m_rcThumb.bottom){
					if (m_pOwner != nullptr) {
						m_pOwner->PageDown();
					}
					else {
						SetScrollPos(m_nScrollPos - GetRect().top + GetRect().bottom);
					}
				}
			}
			else {
				if (event.ptMouse.x < m_rcThumb.left) {
					if (m_pOwner != nullptr) {
						m_pOwner->PageLeft();
					}
					else {
						SetScrollPos(m_nScrollPos + GetRect().left - GetRect().right);
					}
				}
				else if (event.ptMouse.x > m_rcThumb.right){
					if (m_pOwner != nullptr) {
						m_pOwner->PageRight();
					}
					else {
						SetScrollPos(m_nScrollPos - GetRect().left + GetRect().right);
					}
				}
			}
		}

		ButtonDown(event);
		return;
	}
	else if ((event.Type == kEventMouseButtonUp) || (event.Type == kEventWindowKillFocus) ){
		m_nScrollRepeatDelay = 0;
		m_nLastScrollOffset = 0;

		m_weakFlagOwner.Cancel();

		if (IsMouseFocused()) {
			if (GetRect().ContainsPt(event.ptMouse)) {
				m_uThumbState = kControlStateHot;
			}
			else {
				m_uThumbState = kControlStateNormal;
			}
		}
		else if (m_uButton1State == kControlStatePushed) {
			m_uButton1State = kControlStateNormal;
			Invalidate();
		}
		else if (m_uButton2State == kControlStatePushed) {
			m_uButton2State = kControlStateNormal;
			Invalidate();
		}

		ButtonUp(event);
		return;
	}
	else if (event.Type == kEventMouseEnter) {
		MouseEnter(event);
	}
	else if (event.Type == kEventMouseLeave) {
		MouseLeave(event);
	}
	else if (event.Type == kEventMouseMove) {
		if (IsMouseFocused()) {
			if (!m_bHorizontal) {

				int vRange = GetRect().bottom - GetRect().top - m_rcThumb.bottom + m_rcThumb.top;
				if (m_bShowButton1) {
					vRange -= GetFixedWidth();
				}
				if (m_bShowButton2) {
					vRange -= GetFixedWidth();
				}

				if (vRange != 0) {
					m_nLastScrollOffset = (event.ptMouse.y - m_ptLastMouse.y) * m_nRange / vRange;
				}
			}
			else {

				int hRange = GetRect().right - GetRect().left - m_rcThumb.right + m_rcThumb.left;
				if (m_bShowButton1) {
					hRange -= GetFixedHeight();
				}
				if (m_bShowButton2) {
					hRange -= GetFixedHeight();
				}

				if (hRange != 0) {
					m_nLastScrollOffset = (event.ptMouse.x - m_ptLastMouse.x) * m_nRange / hRange;
				}
			}
		}

		return;
	}
	else if (event.Type == kEventSetCursor) {
		if (GetCursorType() == kCursorHand) {
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
			return;
		}
		else if (GetCursorType() == kCursorArrow){
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			return;
		}
		else {
			ASSERT(FALSE);
		}
	}

	if (m_pOwner != nullptr) {
		m_pOwner->SendEvent(event);
	}
}

void ScrollBar::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if ((strName == L"button1_normal_image") || (strName == L"button1normalimage")) {
		SetButton1StateImage(kControlStateNormal, strValue);
	}
	else if ((strName == L"button1_hot_image") || (strName == L"button1hotimage")) {
		SetButton1StateImage(kControlStateHot, strValue);
	}
	else if ((strName == L"button1_pushed_image") || (strName == L"button1pushedimage")) {
		SetButton1StateImage(kControlStatePushed, strValue);
	}
	else if ((strName == L"button1_disabled_image") || (strName == L"button1disabledimage")) {
		SetButton1StateImage(kControlStateDisabled, strValue);
	}
	else if ((strName == L"button2_normal_image") || (strName == L"button2normalimage")) {
		SetButton2StateImage(kControlStateNormal, strValue);
	}
	else if ((strName == L"button2_hot_image") || (strName == L"button2hotimage")) {
		SetButton2StateImage(kControlStateHot, strValue);
	}
	else if ((strName == L"button2_pushed_image") || (strName == L"button2pushedimage")){
		SetButton2StateImage(kControlStatePushed, strValue);
	}
	else if ((strName == L"button2_disabled_image") || (strName == L"button2disabledimage")) {
		SetButton2StateImage(kControlStateDisabled, strValue);
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
	else if ((strName == L"rail_normal_image") && (strName == L"railnormalimage")) {
		SetRailStateImage(kControlStateNormal, strValue);
	}
	else if ((strName == L"rail_hot_image") && (strName == L"railhotimage")) {
		SetRailStateImage(kControlStateHot, strValue);
	}
	else if ((strName == L"rail_pushed_image") && (strName == L"railpushedimage")) {
		SetRailStateImage(kControlStatePushed, strValue);
	}
	else if ((strName == L"rail_disabled_image") && (strName == L"raildisabledimage")) {
		SetRailStateImage(kControlStateDisabled, strValue);
	}
	else if ((strName == L"bk_normal_image") || (strName == L"bknormalimage")) {
		SetBkStateImage(kControlStateNormal, strValue);
	}
	else if ((strName == L"bk_hot_image") || (strName == L"bkhotimage")) {
		SetBkStateImage(kControlStateHot, strValue);
	}
	else if ((strName == L"bk_pushed_image") || (strName == L"bkpushedimage")) {
		SetBkStateImage(kControlStatePushed, strValue);
	}
	else if ((strName == L"bk_disabled_image") || (strName == L"bkdisabledimage")) {
		SetBkStateImage(kControlStateDisabled, strValue);
	}
	else if ((strName == L"horizontal") || (strName == L"hor")) {
		SetHorizontal(strValue == L"true");
	}
	else if ((strName == L"line_size") || (strName == L"linesize")) {
		SetLineSize(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"thumb_min_length") || (strName == L"thumbminlength")) {
		SetThumbMinLength(_wtoi(strValue.c_str()));
	}
	else if (strName == L"range") {
		SetScrollRange(_wtoi(strValue.c_str()));
	}
	else if (strName == L"value") {
		SetScrollPos(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"show_button1") || (strName == L"showbutton1")) {
		SetShowButton1(strValue == L"true");
	}
	else if ((strName == L"show_button2") || (strName == L"showbutton2")) {
		SetShowButton2(strValue == L"true");
	}
	else if ((strName == L"auto_hide_scroll") || (strName == L"autohidescroll")){
		SetAutoHideScroll(strValue == L"true");
	}
	else {
		Control::SetAttribute(strName, strValue);
	}
}

void ScrollBar::Paint(IRender* pRender, const UiRect& rcPaint)
{
	UiRect paintRect = GetPaintRect();
	if (!UiRect::Intersect(paintRect, rcPaint, GetRect())) {
		return;
	}
	SetPaintRect(paintRect);

	PaintBk(pRender);
	PaintButton1(pRender);
	PaintButton2(pRender);
	PaintThumb(pRender);
	PaintRail(pRender);
}

void ScrollBar::ClearImageCache()
{
	__super::ClearImageCache();
	m_bkStateImage->ClearImageCache();
	m_button1StateImage->ClearImageCache();
	m_button2StateImage->ClearImageCache();
	m_thumbStateImage->ClearImageCache();
	m_railStateImage->ClearImageCache();
}

bool ScrollBar::IsHorizontal()
{
	return m_bHorizontal;
}

void ScrollBar::SetHorizontal(bool bHorizontal)
{
	if (m_bHorizontal == bHorizontal) {
		return;
	}

	m_bHorizontal = bHorizontal;
	if( m_bHorizontal ) {
		if(GetFixedHeight() == 0 ) {
			SetFixedWidth(0, false, false);
			SetFixedHeight(DEFAULT_SCROLLBAR_SIZE, false);
		}
	}
	else {
		if(GetFixedWidth() == 0) {
			SetFixedWidth(DEFAULT_SCROLLBAR_SIZE, false, false);
			SetFixedHeight(0, false);
		}
	}

	if (m_pOwner != nullptr) {
		m_pOwner->Arrange();
	}		
	else {
		ArrangeAncestor();
	}
}

int64_t ScrollBar::GetScrollRange() const
{
	return m_nRange;
}

void ScrollBar::SetScrollRange(int64_t nRange)
{
	if (m_nRange == nRange) {
		return;
	}
	ASSERT(nRange >= 0);
	m_nRange = nRange;
	if (m_nRange < 0) {
		m_nRange = 0;
	}
	if (m_nScrollPos > m_nRange) {
		m_nScrollPos = m_nRange;
	}

	if (m_nRange == 0) {
		SetVisible(false);
	}
	else if (!m_bAutoHide && !IsVisible()) {
		SetFadeVisible(true);
	}
	SetPos(GetRect());
}

int64_t ScrollBar::GetScrollPos() const
{
	return m_nScrollPos;
}

void ScrollBar::SetScrollPos(int64_t nPos)
{
	//ASSERT(nPos >= 0);
	if (nPos < 0) {
		nPos = 0;
	}
	if (nPos > m_nRange) {
		nPos = m_nRange;
	}
	if (m_nScrollPos == nPos) {
		return;
	}
	m_nScrollPos = nPos;	
	SetPos(GetRect());
}

int ScrollBar::GetLineSize() const
{
	return m_nLineSize;
}

void ScrollBar::SetLineSize(int nSize)
{
	ASSERT(nSize > 0);
	if (nSize > 0) {
		GlobalManager::Instance().Dpi().ScaleInt(nSize);
		m_nLineSize = nSize;
	}	
}

int ScrollBar::GetThumbMinLength() const
{
	return m_nThumbMinLength;
}

void ScrollBar::SetThumbMinLength(int nThumbMinLength)
{
	ASSERT(nThumbMinLength > 0);
	if (nThumbMinLength > 0) {
		GlobalManager::Instance().Dpi().ScaleInt(nThumbMinLength);
		m_nThumbMinLength = nThumbMinLength;
	}
}

bool ScrollBar::IsShowButton1()
{
	return m_bShowButton1;
}

void ScrollBar::SetShowButton1(bool bShow)
{
	m_bShowButton1 = bShow;
	SetPos(GetRect());
}

std::wstring ScrollBar::GetButton1StateImage(ControlStateType stateType)
{
	return (*m_button1StateImage).GetImageString(stateType);
}

void ScrollBar::SetButton1StateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
	(*m_button1StateImage).SetImageString(stateType, pStrImage);
	Invalidate();
}

bool ScrollBar::IsShowButton2()
{
	return m_bShowButton2;
}

void ScrollBar::SetShowButton2(bool bShow)
{
	m_bShowButton2 = bShow;
	SetPos(GetRect());
}

std::wstring ScrollBar::GetButton2StateImage(ControlStateType stateType)
{
	return (*m_button2StateImage).GetImageString(stateType);
}

void ScrollBar::SetButton2StateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
	(*m_button2StateImage).SetImageString(stateType, pStrImage);
	Invalidate();
}

std::wstring ScrollBar::GetThumbStateImage(ControlStateType stateType)
{
	return (*m_thumbStateImage).GetImageString(stateType);
}

void ScrollBar::SetThumbStateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
	(*m_thumbStateImage).SetImageString(stateType, pStrImage);
	Invalidate();
}

std::wstring ScrollBar::GetRailStateImage(ControlStateType stateType)
{
	return (*m_railStateImage).GetImageString(stateType);
}

void ScrollBar::SetRailStateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
	(*m_railStateImage).SetImageString(stateType, pStrImage);
	Invalidate();
}

std::wstring ScrollBar::GetBkStateImage(ControlStateType stateType)
{
	return (*m_bkStateImage).GetImageString(stateType);
}

void ScrollBar::SetBkStateImage(ControlStateType stateType, const std::wstring& pStrImage)
{
	(*m_bkStateImage).SetImageString(stateType, pStrImage);
	Invalidate();
}

void ScrollBar::SetAutoHideScroll(bool hide)
{
	m_bAutoHide = hide;
}

void ScrollBar::ScrollTimeHandle()
{
	++m_nScrollRepeatDelay;
	if(m_uThumbState == kControlStatePushed) {
		if( !m_bHorizontal ) {
			if (m_pOwner != nullptr) {
				m_pOwner->SetScrollPos(UiSize(m_pOwner->GetScrollPos().cx,
					                          static_cast<int>(m_nLastScrollPos + m_nLastScrollOffset)));
			}
			else {
				SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
			}
		}
		else {
			if (m_pOwner != nullptr) {
				m_pOwner->SetScrollPos(UiSize(static_cast<int>(m_nLastScrollPos + m_nLastScrollOffset),
									          m_pOwner->GetScrollPos().cy));
			}
			else {
				SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
			}
		}
	}
	else if( m_uButton1State == kControlStatePushed ) {
		if (m_nScrollRepeatDelay <= 5) {
			return;
		}
		if( !m_bHorizontal ) {
			if (m_pOwner != nullptr) {
				m_pOwner->LineUp();
			}
			else {
				SetScrollPos(m_nScrollPos - m_nLineSize);
			}
		}
		else {
			if (m_pOwner != nullptr) {
				m_pOwner->LineLeft();
			}
			else {
				SetScrollPos(m_nScrollPos - m_nLineSize);
			}
		}
	}
	else if( m_uButton2State == kControlStatePushed ) {
		if (m_nScrollRepeatDelay <= 5) {
			return;
		}
		if( !m_bHorizontal ) {
			if (m_pOwner != nullptr) {
				m_pOwner->LineDown();
			}
			else {
				SetScrollPos(m_nScrollPos + m_nLineSize);
			}
		}
		else {
			if (m_pOwner != nullptr) {
				m_pOwner->LineRight();
			}
			else {
				SetScrollPos(m_nScrollPos + m_nLineSize);
			}
		}
	}
	else {
		if (m_nScrollRepeatDelay <= 5) {
			return;
		}
		POINT pt = { 0 };
		::GetCursorPos(&pt);
		::ScreenToClient(GetWindow()->GetHWND(), &pt);
		if( !m_bHorizontal ) {
			if( pt.y < m_rcThumb.top ) {
				if (m_pOwner != nullptr) {
					m_pOwner->PageUp();
				}
				else {
					SetScrollPos(m_nScrollPos + GetRect().top - GetRect().bottom);
				}
			}
			else if ( pt.y > m_rcThumb.bottom ){
				if (m_pOwner != nullptr) {
					m_pOwner->PageDown();
				}
				else {
					SetScrollPos(m_nScrollPos - GetRect().top + GetRect().bottom);
				}
			}
		}
		else {
			if( pt.x < m_rcThumb.left ) {
				if (m_pOwner != nullptr) {
					m_pOwner->PageLeft();
				}
				else {
					SetScrollPos(m_nScrollPos + GetRect().left - GetRect().right);
				}
			}
			else if ( pt.x > m_rcThumb.right ){
				if (m_pOwner != nullptr) {
					m_pOwner->PageRight();
				}
				else {
					SetScrollPos(m_nScrollPos - GetRect().left + GetRect().right);
				}
			}
		}
	}
	return;
}

void ScrollBar::PaintBk(IRender* pRender)
{
	m_bkStateImage->PaintStateImage(pRender, GetState());
}

void ScrollBar::PaintButton1(IRender* pRender)
{
	if (!m_bShowButton1) {
		return;
	}

	m_sImageModify.clear();
	m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", m_rcButton1.left - GetRect().left, \
		m_rcButton1.top - GetRect().top, m_rcButton1.right - GetRect().left, m_rcButton1.bottom - GetRect().top);

	if (m_uButton1State == kControlStateDisabled) {
		if (PaintImage(pRender, (*m_button1StateImage).GetStateImage(kControlStateDisabled), m_sImageModify)) {
			return;
		}
	}
	else if (m_uButton1State == kControlStatePushed) {
		if (PaintImage(pRender, (*m_button1StateImage).GetStateImage(kControlStatePushed), m_sImageModify)) {
			return;
		}
		else if (PaintImage(pRender, (*m_button1StateImage).GetStateImage(kControlStateHot), m_sImageModify)) {
			return;
		}
	}
	else if (m_uButton1State == kControlStateHot || m_uThumbState == kControlStatePushed) {
		if (PaintImage(pRender, (*m_button1StateImage).GetStateImage(kControlStateHot), m_sImageModify)) {
			return;
		}
	}
	//如果各个状态绘制失败，默认绘制Normal状态的图片
	PaintImage(pRender, (*m_button1StateImage).GetStateImage(kControlStateNormal), m_sImageModify);
}

void ScrollBar::PaintButton2(IRender* pRender)
{
	if (!m_bShowButton2) {
		return;
	}
	m_sImageModify.clear();
	m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", m_rcButton2.left - GetRect().left, \
		m_rcButton2.top - GetRect().top, m_rcButton2.right - GetRect().left, m_rcButton2.bottom - GetRect().top);

	if (m_uButton2State == kControlStateDisabled) {
		if (PaintImage(pRender, (*m_button2StateImage).GetStateImage(kControlStateDisabled), m_sImageModify)) {
			return;
		}
	}
	else if (m_uButton2State == kControlStatePushed) {
		if (PaintImage(pRender, (*m_button2StateImage).GetStateImage(kControlStatePushed), m_sImageModify)) {
			if (PaintImage(pRender, (*m_button2StateImage).GetStateImage(kControlStateHot), m_sImageModify)) {
				return;
			}
		}
	}
	else if (m_uButton2State == kControlStateHot || m_uThumbState == kControlStatePushed) {
		if (PaintImage(pRender, (*m_button2StateImage).GetStateImage(kControlStateHot), m_sImageModify)) {
			return;
		}
	}
	//如果各个状态绘制失败，默认绘制Normal状态的图片
	PaintImage(pRender, (*m_button2StateImage).GetStateImage(kControlStateNormal), m_sImageModify);
}

void ScrollBar::PaintThumb(IRender* pRender)
{
	if (m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0) {
		return;
	}

	m_sImageModify.clear();
	m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", 
										  m_rcThumb.left - GetRect().left, 
										  m_rcThumb.top - GetRect().top, 
										  m_rcThumb.right - GetRect().left, 
									      m_rcThumb.bottom - GetRect().top);
	m_thumbStateImage->PaintStateImage(pRender, m_uThumbState, m_sImageModify);
}

void ScrollBar::PaintRail(IRender* pRender)
{
	if (m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0) {
		return;
	}

	m_sImageModify.clear();
	if (!m_bHorizontal) {
		m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", m_rcThumb.left - GetRect().left, \
			(m_rcThumb.top + m_rcThumb.bottom) / 2 - GetRect().top - GetFixedWidth() / 2, \
			m_rcThumb.right - GetRect().left, \
			(m_rcThumb.top + m_rcThumb.bottom) / 2 - GetRect().top + GetFixedWidth() - GetFixedWidth() / 2);
	}
	else {
		m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", \
			(m_rcThumb.left + m_rcThumb.right) / 2 - GetRect().left - GetFixedHeight() / 2, \
			m_rcThumb.top - GetRect().top, \
			(m_rcThumb.left + m_rcThumb.right) / 2 - GetRect().left + GetFixedHeight() - GetFixedHeight() / 2, \
			m_rcThumb.bottom - GetRect().top);
	}

	if (m_uThumbState == kControlStateDisabled) {
		if (PaintImage(pRender, (*m_railStateImage).GetStateImage(kControlStateDisabled), m_sImageModify)) {
			return;
		}
	}
	else if (m_uThumbState == kControlStatePushed) {
		if (PaintImage(pRender, (*m_railStateImage).GetStateImage(kControlStatePushed), m_sImageModify)) {
			if (PaintImage(pRender, (*m_railStateImage).GetStateImage(kControlStateHot), m_sImageModify)) {
				return;
			}
		}
	}
	else if (m_uThumbState == kControlStateHot) {
		if (PaintImage(pRender, (*m_railStateImage).GetStateImage(kControlStateHot), m_sImageModify)) {
			return;
		}
	}
	//绘制Normal状态的图片
	PaintImage(pRender, (*m_railStateImage).GetStateImage(kControlStateNormal), m_sImageModify);
}

}//namespace ui
