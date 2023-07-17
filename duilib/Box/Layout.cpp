#include "Layout.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"

namespace ui 
{
Layout::Layout() :
	m_rcPadding(0, 0, 0, 0),
	m_iChildMarginX(0),
	m_iChildMarginY(0),
	m_pOwner(nullptr)
{

}

void Layout::SetOwner(Box* pOwner)
{
	m_pOwner = pOwner;
}

UiSize64 Layout::SetFloatPos(Control* pControl, UiRect rcContainer)
{
	ASSERT(pControl != nullptr);
	if ((pControl == nullptr) || (!pControl->IsVisible())) {
		return UiSize64();
	}
	if (rcContainer.Width() < 0) {
		rcContainer.right = rcContainer.left;
	}
	if (rcContainer.Height() < 0) {
		rcContainer.bottom = rcContainer.bottom;
	}

	int childLeft = 0;
	int childRight = 0;
	int childTop = 0;
	int childBottm = 0;
	UiRect rcMargin = pControl->GetMargin();
	int iPosLeft = rcContainer.left + rcMargin.left;
	int iPosRight = rcContainer.right - rcMargin.right;
	int iPosTop = rcContainer.top + rcMargin.top;
	int iPosBottom = rcContainer.bottom - rcMargin.bottom;
	UiSize szAvailable(iPosRight - iPosLeft, iPosBottom - iPosTop);
	UiSize childSize = pControl->EstimateSize(szAvailable);
	ASSERT(childSize.cx >= DUI_LENGTH_STRETCH);
	ASSERT(childSize.cy >= DUI_LENGTH_STRETCH);

	if (childSize.cx == DUI_LENGTH_STRETCH) {
		childSize.cx = std::max(0, (int)szAvailable.cx);
	}
	if (childSize.cx < pControl->GetMinWidth()) {
		childSize.cx = pControl->GetMinWidth();
	}
	if (childSize.cx > pControl->GetMaxWidth()) {
		childSize.cx = pControl->GetMaxWidth();
	}

	if (childSize.cy == DUI_LENGTH_STRETCH) {
		childSize.cy = std::max(0, (int)szAvailable.cy);
	}
	if (childSize.cy < pControl->GetMinHeight()) {
		childSize.cy = pControl->GetMinHeight();
	}
	if (childSize.cy > pControl->GetMaxHeight()) {
		childSize.cy = pControl->GetMaxHeight();
	}

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
	//TODO: 64ŒªºÊ»›–‘ºÏ≤È
	return UiSize64(childPos.Width(), childPos.Height());
}

bool Layout::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	bool hasAttribute = true;
	if (strName == L"padding") {
		UiRect rcPadding;
		AttributeUtil::ParseRectValue(strValue.c_str(), rcPadding);
		SetPadding(rcPadding, true);
	}
	else if ((strName == L"child_margin") || (strName == L"childmargin")) {
		int32_t iMargin = _wtoi(strValue.c_str());
		SetChildMargin(iMargin);
	}
	else if ((strName == L"child_margin_x") || (strName == L"childmarginx")) {
		int32_t iMargin = _wtoi(strValue.c_str());
		SetChildMarginX(iMargin);
	}
	else if ((strName == L"child_margin_y") || (strName == L"childmarginy")) {
		int32_t iMargin = _wtoi(strValue.c_str());
		SetChildMarginY(iMargin);
	}
	else {
		hasAttribute = false;
	}

	return hasAttribute;
}

UiSize64 Layout::ArrangeChild(const std::vector<Control*>& items, UiRect rc)
{
	UiSize64 size;
	for (Control* pControl : items) {
		if ((pControl == nullptr) || (!pControl->IsVisible())) {
			continue;
		}
		UiSize64 controlSize = SetFloatPos(pControl, rc);
		size.cx = std::max(size.cx, controlSize.cx);
		size.cy = std::max(size.cy, controlSize.cy);
	}
	return size;
}

UiSize Layout::EstimateSizeByChild(const std::vector<Control*>& items, UiSize szAvailable)
{
	UiSize maxSize;
	UiSize itemSize;
	for (Control* pControl : items) {
		if (pControl == nullptr) {
			continue;
		}
		if (!pControl->IsVisible() || pControl->IsFloat()) {
			continue;
		}
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
		itemSize.cx = std::max((int)itemSize.cx, 0);
		itemSize.cy = std::max((int)itemSize.cy, 0);

		maxSize.cx = std::max(itemSize.cx + pControl->GetMargin().left + pControl->GetMargin().right, maxSize.cx);
		maxSize.cy = std::max(itemSize.cy + pControl->GetMargin().top + pControl->GetMargin().bottom, maxSize.cy);
	}
	maxSize.cx += m_rcPadding.left + m_rcPadding.right;
	maxSize.cy += m_rcPadding.top + m_rcPadding.bottom;
	return maxSize;
}

void Layout::SetPadding(UiRect rcPadding, bool bNeedDpiScale /*= true*/)
{
	rcPadding.left = std::max((int)rcPadding.left, 0);
	rcPadding.right = std::max((int)rcPadding.right, 0);
	rcPadding.top = std::max((int)rcPadding.top, 0);
	rcPadding.bottom = std::max((int)rcPadding.bottom, 0);

	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleRect(rcPadding);
	}
	m_rcPadding = rcPadding;
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner != nullptr) {
		m_pOwner->Arrange();
	}
}

void Layout::SetChildMargin(int32_t iMargin)
{
	ASSERT(iMargin >= 0);
	iMargin = std::max(iMargin, 0);
	GlobalManager::Instance().Dpi().ScaleInt(iMargin);
	bool isChanged = (m_iChildMarginX != iMargin) || (m_iChildMarginY != iMargin);
	m_iChildMarginX = iMargin;
	m_iChildMarginY = iMargin;
	ASSERT(m_pOwner != nullptr);
	if (isChanged && (m_pOwner != nullptr)) {
		m_pOwner->Arrange();
	}
}

void Layout::SetChildMarginX(int32_t iMarginX)
{
	ASSERT(iMarginX >= 0);
	iMarginX = std::max(iMarginX, 0);
	GlobalManager::Instance().Dpi().ScaleInt(iMarginX);
	bool isChanged = (m_iChildMarginX != iMarginX);
	m_iChildMarginX = iMarginX;
	ASSERT(m_pOwner != nullptr);
	if (isChanged && (m_pOwner != nullptr)) {
		m_pOwner->Arrange();
	}
}

void Layout::SetChildMarginY(int32_t iMarginY)
{
	ASSERT(iMarginY >= 0);
	iMarginY = std::max(iMarginY, 0);
	GlobalManager::Instance().Dpi().ScaleInt(iMarginY);
	bool isChanged = (m_iChildMarginY != iMarginY);
	m_iChildMarginY = iMarginY;
	ASSERT(m_pOwner != nullptr);
	if (isChanged && (m_pOwner != nullptr)) {
		m_pOwner->Arrange();
	}
}

UiRect Layout::GetInternalPos() const
{
	ASSERT(m_pOwner != nullptr);
	if (m_pOwner == nullptr) {
		return UiRect();
	}
	UiRect internalPos = m_pOwner->GetPos();
	internalPos.Deflate(m_rcPadding.left, m_rcPadding.top, m_rcPadding.right, m_rcPadding.bottom);
	return internalPos;
}

} // namespace ui
