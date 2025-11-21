#include "Image_GIF.h"
#include "FrameSequence_gif.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

#include "duilib/third_party/giflib/gif_lib.h"

namespace ui
{
//内存数据源结构体：存储内存中的 GIF 数据、总大小和当前读取位置
typedef struct {
    const unsigned char* data;  // 指向内存中的 GIF 原始数据
    size_t size;                // 数据总字节数
    size_t position;            // 当前读取偏移量（从 0 开始）
} UiGifMemorySource;

/** 自定义内存读取函数
* @param gif: giflib 内部文件句柄
* @param buf: 接收数据的缓冲区
* @param len: 请求读取的字节数
* @return: 实际读取的字节数（0 表示已读完）
*/
static int UiGifMemoryReadFunc(GifFileType* gif, GifByteType* buf, int len)
{
    if (gif == nullptr || buf == nullptr || len <= 0) {
        return 0;
    }

    // 从 UserData 中获取内存数据源
    UiGifMemorySource* source = (UiGifMemorySource*)gif->UserData;
    if (source == nullptr || source->data == nullptr || source->position >= source->size) {
        return 0;  // 数据已耗尽
    }

    // 计算实际可读取的字节数（避免越界）
    int bytes_to_read = (len > (int)(source->size - source->position)) ? (int)(source->size - source->position) : len;

    // 从内存复制数据到缓冲区
    memcpy(buf, source->data + source->position, bytes_to_read);
    source->position += bytes_to_read;

    return bytes_to_read;
}

/** 初始化解码器：从内存数据创建 GIF 解码句柄
* @param data: 内存中的 GIF 原始数据
* @param size: 数据总字节数
* @param error_code: 输出错误码（参考 D_GIF_* 常量）
* @return: 成功返回 GifFileType 句柄，失败返回 nullptr
*/
static GifFileType* UiGifInitDecoder(const unsigned char* data, size_t size, int* error_code)
{
    // 校验输入参数
    if (data == nullptr || size == 0 || error_code == nullptr) {
        if (error_code != nullptr) {
            *error_code = D_GIF_ERR_OPEN_FAILED;
        }
        return nullptr;
    }

    // 初始化内存数据源
    UiGifMemorySource* source = (UiGifMemorySource*)malloc(sizeof(UiGifMemorySource));
    if (source == nullptr) {
        *error_code = D_GIF_ERR_NOT_ENOUGH_MEM;
        return nullptr;
    }
    source->data = data;
    source->size = size;
    source->position = 0;

    // 调用 DGifOpen 创建解码句柄
    GifFileType* gif = DGifOpen(source, UiGifMemoryReadFunc, error_code);
    if (gif == nullptr) {
        free(source);
        return nullptr;
    }
    return gif;
}

/** 释放解码器资源（包括内存数据源和 GIF 句柄）
 * @param gif: 需释放的 GifFileType 句柄
 * @param error_code: 输出关闭时的错误码（可传 nullptr）
 */
static void UiGifFreeDecoder(GifFileType* gif, int* error_code)
{
    if (gif == nullptr) {
        return;
    }

    // 释放内存数据源（存储在 UserData 中）
    if (gif->UserData != nullptr) {
        free(gif->UserData);
        gif->UserData = nullptr;
    }

    // 关闭 GIF 句柄
    DGifCloseFile(gif, error_code);
}

// RGBA 像素结构体
struct UiGifRGBA {
    uint8_t r, g, b, a;
};

/** 将GIF文件解析为逐帧RGBA数据
 * @param gif gif图片数据解码器
 * @param nFrameIndex 加载哪一帧
 * @param fImageSizeScale 图片的缩放比例
 * @param canvas 画布
 * @param nPrevFrameIndex 上一帧的索引号
 * @return 返回创建的帧数据
 */
static AnimationFramePtr UiGifToRgbaFrames(FrameSequence_gif& gif,
                                           int32_t nFrameIndex,
                                           float fImageSizeScale,
                                           std::vector<UiGifRGBA>& canvas,
                                           int32_t& nPrevFrameIndex)
{
    ASSERT((nFrameIndex >= 0) && (nFrameIndex < gif.GetFrameCount()));
    if ((nFrameIndex < 0) || (nFrameIndex >= gif.GetFrameCount())) {
        return nullptr;
    }

    ASSERT(nPrevFrameIndex == (nFrameIndex - 1));
    if (nPrevFrameIndex != (nFrameIndex - 1)) {
        return nullptr;
    }

    // 检查GIF宽度和高度是否有效
    ASSERT((gif.GetWidth() > 0) && (gif.GetHeight() > 0));
    if ((gif.GetWidth() <= 0) || (gif.GetHeight() <= 0)) {
        return nullptr;
    }

    // 获取渲染工厂实例
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return nullptr;
    }

    const int nImageWidth = gif.GetWidth();
    const int nImageHeight = gif.GetHeight();
    const int outputPixelStride = nImageWidth;
    const int canvas_pixel_count = nImageWidth * nImageHeight;
    if ((int)canvas.size() != canvas_pixel_count) {
        canvas.resize(canvas_pixel_count); // 初始化画布
    }

    // 预创建当前帧的位图对象
    std::shared_ptr<IBitmap> pBitmap(pRenderFactory->CreateBitmap());
    if (pBitmap == nullptr) {
        return nullptr;
    }

    AnimationFramePtr pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
    pFrameData->m_nFrameIndex = nFrameIndex;
    pFrameData->m_nOffsetX = 0;
    pFrameData->m_nOffsetY = 0;
    pFrameData->m_bDataPending = false;
    pFrameData->m_pBitmap = pBitmap;

    // 获取当前帧的数据
    gif.DrawFrame(nFrameIndex, (Color8888*)canvas.data(), outputPixelStride, nPrevFrameIndex);

    // 更新位图数据
#ifdef DUILIB_BUILD_FOR_WIN
    std::vector<UiGifRGBA> canvasWin = canvas;
    //交换R和G，Windows平台使用ABGR格式
    for (int y = 0; y < nImageHeight; y++) {
        for (int x = 0; x < nImageWidth; x++) {
            UiGifRGBA& pixelColor = canvasWin[y * outputPixelStride + x];
            std::swap(pixelColor.b, pixelColor.r);
        }
    }
    pFrameData->m_pBitmap->Init(nImageWidth, nImageHeight, canvasWin.data(), fImageSizeScale);
#else
    pFrameData->m_pBitmap->Init(nImageWidth, nImageHeight, canvas.data(), fImageSizeScale);
#endif
    
    nPrevFrameIndex = nFrameIndex;
    return pFrameData;
}

struct Image_GIF::TImpl
{
    //图片文件路径
    FilePath m_imageFilePath;

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

    //图片数据出错时，是否允许断言
    bool m_bAssertEnabled = true;

    //是否存在图片数据解码错误
    bool m_bDecodeError = false;

    //缩放比例
    float m_fImageSizeScale = IMAGE_SIZE_SCALE_NONE;

    //各个图片帧的数据
    std::vector<AnimationFramePtr> m_frames;

    //每一帧的播放延迟时间，毫秒
    std::vector<int32_t> m_framesDelayMs;

public:
    //各个图片帧的数据(延迟解码的数据)
    std::vector<AnimationFramePtr> m_delayFrames;

    //是否支持异步线程解码图片数据
    bool m_bAsyncDecode = false;

    //是否正在解码图片数据
    std::atomic<bool> m_bAsyncDecoding = false;

    //加载后的句柄
    GifFileType* m_gifDecoder = nullptr;

    //gif解码的实现封装
    FrameSequence_gif m_gifFrameSequence;

    //GIF图片绘制的画布
    std::vector<UiGifRGBA> m_gifCanvas;

    //上一帧的索引号
    int32_t m_nLastFrameIndex = -1;

public:
    //从已经打开的文件句柄，初始化
    bool InitImageData(GifFileType* dec,
                       std::vector<uint8_t>& fileData,
                       bool bLoadAllFrames,
                       bool bAsyncDecode,
                       float fImageSizeScale,
                       const UiSize& rcMaxDestRectSize)
    {
        ASSERT(dec != nullptr);
        if (dec == nullptr) {
            return false;
        }
        if (!m_gifFrameSequence.Init(dec)) {
            m_gifFrameSequence.Clear();
            UiGifFreeDecoder(dec, nullptr);
            //加载失败时，需要恢复原文件数据
            m_fileData.swap(fileData);
            return false;
        }

        m_fImageSizeScale = fImageSizeScale;
        m_bLoadAllFrames = bLoadAllFrames;
        m_bAsyncDecode = bAsyncDecode;

        m_nWidth = (uint32_t)m_gifFrameSequence.GetWidth();
        m_nHeight = (uint32_t)m_gifFrameSequence.GetHeight();
        m_nFrameCount = (int32_t)m_gifFrameSequence.GetFrameCount();

        if (m_bAssertEnabled) {
            ASSERT(m_nWidth > 0);
            ASSERT(m_nHeight > 0);
            ASSERT(m_nFrameCount > 0);
        }
        if ((m_nFrameCount <= 0) || ((int32_t)m_nWidth <= 0) || ((int32_t)m_nHeight <= 0)) {
            //图片格式正确，但图片数据有错误(不还原图片数据)
            m_bDecodeError = true;
            UiGifFreeDecoder(dec, nullptr);
            m_gifFrameSequence.Clear();
            return false;
        }
        

        float fScale = fImageSizeScale;
        if (ImageUtil::GetBestImageScale(rcMaxDestRectSize, m_nWidth, m_nHeight, fImageSizeScale, fScale)) {
            m_nWidth = ImageUtil::GetScaledImageSize(m_nWidth, fScale);
            m_nHeight = ImageUtil::GetScaledImageSize(m_nHeight, fScale);
            m_fImageSizeScale = fScale;
        }
        else {
            m_nWidth = ImageUtil::GetScaledImageSize(m_nWidth, fImageSizeScale);
            m_nHeight = ImageUtil::GetScaledImageSize(m_nHeight, fImageSizeScale);
        }
        ASSERT(m_nWidth > 0);
        ASSERT(m_nHeight > 0);
        ASSERT(m_nFrameCount > 0);

        if ((m_nFrameCount <= 0) || ((int32_t)m_nWidth <= 0) || ((int32_t)m_nHeight <= 0)) {
            //加载失败
            m_bDecodeError = true;
            UiGifFreeDecoder(dec, nullptr);
            m_gifFrameSequence.Clear();
            return false;
        }
        //循环播放次数
        m_nLoops = m_gifFrameSequence.GetDefaultLoopCount();
        if (m_nLoops < 1) {
            m_nLoops = -1;
        }

        //解出每一帧的播放时间
        m_framesDelayMs.clear();
        for (int frame_idx = 0; frame_idx < m_nFrameCount; ++frame_idx) {
            GraphicsControlBlock gcb;
            memset(&gcb, 0, sizeof(GraphicsControlBlock));
            DGifSavedExtensionToGCB(dec, frame_idx, &gcb);
            m_framesDelayMs.push_back(gcb.DelayTime * 10);
        }

        m_gifDecoder = dec;
        m_gifCanvas.clear();
        m_nLastFrameIndex = -1;
        return true;
    }

    //清理资源
    void ClearImageData()
    {
        if (m_gifDecoder != nullptr) {
            UiGifFreeDecoder(m_gifDecoder, nullptr);
            m_gifDecoder = nullptr;
        }
        m_gifFrameSequence.Clear();
        std::vector<uint8_t> fileData;
        m_fileData.swap(fileData);

        std::vector<UiGifRGBA> gifCanvas;
        m_gifCanvas.swap(gifCanvas);
    }

    //解码是否完成
    bool IsDecodeFinished() const
    {
        if (((int32_t)m_frames.size() == m_nFrameCount) || m_bDecodeError) {
            return true;
        }
        return false;
    }
};

Image_GIF::Image_GIF()
{
    m_impl = std::make_unique<TImpl>();
}

Image_GIF::~Image_GIF()
{
    m_impl->ClearImageData();
}

bool Image_GIF::LoadImageFile(std::vector<uint8_t>& fileData,
                              const FilePath& imageFilePath,
                              bool bLoadAllFrames,
                              bool bAsyncDecode,
                              float fImageSizeScale,
                              const UiSize& rcMaxDestRectSize,
                              bool bAssertEnabled)
{
    ASSERT(!fileData.empty() || !imageFilePath.IsEmpty());
    if (fileData.empty() && imageFilePath.IsEmpty()) {
        return false;
    }
    m_impl->m_bAssertEnabled = bAssertEnabled;
    m_impl->m_imageFilePath = imageFilePath;
    m_impl->m_fileData.clear();
    m_impl->m_fileData.swap(fileData);

    if (!m_impl->m_fileData.empty()) {
        int nErrorCode = 0;
        GifFileType* dec = UiGifInitDecoder(m_impl->m_fileData.data(), m_impl->m_fileData.size(), &nErrorCode);
        if (dec == nullptr) {
            //加载失败时，需要恢复原文件数据
            m_impl->m_fileData.swap(fileData);
            return false;
        }
        return m_impl->InitImageData(dec, fileData, bLoadAllFrames, bAsyncDecode, fImageSizeScale, rcMaxDestRectSize);
    }
    else {
        DStringA gifFileName = imageFilePath.NativePathA();
        ASSERT(!gifFileName.empty());
        int nErrorCode = 0;
        GifFileType* dec = DGifOpenFileName(gifFileName.c_str(), &nErrorCode);
        if (dec == nullptr) {
            return false;
        }
        std::vector<uint8_t> emptyFileData;
        return m_impl->InitImageData(dec, emptyFileData, bLoadAllFrames, bAsyncDecode, fImageSizeScale, rcMaxDestRectSize);
    }
}

bool Image_GIF::IsDelayDecodeEnabled() const
{
    if (m_impl->m_bAsyncDecode && (m_impl->m_gifDecoder != nullptr)) {
        return true;
    }
    return false;
}

bool Image_GIF::IsDelayDecodeFinished() const
{
    if (m_impl->m_bAsyncDecoding) {
        return false;
    }
    if (m_impl->m_bDecodeError || (m_impl->m_gifDecoder == nullptr)) {
        return true;
    }
    return (int32_t)(m_impl->m_frames.size() + m_impl->m_delayFrames.size()) == m_impl->m_nFrameCount;
}

uint32_t Image_GIF::GetDecodedFrameIndex() const
{
    if (m_impl->m_frames.empty()) {
        return 0;
    }
    else {
        return (uint32_t)m_impl->m_frames.size() - 1;
    }
}

bool Image_GIF::DelayDecode(uint32_t nMinFrameIndex, std::function<bool(void)> IsAborted, bool* bDecodeError)
{
    if (!IsDelayDecodeEnabled()) {
        return false;
    }
    if (m_impl->m_bAsyncDecoding) {
        return false;
    }
    ASSERT(m_impl->m_nFrameCount > 0);
    if (m_impl->m_nFrameCount <= 0) {
        m_impl->m_bDecodeError = true;
    }
    if (m_impl->m_bDecodeError) {
        if (bDecodeError != nullptr) {
            *bDecodeError = true;
        }
        return false;
    }
    m_impl->m_bAsyncDecoding = true;
    const size_t nFrameCount = (size_t)m_impl->m_nFrameCount;

    bool bRet = true;
    float fImageSizeScale = m_impl->m_fImageSizeScale;    
    while (((IsAborted == nullptr) || !IsAborted()) &&
           (nMinFrameIndex >= (m_impl->m_frames.size() + m_impl->m_delayFrames.size())) &&
           ((m_impl->m_frames.size() + m_impl->m_delayFrames.size()) < nFrameCount)) {        
        //每次解码一帧图片
        const int32_t nFrameIndex = (int32_t)(m_impl->m_delayFrames.size() + m_impl->m_frames.size());
        AnimationFramePtr pNewAnimationFrame;
        pNewAnimationFrame = UiGifToRgbaFrames(m_impl->m_gifFrameSequence,
                                               nFrameIndex,
                                               fImageSizeScale,
                                               m_impl->m_gifCanvas,
                                               m_impl->m_nLastFrameIndex);
        if (pNewAnimationFrame != nullptr) {
            pNewAnimationFrame->SetDelayMs(GetFrameDelayMs(nFrameIndex));
            m_impl->m_delayFrames.push_back(pNewAnimationFrame);
        }
        else {
            bRet = false;
            m_impl->m_bDecodeError = true;
            if (bDecodeError != nullptr) {
                *bDecodeError = true;
            }
            break;
        }
    }

    m_impl->m_bAsyncDecoding = false;
    return bRet;
}

bool Image_GIF::MergeDelayDecodeData()
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
        bool bDecodeFinished = (int32_t)m_impl->m_frames.size() == m_impl->m_nFrameCount;
        if (bDecodeFinished || m_impl->m_bDecodeError) {
            m_impl->ClearImageData();
        }
    }
    return bRet;
}

uint32_t Image_GIF::GetWidth() const
{
    return m_impl->m_nWidth;
}

uint32_t Image_GIF::GetHeight() const
{
    return m_impl->m_nHeight;
}

float Image_GIF::GetImageSizeScale() const
{
    return m_impl->m_fImageSizeScale;
}

int32_t Image_GIF::GetFrameCount() const
{
    return m_impl->m_nFrameCount;
}

int32_t Image_GIF::GetLoopCount() const
{
    return m_impl->m_nLoops;
}

bool Image_GIF::IsFrameDataReady(uint32_t nFrameIndex)
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

int32_t Image_GIF::GetFrameDelayMs(uint32_t nFrameIndex)
{
    if (nFrameIndex < m_impl->m_frames.size()) {
        auto pAnimationFrame = m_impl->m_frames[nFrameIndex];
        if (pAnimationFrame != nullptr) {
            return pAnimationFrame->GetDelayMs();
        }
    }
    if (nFrameIndex < m_impl->m_framesDelayMs.size()) {
        IAnimationImage::AnimationFrame frame;
        frame.SetDelayMs(m_impl->m_framesDelayMs[nFrameIndex]);
        return frame.GetDelayMs();
    }
    return IMAGE_ANIMATION_DELAY_MS;
}

bool Image_GIF::ReadFrameData(int32_t nFrameIndex, const UiSize& /*szDestRectSize*/, AnimationFrame* pAnimationFrame)
{
    GlobalManager::Instance().AssertUIThread();
    ASSERT(pAnimationFrame != nullptr);
    if (pAnimationFrame == nullptr) {
        return false;
    }
    pAnimationFrame->m_bDataPending = true;
    pAnimationFrame->m_bDataError = false;
    ASSERT((nFrameIndex >= 0) && (nFrameIndex < m_impl->m_nFrameCount));
    if ((nFrameIndex < 0) || (nFrameIndex >= m_impl->m_nFrameCount)) {
        return false;
    }
    ASSERT(m_impl->m_nFrameCount > 0);
    if (m_impl->m_nFrameCount <= 0) {
        m_impl->m_bDecodeError = true;
        pAnimationFrame->m_bDataError = true;
        return false;
    }
    if (m_impl->m_bDecodeError) {
        pAnimationFrame->m_bDataError = true;
        return false;
    }

    if (!m_impl->m_bAsyncDecode) {
        //同步解码的情况, 解码所需要的帧
        while ((nFrameIndex >= (int32_t)m_impl->m_frames.size()) &&
               ((int32_t)m_impl->m_frames.size() < m_impl->m_nFrameCount)) {
            ASSERT(m_impl->m_delayFrames.empty());
            uint32_t nInitFrameIndex = (uint32_t)m_impl->m_frames.size();
            float fImageSizeScale = m_impl->m_fImageSizeScale;

            //一次解码一帧图片
            AnimationFramePtr pNewAnimationFrame;
            pNewAnimationFrame = UiGifToRgbaFrames(m_impl->m_gifFrameSequence,
                                                   nInitFrameIndex,
                                                   fImageSizeScale,
                                                   m_impl->m_gifCanvas,
                                                   m_impl->m_nLastFrameIndex);

            if (pNewAnimationFrame != nullptr) {
                pNewAnimationFrame->SetDelayMs(GetFrameDelayMs(nFrameIndex));
                m_impl->m_frames.push_back(pNewAnimationFrame);
            }
            else {
                //图片解码错误
                m_impl->m_bDecodeError = true;
                pAnimationFrame->m_bDataError = true;
                break;
            }
        }

        if (m_impl->IsDecodeFinished()) {
            m_impl->ClearImageData();
        }
        else if (!m_impl->m_bDecodeError) {
            ASSERT((nFrameIndex < (int32_t)m_impl->m_frames.size()));
            if ((nFrameIndex >= (int32_t)m_impl->m_frames.size())) {
                pAnimationFrame->m_bDataError = true;
                return false;
            }
        }
    }
    else {
        //合并数据
        MergeDelayDecodeData();
    }
    bool bRet = false;
    if (nFrameIndex < (int32_t)m_impl->m_frames.size()) {
        AnimationFramePtr pFrameData = m_impl->m_frames[nFrameIndex];
        if (pFrameData != nullptr) {
            ASSERT(pFrameData->m_nFrameIndex == nFrameIndex);
            *pAnimationFrame = *pFrameData;
            pAnimationFrame->m_bDataPending = false;
            ASSERT(pAnimationFrame->m_pBitmap != nullptr);
            bRet = true;
        }
        else {
            m_impl->m_bDecodeError = true;
            pAnimationFrame->m_bDataError = true;
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
            m_impl->m_bDecodeError = true;
            pAnimationFrame->m_bDataError = true;
        }
    }
    else {
        m_impl->m_bDecodeError = true;
        pAnimationFrame->m_bDataError = true;
    }
    return bRet;
}

} //namespace ui
