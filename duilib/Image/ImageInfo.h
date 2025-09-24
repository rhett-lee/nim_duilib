#ifndef UI_IMAGE_IMAGE_INFO_H_
#define UI_IMAGE_IMAGE_INFO_H_

#include "duilib/Render/IRender.h"
#include "duilib/Core/UiTypes.h"
#include "duilib/Image/ImageDecoder.h"
#include "duilib/Image/ImageLoadParam.h"

namespace ui 
{
    class IRender;
    class Control;
    class DpiManager;

/** 图片信息
*/
class UILIB_API ImageInfo
{
public:
    ImageInfo();
    ~ImageInfo();

    ImageInfo(const ImageInfo&) = delete;
    ImageInfo& operator = (const ImageInfo&) = delete;

public:
    /** 获取图片加载时的DPI界面缩放比（举例：100代表缩放百分比为100% ，无缩放）
    */
    uint32_t GetLoadDpiScale() const;

    /** 获取DPI自适应图片的DPI缩放比（举例：DPI缩放百分比为120（即放大到120%）的图片："image.png" 对应于 "image@120.png"）
    */
    uint32_t GetImageFileDpiScale() const;

    /** 获取图片宽度（注意：这个宽度与图片位图的宽度不一定相同，这个宽度用于计算布局的宽度）
    *   单帧图片/svg图片：这个宽度与图片的宽度一致
    *   
    */
    int32_t GetWidth() const;

    /** 获取图片高度（注意：这个宽度与图片位图的宽度不一定相同，这个宽度用于计算布局的宽度）
    */
    int32_t GetHeight() const;

public:
    /** 是否为SVG图片
    */
    bool IsSvgImage() const;

    /** 获取Svg图片的位图，支持矢量缩放
    * @param [in] rcDest 绘制目标区域
    * @param [in] rcSource 图片源区域
    */
    std::shared_ptr<IBitmap> GetSvgBitmap(const UiRect& rcDest, UiRect& rcSource);

public:
    /** 获取一个位图图片数据（单帧图片）
    */
    std::shared_ptr<IBitmap> GetBitmap();

public:
    /** 查询是某帧的图片数据是否有准备完成（多线程解码时，帧数据在后台线程解码）
    * @param [in] nFrameIndex 图片帧的索引号，从0开始编号的下标值，取值范围:[0, GetFrameCount())
    */
    bool IsFrameDataReady(uint32_t nFrameIndex);

    /** 获取一个图片帧数据
    * @param [in] nFrameIndex 图片帧的索引号，从0开始编号的下标值，取值范围:[0, GetFrameCount())
    */
    AnimationFramePtr GetFrame(uint32_t nFrameIndex);

    /** 获取一个图片帧的播放持续时间，单位为毫秒
    * @param [in] nFrameIndex 图片帧的索引号，从0开始编号的下标值，取值范围:[0, GetFrameCount())
    */
    int32_t GetFrameDelayMs(uint32_t nFrameIndex);

    /** 获取图片的帧数
    */
    uint32_t GetFrameCount() const;

    /** 是否位多帧图片(比如GIF等)
    */
    bool IsMultiFrameImage() const;

    /** 获取循环播放次数（TODO：未使用，需要修改播放的实现源码）
    *@return 返回值：-1 表示动画是一致循环播放的
    *              >= 0 表示动画循环播放的具体次数
    */
    int32_t GetLoopCount() const;

public:
    /** 获取图片的加载KEY
    */
    DString GetLoadKey() const;

    /** 设置实际图片的KEY, 用于图片的生命周期管理
    * @param [in] imageKey 实际图片时的KEY
    */
    void SetImageKey(const DString& imageKey);

    /** 获取实际图片的KEY
    */
    DString GetImageKey() const;

    /** 设置原图数据接口
    * @param [in] loadParam 加载参数
    * @param [in] pImageData 原图数据接口
    * @param [in] bEnableImageDpiScale 该图片是否支持DPI自适应
    * @param [in] nImageFileDpiScale DPI自适应图片的DPI缩放比
    */
    bool SetImageData(const ImageLoadParam& loadParam,
                      const std::shared_ptr<IImage>& pImageData,
                      bool bEnableImageDpiScale,
                      uint32_t nImageFileDpiScale);

    /** 获取关联的原图的图片数据接口
    */
    std::shared_ptr<IImage> GetImageData() const;

    /** 对图片的源区域、目标区域、圆角大小进行校验修正和DPI自适应
    * @param [in] dpi DPI缩放接口
    * @param [out] rcDestCorners 绘制目标区域的圆角信息，传出参数，内部根据rcImageCorners来设置，然后传出
    * @param [in/out] rcSource 图片区域
    * @param [in/out] rcSourceCorners 图片区域的圆角信息
    */
    void ScaleImageSourceRect(const DpiManager& dpi, UiRect& rcDestCorners, UiRect& rcSource, UiRect& rcSourceCorners);
    void ScaleImageSourceRect(const DpiManager& dpi, UiRect& rcSource);

private:
    /** 释放图片资源（延迟释放，以便于共享）
    */
    void ReleaseImage();

    /** 获取多帧图片的接口
    */
    std::shared_ptr<IAnimationImage> GetAnimationImage(uint32_t nFrameIndex) const;

    /** 计算ImageInfo的大小(该大小与图片本身的大小可能会不同)
    */
    void CalcImageInfoSize(const ImageLoadParam& loadParam,
                           const std::shared_ptr<IImage>& pImageData,
                           bool bEnableImageDpiScale,
                           uint32_t nImageFileDpiScale,
                           int32_t& nImageInfoWidth,
                           int32_t& nImageInfoHeight) const;

    /** 获取Svg图片的位图，支持矢量缩放
    * @param [in] fImageSizeScale 图片缩放的比例
    */
    std::shared_ptr<IBitmap> GetSvgBitmap(float fImageSizeScale);


private:
    /** 实际图片的KEY, 用于图片的生命周期管理（多个DPI的图片，实际可能指向同一个文件）
    */
    UiString m_imageKey;

    /** 图片的加载参数
    */
    ImageLoadParam m_loadParam;

    /** 是否有用户自定义的缩放比例/用户自定义的缩放比例 X/Y方向
    *   这几个变量，仅在多帧图片时使用
    */
    bool m_bHasCustomSizeScale;
    float m_fCustomSizeScaleX;
    float m_fCustomSizeScaleY;

    /** 获取DPI自适应图片的DPI缩放比（举例：DPI缩放百分比为120（即放大到120%）的图片："image.png" 对应于 "image@120.png"）
    */
    uint32_t m_nImageFileDpiScale;

    /** 是否支持图片DPI自适应
    */
    bool m_bEnableImageDpiScale;

private:
    /** 循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
    */
    int32_t m_nLoopCount;

    /** 当前图片的总帧数
    */
    uint32_t m_nFrameCount;

    /** 图片的宽度
    */
    int32_t m_nImageInfoWidth;

    /**图片的高度
    */
    int32_t m_nImageInfoHeight;

    /** 原图的图像接口(解码绘制的图片数据由此原图中提取，提取完成后，不再使用，可以释放，以减少内存占用)
    *   使用完成后，放到ImageManager::m_delayReleaseImageList中，延迟释放，以实现原图共享功能
    */
    std::shared_ptr<IImage> m_pImageData;

private:
    /** 图片类型
    */
    ImageType m_imageType;

    /** 图片数据(单帧，缓存；但svg图片无缓存)
    */
    std::shared_ptr<IBitmap> m_pBitmap;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_INFO_H_
