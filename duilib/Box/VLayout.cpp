#include "VLayout.h"
#include "duilib/Core/Box.h"
#include <map>

namespace ui 
{

VLayout::VLayout()
{
}

UiSize64 VLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	DeflatePadding(rc);
	const UiSize szAvailable(rc.Width(), rc.Height());

	//高度为stretch的控件数
	int32_t stretchCount = 0;
	//固定高度的控件，总的高度
	int32_t cyFixedTotal = 0;
	//需要进行布局处理的所有控件(KEY是控件，VALUE是宽度和高度)
	std::map<Control*, UiEstSize> itemsMap;

	//计算每个控件的宽度和高度，并记录到Map中
	for(auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}

		UiEstSize estSize = pControl->EstimateSize(szAvailable);
		UiSize sz = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
		UiMargin rcMargin = pControl->GetMargin();
		//计算高度
		if(estSize.cy.IsStretch()) {
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
		if (estSize.cx.IsStretch()) {
			sz.cx = CalcStretchValue(estSize.cx, szAvailable.cx) - rcMargin.left - rcMargin.right;
			sz.cx = std::max(sz.cx, 0);
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
		if (!estSize.cy.IsStretch()) {
			estSize.cy.SetInt32(sz.cy);
		}
		estSize.cx.SetInt32(sz.cx);//cx是已经计算好的确定数值，不再有拉伸和自动类型值
		itemsMap[pControl] = estSize;
	}
	if (!itemsMap.empty()) {
		cyFixedTotal += ((int32_t)itemsMap.size() - 1) * GetChildMarginY();
	}
	
	float fStretchValue = 0;	//每个拉伸控件，按设置为100%时，应该分配的高度值
	float fTotalStretch = 0;	//按设置为100%时为一个控件单位，总共有多少个控件单位
	if (stretchCount > 0) {		
		for (auto iter : itemsMap) {
			const UiEstSize& itemEstSize = iter.second;
			if (itemEstSize.cy.IsStretch()) {
				fTotalStretch += itemEstSize.cy.GetStretchPercentValue() / 100.0f;
			}
		}
		ASSERT(fTotalStretch > 0);
		if (fTotalStretch > 0) {
			fStretchValue = std::max(0, (szAvailable.cy - cyFixedTotal)) / fTotalStretch;
		}
	}

	//做一次预估：去除需要使用minheight/maxheight的控件数后，重新计算平均高度
	bool bStretchCountChanged = false;
	if ((fStretchValue > 0) && !itemsMap.empty()) {
		for (auto iter = itemsMap.begin(); iter != itemsMap.end(); ++iter) {
			Control* pControl = iter->first;
			UiEstSize estSize = iter->second;
			UiSize sz(estSize.cx.GetInt32(), estSize.cy.GetInt32());
			if (estSize.cy.IsStretch()) {
				int32_t cyStretch = static_cast<int32_t>(fStretchValue * estSize.cy.GetStretchPercentValue() / 100.0f);
				sz.cy = cyStretch;
				if (sz.cy < pControl->GetMinHeight()) {
					sz.cy = pControl->GetMinHeight();
				}
				if (sz.cy > pControl->GetMaxHeight()) {
					sz.cy = pControl->GetMaxHeight();
				}
				if (sz.cy != cyStretch) {
					//这个控件需要使用min或者max高度，从平均值中移除，按照Fixed控件算
					estSize.cy.SetInt32(sz.cy);
					iter->second = estSize;
					--stretchCount;
					cyFixedTotal += sz.cy; //Margin已经累加过，不需要重新累加
					bStretchCountChanged = true;
				}
			}
		}
	}

	//重新计算Stretch控件的高度，最终以这次计算的为准；
	//如果纵向总空间不足，则按原来评估的平均高度，优先保证前面的控件可以正常显示
	if (bStretchCountChanged && (stretchCount > 0) && (szAvailable.cy > cyFixedTotal)) {
		fTotalStretch = 0;
		for (auto iter : itemsMap) {
			const UiEstSize& itemEstSize = iter.second;
			if (itemEstSize.cy.IsStretch()) {
				fTotalStretch += itemEstSize.cy.GetStretchPercentValue() / 100.0f;
			}
		}
		ASSERT(fTotalStretch > 0);
		if (fTotalStretch > 0) {
			fStretchValue = std::max(0, (szAvailable.cy - cyFixedTotal)) / fTotalStretch;
		}
	}

	int32_t iPosLeft = rc.left;
	int32_t iPosRight = rc.right;
	int32_t iPosY = rc.top;

	// Place elements
	int64_t cyNeeded = 0;//需要的总高度
	int64_t cxNeeded = 0;//需要的总宽度（取各个子控件的宽度最大值）
	int32_t assignedStretch = 0; //已经分配的拉伸空间大小

	for(auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if( pControl->IsFloat() ) {
			SetFloatPos(pControl, rc);
			continue;
		}

		UiMargin rcMargin = pControl->GetMargin();
		ASSERT(itemsMap.find(pControl) != itemsMap.end());
		UiEstSize estSize = itemsMap[pControl];
		UiSize sz(estSize.cx.GetInt32(), estSize.cy.GetInt32());

		//计算高度
		if(estSize.cy.IsStretch()) {
			int32_t cyStretch = static_cast<int32_t>(fStretchValue * estSize.cy.GetStretchPercentValue() / 100.0f);
			sz.cy = cyStretch;
			if (sz.cy < pControl->GetMinHeight()) {
				sz.cy = pControl->GetMinHeight();
			}
			if (sz.cy > pControl->GetMaxHeight()) {
				sz.cy = pControl->GetMaxHeight();
			}
			assignedStretch += sz.cy;
			--stretchCount;
			if (stretchCount == 0) {
				//在最后一个拉伸控件上，修正计算偏差
				int32_t deviation = szAvailable.cy - cyFixedTotal - assignedStretch;
				if (deviation > 0) {
					sz.cy += deviation;
				}
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
		cxNeeded = std::max(cxNeeded, (int64_t)controlRect.Width() + rcMargin.left + rcMargin.right);

		//调整当前Y轴坐标值
		iPosY += (sz.cy + rcMargin.top + rcMargin.bottom + GetChildMarginY());
		cyNeeded += (sz.cy + rcMargin.top + rcMargin.bottom);
	}
	if (!itemsMap.empty()) {
		cyNeeded += ((int64_t)itemsMap.size() - 1) * GetChildMarginY();
	}

	UiSize64 size(cxNeeded, cyNeeded);
	UiPadding rcPadding;
	if (GetOwner() != nullptr) {
		rcPadding = GetOwner()->GetPadding();
	}
	if (size.cx > 0) {
		size.cx += (rcPadding.left + rcPadding.right);
	}
	if (size.cy > 0) {
		size.cy += (rcPadding.top + rcPadding.bottom);
	}
	return size;
}

UiSize VLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	//宽度：取所有子控件宽度的最大值，加上Margin、Padding等，不含拉伸类型的子控件
	//高度：所有子控件高度之和，加上Margin、Padding等，不含拉伸类型的子控件
	UiSize totalSize;
	UiSize itemSize;
	int32_t estimateCount = 0;
	for (Control* pControl : items)	{
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}

		estimateCount++;
		UiMargin rcMargin = pControl->GetMargin();
		UiEstSize estSize = pControl->EstimateSize(szAvailable);
		itemSize = UiSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
		if (estSize.cx.IsStretch()) {
			//拉伸类型的子控件，不计入
			itemSize.cx = 0;
		}
		else {
			if (itemSize.cx < pControl->GetMinWidth()) {
				itemSize.cx = pControl->GetMinWidth();
			}
			if (itemSize.cx > pControl->GetMaxWidth()) {
				itemSize.cx = pControl->GetMaxWidth();
			}
		}
		if (estSize.cy.IsStretch()) {
			//拉伸类型的子控件，不计入
			itemSize.cy = 0;
		}
		else {
			if (itemSize.cy < pControl->GetMinHeight()) {
				itemSize.cy = pControl->GetMinHeight();
			}
			if (itemSize.cy > pControl->GetMaxHeight()) {
				itemSize.cy = pControl->GetMaxHeight();
			}
		}		
		
		if (itemSize.cx > 0) {
			totalSize.cx = std::max(itemSize.cx + rcMargin.left + rcMargin.right, totalSize.cx);
		}
		if (itemSize.cy > 0) {
			totalSize.cy += (itemSize.cy + rcMargin.top + rcMargin.bottom);
		}		
	}

	if ((totalSize.cy > 0) && ((estimateCount - 1) > 0)) {
		totalSize.cy += (estimateCount - 1) * GetChildMarginY();
	}
	UiPadding rcPadding;
	if (GetOwner() != nullptr) {
		rcPadding = GetOwner()->GetPadding();
	}
	if (totalSize.cx > 0) {
		totalSize.cx += (rcPadding.left + rcPadding.right);
	}
	if (totalSize.cy > 0) {
		totalSize.cy += (rcPadding.top + rcPadding.bottom);
	}
	if ((totalSize.cx == 0) || (totalSize.cy == 0)) {
		CheckConfig(items);
	}
	return totalSize;
}

} // namespace ui
