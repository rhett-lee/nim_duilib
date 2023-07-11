#include "Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"

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
	if ((pControl == nullptr) || (!pControl->IsVisible())) {
		return UiSize();
	}
	if (rcContainer.GetWidth() < 0) {
		rcContainer.right = rcContainer.left;
	}
	if (rcContainer.GetHeight() < 0) {
		rcContainer.bottom = rcContainer.bottom;
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
	ASSERT(childSize.cx >= DUI_LENGTH_STRETCH);
	ASSERT(childSize.cy >= DUI_LENGTH_STRETCH);

	if (childSize.cx == DUI_LENGTH_STRETCH) {
		childSize.cx = std::max(0, (int)szAvailable.cx);
	}
	if (childSize.cx < pControl->GetMinWidth()) {
		childSize.cx = pControl->GetMinWidth();
	}
	if (childSize.cx > pControl->GetMaxWidth()) {
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
	if(strName == L"padding") {
		UiRect rcPadding;
		LPTSTR pstr = NULL;
		rcPadding.left = wcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);    
		rcPadding.top = wcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		rcPadding.right = wcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		rcPadding.bottom = wcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetPadding(rcPadding, true);
	}
	else if( strName == L"childmargin") {
		SetChildMargin(_wtoi(strValue.c_str()));
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
		if ((pControl == nullptr) || (!pControl->IsVisible())){
			continue;
		}
		UiSize controlSize = SetFloatPos(pControl, rc);
		size.cx = std::max(size.cx, controlSize.cx);
		size.cy = std::max(size.cy, controlSize.cy);
	}
	return size;
}

UiSize Layout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	UiSize maxSize;
	UiSize itemSize;
	for (Control* pControl : items) {
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}
		itemSize = pControl->EstimateSize(szAvailable);
		if (itemSize.cx < pControl->GetMinWidth()) {
			itemSize.cx = pControl->GetMinWidth();
		}
		if (itemSize.cx > pControl->GetMaxWidth()) {
			itemSize.cx = pControl->GetMaxWidth();
		}
		if (itemSize.cy < pControl->GetMinHeight()) {
			itemSize.cy = pControl->GetMinHeight();
		}
		if (itemSize.cy > pControl->GetMaxHeight()) {
			itemSize.cy = pControl->GetMaxHeight();
		}
		itemSize.cx = std::max((int)itemSize.cx, 0);
		itemSize.cy = std::max((int)itemSize.cy, 0);
		
		maxSize.cx = std::max(itemSize.cx + pControl->GetMargin().left + pControl->GetMargin().right, maxSize.cx);
		maxSize.cy = std::max(itemSize.cy + pControl->GetMargin().top + pControl->GetMargin().bottom, maxSize.cy);
	}
	maxSize.cx += m_rcPadding.left + m_rcPadding.right;
	maxSize.cy += m_rcPadding.top + m_rcPadding.bottom;
	return maxSize;
}

void Layout::SetPadding(UiRect rcPadding, bool bNeedDpiScale /*= true*/)
{
	rcPadding.left = std::max((int)rcPadding.left, 0);
	rcPadding.right = std::max((int)rcPadding.right, 0);
	rcPadding.top = std::max((int)rcPadding.top, 0);
	rcPadding.bottom = std::max((int)rcPadding.bottom, 0);

	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleRect(rcPadding);
	}
	m_rcPadding = rcPadding;
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner != nullptr) {
		m_pOwner->Arrange();
	}
}

void Layout::SetChildMargin(int iMargin)
{
	ASSERT(iMargin >= 0);
	iMargin = std::max(iMargin, 0);
	GlobalManager::Instance().Dpi().ScaleInt(iMargin);
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
	else if (strName == L"mousechild") {
		SetMouseChildEnabled(strValue == L"true");
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
	if ((GetFixedWidth() != DUI_LENGTH_AUTO) && (GetFixedHeight() != DUI_LENGTH_AUTO)) {
		//如果宽高都不是auto属性，则直接返回
		return fixedSize;
	}
	if (!IsReEstimateSize()) {
		//使用缓存中的估算结果
		return GetEstimateSize();
	}

	szAvailable.cx -= (m_pLayout->GetPadding().left + m_pLayout->GetPadding().right);
	szAvailable.cy -= (m_pLayout->GetPadding().top + m_pLayout->GetPadding().bottom);
	szAvailable.cx = std::max((int)szAvailable.cx, 0);
	szAvailable.cy = std::max((int)szAvailable.cy, 0);
	UiSize sizeByChild = m_pLayout->EstimateSizeByChild(m_items, szAvailable);
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
		if (!pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}
		if ((pControl->GetFixedWidth() == DUI_LENGTH_AUTO) || 
			(pControl->GetFixedHeight() == DUI_LENGTH_AUTO)) {
			if (pControl->IsReEstimateSize()) {
				SetReEstimateSize(true);
				break;
			}
		}
	}
	SetEstimateSize(fixedSize);
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

} // namespace ui
