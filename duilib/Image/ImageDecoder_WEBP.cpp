#include "ImageDecoder_WEBP.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_WEBP.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui
{
ImageDecoder_WEBP::ImageDecoder_WEBP()
{
}

ImageDecoder_WEBP::~ImageDecoder_WEBP()
{
}

DString ImageDecoder_WEBP::GetFormatName() const
{
    return _T("WEBP");
}

bool ImageDecoder_WEBP::CanDecode(const DString& imageFilePath) const
{
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt == _T("WEBP")) {
        return true;
    }
    return false;
}

bool ImageDecoder_WEBP::CanDecode(const uint8_t* data, size_t dataLen) const
{
    //WEBP格式签名
    //std::vector<uint8_t> webpSignature = { 0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x45, 0x42, 0x50 };
    if (dataLen >= 12 && (data != nullptr) &&
        data[0] == 0x52 && data[1] == 0x49 && data[2] == 0x46 && data[3] == 0x46 &&
        data[8] == 0x57 && data[9] == 0x45 && data[10] == 0x42 && data[11] == 0x50) {
        return true;
    }
    return false;
}

std::unique_ptr<IImage> ImageDecoder_WEBP::LoadImageData(const ImageDecodeParam& decodeParam)
{
    if ((decodeParam.m_pFileData == nullptr) || decodeParam.m_pFileData->empty()) {
        return nullptr;
    }
    std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
    bool bLoadAllFrames = decodeParam.m_bLoadAllFrames;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    Image_WEBP* pImageWEBP = new Image_WEBP;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImageWEBP);
    if (!pImageWEBP->LoadImageFromMemory(fileData, bLoadAllFrames, decodeParam.m_bAsyncDecode, fImageSizeScale)) {
        ASSERT(0);
        return nullptr;
    }
    if (!bLoadAllFrames || (pImageWEBP->GetFrameCount() == 1)) {
        //单帧，加载位图图片
        return Image_Bitmap::MakeImage(pAnimationImage, fImageSizeScale);
    }
    else {
        //多帧图片
        std::unique_ptr<IImage> pImage(new Image_Animation(pAnimationImage, fImageSizeScale));
        return pImage;
    }
}

} //namespace ui
