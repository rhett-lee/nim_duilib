#include "Box.h"
#include "duilib/Core/Window.h"

namespace ui
{
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
	if (m_pLayout != nullptr) {
		delete m_pLayout;
		m_pLayout = nullptr;
	}
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

void Box::SetWindow(Window* pManager)
{
	Control::SetWindow(pManager);
}

void Box::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (m_pLayout->SetAttribute(strName, strValue))	{
		return;
	}
	else if ((strName == L"mouse_child") || (strName == L"mousechild")) {
		SetMouseChildEnabled(strValue == L"true");
	}
	else {
		Control::SetAttribute(strName, strValue);
	}
}

void Box::SetPos(UiRect rc)
{
	Control::SetPos(rc);
	if (m_pLayout != nullptr) {
		m_pLayout->ArrangeChild(m_items, rc);	
	}	
}

UiRect Box::GetPosWithoutPadding() const
{
	UiRect rc = GetPos();
	if (m_pLayout) {
		rc.Deflate(m_pLayout->GetPadding());
	}
	rc.Validate();
	return rc;
}

UiRect Box::GetRectWithoutPadding() const
{
	UiRect rc = GetRect();
	if (m_pLayout) {
		rc.Deflate(m_pLayout->GetPadding());
	}	
	rc.Validate();
	return rc;
}

void Box::PaintChild(IRender* pRender, const UiRect& rcPaint)
{
	UiRect rcTemp;
	if (!UiRect::Intersect(rcTemp, rcPaint, GetRect())) {
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

UiEstSize Box::EstimateSize(UiSize szAvailable)
{
	UiFixedSize fixedSize = GetFixedSize();
	if (!fixedSize.cx.IsAuto() && !fixedSize.cy.IsAuto()) {
		//如果宽高都不是auto属性，则直接返回
		return MakeEstSize(fixedSize);
	}
	szAvailable.Validate();
	if (!IsReEstimateSize(szAvailable)) {
		//使用缓存中的估算结果
		return GetEstimateSize();
	}
	//Box控件本身的大小，不包含外边距（本身也是一个控件，可以有文字/背景图片等）
	UiPadding rcPadding = m_pLayout->GetPadding();
	UiSize szNewAvailable = szAvailable;
	szNewAvailable.cx -= (rcPadding.left + rcPadding.right);
	szNewAvailable.cy -= (rcPadding.top + rcPadding.bottom);
	UiEstSize estSizeBySelf = __super::EstimateSize(szNewAvailable);
	UiSize sizeBySelf;
	if (estSizeBySelf.cx.IsInt32()) {
		sizeBySelf.cx = estSizeBySelf.cx.GetInt32() + rcPadding.left + rcPadding.right;
	}
	if (estSizeBySelf.cy.IsInt32()) {
		sizeBySelf.cy = estSizeBySelf.cy.GetInt32() + rcPadding.top + rcPadding.bottom;
	}

	//子控件的大小，包含内边距，但不包含外边距
	UiSize sizeByChild = m_pLayout->EstimateSizeByChild(m_items, szAvailable);
	
	SetReEstimateSize(false);
	for (auto pControl : m_items) {
		ASSERT(pControl != nullptr);
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}
		if ((pControl->GetFixedWidth().IsAuto()) || 
			(pControl->GetFixedHeight().IsAuto())) {
			if (pControl->IsReEstimateSize(szAvailable)) {
				SetReEstimateSize(true);
				break;
			}
		}
	}
	if (fixedSize.cx.IsAuto()) {
		fixedSize.cx.SetInt32(std::max(sizeByChild.cx, sizeBySelf.cx));
	}
	if (fixedSize.cy.IsAuto()) {
		fixedSize.cy.SetInt32(std::max(sizeByChild.cy, sizeBySelf.cy));
	}

	UiEstSize estSize = MakeEstSize(fixedSize);
	SetEstimateSize(estSize, szAvailable);
	return estSize;
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
		UiPoint pt(*(static_cast<UiPoint*>(pData)));
		if ((pData != nullptr) && !GetRect().ContainsPt(pt)) {
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
	UiRect rc = GetRectWithoutPadding();
	if ((uFlags & UIFIND_TOP_FIRST) != 0) {
		for (int it = (int)m_items.size() - 1; it >= 0; --it) {
			if (m_items[it] == nullptr) {
				continue;
			}
			Control* pControl = nullptr;
			if ((uFlags & UIFIND_HITTEST) != 0) {
				ASSERT(pData != nullptr);
				if (pData != nullptr) {
					UiPoint newPoint(*(static_cast<UiPoint*>(pData)));
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
					!rc.ContainsPt(*(static_cast<UiPoint*>(pData)))) {
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
					UiPoint newPoint(*(static_cast<UiPoint*>(pData)));
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
					!rc.ContainsPt(*(static_cast<UiPoint*>(pData)))) {
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
		m_pLayout = pLayout;
		m_pLayout->SetOwner(this);
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
