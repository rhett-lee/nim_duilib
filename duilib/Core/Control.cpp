#include "Control.h"
#include "duilib/Image/Image.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ColorManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"

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
	m_boxShadow(),
	m_isBoxShadowPainted(false)
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
}

std::wstring Control::GetType() const { return L"Control"; }

void Control::SetBkColor(const std::wstring& strColor)
{
	ASSERT(strColor.empty() || HasUiColor(strColor));
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
	ASSERT(strColor.empty() || HasUiColor(strColor));
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
	RelayoutOrRedraw();
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
	RelayoutOrRedraw();
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
	GlobalManager::Instance().Dpi().ScaleRect(rc);
	m_rcBorderSize = rc;
	Invalidate();
}

int Control::GetLeftBorderSize() const
{
	return m_rcBorderSize.left;
}

void Control::SetLeftBorderSize(int nSize)
{
	GlobalManager::Instance().Dpi().ScaleInt(nSize);
	m_rcBorderSize.left = nSize;
	Invalidate();
}

int Control::GetTopBorderSize() const
{
	return m_rcBorderSize.top;
}

void Control::SetTopBorderSize(int nSize)
{
	GlobalManager::Instance().Dpi().ScaleInt(nSize);
	m_rcBorderSize.top = nSize;
	Invalidate();
}

int Control::GetRightBorderSize() const
{
	return m_rcBorderSize.right;
}

void Control::SetRightBorderSize(int nSize)
{
	GlobalManager::Instance().Dpi().ScaleInt(nSize);
	m_rcBorderSize.right = nSize;
	Invalidate();
}

int Control::GetBottomBorderSize() const
{
	return m_rcBorderSize.bottom;
}

void Control::SetBottomBorderSize(int nSize)
{
	GlobalManager::Instance().Dpi().ScaleInt(nSize);
	m_rcBorderSize.bottom = nSize;
	Invalidate();
}

const UiSize& Control::GetBorderRound() const
{
    return m_cxyBorderRound;
}

void Control::SetBorderRound(UiSize cxyRound)
{
	int cx = cxyRound.cx;
	int cy = cxyRound.cy;
	ASSERT(cx >= 0);
	ASSERT(cy >= 0);
	if ((cx < 0) || (cy < 0)) {
		return;
	}
	//两个参数要么同时等于0，要么同时大于0，否则参数无效
	ASSERT(((cx > 0) && (cy > 0)) || ((cx == 0) && (cy == 0)));
	if (cx == 0) {
		if (cy != 0) {
			return;
		}
	}
	else {
		if (cy == 0) {
			return;
		}
	}
	GlobalManager::Instance().Dpi().ScaleSize(cxyRound);
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
		strText = GlobalManager::Instance().Lang().GetStringViaID(m_sToolTipTextId);
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
	StringHelper::ReplaceAll(L"<n>",L"\r\n", strTemp);
	m_sToolTipText = strTemp;

	Invalidate();
}

void Control::SetUTF8ToolTipText(const std::string& strText)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strText, strOut, CP_UTF8);
	if (strOut.empty()) {
		m_sToolTipText.clear();
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
	GlobalManager::Instance().Dpi().ScaleInt(nWidth);
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
		(!m_bMouseEnabled || ((pData != nullptr) && !GetRect().ContainsPt(*static_cast<UiPoint*>(pData))))) {
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

	if (GetRect().Equals(rc)) {
		SetArranged(false);
		return;
	}

	UiRect invalidateRc = GetRect();
	if (invalidateRc.IsEmpty()) {
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
		if (!UiRect::Intersect(invalidateRc, rcTemp, rcParent)) {
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
	rcMargin.left = std::max((int)rcMargin.left, 0);
	rcMargin.right = std::max((int)rcMargin.right, 0);
	rcMargin.top = std::max((int)rcMargin.top, 0);
	rcMargin.bottom = std::max((int)rcMargin.bottom, 0);

	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleRect(rcMargin);
	}

	if (!m_rcMargin.Equals(rcMargin)) {
		m_rcMargin = rcMargin;
		ArrangeAncestor();
	}
}

UiSize Control::EstimateSize(UiSize szAvailable)
{
	UiSize imageSize = GetFixedSize();
	if ((GetFixedWidth() != DUI_LENGTH_AUTO) && (GetFixedHeight() != DUI_LENGTH_AUTO)) {
		//如果宽高都不是auto属性，则直接返回
		return imageSize;
	}
	if (!IsReEstimateSize()) {
		//使用缓存中的估算结果
		return GetEstimateSize();
	}

	//估算图片区域大小
	Image* image = GetEstimateImage();
	if (image != nullptr) {
		//加载图片：需要获取图片的宽和高
		LoadImageData(*image);
		std::shared_ptr<ImageInfo> imageCache = image->GetImageCache();
		if (imageCache != nullptr) {
			ImageAttribute imageAttribute = image->GetImageAttribute();
			UiRect rcDest;
			if (ImageAttribute::HasValidImageRect(imageAttribute.rcDest)) {
				//使用配置中指定的目标区域
				rcDest = imageAttribute.rcDest;
			}
			UiRect rcDestCorners;
			UiRect rcSource = imageAttribute.rcSource;
			UiRect rcSourceCorners = imageAttribute.rcCorner;
			ImageAttribute::ScaleImageRect(imageCache->GetWidth(), imageCache->GetHeight(),
										   imageCache->IsBitmapSizeDpiScaled(),
										   rcDestCorners,
										   rcSource,
										   rcSourceCorners);
			if (GetFixedWidth() == DUI_LENGTH_AUTO) {
				if (rcDest.Width() > 0) {
					imageSize.cx = rcDest.Width();
				}
				else if (rcSource.Width() > 0) {
					imageSize.cx = rcSource.Width();
				}
				else {
					imageSize.cx = imageCache->GetWidth();
				}
			}
			if (GetFixedHeight() == DUI_LENGTH_AUTO) {
				if (rcDest.Height() > 0) {
					imageSize.cy = rcDest.Height();
				}
				else if (rcSource.Height() > 0) {
					imageSize.cy = rcSource.Height();
				}
				else {
					imageSize.cy = imageCache->GetHeight();
				}
			}
		}
	}
	//估算文本区域大小
	UiSize textSize = EstimateText(szAvailable);

	//选取图片和文本区域大小的最大值
	if ((GetFixedWidth() == DUI_LENGTH_AUTO) && (imageSize.cx < textSize.cx)) {
		imageSize.cx = textSize.cx;
	}
	if ((GetFixedHeight() == DUI_LENGTH_AUTO) && (imageSize.cy < textSize.cy)) {
		imageSize.cy = textSize.cy;
	}

	//对估算结果进行有效性校验
	ASSERT(DUI_LENGTH_AUTO == -2);
	ASSERT(DUI_LENGTH_STRETCH == -1);
	if (imageSize.cx <= DUI_LENGTH_AUTO) {
		imageSize.cx = 0;
	}
	if (imageSize.cy <= DUI_LENGTH_AUTO) {
		imageSize.cy = 0;
	}
	//保持结果到缓存，避免每次都重新估算
	SetEstimateSize(imageSize);
	SetReEstimateSize(false);
	return imageSize;
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

UiSize Control::EstimateText(UiSize /*szAvailable*/)
{
	return UiSize(0, 0);
}

bool Control::IsPointInWithScrollOffset(const UiPoint& point) const
{
	UiPoint scrollOffset = GetScrollOffset();
	UiPoint newPoint = point;
	newPoint.Offset(scrollOffset);
	return GetRect().ContainsPt(newPoint);
}

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
	if (strName == L"class") {
		SetClass(strValue);
	}
	else if (strName == L"halign") {
		if (strValue == L"left") {
			SetHorAlignType(kHorAlignLeft);
		}
		else if (strValue == L"center") {
			SetHorAlignType(kHorAlignCenter);
		}
		else if (strValue == L"right") {
			SetHorAlignType(kHorAlignRight);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (strName == L"valign") {
		if (strValue == L"top") {
			SetVerAlignType(kVerAlignTop);
		}
		else if (strValue == L"center") {
			SetVerAlignType(kVerAlignCenter);
		}
		else if (strValue == L"bottom") {
			SetVerAlignType(kVerAlignBottom);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (strName == L"margin") {
		UiRect rcMargin;
		AttributeUtil::ParseRectValue(strValue.c_str(), rcMargin);
		SetMargin(rcMargin, true);
	}
	else if (strName == L"bkcolor" || strName == L"bkcolor1") {
		SetBkColor(strValue);
	}
	else if ((strName == L"border_size") || (strName == L"bordersize")){
		std::wstring nValue = strValue;
		if (nValue.find(L',') == std::wstring::npos) {
			int nBorderSize = _wtoi(strValue.c_str());
			if (nBorderSize < 0) {
				nBorderSize = 0;
			}
			UiRect rcBorder(nBorderSize, nBorderSize, nBorderSize, nBorderSize);
			SetBorderSize(rcBorder);
		}
		else {
			UiRect rcBorder;
			AttributeUtil::ParseRectValue(strValue.c_str(), rcBorder);
			SetBorderSize(rcBorder);
		}
	}
	else if ((strName == L"border_round") || (strName == L"borderround")) {
		UiSize cxyRound;
		AttributeUtil::ParseSizeValue(strValue.c_str(), cxyRound);
		SetBorderRound(cxyRound);
	}
	else if ((strName == L"box_shadow") || (strName == L"boxshadow")) {
		SetBoxShadow(strValue);
	}
	else if(strName == L"width") {
		if (strValue == L"stretch") {
			SetFixedWidth(DUI_LENGTH_STRETCH, true, true);
		}
		else if (strValue == L"auto") {
			SetFixedWidth(DUI_LENGTH_AUTO, true, true);
		}
		else {
			ASSERT(_wtoi(strValue.c_str()) >= 0);
			SetFixedWidth(_wtoi(strValue.c_str()), true, true);
		}
	}
	else if(strName == L"height") {
		if (strValue == L"stretch") {
			SetFixedHeight(DUI_LENGTH_STRETCH, true);
		}
		else if (strValue == L"auto") {
			SetFixedHeight(DUI_LENGTH_AUTO, true);
		}
		else {
			ASSERT(_wtoi(strValue.c_str()) >= 0);
			SetFixedHeight(_wtoi(strValue.c_str()), true);
		}
	}
	else if(strName == L"state") {
		if (strValue == L"normal") {
			SetState(kControlStateNormal);
		}
		else if (strValue == L"hot") {
			SetState(kControlStateHot);
		}
		else if (strValue == L"pushed") {
			SetState(kControlStatePushed);
		}
		else if (strValue == L"disabled") {
			SetState(kControlStateDisabled);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if (strName == L"cursortype") {
		if (strValue == L"arrow") {
			SetCursorType(kCursorArrow);
		}
		else if (strValue == L"hand") {
			SetCursorType(kCursorHand);
		}
		else if (strValue == L"ibeam") {
			SetCursorType(kCursorHandIbeam);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else if ((strName == L"render_offset") || (strName == L"renderoffset")) {
		UiPoint renderOffset;
		AttributeUtil::ParsePointValue(strValue.c_str(), renderOffset);
		GlobalManager::Instance().Dpi().ScalePoint(renderOffset);
		SetRenderOffset(renderOffset);
	}
	else if ((strName == L"normal_color") || (strName == L"normalcolor")) {
		SetStateColor(kControlStateNormal, strValue);
	}
	else if ((strName == L"hot_color") || (strName == L"hotcolor")) {
		SetStateColor(kControlStateHot, strValue);
	}
	else if ((strName == L"pushed_color") || (strName == L"pushedcolor")) {
		SetStateColor(kControlStatePushed, strValue);
	}
	else if ((strName == L"disabled_color") || (strName == L"disabledcolor")) {
		SetStateColor(kControlStateDisabled, strValue);
	}
	else if ((strName == L"border_color") || (strName == L"bordercolor")) {
		SetBorderColor(strValue);
	}
	else if ((strName == L"left_border_size") || (strName == L"leftbordersize")) {
		SetLeftBorderSize(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"top_border_size") || (strName == L"topbordersize")){
		SetTopBorderSize(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"right_border_size") || (strName == L"rightbordersize")) {
		SetRightBorderSize(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"bottom_border_size") || (strName == L"bottombordersize")) {
		SetBottomBorderSize(_wtoi(strValue.c_str()));
	}
	else if (strName == L"bkimage") {
		SetBkImage(strValue);
	}
	else if ((strName == L"min_width") || (strName == L"minwidth")) {
		SetMinWidth(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"max_width") || (strName == L"maxwidth")) {
		SetMaxWidth(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"min_height") || (strName == L"minheight")) {
		SetMinHeight(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"max_height") || (strName == L"maxheight")) {
		SetMaxHeight(_wtoi(strValue.c_str()));
	}
	else if (strName == L"name") {
		SetName(strValue);
	}
	else if ((strName == L"tooltip_text") || (strName == L"tooltiptext")) {
		SetToolTipText(strValue);
	}
	else if ((strName == L"tooltip_textid") || (strName == L"tooltiptextid")) {
		SetToolTipTextId(strValue);
	}
	else if (strName == L"dataid") {
		SetDataID(strValue);
	}
	else if (strName == L"enabled") {
		SetEnabled(strValue == L"true");
	}
	else if (strName == L"mouse") {
		SetMouseEnabled(strValue == L"true");
	}
	else if (strName == L"keyboard") {
		SetKeyboardEnabled(strValue == L"true");
	}
	else if (strName == L"visible") {
		SetVisible(strValue == L"true");
	}
	else if ((strName == L"fade_visible") || (strName == L"fadevisible")) {
		SetFadeVisible(strValue == L"true");
	}
	else if (strName == L"float") {
		SetFloat(strValue == L"true");
	}
	else if (strName == L"cache") {
		SetUseCache(strValue == L"true");
	}
	else if (strName == L"nofocus") {
		SetNoFocus();
	}
	else if (strName == L"alpha") {
		SetAlpha(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"normal_image") || (strName == L"normalimage")) {
		SetStateImage(kControlStateNormal, strValue);
	}
	else if ((strName == L"hot_image") || (strName == L"hotimage")) {
		SetStateImage(kControlStateHot, strValue);
	}
	else if ((strName == L"pushed_image") || (strName == L"pushedimage")) {
		SetStateImage(kControlStatePushed, strValue);
	}
	else if ((strName == L"disabled_image") || (strName == L"disabledimage")) {
		SetStateImage(kControlStateDisabled, strValue);
	}
	else if ((strName == L"fore_normal_image") || (strName == L"forenormalimage")) {
		SetForeStateImage(kControlStateNormal, strValue);
	}
	else if ((strName == L"fore_hot_image") || (strName == L"forehotimage")) {
		SetForeStateImage(kControlStateHot, strValue);
	}
	else if ((strName == L"fore_pushed_image") || (strName == L"forepushedimage")) {
		SetForeStateImage(kControlStatePushed, strValue);
	}
	else if ((strName == L"fore_disabled_image") || (strName == L"foredisabledimage")) {
		SetForeStateImage(kControlStateDisabled, strValue);
	}
	else if ((strName == L"fade_alpha") || (strName == L"fadealpha")) {
		m_animationManager->SetFadeAlpha(strValue == L"true");
	}
	else if ((strName == L"fade_hot") || (strName == L"fadehot")) {
		m_animationManager->SetFadeHot(strValue == L"true");
	}
	else if ((strName == L"fade_width") || (strName == L"fadewidth")) {
		m_animationManager->SetFadeWidth(strValue == L"true");
	}
	else if ((strName == L"fade_height") || (strName == L"fadeheight")) {
		m_animationManager->SetFadeHeight(strValue == L"true");
	}
	else if ((strName == L"fade_in_out_x_from_left") || (strName == L"fadeinoutxfromleft")) {
		m_animationManager->SetFadeInOutX(strValue == L"true", false);
	}
	else if ((strName == L"fade_in_out_x_from_right") || (strName == L"fadeinoutxfromright")) {
		m_animationManager->SetFadeInOutX(strValue == L"true", true);
	}
	else if ((strName == L"fade_in_out_y_from_top") || (strName == L"fadeinoutyfromtop")) {
		m_animationManager->SetFadeInOutY(strValue == L"true", false);
	}
	else if ((strName == L"fade_in_out_y_from_bottom") || (strName == L"fadeinoutyfrombottom")) {
		m_animationManager->SetFadeInOutY(strValue == L"true", true);
	}
	else if ((strName == L"tab_stop") || (strName == L"tabstop")) {
		SetTabStop(strValue == L"true");
	}
	else if ((strName == L"loading_image") || (strName == L"loadingimage")) {
		SetLoadingImage(strValue);
	}
	else if ((strName == L"loading_bkcolor") || (strName == L"loadingbkcolor")){
		SetLoadingBkColor(strValue);
	}
	else {
		ASSERT(!"Control::SetAttribute失败: 发现不能识别的属性");
	}
}

void Control::SetClass(const std::wstring& strClass)
{
	std::list<std::wstring> splitList = StringHelper::Split(strClass, L" ");
	for (auto it = splitList.begin(); it != splitList.end(); it++) {
		std::wstring pDefaultAttributes = GlobalManager::Instance().GetClassAttributes((*it));
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
	//属性列表，先解析，然后再应用
	std::vector<std::pair<std::wstring, std::wstring>> attributeList;
	AttributeUtil::ParseAttributeList(strList, L'\"', attributeList);
	for (const auto& attribute : attributeList) {
		SetAttribute(attribute.first, attribute.second);
	}
}

bool Control::OnApplyAttributeList(const std::wstring& strReceiver, const std::wstring& strList, const EventArgs& /*eventArgs*/)
{
	bool isFindSubControl = false;
	std::wstring receiverName = strReceiver;
	if (receiverName.size() >= 2) {
		if (receiverName.substr(0, 2) == L".\\" || receiverName.substr(0, 2) == L"./") {
			receiverName = receiverName.substr(2);
			isFindSubControl = true;
		}
	}
	Control* pReceiverControl = nullptr;
	if (isFindSubControl) {
		Box* pBox = dynamic_cast<Box*>(this);
		if (pBox != nullptr) {
			pReceiverControl = pBox->FindSubControl(receiverName);
		}
	}
	else {
		pReceiverControl = GetWindow()->FindControl(receiverName);
	}

	if (pReceiverControl != nullptr) {
		std::wstring strValueList = strList;
		//这个是手工写入的属性，以花括号{}代替双引号，编写的时候就不需要转义字符了；
		StringHelper::ReplaceAll(L"{", L"\"", strValueList);
		StringHelper::ReplaceAll(L"}", L"\"", strValueList);
		pReceiverControl->ApplyAttributeList(strValueList);
		return true;
	}
	else {
		ASSERT(!"Control::OnApplyAttributeList error!");
		return false;
	}	
}

bool Control::PaintImage(IRender* pRender,  Image& duiImage,
					    const std::wstring& strModify, int nFade, IMatrix* pMatrix)
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
	std::shared_ptr<ImageInfo> imageInfo = duiImage.GetImageCache();
	ASSERT(imageInfo != nullptr);
	if (imageInfo == nullptr) {
		return false;
	}

	bool isPlayingGif = false;
	if (m_bGifPlay && !m_bkImage->IsPlaying()   &&
		(m_bkImage->GetImageCache() != nullptr) &&
		m_bkImage->GetImageCache()->IsMultiFrameImage()) {
		isPlayingGif = GifPlay();
	}
	if (isPlayingGif) {
		//如果正在播放背景动画，则不绘制其他图片
		return true;
	}

	IBitmap* pBitmap = duiImage.GetCurrentBitmap();
	ASSERT(pBitmap != nullptr);
	if (pBitmap == nullptr) {
		return false;
	}

	ImageAttribute newImageAttribute = duiImage.GetImageAttribute();
	if (!strModify.empty()) {
		newImageAttribute.ModifyAttribute(strModify);
	}
	UiRect rcDest = GetRect();
	if (ImageAttribute::HasValidImageRect(newImageAttribute.rcDest)) {
		//使用配置中指定的目标区域
		if ((newImageAttribute.rcDest.Width() <= rcDest.Width()) &&
			(newImageAttribute.rcDest.Height() <= rcDest.Height())) {
			rcDest = newImageAttribute.rcDest;
			rcDest.Offset(GetRect().left, GetRect().top);
		}
	}
	UiRect rcDestCorners;
	UiRect rcSource = newImageAttribute.rcSource;
	UiRect rcSourceCorners = newImageAttribute.rcCorner;
	ImageAttribute::ScaleImageRect(pBitmap->GetWidth(), pBitmap->GetHeight(), 
								   imageInfo->IsBitmapSizeDpiScaled(),
		                           rcDestCorners,
		                           rcSource,
		                           rcSourceCorners);
	
	//图片透明度属性
	uint8_t iFade = (nFade == DUI_NOSET_VALUE) ? newImageAttribute.bFade : static_cast<uint8_t>(nFade);
	if (pMatrix != nullptr) {
		//矩阵绘制: 对不支持的属性，增加断言，避免出错
		ASSERT(newImageAttribute.rcCorner.IsEmpty());
		ASSERT(!newImageAttribute.bTiledX);
		ASSERT(!newImageAttribute.bTiledY);
		pRender->DrawImageRect(m_rcPaint, pBitmap, rcDest, rcSource, iFade, pMatrix);
	}
	else{
		pRender->DrawImage(m_rcPaint, pBitmap, rcDest, rcDestCorners, rcSource, rcSourceCorners,
						   iFade, newImageAttribute.bTiledX, newImageAttribute.bTiledY, 
						   newImageAttribute.bFullTiledX, newImageAttribute.bFullTiledY,
						   newImageAttribute.nTiledMargin);
	}
	return true;
}

IRender* Control::GetRender()
{
	if (m_render == nullptr) {
		IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
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

	//绘制剪辑区域
	UiRect rcUnion;
	if (!UiRect::Intersect(rcUnion, rcPaint, GetRect())) {
		return;
	}

	//是否为圆角矩形区域裁剪
	bool bRoundClip = ShouldBeRoundRectFill();

	//当前控件是否设置了透明度（透明度值不是255）
	const bool isAlpha = IsAlpha();

	//是否使用绘制缓存(如果存在box-shadow，就不能使用绘制缓存，因为box-shadow绘制的时候是超出GetRect来绘制外部阴影的)
	const bool isUseCache = IsUseCache() && !m_boxShadow.HasShadow();

	if (isAlpha || isUseCache) {
		//绘制区域（局部绘制）
		UiRect rcUnionRect = rcUnion;
		if (isUseCache) {
			//如果使用绘制缓存，绘制的时候，必须绘制整个区域，因为局部绘制每次请求绘制的区域是不同的，缓存中保存的必须是完整的缓存图
			rcUnionRect = GetRect();
		}
		UiSize size{GetRect().Width(), GetRect().Height() };
		IRender* pCacheRender = GetRender();
		ASSERT(pCacheRender != nullptr);
		if (pCacheRender == nullptr) {
			return;
		}
		bool isSizeChanged = (size.cx != pCacheRender->GetWidth()) || (size.cy != pCacheRender->GetHeight());
		if (!pCacheRender->Resize(size.cx, size.cy)) {
			//存在错误，绘制失败
			ASSERT(!"pCacheRender->Resize failed!");
			return;
		}
		if (isSizeChanged) {
			//Render画布大小发生变化，需要设置缓存脏标记
			SetCacheDirty(true);
		}			
		if (IsCacheDirty()) {
			//重新绘制，首先清楚原内容
			pCacheRender->Clear();
			pCacheRender->SetRenderTransparent(true);

			UiPoint ptOffset(GetRect().left + m_renderOffset.x, GetRect().top + m_renderOffset.y);
			UiPoint ptOldOrg = pCacheRender->OffsetWindowOrg(ptOffset);

			bool hasBoxShadowPainted = m_boxShadow.HasShadow();
			if (hasBoxShadowPainted) {
				//先绘制box-shadow，可能会超出rect边界绘制(如果使用裁剪，可能会显示不全)
				m_isBoxShadowPainted = false;
				PaintShadow(pCacheRender);
				m_isBoxShadowPainted = true;
			}

			UiRect rcClip = { 0, 0, size.cx,size.cy};
			rcClip.Offset((GetRect().left + m_renderOffset.x), (GetRect().top + m_renderOffset.y));
			AutoClip alphaClip(pCacheRender, rcClip, IsClip());
			AutoClip roundAlphaClip(pCacheRender, rcClip, m_cxyBorderRound.cx, m_cxyBorderRound.cy, bRoundClip);		

			//首先绘制自己
			Paint(pCacheRender, rcUnionRect);
			if (hasBoxShadowPainted) {
				//Paint绘制后，立即复位标志，避免影响其他绘制逻辑
				m_isBoxShadowPainted = false;
			}
			if (isAlpha) {
				//设置了透明度，需要先绘制子控件（绘制到pCacheRender上面），然后整体AlphaBlend到pRender
				PaintChild(pCacheRender, rcUnionRect);
			}		
			pCacheRender->SetWindowOrg(ptOldOrg);
			SetCacheDirty(false);
		}

		pRender->AlphaBlend(rcUnionRect.left,
							rcUnionRect.top,
							rcUnionRect.Width(),
							rcUnionRect.Height(),
							pCacheRender,
							rcUnionRect.left - GetRect().left,
							rcUnionRect.top - GetRect().top,
							rcUnionRect.Width(),
							rcUnionRect.Height(),
				            static_cast<uint8_t>(m_nAlpha));
		if (!isAlpha) {
			//没有设置透明度，后绘制子控件（直接绘制到pRender上面）
			PaintChild(pRender, rcUnionRect);
		}
		if (isAlpha) {
			SetCacheDirty(true);
			m_render.reset();
		}
	}
	else {
		UiPoint ptOldOrg = pRender->OffsetWindowOrg(m_renderOffset);
		bool hasBoxShadowPainted = m_boxShadow.HasShadow();
		if (hasBoxShadowPainted) {
			//先绘制box-shadow，可能会超出rect边界绘制(如果使用裁剪，可能会显示不全)
			m_isBoxShadowPainted = false;
			PaintShadow(pRender);
			m_isBoxShadowPainted = true;
		}
		UiRect rcClip = GetRect();
		AutoClip clip(pRender, rcClip, IsClip());
		AutoClip roundClip(pRender, rcClip, m_cxyBorderRound.cx, m_cxyBorderRound.cy, bRoundClip);		
		Paint(pRender, rcPaint);
		if (hasBoxShadowPainted) {
			//Paint绘制后，立即复位标志，避免影响其他绘制逻辑
			m_isBoxShadowPainted = false;
		}
		PaintChild(pRender, rcPaint);
		pRender->SetWindowOrg(ptOldOrg);
	}
}

void Control::SetPaintRect(const UiRect& rect)
{ 
	m_rcPaint = rect; 
}

void Control::Paint(IRender* pRender, const UiRect& rcPaint)
{
	if (!UiRect::Intersect(m_rcPaint, rcPaint, GetRect())) {
		return;
	}	
	if (!m_isBoxShadowPainted) {
		//绘制box-shadow，可能会超出rect边界绘制(如果使用裁剪，可能会显示不全)
		PaintShadow(pRender);
	}	

	//绘制其他内容
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
	if (pRender != nullptr) {
		pRender->DrawBoxShadow(m_rcPaint,
							   m_cxyBorderRound,
							   m_boxShadow.m_cpOffset,
							   m_boxShadow.m_nBlurRadius,
							   m_boxShadow.m_nSpreadRadius,
							   GlobalManager::Instance().Color().GetColor(m_boxShadow.m_strColor));
	}	
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

	UiColor dwBackColor = GetUiColor(m_strBkColor);
	if(dwBackColor.GetARGB() != 0) {
		int nBorderSize = 0;
		if ((m_rcBorderSize.left > 0) &&
			(m_rcBorderSize.left == m_rcBorderSize.right) &&
			(m_rcBorderSize.left == m_rcBorderSize.top) &&
			(m_rcBorderSize.left == m_rcBorderSize.bottom)) {
			//四个边都存在，且大小相同
			nBorderSize = m_rcBorderSize.left;
		}
		nBorderSize /= 2;

		//背景填充矩形范围
		UiRect fillRect = GetRect();
		if (nBorderSize > 0) {
			//如果存在边线，则填充的时候，不填充边线所在位置，避免出现背景色的锯齿现象
			UiRect borderRect(nBorderSize, nBorderSize, nBorderSize, nBorderSize);
			fillRect.Deflate(borderRect.left, borderRect.top, borderRect.right, borderRect.bottom);
		}
		if (ShouldBeRoundRectFill()) {
			//需要绘制圆角矩形，填充也需要填充圆角矩形
			FillRoundRect(pRender, fillRect, m_cxyBorderRound, dwBackColor);
		}
		else {
			pRender->FillRect(fillRect, dwBackColor);
		}
	}
}

void Control::PaintBorder(IRender* pRender)
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	UiColor dwBorderColor((UiColor::ARGB)0);
	if (!m_strBorderColor.empty()) {
		dwBorderColor = GetUiColor(m_strBorderColor);
	}
	if (dwBorderColor.GetARGB() == 0) {
		return;
	}
	if ((m_rcBorderSize.left > 0) &&
		(m_rcBorderSize.left == m_rcBorderSize.right) &&
		(m_rcBorderSize.left == m_rcBorderSize.top) &&
		(m_rcBorderSize.left == m_rcBorderSize.bottom)) {
		//四个边都存在，且大小相同，则直接绘制矩形, 支持圆角矩形
		PaintBorders(pRender, GetRect(), (int)m_rcBorderSize.left, dwBorderColor);
	}
	else {
		//四个边分别按照设置绘制边线
		if (m_rcBorderSize.left > 0) {
			UiRect rcBorder = GetRect();
			rcBorder.right = rcBorder.left = GetRect().left + m_rcBorderSize.left / 2;
			if (m_rcBorderSize.left == 1) {
				rcBorder.bottom -= 1;
			}
			pRender->DrawLine(UiPoint(rcBorder.left, rcBorder.top), UiPoint(rcBorder.right, rcBorder.bottom), dwBorderColor, m_rcBorderSize.left);
		}
		if (m_rcBorderSize.top > 0) {
			UiRect rcBorder = GetRect();
			rcBorder.bottom = rcBorder.top = GetRect().top + m_rcBorderSize.top / 2;
			if (m_rcBorderSize.top == 1) {
				rcBorder.right -= 1;
			}
			pRender->DrawLine(UiPoint(rcBorder.left, rcBorder.top), UiPoint(rcBorder.right, rcBorder.bottom), dwBorderColor, m_rcBorderSize.top);
		}
		if (m_rcBorderSize.right > 0) {
			UiRect rcBorder = GetRect();
			rcBorder.left = rcBorder.right = GetRect().right - (m_rcBorderSize.right + 1) / 2;
			if (m_rcBorderSize.right == 1) {
				rcBorder.bottom -= 1;
			}
			pRender->DrawLine(UiPoint(rcBorder.left, rcBorder.top), UiPoint(rcBorder.right, rcBorder.bottom), dwBorderColor, m_rcBorderSize.right);
		}
		if (m_rcBorderSize.bottom > 0) {
			UiRect rcBorder = GetRect();
			rcBorder.top = rcBorder.bottom = GetRect().bottom - (m_rcBorderSize.bottom + 1) / 2;
			if (m_rcBorderSize.bottom == 1) {
				rcBorder.right -= 1;
			}
			pRender->DrawLine(UiPoint(rcBorder.left, rcBorder.top), UiPoint(rcBorder.right, rcBorder.bottom), dwBorderColor, m_rcBorderSize.bottom);
		}
	}
}

void Control::PaintBorders(IRender* pRender, UiRect rcDraw,
	                       int nBorderSize, UiColor dwBorderColor) const
{
	if ((pRender == nullptr) || rcDraw.IsEmpty() || (nBorderSize < 1) || (dwBorderColor.GetARGB() == 0)) {
		return;
	}
	int nDeltaValue = nBorderSize / 2;
	rcDraw.top += nDeltaValue;
	rcDraw.bottom -= nDeltaValue;
	if (nBorderSize % 2 != 0) {
		rcDraw.bottom -= 1;
	}
	rcDraw.left += nDeltaValue;
	rcDraw.right -= nDeltaValue;
	if (nBorderSize % 2 != 0) {
		rcDraw.right -= 1;
	}
	if (ShouldBeRoundRectBorders()) {
		DrawRoundRect(pRender, rcDraw, m_cxyBorderRound, dwBorderColor, nBorderSize);
	}
	else {
		pRender->DrawRect(rcDraw, dwBorderColor, nBorderSize);
	}
}

bool Control::ShouldBeRoundRectFill() const
{
	bool isRoundRect = false;
	if ((m_rcBorderSize.left >= 0) &&
		(m_rcBorderSize.left == m_rcBorderSize.right) &&
		(m_rcBorderSize.left == m_rcBorderSize.top)   &&
		(m_rcBorderSize.left == m_rcBorderSize.bottom)) {
		//四个边大小相同(无论是零还是大于零)，支持圆角矩形
		if (m_cxyBorderRound.cx > 0 && m_cxyBorderRound.cy > 0) {
			isRoundRect = true;
		}
	}
	if (isRoundRect) {
		//窗口最大化时，对于Root Box控件，不绘制圆角边线
		if (IsRootBox() && (GetWindow() != nullptr)) {
			if (GetWindow()->IsWindowMaximized()) {
				isRoundRect = false;
			}
		}
	}	
	return isRoundRect;
}

bool Control::ShouldBeRoundRectBorders() const
{
	bool isRoundRect = ShouldBeRoundRectFill();
	return isRoundRect && (m_rcBorderSize.left > 0);
}

bool Control::IsRootBox() const
{
	bool isRootBox = false;
	Window* pWindow = GetWindow();
	if (pWindow != nullptr) {
		Box* pRoot = pWindow->GetRoot();
		if ((Control*)pRoot == this) {
			//没有Attach到阴影的情况
			isRootBox = true;
		}
		else if ((pRoot != nullptr) && pWindow->IsShadowAttached() && (pRoot->GetItemAt(0) == this)) {
			//已经Attach到阴影的情况
			isRootBox = true;
		}
	}
	return isRootBox;
}

bool Control::IsWindowRoundRect() const
{
	bool isWindowRoundRect = false;
	Window* pWindow = GetWindow();
	if (pWindow != nullptr) {
		UiSize roundCorner = pWindow->GetRoundCorner();
		if ((roundCorner.cx > 0) && (roundCorner.cy > 0)) {
			isWindowRoundRect = true;
		}
	}
	return isWindowRoundRect;
}

void Control::AddRoundRectPath(IPath* path, const UiRect& rc, const UiSize& roundSize) const
{
	ASSERT(path != nullptr);
	if (path == nullptr) {
		return;
	}
	//这种画法的圆角形状，与CreateRoundRectRgn产生的圆角形状，基本一致的
	path->AddArc(UiRect((INT)rc.left, rc.top, rc.left + roundSize.cx, rc.top + roundSize.cy), 180, 90);
	path->AddLine(rc.left + roundSize.cx / 2, (INT)rc.top, rc.right - roundSize.cx / 2, rc.top);
	path->AddArc(UiRect(rc.right - roundSize.cx, (INT)rc.top, rc.right, rc.top + roundSize.cy), 270, 90);
	path->AddLine((INT)rc.right, rc.top + roundSize.cy / 2, rc.right, rc.bottom - roundSize.cy / 2);
	path->AddArc(UiRect(rc.right - roundSize.cx, rc.bottom - roundSize.cy, rc.right, rc.bottom), 0, 90);
	path->AddLine(rc.right - roundSize.cx / 2, (INT)rc.bottom, rc.left + roundSize.cx / 2, rc.bottom);
	path->AddArc(UiRect(rc.left, rc.bottom - roundSize.cy, rc.left + roundSize.cx, rc.bottom), 90, 90);
	path->AddLine((INT)rc.left, rc.bottom - roundSize.cy / 2, rc.left, rc.top + roundSize.cy / 2);
	path->Close();
}

void Control::DrawRoundRect(IRender* pRender, const UiRect& rc, const UiSize& roundSize, UiColor dwBorderColor, int nBorderSize) const
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (pRender->GetRenderType() != RenderType::kRenderType_Skia) {
		pRender->DrawRoundRect(rc, roundSize, dwBorderColor, nBorderSize);
		return;
	}
	bool isDrawOk = false;
	if (IsRootBox() && IsWindowRoundRect()) {
		//使用与Windows一致的绘制方式，避免与Windows的不一致
		//参见：Window::OnSizeMsg中的CreateRoundRectRgn（Skia的圆角画法和CreateRoundRectRgn不一样）
		IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
		if (pRenderFactory != nullptr) {
			std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(dwBorderColor, nBorderSize));
			std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
			if (pen && path) {
				//这种画法的圆角形状，与CreateRoundRectRgn产生的圆角形状，基本一致的
				AddRoundRectPath(path.get(), rc, roundSize);
				pRender->DrawPath(path.get(), pen.get());
				isDrawOk = true;
			}
		}
	}
	if(!isDrawOk) {
		pRender->DrawRoundRect(rc, roundSize, dwBorderColor, nBorderSize);
	}
}

void Control::FillRoundRect(IRender* pRender, const UiRect& rc, const UiSize& roundSize, UiColor dwColor) const
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (pRender->GetRenderType() != RenderType::kRenderType_Skia) {
		pRender->FillRoundRect(rc, roundSize, dwColor);
		return;
	}

	bool isDrawOk = false;
	if (IsRootBox() && IsWindowRoundRect()) {
		//使用与Windows一致的绘制方式，避免与Windows的不一致
		//参见：Window::OnSizeMsg中的CreateRoundRectRgn（Skia的圆角画法和CreateRoundRectRgn不一样）
		IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
		if (pRenderFactory != nullptr) {
			std::unique_ptr<IBrush> brush(pRenderFactory->CreateBrush(dwColor));
			std::unique_ptr<IPath> path(pRenderFactory->CreatePath());
			if (brush && path) {
				//这种画法的圆角形状，与CreateRoundRectRgn产生的圆角形状，基本一致的
				AddRoundRectPath(path.get(), rc, roundSize);
				pRender->FillPath(path.get(), brush.get());
				isDrawOk = true;
			}
		}
	}
	if (!isDrawOk) {
		pRender->FillRoundRect(rc, roundSize, dwColor);
	}	
}

void Control::PaintBkImage(IRender* pRender)
{
	PaintImage(pRender, *m_bkImage);
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
	rcFill.left = GetRect().left + (GetRect().Width() - imageWidth) / 2;
	rcFill.right = rcFill.left + imageWidth;
	rcFill.top = GetRect().top + (GetRect().Height() - imageHeight) / 2;
	rcFill.bottom = rcFill.top + imageHeight;

	ui::UiRect rcDest = m_loadingImage->GetImageAttribute().rcDest;
	if (!rcDest.IsEmpty()) {
		rcFill = rcDest;
		rcFill.Offset(GetRect().left, GetRect().top);
	}

    if (!m_strLoadingBkColor.empty()) {
        pRender->FillRect(rcFill, GetUiColor(m_strLoadingBkColor));
    }

	UiRect imageDestRect = rcFill;
	rcFill.Offset(-GetRect().left, -GetRect().top);

	//图片旋转矩阵
	IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
	ASSERT(pRenderFactory != nullptr);
	if (pRenderFactory == nullptr) {
		return;
	}
	std::unique_ptr<IMatrix> spMatrix(pRenderFactory->CreateMatrix());
	if (spMatrix != nullptr){
		spMatrix->RotateAt((float)m_fCurrrentAngele, imageDestRect.Center());
	}
	
	wchar_t modify[64] = { 0 };
	swprintf_s(modify, L"destscale='false' dest='%d,%d,%d,%d'", rcFill.left, rcFill.top, rcFill.right, rcFill.bottom);	
	
	//绘制时需要设置裁剪区域，避免绘制超出范围（因为旋转图片后，图片区域会超出显示区域）
	AutoClip autoClip(pRender, imageDestRect, true);
	PaintImage(pRender, *m_loadingImage, modify, -1, spMatrix.get());
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
		GlobalManager::Instance().Timer().AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(),
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
				GlobalManager::Instance().Timer().AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(),
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
		GlobalManager::Instance().Timer().AddCancelableTimer(m_gifWeakFlag.GetWeakFlag(),
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
	std::wstring imageFullPath = GlobalManager::Instance().GetResFullPath(pWindow->GetResourcePath(), sImagePath);
	ImageLoadAttribute imageLoadAttr = duiImage.GetImageLoadAttribute();
	imageLoadAttr.SetImageFullPath(imageFullPath);
	std::shared_ptr<ImageInfo> imageCache = duiImage.GetImageCache();
	if ((imageCache == nullptr) || 
		(imageCache->GetCacheKey() != imageLoadAttr.GetCacheKey())) {
		//如果图片没有加载则执行加载图片；如果图片发生变化，则重新加载该图片
		imageCache = GlobalManager::Instance().Image().GetImage(imageLoadAttr);
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
	std::wstring imageFullPath = GlobalManager::Instance().GetResFullPath(pWindow->GetResourcePath(), sImagePath);
	ImageLoadAttribute imageLoadAttr = m_bkImage->GetImageLoadAttribute();
	imageLoadAttr.SetImageFullPath(imageFullPath);
	if (!m_bkImage->GetImageCache() || m_bkImage->GetImageCache()->GetCacheKey() != imageLoadAttr.GetCacheKey()) {
		auto shared_image = GlobalManager::Instance().Image().GetCachedImage(imageLoadAttr);
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

bool Control::HasUiColor(const std::wstring& colorName) const
{
	UiColor color = GetUiColorByName(colorName);
	return color.GetARGB() != 0;
}

UiColor Control::GetUiColor(const std::wstring& colorName) const
{
	UiColor color = GetUiColorByName(colorName);
	ASSERT(color.GetARGB() != 0);
	return color;
}

UiColor Control::GetUiColorByName(const std::wstring& colorName) const
{
	UiColor color;
	if (colorName.empty()) {
		return color;
	}
	if (colorName.at(0) == L'#') {
		//优先级1：以'#'字符开头，直接指定颜色值，举例：#FFFFFFFF
		color = ColorManager::ConvertToUiColor(colorName);
	}
	if (color.GetARGB() == 0) {
		Window* pWindow = GetWindow();
		if (pWindow != nullptr) {
			//优先级2：获取在配置XML中的<Window>节点中定义子节点<TextColor>指定的颜色
			color = pWindow->GetTextColor(colorName);
		}
	}
	if (color.GetARGB() == 0) {
		//优先级3：获取在global.xml中的<Global>节点中定义子节点<TextColor>指定的颜色
		color = GlobalManager::Instance().Color().GetColor(colorName);
	}
	if (color.GetARGB() == 0) {
		//优先级4：直接指定预定义的颜色别名
		color = GlobalManager::Instance().Color().GetColor(colorName);
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
	GlobalManager::Instance().Timer().AddCancelableTimer(m_loadingImageFlag.GetWeakFlag(), nbase::Bind(&Control::Loading, this),
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
