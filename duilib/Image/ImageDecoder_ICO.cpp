#include "ImageDecoder_ICO.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/ImageDecoder.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Image/ImageUtil.h"

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

    std::vector<ImageDecoder::ImageData> imageData;
    uint32_t nOutFrameCount = 0;
    bool isIconFile = true;
    uint32_t iconSize = 32;
    bool bLoadAllFrames = false;
    if (pExtraParam != nullptr) {
        iconSize = pExtraParam->m_nIconSize;
    }
    if (iconSize == 0) {
        iconSize = 32;
    }
    //计算期望大小
    iconSize = ImageUtil::GetScaledImageSize(iconSize, fImageSizeScale);
    bool bLoaded = CxImageLoader::LoadImageFromMemory(data, imageData, isIconFile, iconSize, bLoadAllFrames, nOutFrameCount);
    if (bLoaded) {
        ASSERT(imageData.size() == 1);        
        if (imageData.size() == 1) {
            ImageDecoder::ImageData& bitmapData = imageData[0];
            ASSERT(bitmapData.m_imageHeight > 0);
            ASSERT(bitmapData.m_imageWidth > 0);
            ASSERT(bitmapData.m_imageWidth == bitmapData.m_imageHeight);
            ASSERT(bitmapData.m_bitmapData.size() == bitmapData.m_imageHeight* bitmapData.m_imageWidth*4);
            if ((bitmapData.m_imageHeight > 0) && (bitmapData.m_imageWidth > 0) &&
                (bitmapData.m_bitmapData.size() == bitmapData.m_imageHeight * bitmapData.m_imageWidth * 4)) {
                //CxImage加载的数据，需要翻转，以屏幕左上角为顶点
                ImageUtil::FlipPixelBits(bitmapData.m_bitmapData.data(), bitmapData.m_bitmapData.size(), bitmapData.m_imageWidth, bitmapData.m_imageHeight);
                float fNewImageSizeScale = static_cast<float>(iconSize) / bitmapData.m_imageWidth;
                pImage = Image_Bitmap::MakeImage(bitmapData.m_imageWidth, bitmapData.m_imageHeight, bitmapData.m_bitmapData.data(), fNewImageSizeScale);
            }
        }
    }
    return pImage;
}

} //namespace ui
