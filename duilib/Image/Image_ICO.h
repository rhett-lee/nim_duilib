#ifndef UI_IMAGE_IMAGE_ICO_H_
#define UI_IMAGE_IMAGE_ICO_H_

#include "duilib/Image/ImageDecoder.h"
#include "duilib/Image/ImageDecoderUtil.h"

namespace ui
{
/** ICO格式的图片数据(多帧模式)
*/
class Image_ICO: public IAnimationImage
{
public:
    Image_ICO();
    virtual ~Image_ICO() override;

public:
    /** 加载图像数据
    * @param [in] imageData 图片文件数据
    * @param [in] fImageSizeScale 图片缩放百分比
    * @param [in] nIconSize 指定需要加载的ICO图片的大小
    * @param [in] nFrameDelayMs 每帧的时间间隔，毫秒
    */
    bool LoadImageFromMemory(const std::vector<UiImageData>& imageData,
                             float fImageSizeScale,
                             uint32_t nIconSize,
                             int32_t nFrameDelayMs);

public:
    /** 获取图片宽度
    */
    virtual uint32_t GetWidth() const override;

    /** 获取图片高度
    */
    virtual uint32_t GetHeight() const override;

    /** 获取图片的帧数
    */
    virtual int32_t GetFrameCount() const override;

    /** 获取循环播放的次数
    * @return 返回循环播放的次数，-1表示一直播放
    */
    virtual int32_t GetLoopCount() const override;

    /** 查询是某帧的图片数据是否有准备完成（多线程解码时，帧数据在后台线程解码）
    * @param [in] nFrameIndex 图片帧的索引号，从0开始编号的下标值，取值范围:[0, GetFrameCount())
    */
    virtual bool IsFrameDataReady(uint32_t nFrameIndex) override;

    /** 获取一个图片帧的播放持续时间，单位为毫秒
    * @param [in] nFrameIndex 图片帧的索引号，从0开始编号的下标值，取值范围:[0, GetFrameCount())
    */
    virtual int32_t GetFrameDelayMs(uint32_t nFrameIndex) override;

    /** 读取一帧数据
    * @param [in] nFrameIndex 图片帧的索引号，从0开始编号的下标值，取值范围:[0, GetFrameCount())
    * @param [out] pAnimationFrame 返回该帧的图片位图数据
    * @return 成功返回true，失败则返回false
    */
    virtual bool ReadFrameData(int32_t nFrameIndex, AnimationFrame* pAnimationFrame) override;

public:
    /** 是否需要解码数据
    * @return 返回true表示需要解码，返回false表示不需要解码
    */
    virtual bool IsDecodeImageDataEnabled() const override;

    /** 设置开始解码数据（放入队列）
    */
    virtual void SetDecodeImageDataStarted() override;

    /** 解码数据（可以在多线程中调用）
    */
    virtual bool DecodeImageData() override;

    /** 设置终止解码数据（关联的Image对象已经销毁）
    */
    virtual void SetDecodeImageDataAborted() override;

private:
    /** 私有实现数据
    */
    struct TImpl;
    std::unique_ptr<TImpl> m_impl;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_ICO_H_
