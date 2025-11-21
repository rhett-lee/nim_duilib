#ifndef UI_IMAGE_IMAGE_BITMAP_H_
#define UI_IMAGE_IMAGE_BITMAP_H_

#include "duilib/Image/ImageDecoder.h"
#include "duilib/Render/IRender.h"

namespace ui
{
/** 位图格式的图片数据
*/
class Image_Bitmap: public IImage
{
public:
    /** 创建一个位图的图片数据（ARGB格式）
    @param [in] nWidth 宽度
    @param [in] nHeight 高度
    @param [in] pPixelBits 位图数据, 如果为nullptr表示窗口空位图，如果不为nullptr，其数据长度为：nWidth*4*nHeight
    @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
    @param [in] alphaType 位图的Alpha类型    
    */
    static std::unique_ptr<IImage> MakeImage(uint32_t nWidth, uint32_t nHeight, const void* pPixelBits,
                                             float fImageSizeScale = 1.0f,
                                             BitmapAlphaType alphaType = BitmapAlphaType::kPremul_SkAlphaType);


    /** 创建一个位图的图片数据（ARGB格式）
    @param [in] pBitmap 已经生成的位图
    @param [in] fImageSizeScale 图片的缩放比例，1.0f表示原值
    */
    static std::unique_ptr<IImage> MakeImage(const std::shared_ptr<IBitmap>& pBitmap, float fImageSizeScale);

    /** 从一个支持延迟解码的位图对象创建
    */
    static std::unique_ptr<IImage> MakeImage(const std::shared_ptr<IBitmapImage>& pBitmap);

    /** 从一个支持延迟解码的动画图片对象创建(取第一帧)
    */
    static std::unique_ptr<IImage> MakeImage(const std::shared_ptr<IAnimationImage>& pAnimationImage);

public:
    Image_Bitmap();
    virtual ~Image_Bitmap() override;

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
    * @return 仅当ImageType==ImageType::kImageBitmap时返回图片数据
    */
    virtual std::shared_ptr<IBitmapImage> GetImageBitmap() const override;

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
    * @param [out] bDecodeError 返回true表示遇到图片解码错误
    * @return 返回true表示成功，返回false表示解码失败或者外部终止
    */
    virtual bool AsyncDecode(uint32_t nMinFrameIndex,
                             std::function<bool(void)> IsAborted,
                             bool* bDecodeError) override;

    /** 合并异步解码图片数据的结果
    */
    virtual bool MergeAsyncDecodeData() override;

private:
    //IBitmapImage接口的内部实现
    class BitmapImageImpl;

    /** 关联的IBitmapImage指针
    */
    std::shared_ptr<IBitmapImage> m_pBitmapImage;

    /** 异步解码任务ID
    */
    size_t m_nAsyncDecodeTaskId;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_BITMAP_H_
