#include "Image_Svg.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{
Image_Svg::Image_Svg():
    m_fImageSizeScale(IMAGE_SIZE_SCALE_NONE)
{
}

Image_Svg::~Image_Svg()
{
}

std::unique_ptr<IImage> Image_Svg::MakeImage(const std::shared_ptr<ISvgImage>& pSvgImage,
                                             float fImageSizeScale)
{
    ASSERT(pSvgImage != nullptr);
    if (pSvgImage == nullptr) {
        return nullptr;
    }
    Image_Svg* pImageObj = new Image_Svg;
    std::unique_ptr<IImage> pImage(pImageObj);
    pImageObj->m_pSvgImage = pSvgImage;
    pImageObj->m_fImageSizeScale = fImageSizeScale;
    return pImage;
}

int32_t Image_Svg::GetWidth() const
{
    if (m_pSvgImage != nullptr) {
        return m_pSvgImage->GetWidth();
    }
    return 0;
}

int32_t Image_Svg::GetHeight() const
{
    if (m_pSvgImage != nullptr) {
        return m_pSvgImage->GetHeight();
    }
    return 0;
}

float Image_Svg::GetImageSizeScale() const
{
    return m_fImageSizeScale;
}

ImageType Image_Svg::GetImageType() const
{
    return ImageType::kImageSvg;
}

std::shared_ptr<ISvgImage> Image_Svg::GetImageSvg() const
{
    return m_pSvgImage;
}

} //namespace ui
