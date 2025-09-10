#include "ImageDecoder_JPEG.h"
#include "duilib/Image/Image_JPEG.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/StringUtil.h"

#ifdef DUILIB_IMAGE_SUPPORT_JPEG_TURBO

namespace ui
{
ImageDecoder_JPEG::ImageDecoder_JPEG()
{
}

ImageDecoder_JPEG::~ImageDecoder_JPEG()
{
}

DString ImageDecoder_JPEG::GetFormatName() const
{
    return _T("JPEG");
}

bool ImageDecoder_JPEG::CanDecode(const DString& imageFileString, bool& bVirtualFile) const
{
    bVirtualFile = false;
    DString fileExtentions = _T("JPG;JPEG;JPE;JIF;JFIF;JFI");
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

bool ImageDecoder_JPEG::CanDecode(const uint8_t* data, size_t dataLen) const
{
    //JPEG格式签名
    std::vector<uint8_t> jpegSignature = { 0xFF, 0xD8, 0xFF };
    bool bJPEG = false;
    if ((dataLen > jpegSignature.size()) && (data != nullptr)) {
        bJPEG = true;
        for (size_t i = 0; i < jpegSignature.size(); ++i) {
            if (data[i] != jpegSignature[i]) {
                bJPEG = false;
                break;
            }
        }
    }
    return bJPEG;
}

std::unique_ptr<IImage> ImageDecoder_JPEG::LoadImageData(const DString& /*imageFileString*/,
                                                           std::vector<uint8_t>& data,
                                                           float fImageSizeScale,
                                                           const IImageDecoder::ExtraParam* /*pExtraParam*/)
{
    ASSERT(!data.empty());
    if (data.empty()) {
        return nullptr;
    }
    Image_JPEG* pImageJPEG = new Image_JPEG;
    std::unique_ptr<IImage> pImage(pImageJPEG);
    if (!pImageJPEG->LoadImageData(data, fImageSizeScale)) {
        pImage.reset();
    }     
    return pImage;
}

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_JPEG_TURBO
