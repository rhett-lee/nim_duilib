#include "ScrollBox.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{
ScrollBox::ScrollBox(Layout* pLayout) :
	Box(pLayout),
	m_pVerticalScrollBar(),
	m_pHorizontalScrollBar(),
	m_bScrollProcess(false),
	m_bScrollBarFloat(true),
	m_bVScrollBarLeftPos(false),
	m_bHoldEnd(false),
	m_rcScrollBarPadding()
{
	m_nVerScrollUnitPixels = GlobalManager::Instance().Dpi().GetScaleInt(m_nVerScrollUnitPixelsDefault);
	m_nHerScrollUnitPixels = GlobalManager::Instance().Dpi().GetScaleInt(m_nHerScrollUnitPixelsDefault);
	m_scrollAnimation = std::make_unique<AnimationPlayer>();
	m_renderOffsetYAnimation = std::make_unique<AnimationPlayer>();
}

std::wstring ScrollBox::GetType() const { return DUI_CTR_SCROLLBOX; }//ScrollBox

void ScrollBox::SetAttribute(const std::wstring& pstrName, const std::wstring& pstrValue)
{
	if(pstrName == L"vscrollbar") {
		EnableScrollBar(pstrValue == L"true", GetHorizontalScrollBar() != nullptr);
	}
	else if( pstrName == L"vscrollbarstyle") {
		EnableScrollBar(true, GetHorizontalScrollBar() != nullptr);
		if (GetVerticalScrollBar() != nullptr) {
			GetVerticalScrollBar()->ApplyAttributeList(pstrValue);
		}
	}
	else if(pstrName == L"hscrollbar") {
		EnableScrollBar(GetVerticalScrollBar() != nullptr, pstrValue == L"true");
	}
	else if( pstrName == L"hscrollbarstyle") {
		EnableScrollBar(GetVerticalScrollBar() != nullptr, true);
		if (GetHorizontalScrollBar() != nullptr) {
			GetHorizontalScrollBar()->ApplyAttributeList(pstrValue);
		}
	}
	else if(pstrName == L"scrollbarpadding") {
		UiRect rcScrollbarPadding;
		LPTSTR pstr = NULL;
		rcScrollbarPadding.left = wcstol(pstrValue.c_str(), &pstr, 10);  ASSERT(pstr);    
		rcScrollbarPadding.top = wcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		rcScrollbarPadding.right = wcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		rcScrollbarPadding.bottom = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetScrollBarPadding(rcScrollbarPadding);
	}
	else if (pstrName == L"vscrollunit") SetVerScrollUnitPixels(_wtoi(pstrValue.c_str()));
	else if (pstrName == L"hscrollunit") SetHorScrollUnitPixels(_wtoi(pstrValue.c_str()));
	else if (pstrName == L"scrollbarfloat") SetScrollBarFloat(pstrValue == L"true");
	else if (pstrName == L"vscrollbarleft") SetVScrollBarLeftPos(pstrValue == L"true");
	else if (pstrName == L"holdend") SetHoldEnd(pstrValue == L"true");
	else Box::SetAttribute(pstrName, pstrValue);
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
	Layout* pLayout = GetLayout();
	ASSERT(pLayout != nullptr);
	Control::SetPos(rc);
	UiRect rcRaw = rc;
	rc.left += pLayout->GetPadding().left;
	rc.top += pLayout->GetPadding().top;
	rc.right -= pLayout->GetPadding().right;
	rc.bottom -= pLayout->GetPadding().bottom;

	UiSize requiredSize = CalcRequiredSize(rc);
	ProcessVScrollBar(rcRaw, requiredSize.cy);
	ProcessHScrollBar(rcRaw, requiredSize.cx);
}

UiSize ScrollBox::CalcRequiredSize(const UiRect& rc)
{
	UiSize requiredSize;
	if (!m_items.empty()) {
		UiRect childSize = rc;
		if (!m_bScrollBarFloat && m_pVerticalScrollBar && m_pVerticalScrollBar->IsValid()) {
			if (m_bVScrollBarLeftPos) {
				childSize.left += m_pVerticalScrollBar->GetFixedWidth();
			}
			else {
				childSize.right -= m_pVerticalScrollBar->GetFixedWidth();
			}
		}
		if (!m_bScrollBarFloat && m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsValid()) {
			childSize.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
		}
		requiredSize = GetLayout()->ArrangeChild(m_items, childSize);
	}
	return requiredSize;
}

void ScrollBox::HandleEvent(const EventArgs& event)
{
	if( (!IsMouseEnabled() && (event.Type > kEventMouseBegin) && (event.Type < kEventMouseEnd)) || 
		(event.Type == kEventLast)) {
		if (GetParent() != nullptr) {
			GetParent()->SendEvent(event);
		}
		else {
			Box::HandleEvent(event);
		}
		return;
	}
		
	if( (m_pVerticalScrollBar != nullptr) && m_pVerticalScrollBar->IsValid() && m_pVerticalScrollBar->IsEnabled() ) {
		if( event.Type == kEventKeyDown ) {
			switch( event.chKey ) {
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
		else if( event.Type == kEventMouseWheel ) {
			int deltaValue = static_cast<int>(event.wParam);
			if (event.lParam != 0) {
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
	else if( (m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsValid() && m_pHorizontalScrollBar->IsEnabled() ) {
		if( event.Type == kEventKeyDown ) {
			switch( event.chKey ) {
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
		else if( event.Type == kEventMouseWheel )	{
			int deltaValue = static_cast<int>(event.wParam);
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
		
	Box::HandleEvent(event);
}

bool ScrollBox::MouseEnter(const EventArgs& msg)
{
	bool bRet = __super::MouseEnter(msg);
	if (bRet && (m_pVerticalScrollBar != nullptr) && m_pVerticalScrollBar->IsValid() && m_pVerticalScrollBar->IsEnabled()) {
		if (m_pVerticalScrollBar->IsAutoHideScroll()) {
			m_pVerticalScrollBar->SetFadeVisible(true);
		}
	}
	if (bRet && (m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsValid() && m_pHorizontalScrollBar->IsEnabled()) {
		if (m_pHorizontalScrollBar->IsAutoHideScroll()) {
			m_pHorizontalScrollBar->SetFadeVisible(true);
		}
	}
	return bRet;
}

bool ScrollBox::MouseLeave(const EventArgs& msg)
{
	bool bRet = __super::MouseLeave(msg);
	if (bRet && (m_pVerticalScrollBar != nullptr) && m_pVerticalScrollBar->IsValid() && m_pVerticalScrollBar->IsEnabled()) {
		if ((m_pVerticalScrollBar->GetThumbState() == kControlStateNormal) && 
			 m_pVerticalScrollBar->IsAutoHideScroll()) {
			m_pVerticalScrollBar->SetFadeVisible(false);
		}
	}
	if (bRet && (m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsValid() && m_pHorizontalScrollBar->IsEnabled()) {
		if ((m_pHorizontalScrollBar->GetThumbState() == kControlStateNormal) && 
			 m_pHorizontalScrollBar->IsAutoHideScroll()) {
			m_pHorizontalScrollBar->SetFadeVisible(false);
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
	if (!::IntersectRect(&rcTemp, &rcPaint, &GetRect())) {
		return;
	}

	for (Control* pControl : m_items) {
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
		if (pControl->IsFloat()) {
			pControl->AlphaPaint(pRender, rcPaint);	
		}
		else {
			UiSize scrollPos = GetScrollPos();
			UiRect rcNewPaint = GetPaddingPos();
			AutoClip alphaClip(pRender, rcNewPaint, IsClip());
			rcNewPaint.Offset(scrollPos.cx, scrollPos.cy);
			rcNewPaint.Offset(GetRenderOffset().x, GetRenderOffset().y);

			UiPoint ptOffset(scrollPos.cx, scrollPos.cy);
			UiPoint ptOldOrg = pRender->OffsetWindowOrg(ptOffset);
			pControl->AlphaPaint(pRender, rcNewPaint);
			pRender->SetWindowOrg(ptOldOrg);
		}
	}

	if( (m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsVisible()) {
		m_pHorizontalScrollBar->AlphaPaint(pRender, rcPaint);
	}
		
	if( (m_pVerticalScrollBar != nullptr) && m_pVerticalScrollBar->IsVisible()) {
		m_pVerticalScrollBar->AlphaPaint(pRender, rcPaint);
	}
	
	static bool bFirstPaint = true;//TODO: static变量待改进
	if (bFirstPaint) {
		bFirstPaint = false;
		LoadImageCache(true);
	}
}

void ScrollBox::SetMouseEnabled(bool bEnabled)
{
	if (m_pVerticalScrollBar != nullptr) {
		m_pVerticalScrollBar->SetMouseEnabled(bEnabled);
	}
	if (m_pHorizontalScrollBar != nullptr) {
		m_pHorizontalScrollBar->SetMouseEnabled(bEnabled);
	}
	Box::SetMouseEnabled(bEnabled);
}

void ScrollBox::SetWindow(Window* pManager, Box* pParent, bool bInit)
{
	if (m_pVerticalScrollBar != nullptr) {
		m_pVerticalScrollBar->SetWindow(pManager, this, bInit);
	}
	if (m_pHorizontalScrollBar != nullptr) {
		m_pHorizontalScrollBar->SetWindow(pManager, this, bInit);
	}
	Box::SetWindow(pManager, pParent, bInit);
}

Control* ScrollBox::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, UiPoint /*scrollPos*/)
{
	// Check if this guy is valid
	if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
		return nullptr;
	}
	if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
		return nullptr;
	}
	if ((uFlags & UIFIND_HITTEST) != 0) {
		ASSERT(pData != nullptr);
		if (pData == nullptr) {
			return nullptr;
		}
		if (!::PtInRect(&GetRect(), *(static_cast<LPPOINT>(pData)))) {
			return nullptr;
		}
		if (!IsMouseChildEnabled()) {
			Control* pResult = nullptr;
			if (m_pVerticalScrollBar != nullptr) {
				pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
			}
			if (pResult == nullptr && m_pHorizontalScrollBar != nullptr) {
				pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
			}
			if (pResult == nullptr) {
				pResult = Control::FindControl(Proc, pData, uFlags);
			}
			return pResult;
		}
	}

	Control* pResult = nullptr;
	if (m_pVerticalScrollBar != nullptr) {
		pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
	}
	if (pResult == nullptr && m_pHorizontalScrollBar != nullptr) {
		pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
	}
	if (pResult != nullptr) {
		return pResult;
	}

	UiPoint ptNewScrollPos(GetScrollPos().cx, GetScrollPos().cy);
	return Box::FindControl(Proc, pData, uFlags, ptNewScrollPos);
}

UiSize ScrollBox::GetScrollPos() const
{
	UiSize sz;
	if ((m_pVerticalScrollBar != nullptr) && m_pVerticalScrollBar->IsValid()) {
		sz.cy = static_cast<LONG>(m_pVerticalScrollBar->GetScrollPos());
	}
	if ((m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsValid()) {
		sz.cx = static_cast<LONG>(m_pHorizontalScrollBar->GetScrollPos());
	}
	return sz;
}

UiSize ScrollBox::GetScrollRange() const
{
	UiSize sz;
	if ((m_pVerticalScrollBar != nullptr) && m_pVerticalScrollBar->IsValid()) {
		sz.cy = static_cast<LONG>(m_pVerticalScrollBar->GetScrollRange());
	}
	if ((m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsValid()) {
		sz.cx = static_cast<LONG>(m_pHorizontalScrollBar->GetScrollRange());
	}
	return sz;
}

void ScrollBox::SetScrollPos(UiSize szPos)
{
	if (szPos.cy < 0) {
		szPos.cy = 0;
		m_scrollAnimation->Reset();
	}
	else if (szPos.cy > GetScrollRange().cy) {
		szPos.cy = GetScrollRange().cy;
		m_scrollAnimation->Reset();
	}

	int64_t cx = 0;
	int64_t cy = 0;
	if( (m_pVerticalScrollBar != nullptr) && m_pVerticalScrollBar->IsValid() ) {
		int64_t iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
		m_pVerticalScrollBar->SetScrollPos(szPos.cy);
		cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
	}

	if( (m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsValid() ) {
		int64_t iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
		m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
		cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
	}

	if (cx == 0 && cy == 0) {
		return;
	}
	LoadImageCache(cy > 0);
	Invalidate();
	SendEvent(kEventScrollChange, (cy == 0) ? 0 : 1, (cx == 0) ? 0 : 1);
}

void ScrollBox::LoadImageCache(bool bFromTopLeft)
{
	UiSize scrollPos = GetScrollPos();
	UiRect rcImageCachePos = GetPos();
	rcImageCachePos.Offset(scrollPos.cx, scrollPos.cy);
	rcImageCachePos.Offset(GetRenderOffset().x, GetRenderOffset().y);
	rcImageCachePos.Inflate(UiRect(0, 730, 0, 730));

	auto forEach = [this, scrollPos, rcImageCachePos](ui::Control* pControl) {
		if (pControl == nullptr) {
			return;
		}
		if (!pControl->IsVisible()) {
			return;
		}
		if (pControl->IsFloat()) {
			return;
		}
		UiRect rcTemp;
		UiRect controlPos = pControl->GetPos();
		if (!::IntersectRect(&rcTemp, &rcImageCachePos, &controlPos)) {
			pControl->UnLoadImageCache();
		}
		else {
			pControl->InvokeLoadImageCache();
		}
	};

	if (!bFromTopLeft) {
		std::for_each(m_items.rbegin(), m_items.rend(), forEach);
	}
	else {
		std::for_each(m_items.begin(), m_items.end(), forEach);
	}
}

void ScrollBox::SetScrollPosY(int y)
{
	UiSize scrollPos = GetScrollPos();
	scrollPos.cy = y;
	SetScrollPos(scrollPos);
}

void ScrollBox::SetScrollPosX(int x)
{
    UiSize scrollPos = GetScrollPos();
    scrollPos.cx = x;
    SetScrollPos(scrollPos);
}

void ScrollBox::LineUp(int deltaValue, bool withAnimation)
{
	int cyLine = GetVerScrollUnitPixels();
	if (cyLine == 0) {
		cyLine = GlobalManager::Instance().Dpi().GetScaleInt(m_nVerScrollUnitPixelsDefault);
	}
	if (deltaValue != DUI_NOSET_VALUE) {
		cyLine = std::min(cyLine, deltaValue);
	}

	UiSize scrollPos = GetScrollPos();
	if (scrollPos.cy <= 0) {
		return;
	}

	if (!withAnimation) {
		scrollPos.cy -= cyLine;
		if (scrollPos.cy < 0)
		{
			scrollPos.cy = 0;
		}
		SetScrollPos(scrollPos);
	}
	else {
		m_scrollAnimation->SetStartValue(scrollPos.cy);
		if (m_scrollAnimation->IsPlaying()) {
			if (m_scrollAnimation->GetEndValue() > m_scrollAnimation->GetStartValue()) {
				m_scrollAnimation->SetEndValue(scrollPos.cy - cyLine);
			}
			else {
				m_scrollAnimation->SetEndValue(m_scrollAnimation->GetEndValue() - cyLine);
			}
		}
		else {
			m_scrollAnimation->SetEndValue(scrollPos.cy - cyLine);
		}
		m_scrollAnimation->SetSpeedUpRatio(0);
		m_scrollAnimation->SetSpeedDownfactorA(-0.012);
		m_scrollAnimation->SetSpeedDownRatio(0.5);
		m_scrollAnimation->SetTotalMillSeconds(DUI_NOSET_VALUE);
		m_scrollAnimation->SetCallback(nbase::Bind(&ScrollBox::SetScrollPosY, this, std::placeholders::_1));
		m_scrollAnimation->Start();
	}
}

void ScrollBox::LineDown(int deltaValue, bool withAnimation)
{
	int cyLine = GetVerScrollUnitPixels();
	if (cyLine == 0) {
		cyLine = GlobalManager::Instance().Dpi().GetScaleInt(m_nVerScrollUnitPixelsDefault);
	}
	if (deltaValue != DUI_NOSET_VALUE) {
		cyLine = std::min(cyLine, deltaValue);
	}

	UiSize scrollPos = GetScrollPos();
	if (scrollPos.cy >= GetScrollRange().cy) {
		return;
	}

	if (!withAnimation) {
		scrollPos.cy += cyLine;
		if (scrollPos.cy < 0)
		{
			scrollPos.cy = 0;
		}
		SetScrollPos(scrollPos);
	}
	else {
		m_scrollAnimation->SetStartValue(scrollPos.cy);
		if (m_scrollAnimation->IsPlaying()) {
			if (m_scrollAnimation->GetEndValue() < m_scrollAnimation->GetStartValue()) {
				m_scrollAnimation->SetEndValue(scrollPos.cy + cyLine);
			}
			else {
				m_scrollAnimation->SetEndValue(m_scrollAnimation->GetEndValue() + cyLine);
			}
		}
		else {
			m_scrollAnimation->SetEndValue(scrollPos.cy + cyLine);
		}
		m_scrollAnimation->SetSpeedUpRatio(0);
		m_scrollAnimation->SetSpeedDownfactorA(-0.012);
		m_scrollAnimation->SetSpeedDownRatio(0.5);
		m_scrollAnimation->SetTotalMillSeconds(DUI_NOSET_VALUE);
		m_scrollAnimation->SetCallback(nbase::Bind(&ScrollBox::SetScrollPosY, this, std::placeholders::_1));
		m_scrollAnimation->Start();
	}
}
void ScrollBox::LineLeft(int detaValue)
{
    int cxLine = GetHorScrollUnitPixels();
    if (cxLine == 0) {
        cxLine = GlobalManager::Instance().Dpi().GetScaleInt(m_nHerScrollUnitPixelsDefault);
    }
    if (detaValue != DUI_NOSET_VALUE) {
        cxLine = std::min(cxLine, detaValue);
    }

    UiSize scrollPos = GetScrollPos();
    if (scrollPos.cx <= 0) {
        return;
    }
    scrollPos.cx -= cxLine;
    if (scrollPos.cx < 0)
    {
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

void ScrollBox::LineRight(int detaValue)
{
    int cxLine = GetHorScrollUnitPixels();
    if (cxLine == 0) {
        cxLine = GlobalManager::Instance().Dpi().GetScaleInt(m_nHerScrollUnitPixelsDefault);
    }
    if (detaValue != DUI_NOSET_VALUE) {
        cxLine = std::min(cxLine, detaValue);
    }

    UiSize scrollPos = GetScrollPos();
    if (scrollPos.cx >= GetScrollRange().cx) {
        return;
    }
    scrollPos.cx += cxLine;
    if (scrollPos.cx > GetScrollRange().cx)
    {
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
	UiSize sz = GetScrollPos();
	int iOffset = GetRect().bottom - GetRect().top - GetLayout()->GetPadding().top - GetLayout()->GetPadding().bottom;
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsValid() ) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
	sz.cy -= iOffset;
	SetScrollPos(sz);
}

void ScrollBox::PageDown()
{
	UiSize sz = GetScrollPos();
	int iOffset = GetRect().bottom - GetRect().top - GetLayout()->GetPadding().top - GetLayout()->GetPadding().bottom;
	if ((m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsValid()) {
		iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
	}
	sz.cy += iOffset;
	SetScrollPos(sz);
}

void ScrollBox::HomeUp()
{
	UiSize sz = GetScrollPos();
	sz.cy = 0;
	SetScrollPos(sz);
}

void ScrollBox::EndDown(bool arrange, bool withAnimation)
{
	if (arrange) {
		SetPosInternally(GetPos());
	}
	
	int renderOffsetY = GetScrollRange().cy - GetScrollPos().cy + (m_renderOffsetYAnimation->GetEndValue() - GetRenderOffset().y);
	if (withAnimation == true && IsVScrollBarValid() && renderOffsetY > 0) {
		PlayRenderOffsetYAnimation(-renderOffsetY);
	}

	UiSize sz = GetScrollPos();
	sz.cy = GetScrollRange().cy;
	SetScrollPos(sz);
}

void ScrollBox::PageLeft()
{
	UiSize sz = GetScrollPos();
	int iOffset = GetRect().right - GetRect().left - GetLayout()->GetPadding().left - GetLayout()->GetPadding().right;
	//if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsValid() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
	sz.cx -= iOffset;
	SetScrollPos(sz);
}

void ScrollBox::PageRight()
{
	UiSize sz = GetScrollPos();
	int iOffset = GetRect().right - GetRect().left - GetLayout()->GetPadding().left - GetLayout()->GetPadding().right;
	//if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsValid() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
	sz.cx += iOffset;
	SetScrollPos(sz);
}

void ScrollBox::HomeLeft()
{
	UiSize sz = GetScrollPos();
	sz.cx = 0;
	SetScrollPos(sz);
}

void ScrollBox::EndRight()
{
	UiSize sz = GetScrollPos();
	sz.cx = GetScrollRange().cx;
	SetScrollPos(sz);
}

void ScrollBox::TouchUp(int deltaValue)
{
	UiSize scrollPos = GetScrollPos();
	if (scrollPos.cy <= 0) {
		return;
	}

	scrollPos.cy = scrollPos.cy - deltaValue;
	SetScrollPos(scrollPos);
}

void ScrollBox::TouchDown(int deltaValue)
{
	UiSize scrollPos = GetScrollPos();
	if (scrollPos.cy >= GetScrollRange().cy) {
		return;
	}

	scrollPos.cy = scrollPos.cy + deltaValue;
	SetScrollPos(scrollPos);
}

void ScrollBox::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
	if( bEnableVertical && (m_pVerticalScrollBar == nullptr) ) {
		m_pVerticalScrollBar.reset(new ScrollBar);
		m_pVerticalScrollBar->SetVisible(false);
		m_pVerticalScrollBar->SetScrollRange(0);
		m_pVerticalScrollBar->SetOwner(this);
		m_pVerticalScrollBar->SetWindow(GetWindow(), nullptr, false);
		m_pVerticalScrollBar->SetClass(L"vscrollbar");
	}
	else if( !bEnableVertical && (m_pVerticalScrollBar != nullptr) ) {
		m_pVerticalScrollBar.reset();
	}

	if( bEnableHorizontal && (m_pHorizontalScrollBar == nullptr)) {
		m_pHorizontalScrollBar.reset(new ScrollBar);
		m_pHorizontalScrollBar->SetVisible(false);
		m_pHorizontalScrollBar->SetScrollRange(0);
		m_pHorizontalScrollBar->SetHorizontal(true);
		m_pHorizontalScrollBar->SetOwner(this);
		m_pHorizontalScrollBar->SetWindow(GetWindow(), nullptr, false);
		m_pHorizontalScrollBar->SetClass(L"hscrollbar");
	}
	else if( !bEnableHorizontal && (m_pHorizontalScrollBar != nullptr)) {
		m_pHorizontalScrollBar.reset();
	}

	Arrange();
}

ScrollBar* ScrollBox::GetVerticalScrollBar() const
{
	return m_pVerticalScrollBar.get();
}

ScrollBar* ScrollBox::GetHorizontalScrollBar() const
{
	return m_pHorizontalScrollBar.get();
}

void ScrollBox::ProcessVScrollBar(UiRect rc, int cyRequired)
{
	UiRect rcScrollBarPos = rc;
	rcScrollBarPos.left += m_rcScrollBarPadding.left;
	rcScrollBarPos.top += m_rcScrollBarPadding.top;
	rcScrollBarPos.right -= m_rcScrollBarPadding.right;
	rcScrollBarPos.bottom -= m_rcScrollBarPadding.bottom;

	if (m_pVerticalScrollBar == nullptr) {
		return;
	}

	rc.left += GetLayout()->GetPadding().left;
	rc.top += GetLayout()->GetPadding().top;
	rc.right -= GetLayout()->GetPadding().right;
	rc.bottom -= GetLayout()->GetPadding().bottom;
	int nHeight = rc.bottom - rc.top;
	if (cyRequired > nHeight && !m_pVerticalScrollBar->IsValid()) {
		m_pVerticalScrollBar->SetScrollRange(cyRequired - nHeight);
		m_pVerticalScrollBar->SetScrollPos(0);
		m_bScrollProcess = true;
		SetPos(GetRect());
		m_bScrollProcess = false;

		return;
	}
	// No scrollbar required
	if (!m_pVerticalScrollBar->IsValid()) {
		return;
	}

	// Scroll not needed anymore?
	int cyScroll = cyRequired - nHeight;
	if( cyScroll <= 0 && !m_bScrollProcess) {
		m_pVerticalScrollBar->SetScrollPos(0);
		m_pVerticalScrollBar->SetScrollRange(0);
		SetPos(GetRect());
	}
	else {
		if (m_bVScrollBarLeftPos) {
			UiRect rcVerScrollBarPos(rcScrollBarPos.left, rcScrollBarPos.top, rcScrollBarPos.left + m_pVerticalScrollBar->GetFixedWidth(), rcScrollBarPos.bottom);
			m_pVerticalScrollBar->SetPos(rcVerScrollBarPos);
		}
		else {
			UiRect rcVerScrollBarPos(rcScrollBarPos.right - m_pVerticalScrollBar->GetFixedWidth(), rcScrollBarPos.top, rcScrollBarPos.right, rcScrollBarPos.bottom);
			m_pVerticalScrollBar->SetPos(rcVerScrollBarPos);
		}

		if( m_pVerticalScrollBar->GetScrollRange() != cyScroll ) {
			int64_t iScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollRange(::abs(cyScroll));
			if( !m_pVerticalScrollBar->IsValid() ) {
				m_pVerticalScrollBar->SetScrollPos(0);
			}

			if( iScrollPos > m_pVerticalScrollBar->GetScrollPos() ) {
				SetPos(GetRect());
			}
		}
	}
}

void ScrollBox::ProcessHScrollBar(UiRect rc, int cxRequired)
{
	UiRect rcScrollBarPos = rc;
	rcScrollBarPos.left += m_rcScrollBarPadding.left;
	rcScrollBarPos.top += m_rcScrollBarPadding.top;
	rcScrollBarPos.right -= m_rcScrollBarPadding.right;
	rcScrollBarPos.bottom -= m_rcScrollBarPadding.bottom;

	if (m_pHorizontalScrollBar == nullptr) {
		return;
	}

	rc.left += GetLayout()->GetPadding().left;
	rc.top += GetLayout()->GetPadding().top;
	rc.right -= GetLayout()->GetPadding().right;
	rc.bottom -= GetLayout()->GetPadding().bottom;
	int nWidth = rc.right - rc.left;
	if (cxRequired > nWidth && !m_pHorizontalScrollBar->IsValid()) {
		m_pHorizontalScrollBar->SetScrollRange(cxRequired - nWidth);
		m_pHorizontalScrollBar->SetScrollPos(0);
		m_bScrollProcess = true;
		SetPos(GetRect());
		m_bScrollProcess = false;

		return;
	}
	// No scrollbar required
	if (!m_pHorizontalScrollBar->IsValid()) {
		return;
	}

	// Scroll not needed anymore?
	int cxScroll = cxRequired - nWidth;
	if (cxScroll <= 0 && !m_bScrollProcess) {
		m_pHorizontalScrollBar->SetScrollPos(0);
		m_pHorizontalScrollBar->SetScrollRange(0);
		SetPos(GetRect());
	}
	else {
		UiRect rcVerScrollBarPos(rcScrollBarPos.left, rcScrollBarPos.bottom - m_pHorizontalScrollBar->GetFixedHeight(), rcScrollBarPos.right, rcScrollBarPos.bottom);
		m_pHorizontalScrollBar->SetPos(rcVerScrollBarPos);

		if (m_pHorizontalScrollBar->GetScrollRange() != cxScroll) {
			int64_t iScrollPos = m_pHorizontalScrollBar->GetScrollPos();
			m_pHorizontalScrollBar->SetScrollRange(::abs(cxScroll));
			if (!m_pHorizontalScrollBar->IsValid()) {
				m_pHorizontalScrollBar->SetScrollPos(0);
			}

			if (iScrollPos > m_pHorizontalScrollBar->GetScrollPos()) {
				SetPos(GetRect());
			}
		}
	}
}

bool ScrollBox::IsVScrollBarValid() const
{
	if (m_pVerticalScrollBar != nullptr) {
		return m_pVerticalScrollBar->IsValid();
	}
	return false;
}

bool ScrollBox::IsHScrollBarValid() const
{
	if (m_pHorizontalScrollBar != nullptr) {
		return m_pHorizontalScrollBar->IsValid();
	}
	return false;
}

void ScrollBox::ReomveLastItemAnimation()
{
	int nStartRang = GetScrollRange().cy;
	SetPosInternally(GetPos());
	int nEndRang = GetScrollRange().cy;

	int nRenderOffset = nEndRang - nStartRang + (m_renderOffsetYAnimation->GetEndValue() - GetRenderOffset().y);
	if (nRenderOffset < 0) {
		PlayRenderOffsetYAnimation(-nRenderOffset);
	}
}

void ScrollBox::PlayRenderOffsetYAnimation(int nRenderY)
{
	m_renderOffsetYAnimation->SetStartValue(nRenderY);
	m_renderOffsetYAnimation->SetEndValue(0);
	m_renderOffsetYAnimation->SetSpeedUpRatio(0.3);
	m_renderOffsetYAnimation->SetSpeedUpfactorA(0.003);
	m_renderOffsetYAnimation->SetSpeedDownRatio(0.7);
	m_renderOffsetYAnimation->SetTotalMillSeconds(DUI_NOSET_VALUE);
	m_renderOffsetYAnimation->SetMaxTotalMillSeconds(650);
	std::function<void(int)> playCallback = nbase::Bind(&ScrollBox::SetRenderOffsetY, this, std::placeholders::_1);
	m_renderOffsetYAnimation->SetCallback(playCallback);
	m_renderOffsetYAnimation->Start();
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
	return m_nVerScrollUnitPixels;
}

void ScrollBox::SetVerScrollUnitPixels(int nUnitPixels)
{
	GlobalManager::Instance().Dpi().ScaleInt(nUnitPixels);
	m_nVerScrollUnitPixels = nUnitPixels;
}

int ScrollBox::GetHorScrollUnitPixels() const
{
    return m_nHerScrollUnitPixels;
}

void ScrollBox::SetHorScrollUnitPixels(int nUnitPixels)
{
    GlobalManager::Instance().Dpi().ScaleInt(nUnitPixels);
    m_nHerScrollUnitPixels = nUnitPixels;
}

bool ScrollBox::GetScrollBarFloat() const
{
	return m_bScrollBarFloat;
}

void ScrollBox::SetScrollBarFloat(bool bScrollBarFloat)
{
	m_bScrollBarFloat = bScrollBarFloat;
}

bool ScrollBox::GetVScrollBarLeftPos() const
{
	return m_bVScrollBarLeftPos;
}

void ScrollBox::SetVScrollBarLeftPos(bool bLeftPos)
{
	m_bVScrollBarLeftPos = bLeftPos;
}

ui::UiRect ScrollBox::GetScrollBarPadding() const
{
	return m_rcScrollBarPadding;
}

void ScrollBox::SetScrollBarPadding(UiRect rcScrollBarPadding)
{
	GlobalManager::Instance().Dpi().ScaleRect(rcScrollBarPadding);
	m_rcScrollBarPadding = rcScrollBarPadding;
}

void ScrollBox::ClearImageCache()
{
	__super::ClearImageCache();

	if (m_pHorizontalScrollBar != nullptr) {
		m_pHorizontalScrollBar->ClearImageCache();
	}
	if (m_pVerticalScrollBar != nullptr) {
		m_pVerticalScrollBar->ClearImageCache();
	}
}

} // namespace ui
