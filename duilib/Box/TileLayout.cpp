#include "TileLayout.h"
#include "duilib/Core/Box.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui 
{
TileLayout::TileLayout(): m_nColumns(0), m_szItem(0, 0)
{
}

void TileLayout::CalcTileWidthAndColumns(const std::vector<Control*>& items, UiSize szAvailable,
							             int32_t& itemWidth, int32_t& columns) const
{
	szAvailable.Validate();
	columns = 0;
	//优先使用设置的宽度值
	itemWidth = m_szItem.cx;
	if (m_nColumns > 0) {
		//优先使用使用设置的列数
		columns = m_nColumns;
	}	
	if (itemWidth <= 0) {
		//如果没有设置宽度，则计算宽度
		int32_t maxWidth = 0;
		int32_t widthTotal = 0;
		size_t count = 0;
		for (Control* pChild : items) {
			if ((pChild == nullptr) || !pChild->IsVisible() || pChild->IsFloat()) {
				continue;
			}
			UiRect rcMargin = pChild->GetMargin();
			UiSize szAvailableLeft(szAvailable.cx - widthTotal, szAvailable.cy);
			UiSize childSize = pChild->EstimateSize(szAvailable);			
			if (childSize.cx == DUI_LENGTH_STRETCH) {
				//子控件的宽度为拉伸的情况
				childSize.cx = 0;
			}
			if (childSize.cx < pChild->GetMinWidth()) {
				childSize.cx = pChild->GetMinWidth();
			}
			if (childSize.cx > pChild->GetMaxWidth()) {
				childSize.cx = pChild->GetMaxWidth();
			}

			int32_t childWidth = 0;
			if (childSize.cx > 0) {
				childWidth += childSize.cx;
			}
			if (count == 0) {
				childWidth += GetChildMarginX() / 2;
			}
			else {
				childWidth += GetChildMarginX();
			}
			if (columns <= 0) {
				//如果列数未确定，则按当前区域总宽度估算
				if ((widthTotal + childWidth) > szAvailable.Width()) {
					break;
				}
			}
			else {
				//列数已经确定，则按该列中的各个控件估算
				if (count >= columns) {
					break;
				}
			}
			widthTotal += childWidth;
			maxWidth = std::max(maxWidth, childSize.cx + rcMargin.left + rcMargin.right);
			++count;
		}		
		//取第一行的宽度最大值，作为每个Item的宽度
		itemWidth = maxWidth;
	}

	if ((columns <= 0) && (itemWidth > 0)) {
		//计算列数
		columns = 0;
		int32_t totalWidth = szAvailable.Width();
		while (totalWidth > 0) {
			totalWidth -= itemWidth;
			if (columns != 0) {
				totalWidth -= GetChildMarginX();
			}
			if (totalWidth >= 0) {
				++columns;
			}
		}
	}
	if (columns <= 0) {
		//无法计算列数，使用最小值
		columns = 1;
	}
	if (itemWidth <= 0) {
		itemWidth = (szAvailable.Width() - (columns - 1) * GetChildMarginX()) / columns;
	}
}

int32_t TileLayout::CalcTileHeight(const std::vector<Control*>& items,
								   const std::vector<Control*>::const_iterator iterBegin,
								   int32_t nColumns,
								   int32_t tileWidth,
								   int32_t tileBoxHeight)
{
	//一行的开始，计算这一行的高度，取这一行子控件中高度最大值
	int32_t cyHeight = m_szItem.cy;
	if (cyHeight > 0) {
		//如果设置了高度，则优先使用设置的高度值
		return cyHeight;
	}

	int32_t iIndex = 0;
	for (auto it = iterBegin; it != items.end(); ++it) {
		auto pChild = *it;
		if ((pChild == nullptr) || !pChild->IsVisible() || pChild->IsFloat()) {
			continue;
		}

		UiRect rcMargin = pChild->GetMargin();
		UiSize szAvailable(tileWidth - rcMargin.left - rcMargin.right,
			               tileBoxHeight - rcMargin.top - rcMargin.bottom);
		if (iIndex == 0 || (iIndex + 1) % nColumns == 0) {
			//行首的子控件或者是行尾的子控件
			szAvailable.cx -= GetChildMarginX() / 2;
		}
		else {
			//这行中间的子控件
			szAvailable.cx -= GetChildMarginX();
		}

		UiSize szTile = pChild->EstimateSize(szAvailable);
		if (szTile.cy == DUI_LENGTH_STRETCH) {
			szTile.cy = szAvailable.cy;
		}
		if (szTile.cy < pChild->GetMinHeight()) {
			szTile.cy = pChild->GetMinHeight();
		}
		if (szTile.cy > pChild->GetMaxHeight()) {
			szTile.cy = pChild->GetMaxHeight();
		}
		//保留高度最大值
		cyHeight = std::max(cyHeight, szTile.cy + rcMargin.top + rcMargin.bottom);

		++iIndex;
		if ((iIndex % nColumns) == 0) {
			//换行，退出
			break;
		}
	}
	return cyHeight;
}

UiSize64 TileLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	UiSize szRectSize(rc.Width(), rc.Height());
	int32_t itemWidth = 0;
	int32_t nColumns = 0;
	CalcTileWidthAndColumns(items, szRectSize, itemWidth, nColumns);
	ASSERT((nColumns > 0) && (itemWidth > 0));
	if ((nColumns <= 0) || (itemWidth <= 0)) {
		return UiSize64();
	}

	//需要的总高度
	int64_t cyNeeded = 0;
	//每个控件（瓦片）的高度
	int32_t cyHeight = 0;
	//每个控件（瓦片）的宽度, 固定值
	const int32_t cxWidth = itemWidth;
	//每个控件（瓦片）的顶点坐标
	UiPoint ptTile(rc.left, rc.top);
	//行首的X坐标值
	const int32_t iLinePosX = rc.left;
	//计数值
	int32_t iCount = 0;
	for( auto it = items.begin(); it != items.end(); ++it ) {
		auto pControl = *it;
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if( pControl->IsFloat() ) {
			SetFloatPos(pControl, rc);
			continue;
		}
		
		if ((iCount % nColumns) == 0) {
			//一行的开始，计算这一行的高度
			cyHeight = CalcTileHeight(items, it, nColumns, cxWidth, rc.Height());
		}

		//当前控件（瓦片）的矩形区域
		UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y + cyHeight);
		UiRect rcMargin = pControl->GetMargin();
		rcTile.Deflate(rcMargin.left, rcMargin.top, rcMargin.right, rcMargin.bottom);

		UiSize szAvailable(rcTile.Width(), rcTile.Height());
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

		//如果瓦片的大小不匹配，则按居中对齐显示
		int32_t tileLeft = (rcTile.left + rcTile.right - szTile.cx) / 2;
		int32_t tileTop = (rcTile.top + rcTile.bottom - szTile.cy) / 2;

		UiRect rcPos(tileLeft, tileTop,	tileLeft + szTile.cx, tileTop + szTile.cy);
		pControl->SetPos(rcPos);
			
		//pControl->SetPos(rcTile);

		++iCount;
		if( (iCount % nColumns) == 0 ) {
			//换行，重新设置坐标
			ptTile.x = iLinePosX;
			ptTile.y += cyHeight + GetChildMarginY();
		}
		else {
			//同一行，向右切换坐标
			ptTile.x += cxWidth + GetChildMarginX();
		}
		cyNeeded = rcTile.bottom - rc.top;
	}

	UiSize64 size(rc.Width(), cyNeeded);
	return size;
}

UiSize TileLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	szAvailable.Validate();
	UiSize size;
	if (m_pOwner != nullptr) {
		//父控件容器自身的大小（不含容器中的子控件）
		size = m_pOwner->Control::EstimateSize(szAvailable);
		if (!items.empty()) {
			Control* pChild = items.front();
			if (pChild != nullptr) {
				//第一个子控件的大小
				UiSize childSize = pChild->EstimateSize(szAvailable);
				//最终结果使用
				size.cx = std::max(size.cx, childSize.cx);
			}
		}
	}
	size.cy = 0;

	if (m_szItem.cx > 0) {
		m_nColumns = m_pOwner->GetFixedWidth() / m_szItem.cx;
	}
	int32_t nColumns = m_nColumns;
	if (nColumns < 1) {
		nColumns = 1;
	}
	int32_t visibleCount = (int32_t)items.size();
	for (auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			visibleCount--;
		}
	}
	int32_t rows = visibleCount / nColumns;
	if (visibleCount % nColumns != 0) {
		rows += 1;
	}
	if (visibleCount > 0) {
		int32_t childMarginTotal = 0;
		if (visibleCount % nColumns == 0) {
			childMarginTotal = (visibleCount / nColumns - 1) * GetChildMarginY();
		}
		else {
			childMarginTotal = (visibleCount / nColumns) * GetChildMarginY();
		}
		Control* pChild = nullptr;
		if (!items.empty()) {
			pChild = items.front();
		}
		int32_t nChildHeight = 0;
		if (pChild != nullptr) {
			//第一个子控件的大小
			UiSize childSize = pChild->EstimateSize(szAvailable);
			nChildHeight = childSize.cy;
		}
		if (nChildHeight < 0) {
			nChildHeight = 0;
		}
		UiRect rcPadding = GetPadding();
		size.cy += nChildHeight * rows + rcPadding.top + rcPadding.bottom + childMarginTotal;
	}
	return size;
}

bool TileLayout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	bool hasAttribute = true;
	if((strName == L"item_size") || (strName == L"itemsize")){
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
	szItem.cx = std::max(szItem.cx, 0);
	szItem.cy = std::max(szItem.cy, 0);
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleSize(szItem);
	}

	if( (m_szItem.cx != szItem.cx) || (m_szItem.cy != szItem.cy) ) {
		m_szItem = szItem;
		m_pOwner->Arrange();
	}
}

int32_t TileLayout::GetColumns() const
{
	return m_nColumns;
}

void TileLayout::SetColumns(int32_t nCols)
{
	nCols = std::max(nCols, 0);
	if (m_nColumns != nCols) {
		m_nColumns = nCols;
		m_pOwner->Arrange();
	}	
}

} // namespace ui
