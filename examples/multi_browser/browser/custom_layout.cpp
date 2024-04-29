#include "custom_layout.h"

using namespace ui;

ui::UiSize64 CustomLayout::ArrangeChild(const std::vector<ui::Control*>& m_items, ui::UiRect rc)
{
	UiSize size;
	for (auto it = m_items.begin(); it != m_items.end(); it++) {
		Control* pControl = *it;
		UiSize new_size = this->SetCustomPos(pControl, rc);
		size.cx = std::max(size.cx, new_size.cx);
		size.cy = std::max(size.cy, new_size.cy);
	}
	return UiSize64(size.cx, size.cy);
}

UiSize CustomLayout::SetCustomPos(Control* pControl, UiRect containerRect)
{
	int childLeft = 0;
	int childRight = 0;
	int childTop = 0;
	int childBottm = 0;
	UiMargin rcMargin = pControl->GetMargin();
	int iPosLeft = containerRect.left + rcMargin.left;
	int iPosRight = containerRect.right - rcMargin.right;
	int iPosTop = containerRect.top + rcMargin.top;
	int iPosBottom = containerRect.bottom - rcMargin.bottom;
	UiSize szAvailable(iPosRight - iPosLeft, iPosBottom - iPosTop);
	UiEstSize estSize = pControl->EstimateSize(szAvailable);
	UiSize childSize(estSize.cx.GetInt32(), estSize.cy.GetInt32());
	if (estSize.cx.IsStretch()) {
		childSize.cx = std::max(0, szAvailable.cx);
	}
	if (childSize.cx < pControl->GetMinWidth()) childSize.cx = pControl->GetMinWidth();
	if (pControl->GetMaxWidth() >= 0 && childSize.cx > pControl->GetMaxWidth()) childSize.cx = pControl->GetMaxWidth();

	if (estSize.cy.IsStretch()) {
		childSize.cy = std::max(0, szAvailable.cy);
	}
	if (childSize.cy < pControl->GetMinHeight()) childSize.cy = pControl->GetMinHeight();
	if (childSize.cy > pControl->GetMaxHeight()) childSize.cy = pControl->GetMaxHeight();


	int childWidth = childSize.cx;
	int childHeight = childSize.cy;
	HorAlignType horAlignType = pControl->GetHorAlignType();
	VerAlignType verAlignType = pControl->GetVerAlignType();

	if (horAlignType == kHorAlignLeft) {
		childLeft = iPosLeft;
		childRight = childLeft + childWidth;
	}
	else if (horAlignType == kHorAlignRight) {
		childRight = iPosRight;
		childLeft = childRight - childWidth;
	}
	else if (horAlignType == kHorAlignCenter) {
		childLeft = iPosLeft + (iPosRight - iPosLeft - childWidth) / 2;
		childRight = childLeft + childWidth;
	}

	if (verAlignType == kVerAlignTop) {
		childTop = iPosTop;
		childBottm = childTop + childHeight;
	}
	else if (verAlignType == kVerAlignBottom) {
		childBottm = iPosBottom;
		childTop = childBottm - childHeight;
	}
	else if (verAlignType == kVerAlignCenter) {
		childTop = iPosTop + (iPosBottom - iPosTop - childHeight) / 2;
		childBottm = childTop + childHeight;
	}

	UiRect childPos(childLeft, childTop, childRight, childBottm);
	pControl->SetPos(childPos);
	return UiSize(childPos.Width(), childPos.Height());
}