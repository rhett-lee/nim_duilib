#include "ImageDecoder_GIF.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_GIF.h"
#include "duilib/Image/Image_Animation.h"
#include "duilib/Utils/FilePathUtil.h"

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
    std::vector<uint8_t> gifSignature1 = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 }; // GIF87a
    std::vector<uint8_t> gifSignature2 = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 }; // GIF89a
    //GIF格式签名
    bool bGIF87a = false;
    bool bGIF89a = false;
    if ((dataLen > gifSignature1.size()) && (data != nullptr)) {
        bGIF87a = true;
        for (size_t i = 0; i < gifSignature1.size(); ++i) {
            if (data[i] != gifSignature1[i]) {
                bGIF87a = false;
                break;
            }
        }
    }
    if (!bGIF87a) {
        if ((dataLen > gifSignature2.size()) && (data != nullptr)) {
            bGIF89a = true;
            for (size_t i = 0; i < gifSignature2.size(); ++i) {
                if (data[i] != gifSignature2[i]) {
                    bGIF89a = false;
                    break;
                }
            }
        }
    }
    return bGIF87a || bGIF89a;
}

std::unique_ptr<IImage> ImageDecoder_GIF::LoadImageData(const ImageDecodeParam& decodeParam)
{
    bool bLoadAllFrames = decodeParam.m_bLoadAllFrames;
    bool bAsyncDecode = decodeParam.m_bAsyncDecode;
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    const UiSize& rcMaxDestRectSize = decodeParam.m_rcMaxDestRectSize;
    Image_GIF* pImageGIF = new Image_GIF;
    std::shared_ptr<IAnimationImage> pAnimationImage(pImageGIF);

    if ((decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty()) {
        std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
        if (!pImageGIF->LoadImageFromMemory(fileData,
                                            bLoadAllFrames,
                                            bAsyncDecode,
                                            fImageSizeScale,
                                            rcMaxDestRectSize)) {
            return nullptr;
        }
    }
    else if (!decodeParam.m_imageFilePath.IsEmpty()) {
        if (!pImageGIF->LoadImageFromFile(decodeParam.m_imageFilePath,
                                          bLoadAllFrames,
                                          bAsyncDecode,
                                          fImageSizeScale,
                                          rcMaxDestRectSize)) {
            return nullptr;
        }
    }
    else {
        ASSERT(0);
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
