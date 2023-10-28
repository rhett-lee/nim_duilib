#include "HLayout.h"
#include "duilib/Core/Box.h"
#include <map>

namespace ui 
{

HLayout::HLayout()
{
}

UiSize64 HLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	DeflatePadding(rc);
	const UiSize szAvailable(rc.Width(), rc.Height());
	
	//宽度为stretch的控件数
	int32_t stretchCount = 0;
	//固定宽度的控件，总的高度
	int32_t cxFixedTotal = 0;
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
		//计算宽度
		if(estSize.cx.IsStretch()) {
			stretchCount++;
			cxFixedTotal += (rcMargin.left + rcMargin.right);
		}
		else {
			if (sz.cx < pControl->GetMinWidth()) {
				sz.cx = pControl->GetMinWidth();
			}
			if (sz.cx > pControl->GetMaxWidth()) {
				sz.cx = pControl->GetMaxWidth();
			}
			if (sz.cx < 0) {
				sz.cx = 0;
			}
			cxFixedTotal += (sz.cx + rcMargin.left + rcMargin.right);
		}
		
		//计算高度
		if (estSize.cy.IsStretch()) {
			sz.cy = (CalcStretchValue(estSize.cy, szAvailable.cy) - rcMargin.top - rcMargin.bottom);
			sz.cy = std::max(sz.cy, 0);
		}
		if (sz.cy < pControl->GetMinHeight()) {
			sz.cy = pControl->GetMinHeight();
		}
		if (sz.cy > pControl->GetMaxHeight()) {
			sz.cy = pControl->GetMaxHeight();
		}
		if (sz.cy < 0) {
			sz.cy = 0;
		}
		if (!estSize.cx.IsStretch()) {
			estSize.cx.SetInt32(sz.cx);
		}
		estSize.cy.SetInt32(sz.cy);//cy是已经计算好的确定数值，不再有拉伸和自动类型值
		itemsMap[pControl] = estSize;
	}
	if (!itemsMap.empty()) {
		cxFixedTotal += ((int32_t)itemsMap.size() - 1) * GetChildMarginX();
	}

	float fStretchValue = 0;	//每个拉伸控件，按设置为100%时，应该分配的高度值
	float fTotalStretch = 0;	//按设置为100%时为一个控件单位，总共有多少个控件单位
	if (stretchCount > 0) {
		for (auto iter : itemsMap) {
			const UiEstSize& itemEstSize = iter.second;
			if (itemEstSize.cx.IsStretch()) {
				fTotalStretch += itemEstSize.cx.GetStretchPercentValue() / 100.0f;
			}
		}
		ASSERT(fTotalStretch > 0);
		if (fTotalStretch > 0) {
			fStretchValue = std::max(0, (szAvailable.cx - cxFixedTotal)) / fTotalStretch;
		}
	}

	//做一次预估：去除需要使用minwidth/maxwidth的控件数后，重新计算平均高度
	bool bStretchCountChanged = false;
	if ((fStretchValue > 0) && !itemsMap.empty()) {
		for (auto iter = itemsMap.begin(); iter != itemsMap.end(); ++iter) {
			Control* pControl = iter->first;
			UiEstSize estSize = iter->second;
			UiSize sz(estSize.cx.GetInt32(), estSize.cy.GetInt32());
			if (estSize.cx.IsStretch()) {
				int32_t cxStretch = static_cast<int32_t>(fStretchValue * estSize.cx.GetStretchPercentValue() / 100.0f);
				sz.cx = cxStretch;
				if (sz.cx < pControl->GetMinWidth()) {
					sz.cx = pControl->GetMinWidth();
				}
				if (sz.cx > pControl->GetMaxWidth()) {
					sz.cx = pControl->GetMaxWidth();
				}
				if (sz.cx != cxStretch) {
					//这个控件需要使用min或者max宽度，从平均值中移除，按照Fixed控件算
					estSize.cx.SetInt32(sz.cx);
					iter->second = estSize;
					--stretchCount;
					cxFixedTotal += sz.cx; //Margin已经累加过，不需要重新累加
					bStretchCountChanged = true;
				}
			}
		}
	}
	//重新计算Stretch控件的宽度，最终以这次计算的为准；
	//如果横向总空间不足，则按原来评估的平均高度，优先保证前面的控件可以正常显示
	if (bStretchCountChanged && (stretchCount > 0) && (szAvailable.cx > cxFixedTotal)) {
		fTotalStretch = 0;
		for (auto iter : itemsMap) {
			const UiEstSize& itemEstSize = iter.second;
			if (itemEstSize.cx.IsStretch()) {
				fTotalStretch += itemEstSize.cx.GetStretchPercentValue() / 100.0f;
			}
		}
		ASSERT(fTotalStretch > 0);
		if (fTotalStretch > 0) {
			fStretchValue = std::max(0, (szAvailable.cx - cxFixedTotal)) / fTotalStretch;
		}
	}

	int32_t iPosTop = rc.top;
	int32_t iPosBottom = rc.bottom;
	int32_t iPosX = rc.left;

	// Place elements
	int64_t cyNeeded = 0;//需要的总高度（取各个子控件的高度最大值）
	int64_t cxNeeded = 0;//需要的总宽度
	int32_t assignedStretch = 0; //已经分配的拉伸空间大小

	for(auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if (pControl->IsFloat()) {
			SetFloatPos(pControl, rc);
			continue;
		}

		UiMargin rcMargin = pControl->GetMargin();
		ASSERT(itemsMap.find(pControl) != itemsMap.end());
		UiEstSize estSize = itemsMap[pControl];
		UiSize sz(estSize.cx.GetInt32(), estSize.cy.GetInt32());

		//计算宽度
		if(estSize.cx.IsStretch()) {
			int32_t cxStretch = static_cast<int32_t>(fStretchValue * estSize.cx.GetStretchPercentValue() / 100.0f);
			sz.cx = cxStretch;
			if (sz.cx < pControl->GetMinWidth()) {
				sz.cx = pControl->GetMinWidth();
			}
			if (sz.cx > pControl->GetMaxWidth()) {
				sz.cx = pControl->GetMaxWidth();
			}
			assignedStretch += sz.cx;
			--stretchCount;
			if (stretchCount == 0) {
				//在最后一个拉伸控件上，修正计算偏差
				int32_t deviation = szAvailable.cx - cxFixedTotal - assignedStretch;
				if (deviation > 0) {
					sz.cx += deviation;
				}
			}
		}

		//调整纵向对齐方式，确定Y轴坐标
		int32_t childTop = 0;
		int32_t childBottm = 0;
		VerAlignType verAlignType = pControl->GetVerAlignType();
		if (verAlignType == kVerAlignTop) {
			childTop = iPosTop + rcMargin.top;
			childBottm = childTop + sz.cy;
		}
		else if (verAlignType == kVerAlignBottom) {
			childBottm = iPosBottom - rcMargin.bottom;
			childTop = childBottm - sz.cy;
		}
		else if (verAlignType == kVerAlignCenter) {
			childTop = iPosTop + (iPosBottom - iPosTop + rcMargin.top - rcMargin.bottom - sz.cy) / 2;
			childBottm = childTop + sz.cy;
		}

		//设置控件的位置
		UiRect rcChildPos(iPosX + rcMargin.left, childTop, iPosX + rcMargin.left + sz.cx, childBottm);
		pControl->SetPos(rcChildPos);
		cyNeeded = std::max(cyNeeded, (int64_t)rcChildPos.Height() + rcMargin.top + rcMargin.bottom);

		//调整当前Y轴坐标值
		iPosX += (sz.cx + rcMargin.left + GetChildMarginX() + rcMargin.right);
		cxNeeded += (sz.cx + rcMargin.left + rcMargin.right);
	}
	if (!itemsMap.empty()) {
		cxNeeded += ((int64_t)itemsMap.size() - 1) * GetChildMarginX();
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

UiSize HLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	//宽度：所有子控件宽度之和，加上Margin、Padding等，不含拉伸类型的子控件
	//高度：取所有子控件高度的最大值，加上Margin、Padding等，不含拉伸类型的子控件
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
		
		if (itemSize.cy > 0) {
			totalSize.cy = std::max(itemSize.cy + rcMargin.top + rcMargin.bottom, totalSize.cy);
		}
		if (itemSize.cx > 0) {
			totalSize.cx += (itemSize.cx + rcMargin.left + rcMargin.right);
		}		
	}

	if ((totalSize.cx > 0) && ((estimateCount - 1) > 0)) {
		totalSize.cx += (estimateCount - 1) * GetChildMarginX();
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
