#ifndef UI_IMAGE_APNG_DECODER_H_
#define UI_IMAGE_APNG_DECODER_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/third_party/zlib/zlib.h"
#include "duilib/third_party/libpng/pngpriv.h"
#include "duilib/third_party/libpng/pngstruct.h"
#include "duilib/third_party/libpng/pnginfo.h"
#include "duilib/third_party/libpng/png.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <fstream>

namespace ui
{
/** APNG解码器类，用于加载和处理APNG(Animated Portable Network Graphics)格式图片
    该类封装了libpng库的APNG解码功能，提供面向对象的接口，简化APNG图片的加载、帧管理和资源释放流程
 */
class APngDecoder
{
public:
    /** 构造函数
     */
    APngDecoder();

    /** 析构函数，自动释放所有资源
     */
    ~APngDecoder();

    /** 禁止拷贝构造函数
     */
    APngDecoder(const APngDecoder&) = delete;

    /** 禁止赋值运算符
     */
    APngDecoder& operator=(const APngDecoder&) = delete;

    /** 从内存数据加载APNG图片
     * @param pBuf 指向图片数据的指针
     * @param nLen 图片数据的长度(字节数)
     * @param bLoadAllFrames 是否加载所有帧，false则只加载第一帧
     * @return 加载成功返回true，失败返回false
     */
    bool LoadFromMemory(const uint8_t* pBuf, size_t nLen, bool bLoadAllFrames);

    /** 从文件加载APNG图片
     * @param filePath 文件路径
     * @param bLoadAllFrames 是否加载所有帧，false则只加载第一帧
     * @return 加载成功返回true，失败返回false
     */
    bool LoadFromFile(const std::string& filePath, bool bLoadAllFrames);

    /** 解码下一帧（分步解码使用）
     * @return 解码成功返回true，所有帧解码完成或失败返回false
     */
    bool DecodeNextFrame();

    /** 获取解码进度
     * @param[out] pCurFrame 当前已解码帧数（注意：不是帧的索引号）
     * @param[out] pTotalFrames 总帧数
     */
    void GetProgress(int32_t* pCurFrame, int32_t* pTotalFrames) const;

    /** 获取图片宽度
     * @return 图片宽度(像素)，若未加载成功返回0
     */
    int32_t GetWidth() const;

    /** 获取图片高度
     * @return 图片高度(像素)，若未加载成功返回0
     */
    int32_t GetHeight() const;

    /** 获取总帧数
     * @return 总帧数，若未加载成功返回0
     */
    int32_t GetFrameCount() const;

    /** 获取循环播放次数
     * @return 循环次数，0表示无限循环
     */
    int32_t GetLoopCount() const;

    /** 获取指定帧的延迟时间
     * @param frameIndex 帧索引(从0开始)
     * @return 延迟时间(毫秒)，若索引无效返回0
     */
    int32_t GetFrameDelay(int32_t frameIndex) const;

    /** 获取指定帧的像素数据（非预乘Alpha）
     * 像素格式为RGBA8888(每个像素4字节，依次为红、绿、蓝、alpha通道)
     * @param frameIndex 帧索引(从0开始)
     * @return 指向像素数据的指针，若索引无效返回nullptr
     */
    const uint8_t* GetFrameData(int32_t frameIndex) const;

    /** 获取指定帧的像素数据（预乘Alpha）
     *
     * 像素格式为RGBA8888，其中RGB通道值已经与Alpha通道值进行了预乘处理，
     * 即R = R * A / 255, G = G * A / 255, B = B * A / 255
     *
     * @param frameIndex 帧索引(从0开始)
     * @param[out] outData 用于存储预乘处理后数据的缓冲区，需确保缓冲区大小足够
     *                     （大小应为：宽度 * 高度 * 4字节）
     * @return 成功返回true，失败返回false
     */
    bool GetFrameDataPremultiplied(int32_t frameIndex, uint8_t* outData) const;

    /** 释放所有资源，恢复到未加载状态
     */
    void Destroy();

private:
    // 内部PNG读取器接口
    struct IPngReader
    {
        virtual ~IPngReader() = default;
        virtual size_t read(png_bytep data, png_size_t length) = 0;
    };

    // 内存读取器实现
    struct MemReader : public IPngReader
    {
        const char* pbuf;
        size_t nLen;

        MemReader(const char* buf, size_t len) : pbuf(buf), nLen(len) {}
        // 读取数据到libpng缓冲区
        virtual size_t read(png_bytep data, png_size_t length) override;
    };

    // 文件读取器实现
    struct FileReader : public IPngReader
    {
        std::ifstream fs;

        FileReader(const std::string& filePath);
        // 读取数据到libpng缓冲区
        virtual size_t read(png_bytep data, png_size_t length) override;
    };

    // PNG警告回调函数
    static void PngWarningCallback(png_structp png_ptr, png_const_charp message);

    // PNG错误回调函数
    static void PngErrorCallback(png_structp png_ptr, png_const_charp message);

    // libpng读取回调函数
    static void PngReadData(png_structp png_ptr, png_bytep data, png_size_t length);

    // 实际加载PNG/APNG的内部实现
    bool LoadPng(IPngReader* reader, bool bLoadAllFrames);

    // 解码状态枚举
    enum class State
    {
        INIT,           // 初始状态
        READ_HEADER,    // 已读取头部信息
        DECODING_FRAMES,// 正在解码帧
        FINISHED,       // 解码完成
        FAILED          // 错误状态
    };

    // 图片宽度
    int32_t m_width = 0;

    // 图片高度
    int32_t m_height = 0;

    // 总帧数
    int32_t m_frameCount = 0;

    // 循环次数
    int32_t m_loopCount = 0;

    // 当前解码帧索引
    int32_t m_currentFrame = 0;

    // 每帧延迟时间(毫秒)
    std::unique_ptr<uint16_t[]> m_frameDelays;

    // 所有帧的像素数据(RGBA8888)
    std::unique_ptr<uint8_t[]> m_frameData;

    // 单帧数据大小(字节) = width * height * 4
    size_t m_frameSize = 0;

    // 每行字节数
    size_t m_bytesPerRow = 0;

    // 当前帧缓存(用于帧合成)
    std::unique_ptr<uint8_t[]> m_currentFrameBuffer;

    // 临时帧数据缓存
    std::unique_ptr<uint8_t[]> m_frameBuffer;

    // 行指针数组(用于libpng)
    std::unique_ptr<png_bytep[]> m_rowPointers;

    // libpng结构体
    png_structp m_pngPtr = nullptr;

    // libpng信息结构体
    png_infop m_infoPtr = nullptr;

    // 数据读取器
    std::unique_ptr<IPngReader> m_reader;

    // 解码状态
    State m_state = State::INIT;

    // 是否加载所有帧
    bool m_loadAllFrames = true;
};

}//namespace ui

#endif // UI_IMAGE_APNG_DECODER_H_
