#include "ImageDecoder_GIF.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_GIF.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"
#include <cstring>

namespace ui
{
ImageDecoder_GIF::ImageDecoder_GIF()
{
}

ImageDecoder_GIF::~ImageDecoder_GIF()
{
}

DString ImageDecoder_GIF::GetFormatName() const
{
    return _T("GIF");
}

bool ImageDecoder_GIF::CanDecode(const DString& imageFilePath) const
{
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt == _T("GIF")) {
        return true;
    }
    return false;
}

bool ImageDecoder_GIF::CanDecode(const uint8_t* data, size_t dataLen) const
{
    //GIF87a/GIF89a格式签名
    static const uint8_t gifSignature1[] = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 };
    static const uint8_t gifSignature2[] = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 };
    if ((data == nullptr) || (dataLen < sizeof(gifSignature1))) {
        return false;
    }
    return (memcmp(data, gifSignature1, sizeof(gifSignature1)) == 0) ||
           (memcmp(data, gifSignature2, sizeof(gifSignature2)) == 0);
}

std::unique_ptr<IImage> ImageDecoder_GIF::LoadImageData(const ImageDecodeParam& decodeParam)
{
    bool bLoadAllFrames = decodeParam.m_bLoadAllFrames;
    bool bAsyncDecode = decodeParam.m_bAsyncDecode;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    const UiSize& rcMaxDestRectSize = decodeParam.m_rcMaxDestRectSize;
    bool bAssertEnabled = decodeParam.m_bAssertEnabled;
    std::vector<uint8_t> emptyFileData;
    std::vector<uint8_t>& fileData = (decodeParam.m_pFileData != nullptr) ? *decodeParam.m_pFileData : emptyFileData;
    const FilePath& imageFilePath = decodeParam.m_imageFilePath;

    Image_GIF* pImageGIF = new Image_GIF;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImageGIF);

    if (!pImageGIF->LoadImageFile(fileData,
                                  imageFilePath,
                                  bLoadAllFrames,
                                  bAsyncDecode,
                                  fImageSizeScale,
                                  rcMaxDestRectSize,
                                  bAssertEnabled)) {
            return nullptr;
    }    
    if (!bLoadAllFrames || (pImageGIF->GetFrameCount() == 1)) {
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
