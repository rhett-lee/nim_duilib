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
    /** 判断一个文件名是否为虚拟文件名
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @return 返回true表示imageFileString为虚拟文件名，返回false表示imageFileString为实体文件名
    */
    bool IsVirtualImageFile(const DString& imageFileString) const;

    /** 加载解码图片数据，返回解码后的图像数据
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @param [in] data 数据的起始地址（当imageFileString为虚拟文件名时，可为nullptr）
    * @param [in] dataLen 数据的长度（当imageFileString为虚拟文件名时，可为0）
    * @param [in] fImageSizeScale 解码图片大小的缩放比(解码后对图片执行resize操作，按比例调整图片的宽和高)
    * @param [in] pExtraParam 图片解码的额外参数
    */
    std::unique_ptr<IImage> LoadImageData(const DString& imageFileString,
                                          const uint8_t* data, size_t dataLen,
                                          float fImageSizeScale = 1.0f,
                                          const IImageDecoder::ExtraParam* pExtraParam = nullptr);

    /** 加载解码图片数据，返回解码后的图像数据
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @param [in] data 待解码的数据, 容器的数据会被交换到内部实现（当imageFileString为虚拟文件名时，可为空）
    * @param [in] fImageSizeScale 解码图片大小的缩放比(解码后对图片执行resize操作，按比例调整图片的宽和高)
    * @param [in] pExtraParam 图片解码的额外参数
    */
    std::unique_ptr<IImage> LoadImageData(const DString& imageFileString,
                                          std::vector<uint8_t>& data,
                                          float fImageSizeScale = 1.0f,
                                          const IImageDecoder::ExtraParam* pExtraParam = nullptr);

    /** 解码一个文件数据为位图(不支持多帧图片，如果图片为多帧，则只解码第一帧)
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @param [in] data 数据的起始地址（当imageFileString为虚拟文件名时，可为nullptr）
    * @param [in] dataLen 数据的长度（当imageFileString为虚拟文件名时，可为0）
    * @param [in] fImageSizeScale 解码图片大小的缩放比(解码后对图片执行resize操作，按比例调整图片的宽和高)
    * @param [in] pExtraParam 图片解码的额外参数
    */
    std::shared_ptr<IBitmap> DecodeImageData(const DString& imageFileString,
                                             const uint8_t* data, size_t dataLen,
                                             float fImageSizeScale = 1.0f,
                                             const IImageDecoder::ExtraParam* pExtraParam = nullptr);

    /** 解码一个文件数据为位图(不支持多帧图片，如果图片为多帧，则只解码第一帧)
    * @param [in] imageFileString 实体文件名(比如："File.jpg"，可以带路径), 或者虚拟文件名（比如： "icon:1"）
    * @param [in] data 待解码的数据, 容器的数据会被交换到内部实现（当imageFileString为虚拟文件名时，可为空）
    * @param [in] fImageSizeScale 解码图片大小的缩放比(解码后对图片执行resize操作，按比例调整图片的宽和高)
    * @param [in] pExtraParam 图片解码的额外参数
    */
    std::shared_ptr<IBitmap> DecodeImageData(const DString& imageFileString,
                                             std::vector<uint8_t>& data,
                                             float fImageSizeScale = 1.0f,
                                             const IImageDecoder::ExtraParam* pExtraParam = nullptr);

private:
    /** 图片解码器
    */
    std::vector<std::shared_ptr<IImageDecoder>> m_imageDecoders;
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_DECODER_FACTORY_H_
