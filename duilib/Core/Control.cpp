#include "Control.h"
#include "duilib/Image/Image.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/BitmapHelper.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/DpiManager.h"
#include "duilib/Utils/MultiLangSupport.h"
#include "duilib/Utils/TimerManager.h"
#include <tchar.h>

namespace ui 
{
Control::Control() :
	m_OnXmlEvent(),
	m_OnEvent(),
	m_bContextMenuUsed(false),
	m_bEnabled(true),
	m_bMouseEnabled(true),
	m_bKeyboardEnabled(true),
	m_bMouseFocused(false),
	m_bNoFocus(false),
	m_bClip(true),
	m_bGifPlay(true),
	m_bAllowTabstop(true),
    m_bIsLoading(false),
	m_renderOffset(),
	m_cxyBorderRound(),
	m_rcMargin(),
	m_rcPaint(),
	m_rcBorderSize(),
	m_cursorType(kCursorArrow),
	m_controlState(kControlStateNormal),
	m_nBorderSize(0),
	m_nTooltipWidth(300),
	m_nAlpha(255),
	m_nHotAlpha(0),
    m_fCurrrentAngele(0),
	m_sToolTipText(),
	m_sToolTipTextId(),
	m_sUserDataID(),
	m_strBkColor(),
	m_strBorderColor(),
	m_gifWeakFlag(),	
	m_loadBkImageWeakFlag(),
    m_loadingImageFlag(),
#if defined(ENABLE_UIAUTOMATION)
	m_pUIAProvider(nullptr),
#endif
	m_boxShadow()
{
	m_colorMap = std::make_unique<StateColorMap>();
	m_imageMap = std::make_unique<StateImageMap>();
	m_bkImage = std::make_unique<Image>();
	m_loadingImage = std::make_unique<Image>();

	m_animationManager = std::make_unique<AnimationManager>(),
	m_colorMap->SetControl(this);
	m_imageMap->SetControl(this);
	m_animationManager->Init(this);
}

AnimationManager& Control::GetAnimationManager() const
{ 
	return *m_animationManager;
}

Control::~Control()
{
	//清理动画相关资源，避免定时器再产生回调，引发错误
	m_animationManager->Clear(this);
	m_animationManager.reset();

	SendEvent(kEventLast);

	Window* pWindow = GetWindow();
	if (pWindow) {
		pWindow->ReapObjects(this);
	}

#if defined(ENABLE_UIAUTOMATION)
	if (nullptr != m_pUIAProvider) {
		// Coz UiaDisconnectProviderd require at least win8
		// UiaDisconnectProvider(m_pUIAProvider);
		m_pUIAProvider->ResetControl();
		m_pUIAProvider->Release();
		m_pUIAProvider = nullptr;
	}
#endif
}

std::wstring Control::GetType() const { return _T("Control"); }

void Control::SetBkColor(const std::wstring& strColor)
{
	ASSERT(strColor.empty() || this->GetWindowColor(strColor).GetARGB() != 0);
	if (m_strBkColor == strColor) {
		return;
	}
	m_strBkColor = strColor;
	Invalidate();
}

std::wstring Control::GetStateColor(ControlStateType stateType) const
{
	return m_colorMap->GetStateColor(stateType);
}

void Control::SetStateColor(ControlStateType stateType, const std::wstring& strColor)
{
	ASSERT(this->GetWindowColor(strColor).GetARGB() != 0);
	if (m_colorMap->GetStateColor(stateType) == strColor) {
		return;
	}

	if (stateType == kControlStateHot) {
		m_animationManager->SetFadeHot(true);
	}
	m_colorMap->SetStateColor(stateType, strColor);
	Invalidate();
}

std::wstring Control::GetBkImage() const
{
	return m_bkImage->GetImageString();
}

std::string Control::GetUTF8BkImage() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(m_bkImage->GetImageString().c_str(), strOut, CP_UTF8);
	return strOut;
}

void Control::SetBkImage(const std::wstring& strImage)
{
	StopGifPlay();
	m_bkImage->SetImageString(strImage);
	m_bGifPlay = m_bkImage->GetImageAttribute().nPlayCount != 0;
	if (GetFixedWidth() == DUI_LENGTH_AUTO || GetFixedHeight() == DUI_LENGTH_AUTO) {
		ArrangeAncestor();
	}
	else {
		Invalidate();
	}
}

void Control::SetUTF8BkImage(const std::string& strImage)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strImage, strOut, CP_UTF8);
	SetBkImage(strOut);
}

std::wstring Control::GetLoadingImage() const 
{
	return m_loadingImage->GetImageString();
}

void Control::SetLoadingImage(const std::wstring& strImage) 
{
	StopGifPlay();
	m_loadingImage->SetImageString(strImage);
	Invalidate();
}

void Control::SetLoadingBkColor(const std::wstring& strColor) 
{
    if (m_strLoadingBkColor == strColor) {
        return;
    }
    m_strLoadingBkColor = strColor;
    Invalidate();
}

std::wstring Control::GetStateImage(ControlStateType stateType)
{
	return m_imageMap->GetImageString(kStateImageBk, stateType);
}

void Control::SetStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (stateType == kControlStateHot) {
		m_animationManager->SetFadeHot(true);
	}
	m_imageMap->SetImageString(kStateImageBk, stateType, strImage);
	if (GetFixedWidth() == DUI_LENGTH_AUTO || GetFixedHeight() == DUI_LENGTH_AUTO) {
		ArrangeAncestor();
	}
	else {
		Invalidate();
	}
}

std::wstring Control::GetForeStateImage(ControlStateType stateType)
{
	return m_imageMap->GetImageString(kStateImageFore, stateType);
}

void Control::SetForeStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (stateType == kControlStateHot) {
		m_animationManager->SetFadeHot(true);
	}
	m_imageMap->SetImageString(kStateImageFore, stateType, strImage);
	Invalidate();
}

ControlStateType Control::GetState() const
{
	return m_controlState;
}

void Control::SetState(ControlStateType controlState)
{
	if (controlState == kControlStateNormal) {
		m_nHotAlpha = 0;
	}
	else if (controlState == kControlStateHot) {
		m_nHotAlpha = 255;
	}
	m_controlState = controlState;
	Invalidate();
}

Image* Control::GetEstimateImage()
{
	Image* estimateImage = nullptr;
	if (!m_bkImage->GetImagePath().empty()) {
		estimateImage = m_bkImage.get();
	}
	else {
		estimateImage = m_imageMap->GetEstimateImage(kStateImageBk);
	}

	return estimateImage;
}

int Control::GetBorderSize() const
{
	return m_nBorderSize;
}

void Control::SetBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	if (m_nBorderSize == nSize) {
		return;
	}

	m_nBorderSize = nSize;
	Invalidate();
}

const std::wstring& Control::GetBorderColor() const
{
    return m_strBorderColor;
}

void Control::SetBorderColor(const std::wstring& strBorderColor)
{
	if (m_strBorderColor == strBorderColor) {
		return;
	}
    m_strBorderColor = strBorderColor;
    Invalidate();
}

void Control::SetBorderSize(UiRect rc)
{
	DpiManager::GetInstance()->ScaleRect(rc);
	m_rcBorderSize = rc;
	Invalidate();
}

int Control::GetLeftBorderSize() const
{
	return m_rcBorderSize.left;
}

void Control::SetLeftBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	m_rcBorderSize.left = nSize;
	Invalidate();
}

int Control::GetTopBorderSize() const
{
	return m_rcBorderSize.top;
}

void Control::SetTopBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	m_rcBorderSize.top = nSize;
	Invalidate();
}

int Control::GetRightBorderSize() const
{
	return m_rcBorderSize.right;
}

void Control::SetRightBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	m_rcBorderSize.right = nSize;
	Invalidate();
}

int Control::GetBottomBorderSize() const
{
	return m_rcBorderSize.bottom;
}

void Control::SetBottomBorderSize(int nSize)
{
	DpiManager::GetInstance()->ScaleInt(nSize);
	m_rcBorderSize.bottom = nSize;
	Invalidate();
}

const UiSize& Control::GetBorderRound() const
{
    return m_cxyBorderRound;
}

void Control::SetBorderRound(UiSize cxyRound)
{
	DpiManager::GetInstance()->ScaleSize(cxyRound);
    m_cxyBorderRound = cxyRound;
    Invalidate();
}

void Control::SetBoxShadow(const std::wstring& strShadow)
{
	m_boxShadow.SetBoxShadowString(strShadow);
}

CursorType Control::GetCursorType() const
{
	return m_cursorType;
}

void Control::SetCursorType(CursorType cursorType)
{
	m_cursorType = cursorType;
}

std::wstring Control::GetToolTipText() const
{
	std::wstring strText = m_sToolTipText;
	if (strText.empty() && !m_sToolTipTextId.empty()) {
		strText = MultiLangSupport::GetInstance()->GetStringViaID(m_sToolTipTextId);
	}
	return strText;
}

std::string Control::GetUTF8ToolTipText() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(GetToolTipText(), strOut, CP_UTF8);
	return strOut;
}

void Control::SetToolTipText(const std::wstring& strText)
{
	std::wstring strTemp(strText);
	StringHelper::ReplaceAll(_T("<n>"),_T("\r\n"), strTemp);
	m_sToolTipText = strTemp;

	Invalidate();
}

void Control::SetUTF8ToolTipText(const std::string& strText)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strText, strOut, CP_UTF8);
	if (strOut.empty()) {
		m_sToolTipText = _T("");
		Invalidate();
		return ;
	}

	if (m_sToolTipText != strOut) {
		SetToolTipText(strOut);
	}
}

void Control::SetToolTipTextId(const std::wstring& strTextId)
{
	if (m_sToolTipTextId == strTextId) {
		return;
	}
	m_sToolTipTextId = strTextId;
	Invalidate();
}

void Control::SetUTF8ToolTipTextId(const std::string& strTextId)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strTextId, strOut, CP_UTF8);
	SetToolTipTextId(strOut);
}

void Control::SetToolTipWidth( int nWidth )
{
	DpiManager::GetInstance()->ScaleInt(nWidth);
	m_nTooltipWidth = nWidth;
}

int Control::GetToolTipWidth(void) const
{
	return m_nTooltipWidth;
}

void Control::SetContextMenuUsed(bool bMenuUsed)
{
	m_bContextMenuUsed = bMenuUsed;
}

std::wstring Control::GetDataID() const
{
    return m_sUserDataID;
}

std::string Control::GetUTF8DataID() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(m_sUserDataID, strOut, CP_UTF8);
	return strOut;
}

void Control::SetDataID(const std::wstring& strText)
{
	m_sUserDataID = strText;
}

void Control::SetUTF8DataID(const std::string& strText)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strText, strOut, CP_UTF8);
	m_sUserDataID = strOut;
}

void Control::SetFadeVisible(bool bVisible)
{
	if (bVisible) {
		m_animationManager->Appear();
	}
	else {
		m_animationManager->Disappear();
	}
}

void Control::SetVisible(bool bVisible)
{
	if (IsVisible() == bVisible) {
		return;
	}
	bool v = IsVisible();
	__super::SetVisible(bVisible);

	if (!IsVisible()) {
		EnsureNoFocus();
	}

	if (IsVisible() != v) {
		ArrangeAncestor();
	}

	if (!IsVisible()) {
		StopGifPlay();
	}

	SendEvent(kEventVisibleChange);
}

void Control::SetEnabled(bool bEnabled)
{
	if (m_bEnabled == bEnabled) {
		return;
	}

    m_bEnabled = bEnabled;
	if (m_bEnabled) {
		m_controlState = kControlStateNormal;
		m_nHotAlpha = 0;
	}
	else {
		m_controlState = kControlStateDisabled;
	}
    Invalidate();
}

void Control::SetMouseEnabled(bool bEnabled)
{
    m_bMouseEnabled = bEnabled;
}

void Control::SetKeyboardEnabled(bool bEnabled)
{
	m_bKeyboardEnabled = bEnabled ; 
}

bool Control::IsFocused() const
{
	Window* pWindow = GetWindow();
    return ((pWindow != nullptr) && (pWindow->GetFocus() == this) );
}

void Control::SetFocus()
{
	if (m_bNoFocus) {
		return;
	}
	Window* pWindow = GetWindow();
	if (pWindow != nullptr) {
		pWindow->SetFocus(this);
	}
}

UINT Control::GetControlFlags() const
{
	return IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

void Control::SetNoFocus()
{
    m_bNoFocus = true;
	EnsureNoFocus();
}

void Control::Activate()
{

}

bool Control::IsActivatable() const
{
	if (!IsVisible() || !IsEnabled()) {
		return false;
	}
	return true;
}

Control* Control::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags, UiPoint /*scrollPos*/)
{
	if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
		return nullptr;
	}
	if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
		return nullptr;
	}
	if ((uFlags & UIFIND_HITTEST) != 0 && 
		(!m_bMouseEnabled || ((pData != nullptr) && !::PtInRect(&GetRect(), *static_cast<LPPOINT>(pData))))) {
		return nullptr;
	}
    return Proc(this, pData);
}

UiRect Control::GetPos(bool bContainShadow) const
{
	UiRect pos = GetRect();
	Window* pWindow = GetWindow();
	if (pWindow && !bContainShadow) {
		UiRect shadowLength = pWindow->GetShadowCorner();
		pos.Offset(-shadowLength.left, -shadowLength.top);
	}
	return pos;
}

void Control::SetPos(UiRect rc)
{
	if (rc.right < rc.left) {
		rc.right = rc.left;
	}
	if (rc.bottom < rc.top) {
		rc.bottom = rc.top;
	}

	if (GetRect().Equal(rc)) {
		SetArranged(false);
		return;
	}

	UiRect invalidateRc = GetRect();
	if (::IsRectEmpty(&invalidateRc)) {
		invalidateRc = rc;
	}

	SetRect(rc);
	if (GetWindow() == nullptr) {
		return;
	}

	SetArranged(false);
	invalidateRc.Union(GetRect());

	bool needInvalidate = true;
	UiRect rcTemp;
	UiRect rcParent;
	UiPoint offset = GetScrollOffset();
	invalidateRc.Offset(-offset.x, -offset.y);
	Control* pParent = GetParent();
	while (pParent != nullptr) {
		rcTemp = invalidateRc;
		rcParent = pParent->GetPos();
		if (!::IntersectRect(&invalidateRc, &rcTemp, &rcParent)) {
			needInvalidate = false;
			break;
		}
		pParent = pParent->GetParent();
	}
	if (needInvalidate && (GetWindow() != nullptr)) {
		GetWindow()->Invalidate(invalidateRc);
	}

	SendEvent(kEventResize);
}

UiRect Control::GetMargin() const
{
	return m_rcMargin;
}

void Control::SetMargin(UiRect rcMargin, bool bNeedDpiScale)
{
	if (bNeedDpiScale)
		DpiManager::GetInstance()->ScaleRect(rcMargin);

	if (!m_rcMargin.Equal(rcMargin)) {
		m_rcMargin = rcMargin;
		ArrangeAncestor();
	}
}

UiSize Control::EstimateSize(UiSize szAvailable)
{
	UiSize imageSize = GetFixedSize();
	if (GetFixedWidth() == DUI_LENGTH_AUTO || GetFixedHeight() == DUI_LENGTH_AUTO) {
		if (!IsReEstimateSize()) {
			return GetEstimateSize();
		}
		Image* image = GetEstimateImage();
		if (image) {
			auto imageAttribute = image->GetImageAttribute();
			if (imageAttribute.rcSource.left != DUI_NOSET_VALUE && imageAttribute.rcSource.top != DUI_NOSET_VALUE
				&& imageAttribute.rcSource.right != DUI_NOSET_VALUE && imageAttribute.rcSource.bottom != DUI_NOSET_VALUE) {
				if ((GetFixedWidth() != imageAttribute.rcSource.right - imageAttribute.rcSource.left)) {
					SetFixedWidth(imageAttribute.rcSource.right - imageAttribute.rcSource.left, true, true);
				}
				if ((GetFixedHeight() != imageAttribute.rcSource.bottom - imageAttribute.rcSource.top)) {
					SetFixedHeight(imageAttribute.rcSource.bottom - imageAttribute.rcSource.top, true);
				}
				return GetFixedSize();
			}

			LoadImageData(*image);
			std::shared_ptr<ImageInfo> imageCache = image->GetImageCache();
			if (imageCache) {
				if (GetFixedWidth() == DUI_LENGTH_AUTO) {
					int image_width = imageCache->GetWidth();
					imageSize.cx = image_width;
				}
				if (GetFixedHeight() == DUI_LENGTH_AUTO) {
					int image_height = imageCache->GetHeight();
					imageSize.cy = image_height;
				}
			}
		}

		SetReEstimateSize(false);
		bool bReEstimateSize = IsReEstimateSize();
		UiSize textSize = EstimateText(szAvailable, bReEstimateSize);
		SetReEstimateSize(bReEstimateSize);
		if (GetFixedWidth() == DUI_LENGTH_AUTO && imageSize.cx < textSize.cx) {
			imageSize.cx = textSize.cx;
		}
		if (GetFixedHeight() == DUI_LENGTH_AUTO && imageSize.cy < textSize.cy) {
			imageSize.cy = textSize.cy;
		}
		SetEstimateSize(imageSize);
	}

	return imageSize;
}

UiSize Control::EstimateText(UiSize /*szAvailable*/, bool& /*bReEstimateSize*/)
{
	return UiSize();
}

bool Control::IsPointInWithScrollOffset(const UiPoint& point) const
{
	UiPoint scrollOffset = GetScrollOffset();
	UiPoint newPoint = point;
	newPoint.Offset(scrollOffset);
	return GetRect().IsPointIn(newPoint);
}

#if defined(ENABLE_UIAUTOMATION)
UIAControlProvider* Control::GetUIAProvider()
{
	if (m_pUIAProvider == nullptr)
	{
		m_pUIAProvider = new (std::nothrow) UIAControlProvider(this);
	}
	return m_pUIAProvider;
}
#endif;

void Control::SendEvent(EventType eventType, 
					    WPARAM wParam, 
					    LPARAM lParam, 
					    TCHAR tChar, 
					    const UiPoint& mousePos)
{
	EventArgs msg;
	msg.pSender = this;
	msg.Type = eventType;
	msg.chKey = tChar;
	msg.wParam = wParam;
	msg.lParam = lParam;
	if ((mousePos.x == 0) && (mousePos.y == 0)) {
		Window* pWindow = GetWindow();
		if (pWindow != nullptr) {
			msg.ptMouse = pWindow->GetLastMousePos();
		}
	}
	else {
		msg.ptMouse = mousePos;
	}
	msg.dwTimestamp = ::GetTickCount();

	SendEvent(msg);
}

void Control::SendEvent(const EventArgs& msg)
{
	bool bRet = FireAllEvents(msg);	
    if(bRet) {
		HandleEvent(msg);
	}
}

void Control::HandleEvent(const EventArgs& msg)
{
	if( !IsMouseEnabled() && 
		(msg.Type > kEventMouseBegin) && 
		(msg.Type < kEventMouseEnd)) {
		//当前控件禁止接收鼠标消息时，将鼠标相关消息转发给上层处理
		Box* pParent = GetParent();
		if (pParent != nullptr) {
			pParent->SendEvent(msg);
		}
		return;
	}
	else if( msg.Type == kEventSetCursor ) {
		if (m_cursorType == kCursorHand) {
			if (IsEnabled()) {
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
			}
			else {
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			}
			return;
		}
		else if (m_cursorType == kCursorArrow){
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			return;
		}
		else if (m_cursorType == kCursorHandIbeam){
			::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
			return;
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (msg.Type == kEventSetFocus && m_controlState == kControlStateNormal) {
		SetState(kControlStateHot);
		Invalidate();
		return;
	}
	else if (msg.Type == kEventKillFocus && m_controlState == kControlStateHot) {
		SetState(kControlStateNormal);
		Invalidate();
		return;
	}
	else if( msg.Type == kEventMouseEnter ) {
		if (GetWindow()) {
			if (!IsChild(this, GetWindow()->GetHoverControl())) {
				return;
			}
		}
		if (!MouseEnter(msg))
			return;
	}
	else if( msg.Type == kEventMouseLeave ) {
		if (GetWindow()) {
			if (IsChild(this, GetWindow()->GetHoverControl())) {
				return;
			}
		}
		if (!MouseLeave(msg))
			return;
	}
	else if (msg.Type == kEventMouseButtonDown) {
		ButtonDown(msg);
		return;
	}
	else if (msg.Type == kEventMouseButtonUp) {
		ButtonUp(msg);
		return;
	}

	if (GetParent() != nullptr) {
		GetParent()->SendEvent(msg);
	}
}

bool Control::HasHotState()
{
	return m_colorMap->HasHotColor() || m_imageMap->HasHotImage();
}

bool Control::MouseEnter(const EventArgs& /*msg*/)
{
	if( IsEnabled() ) {
		if (m_controlState == kControlStateNormal) {
			m_controlState = kControlStateHot;
			if (HasHotState()) {
				m_animationManager->MouseEnter();
				Invalidate();
			}
			return true;
		}
		else {
			return false;
		}
	}

	return true;
}

bool Control::MouseLeave(const EventArgs& /*msg*/)
{
	if( IsEnabled() ) {
		if (m_controlState == kControlStateHot) {
			m_controlState = kControlStateNormal;
			if (HasHotState()) {
				m_animationManager->MouseLeave();
				Invalidate();
			}
			return true;
		}
		else {
			return false;
		}
	}

	return true;
}

bool Control::ButtonDown(const EventArgs& /*msg*/)
{
	bool ret = false;
	if( IsEnabled() ) {
		m_controlState = kControlStatePushed;
		SetMouseFocused(true);
		Invalidate();
		ret = true;
	}

	return ret;
}

bool Control::ButtonUp(const EventArgs& msg)
{
	bool ret = false;
	if( IsMouseFocused() ) {
		SetMouseFocused(false);
		auto player = m_animationManager->GetAnimationPlayer(kAnimationHot);
		if (player)
			player->Stop();

		Invalidate();
		if( IsPointInWithScrollOffset(UiPoint(msg.ptMouse)) ) {
			m_controlState = kControlStateHot;
			m_nHotAlpha = 255;
			Activate();
			ret = true;
		}
		else {
			m_controlState = kControlStateNormal;
			m_nHotAlpha = 0;
		}
	}

	return ret;
}

void Control::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
	if (strName == _T("class")) {
		SetClass(strValue);
	}
	else if (strName == _T("halign")) {
		if (strValue == _T("left")) {
			SetHorAlignType(kHorAlignLeft);
		}
		else if (strValue == _T("center")) {
			SetHorAlignType(kHorAlignCenter);
		}
		else if (strValue == _T("right")) {
			SetHorAlignType(kHorAlignRight);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (strName == _T("valign")) {
		if (strValue == _T("top")) {
			SetVerAlignType(kVerAlignTop);
		}
		else if (strValue == _T("center")) {
			SetVerAlignType(kVerAlignCenter);
		}
		else if (strValue == _T("bottom")) {
			SetVerAlignType(kVerAlignBottom);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (strName == _T("margin")) {
		UiRect rcMargin;
		LPTSTR pstr = NULL;
		rcMargin.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
		rcMargin.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		rcMargin.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		rcMargin.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetMargin(rcMargin, true);
	}
	else if (strName == _T("bkcolor") || strName == _T("bkcolor1")) {
		LPCTSTR pValue = strValue.c_str();
		while (*pValue > _T('\0') && *pValue <= _T(' ')) pValue = ::CharNext(pValue);
		SetBkColor(pValue);
	}
	else if (strName == _T("bordersize")) {
		std::wstring nValue = strValue;
		if (nValue.find(',') == std::wstring::npos) {
			SetBorderSize(_ttoi(strValue.c_str()));
			UiRect rcBorder;
			SetBorderSize(rcBorder);
		}
		else {
			UiRect rcBorder;
			LPTSTR pstr = NULL;
			rcBorder.left = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
			rcBorder.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcBorder.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcBorder.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetBorderSize(rcBorder);
		}
	}
	else if (strName == _T("borderround")) {
		UiSize cxyRound;
		LPTSTR pstr = NULL;
		cxyRound.cx = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
		cxyRound.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		SetBorderRound(cxyRound);
	}
	else if (strName == _T("boxshadow")) SetBoxShadow(strValue);
	else if( strName == _T("width") ) {
		if ( strValue == _T("stretch") ) {
			SetFixedWidth(DUI_LENGTH_STRETCH, true, true);
		}
		else if ( strValue == _T("auto") ) {
			SetFixedWidth(DUI_LENGTH_AUTO, true, true);
		}
		else {
			ASSERT(_ttoi(strValue.c_str()) >= 0);
			SetFixedWidth(_ttoi(strValue.c_str()), true, true);
		}
	}
	else if( strName == _T("height") ) {
		if ( strValue == _T("stretch") ) {
			SetFixedHeight(DUI_LENGTH_STRETCH, true);
		}
		else if ( strValue == _T("auto") ) {
			SetFixedHeight(DUI_LENGTH_AUTO, true);
		}
		else {
			ASSERT(_ttoi(strValue.c_str()) >= 0);
			SetFixedHeight(_ttoi(strValue.c_str()), true);
		}
	}
	else if( strName == _T("maxwidth") ) {
		if ( strValue == _T("stretch") ) {
			SetMaxWidth(DUI_LENGTH_STRETCH);
		}
		else if ( strValue == _T("auto") ) {
			SetMaxWidth(DUI_LENGTH_AUTO);
		}
		else {
			ASSERT(_ttoi(strValue.c_str()) >= 0);
			SetMaxWidth(_ttoi(strValue.c_str()));
		}
	}
	else if( strName == _T("maxheight") ) {
		if ( strValue == _T("stretch") ) {
			SetMaxHeight(DUI_LENGTH_STRETCH);
		}
		else if ( strValue == _T("auto") ) {
			SetMaxHeight(DUI_LENGTH_AUTO);
		}
		else {
			ASSERT(_ttoi(strValue.c_str()) >= 0);
			SetMaxHeight(_ttoi(strValue.c_str()));
		}
	}
	else if( strName == _T("state") ) {
		if( strValue == _T("normal") ) SetState(kControlStateNormal);
		else if( strValue == _T("hot") ) SetState(kControlStateHot);
		else if( strValue == _T("pushed") ) SetState(kControlStatePushed);
		else if( strValue == _T("disabled") ) SetState(kControlStateDisabled);
		else ASSERT(FALSE);
	}
	else if( strName == _T("cursortype") ) {
		if (strValue == _T("arrow")) {
			SetCursorType(kCursorArrow);
		}
		else if ( strValue == _T("hand") ) {
			SetCursorType(kCursorHand);
		}
		else if (strValue == _T("ibeam")) {
			SetCursorType(kCursorHandIbeam);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (strName == _T("renderoffset")) {
		UiPoint renderOffset;
		LPTSTR pstr = NULL;
		renderOffset.x = _tcstol(strValue.c_str(), &pstr, 10);  ASSERT(pstr);
		renderOffset.y = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);

		DpiManager::GetInstance()->ScalePoint(renderOffset);
		SetRenderOffset(renderOffset);
	}
	else if (strName == _T("normalcolor"))	SetStateColor(kControlStateNormal, strValue);
	else if (strName == _T("hotcolor"))	SetStateColor(kControlStateHot, strValue);
	else if (strName == _T("pushedcolor"))	SetStateColor(kControlStatePushed, strValue);
	else if (strName == _T("disabledcolor"))	SetStateColor(kControlStateDisabled, strValue);
	else if (strName == _T("bordercolor")) SetBorderColor(strValue);
	else if (strName == _T("leftbordersize")) SetLeftBorderSize(_ttoi(strValue.c_str()));
	else if (strName == _T("topbordersize")) SetTopBorderSize(_ttoi(strValue.c_str()));
	else if (strName == _T("rightbordersize")) SetRightBorderSize(_ttoi(strValue.c_str()));
	else if (strName == _T("bottombordersize")) SetBottomBorderSize(_ttoi(strValue.c_str()));
	else if (strName == _T("bkimage")) SetBkImage(strValue);
	else if (strName == _T("minwidth")) SetMinWidth(_ttoi(strValue.c_str()));
	else if (strName == _T("minheight")) SetMinHeight(_ttoi(strValue.c_str()));
	else if (strName == _T("name")) SetName(strValue);
	else if (strName == _T("tooltiptext")) SetToolTipText(strValue);
	else if (strName == _T("tooltiptextid")) SetToolTipTextId(strValue);
	else if (strName == _T("dataid")) SetDataID(strValue);
	else if (strName == _T("enabled")) SetEnabled(strValue == _T("true"));
	else if (strName == _T("mouse")) SetMouseEnabled(strValue == _T("true"));
	else if (strName == _T("keyboard")) SetKeyboardEnabled(strValue == _T("true"));
	else if (strName == _T("visible")) SetVisible(strValue == _T("true"));
	else if (strName == _T("fadevisible")) SetFadeVisible(strValue == _T("true"));
	else if (strName == _T("float")) SetFloat(strValue == _T("true"));
	else if (strName == _T("cache")) SetUseCache(strValue == _T("true"));
	else if (strName == _T("nofocus")) SetNoFocus();
	else if (strName == _T("alpha")) SetAlpha(_ttoi(strValue.c_str()));
	else if (strName == _T("normalimage") ) SetStateImage(kControlStateNormal, strValue);
	else if (strName == _T("hotimage") ) SetStateImage(kControlStateHot, strValue);
	else if (strName == _T("pushedimage") ) SetStateImage(kControlStatePushed, strValue);
	else if (strName == _T("disabledimage") ) SetStateImage(kControlStateDisabled, strValue);
	else if (strName == _T("forenormalimage") ) SetForeStateImage(kControlStateNormal, strValue);
	else if (strName == _T("forehotimage") ) SetForeStateImage(kControlStateHot, strValue);
	else if (strName == _T("forepushedimage") ) SetForeStateImage(kControlStatePushed, strValue);
	else if (strName == _T("foredisabledimage") ) SetForeStateImage(kControlStateDisabled, strValue);
	else if (strName == _T("fadealpha")) m_animationManager->SetFadeAlpha(strValue == _T("true"));
	else if (strName == _T("fadehot")) m_animationManager->SetFadeHot(strValue == _T("true"));
	else if (strName == _T("fadewidth")) m_animationManager->SetFadeWidth(strValue == _T("true"));
	else if (strName == _T("fadeheight")) m_animationManager->SetFadeHeight(strValue == _T("true"));
	else if (strName == _T("fadeinoutxfromleft")) m_animationManager->SetFadeInOutX(strValue == _T("true"), false);
	else if (strName == _T("fadeinoutxfromright")) m_animationManager->SetFadeInOutX(strValue == _T("true"), true);
	else if (strName == _T("fadeinoutyfromtop")) m_animationManager->SetFadeInOutY(strValue == _T("true"), false);
	else if (strName == _T("fadeinoutyfrombottom")) m_animationManager->SetFadeInOutY(strValue == _T("true"), true);
	else if (strName == _T("tabstop")) SetTabStop(strValue == _T("true"));
	else if (strName == _T("loadingimage")) SetLoadingImage(strValue);
	else if (strName == _T("loadingbkcolor")) SetLoadingBkColor(strValue);
	else {
		ASSERT(!"Control::SetAttribute失败: 发现不能识别的属性");
	}
}

void Control::SetClass(const std::wstring& strClass)
{
	std::list<std::wstring> splitList = StringHelper::Split(strClass, L" ");
	for (auto it = splitList.begin(); it != splitList.end(); it++) {
		std::wstring pDefaultAttributes = GlobalManager::GetClassAttributes((*it));
		Window* pWindow = GetWindow();
		if (pDefaultAttributes.empty() && (pWindow != nullptr)) {
			pDefaultAttributes = pWindow->GetClassAttributes(*it);
		}

		ASSERT(!pDefaultAttributes.empty());
		if( !pDefaultAttributes.empty() ) {
			ApplyAttributeList(pDefaultAttributes);
		}
	}
}

void Control::ApplyAttributeList(const std::wstring& strList)
{
    std::wstring sItem;
    std::wstring sValue;
	LPCTSTR pstrList = strList.c_str();
    while( *pstrList != _T('\0') ) {
        sItem.clear();
        sValue.clear();
        while( *pstrList != _T('\0') && *pstrList != _T('=') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sItem += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('=') );
        if( *pstrList++ != _T('=') ) return;
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return;
        while( *pstrList != _T('\0') && *pstrList != _T('\"') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sValue += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return;
        SetAttribute(sItem, sValue);
        if( *pstrList++ != _T(' ') ) return;
    }
    return;
}

bool Control::OnApplyAttributeList(const std::wstring& strReceiver, const std::wstring& strList, const EventArgs& /*eventArgs*/)
{
	Control* pReceiverControl = nullptr;
	if (strReceiver.substr(0, 2) == L".\\" || strReceiver.substr(0, 2) == L"./") {
		pReceiverControl = ((Box*)this)->FindSubControl(strReceiver.substr(2));
	}
	else {
		pReceiverControl = GetWindow()->FindControl(strReceiver);
	}

	if (pReceiverControl) {
		pReceiverControl->ApplyAttributeList(strList);
	}
	else {
		ASSERT(FALSE);
	}

	return true;
}

bool Control::DrawImage(IRender* pRender, Image& duiImage, const std::wstring& strModify, int nFade)
{
	//注解：strModify参数，目前外部传入的主要是："destscale='false' dest='%d,%d,%d,%d'"
	//                   也有一个类传入了：L" corner='%d,%d,%d,%d'"。
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return false;
	}
	if (duiImage.GetImagePath().empty()) {
		return false;
	}

	LoadImageData(duiImage);
	if (!duiImage.GetImageCache()) {
		ASSERT(FALSE);
		duiImage.InitImageAttribute();
		return false;
	}

	ImageAttribute newImageAttribute = duiImage.GetImageAttribute();
	if (!strModify.empty()) {
		newImageAttribute.ModifyAttribute(strModify);
	}
	UiRect rcNewDest = GetRect();
	if ((newImageAttribute.rcDest.left != DUI_NOSET_VALUE)   && 
		(newImageAttribute.rcDest.top != DUI_NOSET_VALUE)    && 
		(newImageAttribute.rcDest.right != DUI_NOSET_VALUE)  && 
		(newImageAttribute.rcDest.bottom != DUI_NOSET_VALUE)) {
		rcNewDest.left   = GetRect().left + newImageAttribute.rcDest.left;
		rcNewDest.right  = GetRect().left + newImageAttribute.rcDest.right;
		rcNewDest.top    = GetRect().top + newImageAttribute.rcDest.top;
		rcNewDest.bottom = GetRect().top + newImageAttribute.rcDest.bottom;
	}
	
	bool isPlayingGif = false;
	if (m_bkImage->GetImageCache() && m_bkImage->GetImageCache()->IsMultiFrameImage() && m_bGifPlay && !m_bkImage->IsPlaying()) {
		isPlayingGif = GifPlay();
	}
	
	if(!isPlayingGif){
		BYTE iFade = (nFade == DUI_NOSET_VALUE) ? newImageAttribute.bFade : static_cast<BYTE>(nFade);
		std::shared_ptr<ImageInfo> imageInfo = duiImage.GetImageCache();
		if (imageInfo) {
			IBitmap* pNewBitmap = nullptr;
			IBitmap* pCurrentBitmap = duiImage.GetCurrentBitmap();
			if (!strModify.empty()) {
				pNewBitmap = BitmapHelper::RotateBitmapAroundCenter(pCurrentBitmap, m_fCurrrentAngele);
			}
            pRender->DrawImage(m_rcPaint, 
							   (pNewBitmap != nullptr) ? pNewBitmap : pCurrentBitmap,
							   rcNewDest, 
							   newImageAttribute.rcSource, 
							   newImageAttribute.rcCorner, 
							   imageInfo->IsBitmapSizeDpiScaled(), 
							   iFade,
							   newImageAttribute.bTiledX, 
							   newImageAttribute.bTiledY, 
							   newImageAttribute.bFullTiledX, 
							   newImageAttribute.bFullTiledY,
							   newImageAttribute.nTiledMargin);
			if (pNewBitmap != nullptr) {
				delete pNewBitmap;
				pNewBitmap = nullptr;
			}
		}
	}

	return true;
}

IRender* Control::GetRender()
{
	if (!m_render) {
		IRenderFactory* pRenderFactory = GlobalManager::GetRenderFactory();
		ASSERT(pRenderFactory != nullptr);
		if (pRenderFactory != nullptr) {
			m_render.reset(pRenderFactory->CreateRender());
		}
	}
	return m_render.get();
}


void Control::ClearRender()
{
	if (m_render) {
		m_render.reset();
	}
}

void Control::AlphaPaint(IRender* pRender, const UiRect& rcPaint)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (m_nAlpha == 0) {
		//控件完全透明，不绘制
		return;
	}

	UiRect rcUnion;
	if( !::IntersectRect(&rcUnion, &rcPaint, &GetRect()) ) return;

	bool bRoundClip = false;
	if (m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0) {
		bRoundClip = true;
	}

	if (IsAlpha()) {
		UiSize size;
		size.cx = GetRect().right - GetRect().left;
		size.cy = GetRect().bottom - GetRect().top;
		auto pCacheRender = GetRender();
		if (pCacheRender) {
			if (pCacheRender->Resize(size.cx, size.cy)) {
				SetCacheDirty(true);
			}

			if (IsCacheDirty()) {
				pCacheRender->Clear();
				int scaleOffset = m_boxShadow.HasShadow() ? (m_boxShadow.m_nSpreadRadius * 2 + abs(m_boxShadow.m_cpOffset.x)) : 0;
				UiRect rcClip = { 0, 0, size.cx + scaleOffset,size.cy + scaleOffset };

				AutoClip alphaClip(pCacheRender, rcClip, IsClip());
				AutoClip roundAlphaClip(pCacheRender, rcClip, m_cxyBorderRound.cx, m_cxyBorderRound.cy, bRoundClip);

				pCacheRender->SetRenderTransparent(true);
				UiPoint ptOffset(GetRect().left + m_renderOffset.x, GetRect().top + m_renderOffset.y);
				UiPoint ptOldOrg = pCacheRender->OffsetWindowOrg(ptOffset);
				Paint(pCacheRender, GetRect());
				PaintChild(pCacheRender, rcPaint);
				pCacheRender->SetWindowOrg(ptOldOrg);
				SetCacheDirty(false);
			}

			pRender->AlphaBlend(rcUnion.left, 
				                rcUnion.top, 
				                rcUnion.right - rcUnion.left, 
				                rcUnion.bottom - rcUnion.top, 
				                pCacheRender,
				                rcUnion.left - GetRect().left,
				                rcUnion.top - GetRect().top,
				                rcUnion.right - rcUnion.left, 
				                rcUnion.bottom - rcUnion.top, 
				                static_cast<uint8_t>(m_nAlpha));
			m_render.reset();
		}
	}
	else if (IsUseCache()) {
		UiSize size;
		size.cx = GetRect().right - GetRect().left;
		size.cy = GetRect().bottom - GetRect().top;
		auto pCacheRender = GetRender();
		if (pCacheRender) {
			if (pCacheRender->Resize(size.cx, size.cy)) {
				SetCacheDirty(true);
			}

			if (IsCacheDirty()) {
				pCacheRender->Clear();
				int scaleOffset = m_boxShadow.HasShadow() ? (m_boxShadow.m_nSpreadRadius * 2 + abs(m_boxShadow.m_cpOffset.x)) : 0;
				UiRect rcClip = { 0,0,size.cx + scaleOffset,size.cy + scaleOffset };
				AutoClip alphaClip(pCacheRender, rcClip, IsClip());
				AutoClip roundAlphaClip(pCacheRender, rcClip, m_cxyBorderRound.cx, m_cxyBorderRound.cy, bRoundClip);

				pCacheRender->SetRenderTransparent(true);
				UiPoint ptOffset(GetRect().left + m_renderOffset.x, GetRect().top + m_renderOffset.y);
				UiPoint ptOldOrg = pCacheRender->OffsetWindowOrg(ptOffset);
				Paint(pCacheRender, GetRect());
				pCacheRender->SetWindowOrg(ptOldOrg);
				SetCacheDirty(false);
			}

			pRender->AlphaBlend(rcUnion.left, 
								rcUnion.top, 
							    rcUnion.right - rcUnion.left, 
								rcUnion.bottom - rcUnion.top, 
								pCacheRender,
							    rcUnion.left - GetRect().left, 
								rcUnion.top - GetRect().top, 
								rcUnion.right - rcUnion.left, 
								rcUnion.bottom - rcUnion.top, 
								static_cast<uint8_t>(m_nAlpha));
			PaintChild(pRender, rcPaint);
		}
	}
	else {
		int scaleOffset = m_boxShadow.HasShadow() ? (m_boxShadow.m_nSpreadRadius + abs(m_boxShadow.m_cpOffset.x)) : 0;
		UiRect rcClip = { GetRect().left - scaleOffset,
					      GetRect().top - scaleOffset,
					      GetRect().right + scaleOffset,
						  GetRect().bottom + scaleOffset,
		};
		AutoClip clip(pRender, rcClip, IsClip());
		AutoClip roundClip(pRender, rcClip, m_cxyBorderRound.cx, m_cxyBorderRound.cy, bRoundClip);
		UiPoint ptOldOrg = pRender->OffsetWindowOrg(m_renderOffset);
		Paint(pRender, rcPaint);
		PaintChild(pRender, rcPaint);
		pRender->SetWindowOrg(ptOldOrg);
	}
}

void Control::Paint(IRender* pRender, const UiRect& rcPaint)
{
	if (!::IntersectRect(&m_rcPaint, &rcPaint, &GetRect())) {
		return;
	}

	PaintShadow(pRender);
	PaintBkColor(pRender);
	PaintBkImage(pRender);
	PaintStatusColor(pRender);
	PaintStatusImage(pRender);
	PaintText(pRender);
	PaintBorder(pRender);
    PaintLoading(pRender);
}

void Control::PaintShadow(IRender* pRender)
{
	if (!m_boxShadow.HasShadow()) {
		return;
	}

	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	pRender->DrawBoxShadow(m_rcPaint,
						   m_cxyBorderRound,
						   m_boxShadow.m_cpOffset,
						   m_boxShadow.m_nBlurRadius,
						   m_boxShadow.m_nSpreadRadius,
						   GlobalManager::GetTextColor(m_boxShadow.m_strColor),
						   m_boxShadow.m_bExclude);
}

void Control::PaintBkColor(IRender* pRender)
{
	if (m_strBkColor.empty()) {
		return;
	}
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}

	UiColor dwBackColor = this->GetWindowColor(m_strBkColor);
	if(dwBackColor.GetARGB() != 0) {
		if (dwBackColor.GetARGB() >= 0xFF000000) {
			pRender->DrawColor(m_rcPaint, dwBackColor);
		}
		else {
			pRender->DrawColor(GetRect(), dwBackColor);
		}
	}
}

void Control::PaintBkImage(IRender* pRender)
{
    DrawImage(pRender, *m_bkImage);
}

void Control::PaintStatusColor(IRender* pRender)
{
	m_colorMap->PaintStateColor(pRender, m_rcPaint, m_controlState);
}

void Control::PaintStatusImage(IRender* pRender)
{
	m_imageMap->PaintStateImage(pRender, kStateImageBk, m_controlState);
	m_imageMap->PaintStateImage(pRender, kStateImageFore, m_controlState);
}

void Control::PaintText(IRender* /*pRender*/)
{
    return;
}

void Control::PaintBorder(IRender* pRender)
{
	if (m_strBorderColor.empty()) {
		return;
	}
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	UiColor dwBorderColor = GetWindowColor(m_strBorderColor);
	if (dwBorderColor.GetARGB() != 0) {
		if (m_rcBorderSize.left > 0 || m_rcBorderSize.top > 0 || m_rcBorderSize.right > 0 || m_rcBorderSize.bottom > 0) {
			UiRect rcBorder;
			if (m_rcBorderSize.left > 0) {
				rcBorder = GetRect();
				rcBorder.right = rcBorder.left = GetRect().left + m_rcBorderSize.left / 2;
				if (m_rcBorderSize.left == 1) {
					rcBorder.bottom -= 1;
				}
				pRender->DrawLine(UiPoint(rcBorder.left, rcBorder.top), UiPoint(rcBorder.right, rcBorder.bottom), dwBorderColor, m_rcBorderSize.left);
			}
			if (m_rcBorderSize.top > 0) {
				rcBorder = GetRect();
				rcBorder.bottom = rcBorder.top = GetRect().top + m_rcBorderSize.top / 2;
				if (m_rcBorderSize.top == 1) {
					rcBorder.right -= 1;
				}
				pRender->DrawLine(UiPoint(rcBorder.left, rcBorder.top), UiPoint(rcBorder.right, rcBorder.bottom), dwBorderColor, m_rcBorderSize.top);
			}
			if (m_rcBorderSize.right > 0) {
				rcBorder = GetRect();
				rcBorder.left = rcBorder.right = GetRect().right - (m_rcBorderSize.right + 1) / 2;
				if (m_rcBorderSize.right == 1) {
					rcBorder.bottom -= 1;
				}
				pRender->DrawLine(UiPoint(rcBorder.left, rcBorder.top), UiPoint(rcBorder.right, rcBorder.bottom), dwBorderColor, m_rcBorderSize.right);
			}
			if (m_rcBorderSize.bottom > 0) {
				rcBorder = GetRect();
				rcBorder.top = rcBorder.bottom = GetRect().bottom - (m_rcBorderSize.bottom + 1) / 2;
				if (m_rcBorderSize.bottom == 1) {
					rcBorder.right -= 1;
				}
				pRender->DrawLine(UiPoint(rcBorder.left, rcBorder.top), UiPoint(rcBorder.right, rcBorder.bottom), dwBorderColor, m_rcBorderSize.bottom);
			}
		}
		else if (m_nBorderSize > 0) {
			UiRect rcDraw = GetRect();
			int nDeltaValue = m_nBorderSize / 2;
			rcDraw.top += nDeltaValue;
			rcDraw.bottom -= nDeltaValue;
			if (m_nBorderSize % 2 != 0) {
				rcDraw.bottom -= 1;
			}
			rcDraw.left += nDeltaValue;
			rcDraw.right -= nDeltaValue;
			if (m_nBorderSize % 2 != 0) {
				rcDraw.right -= 1;
			}

			if (m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0) {
				pRender->DrawRoundRect(rcDraw, m_cxyBorderRound, dwBorderColor, m_nBorderSize);
			}
			else {
				pRender->DrawRect(rcDraw, dwBorderColor, m_nBorderSize);
			}
		}
	}
}

void Control::PaintLoading(IRender* pRender)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
    if (!m_bIsLoading || m_loadingImage->GetImagePath().empty()) {
        return;
    }

	LoadImageData(*m_loadingImage);
	std::shared_ptr<ImageInfo> spImageInfo = m_loadingImage->GetImageCache();
	ASSERT(spImageInfo != nullptr);
    if (!spImageInfo) {
        return;
    }

	IBitmap* pBitmap = spImageInfo->GetBitmap(0);
	ASSERT(pBitmap != nullptr);
	if (pBitmap == nullptr) {
		return;
	}
	int imageWidth = pBitmap->GetWidth();
	int imageHeight = pBitmap->GetHeight();

	//居中
	ui::UiRect rcFill = GetRect();
	rcFill.left = GetRect().left + (GetRect().GetWidth() - imageWidth) / 2;
	rcFill.right = rcFill.left + imageWidth;
	rcFill.top = GetRect().top + (GetRect().GetHeight() - imageHeight) / 2;
	rcFill.bottom = rcFill.top + imageHeight;

	ui::UiRect rcDest = m_loadingImage->GetImageAttribute().rcDest;
	if (!rcDest.IsRectEmpty()) {
		rcFill.left = GetRect().left + rcDest.left;
		rcFill.right = GetRect().left + rcDest.right;
		rcFill.top = GetRect().top + rcDest.top;
		rcFill.bottom = GetRect().bottom + rcDest.bottom;
	}

    if (!m_strLoadingBkColor.empty()) {
        pRender->DrawColor(rcFill, GetWindowColor(m_strLoadingBkColor));
    }
	
	wchar_t modify[64] = { 0 };
	swprintf_s(modify, L"dest='%d,%d,%d,%d'", rcFill.left - GetRect().left, rcFill.top - GetRect().top, rcFill.right - GetRect().left, rcFill.bottom - GetRect().top);
	DrawImage(pRender, *m_loadingImage, modify);
}

void Control::SetAlpha(int alpha)
{
	ASSERT(alpha >= 0 && alpha <= 255);
	m_nAlpha = alpha;
	Invalidate();
}

void Control::SetHotAlpha(int nHotAlpha)
{
	ASSERT(nHotAlpha >= 0 && nHotAlpha <= 255);
	m_nHotAlpha = nHotAlpha;
	Invalidate();
}

void Control::SetTabStop(bool enable)
{
	m_bAllowTabstop = enable;
}

void Control::SetRenderOffset(UiPoint renderOffset)
{
	m_renderOffset = renderOffset;
	Invalidate();
}

void Control::SetRenderOffsetX(int renderOffsetX)
{
	m_renderOffset.x = renderOffsetX;
	Invalidate();
}

void Control::SetRenderOffsetY(int renderOffsetY)
{
	m_renderOffset.y = renderOffsetY;
	Invalidate();
}

bool Control::GifPlay()
{
	if (!m_bkImage->GetImageCache() || !m_bkImage->GetImageCache()->IsMultiFrameImage() || !m_bkImage->ContinuePlay()) {
		m_bkImage->SetPlaying(false);
		m_gifWeakFlag.Cancel();
		return false;
	}

	if (!m_bkImage->IsPlaying()) {
		m_bkImage->SetCurrentFrame(0);
		m_gifWeakFlag.Cancel();
		int timerInterval = m_bkImage->GetCurrentInterval();//播放间隔：毫秒
		if (timerInterval <= 0) {
			return false;
		}
		m_bkImage->SetPlaying(true);
		auto gifPlayCallback = nbase::Bind(&Control::GifPlay, this);
		TimerManager::GetInstance()->AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(), 
													    gifPlayCallback,
														timerInterval, 
													    TimerManager::REPEAT_FOREVER);
	}
	else {
		int preInterval = m_bkImage->GetCurrentInterval();
		m_bkImage->IncrementCurrentFrame();
		int nowInterval = m_bkImage->GetCurrentInterval();
		if (!m_bkImage->ContinuePlay()) {
			StopGifPlayForUI(true, kGifStopLast);
		}
		else
		{
			if ((preInterval <= 0) || (nowInterval <= 0)) {
				m_bkImage->SetPlaying(false);
				m_gifWeakFlag.Cancel();
				return false;
			}

			if (preInterval != nowInterval) {
				m_gifWeakFlag.Cancel();
				m_bkImage->SetPlaying(true);
				auto gifPlayCallback = nbase::Bind(&Control::GifPlay, this);
				TimerManager::GetInstance()->AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(), 
																gifPlayCallback,
																nowInterval, 
															    TimerManager::REPEAT_FOREVER);
			}
		}			
	}
	Invalidate();
	return m_bkImage->IsPlaying();
}

void Control::StopGifPlay(GifStopType frame)
{
	if (m_bkImage->GetImageCache() && m_bkImage->GetImageCache()->IsMultiFrameImage()) {
		m_bkImage->SetPlaying(false);
		m_gifWeakFlag.Cancel();
		size_t index = GetGifFrameIndex(frame);
		m_bkImage->SetCurrentFrame(index);
		Invalidate();
	}
}

void Control::StartGifPlayForUI(GifStopType frame, int playcount)
{
	LoadImageData(*m_bkImage);
	if (!m_bkImage->GetImageCache() || !m_bkImage->GetImageCache()->IsMultiFrameImage()) {
		m_bGifPlay = false;
		m_bkImage->SetPlaying(false);
		m_gifWeakFlag.Cancel();
		return;
	}
	if (playcount == 0)	{
		StopGifPlayForUI(false);
	}		
	else
	{
		m_gifWeakFlag.Cancel();
		m_bGifPlay = true;
		m_bkImage->SetCurrentFrame(GetGifFrameIndex(frame));
		int timerInterval = m_bkImage->GetCurrentInterval();
		if (timerInterval <= 0) {
			m_bGifPlay = false;
			return;
		}
		m_bkImage->SetPlaying(true);
		m_bkImage->SetImagePlayCount(playcount);
		m_bkImage->ClearCycledCount();
		auto gifPlayCallback = nbase::Bind(&Control::GifPlay, this);
		TimerManager::GetInstance()->AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(), 
													    gifPlayCallback,
													    timerInterval, 
													    TimerManager::REPEAT_FOREVER);
		Invalidate();
	}	
}

void Control::StopGifPlayForUI(bool transfer, GifStopType frame)
{
	m_bGifPlay = false;
	StopGifPlay(frame);
	if (transfer) {
		BroadcastGifEvent(m_nVirtualEventGifStop);
	}
}

size_t Control::GetGifFrameIndex(GifStopType frame)
{
	size_t ret = frame;
	switch (frame)
	{
	case kGifStopCurrent:
		ret = m_bkImage->GetCurrentFrameIndex();
		break;
	case kGifStopFirst:
		ret = 0;
		break;
	case kGifStopLast:
	{
		size_t nFrameCount = m_bkImage->GetImageCache()->GetFrameCount();
		ret = nFrameCount > 0 ? nFrameCount - 1 : 0;		
	}
	break;
	}
	return ret;
}
void Control::BroadcastGifEvent(int nVirtualEvent)
{
	auto callback = m_OnGifEvent.find(nVirtualEvent);
	if (callback != m_OnGifEvent.end()) {
		EventArgs param;
		param.pSender = this;
		callback->second(param);
	}
}

bool Control::LoadImageData(Image& duiImage) const
{
	Window* pWindow = GetWindow();
	ASSERT(pWindow != nullptr);
	if (pWindow == nullptr) {
		return false;
	}

	std::wstring sImagePath = duiImage.GetImagePath();
	if (sImagePath.empty()) {
		return false;
	}
	std::wstring imageFullPath = GlobalManager::GetResFullPath(pWindow->GetResourcePath(), sImagePath);
	ImageLoadAttribute imageLoadAttr = duiImage.GetImageLoadAttribute();
	imageLoadAttr.SetImageFullPath(imageFullPath);
	std::shared_ptr<ImageInfo> imageCache = duiImage.GetImageCache();
	if (!imageCache || (imageCache->GetCacheKey() != imageLoadAttr.GetCacheKey())) {
		imageCache = GlobalManager::GetImage(imageLoadAttr);
		duiImage.SetImageCache(imageCache);
	}
	return imageCache ? true : false;
}

void Control::InvokeLoadImageCache()
{
	if (m_loadBkImageWeakFlag.HasUsed()) {
		return;
	}
	std::wstring sImagePath = m_bkImage->GetImagePath();
	if (sImagePath.empty()) {
		return;
	}
	Window* pWindow = GetWindow();
	if (pWindow == nullptr) {
		return;
	}
	std::wstring imageFullPath = GlobalManager::GetResFullPath(pWindow->GetResourcePath(), sImagePath);
	ImageLoadAttribute imageLoadAttr = m_bkImage->GetImageLoadAttribute();
	imageLoadAttr.SetImageFullPath(imageFullPath);
	if (!m_bkImage->GetImageCache() || m_bkImage->GetImageCache()->GetCacheKey() != imageLoadAttr.GetCacheKey()) {
		auto shared_image = GlobalManager::GetCachedImage(imageLoadAttr);
		if (shared_image) {
			m_bkImage->SetImageCache(shared_image);
			return;
		}
	}
}

void Control::UnLoadImageCache()
{
	m_loadBkImageWeakFlag.Cancel();
	m_bkImage->ClearImageCache();
}

void Control::ClearImageCache()
{
	m_imageMap->ClearImageCache();
	m_bkImage->ClearImageCache();
}

void Control::AttachEvent(EventType type, const EventCallback& callback)
{ 
	m_OnEvent[type] += callback; 
	if ((type == kEventMouseMenu) || (type == kEventAll)) {
		SetContextMenuUsed(true);
	}
}

void Control::DetachEvent(EventType type)
{
	auto event = m_OnEvent.find(type);
	if (event != m_OnEvent.end()) {
		m_OnEvent.erase(event);
	}
	if ((type == kEventMouseMenu) || (type == kEventAll)) {
		if ((m_OnEvent.find(kEventAll) == m_OnEvent.end()) &&
			(m_OnEvent.find(kEventMouseMenu) == m_OnEvent.end())) {
			SetContextMenuUsed(false);
		}
	}
}

void Control::AttachXmlEvent(EventType eventType, const EventCallback& callback)
{
	m_OnXmlEvent[eventType] += callback; 
}

void Control::DetachXmlEvent(EventType type)
{
	auto event = m_OnXmlEvent.find(type);
	if (event != m_OnXmlEvent.end()) {
		m_OnXmlEvent.erase(event);
	}
}

void Control::AttachBubbledEvent(EventType eventType, const EventCallback& callback)
{
	m_OnBubbledEvent[eventType] += callback;
}

void Control::DetachBubbledEvent(EventType eventType)
{
	auto event = m_OnBubbledEvent.find(eventType);
	if (event != m_OnBubbledEvent.end()) {
		m_OnBubbledEvent.erase(eventType);
	}
}

void Control::AttachXmlBubbledEvent(EventType eventType, const EventCallback& callback)
{
	m_OnXmlBubbledEvent[eventType] += callback;
}

void Control::DetachXmlBubbledEvent(EventType eventType)
{
	auto event = m_OnXmlBubbledEvent.find(eventType);
	if (event != m_OnXmlBubbledEvent.end())	{
		m_OnXmlBubbledEvent.erase(eventType);
	}
}

bool Control::FireAllEvents(const EventArgs& msg)
{
	std::weak_ptr<nbase::WeakFlag> weakflag = GetWeakFlag();
	bool bRet = true;//当值为false时，就不再调用回调函数和处理函数

	if (msg.pSender == this) {
		if (bRet && !m_OnEvent.empty()) {
			auto callback = m_OnEvent.find(msg.Type);
			if (callback != m_OnEvent.end()) {
				bRet = callback->second(msg);
			}
			if (weakflag.expired()) {
				return false;
			}
		}

		if (bRet && !m_OnEvent.empty()) {
			auto callback = m_OnEvent.find(kEventAll);
			if (callback != m_OnEvent.end()) {
				bRet = callback->second(msg);
			}
			if (weakflag.expired()) {
				return false;
			}
		}

		if (bRet && !m_OnXmlEvent.empty()) {
			auto callback = m_OnXmlEvent.find(msg.Type);
			if (callback != m_OnXmlEvent.end()) {
				bRet = callback->second(msg);
			}
			if (weakflag.expired()) {
				return false;
			}
		}

		if (bRet && !m_OnXmlEvent.empty()) {
			auto callback = m_OnXmlEvent.find(kEventAll);
			if (callback != m_OnXmlEvent.end()) {
				bRet = callback->second(msg);
			}
			if (weakflag.expired()) {
				return false;
			}
		}
	}

	if (bRet && !m_OnBubbledEvent.empty()) {
		auto callback = m_OnBubbledEvent.find(msg.Type);
		if (callback != m_OnBubbledEvent.end()) {
			bRet = callback->second(msg);
		}
		if (weakflag.expired()) {
			return false;
		}
	}

	if (bRet && !m_OnBubbledEvent.empty()) {
		auto callback = m_OnBubbledEvent.find(kEventAll);
		if (callback != m_OnBubbledEvent.end()) {
			bRet = callback->second(msg);
		}
		if (weakflag.expired()) {
			return false;
		}
	}

	if (bRet && !m_OnXmlBubbledEvent.empty()) {
		auto callback = m_OnXmlBubbledEvent.find(msg.Type);
		if (callback != m_OnXmlBubbledEvent.end()) {
			bRet = callback->second(msg);
		}
		if (weakflag.expired()) {
			return false;
		}
	}

	if (bRet && !m_OnXmlBubbledEvent.empty()) {
		auto callback = m_OnXmlBubbledEvent.find(kEventAll);
		if (callback != m_OnXmlBubbledEvent.end()) {
			bRet = callback->second(msg);
		}
		if (weakflag.expired()) {
			return false;
		}
	}

	return bRet && !weakflag.expired();
}

UiColor Control::GetWindowColor(const std::wstring& strName) const
{
	UiColor color;
	Window* pWindow = GetWindow();
	if (pWindow != nullptr) {
		color = pWindow->GetTextColor(strName);
	}

	if (color.GetARGB() == 0) {
		color = GlobalManager::GetTextColor(strName);
	}

	ASSERT(color.GetARGB() != 0);
	return color;
}

bool Control::IsSelectableType() const
{
	return false;
}

bool Control::IsWantTab() const
{
	return false;
}

bool Control::CanPlaceCaptionBar() const
{
	return false;
}

void Control::StartLoading(int fStartAngle) 
{
    if (fStartAngle >= 0) {
        m_fCurrrentAngele = fStartAngle;
    }
    if (m_bIsLoading) {
        return;
    }

    m_bIsLoading = true;
    SetEnabled(false);
    TimerManager::GetInstance()->AddCancelableTimer(m_loadingImageFlag.GetWeakFlag(), nbase::Bind(&Control::Loading, this),
        50, TimerManager::REPEAT_FOREVER);
}

void Control::StopLoading(GifStopType frame) 
{
    if (!m_bIsLoading) {
        return;
    }

    switch (frame) {
    case kGifStopFirst:
        m_fCurrrentAngele = 0;
        break;
    case kGifStopCurrent:
        break;
    case  kGifStopLast:
        m_fCurrrentAngele = 360;
    }
    m_bIsLoading = false;
    SetEnabled(true);

    m_loadingImageFlag.Cancel();
}

void Control::Loading() 
{
    if (!m_bIsLoading) {
        return;
    }
    m_fCurrrentAngele += 10;
    if (m_fCurrrentAngele == INT32_MIN) {
        m_fCurrrentAngele = 0;
    }

    Invalidate();
}

bool Control::IsLoading() 
{
    return m_bIsLoading;
}

bool Control::CheckVisibleAncestor(void) const
{
	bool isVisible = IsVisible();
	if (isVisible) {
		Control* parent = GetParent();
		while (parent != nullptr)
		{
			if (!parent->IsVisible()) {
				isVisible = false;
				break;
			}
			parent = parent->GetParent();
		}
	}
	return isVisible;
}

void Control::EnsureNoFocus()
{
	Window* pWindow = GetWindow();
	if ((pWindow != nullptr) && pWindow->GetFocus() != nullptr) {
		if (pWindow->GetFocus() == this) {
			pWindow->SetFocus(nullptr);
		}
		/*
		else if (IsChild(this, pWindow->GetFocus())) {
			pWindow->SetFocus(nullptr);
		}
		*/
	}
}

} // namespace ui
