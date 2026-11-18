#ifndef UI_IMAGE_IMAGE_DECODER_ICO_H_
#define UI_IMAGE_IMAGE_DECODER_ICO_H_

#include "duilib/Image/ImageDecoder.h"

namespace ui
{
/** ICO/CUR格式的图片解码器接口
*/
class ImageDecoder_ICO: public IImageDecoder
{
public:
    ImageDecoder_ICO();
    virtual ~ImageDecoder_ICO() override;
        
    /** 获取该解码器支持的格式名称
    */
    virtual DString GetFormatName() const override;

    /** 检查该解码器是否支持给定的文件名
    * @param [in] imageFilePath 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    */
    virtual bool CanDecode(const DString& imageFilePath) const override;
         
    /** 检查该解码器是否支持给定的数据流
    * @param [in] data 数据的起始地址
    * @param [in] dataLen 数据的长度
    */
    virtual bool CanDecode(const uint8_t* data, size_t dataLen) const override;
   
    /** 加载解码图片数据，返回解码后的图像数据
    @param [in] decodeParam 图片解码的相关参数
    */
    virtual std::unique_ptr<IImage> LoadImageData(const ImageDecodeParam& decodeParam) override;

private:
    /** 异步解码：加载阶段只解析ICO文件的目录信息，像素解码放到解码线程中执行
    @param [in] decodeParam 图片解码的相关参数
    @param [in] nIconSizeScaled 期望加载的图标大小（已经按缩放比例调整过）
    @return 返回nullptr表示解析失败，需要回退到同步解码
    */
    std::unique_ptr<IImage> LoadImageDataAsync(const ImageDecodeParam& decodeParam, uint32_t nIconSizeScaled);
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_DECODER_ICO_H_
