#include "ImageLoadAttribute.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePath.h"

namespace ui 
{
ImageLoadAttribute::ImageLoadAttribute(DString srcWidth,
                                       DString srcHeight,
                                       bool srcDpiScale,
                                       bool bHasSrcDpiScale,
                                       uint32_t iconSize):
    m_srcDpiScale(srcDpiScale),
    m_bHasSrcDpiScale(bHasSrcDpiScale),
    m_iconSize(iconSize)
{
    StringUtil::Trim(srcWidth);
    StringUtil::Trim(srcHeight);
    m_srcWidth = srcWidth;
    m_srcHeight = srcHeight;    
}

void ImageLoadAttribute::SetImageFullPath(const DString& imageFullPath)
{
    FilePath fullPath(imageFullPath);
    fullPath.NormalizeFilePath();
    m_srcImageFullPath = fullPath.ToString();
}

DString ImageLoadAttribute::GetImageFullPath() const
{
    ASSERT(!m_srcImageFullPath.empty());
    return m_srcImageFullPath.c_str();
}

bool ImageLoadAttribute::HasImageFullPath() const
{
    return !m_srcImageFullPath.empty();
}

DString ImageLoadAttribute::GetCacheKey(uint32_t nDpiScale) const
{
    ASSERT(!m_srcImageFullPath.empty());
    DString fullPath = m_srcImageFullPath.c_str();
    if ((nDpiScale != 0) && (nDpiScale != 100)) {
        //追加缩放百分比
        fullPath += _T("@");
        fullPath += StringUtil::UInt32ToString(nDpiScale);
    }
    if (!m_srcWidth.empty() || !m_srcHeight.empty()) {
        fullPath += _T("@");
        fullPath += m_srcWidth.c_str();
        fullPath += _T(":");
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

uint32_t ImageLoadAttribute::GetScacledSize(const DString& srcSize, uint32_t nImageSize) const
{
    if (srcSize.empty()) {
        return 0;
    }
    uint32_t nScaledSize = 0;
    if (srcSize.back() == _T('%')) {
        //按照百分比缩放
        double ratio = StringUtil::StringToDouble(srcSize);
        nScaledSize = static_cast<uint32_t>(nImageSize * ratio / 100);
    }
    else {
        //设置固定值
        nScaledSize = StringUtil::StringToInt32(srcSize.c_str());
    }

    if (nScaledSize != nImageSize) {
        return nScaledSize;
    }
    return 0;
}

}
