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

bool ImageDecoderFactory::IsVirtualImageFile(const DString& imageFileString) const
{
    for (std::shared_ptr<IImageDecoder> pImageDecoder : m_imageDecoders) {
        if (pImageDecoder != nullptr) {
            bool bVirtualFile = false;
            if (pImageDecoder->CanDecode(imageFileString, bVirtualFile)) {
                if (bVirtualFile) {
                    //该文件为虚拟文件名
                    return true;
                }
            }
        }
    }
    return false;
}

std::unique_ptr<IImage> ImageDecoderFactory::LoadImageData(const DString& imageFileString,
                                                           const uint8_t* data, size_t dataLen,
                                                           float fImageSizeScale,
                                                           const IImageDecoder::ExtraParam* pExtraParam)
{
    std::vector<uint8_t> imageData;
    if ((data != nullptr) && (dataLen > 0)) {
        imageData.resize(dataLen);
        memcpy(imageData.data(), data, dataLen);
    }
    return LoadImageData(imageFileString, imageData, fImageSizeScale, pExtraParam);
}

std::unique_ptr<IImage> ImageDecoderFactory::LoadImageData(const DString& imageFileString,
                                                           std::vector<uint8_t>& data,
                                                           float fImageSizeScale,
                                                           const IImageDecoder::ExtraParam* pExtraParam)
{
    bool bCanRetry = true;
    std::unique_ptr<IImage> pImageData;
    for (std::shared_ptr<IImageDecoder> pImageDecoder : m_imageDecoders) {
        if (pImageDecoder != nullptr) {
            if (data.empty()) {
                //按虚拟文件名处理，无实体文件数据
                bool bVirtualFile = false;
                if (pImageDecoder->CanDecode(imageFileString, bVirtualFile)) {
                    if (bVirtualFile) {
                        pImageData = pImageDecoder->LoadImageData(imageFileString, data, fImageSizeScale, pExtraParam);
                        break;
                    }
                }
            }
            else {
                bool bVirtualFile = false;
                if (pImageDecoder->CanDecode(imageFileString, bVirtualFile) &&
                    !bVirtualFile &&
                    pImageDecoder->CanDecode(data.data(), data.size())) {
                    pImageData = pImageDecoder->LoadImageData(imageFileString, data, fImageSizeScale, pExtraParam);
                    bCanRetry = false;
                    break;
                }
            }
        }
    }
    if ((pImageData == nullptr) && bCanRetry && !data.empty()) {
        //如果按扩展名无法匹配，则按图片数据流尝试加载(从而使得扩展名与文件格式不一致的情况下，也能正常加载图片)
        for (std::shared_ptr<IImageDecoder> pImageDecoder : m_imageDecoders) {
            if (pImageDecoder != nullptr) {
                if (pImageDecoder->CanDecode(data.data(), data.size())) {
                    pImageData = pImageDecoder->LoadImageData(imageFileString, data, fImageSizeScale, pExtraParam);
                    if (pImageData != nullptr) {
                        break;
                    }
                }
            }
        }
    }

    ///TEST
    if (pImageData == nullptr) {
        int32_t nHeight = 32;
        int32_t nWidth = 32;
        std::vector<uint8_t> data2;
        data2.resize(nHeight * nWidth * 4);
        pImageData = Image_Bitmap::MakeImage(nWidth, nHeight, data2.data(), fImageSizeScale);
    }
    ///TEST
    return pImageData;
}

std::shared_ptr<IBitmap> ImageDecoderFactory::DecodeImageData(const DString& imageFileString,
                                                              const uint8_t* data, size_t dataLen,
                                                              float fImageSizeScale,
                                                              const IImageDecoder::ExtraParam* pExtraParam)
{
    std::vector<uint8_t> imageData;
    if ((data != nullptr) && (dataLen > 0)) {
        imageData.resize(dataLen);
        memcpy(imageData.data(), data, dataLen);
    }
    return DecodeImageData(imageFileString, imageData, fImageSizeScale, pExtraParam);
}

std::shared_ptr<IBitmap> ImageDecoderFactory::DecodeImageData(const DString& imageFileString,
                                                              std::vector<uint8_t>& data,
                                                              float fImageSizeScale,
                                                              const IImageDecoder::ExtraParam* pExtraParam)
{
    std::shared_ptr<IBitmap> pBitmap;
    IImageDecoder::ExtraParam extraParam;
    if (pExtraParam != nullptr) {
        extraParam = *pExtraParam;
    }
    extraParam.m_bLoadAllFrames = false; //只加载单帧图片，不支持多帧
    std::shared_ptr<IImage> pImage = LoadImageData(imageFileString, data, fImageSizeScale, &extraParam);
    ASSERT(pImage != nullptr);
    if (pImage != nullptr) {
        ASSERT(pImage->GetImageType() == ImageType::kImageBitmap);
        if (pImage->GetImageType() == ImageType::kImageBitmap) {
            pBitmap = pImage->GetImageBitmap();
        }
    }
    return pBitmap;
}

} // namespace ui
