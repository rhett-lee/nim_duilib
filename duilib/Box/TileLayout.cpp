#include "TileLayout.h"
#include "duilib/Core/Box.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/GlobalManager.h"
#include <numeric>

namespace ui 
{
TileLayout::TileLayout(): 
	m_nColumns(0), 
	m_szItem(0, 0),
	m_bAutoCalcColumns(false)
{
}

UiSize TileLayout::CalcEstimateSize(Control* pControl, const UiSize& szItem, UiRect rc) const
{
	if ((pControl == nullptr) || !pControl->IsVisible()) {
		return UiSize();
	}

	UiSize szAvailable;
	UiMargin rcMargin = pControl->GetMargin();
	int32_t width = szItem.cx;
	if (width == 0) {
		width = rc.Width();
	}
	width = width - rcMargin.left - rcMargin.right;
	int32_t height = szItem.cy;
	if (height == 0) {
		height = rc.Height();
	}
	height = height - rcMargin.top - rcMargin.bottom;
	szAvailable = UiSize(width, height);
	szAvailable.Validate();

	UiEstSize estSize = pControl->EstimateSize(szAvailable);
	UiSize childSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
	if (estSize.cy.IsStretch()) {
		childSize.cy = 0;
	}
	if (childSize.cy < pControl->GetMinHeight()) {
		childSize.cy = pControl->GetMinHeight();
	}
	if (childSize.cy > pControl->GetMaxHeight()) {
		childSize.cy = pControl->GetMaxHeight();
	}

	if (estSize.cx.IsStretch()) {
		childSize.cx = 0;
	}
	if (childSize.cx < pControl->GetMinWidth()) {
		childSize.cx = pControl->GetMinWidth();
	}
	if (childSize.cx > pControl->GetMaxWidth()) {
		childSize.cx = pControl->GetMaxWidth();
	}
	return childSize;
}

void TileLayout::CalcTileWidthAndColumns(const std::vector<Control*>& items, UiRect rc,
										 UiSize& szItem, int32_t& nColumns) const
{
	szItem = m_szItem;		//瓦片控件宽度和高度（设置值）
	szItem.Validate();
	nColumns = m_nColumns;  //列数（设置值）, 如果不是自动计算列数，则优先使用设置的列数
	if (nColumns < 0) {
		nColumns = 0;
	}
	if (!m_bAutoCalcColumns && (nColumns == 0) && (szItem.cx == 0)) {
		//自由布局模式
		return;
	}
	if (m_bAutoCalcColumns) {
		//如果自动计算列数，则重新计算列数
		nColumns = 0;
	}
	
	if ((szItem.cx == 0) && (nColumns > 0)) {
		//已经指定列数，根据列数计算瓦片控件的宽度
		szItem.cx = (rc.Width() - (nColumns - 1) * GetChildMarginX()) / nColumns;
		if (szItem.cx <= 0) {
			szItem.cx = rc.Width() / nColumns;
		}
	}
	if ((szItem.cx == 0) && (nColumns == 0)) {
		//需要先计算瓦片控件宽度，然后根据宽度计算列数
		int32_t maxWidth = 0;
		int64_t areaTotal = 0;
		const int64_t maxArea = (int64_t)rc.Width() * rc.Height();
		for (Control* pChild : items) {
			if ((pChild == nullptr) || !pChild->IsVisible() || pChild->IsFloat()) {
				continue;
			}
			UiMargin rcMargin = pChild->GetMargin();
			UiSize childSize = CalcEstimateSize(pChild, UiSize(0, 0), rc);
			if (childSize.cx > 0) {
				maxWidth = std::max(maxWidth, childSize.cx + rcMargin.left + rcMargin.right);
			}
			int32_t childWidth = childSize.cx;
			int32_t childHeight = childSize.cy;
			if (childWidth > 0) {
				childWidth += GetChildMarginX();
			}			
			if (childHeight > 0) {
				childHeight += GetChildMarginY();
			}
			areaTotal += ((int64_t)childWidth * childHeight);
			if (areaTotal > maxArea) {
				//按当前可视区展示的面积估算
				break;
			}
		}
		//取第一行的宽度最大值，作为每个Item的宽度
		szItem.cx = maxWidth;		
	}
	if ((nColumns == 0) && (szItem.cx > 0)) {
		//使用设置的宽度作为瓦片控件的宽度，并通过设置的宽度，计算列数
		int32_t totalWidth = rc.Width();
		while (totalWidth > 0) {
			totalWidth -= szItem.cx;
			if (nColumns != 0) {
				totalWidth -= GetChildMarginX();
			}
			if (totalWidth >= 0) {
				++nColumns;
			}
		}		
	}
	if (szItem.cx == 0) {
		//当子控件全部都是拉伸类型时，仍然得不到有效值，此时需要赋固定值
		szItem.cx = rc.Width();
		nColumns = 1;
	}
	if (nColumns < 1) {
		nColumns = 1;
	}
}

void TileLayout::CalcStretchTileSize(const std::vector<Control*>& items, UiRect rc,
								     int32_t nColumns, UiSize& szMaxItem) const 
{
	ASSERT(nColumns > 0);
	int32_t maxWidth = 0;  //最大宽度
	int32_t maxHeight = 0; //最大高度
	const int64_t maxArea = (int64_t)rc.Width() * rc.Height();
	int64_t calcArea = 0;
	for (Control* pChild : items) {
		if ((pChild == nullptr) || !pChild->IsVisible() || pChild->IsFloat()) {
			continue;
		}
		UiMargin rcMargin = pChild->GetMargin();
		UiSize childSize = CalcEstimateSize(pChild, UiSize(0, 0), rc);
		if (childSize.cx > 0) {
			maxWidth = std::max(maxWidth, childSize.cx + rcMargin.left + rcMargin.right);
		}
		if (childSize.cy > 0) {
			maxHeight = std::max(maxHeight, childSize.cy + rcMargin.top + rcMargin.bottom);
		}

		int32_t childWidth = childSize.cx;
		if (childWidth > 0) {
			childWidth += GetChildMarginX();
		}
		
		int32_t childHeight = childSize.cy;
		if (childHeight > 0) {
			childHeight += GetChildMarginY();
		}
		calcArea += ((int64_t)childWidth * childHeight);
		if (calcArea > maxArea) {
			//当前显示区已满，不再测算
			break;
		}
	}
	//取第一行的宽度最大值，作为每个Item的宽度
	szMaxItem.cx = maxWidth;
	szMaxItem.cy = maxHeight;
	if ((nColumns > 0) && (szMaxItem.cx == 0)) {
		szMaxItem.cx = (rc.Width() - (nColumns - 1) * GetChildMarginX()) / nColumns;
		if (szMaxItem.cx <= 0) {
			szMaxItem.cx = rc.Width() / nColumns;
		}
	}
	if ((szMaxItem.cx > 0) && (szMaxItem.cy == 0)) {
		szMaxItem.cy = szMaxItem.cx;
	}
}

UiSize64 TileLayout::EstimateFloatSize(Control* pControl, UiRect rc) const
{
	ASSERT(pControl != nullptr);
	if ((pControl == nullptr) || !pControl->IsVisible()) {
		return UiSize64();
	}
	UiMargin margin = pControl->GetMargin();
	UiSize childSize = CalcEstimateSize(pControl, UiSize(0, 0), rc);
	if (childSize.cx > 0) {
		childSize.cx += (margin.left + margin.right);
	}
	if (childSize.cy > 0) {
		childSize.cy += (margin.top + margin.bottom);
	}	
	return UiSize64(childSize.cx, childSize.cy);
}

int32_t TileLayout::CalcTileLineHeight(const std::vector<Control*>& items,
									   const std::vector<Control*>::const_iterator iterBegin,
									   int32_t nColumns,
									   const UiSize& szItem,
									   const UiRect& rc) const
{
	ASSERT((szItem.cx > 0) && (nColumns > 0));
	int32_t cyHeight = szItem.cy;
	if (cyHeight > 0) {
		//如果设置了高度，则优先使用设置的高度值
		return cyHeight;
	}
	if (nColumns == 0) {
		return 0;
	}

	int32_t iIndex = 0;
	for (auto it = iterBegin; it != items.end(); ++it) {
		auto pChild = *it;
		if ((pChild == nullptr) || !pChild->IsVisible() || pChild->IsFloat()) {
			continue;
		}

		UiMargin rcMargin = pChild->GetMargin();
		UiSize szTile = CalcEstimateSize(pChild, szItem, rc);

		//保留高度最大值
		if (szTile.cy > 0) {
			cyHeight = std::max(cyHeight, szTile.cy + rcMargin.top + rcMargin.bottom);
		}

		++iIndex;
		if ((iIndex % nColumns) == 0) {
			//换行，退出
			break;
		}
	}
	return cyHeight;
}

UiSize TileLayout::CalcTilePosition(Control* pControl, 
	                                int32_t itemWidth, int32_t itemHeight,
							        const UiSize& szStretchItem, const UiRect& rcContainer,
	                                const UiPoint& ptTile, UiRect& szTilePos) const
{
	szTilePos.Clear();
	ASSERT(pControl != nullptr);
	if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
		return UiSize();
	}
	//目标区域大小（宽和高）
	UiSize szItem(itemWidth, itemHeight);
	szItem.Validate();

	//瓦片控件大小(宽和高)
	UiSize childSize = CalcEstimateSize(pControl, szItem, rcContainer);
	if (childSize.cx == 0) {
		childSize.cx = szStretchItem.cx;
	}
	if (childSize.cy == 0) {
		childSize.cy = szStretchItem.cy;
	}
	childSize.Validate();

	if ((szItem.cx == 0) && (childSize.cx > 0)) {
		szItem.cx = childSize.cx;
	}
	if ((childSize.cx == 0) && (szItem.cx > 0)) {
		childSize.cx = szItem.cx;
	}

	if ((szItem.cy == 0) && (childSize.cy > 0)) {
		szItem.cy = childSize.cy;
	}
	if ((childSize.cy == 0) && (szItem.cy > 0)) {
		childSize.cy = szItem.cy;
	}
	
	int32_t cxWidth = szItem.cx;	//每个控件（瓦片）的宽度（动态计算值）
	int32_t cyHeight = szItem.cy;	//每个控件（瓦片）的高度（动态计算值）

	//目标区域矩（左上角坐标，宽和高）
	UiRect rcTile(ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y + cyHeight);
	szTilePos = GetFloatPos(pControl, rcTile, childSize);
	return UiSize(cxWidth, cyHeight);
}

UiSize64 TileLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	//总体布局策略：
	// (1) 横向尽量不超出边界（除非行首的第一个元素大小比rc宽，这种情况下横向会超出边界），
	//     纵向可能会出现超出边界。
	// (2) 瓦片的宽高比：默认保持。
	//     如果宽度或者高度出现了缩放，需要保持宽高比，避免出现变形；
	//     但会提供一个选项，不保持宽高比，这种情况下，会有变形现象。
	// (3) 对于不是自由模式的情况，如果m_bAutoCalcColumns为true，则m_nColumns被置零
	//布局的几种用例:
	// (1) !m_bAutoCalcColumns && (m_nColumns == 0) && (m_szItem.cx == 0)
	//     布局策略：	1、列数：自由布局，不分列，每行是要输出到边界，就换行（每行的列数可能都不同）；
	//				2、瓦片控件的宽度：按其实际宽度展示；
	//              3、瓦片控件的高度：
	//                （1）如果m_szItem.cy > 0：限制为固定m_szItem.cy
	//                （2）如果m_szItem.cy == 0: 按其实际宽度展示；
	// (2) (m_nColumns == 0) && (m_szItem.cx > 0)
	//     布局策略：	1、列数：按照 rc.Width() 与 m_szItem.cx 来计算应该分几列，列数固定；
	//				2、瓦片控件的宽度：固定为 m_szItem.cx；
	//              3、瓦片控件的高度：
	//                （1）如果m_szItem.cy > 0：限制为固定m_szItem.cy
	//                （2）如果m_szItem.cy == 0: 按其实际宽度展示；
	// (3) (m_nColumns > 0) && (m_szItem.cx == 0)
	//     布局策略：	1、列数：列数固定为m_nColumns；
	//				2、瓦片控件的宽度：按其实际宽度展示；
	//              3、瓦片控件的高度：
	//                （1）如果m_szItem.cy > 0：限制为固定m_szItem.cy
	//                （2）如果m_szItem.cy == 0: 按其实际宽度展示；
	// (4) (m_nColumns > 0) && (m_szItem.cx > 0)
	//     布局策略：	1、列数：列数固定为m_nColumns；
	//				2、瓦片控件的宽度：固定为 m_szItem.cx；
	//              3、瓦片控件的高度：
	//                （1）如果m_szItem.cy > 0：限制为固定m_szItem.cy
	//                （2）如果m_szItem.cy == 0: 按其实际宽度展示；

	std::vector<int32_t> inColumnWidths;
	std::vector<int32_t> outColumnWidths;
	ArrangeChildInternal(items, rc, true, inColumnWidths, outColumnWidths);
	inColumnWidths.swap(outColumnWidths);
	return ArrangeChildInternal(items, rc, false, inColumnWidths, outColumnWidths);
}

UiSize64 TileLayout::ArrangeChildInternal(const std::vector<Control*>& items,
										  UiRect rect,
										  bool isCalcOnly,
										  const std::vector<int32_t>& inColumnWidths,
										  std::vector<int32_t>& outColumnWidths) const
{
	UiRect orgRect = rect;
	DeflatePadding(rect);           //剪去内边距，剩下的是可用区域
	const UiRect& rc = rect;

	UiSize szItem;					//瓦片控件宽度和高度（设置值）
	int32_t nColumns = 0;			//列数（设置值）
	CalcTileWidthAndColumns(items, rc, szItem, nColumns);
	if ((nColumns == 0) && (szItem.cx == 0)) {
		//使用自由布局排列控件(无固定列数，尽量充分利用展示空间，显示尽可能多的内容)
		return ArrangeChildFreeLayout(items, orgRect, isCalcOnly);
	}
	ASSERT(nColumns > 0);
	if (nColumns <= 0) {
		return UiSize64();
	}
	
	//计算显示区内最大的宽高, 用于作为拉伸类型的控件宽度和高度
	UiSize szStretchItem;
	CalcStretchTileSize(items, rc, nColumns, szStretchItem);

#ifdef _DEBUG
	{
		int32_t cx = szItem.cx;
		int32_t cy = szItem.cy;
		int32_t col = nColumns;

		int32_t cx_s = szStretchItem.cx;
		int32_t cy_s = szStretchItem.cy;

		int32_t cx_config = m_szItem.cx;
		int32_t cy_config = m_szItem.cy;
		int32_t col_config = m_nColumns;
		bool autoCalc = m_bAutoCalcColumns;

		int32_t ii = 0;
	}
#endif

	int64_t cxNeededFloat = 0;	//浮动控件需要的总宽度	
	int64_t cyNeededFloat = 0;	//浮动控件需要的总高度	
	int32_t cyLineHeight = 0;   //每行控件（瓦片）的高度（动态计算值）

	std::vector<int32_t> rowHeights;   //每列的宽度值
	std::vector<int32_t> columnWidths; //每行的高度值
	columnWidths.resize(nColumns);
	rowHeights.resize(1);

	int32_t nRowTileCount = 0;  //本行容纳的瓦片控件个数
	int32_t nRowIndex = 0;      //当前的行号

	UiPoint ptTile(rc.left, rc.top);	//每个控件（瓦片）的顶点坐标	
	for( auto it = items.begin(); it != items.end(); ++it ) {
		auto pControl = *it;
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if( pControl->IsFloat() ) {
			//浮动控件
			UiSize64 floatSize;
			if (!isCalcOnly) {
				//设置浮动控件的位置
				floatSize = SetFloatPos(pControl, rc);
			}
			else {
				//计算Float控件的大小
				floatSize = EstimateFloatSize(pControl, rc);				
			}
			if (cxNeededFloat < floatSize.cx) {
				cxNeededFloat = floatSize.cx;
			}
			if (cyNeededFloat < floatSize.cy) {
				cyNeededFloat = floatSize.cy;
			}
			continue;
		}
		
		if (nRowTileCount == 0) {
			//一行的开始，计算这一行的高度
			nRowTileCount = nColumns;
			cyLineHeight = CalcTileLineHeight(items, it, nColumns, szItem, rc);
			if (cyLineHeight == 0) {
				//如果本行全部都是拉伸类型的子控件，赋值
				cyLineHeight = szStretchItem.cy;
			}
			ASSERT(cyLineHeight > 0);
		}
		
		const int32_t colIndex = nColumns - nRowTileCount;//当前列下标[0, nColumns)
		int32_t columnWidth = 0; //当前传入的列宽度
		if (((int32_t)inColumnWidths.size() == nColumns) &&
			(colIndex < inColumnWidths.size())) {
			columnWidth = inColumnWidths[colIndex];
		}

		//计算当前瓦片控件的位置坐标、宽度(cxWidth)和高度(cyHeight)
		int32_t itemWidth = szItem.cx;
		if (columnWidth > 0) {
			itemWidth = columnWidth;
		}
		UiRect rcTilePos;
		UiSize szTileSize = CalcTilePosition(pControl, itemWidth, cyLineHeight, szStretchItem, rc, ptTile, rcTilePos);
		UiMargin rcMargin = pControl->GetMargin();
		int32_t cxWidth = szTileSize.cx + rcMargin.left + rcMargin.right;
		if (!isCalcOnly) {
			pControl->SetPos(rcTilePos);
		}

		//计算本行高度最大值，高度值需要包含外边距
		int32_t tileHeight = rcTilePos.Height() + rcMargin.top + rcMargin.bottom;
		rowHeights[nRowIndex] = std::max(tileHeight, rowHeights[nRowIndex]);

		--nRowTileCount;
		if(nRowTileCount == 0 ) {
			//换行
			rowHeights.push_back(0);
			nRowIndex = (int32_t)rowHeights.size() - 1;			
			
			//重新设置X坐标和Y坐标的位置
			ptTile.x = rc.left;
			//Y轴坐标切换到下一行，按行高切换
			ptTile.y += cyLineHeight + GetChildMarginY();
		}
		else {
			//同一行，向右切换坐标，按当前瓦片控件的宽度切换
			if (columnWidth > 0) {
				ptTile.x += columnWidth + GetChildMarginX();
			}
			else {
				ptTile.x += cxWidth + GetChildMarginX();
			}
		}		
		//记录每列的宽度（取这一列中，控件宽度的最大值，包含此控件的外边距）
		if (colIndex < columnWidths.size()) {
			int32_t tileWidth = rcTilePos.Width() + rcMargin.left + rcMargin.right;
			tileWidth = std::min(tileWidth, szItem.cx);
			columnWidths[colIndex] = std::max(tileWidth, columnWidths[colIndex]);
		}
	}

	//由于内边距已经剪掉，计算宽度和高度的时候，需要算上内边距
	const UiPadding& padding = GetPadding();

	//计算所需宽度
	int64_t cxNeeded = std::accumulate(columnWidths.begin(), columnWidths.end(), 0);
	if (columnWidths.size() > 1) {
		cxNeeded += (columnWidths.size() - 1) * GetChildMarginX();
	}
	cxNeeded = std::max(cxNeeded, cxNeededFloat);
	cxNeeded += (padding.left + padding.right);

	//计算所需高度
	int64_t cyNeeded = std::accumulate(rowHeights.begin(), rowHeights.end(), 0);
	if (rowHeights.size() > 1) {
		cyNeeded += (rowHeights.size() - 1) * GetChildMarginY();
	}
	cyNeeded = std::max(cyNeeded, cyNeededFloat);
	cyNeeded += (padding.top + padding.bottom);	

	outColumnWidths.swap(columnWidths);
	UiSize64 size(cxNeeded, cyNeeded);
	return size;
}

UiSize64 TileLayout::ArrangeChildFreeLayout(const std::vector<Control*>& items, UiRect rc, bool isCalcOnly) const
{
	DeflatePadding(rc);				//剪去内边距，剩下的是可用区域
	return UiSize64();
}

int32_t TileLayout::CalcLineHeightAndTileCount(const std::vector<Control*>& items,
	const std::vector<Control*>::const_iterator iterBegin,
	int32_t nColumns,
	const UiSize& szItem,
	const UiSize& szStretchItem,
	const UiRect& rc,
	int32_t& nLineTileCount) const
{
	return 0;
	//nLineTileCount = 0;
	////如果 (nColumns == 0) && (szItem.cx == 0)，则使用自由布局排列控件
	//bool isValid = ((nColumns == 0) && (szItem.cx == 0)) || ((nColumns > 0) && (szItem.cx > 0));
	//ASSERT(isValid);
	//if (!isValid) {
	//	return 0;
	//}

	//int32_t cyHeight = szItem.cy;
	//if ((cyHeight > 0) && (nColumns > 0)) {
	//	//如果设置了高度和列数，则优先使用设置的值
	//	nLineTileCount = nColumns;
	//	return cyHeight;
	//}

	//int32_t stretchWidth = szItem.cx; //拉伸类型的控件的宽度
	//if (stretchWidth == 0) {
	//	stretchWidth = szStretchItem.cx;
	//}

	//int32_t lineLeftWidth = rc.Width(); //本行剩余可用宽度
	//int32_t iIndex = 0; //控件编号
	//bool bLineFirstTile = true; //当前是不是行首的控件
	//for (auto it = iterBegin; it != items.end(); ++it) {
	//	auto pChild = *it;
	//	if ((pChild == nullptr) || !pChild->IsVisible() || pChild->IsFloat()) {
	//		continue;
	//	}
	//	UiMargin rcMargin = pChild->GetMargin();
	//	UiSize szAvailable(lineLeftWidth - rcMargin.left - rcMargin.right,
	//		rc.Height() - rcMargin.top - rcMargin.bottom);

	//	if (iIndex == 0) {
	//		bLineFirstTile = true;
	//	}
	//	else if ((nColumns > 0) && ((iIndex + 1) % nColumns == 0)) {
	//		bLineFirstTile = true;
	//	}
	//	else if (nColumns == 0) {
	//		//
	//	}
	//	else {
	//		bLineFirstTile = false;
	//	}
	//	if (!bLineFirstTile) {
	//		//行中间的子控件
	//		szAvailable.cx -= GetChildMarginX();
	//	}

	//	UiEstSize estSize = pChild->EstimateSize(szAvailable);
	//	UiSize szTile(estSize.cx.GetInt32(), estSize.cy.GetInt32());
	//	if (estSize.cy.IsStretch()) {
	//		//该控件是拉伸类型的控件
	//		szTile.cy = 0;
	//	}
	//	if (szTile.cy < pChild->GetMinHeight()) {
	//		szTile.cy = pChild->GetMinHeight();
	//	}
	//	if (szTile.cy > pChild->GetMaxHeight()) {
	//		szTile.cy = pChild->GetMaxHeight();
	//	}
	//	//保留高度最大值
	//	cyHeight = std::max(cyHeight, szTile.cy + rcMargin.top + rcMargin.bottom);

	//	++iIndex;
	//	if ((nColumns > 0) && (iIndex % nColumns) == 0) {
	//		//换行，退出
	//		break;
	//	}
	//	else {
	//		//
	//	}
	//}
	//return cyHeight;
}

UiSize TileLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	szAvailable.Validate();
	UiRect rc(0, 0, szAvailable.Width(), szAvailable.Height());
	std::vector<int32_t> inColumnWidths;
	std::vector<int32_t> outColumnWidths;
	UiSize64 requiredSize = ArrangeChildInternal(items, rc, true, inColumnWidths, outColumnWidths);
	UiSize size(TruncateToInt32(requiredSize.cx), TruncateToInt32(requiredSize.cy));
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
		if (strValue == L"auto") {
			//自动计算列数
			m_bAutoCalcColumns = true;
		}
		else {
			m_bAutoCalcColumns = false;
			SetColumns(_wtoi(strValue.c_str()));
		}
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
