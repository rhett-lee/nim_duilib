#include "PlaceHolder.h"
#include "duilib/Box/ScrollBox.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{

PlaceHolder::PlaceHolder() :
	m_pWindow(nullptr),
	m_cxyMin(0, 0),
	m_cxyMax(INT32_MAX, INT32_MAX),
	m_pParent(nullptr),
	m_horAlignType(kHorAlignLeft),
	m_verAlignType(kVerAlignTop),
	m_bFloat(false),
	m_bVisible(true),
	m_bIsArranged(true),
	m_bUseCache(false),
	m_bCacheDirty(true),
	m_bEnableControlPadding(true),
	m_bInited(false)
{
	//控件的高度和宽度值，默认设置为拉伸
	m_cxyFixed.cx.SetStretch();
	m_cxyFixed.cy.SetStretch();
}

PlaceHolder::~PlaceHolder()
{
}

std::wstring PlaceHolder::GetType() const { return L"PlaceHolder"; }

ui::Box* PlaceHolder::GetAncestor(const std::wstring& strName)
{
	Box* pAncestor = GetParent();
	while ((pAncestor != nullptr) && !pAncestor->IsNameEquals(strName)) {
		pAncestor = pAncestor->GetParent();
	}
	return pAncestor;
}

std::string PlaceHolder::GetUTF8Name() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(GetName(), strOut, CP_UTF8);
	return strOut;
}

std::wstring PlaceHolder::GetName() const
{ 
	return m_sName.c_str();
}

bool PlaceHolder::IsNameEquals(const std::wstring& name) const
{
	return wcscmp(m_sName.c_str(), name.c_str()) == 0;
}

void PlaceHolder::SetName(const std::wstring& strName)
{
	m_sName = strName;
}

void PlaceHolder::SetUTF8Name(const std::string& strName)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strName, strOut, CP_UTF8);
	SetName(strOut);
}

void PlaceHolder::SetParent(Box* pParent)
{
	m_pParent = pParent;
}

void PlaceHolder::SetWindow(Window* pManager)
{
	m_pWindow = pManager;
}

void PlaceHolder::Init()
{
	if (!m_bInited) {
		OnInit();
		m_bInited = true;
	}
}

bool PlaceHolder::IsInited() const
{
	return m_bInited;
}

void PlaceHolder::OnInit()
{
	m_bInited = true;
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

const UiFixedSize& PlaceHolder::GetFixedSize() const
{
	return m_cxyFixed;
}

const UiFixedInt& PlaceHolder::GetFixedHeight() const
{ 
	return m_cxyFixed.cy; 
}

const UiFixedInt& PlaceHolder::GetFixedWidth() const
{ 
	return m_cxyFixed.cx; 
}

void PlaceHolder::SetFixedWidth(UiFixedInt cx, bool bArrange, bool bNeedDpiScale)
{
	ASSERT(cx.IsValid());
	if (!cx.IsValid()) {
		return;
	}
	if (bNeedDpiScale && cx.IsInt32()) {
		GlobalManager::Instance().Dpi().ScaleInt(cx.value);
	}		

	if (m_cxyFixed.cx != cx) {
		m_cxyFixed.cx = cx;

		if (bArrange) {
			ArrangeAncestor();
		}
		else {
			SetReEstimateSize(true);
		}
	}
}

void PlaceHolder::SetFixedWidth64(int64_t cx64)
{
	SetFixedWidth(UiFixedInt(TruncateToInt32(cx64)), true, false);
}

void PlaceHolder::SetFixedHeight(UiFixedInt cy, bool bArrange, bool bNeedDpiScale)
{
	ASSERT(cy.IsValid());
	if (!cy.IsValid()) {
		return;
	}

	if (bNeedDpiScale && cy.IsInt32()) {
		GlobalManager::Instance().Dpi().ScaleInt(cy.value);
	}

	if (m_cxyFixed.cy != cy) {
		m_cxyFixed.cy = cy;

		if (bArrange) {
			ArrangeAncestor();
		}
		else {
			SetReEstimateSize(true);
		}
	}
}

void PlaceHolder::SetFixedHeight64(int64_t cy64)
{
	SetFixedHeight(UiFixedInt(TruncateToInt32(cy64)), true, true);
}

bool PlaceHolder::IsReEstimateSize(const UiSize& szAvailable) const
{ 
	if (!m_estResult.m_bReEstimateSize &&
		szAvailable.Equals(m_estResult.m_szAvailable)) {
		return false;
	}
	return true;
}

void PlaceHolder::SetReEstimateSize(bool bReEstimateSize)
{
	m_estResult.m_bReEstimateSize = bReEstimateSize;
}

const UiEstSize& PlaceHolder::GetEstimateSize() const
{ 
	return m_estResult.m_szEstimateSize;
}

void PlaceHolder::SetEstimateSize(const UiEstSize& szEstimateSize, const UiSize& szAvailable)
{
	m_estResult.m_szAvailable = szAvailable;
	m_estResult.m_szEstimateSize = szEstimateSize;
}

int32_t PlaceHolder::GetMinWidth() const
{ 
	ASSERT(m_cxyMin.cx >= 0); return m_cxyMin.cx; 
}

void PlaceHolder::SetMinWidth(int32_t cx, bool bNeedDpiScale)
{
	ASSERT(cx >= 0);
	if (cx < 0) {
		return;
	}
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(cx);
	}	
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

int32_t PlaceHolder::GetMaxWidth() const
{ 
	ASSERT(m_cxyMax.cx >= 0); return m_cxyMax.cx; 
}

void PlaceHolder::SetMaxWidth(int32_t cx, bool bNeedDpiScale)
{
	ASSERT(cx >= 0);
	if (cx < 0) {
		return;
	}
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(cx);
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
int32_t PlaceHolder::GetMinHeight() const
{ 
	ASSERT(m_cxyMin.cy >= 0); return m_cxyMin.cy; 
}

void PlaceHolder::SetMinHeight(int32_t cy, bool bNeedDpiScale)
{
	ASSERT(cy >= 0);
	if (cy < 0) {
		return;
	}
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(cy);
	}	
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

int32_t PlaceHolder::GetMaxHeight() const
{ 
	ASSERT(m_cxyMax.cy >= 0); return m_cxyMax.cy; 
}

void PlaceHolder::SetMaxHeight(int32_t cy, bool bNeedDpiScale)
{
	ASSERT(cy >= 0);
	if (cy < 0) {
		return;
	}
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(cy);
	}	
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
	if (m_horAlignType != horAlignType) {
		m_horAlignType = TruncateToInt8(horAlignType);
		if (!m_bFloat) {
			ArrangeAncestor();
		}
		else {
			Arrange();
		}
	}
}

HorAlignType PlaceHolder::GetHorAlignType() const
{ 
	return static_cast<HorAlignType>(m_horAlignType);
}

void PlaceHolder::SetVerAlignType(VerAlignType verAlignType)
{
	if (m_verAlignType != verAlignType) {
		m_verAlignType = TruncateToInt8(verAlignType);
		if (!m_bFloat) {
			ArrangeAncestor();
		}
		else {
			Arrange();
		}
	}	
}

VerAlignType PlaceHolder::GetVerAlignType() const
{ 
	return static_cast<VerAlignType>(m_verAlignType);
}

UiMargin PlaceHolder::GetMargin() const
{
	return UiMargin(m_rcMargin.left, m_rcMargin.top, m_rcMargin.right, m_rcMargin.bottom);
}

void PlaceHolder::SetMargin(UiMargin rcMargin, bool bNeedDpiScale)
{
	ASSERT((rcMargin.left >= 0) && (rcMargin.top >= 0) && (rcMargin.right >= 0) && (rcMargin.bottom >= 0));
	rcMargin.Validate();
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleMargin(rcMargin);
	}

	if (!GetMargin().Equals(rcMargin)) {
		m_rcMargin.left = TruncateToUInt16(rcMargin.left);
		m_rcMargin.top = TruncateToUInt16(rcMargin.top);
		m_rcMargin.right = TruncateToUInt16(rcMargin.right);
		m_rcMargin.bottom = TruncateToUInt16(rcMargin.bottom);
		ArrangeAncestor();
	}
}

UiPadding PlaceHolder::GetPadding() const
{
	return UiPadding(m_rcPadding.left, m_rcPadding.top, m_rcPadding.right, m_rcPadding.bottom);
}

void PlaceHolder::SetPadding(UiPadding rcPadding, bool bNeedDpiScale /*= true*/)
{
	ASSERT((rcPadding.left >= 0) && (rcPadding.top >= 0) && (rcPadding.right >= 0) && (rcPadding.bottom >= 0));
	rcPadding.Validate();
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScalePadding(rcPadding);
	}
	if (!GetPadding().Equals(rcPadding)) {
		m_rcPadding.left = TruncateToUInt16(rcPadding.left);
		m_rcPadding.top = TruncateToUInt16(rcPadding.top);
		m_rcPadding.right = TruncateToUInt16(rcPadding.right);
		m_rcPadding.bottom = TruncateToUInt16(rcPadding.bottom);
		ArrangeAncestor();
	}
}

void PlaceHolder::SetEnableControlPadding(bool bEnable)
{
	m_bEnableControlPadding = bEnable;
}

bool PlaceHolder::IsEnableControlPadding() const
{
	return m_bEnableControlPadding;
}

UiPadding PlaceHolder::GetControlPadding() const
{
	//控件本身禁止应用内边距时，返回空
	UiPadding rcPadding;
	if (IsEnableControlPadding()) {		
		rcPadding = GetPadding();
	}
	return rcPadding;
}

void PlaceHolder::Arrange()
{
	if (GetFixedWidth().IsAuto() || GetFixedHeight().IsAuto()) {
		ArrangeAncestor();
	}
	else {
		ArrangeSelf();
	}
}

void PlaceHolder::ArrangeAncestor()
{
	SetReEstimateSize(true);
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
		while (parent && (parent->GetFixedWidth().IsAuto() || parent->GetFixedHeight().IsAuto())) {
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
	SetReEstimateSize(true);
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

void PlaceHolder::SetPos(UiRect rc)
{ 
	SetRect(rc);
}

void PlaceHolder::SetArranged(bool bArranged)
{ 
	m_bIsArranged = bArranged; 
}

void PlaceHolder::SetRect(const UiRect& rc)
{
	//所有调整矩形区域的操作，最终都会通过这里设置
	if (!m_uiRect.Equals(rc)) {
		//区域变化，标注绘制缓存脏标记位
		SetCacheDirty(true);
	}
	m_uiRect = rc;	
}

void PlaceHolder::Invalidate()
{
	if (!IsVisible()) {
		return;
	}

	SetCacheDirty(true);
	UiRect rcInvalidate = GetPos();	
	ui::UiPoint scrollBoxOffset = GetScrollOffsetInScrollBox();
	rcInvalidate.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);
	if (m_pWindow != nullptr) {
		m_pWindow->Invalidate(rcInvalidate);
	}
}

void PlaceHolder::InvalidateRect(const UiRect& rc)
{
	if (!IsVisible()) {
		return;
	}

	SetCacheDirty(true);
	UiRect rcInvalidate = GetPos();
	if (!rc.IsEmpty()) {
		//取交集
		rcInvalidate.Intersect(rc);
	}	
	ui::UiPoint scrollBoxOffset = GetScrollOffsetInScrollBox();
	rcInvalidate.Offset(-scrollBoxOffset.x, -scrollBoxOffset.y);
	if (m_pWindow != nullptr) {
		m_pWindow->Invalidate(rcInvalidate);
	}
}

void PlaceHolder::RelayoutOrRedraw()
{
	if ((GetFixedWidth().IsAuto()) || (GetFixedHeight().IsAuto())) {
		//如果当前控件的宽高有的是AUTO的，需要父控件Box进行布局重排（一般在可能引起布局变化时调用），布局重排后会进行重绘
		ArrangeAncestor();
	}
	else {
		//仅仅进行重绘制
		Invalidate();
	}
}

UiPoint PlaceHolder::GetScrollOffsetInScrollBox() const
{
	UiPoint scrollPos;
	Control* parent = GetParent();
	while (parent != nullptr) {
		ScrollBox* pScrollBox = dynamic_cast<ScrollBox*>(parent);
		if ((pScrollBox != nullptr) &&
			(pScrollBox->IsVScrollBarValid() || pScrollBox->IsHScrollBarValid())) {
			//此父控件是ScrollBox，并且父控件存在横向滚动条或者纵向滚动条
			if (IsFloat() && (pScrollBox == GetParent())) {
				//当前控件是浮动的，父控件是ScrollBox，不计入统计
			}
			else {
				scrollPos.x += pScrollBox->GetScrollOffset().cx;
				scrollPos.y += pScrollBox->GetScrollOffset().cy;
			}
		}
		parent = parent->GetParent();
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