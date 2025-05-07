#include "StateImage.h"
#include "duilib/Image/Image.h"
#include "duilib/Core/Control.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/Animation/AnimationManager.h"

namespace ui 
{
StateImage::StateImage() :
    m_pControl(nullptr),
    m_stateImageMap()
{
}

StateImage::~StateImage()
{
    for (auto iter : m_stateImageMap) {
        Image* pImage = iter.second;
        if (pImage != nullptr) {
            delete pImage;
        }
    }
    m_stateImageMap.clear();
}

void StateImage::SetControl(Control* pControl)
{ 
    m_pControl = pControl;
    for (auto iter : m_stateImageMap) {
        Image* pImage = iter.second;
        if (pImage != nullptr) {
            pImage->SetControl(pControl);
        }
    }
}

void StateImage::SetImageString(ControlStateType stateType, 
                                const DString& strImageString,
                                const DpiManager& dpi)
{
    Image* pImage = nullptr;
    auto iter = m_stateImageMap.find(stateType);
    if (iter != m_stateImageMap.end()) {
        pImage = iter->second;
        if (strImageString.empty()) {
            //如果设置为空，释放资源
            delete pImage;
            m_stateImageMap.erase(iter);
            return;
        }
    }
    if (pImage == nullptr) {
        pImage = new Image;
        pImage->SetControl(m_pControl);
        m_stateImageMap[stateType] = pImage;
    }
    pImage->SetImageString(strImageString, dpi);
}

DString StateImage::GetImageString(ControlStateType stateType) const
{
    DString imageString;
    auto iter = m_stateImageMap.find(stateType);
    if (iter != m_stateImageMap.end()) {
        imageString = iter->second->GetImageString();
    }
    return imageString;
}

DString StateImage::GetImagePath(ControlStateType stateType) const
{
    DString imageFilePath;
    auto iter = m_stateImageMap.find(stateType);
    if (iter != m_stateImageMap.end()) {
        imageFilePath = iter->second->GetImagePath();
    }
    return imageFilePath;
}

bool StateImage::AreImageSourceRectsEqual(ControlStateType stateType1, ControlStateType stateType2) const
{
    auto iter1 = m_stateImageMap.find(stateType1);
    auto iter2 = m_stateImageMap.find(stateType2);
    if ((iter1 != m_stateImageMap.end()) && (iter2 != m_stateImageMap.end())) {
        UiRect rcSource1 = iter1->second->GetImageAttribute().GetImageSourceRect();
        UiRect rcSource2 = iter2->second->GetImageAttribute().GetImageSourceRect();
        return rcSource1.Equals(rcSource2);
    }
    return false;
}

int32_t StateImage::GetImageFade(ControlStateType stateType) const
{
    int32_t nFade = 0xFF;
    auto iter = m_stateImageMap.find(stateType);
    if (iter != m_stateImageMap.end()) {
        nFade = iter->second->GetImageAttribute().m_bFade;
    }
    return nFade;
}

Image* StateImage::GetStateImage(ControlStateType stateType) const
{
    Image* pImage = nullptr;
    auto iter = m_stateImageMap.find(stateType);
    if (iter != m_stateImageMap.end()) {
        pImage = iter->second;
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
                                 const DString& sImageModify, UiRect* pDestRect)
{
    if (m_pControl != nullptr) {
        bool bFadeHot = m_pControl->GetAnimationManager().GetAnimationPlayer(AnimationType::kAnimationHot) != nullptr;
        int32_t nHotAlpha = m_pControl->GetHotAlpha();
        if (bFadeHot) {
            if (stateType == kControlStateNormal || stateType == kControlStateHot) {
                DString strNormalImagePath = GetImagePath(kControlStateNormal);
                DString strHotImagePath = GetImagePath(kControlStateHot);
                if (strNormalImagePath.empty() || 
                    strHotImagePath.empty()    || 
                    (strNormalImagePath != strHotImagePath) || 
                    !AreImageSourceRectsEqual(kControlStateNormal, kControlStateHot)) {
                    m_pControl->PaintImage(pRender, GetStateImage(kControlStateNormal), sImageModify, -1, nullptr, nullptr, pDestRect);
                    int32_t nHotFade = GetImageFade(kControlStateHot);
                    nHotFade = int32_t(nHotFade * (double)nHotAlpha / 255);
                    return m_pControl->PaintImage(pRender, GetStateImage(kControlStateHot), sImageModify, nHotFade);
                }
                else {
                    int32_t nNormalFade = GetImageFade(kControlStateNormal);
                    int32_t nHotFade = GetImageFade(kControlStateHot);
                    int32_t nBlendFade = int32_t((1 - (double)nHotAlpha / 255) * nNormalFade + (double)nHotAlpha / 255 * nHotFade);
                    return m_pControl->PaintImage(pRender, GetStateImage(kControlStateHot), sImageModify, nBlendFade, nullptr, nullptr, pDestRect);
                }
            }
        }
    }

    if (stateType == kControlStatePushed && GetImageString(kControlStatePushed).empty()) {
        stateType = kControlStateHot;
        auto iter = m_stateImageMap.find(kControlStateHot);
        if (iter != m_stateImageMap.end()) {
            iter->second->SetImageFade(255);
        }
    }
    if (stateType == kControlStateHot && GetImageString(kControlStateHot).empty()) {
        stateType = kControlStateNormal;
    }
    if (stateType == kControlStateDisabled && GetImageString(kControlStateDisabled).empty()) {
        stateType = kControlStateNormal;
    }
    Image* pImage = GetStateImage(stateType);
    if (pImage == nullptr) {
        return false;
    }
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        ASSERT(iter->second != nullptr);
        if (iter->second != pImage) {
            //停止其他状态图片的动画
            iter->second->StopGifPlay();
        }
    }
    if (m_pControl != nullptr) {
        return m_pControl->PaintImage(pRender, pImage, sImageModify, -1, nullptr, nullptr, pDestRect);
    }
    return false;
}

Image* StateImage::GetEstimateImage() const
{
    Image* pEstimateImage = nullptr;
    auto iter = m_stateImageMap.find(kControlStateNormal);
    if (iter != m_stateImageMap.end()) {
        if (!iter->second->GetImagePath().empty()) {
            pEstimateImage = iter->second;
        }        
    }
    if(pEstimateImage == nullptr) {
        iter = m_stateImageMap.find(kControlStateHot);
        if (iter != m_stateImageMap.end()) {
            if (!iter->second->GetImagePath().empty()) {
                pEstimateImage = iter->second;
            }
        }
    }
    if (pEstimateImage == nullptr) {
        iter = m_stateImageMap.find(kControlStatePushed);
        if (iter != m_stateImageMap.end()) {
            if (!iter->second->GetImagePath().empty()) {
                pEstimateImage = iter->second;
            }
        }
    }
    if (pEstimateImage == nullptr) {
        iter = m_stateImageMap.find(kControlStateDisabled);
        if (iter != m_stateImageMap.end()) {
            if (!iter->second->GetImagePath().empty()) {
                pEstimateImage = iter->second;
            }
        }
    }
    return pEstimateImage;
}

void StateImage::GetAllImages(std::vector<Image*>& allImages) const
{
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        ASSERT(iter->second != nullptr);
        allImages.push_back(iter->second);
    }
}

void StateImage::ClearImageCache()
{    
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        ASSERT(iter->second != nullptr);
        iter->second->ClearImageCache();
    }
}

void StateImage::StopGifPlay()
{
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        ASSERT(iter->second != nullptr);
        iter->second->StopGifPlay();
    }
}

}
