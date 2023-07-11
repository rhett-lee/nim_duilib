// Copyright 2017, NetEase (Hangzhou) Network Co., Ltd. All rights reserved.
//
// zqw
// 2015/7/22
//
// VirtualListBox class
#include "VirtualListBox.h"
#include "duilib/Core/ScrollBar.h"

namespace ui
{

UiSize VirtualVLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	UiSize sz(rc.GetWidth(), 0);

	VirtualListBox *pList = dynamic_cast<VirtualListBox*>(m_pOwner);
	ASSERT(pList);

	if (pList->UseDefaultLayout()) {
		sz = VLayout::ArrangeChild(items, rc);
	}
	else {
		//TODO: 越界检查
		size_t totalHeight = pList->CalcElementsHeight(Box::InvalidIndex);
		ASSERT(totalHeight <= INT32_MAX);
		sz.cy = static_cast<LONG>(totalHeight);
		pList->ReArrangeChild(false);
	}

	return sz;
}

UiSize VirtualHLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	UiSize sz(0, rc.GetHeight());

	VirtualListBox *pList = dynamic_cast<VirtualListBox*>(m_pOwner);
	ASSERT(pList);

	if (pList->UseDefaultLayout()) {
		sz = HLayout::ArrangeChild(items, rc);
	}
	else {
		//TODO: 越界检查
		size_t totalWidth = pList->CalcElementsHeight(Box::InvalidIndex);
		ASSERT(totalWidth <= INT32_MAX);
		sz.cx = static_cast<LONG>(totalWidth);
		pList->ReArrangeChild(false);
	}

	return sz;
}

VirtualListBox::VirtualListBox(ui::Layout* pLayout) :
ui::ListBox(pLayout),
m_pDataProvider(nullptr),
m_nElementHeight(0),
m_nMaxItemCount(0),
m_nOldScrollPos(0),
m_bArrangedOnce(false),
m_bForceArrange(false)
{
	SetDirection(ListDirection::kListVertical);
}

std::wstring VirtualListBox::GetType() const { return DUI_CTR_VIRTUALLISTBOX; }

void VirtualListBox::SetDataProvider(VirtualListBoxElement *pProvider)
{
	m_pDataProvider = pProvider;
}

void VirtualListBox::SetElementHeight(size_t nHeight)
{
	ASSERT(nHeight > 0);
	if (nHeight > 0) {
		m_nElementHeight = nHeight;
	}
}

void VirtualListBox::InitElement(size_t nMaxItemCount)
{
	ASSERT(m_pDataProvider != 0);
	ASSERT(m_nElementHeight > 0 );
	m_nMaxItemCount = nMaxItemCount;

	size_t nCount = GetElementCount();
	if (nCount > nMaxItemCount) {
		nCount = nMaxItemCount;
	}

	for (size_t i = 0; i < nCount; ++i) {
		Control *pControl = CreateElement();
		this->AddItem(pControl);
		FillElement(pControl, i);
	}
}

void VirtualListBox::SetDirection(ListDirection direction)
{
	m_eDirection = direction;
	ReSetLayout(ListDirection::kListVertical == direction ? dynamic_cast<Layout*>(new VirtualVLayout) : dynamic_cast<Layout*>(new VirtualHLayout));
}

void VirtualListBox::Refresh()
{
	size_t nElementCount = GetElementCount();
	size_t nItemCount = GetItemCount();

	if (nItemCount > nElementCount) {
		size_t n = nItemCount - nElementCount;
		for (size_t i = 0; i < n; ++i) {
			this->RemoveItemAt(0);
		}
	}
	else if (nItemCount < nElementCount) {
		size_t n = 0;
		if (nElementCount <= m_nMaxItemCount) {
			n = nElementCount - nItemCount;
		}
		else {
			n = m_nMaxItemCount - nItemCount;
		}

		for (size_t i = 0; i < n; ++i) {
			Control *pControl = CreateElement();
			this->AddItem(pControl);
		}
	}

	if (UseDefaultLayout()) {
		// 刚从虚拟列表转换到普通模式时，存在布局错误的情况（虚拟列表滚动条接近底部，
		// 然后数据减少，变成普通模式）
		if (nItemCount == m_nMaxItemCount) {
			this->GetLayout()->ArrangeChild(m_items, GetRect());
		}

		for (size_t i = 0; i < m_items.size(); ++i) {
			FillElement(m_items[i], i);
		}
	}
	else {
		if (nElementCount == 0) {
			return;
		}
		ReArrangeChild(true);
		Arrange();
	}
}

void VirtualListBox::RemoveAllItems()
{
	__super::RemoveAllItems();
	if (m_pVerticalScrollBar) {
		m_pVerticalScrollBar->SetScrollPos(0);
	}
	if (m_pHorizontalScrollBar) {
		m_pHorizontalScrollBar->SetScrollPos(0);
	}
	m_nOldScrollPos = 0;
	m_bArrangedOnce = false;
	m_bForceArrange = false;
}

void VirtualListBox::SetForceArrange(bool bForce)
{
	m_bForceArrange = bForce;
}

void VirtualListBox::GetDisplayCollection(std::vector<size_t>& collection)
{
	collection.clear();
	if (GetItemCount() == 0) {
		return;
	}
	ASSERT(m_nElementHeight > 0);
	if (m_nElementHeight < 1) {
		return;
	}

	UiRect rcThis = this->GetPos(false);
	size_t length = (ListDirection::kListVertical == m_eDirection) ? (size_t)rcThis.GetWidth() : (size_t)rcThis.GetHeight();
	size_t scroll_pos = (ListDirection::kListVertical == m_eDirection) ? (size_t)GetScrollPos().cy : (size_t)GetScrollPos().cx;
	size_t min = scroll_pos / m_nElementHeight;
	size_t max = min + (length / m_nElementHeight);
	const size_t nElementCount = GetElementCount();
	if (nElementCount == 0) {
		return;
	}
	if (max >= nElementCount) {
		max = nElementCount - 1;
	}
	for (size_t i = min; i <= max; ++i) {
		collection.push_back(i);
	}
}

void VirtualListBox::EnsureVisible(size_t iIndex, bool bToTop)
{
	if (iIndex >= GetElementCount()) {
		return;
	}
	ASSERT(m_nElementHeight > 0);
	if (m_nElementHeight < 1) {
		return;
	}

	size_t nPos = (m_eDirection == ListDirection::kListVertical) ? (size_t)GetScrollPos().cy : (size_t)GetScrollPos().cx;
	size_t nTopIndex = nPos / m_nElementHeight;
	size_t nNewPos = 0;

	if (bToTop) {
		nNewPos = CalcElementsHeight(iIndex);
		if (m_eDirection == ListDirection::kListVertical) {
			if (nNewPos >= (size_t)GetScrollRange().cy) {
				return;
			}
		}
		else {
			if (nNewPos >= (size_t)GetScrollRange().cx) {
				return;
			}
		}
	}
	else {
		if (IsElementDisplay(iIndex)) {
			return;
		}

		if (iIndex > nTopIndex) {
			// 向下
			size_t length = CalcElementsHeight(iIndex + 1);
			nNewPos = length - ((m_eDirection == ListDirection::kListVertical) ? GetRect().GetHeight() : GetRect().GetWidth());
		}
		else {
			// 向上
			nNewPos = CalcElementsHeight(iIndex);
		}
	}
	UiSize sz;
	if (m_eDirection == ListDirection::kListVertical) {
		sz = UiSize(0, (int)nNewPos);
	}
	else {
		sz = UiSize((int)nNewPos, 0);
	}
	SetScrollPos(sz);
}

void VirtualListBox::ReArrangeChild(bool bForce)
{
	ScrollDirection direction = kScrollUp;
	if (!bForce && !m_bForceArrange) {
		if (!NeedReArrange(direction))
			return;
	}

	size_t nElementCount = GetElementCount();

	size_t nTopIndexBottom = 0;
	size_t nTopIndex = GetTopElementIndex(nTopIndexBottom);

	if (direction == kScrollDown) {
		// 向下滚动
		ui::UiRect rcItem = GetRect();
		if (m_eDirection == ListDirection::kListVertical) {
			rcItem.bottom = rcItem.top + static_cast<LONG>(nTopIndexBottom);
		}
		else {
			rcItem.right = rcItem.left + static_cast<LONG>(nTopIndexBottom);
		}

		for (size_t i = 0; i < m_items.size(); ++i) {
			if (m_eDirection == ListDirection::kListVertical) {
				rcItem.top = rcItem.bottom;
				rcItem.bottom = rcItem.top + static_cast<LONG>(m_nElementHeight);
			}
			else {
				rcItem.left = rcItem.right;
				rcItem.right = rcItem.left + static_cast<LONG>(m_nElementHeight);
			}

			m_items[i]->SetPos(rcItem);

			size_t nElementIndex = nTopIndex + i;
			if (nElementIndex < nElementCount) {
				FillElement(m_items[i], nElementIndex);
			}
		}
	}
	else {
		// 向上滚动
		const size_t nDisplayCount = ((m_eDirection == ListDirection::kListVertical) ? GetRect().GetHeight() : GetRect().GetWidth()) / m_nElementHeight + 1;
		if (m_items.size() < nDisplayCount) {
			return;
		}
		const size_t nHideCount = m_items.size() - nDisplayCount;

		// 上半部分
		UiRect rcItem = GetRect();
		if (m_eDirection == ListDirection::kListVertical) {
			rcItem.top = GetRect().top + static_cast<LONG>(nTopIndexBottom);
		}
		else {
			rcItem.left = rcItem.left + static_cast<LONG>(nTopIndexBottom);
		}
		for (int i = (int)nHideCount - 1; i >= 0; --i) {
			if (m_eDirection == ListDirection::kListVertical) {
				rcItem.bottom = rcItem.top;
				rcItem.top = rcItem.bottom - static_cast<LONG>(m_nElementHeight);
			}
			else {
				rcItem.right = rcItem.left;
				rcItem.left = rcItem.right - static_cast<LONG>(m_nElementHeight);
			}

			m_items[i]->SetPos(rcItem);

			if (nTopIndex >= (nHideCount - i)) {
				size_t nElementIndex = nTopIndex - (nHideCount - i);
				FillElement(m_items[i], nElementIndex);
			}
		}

		// 下半部分
		rcItem = GetRect();
		if (m_eDirection == ListDirection::kListVertical) {
			rcItem.bottom = GetRect().top + static_cast<LONG>(nTopIndexBottom);
		}
		else {
			rcItem.right = rcItem.left + static_cast<LONG>(nTopIndexBottom);
		}
		for (size_t i = nHideCount; i < m_items.size(); ++i) {
			if (m_eDirection == ListDirection::kListVertical) {
				rcItem.top = rcItem.bottom;
				rcItem.bottom = rcItem.top + static_cast<LONG>(m_nElementHeight);
			}
			else {
				rcItem.left = rcItem.right;
				rcItem.right = rcItem.left + static_cast<LONG>(m_nElementHeight);
			}

			m_items[i]->SetPos(rcItem);

			size_t nElementIndex = nTopIndex + (i - nHideCount);
			if (nElementIndex < nElementCount) {
				FillElement(m_items[i], nElementIndex);
			}
		}
	}
}

void VirtualListBox::AddElement(size_t iIndex)
{
	size_t nCount = GetElementCount();
	if (nCount <= m_nMaxItemCount) {
		Control *pControl = CreateElement();
		this->AddItemAt(pControl, iIndex);
		FillElement(pControl, iIndex);
	}
	else {
		ASSERT(FALSE);
		ReArrangeChild(true);
		Arrange();
	}
}

void VirtualListBox::RemoveElement(size_t iIndex)
{
	this->RemoveItemAt(iIndex);
}

void VirtualListBox::SetScrollPos(ui::UiSize szPos)
{
	m_nOldScrollPos = (m_eDirection == ListDirection::kListVertical) ? (size_t)GetScrollPos().cy : (size_t)GetScrollPos().cx;
	ListBox::SetScrollPos(szPos);

	if (UseDefaultLayout()) {
		return;
	}
	ReArrangeChild(false);
}

void VirtualListBox::HandleEvent(const ui::EventArgs& event)
{
	if (!IsMouseEnabled() && event.Type > ui::kEventMouseBegin && event.Type < ui::kEventMouseEnd) {
		if (GetParent() != nullptr) {
			GetParent()->SendEvent(event);
		}
		else {
			ui::ScrollBox::HandleEvent(event);
		}
		return;
	}

	switch (event.Type) {
	case kEventKeyDown: {
		switch (event.chKey) {
		case VK_UP: {
			OnKeyDown(VK_UP);
			return;
		}
		case VK_DOWN: {
			OnKeyDown(VK_DOWN);
			return;
		}
		case VK_HOME:
			if (m_eDirection == ListDirection::kListVertical) {
				SetScrollPosY(0);
			}
			else {
				SetScrollPosX(0);
			}
			return;
		case VK_END: {
			if (m_eDirection == ListDirection::kListVertical) {
				SetScrollPosY(GetScrollRange().cy);
			}
			else {
				SetScrollPosX(GetScrollRange().cx);
			}
			
			return;
		}
		}
	}
	case kEventKeyUp: {
		switch (event.chKey) {
		case VK_UP: {
			OnKeyUp(VK_UP);
			return;
		}
		case VK_DOWN: {
			OnKeyUp(VK_DOWN);
			return;
		}
		}
	}
	}

	__super::HandleEvent(event);
}

void VirtualListBox::SetPos(UiRect rc)
{
	bool bChange = false;
	if (!GetRect().Equal(rc)) {
		bChange = true;
	}

	ListBox::SetPos(rc);

	if (bChange) {
		if (UseDefaultLayout()) {
			return;
		}
		ReArrangeChild(true);
	}
}

void VirtualListBox::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == L"vertical") {
		SetDirection(strValue == L"true" ? ListDirection::kListVertical : ListDirection::kListHorizontal);
	}
	else {
		__super::SetAttribute(strName, strValue);
	}
}

ui::Control* VirtualListBox::CreateElement()
{
	if (m_pDataProvider != nullptr) {
		return m_pDataProvider->CreateElement();
	}
	return nullptr;
}

void VirtualListBox::FillElement(Control *pControl, size_t iIndex)
{
	if (m_pDataProvider != nullptr) {
		m_pDataProvider->FillElement(pControl, iIndex);
	}
}

size_t VirtualListBox::GetElementCount()
{
	if (m_pDataProvider != nullptr) {
		return m_pDataProvider->GetElementCount();
	}
	return 0;
}

bool VirtualListBox::UseDefaultLayout()
{
	return GetElementCount() <= GetItemCount();
}

size_t VirtualListBox::CalcElementsHeight(size_t nCount)
{
	if (!Box::IsValidItemIndex(nCount)) {
		nCount = GetElementCount();
	}
	return nCount * m_nElementHeight;
}

size_t VirtualListBox::GetTopElementIndex(size_t& bottom)
{
	size_t nPos = (m_eDirection == ListDirection::kListVertical) ? (size_t)GetScrollPos().cy : (size_t)GetScrollPos().cx;
	size_t iIndex = nPos / m_nElementHeight;
	bottom = static_cast<int>(iIndex * m_nElementHeight);
	return iIndex;
}

bool VirtualListBox::IsElementDisplay(size_t iIndex)
{
	if (!Box::IsValidItemIndex(iIndex)) {
		return false;
	}

	size_t nPos = (m_eDirection == ListDirection::kListVertical) ? (size_t)GetScrollPos().cy : (size_t)GetScrollPos().cx;
	size_t nElementPos = CalcElementsHeight(iIndex);
	if (nElementPos >= nPos) {
		size_t nLength = (m_eDirection == ListDirection::kListVertical) ? (size_t)this->GetHeight() : (size_t)this->GetWidth();
		if ((nElementPos + m_nElementHeight) <= (nPos + nLength)) {
			return true;
		}
	}
	return false;
}

bool VirtualListBox::NeedReArrange(ScrollDirection& direction)
{
	direction = kScrollUp;
	if (!m_bArrangedOnce) {
		m_bArrangedOnce = true;
		return true;
	}

	size_t nCount = GetItemCount();
	if (nCount == 0){
		return false;
	}

	if (GetElementCount() <= nCount) {
		return false;
	}

	UiRect rcThis = this->GetPos();
	if (rcThis.GetWidth() <= 0) {
		return false;
	}

	size_t nPos = (m_eDirection == ListDirection::kListVertical) ? (size_t)GetScrollPos().cy : (size_t)GetScrollPos().cx;
	UiRect rcItem;

    // 补救措施
    // 情况一：通讯录列表，一开始不可见，切换后可见，如果提前布局，
    // 则Element宽度为0，因此，必须重新布局；
    // 情况二：写信窗口联系人列表，列表宽度会不断变化，因此，需要在宽度变化后
    // 重新布局，否则，导致最终Element布局时的宽度不正确
    rcItem = m_items[0]->GetPos();
    // modified by zqw, 2016/10/12
    // 针对情况二，解决方法是，列表宽度变化 或者 拖动写信窗口右侧，列表position改变，
    // 此时，在SetPos中强制重新布局
    //if (u.GetWidth() != rect.GetWidth()) {
    //    return true;
    //}

	if (nPos >= m_nOldScrollPos) {
		// 下
		rcItem = m_items[nCount - 1]->GetPos();
		if (ListDirection::kListVertical == m_eDirection) {
			int nSub = (rcItem.bottom - rcThis.top) - ((int)nPos + rcThis.GetHeight());
			if (nSub < 0) {
				direction = kScrollDown;
				return true;
			}
		}
		else {
			int nSub = (rcItem.right - rcThis.left) - ((int)nPos + rcThis.GetWidth());
			if (nSub < 0) {
				direction = kScrollDown;
				return true;
			}
		}
	}
	else {
		// 上
		rcItem = m_items[0]->GetPos();
		if (ListDirection::kListVertical == m_eDirection) {
			if (nPos < (rcItem.top - rcThis.top)) {
				direction = kScrollUp;
				return true;
			}
		}
		else {
			if (nPos < (rcItem.left - rcThis.left)) {
				direction = kScrollUp;
				return true;
			}
		}
	}

	return false;
}

}