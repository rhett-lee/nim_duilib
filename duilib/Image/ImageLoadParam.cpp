#include "ImageLoadParam.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePath.h"

namespace ui 
{
ImageLoadParam::ImageLoadParam(DString srcWidth,
                               DString srcHeight,
                               DpiScaleOption nDpiScaleOption,
                               uint32_t nLoadDpiScale,
                               bool bIconAsAnimation,
                               int32_t nIconFrameDelayMs,
                               uint32_t nIconSize,
                               float fPagMaxFrameRate):
    m_nDpiScaleOption(nDpiScaleOption),
    m_nLoadDpiScale(nLoadDpiScale),
    m_bIconAsAnimation(bIconAsAnimation),
    m_nIconFrameDelayMs(nIconFrameDelayMs),
    m_nIconSize(nIconSize),
    m_fPagMaxFrameRate(fPagMaxFrameRate)
{
    StringUtil::Trim(srcWidth);
    StringUtil::Trim(srcHeight);
    m_srcWidth = srcWidth;
    m_srcHeight = srcHeight;
}

void ImageLoadParam::SetImageLoadPath(const ImageLoadPath& imageLoadPath)
{
    m_srcImageLoadPath = imageLoadPath;
    FilePath fullPath(imageLoadPath.m_imageFullPath.c_str());
    fullPath.NormalizeFilePath();
    m_srcImageLoadPath.m_imageFullPath = fullPath.NativePath();//路径规范化    
}

const ImageLoadPath& ImageLoadParam::GetImageLoadPath() const
{
    ASSERT(!m_srcImageLoadPath.m_imageFullPath.empty());
    return m_srcImageLoadPath;
}

bool ImageLoadParam::HasImageLoadPath() const
{
    return !m_srcImageLoadPath.m_imageFullPath.empty();
}

DString ImageLoadParam::GetLoadKey(uint32_t nLoadDpiScale) const
{
    //格式为(中括号内容为可选)：<图片路径>[@nLoadDpiScale][@srcWidth:srcHeight]
    ASSERT(!m_srcImageLoadPath.m_imageFullPath.empty());
    DString fullPath = m_srcImageLoadPath.m_imageFullPath;
    if ((nLoadDpiScale != 0) && (nLoadDpiScale != 100)) {
        //追加缩放百分比
        fullPath += _T("@");
        fullPath += StringUtil::UInt32ToString(nLoadDpiScale);
    }
    if (!m_srcWidth.empty() || !m_srcHeight.empty()) {
        fullPath += _T("@");
        fullPath += m_srcWidth.c_str();
        fullPath += _T(":");
        fullPath += m_srcHeight.c_str();
    }
    return fullPath;
}

void ImageLoadParam::SetDpiScaleOption(DpiScaleOption nDpiScaleOption)
{
    m_nDpiScaleOption = nDpiScaleOption;
}

ImageLoadParam::DpiScaleOption ImageLoadParam::GetDpiScaleOption() const
{
    return m_nDpiScaleOption;
}

void ImageLoadParam::SetLoadDpiScale(uint32_t nLoadDpiScale)
{
    m_nLoadDpiScale = nLoadDpiScale;    
}

uint32_t ImageLoadParam::GetLoadDpiScale() const
{
    if (m_nLoadDpiScale == 0) {
        return 100;
    }
    return m_nLoadDpiScale;
}

bool ImageLoadParam::IsIconAsAnimation() const
{
    return m_bIconAsAnimation;
}

uint32_t ImageLoadParam::GetIconSize() const
{
    return m_nIconSize;
}

int32_t ImageLoadParam::GetIconFrameDelayMs() const
{
    return m_nIconFrameDelayMs;
}

float ImageLoadParam::GetPagMaxFrameRate() const
{
    return m_fPagMaxFrameRate;
}

bool ImageLoadParam::CalcImageLoadSize(uint32_t& nImageWidth, uint32_t& nImageHeight, bool bNeedDpiScale) const
{
    const uint32_t nOldImageWidth = nImageWidth;
    const uint32_t nOldImageHeight = nImageHeight;
    ASSERT((nImageWidth != 0) && (nImageHeight != 0));
    if ((nImageWidth == 0) || (nImageHeight == 0)) {
        return false;
    }
    uint32_t nScaledWidth = GetScacledSize(m_srcWidth.c_str(), nImageWidth, !bNeedDpiScale);
    uint32_t nScaledHeight = GetScacledSize(m_srcHeight.c_str(), nImageHeight, !bNeedDpiScale);
    bool isScaled = false;
    if ((nScaledWidth > 0) && (nScaledHeight > 0)) {
        //宽和高都有具体设置的值
        nImageWidth = nScaledWidth;
        nImageHeight = nScaledHeight;
        isScaled = true;
    }
    else if ((nScaledWidth > 0) && (nScaledHeight == 0)) {
        //设置了宽度，高度按图片原始比例自适应
        nImageHeight = static_cast<uint32_t>(nImageHeight * nScaledWidth * 1.0 / nImageWidth);
        nImageWidth = nScaledWidth;
        isScaled = true;
    }
    else if ((nScaledWidth == 0) && (nScaledHeight > 0)) {
        //设置了高度宽度按图片原始比例自适应
        nImageWidth = static_cast<uint32_t>(nImageWidth * nScaledHeight * 1.0 / nImageHeight + 0.5);
        nImageHeight = nScaledHeight;
        isScaled = true;
    }
    if (!isScaled) {
        nImageWidth = nOldImageWidth;
        nImageHeight = nOldImageHeight;
    }
    //按照配置进行DPI缩放
    if (bNeedDpiScale) {
        uint32_t nLoadDpiScale = GetLoadDpiScale();
        if ((nLoadDpiScale > 0) && (nLoadDpiScale != 100)) {
            ASSERT(m_nDpiScaleOption != DpiScaleOption::kDefault);
            if (m_nDpiScaleOption != DpiScaleOption::kOff) {
                nImageWidth = static_cast<uint32_t>(nImageWidth * nLoadDpiScale * 1.0 / 100.0 + 0.5);
                nImageHeight = static_cast<uint32_t>(nImageHeight * nLoadDpiScale * 1.0 / 100.0 + 0.5);
            }
        }
    }
    if ((nOldImageWidth == nImageWidth) && (nOldImageHeight == nImageHeight)) {
        isScaled = false;
    }
    else {
        isScaled = true;
    }
    return isScaled;
}

uint32_t ImageLoadParam::GetScacledSize(const DString& srcSize, uint32_t nImageSize, bool bImageSizeDpiScaled) const
{
    if (srcSize.empty()) {
        return 0;
    }
    uint32_t nScaledSize = 0;
    if (srcSize.back() == _T('%')) {
        //按照百分比缩放
        double ratio = StringUtil::StringToDouble(srcSize);
        nScaledSize = static_cast<uint32_t>(nImageSize * ratio / 100.0 + 0.5);
    }
    else {
        //设置固定值
        nScaledSize = StringUtil::StringToInt32(srcSize.c_str());
        if (bImageSizeDpiScaled) {
            uint32_t nLoadDpiScale = GetLoadDpiScale();
            if ((nLoadDpiScale > 0) && (nLoadDpiScale != 100)) {
                ASSERT(m_nDpiScaleOption != DpiScaleOption::kDefault);
                if (m_nDpiScaleOption != DpiScaleOption::kOff) {
                    nScaledSize = static_cast<uint32_t>(nScaledSize * nLoadDpiScale * 1.0 / 100.0 + 0.5);
                }
            }
        }
    }

    if (nScaledSize != nImageSize) {
        return nScaledSize;
    }
    return 0;
}

}
