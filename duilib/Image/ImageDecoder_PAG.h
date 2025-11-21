#ifndef UI_IMAGE_IMAGE_DECODER_PAG_H_
#define UI_IMAGE_IMAGE_DECODER_PAG_H_

#include "duilib/Image/ImageDecoder.h"

#ifdef DUILIB_IMAGE_SUPPORT_LIB_PAG

namespace ui
{
/** PAG格式的图片解码器接口
*/
class ImageDecoder_PAG: public IImageDecoder
{
public:
    ImageDecoder_PAG();
    virtual ~ImageDecoder_PAG() override;
        
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

#endif //DUILIB_IMAGE_SUPPORT_LIB_PAG

#endif //UI_IMAGE_IMAGE_DECODER_PAG_H_
