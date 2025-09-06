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

    /** 读取一帧数据
    * @param [in] nFrameIndex 帧的索引号，从0开始编号的下标值
    * @param [out] pAnimationFrame 返回该帧的图片位图数据
    * @return 成功返回true，失败则返回false
    */
    virtual bool ReadFrame(int32_t nFrameIndex, AnimationFrame* pAnimationFrame) override;

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
