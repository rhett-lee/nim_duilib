#include "Image.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Utils//DpiManager.h"
#include "duilib/Animation/AnimationManager.h"
#include <tchar.h>

namespace ui 
{

ImageInfo::ImageInfo():
	m_bAlphaChannel(false),
	m_bCached(false),
	m_bDpiScaled(false),
	m_nWidth(0),
	m_nHeight(0)
{
}

ImageInfo::~ImageInfo()
{
	for (IBitmap* pBitmap : m_frameBitmaps) {
		delete pBitmap;
	}
}

void ImageInfo::SetImageFullPath(const std::wstring& path)
{
	m_imageFullPath = path;
}

const std::wstring& ImageInfo::GetImageFullPath() const
{
	return m_imageFullPath;
}

void ImageInfo::SetFrameInterval(const std::vector<int>& frameIntervals)
{
	m_frameIntervals = frameIntervals;
}

void ImageInfo::PushBackHBitmap(IBitmap* pBitmap)
{
	ASSERT(pBitmap != nullptr);
	if (pBitmap != nullptr) {
		m_frameBitmaps.push_back(pBitmap);
	}
}

IBitmap* ImageInfo::GetBitmap(size_t nIndex) const
{
	ASSERT(nIndex < m_frameBitmaps.size());
	if (nIndex < m_frameBitmaps.size()) {
		return m_frameBitmaps[nIndex];
	}
	return nullptr;
}

void ImageInfo::SetImageSize(int nWidth, int nHeight)
{
	ASSERT(nWidth > 0);
	ASSERT(nHeight > 0);
	if (nWidth > 0) {
		m_nWidth = nWidth; 
	}
	if (nHeight > 0) {
		m_nHeight = nHeight;
	}	
}

size_t ImageInfo::GetFrameCount() const
{
	return m_frameBitmaps.size();
}

bool ImageInfo::IsMultiFrameImage() const
{
	return m_frameBitmaps.size() > 1;
}

int ImageInfo::GetFrameInterval(size_t nIndex)
{
	if (nIndex >= m_frameIntervals.size()) {
		return 0;
	}
	if (nIndex >= m_frameBitmaps.size()) {
		return 0;
	}

	int interval = m_frameIntervals[nIndex]; 
	if (interval < 30) {
		interval = 100;
	}
	else if (interval < 50)	{
		interval = 50;
	}
	return interval;
}

ImageAttribute::ImageAttribute()
{
	Init();
}

void ImageAttribute::Init()
{
	sImageString.clear();
	sImagePath.clear();
	bFade = 0xFF;
	bTiledX = false;
	bFullTiledX = true;
	bTiledY = false;
	bFullTiledY = true;
	nTiledMargin = 0;
	rcDest.left = rcDest.top = rcDest.right = rcDest.bottom = DUI_NOSET_VALUE;
	rcSource.left = rcSource.top = rcSource.right = rcSource.bottom = DUI_NOSET_VALUE;
	rcCorner.left = rcCorner.top = rcCorner.right = rcCorner.bottom = 0;
	nPlayCount = -1;
}

void ImageAttribute::InitByImageString(const std::wstring& strImageString)
{
	Init();
	sImageString = strImageString;
	sImagePath = strImageString;
	ModifyAttribute(strImageString);
}

void ImageAttribute::ModifyAttribute(const std::wstring& strImageString)
{
	ImageAttribute& imageAttribute = *this;

	std::wstring sItem;
	std::wstring sValue;
	LPTSTR pstr = NULL;
	bool bScaleDest = true;

	LPCTSTR pStrImage = strImageString.c_str();
	while (*pStrImage != _T('\0')) {
		sItem.clear();
		sValue.clear();
		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
		while (*pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ')) {
			LPTSTR pstrTemp = ::CharNext(pStrImage);
			while (pStrImage < pstrTemp) {
				sItem += *pStrImage++;
			}
		}
		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
		if (*pStrImage++ != _T('=')) break;
		while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
		if (*pStrImage++ != _T('\'')) break;
		while (*pStrImage != _T('\0') && *pStrImage != _T('\'')) {
			LPTSTR pstrTemp = ::CharNext(pStrImage);
			while (pStrImage < pstrTemp) {
				sValue += *pStrImage++;
			}
		}
		if (*pStrImage++ != _T('\'')) break;
		if (!sValue.empty()) {
			if (sItem == _T("file") || sItem == _T("res")) {
				imageAttribute.sImagePath = sValue;
			}
			else if (sItem == _T("destscale")) {
				bScaleDest = (_tcscmp(sValue.c_str(), _T("true")) == 0);
			}
			else if (sItem == _T("dest")) {
				imageAttribute.rcDest.left = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcDest.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				imageAttribute.rcDest.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcDest.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);

				if (bScaleDest)
					DpiManager::GetInstance()->ScaleRect(imageAttribute.rcDest);
			}
			else if (sItem == _T("source")) {
				imageAttribute.rcSource.left = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcSource.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				imageAttribute.rcSource.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcSource.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			}
			else if (sItem == _T("corner")) {
				imageAttribute.rcCorner.left = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcCorner.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
				imageAttribute.rcCorner.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
				imageAttribute.rcCorner.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			}
			else if (sItem == _T("fade")) {
				imageAttribute.bFade = (BYTE)_tcstoul(sValue.c_str(), &pstr, 10);
			}
			else if (sItem == _T("xtiled")) {
				imageAttribute.bTiledX = (_tcscmp(sValue.c_str(), _T("true")) == 0);
			}
			else if (sItem == _T("fullxtiled")) {
				imageAttribute.bFullTiledX = (_tcscmp(sValue.c_str(), _T("true")) == 0);
			}
			else if (sItem == _T("ytiled")) {
				imageAttribute.bTiledY = (_tcscmp(sValue.c_str(), _T("true")) == 0);
			}
			else if (sItem == _T("fullytiled")) {
				imageAttribute.bFullTiledY = (_tcscmp(sValue.c_str(), _T("true")) == 0);
			}
			else if (sItem == _T("tiledmargin")) {
				imageAttribute.nTiledMargin = _tcstol(sValue.c_str(), &pstr, 10); ASSERT(pstr);
			}
			else if (sItem == _T("playcount"))
			{
				imageAttribute.nPlayCount = _tcstol(sValue.c_str(), &pstr, 10);  ASSERT(pstr);
			}
		}
		if (*pStrImage++ != _T(' ')) {
			break;
		}
	}
}


Image::Image() :
	m_nCurrentFrame(0),
	m_bPlaying(false),
	m_nCycledCount(0)
{
}

void Image::InitImageAttribute()
{
	m_imageAttribute.Init();
}

void Image::SetImageString(const std::wstring& strImageString)
{
	ClearImageCache();
	m_imageAttribute.InitByImageString(strImageString);
}

const std::wstring& Image::GetImageString() const
{
	return m_imageAttribute.sImageString;
}

const std::wstring Image::GetImagePath() const
{
	return m_imageAttribute.sImagePath;
}

void Image::ClearImageCache()
{
	m_nCurrentFrame = 0;
	m_bPlaying = false;
	m_imageCache.reset();
	m_nCycledCount = 0;
}

void Image::SetImagePlayCount(int nPlayCount)
{
	m_imageAttribute.nPlayCount = nPlayCount;
}

void Image::SetImageFade(uint8_t nFade)
{
	m_imageAttribute.bFade = nFade;
}

bool Image::IncrementCurrentFrame()
{
	if (!m_imageCache) {
		return false;
	}
	m_nCurrentFrame++;
	if (m_nCurrentFrame == m_imageCache->GetFrameCount()) {
		m_nCurrentFrame = 0;
		m_nCycledCount += 1;
	}
	return true;
}

void Image::SetCurrentFrame(size_t nCurrentFrame)
{
	m_nCurrentFrame = nCurrentFrame;
}

IBitmap* Image::GetCurrentBitmap() const
{
	if (!m_imageCache) {
		return nullptr;
	}
	return m_imageCache->GetBitmap(m_nCurrentFrame);
}

int Image::GetCurrentInterval() const
{
	if (!m_imageCache) {
		return 0;
	}
	return m_imageCache->GetFrameInterval(m_nCurrentFrame);
}

size_t Image::GetCurrentFrameIndex() const
{
	return m_nCurrentFrame;
}

int Image::GetCycledCount() const
{
	return m_nCycledCount;
}

void Image::ClearCycledCount()
{
	m_nCycledCount = 0;
}

bool Image::ContinuePlay() const
{
	if (m_imageAttribute.nPlayCount < 0) {
		return true;
	}
	else if (m_imageAttribute.nPlayCount == 0) {
		return m_bPlaying;
	}
	else {
		return m_nCycledCount < m_imageAttribute.nPlayCount;
	}
}

const ImageAttribute& Image::GetImageAttribute() const
{
	return m_imageAttribute;
}

const std::shared_ptr<ImageInfo>& Image::GetImageCache() const
{
	return m_imageCache;
}

void Image::SetImageCache(const std::shared_ptr<ImageInfo>& imageInfo)
{
	m_imageCache = imageInfo;
}

StateImage::StateImage() :
	m_pControl(nullptr),
	m_stateImageMap()
{

}

void StateImage::SetImageString(ControlStateType stateType, const std::wstring& strImageString)
{
	m_stateImageMap[stateType].SetImageString(strImageString);
}

std::wstring StateImage::GetImageString(ControlStateType stateType) const
{
	std::wstring imageString;
	auto iter = m_stateImageMap.find(stateType);
	if (iter != m_stateImageMap.end()) {
		imageString = iter->second.GetImageString();
	}
	return imageString;
}

std::wstring StateImage::GetImagePath(ControlStateType stateType) const
{
	std::wstring imageFilePath;
	auto iter = m_stateImageMap.find(stateType);
	if (iter != m_stateImageMap.end()) {
		imageFilePath = iter->second.GetImagePath();
	}
	return imageFilePath;
}

UiRect StateImage::GetImageSourceRect(ControlStateType stateType) const
{
	UiRect rcSource;
	auto iter = m_stateImageMap.find(stateType);
	if (iter != m_stateImageMap.end()) {
		rcSource = iter->second.GetImageAttribute().rcSource;
	}
	return rcSource;
}

int StateImage::GetImageFade(ControlStateType stateType) const
{
	int nFade = 0xFF;
	auto iter = m_stateImageMap.find(stateType);
	if (iter != m_stateImageMap.end()) {
		nFade = iter->second.GetImageAttribute().bFade;
	}
	return nFade;
}

bool StateImage::HasHotImage() const
{
	return !GetImageString(kControlStateHot).empty();
}

bool StateImage::HasImage() const
{
	return !GetImageString(kControlStateNormal).empty() ||
		   !GetImageString(kControlStateHot).empty()    ||
		   !GetImageString(kControlStatePushed).empty() ||
		   !GetImageString(kControlStateDisabled).empty();
}

bool StateImage::PaintStateImage(IRenderContext* pRender, ControlStateType stateType, const std::wstring& sImageModify)
{
	if (m_pControl != nullptr) {
		bool bFadeHot = m_pControl->GetAnimationManager().GetAnimationPlayer(kAnimationHot) != nullptr;
		int nHotAlpha = m_pControl->GetHotAlpha();
		if (bFadeHot) {
			if (stateType == kControlStateNormal || stateType == kControlStateHot) {
				std::wstring strNormalImagePath = GetImagePath(kControlStateNormal);
				std::wstring strHotImagePath = GetImagePath(kControlStateHot);
				if (strNormalImagePath.empty() || 
					strHotImagePath.empty()    || 
					(strNormalImagePath != strHotImagePath) || 
					!GetImageSourceRect(kControlStateNormal).Equal(GetImageSourceRect(kControlStateHot))) {

					m_pControl->DrawImage(pRender, GetStateImage(kControlStateNormal), sImageModify);
					int nHotFade = GetImageFade(kControlStateHot);
					nHotFade = int(nHotFade * (double)nHotAlpha / 255);
					return m_pControl->DrawImage(pRender, GetStateImage(kControlStateHot), sImageModify, nHotFade);
				}
				else {
					int nNormalFade = GetImageFade(kControlStateNormal);
					int nHotFade = GetImageFade(kControlStateHot);
					int nBlendFade = int((1 - (double)nHotAlpha / 255) * nNormalFade + (double)nHotAlpha / 255 * nHotFade);
					return m_pControl->DrawImage(pRender, GetStateImage(kControlStateHot), sImageModify, nBlendFade);
				}
			}
		}
	}

	if (stateType == kControlStatePushed && GetImageString(kControlStatePushed).empty()) {
		stateType = kControlStateHot;
		m_stateImageMap[kControlStateHot].SetImageFade(255);
	}
	if (stateType == kControlStateHot && GetImageString(kControlStateHot).empty()) {
		stateType = kControlStateNormal;
	}
	if (stateType == kControlStateDisabled && GetImageString(kControlStateDisabled).empty()) {
		stateType = kControlStateNormal;
	}

	return m_pControl->DrawImage(pRender, GetStateImage(stateType), sImageModify);
}

Image* StateImage::GetEstimateImage()
{
	Image* pEstimateImage = nullptr;
	auto iter = m_stateImageMap.find(kControlStateNormal);
	if (iter != m_stateImageMap.end()) {
		if (!iter->second.GetImagePath().empty()) {
			pEstimateImage = &(iter->second);
		}		
	}
	if(pEstimateImage == nullptr) {
		iter = m_stateImageMap.find(kControlStateHot);
		if (iter != m_stateImageMap.end()) {
			if (!iter->second.GetImagePath().empty()) {
				pEstimateImage = &(iter->second);
			}
		}
	}
	if (pEstimateImage == nullptr) {
		iter = m_stateImageMap.find(kControlStatePushed);
		if (iter != m_stateImageMap.end()) {
			if (!iter->second.GetImagePath().empty()) {
				pEstimateImage = &(iter->second);
			}
		}
	}
	if (pEstimateImage == nullptr) {
		iter = m_stateImageMap.find(kControlStateDisabled);
		if (iter != m_stateImageMap.end()) {
			if (!iter->second.GetImagePath().empty()) {
				pEstimateImage = &(iter->second);
			}
		}
	}
	return pEstimateImage;
}

void StateImage::ClearImageCache()
{	
	for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter)	{
		iter->second.ClearImageCache();
	}
}

StateImageMap::StateImageMap():
	m_pControl(nullptr)
{
}

void StateImageMap::SetControl(Control* control)
{
	m_pControl = control;
	for (auto& it : m_stateImageMap) {
		it.second.SetControl(control);
	}
}

void StateImageMap::SetImageString(StateImageType stateImageType, ControlStateType stateType, const std::wstring& strImagePath)
{
	StateImage& stateImage = m_stateImageMap[stateImageType];
	stateImage.SetControl(m_pControl);
	stateImage.SetImageString(stateType, strImagePath);
}

std::wstring StateImageMap::GetImageString(StateImageType stateImageType, ControlStateType stateType) const
{
	std::wstring imageString;
	auto iter = m_stateImageMap.find(stateImageType);
	if (iter != m_stateImageMap.end()) {
		imageString = iter->second.GetImageString(stateType);
	}
	return imageString;
}

bool StateImageMap::HasHotImage() const
{
	for (auto& it : m_stateImageMap) {
		if (it.second.HasHotImage()) {
			return true;
		}
	}
	return false;
}

bool StateImageMap::HasImageType(StateImageType stateImageType) const
{
	bool bHasImage = false;
	auto iter = m_stateImageMap.find(stateImageType);
	if (iter != m_stateImageMap.end()) {
		bHasImage = iter->second.HasImage();
	}
	return bHasImage;
}

bool StateImageMap::PaintStateImage(IRenderContext* pRender, StateImageType stateImageType, ControlStateType stateType, const std::wstring& sImageModify /*= L""*/)
{
	auto it = m_stateImageMap.find(stateImageType);
	if (it != m_stateImageMap.end()) {
		return it->second.PaintStateImage(pRender, stateType, sImageModify);
	}
	return false;
}

Image* StateImageMap::GetEstimateImage(StateImageType stateImageType)
{
	auto it = m_stateImageMap.find(stateImageType);
	if (it != m_stateImageMap.end()) {
		return it->second.GetEstimateImage();
	}
	return nullptr;
}

void StateImageMap::ClearImageCache()
{
	for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
		iter->second.ClearImageCache();
	}
}

StateColorMap::StateColorMap() :
	m_pControl(nullptr),
	m_stateColorMap()
{
}

void StateColorMap::SetControl(Control* control)
{
	m_pControl = control;
}

bool StateColorMap::HasHotColor() const
{
	return m_stateColorMap.find(kControlStateHot) != m_stateColorMap.end();
}

bool StateColorMap::HasStateColors() const
{
	return (m_stateColorMap.find(kControlStateNormal) != m_stateColorMap.end()) ||
		   (m_stateColorMap.find(kControlStateHot) != m_stateColorMap.end())    ||
	       (m_stateColorMap.find(kControlStatePushed) != m_stateColorMap.end()) ||
		   (m_stateColorMap.find(kControlStateDisabled) != m_stateColorMap.end()) ;
}

bool StateColorMap::HasStateColor(ControlStateType stateType) const
{
	return m_stateColorMap.find(stateType) != m_stateColorMap.end();
}

std::wstring StateColorMap::GetStateColor(ControlStateType stateType) const
{
	auto iter = m_stateColorMap.find(stateType);
	if (iter != m_stateColorMap.end()) {
		return iter->second;
	}
	return std::wstring();
}

void StateColorMap::SetStateColor(ControlStateType stateType, const std::wstring& color)
{
	if (!color.empty()) {
		m_stateColorMap[stateType] = color;
	}
	else {
		//确保颜色值不是空字符串
		auto iter = m_stateColorMap.find(stateType);
		if (iter != m_stateColorMap.end()) {
			m_stateColorMap.erase(iter);
		}
	}
}

void StateColorMap::PaintStateColor(IRenderContext* pRender, UiRect rcPaint, ControlStateType stateType) const
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (m_pControl != nullptr) {
		bool bFadeHot = m_pControl->GetAnimationManager().GetAnimationPlayer(kAnimationHot) != nullptr;
		int nHotAlpha = m_pControl->GetHotAlpha();
		if (bFadeHot) {
			if ((stateType == kControlStateNormal || stateType == kControlStateHot) && HasStateColor(kControlStateHot)) {
				std::wstring strColor = GetStateColor(kControlStateNormal);
				if (!strColor.empty()) {
					pRender->DrawColor(rcPaint, m_pControl->GetWindowColor(strColor));
				}
				if (nHotAlpha > 0) {
					pRender->DrawColor(rcPaint, m_pControl->GetWindowColor(GetStateColor(kControlStateHot)), static_cast<BYTE>(nHotAlpha));
				}
				return;
			}
		}
	}

	if (stateType == kControlStatePushed && HasStateColor(kControlStatePushed)) {
		stateType = kControlStateHot;
	}
	if (stateType == kControlStateHot && HasStateColor(kControlStateHot)) {
		stateType = kControlStateNormal;
	}
	if (stateType == kControlStateDisabled && HasStateColor(kControlStateDisabled)) {
		stateType = kControlStateNormal;
	}
	std::wstring strColor = GetStateColor(stateType);
	if (!strColor.empty()) {
		UiColor color = m_pControl ? m_pControl->GetWindowColor(strColor) : GlobalManager::GetTextColor(strColor);
		pRender->DrawColor(rcPaint, color);
	}
}

}