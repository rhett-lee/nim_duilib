#include "ImageLoadParam.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui 
{
ImageLoadParam::ImageLoadParam():
    m_loadDpiScaleOption(DpiScaleOption::kDefault),
    m_imageSizeDpiScaleOption(DpiScaleOption::kDefault),
    m_nLoadDpiScale(100),
    m_bAsyncDecode(false),
    m_bIconAsAnimation(false),
    m_nIconFrameDelayMs(1000),
    m_nIconSize(0),
    m_fPagMaxFrameRate(30.0f)
{
}

ImageLoadParam::ImageLoadParam(DString srcWidth,
                               DString srcHeight,
                               DpiScaleOption loadDpiScaleOption,
                               DpiScaleOption imageSizeDpiScaleOption,
                               uint32_t nLoadDpiScale,
                               bool bAsyncDecode,
                               bool bIconAsAnimation,
                               int32_t nIconFrameDelayMs,
                               uint32_t nIconSize,
                               float fPagMaxFrameRate,
                               const DString& pagFilePwd):
    m_loadDpiScaleOption(loadDpiScaleOption),
    m_imageSizeDpiScaleOption(imageSizeDpiScaleOption),
    m_nLoadDpiScale(nLoadDpiScale),
    m_bAsyncDecode(bAsyncDecode),
    m_bIconAsAnimation(bIconAsAnimation),
    m_nIconFrameDelayMs(nIconFrameDelayMs),
    m_nIconSize(nIconSize),
    m_fPagMaxFrameRate(fPagMaxFrameRate),
    m_pagFilePwd(pagFilePwd)
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

bool ImageLoadParam::IsSvgImageFile() const
{
    if (m_srcImageLoadPath.m_imageFullPath.empty()) {
        return false;
    }
    DString fileExt = FilePathUtil::GetFileExtension(m_srcImageLoadPath.m_imageFullPath);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt == _T("SVG")) {
        return true;
    }
    return false;
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

void ImageLoadParam::SetLoadDpiScaleOption(DpiScaleOption loadDpiScaleOption)
{
    m_loadDpiScaleOption = loadDpiScaleOption;
}

DpiScaleOption ImageLoadParam::GetLoadDpiScaleOption() const
{
    return m_loadDpiScaleOption;
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

DString ImageLoadParam::GetPagFilePwd() const
{
    return m_pagFilePwd.c_str();
}

bool ImageLoadParam::IsAsyncDecodeEnabled() const
{
    return m_bAsyncDecode;
}

bool ImageLoadParam::HasImageFixedSize(void) const
{
    uint32_t nImageFixedWidth = 0;
    uint32_t nImageFixedHeight = 0;
    return GetImageFixedSize(nImageFixedWidth, nImageFixedHeight, false);
}

bool ImageLoadParam::GetImageFixedSize(uint32_t& nImageWidth, uint32_t& nImageHeight, bool bNeedDpiScale) const
{
    if (!GetScaledFixedSize(m_srcWidth.c_str(), nImageWidth, bNeedDpiScale)) {
        nImageWidth = 0;
    }
    if (!GetScaledFixedSize(m_srcHeight.c_str(), nImageHeight, bNeedDpiScale)) {
        nImageHeight = 0;
    }
    return (nImageHeight > 0) || (nImageWidth > 0);
}

bool ImageLoadParam::GetScaledFixedSize(const DString& srcSize, uint32_t& nScaledSize, bool bNeedDpiScale) const
{
    nScaledSize = 0;
    if (!srcSize.empty()) {
        if (srcSize.back() == _T('%')) {
            //按照百分比缩放
            nScaledSize = 0;
        }
        else {
            //设置固定值
            nScaledSize = StringUtil::StringToInt32(srcSize.c_str());
            if (bNeedDpiScale && (nScaledSize > 0)) {
                uint32_t nLoadDpiScale = GetLoadDpiScale();
                if ((nLoadDpiScale > 0) && (nLoadDpiScale != 100)) {
                    if (m_imageSizeDpiScaleOption != DpiScaleOption::kOff) {
                        nScaledSize = static_cast<uint32_t>(nScaledSize * nLoadDpiScale * 1.0 / 100.0 + 0.5);
                    }
                }
            }
        }
    }
    return nScaledSize > 0;
}

bool ImageLoadParam::HasImageFixedPercent() const
{
    float fImageFixedWidthPercent = 1.0f;
    float fImageFixedHeightPercent = 1.0f;
    return GetImageFixedPercent(fImageFixedWidthPercent, fImageFixedHeightPercent, false);
}

bool ImageLoadParam::GetImageFixedPercent(float& fImageWidthPercent, float& fImageHeightPercent, bool bNeedDpiScale) const
{
    bool bRetWidth = GetScaledFixedPercent(m_srcWidth.c_str(), fImageWidthPercent, bNeedDpiScale);
    if (!bRetWidth) {
        fImageWidthPercent = 1.0f;
    }
    bool bRetHeight = GetScaledFixedPercent(m_srcHeight.c_str(), fImageHeightPercent, bNeedDpiScale);
    if (!bRetHeight) {
        fImageHeightPercent = 1.0f;
    }
    return bRetWidth || bRetHeight;
}

bool ImageLoadParam::GetScaledFixedPercent(const DString& srcSize, float& fScaledPercent, bool bNeedDpiScale) const
{
    bool bRet = false;
    fScaledPercent = 1.0f;
    if (!srcSize.empty()) {
        if (srcSize.back() == _T('%')) {
            //按照百分比缩放(实际值需要除以100)
            double fRatio = StringUtil::StringToDouble(srcSize);            
            if (fRatio > 1) {//最小值为1%
                bRet = true;                
                fRatio /= 100;
                fScaledPercent = static_cast<float>(fRatio);
            }
        }
    }
    if (bRet && bNeedDpiScale) {
        uint32_t nLoadDpiScale = GetLoadDpiScale();
        if ((nLoadDpiScale > 0) && (nLoadDpiScale != 100)) {
            if (m_imageSizeDpiScaleOption != DpiScaleOption::kOff) {
                fScaledPercent = fScaledPercent * nLoadDpiScale / 100.0f;
            }
        }
    }
    return bRet && !ImageUtil::IsSameImageScale(fScaledPercent, 1.0f);
}

void ImageLoadParam::SetMaxDestRectSize(const UiSize& rcMaxDestRectSize)
{
    m_rcMaxDestRectSize = rcMaxDestRectSize;
}

UiSize ImageLoadParam::GetMaxDestRectSize() const
{
    return m_rcMaxDestRectSize;
}

}
