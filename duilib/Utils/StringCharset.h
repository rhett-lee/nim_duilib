#ifndef UI_UTILS_STRING_CHARSET_H_
#define UI_UTILS_STRING_CHARSET_H_

#include "duilib/duilib_defs.h"
#include <string>

namespace ui
{
/** 支持的字符集定义
*/
enum class UILIB_API CharsetType
{
    UNKNOWN,
    ANSI,
    UTF8,
    UTF16_LE,
    UTF16_BE
};

/** 字符串字符集辅助类
*/
class UILIB_API StringCharset
{   
public:
    /** 获取指定编码的BOM头长度
    @param [in] charsetType 编码类型
    */
    static uint32_t GetBOMSize(CharsetType charsetType);
    
    /** 检测数据的字符集类型, 仅根据数据的BOM头进行检测
    @param [in] data 数据起始地址
    @param [in] length 数据长度
    @return 返回字符集类型，如果检测失败则返回未知类型
    */
    static CharsetType GetDataCharsetByBOM(const char* data, uint32_t length);

    /** 检测数据的字符集类型, 仅根据数据的类型进行检测，不检测BOM头数据
    @param [in] data 数据起始地址
    @param [in] length 数据长度
    @return 返回字符集类型，如果检测失败则返回未知类型
    */
    static CharsetType GetDataCharset(const char* data, uint32_t length);

    /** 将流数据转换为字符串, 优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    @param [in] data 数据起始地址, 数据为未知编码数据
    @param [in] length 数据长度
    @param [out] result 返回Unicode字符串值
    */
    static bool GetDataAsString(const char* data, uint32_t length, std::wstring& result);

    /** 将流数据转换为字符串, 优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    @param [in] data 数据起始地址, 数据为未知编码数据
    @param [in] length 数据长度
    @param [in] inCharsetType 读取文件所采用的编码，如果为UNKNOWN则自动检测文本编码类型
    @param [out] result 返回Unicode字符串值
    */
    static bool GetDataAsString(const char* data, uint32_t length, CharsetType inCharsetType, std::wstring& result);

    /** 将流数据转换为字符串, 优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    @param [in] data 数据起始地址, 数据为未知编码数据
    @param [in] length 数据长度
    @param [out] result 返回Unicode字符串值
    @param [out] outCharsetType 读取文件所采用的编码类型
    @param [out] bomSize 读取文件时，检测到的Unicode BOM头大小
    */
    static bool GetDataAsString(const char* data, uint32_t length, 
                                std::wstring& result, CharsetType& outCharsetType, uint32_t& bomSize);

     /** 将流数据转换为字符串, 优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    @param [in] data 数据起始地址, 数据为未知编码数据
    @param [in] length 数据长度
    @param [in] inCharsetType 读取文件所采用的编码，如果为UNKNOWN则自动检测文本编码类型
    @param [out] result 返回Unicode字符串值
    @param [out] outCharsetType 读取文件所采用的编码类型
    @param [out] bomSize 读取文件时，检测到的Unicode BOM头大小
    */
    static bool GetDataAsString(const char* data, uint32_t length, CharsetType inCharsetType,
                                std::wstring& result, CharsetType& outCharsetType, uint32_t& bomSize);

public:
    /** 用于验证给定的 char 流是否为 ASCII 编码
    @param [in] stream：指向 char 流的指针
    #param [in] length：流的长度，以字节为单位
    */
    static bool IsValidateASCIIStream(const char* stream, uint32_t length);

    /** 用于验证给定的 char 流是否为 GBK 编码
    @param [in] stream：指向 char 流的指针
    #param [in] length：流的长度，以字节为单位
    */
    static bool IsValidateGBKStream(const char* stream, uint32_t length);

    /** 用于验证给定的 char 流是否为 UTF-8 编码
    @param [in] stream：指向 char 流的指针
    #param [in] length：流的长度，以字节为单位
    */
    static bool IsValidateUTF8Stream(const char* stream, uint32_t length);

    /** 用于验证给定的 char 流是否为 UTF16-LE 编码
    @param [in] stream：指向 char 流的指针
    #param [in] length：流的长度，以字节为单位
    */
    static bool IsValidateUTF16LEStream(const char* stream, uint32_t length);

    /** 用于验证给定的 char 流是否为 UTF16-BE 编码
    @param [in] stream：指向 char 流的指针
    #param [in] length：流的长度，以字节为单位
    */
    static bool IsValidateUTF16BEStream(const char* stream, uint32_t length);
};

}//namespace ui

#endif //UI_UTILS_STRING_CHARSET_H_
