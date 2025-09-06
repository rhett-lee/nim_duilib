#ifndef UI_IMAGE_IMAGE_SVG_H_
#define UI_IMAGE_IMAGE_SVG_H_

#include "duilib/Image/ImageDecoder.h"
#include "duilib/Render/IRender.h"

namespace ui
{
/** Svg格式的图片数据(支持矢量缩放)
*/
class Image_Svg: public IImage
{
public:
    /** 创建一个位图的图片数据（ARGB格式）
    @param [in] nWidth 宽度
    @param [in] nHeight 高度
    @param [in] pPixelBits 位图数据, 如果为nullptr表示窗口空位图，如果不为nullptr，其数据长度为：nWidth*4*nHeight
    @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
    @param [in] alphaType 位图的Alpha类型    
    */
    static std::unique_ptr<IImage> MakeImage(const std::shared_ptr<ISvgImage>& pSvgImage,
                                             float fImageSizeScale = 1.0f);

public:
    Image_Svg();
    virtual ~Image_Svg() override;

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
    * @return 仅当ImageType==ImageType::kImageSvg时返回图片数据
    */
    virtual std::shared_ptr<ISvgImage> GetImageSvg() const override;

private:
    /** 位图数据
    */
    std::shared_ptr<ISvgImage> m_pSvgImage;

    /** 原图加载时的缩放比例
    */
    float m_fImageSizeScale;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_SVG_H_
