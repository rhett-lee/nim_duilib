#include "Image_GIF.h"
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

/** 提取GIF帧的透明色索引（遵循GIF89a规范第23节）
 * @param frame 目标帧数据，包含扩展块信息
 * @return 透明色索引（0-255），-1表示无透明色或未找到
 */
static int UiGifGetTransparentIndex(const SavedImage& frame)
{
    // 定义透明色标志位掩码（第3字节最低位）
    constexpr uint8_t TRANSPARENCY_FLAG = 0x01;

    // 遍历帧的所有扩展块
    for (int i = 0; i < frame.ExtensionBlockCount; ++i) {
        const ExtensionBlock& ext = frame.ExtensionBlocks[i];

        // 检查是否为图形控制扩展块（0xF9）且数据有效
        if (ext.Function == GRAPHICS_EXT_FUNC_CODE &&  // 扩展块类型校验
            ext.Bytes != nullptr &&                    // 数据指针非空校验
            ext.ByteCount >= 5) {                      // 数据长度校验（至少5字节）

            // 检查第3字节（索引2）的透明色标志位
            // 根据GIF规范：第3字节最低位为1表示存在透明色
            if (ext.Bytes[2] & TRANSPARENCY_FLAG) {
                // 返回第5字节（索引4）的透明色索引值
                return static_cast<int>(ext.Bytes[4]);
            }
        }
    }

    // 未找到有效的透明色信息
    return -1;
}

/** 获取指定帧的有效调色板（局部优先，无则用全局）
* @param gif: GifFileType 句柄
* @param frame 目标帧数据，包含扩展块信息
*/
static const ColorMapObject* UiGifGetFrameColorMap(const GifFileType* gif, const SavedImage& frame)
{
    // 局部调色板：frame.ImageDesc.ColorMap（非空则优先）
    if (frame.ImageDesc.ColorMap != nullptr) {
        return frame.ImageDesc.ColorMap;
    }
    // 全局调色板：gif->SColorMap（若局部为空，用全局）
    else if (gif->SColorMap != nullptr) {
        return gif->SColorMap;
    }
    // 无调色板 → GIF 格式错误
    ASSERT(0);
    return nullptr;
}

/** 获取GIF帧的Disposal Method（帧间处理规则）
 *
 * 规范依据：GIF89a规范第23节（图形控制扩展块结构）
 * 数据格式：
 *   Byte[0] = 扩展标识(0x21)
 *   Byte[1] = 图形控制标签(0xF9)
 *   Byte[2] = 块大小(固定4字节)
 *   Byte[3] = 处理方式标志位（含disposal method）
 *   Byte[4+] = 附加数据
 *
 * @param frame 目标帧数据，需包含有效的ExtensionBlocks数组
 * @return 取值说明:
 *   0 - 不处理（保留当前帧）
 *   1 - 清除到背景色
 *   2 - 还原为先前状态
 *   3-7 - 保留值（应视为0处理）
 *   -1 - 数据错误（扩展块无效）
 */
static int UiGifGetDisposalMethod(const SavedImage& frame)
{
    // 二进制掩码 00011100（用于提取第3字节的3-5位）
    constexpr uint8_t DISPOSAL_METHOD_MASK = 0x1C;
    // 默认处理方式（规范要求未指定时视为0）
    constexpr int DEFAULT_DISPOSAL_METHOD = 0;

    // 遍历帧的所有扩展块
    for (int i = 0; i < frame.ExtensionBlockCount; ++i) {
        const ExtensionBlock& ext = frame.ExtensionBlocks[i];

        // 三重校验确保数据有效性：
        // 1. 必须是图形控制扩展块（0xF9）
        // 2. 数据指针非空
        // 3. 数据长度至少4字节（含标志位）
        if (ext.Function == GRAPHICS_EXT_FUNC_CODE &&
            ext.Bytes != nullptr &&
            ext.ByteCount >= 4) {

            // 关键位操作：
            // 1. 用掩码提取目标位 (xxxDDDxx -> 000DDD00)
            // 2. 右移3位转换为0-7的整数值
            return (ext.Bytes[2] & DISPOSAL_METHOD_MASK) >> 3;
        }
    }

    // 未找到有效扩展块时返回默认值
    return DEFAULT_DISPOSAL_METHOD;
}

/** 从 SavedImage 中获取当前帧的播放时间（毫秒），返回 - 1 表示无延迟信息
* @param frame 目标帧数据，需包含有效的ExtensionBlocks数组
*/
static int32_t UiGifGetFrameDelayMs(const SavedImage& frame)
{
    // 遍历帧的所有扩展块
    for (int i = 0; i < frame.ExtensionBlockCount; ++i) {
        const ExtensionBlock& ext = frame.ExtensionBlocks[i];

        // 筛选图形控制扩展块（0xF9）
        if (ext.Function == GRAPHICS_EXT_FUNC_CODE) {
            /* 图形控制扩展块标准结构：
             * [0] 保留位 | 处置方法(3bit) | 用户输入标志(1bit) | 透明色标志(1bit)
             * [1-2] 延迟时间（小端序，单位1/100秒）
             * [3] 透明色索引（如果有）
             * 最少需要3字节（不含块头）才能包含延迟时间
             */
            if (ext.ByteCount >= 3 && ext.Bytes != nullptr) {
                // 正确的小端序转换（低字节在前）
                uint16_t delay_centisecs = (static_cast<uint16_t>(ext.Bytes[2]) << 8) |
                    static_cast<uint16_t>(ext.Bytes[1]);

                // 特殊处理：当delay=0时按标准应视为100ms（GIF规范89a第23条）
                return delay_centisecs == 0 ? 100 :
                    static_cast<int32_t>(delay_centisecs * 10);
            }
        }
    }

    // 无有效图形控制块时返回-1（调用方决定是否使用默认值）
    return -1;
}

/** 将GIF文件解析为逐帧RGBA数据
 * @param gif DGifSlurp成功后的GIF数据指针
 * @param nFrameIndex 加载哪一帧
 * @param fImageSizeScale 图片的缩放比例
 * @param backgroundColor 背景色
 * @param canvas 画布
 * @param canvas_backup 备份的画布
 * @param nLastFrameIndex 上一帧的索引号
 * @return 返回创建的帧数据
 *
 * 功能说明：
 * 1. 支持透明色处理（包括全局背景色和帧局部透明色）
 * 2. 正确处理GIF的Disposal Method（0-3）
 * 3. 支持多帧合成画布模式
 * 4. 跨平台颜色格式处理（Windows和其他平台不同）
 */
static AnimationFramePtr UiGifToRgbaFrames(const GifFileType* gif,
                                           int32_t nFrameIndex,
                                           float fImageSizeScale,
                                           UiGifRGBA& backgroundColor,
                                           std::vector<UiGifRGBA>& canvas,
                                           std::vector<UiGifRGBA>& canvas_backup,
                                           int32_t& nLastFrameIndex)
{
    // 参数有效性检查
    ASSERT((gif != nullptr) && (gif->SavedImages != nullptr) && (gif->ImageCount > 0));
    if (gif == nullptr || gif->SavedImages == nullptr || gif->ImageCount <= 0) {
        return nullptr;
    }
    ASSERT((nFrameIndex >= 0) && (nFrameIndex < gif->ImageCount));
    if ((nFrameIndex < 0) || (nFrameIndex >= gif->ImageCount)) {
        return nullptr;
    }

    ASSERT(nLastFrameIndex == (nFrameIndex - 1));
    if (nLastFrameIndex != (nFrameIndex - 1)) {
        //必须逐帧解码，无法跳过帧
        return nullptr;
    }

    // 检查GIF宽度和高度是否有效
    ASSERT((gif->SWidth > 0) && (gif->SHeight > 0));
    if ((gif->SWidth <= 0) || (gif->SHeight <= 0)) {
        return nullptr;
    }

    // 获取渲染工厂实例
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory == nullptr) {
        return nullptr;
    }

    const int nImageWidth = gif->SWidth;
    const int nImageHeight = gif->SHeight;
    const int canvas_pixel_count = nImageWidth * nImageHeight;

    // 初始化画布（考虑全局背景色）
    if (canvas.empty()) {
        backgroundColor = UiGifRGBA{ 0, 0, 0, 0 }; //背景色
        canvas.resize(canvas_pixel_count);
        if (gif->SColorMap != nullptr && gif->SBackGroundColor >= 0 &&
            gif->SBackGroundColor < gif->SColorMap->ColorCount) {
            // 使用全局背景色初始化画布
            const GifColorType& bg_color = gif->SColorMap->Colors[gif->SBackGroundColor];
#ifdef DUILIB_BUILD_FOR_WIN
            UiGifRGBA init_color = {
                static_cast<uint8_t>(bg_color.Blue),
                static_cast<uint8_t>(bg_color.Green),
                static_cast<uint8_t>(bg_color.Red),
                0 // 初始背景设为透明
            };
#else
            UiGifRGBA init_color = {
                static_cast<uint8_t>(bg_color.Red),
                static_cast<uint8_t>(bg_color.Green),
                static_cast<uint8_t>(bg_color.Blue),
                0 // 初始背景设为透明
            };
#endif
            backgroundColor = init_color;
            std::fill(canvas.begin(), canvas.end(), init_color);
        }
        else {
            // 无有效背景色，初始化为全透明
            std::fill(canvas.begin(), canvas.end(), UiGifRGBA{ 0, 0, 0, 0 });
        }
    }

    // 预创建所有帧的位图对象
    std::shared_ptr<IBitmap> pBitmap(pRenderFactory->CreateBitmap());
    if (pBitmap == nullptr) {
        return nullptr;
    }

    AnimationFramePtr pFrameData = std::make_shared<IAnimationImage::AnimationFrame>();
    pFrameData->m_nFrameIndex = nFrameIndex;
    pFrameData->m_nOffsetX = 0; // OffsetX和OffsetY均不需要处理
    pFrameData->m_nOffsetY = 0;
    pFrameData->m_bDataPending = false;
    pFrameData->m_pBitmap = pBitmap;

    // 逐帧处理
    const SavedImage& frame = gif->SavedImages[nFrameIndex];
    const GifImageDesc& img_desc = frame.ImageDesc;

    // 设置帧延迟时间
    pFrameData->SetDelayMs(UiGifGetFrameDelayMs(frame));

    // 获取帧信息
    const ColorMapObject* colormap = UiGifGetFrameColorMap(gif, frame);
    const int transparent_idx = UiGifGetTransparentIndex(frame);
    const int disposal_method = UiGifGetDisposalMethod(frame);

    // 备份画布（用于Disposal Method=3）
    if (disposal_method == 3) {
        canvas_backup = canvas;
    }

    // 处理当前帧像素
    if (colormap != nullptr) {
        const GifByteType* raster_bits = frame.RasterBits;
        for (int y = 0; y < img_desc.Height; ++y) {
            for (int x = 0; x < img_desc.Width; ++x) {
                const int canvas_x = img_desc.Left + x;
                const int canvas_y = img_desc.Top + y;

                if (canvas_x < 0 || canvas_x >= nImageWidth ||
                    canvas_y < 0 || canvas_y >= nImageHeight) {
                    continue;
                }

                const int pixel_idx = canvas_y * nImageWidth + canvas_x;
                const GifByteType color_idx = raster_bits[y * img_desc.Width + x];

                if (color_idx >= colormap->ColorCount) {
                    continue;
                }

                // 透明处理逻辑
                const GifColorType& color = colormap->Colors[color_idx];
                uint8_t alpha = 255;
                if ((transparent_idx >= 0) && (color_idx == transparent_idx)) {
                    alpha = 0; // 明确指定的透明色
                }

#ifdef DUILIB_BUILD_FOR_WIN
                canvas[pixel_idx] = {
                    static_cast<uint8_t>(color.Blue),
                    static_cast<uint8_t>(color.Green),
                    static_cast<uint8_t>(color.Red),
                    alpha
                };
#else
                canvas[pixel_idx] = {
                    static_cast<uint8_t>(color.Red),
                    static_cast<uint8_t>(color.Green),
                    static_cast<uint8_t>(color.Blue),
                    alpha
                };
#endif
            }
        }
    }

    // 处理Disposal Method
    switch (disposal_method) {
    case 2: // 清除当前帧区域, 恢复为背景色
        for (int y = 0; y < img_desc.Height; ++y) {
            for (int x = 0; x < img_desc.Width; ++x) {
                const int canvas_x = img_desc.Left + x;
                const int canvas_y = img_desc.Top + y;
                if (canvas_x >= 0 && canvas_x < nImageWidth &&
                    canvas_y >= 0 && canvas_y < nImageHeight) {
                    canvas[canvas_y * nImageWidth + canvas_x] = backgroundColor;
                }
            }
        }
        break;
    case 3: // 恢复上一帧画布
        canvas = canvas_backup;
        break;
    default:
        break;
    }

    // 更新位图数据
    pFrameData->m_pBitmap->Init(nImageWidth, nImageHeight, canvas.data(), fImageSizeScale);
    nLastFrameIndex = nFrameIndex;
    return pFrameData;
}

struct Image_GIF::TImpl
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

    //GIF图片的背景色
    UiGifRGBA m_backgroundColor;

    //GIF图片绘制的画布
    std::vector<UiGifRGBA> m_gifCanvas;

    //GIF图片绘制的画布（备份）
    std::vector<UiGifRGBA> m_gifCanvasBackup;

    //上一帧的索引号
    int32_t m_nLastFrameIndex = -1;
};

Image_GIF::Image_GIF()
{
    m_impl = std::make_unique<TImpl>();
}

Image_GIF::~Image_GIF()
{
    if (m_impl->m_gifDecoder != nullptr) {
        UiGifFreeDecoder(m_impl->m_gifDecoder, nullptr);
        m_impl->m_gifDecoder = nullptr;
    }
}

bool Image_GIF::LoadImageFromMemory(std::vector<uint8_t>& fileData,
                                    bool bLoadAllFrames,
                                    bool bAsyncDecode,
                                    float fImageSizeScale,
                                    const UiSize& rcMaxDestRectSize)
{
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return false;
    }
    m_impl->m_fileData.clear();
    m_impl->m_fileData.swap(fileData);
    m_impl->m_fImageSizeScale = fImageSizeScale;
    m_impl->m_bLoadAllFrames = bLoadAllFrames;
    m_impl->m_bAsyncDecode = bAsyncDecode;

    int nErrorCode = 0;
    GifFileType* dec = UiGifInitDecoder(m_impl->m_fileData.data(), m_impl->m_fileData.size(), &nErrorCode);
    if (dec == nullptr) {
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }
    if (DGifSlurp(dec) != GIF_OK) {
        UiGifFreeDecoder(dec, nullptr);
        //加载失败时，需要恢复原文件数据
        m_impl->m_fileData.swap(fileData);
        return false;
    }

    m_impl->m_nWidth = (uint32_t)dec->SWidth;
    m_impl->m_nHeight = (uint32_t)dec->SHeight;
    m_impl->m_nFrameCount = (int32_t)dec->ImageCount;

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
    ASSERT(m_impl->m_nWidth > 0);
    ASSERT(m_impl->m_nHeight > 0);
    ASSERT(m_impl->m_nFrameCount > 0);

    if ((m_impl->m_nFrameCount <= 0) || ((int32_t)m_impl->m_nWidth <= 0) || ((int32_t)m_impl->m_nHeight <= 0)) {
        //加载失败时，需要恢复原文件数据
        UiGifFreeDecoder(dec, nullptr);
        m_impl->m_fileData.swap(fileData);
        return false;
    }
    //循环播放固定为一直播放，因GIF格式无此设置
    m_impl->m_nLoops = -1;

    //解出每一帧的播放时间
    m_impl->m_framesDelayMs.clear();
    for (int frame_idx = 0; frame_idx < m_impl->m_nFrameCount; ++frame_idx) {
        const SavedImage& frame = dec->SavedImages[frame_idx];
        m_impl->m_framesDelayMs.push_back(UiGifGetFrameDelayMs(frame));
    }

    m_impl->m_gifDecoder = dec;
    m_impl->m_gifCanvas.clear();
    m_impl->m_gifCanvasBackup.clear();
    m_impl->m_nLastFrameIndex = -1;
    return true;
}

bool Image_GIF::IsDelayDecodeEnabled() const
{
    if (m_impl->m_bAsyncDecode &&
        !m_impl->m_fileData.empty() &&
        (m_impl->m_gifDecoder != nullptr)) {
        return true;
    }
    return false;
}

bool Image_GIF::IsDelayDecodeFinished() const
{
    if (m_impl->m_bAsyncDecoding) {
        return false;
    }
    return (m_impl->m_frames.size() + m_impl->m_delayFrames.size()) == m_impl->m_nFrameCount;
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

bool Image_GIF::DelayDecode(uint32_t nMinFrameIndex, std::function<bool(void)> IsAborted)
{
    if (!IsDelayDecodeEnabled()) {
        return false;
    }
    if (m_impl->m_bAsyncDecoding) {
        return false;
    }
    m_impl->m_bAsyncDecoding = true;

    float fImageSizeScale = m_impl->m_fImageSizeScale;
    uint32_t nFrameIndex = (uint32_t)(m_impl->m_delayFrames.size() + m_impl->m_frames.size());
    while (((IsAborted == nullptr) || !IsAborted()) &&
           (nMinFrameIndex >= (int32_t)(m_impl->m_frames.size() + m_impl->m_delayFrames.size())) &&
           ((m_impl->m_frames.size() + m_impl->m_delayFrames.size()) < m_impl->m_nFrameCount)) {
        AnimationFramePtr pNewAnimationFrame;
        //一次解码一帧图片
        pNewAnimationFrame = UiGifToRgbaFrames(m_impl->m_gifDecoder,
                                               nFrameIndex,
                                               fImageSizeScale,
                                               m_impl->m_backgroundColor,
                                               m_impl->m_gifCanvas,
                                               m_impl->m_gifCanvasBackup,
                                               m_impl->m_nLastFrameIndex);
        if (pNewAnimationFrame != nullptr) {
            m_impl->m_delayFrames.push_back(pNewAnimationFrame);
        }
        else {
            break;
        }
        ++nFrameIndex;
    }

    m_impl->m_bAsyncDecoding = false;
    return true;
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
        if (m_impl->m_frames.size() == m_impl->m_nFrameCount) {
            if (m_impl->m_gifDecoder != nullptr) {
                UiGifFreeDecoder(m_impl->m_gifDecoder, nullptr);
                m_impl->m_gifDecoder = nullptr;
            }
            std::vector<uint8_t> fileData;
            m_impl->m_fileData.swap(fileData);

            std::vector<UiGifRGBA> gifCanvas;
            m_impl->m_gifCanvas.swap(gifCanvas);

            std::vector<UiGifRGBA> gifCanvasBackup;
            m_impl->m_gifCanvasBackup.swap(gifCanvas);
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

bool Image_GIF::ReadFrameData(int32_t nFrameIndex, AnimationFrame* pAnimationFrame)
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
            (m_impl->m_frames.size() < m_impl->m_nFrameCount)) {
            ASSERT(m_impl->m_delayFrames.empty());
            uint32_t nInitFrameIndex = (uint32_t)m_impl->m_frames.size();
            float fImageSizeScale = m_impl->m_fImageSizeScale;

            //一次解码一帧图片
            AnimationFramePtr pNewAnimationFrame;
            pNewAnimationFrame = UiGifToRgbaFrames(m_impl->m_gifDecoder,
                                                   nInitFrameIndex,
                                                   fImageSizeScale,
                                                   m_impl->m_backgroundColor,
                                                   m_impl->m_gifCanvas,
                                                   m_impl->m_gifCanvasBackup,
                                                   m_impl->m_nLastFrameIndex);

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
