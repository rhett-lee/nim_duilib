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

bool StateImage::HasHoveredImage() const
{
    return !GetImageString(kControlStateHovered).empty();
}

bool StateImage::HasImage() const
{
    return !GetImageString(kControlStateNormal).empty() ||
           !GetImageString(kControlStateHovered).empty()    ||
           !GetImageString(kControlStatePressed).empty() ||
           !GetImageString(kControlStateDisabled).empty();
}

bool StateImage::PaintStateImage(IRender* pRender, ControlStateType stateType, 
                                 const DString& sImageModify, UiRect* pDestRect)
{
    if (m_pControl != nullptr) {        
        if (((stateType == kControlStateNormal) || (stateType == kControlStateHovered)) &&
            m_pControl->IsAnimationPlayerPlaying(AnimationType::kAnimationHovered)) {
            //正在播放Hovered状态动画
            uint8_t nHoveredAlpha = m_pControl->GetHoveredAlpha();
            Image* pNormalImage = GetStateImage(kControlStateNormal);
            Image* pHoveredImage = GetStateImage(kControlStateHovered);
            for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
                ASSERT(iter->second != nullptr);
                bool bNeedPause = true;
                if ((pNormalImage != nullptr) && (iter->second == pNormalImage)) {
                    bNeedPause = false;
                }
                if ((pHoveredImage != nullptr) && (iter->second == pHoveredImage)) {
                    bNeedPause = false;
                }
                if (bNeedPause) {
                    //暂停其他状态图片的动画
                    iter->second->PauseImageAnimation();
                }
            }
            bool bNormalPaintd = false;
            bool bHoveredPaintd = false;

            //先绘制Normal图片
            if (pNormalImage != nullptr) {
                int32_t nNormalFade = GetImageFade(kControlStateNormal);
                nNormalFade = int32_t(nNormalFade * (double)(255 - nHoveredAlpha) / 255);
                if (pHoveredImage == nullptr) {
                    nNormalFade = -1;
                }
                bNormalPaintd = m_pControl->PaintImage(pRender, pNormalImage, sImageModify, nNormalFade, nullptr, nullptr, pDestRect);
            }

            //绘制Hovered图片
            if (pHoveredImage != nullptr) {
                int32_t nHoveredFade = GetImageFade(kControlStateHovered);
                nHoveredFade = int32_t(nHoveredFade * (double)nHoveredAlpha / 255);
                bHoveredPaintd = m_pControl->PaintImage(pRender, pHoveredImage, sImageModify, nHoveredFade);                
            }

            if (bNormalPaintd || bHoveredPaintd) {
                return true;
            }
        }
    }

    if (stateType == kControlStatePressed && GetImageString(kControlStatePressed).empty()) {
        stateType = kControlStateHovered;
        auto iter = m_stateImageMap.find(kControlStateHovered);
        if (iter != m_stateImageMap.end()) {
            iter->second->SetImageFade(255);
        }
    }
    if (stateType == kControlStateHovered && GetImageString(kControlStateHovered).empty()) {
        stateType = kControlStateNormal;
    }
    if (stateType == kControlStateDisabled && GetImageString(kControlStateDisabled).empty()) {
        stateType = kControlStateNormal;
    }
    Image* pImage = GetStateImage(stateType);
    if ((pImage == nullptr) && (stateType != kControlStateNormal)) {
        //正常状态的图片，作为保底图片
        stateType = kControlStateNormal;
        pImage = GetStateImage(stateType);
    }
    if (pImage == nullptr) {
        return false;
    }
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        ASSERT(iter->second != nullptr);
        if (iter->second != pImage) {
            //暂停其他状态图片的动画
            iter->second->PauseImageAnimation();
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
        iter = m_stateImageMap.find(kControlStateHovered);
        if (iter != m_stateImageMap.end()) {
            if (!iter->second->GetImagePath().empty()) {
                pEstimateImage = iter->second;
            }
        }
    }
    if (pEstimateImage == nullptr) {
        iter = m_stateImageMap.find(kControlStatePressed);
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

void StateImage::StopImageAnimation()
{
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        ASSERT(iter->second != nullptr);
        iter->second->StopImageAnimation();
    }
}

void StateImage::PauseImageAnimation()
{
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        ASSERT(iter->second != nullptr);
        iter->second->PauseImageAnimation();
    }
}

Image* StateImage::FindImageByName(const DString& imageName) const
{
    if (imageName.empty()) {
        return nullptr;
    }
    for (auto iter = m_stateImageMap.begin(); iter != m_stateImageMap.end(); ++iter) {
        Image* pImage = iter->second;
        ASSERT(pImage != nullptr);
        if (pImage != nullptr) {
            if (pImage->GetImageAttribute().m_sImageName == imageName) {
                return pImage;
            }
        }
    }
    return nullptr;
}

}
