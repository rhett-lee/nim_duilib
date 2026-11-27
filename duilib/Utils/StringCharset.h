#ifndef UI_UTILS_STRING_CHARSET_H_
#define UI_UTILS_STRING_CHARSET_H_

#include "duilib/duilib_defs.h"
#include <string>

namespace ui
{
/** 支持的字符集定义
 */
enum class DUILIB_API CharsetType
{
    // 未知编码（检测失败或调用方未指定）
    UNKNOWN,
    // 本机多字节编码(ANSI)：Windows平台按当前代码页(MBCS)处理，其他平台按UTF8处理；纯ASCII数据也归为该类型
    ANSI,
    // UTF-8编码，BOM头为EF BB BF
    UTF8,
    // UTF-16小端编码，BOM头为FF FE
    UTF16_LE,
    // UTF-16大端编码，BOM头为FE FF
    UTF16_BE
};

/** 字符串字符集辅助类（全部为静态函数）
 *  提供以下功能：
 *  1. 根据BOM头或数据内容检测字节流的编码类型（ASCII/GBK/UTF-8/UTF-16LE/UTF-16BE）；
 *  2. 校验字节流是否符合指定的编码；
 *  3. 将字节流按检测到的或指定的编码转换为Unicode字符串。
 *  注意：所有涉及数据的接口均要求data/stream不能为nullptr，且length不能为0，否则按失败处理。
 */
class DUILIB_API StringCharset
{   
public:
    /** 获取指定编码的BOM头长度
    * @param [in] charsetType 编码类型
    * @return 返回BOM头长度（单位：字节）：UTF8为3，UTF16_LE/UTF16_BE为2，其他类型为0
    */
    static uint32_t GetBOMSize(CharsetType charsetType);
    
    /** 检测数据的字符集类型, 仅根据数据的BOM头进行检测
    * @param [in] data 数据起始地址，不可为nullptr
    * @param [in] length 数据长度（单位：字节，至少需要2字节）
    * @return 返回字符集类型；没有BOM头、长度不足2字节或检测失败时返回CharsetType::UNKNOWN
    */
    static CharsetType GetDataCharsetByBOM(const char* data, uint32_t length);

    /** 检测数据的字符集类型, 仅根据数据的类型进行检测，不检测BOM头数据
    * @param [in] data 数据起始地址，不可为nullptr
    * @param [in] length 数据长度（单位：字节，至少需要1字节）
    * @return 返回字符集类型；纯ASCII数据返回CharsetType::ANSI，检测失败返回CharsetType::UNKNOWN
    */
    static CharsetType GetDataCharset(const char* data, uint32_t length);

    /** 将流数据转换为字符串, 优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    * @param [in] data 数据起始地址, 数据为未知编码数据，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @param [out] result 返回Unicode字符串值（转换前会先清空）
    * @return 转换成功返回true；data为nullptr、length为0或编码无法识别时返回false
    */
    static bool GetDataAsString(const char* data, uint32_t length, std::wstring& result);

    /** 将流数据转换为字符串, 优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    * @param [in] data 数据起始地址, 数据为未知编码数据，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @param [in] inCharsetType 读取数据所采用的编码，如果为UNKNOWN则自动检测编码类型
    * @param [out] result 返回Unicode字符串值（转换前会先清空）
    * @return 转换成功返回true；data为nullptr、length为0、编码无法识别，
    *         或指定的inCharsetType与BOM头/数据检测结果不一致时返回false
    */
    static bool GetDataAsString(const char* data, uint32_t length, CharsetType inCharsetType, std::wstring& result);

    /** 将流数据转换为字符串, 优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    * @param [in] data 数据起始地址, 数据为未知编码数据，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @param [out] result 返回Unicode字符串值（转换前会先清空）
    * @param [out] outCharsetType 返回实际采用的编码类型
    * @param [out] bomSize 返回检测到的Unicode BOM头大小（单位：字节，仅BOM检测成功时非0）
    * @return 转换成功返回true；data为nullptr、length为0或编码无法识别时返回false
    */
    static bool GetDataAsString(const char* data, uint32_t length, 
                                std::wstring& result, CharsetType& outCharsetType, uint32_t& bomSize);

    /** 将流数据转换为字符串, 优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    * @param [in] data 数据起始地址, 数据为未知编码数据，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @param [in] inCharsetType 读取数据所采用的编码，如果为UNKNOWN则自动检测编码类型
    * @param [out] result 返回Unicode字符串值（转换前会先清空）
    * @param [out] outCharsetType 返回实际采用的编码类型
    * @param [out] bomSize 返回检测到的Unicode BOM头大小（单位：字节，仅BOM检测成功时非0）
    * @return 转换成功返回true；data为nullptr、length为0、编码无法识别，
    *         或指定的inCharsetType与BOM头/数据检测结果不一致时返回false
    */
    static bool GetDataAsString(const char* data, uint32_t length, CharsetType inCharsetType,
                                std::wstring& result, CharsetType& outCharsetType, uint32_t& bomSize);

public:
    /** 检测给定的字节流是否为ASCII编码
    * @param [in] stream 指向字节流的指针，不可为nullptr
    * @param [in] length 流的长度（单位：字节）
    * @return 全部字节均为ASCII字符时返回true；stream为nullptr、length为0或含非ASCII字节时返回false
    */
    static bool IsValidateASCIIStream(const char* stream, uint32_t length);

    /** 检测给定的字节流是否为GBK编码（纯ASCII数据也视为有效的GBK编码）
    * @param [in] stream 指向字节流的指针，不可为nullptr
    * @param [in] length 流的长度（单位：字节）
    * @return 符合GBK编码规则时返回true；stream为nullptr、length为0或数据非法时返回false
    */
    static bool IsValidateGBKStream(const char* stream, uint32_t length);

    /** 检测给定的字节流是否为UTF-8编码（纯ASCII数据也视为有效的UTF-8编码）
    * @param [in] stream 指向字节流的指针，不可为nullptr
    * @param [in] length 流的长度（单位：字节）
    * @return 符合UTF-8编码规则时返回true；stream为nullptr、length为0或数据非法时返回false
    */
    static bool IsValidateUTF8Stream(const char* stream, uint32_t length);

    /** 检测给定的字节流是否为UTF16-LE编码
    * @param [in] stream 指向字节流的指针，不可为nullptr
    * @param [in] length 流的长度（单位：字节，必须为2的倍数）
    * @return 符合UTF16-LE编码规则时返回true；stream为nullptr、length为0、长度不是2的倍数或代理对不完整时返回false
    */
    static bool IsValidateUTF16LEStream(const char* stream, uint32_t length);

    /** 检测给定的字节流是否为UTF16-BE编码
    * @param [in] stream 指向字节流的指针，不可为nullptr
    * @param [in] length 流的长度（单位：字节，必须为2的倍数）
    * @return 符合UTF16-BE编码规则时返回true；stream为nullptr、length为0、长度不是2的倍数或代理对不完整时返回false
    */
    static bool IsValidateUTF16BEStream(const char* stream, uint32_t length);
};

}//namespace ui

#endif //UI_UTILS_STRING_CHARSET_H_
