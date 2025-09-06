#include "ImageDecoder_Common.h"
#include "duilib/Image/ImageDecoderUtil.h"
#include "duilib/Image/Image_Bitmap.h"
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

bool ImageDecoder_Common::CanDecode(const DString& imageFileString, bool& bVirtualFile) const
{
    bVirtualFile = false;
    DString fileExtentions = ImageDecoderUtil::GetSupportedFileExtentions();
    if (fileExtentions.empty()) {
        return false;
    }
    DString fileExt = FilePathUtil::GetFileExtension(imageFileString);
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

std::unique_ptr<IImage> ImageDecoder_Common::LoadImageData(const DString& /*imageFileString*/,
                                                           std::vector<uint8_t>& data,
                                                           float fImageSizeScale,
                                                           const IImageDecoder::ExtraParam* /*pExtraParam*/)
{
    std::unique_ptr<IImage> pImage;
    UiImageData imageData;
    if (ImageDecoderUtil::LoadImageFromMemory(data, imageData)) {
        pImage = Image_Bitmap::MakeImage(imageData.m_imageWidth, imageData.m_imageHeight, imageData.m_imageData.data(), fImageSizeScale);
    }    
    return pImage;
}

} //namespace ui
