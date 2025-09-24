#include "Image_Animation.h"

namespace ui
{
Image_Animation::Image_Animation(const std::shared_ptr<IAnimationImage>& pAnimationImage):
    m_pAnimationImage(pAnimationImage),
    m_nAsyncDecodeTaskId(0)
{
}

Image_Animation::~Image_Animation()
{
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
    if (m_pAnimationImage != nullptr) {
        return m_pAnimationImage->GetImageSizeScale();
    }
    return IMAGE_SIZE_SCALE_NONE;
}

ImageType Image_Animation::GetImageType() const
{
    return ImageType::kImageAnimation;
}

std::shared_ptr<IAnimationImage> Image_Animation::GetImageAnimation() const
{
    return m_pAnimationImage;
}

bool Image_Animation::IsAsyncDecodeEnabled() const
{
    if (m_pAnimationImage != nullptr) {
        return m_pAnimationImage->IsDelayDecodeEnabled();
    }
    return false;
}

bool Image_Animation::IsAsyncDecodeFinished() const
{
    if (m_pAnimationImage != nullptr) {
        return m_pAnimationImage->IsDelayDecodeFinished();
    }
    return false;
}

uint32_t Image_Animation::GetDecodedFrameIndex() const
{
    if (m_pAnimationImage != nullptr) {
        return m_pAnimationImage->GetDecodedFrameIndex();
    }
    return 0;
}

void Image_Animation::SetAsyncDecodeTaskId(size_t nTaskId)
{
    m_nAsyncDecodeTaskId = nTaskId;
}

size_t Image_Animation::GetAsyncDecodeTaskId() const
{
    return m_nAsyncDecodeTaskId;
}

bool Image_Animation::AsyncDecode(uint32_t nMinFrameIndex, std::function<bool(void)> IsAborted)
{
    ASSERT(m_pAnimationImage != nullptr);
    if (m_pAnimationImage != nullptr) {
        return m_pAnimationImage->DelayDecode(nMinFrameIndex, IsAborted);
    }
    return false;
}

bool Image_Animation::MergeAsyncDecodeData()
{
    ASSERT(m_pAnimationImage != nullptr);
    if (m_pAnimationImage != nullptr) {
        return m_pAnimationImage->MergeDelayDecodeData();
    }
    return false;
}

} //namespace ui
