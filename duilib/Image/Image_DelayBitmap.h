#ifndef UI_IMAGE_IMAGE_DELAY_BITMAP_H_
#define UI_IMAGE_IMAGE_DELAY_BITMAP_H_

#include "duilib/Image/ImageDecoder.h"
#include <atomic>
#include <functional>

namespace ui
{
/** 单帧位图图片的延迟解码实现
*   加载阶段只解析图片的宽度和高度等基本信息，像素解码放到异步解码线程中执行，
*   用于让加载阶段的解码器（BMP/DIB、ICO 等）也能走异步解码流程，避免在UI线程做整体解码
*/
class Image_DelayBitmap : public IBitmapImage
{
public:
    /** 执行像素解码的回调函数（在解码线程中执行）
    * @param [out] imageData 返回解码后的图像数据
    * @return 返回true表示解码成功，否则返回false
    */
    using DecodeFunction = std::function<bool(UiImageData& imageData)>;

public:
    Image_DelayBitmap();
    virtual ~Image_DelayBitmap() override;

    /** 初始化图片信息（不执行像素解码）
    * @param [in] nWidth 原图宽度（从图片文件头解析得到，未按缩放比例换算）
    * @param [in] nHeight 原图高度（从图片文件头解析得到，未按缩放比例换算）
    * @param [in] fImageSizeScale 原图加载的缩放比例
    * @param [in] bAsyncDecode 是否支持异步线程解码
    * @param [in] decodeFunction 执行像素解码的回调函数
    * @return 返回true表示初始化成功
    */
    bool InitImage(uint32_t nWidth, uint32_t nHeight, float fImageSizeScale,
                   bool bAsyncDecode, DecodeFunction decodeFunction);

public:
    /** 获取图片宽度（按缩放比例换算后的绘制宽度）
    */
    virtual uint32_t GetWidth() const override;

    /** 获取图片高度（按缩放比例换算后的绘制高度）
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
    */
    virtual bool IsDelayDecodeEnabled() const override;

    /** 延迟解码图片数据是否完成
    */
    virtual bool IsDelayDecodeFinished() const override;

    /** 获取当前延迟解码完成的图片帧索引号（从0开始编号）
    */
    virtual uint32_t GetDecodedFrameIndex() const override;

    /** 设置异步解码的状态
    */
    virtual void SetAsyncDecoding(bool bDecoding) override;

    /** 延迟解码图片数据（可以在多线程中调用）
    */
    virtual bool DelayDecode(uint32_t nMinFrameIndex,
                             std::function<bool(void)> IsAborted,
                             bool* bDecodeError) override;

    /** 合并延迟解码图片数据的结果
    */
    virtual bool MergeDelayDecodeData() override;

private:
    /** 执行像素解码，并生成位图数据（可以在多线程中调用）
    */
    std::shared_ptr<IBitmap> DecodeToBitmap();

private:
    /** 图片绘制宽度（原图宽度按缩放比例换算后的值）
    */
    uint32_t m_nWidth;

    /** 图片绘制高度（原图高度按缩放比例换算后的值）
    */
    uint32_t m_nHeight;

    /** 原图加载时的缩放比例
    */
    float m_fImageSizeScale;

    /** 是否支持异步线程解码
    */
    bool m_bAsyncDecode;

    /** 执行像素解码的回调函数
    */
    DecodeFunction m_decodeFunction;

    /** 是否遇到解码错误（工作线程写入，UI线程仅在非解码期间读取）
    */
    std::atomic<bool> m_bDecodeError;

    /** 是否正在异步解码中（UI线程提交任务前置位，工作线程结束时清除）
    */
    std::atomic<bool> m_bAsyncDecoding;

    /** 是否已经完成延迟解码（工作线程写入m_pDelayBitmap后置位，供UI线程安全判断解码状态）
    */
    std::atomic<bool> m_bDelayDecoded;

    /** 位图数据（UI线程访问）
    */
    std::shared_ptr<IBitmap> m_pBitmap;

    /** 位图数据（延迟解码，工作线程写入，UI线程在m_bDelayDecoded置位后读取）
    */
    std::shared_ptr<IBitmap> m_pDelayBitmap;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_DELAY_BITMAP_H_
