#include "Image_WEBP.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

#include "duilib/third_party/libwebp/src/webp/decode.h"
#include "duilib/third_party/libwebp/src/webp/demux.h"

namespace ui
{
//解码WebP图片数据
static bool DecodeImage_WEBP(WebPAnimDecoder* pWebPAnimDecoder,
                             float fImageSizeScale,
                             bool bLoadAllFrames,
                             std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>>& frames,
                             volatile bool* pAbortFlag = nullptr)
{
    ASSERT(pWebPAnimDecoder != nullptr);
    if (pWebPAnimDecoder == nullptr) {
        return false;
    }
    ASSERT(fImageSizeScale > 0);
    if (fImageSizeScale < 0) {
        return false;
    }

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }
    frames.clear();

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(pWebPAnimDecoder, &anim_info)) {
        return false;
    }

    bool bLoaded = false;
    int prev_timestamp = 0;
    int32_t nFrameIndex = 0;
    WebPAnimDecoderReset(pWebPAnimDecoder);
    while (WebPAnimDecoderHasMoreFrames(pWebPAnimDecoder)) {
        uint8_t* pImageData = nullptr;
        int timestamp = 0;
        if (WebPAnimDecoderGetNext(pWebPAnimDecoder, &pImageData, &timestamp)) {
            bLoaded = true;

            // 根据时间戳控制帧显示时长(毫秒)
            int duration = timestamp - prev_timestamp;
            std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
            pFrameData->m_nFrameIndex = nFrameIndex;
            pFrameData->SetDelayMs(duration);
            pFrameData->m_nOffsetX = 0;//OffsetX和OffsetY均不需要处理
            pFrameData->m_nOffsetY = 0;
            pFrameData->m_bDataPending = false;
            pFrameData->m_pBitmap.reset(pRenderFactory->CreateBitmap());
            ASSERT(pFrameData->m_pBitmap != nullptr);
            if (pFrameData->m_pBitmap == nullptr) {
                bLoaded = false;
                break;
            }
            if (!pFrameData->m_pBitmap->Init(anim_info.canvas_width, anim_info.canvas_height, pImageData, fImageSizeScale)) {
                bLoaded = false;
                break;
            }
            frames.push_back(pFrameData);

            ++nFrameIndex;
            if (!bLoadAllFrames) {
                //只加载1帧
                break;
            }
            prev_timestamp = timestamp;
        }
        else {
            //失败
            bLoaded = false;
            break;
        }

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
    WebPAnimDecoder* m_pWebPAnimDecoder = nullptr;

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
    if (m_impl->m_pWebPAnimDecoder != nullptr) {
        WebPAnimDecoderDelete(m_impl->m_pWebPAnimDecoder);
        m_impl->m_pWebPAnimDecoder = nullptr;
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

    //加载第一帧
    if (!DecodeImage_WEBP(dec, fImageSizeScale, false, m_impl->m_frames)) {        
        //加载失败时，需要恢复原文件数据
        WebPAnimDecoderDelete(dec);
        m_impl->m_fileData.swap(fileData);
        return false;
    }
    m_impl->m_pWebPAnimDecoder = dec;
    return true;
}

bool Image_WEBP::IsDecodeImageDataEnabled() const
{
    if (!m_impl->m_fileData.empty() &&
        (m_impl->m_pWebPAnimDecoder != nullptr) &&
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
    if (DecodeImage_WEBP(m_impl->m_pWebPAnimDecoder, fImageSizeScale, true, frames)) {
        ASSERT(!m_impl->m_bDecodeImageDataFinished);
        ASSERT(m_impl->m_delayFrames.empty());

        m_impl->m_delayFrames.swap(frames);
        m_impl->m_bDecodeImageDataFinished = true;
        bLoaded = true;
    }
    //不管是否成功，释放资源
    WebPAnimDecoderDelete(m_impl->m_pWebPAnimDecoder);
    m_impl->m_pWebPAnimDecoder = nullptr;
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

bool Image_WEBP::IsFrameDataReady(uint32_t nFrameIndex)
{
    //TODO: 待实现
    return false;
}

int32_t Image_WEBP::GetFrameDelayMs(uint32_t nFrameIndex)
{
    //TODO: 待实现
    return 5000;
}

bool Image_WEBP::ReadFrameData(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
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
