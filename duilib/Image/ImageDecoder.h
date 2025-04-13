#ifndef UI_IMAGE_IMAGE_DECODER_H_
#define UI_IMAGE_IMAGE_DECODER_H_

#include "duilib/duilib_defs.h"
#include <memory>
#include <vector>
#include <string>

namespace ui 
{
class ImageInfo;
class ImageLoadAttribute;

/** 图片格式解码类
*/
class UILIB_API ImageDecoder
{
public:
    /** 从内存文件数据中加载图片并解码图片数据, 宽和高属性可以只设置一个，另外一个属性则默认按源图片等比计算得出
    * @param [in] fileData 图片文件的数据，部分格式加载过程中内部有增加尾0的写操作
    * @param [in] imageLoadAttribute 图片加载属性, 包括图片路径等
    * @param [in] bEnableDpiScale 是否允许按照DPI对图片大小进行缩放（此为功能开关）
    * @param [in] nImageDpiScale 图片数据对应的DPI缩放百分比（比如：i.jpg为100，i@150.jpg为150）
    * @param [in] nWindowDpiScale 显示目标窗口的DPI缩放百分比
    * @param [in] bLoadAllFrames 当遇到多帧图片时：true表示加载全部图片帧，false表示仅加载第一帧
    * @param [out] nFrameCount 返回图片共有多少帧
    */
    std::unique_ptr<ImageInfo> LoadImageData(std::vector<uint8_t>& fileData,
                                             const ImageLoadAttribute& imageLoadAttribute,                                             
                                             bool bEnableDpiScale, uint32_t nImageDpiScale, uint32_t nWindowDpiScale,
                                             bool bLoadAllFrames, uint32_t& nFrameCount);

public:
    /** 加载后的图片数据
    */
    struct ImageData
    {
        /** 位图数据，每帧图片的数据长度固定为：图像数据长度为 (m_imageHeight*m_imageWidth*4)
        *   每个像素数据固定占4个字节，格式为ARGB格式，位数顺序从高位到低位分别为[第3位:A，第2位:R，第1位:G,第0位:B]
        */
        std::vector<uint8_t> m_bitmapData;

        /** 图像宽度
        */
        uint32_t m_imageWidth = 0;

        /** 图像高度
        */
        uint32_t m_imageHeight = 0;

        /** 动画播放时的延迟时间，单位为毫秒
        */
        uint32_t m_frameInterval = 0;

        /** 创建位图的时候，是否需要翻转高度（不同加载引擎，此属性可能不同）
        */
        bool bFlipHeight = true;
    };

private:
    /** 对图片数据进行解码，生成位图数据
    * @param [in] fileData 原始图片数据
    * @param [in] imageLoadAttribute 图片的加载属性信息
    * @param [in] bLoadAllFrames 对于多帧图片，是否加载全部帧（true加载全部帧，false仅加载第1帧）
    * @param [in] bEnableDpiScale 是否允许按照DPI对图片大小进行缩放（此为功能开关）
    * @param [in] nImageDpiScale 图片数据对应的DPI缩放百分比（比如：i.jpg为100，i@150.jpg为150）
    * @param [in] nWindowDpiScale 显示目标窗口的DPI缩放百分比
    * @param [out] imageData 加载成功的图片数据，每个图片帧一个元素
    * @param [out] nFrameCount 返回图片总的帧数
    * @param [out] playCount 动画播放的循环次数(-1表示无效值；大于等于0时表示值有效，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)    
    * @param [out] bDpiScaled 图片加载的时候，图片大小是否进行了DPI自适应操作
    */
    bool DecodeImageData(std::vector<uint8_t>& fileData, 
                         const ImageLoadAttribute& imageLoadAttribute,
                         bool bLoadAllFrames,
                         bool bEnableDpiScale,
                         uint32_t nImageDpiScale,
                         uint32_t nWindowDpiScale,
                         std::vector<ImageData>& imageData,
                         uint32_t& nFrameCount,
                         int32_t& playCount,
                         bool& bDpiScaled);

    /** 对图片数据进行大小缩放
    * @param [in] imageData 需要缩放的图片数据
    * @param [in] nNewWidth 新的宽度
    * @param [in] nNewHeight 新的高度
    */
    bool ResizeImageData(std::vector<ImageData>& imageData, 
                         uint32_t nNewWidth,
                         uint32_t nNewHeight);

    /** 支持的图片文件格式
    */
    enum class ImageFormat {
        kUnknown,
        kPNG,
        kSVG,
        kGIF,
        kWEBP,
        kJPEG,
        kBMP,
        kICO
    };

    /** 根据图片文件的扩展名获取图片格式
    */
    static ImageFormat GetImageFormat(const DString& path);
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_DECODER_H_
