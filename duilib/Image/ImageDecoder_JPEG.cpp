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
    bool bAsyncDecode = decodeParam.m_bAsyncDecode;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    const UiSize& rcMaxDestRectSize = decodeParam.m_rcMaxDestRectSize;
    Image_JPEG* pImageJPEG = new Image_JPEG;
    std::shared_ptr<IBitmapImage> pImage(pImageJPEG);

    if ((decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty()) {
        std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
        if (pImageJPEG->LoadImageFromMemory(fileData,
                                            fImageSizeScale,
                                            bAsyncDecode,
                                            rcMaxDestRectSize)) {
            return Image_Bitmap::MakeImage(pImage);
        }
    }
    else if (!decodeParam.m_imageFilePath.IsEmpty()) {
        if (pImageJPEG->LoadImageFromFile(decodeParam.m_imageFilePath,
                                          fImageSizeScale,
                                          bAsyncDecode,
                                          rcMaxDestRectSize)) {
            return Image_Bitmap::MakeImage(pImage);
        }
    }
    else {
        ASSERT(0);
    }    
    return nullptr;
}

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_JPEG_TURBO
