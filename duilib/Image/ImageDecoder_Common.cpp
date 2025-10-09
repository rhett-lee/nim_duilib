#include "ImageDecoder_Common.h"
#include "duilib/Image/ImageDecoderUtil.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{
ImageDecoder_Common::ImageDecoder_Common()
{
}

ImageDecoder_Common::~ImageDecoder_Common()
{
}

DString ImageDecoder_Common::GetFormatName() const
{
    return ImageDecoderUtil::GetSupportedFileExtentions();
}

bool ImageDecoder_Common::CanDecode(const DString& imageFilePath) const
{
    DString fileExtentions = ImageDecoderUtil::GetSupportedFileExtentions();
    if (fileExtentions.empty()) {
        return false;
    }
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);

    std::list<DString> fileExtList = StringUtil::Split(fileExtentions, _T(";"));
    for (DString& ext : fileExtList) {
        StringUtil::MakeUpperString(ext);
        if (fileExt == ext) {
            return true;
        }
    }    
    return false;
}

bool ImageDecoder_Common::CanDecode(const uint8_t* data, size_t dataLen) const
{
    return ImageDecoderUtil::CanDecode(data, dataLen);
}

std::unique_ptr<IImage> ImageDecoder_Common::LoadImageData(const ImageDecodeParam& decodeParam)
{
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    std::unique_ptr<IImage> pImage;
    UiImageData imageData;
    bool bLoaded = false;
    if ((decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty()) {
        std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
        bLoaded = ImageDecoderUtil::LoadImageFromMemory(fileData, imageData);
    }
    else if (!decodeParam.m_imageFilePath.IsEmpty()) {
        bLoaded = ImageDecoderUtil::LoadImageFromFile(decodeParam.m_imageFilePath, imageData);
    }
    else {
        ASSERT(0);
    }
    if (bLoaded) {
        int32_t nWidth = (int32_t)imageData.m_imageWidth;
        int32_t nHeight = (int32_t)imageData.m_imageHeight;
        float fNewScale = fImageSizeScale;
        if (!ImageUtil::GetBestImageScale(decodeParam.m_rcMaxDestRectSize, nWidth, nHeight, fImageSizeScale, fNewScale)) {
            fNewScale = fImageSizeScale;
        }
        pImage = Image_Bitmap::MakeImage(imageData.m_imageWidth, imageData.m_imageHeight, imageData.m_imageData.data(), fNewScale);
    }    
    return pImage;
}

} //namespace ui
