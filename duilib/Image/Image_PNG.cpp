#include "Image_PNG.h"
#include "duilib/Image/APngDecoder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

#include <vector>
#include <cmath>
#include <fstream>

namespace ui
{
//PNG/APNG的解码功能封装
namespace ReadPngHeader
{
    struct FrameDuration {
        uint32_t sequence;      // 帧序号
        uint16_t delay_num;     // 延迟分子
        uint16_t delay_den;     // 延迟分母
        uint32_t duration_ms;   // 转换后的毫秒数
    };

    struct PngImageInfo {
        uint32_t width;         //图片宽度
        uint32_t height;        //图片高度
        bool is_apng;           //true表示为APNG图片，false表示为普通PNG图片
        uint32_t frame_count;   //图片总帧数
        int32_t  loop_count;    //播放循环次数
        std::vector<FrameDuration> frames; //每帧的播放延迟时间
    };

    // 自定义块回调函数
    // 注意：返回值类型为 int，用于指示是否保留块
    static int read_chunk_callback(png_structp png_ptr, png_unknown_chunkp chunk)
    {
        // libpng 1.6 中通过返回值控制是否保留块
        int keep = 0;

        if ((chunk->size == 26) && (memcmp(chunk->name, "fcTL", 4) == 0)) {
            PngImageInfo* imageInfo = (PngImageInfo*)png_get_user_chunk_ptr(png_ptr);
            FrameDuration fd;
            // 解析 fcTL 块数据（APNG 帧控制块）
            fd.sequence = png_get_uint_32(chunk->data);                  // 0-3字节：帧序号
            fd.delay_num = png_get_uint_16(chunk->data + 20);             // 20-21字节：延迟分子
            fd.delay_den = png_get_uint_16(chunk->data + 22);             // 22-23字节：延迟分母

            // 计算毫秒数（四舍五入）
            if (fd.delay_den == 0) {
                fd.duration_ms = fd.delay_num * 10;  // 默认为100分母（1000/100=10）
            }
            else {
                fd.duration_ms = static_cast<unsigned int>(
                    std::round(static_cast<double>(fd.delay_num) * 1000 / fd.delay_den)
                    );
            }
            if (fd.duration_ms == 0) {
                fd.duration_ms = 100; //默认值
            }
            if (imageInfo != nullptr) {
                imageInfo->frames.push_back(fd);
                keep = 1; // 保留该块
            }
        }
        return 0; // 返回1表示保留块，0表示不保留
    }

    // 从内存数据解析（重载版本）
    struct PngBuffer {
        const uint8_t* data;
        size_t size;
        size_t offset;
    };

    //从内存数据读取数据的回调函数
    static void png_read_mem_callback(png_structp png, png_bytep data, png_size_t length) {
        PngBuffer* buf = static_cast<PngBuffer*>(png_get_io_ptr(png));
        if (buf->offset + length > buf->size) {
            png_error(png, "Read error");
            return;
        }
        memcpy(data, buf->data + buf->offset, length);
        buf->offset += length;
    }

    //从内存数据加载图片信息
    bool load_apng_image_info(const std::vector<uint8_t>& fileData, PngImageInfo& pngImageInfo)
    {
        pngImageInfo.width = 0;
        pngImageInfo.height = 0;
        pngImageInfo.is_apng = false;
        pngImageInfo.frame_count = 0;
        pngImageInfo.loop_count = 0;
        pngImageInfo.frames.clear();
        if (fileData.size() < 8) {
            return false;  // 至少需要8字节签名
        }

        // 检查PNG签名
        if (png_sig_cmp(reinterpret_cast<const png_byte*>(fileData.data()), 0, 8) != 0) {
            return false;
        }

        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            return false;
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_read_struct(&png, nullptr, nullptr);
            return false;
        }
#pragma warning (push)
#pragma warning (disable: 4611)
        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &info, nullptr);
            return false;
        }
#pragma warning (pop)

        // 设置内存读取回调和用户数据
        PngBuffer buf = { fileData.data(), fileData.size(), 0 };
        png_set_read_fn(png, &buf, png_read_mem_callback);

        // 注册用户块回调
        png_set_read_user_chunk_fn(png, &pngImageInfo, read_chunk_callback);
        const png_byte chunk_name[] = "fcTL";
        png_set_keep_unknown_chunks(png, PNG_HANDLE_CHUNK_ALWAYS, chunk_name, 1);

        png_read_info(png, info);
        png_read_update_info(png, info);

        pngImageInfo.width = info->width;
        pngImageInfo.height = info->height;

        if (!png_get_valid(png, info, PNG_INFO_acTL)) {
            //单帧图片
            pngImageInfo.frame_count = 1;
            pngImageInfo.loop_count = 0;
            pngImageInfo.is_apng = false;
        }
        else {            
            pngImageInfo.is_apng = true;
        }
        png_read_end(png, info);
        if (pngImageInfo.is_apng) {
            pngImageInfo.frame_count = (uint32_t)pngImageInfo.frames.size();
            pngImageInfo.loop_count = png_get_num_plays(png, info);
        }
        png_destroy_read_struct(&png, &info, nullptr);

        ASSERT((pngImageInfo.width > 0) && (pngImageInfo.height > 0));
        if ((pngImageInfo.width <= 0) || (pngImageInfo.height <= 0)) {
            return false;
        }
        ASSERT(pngImageInfo.frame_count > 0);
        if (pngImageInfo.frame_count <= 0) {
            return false;
        }
        if (pngImageInfo.frame_count > 1) {
            //多帧图片
            ASSERT(pngImageInfo.frame_count == pngImageInfo.frames.size());
            if (pngImageInfo.frame_count != pngImageInfo.frames.size()) {
                return false;
            }
            if (pngImageInfo.loop_count <= 0) {
                pngImageInfo.loop_count = -1;
            }
        }
        return true;
    }

    // 从文件读取数据的回调函数
    static void png_read_file_callback(png_structp png, png_bytep data, png_size_t length) {
        std::ifstream* fp = static_cast<std::ifstream*>(png_get_io_ptr(png));
        fp->read(reinterpret_cast<char*>(data), length);
        if (!fp->good()) {
            png_error(png, "Read error");
        }
    }

    //从文件加载图片信息
    bool load_apng_image_info(const std::string& filePath, PngImageInfo& pngImageInfo)
    {
        // 初始化输出结构
        pngImageInfo.width = 0;
        pngImageInfo.height = 0;
        pngImageInfo.is_apng = false;
        pngImageInfo.frame_count = 0;
        pngImageInfo.loop_count = 0;
        pngImageInfo.frames.clear();

        // 打开文件
        std::ifstream fp(filePath, std::ios::binary);
        if (!fp) {
            return false;
        }

        // 检查 PNG 签名（8字节）
        png_byte sig[8] = {0};
        fp.read(reinterpret_cast<char*>(sig), 8);
        if (!fp || png_sig_cmp(sig, 0, 8) != 0) {
            return false;
        }

        // 创建 png_struct 和 png_info
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            return false;
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_read_struct(&png, nullptr, nullptr);
            return false;
        }

        // 设置错误处理（libpng 用 setjmp/longjmp）
#pragma warning(push)
#pragma warning(disable: 4611)
        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &info, nullptr);
            return false;
        }
#pragma warning(pop)

        // 设置文件读取回调
        png_set_read_fn(png, &fp, png_read_file_callback);

        // 告诉 libpng 我们已经读取了 8 字节签名
        png_set_sig_bytes(png, 8);

        // 注册用户块回调
        png_set_read_user_chunk_fn(png, &pngImageInfo, read_chunk_callback);
        const png_byte chunk_name[] = "fcTL";
        png_set_keep_unknown_chunks(png, PNG_HANDLE_CHUNK_ALWAYS, chunk_name, 1);

        png_read_info(png, info);
        png_read_update_info(png, info);

        pngImageInfo.width = info->width;
        pngImageInfo.height = info->height;

        if (!png_get_valid(png, info, PNG_INFO_acTL)) {
            //单帧图片
            pngImageInfo.frame_count = 1;
            pngImageInfo.loop_count = 0;
            pngImageInfo.is_apng = false;
        }
        else {
            pngImageInfo.is_apng = true;
        }
        png_read_end(png, info);
        if (pngImageInfo.is_apng) {
            pngImageInfo.frame_count = (uint32_t)pngImageInfo.frames.size();
            pngImageInfo.loop_count = png_get_num_plays(png, info);
        }
        png_destroy_read_struct(&png, &info, nullptr);

        ASSERT((pngImageInfo.width > 0) && (pngImageInfo.height > 0));
        if ((pngImageInfo.width <= 0) || (pngImageInfo.height <= 0)) {
            return false;
        }
        ASSERT(pngImageInfo.frame_count > 0);
        if (pngImageInfo.frame_count <= 0) {
            return false;
        }
        if (pngImageInfo.frame_count > 1) {
            //多帧图片
            ASSERT(pngImageInfo.frame_count == pngImageInfo.frames.size());
            if (pngImageInfo.frame_count != pngImageInfo.frames.size()) {
                return false;
            }
            if (pngImageInfo.loop_count <= 0) {
                pngImageInfo.loop_count = -1;
            }
        }
        return true;
    }
}

struct Image_PNG::TImpl
{
    //文件数据
    std::vector<uint8_t> m_fileData;

    //文件路径
    FilePath m_filePath;

    //图片宽度(按照m_fImageSizeScale缩放过的值)
    uint32_t m_nWidth = 0;

    //图片高度(按照m_fImageSizeScale缩放过的值)
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
    std::vector<AnimationFramePtr> m_frames;

    //每一帧的播放延迟时间，毫秒
    std::vector<int32_t> m_framesDelayMs;

public:
    //图片数据解码器
    std::unique_ptr<APngDecoder> m_pImageDecoder;

    //是否支持异步线程解码图片数据
    bool m_bAsyncDecode = false;

    //是否正在解码图片数据
    std::atomic<bool> m_bAsyncDecoding = false;

    //各个图片帧的数据(延迟解码的数据)
    std::vector<AnimationFramePtr> m_delayFrames;

public:
    //从已经加载成功的文件初始化图片信息
    bool InitImageData(std::vector<uint8_t>& fileData,
                       const ReadPngHeader::PngImageInfo& pngImageInfo,
                       bool bLoadAllFrames,
                       bool bAsyncDecode,
                       float fImageSizeScale,
                       const UiSize& rcMaxDestRectSize)
    {
        m_fImageSizeScale = fImageSizeScale;
        m_bLoadAllFrames = bLoadAllFrames;
        m_bAsyncDecode = bAsyncDecode;

        //只加载关键信息，不解码图片数据(记录总帧数和宽高)
        m_nWidth = pngImageInfo.width;
        m_nHeight = pngImageInfo.height;
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
        m_nFrameCount = pngImageInfo.frame_count;
        m_nLoops = pngImageInfo.loop_count;
        if (m_nLoops <= 0) {
            m_nLoops = -1;
        }
        ASSERT(m_nWidth > 0);
        ASSERT(m_nHeight > 0);
        ASSERT(m_nFrameCount > 0);

        bool bLoaded = true;
        if ((m_nFrameCount <= 0) || ((int32_t)m_nWidth <= 0) || ((int32_t)m_nHeight <= 0)) {
            bLoaded = false;
        }
        else if (bLoadAllFrames) {
            //支持加载多帧
            m_framesDelayMs.clear();
            if (pngImageInfo.frame_count > 1) {
                for (const ReadPngHeader::FrameDuration& frameDuration : pngImageInfo.frames) {
                    AnimationFrame animFrame;
                    animFrame.SetDelayMs(frameDuration.duration_ms);
                    m_framesDelayMs.push_back(animFrame.GetDelayMs());
                }
            }
        }
        else {
            //按单帧加载
            m_nFrameCount = 1;
        }
        if (!bLoaded) {
            m_fileData.swap(fileData);
        }
        return bLoaded;
    }
};

Image_PNG::Image_PNG()
{
    m_impl = std::make_unique<TImpl>();
}

Image_PNG::~Image_PNG()
{
}

bool Image_PNG::LoadImageFromFile(const FilePath& filePath,
                                  bool bLoadAllFrames,
                                  bool bAsyncDecode,
                                  float fImageSizeScale,
                                  const UiSize& rcMaxDestRectSize)
{
    DStringA pngFileName = filePath.NativePathA();
    ASSERT(!pngFileName.empty());
    if (pngFileName.empty()) {
        return false;
    }
    ReadPngHeader::PngImageInfo pngImageInfo;
    bool bLoaded = ReadPngHeader::load_apng_image_info(pngFileName, pngImageInfo);
    if (!bLoaded) {
        return false;
    }
    m_impl->m_fileData.clear();
    m_impl->m_filePath = filePath;
    std::vector<uint8_t> fileData;
    return m_impl->InitImageData(fileData, pngImageInfo, bLoadAllFrames, bAsyncDecode, fImageSizeScale, rcMaxDestRectSize);
}

bool Image_PNG::LoadImageFromMemory(std::vector<uint8_t>& fileData,
                                    bool bLoadAllFrames,
                                    bool bAsyncDecode,
                                    float fImageSizeScale,
                                    const UiSize& rcMaxDestRectSize)
{
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return false;
    }
    m_impl->m_filePath.Clear();
    m_impl->m_fileData.clear();
    m_impl->m_fileData.swap(fileData);
    //只加载关键信息，不解码图片数据
    ReadPngHeader::PngImageInfo pngImageInfo;
    bool bLoaded = ReadPngHeader::load_apng_image_info(m_impl->m_fileData, pngImageInfo);
    if (!bLoaded) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }
    return m_impl->InitImageData(fileData, pngImageInfo, bLoadAllFrames, bAsyncDecode, fImageSizeScale, rcMaxDestRectSize);
}

AnimationFramePtr Image_PNG::DecodeImageFrame()
{
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return nullptr;
    }

    bool bLoaded = true;
    float fImageSizeScale = m_impl->m_fImageSizeScale;
    if (m_impl->m_pImageDecoder == nullptr) {
        m_impl->m_pImageDecoder = std::make_unique<APngDecoder>();
        if (!m_impl->m_fileData.empty()) {
            bLoaded = m_impl->m_pImageDecoder->LoadFromMemory(m_impl->m_fileData.data(), m_impl->m_fileData.size(), m_impl->m_bLoadAllFrames);
        }
        else if (!m_impl->m_filePath.IsEmpty()) {
            bLoaded = m_impl->m_pImageDecoder->LoadFromFile(m_impl->m_filePath.NativePathA(), m_impl->m_bLoadAllFrames);
        }
        else {
            ASSERT(0);
        }
        if (!bLoaded) {
            m_impl->m_pImageDecoder.reset();
            std::vector<uint8_t> fileData;
            m_impl->m_fileData.swap(fileData);
        }
    }
    AnimationFramePtr pFrameData;
    if (bLoaded) {
        APngDecoder& pngDecoder = *(m_impl->m_pImageDecoder);
        ASSERT(m_impl->m_nWidth == ImageUtil::GetScaledImageSize((uint32_t)pngDecoder.GetWidth(), fImageSizeScale));
        ASSERT(m_impl->m_nHeight == ImageUtil::GetScaledImageSize((uint32_t)pngDecoder.GetHeight(), fImageSizeScale));

        if (pngDecoder.DecodeNextFrame()) {            
            int32_t nCurFrame = 0;
            int32_t nTotalFrames = 0;
            pngDecoder.GetProgress(&nCurFrame, &nTotalFrames);
            int32_t nFrameIndex = nCurFrame - 1;
            ASSERT(nTotalFrames == m_impl->m_nFrameCount);
            ASSERT(nFrameIndex < m_impl->m_nFrameCount);

            pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
            pFrameData->m_nFrameIndex = nFrameIndex;
            pFrameData->SetDelayMs(pngDecoder.GetFrameDelay(nFrameIndex));
            pFrameData->m_nOffsetX = 0;
            pFrameData->m_nOffsetY = 0;
            pFrameData->m_bDataPending = false;
            pFrameData->m_pBitmap.reset(pRenderFactory->CreateBitmap());
            ASSERT(pFrameData->m_pBitmap != nullptr);
            if (pFrameData->m_pBitmap != nullptr) {
                std::vector<uint8_t> bitmapData;
                bitmapData.resize(pngDecoder.GetHeight() * pngDecoder.GetWidth() * 4);
                if (pngDecoder.GetFrameDataPremultiplied(nFrameIndex, bitmapData.data())) {
                    bool bRet = pFrameData->m_pBitmap->Init(pngDecoder.GetWidth(), pngDecoder.GetHeight(), bitmapData.data(), fImageSizeScale);
                    if (!bRet) {
                        ASSERT(!"Init bitmap data failed!");
                        pFrameData.reset();
                    }
                }
                else {
                    ASSERT(!"GetFrameDataPremultiplied failed!");
                    pFrameData.reset();
                }                
            }
        }
    }
    return pFrameData;
}

bool Image_PNG::IsDelayDecodeEnabled() const
{
    if (m_impl->m_bAsyncDecode && (!m_impl->m_fileData.empty() || !m_impl->m_filePath.IsEmpty())) {
        //仅多帧时支持多线程解码
        return true;
    }
    return false;
}

bool Image_PNG::IsDelayDecodeFinished() const
{
    if (m_impl->m_bAsyncDecoding) {
        return false;
    }
    return (int32_t)(m_impl->m_frames.size() + m_impl->m_delayFrames.size()) == m_impl->m_nFrameCount;
}

uint32_t Image_PNG::GetDecodedFrameIndex() const
{
    if (m_impl->m_frames.empty()) {
        return 0;
    }
    else {
        return (uint32_t)m_impl->m_frames.size() - 1;
    }
}

bool Image_PNG::DelayDecode(uint32_t nMinFrameIndex, std::function<bool(void)> IsAborted)
{
    if (!IsDelayDecodeEnabled()) {
        return false;
    }
    if (m_impl->m_bAsyncDecoding) {
        return false;
    }
    m_impl->m_bAsyncDecoding = true;

    while (((IsAborted == nullptr) || !IsAborted()) &&
           (nMinFrameIndex >= (int32_t)(m_impl->m_frames.size() + m_impl->m_delayFrames.size())) &&
           ((int32_t)(m_impl->m_frames.size() + m_impl->m_delayFrames.size()) < m_impl->m_nFrameCount)) {
        AnimationFramePtr pNewAnimationFrame = DecodeImageFrame();
        if (pNewAnimationFrame != nullptr) {
            m_impl->m_delayFrames.push_back(pNewAnimationFrame);
        }
        else {
            break;
        }
    }

    m_impl->m_bAsyncDecoding = false;
    return true;
}

bool Image_PNG::MergeDelayDecodeData()
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
        if ((int32_t)m_impl->m_frames.size() == m_impl->m_nFrameCount) {
            m_impl->m_pImageDecoder.reset();
            std::vector<uint8_t> fileData;
            m_impl->m_fileData.swap(fileData);
        }
    }
    return bRet;
}

uint32_t Image_PNG::GetWidth() const
{
    return m_impl->m_nWidth;
}

uint32_t Image_PNG::GetHeight() const
{
    return m_impl->m_nHeight;
}

float Image_PNG::GetImageSizeScale() const
{
    return m_impl->m_fImageSizeScale;
}

int32_t Image_PNG::GetFrameCount() const
{
    return m_impl->m_nFrameCount;
}

int32_t Image_PNG::GetLoopCount() const
{
    return m_impl->m_nLoops;
}

bool Image_PNG::IsFrameDataReady(uint32_t nFrameIndex)
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

int32_t Image_PNG::GetFrameDelayMs(uint32_t nFrameIndex)
{
    GlobalManager::Instance().AssertUIThread();
    if (nFrameIndex < m_impl->m_framesDelayMs.size()) {
        return m_impl->m_framesDelayMs[nFrameIndex];
    }
    return IMAGE_ANIMATION_DELAY_MS;
}

bool Image_PNG::ReadFrameData(int32_t nFrameIndex, const UiSize& /*szDestRectSize*/, AnimationFrame* pAnimationFrame)
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
               ((int32_t)m_impl->m_frames.size() < m_impl->m_nFrameCount)) {
            AnimationFramePtr pNewAnimationFrame = DecodeImageFrame();
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
        if ((int32_t)m_impl->m_frames.size() == m_impl->m_nFrameCount) {
            //解码完成，释放资源
            if (m_impl->m_pImageDecoder != nullptr) {
                m_impl->m_pImageDecoder.reset();
            }
            if (!m_impl->m_fileData.empty()) {
                std::vector<uint8_t> fileData;
                m_impl->m_fileData.swap(fileData);
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
