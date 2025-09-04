#ifndef UI_IMAGE_IMAGE_UTIL_H_
#define UI_IMAGE_IMAGE_UTIL_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** 位图接口
*/
class IBitmap;

/** SVG矢量图片接口
*/
class UILIB_API ImageUtil
{
public:
    /** 判断此图片缩放比例下，是否需要对图片进行resize操作
    * @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
    */
    static bool NeedResizeImage(float fImageSizeScale);

    /** 判断两个图片的缩放比例是否相同
    */
    static bool IsSameImageScale(float fImageSizeScale1, float fImageSizeScale2);

    /** 计算出按比例缩放后的图片尺寸（按比例缩放）
    * @param [in] nImageSize 原图片的尺寸，宽度或者高度
    * @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
    */
    static uint32_t GetScaledImageSize(uint32_t nImageSize, float fImageSizeScale);

    /** 计算出按比例缩放后的图片尺寸（按DPI显示百分比缩放）
    * @param [in] nImageSize 原图片的尺寸，宽度或者高度
    * @param [in] nNewDpiScale 新的DPI缩放百分比，100为原值，200表示缩放200%
    * @param [in] nOrgDpiScale nImageSize值对应的DPI缩放百分比
    */
    static uint32_t GetScaledImageSize(uint32_t nImageSize, uint32_t nNewDpiScale, uint32_t nOrgDpiScale = 100);

    /** 对 32 位像素格式（RGBA/ARGB 等，4 字节/像素）的图像进行高度翻转（上下翻转）
    * @param [in] pPixelBits 图像数据的起始地址（需可写）
    * @param [in] nPixelBitsLen 像素数据总字节数（需等于 nHeight * nWidth * 4）
    * @param [in] nWidth 图像数据的宽度
    * @param [in] nHeight 图像数据的高度
    * @return 成功返回 true，参数无效返回 false
    */
    static bool FlipPixelBits(uint8_t* pPixelBits, size_t nPixelBitsLen, uint32_t nWidth, uint32_t nHeight);

    /** 对图片大小进行调整
    * @param [in] pPixelBits 图像数据的起始地址
    * @param [in] nPixelBitsLen 像素数据总字节数（需等于 nHeight * nWidth * 4）
    * @param [in] nWidth 图像数据的宽度
    * @param [in] nHeight 图像数据的高度
    * @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
    * @param [out] outPixelBits 调整大小后的图像数据（数据长度等于nImageHeight * nNewWidth * 4）
    * @param [out] nNewWidth 调整后的图像宽度
    * @param [out] nNewHeight 调整后的图像高度
    */
    static bool ResizeImageData(const uint8_t* pPixelBits, size_t nPixelBitsLen, uint32_t nWidth, uint32_t nHeight,
                                float fImageSizeScale,
                                std::vector<uint8_t>& outPixelBits, uint32_t& nNewWidth, uint32_t& nNewHeight);

    /** 对图片大小进行调整
    * @param [in] pPixelBits 图像数据的起始地址
    * @param [in] nPixelBitsLen 像素数据总字节数（需等于 nHeight * nWidth * 4）
    * @param [in] nWidth 图像数据的宽度
    * @param [in] nHeight 图像数据的高度
    * @param [in] pOutPixelBits 输出调整大小后的图像数据（可写，数据长度等于nImageHeight * nNewWidth * 4）
    * @param [in] nNewWidth 调整后的图像宽度
    * @param [in] nNewHeight 调整后的图像高度
    */
    static bool ResizeImageData(const uint8_t* pPixelBits, size_t nPixelBitsLen, uint32_t nWidth, uint32_t nHeight,
                                uint8_t* pOutPixelBits, size_t nOutPixelBitsLen, uint32_t nNewWidth, uint32_t nNewHeight);

    /** 对位图调整大小
    * @param [in] pBitmap 需要调整大小的位图接口
    * @param [in] nNewWidth 调整后的图像宽度
    * @param [in] nNewHeight 调整后的图像高度
    */
    static std::unique_ptr<IBitmap> ResizeImageBitmap(IBitmap* pBitmap, int32_t nNewWidth, int32_t nNewHeight);
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_UTIL_H_
