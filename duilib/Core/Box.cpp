#include "Box.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/Window.h"
#include "duilib/Control/ScrollBar.h"
#include "duilib/Utils/DpiManager.h"
#include "duilib/Animation/AnimationPlayer.h"

#include <tchar.h>
#include <algorithm>

namespace ui
{
Layout::Layout() :
	m_rcPadding(0, 0, 0, 0),
	m_iChildMargin(0),
	m_pOwner(nullptr)
{

}

void Layout::SetOwner(Box* pOwner)
{
	m_pOwner = pOwner;
}

UiSize Layout::SetFloatPos(Control* pControl, UiRect rcContainer)
{
	ASSERT(pControl != nullptr);
	if (pControl == nullptr) {
		return UiSize();
	}
	if (!pControl->IsVisible()) {
		return UiSize();
	}

	int childLeft = 0;
	int childRight = 0;
	int childTop = 0;
	int childBottm = 0;
	UiRect rcMargin = pControl->GetMargin();
	int iPosLeft = rcContainer.left + rcMargin.left;
	int iPosRight = rcContainer.right - rcMargin.right;
	int iPosTop = rcContainer.top + rcMargin.top;
	int iPosBottom = rcContainer.bottom - rcMargin.bottom;
	UiSize szAvailable(iPosRight - iPosLeft, iPosBottom - iPosTop);
	UiSize childSize = pControl->EstimateSize(szAvailable);
	if ((pControl->GetFixedWidth() == DUI_LENGTH_AUTO)  && 
		(pControl->GetFixedHeight() == DUI_LENGTH_AUTO)	&& 
		(pControl->GetMaxWidth() == DUI_LENGTH_STRETCH)) {
		int maxwidth = std::max(0, (int)szAvailable.cx);
		if (childSize.cx > maxwidth) {
			pControl->SetFixedWidth(maxwidth, false, true);
			childSize = pControl->EstimateSize(szAvailable);
			pControl->SetFixedWidth(DUI_LENGTH_AUTO, false, true);
		}
	}
	if (childSize.cx == DUI_LENGTH_STRETCH) {
		childSize.cx = std::max(0, (int)szAvailable.cx);
	}
	if (childSize.cx < pControl->GetMinWidth()) {
		childSize.cx = pControl->GetMinWidth();
	}
	if (pControl->GetMaxWidth() >= 0 && childSize.cx > pControl->GetMaxWidth()) {
		childSize.cx = pControl->GetMaxWidth();
	}

	if (childSize.cy == DUI_LENGTH_STRETCH) {
		childSize.cy = std::max(0, (int)szAvailable.cy);
	}
	if (childSize.cy < pControl->GetMinHeight()) {
		childSize.cy = pControl->GetMinHeight();
	}
	if (childSize.cy > pControl->GetMaxHeight()) {
		childSize.cy = pControl->GetMaxHeight();
	}

	int childWidth = childSize.cx;
	int childHeight = childSize.cy;
	HorAlignType horAlignType = pControl->GetHorAlignType();
	VerAlignType verAlignType = pControl->GetVerAlignType();

	if (horAlignType == kHorAlignLeft) {
		childLeft = iPosLeft;
		childRight = childLeft + childWidth;
	}
	else if (horAlignType == kHorAlignRight) {
		childRight = iPosRight;
		childLeft = childRight - childWidth;
	}
	else if (horAlignType == kHorAlignCenter) {
		childLeft = iPosLeft + (iPosRight - iPosLeft - childWidth) / 2;
		childRight = childLeft + childWidth;
	}

	if (verAlignType == kVerAlignTop) {
		childTop = iPosTop;
		childBottm = childTop + childHeight;
	}
	else if (verAlignType == kVerAlignBottom) {
		childBottm = iPosBottom;
		childTop = childBottm - childHeight;
	}
	else if (verAlignType == kVerAlignCenter) {
		childTop = iPosTop + (iPosBottom - iPosTop - childHeight) / 2;
		childBottm = childTop + childHeight;
	}

	UiRect childPos(childLeft, childTop, childRight, childBottm);
	pControl->SetPos(childPos);
	return UiSize(childPos.GetWidth(), childPos.GetHeight());
}

bool Layout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	bool hasAttribute = true;
	if( strName == _T("padding") ) {
		UiRect rcPadding;
		LPTSTR pstr = NULL;
		rcPadding.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
		rcPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetPadding(rcPadding, true);
	}
	else if( strName == _T("childmargin") ) {
		SetChildMargin(_ttoi(strValue.c_str()));
	}
	else {
		hasAttribute = false;
	}

	return hasAttribute;
}

UiSize Layout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	UiSize size;
	for (Control* pControl : items) {
		if (pControl == nullptr){
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
		UiSize new_size = SetFloatPos(pControl, rc);
		size.cx = std::max(size.cx, new_size.cx);
		size.cy = std::max(size.cy, new_size.cy);
	}
	return size;
}

UiSize Layout::AjustSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	UiSize maxSize(-9999, -9999);
	UiSize itemSize;
	for (Control* pControl : items) {
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
		itemSize = pControl->EstimateSize(szAvailable);
		if (itemSize.cx < pControl->GetMinWidth()) {
			itemSize.cx = pControl->GetMinWidth();
		}
		if ((pControl->GetMaxWidth() >= 0) && (itemSize.cx > pControl->GetMaxWidth())) {
			itemSize.cx = pControl->GetMaxWidth();
		}
		if (itemSize.cy < pControl->GetMinHeight()) {
			itemSize.cy = pControl->GetMinHeight();
		}
		if ((pControl->GetMaxHeight() >= 0) && (itemSize.cy > pControl->GetMaxHeight())) {
			itemSize.cy = pControl->GetMaxHeight();
		}
		maxSize.cx = std::max(itemSize.cx + pControl->GetMargin().left + pControl->GetMargin().right, maxSize.cx);
		maxSize.cy = std::max(itemSize.cy + pControl->GetMargin().top + pControl->GetMargin().bottom, maxSize.cy);
	}
	maxSize.cx += m_rcPadding.left + m_rcPadding.right;
	maxSize.cy += m_rcPadding.top + m_rcPadding.bottom;
	return maxSize;
}

void Layout::SetPadding(UiRect rcPadding, bool bNeedDpiScale /*= true*/)
{
	if (bNeedDpiScale) {
		DpiManager::GetInstance()->ScaleRect(rcPadding);
	}
	m_rcPadding = rcPadding;
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner != nullptr) {
		m_pOwner->Arrange();
	}
}

void Layout::SetChildMargin(int iMargin)
{
	DpiManager::GetInstance()->ScaleInt(iMargin);
	m_iChildMargin = iMargin;
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner != nullptr) {
		m_pOwner->Arrange();
	}
}

UiRect Layout::GetInternalPos() const
{
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner == nullptr) {
		return UiRect();
	}
	UiRect internalPos = m_pOwner->GetPos();
	internalPos.Deflate(m_rcPadding);
	return internalPos;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

Box::Box(Layout* pLayout) :
	m_pLayout(pLayout),
	m_bAutoDestroyChild(true),
	m_bDelayedDestroy(true),
	m_bMouseChildEnabled(true),
	m_items()
{
	ASSERT(m_pLayout != nullptr);
	if (m_pLayout) {
		m_pLayout->SetOwner(this);
	}
}

Box::~Box()
{
	m_bDelayedDestroy = false;
	RemoveAllItems();
}

std::wstring Box::GetType() const { return DUI_CTR_BOX; }

#if defined(ENABLE_UIAUTOMATION)
UIAControlProvider* Box::GetUIAProvider()
{
	if (m_pUIAProvider == nullptr)
	{
		m_pUIAProvider = static_cast<UIAControlProvider*>(new (std::nothrow) UIABoxProvider(this));
	}
	return m_pUIAProvider;
}
#endif

void Box::SetWindow(Window* pManager, Box* pParent, bool bInit)
{
	for (auto pControl : m_items) {
		ASSERT(pControl != nullptr);
		if (pControl != nullptr) {
			pControl->SetWindow(pManager, this, bInit);
		}		
	}
	Control::SetWindow(pManager, pParent, bInit);
}

void Box::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (m_pLayout->SetAttribute(strName, strValue))	{
		return;
	}
	else if (strName == _T("mousechild")) {
		SetMouseChildEnabled(strValue == _T("true"));
	}
	else {
		Control::SetAttribute(strName, strValue);
	}
}

void Box::SetPos(UiRect rc)
{
	UiRect rect = rc;	
	rc.left += m_pLayout->GetPadding().left;
	rc.top += m_pLayout->GetPadding().top;
	rc.right -= m_pLayout->GetPadding().right;
	rc.bottom -= m_pLayout->GetPadding().bottom;
	m_pLayout->ArrangeChild(m_items, rc);

	Control::SetPos(rect);
}

void Box::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
	UiRect rcTemp;
	if (!::IntersectRect(&rcTemp, &rcPaint, &GetRect())) {
		return;
	}

	for (auto pControl : m_items) {
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
		pControl->AlphaPaint(pRender, rcPaint);
	}
}

void Box::SetEnabled(bool bEnabled)
{
	if (IsEnabled() == bEnabled) {
		return;
	}

	Control::SetEnabled(bEnabled);

	//子控件的Enable状态，与父控件是同步的(如果支持不同步，相关业务逻辑需要做调整)
	for (auto pControl : m_items) {
		ASSERT(pControl != nullptr);
		if (pControl != nullptr) {
			pControl->SetEnabled(bEnabled);
		}
	}

	Invalidate();
}

void Box::SetVisible(bool bVisible)
{
	if (IsVisible() == bVisible) {
		return;
	}
	bool v = IsVisible();
	__super::SetVisible(bVisible);
	if (IsVisible() != v) {
		//子控件的Visible控件是同步的(如果支持不同步，相关业务逻辑需要做调整，除了判断控件自身是否可见，还要判断父控件是否可见)
		for (auto pControl : m_items){
			ASSERT(pControl != nullptr);
			if (pControl != nullptr) {
				pControl->SetVisible(bVisible);
			}
		}
	}
}

UiSize Box::EstimateSize(UiSize szAvailable)
{
	UiSize fixedSize = GetFixedSize();
	if (GetFixedWidth() == DUI_LENGTH_AUTO || GetFixedHeight() == DUI_LENGTH_AUTO) {
		if (!IsReEstimateSize()) {
			return GetEstimateSize();
		}

		szAvailable.cx -= (m_pLayout->GetPadding().left + m_pLayout->GetPadding().right);
		szAvailable.cy -= (m_pLayout->GetPadding().top + m_pLayout->GetPadding().bottom);
		UiSize sizeByChild = m_pLayout->AjustSizeByChild(m_items, szAvailable);
		if (GetFixedWidth() == DUI_LENGTH_AUTO) {
			fixedSize.cx = sizeByChild.cx;
		}
		if (GetFixedHeight() == DUI_LENGTH_AUTO) {
			fixedSize.cy = sizeByChild.cy;
		}

		SetReEstimateSize(false);
		for (auto pControl : m_items) {
			ASSERT(pControl != nullptr);
			if (pControl == nullptr) {
				continue;
			}
			if (!pControl->IsVisible()) {
				continue;
			}
			if (pControl->GetFixedWidth() == DUI_LENGTH_AUTO || pControl->GetFixedHeight() == DUI_LENGTH_AUTO) {
				if (pControl->IsReEstimateSize()) {
					SetReEstimateSize(true);
					break;
				}
			}
		}
		SetEstimateSize(fixedSize);
	}

	return fixedSize;
}

Control* Box::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, UiPoint scrollPos)
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
		if ((pData != nullptr) && !::PtInRect(&GetRect(), *(static_cast<LPPOINT>(pData)))) {
			return nullptr;
		}
		if (!m_bMouseChildEnabled) {
			Control* pResult = Control::FindControl(Proc, pData, uFlags);
			return pResult;
		}
	}

	if ((uFlags & UIFIND_ME_FIRST) != 0) {
		Control* pControl = Control::FindControl(Proc, pData, uFlags);
		if (pControl != nullptr) {
			return pControl;
		}
	}
	UiRect rc = GetRect();
	rc.left += m_pLayout->GetPadding().left;
	rc.top += m_pLayout->GetPadding().top;
	rc.right -= m_pLayout->GetPadding().right;
	rc.bottom -= m_pLayout->GetPadding().bottom;

	if ((uFlags & UIFIND_TOP_FIRST) != 0) {
		for (int it = (int)m_items.size() - 1; it >= 0; --it) {
			if (m_items[it] == nullptr) {
				continue;
			}
			Control* pControl = nullptr;
			if ((uFlags & UIFIND_HITTEST) != 0) {
				ASSERT(pData != nullptr);
				if (pData != nullptr) {
					UiPoint newPoint(*(static_cast<LPPOINT>(pData)));
					newPoint.Offset(scrollPos);
					pControl = m_items[it]->FindControl(Proc, &newPoint, uFlags);
				}				
			}
			else {
				pControl = m_items[it]->FindControl(Proc, pData, uFlags);
			}
			if (pControl != nullptr) {
				if ((uFlags & UIFIND_HITTEST) != 0 &&
					!pControl->IsFloat() && 
					(pData != nullptr) &&
					!::PtInRect(&rc, *(static_cast<LPPOINT>(pData)))) {
					continue;
				}
				else {
					return pControl;
				}
			}
		}
	}
	else {
		for (Control* pItemControl : m_items) {
			if (pItemControl == nullptr) {
				continue;
			}
			Control* pControl = nullptr;
			if ((uFlags & UIFIND_HITTEST) != 0) {
				ASSERT(pData != nullptr);
				if (pData != nullptr) {
					UiPoint newPoint(*(static_cast<LPPOINT>(pData)));
					newPoint.Offset(scrollPos);
					pControl = pItemControl->FindControl(Proc, &newPoint, uFlags);
				}
			}
			else {
				pControl = pItemControl->FindControl(Proc, pData, uFlags);
			}
			if (pControl != nullptr) {
				if ((uFlags & UIFIND_HITTEST) != 0 && 
					!pControl->IsFloat() && 
					(pData != nullptr) &&
					!::PtInRect(&rc, *(static_cast<LPPOINT>(pData)))) {
					continue;
				}
				else {
					return pControl;
				}
			}
		}
	}

	Control* pResult = nullptr;
	if ((uFlags & UIFIND_ME_FIRST) == 0) {
		pResult = Control::FindControl(Proc, pData, uFlags);
	}
	return pResult;
}

Control* Box::FindSubControl(const std::wstring& pstrSubControlName)
{
	Control* pSubControl = GetWindow()->FindSubControlByName(this, pstrSubControlName);
	return pSubControl;
}

size_t Box::FindSelectable(size_t iIndex, bool bForward /*= true*/) const
{
	// NOTE: This is actually a helper-function for the list/combo/ect controls
	//       that allow them to find the next enabled/available selectable item
	const size_t itemCount = GetItemCount();
	if (itemCount == 0) {
		return Box::InvalidIndex;
	}
	if (!Box::IsValidItemIndex(iIndex)) {
		iIndex = 0;
	}
	if (iIndex >= (itemCount - 1)) {
		iIndex = itemCount - 1;
	}
	if (bForward) {
		for (size_t i = iIndex; i < itemCount; ++i) {
			Control* pControl = GetItemAt(i);
			if (pControl == nullptr) {
				continue;
			}
			if (pControl->IsSelectableType() &&
				pControl->IsVisible()        &&
				pControl->IsEnabled()) {
				return i;
			}
		}
		return Box::InvalidIndex;
	}
	else {
		for (int i = (int)iIndex; i >= 0; --i) {
			Control* pControl = GetItemAt(static_cast<size_t>(i));
			if (pControl == nullptr) {
				continue;
			}
			if (pControl->IsSelectableType() &&
				pControl->IsVisible()        &&
				pControl->IsEnabled()) {
				return static_cast<size_t>(i);
			}
		}
		return FindSelectable(0, true);
	}
}

Control* Box::GetItemAt(size_t iIndex) const
{
	if (iIndex >= m_items.size()) {
		return nullptr;
	}
	return m_items[iIndex];
}

size_t Box::GetItemIndex(Control* pControl) const
{
	auto it = std::find(m_items.begin(), m_items.end(), pControl);
	if (it == m_items.end()) {
		return Box::InvalidIndex;
	}
	return static_cast<int>(it - m_items.begin());
}

bool Box::SetItemIndex(Control* pControl, size_t iIndex)
{
	if (iIndex >= m_items.size()) {
		return false;
	}
	for (auto it = m_items.begin(); it != m_items.end(); ++it) {
		if( *it == pControl ) {
			Arrange();            
			m_items.erase(it);
			m_items.insert(m_items.begin() + iIndex, pControl);
			return true;
		}
	}
	return false;
}

size_t Box::GetItemCount() const
{
	return m_items.size();
}

bool Box::AddItem(Control* pControl)
{
	return DoAddItemAt(pControl, m_items.size());
}

bool Box::AddItemAt(Control* pControl, size_t iIndex)
{
	return DoAddItemAt(pControl, iIndex);
}

bool Box::DoAddItemAt(Control* pControl, size_t iIndex)
{
	ASSERT(pControl != nullptr);
	if (pControl == NULL) {
		return false;
	}
	if(iIndex > m_items.size() ) {
		ASSERT(FALSE);
		return false;
	}
	Window* pWindow = GetWindow();
	if (pWindow != nullptr) {
		pWindow->InitControls(pControl, this);
	}
	m_items.insert(m_items.begin() + iIndex, pControl);
	if (IsVisible()) {
		Arrange();
	}	
	return true;
}

bool Box::RemoveItem(Control* pControl)
{
	return DoRemoveItem(pControl);
}

bool Box::RemoveItemAt(size_t iIndex)
{
	Control* pControl = GetItemAt(iIndex);
	if (pControl != NULL) {
		return DoRemoveItem(pControl);
	}
	return false;
}

bool Box::DoRemoveItem(Control* pControl)
{
	ASSERT(pControl != nullptr);
	if (pControl == nullptr) {
		return false;
	}

	Window* pWindow = GetWindow();
	for (auto it = m_items.begin(); it != m_items.end(); ++it) {
		if (*it == pControl) {
			Arrange();
			if (m_bAutoDestroyChild) {
				if (m_bDelayedDestroy && (pWindow != nullptr)) {
					pWindow->AddDelayedCleanup(pControl);
				}
				else {
					delete pControl;
				}
			}
			m_items.erase(it);
			return true;
		}
	}
	return false;
}

void Box::RemoveAllItems()
{
	if (m_bAutoDestroyChild) {
		Window* pWindow = GetWindow();
		for (auto it = m_items.begin(); it != m_items.end(); ++it) {
			if (m_bDelayedDestroy && (pWindow != nullptr)) {
				pWindow->AddDelayedCleanup((*it));
			}
			else {
				delete (*it);
			}
		}
	}

	m_items.clear();
	Arrange();
}

void Box::ReSetLayout(Layout* pLayout)
{
	ASSERT(pLayout != nullptr);
	if (pLayout != nullptr) {
		m_pLayout.reset(pLayout);
		m_pLayout->SetOwner(this);
	}	
}

UiRect Box::GetPaddingPos() const
{
	UiRect pos = GetPos();
	UiRect padding = m_pLayout->GetPadding();
	pos.left += padding.left;
	pos.top += padding.top;
	pos.right -= padding.right;
	pos.bottom -= padding.bottom;
	return pos;
}

void Box::InvokeLoadImageCache()
{
	__super::InvokeLoadImageCache();
	for (Control* pControl : m_items) {
		if (pControl != nullptr) {
			pControl->InvokeLoadImageCache();
		}		
	}
}

void Box::UnLoadImageCache()
{
	__super::UnLoadImageCache();
	for (Control* pControl : m_items) {
		if (pControl != nullptr) {
			pControl->UnLoadImageCache();
		}
	}
}

void Box::ClearImageCache()
{
	__super::ClearImageCache();
	for (Control* pControl : m_items) {
		if (pControl != nullptr) {
			pControl->ClearImageCache();
		}
	}
}

UINT Box::GetControlFlags() const
{
	return UIFLAG_DEFAULT; // Box 默认不支持 TAB 切换焦点
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
ScrollableBox::ScrollableBox(Layout* pLayout) :
	Box(pLayout),
	m_pVerticalScrollBar(),
	m_pHorizontalScrollBar(),
	m_nVerScrollUnitPixels(30),
    m_nHerScrollUnitPixels(30),
	m_bScrollProcess(false),
	m_bScrollBarFloat(true),
	m_bVScrollBarLeftPos(false),
	m_bHoldEnd(false),
	m_rcScrollBarPadding()
{
	m_scrollAnimation = std::make_unique<AnimationPlayer>();
	m_renderOffsetYAnimation = std::make_unique<AnimationPlayer>();
}

std::wstring ScrollableBox::GetType() const { return std::wstring(_T("Scrollable")) + DUI_CTR_BOX; }//ScrollableBox

#if defined(ENABLE_UIAUTOMATION)
UIAControlProvider* ScrollableBox::GetUIAProvider()
{
	if (m_pUIAProvider == nullptr)
	{
		m_pUIAProvider = static_cast<UIAControlProvider*>(new (std::nothrow) UIAScrollableBoxProvider(this));
	}
	return m_pUIAProvider;
}
#endif

void ScrollableBox::SetAttribute(const std::wstring& pstrName, const std::wstring& pstrValue)
{
	if( pstrName == _T("vscrollbar") ) {
		EnableScrollBar(pstrValue == _T("true"), GetHorizontalScrollBar() != nullptr);
	}
	else if( pstrName == _T("vscrollbarstyle") ) {
		EnableScrollBar(true, GetHorizontalScrollBar() != nullptr);
		if (GetVerticalScrollBar() != nullptr) {
			GetVerticalScrollBar()->ApplyAttributeList(pstrValue);
		}
	}
	else if( pstrName == _T("hscrollbar") ) {
		EnableScrollBar(GetVerticalScrollBar() != nullptr, pstrValue == _T("true"));
	}
	else if( pstrName == _T("hscrollbarstyle") ) {
		EnableScrollBar(GetVerticalScrollBar() != nullptr, true);
		if (GetHorizontalScrollBar() != nullptr) {
			GetHorizontalScrollBar()->ApplyAttributeList(pstrValue);
		}
	}
	else if( pstrName == _T("scrollbarpadding") ) {
		UiRect rcScrollbarPadding;
		LPTSTR pstr = NULL;
		rcScrollbarPadding.left = _tcstol(pstrValue.c_str(), &pstr, 10);  ASSERT(pstr);    
		rcScrollbarPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcScrollbarPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcScrollbarPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
		SetScrollBarPadding(rcScrollbarPadding);
	}
	else if( pstrName == _T("vscrollunit") ) SetVerScrollUnitPixels(_ttoi(pstrValue.c_str()));
	else if (pstrName == _T("hscrollunit")) SetHorScrollUnitPixels(_ttoi(pstrValue.c_str()));
	else if (pstrName == _T("scrollbarfloat")) SetScrollBarFloat(pstrValue == _T("true"));
	else if (pstrName == _T("vscrollbarleft")) SetVScrollBarLeftPos(pstrValue == _T("true"));
	else if( pstrName == _T("holdend") ) SetHoldEnd(pstrValue == _T("true"));
	else Box::SetAttribute(pstrName, pstrValue);
}

void ScrollableBox::SetPos(UiRect rc)
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

void ScrollableBox::SetPosInternally(UiRect rc)
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

UiSize ScrollableBox::CalcRequiredSize(const UiRect& rc)
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

void ScrollableBox::HandleEvent(const EventArgs& event)
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

bool ScrollableBox::MouseEnter(const EventArgs& msg)
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

bool ScrollableBox::MouseLeave(const EventArgs& msg)
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

void ScrollableBox::PaintChild(IRender* pRender, const UiRect& rcPaint)
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
		
	static bool bFirstPaint = true;
	if (bFirstPaint) {
		bFirstPaint = false;
		LoadImageCache(true);
	}
}

void ScrollableBox::SetMouseEnabled(bool bEnabled)
{
	if (m_pVerticalScrollBar != nullptr) {
		m_pVerticalScrollBar->SetMouseEnabled(bEnabled);
	}
	if (m_pHorizontalScrollBar != nullptr) {
		m_pHorizontalScrollBar->SetMouseEnabled(bEnabled);
	}
	Box::SetMouseEnabled(bEnabled);
}

void ScrollableBox::SetWindow(Window* pManager, Box* pParent, bool bInit)
{
	if (m_pVerticalScrollBar != nullptr) {
		m_pVerticalScrollBar->SetWindow(pManager, this, bInit);
	}
	if (m_pHorizontalScrollBar != nullptr) {
		m_pHorizontalScrollBar->SetWindow(pManager, this, bInit);
	}
	Box::SetWindow(pManager, pParent, bInit);
}

Control* ScrollableBox::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, UiPoint /*scrollPos*/)
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

UiSize ScrollableBox::GetScrollPos() const
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

UiSize ScrollableBox::GetScrollRange() const
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

void ScrollableBox::SetScrollPos(UiSize szPos)
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

void ScrollableBox::LoadImageCache(bool bFromTopLeft)
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

void ScrollableBox::SetScrollPosY(int y)
{
	UiSize scrollPos = GetScrollPos();
	scrollPos.cy = y;
	SetScrollPos(scrollPos);
}

void ScrollableBox::SetScrollPosX(int x)
{
    UiSize scrollPos = GetScrollPos();
    scrollPos.cx = x;
    SetScrollPos(scrollPos);
}

void ScrollableBox::LineUp(int deltaValue, bool withAnimation)
{
	int cyLine = GetVerScrollUnitPixels();
	if (cyLine == 0) {
		cyLine = 20;
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
		m_scrollAnimation->SetCallback(nbase::Bind(&ScrollableBox::SetScrollPosY, this, std::placeholders::_1));
		m_scrollAnimation->Start();
	}
}

void ScrollableBox::LineDown(int deltaValue, bool withAnimation)
{
	int cyLine = GetVerScrollUnitPixels();
	if (cyLine == 0) {
		cyLine = 20;
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
		m_scrollAnimation->SetCallback(nbase::Bind(&ScrollableBox::SetScrollPosY, this, std::placeholders::_1));
		m_scrollAnimation->Start();
	}
}
void ScrollableBox::LineLeft(int detaValue)
{
    int cxLine = GetHorScrollUnitPixels();
    if (cxLine == 0) {
        cxLine = 20;
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
    m_scrollAnimation.SetCallback(nbase::Bind(&ScrollableBox::SetScrollPosX, this, std::placeholders::_1));
    m_scrollAnimation.Start();*/

}

void ScrollableBox::LineRight(int detaValue)
{
    int cxLine = GetHorScrollUnitPixels();
    if (cxLine == 0) {
        cxLine = 20;
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
    //m_scrollAnimation.SetCallback(nbase::Bind(&ScrollableBox::SetScrollPosX, this, std::placeholders::_1));
    //m_scrollAnimation.Start();
}
void ScrollableBox::PageUp()
{
	UiSize sz = GetScrollPos();
	int iOffset = GetRect().bottom - GetRect().top - GetLayout()->GetPadding().top - GetLayout()->GetPadding().bottom;
	if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsValid() ) iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
	sz.cy -= iOffset;
	SetScrollPos(sz);
}

void ScrollableBox::PageDown()
{
	UiSize sz = GetScrollPos();
	int iOffset = GetRect().bottom - GetRect().top - GetLayout()->GetPadding().top - GetLayout()->GetPadding().bottom;
	if ((m_pHorizontalScrollBar != nullptr) && m_pHorizontalScrollBar->IsValid()) {
		iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
	}
	sz.cy += iOffset;
	SetScrollPos(sz);
}

void ScrollableBox::HomeUp()
{
	UiSize sz = GetScrollPos();
	sz.cy = 0;
	SetScrollPos(sz);
}

void ScrollableBox::EndDown(bool arrange, bool withAnimation)
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

void ScrollableBox::PageLeft()
{
	UiSize sz = GetScrollPos();
	int iOffset = GetRect().right - GetRect().left - GetLayout()->GetPadding().left - GetLayout()->GetPadding().right;
	//if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsValid() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
	sz.cx -= iOffset;
	SetScrollPos(sz);
}

void ScrollableBox::PageRight()
{
	UiSize sz = GetScrollPos();
	int iOffset = GetRect().right - GetRect().left - GetLayout()->GetPadding().left - GetLayout()->GetPadding().right;
	//if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsValid() ) iOffset -= m_pVerticalScrollBar->GetFixedWidth();
	sz.cx += iOffset;
	SetScrollPos(sz);
}

void ScrollableBox::HomeLeft()
{
	UiSize sz = GetScrollPos();
	sz.cx = 0;
	SetScrollPos(sz);
}

void ScrollableBox::EndRight()
{
	UiSize sz = GetScrollPos();
	sz.cx = GetScrollRange().cx;
	SetScrollPos(sz);
}

void ScrollableBox::TouchUp(int deltaValue)
{
	UiSize scrollPos = GetScrollPos();
	if (scrollPos.cy <= 0) {
		return;
	}

	scrollPos.cy = scrollPos.cy - deltaValue;
	SetScrollPos(scrollPos);
}

void ScrollableBox::TouchDown(int deltaValue)
{
	UiSize scrollPos = GetScrollPos();
	if (scrollPos.cy >= GetScrollRange().cy) {
		return;
	}

	scrollPos.cy = scrollPos.cy + deltaValue;
	SetScrollPos(scrollPos);
}

void ScrollableBox::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
{
	if( bEnableVertical && (m_pVerticalScrollBar == nullptr) ) {
		m_pVerticalScrollBar.reset(new ScrollBar);
		m_pVerticalScrollBar->SetVisible(false);
		m_pVerticalScrollBar->SetScrollRange(0);
		m_pVerticalScrollBar->SetOwner(this);
		m_pVerticalScrollBar->SetWindow(GetWindow(), nullptr, false);
		m_pVerticalScrollBar->SetClass(_T("vscrollbar"));
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
		m_pHorizontalScrollBar->SetClass(_T("hscrollbar"));
	}
	else if( !bEnableHorizontal && (m_pHorizontalScrollBar != nullptr)) {
		m_pHorizontalScrollBar.reset();
	}

	Arrange();
}

ScrollBar* ScrollableBox::GetVerticalScrollBar() const
{
	return m_pVerticalScrollBar.get();
}

ScrollBar* ScrollableBox::GetHorizontalScrollBar() const
{
	return m_pHorizontalScrollBar.get();
}

void ScrollableBox::ProcessVScrollBar(UiRect rc, int cyRequired)
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

void ScrollableBox::ProcessHScrollBar(UiRect rc, int cxRequired)
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

bool ScrollableBox::IsVScrollBarValid() const
{
	if (m_pVerticalScrollBar != nullptr) {
		return m_pVerticalScrollBar->IsValid();
	}
	return false;
}

bool ScrollableBox::IsHScrollBarValid() const
{
	if (m_pHorizontalScrollBar != nullptr) {
		return m_pHorizontalScrollBar->IsValid();
	}
	return false;
}

void ScrollableBox::ReomveLastItemAnimation()
{
	int nStartRang = GetScrollRange().cy;
	SetPosInternally(GetPos());
	int nEndRang = GetScrollRange().cy;

	int nRenderOffset = nEndRang - nStartRang + (m_renderOffsetYAnimation->GetEndValue() - GetRenderOffset().y);
	if (nRenderOffset < 0) {
		PlayRenderOffsetYAnimation(-nRenderOffset);
	}
}

void ScrollableBox::PlayRenderOffsetYAnimation(int nRenderY)
{
	m_renderOffsetYAnimation->SetStartValue(nRenderY);
	m_renderOffsetYAnimation->SetEndValue(0);
	m_renderOffsetYAnimation->SetSpeedUpRatio(0.3);
	m_renderOffsetYAnimation->SetSpeedUpfactorA(0.003);
	m_renderOffsetYAnimation->SetSpeedDownRatio(0.7);
	m_renderOffsetYAnimation->SetTotalMillSeconds(DUI_NOSET_VALUE);
	m_renderOffsetYAnimation->SetMaxTotalMillSeconds(650);
	std::function<void(int)> playCallback = nbase::Bind(&ScrollableBox::SetRenderOffsetY, this, std::placeholders::_1);
	m_renderOffsetYAnimation->SetCallback(playCallback);
	m_renderOffsetYAnimation->Start();
}

bool ScrollableBox::IsAtEnd() const
{
	return GetScrollRange().cy <= GetScrollPos().cy;
}

bool ScrollableBox::IsHoldEnd() const
{
	return m_bHoldEnd;
}

void ScrollableBox::SetHoldEnd(bool bHoldEnd)
{
	m_bHoldEnd = bHoldEnd;
}

int ScrollableBox::GetVerScrollUnitPixels() const
{
	return m_nVerScrollUnitPixels;
}

void ScrollableBox::SetVerScrollUnitPixels(int nUnitPixels)
{
	DpiManager::GetInstance()->ScaleInt(nUnitPixels);
	m_nVerScrollUnitPixels = nUnitPixels;
}

int ScrollableBox::GetHorScrollUnitPixels() const
{
    return m_nHerScrollUnitPixels;
}

void ScrollableBox::SetHorScrollUnitPixels(int nUnitPixels)
{
    DpiManager::GetInstance()->ScaleInt(nUnitPixels);
    m_nHerScrollUnitPixels = nUnitPixels;
}

bool ScrollableBox::GetScrollBarFloat() const
{
	return m_bScrollBarFloat;
}

void ScrollableBox::SetScrollBarFloat(bool bScrollBarFloat)
{
	m_bScrollBarFloat = bScrollBarFloat;
}

bool ScrollableBox::GetVScrollBarLeftPos() const
{
	return m_bVScrollBarLeftPos;
}

void ScrollableBox::SetVScrollBarLeftPos(bool bLeftPos)
{
	m_bVScrollBarLeftPos = bLeftPos;
}

ui::UiRect ScrollableBox::GetScrollBarPadding() const
{
	return m_rcScrollBarPadding;
}

void ScrollableBox::SetScrollBarPadding(UiRect rcScrollBarPadding)
{
	DpiManager::GetInstance()->ScaleRect(rcScrollBarPadding);
	m_rcScrollBarPadding = rcScrollBarPadding;
}

void ScrollableBox::ClearImageCache()
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
