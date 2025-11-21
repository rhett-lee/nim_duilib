#ifndef UI_IMAGE_IMAGE_JPEG_H_
#define UI_IMAGE_IMAGE_JPEG_H_

#include "duilib/Image/ImageDecoder.h"
#include "duilib/Render/IRender.h"

#ifdef DUILIB_IMAGE_SUPPORT_JPEG_TURBO

namespace ui
{
/** JPEG格式的图片数据(加载时不做图像数据解码，在获取位图时解码图像数据)
*/
class Image_JPEG: public IBitmapImage
{
public:
    /** 加载图片数据
     * @param [in] fileData 图片文件数据(如果不为空，则优先使用文件数据)
     * @param [in] imageFilePath 图片文件路径
     * @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
     * @param [in] bAsyncDecode 是否支持异步线程解码图片数据
     * @param [in] rcMaxDestRectSize 目标区域大小，用于优化加载性能
     * @param [in] bAssertEnabled 当遇到图片数据错误时，是否允许断言
     */
    bool LoadImageFile(std::vector<uint8_t>& fileData,
                       const FilePath& imageFilePath,
                       float fImageSizeScale,
                       bool bAsyncDecode,
                       const UiSize& rcMaxDestRectSize,
                       bool bAssertEnabled);

public:
    Image_JPEG();
    virtual ~Image_JPEG() override;

    /** 获取图片宽度
    */
    virtual uint32_t GetWidth() const override;

    /** 获取图片高度
    */
    virtual uint32_t GetHeight() const override;

    /** 原图加载的宽度和高度缩放比例(1.0f表示无缩放)
    */
    virtual float GetImageSizeScale() const override;

    /** 获取位图
    * @param [out] bDecodeError 返回值代表是否遇到图片解码错误
    * @return 返回位图的接口指针，如果返回nullptr并且bDecodeError为false表示图片尚未完成解码（多线程解码的情况下）
    *                          如果返回nullptr并且bDecodeError为true代表图片解码出现错误
    */
    virtual std::shared_ptr<IBitmap> GetBitmap(bool* bDecodeError) override;

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
    * @param [out] bDecodeError 返回true表示遇到图片解码错误
    * @return 返回true表示成功，返回false表示解码失败或者外部终止
    */
    virtual bool DelayDecode(uint32_t nMinFrameIndex,
                             std::function<bool(void)> IsAborted,
                             bool* bDecodeError) override;

    /** 合并延迟解码图片数据的结果
    */
    virtual bool MergeDelayDecodeData() override;

private:
    /** 解码图片数据
    */
    std::shared_ptr<IBitmap> DecodeBitmap() const;

private:
    /** 私有实现数据
    */
    struct TImpl;
    std::unique_ptr<TImpl> m_impl;
};

} //namespace ui

#endif //DUILIB_IMAGE_SUPPORT_JPEG_TURBO
#endif //UI_IMAGE_IMAGE_JPEG_H_
