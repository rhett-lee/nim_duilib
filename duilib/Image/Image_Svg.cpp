#include "Image_Svg.h"
#include "duilib/Core/GlobalManager.h"

namespace ui
{
Image_Svg::Image_Svg()
{
}

Image_Svg::~Image_Svg()
{
}

std::unique_ptr<IImage> Image_Svg::MakeImage(const std::shared_ptr<ISvgImage>& pSvgImage)
{
    ASSERT(pSvgImage != nullptr);
    if (pSvgImage == nullptr) {
        return nullptr;
    }
    Image_Svg* pImageObj = new Image_Svg;
    std::unique_ptr<IImage> pImage(pImageObj);
    pImageObj->m_pSvgImage = pSvgImage;
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
    if (m_pSvgImage != nullptr) {
        return m_pSvgImage->GetImageSizeScale();
    }
    return IMAGE_SIZE_SCALE_NONE;
}

ImageType Image_Svg::GetImageType() const
{
    return ImageType::kImageSvg;
}

std::shared_ptr<ISvgImage> Image_Svg::GetImageSvg() const
{
    return m_pSvgImage;
}

bool Image_Svg::IsAsyncDecodeEnabled() const
{
    //不支持延迟解码
    return false;
}

bool Image_Svg::IsAsyncDecodeFinished() const
{
    return true;
}

uint32_t Image_Svg::GetDecodedFrameIndex() const
{
    return 0;
}

void Image_Svg::SetAsyncDecodeTaskId(size_t /*nTaskId*/)
{
}

size_t Image_Svg::GetAsyncDecodeTaskId() const
{
    return 0;
}

bool Image_Svg::AsyncDecode(uint32_t /*nMinFrameIndex*/, std::function<bool(void)> /*IsAborted*/)
{
    return false;
}

bool Image_Svg::MergeAsyncDecodeData()
{
    return false;
}

} //namespace ui
