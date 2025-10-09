#include "ImageDecoder_LOTTIE.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_LOTTIE.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui
{
ImageDecoder_LOTTIE::ImageDecoder_LOTTIE()
{
}

ImageDecoder_LOTTIE::~ImageDecoder_LOTTIE()
{
}

DString ImageDecoder_LOTTIE::GetFormatName() const
{
    return _T("LOTTIE-JSON");
}

bool ImageDecoder_LOTTIE::CanDecode(const DString& imageFilePath) const
{
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt == _T("JSON")) {
        return true;
    }
    return false;
}

bool ImageDecoder_LOTTIE::CanDecode(const uint8_t* data, size_t dataLen) const
{
    if ((data == nullptr) || (dataLen == 0)) {
        return false;
    }
    return true;
}

std::unique_ptr<IImage> ImageDecoder_LOTTIE::LoadImageData(const ImageDecodeParam& decodeParam)
{
    if ((decodeParam.m_pFileData == nullptr) || decodeParam.m_pFileData->empty()) {
        return nullptr;
    }
    std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
    bool bLoadAllFrames = decodeParam.m_bLoadAllFrames;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    const UiSize& rcMaxDestRectSize = decodeParam.m_rcMaxDestRectSize;
    Image_LOTTIE* pImageLOTTIE = new Image_LOTTIE;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImageLOTTIE);
    if (!pImageLOTTIE->LoadImageFromMemory(fileData,
                                           bLoadAllFrames,
                                           fImageSizeScale,
                                           rcMaxDestRectSize)) {
        //ASSERT(0);
        return nullptr;
    }
    if (!bLoadAllFrames || (pImageLOTTIE->GetFrameCount() == 1)) {
        //单帧，加载位图图片
        return Image_Bitmap::MakeImage(pAnimationImage);
    }
    else {
        //多帧图片
        std::unique_ptr<IImage> pImage(new Image_Animation(pAnimationImage));
        return pImage;
    }
}

} //namespace ui
