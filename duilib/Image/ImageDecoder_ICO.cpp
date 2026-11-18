#include "ImageDecoder_ICO.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_DelayBitmap.h"
#include "duilib/Image/ImageDecoderUtil.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Image/Image_ICO.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"
#include <cstring>

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

bool ImageDecoder_ICO::CanDecode(const DString& imageFilePath) const
{
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);
    if ((fileExt == _T("ICO")) || (fileExt == _T("CUR"))) {
        return true;
    }
    return false;
}

bool ImageDecoder_ICO::CanDecode(const uint8_t* data, size_t dataLen) const
{
    //ICO/CUR格式签名
    static const uint8_t icoSignature[] = { 0x00, 0x00, 0x01, 0x00 };
    static const uint8_t curSignature[] = { 0x00, 0x00, 0x02, 0x00 };
    if ((data == nullptr) || (dataLen < sizeof(icoSignature))) {
        return false;
    }
    return (memcmp(data, icoSignature, sizeof(icoSignature)) == 0) ||
           (memcmp(data, curSignature, sizeof(curSignature)) == 0);
}

std::unique_ptr<IImage> ImageDecoder_ICO::LoadImageData(const ImageDecodeParam& decodeParam)
{
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    uint32_t nIconSize = decodeParam.m_nIconSize;
    int32_t nFrameDelayMs = decodeParam.m_nIconFrameDelayMs; // 每帧的时间间隔，毫秒
    bool bIconAsAnimation = decodeParam.m_bIconAsAnimation;
    if (nIconSize == 0) {
        nIconSize = bIconAsAnimation ? 48 : 32;
    }

    std::unique_ptr<IImage> pImage;
    std::vector<UiImageData> imageData;
    //计算期望大小
    const uint32_t nIconSizeScaled = ImageUtil::GetScaledImageSize(nIconSize, fImageSizeScale);

    if (decodeParam.m_bAsyncDecode && !bIconAsAnimation && (nIconSizeScaled > 0)) {
        //异步解码：加载阶段只解析ICO文件的目录信息，像素解码放到解码线程中执行
        pImage = LoadImageDataAsync(decodeParam, nIconSizeScaled);
        if (pImage != nullptr) {
            return pImage;
        }
        //解析失败时，回退到同步解码
    }

    bool bLoaded = false;
    if ((decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty()) {
        std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
        bLoaded = ImageDecoderUtil::LoadIcoFromMemory(fileData, bIconAsAnimation, nIconSizeScaled, imageData);
    }
    else if (!decodeParam.m_imageFilePath.IsEmpty()) {
        bLoaded = ImageDecoderUtil::LoadIcoFromFile(decodeParam.m_imageFilePath, bIconAsAnimation, nIconSizeScaled, imageData);
    }
    else {
        ASSERT(0);
    }
    
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
                pImage.reset(new Image_Animation(pAnimationImage));
            }
        }
    }
    return pImage;
}

std::unique_ptr<IImage> ImageDecoder_ICO::LoadImageDataAsync(const ImageDecodeParam& decodeParam,
                                                            uint32_t nIconSizeScaled)
{
    //获取ICO文件的数据（文件路径的情况下，需要读入到内存，用于解析目录信息）
    std::shared_ptr<std::vector<uint8_t>> pFileData;
    if ((decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty()) {
        pFileData = decodeParam.m_pFileData;
    }
    else if (!decodeParam.m_imageFilePath.IsEmpty()) {
        pFileData = std::make_shared<std::vector<uint8_t>>();
        FileUtil::ReadFileData(decodeParam.m_imageFilePath, *pFileData);
        if (pFileData->empty()) {
            pFileData.reset();
        }
    }
    if (pFileData == nullptr) {
        return nullptr;
    }

    //只解析目录信息，选择需要显示的图标项
    std::vector<ImageDecoderUtil::IcoImageInfo> iconInfoList;
    if (!ImageDecoderUtil::GetIcoImageInfo(*pFileData, iconInfoList)) {
        return nullptr;
    }
    const int32_t nIconIndex = ImageDecoderUtil::SelectIcoImageInfo(iconInfoList, nIconSizeScaled);
    if ((nIconIndex < 0) || (nIconIndex >= (int32_t)iconInfoList.size())) {
        return nullptr;
    }
    const ImageDecoderUtil::IcoImageInfo& iconInfo = iconInfoList[(size_t)nIconIndex];
    if ((iconInfo.m_nWidth == 0) || (iconInfo.m_nHeight == 0)) {
        return nullptr;
    }

    //计算图片的缩放比例（与原实现保持一致）
    const float fImageSizeScale = static_cast<float>(nIconSizeScaled) / iconInfo.m_nWidth;
    std::shared_ptr<Image_DelayBitmap> pDelayBitmap = std::make_shared<Image_DelayBitmap>();
    Image_DelayBitmap::DecodeFunction decodeFunction =
        [pFileData, nIconSizeScaled](UiImageData& imageData) -> bool {
            std::vector<UiImageData> imageDataList;
            if (!ImageDecoderUtil::LoadIcoFromMemory(*pFileData, false, nIconSizeScaled, imageDataList) ||
                imageDataList.empty()) {
                return false;
            }
            imageData = std::move(imageDataList[0]);
            return true;
        };
    if (!pDelayBitmap->InitImage(iconInfo.m_nWidth, iconInfo.m_nHeight, fImageSizeScale, true, decodeFunction)) {
        return nullptr;
    }
    return Image_Bitmap::MakeImage(pDelayBitmap);
}

} //namespace ui
