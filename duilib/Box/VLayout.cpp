#include "VLayout.h"

namespace ui 
{

VLayout::VLayout()
{
}

UiSize64 VLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	UiSize szAvailable(rc.Width(), rc.Height());
	szAvailable.Validate();
	if (rc.Width() < 0) {
		rc.right = rc.left;
	}
	if (rc.Height() < 0) {
		rc.bottom = rc.top;
	}

	//高度为stretch的控件数
	int32_t stretchCount = 0;
	//固定高度的控件，总的高度
	int32_t cyFixedTotal = 0;
	//需要进行布局处理的所有控件(KEY是控件，VALUE是宽度和高度)
	std::map<Control*, UiSize> itemsMap;

	//计算每个控件的宽度和高度，并记录到Map中
	for(auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}

		UiSize sz = pControl->EstimateSize(szAvailable);
		ASSERT((sz.cx >= DUI_LENGTH_STRETCH) && (sz.cy >= DUI_LENGTH_STRETCH));

		UiRect rcMargin = pControl->GetMargin();
		//计算高度
		if( sz.cy == DUI_LENGTH_STRETCH ) {
			stretchCount++;
			cyFixedTotal += (rcMargin.top + rcMargin.bottom);
		}
		else {
			if (sz.cy < pControl->GetMinHeight()) {
				sz.cy = pControl->GetMinHeight();
			}
			if (sz.cy > pControl->GetMaxHeight()) {
				sz.cy = pControl->GetMaxHeight();
			}
			if (sz.cy < 0) {
				sz.cy = 0;
			}
			cyFixedTotal += (sz.cy + rcMargin.top + rcMargin.bottom);
		}

		//计算宽度
		if (sz.cx == DUI_LENGTH_STRETCH) {
			sz.cx = szAvailable.cx - rcMargin.left - rcMargin.right;
		}
		if (sz.cx < pControl->GetMinWidth()) {
			sz.cx = pControl->GetMinWidth();
		}
		if (sz.cx > pControl->GetMaxWidth()) {
			sz.cx = pControl->GetMaxWidth();
		}
		if (sz.cx < 0) {
			sz.cx = 0;
		}
		itemsMap[pControl] = sz;
	}
	if (!itemsMap.empty()) {
		cyFixedTotal += ((int32_t)itemsMap.size() - 1) * GetChildMarginY();
	}
	
	//每个高度为stretch的控件，给与分配的实际高度（取平均值）
	int32_t cyStretch = 0;
	if (stretchCount > 0) {
		cyStretch = std::max(0, (szAvailable.cy - cyFixedTotal) / stretchCount);
	}

	//做一次预估：去除需要使用minheight/maxheight的控件数后，重新计算平均高度
	if ((cyStretch > 0) && !itemsMap.empty()) {
		for (auto iter = itemsMap.begin(); iter != itemsMap.end(); ++iter) {
			Control* pControl = iter->first;
			UiSize sz = iter->second;
			if (sz.cy == DUI_LENGTH_STRETCH) {
				sz.cy = cyStretch;
				if (sz.cy < pControl->GetMinHeight()) {
					sz.cy = pControl->GetMinHeight();
				}
				if (sz.cy > pControl->GetMaxHeight()) {
					sz.cy = pControl->GetMaxHeight();
				}
				if (sz.cy != cyStretch) {
					//这个控件需要使用min或者max高度，从平均值中移除，按照Fixed控件算
					iter->second = sz;
					--stretchCount;
					cyFixedTotal += sz.cy; //Margin已经累加过，不需要重新累加
				}
			}
		}
	}

	//重新计算Stretch控件的高度，最终以这次计算的为准；
	//如果纵向总空间不足，则按原来评估的平均高度，优先保证前面的控件可以正常显示
	if ((stretchCount > 0) && (szAvailable.cy > cyFixedTotal)){
		cyStretch = std::max(0, (szAvailable.cy - cyFixedTotal) / stretchCount);
	}

	// Position the elements
	int32_t deviation = szAvailable.cy - cyFixedTotal - cyStretch * stretchCount;//剩余可用高度，用于纠正偏差
	if (deviation < 0) {
		deviation = 0;
	}

	int32_t iPosLeft = rc.left;
	int32_t iPosRight = rc.right;
	int32_t iPosY = rc.top;

	// Place elements
	int64_t cyNeeded = 0;//需要的总高度
	int64_t cxNeeded = 0;//需要的总宽度（取各个子控件的宽度最大值）

	for(auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if( pControl->IsFloat() ) {
			SetFloatPos(pControl, rc);
			continue;
		}

		UiRect rcMargin = pControl->GetMargin();
		ASSERT(itemsMap.find(pControl) != itemsMap.end());
		UiSize sz = itemsMap[pControl];

		//计算高度
		if( sz.cy == DUI_LENGTH_STRETCH ) {
			sz.cy = cyStretch;
			if (sz.cy < pControl->GetMinHeight()) {
				sz.cy = pControl->GetMinHeight();
			}
			if (sz.cy > pControl->GetMaxHeight()) {
				sz.cy = pControl->GetMaxHeight();
			}
			if ((sz.cy <= cyStretch) && (deviation > 0)) {
				sz.cy += 1;
				deviation--;
			}
		}
				
		//调整横向对齐方式，确定X轴坐标
		int32_t childLeft = 0;
		int32_t childRight = 0;
		HorAlignType horAlignType = pControl->GetHorAlignType();
		if (horAlignType == kHorAlignLeft) {
			childLeft = iPosLeft + rcMargin.left;
			childRight = childLeft + sz.cx;
		}
		else if (horAlignType == kHorAlignRight) {
			childRight = iPosRight - rcMargin.right;
			childLeft = childRight - sz.cx;
		}
		else if (horAlignType == kHorAlignCenter) {
			childLeft = iPosLeft + (iPosRight - iPosLeft + rcMargin.left - rcMargin.right - sz.cx) / 2;
			childRight = childLeft + sz.cx;
		}

		//设置控件的位置
		UiRect controlRect(childLeft, iPosY + rcMargin.top, childRight, iPosY + rcMargin.top + sz.cy);		
		pControl->SetPos(controlRect);
		cxNeeded = std::max(cxNeeded, (int64_t)controlRect.Width());

		//调整当前Y轴坐标值
		iPosY += (sz.cy + rcMargin.top + rcMargin.bottom + GetChildMarginY());
		cyNeeded += (sz.cy + rcMargin.top + rcMargin.bottom);
	}
	if (!itemsMap.empty()) {
		cyNeeded += ((int64_t)itemsMap.size() - 1) * GetChildMarginY();
	}

	UiSize64 size(cxNeeded, cyNeeded);
	return size;
}

UiSize VLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	UiSize totalSize;
	UiSize itemSize;
	int32_t estimateCount = 0;
	for(Control* pControl : items)	{
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}

		estimateCount++;
		UiRect rcMargin = pControl->GetMargin();
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
		totalSize.cx = std::max(itemSize.cx + rcMargin.left + rcMargin.right, totalSize.cx);
		totalSize.cy += (itemSize.cy + rcMargin.top + rcMargin.bottom);
	}

	if ((estimateCount - 1) > 0) {
		totalSize.cy += (estimateCount - 1) * GetChildMarginY();
	}
	UiRect rcPadding = GetPadding();
	totalSize.cx += (rcPadding.left + rcPadding.right);
	totalSize.cy += (rcPadding.top + rcPadding.bottom);
	return totalSize;
}

} // namespace ui
