#include "ImageList.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Core/GlobalManager.h"

namespace ui 
{

ImageList::ImageList():
    m_nNextID(0)
{
}

ImageList::~ImageList()
{
}

void ImageList::SetImageSize(UiSize imageSize, bool bNeedDpiScale)
{
    if (imageSize.cx < 0) {
        imageSize.cx = 0;
    }
    if (imageSize.cy < 0) {
        imageSize.cy = 0;
    }
    if (bNeedDpiScale) {
        GlobalManager::Instance().Dpi().ScaleSize(imageSize);
    }
    m_imageSize = imageSize;
}

UiSize ImageList::GetImageSize() const
{
    return m_imageSize;
}

int32_t ImageList::AddImageString(const std::wstring& imageString)
{
    int32_t imageId = -1;
    if (!imageString.empty()) {
        imageId = GetImageStringId(imageString);
        if (imageId < 0) {
            imageId = m_nNextID++;
            ImagePtr spImage = std::make_shared<Image>();
            spImage->SetImageString(imageString);
            m_imageMap[imageId] = spImage;
        }
    }    
    return imageId;
}

std::wstring ImageList::GetImageString(int32_t imageId) const
{
    std::wstring imageString;
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

int32_t ImageList::GetImageStringId(const std::wstring& imageString) const
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
