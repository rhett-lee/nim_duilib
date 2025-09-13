#include "Image_PNG.h"
#include "duilib/Image/APngDecoder.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Render/IRender.h"

#include <vector>
#include <cmath>

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

    static void png_read_mem_callback(png_structp png, png_bytep data, png_size_t length) {
        PngBuffer* buf = static_cast<PngBuffer*>(png_get_io_ptr(png));
        if (buf->offset + length > buf->size) {
            png_error(png, "Read error");
            return;
        }
        memcpy(data, buf->data + buf->offset, length);
        buf->offset += length;
    }

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
}
//
////解码PNG/APNG图片数据
//static bool DecodeImage_APNG(APNGDATA* pngData,
//                             float fImageSizeScale,
//                             std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>>& frames,
//                             volatile bool* pAbortFlag = nullptr)
//{
//    ASSERT((pngData != nullptr) && (pngData->nWid > 0) && (pngData->nHei > 0) && (pngData->nFrames > 0));
//    const int nWid = pngData->nWid;
//    const int nHei = pngData->nHei;
//    if ((nWid <= 0) || (nHei <= 0) || (pngData->nFrames < 1)) {
//        return false;
//    }
//    ASSERT(fImageSizeScale > 0);
//    if (fImageSizeScale < 0) {
//        return false;
//    }
//
//    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
//    ASSERT(pRenderFactory != nullptr);
//    if (pRenderFactory == nullptr) {
//        return false;
//    }
//    frames.clear();
//
//    //swap rgba to bgra and do premultiply
//    uint8_t* p = pngData->pdata;
//    if (p == nullptr) {
//        return false;
//    }
//    int pixel_count = nWid * nHei * pngData->nFrames;
//    for (int i = 0; i < pixel_count; ++i) {
//        uint8_t a = p[3];
//        uint8_t t = p[0];
//        if (a)
//        {
//#ifdef DUILIB_BUILD_FOR_WIN
//            //数据格式：Window平台BGRA，其他平台RGBA
//            p[0] = (p[2] * a) / 255;
//            p[1] = (p[1] * a) / 255;
//            p[2] = (t * a) / 255;
//#else
//            p[0] = (p[0] * a) / 255;
//            p[1] = (p[1] * a) / 255;
//            p[2] = (p[2] * a) / 255;
//#endif
//        }
//        else
//        {
//            memset(p, 0, 4);
//        }
//        p += 4;
//    }
//
//    p = pngData->pdata;
//
//    frames.resize(pngData->nFrames);
//    for (int i = 0; i < pngData->nFrames; ++i) {
//        std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
//        pFrameData->m_nFrameIndex = i;
//        pFrameData->SetDelayMs(pngData->pDelay ? pngData->pDelay[i] : 0);
//        pFrameData->m_nOffsetX = 0;
//        pFrameData->m_nOffsetY = 0;
//        pFrameData->m_bDataPending = false;
//        pFrameData->m_pBitmap.reset(pRenderFactory->CreateBitmap());
//        ASSERT(pFrameData->m_pBitmap != nullptr);
//        if (pFrameData->m_pBitmap == nullptr) {
//            return false;
//        }
//        pFrameData->m_pBitmap->Init(nWid, nHei, p, fImageSizeScale);
//        frames[i] = pFrameData;
//
//        //下一个帧
//        const int imageDataSize = nWid * nHei * 4;
//        p += imageDataSize;
//        if (pAbortFlag && *pAbortFlag) {
//            //已经取消
//            return false;
//        }
//    }
//    return true;
//}

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

    //延迟解码是否已经开始
    bool m_bDecodeImageDataRunning = false;

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

    //每一帧的播放延迟时间，毫秒
    std::vector<int32_t> m_framesDelayMs;
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
    m_impl->m_fileData.clear();
    m_impl->m_fileData.swap(fileData);
    m_impl->m_fImageSizeScale = fImageSizeScale;
    m_impl->m_bLoadAllFrames = bLoadAllFrames;

    //只加载关键信息，不解码图片数据
    ReadPngHeader::PngImageInfo pngImageInfo;
    bool bLoaded = ReadPngHeader::load_apng_image_info(m_impl->m_fileData, pngImageInfo);
    if (!bLoaded) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
    }
    else {
        //记录总帧数            
        m_impl->m_nWidth = ImageUtil::GetScaledImageSize(pngImageInfo.width, fImageSizeScale);
        m_impl->m_nHeight = ImageUtil::GetScaledImageSize(pngImageInfo.height, fImageSizeScale);
        m_impl->m_nFrameCount = pngImageInfo.frame_count;
        m_impl->m_nLoops = pngImageInfo.loop_count;
        if (m_impl->m_nLoops <= 0) {
            m_impl->m_nLoops = -1;
        }
        ASSERT(m_impl->m_nWidth > 0);
        ASSERT(m_impl->m_nHeight > 0);
        ASSERT(m_impl->m_nFrameCount > 0);

        if (bLoadAllFrames) {
            //支持加载多帧
            m_impl->m_framesDelayMs.clear();
            if (pngImageInfo.frame_count > 1) {
                for (const ReadPngHeader::FrameDuration& frameDuration : pngImageInfo.frames) {
                    AnimationFrame animFrame;
                    animFrame.SetDelayMs(frameDuration.duration_ms);
                    m_impl->m_framesDelayMs.push_back(animFrame.GetDelayMs());
                }
            }
        }
        else {
            //按单帧加载
            m_impl->m_nFrameCount = 1;
        }
    }
    return bLoaded;
}

bool Image_PNG::IsDecodeImageDataEnabled() const
{
    if (!m_impl->m_fileData.empty() &&
        m_impl->m_bLoadAllFrames &&
        (m_impl->m_nFrameCount > 1) &&
        !m_impl->m_bDecodeImageDataStarted &&
        !m_impl->m_bDecodeImageDataAborted) {
        //仅多帧时支持多线程解码
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
    if ((m_impl->m_fileData.empty() || !m_impl->m_bLoadAllFrames) ||
        (m_impl->m_nFrameCount <= 1) || m_impl->m_bDecodeImageDataRunning) {
        return false;
    }
    m_impl->m_bDecodeImageDataRunning = true;
    float fImageSizeScale = m_impl->m_fImageSizeScale;
    uint32_t nOutFrameCount = 0;

    ////多帧图片延迟解码
    //bool bLoaded = false;
    //APNGDATA* apngData = LoadAPNG_from_memory((const char*)m_impl->m_fileData.data(), m_impl->m_fileData.size(), true, nOutFrameCount);
    //if (apngData != nullptr) {
    //    //加载图片数据
    //    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> frames;
    //    if (DecodeImage_APNG(apngData, fImageSizeScale, frames, &(m_impl->m_bDecodeImageDataAborted))) {
    //        bLoaded = true;
    //        //记录总帧数
    //        ASSERT(m_impl->m_nWidth == ImageUtil::GetScaledImageSize((uint32_t)apngData->nWid, fImageSizeScale));
    //        ASSERT(m_impl->m_nHeight == ImageUtil::GetScaledImageSize((uint32_t)apngData->nHei, fImageSizeScale));
    //        ASSERT(m_impl->m_nFrameCount == (int32_t)nOutFrameCount);

    //        ASSERT(!m_impl->m_bDecodeImageDataFinished);
    //        ASSERT(m_impl->m_delayFrames.empty());

    //        m_impl->m_delayFrames.swap(frames);
    //        m_impl->m_bDecodeImageDataFinished = true;
    //    }
    //    APNG_Destroy(apngData);
    //}
    //return bLoaded;
    return true;
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

bool Image_PNG::IsFrameDataReady(uint32_t nFrameIndex)
{
    if (nFrameIndex < m_impl->m_frames.size()) {
        return true;
    }
    if (m_impl->m_bDecodeImageDataFinished) {
        if (nFrameIndex < m_impl->m_delayFrames.size()) {
            return true;
        }
    }
    return false;
}

int32_t Image_PNG::GetFrameDelayMs(uint32_t nFrameIndex)
{
    if (nFrameIndex < m_impl->m_framesDelayMs.size()) {
        return m_impl->m_framesDelayMs[nFrameIndex];
    }
    return IMAGE_ANIMATION_DELAY_MS;
}

bool Image_PNG::ReadFrameData(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
{
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

    if (m_impl->m_frames.empty()) {
        //尚未解码，解码第一帧
        float fImageSizeScale = m_impl->m_fImageSizeScale;
        APngDecoder pngDecoder;
        bool bLoaded = pngDecoder.LoadFromMemory(m_impl->m_fileData.data(), m_impl->m_fileData.size(), true);
        if (bLoaded) {
            ASSERT(m_impl->m_nWidth == ImageUtil::GetScaledImageSize((uint32_t)pngDecoder.GetWidth(), fImageSizeScale));
            ASSERT(m_impl->m_nHeight == ImageUtil::GetScaledImageSize((uint32_t)pngDecoder.GetHeight(), fImageSizeScale));

            IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
            ASSERT(pRenderFactory != nullptr);
            if (pRenderFactory == nullptr) {
                return false;
            }

            int32_t nFrameIndex = 0;
            while (pngDecoder.DecodeNextFrame()) {
                std::shared_ptr<IAnimationImage::AnimationFrame> pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
                pFrameData->m_nFrameIndex = nFrameIndex;
                pFrameData->SetDelayMs(pngDecoder.GetFrameDelay(nFrameIndex));
                pFrameData->m_nOffsetX = 0;
                pFrameData->m_nOffsetY = 0;
                pFrameData->m_bDataPending = false;
                pFrameData->m_pBitmap.reset(pRenderFactory->CreateBitmap());
                ASSERT(pFrameData->m_pBitmap != nullptr);
                if (pFrameData->m_pBitmap == nullptr) {
                    return false;
                }
                std::vector<uint8_t> bitmapData;
                bitmapData.resize(pngDecoder.GetHeight() * pngDecoder.GetWidth() * 4);
                pngDecoder.GetFrameDataPremultiplied(nFrameIndex, bitmapData.data());//返回值

                pFrameData->m_pBitmap->Init(pngDecoder.GetWidth(), pngDecoder.GetHeight(), bitmapData.data(), fImageSizeScale);
                m_impl->m_frames.push_back(pFrameData);
                ++nFrameIndex;
            }

            *pAnimationFrame = *m_impl->m_frames[0];
        }
        return bLoaded;
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

    //ASSERT(!m_impl->m_frames.empty());
    if (m_impl->m_frames.empty()) {
        return false;
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
