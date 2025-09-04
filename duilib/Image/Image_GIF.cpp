#include "Image_GIF.h"

#pragma warning (push)
#pragma warning (disable: 4996)
    #include "duilib/third_party/cximage/ximage.h"
#pragma warning (pop)

namespace ui
{
struct Image_GIF::TImpl
{
    //文件数据
    std::vector<uint8_t> m_fileData;

    //CxImage实现相关类型对象
    std::unique_ptr<CxMemFile> m_memFile;
    std::unique_ptr<CxImage> m_cxImage;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;
};

Image_GIF::Image_GIF()
{
    m_impl = std::make_unique<TImpl>();
}

Image_GIF::~Image_GIF()
{
    m_impl->m_cxImage.reset();
    m_impl->m_memFile.reset();
    m_impl->m_fileData.clear();
}

bool Image_GIF::LoadImageFromMemory(std::vector<uint8_t>& fileData,
                                    bool bLoadAllFrames,
                                    float fImageSizeScale)
{
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return false;
    }
    m_impl->m_fileData.clear();
    m_impl->m_fileData.swap(fileData);
    m_impl->m_fImageSizeScale = fImageSizeScale;
    m_impl->m_memFile = std::make_unique<CxMemFile>(m_impl->m_fileData.data(), (uint32_t)m_impl->m_fileData.size());

    uint32_t imagetype = CXIMAGE_FORMAT_GIF;
    m_impl->m_cxImage = std::make_unique<CxImage>(imagetype);
    m_impl->m_cxImage->SetRetreiveAllFrames(bLoadAllFrames);
    bool isLoaded = m_impl->m_cxImage->Decode(m_impl->m_memFile.get(), imagetype);
    ASSERT(isLoaded && m_impl->m_cxImage->IsValid());
    if (!isLoaded || !m_impl->m_cxImage->IsValid()) {
        //失败
        m_impl->m_cxImage.reset();
        m_impl->m_memFile.reset();
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    int32_t frameCount = m_impl->m_cxImage->GetNumFrames();
    uint32_t nWidth = m_impl->m_cxImage->GetWidth();
    uint32_t nHeight = m_impl->m_cxImage->GetHeight();
    ASSERT((nWidth > 0) && (nHeight > 0) && (frameCount > 0));
    if ((nWidth == 0) || (nHeight == 0) || (frameCount < 1)) {
        //失败
        m_impl->m_cxImage.reset();
        m_impl->m_memFile.reset();
        m_impl->m_fileData.swap(fileData);
        return false;
    }
    return true;
}

uint32_t Image_GIF::GetWidth() const
{
    if (m_impl->m_cxImage != nullptr) {
        return m_impl->m_cxImage->GetWidth();
    }
    return 0;
}

uint32_t Image_GIF::GetHeight() const
{
    if (m_impl->m_cxImage != nullptr) {
        return m_impl->m_cxImage->GetHeight();
    }
    return 0;
}

int32_t Image_GIF::GetFrameCount() const
{
    if (m_impl->m_cxImage != nullptr) {
        return m_impl->m_cxImage->GetNumFrames();
    }
    return 0;
}

int32_t Image_GIF::GetLoopCount() const
{
    return -1;
}

bool Image_GIF::ReadFrame(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
{
    //无缓存
    if (m_impl->m_cxImage == nullptr) {
        return false;
    }

    return true;
}

bool Image_GIF::IsDecodeImageDataEnabled() const
{
    return false;
}

void Image_GIF::SetDecodeImageDataStarted()
{
}

bool Image_GIF::DecodeImageData()
{
    return false;
}

void Image_GIF::SetDecodeImageDataAborted()
{
}

} //namespace ui
