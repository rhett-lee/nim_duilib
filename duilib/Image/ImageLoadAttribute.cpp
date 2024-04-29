#include "ImageLoadAttribute.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"

namespace ui 
{
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

}