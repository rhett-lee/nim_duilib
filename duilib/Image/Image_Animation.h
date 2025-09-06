#ifndef UI_IMAGE_IMAGE_ANIMATION_H_
#define UI_IMAGE_IMAGE_ANIMATION_H_

#include "duilib/Image/ImageDecoder.h"

namespace ui
{
/** 动画格式的图片数据
*/
class Image_Animation: public IImage
{
public:
    Image_Animation(const std::shared_ptr<IAnimationImage>& pAnimationImage,
                    float fImageSizeScale);
    virtual ~Image_Animation() override;

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
    * @return 仅当ImageType==ImageType::kImageAnimation时返回图片数据
    */
    virtual std::shared_ptr<IAnimationImage> GetImageAnimation() const override;

private:
    /** 动画图片
    */
    std::shared_ptr<IAnimationImage> m_pAnimationImage;

    /** 图片的缩放比例
    */
    float m_fImageSizeScale;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_ANIMATION_H_
