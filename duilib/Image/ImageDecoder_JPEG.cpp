#include "ImageDecoder_JPEG.h"
#include "duilib/Image/Image_JPEG.h"
#include "duilib/Image/Image_Bitmap.h"
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

bool ImageDecoder_JPEG::CanDecode(const DString& imageFilePath) const
{
    DString fileExtentions = _T("JPG;JPEG;JPE;JIF;JFIF;JFI");
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

std::unique_ptr<IImage> ImageDecoder_JPEG::LoadImageData(const ImageDecodeParam& decodeParam)
{
    if ((decodeParam.m_pFileData == nullptr) || decodeParam.m_pFileData->empty()) {
        return nullptr;
    }
    std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    Image_JPEG* pImageJPEG = new Image_JPEG;
    std::shared_ptr<IBitmapImage> pImage(pImageJPEG);
    if (pImageJPEG->LoadImageData(fileData, fImageSizeScale, decodeParam.m_bAsyncDecode)) {
        return Image_Bitmap::MakeImage(pImage);        
    }
    return nullptr;
}

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_JPEG_TURBO
