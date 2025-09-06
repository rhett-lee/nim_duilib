#ifndef UI_IMAGE_IMAGE_DECODER_UTIL_H_
#define UI_IMAGE_IMAGE_DECODER_UTIL_H_

#include "duilib/Core/UiTypes.h"

namespace ui 
{
class ImageDecoderUtil
{
public:
    /** 查询支持哪些类型的图片（按后缀名,后缀名不包含'.'字符，比如"BMP;JPG"等）
    */
    static DString GetSupportedFileExtentions();

    /** 加载图片(通用，支持多种文件格式, 但不支持加载多帧)
    * @param [in] fileData ICO文件数据
    * @param [out] imageData 返回加载的位图数据
    */
    static bool LoadImageFromMemory(const std::vector<uint8_t>& fileData,
                                    UiImageData& imageData);

public:
    /** 加载ICO图片（该函数只支持ICO格式）
    * @param [in] fileData ICO文件数据
    * @param [in] bLoadAllFrames 对于多帧图片，是否加载全部帧（true加载全部帧，false仅加载第1帧）
    * @param [in] iconSize 仅当bLoadAllFrames为false时有效，需要加载ICO图标的大小
    *                      因ICO文件中包含了各种大小的图标，加载的时候，只加载其中一个图标
    * @param [out] imageData 返回加载的位图数据
    */
    static bool LoadIcoFromMemory(const std::vector<uint8_t>& fileData,
                                  bool bLoadAllFrames,
                                  uint32_t iconSize /*仅当bLoadAllFrames为false时有效*/,
                                  std::vector<UiImageData>& imageData);
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_DECODER_UTIL_H_
