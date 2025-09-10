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
    return _T("LOTTIE");
}

bool ImageDecoder_LOTTIE::CanDecode(const DString& imageFileString, bool& bVirtualFile) const
{
    bVirtualFile = false;
    DString fileExt = FilePathUtil::GetFileExtension(imageFileString);
    StringUtil::MakeUpperString(fileExt);
    if ((fileExt == _T("JSON")) && (fileExt == _T("LOTTIE"))) {
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

std::unique_ptr<IImage> ImageDecoder_LOTTIE::LoadImageData(const DString& /*imageFileString*/,
                                                           std::vector<uint8_t>& data,
                                                           float fImageSizeScale,
                                                           const IImageDecoder::ExtraParam* pExtraParam)
{
    if (data.empty()) {
        return nullptr;
    }
    bool bLoadAllFrames = true;
    if (pExtraParam) {
        bLoadAllFrames = pExtraParam->m_bLoadAllFrames;
    }
    Image_LOTTIE* pImageLOTTIE = new Image_LOTTIE;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImageLOTTIE);
    if (!pImageLOTTIE->LoadImageFromMemory(data, bLoadAllFrames, fImageSizeScale)) {
        ASSERT(0);
        return nullptr;
    }
    if (!bLoadAllFrames || (pImageLOTTIE->GetFrameCount() == 1)) {
        //单帧，加载位图图片
        IAnimationImage::AnimationFrame frame;
        if (pImageLOTTIE->ReadFrame(0, &frame)) {
            return Image_Bitmap::MakeImage(frame.m_pBitmap, fImageSizeScale);
        }
        else {
            ASSERT(0);
            return nullptr;
        }
    }
    else {
        //多帧图片
        std::unique_ptr<IImage> pImage(new Image_Animation(pAnimationImage, fImageSizeScale));
        return pImage;
    }
}

} //namespace ui
