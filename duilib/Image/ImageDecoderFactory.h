#ifndef UI_IMAGE_IMAGE_DECODER_FACTORY_H_
#define UI_IMAGE_IMAGE_DECODER_FACTORY_H_

#include "duilib/Image/ImageDecoder.h"
#include "duilib/Image/ImageUtil.h"

namespace ui 
{
/** 图片格式解码工程类
*/
class UILIB_API ImageDecoderFactory
{
public:
    ImageDecoderFactory();
    ~ImageDecoderFactory();

    /** 添加一个图片格式解码器
    * @param [in] pImageDecoder 需要添加的图片解码器
    */
    bool AddImageDecoder(const std::shared_ptr<IImageDecoder>& pImageDecoder);

    /** 移除一个图片格式解码器
    * @param [in] pImageDecoder 需要添加的图片解码器
    */
    bool RemoveImageDecoder(const std::shared_ptr<IImageDecoder>& pImageDecoder);

    /** 清除所有的图片格式解码器
    */
    void Clear();

public:
    /** 加载解码图片数据，返回解码后的图像数据
    * @param [in] decodeParam 图片解码的相关参数
    */
    std::unique_ptr<IImage> LoadImageData(const ImageDecodeParam& decodeParam);

    /** 解码一个文件数据为位图(不支持多帧图片，如果图片为多帧，则只解码第一帧)
    * @param [in] decodeParam 图片解码的相关参数
    */
    std::shared_ptr<IBitmap> DecodeImageData(const ImageDecodeParam& decodeParam);

private:
    /** 图片解码器
    */
    std::vector<std::shared_ptr<IImageDecoder>> m_imageDecoders;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_DECODER_FACTORY_H_
