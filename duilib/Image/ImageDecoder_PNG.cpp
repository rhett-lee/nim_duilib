#include "ImageDecoder_PNG.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_PNG.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"

namespace ui
{
ImageDecoder_PNG::ImageDecoder_PNG()
{
}

ImageDecoder_PNG::~ImageDecoder_PNG()
{
}

DString ImageDecoder_PNG::GetFormatName() const
{
    return _T("PNG/APNG");
}

bool ImageDecoder_PNG::CanDecode(const DString& imageFileString, bool& bVirtualFile) const
{
    bVirtualFile = false;
    DString fileExt = FilePathUtil::GetFileExtension(imageFileString);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt == _T("PNG")) {
        return true;
    }
    return false;
}

bool ImageDecoder_PNG::CanDecode(const uint8_t* data, size_t dataLen) const
{
    //PNG/APNG格式签名
    std::vector<uint8_t> pngSignature = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };    
    bool bPNG = false;
    if ((dataLen > pngSignature.size()) && (data != nullptr)) {
        bPNG = true;
        for (size_t i = 0; i < pngSignature.size(); ++i) {
            if (data[i] != pngSignature[i]) {
                bPNG = false;
                break;
            }
        }
    }
    return bPNG;
}

std::unique_ptr<IImage> ImageDecoder_PNG::LoadImageData(const DString& /*imageFileString*/,
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
    Image_PNG* pImagePNG = new Image_PNG;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImagePNG);
    if (!pImagePNG->LoadImageFromMemory(data, bLoadAllFrames, fImageSizeScale)) {
        ASSERT(0);
        return nullptr;
    }
    if (!bLoadAllFrames || (pImagePNG->GetFrameCount() == 1)) {
        //单帧，加载位图图片
        IAnimationImage::AnimationFrame frame;
        if (pImagePNG->ReadFrameData(0, &frame)) {
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
