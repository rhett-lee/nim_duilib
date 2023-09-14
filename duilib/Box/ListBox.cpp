#include "ListBox.h"
#include "duilib/Core/ScrollBar.h"

namespace ui 
{

//多选的时候，是否显示选择背景色: 0 - 默认规则; 1 - 显示背景色; 2: 不显示背景色
enum ePaintSelectedColors
{
	PAINT_SELECTED_COLORS_DEFAULT = 0,
	PAINT_SELECTED_COLORS_YES = 1,
	PAINT_SELECTED_COLORS_NO = 2
};

ListBox::ListBox(Layout* pLayout) : 
	ScrollBox(pLayout),
	m_bScrollSelect(false),
	m_bMultiSelect(false),
	m_uPaintSelectedColors(PAINT_SELECTED_COLORS_DEFAULT),
	m_bSelNextWhenRemoveActive(true),
	m_iCurSel(Box::InvalidIndex),
	m_pCompareFunc(nullptr),
	m_pCompareContext(nullptr)
{
}

std::wstring ListBox::GetType() const { return L"ListBox"; }

void ListBox::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"multi_select") {
		SetMultiSelect(strValue == L"true");
	}
	else if (strName == L"paint_selected_colors") {
		if (strValue == L"true") {
			m_uPaintSelectedColors = PAINT_SELECTED_COLORS_YES;
		}
		else {
			m_uPaintSelectedColors = PAINT_SELECTED_COLORS_NO;
		}
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

void ListBox::HandleEvent(const EventArgs& msg)
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
	if (IsMultiSelect()) {
		//允许多选的情况下，不支持下面的单选逻辑
		__super::HandleEvent(msg);
		return;
	}

	size_t itemIndex = Box::InvalidIndex;
	switch (msg.Type) {
	case kEventKeyDown:
		switch (msg.chKey) {
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
		int detaValue = GET_WHEEL_DELTA_WPARAM(msg.wParam);
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

	ScrollBox::HandleEvent(msg);
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
	if (IsMultiSelect()) {
		//多选
		return SelectItemMulti(iIndex, bTakeFocus, bTriggerEvent);
	}
	else {
		//单选
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
			//仅在状态变化时重绘
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
	//单选
	if (iIndex == m_iCurSel) {
		Control* pControl = GetItemAt(iIndex);
		if (pControl == nullptr) {
			m_iCurSel = Box::InvalidIndex;
			return false;
		}		
		//确保可见，然后返回		
		if (bTakeFocus) {
			pControl->SetFocus();
		}
		EnsureVisible(pControl->GetPos());
		ListBoxItem* pListItem = dynamic_cast<ListBoxItem*>(pControl);
		if ((pListItem != nullptr) && !pListItem->IsSelected()) {
			pListItem->OptionTemplate<Box>::Selected(true, false);			
		}
		if (bTriggerEvent) {
			SendEvent(kEventSelect, m_iCurSel, m_iCurSel);
		}
		Invalidate();
		return true;
	}
	const size_t iOldSel = m_iCurSel;
	if (Box::IsValidItemIndex(iOldSel)) {
		//取消旧选择项的选择状态
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
	//设置选择状态
	pListItem->OptionTemplate<Box>::Selected(true, bTriggerEvent);
	pControl = GetItemAt(m_iCurSel);
	if (pControl != nullptr) {		
		if (bTakeFocus) {
			pControl->SetFocus();
		}
		EnsureVisible(pControl->GetPos());
	}

	if (bTriggerEvent) {
		SendEvent(kEventSelect, m_iCurSel, iOldSel);
	}
	Invalidate();
	return true;
}

bool ListBox::SelectItemMulti(size_t iIndex, bool bTakeFocus, bool bTriggerEvent)
{
	//多选
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
		//多选时，再次选择时，按取消选择处理
		pListItem->OptionTemplate<Box>::Selected(false, bTriggerEvent);
		if (bTriggerEvent) {
			SendEvent(kEventUnSelect, iIndex, Box::InvalidIndex);
		}
	}
	else {
		//如果原来是非选择状态，更新为选择状态
		pListItem->OptionTemplate<Box>::Selected(true, bTriggerEvent);		
		if (bTakeFocus) {			
			pControl->SetFocus();
		}
		EnsureVisible(pControl->GetPos());
		if (bTriggerEvent) {
			SendEvent(kEventSelect, iIndex, iOldSel);
		}
	}	
	Invalidate();
	return true;
}

void ListBox::EnsureVisible(const UiRect& rcItem)
{
	EnsureVisible(rcItem, false);
}

void ListBox::EnsureVisible(const UiRect& rcItem, bool bAtCenter)
{
	UiRect rcNewItem = rcItem;
	UiSize scrollOffset = GetScrollOffset();
	rcNewItem.Offset(-scrollOffset.cx, -scrollOffset.cy);
	UiRect rcList = GetPos();
	rcList.Deflate(GetPadding());

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
	if (bAtCenter) {
		if (rcNewItem.left < rcList.CenterX()) {
			dx = rcNewItem.left - rcList.CenterX();
		}
		if (rcNewItem.right > rcList.CenterX()) {
			dx = rcNewItem.right - rcList.CenterX();
		}
	}
	else {
		if (rcNewItem.left < rcList.left) {
			dx = rcNewItem.left - rcList.left;
		}
		if (rcNewItem.right > rcList.right) {
			dx = rcNewItem.right - rcList.right;
		}
	}
	int32_t dy = 0;
	if (bAtCenter) {
		if (rcNewItem.top < rcList.CenterY()) {
			dy = rcNewItem.top - rcList.CenterY();
		}
		if (rcNewItem.bottom > rcList.CenterY()) {
			dy = rcNewItem.bottom - rcList.CenterY();
		}
	}
	else {
		if (rcNewItem.top < rcList.top) {
			dy = rcNewItem.top - rcList.top;
		}
		if (rcNewItem.bottom > rcList.bottom) {
			dy = rcNewItem.bottom - rcList.bottom;
		}
	}
	UiSize64 sz = GetScrollPos();
	SetScrollPos(UiSize64(sz.cx + dx, sz.cy + dy));
	Invalidate();
}

void ListBox::StopScroll()
{
	StopScrollAnimation();
}

bool ListBox::CanPaintSelectedColors(bool bHasStateImages) const
{
	if (m_uPaintSelectedColors == PAINT_SELECTED_COLORS_YES) {
		return true;
	}
	else if (m_uPaintSelectedColors == PAINT_SELECTED_COLORS_NO) {
		return false;
	}
	if (bHasStateImages && IsMultiSelect()) {
		//如果有CheckBox，多选的时候，默认不显示选择背景色
		return false;
	}
	return true;
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
		//横向布局
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
		//纵向布局
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
		//横向布局
		int32_t listLeft = GetPos().left + GetPadding().left + GetScrollOffset().cx;
		for (Control* pControl : m_items) {
			ASSERT(pControl != nullptr);
			if (pControl->IsVisible() && !pControl->IsFloat() && pControl->GetPos().right >= listLeft) {
				return pControl;
			}
		}
	}
	else {
		//纵向布局
		int32_t listTop = GetPos().top + GetPadding().top + GetScrollOffset().cy;
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
		EnsureVisible(rcItem, false);
	}
}

void ListBox::EnsureVisible(size_t iIndex, bool bAtCenter)
{
	Control* pControl = GetItemAt(iIndex);
	ASSERT(pControl != nullptr);
	if (pControl != nullptr) {
		UiRect rcItem = pControl->GetPos();
		EnsureVisible(rcItem, bAtCenter);
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
			if (!IsMultiSelect() && m_bSelNextWhenRemoveActive) {
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

bool ListBox::IsMultiSelect() const
{
	return m_bMultiSelect;
}

void ListBox::SetMultiSelect(bool bMultiSelect)
{
	m_bMultiSelect = bMultiSelect;
	if (!bMultiSelect) {
		//只要bMultiSelect为false，就执行取消现有多选的逻辑
		//此处不能判断与原值是否变化来决定是否执行下面的代码，否则可能会影响子类（TreeView的逻辑）
		if (OnSwitchToSingleSelect()) {
			Invalidate();
		}
	}
}

bool ListBox::OnSwitchToSingleSelect()
{
	bool bChanged = false;
	ListBoxItem* pItem = nullptr;
	const size_t itemCount = m_items.size();
	for (size_t i = 0; i < itemCount; ++i) {
		pItem = dynamic_cast<ListBoxItem*>(m_items[i]);
		if ((pItem != nullptr) && pItem->IsSelected()) {
			if (m_iCurSel != i) {
				pItem->SetSelected(false);
				pItem->Invalidate();
				bChanged = true;
			}
		}
	}
	if (UpdateCurSelItemSelectStatus()) {
		bChanged = true;
	}
	return bChanged;
}

bool ListBox::UpdateCurSelItemSelectStatus()
{
	//同步当前选择项的状态
	bool bChanged = false;
	size_t curSelIndex = GetCurSel();
	if (Box::IsValidItemIndex(curSelIndex)) {
		bool bSelectItem = false;
		ListBoxItem* pItem = dynamic_cast<ListBoxItem*>(GetItemAt(curSelIndex));
		if (pItem != nullptr) {
			bSelectItem = pItem->IsSelected();
		}
		if (!bSelectItem) {
			SetCurSel(Box::InvalidIndex);
			bChanged = true;
		}
	}
	return bChanged;
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

void ListBoxItem::SetItemSelected(bool bSelected)
{
	if (__super::IsSelected() == bSelected) {
		return;
	}
	if (m_pOwner == nullptr) {
		__super::SetSelected(bSelected);
		return;
	}
	if (m_pOwner->IsMultiSelect()) {
		//多选：直接修改状态
		__super::SetSelected(bSelected);
		
		//同步ListBox的选择ID
		if (bSelected) {
			m_pOwner->SetCurSel(m_iListBoxIndex);
		}
		else {
			if (m_pOwner->GetCurSel() == m_iListBoxIndex) {
				m_pOwner->SetCurSel(Box::InvalidIndex);
			}
		}
	}
	else {
		//单选：需要调用选择函数
		__super::SetSelected(bSelected);
		m_pOwner->SelectItem(m_iListBoxIndex, false, false);
	}
}

bool ListBoxItem::CanPaintSelectedColors() const
{
	bool bHasStateImages = HasStateImages();
	if (m_pOwner != nullptr) {
		return m_pOwner->CanPaintSelectedColors(bHasStateImages);
	}
	return __super::CanPaintSelectedColors();
}

void ListBoxItem::HandleEvent(const EventArgs& msg)
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
	if (msg.Type == kEventMouseDoubleClick) {
		if (!IsActivatable()) {
			return;
		}
	}
	else if (msg.Type == kEventKeyDown && IsEnabled()) {
		if (msg.chKey == VK_RETURN) {
			if (IsActivatable()) {
				SendEvent(kEventReturn);
			}
			return;
		}
	}
	__super::HandleEvent(msg);

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
