#include "PlaceHolder.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/DpiManager.h"

namespace ui
{

PlaceHolder::PlaceHolder() :
	m_pWindow(nullptr),
	m_sName(),
	m_cxyFixed(DUI_LENGTH_STRETCH, DUI_LENGTH_STRETCH),
	m_cxyMin(-1, -1),
	m_cxyMax(9999999, 9999999),
	m_pParent(nullptr),
	m_horAlignType(kHorAlignLeft),
	m_verAlignType(kVerAlignTop),
	m_bFloat(false),
	m_bReEstimateSize(true),
	m_bVisible(true),
	m_bIsArranged(true),
	m_bUseCache(false),
	m_bCacheDirty(false)
{
}

PlaceHolder::~PlaceHolder()
{	
}

ui::Box* PlaceHolder::GetAncestor(const std::wstring& strName)
{
	Box* pAncestor = this->GetParent();
	while ((pAncestor != nullptr) && pAncestor->GetName() != strName) {
		pAncestor = pAncestor->GetParent();
	}
	return pAncestor;
}

std::string PlaceHolder::GetUTF8Name() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(m_sName, strOut, CP_UTF8);
	return strOut;
}

void PlaceHolder::SetName(const std::wstring& strName)
{
	m_sName = strName;
}

void PlaceHolder::SetUTF8Name(const std::string& strName)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strName, strOut, CP_UTF8);
	m_sName = strOut;
}

void PlaceHolder::SetWindow(Window* pManager, Box* pParent, bool bInit)
{
	m_pWindow = pManager;
	m_pParent = pParent;
	if (bInit && m_pParent) {
		Init();
	}
}

void PlaceHolder::SetWindow(Window* pManager)
{
	m_pWindow = pManager;
}

void PlaceHolder::Init()
{
	DoInit();
}

void PlaceHolder::DoInit()
{

}

void PlaceHolder::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;
}

void PlaceHolder::SetFloat(bool bFloat)
{
	if (m_bFloat == bFloat) {
		return;
	}
	m_bFloat = bFloat;
	ArrangeAncestor();
}

void PlaceHolder::SetFixedWidth(int cx, bool bArrange, bool bNeedDpiScale)
{
	if (cx < 0 && cx != DUI_LENGTH_STRETCH && cx != DUI_LENGTH_AUTO) {
		ASSERT(FALSE);
		return;
	}

	if (bNeedDpiScale && cx > 0) {
		DpiManager::GetInstance()->ScaleInt(cx);
	}		

	if (m_cxyFixed.cx != cx) {
		m_cxyFixed.cx = cx;

		if (bArrange) {
			ArrangeAncestor();
		}
		else {
			m_bReEstimateSize = true;
		}
	}
}

void PlaceHolder::SetFixedHeight(int cy, bool bNeedDpiScale)
{
	if (cy < 0 && cy != DUI_LENGTH_STRETCH && cy != DUI_LENGTH_AUTO) {
		ASSERT(FALSE);
		return;
	}

	if (bNeedDpiScale && cy > 0) {
		DpiManager::GetInstance()->ScaleInt(cy);
	}

	if (m_cxyFixed.cy != cy) {
		m_cxyFixed.cy = cy;
		ArrangeAncestor();
	}
}

void PlaceHolder::SetMinWidth(int cx)
{
	if (cx < 0) {
		return;
	}
	DpiManager::GetInstance()->ScaleInt(cx);
	if (m_cxyMin.cx == cx) {
		return;
	}
	m_cxyMin.cx = cx;
	if (!m_bFloat) {
		ArrangeAncestor();
	}
	else {
		Arrange();
	}
}

void PlaceHolder::SetMaxWidth(int cx)
{
	//可能是 DUI_LENGTH_AUTO 或者 DUI_LENGTH_STRETCH
	if (cx > 0) {
		DpiManager::GetInstance()->ScaleInt(cx);
	}	
	if (m_cxyMax.cx == cx) {
		return;
	}

	m_cxyMax.cx = cx;
	if (!m_bFloat) {
		ArrangeAncestor();
	}
	else {
		Arrange();
	}
}

void PlaceHolder::SetMinHeight(int cy)
{
	if (cy < 0) {
		return;
	}
	DpiManager::GetInstance()->ScaleInt(cy);
	if (m_cxyMin.cy == cy) {
		return;
	}
	m_cxyMin.cy = cy;
	if (!m_bFloat) {
		ArrangeAncestor();
	}
	else {
		Arrange();
	}
}

void PlaceHolder::SetMaxHeight(int cy)
{
	DpiManager::GetInstance()->ScaleInt(cy);
	if (m_cxyMax.cy == cy) {
		return;
	}

	m_cxyMax.cy = cy;
	if (!m_bFloat) {
		ArrangeAncestor();
	}
	else {
		Arrange();
	}
}

void PlaceHolder::SetHorAlignType(HorAlignType horAlignType)
{
	m_horAlignType = horAlignType;
}

void PlaceHolder::SetVerAlignType(VerAlignType verAlignType)
{
	m_verAlignType = verAlignType;
}

void PlaceHolder::SetReEstimateSize(bool bReEstimateSize)
{
	m_bReEstimateSize = bReEstimateSize;
}

void PlaceHolder::Arrange()
{
	if (GetFixedWidth() == DUI_LENGTH_AUTO || GetFixedHeight() == DUI_LENGTH_AUTO) {
		ArrangeAncestor();
	}
	else {
		ArrangeSelf();
	}
}

void PlaceHolder::ArrangeAncestor()
{
	m_bReEstimateSize = true;
	if ((m_pWindow == nullptr) || (m_pWindow->GetRoot() == nullptr)) {
		if (GetParent()) {
			GetParent()->ArrangeSelf();
		}
		else {
			ArrangeSelf();
		}
	}
	else {
		Control* parent = GetParent();
		while (parent && (parent->GetFixedWidth() == DUI_LENGTH_AUTO || parent->GetFixedHeight() == DUI_LENGTH_AUTO)) {
			parent->SetReEstimateSize(true);
			parent = parent->GetParent();
		}
		if (parent) {
			parent->ArrangeSelf();
		}
		else {
			//说明root具有AutoAdjustSize属性
			m_pWindow->GetRoot()->ArrangeSelf();
		}
	}

}

void PlaceHolder::ArrangeSelf()
{
	if (!IsVisible()) {
		return;
	}
	m_bReEstimateSize = true;
	m_bIsArranged = true;
	Invalidate();

	if (m_pWindow != nullptr) {
		m_pWindow->SetArrange(true);
	}
}

void PlaceHolder::SetUseCache(bool cache)
{
	m_bUseCache = cache;
}

void PlaceHolder::SetCacheDirty(bool dirty)
{
	m_bCacheDirty = dirty;
}

void PlaceHolder::Invalidate()
{
	if (!IsVisible()) {
		return;
	}

	SetCacheDirty(true);
	UiRect rcInvalidate = GetPosWithScrollOffset(true);
	if (m_pWindow != nullptr) {
		m_pWindow->Invalidate(rcInvalidate);
	}
}

UiRect PlaceHolder::GetPosWithScrollOffset(bool bContainShadow) const
{
	UiRect pos = GetPos(bContainShadow);
	UiPoint offset = GetScrollOffset();
	pos.Offset(-offset.x, -offset.y);
	return pos;
}

UiPoint PlaceHolder::GetScrollOffset() const
{
	UiPoint scrollPos;
	Control* parent = GetParent();
	ScrollableBox* lbParent = dynamic_cast<ScrollableBox*>(parent);
	if (lbParent && (lbParent->IsVScrollBarValid() || lbParent->IsHScrollBarValid()) && IsFloat()) {
		return scrollPos;
	}

	while (parent != nullptr) {
		ScrollableBox* listbox = dynamic_cast<ScrollableBox*>(parent);
		while ( (parent != nullptr) && 
			    ((listbox == nullptr) || (!listbox->IsVScrollBarValid() && !listbox->IsHScrollBarValid()))) {
			parent = parent->GetParent();
			listbox = dynamic_cast<ScrollableBox*>(parent);
		}

		if (parent != nullptr) {
			//说明控件在Listbox内部			
			if (listbox != nullptr) {
				scrollPos.x += listbox->GetScrollPos().cx;
				scrollPos.y += listbox->GetScrollPos().cy;
			}			
			parent = parent->GetParent();
		}
	}

	return scrollPos;
}

bool PlaceHolder::IsChild(PlaceHolder* pAncestor, PlaceHolder* pControl) const
{
	while (pControl && pControl != pAncestor) {
		pControl = pControl->GetParent();
	}
	return pControl != nullptr;
}

}