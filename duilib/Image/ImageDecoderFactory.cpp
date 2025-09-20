#include "ImageDecoderFactory.h"

/// TEST
#include "duilib/Image/Image_Bitmap.h"
/// TEST

namespace ui 
{
ImageDecoderFactory::ImageDecoderFactory()
{
}

ImageDecoderFactory::~ImageDecoderFactory()
{
}

bool ImageDecoderFactory::AddImageDecoder(const std::shared_ptr<IImageDecoder>& pImageDecoder)
{
    ASSERT(pImageDecoder != nullptr);
    if (pImageDecoder == nullptr) {
        return false;
    }
    auto iter = std::find(m_imageDecoders.begin(), m_imageDecoders.end(), pImageDecoder);
    if (iter == m_imageDecoders.end()) {
        m_imageDecoders.push_back(pImageDecoder);
    }
    return false;
}

bool ImageDecoderFactory::RemoveImageDecoder(const std::shared_ptr<IImageDecoder>& pImageDecoder)
{
    ASSERT(pImageDecoder != nullptr);
    if (pImageDecoder == nullptr) {
        return false;
    }
    auto iter = std::find(m_imageDecoders.begin(), m_imageDecoders.end(), pImageDecoder);
    if (iter != m_imageDecoders.end()) {
        m_imageDecoders.erase(iter);
        return true;
    }
    return false;
}

void ImageDecoderFactory::Clear()
{
    m_imageDecoders.clear();
}

std::unique_ptr<IImage> ImageDecoderFactory::LoadImageData(const ImageDecodeParam& decodeParam)
{
    const bool bHasFileData = (decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty(); //图片文件数据
    const DString& imageFilePath = decodeParam.m_imagePath; //图片文件路径
    ASSERT(!imageFilePath.empty() || bHasFileData);
    if (imageFilePath.empty() && !bHasFileData) {
        return nullptr;
    }

    //文件头数据，用做图片格式的签名校验
    const std::vector<uint8_t>& signatureData = bHasFileData ? *decodeParam.m_pFileData : decodeParam.m_fileHeaderData; 
    std::vector<std::shared_ptr<IImageDecoder>> untriedDecoders;//未尝试的解码器

    std::unique_ptr<IImage> pImageData;
    for (std::shared_ptr<IImageDecoder> pImageDecoder : m_imageDecoders) {
        ASSERT(pImageDecoder != nullptr);
        if (pImageDecoder == nullptr) {
            continue;
        }
        bool bCanDecode = false;
        if (!imageFilePath.empty()) {
            //文件名不为空, 优先按文件后缀匹配解码器
            if (pImageDecoder->CanDecode(imageFilePath)) {
                if (signatureData.empty()) {
                    bCanDecode = true;
                }
                else if (pImageDecoder->CanDecode(signatureData.data(), signatureData.size())) {                    
                    bCanDecode = true;
                }
            }
        }
        else if (!signatureData.empty()) {
            //文件名为空，按文件数据签名匹配解码器
            if (pImageDecoder->CanDecode(signatureData.data(), signatureData.size())) {
                bCanDecode = true;
            }
        }
        if (bCanDecode) {
            pImageData = pImageDecoder->LoadImageData(decodeParam);
            if (pImageData != nullptr) {
                break;
            }
        }
        else {
            untriedDecoders.push_back(pImageDecoder);
        }
    }
    if ((pImageData == nullptr) && !signatureData.empty() && !untriedDecoders.empty()) {
        //如果按扩展名无法匹配，则按图片数据流尝试加载(从而使得扩展名与文件格式不一致的情况下，也能正常加载图片)
        for (std::shared_ptr<IImageDecoder> pImageDecoder : untriedDecoders) {
            if (pImageDecoder->CanDecode(signatureData.data(), signatureData.size())) {
                pImageData = pImageDecoder->LoadImageData(decodeParam);
                if (pImageData != nullptr) {
                    break;
                }
            }
        }
    }
    ASSERT(pImageData != nullptr);
    return pImageData;
}

std::shared_ptr<IBitmap> ImageDecoderFactory::DecodeImageData(const ImageDecodeParam& decodeParam)
{
    std::shared_ptr<IBitmap> pBitmap;
    ImageDecodeParam newDecodeParam = decodeParam;
    newDecodeParam.m_bLoadAllFrames = false;//只加载单帧图片，不支持多帧
    newDecodeParam.m_bAsyncDecode = false;  //不支持异步线程解码
    std::shared_ptr<IImage> pImage = LoadImageData(newDecodeParam);
    ASSERT(pImage != nullptr);
    if (pImage != nullptr) {
        ASSERT(pImage->GetImageType() == ImageType::kImageBitmap);
        if (pImage->GetImageType() == ImageType::kImageBitmap) {
            std::shared_ptr<IBitmapImage> pBitmapImage = pImage->GetImageBitmap();
            ASSERT(pBitmapImage != nullptr);
            if (pBitmapImage != nullptr) {
                pBitmap = pBitmapImage->GetBitmap();
            }
        }
    }
    return pBitmap;
}

} // namespace ui
