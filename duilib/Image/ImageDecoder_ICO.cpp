#include "ImageDecoder_ICO.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/ImageDecoderUtil.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Image/Image_ICO.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui
{
ImageDecoder_ICO::ImageDecoder_ICO()
{
}

ImageDecoder_ICO::~ImageDecoder_ICO()
{
}

DString ImageDecoder_ICO::GetFormatName() const
{
    return _T("ICO/CUR");
}

bool ImageDecoder_ICO::CanDecode(const DString& imageFileString, bool& bVirtualFile) const
{
    bVirtualFile = false;
    DString fileExt = FilePathUtil::GetFileExtension(imageFileString);
    StringUtil::MakeUpperString(fileExt);
    if ((fileExt == _T("ICO")) || (fileExt == _T("CUR"))) {
        return true;
    }
    return false;
}

bool ImageDecoder_ICO::CanDecode(const uint8_t* data, size_t dataLen) const
{
    //ICO格式签名
    bool bICO = false;
    bool bCur = false;
    std::vector<uint8_t> icoSignature = { 0x00, 0x00, 0x01, 0x00 };
    if ((dataLen > icoSignature.size()) && (data != nullptr)) {
        bICO = true;
        for (size_t i = 0; i < icoSignature.size(); ++i) {
            if (data[i] != icoSignature[i]) {
                bICO = false;
                break;
            }
        }
    }
    if (!bICO) {
        //CUR格式签名
        std::vector<uint8_t> curSignature = { 0x00, 0x00, 0x02, 0x00 };
        if ((dataLen > curSignature.size()) && (data != nullptr)) {
            bCur = true;
            for (size_t i = 0; i < curSignature.size(); ++i) {
                if (data[i] != curSignature[i]) {
                    bCur = false;
                    break;
                }
            }
        }
    }
    return bICO || bCur;
}

std::unique_ptr<IImage> ImageDecoder_ICO::LoadImageData(const DString& /*imageFileString*/,
                                                        std::vector<uint8_t>& data,
                                                        float fImageSizeScale,
                                                        const IImageDecoder::ExtraParam* pExtraParam)
{
    std::unique_ptr<IImage> pImage;
    if (data.empty()) {
        return pImage;
    }

    std::vector<UiImageData> imageData;
    uint32_t nIconSize = 0;
    int32_t nFrameDelayMs = 1000; // 每帧的时间间隔，毫秒
    bool bIconAsAnimation = false;
    if (pExtraParam != nullptr) {
        nIconSize = pExtraParam->m_nIconSize;
        bIconAsAnimation = pExtraParam->m_bIconAsAnimation && pExtraParam->m_bLoadAllFrames;
        nFrameDelayMs = pExtraParam->m_nIconFrameDelayMs;
    }
    if (nIconSize == 0) {
        nIconSize = bIconAsAnimation ? 48 : 32;
    }
    //计算期望大小
    const uint32_t nIconSizeScaled = ImageUtil::GetScaledImageSize(nIconSize, fImageSizeScale);
    bool bLoaded = ImageDecoderUtil::LoadIcoFromMemory(data, bIconAsAnimation, nIconSizeScaled, imageData);
    if (bLoaded) {
        ASSERT(!imageData.empty());        
        if (imageData.size() == 1) {
            UiImageData& bitmapData = imageData[0];
            ASSERT(bitmapData.m_imageHeight > 0);
            ASSERT(bitmapData.m_imageWidth > 0);
            ASSERT(bitmapData.m_imageWidth == bitmapData.m_imageHeight);
            ASSERT(bitmapData.m_imageData.size() == bitmapData.m_imageHeight* bitmapData.m_imageWidth*4);
            if ((bitmapData.m_imageHeight > 0) && (bitmapData.m_imageWidth > 0) &&
                (bitmapData.m_imageData.size() == bitmapData.m_imageHeight * bitmapData.m_imageWidth * 4)) {
                float fNewImageSizeScale = static_cast<float>(nIconSizeScaled) / bitmapData.m_imageWidth;
                pImage = Image_Bitmap::MakeImage(bitmapData.m_imageWidth, bitmapData.m_imageHeight, bitmapData.m_imageData.data(), fNewImageSizeScale);
            }
        }
        else {
            Image_ICO* pImageICO = new Image_ICO;
            std::shared_ptr<IAnimationImage> pAnimationImage(pImageICO);
            if (pImageICO->LoadImageFromMemory(imageData, fImageSizeScale, nIconSize, nFrameDelayMs)) {
                pImage.reset(new Image_Animation(pAnimationImage, fImageSizeScale));
            }
        }
    }
    return pImage;
}

} //namespace ui
