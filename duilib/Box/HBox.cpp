#include "HBox.h"

namespace ui
{

HLayout::HLayout()
{

}

UiSize64 HLayout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	UiSize szAvailable(rc.Width(), rc.Height());
	szAvailable.cx = std::max((int)szAvailable.cx, 0);
	szAvailable.cy = std::max((int)szAvailable.cy, 0);
	if (rc.Width() < 0) {
		rc.right = rc.left;
	}
	if (rc.Height() < 0) {
		rc.bottom = rc.bottom;
	}

	//宽度为stretch的控件数
	int stretchCount = 0;
	//固定宽度的控件，总的高度
	int cxFixedTotal = 0;
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
		//计算宽度
		if( sz.cx == DUI_LENGTH_STRETCH ) {
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
		if (sz.cy == DUI_LENGTH_STRETCH) {
			sz.cy = (szAvailable.cy - rcMargin.top - rcMargin.bottom);
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
		itemsMap[pControl] = sz;
	}
	if (!itemsMap.empty()) {
		cxFixedTotal += ((int)itemsMap.size() - 1) * GetChildMarginX();
	}

	int cxStretch = 0;	
	if (stretchCount > 0) {
		cxStretch = std::max(0, (int)(szAvailable.cx - cxFixedTotal) / stretchCount);
	}
	//做一次预估：去除需要使用minwidth/maxwidth的控件数后，重新计算平均高度
	if ((cxStretch > 0) && !itemsMap.empty()) {
		for (auto iter = itemsMap.begin(); iter != itemsMap.end(); ++iter) {
			Control* pControl = iter->first;
			UiSize sz = iter->second;
			if (sz.cx == DUI_LENGTH_STRETCH) {
				sz.cx = cxStretch;
				if (sz.cx < pControl->GetMinWidth()) {
					sz.cx = pControl->GetMinWidth();
				}
				if (sz.cx > pControl->GetMaxWidth()) {
					sz.cx = pControl->GetMaxWidth();
				}
				if (sz.cx != cxStretch) {
					//这个控件需要使用min或者max宽度，从平均值中移除，按照Fixed控件算
					iter->second = sz;
					--stretchCount;
					cxFixedTotal += sz.cx; //Margin已经累加过，不需要重新累加
				}
			}
		}
	}
	//重新计算Stretch控件的宽度，最终以这次计算的为准；
	//如果横向总空间不足，则按原来评估的平均高度，优先保证前面的控件可以正常显示
	if ((stretchCount > 0) && ((int)szAvailable.cx > cxFixedTotal)) {
		cxStretch = std::max(0, ((int)szAvailable.cx - cxFixedTotal) / stretchCount);
	}

	// Position the elements
	int deviation = szAvailable.cx - cxFixedTotal - cxStretch * stretchCount;//剩余可用宽度，用于纠正偏差
	if (deviation < 0) {
		deviation = 0;
	}

	int iPosTop = rc.top;
	int iPosBottom = rc.bottom;
	int iPosX = rc.left;

	// Place elements
	int64_t cyNeeded = 0;//需要的总高度（取各个子控件的高度最大值）
	int64_t cxNeeded = 0;//需要的总宽度

	for(auto pControl : items) {
		if ((pControl == nullptr) || !pControl->IsVisible()) {
			continue;
		}
		if (pControl->IsFloat()) {
			SetFloatPos(pControl, rc);
			continue;
		}

		UiRect rcMargin = pControl->GetMargin();
		ASSERT(itemsMap.find(pControl) != itemsMap.end());
		UiSize sz = itemsMap[pControl];

		//计算宽度
		if( sz.cx == DUI_LENGTH_STRETCH ) {
			sz.cx = cxStretch;
			if (sz.cx < pControl->GetMinWidth()) {
				sz.cx = pControl->GetMinWidth();
			}
			if (sz.cx > pControl->GetMaxWidth()) {
				sz.cx = pControl->GetMaxWidth();
			}
			if ((sz.cx <= cxStretch) && (deviation > 0)) {
				sz.cx += 1;
				deviation--;
			}
		}

		//调整纵向对齐方式，确定Y轴坐标
		int childTop = 0;
		int childBottm = 0;
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

		UiRect rcChildPos(iPosX + rcMargin.left, childTop, iPosX + rcMargin.left + sz.cx, childBottm);
		pControl->SetPos(rcChildPos);
		cyNeeded = std::max(cyNeeded, (int64_t)rcChildPos.Height());

		iPosX += (sz.cx + rcMargin.left + GetChildMarginX() + rcMargin.right);
		cxNeeded += (sz.cx + rcMargin.left + rcMargin.right);
	}
	if (!itemsMap.empty()) {
		cxNeeded += ((int)itemsMap.size() - 1) * GetChildMarginX();
	}

	UiSize64 size(cxNeeded, cyNeeded);
	return size;
}

UiSize HLayout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	UiSize totalSize;
	UiSize itemSize;
	int estimateCount = 0;
	for(Control* pControl : items)	{
		if ((pControl == nullptr) || !pControl->IsVisible() || pControl->IsFloat()){
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
		
		totalSize.cy = std::max(itemSize.cy + rcMargin.top + rcMargin.bottom, totalSize.cy);
		totalSize.cx += (itemSize.cx + rcMargin.left + rcMargin.right);
	}

	if ((estimateCount - 1) > 0) {
		totalSize.cx += (estimateCount - 1) * GetChildMarginX();
	}

	UiRect rcPadding = GetPadding();
	totalSize.cx += (rcPadding.left + rcPadding.right);
	totalSize.cy += (rcPadding.top + rcPadding.bottom);
	return totalSize;
}

HBox::HBox() : Box(new HLayout())
{

}

std::wstring HBox::GetType() const { return DUI_CTR_HBOX; }

} //namespace ui

