#include "Image_WEBP.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

#include "duilib/third_party/libwebp/src/webp/decode.h"
#include "duilib/third_party/libwebp/src/webp/demux.h"

namespace ui
{
//解码WebP图片数据, 一次解码一帧
static std::shared_ptr<IAnimationImage::AnimationFrame> DecodeImage_WEBP(WebPAnimDecoder* pWebPAnimDecoder,
                                                                         float fImageSizeScale,
                                                                         uint32_t nFrameIndex,
                                                                         int32_t& nPrevTimestamp)
{
    ASSERT(pWebPAnimDecoder != nullptr);
    if (pWebPAnimDecoder == nullptr) {
        return nullptr;
    }
    ASSERT(fImageSizeScale > 0);
    if (fImageSizeScale < 0) {
        return nullptr;
    }

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return nullptr;
    }

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(pWebPAnimDecoder, &anim_info)) {
        return nullptr;
    }

    std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData;
    //WebPAnimDecoderReset(pWebPAnimDecoder);
    if (WebPAnimDecoderHasMoreFrames(pWebPAnimDecoder)) {
        uint8_t* pImageData = nullptr;
        int timestamp = 0;
        if (WebPAnimDecoderGetNext(pWebPAnimDecoder, &pImageData, &timestamp)) {
            // 根据时间戳控制帧显示时长(毫秒)
            int32_t duration = timestamp - nPrevTimestamp;
            pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
            pFrameData->m_nFrameIndex = nFrameIndex;
            pFrameData->SetDelayMs(duration);
            pFrameData->m_nOffsetX = 0;//OffsetX和OffsetY均不需要处理
            pFrameData->m_nOffsetY = 0;
            pFrameData->m_bDataPending = false;
            pFrameData->m_pBitmap.reset(pRenderFactory->CreateBitmap());
            ASSERT(pFrameData->m_pBitmap != nullptr);
            if (pFrameData->m_pBitmap == nullptr) {
                pFrameData.reset();
            }
            else if (!pFrameData->m_pBitmap->Init(anim_info.canvas_width, anim_info.canvas_height, pImageData, fImageSizeScale)) {
                pFrameData.reset();
            }
            else {
                nPrevTimestamp = timestamp;
            }
        }
        else {
            //失败
            ASSERT(0);
        }
    }
    return pFrameData;
}

struct Image_WEBP::TImpl
{
    //文件数据
    std::vector<uint8_t> m_fileData;

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

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;

    //各个图片帧的数据
    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> m_frames;

public:
    //是否支持异步线程解码图片数据
    bool m_bAsyncDecode = false;

    //是否正在解码图片数据
    std::atomic<bool> m_bAsyncDecoding = false;

    //加载后的句柄
    WebPAnimDecoder* m_pWebPAnimDecoder = nullptr;

    //前一帧的时间戳，用于计算播放持续时间
    int32_t m_nPrevTimestamp = 0;

    //各个图片帧的数据(延迟解码的数据)
    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> m_delayFrames;
};

Image_WEBP::Image_WEBP()
{
    m_impl = std::make_unique<TImpl>();
}

Image_WEBP::~Image_WEBP()
{
    if (m_impl->m_pWebPAnimDecoder != nullptr) {
        WebPAnimDecoderDelete(m_impl->m_pWebPAnimDecoder);
        m_impl->m_pWebPAnimDecoder = nullptr;
    }
}

bool Image_WEBP::LoadImageFromMemory(std::vector<uint8_t>& fileData,
                                     bool bLoadAllFrames,
                                     bool bAsyncDecode,
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
    m_impl->m_bAsyncDecode = bAsyncDecode;

    WebPData wd = { m_impl->m_fileData.data() , m_impl->m_fileData.size() };

    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
#ifdef DUILIB_BUILD_FOR_WIN
    //数据格式：Window平台BGRA(预乘)，其他平台RGBA(预乘)
    dec_options.color_mode = MODE_bgrA;
#else
    dec_options.color_mode = MODE_rgbA;
#endif

    //开启多线程解码
    dec_options.use_threads = 1;

    WebPAnimDecoder* dec = WebPAnimDecoderNew(&wd, &dec_options);
    if (dec == nullptr) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }
    WebPAnimInfo anim_info;
    int nRet = WebPAnimDecoderGetInfo(dec, &anim_info);
    if (nRet == 0) {
        //加载失败时，需要恢复原文件数据
        WebPAnimDecoderDelete(dec);
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    m_impl->m_nWidth = anim_info.canvas_width;
    m_impl->m_nHeight = anim_info.canvas_height;
    m_impl->m_nFrameCount = (int32_t)anim_info.frame_count;

    if ((m_impl->m_nFrameCount <= 0) || ((int32_t)m_impl->m_nWidth <= 0) || ((int32_t)m_impl->m_nHeight <= 0)) {        
        //加载失败时，需要恢复原文件数据
        WebPAnimDecoderDelete(dec);
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    m_impl->m_nWidth = ImageUtil::GetScaledImageSize(m_impl->m_nWidth, fImageSizeScale);
    m_impl->m_nHeight = ImageUtil::GetScaledImageSize(m_impl->m_nHeight, fImageSizeScale);
    ASSERT(m_impl->m_nWidth > 0);
    ASSERT(m_impl->m_nHeight > 0);

    m_impl->m_nLoops = (int32_t)anim_info.loop_count;
    if (m_impl->m_nLoops <= 0) {
        m_impl->m_nLoops = -1;
    }
    m_impl->m_pWebPAnimDecoder = dec;
    return true;
}

bool Image_WEBP::IsDelayDecodeEnabled() const
{
    if (m_impl->m_bAsyncDecode &&
        !m_impl->m_fileData.empty() &&
        (m_impl->m_pWebPAnimDecoder != nullptr)) {
        return true;
    }
    return false;
}

bool Image_WEBP::IsDelayDecodeFinished() const
{
    if (m_impl->m_bAsyncDecoding) {
        return false;
    }
    return (m_impl->m_frames.size() + m_impl->m_delayFrames.size()) == m_impl->m_nFrameCount;
}

uint32_t Image_WEBP::GetDecodedFrameIndex() const
{
    if (m_impl->m_frames.empty()) {
        return 0;
    }
    else {
        return (uint32_t)m_impl->m_frames.size() - 1;
    }
}

bool Image_WEBP::DelayDecode(uint32_t nMinFrameIndex, std::function<bool(void)> IsAborted)
{
    if (!IsDelayDecodeEnabled()) {
        return false;
    }
    if (m_impl->m_bAsyncDecoding) {
        return false;
    }
    m_impl->m_bAsyncDecoding = true;

    float fImageSizeScale = m_impl->m_fImageSizeScale;
    uint32_t nFrameIndex = (uint32_t)(m_impl->m_delayFrames.size() + m_impl->m_frames.size());
    while (((IsAborted == nullptr) || !IsAborted()) &&
           (nMinFrameIndex >= (int32_t)(m_impl->m_frames.size() + m_impl->m_delayFrames.size())) &&
           ((m_impl->m_frames.size() + m_impl->m_delayFrames.size()) < m_impl->m_nFrameCount)) {
        std::shared_ptr<IAnimationImage::AnimationFrame> pNewAnimationFrame;
        pNewAnimationFrame = DecodeImage_WEBP(m_impl->m_pWebPAnimDecoder,
                                              fImageSizeScale,
                                              nFrameIndex,
                                              m_impl->m_nPrevTimestamp);
        if (pNewAnimationFrame != nullptr) {
            m_impl->m_delayFrames.push_back(pNewAnimationFrame);
        }
        else {
            break;
        }
        ++nFrameIndex;
    }

    m_impl->m_bAsyncDecoding = false;
    return true;
}

bool Image_WEBP::MergeDelayDecodeData()
{
    GlobalManager::Instance().AssertUIThread();
    bool bRet = false;
    if (!m_impl->m_bAsyncDecoding && !m_impl->m_delayFrames.empty()) {
        //合并数据
        for (auto p : m_impl->m_delayFrames) {
            m_impl->m_frames.push_back(p);
        }
        m_impl->m_delayFrames.clear();
        bRet = true;
    }
    if (!m_impl->m_bAsyncDecoding) {
        //如果解码完成，则释放图片资源
        if (m_impl->m_frames.size() == m_impl->m_nFrameCount) {
            if (m_impl->m_pWebPAnimDecoder != nullptr) {
                WebPAnimDecoderDelete(m_impl->m_pWebPAnimDecoder);
                m_impl->m_pWebPAnimDecoder = nullptr;
            }
            std::vector<uint8_t> fileData;
            m_impl->m_fileData.swap(fileData);
        }
    }
    return bRet;
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

bool Image_WEBP::IsFrameDataReady(uint32_t nFrameIndex)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_impl->m_bAsyncDecode) {
        if (nFrameIndex < m_impl->m_frames.size()) {
            return true;
        }
        return false;
    }
    else {
        return true;
    }
}

int32_t Image_WEBP::GetFrameDelayMs(uint32_t nFrameIndex)
{
    GlobalManager::Instance().AssertUIThread();
    int32_t nDelayMs = 1000;
    if (nFrameIndex < m_impl->m_frames.size()) {
        std::shared_ptr<IAnimationImage::AnimationFrame> pFrame = m_impl->m_frames[nFrameIndex];
        if (pFrame != nullptr) {
            nDelayMs = pFrame->GetDelayMs();
        }
    }
    else if (!m_impl->m_frames.empty()) {
        std::shared_ptr<IAnimationImage::AnimationFrame> pFrame = m_impl->m_frames.back();
        if (pFrame != nullptr) {
            nDelayMs = pFrame->GetDelayMs();
        }
    }
    return nDelayMs;
}

bool Image_WEBP::ReadFrameData(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        return false;
    }
    pAnimationFrame->m_bDataPending = true;
    ASSERT((nFrameIndex >= 0) && (nFrameIndex < m_impl->m_nFrameCount));
    if ((nFrameIndex < 0) || (nFrameIndex >= m_impl->m_nFrameCount)) {
        return false;
    }
    ASSERT(m_impl->m_nFrameCount > 0);
    if (m_impl->m_nFrameCount <= 0) {
        return false;
    }

    if (!m_impl->m_bAsyncDecode) {
        //同步解码的情况, 解码所需要的帧
        while ((nFrameIndex >= (int32_t)m_impl->m_frames.size()) &&
               (m_impl->m_frames.size() < m_impl->m_nFrameCount)) {
            ASSERT(m_impl->m_delayFrames.empty());
            uint32_t nInitFrameIndex = (uint32_t)m_impl->m_frames.size();
            float fImageSizeScale = m_impl->m_fImageSizeScale;
            std::shared_ptr<IAnimationImage::AnimationFrame> pNewAnimationFrame;
            pNewAnimationFrame = DecodeImage_WEBP(m_impl->m_pWebPAnimDecoder,
                                                  fImageSizeScale,
                                                  nInitFrameIndex,
                                                  m_impl->m_nPrevTimestamp);
            if (pNewAnimationFrame != nullptr) {
                m_impl->m_frames.push_back(pNewAnimationFrame);
            }
            else {
                break;
            }
        }

        ASSERT((nFrameIndex < (int32_t)m_impl->m_frames.size()));
        if ((nFrameIndex >= (int32_t)m_impl->m_frames.size())) {
            return false;
        }
    }
    else {
        //合并数据
        MergeDelayDecodeData();
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
    else if (m_impl->m_bAsyncDecode) {
        if ((int32_t)m_impl->m_frames.size() < m_impl->m_nFrameCount) {
            //尚未完成多帧解码
            pAnimationFrame->m_bDataPending = true;
            pAnimationFrame->m_pBitmap.reset();
            bRet = true;
        }
        else {
            ASSERT(0);
        }
    }
    else {
        ASSERT(0);
    }
    return bRet;
}

} //namespace ui
