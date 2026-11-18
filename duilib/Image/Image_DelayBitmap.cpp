#include "Image_DelayBitmap.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Image/ImageUtil.h"
#include "duilib/Render/IRender.h"

namespace ui
{

Image_DelayBitmap::Image_DelayBitmap():
    m_nWidth(0),
    m_nHeight(0),
    m_fImageSizeScale(IMAGE_SIZE_SCALE_NONE),
    m_bAsyncDecode(false),
    m_bDecodeError(false),
    m_bAsyncDecoding(false),
    m_bDelayDecoded(false)
{
}

Image_DelayBitmap::~Image_DelayBitmap() = default;

bool Image_DelayBitmap::InitImage(uint32_t nWidth, uint32_t nHeight, float fImageSizeScale,
                                  bool bAsyncDecode, DecodeFunction decodeFunction)
{
    if ((nWidth == 0) || (nHeight == 0) || !decodeFunction) {
        return false;
    }
    //与IBitmap::Init的处理保持一致：GetWidth()/GetHeight()返回按缩放比例换算后的绘制尺寸，
    //而传入的nWidth/nHeight是原图尺寸（解码函数返回的数据也是原图尺寸）
    m_nWidth = ImageUtil::GetScaledImageSize(nWidth, fImageSizeScale);
    m_nHeight = ImageUtil::GetScaledImageSize(nHeight, fImageSizeScale);
    if ((m_nWidth == 0) || (m_nHeight == 0)) {
        return false;
    }
    m_fImageSizeScale = fImageSizeScale;
    m_bAsyncDecode = bAsyncDecode;
    m_decodeFunction = std::move(decodeFunction);
    return true;
}

uint32_t Image_DelayBitmap::GetWidth() const
{
    return m_nWidth;
}

uint32_t Image_DelayBitmap::GetHeight() const
{
    return m_nHeight;
}

float Image_DelayBitmap::GetImageSizeScale() const
{
    return m_fImageSizeScale;
}

std::shared_ptr<IBitmap> Image_DelayBitmap::GetBitmap(bool* bDecodeError)
{
    GlobalManager::Instance().AssertUIThread();
    if (m_pBitmap != nullptr) {
        if (bDecodeError != nullptr) {
            *bDecodeError = false;
        }
        return m_pBitmap;
    }
    if (!m_bAsyncDecode) {
        //不支持异步解码时，在UI线程中同步解码（延迟到首次绘制时执行）
        m_pBitmap = DecodeToBitmap();
        if (m_pBitmap == nullptr) {
            m_bDecodeError = true;
            if (bDecodeError != nullptr) {
                *bDecodeError = true;
            }
        }
    }
    else if (m_bDecodeError && (bDecodeError != nullptr)) {
        *bDecodeError = true;
    }
    if ((m_pBitmap != nullptr) && (bDecodeError != nullptr)) {
        *bDecodeError = false;
    }
    return m_pBitmap;
}

bool Image_DelayBitmap::IsDelayDecodeEnabled() const
{
    return m_bAsyncDecode && !m_bDelayDecoded && !m_bDecodeError && (m_decodeFunction != nullptr);
}

bool Image_DelayBitmap::IsDelayDecodeFinished() const
{
    if (m_bAsyncDecoding) {
        return false;
    }
    return (m_pBitmap != nullptr) || m_bDelayDecoded || m_bDecodeError;
}

uint32_t Image_DelayBitmap::GetDecodedFrameIndex() const
{
    //单帧图片：位图生成完成才算解码出一帧
    //(m_pBitmap只在UI线程中读写；m_bDelayDecoded/m_bDecodeError是原子变量)
    if ((m_pBitmap != nullptr) || (m_bDelayDecoded && !m_bDecodeError)) {
        return 0;
    }
    //没有已解码的帧
    return (uint32_t)-1;
}

void Image_DelayBitmap::SetAsyncDecoding(bool bDecoding)
{
    m_bAsyncDecoding = bDecoding;
}

bool Image_DelayBitmap::DelayDecode(uint32_t /*nMinFrameIndex*/, std::function<bool(void)> IsAborted, bool* bDecodeError)
{
    if (!m_bAsyncDecode || m_bDecodeError || (m_decodeFunction == nullptr)) {
        return false;
    }
    if ((IsAborted != nullptr) && IsAborted()) {
        return false;
    }
    //正常调度时，该状态已由UI线程在提交任务前置位；此处置位用于兼容直接调用DelayDecode的场景
    m_bAsyncDecoding = true;
    ASSERT(m_pDelayBitmap == nullptr);
    if (m_pDelayBitmap == nullptr) {
        m_pDelayBitmap = DecodeToBitmap();
        if (m_pDelayBitmap == nullptr) {
            m_bDecodeError = true;
            if (bDecodeError != nullptr) {
                *bDecodeError = true;
            }
        }
        //置位后，UI线程才能安全地读取m_pDelayBitmap
        m_bDelayDecoded = true;
    }
    m_bAsyncDecoding = false;
    return m_pDelayBitmap != nullptr;
}

bool Image_DelayBitmap::MergeDelayDecodeData()
{
    GlobalManager::Instance().AssertUIThread();
    bool bRet = false;
    bool bDecodeFinished = m_bDecodeError || (m_pBitmap != nullptr);
    if ((m_pDelayBitmap != nullptr) && (m_pBitmap == nullptr)) {
        m_pBitmap = m_pDelayBitmap;
        m_pDelayBitmap.reset();

        bDecodeFinished = true;
        bRet = true;
    }
    if (bDecodeFinished) {
        //解码完成，释放解码源数据
        m_decodeFunction = nullptr;
    }
    return bRet;
}

std::shared_ptr<IBitmap> Image_DelayBitmap::DecodeToBitmap()
{
    if (!m_decodeFunction) {
        return nullptr;
    }
    UiImageData imageData;
    if (!m_decodeFunction(imageData)) {
        return nullptr;
    }
    if (imageData.m_imageData.empty() || (imageData.m_imageWidth == 0) || (imageData.m_imageHeight == 0)) {
        return nullptr;
    }
    if (imageData.m_imageData.size() != (size_t)imageData.m_imageWidth * imageData.m_imageHeight * 4) {
        ASSERT(0);
        return nullptr;
    }
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return nullptr;
    }
    std::shared_ptr<IBitmap> pBitmap;
    IBitmap* pNewBitmap = pRenderFactory->CreateBitmap();
    ASSERT(pNewBitmap != nullptr);
    if (pNewBitmap == nullptr) {
        return nullptr;
    }
    if (!pNewBitmap->Init(imageData.m_imageWidth, imageData.m_imageHeight,
                          imageData.m_imageData.data(), m_fImageSizeScale)) {
        delete pNewBitmap;
        return nullptr;
    }
    pBitmap.reset(pNewBitmap);
    return pBitmap;
}

} //namespace ui
