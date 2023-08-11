#include "Image.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/AttributeUtil.h"

namespace ui 
{

ImageInfo::ImageInfo():
	m_bDpiScaled(false),
	m_nWidth(0),
	m_nHeight(0),
	m_nPlayCount(-1),
	m_pFrameIntervals(nullptr),
	m_nFrameCount(0),
	m_pFrameBitmaps(nullptr)
{
}

ImageInfo::~ImageInfo()
{
	if (m_pFrameBitmaps != nullptr) {
		for (uint32_t i = 0; i < m_nFrameCount; ++i) {
			delete m_pFrameBitmaps[i];
		}
		delete m_pFrameBitmaps;
		m_pFrameBitmaps = nullptr;
	}
	
	if (m_pFrameIntervals != nullptr) {
		delete m_pFrameIntervals;
		m_pFrameIntervals = nullptr;
	}
}

void ImageInfo::SetImageFullPath(const std::wstring& path)
{
	m_imageFullPath = path;
}

std::wstring ImageInfo::GetImageFullPath() const
{
	return m_imageFullPath.c_str();
}

void ImageInfo::SetFrameInterval(const std::vector<int32_t>& frameIntervals)
{
	if (m_pFrameIntervals == nullptr) {
		m_pFrameIntervals = new std::vector<int32_t>;
	}
	*m_pFrameIntervals = frameIntervals;
}

void ImageInfo::SetFrameBitmap(const std::vector<IBitmap*>& frameBitmaps)
{
	if (m_pFrameBitmaps != nullptr) {
		for (uint32_t i = 0; i < m_nFrameCount; ++i) {
			delete m_pFrameBitmaps[i];
		}
		delete m_pFrameBitmaps;
		m_pFrameBitmaps = nullptr;
	}
	m_nFrameCount = (uint32_t)frameBitmaps.size();
	if (m_nFrameCount > 0) {
		m_pFrameBitmaps = new IBitmap*[m_nFrameCount];
		for (uint32_t i = 0; i < m_nFrameCount; ++i) {
			m_pFrameBitmaps[i] = frameBitmaps[i];
		}
	}	
}

IBitmap* ImageInfo::GetBitmap(uint32_t nIndex) const
{
	ASSERT((nIndex < m_nFrameCount) && (m_pFrameBitmaps != nullptr));
	if ((nIndex < m_nFrameCount) && (m_pFrameBitmaps != nullptr)){
		return m_pFrameBitmaps[nIndex];
	}
	return nullptr;
}

void ImageInfo::SetImageSize(int32_t nWidth, int32_t nHeight)
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

uint32_t ImageInfo::GetFrameCount() const
{
	return m_nFrameCount;
}

bool ImageInfo::IsMultiFrameImage() const
{
	return GetFrameCount() > 1;
}

int32_t ImageInfo::GetFrameInterval(uint32_t nIndex) const
{
	if (m_pFrameIntervals == nullptr) {
		return 0;
	}
	const std::vector<int32_t>& frameIntervals = *m_pFrameIntervals;
	if (nIndex >= frameIntervals.size()) {
		return 0;
	}
	int32_t interval = frameIntervals[nIndex];
	if (interval < 30) {
		interval = 100;
	}
	else if (interval < 50)	{
		interval = 50;
	}
	return interval;
}

void ImageInfo::SetPlayCount(int32_t nPlayCount)
{
	m_nPlayCount = nPlayCount;
}

int32_t ImageInfo::GetPlayCount() const
{
	return m_nPlayCount;
}

void ImageInfo::SetCacheKey(const std::wstring& cacheKey)
{
	m_cacheKey = cacheKey;
}

std::wstring ImageInfo::GetCacheKey() const
{
	return m_cacheKey.c_str();
}

ImageAttribute::ImageAttribute():
	rcDest(nullptr),
	rcPadding(nullptr),
	rcSource(nullptr),
	rcCorner(nullptr)
{
	Init();
}

ImageAttribute::ImageAttribute(const ImageAttribute& r) :
	rcDest(nullptr),
	rcPadding(nullptr),
	rcSource(nullptr),
	rcCorner(nullptr)
{
	Init();
	*this = r;
}

ImageAttribute& ImageAttribute::operator=(const ImageAttribute& r)
{
	if (&r == this) {
		return *this;
	}

	sImageString = r.sImageString;
	sImagePath = r.sImagePath;
	srcWidth = r.srcWidth;
	srcHeight = r.srcHeight;

	srcDpiScale = r.srcDpiScale;
	bHasSrcDpiScale = r.bHasSrcDpiScale;
	hAlign = r.hAlign;
	vAlign = r.vAlign;

	bFade = r.bFade;
	bTiledX = r.bTiledX;
	bFullTiledX = r.bFullTiledX;
	bTiledY = r.bTiledY;
	bFullTiledY = r.bFullTiledY;
	nTiledMargin = r.nTiledMargin;
	nPlayCount = r.nPlayCount;
	iconSize = r.iconSize;
	bPaintEnabled = r.bPaintEnabled;

	if (r.rcDest != nullptr) {
		if (rcDest == nullptr) {
			rcDest = new UiRect;
		}
		*rcDest = *r.rcDest;
	}
	else {
		if (rcDest != nullptr) {
			delete rcDest;
			rcDest = nullptr;
		}
	}

	if (r.rcPadding != nullptr) {
		if (rcPadding == nullptr) {
			rcPadding = new UiPadding16;
		}
		*rcPadding = *r.rcPadding;
	}
	else {
		if (rcPadding != nullptr) {
			delete rcPadding;
			rcPadding = nullptr;
		}
	}

	if (r.rcSource != nullptr) {
		if (rcSource == nullptr) {
			rcSource = new UiRect;
		}
		*rcSource = *r.rcSource;
	}
	else {
		if (rcSource != nullptr) {
			delete rcSource;
			rcSource = nullptr;
		}
	}

	if (r.rcCorner != nullptr) {
		if (rcCorner == nullptr) {
			rcCorner = new UiRect;
		}
		*rcCorner = *r.rcCorner;
	}
	else {
		if (rcCorner != nullptr) {
			delete rcCorner;
			rcCorner = nullptr;
		}
	}

	return *this;
}

void ImageAttribute::Init()
{
	sImageString.clear();
	sImagePath.clear(); 
	srcWidth.clear();
	srcHeight.clear();

	srcDpiScale = false;
	bHasSrcDpiScale = false;
	hAlign.clear();
	vAlign.clear();

	bFade = 0xFF;
	bTiledX = false;
	bFullTiledX = true;
	bTiledY = false;
	bFullTiledY = true;
	nTiledMargin = 0;
	nPlayCount = -1;
	iconSize = 0;
	bPaintEnabled = true;

	if (rcDest != nullptr) {
		delete rcDest;
		rcDest = nullptr;
	}
	if (rcSource != nullptr) {
		delete rcSource;
		rcSource = nullptr;
	}
	if (rcPadding != nullptr) {
		delete rcPadding;
		rcPadding = nullptr;
	}
	if (rcCorner != nullptr) {
		delete rcCorner;
		rcCorner = nullptr;
	}
}

ImageAttribute::~ImageAttribute()
{
	if (rcDest != nullptr) {
		delete rcDest;
		rcDest = nullptr;
	}
	if (rcSource != nullptr) {
		delete rcSource;
		rcSource = nullptr;
	}
	if (rcPadding != nullptr) {
		delete rcPadding;
		rcPadding = nullptr;
	}
	if (rcCorner != nullptr) {
		delete rcCorner;
		rcCorner = nullptr;
	}
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
	if (strImageString.find(L'=') == std::wstring::npos) {
		//不含有等号，说明没有属性，直接返回
		return;
	}
	std::vector<std::pair<std::wstring, std::wstring>> attributeList;
	AttributeUtil::ParseAttributeList(strImageString, L'\'', attributeList);

	ImageAttribute& imageAttribute = *this;
	bool bDisalbeScaleDest = false;
	bool bHasDest = false;
	bHasSrcDpiScale = false;
	for (const auto& attribute : attributeList) {
		const std::wstring& name = attribute.first;
		const std::wstring& value = attribute.second;
		if (name.empty() || value.empty()) {
			continue;
		}
		if (name == L"file" || name == L"res") {
			//图片资源文件名，根据此设置去加载图片资源
			imageAttribute.sImagePath = value;
		}
		else if (name == L"width") {
			//设置图片宽度，可以放大或缩小图像：pixels或者百分比%，比如300，或者30%
			imageAttribute.srcWidth = value;
		}
		else if (name == L"height") {
			//设置图片高度，可以放大或缩小图像：pixels或者百分比%，比如200，或者30%
			imageAttribute.srcHeight = value;
		}
		else if (name == L"source") {
			//图片源区域设置：可以用于仅包含源图片的部分图片内容（比如通过此机制，将按钮的各个状态图片整合到一张大图片上，方便管理图片资源）
			if (imageAttribute.rcSource == nullptr) {
				imageAttribute.rcSource = new UiRect;
			}
			AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.rcSource);
		}
		else if (name == L"corner") {
			//图片的圆角属性，如果设置此属性，绘制图片的时候，采用九宫格绘制方式绘制图片：
			//    四个角不拉伸图片，四个边部分拉伸，中间部分可以拉伸或者根据xtiled、ytiled属性来平铺绘制
			if (imageAttribute.rcCorner == nullptr) {
				imageAttribute.rcCorner = new UiRect;
			}
			AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.rcCorner);
		}
		else if (name == L"dpiscale") {
			//加载图片时，按照DPI缩放图片大小（会影响width属性、height属性、sources属性、corner属性）
			imageAttribute.srcDpiScale = (value == L"true");
			bHasSrcDpiScale = true;
		}
		else if (name == L"dest") {
			//设置目标区域，该区域是指相对于所属控件的Rect区域
			if (imageAttribute.rcDest == nullptr) {
				imageAttribute.rcDest = new UiRect;
			}
			AttributeUtil::ParseRectValue(value.c_str(), *imageAttribute.rcDest);
			bHasDest = true;
		}
		else if (name == L"destscale") {
			//加载时，对dest属性按照DPI缩放图片，仅当设置了dest属性时有效（会影响dest属性）
			//绘制时（内部使用），控制是否对dest属性进行DPI缩放
			bDisalbeScaleDest = (value == L"false");
		}
		else if (name == L"padding") {
			//在目标区域中设置内边距
			UiPadding padding;
			AttributeUtil::ParsePaddingValue(value.c_str(), padding);
			GlobalManager::Instance().Dpi().ScalePadding(padding);
			imageAttribute.SetPadding(padding);
		}
		else if (name == L"halign") {
			//在目标区域中设置横向对齐方式			
			ASSERT((value == L"left") || (value == L"center") || (value == L"right"));
			if ((value == L"left") || (value == L"center") || (value == L"right")) {
				imageAttribute.hAlign = value;
			}
		}
		else if (name == L"valign") {
			//在目标区域中设置纵向对齐方式
			ASSERT((value == L"top") || (value == L"center") || (value == L"bottom"));
			if ((value == L"top") || (value == L"center") || (value == L"bottom")) {
				imageAttribute.vAlign = value;
			}
		}
		else if (name == L"fade") {
			//图片的透明度
			imageAttribute.bFade = (uint8_t)wcstoul(value.c_str(), nullptr, 10);
		}
		else if (name == L"xtiled") {
			//横向平铺
			imageAttribute.bTiledX = (value == L"true");
		}
		else if (name == L"fullxtiled") {
			//横向平铺时，保证整张图片绘制
			imageAttribute.bFullTiledX = (value == L"true");
		}
		else if (name == L"ytiled") {
			//纵向平铺
			imageAttribute.bTiledY = (value == L"true");
		}
		else if (name == L"fullytiled") {
			//纵向平铺时，保证整张图片绘制
			imageAttribute.bFullTiledY = (value == L"true");
		}
		else if (name == L"tiledmargin") {
			//平铺绘制时，各平铺图片之间的间隔，包括横向平铺和纵向平铺
			imageAttribute.nTiledMargin = wcstol(value.c_str(), nullptr, 10);
		}
		else if (name == L"playcount") {
			//如果是GIF、APNG、WEBP等动画图片，可以指定播放次数 -1 ：一直播放，缺省值。
			imageAttribute.nPlayCount = wcstol(value.c_str(), nullptr, 10);
			if (imageAttribute.nPlayCount < 0) {
				imageAttribute.nPlayCount = -1;
			}
		}
		else if (name == L"iconsize") {
			//指定加载ICO文件的图片大小(仅当图片文件是ICO文件时有效)
			imageAttribute.iconSize = (uint32_t)wcstol(value.c_str(), nullptr, 10);
		}
		else {
			ASSERT(!"ImageAttribute::ModifyAttribute: fount unknown attribute!");
		}
	}
	if (bHasDest && !bDisalbeScaleDest) {
		//如果没有配置"destscale" 或者 destscale="true"的情况，都需要对rcDest进行DPI自适应
		//只有设置了destscale="false"的时候，才禁止对rcDest进行DPI自适应
		if (imageAttribute.rcDest != nullptr) {
			GlobalManager::Instance().Dpi().ScaleRect(*imageAttribute.rcDest);
		}		
	}
}

bool ImageAttribute::HasValidImageRect(const UiRect& rcDest)
{
	if (rcDest.IsZero() || rcDest.IsEmpty()) {
		return false;
	}
	if ((rcDest.left >= 0) && (rcDest.top >= 0) && 
		(rcDest.Width() > 0) && (rcDest.Height() > 0)){
		return true;
	}
	return false;
}

void ImageAttribute::ScaleImageRect(uint32_t imageWidth, uint32_t imageHeight, bool bImageDpiScaled,
					                UiRect& rcDestCorners,
					                UiRect& rcSource, UiRect& rcSourceCorners)
{
	ASSERT((imageWidth > 0) && (imageHeight > 0));
	if ((imageWidth == 0) || (imageHeight == 0)) {
		return;
	}
	//对rcImageSourceCorners进行处理：对边角值进行容错处理（四个边代表边距，不代表矩形区域）
	//在XML解析加载的时候，未做DPI自适应；
	//在绘制的时候，如果图片做过DPI自适应，也要做DPI自适应，如果图片未做DPI自适应，也不需要做。	
	if ((rcSourceCorners.left < 0) || (rcSourceCorners.top < 0) ||
		(rcSourceCorners.right < 0)|| (rcSourceCorners.bottom < 0)) {
		rcSourceCorners.Clear();
	}
	else if (bImageDpiScaled) {
		GlobalManager::Instance().Dpi().ScaleRect(rcSourceCorners);
	}

	//对rcDestCorners进行处理：由rcSourceCorners赋值，边角保持一致，避免绘制图片的时候四个角有变形；
	//采用九宫格绘制的时候，四个角的存在，是为了避免绘制的时候四个角出现变形
	rcDestCorners = rcSourceCorners;
	if (!bImageDpiScaled) {
		//rcDestCorners必须做DPI自适应，rcSourceCorners可能不做DPI自适应（根据配置指定，跟随图片）
		GlobalManager::Instance().Dpi().ScaleRect(rcDestCorners);
	}

	// 如果源位图已经按照DPI缩放过，那么对应的rcImageSource也需要缩放
	if ((rcSource.left < 0) || (rcSource.top < 0) ||
		(rcSource.right < 0) || (rcSource.bottom < 0) ||
		(rcSource.Width() <= 0) || (rcSource.Height() <= 0)) {
		//如果是无效值，则重置为整个图片大小
		rcSource.left = 0;
		rcSource.top = 0;
		rcSource.right = (int32_t)imageWidth;
		rcSource.bottom = (int32_t)imageHeight;
	}
	else if (bImageDpiScaled) {
		//如果外部设置此值，做DPI自适应处理
		GlobalManager::Instance().Dpi().ScaleRect(rcSource);
	}

	//图片源容错处理
	if (rcSource.right > (int32_t)imageWidth) {
		rcSource.right = (int32_t)imageWidth;
	}
	if (rcSource.bottom > (int32_t)imageHeight) {
		rcSource.bottom = (int32_t)imageHeight;
	}
}

UiRect ImageAttribute::GetSourceRect() const
{
	UiRect rc;
	if (rcSource != nullptr) {
		rc = *rcSource;
	}
	return rc;
}

UiRect ImageAttribute::GetDestRect() const
{
	UiRect rc;
	if (rcDest != nullptr) {
		rc = *rcDest;
	}
	return rc;
}

UiPadding ImageAttribute::GetPadding() const
{
	UiPadding rc;
	if (rcPadding != nullptr) {
		rc = UiPadding(rcPadding->left, rcPadding->top, rcPadding->right, rcPadding->bottom);
	}
	return rc;
}

void ImageAttribute::SetPadding(const UiPadding& newPadding)
{
	if (rcPadding == nullptr) {
		rcPadding = new UiPadding16;
	}
	rcPadding->left = TruncateToUInt16(newPadding.left);
	rcPadding->top = TruncateToUInt16(newPadding.top);
	rcPadding->right = TruncateToUInt16(newPadding.right);
	rcPadding->bottom = TruncateToUInt16(newPadding.bottom);
}

UiRect ImageAttribute::GetCorner() const
{
	UiRect rc;
	if (rcCorner != nullptr) {
		rc = *rcCorner;
	}
	return rc;
}

ImageLoadAttribute::ImageLoadAttribute(std::wstring srcWidth,
									   std::wstring srcHeight,
	                                   bool srcDpiScale,
									   bool bHasSrcDpiScale,
	                                   uint32_t iconSize):
	m_srcDpiScale(srcDpiScale),
	m_bHasSrcDpiScale(bHasSrcDpiScale),
	m_iconSize(iconSize)
{
	StringHelper::Trim(srcWidth);
	StringHelper::Trim(srcHeight);
	m_srcWidth = srcWidth;
	m_srcHeight = srcHeight;	
}

void ImageLoadAttribute::SetImageFullPath(const std::wstring& imageFullPath)
{
	std::wstring fullPath = StringHelper::NormalizeFilePath(imageFullPath);
	StringHelper::Trim(fullPath);
	m_srcImageFullPath = fullPath;	
}

std::wstring ImageLoadAttribute::GetImageFullPath() const
{
	ASSERT(!m_srcImageFullPath.empty());
	return m_srcImageFullPath.c_str();
}

std::wstring ImageLoadAttribute::GetCacheKey() const
{
	ASSERT(!m_srcImageFullPath.empty());
	std::wstring fullPath = m_srcImageFullPath.c_str();
	if (!m_srcWidth.empty() || !m_srcHeight.empty()) {
		fullPath += L"@";
		fullPath += m_srcWidth.c_str();
		fullPath += L":";
		fullPath += m_srcHeight.c_str();
	}	
	return fullPath;
}

bool ImageLoadAttribute::NeedDpiScale() const
{
	return m_srcDpiScale;
}

void ImageLoadAttribute::SetNeedDpiScale(bool bNeedDpiScale)
{
	m_srcDpiScale = bNeedDpiScale;
}

bool ImageLoadAttribute::HasSrcDpiScale() const
{
	return m_bHasSrcDpiScale;
}

uint32_t ImageLoadAttribute::GetIconSize() const
{
	return m_iconSize;
}

bool ImageLoadAttribute::CalcImageLoadSize(uint32_t& nImageWidth, uint32_t& nImageHeight) const
{
	ASSERT((nImageWidth != 0) && (nImageHeight != 0));
	if ((nImageWidth == 0) || (nImageHeight == 0)) {
		return false;
	}
	uint32_t nScaledWidth = GetScacledSize(m_srcWidth.c_str(), nImageWidth);
	uint32_t nScaledHeight = GetScacledSize(m_srcHeight.c_str(), nImageHeight);
	bool isScaled = false;
	if ((nScaledWidth > 0) && (nScaledHeight > 0)) {
		//宽和高都有具体设置的值
		nImageWidth = nScaledWidth;
		nImageHeight = nScaledHeight;
		isScaled = true;
	}
	else if ((nScaledWidth > 0) && (nScaledHeight == 0)) {
		//设置了宽度，高度按图片原始比例自适应
		nImageHeight = nImageHeight * nScaledWidth / nImageWidth;
		nImageWidth = nScaledWidth;		
		isScaled = true;
	}
	else if ((nScaledWidth == 0) && (nScaledHeight > 0)) {
		//设置了高度宽度按图片原始比例自适应
		nImageWidth = nImageWidth * nScaledHeight / nImageHeight;
		nImageHeight = nScaledHeight;
		isScaled = true;
	}
	return isScaled;
}

uint32_t ImageLoadAttribute::GetScacledSize(const std::wstring& srcSize, uint32_t nImageSize) const
{
	if (srcSize.empty()) {
		return 0;
	}
	uint32_t nScaledSize = 0;
	if (srcSize.back() == L'%') {
		//按照百分比缩放
		double ratio = wcstod(srcSize.c_str(), nullptr);
		nScaledSize = static_cast<uint32_t>(nImageSize * ratio / 100);
	}
	else {
		//设置固定值
		nScaledSize = wcstol(srcSize.c_str(), nullptr, 10);
	}

	if (nScaledSize != nImageSize) {
		return nScaledSize;
	}
	return 0;
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

std::wstring Image::GetImageString() const
{
	return m_imageAttribute.sImageString.c_str();
}

std::wstring Image::GetImagePath() const
{
	return m_imageAttribute.sImagePath.c_str();
}

void Image::SetImagePadding(const UiPadding& newPadding)
{
	m_imageAttribute.SetPadding(newPadding);
}

UiPadding Image::GetImagePadding() const
{
	return m_imageAttribute.GetPadding();
}

bool Image::IsImagePaintEnabled() const
{
	return m_imageAttribute.bPaintEnabled;
}

void Image::SetImagePaintEnabled(bool bEnable)
{
	m_imageAttribute.bPaintEnabled = bEnable;
}

void Image::ClearImageCache()
{
	m_nCurrentFrame = 0;
	m_bPlaying = false;
	m_imageCache.reset();
	m_nCycledCount = 0;
}

void Image::SetImagePlayCount(int32_t nPlayCount)
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

void Image::SetCurrentFrame(uint32_t nCurrentFrame)
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

int32_t Image::GetCurrentInterval() const
{
	if (!m_imageCache) {
		return 0;
	}
	return m_imageCache->GetFrameInterval(m_nCurrentFrame);
}

uint32_t Image::GetCurrentFrameIndex() const
{
	return m_nCurrentFrame;
}

int32_t Image::GetCycledCount() const
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

ImageLoadAttribute Image::GetImageLoadAttribute() const
{
	return ImageLoadAttribute(m_imageAttribute.srcWidth.c_str(),
						      m_imageAttribute.srcHeight.c_str(),
		                      m_imageAttribute.srcDpiScale,
							  m_imageAttribute.bHasSrcDpiScale,
		                      m_imageAttribute.iconSize);
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
		rcSource = iter->second.GetImageAttribute().GetSourceRect();
	}
	return rcSource;
}

int32_t StateImage::GetImageFade(ControlStateType stateType) const
{
	int32_t nFade = 0xFF;
	auto iter = m_stateImageMap.find(stateType);
	if (iter != m_stateImageMap.end()) {
		nFade = iter->second.GetImageAttribute().bFade;
	}
	return nFade;
}

Image* StateImage::GetStateImage(ControlStateType stateType)
{
	Image* pImage = nullptr;
	auto iter = m_stateImageMap.find(stateType);
	if (iter != m_stateImageMap.end()) {
		pImage = &(iter->second);
	}
	return pImage;
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

bool StateImage::PaintStateImage(IRender* pRender, ControlStateType stateType, 
							     const std::wstring& sImageModify, UiRect* pDestRect)
{
	if (m_pControl != nullptr) {
		bool bFadeHot = m_pControl->GetAnimationManager().GetAnimationPlayer(kAnimationHot) != nullptr;
		int32_t nHotAlpha = m_pControl->GetHotAlpha();
		if (bFadeHot) {
			if (stateType == kControlStateNormal || stateType == kControlStateHot) {
				std::wstring strNormalImagePath = GetImagePath(kControlStateNormal);
				std::wstring strHotImagePath = GetImagePath(kControlStateHot);
				if (strNormalImagePath.empty() || 
					strHotImagePath.empty()    || 
					(strNormalImagePath != strHotImagePath) || 
					!GetImageSourceRect(kControlStateNormal).Equals(GetImageSourceRect(kControlStateHot))) {

					m_pControl->PaintImage(pRender, GetStateImage(kControlStateNormal), sImageModify, -1, nullptr, pDestRect);
					int32_t nHotFade = GetImageFade(kControlStateHot);
					nHotFade = int32_t(nHotFade * (double)nHotAlpha / 255);
					return m_pControl->PaintImage(pRender, GetStateImage(kControlStateHot), sImageModify, nHotFade);
				}
				else {
					int32_t nNormalFade = GetImageFade(kControlStateNormal);
					int32_t nHotFade = GetImageFade(kControlStateHot);
					int32_t nBlendFade = int32_t((1 - (double)nHotAlpha / 255) * nNormalFade + (double)nHotAlpha / 255 * nHotFade);
					return m_pControl->PaintImage(pRender, GetStateImage(kControlStateHot), sImageModify, nBlendFade, nullptr, pDestRect);
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

	return m_pControl->PaintImage(pRender, GetStateImage(stateType), sImageModify, -1, nullptr, pDestRect);
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

void StateImage::GetAllImages(std::vector<Image*>& allImages)
{
	for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
		allImages.push_back(&iter->second);
	}
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

bool StateImageMap::HasStateImages(void) const
{
	for (auto& it : m_stateImageMap) {
		if (it.second.HasImage()) {
			return true;
		}
	}
	return false;
}

bool StateImageMap::HasStateImage(StateImageType stateImageType) const
{
	bool bHasImage = false;
	auto iter = m_stateImageMap.find(stateImageType);
	if (iter != m_stateImageMap.end()) {
		bHasImage = iter->second.HasImage();
	}
	return bHasImage;
}

bool StateImageMap::PaintStateImage(IRender* pRender, StateImageType stateImageType, ControlStateType stateType, const std::wstring& sImageModify /*= L""*/)
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

Image* StateImageMap::GetStateImage(StateImageType stateImageType, ControlStateType stateType)
{
	auto it = m_stateImageMap.find(stateImageType);
	if (it != m_stateImageMap.end()) {
		StateImage& stateImage = it->second;
		return stateImage.GetStateImage(stateType);
	}
	return nullptr;
}

void StateImageMap::GetAllImages(std::vector<Image*>& allImages)
{
	for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
		iter->second.GetAllImages(allImages);
	}
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
		return iter->second.c_str();
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

void StateColorMap::PaintStateColor(IRender* pRender, UiRect rcPaint, ControlStateType stateType) const
{
	ASSERT(pRender != nullptr);
	if (pRender == nullptr) {
		return;
	}
	if (m_pControl != nullptr) {
		bool bFadeHot = m_pControl->GetAnimationManager().GetAnimationPlayer(kAnimationHot) != nullptr;
		int32_t nHotAlpha = m_pControl->GetHotAlpha();
		if (bFadeHot) {
			if ((stateType == kControlStateNormal || stateType == kControlStateHot) && HasStateColor(kControlStateHot)) {
				std::wstring strColor = GetStateColor(kControlStateNormal);
				if (!strColor.empty()) {
					pRender->FillRect(rcPaint, m_pControl->GetUiColor(strColor));
				}
				if (nHotAlpha > 0) {
					pRender->FillRect(rcPaint, m_pControl->GetUiColor(GetStateColor(kControlStateHot)), static_cast<BYTE>(nHotAlpha));
				}
				return;
			}
		}
	}

	if (stateType == kControlStatePushed && !HasStateColor(kControlStatePushed)) {
		stateType = kControlStateHot;
	}
	if (stateType == kControlStateHot && !HasStateColor(kControlStateHot)) {
		stateType = kControlStateNormal;
	}
	if (stateType == kControlStateDisabled && !HasStateColor(kControlStateDisabled)) {
		stateType = kControlStateNormal;
	}
	std::wstring strColor = GetStateColor(stateType);
	if (!strColor.empty()) {
		UiColor color = m_pControl ? m_pControl->GetUiColor(strColor) : 
			                         GlobalManager::Instance().Color().GetColor(strColor);
		pRender->FillRect(rcPaint, color);
	}
}

}