#include "Control.h"
#include "duilib/Core/ControlLoading.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/Box.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/ColorManager.h"
#include "duilib/Core/StateColorMap.h"
#include "duilib/Image/Image.h"
#include "duilib/Render/IRender.h"
#include "duilib/Render/AutoClip.h"
#include "duilib/Animation/AnimationPlayer.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui 
{
Control::Control() :
	m_bContextMenuUsed(false),
	m_bEnabled(true),
	m_bMouseEnabled(true),
	m_bKeyboardEnabled(true),
	m_bMouseFocused(false),
	m_bNoFocus(false),
	m_bClip(true),
	m_bAllowTabstop(true),
	m_renderOffset(),
	m_cxyBorderRound(),
	m_rcPaint(),
	m_rcBorderSize(),
	m_cursorType(kCursorArrow),
	m_controlState(kControlStateNormal),
	m_nTooltipWidth(300),
	m_nAlpha(255),
	m_nHotAlpha(0),
	m_sToolTipText(),
	m_sToolTipTextId(),
	m_sUserDataID(),
	m_strBkColor(),
	m_pBoxShadow(nullptr),
	m_isBoxShadowPainted(false),
	m_uUserDataID((size_t)-1),
	m_pOnEvent(nullptr),
	m_pOnXmlEvent(nullptr),
	m_pOnBubbledEvent(nullptr),
	m_pOnXmlBubbledEvent(nullptr),
	m_pLoading(nullptr),
	m_bShowFocusRect(false),
	m_nPaintOrder(0)
{
}

Control::~Control()
{
	//派发最后一个事件
	SendEvent(kEventLast);

	//清理动画相关资源，避免定时器再产生回调，引发错误
	if (m_animationManager != nullptr) {
		m_animationManager->Clear(this);
	}	
	m_animationManager.reset();

	Window* pWindow = GetWindow();
	if (pWindow) {
		pWindow->ReapObjects(this);
	}

	if (m_pLoading != nullptr) {
		delete m_pLoading;
		m_pLoading = nullptr;
	}

	if (m_pBoxShadow != nullptr) {
		delete m_pBoxShadow;
		m_pBoxShadow = nullptr;
	}
	if (m_pOnEvent != nullptr) {
		delete m_pOnEvent;
		m_pOnEvent = nullptr;
	}
	if (m_pOnXmlEvent != nullptr) {
		delete m_pOnXmlEvent;
		m_pOnXmlEvent = nullptr;
	}
	if (m_pOnBubbledEvent != nullptr) {
		delete m_pOnBubbledEvent;
		m_pOnBubbledEvent = nullptr;
	}
	if (m_pOnXmlBubbledEvent != nullptr) {
		delete m_pOnXmlBubbledEvent;
		m_pOnXmlBubbledEvent = nullptr;
	}
}

std::wstring Control::GetType() const { return DUI_CTR_CONTROL; }

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
		UiMargin rcMargin;
		AttributeUtil::ParseMarginValue(strValue.c_str(), rcMargin);
		SetMargin(rcMargin, true);
	}
	else if (strName == L"padding") {
		UiPadding rcPadding;
		AttributeUtil::ParsePaddingValue(strValue.c_str(), rcPadding);
		SetPadding(rcPadding, true);
	}
	else if (strName == L"control_padding") {
		SetEnableControlPadding(strValue == L"true");
	}
	else if (strName == L"bkcolor") {
		SetBkColor(strValue);
	}
	else if ((strName == L"border_size") || (strName == L"bordersize")) {
		std::wstring nValue = strValue;
		if (nValue.find(L',') == std::wstring::npos) {
			int32_t nBorderSize = _wtoi(strValue.c_str());
			if (nBorderSize < 0) {
				nBorderSize = 0;
			}
			UiRect rcBorder(nBorderSize, nBorderSize, nBorderSize, nBorderSize);
			SetBorderSize(rcBorder);
		}
		else {
			UiMargin rcMargin;
			AttributeUtil::ParseMarginValue(strValue.c_str(), rcMargin);
			UiRect rcBorder(rcMargin.left, rcMargin.top, rcMargin.right, rcMargin.bottom);
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
	else if (strName == L"width") {
		if (strValue == L"stretch") {
			//宽度为拉伸：由父容器负责分配宽度
			SetFixedWidth(UiFixedInt::MakeStretch(), true, true);
		}
		else if (strValue == L"auto") {
			//宽度为自动：根据控件的文本、图片等自动计算宽度
			SetFixedWidth(UiFixedInt::MakeAuto(), true, true);
		}
		else if (!strValue.empty()) {
			if (strValue.back() == L'%') {
				//宽度为拉伸：由父容器负责按百分比分配宽度，比如 width="30%"，代表该控件的宽度期望值为父控件宽度的30%
				int32_t iValue = _wtoi(strValue.c_str());
				if ((iValue <= 0) || (iValue > 100)) {
					iValue = 100;
				}
				SetFixedWidth(UiFixedInt::MakeStretch(iValue), true, false);
			}
			else {
				//宽度为固定值
				ASSERT(_wtoi(strValue.c_str()) >= 0);
				SetFixedWidth(UiFixedInt(_wtoi(strValue.c_str())), true, true);
			}
		}
		else {
			SetFixedWidth(UiFixedInt(0), true, true);
		}
	}
	else if (strName == L"height") {
		if (strValue == L"stretch") {
			//高度为拉伸：由父容器负责分配高度
			SetFixedHeight(UiFixedInt::MakeStretch(), true, true);
		}
		else if (strValue == L"auto") {
			//高度为自动：根据控件的文本、图片等自动计算高度
			SetFixedHeight(UiFixedInt::MakeAuto(), true, true);
		}
		else if (!strValue.empty()) {
			if (strValue.back() == L'%') {
				//高度为拉伸：由父容器负责按百分比分配高度，比如 height="30%"，代表该控件的高度期望值为父控件高度的30%
				int32_t iValue = _wtoi(strValue.c_str());
				if ((iValue <= 0) || (iValue > 100)) {
					iValue = 100;
				}
				SetFixedHeight(UiFixedInt::MakeStretch(iValue), true, false);
			}
			else {
				//高度为固定值
				ASSERT(_wtoi(strValue.c_str()) >= 0);
				SetFixedHeight(UiFixedInt(_wtoi(strValue.c_str())), true, true);
			}
		}
		else {
			SetFixedHeight(UiFixedInt(0), true, true);
		}
	}
	else if (strName == L"state") {
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
	else if ((strName == L"cursor_type") || (strName == L"cursortype")) {
		if (strValue == L"arrow") {
			SetCursorType(kCursorArrow);
		}
		else if (strValue == L"hand") {
			SetCursorType(kCursorHand);
		}
		else if (strValue == L"ibeam") {
			SetCursorType(kCursorHandIbeam);
		}
		else if (strValue == L"sizewe") {
			SetCursorType(kCursorSizeWE);
		}
		else if (strValue == L"sizens") {
			SetCursorType(kCursorSizeNS);
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
	else if (strName == L"normal_border_color") {
		SetBorderColor(kControlStateNormal, strValue);
	}
	else if (strName == L"hot_border_color") {
		SetBorderColor(kControlStateHot, strValue);
	}
	else if (strName == L"pushed_border_color") {
		SetBorderColor(kControlStatePushed, strValue);
	}
	else if (strName == L"disabled_border_color") {
		SetBorderColor(kControlStateDisabled, strValue);
	}
	else if (strName == L"focus_border_color") {
		SetFocusBorderColor(strValue);
	}
	else if ((strName == L"left_border_size") || (strName == L"leftbordersize")) {
		SetLeftBorderSize(_wtoi(strValue.c_str()));
	}
	else if ((strName == L"top_border_size") || (strName == L"topbordersize")) {
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
	else if (strName == L"user_dataid") {
		SetUserDataID(_wtoi(strValue.c_str()));
	}
	else if (strName == L"enabled") {
		SetEnabled(strValue == L"true");
	}
	else if ((strName == L"mouse_enabled") || (strName == L"mouse")) {
		SetMouseEnabled(strValue == L"true");
	}
	else if ((strName == L"keyboard_enabled") || (strName == L"keyboard")) {
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
		GetAnimationManager().SetFadeAlpha(strValue == L"true");
	}
	else if ((strName == L"fade_hot") || (strName == L"fadehot")) {
		GetAnimationManager().SetFadeHot(strValue == L"true");
	}
	else if ((strName == L"fade_width") || (strName == L"fadewidth")) {
		GetAnimationManager().SetFadeWidth(strValue == L"true");
	}
	else if ((strName == L"fade_height") || (strName == L"fadeheight")) {
		GetAnimationManager().SetFadeHeight(strValue == L"true");
	}
	else if ((strName == L"fade_in_out_x_from_left") || (strName == L"fadeinoutxfromleft")) {
		GetAnimationManager().SetFadeInOutX(strValue == L"true", false);
	}
	else if ((strName == L"fade_in_out_x_from_right") || (strName == L"fadeinoutxfromright")) {
		GetAnimationManager().SetFadeInOutX(strValue == L"true", true);
	}
	else if ((strName == L"fade_in_out_y_from_top") || (strName == L"fadeinoutyfromtop")) {
		GetAnimationManager().SetFadeInOutY(strValue == L"true", false);
	}
	else if ((strName == L"fade_in_out_y_from_bottom") || (strName == L"fadeinoutyfrombottom")) {
		GetAnimationManager().SetFadeInOutY(strValue == L"true", true);
	}
	else if ((strName == L"tab_stop") || (strName == L"tabstop")) {
		SetTabStop(strValue == L"true");
	}
	else if ((strName == L"loading_image") || (strName == L"loadingimage")) {
		SetLoadingImage(strValue);
	}
	else if ((strName == L"loading_bkcolor") || (strName == L"loadingbkcolor")) {
		SetLoadingBkColor(strValue);
	}
	else if (strName == L"show_focus_rect") {
		SetShowFocusRect(strValue == L"true");
	}
	else if (strName == L"focus_rect_color") {
		SetFocusRectColor(strValue);
	}
	else if (strName == L"paint_order") {
		uint8_t nPaintOrder = TruncateToUInt8(_wtoi(strValue.c_str()));
		SetPaintOrder(nPaintOrder);
	}
	else if (strName == L"start_gif_play") {
		int32_t nPlayCount = _wtoi(strValue.c_str());
		StartGifPlay(kGifFrameCurrent, nPlayCount);
	}
	else if (strName == L"stop_gif_play") {
		GifFrameType nStopFrame = (GifFrameType)_wtoi(strValue.c_str());
		StopGifPlay(false, nStopFrame);
	}
	else {
		ASSERT(!"Control::SetAttribute失败: 发现不能识别的属性");
	}
}

void Control::SetClass(const std::wstring& strClass)
{
	if (strClass.empty()) {
		return;
	}
	std::list<std::wstring> splitList = StringHelper::Split(strClass, L" ");
	for (auto it = splitList.begin(); it != splitList.end(); it++) {
		std::wstring pDefaultAttributes = GlobalManager::Instance().GetClassAttributes((*it));
		Window* pWindow = GetWindow();
		if (pDefaultAttributes.empty() && (pWindow != nullptr)) {
			pDefaultAttributes = pWindow->GetClassAttributes(*it);
		}

		ASSERT(!pDefaultAttributes.empty());
		if (!pDefaultAttributes.empty()) {
			ApplyAttributeList(pDefaultAttributes);
		}
	}
}

void Control::ApplyAttributeList(const std::wstring& strList)
{
	//属性列表，先解析，然后再应用
	if (strList.empty()) {
		return;
	}
	std::vector<std::pair<std::wstring, std::wstring>> attributeList;
	if (strList.find(L'\"') != std::wstring::npos) {
		AttributeUtil::ParseAttributeList(strList, L'\"', attributeList);
	}	
	else if (strList.find(L'\'') != std::wstring::npos) {
		AttributeUtil::ParseAttributeList(strList, L'\'', attributeList);
	}
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
	else if (!receiverName.empty()) {
		pReceiverControl = GetWindow()->FindControl(receiverName);
	}
	else {
		pReceiverControl = this;
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

AnimationManager& Control::GetAnimationManager()
{
	if (m_animationManager == nullptr) {
		m_animationManager = std::make_unique<AnimationManager>(),
		m_animationManager->Init(this);
	}
	return *m_animationManager;
}

void Control::SetBkColor(const std::wstring& strColor)
{
	ASSERT(strColor.empty() || HasUiColor(strColor));
	if (m_strBkColor == strColor) {
		return;
	}
	m_strBkColor = strColor;
	Invalidate();
}

void Control::SetBkColor(const UiColor& color)
{
	if (color.IsEmpty()) {
		SetBkColor(L"");
	}
	else {
		SetBkColor(GetColorString(color));
	}	
}

std::wstring Control::GetStateColor(ControlStateType stateType) const
{
	if (m_pColorMap != nullptr) {
		return m_pColorMap->GetStateColor(stateType);
	}
	return std::wstring();
}

void Control::SetStateColor(ControlStateType stateType, const std::wstring& strColor)
{
	ASSERT(strColor.empty() || HasUiColor(strColor));
	if (m_pColorMap != nullptr) {
		if (m_pColorMap->GetStateColor(stateType) == strColor) {
			return;
		}
	}
	if (m_pColorMap == nullptr) {
		m_pColorMap = std::make_unique<StateColorMap>();
		m_pColorMap->SetControl(this);
	}
	m_pColorMap->SetStateColor(stateType, strColor);
	if (stateType == kControlStateHot) {
		GetAnimationManager().SetFadeHot(true);
	}
	Invalidate();
}

std::wstring Control::GetBkImage() const
{
	if (m_pBkImage != nullptr) {
		return m_pBkImage->GetImageString();
	}
	return std::wstring();
}

std::string Control::GetUTF8BkImage() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(GetBkImage(), strOut, CP_UTF8);
	return strOut;
}

void Control::SetBkImage(const std::wstring& strImage)
{
	CheckStopGifPlay();
	if (!strImage.empty()) {
		if (m_pBkImage == nullptr) {
			m_pBkImage = std::make_shared<Image>();
			m_pBkImage->SetControl(this);
		}
	}
	if (m_pBkImage != nullptr) {
		m_pBkImage->SetImageString(strImage);
	}
	RelayoutOrRedraw();
}

void Control::SetUTF8BkImage(const std::string& strImage)
{
	std::wstring strOut;
	StringHelper::MBCSToUnicode(strImage, strOut, CP_UTF8);
	SetBkImage(strOut);
}

void Control::SetLoadingImage(const std::wstring& strImage) 
{
	if (!strImage.empty()) {
		if (m_pLoading == nullptr) {
			m_pLoading = new ControlLoading(this);
		}
	}
	if (m_pLoading != nullptr) {
		if (m_pLoading->SetLoadingImage(strImage)) {
			Invalidate();
		}
	}
}

void Control::SetLoadingBkColor(const std::wstring& strColor) 
{
	if (m_pLoading != nullptr) {
		if (m_pLoading->SetLoadingBkColor(strColor)) {
			Invalidate();
		}
	}    
}

void Control::StartLoading(int32_t fStartAngle)
{
	if ((m_pLoading != nullptr) && m_pLoading->StartLoading(fStartAngle)) {
		SetEnabled(false);
	}
}

void Control::StopLoading(GifFrameType frame)
{
	if (m_pLoading != nullptr) {
		m_pLoading->StopLoading(frame);
	}
	SetEnabled(true);
}

bool Control::HasStateImages(void) const
{
	if (m_pImageMap != nullptr) {
		return m_pImageMap->HasStateImages();
	}
	return false;
}

bool Control::HasStateImage(StateImageType stateImageType) const
{
	if (m_pImageMap != nullptr) {
		return m_pImageMap->HasStateImage(stateImageType);
	}
	return false;
}

std::wstring Control::GetStateImage(StateImageType imageType, ControlStateType stateType) const
{
	if (m_pImageMap != nullptr) {
		return m_pImageMap->GetImageString(imageType, stateType);
	}
	return std::wstring();
}

void Control::SetStateImage(StateImageType imageType, ControlStateType stateType, const std::wstring& strImage)
{
	if (m_pImageMap == nullptr) {
		m_pImageMap = std::make_unique<StateImageMap>();
		m_pImageMap->SetControl(this);
	}
	m_pImageMap->SetImageString(imageType, stateType, strImage);
}

bool Control::PaintStateImage(IRender* pRender, StateImageType stateImageType, 
							  ControlStateType stateType, 
						      const std::wstring& sImageModify,
	                          UiRect* pDestRect)
{
	if (m_pImageMap != nullptr) {
		return m_pImageMap->PaintStateImage(pRender, stateImageType, stateType, sImageModify, pDestRect);
	}
	return false;
}

UiSize Control::GetStateImageSize(StateImageType imageType, ControlStateType stateType)
{
	Image* pImage = nullptr;
	if (m_pImageMap != nullptr) {
		pImage = m_pImageMap->GetStateImage(imageType, stateType);
	}
	UiSize imageSize;
	if (pImage != nullptr) {
		LoadImageData(*pImage);
		std::shared_ptr<ImageInfo> imageInfo = pImage->GetImageCache();
		if (imageInfo != nullptr) {
			imageSize.cx = imageInfo->GetWidth();
			imageSize.cy = imageInfo->GetHeight();
		}
	}
	return imageSize;
}

void Control::ClearStateImages()
{
	m_pImageMap.reset();
	RelayoutOrRedraw();
}

std::wstring Control::GetStateImage(ControlStateType stateType) const
{
	return GetStateImage(kStateImageBk, stateType);
}

void Control::SetStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (stateType == kControlStateHot) {
		GetAnimationManager().SetFadeHot(true);
	}
	SetStateImage(kStateImageBk, stateType, strImage);
	RelayoutOrRedraw();
}

std::wstring Control::GetForeStateImage(ControlStateType stateType) const
{
	return GetStateImage(kStateImageFore, stateType);
}

void Control::SetForeStateImage(ControlStateType stateType, const std::wstring& strImage)
{
	if (stateType == kControlStateHot) {
		GetAnimationManager().SetFadeHot(true);
	}
	SetStateImage(kStateImageFore, stateType, strImage);
	Invalidate();
}

bool Control::AdjustStateImagesPaddingLeft(int32_t leftOffset, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(leftOffset);
	}
	if (leftOffset == 0) {
		return false;
	}
	std::vector<Image*> allImages;
	if (m_pImageMap != nullptr) {
		m_pImageMap->GetAllImages(allImages);
	}
	bool bSetOk = false;
	UiPadding rcPadding;
	for (Image* pImage : allImages) {
		rcPadding = pImage->GetImagePadding();
		rcPadding.left += leftOffset;
		if (rcPadding.left < 0) {
			rcPadding.left = 0;
		}
		if (!pImage->GetImagePadding().Equals(rcPadding)) {
			pImage->SetImagePadding(rcPadding);
			bSetOk = true;
		}
	}
	if (bSetOk) {
		Invalidate();
	}
	return bSetOk;
}

UiPadding Control::GetBkImagePadding() const
{
	UiPadding rcPadding;
	if (m_pBkImage != nullptr) {
		rcPadding = m_pBkImage->GetImagePadding();
	}
	return rcPadding;
}

bool Control::SetBkImagePadding(UiPadding rcPadding, bool bNeedDpiScale)
{
	bool bSetOk = false;
	if (m_pBkImage != nullptr) {
		if (bNeedDpiScale) {
			GlobalManager::Instance().Dpi().ScalePadding(rcPadding);
		}
		if (!m_pBkImage->GetImagePadding().Equals(rcPadding)) {
			m_pBkImage->SetImagePadding(rcPadding);
			bSetOk = true;
			Invalidate();
		}		
	}
	return bSetOk;
}

bool Control::IsBkImagePaintEnabled() const
{
	if (m_pBkImage != nullptr) {
		return m_pBkImage->IsImagePaintEnabled();
	}
	return true;
}

void Control::SetBkImagePaintEnabled(bool bEnable)
{
	if (m_pBkImage != nullptr) {
		bool isChange = m_pBkImage->IsImagePaintEnabled() != bEnable;		
		if (isChange) {
			m_pBkImage->SetImagePaintEnabled(bEnable);
			Invalidate();
		}
	}
}

std::wstring Control::GetBkImagePath() const
{
	if (m_pBkImage != nullptr) {
		return m_pBkImage->GetImagePath();
	}
	return std::wstring();
}

UiSize Control::GetBkImageSize() const
{
	UiSize imageSize;
	if (m_pBkImage != nullptr) {
		std::shared_ptr<ImageInfo> imageInfo = m_pBkImage->GetImageCache();
		if (imageInfo == nullptr) {
			LoadImageData(*m_pBkImage);
			imageInfo = m_pBkImage->GetImageCache();
		}
		if (imageInfo != nullptr) {
			imageSize.cx = imageInfo->GetWidth();
			imageSize.cy = imageInfo->GetHeight();
		}
	}
	return imageSize;
}

ControlStateType Control::GetState() const
{
	return static_cast<ControlStateType>(m_controlState);
}

void Control::SetState(ControlStateType controlState)
{
	if (controlState == kControlStateNormal) {
		m_nHotAlpha = 0;
	}
	else if (controlState == kControlStateHot) {
		m_nHotAlpha = 255;
	}
	PrivateSetState(controlState);
	Invalidate();
}

void Control::PrivateSetState(ControlStateType controlState)
{
	if (GetState() != controlState) {
		ControlStateType oldState = GetState();
		m_controlState = TruncateToInt8(controlState);
		SendEvent(kEventStateChange, controlState, oldState);
		Invalidate();
	}
}

bool Control::IsHotState() const
{
	return (GetState() == kControlStateHot) ? true : false;
}

std::wstring Control::GetBorderColor(ControlStateType stateType) const
{
	std::wstring borderColor;
	if (m_pBorderColorMap != nullptr) {
		borderColor = m_pBorderColorMap->GetStateColor(stateType);
	}
	return borderColor;
}

void Control::SetBorderColor(const std::wstring& strBorderColor)
{
	SetBorderColor(kControlStateNormal, strBorderColor);
	SetBorderColor(kControlStateHot, strBorderColor);
	SetBorderColor(kControlStatePushed, strBorderColor);
	SetBorderColor(kControlStateDisabled, strBorderColor);
}

void Control::SetBorderColor(ControlStateType stateType, const std::wstring& strBorderColor)
{
	if (m_pBorderColorMap == nullptr) {
		m_pBorderColorMap = std::make_unique<StateColorMap>();
		m_pBorderColorMap->SetControl(this);
	}
	if (GetBorderColor(stateType) != strBorderColor) {
		m_pBorderColorMap->SetStateColor(stateType, strBorderColor);
		Invalidate();
	}
}

void Control::SetFocusBorderColor(const std::wstring& strBorderColor)
{
	if (m_focusBorderColor != strBorderColor) {
		m_focusBorderColor = strBorderColor;
		Invalidate();
	}
}

std::wstring Control::GetFocusBorderColor() const
{
	return m_focusBorderColor.c_str();
}

void Control::SetBorderSize(UiRect rc, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleRect(rc);
	}
	rc.left = std::max(rc.left, 0);
	rc.top = std::max(rc.top, 0);
	rc.right = std::max(rc.right, 0);
	rc.bottom = std::max(rc.bottom, 0);
	if (m_rcBorderSize != rc) {
		m_rcBorderSize = rc;
		Invalidate();
	}	
}

int32_t Control::GetLeftBorderSize() const
{
	return m_rcBorderSize.left;
}

void Control::SetLeftBorderSize(int32_t nSize, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(nSize);
	}
	if (m_rcBorderSize.left != nSize) {
		m_rcBorderSize.left = nSize;
		Invalidate();
	}	
}

int32_t Control::GetTopBorderSize() const
{
	return m_rcBorderSize.top;
}

void Control::SetTopBorderSize(int32_t nSize, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(nSize);
	}
	if (m_rcBorderSize.top != nSize) {
		m_rcBorderSize.top = nSize;
		Invalidate();
	}	
}

int32_t Control::GetRightBorderSize() const
{
	return m_rcBorderSize.right;
}

void Control::SetRightBorderSize(int32_t nSize, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(nSize);
	}
	if (m_rcBorderSize.right != nSize) {
		m_rcBorderSize.right = nSize;
		Invalidate();
	}	
}

int32_t Control::GetBottomBorderSize() const
{
	return m_rcBorderSize.bottom;
}

void Control::SetBottomBorderSize(int32_t nSize, bool bNeedDpiScale)
{
	if (bNeedDpiScale) {
		GlobalManager::Instance().Dpi().ScaleInt(nSize);
	}
	if (m_rcBorderSize.bottom != nSize) {
		m_rcBorderSize.bottom = nSize;
		Invalidate();
	}	
}

const UiSize& Control::GetBorderRound() const
{
    return m_cxyBorderRound;
}

void Control::SetBorderRound(UiSize cxyRound)
{
	int32_t cx = cxyRound.cx;
	int32_t cy = cxyRound.cy;
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
	if (strShadow.empty()) {
		return;
	}
	if (m_pBoxShadow == nullptr) {
		m_pBoxShadow = new BoxShadow;
	}
	m_pBoxShadow->SetBoxShadowString(strShadow);
}

CursorType Control::GetCursorType() const
{
	return static_cast<CursorType>(m_cursorType);
}

void Control::SetCursorType(CursorType cursorType)
{
	m_cursorType = TruncateToInt8(cursorType);
}

std::wstring Control::GetToolTipText() const
{
	std::wstring strText = m_sToolTipText.c_str();
	if (strText.empty() && !m_sToolTipTextId.empty()) {
		strText = GlobalManager::Instance().Lang().GetStringViaID(m_sToolTipTextId.c_str());
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
	if (strText != m_sToolTipText) {
		std::wstring strTemp(strText);
		StringHelper::ReplaceAll(L"<n>", L"\r\n", strTemp);
		m_sToolTipText = strTemp;
		Invalidate();

		if (GetWindow() != nullptr) {
			Control* pHover = GetWindow()->GetHoverControl();
			if (pHover == this) {
				//更新ToolTip的显示
				GetWindow()->UpdateToolTip();
			}
		}		
	}	
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

void Control::SetToolTipWidth( int32_t nWidth )
{
	GlobalManager::Instance().Dpi().ScaleInt(nWidth);
	if (nWidth < 0) {
		nWidth = 0;
	}
	m_nTooltipWidth = TruncateToUInt16(nWidth);
}

int32_t Control::GetToolTipWidth(void) const
{
	return m_nTooltipWidth;
}

void Control::SetContextMenuUsed(bool bMenuUsed)
{
	m_bContextMenuUsed = bMenuUsed;
}

std::wstring Control::GetDataID() const
{
    return m_sUserDataID.c_str();
}

std::string Control::GetUTF8DataID() const
{
	std::string strOut;
	StringHelper::UnicodeToMBCS(GetDataID(), strOut, CP_UTF8);
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

void Control::SetUserDataID(size_t dataID)
{
	m_uUserDataID = dataID;
}

size_t Control::GetUserDataID() const
{
	return m_uUserDataID;
}

void Control::SetFadeVisible(bool bVisible)
{
	if (bVisible) {
		GetAnimationManager().Appear();
	}
	else {
		GetAnimationManager().Disappear();
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
		CheckStopGifPlay();
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
		PrivateSetState(kControlStateNormal);
		m_nHotAlpha = 0;
	}
	else {
		PrivateSetState(kControlStateDisabled);
	}

	if (!IsEnabled()) {
		CheckStopGifPlay();
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

uint32_t Control::GetControlFlags() const
{
	return IsAllowTabStop() ? UIFLAG_TABSTOP : UIFLAG_DEFAULT;
}

void Control::SetNoFocus()
{
    m_bNoFocus = true;
	EnsureNoFocus();
}

void Control::SetShowFocusRect(bool bShowFocusRect)
{
	m_bShowFocusRect = bShowFocusRect;
}

bool Control::IsShowFocusRect() const
{
	return m_bShowFocusRect;
}

void Control::SetFocusRectColor(const std::wstring& focusRectColor)
{
	m_focusRectColor = focusRectColor;
}

std::wstring Control::GetFocusRectColor() const
{
	return m_focusRectColor.c_str();
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

Control* Control::FindControl(FINDCONTROLPROC Proc, LPVOID pProcData,
							  uint32_t uFlags, const UiPoint& ptMouse,
							  const UiPoint& scrollPos)
{
	if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
		return nullptr;
	}
	if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
		return nullptr;
	}
	UiPoint pt(ptMouse);
	pt.Offset(scrollPos);
#ifdef _DEBUG
	if (((uFlags & UIFIND_HITTEST) != 0) && ((uFlags & UIFIND_DRAG_DROP) == 0) && (pProcData != nullptr)) {
		UiPoint ptOrg(*(UiPoint*)pProcData);
		ptOrg.Offset(this->GetScrollOffsetInScrollBox());
		ASSERT(ptOrg == pt);
	}
#endif // _DEBUG
	if ((uFlags & UIFIND_HITTEST) != 0 && 
		(!m_bMouseEnabled || !GetRect().ContainsPt(pt))) {
		return nullptr;
	}
    return Proc(this, pProcData);
}

UiRect Control::GetPos() const
{
	return __super::GetPos();
}

void Control::SetPos(UiRect rc)
{
	//有很多类似的代码：SetPos(GetPos()), 代表设置位置，并重新绘制
	rc.Validate();
	SetArranged(false);
	bool isPosChanged = !GetRect().Equals(rc);

	UiRect invalidateRc = GetRect();
	if (invalidateRc.IsEmpty()) {
		invalidateRc = rc;
	}

	SetRect(rc);
	if (GetWindow() == nullptr) {
		return;
	}
	invalidateRc.Union(GetRect());
	bool needInvalidate = true;
	UiRect rcTemp;
	UiRect rcParent;
	UiPoint offset = GetScrollOffsetInScrollBox();
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

	if (isPosChanged) {
		SendEvent(kEventResize);
	}	
}

UiEstSize Control::EstimateSize(UiSize szAvailable)
{
	UiFixedSize fixedSize = GetFixedSize();
	if (!fixedSize.cx.IsAuto() && !fixedSize.cy.IsAuto()) {
		//如果宽高都不是auto属性，则直接返回
		return MakeEstSize(fixedSize);
	}
	szAvailable.Validate();
	if (!IsReEstimateSize(szAvailable)) {
		//使用缓存中的估算结果
		return GetEstimateSize();
	}

	//估算图片区域大小
	UiSize imageSize;
	std::shared_ptr<ImageInfo> imageCache;
	Image* image = GetEstimateImage();
	if (image != nullptr) {
		//加载图片：需要获取图片的宽和高
		LoadImageData(*image);
		imageCache = image->GetImageCache();		
	}
	if (imageCache != nullptr) {
		ImageAttribute imageAttribute = image->GetImageAttribute();
		UiRect rcDest;
		bool hasDestAttr = false;
		if (ImageAttribute::HasValidImageRect(imageAttribute.GetDestRect())) {
			//使用配置中指定的目标区域
			rcDest = imageAttribute.GetDestRect();
			hasDestAttr = true;
		}
		UiRect rcDestCorners;
		UiRect rcSource = imageAttribute.GetSourceRect();
		UiRect rcSourceCorners = imageAttribute.GetCorner();
		ImageAttribute::ScaleImageRect(imageCache->GetWidth(), imageCache->GetHeight(),
									   imageCache->IsBitmapSizeDpiScaled(),
									   rcDestCorners,
									   rcSource,
									   rcSourceCorners);
		if (rcDest.Width() > 0) {
			imageSize.cx = rcDest.Width();
		}
		else if (rcSource.Width() > 0) {
			imageSize.cx = rcSource.Width();
		}
		else {
			imageSize.cx = imageCache->GetWidth();
		}

		if (rcDest.Height() > 0) {
			imageSize.cy = rcDest.Height();
		}
		else if (rcSource.Height() > 0) {
			imageSize.cy = rcSource.Height();
		}
		else {
			imageSize.cy = imageCache->GetHeight();
		}
		if (!hasDestAttr) {
			//如果没有rcDest属性，则需要增加图片的内边距
			UiPadding rcPadding = imageAttribute.GetPadding();
			imageSize.cx += (rcPadding.left + rcPadding.right);
			imageSize.cy += (rcPadding.top + rcPadding.bottom);
		}
	}
	imageCache.reset();

	//图片大小，需要附加控件的内边距
	UiPadding rcPadding = this->GetControlPadding();
	if (imageSize.cx > 0) {
		imageSize.cx += (rcPadding.left + rcPadding.right);
	}
	if (imageSize.cy > 0) {
		imageSize.cy += (rcPadding.top + rcPadding.bottom);
	}

	//估算文本区域大小, 函数计算时，已经包含了内边距
	UiSize textSize = EstimateText(szAvailable);

	//选取图片和文本区域高度和宽度的最大值
	if (fixedSize.cx.IsAuto()) {
		fixedSize.cx.SetInt32(std::max(imageSize.cx, textSize.cx));
	}
	if (fixedSize.cy.IsAuto()) {
		fixedSize.cy.SetInt32(std::max(imageSize.cy, textSize.cy));
	}
	//保持结果到缓存，避免每次都重新估算
	UiEstSize estSize = MakeEstSize(fixedSize);
	SetEstimateSize(estSize, szAvailable);
	SetReEstimateSize(false);
	return estSize;
}

Image* Control::GetEstimateImage()
{
	Image* estimateImage = nullptr;
	if ((m_pBkImage != nullptr) && !m_pBkImage->GetImagePath().empty()) {
		estimateImage = m_pBkImage.get();
	}
	else if(m_pImageMap != nullptr){
		estimateImage = m_pImageMap->GetEstimateImage(kStateImageBk);
		if (estimateImage == nullptr) {
			estimateImage = m_pImageMap->GetEstimateImage(kStateImageSelectedBk);
		}
	}
	return estimateImage;
}

UiSize Control::EstimateText(UiSize /*szAvailable*/)
{
	return UiSize(0, 0);
}

bool Control::IsPointInWithScrollOffset(const UiPoint& point) const
{
	UiPoint scrollOffset = GetScrollOffsetInScrollBox();
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
//#ifdef _DEBUG
//	std::wstring eventType = EventTypeToString(msg.Type);
//	std::wstring type = GetType();
//	wchar_t buf[256] = {};
//	swprintf_s(buf, L"Control::SendEvent: type=%s, eventType=%s\r\n", type.c_str(), eventType.c_str());
//	::OutputDebugStringW(buf);	
//#endif
	bool bRet = FireAllEvents(msg);	
    if(bRet) {
		HandleEvent(msg);
	}
}

bool Control::IsDisabledEvents(const EventArgs& msg) const
{
	if ((msg.Type > kEventMouseBegin) && (msg.Type < kEventMouseEnd)) {
		//当前控件禁止接收鼠标消息时，将鼠标相关消息转发给上层处理
		if (!IsEnabled() || !IsMouseEnabled()) {
			return true;
		}
	}
	else if ((msg.Type > kEventKeyBegin) && (msg.Type < kEventKeyEnd)) {
		//当前控件禁止接收键盘消息时，将键盘相关消息转发给上层处理
		if (!IsEnabled() || !IsKeyboardEnabled()) {
			return true;
		}
	}
	else if (msg.Type == kEventLast) {
		//转发给上层控件
		return true;
	}
	return false;
}

void Control::HandleEvent(const EventArgs& msg)
{
	if (IsDisabledEvents(msg)) {
		//如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
		Box* pParent = GetParent();
		if (pParent != nullptr) {
			pParent->SendEvent(msg);
		}
		return;
	}
	if( msg.Type == kEventSetCursor ) {
		if (OnSetCursor(msg)) {
			return;
		}		
	}
	else if (msg.Type == kEventSetFocus) {
		if (OnSetFocus(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventKillFocus) {
		if (OnKillFocus(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventWindowKillFocus) {
		if (OnWindowKillFocus(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventImeStartComposition) {
		if (OnImeStartComposition(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventImeEndComposition) {
		if (OnImeEndComposition(msg)) {
			return;
		}
	}
	else if( msg.Type == kEventMouseEnter ) {
		if (GetWindow()) {
			if (!IsChild(this, GetWindow()->GetHoverControl())) {
				return;
			}
		}
		if (MouseEnter(msg)) {
			return;
		}
	}
	else if( msg.Type == kEventMouseLeave ) {
		if (GetWindow()) {
			if (IsChild(this, GetWindow()->GetHoverControl())) {
				return;
			}
		}
		if (MouseLeave(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventMouseButtonDown) {
		if (ButtonDown(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventMouseButtonUp) {
		if (ButtonUp(msg)) {
			return;
		}		
	}
	else if (msg.Type == kEventMouseDoubleClick) {
		if (ButtonDoubleClick(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventMouseRButtonDown) {
		if (RButtonDown(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventMouseRButtonUp) {
		if (RButtonUp(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventMouseRDoubleClick) {
		if (RButtonDoubleClick(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventMouseMove) {
		if (MouseMove(msg)) {
			return;
		}		
	}
	else if (msg.Type == kEventMouseHover) {
		if (MouseHover(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventMouseWheel) {
		if (MouseWheel(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventMouseMenu) {
		if (MouseMenu(msg)) {
			return;
		}		
	}
	else if (msg.Type == kEventChar) {
		if (OnChar(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventKeyDown) {
		if (OnKeyDown(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventKeyUp) {
		if (OnKeyUp(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventSysKeyDown) {
		if (OnSysKeyDown(msg)) {
			return;
		}
	}
	else if (msg.Type == kEventSysKeyUp) {
		if (OnSysKeyUp(msg)) {
			return;
		}
	}

	if (GetParent() != nullptr) {
		GetParent()->SendEvent(msg);
	}
}

bool Control::HasHotState()
{
	bool bState = false;
	if (m_pColorMap != nullptr) {
		bState = m_pColorMap->HasHotColor();
	}
	if (!bState && (m_pImageMap != nullptr)) {
		bState = m_pImageMap->HasHotImage();
	}
	if (!bState && (m_pBorderColorMap != nullptr)) {
		bState = m_pBorderColorMap->HasHotColor();
	}
	return bState;
}

bool Control::MouseEnter(const EventArgs& /*msg*/)
{
	if( IsEnabled() ) {
		if (GetState() == kControlStateNormal) {
			PrivateSetState(kControlStateHot);
			if (HasHotState()) {
				GetAnimationManager().MouseEnter();
				Invalidate();
			}
			return false;
		}
		else {
			return true;
		}
	}
	return false;
}

bool Control::MouseLeave(const EventArgs& /*msg*/)
{
	if( IsEnabled() ) {
		if (GetState() == kControlStateHot) {
			PrivateSetState(kControlStateNormal);
			if (HasHotState()) {
				GetAnimationManager().MouseLeave();
				Invalidate();
			}
			return false;
		}
		else {
			return true;
		}
	}

	return false;
}

bool Control::ButtonDown(const EventArgs& /*msg*/)
{
	if( IsEnabled() ) {
		PrivateSetState(kControlStatePushed);
		SetMouseFocused(true);
		Invalidate();
	}
	return true;
}

bool Control::ButtonUp(const EventArgs& msg)
{
	if( IsMouseFocused() ) {
		SetMouseFocused(false);
		auto player = GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot);
		if (player != nullptr) {
			player->Stop();
		}
		Invalidate();
		if( IsPointInWithScrollOffset(msg.ptMouse) ) {
			PrivateSetState(kControlStateHot);
			m_nHotAlpha = 255;
			Activate();
		}
		else {
			PrivateSetState(kControlStateNormal);
			m_nHotAlpha = 0;
		}
	}
	return true;
}

bool Control::ButtonDoubleClick(const EventArgs& /*msg*/)
{
	return true;
}

bool Control::RButtonDown(const EventArgs& /*msg*/)
{
	if (IsEnabled()) {
		SetMouseFocused(true);
	}
	return true;
}

bool Control::RButtonUp(const EventArgs& msg)
{
	if (IsMouseFocused()) {
		SetMouseFocused(false);
		if (IsPointInWithScrollOffset(msg.ptMouse)) {
			SendEvent(kEventRClick);
		}
	}
	return true;
}

bool Control::RButtonDoubleClick(const EventArgs& /*msg*/)
{
	return true;
}

bool Control::MouseMove(const EventArgs& /*msg*/)
{
	return true;
}

bool Control::MouseHover(const EventArgs& /*msg*/)
{
	return true;
}

bool Control::MouseWheel(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	//int deltaValue = GET_WHEEL_DELTA_WPARAM(msg.wParam);
	return false;
}

bool Control::MouseMenu(const EventArgs& /*msg*/)
{
	//按Shif + F10由系统产生上下文菜单, 或者点击右键触发菜单：默认不处理，交由父控件处理
	return false;
}

bool Control::OnChar(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	return false;
}

bool Control::OnKeyDown(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	return false;
}

bool Control::OnKeyUp(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	return false;
}

bool Control::OnSysKeyDown(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	return false;
}

bool Control::OnSysKeyUp(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	return false;
}

bool Control::OnSetCursor(const EventArgs& /*msg*/)
{
	if (m_cursorType == kCursorHand) {
		if (IsEnabled()) {
			SetCursor(kCursorHand);
		}
		else {
			SetCursor(kCursorArrow);
		}
	}
	else if (m_cursorType == kCursorArrow) {
		SetCursor(kCursorArrow);
	}
	else if (m_cursorType == kCursorHandIbeam) {
		SetCursor(kCursorHandIbeam);
	}
	else if (m_cursorType == kCursorSizeWE) {
		SetCursor(kCursorSizeWE);
	}
	else if (m_cursorType == kCursorSizeNS) {
		SetCursor(kCursorSizeNS);
	}
	else {
		return false;
	}
	return true;
}

void Control::SetCursor(CursorType cursorType)
{
	switch (cursorType) {
	case kCursorArrow:
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		break;
	case kCursorHand:
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
		break;
	case kCursorHandIbeam:
		::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
		break;
	case kCursorSizeWE:
		::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
		break;
	case kCursorSizeNS:
		::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
		break;
	default:
		break;
	}
}

bool Control::OnSetFocus(const EventArgs& /*msg*/)
{
	if (GetState() == kControlStateNormal) {
		SetState(kControlStateHot);
		Invalidate();
	}
	return true;
}

bool Control::OnKillFocus(const EventArgs& /*msg*/)
{
	if (GetState() == kControlStateHot) {
		SetState(kControlStateNormal);
	}
	else if (GetState() == kControlStatePushed) {
		//失去焦点时，修复控件状态（如果鼠标按下时，窗口失去焦点，鼠标弹起事件这个控件就收不到了）
		SetMouseFocused(false);
		auto player = GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot);
		if (player != nullptr) {
			player->Stop();
		}
		SetState(kControlStateNormal);
	}
	Invalidate();
	return true;
}

bool Control::OnWindowKillFocus(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	return false;
}

bool Control::OnImeStartComposition(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	return false;
}

bool Control::OnImeEndComposition(const EventArgs& /*msg*/)
{
	//默认不处理，交由父控件处理
	return false;
}

bool Control::PaintImage(IRender* pRender, Image* pImage,
					    const std::wstring& strModify, int32_t nFade, 
	                    IMatrix* pMatrix, UiRect* pInRect, UiRect* pPaintedRect) const
{
	//注解：strModify参数，目前外部传入的主要是："destscale='false' dest='%d,%d,%d,%d'"
	//                   也有一个类传入了：L" corner='%d,%d,%d,%d'"。
	if (pImage == nullptr) {
		//这里可能为空，不需要加断言，为空直接返回
		return false;
	}
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return false;
	}

	Image& duiImage = *pImage;

	if (duiImage.GetImagePath().empty()) {
		return false;
	}

	if (!duiImage.GetImageAttribute().bPaintEnabled) {
		//该图片禁止绘制，返回
		return false;
	}

	LoadImageData(duiImage);
	std::shared_ptr<ImageInfo> imageInfo = duiImage.GetImageCache();
	ASSERT(imageInfo != nullptr);
	if (imageInfo == nullptr) {
		return false;
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
	bool hasDestAttr = false; // 外部是否设置了rcDest属性
	UiRect rcDest = GetRect();
	rcDest.Deflate(GetControlPadding());//去掉内边距
	if (pInRect != nullptr) {
		//使用外部传入的矩形区域绘制图片
		rcDest = *pInRect;
	}
	if (ImageAttribute::HasValidImageRect(newImageAttribute.GetDestRect())) {
		//使用配置中指定的目标区域
		if ((newImageAttribute.GetDestRect().Width() <= rcDest.Width()) &&
			(newImageAttribute.GetDestRect().Height() <= rcDest.Height())) {
			rcDest = newImageAttribute.GetDestRect();
			rcDest.Offset(GetRect().left, GetRect().top);
			hasDestAttr = true;
		}
	}

	UiRect rcDestCorners;
	UiRect rcSource = newImageAttribute.GetSourceRect();
	UiRect rcSourceCorners = newImageAttribute.GetCorner();
	ImageAttribute::ScaleImageRect(pBitmap->GetWidth(), pBitmap->GetHeight(), 
								   imageInfo->IsBitmapSizeDpiScaled(),
		                           rcDestCorners,
		                           rcSource,
		                           rcSourceCorners);
	
	if (!hasDestAttr) {
		//运用rcPadding、hAlign、vAlign 三个图片属性
		rcDest.Deflate(newImageAttribute.GetPadding());
		rcDest.Validate();
		rcSource.Validate();
		const int32_t imageWidth = rcSource.Width();
		const int32_t imageHeight = rcSource.Height();

		//应用对齐方式后，图片将不再拉伸，而是按原大小展示
		if (!newImageAttribute.hAlign.empty()) {
			if (newImageAttribute.hAlign == L"left") {
				rcDest.right = rcDest.left + imageWidth;
			}
			else if (newImageAttribute.hAlign == L"center") {
				rcDest.left = rcDest.CenterX() - imageWidth / 2;
				rcDest.right = rcDest.left + imageWidth;
			}
			else if (newImageAttribute.hAlign == L"right") {
				rcDest.left = rcDest.right - imageWidth;
			}
			else {
				rcDest.right = rcDest.left + imageWidth;
			}

			if (newImageAttribute.vAlign.empty()) {
				rcDest.bottom = rcDest.top + imageHeight;
			}				
		}
		if (!newImageAttribute.vAlign.empty()) {
			if (newImageAttribute.vAlign == L"top") {
				rcDest.bottom = rcDest.top + imageHeight;
			}
			else if (newImageAttribute.vAlign == L"center") {
				rcDest.top = rcDest.CenterY() - imageHeight / 2;
				rcDest.bottom = rcDest.top + imageHeight;
			}
			else if (newImageAttribute.vAlign == L"right") {
				rcDest.top = rcDest.bottom - imageHeight;
			}
			else {
				rcDest.bottom = rcDest.top + imageHeight;
			}

			if (newImageAttribute.hAlign.empty()) {
				rcDest.right = rcDest.left + imageWidth;
			}
		}
	}

	if (pPaintedRect) {
		//返回绘制的目标区域
		*pPaintedRect = rcDest;
	}

	//图片透明度属性
	uint8_t iFade = (nFade == DUI_NOSET_VALUE) ? newImageAttribute.bFade : static_cast<uint8_t>(nFade);
	if (pMatrix != nullptr) {
		//矩阵绘制: 对不支持的属性，增加断言，避免出错
		ASSERT(newImageAttribute.GetCorner().IsEmpty());
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
	//按需启动动画
	duiImage.CheckStartGifPlay(rcDest);
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
	const bool isUseCache = IsUseCache() && !HasBoxShadow();

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

			bool hasBoxShadowPainted = HasBoxShadow();
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
		bool hasBoxShadowPainted = HasBoxShadow();
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
	PaintBkColor(pRender);		//背景颜色(覆盖整个矩形)
	PaintStateColors(pRender);	//控件指定状态的颜色：普通状态、焦点状态、按下状态、禁用状态(覆盖整个矩形)
	PaintBkImage(pRender);		//背景图片，无状态
	PaintStateImages(pRender);	//先绘制背景图片，然后绘制前景图片，每个图片有指定的状态：普通状态、焦点状态、按下状态、禁用状态
	PaintText(pRender);			//绘制文本
	PaintBorder(pRender);		//绘制边框
	PaintFocusRect(pRender);	//绘制焦点状态
    PaintLoading(pRender);		//绘制Loading图片，无状态
}

void Control::PaintShadow(IRender* pRender)
{
	if (!HasBoxShadow()) {
		return;
	}
	BoxShadow boxShadow;
	if (m_pBoxShadow != nullptr) {
		boxShadow = *m_pBoxShadow;
	}

	ASSERT(pRender != nullptr);
	if (pRender != nullptr) {
		pRender->DrawBoxShadow(m_rcPaint,
							   m_cxyBorderRound,
							   boxShadow.m_cpOffset,
							   boxShadow.m_nBlurRadius,
							   boxShadow.m_nSpreadRadius,
							   GlobalManager::Instance().Color().GetColor(boxShadow.m_strColor));
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

	UiColor dwBackColor = GetUiColor(m_strBkColor.c_str());
	if(dwBackColor.GetARGB() != 0) {
		int32_t nBorderSize = 0;
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
	UiColor dwBorderColor;
	std::wstring borderColor;
	if (IsFocused()) {
		if (IsHotState()) {
			borderColor = GetBorderColor(GetState());
		}
		if (borderColor.empty()) {
			borderColor = GetFocusBorderColor();
		}
	}
	if (borderColor.empty()) {
		borderColor = GetBorderColor(GetState());
	}
	if (!borderColor.empty()) {
		dwBorderColor = GetUiColor(borderColor.c_str());
	}
	if (dwBorderColor.GetARGB() == 0) {
		return;
	}
	bool bPainted = false;
	if ((m_rcBorderSize.left > 0) &&
		(m_rcBorderSize.left == m_rcBorderSize.right) &&
		(m_rcBorderSize.left == m_rcBorderSize.top) &&
		(m_rcBorderSize.left == m_rcBorderSize.bottom)) {
		//四个边都存在，且大小相同，则直接绘制矩形, 支持圆角矩形
		if (ShouldBeRoundRectBorders()) {
			//仅圆角矩形，使用这个函数绘制边线
			PaintBorders(pRender, GetRect(), (int32_t)m_rcBorderSize.left, dwBorderColor);
			bPainted = true;
		}
	}

	if(!bPainted) {
		//非圆角矩形，四个边分别按照设置绘制边线
		if (m_rcBorderSize.left > 0) {
			//左边线
			UiRect rcBorder = GetRect();
			if (m_rcBorderSize.left == 1) {
				rcBorder.bottom -= 1;
			}
			float fWidth = (float)m_rcBorderSize.left;
			UiPointF pt1((float)rcBorder.left + fWidth / 2, (float)rcBorder.top);
			UiPointF pt2((float)rcBorder.left + fWidth / 2, (float)rcBorder.bottom);
			pRender->DrawLine(pt1, pt2, dwBorderColor, fWidth);
		}
		if (m_rcBorderSize.top > 0) {
			//上边线
			UiRect rcBorder = GetRect();
			if (m_rcBorderSize.top == 1) {
				rcBorder.right -= 1;
			}
			float fWidth = (float)m_rcBorderSize.top;
			UiPointF pt1((float)rcBorder.left, (float)rcBorder.top + fWidth / 2);
			UiPointF pt2((float)rcBorder.right, (float)rcBorder.top + fWidth / 2);
			pRender->DrawLine(pt1, pt2, dwBorderColor, fWidth);
		}
		if (m_rcBorderSize.right > 0) {
			//右边线
			UiRect rcBorder = GetRect();
			if (m_rcBorderSize.right == 1) {
				rcBorder.bottom -= 1;
			}
			float fWidth = (float)m_rcBorderSize.right;
			UiPointF pt1((float)rcBorder.right - fWidth / 2, (float)rcBorder.top);
			UiPointF pt2((float)rcBorder.right - fWidth / 2, (float)rcBorder.bottom);
			pRender->DrawLine(pt1, pt2, dwBorderColor, fWidth);
		}
		if (m_rcBorderSize.bottom > 0) {
			//下边线
			UiRect rcBorder = GetRect();
			if (m_rcBorderSize.bottom == 1) {
				rcBorder.right -= 1;
			}
			float fWidth = (float)m_rcBorderSize.bottom;
			UiPointF pt1((float)rcBorder.left, (float)rcBorder.bottom - fWidth / 2);
			UiPointF pt2((float)rcBorder.right, (float)rcBorder.bottom - fWidth / 2);			
			pRender->DrawLine(pt1, pt2, dwBorderColor, fWidth);
		}
	}
}

void Control::PaintBorders(IRender* pRender, UiRect rcDraw,
	                       int32_t nBorderSize, UiColor dwBorderColor) const
{
	if ((pRender == nullptr) || rcDraw.IsEmpty() || (nBorderSize < 1) || (dwBorderColor.GetARGB() == 0)) {
		return;
	}
	int32_t nDeltaValue = nBorderSize / 2;
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
		pRender->DrawRect(rcDraw, dwBorderColor, nBorderSize, false);
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

void Control::PaintFocusRect(IRender* pRender)
{
	if ((pRender != nullptr) && IsShowFocusRect() && IsFocused()) {
		DoPaintFocusRect(pRender);	//绘制焦点状态
	}
}

void Control::DoPaintFocusRect(IRender* pRender)
{
	IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
	if (pRenderFactory == nullptr) {
		return;
	}
	int32_t nWidth = ui::GlobalManager::Instance().Dpi().GetScaleInt(1); //画笔宽度
	UiColor dwBorderColor;//画笔颜色
	std::wstring focusRectColor = GetFocusRectColor();
	if (!focusRectColor.empty()) {
		dwBorderColor = GetUiColor(focusRectColor);
	}
	if(dwBorderColor.IsEmpty()) {
		dwBorderColor = UiColor(UiColors::Gray);
	}
	UiRect rcBorderSize(1, 1, 1, 1);
	UiRect rcFocusRect = GetRect();
	int32_t nFocusWidth = ui::GlobalManager::Instance().Dpi().GetScaleInt(2); //矩形间隙
	rcFocusRect.Deflate(nFocusWidth, nFocusWidth);
	if (rcFocusRect.IsEmpty()) {
		return;
	}
	std::unique_ptr<IPen> pPen(pRenderFactory->CreatePen(dwBorderColor, nWidth));
	pPen->SetDashStyle(IPen::DashStyle::kDashStyleDot);

	if (rcBorderSize.left > 0) {
		//左边线
		UiPoint pt1(rcFocusRect.left, rcFocusRect.top);
		UiPoint pt2(rcFocusRect.left, rcFocusRect.bottom);
		pRender->DrawLine(pt1, pt2, pPen.get());
	}
	if (rcBorderSize.top > 0) {
		//上边线
		UiPoint pt1(rcFocusRect.left, rcFocusRect.top);
		UiPoint pt2(rcFocusRect.right, rcFocusRect.top);
		pRender->DrawLine(pt1, pt2, pPen.get());
	}
	if (rcBorderSize.right > 0) {
		//右边线
		UiPoint pt1(rcFocusRect.right, rcFocusRect.top);
		UiPoint pt2(rcFocusRect.right, rcFocusRect.bottom);
		pRender->DrawLine(pt1, pt2, pPen.get());
	}
	if (rcBorderSize.bottom > 0) {
		//下边线
		UiPoint pt1(rcFocusRect.left, rcFocusRect.bottom);
		UiPoint pt2(rcFocusRect.right, rcFocusRect.bottom);
		pRender->DrawLine(pt1, pt2, pPen.get());
	}
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

void Control::AddRoundRectPath(IPath* path, const UiRect& rc, UiSize roundSize) const
{
	ASSERT(path != nullptr);
	if (path == nullptr) {
		return;
	}
	//确保圆角宽度和高度都是偶数
	if ((roundSize.cx % 2) != 0) {
		roundSize.cx += 1;
	}
	if ((roundSize.cy % 2) != 0) {
		roundSize.cy += 1;
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

void Control::DrawRoundRect(IRender* pRender, const UiRect& rc, const UiSize& roundSize, UiColor dwBorderColor, int32_t nBorderSize) const
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
	if (m_pBkImage != nullptr) {
		PaintImage(pRender, m_pBkImage.get());
	}	
}

void Control::PaintStateColor(IRender* pRender, const UiRect& rcPaint, ControlStateType stateType) const
{
	if (m_pColorMap != nullptr) {
		m_pColorMap->PaintStateColor(pRender, rcPaint, stateType);
	}
}

void Control::PaintStateColors(IRender* pRender)
{
	if (m_pColorMap != nullptr) {
		m_pColorMap->PaintStateColor(pRender, m_rcPaint, GetState());
	}	
}

void Control::PaintStateImages(IRender* pRender)
{
	if (m_pImageMap != nullptr) {
		m_pImageMap->PaintStateImage(pRender, kStateImageBk, GetState());
		m_pImageMap->PaintStateImage(pRender, kStateImageFore, GetState());
	}	
}

void Control::PaintText(IRender* /*pRender*/)
{
    return;
}

void Control::PaintLoading(IRender* pRender)
{
	if (m_pLoading != nullptr) {
		m_pLoading->PaintLoading(pRender);
	}
}

void Control::SetAlpha(int64_t alpha)
{
	ASSERT(alpha >= 0 && alpha <= 255);
	if (m_nAlpha != (uint8_t)alpha) {
		m_nAlpha = (uint8_t)alpha;
		Invalidate();
	}
}

void Control::SetHotAlpha(int64_t nHotAlpha)
{
	ASSERT(nHotAlpha >= 0 && nHotAlpha <= 255);
	if (m_nHotAlpha != (uint8_t)nHotAlpha) {
		m_nHotAlpha = (uint8_t)nHotAlpha;
		Invalidate();
	}
}

void Control::SetTabStop(bool enable)
{
	m_bAllowTabstop = enable;
}

void Control::SetRenderOffset(UiPoint renderOffset)
{
	if (m_renderOffset != renderOffset) {
		m_renderOffset = renderOffset;
		Invalidate();
	}	
}

void Control::SetRenderOffsetX(int64_t renderOffsetX)
{
	int32_t x = TruncateToInt32(renderOffsetX);
	if (m_renderOffset.x != x) {
		m_renderOffset.x = x;
		Invalidate();
	}
}

void Control::SetRenderOffsetY(int64_t renderOffsetY)
{
	int32_t y = TruncateToInt32(renderOffsetY);
	if (m_renderOffset.y != y) {
		m_renderOffset.y = y;
		Invalidate();
	}
}

void Control::CheckStopGifPlay()
{
	if (m_pBkImage != nullptr) {
		m_pBkImage->CheckStopGifPlay();
	}
	if (m_pImageMap != nullptr) {
		m_pImageMap->StopGifPlay();
	}
}

bool Control::StartGifPlay(GifFrameType nStartFrame, int32_t nPlayCount)
{
	if (m_pBkImage == nullptr) {
		return false;
	}
	if (!LoadImageData(*m_pBkImage)) {
		return false;
	}
	return m_pBkImage->StartGifPlay(nStartFrame, nPlayCount);
}

void Control::StopGifPlay(bool bTriggerEvent, GifFrameType stopType)
{
	if (m_pBkImage != nullptr) {
		m_pBkImage->StopGifPlay(bTriggerEvent, stopType);
	}
}

void Control::AttachGifPlayStop(const EventCallback& callback)
{
	if (m_pBkImage == nullptr) {
		m_pBkImage = std::make_shared<Image>();
		m_pBkImage->SetControl(this);
	}
	m_pBkImage->AttachGifPlayStop(callback);
}

bool Control::LoadImageData(Image& duiImage) const
{
	if (duiImage.GetImageCache() != nullptr) {
		//如果图片缓存存在，则不再加载（当图片变化的时候，会清空这个缓存）
		return true;
	}
	Window* pWindow = GetWindow();
	ASSERT(pWindow != nullptr);
	if (pWindow == nullptr) {
		return false;
	}

	std::wstring sImagePath = duiImage.GetImagePath();
	if (sImagePath.empty()) {
		return false;
	}
	std::wstring imageFullPath;

#ifdef UILIB_IMPL_WINSDK
	if (GlobalManager::Instance().Icon().IsIconString(sImagePath)) {
		//ICON句柄
		imageFullPath = sImagePath;
	}
#endif

	if(imageFullPath.empty()) {
		imageFullPath = GlobalManager::Instance().GetResFullPath(pWindow->GetResourcePath(), sImagePath);
	}
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

void Control::ClearImageCache()
{
	if (m_pImageMap != nullptr) {
		m_pImageMap->ClearImageCache();
	}
	if (m_pBkImage != nullptr) {
		m_pBkImage->ClearImageCache();
	}
}

void Control::AttachEvent(EventType type, const EventCallback& callback)
{
	if (m_pOnEvent == nullptr) {
		m_pOnEvent = new EventMap;
	}
	(*m_pOnEvent)[type] += callback;
	if ((type == kEventMouseMenu) || (type == kEventAll)) {
		SetContextMenuUsed(true);
	}
}

void Control::DetachEvent(EventType type)
{
	if (m_pOnEvent == nullptr) {
		return;
	}
	auto event = m_pOnEvent->find(type);
	if (event != m_pOnEvent->end()) {
		m_pOnEvent->erase(event);
	}
	if ((type == kEventMouseMenu) || (type == kEventAll)) {
		if ((m_pOnEvent->find(kEventAll) == m_pOnEvent->end()) &&
			(m_pOnEvent->find(kEventMouseMenu) == m_pOnEvent->end())) {
			SetContextMenuUsed(false);
		}
	}
}

void Control::AttachXmlEvent(EventType eventType, const EventCallback& callback)
{
	if (m_pOnXmlEvent == nullptr) {
		m_pOnXmlEvent = new EventMap;
	}
	(*m_pOnXmlEvent)[eventType] += callback; 
}

void Control::DetachXmlEvent(EventType type)
{
	if (m_pOnXmlEvent == nullptr) {
		return;
	}
	auto event = m_pOnXmlEvent->find(type);
	if (event != m_pOnXmlEvent->end()) {
		m_pOnXmlEvent->erase(event);
	}
}

void Control::AttachBubbledEvent(EventType eventType, const EventCallback& callback)
{
	if (m_pOnBubbledEvent == nullptr) {
		m_pOnBubbledEvent = new EventMap;
	}
	(*m_pOnBubbledEvent)[eventType] += callback;
}

void Control::DetachBubbledEvent(EventType eventType)
{
	if (m_pOnBubbledEvent == nullptr) {
		return;
	}
	auto event = m_pOnBubbledEvent->find(eventType);
	if (event != m_pOnBubbledEvent->end()) {
		m_pOnBubbledEvent->erase(eventType);
	}
}

void Control::AttachXmlBubbledEvent(EventType eventType, const EventCallback& callback)
{
	if (m_pOnXmlBubbledEvent == nullptr) {
		m_pOnXmlBubbledEvent = new EventMap;
	}
	(*m_pOnXmlBubbledEvent)[eventType] += callback;
}

void Control::DetachXmlBubbledEvent(EventType eventType)
{
	if (m_pOnXmlBubbledEvent == nullptr) {
		return;
	}
	auto event = m_pOnXmlBubbledEvent->find(eventType);
	if (event != m_pOnXmlBubbledEvent->end())	{
		m_pOnXmlBubbledEvent->erase(eventType);
	}
}

bool Control::FireAllEvents(const EventArgs& msg)
{
	std::weak_ptr<nbase::WeakFlag> weakflag = GetWeakFlag();
	bool bRet = true;//当值为false时，就不再调用回调函数和处理函数

	if (msg.pSender == this) {
		if (bRet && (m_pOnEvent != nullptr) && !m_pOnEvent->empty()) {
			auto callback = m_pOnEvent->find(msg.Type);
			if (callback != m_pOnEvent->end()) {
				bRet = callback->second(msg);
			}
			if (weakflag.expired()) {
				return false;
			}

			callback = m_pOnEvent->find(kEventAll);
			if (callback != m_pOnEvent->end()) {
				bRet = callback->second(msg);
			}
			if (weakflag.expired()) {
				return false;
			}
		}

		if (bRet && (m_pOnXmlEvent != nullptr) && !m_pOnXmlEvent->empty()) {
			auto callback = m_pOnXmlEvent->find(msg.Type);
			if (callback != m_pOnXmlEvent->end()) {
				bRet = callback->second(msg);
			}
			if (weakflag.expired()) {
				return false;
			}

			callback = m_pOnXmlEvent->find(kEventAll);
			if (callback != m_pOnXmlEvent->end()) {
				bRet = callback->second(msg);
			}
			if (weakflag.expired()) {
				return false;
			}
		}
	}

	if (bRet && (m_pOnBubbledEvent != nullptr) && !m_pOnBubbledEvent->empty()) {
		auto callback = m_pOnBubbledEvent->find(msg.Type);
		if (callback != m_pOnBubbledEvent->end()) {
			bRet = callback->second(msg);
		}
		if (weakflag.expired()) {
			return false;
		}

		callback = m_pOnBubbledEvent->find(kEventAll);
		if (callback != m_pOnBubbledEvent->end()) {
			bRet = callback->second(msg);
		}
		if (weakflag.expired()) {
			return false;
		}
	}

	if (bRet && (m_pOnXmlBubbledEvent != nullptr) && !m_pOnXmlBubbledEvent->empty()) {
		auto callback = m_pOnXmlBubbledEvent->find(msg.Type);
		if (callback != m_pOnXmlBubbledEvent->end()) {
			bRet = callback->second(msg);
		}
		if (weakflag.expired()) {
			return false;
		}

		callback = m_pOnXmlBubbledEvent->find(kEventAll);
		if (callback != m_pOnXmlBubbledEvent->end()) {
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
	if (colorName.empty()) {
		return false;
	}
	UiColor color = GetUiColorByName(colorName);
	return color.GetARGB() != 0;
}

UiColor Control::GetUiColor(const std::wstring& colorName) const
{
	if (colorName.empty()) {
		return UiColor();
	}
	UiColor color = GetUiColorByName(colorName);
	ASSERT(!color.IsEmpty());
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
		color = GlobalManager::Instance().Color().GetStandardColor(colorName);
	}
	ASSERT(color.GetARGB() != 0);
	return color;
}

std::wstring Control::GetColorString(const UiColor& color) const
{
	if (color.IsEmpty()) {
		return std::wstring();
	}
	else {
		return StringHelper::Printf(L"#%02X%02X%02X%02X", color.GetA(), color.GetR(), color.GetG(), color.GetB());
	}
}

bool Control::HasBoxShadow() const
{
	if (m_pBoxShadow != nullptr) {
		return m_pBoxShadow->HasShadow();
	}
	return false;
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

bool Control::ScreenToClient(UiPoint& pt)
{
	Window* pWindow = GetWindow();
	if ((pWindow == nullptr) || !pWindow->IsWindow()) {
		return false;
	}
	pWindow->ScreenToClient(pt);
	return true;
}

bool Control::ClientToScreen(UiPoint& pt)
{
	Window* pWindow = GetWindow();
	if ((pWindow == nullptr) || !pWindow->IsWindow()) {
		return false;
	}
	pWindow->ClientToScreen(pt);
	return true;
}

void Control::SetPaintOrder(uint8_t nPaintOrder)
{
	m_nPaintOrder = nPaintOrder;
}

uint8_t Control::GetPaintOrder() const
{
	return m_nPaintOrder;
}

} // namespace ui
