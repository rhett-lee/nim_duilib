#ifndef UI_IMAGE_IMAGE_WEBP_H_
#define UI_IMAGE_IMAGE_WEBP_H_

#include "duilib/Image/ImageDecoder.h"

namespace ui
{
/** WEBP格式的图片数据
*/
class Image_WEBP: public IAnimationImage
{
public:
    Image_WEBP();
    virtual ~Image_WEBP() override;

public:
    /** 加载图像数据
    * @param [in] fileData 图片文件数据(如果不为空，则优先使用文件数据)
    * @param [in] imageFilePath 图片文件路径
    * @param [in] bLoadAllFrames 是否加载全部帧，如果为false只加载第1帧，如果为true则加载全部帧
    * @param [in] bAsyncDecode 是否支持异步线程解码图片数据
    * @param [in] fImageSizeScale 图片缩放百分比
    * @param [in] rcMaxDestRectSize 目标区域大小，用于优化加载性能
    */
    bool LoadImageFile(std::vector<uint8_t>& fileData,
                       const FilePath& imageFilePath,
                       bool bLoadAllFrames,
                       bool bAsyncDecode,
                       float fImageSizeScale,
                       const UiSize& rcMaxDestRectSize,
                       bool bAssertEnabled);

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
    * @param [in] szDestRectSize 目标区域的大小，用于矢量图的缩放
    * @param [out] pAnimationFrame 返回该帧的图片位图数据
    * @return 成功返回true，失败则返回false
    */
    virtual bool ReadFrameData(int32_t nFrameIndex, const UiSize& szDestRectSize, AnimationFrame* pAnimationFrame) override;

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

    /** 设置异步解码的状态（UI线程提交解码任务前置位，解码任务结束时清除）
    */
    virtual void SetAsyncDecoding(bool bDecoding) override;

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
    /** 异步解码：加载阶段只读取文件头部的数据，解析静态图片的基本信息
    *   该方法只支持静态图片（非动画），动画图片仍需要读取完整的文件数据
    * @param [in] imageFilePath 图片文件路径
    * @param [in] fImageSizeScale 图片缩放百分比
    * @param [in] rcMaxDestRectSize 目标区域大小，用于优化加载性能
    * @param [in] bAssertEnabled 当遇到图片数据错误时，是否允许断言
    * @return 返回true表示解析成功（完整的文件数据延迟到解码线程读取），返回false表示需要回退到读取完整文件
    */
    bool LoadImageInfoFromFileHeader(const FilePath& imageFilePath,
                                     float fImageSizeScale,
                                     const UiSize& rcMaxDestRectSize,
                                     bool bAssertEnabled);

    /** 准备解码所需的数据：完整的文件数据与解码句柄（可以在多线程中调用）
    *   加载阶段对于大文件只记录了文件路径，文件数据与解码句柄在此处补齐
    * @return 返回true表示数据准备成功
    */
    bool PrepareDecodeData();

private:
    /** 私有实现数据
    */
    struct TImpl;
    std::unique_ptr<TImpl> m_impl;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_WEBP_H_
