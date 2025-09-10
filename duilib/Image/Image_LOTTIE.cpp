#include "Image_LOTTIE.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

#include "duilib/RenderSkia/SkiaHeaderBegin.h"
#include "modules/skottie/include/Skottie.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "duilib/RenderSkia/SkiaHeaderEnd.h"

namespace ui
{
//解码LOTTIE图片数据
static bool DecodeImage_LOTTIE(sk_sp<skottie::Animation>& pSkAnimation,                               
                               uint32_t nImageWidth,
                               uint32_t nImageHeight,
                               int32_t nFrameCount,
                               bool bLoadAllFrames,
                               std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>>& frames,
                               volatile bool* pAbortFlag = nullptr)
{
    ASSERT(pSkAnimation != nullptr);
    if (pSkAnimation == nullptr) {
        return false;
    }
    ASSERT(nFrameCount > 0);
    if (nFrameCount < 0) {
        return false;
    }
    ASSERT((nImageWidth > 0) && (nImageHeight > 0));
    if ((nImageWidth <= 0) || (nImageHeight <= 0)) {
        return false;
    }

    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }

    //生成位图，矢量缩放
    frames.clear();
    for (int32_t nFrame = 0; nFrame < nFrameCount; ++nFrame) {
        pSkAnimation->seekFrame(static_cast<double>(nFrame));
        std::shared_ptr<IBitmap> pBitmap(pRenderFactory->CreateBitmap());
        ASSERT(pBitmap != nullptr);
        if (pBitmap == nullptr) {
            frames.clear();
            return false;
        }

        SkBitmap skBitmap;
#ifdef DUILIB_BUILD_FOR_WIN
        SkImageInfo info = SkImageInfo::Make((int32_t)nImageWidth, (int32_t)nImageHeight, SkColorType::kN32_SkColorType, SkAlphaType::kPremul_SkAlphaType);
#else
        SkImageInfo info = SkImageInfo::Make((int32_t)nImageWidth, (int32_t)nImageHeight, SkColorType::kRGBA_8888_SkColorType, SkAlphaType::kPremul_SkAlphaType);
#endif
        skBitmap.allocPixels(info);
        SkCanvas canvas(skBitmap);
        pSkAnimation->render(&canvas);
        if (!pBitmap->Init(nImageWidth, nImageHeight, skBitmap.getPixels())) {
            pBitmap.reset();
        }
        auto pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
        pFrameData->m_nFrameIndex = nFrame;
        pFrameData->m_nOffsetX = 0; // OffsetX和OffsetY均不需要处理
        pFrameData->m_nOffsetY = 0;
        pFrameData->m_bDataPending = false;
        pFrameData->SetDelayMs(int32_t(pSkAnimation->duration() * 1000) / nFrameCount);
        pFrameData->m_pBitmap = pBitmap;
        frames.push_back(pFrameData);

        if (!bLoadAllFrames) {
            break;
        }
        if (pAbortFlag && *pAbortFlag) {
            //已经取消
            frames.clear();
            return false;
        }
    }    
    return true;
}

struct Image_LOTTIE::TImpl
{
    //文件数据
    std::vector<uint8_t> m_fileData;

    //加载后的句柄
    sk_sp<skottie::Animation> m_pSkAnimation;

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

Image_LOTTIE::Image_LOTTIE()
{
    m_impl = std::make_unique<TImpl>();
}

Image_LOTTIE::~Image_LOTTIE()
{
    m_impl->m_pSkAnimation.reset();
}

bool Image_LOTTIE::LoadImageFromMemory(std::vector<uint8_t>& fileData,
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

    // 加载Lottie动画
    m_impl->m_pSkAnimation = skottie::Animation::Builder().make((const char*)m_impl->m_fileData.data(), m_impl->m_fileData.size());
    if (!m_impl->m_pSkAnimation) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    SkISize imageSize = m_impl->m_pSkAnimation->size().toCeil();
    m_impl->m_nWidth = (uint32_t)imageSize.fWidth;
    m_impl->m_nHeight = (uint32_t)imageSize.fHeight;
    m_impl->m_nFrameCount = static_cast<int32_t>(m_impl->m_pSkAnimation->duration() * m_impl->m_pSkAnimation->fps() + 0.5);

    if ((m_impl->m_nFrameCount <= 0) || ((int32_t)m_impl->m_nWidth <= 0) || ((int32_t)m_impl->m_nHeight <= 0)) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    m_impl->m_nWidth = ImageUtil::GetScaledImageSize(m_impl->m_nWidth, fImageSizeScale);
    m_impl->m_nHeight = ImageUtil::GetScaledImageSize(m_impl->m_nHeight, fImageSizeScale);
    ASSERT(m_impl->m_nWidth > 0);
    ASSERT(m_impl->m_nHeight > 0);

    //循环播放固定为一直播放，因GIF格式无此设置
    m_impl->m_nLoops = -1;

    //加载第一帧
    if (!DecodeImage_LOTTIE(m_impl->m_pSkAnimation, m_impl->m_nWidth, m_impl->m_nHeight, m_impl->m_nFrameCount, false, m_impl->m_frames)) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        m_impl->m_pSkAnimation.reset();
        return false;
    }
    return true;
}

bool Image_LOTTIE::IsDecodeImageDataEnabled() const
{
    if (!m_impl->m_fileData.empty() &&
        (m_impl->m_pSkAnimation != nullptr) &&
        m_impl->m_bLoadAllFrames &&
        (m_impl->m_nFrameCount > 1) &&
        (m_impl->m_frames.size() == 1)) {
        return true;
    }
    return false;
}

void Image_LOTTIE::SetDecodeImageDataStarted()
{
    m_impl->m_bDecodeImageDataStarted = true;
    m_impl->m_bDecodeImageDataAborted = false;
}

bool Image_LOTTIE::DecodeImageData()
{
    if (!IsDecodeImageDataEnabled()) {
        return false;
    }
    std::vector<uint8_t> fileData;
    m_impl->m_fileData.swap(fileData);

    //加载所有帧
    bool bLoaded = false;
    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> frames;
    if (DecodeImage_LOTTIE(m_impl->m_pSkAnimation, m_impl->m_nWidth, m_impl->m_nHeight, m_impl->m_nFrameCount, true, frames)) {
        ASSERT(!m_impl->m_bDecodeImageDataFinished);
        ASSERT(m_impl->m_delayFrames.empty());

        m_impl->m_delayFrames.swap(frames);
        m_impl->m_bDecodeImageDataFinished = true;
        bLoaded = true;
    }
    //不管是否成功，释放资源
    m_impl->m_pSkAnimation.reset();
    return bLoaded;
}

void Image_LOTTIE::SetDecodeImageDataAborted()
{
    m_impl->m_bDecodeImageDataAborted = true;
}

uint32_t Image_LOTTIE::GetWidth() const
{
    return m_impl->m_nWidth;
}

uint32_t Image_LOTTIE::GetHeight() const
{
    return m_impl->m_nHeight;
}

int32_t Image_LOTTIE::GetFrameCount() const
{
    return m_impl->m_nFrameCount;
}

int32_t Image_LOTTIE::GetLoopCount() const
{
    return m_impl->m_nLoops;
}

bool Image_LOTTIE::ReadFrame(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
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
