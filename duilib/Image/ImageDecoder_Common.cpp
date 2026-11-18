#include "ImageDecoder_Common.h"
#include "duilib/Image/ImageDecoderUtil.h"
#include "duilib/Image/Image_Bitmap.h"
#include "duilib/Image/Image_DelayBitmap.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/StringUtil.h"
#include <climits>

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

bool ImageDecoder_Common::CanDecode(const DString& imageFilePath) const
{
    const DString fileExtentions = ImageDecoderUtil::GetSupportedFileExtentions();
    if (fileExtentions.empty()) {
        return false;
    }
    DString fileExt = FilePathUtil::GetFileExtension(imageFilePath);
    StringUtil::MakeUpperString(fileExt);
    if (fileExt.empty()) {
        return false;
    }

    //直接扫描分号分隔的后缀列表逐个比较，避免每次调用都拆分字符串生成临时容器
    const DString::value_type* pszBegin = fileExtentions.c_str();
    while (*pszBegin != 0) {
        const DString::value_type* pszEnd = pszBegin;
        while ((*pszEnd != 0) && (*pszEnd != _T(';'))) {
            ++pszEnd;
        }
        DString ext(pszBegin, (size_t)(pszEnd - pszBegin));
        StringUtil::MakeUpperString(ext);
        if (fileExt == ext) {
            return true;
        }
        pszBegin = (*pszEnd == _T(';')) ? (pszEnd + 1) : pszEnd;
    }
    return false;
}

bool ImageDecoder_Common::CanDecode(const uint8_t* data, size_t dataLen) const
{
    return ImageDecoderUtil::CanDecode(data, dataLen);
}

std::unique_ptr<IImage> ImageDecoder_Common::LoadImageData(const ImageDecodeParam& decodeParam)
{
    float fImageSizeScale = decodeParam.m_fImageSizeScale;
    std::unique_ptr<IImage> pImage;
    if (decodeParam.m_bAsyncDecode) {
        //异步解码：加载阶段只解析图片的宽高，像素解码放到解码线程中执行
        pImage = LoadImageDataAsync(decodeParam);
        if (pImage != nullptr) {
            return pImage;
        }
        //解析图片信息失败时，回退到同步解码
    }
    UiImageData imageData;
    bool bLoaded = false;
    if ((decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty()) {
        std::vector<uint8_t>& fileData = *decodeParam.m_pFileData;
        bLoaded = ImageDecoderUtil::LoadImageFromMemory(fileData, imageData);
    }
    else if (!decodeParam.m_imageFilePath.IsEmpty()) {
        bLoaded = ImageDecoderUtil::LoadImageFromFile(decodeParam.m_imageFilePath, imageData);
    }
    else {
        ASSERT(0);
    }
    if (bLoaded) {
        // 防止 uint32_t → int32_t 截断为负数
        if ((imageData.m_imageWidth == 0) || (imageData.m_imageHeight == 0) ||
            (imageData.m_imageWidth > (uint32_t)INT32_MAX) || (imageData.m_imageHeight > (uint32_t)INT32_MAX)) {
            ASSERT(!"ImageDecoder_Common: invalid image dimensions");
            return nullptr;
        }
        int32_t nWidth = (int32_t)imageData.m_imageWidth;
        int32_t nHeight = (int32_t)imageData.m_imageHeight;
        float fNewScale = fImageSizeScale;
        if (!ImageUtil::GetBestImageScale(decodeParam.m_rcMaxDestRectSize, nWidth, nHeight, fImageSizeScale, fNewScale)) {
            fNewScale = fImageSizeScale;
        }
        pImage = Image_Bitmap::MakeImage(imageData.m_imageWidth, imageData.m_imageHeight, imageData.m_imageData.data(), fNewScale);
    }
    return pImage;
}

std::unique_ptr<IImage> ImageDecoder_Common::LoadImageDataAsync(const ImageDecodeParam& decodeParam)
{
    uint32_t nWidth = 0;
    uint32_t nHeight = 0;
    std::shared_ptr<std::vector<uint8_t>> pFileData;
    FilePath imageFilePath;
    bool bGotImageSize = false;
    if ((decodeParam.m_pFileData != nullptr) && !decodeParam.m_pFileData->empty()) {
        pFileData = decodeParam.m_pFileData;
        bGotImageSize = ImageDecoderUtil::GetImageSizeFromMemory(*pFileData, nWidth, nHeight);
    }
    else if (!decodeParam.m_imageFilePath.IsEmpty()) {
        imageFilePath = decodeParam.m_imageFilePath;
        bGotImageSize = ImageDecoderUtil::GetImageSizeFromFile(imageFilePath, nWidth, nHeight);
    }
    if (!bGotImageSize || (nWidth == 0) || (nHeight == 0) ||
        (nWidth > (uint32_t)INT32_MAX) || (nHeight > (uint32_t)INT32_MAX)) {
        return nullptr;
    }

    float fNewScale = decodeParam.m_fImageSizeScale;
    if (!ImageUtil::GetBestImageScale(decodeParam.m_rcMaxDestRectSize, (int32_t)nWidth, (int32_t)nHeight,
                                      decodeParam.m_fImageSizeScale, fNewScale)) {
        fNewScale = decodeParam.m_fImageSizeScale;
    }

    std::shared_ptr<Image_DelayBitmap> pDelayBitmap = std::make_shared<Image_DelayBitmap>();
    Image_DelayBitmap::DecodeFunction decodeFunction =
        [pFileData, imageFilePath](UiImageData& imageData) -> bool {
            if (pFileData != nullptr) {
                return ImageDecoderUtil::LoadImageFromMemory(*pFileData, imageData);
            }
            return ImageDecoderUtil::LoadImageFromFile(imageFilePath, imageData);
        };
    if (!pDelayBitmap->InitImage(nWidth, nHeight, fNewScale, true, decodeFunction)) {
        return nullptr;
    }
    return Image_Bitmap::MakeImage(pDelayBitmap);
}

} //namespace ui
