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
    // GBK编码：Windows平台按代码页936处理；其他平台暂不支持转换（转换接口返回false）
    GBK,
    // UTF-8编码，BOM头为EF BB BF
    UTF8,
    // UTF-16小端编码，BOM头为FF FE
    UTF16_LE,
    // UTF-16大端编码，BOM头为FE FF
    UTF16_BE,
    // UTF-32小端编码，BOM头为FF FE 00 00
    UTF32_LE,
    // UTF-32大端编码，BOM头为00 00 FE FF
    UTF32_BE
};

/** 编码检测策略
 */
enum class DUILIB_API CharsetDetectPolicy
{
    // 默认策略：优先BOM头；无BOM头时按文本特征与内容校验检测，包含 UTF-16/UTF-32
    kDefault,
    // 严格策略：无BOM头时只识别 ASCII/GBK/UTF-8，不尝试 UTF-16/UTF-32，避免二进制数据被误判
    kStrict
};

/** 字符串字符集辅助类（全部为静态函数），功能要点如下：
 *  1. 检测字节流的编码类型（BOM头优先，其次按数据内容检测）；
 *  2. 校验字节流是否符合指定的编码；
 *  3. 将字节流按检测到的或指定的编码转换为Unicode字符串；
 *  4. 将Unicode字符串按指定编码转换为字节流（可选择写入BOM头）。
 *  注意：所有涉及数据的接口均要求data/stream不能为nullptr，且length不能为0，否则按失败处理。
 */
class DUILIB_API StringCharset
{
public:
    /** 获取指定编码的BOM头长度
    * @param [in] charsetType 编码类型
    * @return 返回BOM头长度（单位：字节）：UTF8为3，UTF16_LE/UTF16_BE为2，UTF32_LE/UTF32_BE为4，其他类型为0
    */
    static uint32_t GetBOMSize(CharsetType charsetType);

    /** 判断数据是否以BOM头开始
    * @param [in] data 数据起始地址，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @return 数据以可识别的BOM头开始时返回true，否则返回false
    */
    static bool HasBOM(const char* data, uint32_t length);

    /** 获取数据中BOM头的长度
    * @param [in] data 数据起始地址，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @return 返回BOM头的长度（单位：字节）；无BOM头或无法识别时返回0
    */
    static uint32_t SkipBOM(const char* data, uint32_t length);

    /** 检测数据的字符集类型，仅根据数据的BOM头进行检测
    * @param [in] data 数据起始地址，不可为nullptr
    * @param [in] length 数据长度（单位：字节，至少需要2字节）
    * @return 返回字符集类型；没有BOM头、长度不足或检测失败时返回CharsetType::UNKNOWN
    */
    static CharsetType GetDataCharsetByBOM(const char* data, uint32_t length);

    /** 检测数据的字符集类型，根据数据内容进行检测，不检测BOM头
    * @param [in] data 数据起始地址，不可为nullptr
    * @param [in] length 数据长度（单位：字节，至少需要1字节）
    * @param [in] policy 检测策略：kDefault 会尝试 UTF-16/UTF-32 检测；kStrict 只识别 ASCII/GBK/UTF-8
    * @return 返回字符集类型；纯ASCII数据返回CharsetType::ANSI，检测失败返回CharsetType::UNKNOWN
    * 说明：GBK 与 UTF-8、UTF-16 与纯ASCII文本存在固有歧义，检测结果为首选推测；
    *        调用方明确知道编码时，应通过 inCharsetType 参数指定。
    */
    static CharsetType GetDataCharset(const char* data, uint32_t length,
                                      CharsetDetectPolicy policy = CharsetDetectPolicy::kDefault);

    /** 将流数据转换为字符串，优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    * @param [in] data 数据起始地址，数据为未知编码数据，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @param [out] result 返回Unicode字符串值（转换前会先清空）
    * @return 转换成功返回true；data为nullptr、length为0、编码无法识别或解码失败时返回false
    */
    static bool GetDataAsString(const char* data, uint32_t length, std::wstring& result);

    /** 将流数据转换为字符串，优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    * @param [in] data 数据起始地址，数据为未知编码数据，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @param [in] inCharsetType 读取数据所采用的编码；为UNKNOWN时自动检测编码类型
    * @param [out] result 返回Unicode字符串值（转换前会先清空）
    * @return 转换成功返回true；data为nullptr、length为0、编码无法识别、解码失败，
    *         或指定的inCharsetType与BOM头不一致、数据不符合该编码规则时返回false
    */
    static bool GetDataAsString(const char* data, uint32_t length, CharsetType inCharsetType, std::wstring& result);

    /** 将流数据转换为字符串，优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    * @param [in] data 数据起始地址，数据为未知编码数据，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @param [out] result 返回Unicode字符串值（转换前会先清空）
    * @param [out] outCharsetType 返回实际采用的编码类型
    * @param [out] bomSize 返回检测到的BOM头大小（单位：字节，无BOM头时为0）
    * @return 转换成功返回true；data为nullptr、length为0、编码无法识别或解码失败时返回false
    */
    static bool GetDataAsString(const char* data, uint32_t length,
                                std::wstring& result, CharsetType& outCharsetType, uint32_t& bomSize);

    /** 将流数据转换为字符串，优先根据BOM头进行检测类型，如果检测失败则按数据流检测编码类型
    * @param [in] data 数据起始地址，数据为未知编码数据，不可为nullptr
    * @param [in] length 数据长度（单位：字节）
    * @param [in] inCharsetType 读取数据所采用的编码；为UNKNOWN时自动检测编码类型
    * @param [out] result 返回Unicode字符串值（转换前会先清空）
    * @param [out] outCharsetType 返回实际采用的编码类型
    * @param [out] bomSize 返回检测到的BOM头大小（单位：字节，无BOM头时为0）
    * @return 转换成功返回true；data为nullptr、length为0、编码无法识别、解码失败，
    *         或指定的inCharsetType与BOM头不一致、数据不符合该编码规则时返回false
    */
    static bool GetDataAsString(const char* data, uint32_t length, CharsetType inCharsetType,
                                std::wstring& result, CharsetType& outCharsetType, uint32_t& bomSize);

    /** 将Unicode字符串转换为指定编码的字节流
    * @param [in] str 源Unicode字符串
    * @param [in] charsetType 目标编码类型，不可为UNKNOWN
    * @param [out] data 返回转换后的字节流（转换前会先清空）
    * @param [in] bWithBOM 是否在字节流前写入对应编码的BOM头（ANSI/GBK无BOM头）
    * @return 转换成功返回true；charsetType为UNKNOWN、编码不支持（如非Windows平台的GBK）或编码失败时返回false
    */
    static bool WStringToData(const std::wstring& str, CharsetType charsetType,
                              std::string& data, bool bWithBOM = false);

    /** 获取编码类型的名称
    * @param [in] charsetType 编码类型
    * @return 返回编码名称（如 "UTF-8"、"UTF-16LE"），未知类型返回 "UNKNOWN"
    */
    static const char* GetCharsetName(CharsetType charsetType);

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
    * @return 符合UTF-8编码规则时返回true；stream为nullptr、length为0或数据非法
    *         （含过长编码、代理区编码、超出U+10FFFF的码点）时返回false
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

    /** 检测给定的字节流是否为UTF32-LE编码
    * @param [in] stream 指向字节流的指针，不可为nullptr
    * @param [in] length 流的长度（单位：字节，必须为4的倍数）
    * @return 符合UTF32-LE编码规则时返回true；stream为nullptr、长度不足4字节、长度不是4的倍数
    *         或包含非法码点（代理区、超出U+10FFFF）时返回false
    */
    static bool IsValidateUTF32LEStream(const char* stream, uint32_t length);

    /** 检测给定的字节流是否为UTF32-BE编码
    * @param [in] stream 指向字节流的指针，不可为nullptr
    * @param [in] length 流的长度（单位：字节，必须为4的倍数）
    * @return 符合UTF32-BE编码规则时返回true；stream为nullptr、长度不足4字节、长度不是4的倍数
    *         或包含非法码点（代理区、超出U+10FFFF）时返回false
    */
    static bool IsValidateUTF32BEStream(const char* stream, uint32_t length);
};

}//namespace ui

#endif //UI_UTILS_STRING_CHARSET_H_
