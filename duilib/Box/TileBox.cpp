#include "TileBox.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui
{

TileLayout::TileLayout() : m_nColumns(1), m_szItem(0, 0)
{

}

UiSize TileLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	//TileLayout::ArrangeChild
	// Position the elements
	if (m_szItem.cx > 0) {
		m_nColumns = (rc.right - rc.left) / m_szItem.cx;
	}
	if (m_nColumns < 1) {
		m_nColumns = 1;
	}

	int cyNeeded = 0;
	int cxWidth = rc.Width() / m_nColumns;
	int deviation = rc.Width() - cxWidth * m_nColumns;
	int tmpDeviation = deviation;
	int cyHeight = 0;
	int iCount = 0;
	UiPoint ptTile(rc.left, rc.top);
	int iPosX = rc.left;

	for( auto it = items.begin(); it != items.end(); ++it ) {
		auto pControl = *it;
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible()) {
			continue;
		}
		if( pControl->IsFloat() ) {
			SetFloatPos(pControl, rc);
			continue;
		}

		// Determine size
		UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y);
		if (tmpDeviation > 0) {
			rcTile.right += 1;
			tmpDeviation--;
		}
		if( (iCount % m_nColumns) == 0) {
			int iIndex = iCount;
			for (auto it2 = it; it2 != items.end(); ++it2) {
				auto pLineControl = *it2;
				if (pLineControl == nullptr) {
					continue;
				}
				if (!pLineControl->IsVisible()) {
					continue;
				}
				if (pLineControl->IsFloat()) {
					continue;
				}

				UiRect rcMargin = pLineControl->GetMargin();
				UiSize szAvailable(rcTile.right - rcTile.left - rcMargin.left - rcMargin.right, 9999);
				if( iIndex == iCount || (iIndex + 1) % m_nColumns == 0 ) {
					szAvailable.cx -= m_iChildMargin / 2;
				}
				else {
					szAvailable.cx -= m_iChildMargin;
				}

				if (szAvailable.cx < pControl->GetMinWidth()) {
					szAvailable.cx = pControl->GetMinWidth();
				}
				if (szAvailable.cx > pControl->GetMaxWidth()) {
					szAvailable.cx = pControl->GetMaxWidth();
				}

				UiSize szTile = pLineControl->EstimateSize(szAvailable);
				if (szTile.cx < pControl->GetMinWidth()) {
					szTile.cx = pControl->GetMinWidth();
				}
				if (szTile.cx > pControl->GetMaxWidth()) {
					szTile.cx = pControl->GetMaxWidth();
				}
				if (szTile.cy < pControl->GetMinHeight()) {
					szTile.cy = pControl->GetMinHeight();
				}
				if (szTile.cy > pControl->GetMaxHeight()) {
					szTile.cy = pControl->GetMaxHeight();
				}

				cyHeight = std::max(cyHeight, int(szTile.cy + rcMargin.top + rcMargin.bottom));
				if ((++iIndex % m_nColumns) == 0) {
					break;
				}
			}
		}

		UiRect rcMargin = pControl->GetMargin();
		UiRect newRcTile = rcTile;
		newRcTile.left += rcMargin.left + m_iChildMargin / 2;
		newRcTile.right -= rcMargin.right + m_iChildMargin / 2;
			
		// Set position
		newRcTile.top = ptTile.y + rcMargin.top;
		newRcTile.bottom = ptTile.y + cyHeight;

		UiSize szAvailable(newRcTile.right - newRcTile.left, newRcTile.bottom - newRcTile.top);
		UiSize szTile = pControl->EstimateSize(szAvailable);
		if (szTile.cx == DUI_LENGTH_STRETCH) {
			szTile.cx = szAvailable.cx;
		}
		if (szTile.cy == DUI_LENGTH_STRETCH) {
			szTile.cy = szAvailable.cy;
		}
		if (szTile.cx < pControl->GetMinWidth()) {
			szTile.cx = pControl->GetMinWidth();
		}
		if (szTile.cx > pControl->GetMaxWidth()) {
			szTile.cx = pControl->GetMaxWidth();
		}
		if (szTile.cy < pControl->GetMinHeight()) {
			szTile.cy = pControl->GetMinHeight();
		}
		if (szTile.cy > pControl->GetMaxHeight()) {
			szTile.cy = pControl->GetMaxHeight();
		}
		UiRect rcPos((newRcTile.left + newRcTile.right - szTile.cx) / 2, (newRcTile.top + newRcTile.bottom - szTile.cy) / 2,
			         (newRcTile.left + newRcTile.right - szTile.cx) / 2 + szTile.cx, (newRcTile.top + newRcTile.bottom - szTile.cy) / 2 + szTile.cy);
		pControl->SetPos(rcPos);

		if( (++iCount % m_nColumns) == 0 ) {
			ptTile.x = iPosX;
			ptTile.y += cyHeight + m_iChildMargin;
			cyHeight = 0;
			tmpDeviation = deviation;
		}
		else {
			ptTile.x += rcTile.Width();
		}
		cyNeeded = newRcTile.bottom - rc.top;
	}

	UiSize size(rc.right - rc.left, cyNeeded);
	return size;
}

UiSize TileLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	UiSize size = m_pOwner->Control::EstimateSize(szAvailable);
	size.cy = 0;

	if (m_szItem.cx > 0) {
		m_nColumns = m_pOwner->GetFixedWidth() / m_szItem.cx;
	}
	if (m_nColumns < 1) {
		m_nColumns = 1;
	}
	int visibleCount = (int)items.size();
	for (auto it = items.begin(); it != items.end(); it++) {
		if (!(*it)->IsVisible()) {
			visibleCount--;
		}
	}
	int rows = visibleCount / m_nColumns;
	if (visibleCount % m_nColumns != 0) {
		rows += 1;
	}
	if (visibleCount > 0) {
		int childMarginTotal = 0;
		if (visibleCount % m_nColumns == 0) {
			childMarginTotal = (visibleCount / m_nColumns - 1) * m_iChildMargin;
		}
		else {
			childMarginTotal = (visibleCount / m_nColumns) * m_iChildMargin;
		}
		Control* pControl = (Control*)(items[0]);
		if (pControl != nullptr) {
			size.cy += pControl->GetFixedHeight() * rows + m_rcPadding.top + m_rcPadding.bottom + childMarginTotal;
		}		
	}

	return size;
}

bool TileLayout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	bool hasAttribute = true;
	if( strName == L"itemsize") {
		UiSize szItem;
		AttributeUtil::ParseSizeValue(strValue.c_str(), szItem);
		SetItemSize(szItem);
	}
	else if( strName == L"columns") {
		SetColumns(_wtoi(strValue.c_str()));
	}
	else {
		hasAttribute = Layout::SetAttribute(strName, strValue);
	}
	return hasAttribute;
}

const UiSize& TileLayout::GetItemSize() const
{
	return m_szItem;
}

void TileLayout::SetItemSize(UiSize szItem, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleSize(szItem);
	}

	if( (m_szItem.cx != szItem.cx) || (m_szItem.cy != szItem.cy) ) {
		m_szItem = szItem;
		m_pOwner->Arrange();
	}
}

int TileLayout::GetColumns() const
{
	return m_nColumns;
}

void TileLayout::SetColumns(int nCols)
{
	if (nCols <= 0) {
		return;
	}
	m_nColumns = nCols;
	m_pOwner->Arrange();
}

TileBox::TileBox() : Box(new TileLayout())
{

}

std::wstring TileBox::GetType() const {	return DUI_CTR_TILEBOX; }

} //namespace ui

