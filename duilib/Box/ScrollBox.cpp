#include "ScrollBox.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{
ScrollBox::ScrollBox(Layout* pLayout) :
	Box(pLayout),
	m_pVScrollBar(),
	m_pHScrollBar(),
	m_bScrollProcess(false),
	m_bScrollBarFloat(true),
	m_bVScrollBarAtLeft(false),
	m_bHoldEnd(false),
	m_rcScrollBarPadding(),
	m_pScrollAnimation(nullptr),
	m_pRenderOffsetYAnimation(nullptr)
{
	m_nVScrollUnitPixels = GlobalManager::Instance().Dpi().GetScaleInt(m_nVScrollUnitPixelsDefault);
	m_nHScrollUnitPixels = GlobalManager::Instance().Dpi().GetScaleInt(m_nHScrollUnitPixelsDefault);
}

ScrollBox::~ScrollBox()
{
	if (m_pScrollAnimation != nullptr) {
		delete m_pScrollAnimation;
		m_pScrollAnimation = nullptr;
	}
	if (m_pRenderOffsetYAnimation != nullptr) {
		delete m_pRenderOffsetYAnimation;
		m_pRenderOffsetYAnimation = nullptr;
	}
}

std::wstring ScrollBox::GetType() const { return DUI_CTR_SCROLLBOX; }//ScrollBox

void ScrollBox::SetAttribute(const std::wstring& pstrName, const std::wstring& pstrValue)
{
	if (pstrName == L"vscrollbar") {
		EnableScrollBar(pstrValue == L"true", GetHScrollBar() != nullptr);
	}
	else if ((pstrName == L"vscrollbar_style") || (pstrName == L"vscrollbarstyle")) {
		EnableScrollBar(true, GetHScrollBar() != nullptr);
		if (GetVScrollBar() != nullptr) {
			GetVScrollBar()->ApplyAttributeList(pstrValue);
		}
	}
	else if (pstrName == L"hscrollbar") {
		EnableScrollBar(GetVScrollBar() != nullptr, pstrValue == L"true");
	}
	else if ((pstrName == L"hscrollbar_style") || (pstrName == L"hscrollbarstyle")) {
		EnableScrollBar(GetVScrollBar() != nullptr, true);
		if (GetHScrollBar() != nullptr) {
			GetHScrollBar()->ApplyAttributeList(pstrValue);
		}
	}
	else if ((pstrName == L"scrollbar_padding") || (pstrName == L"scrollbarpadding")) {
		UiPadding rcScrollbarPadding;
		AttributeUtil::ParsePaddingValue(pstrValue.c_str(), rcScrollbarPadding);
		SetScrollBarPadding(rcScrollbarPadding);
	}
	else if ((pstrName == L"vscroll_unit") || (pstrName == L"vscrollunit")) {
		SetVerScrollUnitPixels(_wtoi(pstrValue.c_str()));
	}
	else if ((pstrName == L"hscroll_unit") || (pstrName == L"hscrollunit")) {
		SetHorScrollUnitPixels(_wtoi(pstrValue.c_str()));
	}
	else if ((pstrName == L"scrollbar_float") || (pstrName == L"scrollbarfloat")) {
		SetScrollBarFloat(pstrValue == L"true");
	}
	else if ((pstrName == L"vscrollbar_left") || (pstrName == L"vscrollbarleft")) {
		SetVScrollBarAtLeft(pstrValue == L"true");
	}
	else if ((pstrName == L"hold_end") || (pstrName == L"holdend")) {
		SetHoldEnd(pstrValue == L"true");
	}
	else {
		Box::SetAttribute(pstrName, pstrValue);
	}
}

void ScrollBox::SetPos(UiRect rc)
{
	bool bEndDown = false;
	if (IsHoldEnd() && IsVScrollBarValid() && GetScrollRange().cy - GetScrollPos().cy == 0) {
		bEndDown = true;
	}
	SetPosInternally(rc);
	if (bEndDown && IsVScrollBarValid()) {
		EndDown(false, false);
	}
}

void ScrollBox::SetPosInternally(UiRect rc)
{
	Control::SetPos(rc);
	UiSize64 requiredSize = CalcRequiredSize(rc);
	LayoutType layoutType = LayoutType::FloatLayout;
	if (GetLayout() != nullptr) {
		layoutType = GetLayout()->GetLayoutType();
	}
	if ((layoutType != LayoutType::ListCtrlReportLayout) && 
		(requiredSize.cx > 0) && (requiredSize.cy > 0)) {
		//需要按照真实大小再计算一次，因为内部根据rc评估的时候，显示位置是不正确的
		//（比如控件是center或者bottom对齐的时候，会按照rc区域定位坐标，这时是错误的）。		
		int32_t cx = TruncateToInt32(requiredSize.cx);
		if (layoutType == LayoutType::VTileLayout) {
			//VTile模式是限制宽度，但不限制高度
			if (cx > rc.Width()) {
				cx = rc.Width();
			}
		}
		else {
			if (cx < rc.Width()) {
				cx = rc.Width();
			}
		}		
		int32_t cy = TruncateToInt32(requiredSize.cy);
		if (layoutType == LayoutType::HTileLayout) {
			//HTile模式是限制高度，但不限制宽度
			if (cy > rc.Height()) {
				cy = rc.Height();
			}
		}
		else {
			if (cy < rc.Height()) {
				cy = rc.Height();
			}
		}
		UiRect realRect(rc.left, rc.top, rc.left + cx, rc.top + cy);
		if ((realRect.Width() != rc.Width()) || (realRect.Height() != rc.Height())) {
			requiredSize = CalcRequiredSize(realRect);
		}		
	}
	//requiredSize需要剪去内边距，与ProcessVScrollBar/ProcessHScrollBar的逻辑保持一致
	UiPadding rcPadding = GetPadding();
	requiredSize.cy -= (rcPadding.top + rcPadding.bottom);
	requiredSize.cx -= (rcPadding.left + rcPadding.right);
	ProcessVScrollBar(rc, requiredSize.cy);
	ProcessHScrollBar(rc, requiredSize.cx);
}

UiSize64 ScrollBox::CalcRequiredSize(const UiRect& rc)
{
	UiSize64 requiredSize;
	if (m_items.empty()) {
		return requiredSize;
	}
	UiRect childSize = rc;
	if (!m_bScrollBarFloat && m_pVScrollBar && m_pVScrollBar->IsValid()) {
		if (m_bVScrollBarAtLeft) {
			ASSERT(m_pVScrollBar->GetFixedWidth().GetInt32() > 0);
			childSize.left += m_pVScrollBar->GetFixedWidth().GetInt32();
		}
		else {
			ASSERT(m_pVScrollBar->GetFixedWidth().GetInt32() > 0);
			childSize.right -= m_pVScrollBar->GetFixedWidth().GetInt32();
		}
	}
	if (!m_bScrollBarFloat && m_pHScrollBar && m_pHScrollBar->IsValid()) {
		ASSERT(m_pHScrollBar->GetFixedHeight().GetInt32() > 0);
		childSize.bottom -= m_pHScrollBar->GetFixedHeight().GetInt32();
	}
	childSize.Validate();
	requiredSize = GetLayout()->ArrangeChild(m_items, childSize);
	return requiredSize;
}

void ScrollBox::HandleEvent(const EventArgs& msg)
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
	if( (m_pVScrollBar != nullptr) && m_pVScrollBar->IsValid() && m_pVScrollBar->IsEnabled() ) {
		if(msg.Type == kEventKeyDown ) {
			switch(msg.chKey ) {
			case VK_DOWN:
				LineDown();
				return;
			case VK_UP:
				LineUp();
				return;
			case VK_NEXT:
				PageDown();
				return;
			case VK_PRIOR:
				PageUp();
				return;
			case VK_HOME:
				HomeUp();
				return;
			case VK_END:
				EndDown();
				return;
			}
		}
		else if(msg.Type == kEventMouseWheel ) {
			int deltaValue = GET_WHEEL_DELTA_WPARAM(msg.wParam);
			if (msg.lParam != 0) {
				//正常逻辑滚动
				if (deltaValue > 0) {
					LineUp(abs(deltaValue));
				}
				else {
					LineDown(abs(deltaValue));
				}
			}
			else {
				//严格按照传入参数滚动
				if (deltaValue > 0) {
					TouchUp(abs(deltaValue));
				}
				else {
					TouchDown(abs(deltaValue));
				}
			}
			return;
		}		
	}
	else if( (m_pHScrollBar != nullptr) && m_pHScrollBar->IsValid() && m_pHScrollBar->IsEnabled() ) {
		if(msg.Type == kEventKeyDown ) {
			switch(msg.chKey ) {
			case VK_DOWN:
				LineRight();
				return;
			case VK_UP:
				LineLeft();
				return;
			case VK_NEXT:
				PageRight();
				return;
			case VK_PRIOR:
				PageLeft();
				return;
			case VK_HOME:
				HomeLeft();
				return;
			case VK_END:
				EndRight();
				return;
			}
		}
		else if(msg.Type == kEventMouseWheel )	{
			int deltaValue = GET_WHEEL_DELTA_WPARAM(msg.wParam);
			if (deltaValue > 0 ) {
				LineLeft();
				return;
			}
			else {
				LineRight();
				return;
			}
		}
	}
		
	Box::HandleEvent(msg);
}

bool ScrollBox::MouseEnter(const EventArgs& msg)
{
	bool bRet = __super::MouseEnter(msg);
	if (IsHotState() && (m_pVScrollBar != nullptr) && m_pVScrollBar->IsValid() && m_pVScrollBar->IsEnabled()) {
		if (m_pVScrollBar->IsAutoHideScroll()) {
			m_pVScrollBar->SetFadeVisible(true);
		}
	}
	if (IsHotState() && (m_pHScrollBar != nullptr) && m_pHScrollBar->IsValid() && m_pHScrollBar->IsEnabled()) {
		if (m_pHScrollBar->IsAutoHideScroll()) {
			m_pHScrollBar->SetFadeVisible(true);
		}
	}
	return bRet;
}

bool ScrollBox::MouseLeave(const EventArgs& msg)
{
	bool bRet = __super::MouseLeave(msg);
	if (!IsHotState() && (m_pVScrollBar != nullptr) && m_pVScrollBar->IsValid() && m_pVScrollBar->IsEnabled()) {
		if ((m_pVScrollBar->GetThumbState() == kControlStateNormal) && 
			 m_pVScrollBar->IsAutoHideScroll()) {
			Control* pNewHover = nullptr;
			if (GetWindow() != nullptr) {
				pNewHover = GetWindow()->GetHoverControl();
			}
			if (pNewHover != m_pVScrollBar.get()) {
				m_pVScrollBar->SetFadeVisible(false);
			}
		}
	}
	if (!IsHotState() && (m_pHScrollBar != nullptr) && m_pHScrollBar->IsValid() && m_pHScrollBar->IsEnabled()) {
		if ((m_pHScrollBar->GetThumbState() == kControlStateNormal) && 
			 m_pHScrollBar->IsAutoHideScroll()) {
			Control* pNewHover = nullptr;
			if (GetWindow() != nullptr) {
				pNewHover = GetWindow()->GetHoverControl();
			}
			if (pNewHover != m_pHScrollBar.get()) {
				m_pHScrollBar->SetFadeVisible(false);
			}
		}
	}
	return bRet;
}

void ScrollBox::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	UiRect rcTemp;
	if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
		return;
	}

	std::vector<Control*> delayItems;
	for (Control* pControl : m_items) {
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
		if (pControl->GetPaintOrder() != 0) {
			//设置了绘制顺序， 放入延迟绘制列表
			delayItems.push_back(pControl);
			continue;
		}
		UiSize scrollPos = GetScrollOffset();
		UiRect rcNewPaint = GetPosWithoutPadding();
		AutoClip alphaClip(pRender, rcNewPaint, IsClip());
		rcNewPaint.Offset(scrollPos.cx, scrollPos.cy);
		rcNewPaint.Offset(GetRenderOffset().x, GetRenderOffset().y);

		UiPoint ptOffset(scrollPos.cx, scrollPos.cy);
		UiPoint ptOldOrg = pRender->OffsetWindowOrg(ptOffset);
		pControl->AlphaPaint(pRender, rcNewPaint);
		pRender->SetWindowOrg(ptOldOrg);
	}

	if (!delayItems.empty()) {
		std::sort(delayItems.begin(), delayItems.end(), [](const Control* a, const Control* b) {
			return a->GetPaintOrder() < b->GetPaintOrder();
			});
		//绘制延迟绘制的控件
		for (auto pControl : delayItems) {
			UiSize scrollPos = GetScrollOffset();
			UiRect rcNewPaint = GetPosWithoutPadding();
			AutoClip alphaClip(pRender, rcNewPaint, IsClip());
			rcNewPaint.Offset(scrollPos.cx, scrollPos.cy);
			rcNewPaint.Offset(GetRenderOffset().x, GetRenderOffset().y);

			UiPoint ptOffset(scrollPos.cx, scrollPos.cy);
			UiPoint ptOldOrg = pRender->OffsetWindowOrg(ptOffset);
			pControl->AlphaPaint(pRender, rcNewPaint);
			pRender->SetWindowOrg(ptOldOrg);
		}
	}

	if( (m_pHScrollBar != nullptr) && m_pHScrollBar->IsVisible()) {
		m_pHScrollBar->AlphaPaint(pRender, rcPaint);
	}
		
	if( (m_pVScrollBar != nullptr) && m_pVScrollBar->IsVisible()) {
		m_pVScrollBar->AlphaPaint(pRender, rcPaint);
	}
}

void ScrollBox::SetMouseEnabled(bool bEnabled)
{
	if (m_pVScrollBar != nullptr) {
		m_pVScrollBar->SetMouseEnabled(bEnabled);
	}
	if (m_pHScrollBar != nullptr) {
		m_pHScrollBar->SetMouseEnabled(bEnabled);
	}
	Box::SetMouseEnabled(bEnabled);
}

void ScrollBox::SetParent(Box* pParent)
{
	if (m_pVScrollBar != nullptr) {
		m_pVScrollBar->SetParent(this);
	}
	if (m_pHScrollBar != nullptr) {
		m_pHScrollBar->SetParent(this);
	}
	Box::SetParent(pParent);
}

void ScrollBox::SetWindow(Window* pManager)
{
	if (m_pVScrollBar != nullptr) {
		m_pVScrollBar->SetWindow(pManager);
	}
	if (m_pHScrollBar != nullptr) {
		m_pHScrollBar->SetWindow(pManager);
	}
	Box::SetWindow(pManager);
}

Control* ScrollBox::FindControl(FINDCONTROLPROC Proc, LPVOID pProcData,
								uint32_t uFlags, const UiPoint& ptMouse,
								const UiPoint& scrollPos)
{
	//ptMouse: 是适配过容器自身的坐标
	//scrollPos: 是当前容器的滚动条偏移	
	if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
		return nullptr;
	}
	if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
		return nullptr;
	}
	UiPoint boxPt(ptMouse);
	boxPt.Offset(scrollPos);
#ifdef _DEBUG
	if (((uFlags & UIFIND_HITTEST) != 0) && ((uFlags & UIFIND_DRAG_DROP) == 0) && (pProcData != nullptr)) {
		UiPoint ptOrg(*(UiPoint*)pProcData);
		ptOrg.Offset(this->GetScrollOffsetInScrollBox());
		ASSERT(ptOrg == boxPt);
	}
#endif // _DEBUG
	if ((uFlags & UIFIND_HITTEST) != 0) {
		if (!GetRect().ContainsPt(boxPt)) {
			return nullptr;
		}
		if (!IsMouseChildEnabled()) {
			Control* pResult = nullptr;
			if (m_pVScrollBar != nullptr) {
				pResult = m_pVScrollBar->FindControl(Proc, pProcData, uFlags, boxPt);
			}
			if ((pResult == nullptr) && (m_pHScrollBar != nullptr)) {
				pResult = m_pHScrollBar->FindControl(Proc, pProcData, uFlags, boxPt);
			}
			if (pResult == nullptr) {
				pResult = Control::FindControl(Proc, pProcData, uFlags, boxPt);
			}
			return pResult;
		}
	}

	Control* pResult = nullptr;
	if (m_pVScrollBar != nullptr) {
		pResult = m_pVScrollBar->FindControl(Proc, pProcData, uFlags, boxPt);
	}
	if ((pResult == nullptr) && (m_pHScrollBar != nullptr)) {
		pResult = m_pHScrollBar->FindControl(Proc, pProcData, uFlags, boxPt);
	}
	if (pResult != nullptr) {
		return pResult;
	}

	UiSize boxScrollOffset = GetScrollOffset();
	UiPoint boxScrollPos(boxScrollOffset.cx, boxScrollOffset.cy);
	return FindControlInItems(m_items, Proc, pProcData, uFlags, boxPt, boxScrollPos);
}

UiSize64 ScrollBox::GetScrollPos() const
{
	UiSize64 sz;
	if ((m_pVScrollBar != nullptr) && m_pVScrollBar->IsValid()) {
		sz.cy = m_pVScrollBar->GetScrollPos();
	}
	if ((m_pHScrollBar != nullptr) && m_pHScrollBar->IsValid()) {
		sz.cx = m_pHScrollBar->GetScrollPos();
	}
	return sz;
}

UiSize64 ScrollBox::GetScrollRange() const
{
	UiSize64 sz;
	if ((m_pVScrollBar != nullptr) && m_pVScrollBar->IsValid()) {
		sz.cy = m_pVScrollBar->GetScrollRange();
	}
	if ((m_pHScrollBar != nullptr) && m_pHScrollBar->IsValid()) {
		sz.cx = m_pHScrollBar->GetScrollRange();
	}
	return sz;
}

void ScrollBox::SetScrollPos(UiSize64 szPos)
{
	if (szPos.cy < 0) {
		szPos.cy = 0;
		if (m_pScrollAnimation != nullptr) {
			m_pScrollAnimation->Reset();
		}		
	}
	else if (szPos.cy > GetScrollRange().cy) {
		szPos.cy = GetScrollRange().cy;
		if (m_pScrollAnimation != nullptr) {
			m_pScrollAnimation->Reset();
		}
	}

	int64_t cx = 0;
	int64_t cy = 0;
	if( (m_pVScrollBar != nullptr) && m_pVScrollBar->IsValid() ) {
		int64_t iLastScrollPos = m_pVScrollBar->GetScrollPos();
		m_pVScrollBar->SetScrollPos(szPos.cy);
		cy = m_pVScrollBar->GetScrollPos() - iLastScrollPos;
	}

	if( (m_pHScrollBar != nullptr) && m_pHScrollBar->IsValid() ) {
		int64_t iLastScrollPos = m_pHScrollBar->GetScrollPos();
		m_pHScrollBar->SetScrollPos(szPos.cx);
		cx = m_pHScrollBar->GetScrollPos() - iLastScrollPos;
	}

	if (cx == 0 && cy == 0) {
		return;
	}
	Invalidate();
	SendEvent(kEventScrollChange, (cy == 0) ? 0 : 1, (cx == 0) ? 0 : 1);
}

void ScrollBox::SetScrollPosY(int64_t y)
{
	UiSize64 scrollPos = GetScrollPos();
	scrollPos.cy = y;
	SetScrollPos(scrollPos);
}

void ScrollBox::SetScrollPosX(int64_t x)
{
	UiSize64 scrollPos = GetScrollPos();
    scrollPos.cx = x;
    SetScrollPos(scrollPos);
}

void ScrollBox::LineUp(int deltaValue, bool withAnimation)
{
	UiSize64 scrollPos = GetScrollPos();
	if (scrollPos.cy <= 0) {
		return;
	}
	if (deltaValue <= 0) {
		deltaValue = DUI_NOSET_VALUE;
	}

	int cyLine = GetVerScrollUnitPixels();
	if (cyLine == 0) {
		cyLine = GlobalManager::Instance().Dpi().GetScaleInt(m_nVScrollUnitPixelsDefault);
	}
	if (deltaValue != DUI_NOSET_VALUE) {
		cyLine = deltaValue;
	}

	if (!withAnimation) {
		scrollPos.cy -= cyLine;
		if (scrollPos.cy < 0) {
			scrollPos.cy = 0;
		}
		SetScrollPos(scrollPos);
	}
	else {
		if (m_pScrollAnimation == nullptr) {
			m_pScrollAnimation = new AnimationPlayer;
		}
		AnimationPlayer* pScrollAnimation = m_pScrollAnimation;
		pScrollAnimation->SetStartValue(scrollPos.cy);
		int64_t nEndValue = 0;
		if (pScrollAnimation->IsPlaying()) {
			if (pScrollAnimation->GetEndValue() > pScrollAnimation->GetStartValue()) {
				nEndValue = scrollPos.cy - cyLine;
			}
			else {
				nEndValue = pScrollAnimation->GetEndValue() - cyLine;
			}
		}
		else {
			nEndValue = scrollPos.cy - cyLine;
		}
		if (nEndValue < 0) {
			nEndValue = 0;
		}
		pScrollAnimation->SetEndValue(nEndValue);
		pScrollAnimation->SetSpeedUpRatio(0);
		pScrollAnimation->SetSpeedDownfactorA(-0.012);
		pScrollAnimation->SetSpeedDownRatio(0.5);
		pScrollAnimation->SetTotalMillSeconds(DUI_NOSET_VALUE);
		pScrollAnimation->SetCallback(nbase::Bind(&ScrollBox::SetScrollPosY, this, std::placeholders::_1));
		pScrollAnimation->Start();
	}
}

void ScrollBox::LineDown(int deltaValue, bool withAnimation)
{
	UiSize64 scrollPos = GetScrollPos();
	if (scrollPos.cy >= GetScrollRange().cy) {
		return;
	}
	if (deltaValue <= 0) {
		deltaValue = DUI_NOSET_VALUE;
	}

	int cyLine = GetVerScrollUnitPixels();
	if (cyLine == 0) {
		cyLine = GlobalManager::Instance().Dpi().GetScaleInt(m_nVScrollUnitPixelsDefault);
	}
	if (deltaValue != DUI_NOSET_VALUE) {
		cyLine = deltaValue;
	}

	if (!withAnimation) {
		scrollPos.cy += cyLine;
		if (scrollPos.cy < 0) {
			scrollPos.cy = 0;
		}
		SetScrollPos(scrollPos);
	}
	else {
		if (m_pScrollAnimation == nullptr) {
			m_pScrollAnimation = new AnimationPlayer;
		}
		AnimationPlayer* pScrollAnimation = m_pScrollAnimation;
		pScrollAnimation->SetStartValue(scrollPos.cy);
		int64_t nEndValue = 0;
		if (pScrollAnimation->IsPlaying()) {
			if (pScrollAnimation->GetEndValue() < pScrollAnimation->GetStartValue()) {
				nEndValue = scrollPos.cy + cyLine;
			}
			else {
				nEndValue = pScrollAnimation->GetEndValue() + cyLine;
			}
		}
		else {
			nEndValue = scrollPos.cy + cyLine;
		}
		if (nEndValue > GetScrollRange().cy) {
			nEndValue = GetScrollRange().cy;
		}
		pScrollAnimation->SetEndValue(nEndValue);
		pScrollAnimation->SetSpeedUpRatio(0);
		pScrollAnimation->SetSpeedDownfactorA(-0.012);
		pScrollAnimation->SetSpeedDownRatio(0.5);
		pScrollAnimation->SetTotalMillSeconds(DUI_NOSET_VALUE);
		pScrollAnimation->SetCallback(nbase::Bind(&ScrollBox::SetScrollPosY, this, std::placeholders::_1));
		pScrollAnimation->Start();
	}
}
void ScrollBox::LineLeft(int deltaValue)
{
	UiSize64 scrollPos = GetScrollPos();
	if (scrollPos.cx <= 0) {
		return;
	}
	if (deltaValue <= 0) {
		deltaValue = DUI_NOSET_VALUE;
	}

    int cxLine = GetHorScrollUnitPixels();
    if (cxLine == 0) {
        cxLine = GlobalManager::Instance().Dpi().GetScaleInt(m_nHScrollUnitPixelsDefault);
    }
    if (deltaValue != DUI_NOSET_VALUE) {
        cxLine = deltaValue;
    }
    scrollPos.cx -= cxLine;
    if (scrollPos.cx < 0) {
        scrollPos.cx = 0;
    }
    SetScrollPos(scrollPos);
    /*m_scrollAnimation.SetStartValue(scrollPos.cx);
    if (m_scrollAnimation.IsPlaying()) {
        if (m_scrollAnimation.GetEndValue() > m_scrollAnimation.GetStartValue()) {
            m_scrollAnimation.SetEndValue(scrollPos.cx - cxLine);
        }
        else {
            m_scrollAnimation.SetEndValue(m_scrollAnimation.GetEndValue() - cxLine);
        }
    }
    else {
        m_scrollAnimation.SetEndValue(scrollPos.cx - cxLine);
    }
    m_scrollAnimation.SetSpeedUpRatio(0);
    m_scrollAnimation.SetSpeedDownfactorA(-0.012);
    m_scrollAnimation.SetSpeedDownRatio(0.5);
    m_scrollAnimation.SetTotalMillSeconds(DUI_NOSET_VALUE);
    m_scrollAnimation.SetCallback(nbase::Bind(&ScrollBox::SetScrollPosX, this, std::placeholders::_1));
    m_scrollAnimation.Start();*/

}

void ScrollBox::LineRight(int deltaValue)
{
	UiSize64 scrollPos = GetScrollPos();
	if (scrollPos.cx >= GetScrollRange().cx) {
		return;
	}
	if (deltaValue <= 0) {
		deltaValue = DUI_NOSET_VALUE;
	}

    int cxLine = GetHorScrollUnitPixels();
    if (cxLine == 0) {
        cxLine = GlobalManager::Instance().Dpi().GetScaleInt(m_nHScrollUnitPixelsDefault);
    }
    if (deltaValue != DUI_NOSET_VALUE) {
        cxLine = deltaValue;
    }

    scrollPos.cx += cxLine;
    if (scrollPos.cx > GetScrollRange().cx) {
        scrollPos.cx = GetScrollRange().cx;
    }
    SetScrollPos(scrollPos);
    //m_scrollAnimation.SetStartValue(scrollPos.cx);
    //if (m_scrollAnimation.IsPlaying()) {
    //    if (m_scrollAnimation.GetEndValue() < m_scrollAnimation.GetStartValue()) {
    //        m_scrollAnimation.SetEndValue(scrollPos.cx + cxLine);
    //    }
    //    else {
    //        m_scrollAnimation.SetEndValue(m_scrollAnimation.GetEndValue() + cxLine);
    //    }
    //}
    //else {
    //    m_scrollAnimation.SetEndValue(scrollPos.cx + cxLine);
    //}
    //m_scrollAnimation.SetSpeedUpRatio(0);
    //m_scrollAnimation.SetSpeedDownfactorA(-0.012);
    //m_scrollAnimation.SetSpeedDownRatio(0.5);
    //m_scrollAnimation.SetTotalMillSeconds(DUI_NOSET_VALUE);
    //m_scrollAnimation.SetCallback(nbase::Bind(&ScrollBox::SetScrollPosX, this, std::placeholders::_1));
    //m_scrollAnimation.Start();
}
void ScrollBox::PageUp()
{
	UiPadding rcPadding = GetPadding();
	UiSize64 sz = GetScrollPos();
	int iOffset = GetRect().bottom - GetRect().top - rcPadding.top - rcPadding.bottom;
	if (m_pHScrollBar && m_pHScrollBar->IsValid()) {
		ASSERT(m_pHScrollBar->GetFixedHeight().GetInt32() > 0);
		iOffset -= m_pHScrollBar->GetFixedHeight().GetInt32();
	}
	sz.cy -= iOffset;
	SetScrollPos(sz);
}

void ScrollBox::PageDown()
{
	UiPadding rcPadding = GetPadding();
	UiSize64 sz = GetScrollPos();
	int iOffset = GetRect().bottom - GetRect().top - rcPadding.top - rcPadding.bottom;
	if ((m_pHScrollBar != nullptr) && m_pHScrollBar->IsValid()) {
		ASSERT(m_pHScrollBar->GetFixedHeight().GetInt32() > 0);
		iOffset -= m_pHScrollBar->GetFixedHeight().GetInt32();
	}
	sz.cy += iOffset;
	SetScrollPos(sz);
}

void ScrollBox::HomeUp()
{
	UiSize64 sz = GetScrollPos();
	sz.cy = 0;
	SetScrollPos(sz);
}

void ScrollBox::EndDown(bool arrange, bool withAnimation)
{
	if (arrange) {
		SetPosInternally(GetPos());
	}
	int64_t endValue = 0;
	if (m_pRenderOffsetYAnimation != nullptr) {
		endValue = m_pRenderOffsetYAnimation->GetEndValue();
	}
	int64_t renderOffsetY = GetScrollRange().cy - GetScrollPos().cy + (endValue - GetRenderOffset().y);
	if (withAnimation == true && IsVScrollBarValid() && renderOffsetY > 0) {
		PlayRenderOffsetYAnimation(-renderOffsetY);
	}

	UiSize64 sz = GetScrollPos();
	sz.cy = GetScrollRange().cy;
	SetScrollPos(sz);
}

void ScrollBox::PageLeft()
{
	UiPadding rcPadding = GetPadding();
	UiSize64 sz = GetScrollPos();
	int iOffset = GetRect().right - GetRect().left - rcPadding.left - rcPadding.right;
	//if( m_pVScrollBar && m_pVScrollBar->IsValid() ) iOffset -= m_pVScrollBar->GetFixedWidth();
	sz.cx -= iOffset;
	SetScrollPos(sz);
}

void ScrollBox::PageRight()
{
	UiPadding rcPadding = GetPadding();
	UiSize64 sz = GetScrollPos();
	int iOffset = GetRect().right - GetRect().left - rcPadding.left - rcPadding.right;
	//if( m_pVScrollBar && m_pVScrollBar->IsValid() ) iOffset -= m_pVScrollBar->GetFixedWidth();
	sz.cx += iOffset;
	SetScrollPos(sz);
}

void ScrollBox::HomeLeft()
{
	UiSize64 sz = GetScrollPos();
	sz.cx = 0;
	SetScrollPos(sz);
}

void ScrollBox::EndRight()
{
	UiSize64 sz = GetScrollPos();
	sz.cx = GetScrollRange().cx;
	SetScrollPos(sz);
}

void ScrollBox::TouchUp(int deltaValue)
{
	UiSize64 scrollPos = GetScrollPos();
	if (scrollPos.cy <= 0) {
		return;
	}

	scrollPos.cy = scrollPos.cy - deltaValue;
	SetScrollPos(scrollPos);
}

void ScrollBox::TouchDown(int deltaValue)
{
	UiSize64 scrollPos = GetScrollPos();
	if (scrollPos.cy >= GetScrollRange().cy) {
		return;
	}

	scrollPos.cy = scrollPos.cy + deltaValue;
	SetScrollPos(scrollPos);
}

void ScrollBox::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
	if( bEnableVertical && (m_pVScrollBar == nullptr) ) {
		m_pVScrollBar.reset(new ScrollBar);
		m_pVScrollBar->SetVisible(false);
		m_pVScrollBar->SetScrollRange(0);
		m_pVScrollBar->SetOwner(this);
		m_pVScrollBar->SetWindow(GetWindow());
		m_pVScrollBar->SetClass(L"vscrollbar");
	}
	else if( !bEnableVertical && (m_pVScrollBar != nullptr) ) {
		m_pVScrollBar.reset();
	}

	if( bEnableHorizontal && (m_pHScrollBar == nullptr)) {
		m_pHScrollBar.reset(new ScrollBar);
		m_pHScrollBar->SetVisible(false);
		m_pHScrollBar->SetScrollRange(0);
		m_pHScrollBar->SetHorizontal(true);
		m_pHScrollBar->SetOwner(this);
		m_pHScrollBar->SetWindow(GetWindow());
		m_pHScrollBar->SetClass(L"hscrollbar");
	}
	else if( !bEnableHorizontal && (m_pHScrollBar != nullptr)) {
		m_pHScrollBar.reset();
	}

	Arrange();
}

ScrollBar* ScrollBox::GetVScrollBar() const
{
	return m_pVScrollBar.get();
}

ScrollBar* ScrollBox::GetHScrollBar() const
{
	return m_pHScrollBar.get();
}

void ScrollBox::ProcessVScrollBar(UiRect rc, int64_t cyRequired)
{
	UiRect rcScrollBarPos = rc;
	rcScrollBarPos.Deflate(m_rcScrollBarPadding);

	if (m_pVScrollBar == nullptr) {
		return;
	}
	rc.Deflate(GetPadding());

	int32_t nHeight = rc.Height();
	const int64_t cyScroll = std::max(cyRequired - nHeight, (int64_t)0);
	if (cyRequired > nHeight && !m_pVScrollBar->IsValid()) {
		m_pVScrollBar->SetScrollRange(cyScroll);
		m_pVScrollBar->SetScrollPos(0);
		m_bScrollProcess = true;
		SetPos(GetRect());
		m_bScrollProcess = false;
		return;
	}
	// No scrollbar required
	if (!m_pVScrollBar->IsValid()) {
		return;
	}

	// Scroll not needed anymore?	
	if( cyScroll <= 0 && !m_bScrollProcess) {
		m_pVScrollBar->SetScrollPos(0);
		m_pVScrollBar->SetScrollRange(0);
		SetPos(GetRect());
	}
	else {
		if (m_bVScrollBarAtLeft) {
			ASSERT(m_pVScrollBar->GetFixedWidth().GetInt32() > 0);
			UiRect rcVerScrollBarPos(rcScrollBarPos.left, 
									 rcScrollBarPos.top, 
									 rcScrollBarPos.left + m_pVScrollBar->GetFixedWidth().GetInt32(),
									 rcScrollBarPos.bottom);
			m_pVScrollBar->SetPos(rcVerScrollBarPos);
		}
		else {
			ASSERT(m_pVScrollBar->GetFixedWidth().GetInt32() > 0);
			UiRect rcVerScrollBarPos(rcScrollBarPos.right - m_pVScrollBar->GetFixedWidth().GetInt32(),
				                     rcScrollBarPos.top, 
				                     rcScrollBarPos.right, 
				                     rcScrollBarPos.bottom);
			m_pVScrollBar->SetPos(rcVerScrollBarPos);
		}

		if( m_pVScrollBar->GetScrollRange() != cyScroll ) {
			int64_t iScrollPos = m_pVScrollBar->GetScrollPos();
			m_pVScrollBar->SetScrollRange(cyScroll);
			if( !m_pVScrollBar->IsValid() ) {
				m_pVScrollBar->SetScrollPos(0);
			}

			if( iScrollPos > m_pVScrollBar->GetScrollPos() ) {
				SetPos(GetRect());
			}
		}
	}
}

void ScrollBox::ProcessHScrollBar(UiRect rc, int64_t cxRequired)
{
	UiRect rcScrollBarPos = rc;
	rcScrollBarPos.Deflate(m_rcScrollBarPadding);
	if (m_pHScrollBar == nullptr) {
		return;
	}
	rc.Deflate(GetPadding());

	int32_t nWidth = rc.Width();
	const int64_t cxScroll = std::max(cxRequired - nWidth, (int64_t)0);
	if (cxRequired > nWidth && !m_pHScrollBar->IsValid()) {
		m_pHScrollBar->SetScrollRange(cxScroll);
		m_pHScrollBar->SetScrollPos(0);
		m_bScrollProcess = true;
		SetPos(GetRect());
		m_bScrollProcess = false;
		return;
	}
	// No scrollbar required
	if (!m_pHScrollBar->IsValid()) {
		return;
	}

	// Scroll not needed anymore?	
	if (cxScroll <= 0 && !m_bScrollProcess) {
		m_pHScrollBar->SetScrollPos(0);
		m_pHScrollBar->SetScrollRange(0);
		SetPos(GetRect());
	}
	else {
		ASSERT(m_pHScrollBar->GetFixedHeight().GetInt32() > 0);
		UiRect rcVerScrollBarPos(rcScrollBarPos.left, 
								 rcScrollBarPos.bottom - m_pHScrollBar->GetFixedHeight().GetInt32(),
			                     rcScrollBarPos.right, 
			                     rcScrollBarPos.bottom);
		m_pHScrollBar->SetPos(rcVerScrollBarPos);

		if (m_pHScrollBar->GetScrollRange() != cxScroll) {
			int64_t iScrollPos = m_pHScrollBar->GetScrollPos();
			m_pHScrollBar->SetScrollRange(cxScroll);
			if (!m_pHScrollBar->IsValid()) {
				m_pHScrollBar->SetScrollPos(0);
			}

			if (iScrollPos > m_pHScrollBar->GetScrollPos()) {
				SetPos(GetRect());
			}
		}
	}
}

bool ScrollBox::IsVScrollBarValid() const
{
	if (m_pVScrollBar != nullptr) {
		return m_pVScrollBar->IsValid();
	}
	return false;
}

bool ScrollBox::IsHScrollBarValid() const
{
	if (m_pHScrollBar != nullptr) {
		return m_pHScrollBar->IsValid();
	}
	return false;
}

void ScrollBox::PlayRenderOffsetYAnimation(int64_t nRenderY)
{
	if (m_pRenderOffsetYAnimation == nullptr) {
		m_pRenderOffsetYAnimation = new AnimationPlayer;
	}
	AnimationPlayer* pRenderOffsetYAnimation = m_pRenderOffsetYAnimation;
	pRenderOffsetYAnimation->SetStartValue(nRenderY);
	pRenderOffsetYAnimation->SetEndValue(0);
	pRenderOffsetYAnimation->SetSpeedUpRatio(0.3);
	pRenderOffsetYAnimation->SetSpeedUpfactorA(0.003);
	pRenderOffsetYAnimation->SetSpeedDownRatio(0.7);
	pRenderOffsetYAnimation->SetTotalMillSeconds(DUI_NOSET_VALUE);
	pRenderOffsetYAnimation->SetMaxTotalMillSeconds(650);
	auto playCallback = nbase::Bind(&ScrollBox::SetRenderOffsetY, this, std::placeholders::_1);
	pRenderOffsetYAnimation->SetCallback(playCallback);
	pRenderOffsetYAnimation->Start();
}

bool ScrollBox::IsAtEnd() const
{
	return GetScrollRange().cy <= GetScrollPos().cy;
}

bool ScrollBox::IsHoldEnd() const
{
	return m_bHoldEnd;
}

void ScrollBox::SetHoldEnd(bool bHoldEnd)
{
	m_bHoldEnd = bHoldEnd;
}

int ScrollBox::GetVerScrollUnitPixels() const
{
	return m_nVScrollUnitPixels;
}

void ScrollBox::SetVerScrollUnitPixels(int nUnitPixels)
{
	GlobalManager::Instance().Dpi().ScaleInt(nUnitPixels);
	m_nVScrollUnitPixels = nUnitPixels;
}

int ScrollBox::GetHorScrollUnitPixels() const
{
    return m_nHScrollUnitPixels;
}

void ScrollBox::SetHorScrollUnitPixels(int nUnitPixels)
{
    GlobalManager::Instance().Dpi().ScaleInt(nUnitPixels);
    m_nHScrollUnitPixels = nUnitPixels;
}

bool ScrollBox::GetScrollBarFloat() const
{
	return m_bScrollBarFloat;
}

void ScrollBox::SetScrollBarFloat(bool bScrollBarFloat)
{
	m_bScrollBarFloat = bScrollBarFloat;
}

bool ScrollBox::IsVScrollBarAtLeft() const
{
	return m_bVScrollBarAtLeft;
}

void ScrollBox::SetVScrollBarAtLeft(bool bAtLeft)
{
	m_bVScrollBarAtLeft = bAtLeft;
}

const UiPadding& ScrollBox::GetScrollBarPadding() const
{
	return m_rcScrollBarPadding;
}

void ScrollBox::SetScrollBarPadding(UiPadding rcScrollBarPadding)
{
	GlobalManager::Instance().Dpi().ScalePadding(rcScrollBarPadding);
	m_rcScrollBarPadding = rcScrollBarPadding;
}

void ScrollBox::ClearImageCache()
{
	__super::ClearImageCache();

	if (m_pHScrollBar != nullptr) {
		m_pHScrollBar->ClearImageCache();
	}
	if (m_pVScrollBar != nullptr) {
		m_pVScrollBar->ClearImageCache();
	}
}

void ScrollBox::StopScrollAnimation()
{
	if (m_pScrollAnimation != nullptr) {
		m_pScrollAnimation->Reset();
	}	
}

UiSize ScrollBox::GetScrollOffset() const
{
	//这种虚拟滚动条位置的引入，是为了解决UiRect用32位整型值不能支持超大虚表（千万数据量级别以上）的问题
	UiSize64 scrollPos = GetScrollPos();
	UiSize64 scrollVirtualOffset = GetScrollVirtualOffset();
	int64_t scrollPosX = scrollPos.cx - scrollVirtualOffset.cx;
	int64_t scrollPosY = scrollPos.cy - scrollVirtualOffset.cy;

	UiSize realcrollPos;
	realcrollPos.cx = TruncateToInt32(scrollPosX);
	realcrollPos.cy = TruncateToInt32(scrollPosY);
	return realcrollPos;
}

UiSize64 ScrollBox::GetScrollVirtualOffset() const
{
	return m_scrollVirtualOffset;
}

void ScrollBox::SetScrollVirtualOffset(UiSize64 szOffset)
{
	ASSERT(szOffset.cx >= 0);
	ASSERT(szOffset.cy >= 0);
	if ((szOffset.cx >= 0) && (szOffset.cy)) {
		m_scrollVirtualOffset = szOffset;
	}
}

void ScrollBox::SetScrollVirtualOffsetY(int64_t yOffset)
{
	ASSERT(yOffset >= 0);
	if (yOffset >= 0) {
		m_scrollVirtualOffset.cy = yOffset;
	}
}

void ScrollBox::SetScrollVirtualOffsetX(int64_t xOffset)
{
	ASSERT(xOffset >= 0);
	if (xOffset >= 0) {
		m_scrollVirtualOffset.cx = xOffset;
	}
}

} // namespace ui
