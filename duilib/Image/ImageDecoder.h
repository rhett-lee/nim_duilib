#ifndef UI_IMAGE_IMAGE_DECODER_H_
#define UI_IMAGE_IMAGE_DECODER_H_

#include "duilib/Core/UiTypes.h"

namespace ui
{
/** 位图接口
*/
class IBitmap;

/** 支持多线程解码的接口（支持延迟解码，可以在多线程中解码，避免在UI线程解码图片导致卡顿）
*/
class UILIB_API IImageDelayDecode
{
public:
    virtual ~IImageDelayDecode() = default;

    /** 是否需要解码数据
    * @return 返回true表示需要解码，返回false表示不需要解码
    */
    virtual bool IsDecodeImageDataEnabled() const = 0;

    /** 设置开始解码数据（放入队列）
    */
    virtual void SetDecodeImageDataStarted() = 0;

    /** 解码数据（可以在多线程中调用）
    */
    virtual bool DecodeImageData() = 0;

    /** 设置终止解码数据（关联的Image对象已经销毁）
    */
    virtual void SetDecodeImageDataAborted() = 0;
};

/** SVG矢量图片接口
*/
class UILIB_API ISvgImage
{
public:
    virtual ~ISvgImage() = default;

    /** 获取图片宽度
    */
    virtual uint32_t GetWidth() const = 0;

    /** 获取图片高度
    */
    virtual uint32_t GetHeight() const = 0;

    /** 获取指定大小的位图，矢量缩放
    * @param [in] szImageSize 代表获取图片的宽度(cx)和高度(cy)
    */
    virtual std::shared_ptr<IBitmap> GetBitmap(const UiSize& szImageSize) = 0;
};

/** 动画图片默认的播放时间间隔（毫秒）
*/
#define IMAGE_ANIMATION_DELAY_MS        (100)

/** 动画图片默认的播放时间间隔最小值（毫秒）
*/
#define IMAGE_ANIMATION_DELAY_MS_MIN    (20) 

/** 动画图片接口
*/
class UILIB_API IAnimationImage: public IImageDelayDecode
{
public:
    virtual ~IAnimationImage() = default;

    /** 多帧图片的一帧图片数据
    */
    class UILIB_API AnimationFrame
    {
    public:
        bool m_bDataPending = false;        //数据是否处于待解码状态：true表示待解码，需要等待解码完成后再使用
        int32_t m_nFrameIndex = -1;         //图片帧的索引号        
        int32_t m_nOffsetX = 0;             //该帧图片在绘制区域的X轴偏移值，单位为像素
        int32_t m_nOffsetY = 0;             //该帧图片在绘制区域的Y轴偏移值，单位为像素
        std::shared_ptr<IBitmap> m_pBitmap; //该帧图片的位图数据，用于绘制

        /** 设置帧播放持续时间，毫秒
        */
        void SetDelayMs(int32_t nDelayMs)
        {
            if (nDelayMs <= 0) {
                //未设置时，设置为默认值
                nDelayMs = IMAGE_ANIMATION_DELAY_MS;
            }
            else if (nDelayMs < IMAGE_ANIMATION_DELAY_MS_MIN) {
                //低于最小值时，设置为最小值
                nDelayMs = IMAGE_ANIMATION_DELAY_MS_MIN;
            }
            m_nDelayMs = nDelayMs;
        }

        /** 获取帧播放持续时间，毫秒
        */
        int32_t GetDelayMs() const
        {
            return m_nDelayMs;
        }

    private:
        int32_t m_nDelayMs = IMAGE_ANIMATION_DELAY_MS;  //图片帧的播放持续时间，单位为毫秒
    };
public:
    /** 获取图片宽度
    */
    virtual uint32_t GetWidth() const = 0;

    /** 获取图片高度
    */
    virtual uint32_t GetHeight() const = 0;

    /** 获取图片的帧数
    */
    virtual int32_t GetFrameCount() const = 0;

    /** 获取循环播放的次数
    * @return 返回循环播放的次数，-1表示一直播放
    */
    virtual int32_t GetLoopCount() const = 0;

    /** 读取一帧数据
    * @param [in] nFrameIndex 帧的索引号，从0开始编号的下标值
    * @param [out] pAnimationFrame 返回该帧的图片位图数据
    * @return 成功返回true，失败则返回false
    */
    virtual bool ReadFrame(int32_t nFrameIndex, AnimationFrame* pAnimationFrame) = 0;
};

/** 图片类型
*/
enum class UILIB_API ImageType
{
    kImageBitmap,       //位图类型，单帧，图片尺寸缩放时是有损缩放，显示效果会变差
    kImageSvg,          //SVG矢量图，单帧，图片尺寸缩放时是矢量缩放，显示效果较好
    kImageAnimation     //动画图片，多帧
};

/** 原图加载的宽度和高度缩放比例：无缩放的值
*/
#define IMAGE_SIZE_SCALE_NONE (1.0f) 

/** 图片接口
*/
class UILIB_API IImage
{
public:
    virtual ~IImage() = default;

public:
    /** 获取图片宽度
    */
    virtual int32_t GetWidth() const = 0;

    /** 获取图片高度
    */
    virtual int32_t GetHeight() const = 0;

    /** 原图加载的宽度和高度缩放比例(1.0f表示无缩放)
    */
    virtual float GetImageSizeScale() const = 0;

    /** 获取图片的类型
    */
    virtual ImageType GetImageType() const = 0;

    /** 获取图片数据
    * @return 仅当ImageType==ImageType::kImageBitmap时返回图片数据
    */
    virtual std::shared_ptr<IBitmap> GetImageBitmap() const { return nullptr; }

    /** 获取图片数据
    * @return 仅当ImageType==ImageType::kImageSvg时返回图片数据
    */
    virtual std::shared_ptr<ISvgImage> GetImageSvg() const { return nullptr; }

    /** 获取图片数据
    * @return 仅当ImageType==ImageType::kImageAnimation时返回图片数据
    */
    virtual std::shared_ptr<IAnimationImage> GetImageAnimation() const { return nullptr; }
};

/** 图片解码器接口
*/
class IImageDecoder
{
public:
    /** 图片解码的额外参数
    */
    struct ExtraParam
    {
        bool m_bLoadAllFrames = true;       //对于多帧图片，是否加载所有帧（true表示加载所有帧；false表示只加载第1帧, 按单帧图片加载）
        bool m_bIconAsAnimation = false;    //ICO格式，是否按照动画来加载多帧显示（默认情况下，ICO格式是按单帧显示的）
        uint32_t m_nIconSize = 32;          //ICO格式，加载图标的大小值
        uint32_t m_nIconFrameDelayMs = 1000;//ICO格式，每帧播放时间间隔，毫秒（仅当m_bIconAsAnimation && m_bLoadAllFrames为true时有效）
        float m_fPagMaxFrameRate = 30.0f;   //PAG格式，解码动画的帧率
    };
public:
    virtual ~IImageDecoder() = default;
        
    /** 获取该解码器支持的格式名称
    */
    virtual DString GetFormatName() const = 0;

    /** 检查该解码器是否支持给定的文件名
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @param [out] bVirtualFile 返回true代表虚拟文件名，返回false代表实体文件名
    */
    virtual bool CanDecode(const DString& imageFileString, bool& bVirtualFile) const = 0;
         
    /** 检查该解码器是否支持给定的数据流
    * @param [in] data 数据的起始地址
    * @param [in] dataLen 数据的长度
    */
    virtual bool CanDecode(const uint8_t* data, size_t dataLen) const = 0;
   
    /** 加载解码图片数据，返回解码后的图像数据
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @param [in] data 待解码的数据, 容器的数据会被交换到内部实现（当imageFileString为虚拟文件名时，可为空）
    * @param [in] fImageSizeScale 解码图片大小的缩放比(解码后对图片执行resize操作，按比例调整图片的宽和高)
    * @param [in] pExtraParam 图片解码的额外参数
    */
    virtual std::unique_ptr<IImage> LoadImageData(const DString& imageFileString,
                                                  std::vector<uint8_t>& data,
                                                  float fImageSizeScale = 1.0f,
                                                  const IImageDecoder::ExtraParam* pExtraParam = nullptr) = 0;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_DECODER_H_
