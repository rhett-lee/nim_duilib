#ifndef UI_IMAGE_IMAGE_JPEG_H_
#define UI_IMAGE_IMAGE_JPEG_H_

#include "duilib/Image/ImageDecoder.h"
#include "duilib/Render/IRender.h"

#ifdef DUILIB_IMAGE_SUPPORT_JPEG_TURBO

namespace ui
{
/** JPEG格式的图片数据
*/
class Image_JPEG: public IImage
{
public:
    /** 加载图片数据
    @param [in] fileData 文件数据
    @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
    */
    bool LoadImageData(std::vector<uint8_t>& fileData, float fImageSizeScale);

public:
    Image_JPEG();
    virtual ~Image_JPEG() override;

    /** 获取图片宽度
    */
    virtual int32_t GetWidth() const override;

    /** 获取图片高度
    */
    virtual int32_t GetHeight() const override;

    /** 原图加载的宽度和高度缩放比例(1.0f表示无缩放)
    */
    virtual float GetImageSizeScale() const override;

    /** 获取图片的类型
    */
    virtual ImageType GetImageType() const override;

    /** 获取图片数据
    * @return 仅当ImageType==ImageType::kImageBitmap时返回图片数据
    */
    virtual std::shared_ptr<IBitmap> GetImageBitmap() const override;

private:
    /** 私有实现数据
    */
    struct TImpl;
    std::unique_ptr<TImpl> m_impl;
};

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_JPEG_TURBO
#endif //UI_IMAGE_IMAGE_JPEG_H_
