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
	m_bAutoCalcColumns(false),
	m_bScaleDown(true)
{
}

UiSize TileLayout::CalcEstimateSize(Control* pControl, const UiSize& szItem, UiRect rc)
{
	if ((pControl == nullptr) || !pControl->IsVisible()) {
		return UiSize();
	}

	UiSize szAvailable;
	UiMargin rcMargin = pControl->GetMargin();
	int32_t width = szItem.cx;
	if (width <= 0) {
		width = rc.Width();
	}
	width = width - rcMargin.left - rcMargin.right;
	int32_t height = szItem.cy;
	if (height <= 0) {
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

void TileLayout::CalcTileColumns(const std::vector<Control*>& items, const UiRect& rc,
								 int32_t itemWidth, int32_t childMarginX, int32_t childMarginY,
	                             int32_t& nColumns)
{
	nColumns = 0;
	if (itemWidth <= 0) {
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
				childWidth += childMarginX;
			}			
			if (childHeight > 0) {
				childHeight += childMarginY;
			}
			areaTotal += ((int64_t)childWidth * childHeight);
			if (areaTotal > maxArea) {
				//按当前可视区展示的面积估算
				break;
			}
		}
		//取可视区控件宽度最大值，作为每个Item的宽度
		itemWidth = maxWidth;
	}
	if (itemWidth > 0) {
		//使用设置的宽度作为瓦片控件的宽度，并通过设置的宽度，计算列数
		int32_t totalWidth = rc.Width();
		while (totalWidth > 0) {
			totalWidth -= itemWidth;
			if (nColumns != 0) {
				totalWidth -= childMarginX;
			}
			if (totalWidth >= 0) {
				++nColumns;
			}
		}
	}
}

void TileLayout::CalcStretchTileSize(const std::vector<Control*>& items, UiRect rc,
	                                 int32_t childMarginX, int32_t childMarginY,
								     int32_t nColumns, UiSize& szMaxItem)
{
	//ASSERT(nColumns > 0);
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
			childWidth += childMarginX;
		}
		
		int32_t childHeight = childSize.cy;
		if (childHeight > 0) {
			childHeight += childMarginY;
		}
		calcArea += ((int64_t)childWidth * childHeight);
		if (calcArea > maxArea) {
			//当前显示区已满，不再测算
			break;
		}
	}
	//取宽度最大值，作为每个Item的宽度
	szMaxItem.cx = maxWidth;
	szMaxItem.cy = maxHeight;
	if ((nColumns > 0) && (szMaxItem.cx == 0)) {
		szMaxItem.cx = (rc.Width() - (nColumns - 1) * childMarginX) / nColumns;
		if (szMaxItem.cx <= 0) {
			szMaxItem.cx = rc.Width() / nColumns;
		}
	}
	if ((szMaxItem.cx > 0) && (szMaxItem.cy == 0)) {
		szMaxItem.cy = szMaxItem.cx;
	}
}

UiSize64 TileLayout::EstimateFloatSize(Control* pControl, UiRect rc)
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
									   const UiRect& rc)
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
	                                const UiPoint& ptTile, bool bScaleDown, UiRect& szTilePos)
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

	//对控件进行等比例缩放
	if (bScaleDown && 
		(rcTile.Width() > 0) && (rcTile.Height() > 0) &&
		(childSize.cx > 0) && (childSize.cy > 0) ) {
		if ((childSize.cx > rcTile.Width()) || (childSize.cy > rcTile.Height())) {
			UiSize oldSize = childSize;
			double cx = childSize.cx;
			double cy = childSize.cy;
			double cxRatio = cx / rcTile.Width();
			double cyRatio = cy / rcTile.Height();
			if (cxRatio > cyRatio) {
				ASSERT(childSize.cx > rcTile.Width());
				double ratio = cx / childSize.cy;
				childSize.cx = rcTile.Width();
				childSize.cy = static_cast<int32_t>(childSize.cx / ratio + 0.5);
			}
			else {
				ASSERT(childSize.cy > rcTile.Height());
				double ratio = cy / childSize.cx;
				childSize.cy = rcTile.Height();
				childSize.cx = static_cast<int32_t>(childSize.cy / ratio + 0.5);
			}
			//double r0 = 1.0 * oldSize.cy / childSize.cy;
			//double r1 = 1.0 * oldSize.cx / childSize.cx;
			//ASSERT(childSize.cx * oldSize.cy == childSize.cy * oldSize.cx);
		}
	}

	szTilePos = GetFloatPos(pControl, rcTile, childSize);
	if (szTilePos.left < ptTile.x) {
		//如果控件较大，超过边界，则靠左对齐
		int32_t width = szTilePos.Width();
		szTilePos.left = ptTile.x;
		szTilePos.right = ptTile.x + width;
	}
	if (szTilePos.top < ptTile.y) {
		//如果控件较大，超过边界，则靠上对齐
		int32_t height = szTilePos.Height();
		szTilePos.top = ptTile.y;
		szTilePos.bottom = ptTile.y + height;
	}
	return UiSize(cxWidth, cyHeight);
}

UiSize64 TileLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	//总体布局策略：
	// (1) 横向尽量不超出边界（除非行首的第一个元素大小比rc宽，这种情况下横向会超出边界），
	//     纵向可能会出现超出边界。
	// (2) 瓦片的宽高比：默认保持宽高比进行缩小，以适应目标显示区(可用m_bScaleDown控制此行为)。
	//     如果宽度或者高度出现了缩放，需要保持宽高比，避免出现变形；
	//     但会提供一个选项SetScaleDown()，不保持宽高比，这种情况下，会有超出边界的现象。
	// (3) 对于不是自由模式的情况，如果m_bAutoCalcColumns为true，则m_nColumns被置零
	//布局的几种用例:
	// (1) !m_bAutoCalcColumns && (m_nColumns == 0) && (m_szItem.cx == 0)
	//     布局策略：	1、列数：自由布局，不分列，每行是要输出到边界，就换行（每行的列数可能都不同）；
	//				2、瓦片控件的宽度：按其实际宽度展示；
	//              3、瓦片控件的高度：
	//                （1）如果m_szItem.cy > 0：限制为固定m_szItem.cy
	//                （2）如果m_szItem.cy == 0: 按其实际高度展示；
	// (2) (m_nColumns == 0) && (m_szItem.cx > 0)
	//     布局策略：	1、列数：按照 rc.Width() 与 m_szItem.cx 来计算应该分几列，列数固定；
	//				2、瓦片控件的宽度：固定为 m_szItem.cx；
	//              3、瓦片控件的高度：
	//                （1）如果m_szItem.cy > 0：限制为固定m_szItem.cy
	//                （2）如果m_szItem.cy == 0: 按其实际高度展示；
	// (3) (m_nColumns > 0) && (m_szItem.cx == 0)
	//     布局策略：	1、列数：列数固定为m_nColumns；
	//				2、瓦片控件的宽度：按其实际宽度展示；
	//              3、瓦片控件的高度：
	//                （1）如果m_szItem.cy > 0：限制为固定m_szItem.cy
	//                （2）如果m_szItem.cy == 0: 按其实际高度展示；
	// (4) (m_nColumns > 0) && (m_szItem.cx > 0)
	//     布局策略：	1、列数：列数固定为m_nColumns；
	//				2、瓦片控件的宽度：固定为 m_szItem.cx；
	//              3、瓦片控件的高度：
	//                （1）如果m_szItem.cy > 0：限制为固定m_szItem.cy
	//                （2）如果m_szItem.cy == 0: 按其实际高度展示；

	if (IsFreeLayout()) {
		//使用自由布局排列控件(无固定列数，尽量充分利用展示空间，显示尽可能多的内容)
		return ArrangeChildFreeLayout(items, rc, false);
	}
	else {
		std::vector<int32_t> inColumnWidths;
		std::vector<int32_t> outColumnWidths;
		ArrangeChildNormal(items, rc, true, inColumnWidths, outColumnWidths);
		inColumnWidths.swap(outColumnWidths);
		return ArrangeChildNormal(items, rc, false, inColumnWidths, outColumnWidths);
	}
}

bool TileLayout::IsFreeLayout() const
{
	return (!m_bAutoCalcColumns && (m_nColumns == 0) && (m_szItem.cx == 0));
}

UiSize64 TileLayout::ArrangeChildNormal(const std::vector<Control*>& items,
										UiRect rect,
										bool isCalcOnly,
										const std::vector<int32_t>& inColumnWidths,
										std::vector<int32_t>& outColumnWidths) const
{
	ASSERT(!IsFreeLayout());
	DeflatePadding(rect);           //剪去内边距，剩下的是可用区域
	const UiRect& rc = rect;

	int32_t nColumns = m_nColumns;  //列数（设置值）
	if (m_bAutoCalcColumns) {
		//如果自动计算列数，则重新计算列数
		nColumns = 0;
	}
	if (nColumns <= 0) {
		CalcTileColumns(items, rc, m_szItem.cx, GetChildMarginX(), GetChildMarginY(), nColumns);
	}
	if (nColumns < 1) {
		//无法精确计算时，默认值设置为1
		nColumns = 1;
	}

	//列宽设置, 固定值
	std::vector<int32_t> fixedColumnWidths = inColumnWidths;
	if (m_szItem.cx > 0) {
		fixedColumnWidths.clear();
		fixedColumnWidths.resize(nColumns, m_szItem.cx);
	}
	
	//计算显示区内最大的宽高, 用于作为拉伸类型的控件宽度和高度
	UiSize szStretchItem;
	CalcStretchTileSize(items, rc, GetChildMarginX(), GetChildMarginY(), nColumns, szStretchItem);

	int64_t cxNeededFloat = 0;	//浮动控件需要的总宽度	
	int64_t cyNeededFloat = 0;	//浮动控件需要的总高度	
	int32_t cyLineHeight = 0;   //每行控件（瓦片）的高度（动态计算值）

	std::vector<int32_t> rowHeights;   //每列的高度值，计算值
	std::vector<int32_t> columnWidths; //每行的宽度值，计算值
	columnWidths.resize(nColumns);
	rowHeights.resize(1);

	int32_t nRowTileCount = 0;  //本行容纳的瓦片控件个数
	int32_t nRowIndex = 0;      //当前的行号

	int32_t xPosLeft = rc.left; //控件显示内容的左侧坐标值(横向区域居中对齐)
	if (!isCalcOnly && !fixedColumnWidths.empty()) {
		int32_t cxTotal = std::accumulate(fixedColumnWidths.begin(), fixedColumnWidths.end(), 0);
		if (fixedColumnWidths.size() > 1) {
			cxTotal += ((int32_t)fixedColumnWidths.size() - 1) * GetChildMarginX();
		}
		if (cxTotal < rc.Width()) {
			xPosLeft = rc.CenterX() - cxTotal / 2;
		}
	}

	UiPoint ptTile(xPosLeft, rc.top);	//每个控件（瓦片）的顶点坐标	
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
			cyLineHeight = CalcTileLineHeight(items, it, nColumns, m_szItem, rc);
			if (cyLineHeight == 0) {
				//如果本行全部都是拉伸类型的子控件，赋值
				cyLineHeight = szStretchItem.cy;
			}
			ASSERT(cyLineHeight > 0);
		}
		
		const int32_t colIndex = nColumns - nRowTileCount;//当前列下标[0, nColumns)
		int32_t fixedColumnWidth = 0; //当前传入的列宽度, 固定值
		if (((int32_t)fixedColumnWidths.size() == nColumns) &&
			(colIndex < fixedColumnWidths.size())) {
			fixedColumnWidth = fixedColumnWidths[colIndex];
		}

		//计算当前瓦片控件的位置坐标、宽度(cxWidth)和高度(cyHeight)
		UiRect rcTilePos;
		UiSize szTileSize = CalcTilePosition(pControl, fixedColumnWidth, cyLineHeight,
			                                 szStretchItem, rc, ptTile, m_bScaleDown, 
			                                 rcTilePos);
		
		if (!isCalcOnly) {
			pControl->SetPos(rcTilePos);
		}

		UiMargin rcMargin = pControl->GetMargin();
		int32_t cxWidth = szTileSize.cx + rcMargin.left + rcMargin.right;
		if (fixedColumnWidth > 0) {
			cxWidth = fixedColumnWidth;
		}

		//计算本行高度最大值，高度值需要包含外边距
		int32_t tileHeight = rcTilePos.Height() + rcMargin.top + rcMargin.bottom;
		rowHeights[nRowIndex] = std::max(tileHeight, rowHeights[nRowIndex]);

		--nRowTileCount;
		if(nRowTileCount == 0 ) {
			//换行
			rowHeights.push_back(0);
			nRowIndex = (int32_t)rowHeights.size() - 1;			
			
			//重新设置X坐标和Y坐标的位置(行首)
			ptTile.x = xPosLeft;
			//Y轴坐标切换到下一行，按行高切换
			ptTile.y += cyLineHeight + GetChildMarginY();
		}
		else {
			//同一行，向右切换坐标，按当前瓦片控件的宽度切换
			ptTile.x += cxWidth + GetChildMarginX();
		}		
		//记录每列的宽度（取这一列中，控件宽度的最大值，包含此控件的外边距）
		if (colIndex < columnWidths.size()) {
			int32_t tileWidth = rcTilePos.Width();
			if (m_szItem.cx > 0) {
				tileWidth = m_szItem.cx;
			}
			tileWidth += rcMargin.left + rcMargin.right;
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

UiSize64 TileLayout::ArrangeChildFreeLayout(const std::vector<Control*>& items, 
											UiRect rect, bool isCalcOnly) const
{
	DeflatePadding(rect); //剪去内边距，剩下的是可用区域
	const UiRect& rc = rect;

	int64_t cxNeededFloat = 0;	//浮动控件需要的总宽度	
	int64_t cyNeededFloat = 0;	//浮动控件需要的总高度

	int64_t cxNeeded = 0;		//非浮动控件需要的总宽度	
	int64_t cyNeeded = 0;		//非浮动控件需要的总高度

	int32_t cyLineHeight = 0;   //每行控件（瓦片）的高度（动态计算值）

	int32_t xPosLeft = rc.left;         //控件显示内容的左侧坐标值，始终采取左对齐
	UiPoint ptTile(xPosLeft, rc.top);	//每个控件（瓦片）的顶点坐标
	const size_t itemCount = items.size();
	for (size_t index = 0; index < itemCount; ++index) {
		Control* pControl = items[index];
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if (pControl->IsFloat()) {
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
		//控件的外边距
		UiMargin rcMargin = pControl->GetMargin();

		//计算当前瓦片控件的位置坐标、宽度和高度
		UiRect rcTilePos;
		UiSize szTileSize = CalcTilePosition(pControl, 0, 0,
											 UiSize(), rc, ptTile, m_bScaleDown,
											 rcTilePos);
		if (rcTilePos.right >= rc.right) {
			//右侧已经超过边界, 如果不是靠近最左侧，则先换行，再显示
			if (rcTilePos.left > rc.left) {
				//先换行, 然后再显示
				ptTile.x = xPosLeft;
				//Y轴坐标切换到下一行，按行高切换
				ptTile.y += cyLineHeight + GetChildMarginY();
				cyLineHeight = 0;

				szTileSize = CalcTilePosition(pControl, 0, 0,
											  UiSize(), rc, ptTile, m_bScaleDown,
											  rcTilePos);				
			}
		}
		if (!isCalcOnly) {
			pControl->SetPos(rcTilePos);
		}
		cxNeeded = std::max((int64_t)rcTilePos.right + rcMargin.left + rcMargin.right, cxNeeded);
		cyNeeded = std::max((int64_t)rcTilePos.bottom + rcMargin.top + rcMargin.bottom, cyNeeded);

		//更新控件宽度值和行高值
		int32_t cxWidth = rcTilePos.Width() + rcMargin.left + rcMargin.right;
		cyLineHeight = std::max(rcTilePos.Height() + rcMargin.top + rcMargin.bottom, cyLineHeight);

		if (rcTilePos.right >= rc.right) {
			//当前控件已经超出边界，需要换行
			ptTile.x = xPosLeft;
			//Y轴坐标切换到下一行，按行高切换
			ptTile.y += cyLineHeight + GetChildMarginY();
			cyLineHeight = 0;
		}
		else {
			//不换行，向后切换横坐标
			ptTile.x += cxWidth + GetChildMarginX();
		}
	}

	//由于内边距已经剪掉，计算宽度和高度的时候，需要算上内边距
	const UiPadding& padding = GetPadding();
	cxNeeded = std::max(cxNeeded, cxNeededFloat);
	cyNeeded = std::max(cyNeeded, cyNeededFloat);

	if (isCalcOnly) {
		//返回的宽度，最大不超过外层容器的空间，因为此返回值会成为容器最终的宽度值
		if (cxNeeded > (rect.Width())) {
			cxNeeded = rect.Width();
		}
	}

	cxNeeded += (padding.left + padding.right);
	cyNeeded += (padding.top + padding.bottom);

	UiSize64 size(cxNeeded, cyNeeded);
	return size;
}

UiSize TileLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	szAvailable.Validate();
	UiRect rc(0, 0, szAvailable.Width(), szAvailable.Height());
	UiSize64 requiredSize;
	if (IsFreeLayout()) {
		requiredSize = ArrangeChildFreeLayout(items, rc, true);
	}
	else {
		std::vector<int32_t> inColumnWidths;
		std::vector<int32_t> outColumnWidths;
		requiredSize = ArrangeChildNormal(items, rc, true, inColumnWidths, outColumnWidths);
	}
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
			SetAutoCalcColumns(true);
		}
		else {
			SetAutoCalcColumns(false);
			SetColumns(_wtoi(strValue.c_str()));
		}
	}
	else if ((strName == L"scale_down") || (strName == L"scaledown")) {
		SetScaleDown(strValue == L"true");
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
		if (m_pOwner != nullptr) {
			m_pOwner->Arrange();
		}
	}	
}

void TileLayout::SetAutoCalcColumns(bool bAutoCalcColumns)
{
	if (m_bAutoCalcColumns != bAutoCalcColumns) {
		m_bAutoCalcColumns = bAutoCalcColumns;
		if (m_pOwner != nullptr) {
			m_pOwner->Arrange();
		}
	}
}

void TileLayout::SetScaleDown(bool bScaleDown)
{
	if (m_bScaleDown != bScaleDown) {
		m_bScaleDown = bScaleDown;
		if (m_pOwner != nullptr) {
			m_pOwner->Arrange();
		}
	}
}

} // namespace ui
