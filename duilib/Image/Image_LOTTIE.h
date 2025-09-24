#ifndef UI_IMAGE_IMAGE_LOTTIE_H_
#define UI_IMAGE_IMAGE_LOTTIE_H_

#include "duilib/Image/ImageDecoder.h"

namespace ui
{
/** LOTTIE格式的图片数据
*/
class Image_LOTTIE : public IAnimationImage
{
public:
    Image_LOTTIE();
    virtual ~Image_LOTTIE() override;

public:
    /** 加载图像数据
    * @param [in] fileData 图片文件数据
    * @param [in] bLoadAllFrames 是否加载全部帧，如果为false只加载第1帧，如果为true则加载全部帧
    * @param [in] fImageSizeScale 图片缩放百分比
    * @param [in] rcMaxDestRectSize 目标区域大小，用于优化加载性能
    */
    bool LoadImageFromMemory(std::vector<uint8_t>& fileData,
                             bool bLoadAllFrames,
                             float fImageSizeScale,
                             const UiSize& rcMaxDestRectSize);

public:
    /** 获取图片宽度
    */
    virtual uint32_t GetWidth() const override;

    /** 获取图片高度
    */
    virtual uint32_t GetHeight() const override;

    /** 原图加载的宽度和高度缩放比例(1.0f表示无缩放)
    */
    virtual float GetImageSizeScale() const override;

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
    /** 是否支持延迟解码数据
    * @return 返回true表示需要解码，返回false表示不需要解码
    */
    virtual bool IsDelayDecodeEnabled() const override;

    /** 延迟解码图片数据是否完成
    * @return 延迟解码图片数据操作已经完成
    */
    virtual bool IsDelayDecodeFinished() const override;

    /** 获取当前延迟解码完成的图片帧索引号（从0开始编号）
    */
    virtual uint32_t GetDecodedFrameIndex() const override;

    /** 延迟解码图片数据（可以在多线程中调用）
    * @param [in] nMinFrameIndex 至少需要解码到哪一帧（帧索引号，从0开始编号）
    * @param [in] IsAborted 解码终止终止测试函数，返回true表示终止，否则表示正常操作
    * @return 返回true表示成功，返回false表示解码失败或者外部终止
    */
    virtual bool DelayDecode(uint32_t nMinFrameIndex, std::function<bool(void)> IsAborted) override;

    /** 合并延迟解码图片数据的结果
    */
    virtual bool MergeDelayDecodeData() override;

private:
    /** 私有实现数据
    */
    struct TImpl;
    std::unique_ptr<TImpl> m_impl;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_LOTTIE_H_
