#include "Image_LOTTIE.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"
#include "duilib/RenderSkia/FontMgr_Skia.h"

#include "duilib/RenderSkia/SkiaHeaderBegin.h"
#include "modules/skottie/include/Skottie.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "duilib/RenderSkia/SkiaHeaderEnd.h"

namespace ui
{
//解码LOTTIE图片数据(解出一帧图片, 不包含图片播放时间这个字段)
static bool DecodeImage_LOTTIE(sk_sp<skottie::Animation>& pSkAnimation,                               
                               uint32_t nImageWidth,
                               uint32_t nImageHeight,
                               int32_t nFrame,
                               AnimationFramePtr& frame)
{
    ASSERT(pSkAnimation != nullptr);
    if (pSkAnimation == nullptr) {
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
    frame.reset();
    pSkAnimation->seekFrame(static_cast<double>(nFrame));
    std::shared_ptr<IBitmap> pBitmap(pRenderFactory->CreateBitmap());
    ASSERT(pBitmap != nullptr);
    if (pBitmap == nullptr) {
        frame.reset();
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
    SkRect dst = SkRect::MakeLTRB(0.0f, 0.0f, (float)nImageWidth, (float)nImageHeight);
    pSkAnimation->render(&canvas, &dst);
    if (!pBitmap->Init(nImageWidth, nImageHeight, skBitmap.getPixels())) {
        pBitmap.reset();
    }
    auto pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
    pFrameData->m_nFrameIndex = nFrame;
    pFrameData->m_nOffsetX = 0; // OffsetX和OffsetY均不需要处理
    pFrameData->m_nOffsetY = 0;
    pFrameData->m_bDataPending = false;
    pFrameData->m_pBitmap = pBitmap;
    frame = pFrameData;
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

    //每帧之间的播放时间间隔，毫秒
    int32_t m_nFrameDelayMs = IMAGE_ANIMATION_DELAY_MS_MIN;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;
};

Image_LOTTIE::Image_LOTTIE()
{
    m_impl = std::make_unique<TImpl>();
}

Image_LOTTIE::~Image_LOTTIE()
{
    m_impl->m_pSkAnimation.reset();
}

bool Image_LOTTIE::LoadImageFromFile(const FilePath& filePath,
                                     float fImageSizeScale,
                                     const UiSize& rcMaxDestRectSize)
{
    ASSERT(!filePath.IsEmpty());
    if (filePath.IsEmpty()) {
        return false;
    }
    std::vector<uint8_t> fileData;
    return LoadImageFromMemoryOrFile(fileData, filePath, fImageSizeScale, rcMaxDestRectSize);
}

bool Image_LOTTIE::LoadImageFromMemory(std::vector<uint8_t>& fileData,
                                       float fImageSizeScale,
                                       const UiSize& rcMaxDestRectSize)
{
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return false;
    }
    FilePath filePath;
    return LoadImageFromMemoryOrFile(fileData, filePath, fImageSizeScale, rcMaxDestRectSize);
}

bool Image_LOTTIE::LoadImageFromMemoryOrFile(std::vector<uint8_t>& fileData,
                                             const FilePath& filePath,
                                             float fImageSizeScale,
                                             const UiSize& rcMaxDestRectSize)
{
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return false;
    }
    IFontMgr* pFontMgr = pRenderFactory->GetFontMgr();
    ASSERT(pFontMgr != nullptr);
    if (pFontMgr == nullptr) {
        return false;
    }
    FontMgr_Skia* pFontMgrSKia = dynamic_cast<FontMgr_Skia*>(pFontMgr);
    ASSERT(pFontMgrSKia != nullptr);
    if (pFontMgrSKia == nullptr) {
        return false;
    }
    sk_sp<SkFontMgr>* pSkFontMgr = (sk_sp<SkFontMgr>*)pFontMgrSKia->GetSkiaFontMgrPtr();
    ASSERT(pSkFontMgr != nullptr);
    if (pSkFontMgr == nullptr) {
        return false;
    }

    m_impl->m_fileData.clear();
    m_impl->m_fileData.swap(fileData);
    m_impl->m_fImageSizeScale = fImageSizeScale;

    // 加载Lottie动画(同时设置字体管理器)
    if (!m_impl->m_fileData.empty()) {
        m_impl->m_pSkAnimation = skottie::Animation::Builder().setFontManager(*pSkFontMgr).make((const char*)m_impl->m_fileData.data(), m_impl->m_fileData.size());
    }
    else if (!filePath.IsEmpty()) {
        std::string jsonFilePath = filePath.NativePathA();
        m_impl->m_pSkAnimation = skottie::Animation::Builder().setFontManager(*pSkFontMgr).makeFromFile((const char*)jsonFilePath.c_str());
    }
    else {
        ASSERT(0);
    }
    if (!m_impl->m_pSkAnimation) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    SkISize imageSize = m_impl->m_pSkAnimation->size().toCeil();
    m_impl->m_nWidth = (uint32_t)imageSize.fWidth;
    m_impl->m_nHeight = (uint32_t)imageSize.fHeight;
    float fScale = fImageSizeScale;
    if (ImageUtil::GetBestImageScale(rcMaxDestRectSize, m_impl->m_nWidth, m_impl->m_nHeight, fImageSizeScale, fScale)) {
        m_impl->m_nWidth = ImageUtil::GetScaledImageSize(m_impl->m_nWidth, fScale);
        m_impl->m_nHeight = ImageUtil::GetScaledImageSize(m_impl->m_nHeight, fScale);
        m_impl->m_fImageSizeScale = fScale;
    }
    else {
        m_impl->m_nWidth = ImageUtil::GetScaledImageSize(m_impl->m_nWidth, fImageSizeScale);
        m_impl->m_nHeight = ImageUtil::GetScaledImageSize(m_impl->m_nHeight, fImageSizeScale);
    }
    m_impl->m_nFrameCount = static_cast<int32_t>(m_impl->m_pSkAnimation->duration() * m_impl->m_pSkAnimation->fps() + 0.5);

    ASSERT(m_impl->m_nWidth > 0);
    ASSERT(m_impl->m_nHeight > 0);
    ASSERT(m_impl->m_nFrameCount > 0);
    if ((m_impl->m_nFrameCount <= 0) || ((int32_t)m_impl->m_nWidth <= 0) || ((int32_t)m_impl->m_nHeight <= 0)) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }
    m_impl->m_nFrameDelayMs = int32_t(m_impl->m_pSkAnimation->duration() * 1000) / m_impl->m_nFrameCount;
    if (m_impl->m_nFrameDelayMs < IMAGE_ANIMATION_DELAY_MS_MIN) {
        m_impl->m_nFrameDelayMs = IMAGE_ANIMATION_DELAY_MS_MIN;
    }
    //循环播放固定为一直播放，因GIF格式无此设置
    m_impl->m_nLoops = -1;
    return true;
}

bool Image_LOTTIE::IsDelayDecodeEnabled() const
{
    //不需要多线程解码图片数据
    return false;
}

bool Image_LOTTIE::IsDelayDecodeFinished() const
{
    return true;
}

uint32_t Image_LOTTIE::GetDecodedFrameIndex() const
{
    if (m_impl->m_nFrameCount > 0) {
        return m_impl->m_nFrameCount - 1;
    }
    return 0;
}

bool Image_LOTTIE::DelayDecode(uint32_t /*nMinFrameIndex*/, std::function<bool(void)> /*IsAborted*/)
{
    //不需要多线程解码图片数据
    return false;
}

bool Image_LOTTIE::MergeDelayDecodeData()
{
    return false;
}

uint32_t Image_LOTTIE::GetWidth() const
{
    return m_impl->m_nWidth;
}

uint32_t Image_LOTTIE::GetHeight() const
{
    return m_impl->m_nHeight;
}

float Image_LOTTIE::GetImageSizeScale() const
{
    return m_impl->m_fImageSizeScale;
}

int32_t Image_LOTTIE::GetFrameCount() const
{
    return m_impl->m_nFrameCount;
}

int32_t Image_LOTTIE::GetLoopCount() const
{
    return m_impl->m_nLoops;
}

bool Image_LOTTIE::IsFrameDataReady(uint32_t /*nFrameIndex*/)
{
    //实时绘制，数据始终可用
    return m_impl->m_pSkAnimation != nullptr;
}

int32_t Image_LOTTIE::GetFrameDelayMs(uint32_t /*nFrameIndex*/)
{
    //每帧之间的播放间隔，为固定值
    return m_impl->m_nFrameDelayMs;
}

bool Image_LOTTIE::ReadFrameData(int32_t nFrameIndex, const UiSize& szDestRectSize, AnimationFrame* pAnimationFrame)
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

    uint32_t nImageWidth = m_impl->m_nWidth;
    uint32_t nImageHeight = m_impl->m_nHeight;
    ASSERT((nImageWidth > 0) && (nImageHeight > 0));
    if ((nImageWidth == 0) || (nImageHeight == 0)) {
        return false;
    }
    if ((szDestRectSize.cx > 0) || (szDestRectSize.cy > 0)) {
        bool bScaled = false;
        float fScale = 1.0f;        
        if ((szDestRectSize.cx > 0) && (szDestRectSize.cy > 0)) {            
            if ((szDestRectSize.cx < (int32_t)nImageWidth) && (szDestRectSize.cy < (int32_t)nImageHeight)) {
                float fScaleX = static_cast<float>(szDestRectSize.cx) / nImageWidth;
                float fScaleY = static_cast<float>(szDestRectSize.cy) / nImageHeight;
                fScale = std::max(fScaleX, fScaleY);
                bScaled = true;
            }
        }
        else if (szDestRectSize.cx > 0) {
            if (szDestRectSize.cx < (int32_t)nImageWidth) {
                fScale = static_cast<float>(szDestRectSize.cx) / nImageWidth;
                bScaled = true;
            }
        }
        else if (szDestRectSize.cy > 0) {
            if (szDestRectSize.cy < (int32_t)nImageHeight) {
                fScale = static_cast<float>(szDestRectSize.cy) / nImageHeight;
                bScaled = true;
            }
        }
        if (bScaled) {
            //等比例缩小（提高绘制速度）
            uint32_t nNewImageWidth = ImageUtil::GetScaledImageSize(nImageWidth, fScale);
            uint32_t nNewImageHeight = ImageUtil::GetScaledImageSize(nImageHeight, fScale);
            if ((nNewImageWidth > 0) && (nNewImageHeight > 0)) {
                nImageWidth = nNewImageWidth;
                nImageHeight = nNewImageHeight;
            }
        }
    }

    AnimationFramePtr frame;
    if (m_impl->m_pSkAnimation != nullptr) {
        if(DecodeImage_LOTTIE(m_impl->m_pSkAnimation,
                              nImageWidth,
                              nImageHeight,
                              nFrameIndex,
                              frame)) {
            if (frame != nullptr) {
                *pAnimationFrame = *frame;
                pAnimationFrame->SetDelayMs(GetFrameDelayMs(nFrameIndex));
            }
        }
    }    
    return frame != nullptr;
}

} //namespace ui
