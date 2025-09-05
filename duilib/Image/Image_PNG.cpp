#include "Image_PNG.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

#include "duilib/third_party/apng/decoder-apng.h"

namespace ui
{
//解码PNG/APNG图片数据
static bool DecodeImage_APNG(APNGDATA* pngData,
                             float fImageSizeScale,
                             std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>>& frames,
                             volatile bool* pAbortFlag = nullptr)
{
    ASSERT((pngData != nullptr) && (pngData->nWid > 0) && (pngData->nHei > 0) && (pngData->nFrames > 0));
    const int nWid = pngData->nWid;
    const int nHei = pngData->nHei;
    if ((nWid <= 0) || (nHei <= 0) || (pngData->nFrames < 1)) {
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

    //swap rgba to bgra and do premultiply
    uint8_t* p = pngData->pdata;
    if (p == nullptr) {
        return false;
    }
    int pixel_count = nWid * nHei * pngData->nFrames;
    for (int i = 0; i < pixel_count; ++i) {
        uint8_t a = p[3];
        uint8_t t = p[0];
        if (a)
        {
#ifdef DUILIB_BUILD_FOR_WIN
            //数据格式：Window平台BGRA，其他平台RGBA
            p[0] = (p[2] * a) / 255;
            p[1] = (p[1] * a) / 255;
            p[2] = (t * a) / 255;
#else
            p[0] = (p[0] * a) / 255;
            p[1] = (p[1] * a) / 255;
            p[2] = (p[2] * a) / 255;
#endif
        }
        else
        {
            memset(p, 0, 4);
        }
        p += 4;
    }

    p = pngData->pdata;

    frames.resize(pngData->nFrames);
    for (int i = 0; i < pngData->nFrames; ++i) {
        std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
        pFrameData->m_nFrameIndex = i;
        pFrameData->SetDelayMs(pngData->pDelay ? pngData->pDelay[i] : 0);
        pFrameData->m_nOffsetX = 0;
        pFrameData->m_nOffsetY = 0;
        pFrameData->m_bDataPending = false;
        pFrameData->m_pBitmap.reset(pRenderFactory->CreateBitmap());
        ASSERT(pFrameData->m_pBitmap != nullptr);
        if (pFrameData->m_pBitmap == nullptr) {
            return false;
        }
        pFrameData->m_pBitmap->Init(nWid, nHei, p, fImageSizeScale);
        frames[i] = pFrameData;

        //下一个帧
        const int imageDataSize = nWid * nHei * 4;
        p += imageDataSize;
        if (pAbortFlag && *pAbortFlag) {
            //已经取消
            return false;
        }
    }
    return true;
}

struct Image_PNG::TImpl
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

Image_PNG::Image_PNG()
{
    m_impl = std::make_unique<TImpl>();
}

Image_PNG::~Image_PNG()
{  
}

bool Image_PNG::LoadImageFromMemory(std::vector<uint8_t>& fileData,
                                    bool bLoadAllFrames,
                                    float fImageSizeScale)
{
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return false;
    }
    uint32_t nOutFrameCount = 0;
    m_impl->m_fileData.clear();
    m_impl->m_fileData.swap(fileData);
    m_impl->m_fImageSizeScale = fImageSizeScale;
    m_impl->m_bLoadAllFrames = bLoadAllFrames;

    //多帧图片也按单帧加载，其他帧需要延迟加载（多线程）
    bool bLoaded = false;
    APNGDATA* apngData = LoadAPNG_from_memory((const char*)m_impl->m_fileData.data(), m_impl->m_fileData.size(), false, nOutFrameCount);
    if (apngData != nullptr) {
        //加载图片数据（第一帧）
        if (DecodeImage_APNG(apngData, fImageSizeScale, m_impl->m_frames)) {
            bLoaded = true;
            //记录总帧数            
            m_impl->m_nWidth = ImageUtil::GetScaledImageSize((uint32_t)apngData->nWid, fImageSizeScale);
            m_impl->m_nHeight = ImageUtil::GetScaledImageSize((uint32_t)apngData->nHei, fImageSizeScale);
            m_impl->m_nFrameCount = nOutFrameCount;
            m_impl->m_nLoops = apngData->nLoops;
            if (m_impl->m_nLoops == 0) {
                m_impl->m_nLoops = -1;
            }
            ASSERT(m_impl->m_nWidth > 0);
            ASSERT(m_impl->m_nHeight > 0);
            ASSERT(m_impl->m_nFrameCount > 0);
            ASSERT(m_impl->m_frames.size() == 1);
            if (m_impl->m_frames.size() == 1) {
                ASSERT(m_impl->m_frames[0] != nullptr);
                ASSERT(m_impl->m_frames[0]->m_pBitmap  != nullptr);
                if (m_impl->m_frames[0]->m_pBitmap != nullptr) {
                    ASSERT(m_impl->m_frames[0]->m_pBitmap->GetWidth() <= m_impl->m_nWidth);
                    ASSERT(m_impl->m_frames[0]->m_pBitmap->GetHeight() <= m_impl->m_nHeight);
                }
            }
        }
        APNG_Destroy(apngData);
    }
    if (!bLoaded) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
    }
    return bLoaded;
}

bool Image_PNG::IsDecodeImageDataEnabled() const
{
    if (!m_impl->m_fileData.empty() &&
        m_impl->m_bLoadAllFrames &&
        (m_impl->m_nFrameCount > 1) &&
        (m_impl->m_frames.size() == 1)) {
        return true;
    }
    return false;
}

void Image_PNG::SetDecodeImageDataStarted()
{
    m_impl->m_bDecodeImageDataStarted = true;
    m_impl->m_bDecodeImageDataAborted = false;
}

bool Image_PNG::DecodeImageData()
{
    if (!IsDecodeImageDataEnabled()) {
        return false;
    }
    std::vector<uint8_t> fileData;
    m_impl->m_fileData.swap(fileData);
    float fImageSizeScale = m_impl->m_fImageSizeScale;
    uint32_t nOutFrameCount = 0;

    //多帧图片延迟解码
    bool bLoaded = false;
    APNGDATA* apngData = LoadAPNG_from_memory((const char*)fileData.data(), fileData.size(), true, nOutFrameCount);
    if (apngData != nullptr) {
        //加载图片数据
        std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> frames;
        if (DecodeImage_APNG(apngData, fImageSizeScale, frames, &(m_impl->m_bDecodeImageDataAborted))) {
            bLoaded = true;
            //记录总帧数            
            ASSERT(m_impl->m_nWidth == ImageUtil::GetScaledImageSize((uint32_t)apngData->nWid, fImageSizeScale));
            ASSERT(m_impl->m_nHeight == ImageUtil::GetScaledImageSize((uint32_t)apngData->nHei, fImageSizeScale));
            ASSERT(m_impl->m_nFrameCount == (int32_t)nOutFrameCount);

            ASSERT(!m_impl->m_bDecodeImageDataFinished);
            ASSERT(m_impl->m_delayFrames.empty());

            m_impl->m_delayFrames.swap(frames);
            m_impl->m_bDecodeImageDataFinished = true;
        }
        APNG_Destroy(apngData);
    }
    return bLoaded;
}

void Image_PNG::SetDecodeImageDataAborted()
{
    m_impl->m_bDecodeImageDataAborted = true;
}

uint32_t Image_PNG::GetWidth() const
{
    return m_impl->m_nWidth;
}

uint32_t Image_PNG::GetHeight() const
{
    return m_impl->m_nHeight;
}

int32_t Image_PNG::GetFrameCount() const
{
    return m_impl->m_nFrameCount;
}

int32_t Image_PNG::GetLoopCount() const
{
    return m_impl->m_nLoops;
}

bool Image_PNG::ReadFrame(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
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
