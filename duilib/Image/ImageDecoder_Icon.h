#ifndef UI_IMAGE_IMAGE_DECODER_ICON_H_
#define UI_IMAGE_IMAGE_DECODER_ICON_H_

#include "duilib/Image/ImageDecoder.h"

namespace ui
{
/** IconManager管理的图片解码器接口
*/
class ImageDecoder_Icon: public IImageDecoder
{
public:
    ImageDecoder_Icon();
    virtual ~ImageDecoder_Icon() override;
        
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
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_DECODER_ICON_H_
