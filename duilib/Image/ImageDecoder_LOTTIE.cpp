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
    //LOTTIE是JSON格式：跳过UTF-8 BOM和空白字符后，第一个有效字符应该是'{'
    //这样可避免把其他格式(如BMP)的图片数据误判为LOTTIE格式，减少无用的解码尝试
    size_t nPos = 0;
    if ((dataLen >= 3) && (data[0] == 0xEF) && (data[1] == 0xBB) && (data[2] == 0xBF)) {
        nPos = 3;
    }
    while ((nPos < dataLen) &&
           ((data[nPos] == ' ') || (data[nPos] == '\t') || (data[nPos] == '\r') || (data[nPos] == '\n'))) {
        ++nPos;
    }
    return (nPos < dataLen) && (data[nPos] == '{');
}

std::unique_ptr<IImage> ImageDecoder_LOTTIE::LoadImageData(const ImageDecodeParam& decodeParam)
{
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    const UiSize& rcMaxDestRectSize = decodeParam.m_rcMaxDestRectSize;
    bool bAssertEnabled = decodeParam.m_bAssertEnabled;
    std::vector<uint8_t> emptyFileData;
    std::vector<uint8_t>& fileData = (decodeParam.m_pFileData != nullptr) ? *decodeParam.m_pFileData : emptyFileData;
    const FilePath& imageFilePath = decodeParam.m_imageFilePath;

    Image_LOTTIE* pImageLOTTIE = new Image_LOTTIE;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImageLOTTIE);

    if (!pImageLOTTIE->LoadImageFile(fileData,
                                     imageFilePath,
                                     fImageSizeScale,
                                     rcMaxDestRectSize,
                                     bAssertEnabled)) {
        return nullptr;
    }
    
    if (!decodeParam.m_bLoadAllFrames || (pImageLOTTIE->GetFrameCount() == 1)) {
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
