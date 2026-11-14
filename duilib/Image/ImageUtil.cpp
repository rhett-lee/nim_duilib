#include "ImageUtil.h"
#include "duilib/Render/IRender.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/FileUtil.h"
#include <cmath>
#include <cstdint>
#include <cstring>

#pragma warning (push)
#pragma warning (disable: 4505)
    #define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_TRIANGLE
    #include "duilib/third_party/stb_image/stb_image_resize2.h"
#pragma warning (pop)

namespace ui
{

bool ImageUtil::NeedResizeImage(float fImageSizeScale)
{
    if (fImageSizeScale < 0.001f) {
        return false;
    }
    else if (std::fabs(fImageSizeScale - 1.0f) < 0.001f) {
        return false;
    }
    return true;
}

bool ImageUtil::IsValidImageScale(float fImageSizeScale)
{
    if (fImageSizeScale < 0.001f) {
        return false;
    }
    return true;
}

uint32_t ImageUtil::GetScaledImageSize(uint32_t nImageSize, float fImageSizeScale)
{
    if (NeedResizeImage(fImageSizeScale)) {
        nImageSize = static_cast<uint32_t>(std::ceil(nImageSize * fImageSizeScale));
    }
    return nImageSize;
}

int32_t ImageUtil::GetScaledImageOffset(int32_t nImageOffset, float fImageSizeScale)
{
    if (NeedResizeImage(fImageSizeScale)) {
        //计算后，进行四舍五入
        nImageOffset = static_cast<int32_t>(std::ceil(nImageOffset * fImageSizeScale));
    }
    return nImageOffset;
}

bool ImageUtil::IsSameImageScale(float fImageSizeScale1, float fImageSizeScale2)
{
    if (std::fabs(fImageSizeScale1 - fImageSizeScale2) < 0.001f) {
        return true;
    }
    return false;
}

uint32_t ImageUtil::GetScaledImageSize(uint32_t nImageSize, uint32_t nNewDpiScale, uint32_t nOrgDpiScale)
{
    if ((nNewDpiScale != nOrgDpiScale) && (nOrgDpiScale != 0)) {
        nImageSize = static_cast<uint32_t>(std::ceil(nImageSize * static_cast<float>(nNewDpiScale) / static_cast<float>(nOrgDpiScale)));
    }
    return nImageSize;
}

bool ImageUtil::FlipPixelBits(uint8_t* pPixelBits, size_t nPixelBitsLen, uint32_t nWidth, uint32_t nHeight)
{
    ASSERT((pPixelBits != nullptr) && (nPixelBitsLen > 0));
    if ((pPixelBits == nullptr) || (nPixelBitsLen == 0)) {
        return false;
    }
    ASSERT((nWidth > 0) && (nHeight > 0));
    if ((nWidth == 0) || (nHeight == 0)) {
        return false;
    }
    const size_t expectedLen = static_cast<size_t>(nHeight) * nWidth * 4;
    ASSERT(nPixelBitsLen == expectedLen);
    if (nPixelBitsLen != expectedLen) {
        return false;
    }

    const uint32_t rowBytes = nWidth * 4;
    std::vector<uint8_t> tempRow(rowBytes); //临时行缓冲区

    // 按行交换
    for (uint32_t i = 0; i < nHeight / 2; ++i) {
        uint8_t* topRow = pPixelBits + i * rowBytes;
        uint8_t* bottomRow = pPixelBits + (nHeight - 1 - i) * rowBytes;

        // 整行拷贝交换
        memcpy(tempRow.data(), topRow, rowBytes);
        memcpy(topRow, bottomRow, rowBytes);
        memcpy(bottomRow, tempRow.data(), rowBytes);
    }
    return true;
}

bool ImageUtil::ResizeImageData(const uint8_t* pPixelBits, size_t nPixelBitsLen, uint32_t nWidth, uint32_t nHeight,
                                float fImageSizeScale,
                                std::vector<uint8_t>& outPixelBits, uint32_t& nNewWidth, uint32_t& nNewHeight)
{
    ASSERT((pPixelBits != nullptr) && (nPixelBitsLen > 0));
    if ((pPixelBits == nullptr) || (nPixelBitsLen == 0)) {
        return false;
    }
    ASSERT((nWidth > 0) && (nHeight > 0));
    if ((nWidth == 0) || (nHeight == 0)) {
        return false;
    }
    const size_t expectedLen = static_cast<size_t>(nHeight) * nWidth * 4;
    ASSERT(nPixelBitsLen == expectedLen);
    if (nPixelBitsLen != expectedLen) {
        return false;
    }

    nNewWidth = GetScaledImageSize(nWidth, fImageSizeScale);
    nNewHeight = GetScaledImageSize(nHeight, fImageSizeScale);
    size_t nOutPixelBitsLen = static_cast<size_t>(nNewHeight) * nNewWidth * 4;
    outPixelBits.resize(nOutPixelBitsLen);
    uint8_t* pOutPixelBits = outPixelBits.data();
    return ResizeImageData(pPixelBits, nPixelBitsLen, nWidth, nHeight,
                           pOutPixelBits, nOutPixelBitsLen, nNewWidth, nNewHeight);
}

bool ImageUtil::ResizeImageData(const uint8_t* pPixelBits, size_t nPixelBitsLen, uint32_t nWidth, uint32_t nHeight,
                                uint8_t* pOutPixelBits, size_t nOutPixelBitsLen, uint32_t nNewWidth, uint32_t nNewHeight)
{
    ASSERT((pPixelBits != nullptr) && (nPixelBitsLen > 0));
    if ((pPixelBits == nullptr) || (nPixelBitsLen == 0)) {
        return false;
    }
    ASSERT((nWidth > 0) && (nHeight > 0));
    if ((nWidth == 0) || (nHeight == 0)) {
        return false;
    }
    const size_t expectedLen = static_cast<size_t>(nHeight) * nWidth * 4;
    ASSERT(nPixelBitsLen == expectedLen);
    if (nPixelBitsLen != expectedLen) {
        return false;
    }

    ASSERT((pOutPixelBits != nullptr) && (nOutPixelBitsLen > 0));
    if ((pOutPixelBits == nullptr) || (nOutPixelBitsLen == 0)) {
        return false;
    }
    ASSERT((nNewWidth > 0) && (nNewHeight > 0));
    if ((nNewWidth == 0) || (nNewHeight == 0)) {
        return false;
    }
    const size_t outExpectedLen = static_cast<size_t>(nNewHeight) * nNewWidth * 4;
    ASSERT(nOutPixelBitsLen == outExpectedLen);
    if (nOutPixelBitsLen != outExpectedLen) {
        return false;
    }

    if ((nWidth == nNewWidth) && (nHeight == nNewHeight)) {
        memcpy(pOutPixelBits, pPixelBits, outExpectedLen);
        return true;
    }
    //对图像数据执行resize操作
    if (1) {
        //使用stb_image实现
        const unsigned char* input_pixels = (const unsigned char*)pPixelBits;
        int input_w = (int)nWidth;
        int input_h = (int)nHeight;
        int input_stride_in_bytes = 0;
        unsigned char* output_pixels = (unsigned char*)pOutPixelBits;
        int output_w = (int)nNewWidth;
        int output_h = (int)nNewHeight;
        int output_stride_in_bytes = 0;
        stbir_pixel_layout num_channels = STBIR_RGBA_PM;
        unsigned char* result = stbir_resize_uint8_linear(input_pixels, input_w, input_h, input_stride_in_bytes,
                                                          output_pixels, output_w, output_h, output_stride_in_bytes,
                                                          num_channels);
        return result != nullptr;
    }
    return false;
}

std::unique_ptr<IBitmap> ImageUtil::ResizeImageBitmap(IBitmap* pBitmap, int32_t nNewWidth, int32_t nNewHeight)
{
    if ((pBitmap == nullptr) || (pBitmap->GetWidth() < 1) || (pBitmap->GetHeight() < 1)) {
        return nullptr;
    }
    if ((nNewWidth < 1) || (nNewHeight < 1)) {
        return nullptr;
    }

    std::unique_ptr<IBitmap> pNewBitmap;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        pNewBitmap.reset(pRenderFactory->CreateBitmap());
    }
    ASSERT(pNewBitmap != nullptr);
    if (pNewBitmap == nullptr) {
        return nullptr;
    }
    if (!pNewBitmap->Init((uint32_t)nNewWidth, (uint32_t)nNewHeight, nullptr)) {
        ASSERT(0);
        return nullptr;
    }
    void* pOutPixelBits = pNewBitmap->LockPixelBits();
    if (pOutPixelBits == nullptr) {
        return nullptr;
    }
    // 安全计算 buffer 大小（避免 int32_t / uint32_t 乘法溢出）
    // 先做 size_t 提升再相乘，保证中间结果不会截断
    if (nNewHeight > 0 && (size_t)nNewWidth > (SIZE_MAX / 4 / (size_t)nNewHeight)) {
        // 溢出保护
        ASSERT(!"ResizeImageBitmap: image too large");
        pNewBitmap->UnLockPixelBits();
        return nullptr;
    }
    size_t nOutPixelBitsLen = (size_t)nNewWidth * (size_t)nNewHeight * 4;
    if (nOutPixelBitsLen == 0) {
        pNewBitmap->UnLockPixelBits();
        return nullptr;
    }

    void* pPixelBits = pBitmap->LockPixelBits();
    if (pPixelBits == nullptr) {
        pNewBitmap->UnLockPixelBits();
        return nullptr;
    }
    uint32_t nWidth = pBitmap->GetWidth();
    uint32_t nHeight = pBitmap->GetHeight();
    // 同样的溢出保护
    if (nHeight > 0 && nWidth > (SIZE_MAX / 4 / (size_t)nHeight)) {
        ASSERT(!"ResizeImageBitmap: source image too large");
        pBitmap->UnLockPixelBits();
        pNewBitmap->UnLockPixelBits();
        return nullptr;
    }
    size_t nPixelBitsLen = (size_t)nWidth * (size_t)nHeight * 4;
    if (nPixelBitsLen == 0) {
        pBitmap->UnLockPixelBits();
        pNewBitmap->UnLockPixelBits();
        return nullptr;
    }

    if (ResizeImageData((const uint8_t*)pPixelBits, nPixelBitsLen, nWidth, nHeight,
                        (uint8_t*)pOutPixelBits, nOutPixelBitsLen, (uint32_t)nNewWidth, (uint32_t)nNewHeight)) {
        pNewBitmap->UnLockPixelBits();
    }
    else {
        pNewBitmap.reset();
    }
    ASSERT(pNewBitmap != nullptr);
    return pNewBitmap;
}

bool ImageUtil::GetBestImageScale(const UiSize& rcMaxDestRectSize,
                                  int32_t nImageWidth,
                                  int32_t nImageHeight,
                                  float fMaxScale,
                                  float& fScale)
{
    if ((nImageWidth <= 0) || (nImageHeight <= 0)) {
        return false;
    }
    if ((rcMaxDestRectSize.cx > 0) && (rcMaxDestRectSize.cy > 0)) {
        //同时设置的宽度和高度
        if ((nImageWidth <= rcMaxDestRectSize.cx) || (nImageHeight <= rcMaxDestRectSize.cy)) {
            //图片小于目标区域时，计算合适的缩放比（适当放大）
            float fScaleX = static_cast<float>(rcMaxDestRectSize.cx) / nImageWidth;
            float fScaleY = static_cast<float>(rcMaxDestRectSize.cy) / nImageHeight;
            fScale = std::max(fScaleX, fScaleY);
        }
        else {
            //图片大于目标区域时，计算合适的缩放比（适当缩小）
            float fScaleX = static_cast<float>(rcMaxDestRectSize.cx) / nImageWidth;
            float fScaleY = static_cast<float>(rcMaxDestRectSize.cy) / nImageHeight;
            fScale = std::min(fScaleX, fScaleY);
        }
    }
    else if (rcMaxDestRectSize.cx > 0) {
        //只设置了宽度
        fScale = static_cast<float>(rcMaxDestRectSize.cx) / nImageWidth;
    }
    else if (rcMaxDestRectSize.cy > 0) {
        //只设置了高度
        fScale = static_cast<float>(rcMaxDestRectSize.cy) / nImageHeight;
    }
    else {
        return false;
    }

    if (fMaxScale > 0.99f) {
        //限制最大值
        if (fScale > fMaxScale) {
            fScale = fMaxScale;
        }
    }

    //限制最小值，避免缩放过小
    const float fMinScale = 1.0f / 16;
    if (fScale < fMinScale) {
        fScale = fMinScale;
    }
    const int32_t minImageSize = GlobalManager::Instance().Dpi().GetScaleInt(12);
    if ((int32_t)(nImageWidth * fScale) < minImageSize) {
        fScale = static_cast<float>(minImageSize) / nImageWidth;
    }
    if ((int32_t)(nImageHeight * fScale) < minImageSize) {
        fScale = static_cast<float>(minImageSize) / nImageHeight;
    }
    return true;
}

namespace
{
/** 向缓冲区写入16位小端整数
*/
void WriteUint16LE(std::vector<uint8_t>& data, size_t offset, uint16_t value)
{
    data[offset] = (uint8_t)(value & 0xFF);
    data[offset + 1] = (uint8_t)((value >> 8) & 0xFF);
}

/** 向缓冲区写入32位小端整数
*/
void WriteUint32LE(std::vector<uint8_t>& data, size_t offset, uint32_t value)
{
    data[offset] = (uint8_t)(value & 0xFF);
    data[offset + 1] = (uint8_t)((value >> 8) & 0xFF);
    data[offset + 2] = (uint8_t)((value >> 16) & 0xFF);
    data[offset + 3] = (uint8_t)((value >> 24) & 0xFF);
}
} //namespace

bool ImageUtil::SaveBitmapToBmpFile(IBitmap* pBitmap, const FilePath& filePath)
{
    if ((pBitmap == nullptr) || !pBitmap->IsValid()) {
        return false;
    }
    const uint32_t nWidth = pBitmap->GetWidth();
    const uint32_t nHeight = pBitmap->GetHeight();
    const uint32_t nRowBytes = pBitmap->GetRowBytes();
    if ((nWidth == 0) || (nHeight == 0)) {
        return false;
    }
    //每行像素至少需要 nWidth * 4 个字节
    if ((uint64_t)nRowBytes < ((uint64_t)nWidth * 4)) {
        return false;
    }
    //计算像素数据长度与文件长度，避免整数溢出
    const uint64_t nImageSize64 = (uint64_t)nWidth * (uint64_t)nHeight * 4;
    const uint64_t nFileSize64 = 54 + nImageSize64;
    if (nFileSize64 > (uint64_t)0xFFFFFFFF) {
        return false;
    }
    const uint32_t nImageSize = (uint32_t)nImageSize64;
    const uint32_t nFileSize = (uint32_t)nFileSize64;

    const void* pPixelBits = pBitmap->LockPixelBits();
    if (pPixelBits == nullptr) {
        return false;
    }

    std::vector<uint8_t> fileData((size_t)nFileSize, 0);
    //BITMAPFILEHEADER（14字节）
    fileData[0] = 'B';
    fileData[1] = 'M';
    WriteUint32LE(fileData, 2, nFileSize);      //文件总长度
    WriteUint32LE(fileData, 6, 0);              //保留字段
    WriteUint32LE(fileData, 10, 54);            //像素数据偏移
    //BITMAPINFOHEADER（40字节）
    WriteUint32LE(fileData, 14, 40);            //头长度
    WriteUint32LE(fileData, 18, nWidth);        //宽度
    WriteUint32LE(fileData, 22, nHeight);       //高度（正数表示自下而上存储）
    WriteUint16LE(fileData, 26, 1);             //颜色平面数
    WriteUint16LE(fileData, 28, 32);            //每像素位数
    WriteUint32LE(fileData, 30, 0);             //压缩方式：BI_RGB
    WriteUint32LE(fileData, 34, nImageSize);    //像素数据长度
    WriteUint32LE(fileData, 38, 0);             //水平分辨率
    WriteUint32LE(fileData, 42, 0);             //垂直分辨率
    WriteUint32LE(fileData, 46, 0);             //调色板颜色数
    WriteUint32LE(fileData, 50, 0);             //重要颜色数

    //像素数据：BMP为自下而上存储，内存中的字节序为BGRA，与位图（ARGB）在小端平台上一致
    const bool bPremul = (pBitmap->GetAlphaType() == AlphaType::kPremul);
    const size_t nDestRowBytes = (size_t)nWidth * 4;
    for (uint32_t y = 0; y < nHeight; ++y) {
        const uint8_t* pSrcRow = (const uint8_t*)pPixelBits + (size_t)(nHeight - 1 - y) * nRowBytes;
        uint8_t* pDstRow = fileData.data() + 54 + (size_t)y * nDestRowBytes;
        for (uint32_t x = 0; x < nWidth; ++x) {
            uint32_t nColor = 0;
            ::memcpy(&nColor, pSrcRow + (size_t)x * 4, sizeof(nColor));
            const uint8_t nAlpha = (uint8_t)((nColor >> 24) & 0xFF);
            if (bPremul && (nAlpha > 0) && (nAlpha < 255)) {
                //还原为非预乘的RGB值，保证忽略Alpha通道的查看器中颜色正确
                uint8_t nBlue  = (uint8_t)((nColor >> 0) & 0xFF);
                uint8_t nGreen = (uint8_t)((nColor >> 8) & 0xFF);
                uint8_t nRed   = (uint8_t)((nColor >> 16) & 0xFF);
                nBlue  = (uint8_t)(((uint32_t)nBlue * 255 + nAlpha / 2) / nAlpha);
                nGreen = (uint8_t)(((uint32_t)nGreen * 255 + nAlpha / 2) / nAlpha);
                nRed   = (uint8_t)(((uint32_t)nRed * 255 + nAlpha / 2) / nAlpha);
                nColor = ((uint32_t)nAlpha << 24) | ((uint32_t)nRed << 16) | ((uint32_t)nGreen << 8) | nBlue;
            }
            ::memcpy(pDstRow + (size_t)x * 4, &nColor, sizeof(nColor));
        }
    }
    pBitmap->UnLockPixelBits();
    return FileUtil::WriteFileData(filePath, fileData);
}

} //namespace ui
