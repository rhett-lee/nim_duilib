#ifndef UI_IMAGE_IMAGE_DECODER_H_
#define UI_IMAGE_IMAGE_DECODER_H_

#pragma once

#include "duilib/duilib_defs.h"
#include <memory>
#include <vector>
#include <string>

namespace ui 
{
class ImageInfo;

/** 图片格式解码类
*/
class UILIB_API ImageDecoder
{
public:    
    /** 从文件中加载图片并解码图片数据
    * @param [in] imageFullPath 文件的完整路径
    */
    std::unique_ptr<ImageInfo> LoadImageFile(const std::wstring& imageFullPath);

    /** 从内存文件数据中加载图片并解码图片数据
    * @param [in] file_data 图片文件的数据，内部有增加尾0的写操作
    * @param [in] imageFullPath 文件的完整路径
    */
	std::unique_ptr<ImageInfo> LoadImageData(std::vector<unsigned char>& file_data, const std::wstring& imageFullPath); 
};

} // namespace ui

#endif // UI_IMAGE_IMAGE_DECODER_H_
