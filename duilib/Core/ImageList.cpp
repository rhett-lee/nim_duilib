#include "ImageList.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/DpiManager.h"

namespace ui 
{

ImageList::ImageList():
    m_nNextID(0)
{
}

ImageList::~ImageList()
{
}

void ImageList::SetImageSize(UiSize imageSize, const DpiManager& dpi, bool bNeedDpiScale)
{
    ASSERT ((imageSize.cx > 0) && (imageSize.cy > 0) );
    if ((imageSize.cx < 0) || (imageSize.cy < 0)) {
        return;
    }
    m_imageSize = imageSize;
    m_imageSizeNoDpi = imageSize;
    if (bNeedDpiScale) {
        dpi.ScaleSize(m_imageSize);
    }    
}

UiSize ImageList::GetImageSize() const
{
    return m_imageSize;
}

void ImageList::ChangeDpiScale(const DpiManager& dpi, uint32_t nOldDpiScale)
{
    UiSize szImageSize = GetImageSize();
    if ((szImageSize.cx > 0) && (szImageSize.cy > 0)) {
        szImageSize = dpi.GetScaleSize(szImageSize, nOldDpiScale);
        SetImageSize(szImageSize, dpi, false);
    }
}

int32_t ImageList::AddImageString(const DString& imageString, const DpiManager& dpi)
{
    int32_t imageId = -1;
    if (!imageString.empty()) {
        imageId = GetImageStringId(imageString);
        if (imageId < 0) {
            imageId = m_nNextID++;
            ImagePtr spImage = std::make_shared<Image>();
            spImage->SetImageString(imageString, dpi);
            m_imageMap[imageId] = spImage;
        }
    }    
    return imageId;
}

int32_t ImageList::AddImageStringWithSize(const DString& imageString, const DpiManager& dpi)
{
    ASSERT(imageString.find(_T("width=")) == DString::npos);
    ASSERT(imageString.find(_T("height=")) == DString::npos);
    DString newImageString = imageString;
    UiSize szImage = m_imageSizeNoDpi;
    if ((szImage.cx > 0) && (szImage.cy > 0)) {
        newImageString = StringUtil::Printf(_T("file='%s' width='%d' height='%d'"), imageString.c_str(), szImage.cx, szImage.cy);
    }
    return AddImageString(newImageString, dpi);
}

DString ImageList::GetImageString(int32_t imageId) const
{
    DString imageString;
    auto iter = m_imageMap.find(imageId);
    if (iter != m_imageMap.end()) {
        imageString = iter->second->GetImageString();
    }
    return imageString;
}

ImagePtr ImageList::GetImageData(int32_t imageId) const
{
    ImagePtr spImage;
    auto iter = m_imageMap.find(imageId);
    if (iter != m_imageMap.end()) {
        spImage = iter->second;
    }
    return spImage;
}

int32_t ImageList::GetImageStringId(const DString& imageString) const
{
    int32_t imageId = -1;
    if (!imageString.empty()) {
        for (auto iter = m_imageMap.begin(); iter != m_imageMap.end(); ++iter) {
            if (iter->second->EqualToImageString(imageString)) {
                imageId = iter->first;
                break;
            }
        }
    }
    return imageId;
}

bool ImageList::RemoveImageString(int32_t imageId)
{
    bool bRet = false;
    auto iter = m_imageMap.find(imageId);
    if (iter != m_imageMap.end()) {
        m_imageMap.erase(iter);
        bRet = true;
    }
    return bRet;
}

size_t ImageList::GetImageCount() const
{
    return m_imageMap.size();
}

void ImageList::GetImageIdList(std::vector<int32_t>& imageIdList) const
{
    imageIdList.clear();
    for (auto iter = m_imageMap.begin(); iter != m_imageMap.end(); ++iter) {
        imageIdList.push_back(iter->first);
    }
}

void ImageList::Clear()
{
    m_imageMap.clear();
}

} //namespace ui 
