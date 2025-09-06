#ifndef UI_IMAGE_IMAGE_DECODER_H_
#define UI_IMAGE_IMAGE_DECODER_H_

#include "duilib/duilib_defs.h"
#include <memory>
#include <vector>
#include <string>

namespace ui 
{
/** 图片格式解码类
*/
class UILIB_API ImageDecoder
{
public:
    /** 加载后的图片数据
    */
    struct ImageData
    {
        /** 位图数据，每帧图片的数据长度固定为：图像数据长度为 (m_imageHeight*m_imageWidth*4)
        *   每个像素数据固定占4个字节，格式为ARGB格式，位数顺序从高位到低位分别为[第3位:A，第2位:R，第1位:G,第0位:B]
        */
        std::vector<uint8_t> m_bitmapData;

        /** 图像宽度
        */
        uint32_t m_imageWidth = 0;

        /** 图像高度
        */
        uint32_t m_imageHeight = 0;

        /** 动画播放时的延迟时间，单位为毫秒
        */
        uint32_t m_frameInterval = 0;
    };
};

/** 使用cximage加载图片（只支持ICO格式）
* @param [in] fileData ICO文件数据
* @param [in] bLoadAllFrames 对于多帧图片，是否加载全部帧（true加载全部帧，false仅加载第1帧）
* @param [in] iconSize 仅当bLoadAllFrames为false时有效，需要加载ICO图标的大小
*                      因ICO文件中包含了各种大小的图标，加载的时候，只加载其中一个图标
* @param [out] imageData 返回加载的位图数据
*/
namespace CxImageLoader
{
    bool LoadImageFromMemory(const std::vector<uint8_t>& fileData,
                             bool bLoadAllFrames,
                             uint32_t iconSize /*仅当bLoadAllFrames为false时有效*/,
                             std::vector<ImageDecoder::ImageData>& imageData);
}

} // namespace ui

#endif // UI_IMAGE_IMAGE_DECODER_H_
