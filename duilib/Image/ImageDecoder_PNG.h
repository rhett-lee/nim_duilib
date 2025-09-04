#ifndef UI_IMAGE_IMAGE_DECODER_PNG_H_
#define UI_IMAGE_IMAGE_DECODER_PNG_H_

#include "duilib/Image/IImageDecoder.h"

namespace ui
{
/** PNG/APNG格式的图片解码器接口
*/
class ImageDecoder_PNG: public IImageDecoder
{
public:
    ImageDecoder_PNG();
    virtual ~ImageDecoder_PNG() override;
        
    /** 获取该解码器支持的格式名称
    */
    virtual DString GetFormatName() const override;

    /** 检查该解码器是否支持给定的文件名
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @param [out] bVirtualFile 返回true代表虚拟文件名，返回false代表实体文件名
    */
    virtual bool CanDecode(const DString& imageFileString, bool& bVirtualFile) const override;
         
    /** 检查该解码器是否支持给定的数据流
    * @param [in] data 数据的起始地址
    * @param [in] dataLen 数据的长度
    */
    virtual bool CanDecode(const uint8_t* data, size_t dataLen) const override;
   
    /** 加载解码图片数据，返回解码后的图像数据
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @param [in] data 待解码的数据, 容器的数据会被交换到内部实现（当imageFileString为虚拟文件名时，可为空）
    * @param [in] fImageSizeScale 解码图片大小的缩放比(解码后对图片执行resize操作，按比例调整图片的宽和高)
    * @param [in] pExtraParam 图片解码的额外参数
    */
    virtual std::unique_ptr<IImage> LoadImageData(const DString& imageFileString,
                                                  std::vector<uint8_t>& data,
                                                  float fImageSizeScale = 1.0f,
                                                  const IImageDecoder::ExtraParam* pExtraParam = nullptr) override;
};

} //namespace ui

#endif //UI_IMAGE_IMAGE_DECODER_PNG_H_
