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
    /** 判断该图片的大小是否已经做过适应DPI处理
    */
    bool IsBitmapSizeDpiScaled() const;

    /** 获取生成图片加载时的DPI界面缩放比
    */
    uint32_t GetLoadDpiScale() const;

    /** 获取图片宽度（注意：这个宽度与图片位图的宽度不一定相同，这个宽度用于计算布局的宽度）
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
    * @param [in] fImageSizeScale 图片缩放的比例
    */
    std::shared_ptr<IBitmap> GetSvgBitmap(float fImageSizeScale);

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
    std::shared_ptr<IAnimationImage::AnimationFrame> GetFrame(uint32_t nFrameIndex);

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
    * @param [in] nImageInfoWidth 图片信息的宽度
    * @param [in] nImageInfoHeight 图片信息的高度
    * @param [in] pImageData 原图数据接口
    * @param [in] bBitmapSizeDpiScaled 加载的图片宽高是否做过DPI自适应
    */
    bool SetImageData(const ImageLoadParam& loadParam,
                      int32_t nImageInfoWidth,
                      int32_t nImageInfoHeight,
                      const std::shared_ptr<IImage>& pImageData,
                      bool bBitmapSizeDpiScaled);

    /** 获取关联的原图的图片数据接口
    */
    std::shared_ptr<IImage> GetImageData() const;

private:
    /** 释放图片资源（延迟释放，以便于共享）
    */
    void ReleaseImage();

    /** 获取多帧图片的接口
    */
    std::shared_ptr<IAnimationImage> GetAnimationImage(uint32_t nFrameIndex) const;

private:
    /** 实际图片的KEY, 用于图片的生命周期管理（多个DPI的图片，实际可能指向同一个文件）
    */
    UiString m_imageKey;

    /** 图片的加载参数
    */
    ImageLoadParam m_loadParam;

    /** 是否做过DPI自适应
    */
    bool m_bBitmapSizeDpiScaled;

    /** 是否有用户自定义的缩放比例
    */
    bool m_bHasCustomSizeScale;

    /** 用户自定义的缩放比例 X/Y方向
    */
    float m_fCustomSizeScaleX;
    float m_fCustomSizeScaleY;

    /** 原图加载时的缩放比例
    */
    float m_fImageSizeScale;

private:
    /** 循环播放次数(大于等于0，如果等于0，表示动画是循环播放的, APNG格式支持设置循环播放次数)
    */
    int32_t m_nLoopCount;

    /** 当前图片的总帧数
    */
    uint32_t m_nFrameCount;

    /** 图片的宽度
    */
    int32_t m_nWidth;

    /**图片的高度
    */
    int32_t m_nHeight;

    /** 原图的图像接口(解码绘制的图片数据由此原图中提取，提取完成后，不再使用，可以释放，以减少内存占用)
    *   使用完成后，放到ImageManager::m_delayReleaseImageList中，延迟释放，以实现原图共享功能
    */
    std::shared_ptr<IImage> m_pImageData;

private:
    /** 图片类型
    */
    ImageType m_imageType;

    /** 图片数据(单帧，缓存)
    */
    std::shared_ptr<IBitmap> m_pBitmap;

    /** 图片数据(多帧，缓存)
    */
    std::vector<std::shared_ptr<IAnimationImage::AnimationFrame>> m_frameList;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_INFO_H_
