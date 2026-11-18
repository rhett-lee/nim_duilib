#ifndef UI_IMAGE_IMAGE_DECODER_UTIL_H_
#define UI_IMAGE_IMAGE_DECODER_UTIL_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Utils/FilePath.h"

namespace ui 
{
class DUILIB_API ImageDecoderUtil
{
public:
    /** ICO文件中一个图标项的简要信息（只解析文件头得到，不涉及像素解码）
    */
    struct IcoImageInfo
    {
        /** 图标宽度 */
        uint32_t m_nWidth = 0;

        /** 图标高度 */
        uint32_t m_nHeight = 0;

        /** 颜色数（0表示32位真彩色） */
        uint32_t m_nColorCount = 0;
    };

public:
    /** 查询支持哪些类型的图片（按后缀名,后缀名不包含'.'字符，比如"BMP;JPG"等）
    */
    static DString GetSupportedFileExtentions();

    /** 检查该解码器是否支持给定的数据流
    * @param [in] data 数据的起始地址
    * @param [in] dataLen 数据的长度
    */
    static bool CanDecode(const uint8_t* data, size_t dataLen);

    /** 获取图片的宽度和高度（只解析文件头，不执行像素解码，用于延迟解码场景）
    * @param [in] filePath 文件路径
    * @param [out] nWidth 返回图片宽度
    * @param [out] nHeight 返回图片高度
    * @return 返回true表示成功，否则返回false
    */
    static bool GetImageSizeFromFile(const FilePath& filePath, uint32_t& nWidth, uint32_t& nHeight);

    /** 获取图片的宽度和高度（只解析文件头，不执行像素解码，用于延迟解码场景）
    * @param [in] fileData 文件数据
    * @param [out] nWidth 返回图片宽度
    * @param [out] nHeight 返回图片高度
    * @return 返回true表示成功，否则返回false
    */
    static bool GetImageSizeFromMemory(const std::vector<uint8_t>& fileData, uint32_t& nWidth, uint32_t& nHeight);

    /** 解析ICO文件的目录信息（只解析文件头，不执行像素解码，用于延迟解码场景）
    * @param [in] fileData ICO文件数据
    * @param [out] iconInfoList 返回每个图标项的信息（按文件中的顺序）
    * @return 返回true表示成功，否则返回false
    */
    static bool GetIcoImageInfo(const std::vector<uint8_t>& fileData, std::vector<IcoImageInfo>& iconInfoList);

    /** 从ICO文件的目录信息中，选择一个需要加载的图标项（选择规则与解码后的选择规则一致）
    * @param [in] iconInfoList 图标项的信息列表
    * @param [in] iconSize 期望加载的图标大小
    * @return 返回选中图标项在列表中的索引号；返回-1表示选择失败
    */
    static int32_t SelectIcoImageInfo(const std::vector<IcoImageInfo>& iconInfoList, uint32_t iconSize);

    /** 加载图片(通用，支持多种文件格式, 但不支持加载多帧)
    * @param [in] filePath 文件路径
    * @param [out] imageData 返回加载的位图数据
    */
    static bool LoadImageFromFile(const FilePath& filePath,
                                  UiImageData& imageData);

    /** 加载图片(通用，支持多种文件格式, 但不支持加载多帧)
    * @param [in] fileData 文件数据
    * @param [out] imageData 返回加载的位图数据
    */
    static bool LoadImageFromMemory(const std::vector<uint8_t>& fileData,
                                    UiImageData& imageData);

public:
    /** 加载ICO图片（该函数只支持ICO格式）
    * @param [in] filePath ICO文件路径
    * @param [in] bLoadAllFrames 对于多帧图片，是否加载全部帧（true加载全部帧，false仅加载第1帧）
    * @param [in] iconSize 仅当bLoadAllFrames为false时有效，需要加载ICO图标的大小
    *                      因ICO文件中包含了各种大小的图标，加载的时候，只加载其中一个图标
    * @param [out] imageData 返回加载的位图数据
    */
    static bool LoadIcoFromFile(const FilePath& filePath,
                                bool bLoadAllFrames,
                                uint32_t iconSize /*仅当bLoadAllFrames为false时有效*/,
                                std::vector<UiImageData>& imageData);

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
