#include "ScrollBar.h"
#include "duilib/Box/ScrollBox.h"
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

	//TODO: DPI自适应参数，需要确认一下
	SetFixedWidth(UiFixedInt(DEFAULT_SCROLLBAR_SIZE), false, false);
	SetFixedHeight(UiFixedInt(0), true, false);
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
	else if ((strName == L"button2_pushed_image") || (strName == L"button2pushedimage")) {
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
	else if ((strName == L"auto_hide_scroll") || (strName == L"autohidescroll")) {
		SetAutoHideScroll(strValue == L"true");
	}
	else {
		Control::SetAttribute(strName, strValue);
	}
}

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
	UiPoint pt(msg.ptMouse);
	pt.Offset(GetScrollOffsetInScrollBox());
	bool ret = false;
	if( IsMouseFocused() ) {
		SetMouseFocused(false);
		Invalidate();
		UiRect pos = GetPos();
		if (pos.ContainsPt(pt)) {
			SetState(kControlStateHot);
			ret = true;
		}
		else {
			SetState(kControlStateNormal);
		}
	}

	UiRect ownerPos = m_pOwner->GetPos();
	if (m_bAutoHide && !ownerPos.ContainsPt(pt)) {
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
	if (IsHotState()) {
		m_uButton1State = kControlStateHot;
		m_uButton2State = kControlStateHot;
		m_uThumbState = kControlStateHot;
	}
	return ret;
}

bool ScrollBar::MouseLeave(const EventArgs& msg)
{
	bool ret = __super::MouseLeave(msg);
	if (!IsHotState()) {
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
		ASSERT(GetFixedHeight().GetInt32() > 0);
		int cx = rc.Width();
		if (m_bShowButton1) {			
			cx -= GetFixedHeight().GetInt32();
		}
		if (m_bShowButton2) {
			cx -= GetFixedHeight().GetInt32();
		}
		if (cx > GetFixedHeight().GetInt32()) {
			m_rcButton1.left = rc.left;
			m_rcButton1.top = rc.top;
			if (m_bShowButton1) {
				m_rcButton1.right = rc.left + GetFixedHeight().GetInt32();
				m_rcButton1.bottom = rc.top + GetFixedHeight().GetInt32();
			}
			else {
				m_rcButton1.right = m_rcButton1.left;
				m_rcButton1.bottom = m_rcButton1.top;
			}

			m_rcButton2.top = rc.top;
			m_rcButton2.right = rc.right;
			if (m_bShowButton2) {
				m_rcButton2.left = rc.right - GetFixedHeight().GetInt32();
				m_rcButton2.bottom = rc.top + GetFixedHeight().GetInt32();
			}
			else {
				m_rcButton2.left = m_rcButton2.right;
				m_rcButton2.bottom = m_rcButton2.top;
			}

			m_rcThumb.top = rc.top;
			m_rcThumb.bottom = rc.top + GetFixedHeight().GetInt32();
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
			if (cxButton > GetFixedHeight().GetInt32()) {
				cxButton = GetFixedHeight().GetInt32();
			}
			m_rcButton1.left = rc.left;
			m_rcButton1.top = rc.top;
			if (m_bShowButton1) {
				m_rcButton1.right = rc.left + cxButton;
				m_rcButton1.bottom = rc.top + GetFixedHeight().GetInt32();
			}
			else {
				m_rcButton1.right = m_rcButton1.left;
				m_rcButton1.bottom = m_rcButton1.top;
			}

			m_rcButton2.top = rc.top;
			m_rcButton2.right = rc.right;
			if (m_bShowButton2) {
				m_rcButton2.left = rc.right - cxButton;
				m_rcButton2.bottom = rc.top + GetFixedHeight().GetInt32();
			}
			else {
				m_rcButton2.left = m_rcButton2.right;
				m_rcButton2.bottom = m_rcButton2.top;
			}

			::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
		}
	}
	else {
		ASSERT(GetFixedWidth().GetInt32() > 0);
		int cy = rc.Height();
		if (m_bShowButton1) {
			cy -= GetFixedWidth().GetInt32();
		}
		if (m_bShowButton2) {
			cy -= GetFixedWidth().GetInt32();
		}
		if (cy > GetFixedWidth().GetInt32()) {
			m_rcButton1.left = rc.left;
			m_rcButton1.top = rc.top;
			if (m_bShowButton1) {
				m_rcButton1.right = rc.left + GetFixedWidth().GetInt32();
				m_rcButton1.bottom = rc.top + GetFixedWidth().GetInt32();
			}
			else {
				m_rcButton1.right = m_rcButton1.left;
				m_rcButton1.bottom = m_rcButton1.top;
			}

			m_rcButton2.left = rc.left;
			m_rcButton2.bottom = rc.bottom;
			if (m_bShowButton2) {
				m_rcButton2.top = rc.bottom - GetFixedWidth().GetInt32();
				m_rcButton2.right = rc.left + GetFixedWidth().GetInt32();
			}
			else {
				m_rcButton2.top = m_rcButton2.bottom;
				m_rcButton2.right = m_rcButton2.left;
			}

			m_rcThumb.left = rc.left;
			m_rcThumb.right = rc.left + GetFixedWidth().GetInt32();
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
			if (cyButton > GetFixedWidth().GetInt32()) {
				cyButton = GetFixedWidth().GetInt32();
			}
			m_rcButton1.left = rc.left;
			m_rcButton1.top = rc.top;
			if (m_bShowButton1) {
				m_rcButton1.right = rc.left + GetFixedWidth().GetInt32();
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
				m_rcButton2.right = rc.left + GetFixedWidth().GetInt32();
			}
			else {
				m_rcButton2.top = m_rcButton2.bottom;
				m_rcButton2.right = m_rcButton2.left;
			}

			::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
		}
	}
}

void ScrollBar::HandleEvent(const EventArgs& msg)
{
	if (IsDisabledEvents(msg)) {
		//如果是鼠标键盘消息，并且控件是Disabled的，转发给Owner控件
		if (m_pOwner != nullptr) {
			m_pOwner->SendEvent(msg);
		}
		else {
			__super::HandleEvent(msg);
		}
		return;
	}

	UiPoint pt(msg.ptMouse);
	pt.Offset(GetScrollOffsetInScrollBox());
	if ((msg.Type == kEventMouseButtonDown) || (msg.Type == kEventMouseDoubleClick)) {
		m_nLastScrollOffset = 0;
		m_nScrollRepeatDelay = 0;

		auto callback = nbase::Bind(&ScrollBar::ScrollTimeHandle, this);
		GlobalManager::Instance().Timer().AddCancelableTimer(m_weakFlagOwner.GetWeakFlag(), callback, 50, TimerManager::REPEAT_FOREVER);

		if (m_rcButton1.ContainsPt(pt)) {
			//鼠标位置：[上按钮](垂直滚动条) 或者 [左按钮](水平滚动条)
			m_uButton1State = kControlStatePushed;
			if (!m_bHorizontal) {
				//垂直滚动条
				if (m_pOwner != nullptr) {
					m_pOwner->LineUp();
				}
				else {
					SetScrollPos(m_nScrollPos - m_nLineSize);
				}
			}
			else {
				//水平滚动条
				if (m_pOwner != nullptr) {
					m_pOwner->LineLeft();
				}
				else {
					SetScrollPos(m_nScrollPos - m_nLineSize);
				}
			}
		}
		else if (m_rcButton2.ContainsPt(pt)) {
			//鼠标位置：[下按钮](垂直滚动条) 或者 [右按钮](水平滚动条)
			m_uButton2State = kControlStatePushed;
			if (!m_bHorizontal) {
				//垂直滚动条
				if (m_pOwner != nullptr) {
					m_pOwner->LineDown();
				}
				else {
					SetScrollPos(m_nScrollPos + m_nLineSize);
				}
			}
			else {
				//水平滚动条
				if (m_pOwner != nullptr) {
					m_pOwner->LineRight();
				}
				else {
					SetScrollPos(m_nScrollPos + m_nLineSize);
				}
			}
		}
		else if (m_rcThumb.ContainsPt(pt)) {
			//鼠标位置：在滚动条的滑动块按钮上
			m_uThumbState = kControlStatePushed;
			SetMouseFocused(true);
			m_ptLastMouse = pt;
			m_nLastScrollPos = m_nScrollPos;
		}
		else {
			//鼠标位置：滚动条非按钮区域
			if (!m_bHorizontal) {
				//垂直滚动条
				if (pt.y < m_rcThumb.top) {
					if (m_pOwner != nullptr) {
						m_pOwner->PageUp();
					}
					else {
						SetScrollPos(m_nScrollPos + GetRect().top - GetRect().bottom);
					}
				}
				else if (pt.y > m_rcThumb.bottom){
					if (m_pOwner != nullptr) {
						m_pOwner->PageDown();
					}
					else {
						SetScrollPos(m_nScrollPos - GetRect().top + GetRect().bottom);
					}
				}
			}
			else {
				//水平滚动条
				if (pt.x < m_rcThumb.left) {
					if (m_pOwner != nullptr) {
						m_pOwner->PageLeft();
					}
					else {
						SetScrollPos(m_nScrollPos + GetRect().left - GetRect().right);
					}
				}
				else if (pt.x > m_rcThumb.right){
					if (m_pOwner != nullptr) {
						m_pOwner->PageRight();
					}
					else {
						SetScrollPos(m_nScrollPos - GetRect().left + GetRect().right);
					}
				}
			}
		}

		ButtonDown(msg);
		return;
	}
	else if ((msg.Type == kEventMouseButtonUp) || (msg.Type == kEventWindowKillFocus) ){
		m_nScrollRepeatDelay = 0;
		m_nLastScrollOffset = 0;

		m_weakFlagOwner.Cancel();

		if (IsMouseFocused()) {
			if (GetRect().ContainsPt(pt)) {
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

		ButtonUp(msg);
		return;
	}
	else if (msg.Type == kEventMouseEnter) {
		MouseEnter(msg);
	}
	else if (msg.Type == kEventMouseLeave) {
		MouseLeave(msg);
	}
	else if (msg.Type == kEventMouseMove) {
		if (IsMouseFocused()) {
			if (!m_bHorizontal) {
				//垂直滚动条
				int vRange = GetRect().Height() - m_rcThumb.Height();
				if (m_bShowButton1) {
					vRange -= GetFixedWidth().GetInt32();
				}
				if (m_bShowButton2) {
					vRange -= GetFixedWidth().GetInt32();
				}

				if (vRange != 0) {
					m_nLastScrollOffset = (pt.y - m_ptLastMouse.y) * m_nRange / vRange;
				}
			}
			else {
				//水平滚动条
				int hRange = GetRect().Width() - m_rcThumb.Width();
				if (m_bShowButton1) {
					hRange -= GetFixedHeight().GetInt32();
				}
				if (m_bShowButton2) {
					hRange -= GetFixedHeight().GetInt32();
				}

				if (hRange != 0) {
					m_nLastScrollOffset = (pt.x - m_ptLastMouse.x) * m_nRange / hRange;
				}
			}
		}

		return;
	}
	else if (msg.Type == kEventSetCursor) {
		if (GetCursorType() == kCursorHand) {
			SetCursor(kCursorHand);
			return;
		}
		else if (GetCursorType() == kCursorArrow){
			SetCursor(kCursorArrow);
			return;
		}
		else {
			ASSERT(FALSE);
		}
	}

	if (m_pOwner != nullptr) {
		m_pOwner->SendEvent(msg);
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
		if(GetFixedHeight().GetInt32() == 0 ) {
			SetFixedWidth(UiFixedInt(0), false, false);
			SetFixedHeight(UiFixedInt(DEFAULT_SCROLLBAR_SIZE), true, false);
		}
	}
	else {
		if(GetFixedWidth().GetInt32() == 0) {
			SetFixedWidth(UiFixedInt(DEFAULT_SCROLLBAR_SIZE), false, false);
			SetFixedHeight(UiFixedInt(0), true, false);
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
		//鼠标位置：在滑动块上
		if( !m_bHorizontal ) {
			//垂直滚动条
			if (m_pOwner != nullptr) {
				m_pOwner->SetScrollPos(UiSize64(m_pOwner->GetScrollPos().cx,
					                          static_cast<int>(m_nLastScrollPos + m_nLastScrollOffset)));
			}
			else {
				SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
			}
		}
		else {
			//水平滚动条
			if (m_pOwner != nullptr) {
				m_pOwner->SetScrollPos(UiSize64(static_cast<int>(m_nLastScrollPos + m_nLastScrollOffset),
									          m_pOwner->GetScrollPos().cy));
			}
			else {
				SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
			}
		}
	}
	else if( m_uButton1State == kControlStatePushed ) {
		//鼠标位置：[上按钮](垂直滚动条) 或者 [左按钮](水平滚动条)
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
		//鼠标位置：[下按钮](垂直滚动条) 或者 [右按钮](水平滚动条)
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
		//鼠标位置：滚动条非按钮区域
		if (m_nScrollRepeatDelay <= 5) {
			return;
		}
		POINT ptClient = { 0 };
		::GetCursorPos(&ptClient);
		::ScreenToClient(GetWindow()->GetHWND(), &ptClient);
		UiPoint pt(ptClient.x, ptClient.y);
		pt.Offset(GetScrollOffsetInScrollBox());
		if( !m_bHorizontal ) {
			//垂直滚动条
			if( pt.y < m_rcThumb.top ) {
				if (m_pOwner != nullptr) {
					//做处理，避免数据量很大时，滚动条的速度非常慢，达不到鼠标所在位置
					int64_t nScrollOffset = CalcFastScrollOffset(m_rcThumb.top - pt.y);					
					if (nScrollOffset > GetRect().Height()) {
						UiSize64 sz = m_pOwner->GetScrollPos();
						sz.cy -= nScrollOffset;
						m_pOwner->SetScrollPos(sz);
					}
					else {
						m_pOwner->PageUp();
					}
				}
				else {
					SetScrollPos(m_nScrollPos + GetRect().top - GetRect().bottom);
				}
			}
			else if ( pt.y > m_rcThumb.bottom ){
				if (m_pOwner != nullptr) {
					int64_t nScrollOffset = CalcFastScrollOffset(pt.y - m_rcThumb.bottom);
					if (nScrollOffset > GetRect().Height()) {
						UiSize64 sz = m_pOwner->GetScrollPos();
						sz.cy += nScrollOffset;
						m_pOwner->SetScrollPos(sz);
					}
					else {
						m_pOwner->PageDown();
					}
				}
				else {
					SetScrollPos(m_nScrollPos - GetRect().top + GetRect().bottom);
				}
			}
		}
		else {
			//水平滚动条
			if( pt.x < m_rcThumb.left ) {
				if (m_pOwner != nullptr) {
					int64_t nScrollOffset = CalcFastScrollOffset(m_rcThumb.left - pt.x);
					if (nScrollOffset > GetRect().Width()) {
						UiSize64 sz = m_pOwner->GetScrollPos();
						sz.cx -= nScrollOffset;
						m_pOwner->SetScrollPos(sz);
					}
					else {
						m_pOwner->PageLeft();
					}
				}
				else {
					SetScrollPos(m_nScrollPos + GetRect().left - GetRect().right);
				}
			}
			else if ( pt.x > m_rcThumb.right ){
				if (m_pOwner != nullptr) {
					int64_t nScrollOffset = CalcFastScrollOffset(pt.x - m_rcThumb.right);
					if (nScrollOffset > GetRect().Width()) {
						UiSize64 sz = m_pOwner->GetScrollPos();
						sz.cx += nScrollOffset;
						m_pOwner->SetScrollPos(sz);
					}
					else {
						m_pOwner->PageRight();
					}
				}
				else {
					SetScrollPos(m_nScrollPos - GetRect().left + GetRect().right);
				}
			}
		}
	}
	return;
}

int64_t ScrollBar::CalcFastScrollOffset(int32_t posOffset) const
{
	ASSERT(posOffset > 0);
	if (posOffset <= 0) {
		return GetRect().Height();
	}
	//做处理，避免数据量很大时，滚动条的速度非常慢，达不到鼠标所在位置
	int64_t nScrollOffset = 0;
	int vRange = GetRect().Height() - m_rcThumb.Height();
	if (m_bShowButton1) {
		vRange -= GetFixedWidth().GetInt32();
	}
	if (m_bShowButton2) {
		vRange -= GetFixedWidth().GetInt32();
	}
	if (vRange != 0) {
		nScrollOffset = posOffset * m_nRange / vRange;
		//按滚动5次到达鼠标所在位置计算
		nScrollOffset /= 5;
	}
	return nScrollOffset;
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
			(m_rcThumb.top + m_rcThumb.bottom) / 2 - GetRect().top - GetFixedWidth().GetInt32() / 2, \
			m_rcThumb.right - GetRect().left, \
			(m_rcThumb.top + m_rcThumb.bottom) / 2 - GetRect().top + GetFixedWidth().GetInt32() - GetFixedWidth().GetInt32() / 2);
	}
	else {
		m_sImageModify = StringHelper::Printf(L"destscale='false' dest='%d,%d,%d,%d'", \
			(m_rcThumb.left + m_rcThumb.right) / 2 - GetRect().left - GetFixedHeight().GetInt32() / 2, \
			m_rcThumb.top - GetRect().top, \
			(m_rcThumb.left + m_rcThumb.right) / 2 - GetRect().left + GetFixedHeight().GetInt32() - GetFixedHeight().GetInt32() / 2, \
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
