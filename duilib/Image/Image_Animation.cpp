#include "Image_Animation.h"

namespace ui
{
Image_Animation::Image_Animation(const std::shared_ptr<IAnimationImage>& pAnimationImage,
                                 float fImageSizeScale):
    m_pAnimationImage(pAnimationImage),
    m_fImageSizeScale(fImageSizeScale)
{
}

Image_Animation::~Image_Animation()
{
    //关联的Image析构时，终止解码
    if (m_pAnimationImage != nullptr) {
        m_pAnimationImage->SetDecodeImageDataAborted();
    }
}

int32_t Image_Animation::GetWidth() const
{
    if (m_pAnimationImage != nullptr) {
        return m_pAnimationImage->GetWidth();
    }
    return 0;
}

int32_t Image_Animation::GetHeight() const
{
    if (m_pAnimationImage != nullptr) {
        return m_pAnimationImage->GetHeight();
    }
    return 0;
}

float Image_Animation::GetImageSizeScale() const
{
    return m_fImageSizeScale;
}

ImageType Image_Animation::GetImageType() const
{
    return ImageType::kImageAnimation;
}

std::shared_ptr<IAnimationImage> Image_Animation::GetImageAnimation() const
{
    return m_pAnimationImage;
}

} //namespace ui
