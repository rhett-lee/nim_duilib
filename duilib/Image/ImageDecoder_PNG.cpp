#include "ImageDecoder_PNG.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_PNG.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"
#include <cstring>

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

bool ImageDecoder_PNG::CanDecode(const DString& imageFilePath) const
{
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt == _T("PNG")) {
        return true;
    }
    return false;
}

bool ImageDecoder_PNG::CanDecode(const uint8_t* data, size_t dataLen) const
{
    //PNG/APNG格式签名
    static const uint8_t pngSignature[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    if ((data == nullptr) || (dataLen < sizeof(pngSignature))) {
        return false;
    }
    return memcmp(data, pngSignature, sizeof(pngSignature)) == 0;
}

std::unique_ptr<IImage> ImageDecoder_PNG::LoadImageData(const ImageDecodeParam& decodeParam)
{
    bool bLoadAllFrames = decodeParam.m_bLoadAllFrames;
    bool bAsyncDecode = decodeParam.m_bAsyncDecode;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    const UiSize& rcMaxDestRectSize = decodeParam.m_rcMaxDestRectSize;
    bool bAssertEnabled = decodeParam.m_bAssertEnabled;
    std::vector<uint8_t> emptyFileData;
    std::vector<uint8_t>& fileData = (decodeParam.m_pFileData != nullptr) ? *decodeParam.m_pFileData : emptyFileData;
    const FilePath& imageFilePath = decodeParam.m_imageFilePath;

    Image_PNG* pImagePNG = new Image_PNG;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImagePNG);

    if (!pImagePNG->LoadImageFile(fileData,
                                  imageFilePath,
                                  bLoadAllFrames,
                                  bAsyncDecode,
                                  fImageSizeScale,
                                  rcMaxDestRectSize,
                                  bAssertEnabled)) {
        return nullptr;
    }
    
    if (!bLoadAllFrames || (pImagePNG->GetFrameCount() == 1)) {
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
