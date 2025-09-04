#include "Image_WEBP.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

#include "duilib/third_party/libwebp/src/webp/decode.h"
#include "duilib/third_party/libwebp/src/webp/demux.h"

namespace ui
{
//解码WebP图片数据
static bool DecodeImage_WEBP(WebPDemuxer* demuxer,
                             float fImageSizeScale,
                             bool bLoadAllFrames,
                             std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>>& frames,
                             volatile bool* pAbortFlag = nullptr)
{
    ASSERT(demuxer != nullptr);
    if (demuxer == nullptr) {
        return false;
    }
    ASSERT(fImageSizeScale > 0);
    if (fImageSizeScale < 0) {
        return false;
    }

    IRenderFactory* pRenderFactroy = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactroy != nullptr);
    if (pRenderFactroy == nullptr) {
        return false;
    }
    frames.clear();

    int32_t frameCount = (int32_t)WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT);
    if (frameCount <= 0) {
        return false;
    }
    if (!bLoadAllFrames) {
        //只加载第1帧，不加载全部帧
        frameCount = 1;
    }
    frames.resize(frameCount);
    bool bLoaded = true;
    // libwebp's index start with 1
    for (int frame_idx = 1; frame_idx <= (int)frameCount; ++frame_idx) {
        WebPIterator iter;
        int ret = WebPDemuxGetFrame(demuxer, frame_idx, &iter);
        ASSERT(ret != 0);
        if (ret == 0) {
            bLoaded = false;
            WebPDemuxReleaseIterator(&iter);
            break;
        }
        int width = 0;
        int hight = 0;
        uint8_t* decode_data = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
        //数据格式：Window平台BGRA，其他平台RGBA
        decode_data = WebPDecodeBGRA(iter.fragment.bytes, iter.fragment.size, &width, &hight);
#else
        decode_data = WebPDecodeRGBA(iter.fragment.bytes, iter.fragment.size, &width, &hight);
#endif

        ASSERT((decode_data != nullptr) && (width > 0) && (hight > 0));
        if ((decode_data == nullptr) || (width <= 0) || (hight <= 0)) {
            bLoaded = false;
            WebPDemuxReleaseIterator(&iter);
            break;
        }

        std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
        pFrameData->m_nFrameIndex = frame_idx - 1;
        pFrameData->SetDelayMs(iter.duration);
        pFrameData->m_nOffsetX = iter.x_offset;
        pFrameData->m_nOffsetY = iter.y_offset;
        pFrameData->m_bDataPending = false;
        pFrameData->m_pBitmap.reset(pRenderFactroy->CreateBitmap());
        ASSERT(pFrameData->m_pBitmap != nullptr);
        if (pFrameData->m_pBitmap == nullptr) {
            bLoaded = false;
            WebPDemuxReleaseIterator(&iter);
            break;
        }
        pFrameData->m_pBitmap->Init(width, hight, decode_data, fImageSizeScale);

        if (pFrameData->m_nOffsetX < 0) {
            pFrameData->m_nOffsetX = -(int32_t)ImageUtil::GetScaledImageSize((uint32_t)-pFrameData->m_nOffsetX, fImageSizeScale);
        }
        if (pFrameData->m_nOffsetY < 0) {
            pFrameData->m_nOffsetY = -(int32_t)ImageUtil::GetScaledImageSize((uint32_t)-pFrameData->m_nOffsetY, fImageSizeScale);
        }
        frames[pFrameData->m_nFrameIndex] = pFrameData;

        WebPDemuxReleaseIterator(&iter);

        if (pAbortFlag && *pAbortFlag) {
            //已经取消
            bLoaded = false;
            break;
        }
    }
    if (!bLoaded) {
        frames.clear();
    }
    return bLoaded;
}

struct Image_WEBP::TImpl
{
    //文件数据
    std::vector<uint8_t> m_fileData;

    //加载后的句柄
    WebPDemuxer* m_demuxer = nullptr;

    //图片宽度
    uint32_t m_nWidth = 0;

    //图片高度
    uint32_t m_nHeight = 0;

    //图片总帧数
    int32_t m_nFrameCount = 0;

    //播放循环次数
    int32_t m_nLoops = -1;

    //是否加载所有帧
    bool m_bLoadAllFrames = true;

    //延迟解码是否已经开始
    bool m_bDecodeImageDataStarted = false;

    //延迟解码是否已经取消
    volatile bool m_bDecodeImageDataAborted = false;

    //延迟解码是否已经结束
    volatile bool m_bDecodeImageDataFinished = false;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;

    //各个图片帧的数据
    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> m_frames;

    //各个图片帧的数据(延迟解码的数据)
    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> m_delayFrames;
};

Image_WEBP::Image_WEBP()
{
    m_impl = std::make_unique<TImpl>();
}

Image_WEBP::~Image_WEBP()
{
    if (m_impl->m_demuxer != nullptr) {
        WebPDemuxDelete(m_impl->m_demuxer);
        m_impl->m_demuxer = nullptr;
    }
}

bool Image_WEBP::LoadImageFromMemory(std::vector<uint8_t>& fileData,
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
    m_impl->m_bLoadAllFrames = bLoadAllFrames;

    WebPData wd = { m_impl->m_fileData.data() , m_impl->m_fileData.size() };
    WebPDemuxer* demuxer = WebPDemux(&wd);
    if (demuxer == nullptr) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    m_impl->m_nWidth = (int32_t)WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_WIDTH);
    m_impl->m_nHeight = (int32_t)WebPDemuxGetI(demuxer, WEBP_FF_CANVAS_HEIGHT);
    m_impl->m_nFrameCount = (int32_t)WebPDemuxGetI(demuxer, WEBP_FF_FRAME_COUNT);

    if ((m_impl->m_nFrameCount <= 0) || ((int32_t)m_impl->m_nWidth <= 0) || ((int32_t)m_impl->m_nHeight <= 0)) {
        WebPDemuxDelete(demuxer);
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    m_impl->m_nWidth = ImageUtil::GetScaledImageSize(m_impl->m_nWidth, fImageSizeScale);
    m_impl->m_nHeight = ImageUtil::GetScaledImageSize(m_impl->m_nHeight, fImageSizeScale);
    ASSERT(m_impl->m_nWidth > 0);
    ASSERT(m_impl->m_nHeight > 0);

    m_impl->m_nLoops = (int32_t)WebPDemuxGetI(demuxer, WEBP_FF_LOOP_COUNT);
    if (m_impl->m_nLoops == 0) {
        m_impl->m_nLoops = -1;
    }

    //加载第一帧
    if (!DecodeImage_WEBP(demuxer, fImageSizeScale, false, m_impl->m_frames)) {
        WebPDemuxDelete(demuxer);
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }
    m_impl->m_demuxer = demuxer;
    return true;
}

bool Image_WEBP::IsDecodeImageDataEnabled() const
{
    if (!m_impl->m_fileData.empty() &&
        (m_impl->m_demuxer != nullptr) &&
        m_impl->m_bLoadAllFrames &&
        (m_impl->m_nFrameCount > 1) &&
        (m_impl->m_frames.size() == 1)) {
        return true;
    }
    return false;
}

void Image_WEBP::SetDecodeImageDataStarted()
{
    m_impl->m_bDecodeImageDataStarted = true;
    m_impl->m_bDecodeImageDataAborted = false;
}

bool Image_WEBP::DecodeImageData()
{
    if (!IsDecodeImageDataEnabled()) {
        return false;
    }
    std::vector<uint8_t> fileData;
    m_impl->m_fileData.swap(fileData);
    float fImageSizeScale = m_impl->m_fImageSizeScale;

    //加载所有帧
    bool bLoaded = false;
    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> frames;
    if (DecodeImage_WEBP(m_impl->m_demuxer, fImageSizeScale, true, frames)) {
        WebPDemuxDelete(m_impl->m_demuxer);
        m_impl->m_demuxer = nullptr;

        ASSERT(!m_impl->m_bDecodeImageDataFinished);
        ASSERT(m_impl->m_delayFrames.empty());

        m_impl->m_delayFrames.swap(frames);
        m_impl->m_bDecodeImageDataFinished = true;
        bLoaded = true;
    }
    return bLoaded;
}

void Image_WEBP::SetDecodeImageDataAborted()
{
    m_impl->m_bDecodeImageDataAborted = true;
}

uint32_t Image_WEBP::GetWidth() const
{
    return m_impl->m_nWidth;
}

uint32_t Image_WEBP::GetHeight() const
{
    return m_impl->m_nHeight;
}

int32_t Image_WEBP::GetFrameCount() const
{
    return m_impl->m_nFrameCount;
}

int32_t Image_WEBP::GetLoopCount() const
{
    return m_impl->m_nLoops;
}

bool Image_WEBP::ReadFrame(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
{
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        return false;
    }
    ASSERT((nFrameIndex >= 0) && (nFrameIndex < m_impl->m_nFrameCount));
    if ((nFrameIndex < 0) || (nFrameIndex >= m_impl->m_nFrameCount)) {
        return false;
    }
    ASSERT(m_impl->m_nFrameCount > 0);
    if (m_impl->m_nFrameCount <= 0) {
        return false;
    }
    ASSERT(!m_impl->m_frames.empty());
    if (m_impl->m_frames.empty()) {
        return false;
    }

    if (m_impl->m_bDecodeImageDataFinished && !m_impl->m_delayFrames.empty()) {
        //合并延迟解码的数据
        if (m_impl->m_nFrameCount == m_impl->m_delayFrames.size() && (m_impl->m_frames.size() == 1)) {
            auto p = m_impl->m_frames[0];
            m_impl->m_frames.swap(m_impl->m_delayFrames);
            m_impl->m_frames[0] = p;
            m_impl->m_delayFrames.clear();
        }
    }

    bool bRet = false;
    if (nFrameIndex < (int32_t)m_impl->m_frames.size()) {
        std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData = m_impl->m_frames[nFrameIndex];
        if (pFrameData != nullptr) {
            ASSERT(pFrameData->m_nFrameIndex == nFrameIndex);
            *pAnimationFrame = *pFrameData;
            pAnimationFrame->m_bDataPending = false;
            ASSERT(pAnimationFrame->m_pBitmap != nullptr);
            bRet = true;
        }
    }
    else {
        if ((int32_t)m_impl->m_frames.size() < m_impl->m_nFrameCount) {
            //尚未完成多帧解码
            pAnimationFrame->m_bDataPending = true;
            pAnimationFrame->m_pBitmap.reset();
            bRet = true;
        }
    }
    return bRet;
}

} //namespace ui
