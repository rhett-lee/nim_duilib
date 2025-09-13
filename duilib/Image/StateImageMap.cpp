#include "StateImageMap.h"
#include "duilib/Core/Control.h"

namespace ui 
{
StateImageMap::StateImageMap():
    m_pControl(nullptr)
{
}

void StateImageMap::SetControl(Control* pControl)
{
    m_pControl = pControl;
    for (auto& it : m_stateImageMap) {
        it.second.SetControl(pControl);
    }
}

void StateImageMap::SetImageString(StateImageType stateImageType, 
                                   ControlStateType stateType, 
                                   const DString& strImagePath,
                                   const DpiManager& dpi)
{
    StateImage& stateImage = m_stateImageMap[stateImageType];
    stateImage.SetControl(m_pControl);
    stateImage.SetImageString(stateType, strImagePath, dpi);
}

DString StateImageMap::GetImageString(StateImageType stateImageType, ControlStateType stateType) const
{
    DString imageString;
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

bool StateImageMap::PaintStateImage(IRender* pRender, 
                                    StateImageType stateImageType, 
                                    ControlStateType stateType, 
                                    const DString& sImageModify,
                                    UiRect* pDestRect)
{
    bool bRet = false;
    auto it = m_stateImageMap.find(stateImageType);
    if (it != m_stateImageMap.end()) {
        bRet = it->second.PaintStateImage(pRender, stateType, sImageModify, pDestRect);
    }
    //停止其他状态图片的动画
    if ((stateImageType == kStateImageBk) || (stateImageType == kStateImageFore)) {
        for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
            if ((iter->first == kStateImageBk) || (iter->first == kStateImageFore)) {
                continue;
            }
            iter->second.StopImageAnimation();
        }
    }
    else if ((stateImageType == kStateImageSelectedBk) || (stateImageType == kStateImageSelectedFore)) {
        for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
            if ((iter->first == kStateImageSelectedBk) || (iter->first == kStateImageSelectedFore)) {
                continue;
            }
            iter->second.StopImageAnimation();
        }
    }
    else if ((stateImageType == kStateImagePartSelectedBk) || (stateImageType == kStateImagePartSelectedFore)) {
        for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
            if ((iter->first == kStateImagePartSelectedBk) || (iter->first == kStateImagePartSelectedFore)) {
                continue;
            }
            iter->second.StopImageAnimation();
        }
    }
    return bRet;
}

Image* StateImageMap::GetEstimateImage(StateImageType stateImageType) const
{
    auto it = m_stateImageMap.find(stateImageType);
    if (it != m_stateImageMap.end()) {
        return it->second.GetEstimateImage();
    }
    return nullptr;
}

Image* StateImageMap::GetStateImage(StateImageType stateImageType, ControlStateType stateType) const
{
    auto it = m_stateImageMap.find(stateImageType);
    if (it != m_stateImageMap.end()) {
        const StateImage& stateImage = it->second;
        return stateImage.GetStateImage(stateType);
    }
    return nullptr;
}

void StateImageMap::GetAllImages(std::vector<Image*>& allImages) const
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

void StateImageMap::StopImageAnimation()
{
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        iter->second.StopImageAnimation();
    }
}

Image* StateImageMap::FindImageByName(const DString& imageName) const
{
    if (imageName.empty()) {
        return nullptr;
    }
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        Image* pImage = iter->second.FindImageByName(imageName);
        if (pImage != nullptr) {
            return pImage;
        }
    }
    return nullptr;
}

}//namespace ui 
