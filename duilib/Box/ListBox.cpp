#include "ListBox.h"
#include "duilib/Core/ScrollBar.h"

namespace ui 
{

ListBox::ListBox(Layout* pLayout) : 
	ScrollBox(pLayout),
	m_bScrollSelect(false),
	m_bMultiSelect(false),
	m_bSelNextWhenRemoveActive(true),
	m_iCurSel(Box::InvalidIndex),
	m_pCompareFunc(nullptr),
	m_pCompareContext(nullptr)
{
}

std::wstring ListBox::GetType() const { return L"ListBox"; }

void ListBox::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if ((strName == L"multi_select") || (strName == L"multiselect")) {
		SetMultiSelect(strValue == L"true");
	}
	else if ((strName == L"scroll_select") || (strName == L"scrollselect")) {
		SetScrollSelect(strValue == L"true");
	}
	else if (strName == L"select_next_when_active_removed") {
		SelectNextWhenActiveRemoved(strValue == L"true");
	}
	else {
		ScrollBox::SetAttribute(strName, strValue);
	}
}

void ListBox::HandleEvent(const EventArgs& event)
{
	if (!IsMouseEnabled() && (event.Type > kEventMouseBegin) && (event.Type < kEventMouseEnd)) {
		if (GetParent() != nullptr) {
			GetParent()->SendEvent(event);
		}
		else {
			ScrollBox::HandleEvent(event);
		}
		return;
	}

	if (m_bMultiSelect) {
		//������ѡ������£���֧������ĵ�ѡ�߼�
		ScrollBox::HandleEvent(event);
		return;
	}

	size_t itemIndex = Box::InvalidIndex;
	switch (event.Type) {
	case kEventMouseButtonDown:
	case kEventMouseButtonUp:
		break;
	case kEventKeyDown:
		switch (event.chKey) {
		case VK_UP:
			itemIndex = FindSelectable(!Box::IsValidItemIndex(m_iCurSel) ? 0 : m_iCurSel - 1, false);
			SelectItem(itemIndex);
			return;
		case VK_DOWN:
			itemIndex = FindSelectable(!Box::IsValidItemIndex(m_iCurSel) ? 0 : m_iCurSel + 1, true);
			SelectItem(itemIndex);
			return;
		case VK_HOME:
			itemIndex = FindSelectable(0, false);
			SelectItem(itemIndex);
			return;
		case VK_END:
			itemIndex = FindSelectable(GetItemCount() - 1, true);
			SelectItem(itemIndex);
			return;
		}
		break;
	case kEventMouseWheel:
	{
		int detaValue = static_cast<int>(event.wParam);
		if (detaValue > 0) {
			if (m_bScrollSelect) {
				itemIndex = FindSelectable(!Box::IsValidItemIndex(m_iCurSel) ? 0 : m_iCurSel - 1, false);
				SelectItem(itemIndex);
				return;
			}
			break;
		}
		else {
			if (m_bScrollSelect) {
				itemIndex = FindSelectable(!Box::IsValidItemIndex(m_iCurSel) ? 0 : m_iCurSel + 1, true);
				SelectItem(itemIndex);
				return;
			}
			break;
		}
	}
	break;
	}

	ScrollBox::HandleEvent(event);
}

void ListBox::SendEvent(EventType eventType, WPARAM wParam, LPARAM lParam, TCHAR tChar, const UiPoint& mousePos)
{
	return ScrollBox::SendEvent(eventType, wParam, lParam, tChar, mousePos);
}

void ListBox::SendEvent(const EventArgs& event)
{
	ScrollBox::SendEvent(event);
}

size_t ListBox::GetCurSel() const
{
	return m_iCurSel;
}

void ListBox::SetCurSel(size_t iIndex)
{
	m_iCurSel = iIndex;
}

void ListBox::SelectNextWhenActiveRemoved(bool bSelectNextItem)
{
	m_bSelNextWhenRemoveActive = bSelectNextItem;
}

void ListBox::GetSelectedItems(std::vector<size_t>& selectedIndexs) const
{
	selectedIndexs.clear();
	const size_t itemCount = GetItemCount();
	for (size_t iIndex = 0; iIndex < itemCount; ++iIndex) {
		ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(m_items[iIndex]);
		if (pListItem != nullptr) {
			if (pListItem->IsSelected()) {
				selectedIndexs.push_back(iIndex);
			}
		}
	}
}

bool ListBox::SelectItem(size_t iIndex, bool bTakeFocus, bool bTriggerEvent)
{
	if (m_bMultiSelect) {
		//��ѡ
		return SelectItemMulti(iIndex, bTakeFocus, bTriggerEvent);
	}
	else {
		//��ѡ
		return SelectItemSingle(iIndex, bTakeFocus, bTriggerEvent);
	}
}

bool ListBox::UnSelectItem(size_t iIndex, bool bTriggerEvent)
{
	Control* pControl = GetItemAt(iIndex);
	if (pControl != nullptr) {
		ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
		if ((pListItem != nullptr) && pListItem->IsSelected()) {
			pListItem->OptionTemplate<Box>::Selected(false, bTriggerEvent);
			if (bTriggerEvent) {
				SendEvent(kEventUnSelect, iIndex, Box::InvalidIndex);
			}
			//����״̬�仯ʱ�ػ�
			Invalidate();
		}
	}
	if (iIndex == m_iCurSel) {
		m_iCurSel = Box::InvalidIndex;
	}
	return true;
}

bool ListBox::SelectItemSingle(size_t iIndex, bool bTakeFocus, bool bTriggerEvent)
{
	//��ѡ
	if (iIndex == m_iCurSel) {
		Control* pControl = GetItemAt(iIndex);
		if (pControl == nullptr) {
			m_iCurSel = Box::InvalidIndex;
			return false;
		}		
		//ȷ���ɼ���Ȼ�󷵻�	
		UiRect rcItem = pControl->GetPos();
		EnsureVisible(rcItem);
		if (bTakeFocus) {
			pControl->SetFocus();
		}
		ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
		if ((pListItem != nullptr) && !pListItem->IsSelected()) {
			pListItem->OptionTemplate<Box>::Selected(true, false);
			if (bTriggerEvent) {
				SendEvent(kEventSelect, m_iCurSel, Box::InvalidIndex);
			}
		}
		Invalidate();
		return true;
	}
	const size_t iOldSel = m_iCurSel;
	if (Box::IsValidItemIndex(iOldSel)) {
		//ȡ����ѡ�����ѡ��״̬
		Control* pControl = GetItemAt(iOldSel);
		if (pControl != nullptr) {
			ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
			if ((pListItem != nullptr) && pListItem->IsSelected()) {
				pListItem->OptionTemplate<Box>::Selected(false, bTriggerEvent);
				if (bTriggerEvent) {
					SendEvent(kEventUnSelect, iOldSel, Box::InvalidIndex);
				}
			}
		}
		m_iCurSel = Box::InvalidIndex;
	}
	if (!Box::IsValidItemIndex(iIndex)) {
		Invalidate();
		return false;
	}

	Control* pControl = GetItemAt(iIndex);
	if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()) {
		Invalidate();
		return false;
	}
	ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
	if (pListItem == nullptr) {
		Invalidate();
		return false;
	}
	m_iCurSel = iIndex;
	//����ѡ��״̬
	pListItem->OptionTemplate<Box>::Selected(true, bTriggerEvent);
	pControl = GetItemAt(m_iCurSel);
	if (pControl != nullptr) {
		UiRect rcItem = pControl->GetPos();
		EnsureVisible(rcItem);
		if (bTakeFocus) {
			pControl->SetFocus();
		}
	}

	if (bTriggerEvent) {
		SendEvent(kEventSelect, m_iCurSel, iOldSel);
	}
	Invalidate();
	return true;
}

bool ListBox::SelectItemMulti(size_t iIndex, bool bTakeFocus, bool bTriggerEvent)
{
	//��ѡ
	size_t iOldSel = m_iCurSel;
	m_iCurSel = Box::InvalidIndex;
	if (!Box::IsValidItemIndex(iIndex)) {
		Invalidate();
		return false;
	}
	Control* pControl = GetItemAt(iIndex);
	if ((pControl == nullptr) || !pControl->IsVisible() || !pControl->IsEnabled()){
		Invalidate();
		return false;
	}
	ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
	if (pListItem == nullptr) {
		Invalidate();
		return false;
	}
	
	if (pListItem->IsSelected()) {
		//��ѡʱ���ٴ�ѡ��ʱ����ȡ��ѡ����
		pListItem->OptionTemplate<Box>::Selected(false, bTriggerEvent);
		if (bTriggerEvent) {
			SendEvent(kEventUnSelect, iIndex, Box::InvalidIndex);
		}
	}
	else {
		//���ԭ���Ƿ�ѡ��״̬������Ϊѡ��״̬
		pListItem->OptionTemplate<Box>::Selected(true, bTriggerEvent);
		UiRect rcItem = pControl->GetPos();
		EnsureVisible(rcItem);
		if (bTakeFocus) {
			pControl->SetFocus();
		}
		if (bTriggerEvent) {
			SendEvent(kEventSelect, iIndex, iOldSel);
		}
	}	
	Invalidate();
	return true;
}

void ListBox::EnsureVisible(const UiRect& rcItem)
{
	UiRect rcNewItem = rcItem;
	UiSize scrollOffset = GetScrollOffset();
	rcNewItem.Offset(-scrollOffset.cx, -scrollOffset.cy);
	UiRect rcList = GetPos();
	rcList.Deflate(GetLayout()->GetPadding());

	ScrollBar* pHorizontalScrollBar = GetHScrollBar();
	if (pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) {
		ASSERT(pHorizontalScrollBar->GetFixedHeight().GetInt32() > 0);
		rcList.bottom -= pHorizontalScrollBar->GetFixedHeight().GetInt32();
	}

	if ((rcNewItem.left >= rcList.left) && (rcNewItem.top >= rcList.top) && 
		(rcNewItem.right <= rcList.right) && (rcNewItem.bottom <= rcList.bottom)) {
		ListBoxItem* listBoxElement = dynamic_cast<ListBoxItem*>(GetParent());
		IListBoxOwner* lisBoxOwner = nullptr;
		if (listBoxElement != nullptr) {
			lisBoxOwner = listBoxElement->GetOwner();
		}
		if (lisBoxOwner != nullptr) {
			lisBoxOwner->EnsureVisible(rcNewItem);
		}		
		return;
	}

	int32_t dx = 0;
	if (rcNewItem.left < rcList.left) {
		dx = rcNewItem.left - rcList.left;
	}
	if (rcNewItem.right > rcList.right) {
		dx = rcNewItem.right - rcList.right;
	}
	int32_t dy = 0;
	if (rcNewItem.top < rcList.top) {
		dy = rcNewItem.top - rcList.top;
	}
	if (rcNewItem.bottom > rcList.bottom) {
		dy = rcNewItem.bottom - rcList.bottom;
	}
	UiSize64 sz = GetScrollPos();
	SetScrollPos(UiSize64(sz.cx + dx, sz.cy + dy));
	Invalidate();
}

void ListBox::StopScroll()
{
	StopScrollAnimation();
}

bool ListBox::ButtonDown(const EventArgs& msg)
{
	bool ret = __super::ButtonDown(msg);
	StopScroll();
	return ret;
}

bool ListBox::IsHorizontalLayout() const
{
	LayoutType type = GetLayout()->GetLayoutType();
	bool bHorizontal = false;
	if ((type == LayoutType::HLayout) ||
		(type == LayoutType::HTileLayout) ||
		(type == LayoutType::VirtualHLayout) ||
		(type == LayoutType::VirtualHTileLayout)) {
		bHorizontal = true;
	}
	return bHorizontal;
}

bool ListBox::ScrollItemToTop(size_t iIndex)
{
	Control* pControl = GetItemAt(iIndex);
	if ((pControl == nullptr) || !pControl->IsVisible()) {
		return false;
	}
	if (IsHorizontalLayout()) {
		//���򲼾�
		if (GetScrollRange().cx != 0) {
			UiSize64 scrollPos = GetScrollPos();
			scrollPos.cx = pControl->GetPos().left - GetPosWithoutPadding().left;
			if (scrollPos.cx >= 0) {
				SetScrollPos(scrollPos);
				return true;
			}
		}
	}
	else {
		//���򲼾�
		if (GetScrollRange().cy != 0) {
			UiSize64 scrollPos = GetScrollPos();
			scrollPos.cy = pControl->GetPos().top - GetPosWithoutPadding().top;
			if (scrollPos.cy >= 0) {
				SetScrollPos(scrollPos);
				return true;
			}
		}
	}
	return false;
}

bool ListBox::ScrollItemToTop(const std::wstring& itemName)
{
	const size_t itemCount = m_items.size();
	for (size_t iIndex = 0; iIndex < itemCount; ++iIndex) {
		Control* pControl = m_items[iIndex];
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if (pControl->IsNameEquals(itemName)) {
			return ScrollItemToTop(iIndex);
		}
	}
	return false;
}

Control* ListBox::GetTopItem() const
{
	if (IsHorizontalLayout()) {
		//���򲼾�
		int32_t listLeft = GetPos().left + GetLayout()->GetPadding().left + GetScrollOffset().cx;
		for (Control* pControl : m_items) {
			ASSERT(pControl != nullptr);
			if (pControl->IsVisible() && !pControl->IsFloat() && pControl->GetPos().right >= listLeft) {
				return pControl;
			}
		}
	}
	else {
		//���򲼾�
		int32_t listTop = GetPos().top + GetLayout()->GetPadding().top + GetScrollOffset().cy;
		for (Control* pControl : m_items) {
			ASSERT(pControl != nullptr);
			if (pControl->IsVisible() && !pControl->IsFloat() && pControl->GetPos().bottom >= listTop) {
				return pControl;
			}
		}
	}
	return nullptr;
}

bool ListBox::SetItemIndex(Control* pControl, size_t iIndex)
{
	size_t iOrginIndex = GetItemIndex(pControl);
	if (!Box::IsValidItemIndex(iOrginIndex)) {
		return false;
	}
	if (iOrginIndex == iIndex) {
		return true;
	}

	ListBoxItem* pSelectedListItem = nullptr;
	if (Box::IsValidItemIndex(m_iCurSel)) {
		pSelectedListItem = dynamic_cast<ListBoxItem*>(GetItemAt(m_iCurSel));
	}
	if (!ScrollBox::SetItemIndex(pControl, iIndex)) {
		return false;
	}
	size_t iMinIndex = std::min(iOrginIndex, iIndex);
	size_t iMaxIndex = std::max(iOrginIndex, iIndex);
	for(size_t i = iMinIndex; i < iMaxIndex + 1; ++i) {
		Control* pItemControl = GetItemAt(i);
		ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pItemControl);
		if( pListItem != NULL ) {
			pListItem->SetListBoxIndex(i);
		}
	}
	if (Box::IsValidItemIndex(m_iCurSel) && pSelectedListItem != nullptr) {
		m_iCurSel = pSelectedListItem->GetListBoxIndex();
	}
	return true;
}

void ListBox::EnsureVisible(size_t iIndex)
{
	Control* pControl = GetItemAt(iIndex);
	ASSERT(pControl != nullptr);
	if (pControl != nullptr) {
		UiRect rcItem = pControl->GetPos();
		EnsureVisible(rcItem);
	}
}

bool ListBox::SelectItem(size_t iIndex)
{
	return SelectItem(iIndex, true, true);
}

void ListBox::SelectPreviousItem()
{
	if (Box::IsValidItemIndex(m_iCurSel) && (m_iCurSel > 0)) {
		SelectItem(m_iCurSel - 1);
	}
}

void ListBox::SelectNextItem()
{
	if (m_iCurSel < GetItemCount() - 1) {
		SelectItem(m_iCurSel + 1);
	}
}

bool ListBox::AddItem(Control* pControl)
{
	// Override the AddItem() method so we can add items specifically to
	// the intended widgets. Headers are assumed to be
	// answer the correct interface so we can add multiple list headers.
	// The list items should know about us
	ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
	if( pListItem != nullptr) {
		pListItem->SetOwner(this);
		pListItem->SetListBoxIndex(GetItemCount());
	}
	return ScrollBox::AddItem(pControl);
}

bool ListBox::AddItemAt(Control* pControl, size_t iIndex)
{
	// Override the AddItemAt() method so we can add items specifically to
	// the intended widgets. Headers and are assumed to be
	// answer the correct interface so we can add multiple list headers.

	if (!ScrollBox::AddItemAt(pControl, iIndex)) {
		return false;
	}

	// The list items should know about us
	ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
	if( pListItem != nullptr ) {
		pListItem->SetOwner(this);
		pListItem->SetListBoxIndex(iIndex);
	}

	const size_t itemCount = GetItemCount();
	for(size_t i = iIndex + 1; i < itemCount; ++i) {
		Control* p = GetItemAt(i);
		pListItem = dynamic_cast<ListBoxItem*>(p);
		if( pListItem != nullptr ) {
			pListItem->SetListBoxIndex(i);
		}
	}
	if (Box::IsValidItemIndex(m_iCurSel) && (m_iCurSel >= iIndex)) {
		m_iCurSel += 1;
	}
	return true;
}

bool ListBox::RemoveItem(Control* pControl)
{
	size_t iIndex = GetItemIndex(pControl);
	if (!Box::IsValidItemIndex(iIndex)) {
		return false;
	}
	return RemoveItemAt(iIndex);
}

bool ListBox::RemoveItemAt(size_t iIndex)
{
	if (!ScrollBox::RemoveItemAt(iIndex)) {
		return false;
	}
	const size_t itemCount = GetItemCount();
	for(size_t i = iIndex; i < itemCount; ++i) {
		Control* p = GetItemAt(i);
		ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(p);
		if (pListItem != nullptr) {
			pListItem->SetListBoxIndex(i);
		}
	}

	if (Box::IsValidItemIndex(m_iCurSel)) {
		if (iIndex == m_iCurSel) {
			if (!m_bMultiSelect && m_bSelNextWhenRemoveActive) {
				SelectItem(FindSelectable(m_iCurSel--, false));
			}
			else {
				m_iCurSel = Box::InvalidIndex;
			}
		}
		else if (iIndex < m_iCurSel) {
			m_iCurSel -= 1;
		}
	}
	return true;
}

void ListBox::RemoveAllItems()
{
	m_iCurSel = Box::InvalidIndex;
	ScrollBox::RemoveAllItems();
}

bool ListBox::SortItems(PFNCompareFunc pfnCompare, void* pCompareContext)
{
	if (pfnCompare == nullptr) {
		return false;
	}		
	if (m_items.empty()) {
		return true;
	}

	m_pCompareFunc = pfnCompare;
	m_pCompareContext = pCompareContext;
	qsort_s(&(*m_items.begin()), m_items.size(), sizeof(Control*), ListBox::ItemComareFunc, this);	
	ListBoxItem* pItem = nullptr;
	const size_t itemCount = m_items.size();
	for (size_t i = 0; i < itemCount; ++i) {
		pItem = dynamic_cast<ListBoxItem*>(m_items[i]);
		if (pItem != nullptr) {
			pItem->SetListBoxIndex(i);
			pItem->Selected(false, false);
		}
	}
	SelectItem(Box::InvalidIndex);
	SetPos(GetPos());
	Invalidate();
	return true;
}

int __cdecl ListBox::ItemComareFunc(void *pvlocale, const void *item1, const void *item2)
{
	ListBox *pThis = (ListBox*)pvlocale;
	if (!pThis || !item1 || !item2) {
		return 0;
	}
	return pThis->ItemComareFunc(item1, item2);
}

int __cdecl ListBox::ItemComareFunc(const void *item1, const void *item2)
{
	Control *pControl1 = *(Control**)item1;
	Control *pControl2 = *(Control**)item2;
	return m_pCompareFunc(pControl1, pControl2, m_pCompareContext);
}

bool ListBox::GetMultiSelect() const
{
	return m_bMultiSelect;
}

void ListBox::SetMultiSelect(bool bMultiSelect)
{
	m_bMultiSelect = bMultiSelect;
	if (!bMultiSelect) {
		//ȡ�����еĶ�ѡ
		ListBoxItem* pItem = nullptr;
		const size_t itemCount = m_items.size();
		for (size_t i = 0; i < itemCount; ++i) {
			pItem = dynamic_cast<ListBoxItem*>(m_items[i]);
			if ((pItem != nullptr) && pItem->IsSelected()){
				if (m_iCurSel != i) {
					pItem->Selected(false, false);
				}
			}
		}
		Invalidate();
	}
}

bool ListBox::GetScrollSelect() const
{
	return m_bScrollSelect;
}

void ListBox::SetScrollSelect(bool bScrollSelect)
{
	m_bScrollSelect = bScrollSelect;
}

/////////////////////////////////////////////////////////////////////////////////////

ListBoxItem::ListBoxItem() :
	m_iListBoxIndex(Box::InvalidIndex),
	m_iElementIndex(Box::InvalidIndex),
	m_pOwner(nullptr)
{
	SetTextStyle(TEXT_LEFT | TEXT_VCENTER | TEXT_END_ELLIPSIS | TEXT_NOCLIP | TEXT_SINGLELINE, false);
}

std::wstring ListBoxItem::GetType() const {  return DUI_CTR_LISTBOX_ITEM; }

void ListBoxItem::Selected(bool bSelected, bool bTriggerEvent)
{
	if (!IsEnabled()) {
		return;
	}
	if (m_pOwner != nullptr) {
		if (bSelected) {
			m_pOwner->SelectItem(m_iListBoxIndex, false, bTriggerEvent);
		}
		else {
			m_pOwner->UnSelectItem(m_iListBoxIndex, bTriggerEvent);
		}
	}
}

void ListBoxItem::SetSelected(bool bSelected)
{
	__super::SetSelected(bSelected);
	if (m_pOwner != nullptr) {
		//ͬ��ListBox��ѡ��ID
		if (bSelected) {
			m_pOwner->SetCurSel(m_iListBoxIndex);
		}
		else {
			if (m_pOwner->GetCurSel() == m_iListBoxIndex) {
				m_pOwner->SetCurSel(Box::InvalidIndex);
			}
		}
	}
}

void ListBoxItem::HandleEvent(const EventArgs& event)
{
	if (!IsMouseEnabled() && 
		(event.Type > kEventMouseBegin) && 
		(event.Type < kEventMouseEnd)) {
		//��ǰ�ؼ���ֹ���������Ϣʱ������������Ϣת�����ϲ㴦��
		if (m_pOwner != nullptr) {
			m_pOwner->SendEvent(event);
		}
		else {
			__super::HandleEvent(event);
		}
		return;
	}
	else if (event.Type == kEventMouseDoubleClick) {
		if (!IsActivatable()) {
			return;
		}
	}
	else if (event.Type == kEventKeyDown && IsEnabled()) {
		if (event.chKey == VK_RETURN) {
			if (IsActivatable()) {
				SendEvent(kEventReturn);
			}
			return;
		}
	}
	__super::HandleEvent(event);

	// An important twist: The list-item will send the event not to its immediate
	// parent but to the "attached" list. A list may actually embed several components
	// in its path to the item, but key-presses etc. needs to go to the actual list.
	//if( m_pOwner != NULL ) m_pOwner->HandleMessage(event); else Control::HandleMessage(event);
}

IListBoxOwner* ListBoxItem::GetOwner()
{
    return m_pOwner;
}

void ListBoxItem::SetOwner(IListBoxOwner* pOwner)
{
    m_pOwner = pOwner;
}

size_t ListBoxItem::GetListBoxIndex() const
{
    return m_iListBoxIndex;
}

void ListBoxItem::SetListBoxIndex(size_t iIndex)
{
	m_iListBoxIndex = iIndex;
}

size_t ListBoxItem::GetElementIndex() const
{
	return m_iElementIndex;
}

void ListBoxItem::SetElementIndex(size_t iIndex)
{
	m_iElementIndex = iIndex;
}

bool ListBoxItem::IsSelectableType() const
{
	return true;
}

} // namespace ui