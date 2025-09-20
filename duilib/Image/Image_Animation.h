#ifndef UI_IMAGE_IMAGE_ANIMATION_H_
#define UI_IMAGE_IMAGE_ANIMATION_H_

#include "duilib/Image/ImageDecoder.h"

namespace ui
{
/** 动画格式的图片数据
*/
class Image_Animation: public IImage
{
public:
    Image_Animation(const std::shared_ptr<IAnimationImage>& pAnimationImage,
                    float fImageSizeScale);
    virtual ~Image_Animation() override;

    /** 获取图片宽度
    */
    virtual int32_t GetWidth() const override;

    /** 获取图片高度
    */
    virtual int32_t GetHeight() const override;

    /** 原图加载的宽度和高度缩放比例(1.0f表示无缩放)
    */
    virtual float GetImageSizeScale() const override;

    /** 获取图片的类型
    */
    virtual ImageType GetImageType() const override;

    /** 获取图片数据
    * @return 仅当ImageType==ImageType::kImageAnimation时返回图片数据
    */
    virtual std::shared_ptr<IAnimationImage> GetImageAnimation() const override;

private:
    /** 是否需要异步解码图片数据
    * @return 返回true表示需要解码，返回false表示不需要解码
    */
    virtual bool IsAsyncDecodeEnabled() const override;

    /** 异步解码图片数据是否完成
    * @return 异步解码图片数据操作已经完成
    */
    virtual bool IsAsyncDecodeFinished() const override;

    /** 获取当前异步解码完成的图片帧索引号（从0开始编号）
    */
    virtual uint32_t GetDecodedFrameIndex() const override;

    /** 设置异步解码的任务ID
    * @param [in] nTaskId 在子线程中的任务ID
    */
    virtual void SetAsyncDecodeTaskId(size_t nTaskId) override;

    /** 获取异步解码的任务ID
    */
    virtual size_t GetAsyncDecodeTaskId() const override;

    /** 异步解码图片数据（可以在多线程中调用）
    * @param [in] nMinFrameIndex 至少需要解码到哪一帧（帧索引号，从0开始编号）
    * @param [in] IsAborted 解码终止终止测试函数，返回true表示终止，否则表示正常操作
    * @return 返回true表示成功，返回false表示解码失败或者外部终止
    */
    virtual bool AsyncDecode(uint32_t nMinFrameIndex, std::function<bool(void)> IsAborted) override;

    /** 合并异步解码图片数据的结果
    */
    virtual bool MergeAsyncDecodeData() override;

private:
    /** 动画图片
    */
    std::shared_ptr<IAnimationImage> m_pAnimationImage;

    /** 图片的缩放比例
    */
    float m_fImageSizeScale;

    /** 异步解码任务ID
    */
    size_t m_nAsyncDecodeTaskId;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_ANIMATION_H_
