#include "VTileLayout.h"
#include "duilib/Core/Box.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/GlobalManager.h"
#include <numeric>

namespace ui 
{
VTileLayout::VTileLayout(): 
	m_nColumns(0), 
	m_szItem(0, 0),
	m_bAutoCalcColumns(false),
	m_bScaleDown(true)
{
}

UiSize VTileLayout::CalcEstimateSize(Control* pControl, const UiSize& szItem, UiRect rc)
{
	if ((pControl == nullptr) || !pControl->IsVisible()) {
		return UiSize();
	}
	
	//估算的可用区域，始终使用总区域
	UiMargin rcMargin = pControl->GetMargin();
	int32_t width = rc.Width() - rcMargin.left - rcMargin.right;
	int32_t height = rc.Height() - rcMargin.top - rcMargin.bottom;
	UiSize szAvailable(width, height);
	szAvailable.Validate();
	UiEstSize estSize = pControl->EstimateSize(szAvailable);
	UiSize childSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
	if (estSize.cy.IsStretch()) {
		childSize.cy = szItem.cy;
	}
	if (childSize.cy < pControl->GetMinHeight()) {
		childSize.cy = pControl->GetMinHeight();
	}
	if (childSize.cy > pControl->GetMaxHeight()) {
		childSize.cy = pControl->GetMaxHeight();
	}

	if (estSize.cx.IsStretch()) {
		childSize.cx = szItem.cx;
	}
	if (childSize.cx < pControl->GetMinWidth()) {
		childSize.cx = pControl->GetMinWidth();
	}
	if (childSize.cx > pControl->GetMaxWidth()) {
		childSize.cx = pControl->GetMaxWidth();
	}
	return childSize;
}

void VTileLayout::CalcTileColumns(const std::vector<ItemSizeInfo>& normalItems, const UiRect& rc,
								 int32_t tileWidth, int32_t childMarginX, int32_t childMarginY,
	                             int32_t& nColumns)
{
	nColumns = 0;
	if (tileWidth <= 0) {
		//需要先计算瓦片控件宽度，然后根据宽度计算列数
		int32_t maxWidth = 0;
		int64_t areaTotal = 0;
		const int64_t maxArea = (int64_t)rc.Width() * rc.Height();
		for (const ItemSizeInfo& itemSizeInfo : normalItems) {
			UiMargin rcMargin = itemSizeInfo.pControl->GetMargin();
			UiSize childSize(itemSizeInfo.cx, itemSizeInfo.cy);
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
		tileWidth = maxWidth;
	}
	if (tileWidth > 0) {
		//使用设置的宽度作为瓦片控件的宽度，并通过设置的宽度，计算列数
		int32_t totalWidth = rc.Width();
		while (totalWidth > 0) {
			totalWidth -= tileWidth;
			if (nColumns != 0) {
				totalWidth -= childMarginX;
			}
			if (totalWidth >= 0) {
				++nColumns;
			}
		}
	}
}

UiSize64 VTileLayout::EstimateFloatSize(Control* pControl, UiRect rc)
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

UiSize64 VTileLayout::ArrangeFloatChild(const std::vector<Control*>& items,
									   UiRect rc,
									   const UiSize& szItem,
									   bool isCalcOnly,
									   std::vector<ItemSizeInfo>& normalItems)
{
	int64_t cxNeededFloat = 0;	//浮动控件需要的总宽度
	int64_t cyNeededFloat = 0;	//浮动控件需要的总高度
	for (Control* pControl : items) {
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
		}
		else {
			//普通控件
			UiSize childSize = CalcEstimateSize(pControl, szItem, rc);
			if ((childSize.cx <= 0) || (childSize.cy <= 0)) {
				//大小为0的，不可显示控件(可能是拉伸控件)
				if (!isCalcOnly) {
					UiRect rcPos(rc);
					rcPos.right = rcPos.left;
					rcPos.bottom = rcPos.top;
					pControl->SetPos(rcPos);
				}
			}
			else {
				ItemSizeInfo info;
				info.pControl = pControl;
				info.cx = childSize.cx;
				info.cy = childSize.cy;
				normalItems.push_back(info);
			}
		}
	}
	return UiSize64(cxNeededFloat, cyNeededFloat);
}

int32_t VTileLayout::CalcTileRowHeight(const std::vector<ItemSizeInfo>& normalItems,
									  const std::vector<ItemSizeInfo>::const_iterator iterBegin,
									  int32_t nColumns,
									  const UiSize& szItem)
{
	//szItem的宽度和高度值，是包含了控件的外边距和内边距的
	ASSERT(nColumns > 0);
	int32_t cyHeight = szItem.cy;
	if (cyHeight > 0) {
		//如果设置了高度，则优先使用设置的高度值
		return cyHeight;
	}
	if (nColumns <= 0) {
		return 0;
	}

	int32_t iIndex = 0;
	for (auto it = iterBegin; it != normalItems.end(); ++it) {
		const ItemSizeInfo& itemSizeInfo = *it;
		UiMargin rcMargin = itemSizeInfo.pControl->GetMargin();
		UiSize szTile(itemSizeInfo.cx, itemSizeInfo.cy);

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

UiSize VTileLayout::CalcTilePosition(const ItemSizeInfo& itemSizeInfo,
	                                int32_t tileWidth, int32_t tileHeight,
	                                const UiPoint& ptTile, bool bScaleDown, UiRect& szTilePos)
{
	szTilePos.Clear();
	//目标区域大小（宽和高）
	UiSize szItem(tileWidth, tileHeight);
	szItem.Validate();

	//瓦片控件大小(宽和高), 包含外边距
	UiMargin rcMargin = itemSizeInfo.pControl->GetMargin();
	UiSize childSize(itemSizeInfo.cx + rcMargin.left + rcMargin.right, 
		             itemSizeInfo.cy + rcMargin.top + rcMargin.bottom);
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

	//对控件进行等比例缩放(缩放的时候，需要去掉外边距)
	UiRect rcRealTile = rcTile;
	rcRealTile.Deflate(rcMargin);
	UiSize realSize(childSize.cx - rcMargin.left - rcMargin.right,
				    childSize.cy - rcMargin.top - rcMargin.bottom);
	if (bScaleDown && 
		(rcRealTile.Width() > 0) && (rcRealTile.Height() > 0) &&
		(realSize.cx > 0) && (realSize.cy > 0) ) {
		if ((realSize.cx > rcRealTile.Width()) || (realSize.cy > rcRealTile.Height())) {
			//满足缩放条件，进行等比缩放
			UiSize oldSize = realSize;
			double cx = realSize.cx;
			double cy = realSize.cy;
			double cxRatio = cx / rcRealTile.Width();
			double cyRatio = cy / rcRealTile.Height();
			if (cxRatio > cyRatio) {
				ASSERT(realSize.cx > rcRealTile.Width());
				double ratio = cx / realSize.cy;
				realSize.cx = rcRealTile.Width();
				realSize.cy = static_cast<int32_t>(realSize.cx / ratio + 0.5);
			}
			else {
				ASSERT(realSize.cy > rcRealTile.Height());
				double ratio = cy / realSize.cx;
				realSize.cy = rcRealTile.Height();
				realSize.cx = static_cast<int32_t>(realSize.cy / ratio + 0.5);
			}
		}
	}

	//rcTile包含外边距，realSize不包含外边距
	szTilePos = GetFloatPos(itemSizeInfo.pControl, rcTile, realSize);
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

UiSize64 VTileLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
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

bool VTileLayout::IsFreeLayout() const
{
	return (!m_bAutoCalcColumns && (m_nColumns == 0) && (m_szItem.cx == 0) && m_szItem.cy == 0);
}

UiSize64 VTileLayout::ArrangeChildNormal(const std::vector<Control*>& items,
										UiRect rect,
										bool isCalcOnly,
										const std::vector<int32_t>& inColumnWidths,
										std::vector<int32_t>& outColumnWidths) const
{
	ASSERT(!IsFreeLayout());
	DeflatePadding(rect); //剪去内边距，剩下的是可用区域
	const UiRect& rc = rect;

	//调整浮动控件，过滤隐藏控件、不可显示控件等
	//拉伸类型的子控件：如果(m_szItem.cx > 0) && (m_szItem.cy > 0) 为true，则可以显示，否则会被过滤掉
	std::vector<ItemSizeInfo> normalItems;
	ArrangeFloatChild(items, rc, m_szItem, isCalcOnly, normalItems); //浮动控件需要的总宽度和高度

	int32_t nColumns = m_nColumns;  //列数（设置值）
	if (m_bAutoCalcColumns) {
		//如果自动计算列数，则重新计算列数
		nColumns = 0;
	}
	if (nColumns <= 0) {
		CalcTileColumns(normalItems, rc, m_szItem.cx, GetChildMarginX(), GetChildMarginY(), nColumns);
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

	int32_t cyRowHeight = 0;   //每行控件（瓦片）的高度（动态计算值）

	std::vector<int32_t> rowHeights;   //每列的高度值，计算值
	std::vector<int32_t> columnWidths; //每行的宽度值，计算值
	columnWidths.resize(nColumns);
	rowHeights.resize(1);

	int32_t nRowTileCount = 0;  //本行容纳的瓦片控件个数
	int32_t nRowIndex = 0;      //当前的行号

	int32_t xPosLeft = rc.left; 
	//控件显示内容的左侧坐标值(横向区域居中对齐)
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
	for( auto it = normalItems.begin(); it != normalItems.end(); ++it ) {
		const ItemSizeInfo& itemSizeInfo = *it;
		Control* pControl = itemSizeInfo.pControl;
		if (nRowTileCount == 0) {
			//一行的开始，计算这一行的高度
			nRowTileCount = nColumns;
			cyRowHeight = CalcTileRowHeight(normalItems, it, nColumns, m_szItem);
			ASSERT(cyRowHeight > 0);//不可能是零
		}
		
		const int32_t colIndex = nColumns - nRowTileCount;//当前列下标[0, nColumns)
		int32_t fixedColumnWidth = 0; //当前传入的列宽度, 固定值
		if (((int32_t)fixedColumnWidths.size() == nColumns) &&
			(colIndex < fixedColumnWidths.size())) {
			fixedColumnWidth = fixedColumnWidths[colIndex];
		}

		//计算当前瓦片控件的位置坐标、宽度(cxWidth)和高度(cyHeight)		
		UiRect rcTilePos;

		UiPoint posLeftTop = ptTile;         //该控件的左上角坐标值
		int32_t posWidth = fixedColumnWidth; //该控件的最大可用宽度
		int32_t posHeight = cyRowHeight;     //该控件的最大可用高度
		UiSize szTileSize = CalcTilePosition(itemSizeInfo, posWidth, posHeight,
			                                 posLeftTop, m_bScaleDown, rcTilePos);//返回值包含了控件的外边距
		
		if (!isCalcOnly) {
			pControl->SetPos(rcTilePos);
		}

		int32_t cxWidth = szTileSize.cx;
		if (fixedColumnWidth > 0) {
			cxWidth = fixedColumnWidth;
		}

		//计算本行高度最大值，高度值需要包含外边距
		int32_t tileHeight = (m_szItem.cy > 0) ? m_szItem.cy : szTileSize.cy;
		rowHeights[nRowIndex] = std::max(tileHeight, rowHeights[nRowIndex]);

		--nRowTileCount;
		if(nRowTileCount == 0 ) {
			//换行
			rowHeights.push_back(0);
			nRowIndex = (int32_t)rowHeights.size() - 1;			
			
			//重新设置X坐标和Y坐标的位置(行首)
			ptTile.x = xPosLeft;
			//Y轴坐标切换到下一行，按行高切换
			ptTile.y += cyRowHeight + GetChildMarginY();
		}
		else {
			//同一行，向右切换坐标，按当前瓦片控件的宽度切换
			ptTile.x += cxWidth + GetChildMarginX();
		}		
		//记录每列的宽度（取这一列中，控件宽度的最大值，包含此控件的外边距）
		if (colIndex < columnWidths.size()) {
			int32_t tileWidth = (m_szItem.cx > 0) ? m_szItem.cx : cxWidth;	
			columnWidths[colIndex] = std::max(tileWidth, columnWidths[colIndex]);
		}
	}

	//由于内边距已经剪掉，计算宽度和高度的时候，需要算上内边距
	UiPadding rcPadding;
	if (GetOwner() != nullptr) {
		rcPadding = GetOwner()->GetPadding();
	}
	//计算所需宽度
	int64_t cxNeeded = std::accumulate(columnWidths.begin(), columnWidths.end(), 0);
	if (columnWidths.size() > 1) {
		cxNeeded += (columnWidths.size() - 1) * GetChildMarginX();
	}
	cxNeeded += (rcPadding.left + rcPadding.right);

	//计算所需高度
	int64_t cyNeeded = std::accumulate(rowHeights.begin(), rowHeights.end(), 0);
	if (rowHeights.size() > 1) {
		cyNeeded += (rowHeights.size() - 1) * GetChildMarginY();
	}
	cyNeeded += (rcPadding.top + rcPadding.bottom);

	outColumnWidths.swap(columnWidths);
	UiSize64 size(cxNeeded, cyNeeded);
	return size;
}

UiSize64 VTileLayout::ArrangeChildFreeLayout(const std::vector<Control*>& items, 
											UiRect rect, bool isCalcOnly) const
{
	DeflatePadding(rect); //剪去内边距，剩下的是可用区域
	const UiRect& rc = rect;

	//调整浮动控件，过滤隐藏控件、不可显示控件等
	//拉伸类型的子控件：如果(m_szItem.cx > 0) && (m_szItem.cy > 0) 为true，则可以显示，否则会被过滤掉
	std::vector<ItemSizeInfo> normalItems;
	ArrangeFloatChild(items, rc, m_szItem, isCalcOnly, normalItems); //浮动控件需要的总宽度和高度

	int64_t cxNeeded = 0;		//非浮动控件需要的总宽度	
	int64_t cyNeeded = 0;		//非浮动控件需要的总高度

	int32_t cyRowHeight = 0;    //每行控件（瓦片）的高度（动态计算值）

	const int32_t xPosLeft = rc.left;         //控件显示内容的左侧坐标值，始终采取左对齐
	UiPoint ptTile(xPosLeft, rc.top);	//每个控件（瓦片）的顶点坐标
	const size_t itemCount = normalItems.size();
	for (size_t index = 0; index < itemCount; ++index) {
		const ItemSizeInfo& itemSizeInfo = normalItems[index];
		Control* pControl = itemSizeInfo.pControl;

		//计算当前瓦片控件的位置坐标、宽度和高度
		UiRect rcTilePos;
		UiSize szTileSize = CalcTilePosition(itemSizeInfo, 0, 0,
											 ptTile, m_bScaleDown, rcTilePos);
		if (rcTilePos.right >= rc.right) {
			//右侧已经超过边界, 如果不是靠近最左侧，则先换行，再显示
			if (ptTile.x != xPosLeft) {
				//先换行, 然后再显示
				ptTile.x = xPosLeft;
				ptTile.y += cyRowHeight + GetChildMarginY();//下一行
				cyRowHeight = 0;

				szTileSize = CalcTilePosition(itemSizeInfo, 0, 0,
											  ptTile, m_bScaleDown, rcTilePos);				
			}
		}
		if (!isCalcOnly) {
			pControl->SetPos(rcTilePos);
		}

		UiMargin rcMargin = pControl->GetMargin();
		cxNeeded = std::max((int64_t)rcTilePos.right + rcMargin.right, cxNeeded);
		cyNeeded = std::max((int64_t)rcTilePos.bottom + rcMargin.bottom, cyNeeded);

		//更新控件宽度值和行高值
		int32_t cxWidth = rcTilePos.Width() + rcMargin.left + rcMargin.right;
		cyRowHeight = std::max(rcTilePos.Height() + rcMargin.top + rcMargin.bottom, cyRowHeight);

		if (rcTilePos.right >= rc.right) {
			//当前控件已经超出边界，需要换行
			ptTile.x = xPosLeft;
			ptTile.y += cyRowHeight + GetChildMarginY();//下一行
			cyRowHeight = 0;
		}
		else {
			//不换行，向后切换横坐标
			ptTile.x += cxWidth + GetChildMarginX();
		}
	}

	//由于内边距已经剪掉，计算宽度和高度的时候，需要算上内边距
	//(只需要增加右侧和底部的内边距，因为计算的时候，是按照.rigth和.bottom计算的)
	UiPadding rcPadding;
	if (GetOwner() != nullptr) {
		rcPadding = GetOwner()->GetPadding();
	}
	cxNeeded += rcPadding.right;
	cyNeeded += rcPadding.bottom;

	if (isCalcOnly) {
		//返回的宽度，最大不超过外层容器的空间，因为此返回值会成为容器最终的宽度值
		if (cxNeeded > (rect.Width())) {
			cxNeeded = rect.Width();
		}
	}
	UiSize64 size(cxNeeded, cyNeeded);
	return size;
}

UiSize VTileLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
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

bool VTileLayout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	bool hasAttribute = true;
	if((strName == L"item_size") || (strName == L"itemsize")){
		UiSize szItem;
		AttributeUtil::ParseSizeValue(strValue.c_str(), szItem);
		SetItemSize(szItem);
	}
	else if( (strName == L"columns") || (strName == L"rows")) {
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

const UiSize& VTileLayout::GetItemSize() const
{
	return m_szItem;
}

void VTileLayout::SetItemSize(UiSize szItem, bool bNeedDpiScale)
{
	szItem.cx = std::max(szItem.cx, 0);
	szItem.cy = std::max(szItem.cy, 0);
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleSize(szItem);
	}

	if( (m_szItem.cx != szItem.cx) || (m_szItem.cy != szItem.cy) ) {
		m_szItem = szItem;
		if (GetOwner() != nullptr) {
			GetOwner()->Arrange();
		}		
	}
}

int32_t VTileLayout::GetColumns() const
{
	return m_nColumns;
}

void VTileLayout::SetColumns(int32_t nCols)
{
	nCols = std::max(nCols, 0);
	if (m_nColumns != nCols) {
		m_nColumns = nCols;
		if (GetOwner() != nullptr) {
			GetOwner()->Arrange();
		}
	}	
}

void VTileLayout::SetAutoCalcColumns(bool bAutoCalcColumns)
{
	if (m_bAutoCalcColumns != bAutoCalcColumns) {
		m_bAutoCalcColumns = bAutoCalcColumns;
		if (GetOwner() != nullptr) {
			GetOwner()->Arrange();
		}
	}
}

bool VTileLayout::IsAutoCalcColumns() const
{
	return m_bAutoCalcColumns;
}

void VTileLayout::SetScaleDown(bool bScaleDown)
{
	if (m_bScaleDown != bScaleDown) {
		m_bScaleDown = bScaleDown;
		if (GetOwner() != nullptr) {
			GetOwner()->Arrange();
		}
	}
}

bool VTileLayout::IsScaleDown() const
{
	return m_bScaleDown;
}

} // namespace ui
