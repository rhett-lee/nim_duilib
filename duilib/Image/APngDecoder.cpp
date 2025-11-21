#include "APngDecoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cstring>
#include <stdexcept>

// 跨平台调试输出
#ifdef _DEBUG
    #if defined(_WIN32) || defined(_WIN64)
        #define DEBUG_OUTPUT(msg) OutputDebugStringA(msg)
    #else
        #define DEBUG_OUTPUT(msg) fprintf(stderr, "%s", msg)
    #endif
#else
    #define DEBUG_OUTPUT(msg)
#endif

#define SASSERT(x) assert(x)

namespace ui
{

// 内存读取器实现
size_t APngDecoder::MemReader::read(png_bytep data, png_size_t length)
{
    if (nLen < length) {
        length = nLen;
    }
    memcpy(data, pbuf, length);
    pbuf += length;
    nLen -= length;
    return length;
}

// 文件读取器的实现
APngDecoder::FileReader::FileReader(const std::string& filePath)
{
    // 以二进制模式打开文件
    fs.open(filePath, std::ios::binary);
}

size_t APngDecoder::FileReader::read(png_bytep data, png_size_t length)
{
    if (!fs) {
        return 0;
    }
    fs.read(reinterpret_cast<char*>(data), length);
    return static_cast<size_t>(fs.gcount());
}

// PNG警告回调
void APngDecoder::PngWarningCallback(png_structp /*png_ptr*/, png_const_charp message)
{
   // (void)png_ptr;
    std::string warningMsg;
    if (message != nullptr) {
        warningMsg = StringUtil::Printf("PNG decoding warning: %s", message);
    }
#if defined(_WIN32) || defined(_WIN64)
    DEBUG_OUTPUT(warningMsg.c_str());
#endif
}

// PNG错误回调
void APngDecoder::PngErrorCallback(png_structp /*png_ptr*/, png_const_charp message)
{
    std::string errMsg;
    if (message != nullptr) {
        errMsg = StringUtil::Printf("PNG decoding error: %s", message);
    }
    DEBUG_OUTPUT(errMsg.c_str());
    throw std::runtime_error(errMsg.c_str());
}

// libpng读取回调
void APngDecoder::PngReadData(png_structp png_ptr, png_bytep data, png_size_t length)
{
    if (!png_ptr) {
        return;
    }

    IPngReader* reader = static_cast<IPngReader*>(png_get_io_ptr(png_ptr));
    size_t read = reader->read(data, length);
    if (read < length) {
        png_error(png_ptr, "Read error");
    }
}

// 构造函数
APngDecoder::APngDecoder() = default;

// 析构函数
APngDecoder::~APngDecoder()
{
    Destroy();
}

// 从内存加载APNG
bool APngDecoder::LoadFromMemory(const uint8_t* pBuf, size_t nLen, bool bLoadAllFrames)
{
    if ((pBuf == nullptr) || (nLen == 0)) {
        return false;
    }
    //先释放原有资源
    Destroy();
    try {
        m_reader = std::make_unique<MemReader>((const char*)pBuf, nLen);
        return LoadPng(m_reader.get(), bLoadAllFrames);
    }
    catch (...) {
        Destroy();
        return false;
    }
}

bool APngDecoder::LoadFromFile(const std::string& filePath, bool bLoadAllFrames)
{
    if (filePath.empty()) {
        return false;
    }

    // 先释放原有资源
    Destroy();
    try {
        m_reader = std::make_unique<FileReader>(filePath);
        return LoadPng(m_reader.get(), bLoadAllFrames);
    }
    catch (...) {
        Destroy();
        return false;
    }
}

// 加载PNG的内部实现
bool APngDecoder::LoadPng(IPngReader* reader, bool bLoadAllFrames)
{
    m_loadAllFrames = bLoadAllFrames;

    // 验证文件签名
    png_byte sig[8];
    if (reader->read(sig, 8) != 8 || !png_check_sig(sig, 8)) {
        m_state = State::FAILED;
        return false;
    }

    // 初始化libpng结构体
    m_pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                      PngErrorCallback, PngWarningCallback);
    if (!m_pngPtr) {
        m_state = State::FAILED;
        return false;
    }

    m_infoPtr = png_create_info_struct(m_pngPtr);
    if (!m_infoPtr) {
        png_destroy_read_struct(&m_pngPtr, nullptr, nullptr);
        m_pngPtr = nullptr;
        m_state = State::FAILED;
        return false;
    }

#pragma warning (push)
#pragma warning (disable: 4611)
    // 设置错误处理跳转
    if (setjmp(png_jmpbuf(m_pngPtr))) {
        Destroy();
        return false;
    }
#pragma warning (pop)

    // 配置读取函数
    png_set_read_fn(m_pngPtr, reader, PngReadData);
    png_set_sig_bytes(m_pngPtr, 8);

    // 图像格式转换配置
    png_set_expand(m_pngPtr);
    png_set_add_alpha(m_pngPtr, 0xff, PNG_FILLER_AFTER);
    png_set_interlace_handling(m_pngPtr);
    png_set_gray_to_rgb(m_pngPtr);
    png_set_strip_16(m_pngPtr);

    // 读取图像信息
    png_read_info(m_pngPtr, m_infoPtr);
    png_read_update_info(m_pngPtr, m_infoPtr);

    // 初始化图像参数
    m_width = png_get_image_width(m_pngPtr, m_infoPtr);
    m_height = png_get_image_height(m_pngPtr, m_infoPtr);
    m_bytesPerRow = m_width * 4;
    m_frameSize = m_bytesPerRow * m_height;

    // 初始化帧缓存
    m_frameBuffer = std::make_unique<uint8_t[]>(m_frameSize);
    m_currentFrameBuffer = std::make_unique<uint8_t[]>(m_frameSize);
    if (!m_frameBuffer || !m_currentFrameBuffer) {
        throw std::bad_alloc();
    }
    memset(m_currentFrameBuffer.get(), 0, m_frameSize);

    // 初始化行指针
    m_rowPointers = std::make_unique<png_bytep[]>(m_height);
    if (!m_rowPointers) {
        throw std::bad_alloc();
    }
    for (int32_t i = 0; i < m_height; ++i) {
        m_rowPointers[i] = m_frameBuffer.get() + m_bytesPerRow * i;
    }

    // 检查是否为APNG
    if (!png_get_valid(m_pngPtr, m_infoPtr, PNG_INFO_acTL)) {
        // 非动画PNG
        m_frameCount = 1;
        m_loopCount = 0;
        m_frameDelays = std::make_unique<uint16_t[]>(1);
        m_frameDelays[0] = 100; // 默认延迟100ms
        m_frameData = std::make_unique<uint8_t[]>(m_frameSize);
    }
    else {
        // 动画PNG
        m_frameCount = png_get_num_frames(m_pngPtr, m_infoPtr);
        if (m_frameCount < 1) m_frameCount = 1;

        if (!m_loadAllFrames) m_frameCount = 1;

        m_loopCount = png_get_num_plays(m_pngPtr, m_infoPtr);
        m_frameDelays = std::make_unique<uint16_t[]>(m_frameCount);
        m_frameData = std::make_unique<uint8_t[]>(m_frameSize * m_frameCount);

        if (!m_frameDelays || !m_frameData) {
            throw std::bad_alloc();
        }
    }

    m_currentFrame = 0;
    m_state = State::READ_HEADER;
    return true;
}

// 解码下一帧
bool APngDecoder::DecodeNextFrame() {
    if (m_state == State::FINISHED || m_state == State::FAILED) {
        return false;
    }

    try {
#pragma warning (push)
#pragma warning (disable: 4611)
        if (setjmp(png_jmpbuf(m_pngPtr))) {
            m_state = State::FAILED;
            return false;
        }
#pragma warning (pop)

        // 处理单帧PNG
        if (m_frameCount == 1) {
            png_read_image(m_pngPtr, m_rowPointers.get());
            memcpy(m_frameData.get(), m_frameBuffer.get(), m_frameSize);
            m_currentFrame++;
            m_state = State::FINISHED;
            return true;
        }

        // 处理多帧APNG
        if (m_currentFrame < m_frameCount) {
            // 读取帧头信息
            png_read_frame_head(m_pngPtr, m_infoPtr);

            // 计算延迟时间
            if (png_get_valid(m_pngPtr, m_infoPtr, PNG_INFO_fcTL)) {
                png_uint_16 delayNum = m_infoPtr->next_frame_delay_num;
                png_uint_16 delayDen = m_infoPtr->next_frame_delay_den;
                if (delayDen == 0) delayDen = 100;
                m_frameDelays[m_currentFrame] = static_cast<uint16_t>(
                    (delayNum * 1000.0) / delayDen);
            }
            else {
                m_frameDelays[m_currentFrame] = 100;
            }

            // 读取当前帧数据
            png_read_image(m_pngPtr, m_rowPointers.get());

            // 计算绘制位置
            auto xOffset = m_infoPtr->next_frame_x_offset;
            auto yOffset = m_infoPtr->next_frame_y_offset;
            auto frameWidth = m_infoPtr->next_frame_width;
            auto frameHeight = m_infoPtr->next_frame_height;

            uint8_t* dstLine = m_currentFrameBuffer.get() +
                yOffset * m_bytesPerRow + xOffset * 4;
            uint8_t* srcLine = m_frameBuffer.get();

            // 帧混合操作
            switch (m_infoPtr->next_frame_blend_op) {
            case PNG_BLEND_OP_OVER: {
                for (unsigned int y = 0; y < frameHeight; ++y) {
                    uint8_t* dst = dstLine;
                    uint8_t* src = srcLine;
                    for (unsigned int x = 0; x < frameWidth; ++x) {
                        // Alpha混合计算
                        uint8_t alpha = src[3];
                        dst[0] = static_cast<uint8_t>(
                            (dst[0] * (255 - alpha) + src[0] * alpha) / 255);
                        dst[1] = static_cast<uint8_t>(
                            (dst[1] * (255 - alpha) + src[1] * alpha) / 255);
                        dst[2] = static_cast<uint8_t>(
                            (dst[2] * (255 - alpha) + src[2] * alpha) / 255);
                        dst[3] = static_cast<uint8_t>(
                            (dst[3] * (255 - alpha) + src[3] * alpha) / 255);
                        dst += 4;
                        src += 4;
                    }
                    dstLine += m_bytesPerRow;
                    srcLine += m_bytesPerRow;
                }
                break;
            }
            case PNG_BLEND_OP_SOURCE: {
                // 直接覆盖
                for (unsigned int y = 0; y < frameHeight; ++y) {
                    memcpy(dstLine, srcLine, frameWidth * 4);
                    dstLine += m_bytesPerRow;
                    srcLine += m_bytesPerRow;
                }
                break;
            }
            default:
                SASSERT(false);
                break;
            }

            // 保存当前帧
            uint8_t* targetFrame = m_frameData.get() + m_currentFrame * m_frameSize;
            memcpy(targetFrame, m_currentFrameBuffer.get(), m_frameSize);

            // 处理帧清理操作
            dstLine = m_currentFrameBuffer.get() + yOffset * m_bytesPerRow + xOffset * 4;

            switch (m_infoPtr->next_frame_dispose_op) {
            case PNG_DISPOSE_OP_BACKGROUND:
                // 清除背景
                for (unsigned int y = 0; y < frameHeight; ++y) {
                    memset(dstLine, 0, frameWidth * 4);
                    dstLine += m_bytesPerRow;
                }
                break;
            case PNG_DISPOSE_OP_PREVIOUS:
                // 恢复前一帧
                if (m_currentFrame > 0) {
                    memcpy(m_currentFrameBuffer.get(),
                        targetFrame - m_frameSize,
                        m_frameSize);
                }
                break;
            case PNG_DISPOSE_OP_NONE:
                // 不处理
                break;
            default:
                SASSERT(false);
                break;
            }

            m_currentFrame++;
            if (m_currentFrame >= m_frameCount) {
                m_state = State::FINISHED;
            }
            return true;
        }
    }
    catch (...) {
        m_state = State::FAILED;
        return false;
    }

    return false;
}

// 获取解码进度
void APngDecoder::GetProgress(int32_t* pCurFrame, int32_t* pTotalFrames) const
{
    if (pCurFrame != nullptr) {
        *pCurFrame = m_currentFrame;
    }
    if (pTotalFrames != nullptr) {
        *pTotalFrames = m_frameCount;
    }
}

// 获取宽度
int32_t APngDecoder::GetWidth() const
{
    return m_width;
}

// 获取高度
int32_t APngDecoder::GetHeight() const
{
    return m_height;
}

// 获取帧数量
int32_t APngDecoder::GetFrameCount() const
{
    return m_frameCount;
}

// 获取循环次数
int32_t APngDecoder::GetLoopCount() const
{
    return m_loopCount;
}

// 获取指定帧延迟
int32_t APngDecoder::GetFrameDelay(int32_t frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= m_frameCount || !m_frameDelays) {
        return 0;
    }
    return (int32_t)m_frameDelays[frameIndex];
}

// 获取指定帧数据（非预乘）
const uint8_t* APngDecoder::GetFrameData(int32_t frameIndex) const
{
    if (frameIndex < 0 || frameIndex >= m_frameCount || !m_frameData) {
        return nullptr;
    }
    return (const uint8_t*)(m_frameData.get() + frameIndex * m_frameSize);
}

// 获取指定帧数据（预乘Alpha）
bool APngDecoder::GetFrameDataPremultiplied(int32_t frameIndex, uint8_t* outData) const
{
    // 检查参数有效性
    if (frameIndex < 0 || frameIndex >= m_frameCount || !m_frameData || !outData) {
        return false;
    }

    // 获取原始帧数据
    const uint8_t* srcData = (const uint8_t*)(m_frameData.get() + frameIndex * m_frameSize);
    if (!srcData) {
        return false;
    }

    // 计算总像素数
    size_t pixelCount = static_cast<size_t>(m_width) * m_height;

    // 对每个像素执行预乘操作
    size_t pixelIndex = 0;
    for (size_t i = 0; i < pixelCount; ++i) {
        pixelIndex = i * 4;
        // 读取原始RGBA值
        uint8_t r = srcData[pixelIndex];
        uint8_t g = srcData[pixelIndex + 1];
        uint8_t b = srcData[pixelIndex + 2];
        uint8_t a = srcData[pixelIndex + 3];

        // 执行预乘计算：R' = (R * A) / 255
        // 使用整数运算避免浮点精度问题
#if defined (_WIN32) || defined (_WIN64)
        //数据格式：Window平台BGRA，其他平台RGBA
        outData[pixelIndex] = static_cast<uint8_t>(((uint32_t)b * (uint32_t)a) / 255);
        outData[pixelIndex + 1] = static_cast<uint8_t>(((uint32_t)g * (uint32_t)a) / 255);
        outData[pixelIndex + 2] = static_cast<uint8_t>(((uint32_t)r * (uint32_t)a) / 255);
        outData[pixelIndex + 3] = a;  // Alpha值保持不变
#else
        outData[pixelIndex] = static_cast<uint8_t>(((uint32_t)r * (uint32_t)a) / 255);
        outData[pixelIndex + 1] = static_cast<uint8_t>(((uint32_t)g * (uint32_t)a) / 255);
        outData[pixelIndex + 2] = static_cast<uint8_t>(((uint32_t)b * (uint32_t)a) / 255);
        outData[pixelIndex + 3] = a;  // Alpha值保持不变
#endif
    }
    return true;
}

// 释放所有资源
void APngDecoder::Destroy()
{
    // 释放libpng资源
    if (m_pngPtr && m_infoPtr) {
        png_destroy_read_struct(&m_pngPtr, &m_infoPtr, nullptr);
        m_pngPtr = nullptr;
        m_infoPtr = nullptr;
    }

    // 重置成员变量
    m_width = 0;
    m_height = 0;
    m_frameCount = 0;
    m_loopCount = 0;
    m_currentFrame = 0;
    m_frameSize = 0;
    m_bytesPerRow = 0;
    m_frameDelays.reset();
    m_frameData.reset();
    m_currentFrameBuffer.reset();
    m_frameBuffer.reset();
    m_rowPointers.reset();
    m_reader.reset();
    m_state = State::INIT;
    m_loadAllFrames = true;
}

}//namespace ui
