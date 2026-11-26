#ifndef UI_UTILS_STRINGUTIL_H_
#define UI_UTILS_STRINGUTIL_H_

#include "duilib/duilib_defs.h"
#include <string>
#include <list>

namespace ui
{
/** 字符串/路径操作的辅助类
*/
class DUILIB_API StringUtil
{
public:
    /** 格式化字符串（类似C语言的 printf）
    * @param [in] format 格式化字符串，其中可包含 %d、%s 等格式说明符
    * @param [in] ... 可变参数，与 format 中的格式说明符一一对应
    * @return 返回格式化后的字符串（std::wstring 版本处理宽字符，std::string 版本处理窄字符）
    */
    static std::wstring Printf(const wchar_t *format, ...);
    static std::string Printf(const char* format, ...);

    /** 将字符串 output 中出现的 find 全部替换为 replace
    * @param [in] find 要被替换的字符串，为空时不做任何替换
    * @param [in] replace 替换后的新字符串
    * @param [in,out] output 输入为原始字符串，输出为替换后的字符串
    * @return 返回实际替换的次数（std::wstring 版本处理宽字符，std::string 版本处理窄字符）
    */
    static size_t ReplaceAll(const std::wstring& find, const std::wstring& replace, std::wstring& output);
    static size_t ReplaceAll(const std::string& find, const std::string& replace, std::string& output);

    /** 将字符串中的英文字母转换为小写（原地修改）
    * @param [in,out] str 要转换的字符串（std::string 版本处理窄字符，std::wstring 版本处理宽字符）
    */
    static void LowerString(std::string& str);
    static void LowerString(std::wstring& str);

    /** 将字符串中的英文字母转换为大写（原地修改）
    * @param [in,out] str 要转换的字符串（std::string 版本处理窄字符，std::wstring 版本处理宽字符）
    */
    static void UpperString(std::string& str);
    static void UpperString(std::wstring& str);

    /** 返回英文字母转换为小写后的字符串（不修改原字符串）
    * @param [in] str 要转换的字符串
    * @return 返回转换后的新字符串（std::wstring 版本处理宽字符，std::string 版本处理窄字符）
    */
    static std::wstring MakeLowerString(const std::wstring& str);
    static std::string MakeLowerString(const std::string& str);

    /** 返回英文字母转换为大写后的字符串（不修改原字符串）
    * @param [in] str 要转换的字符串
    * @return 返回转换后的新字符串（std::wstring 版本处理宽字符，std::string 版本处理窄字符）
    */
    static std::wstring MakeUpperString(const std::wstring& str);
    static std::string MakeUpperString(const std::string& str);

    /** 去掉字符串左侧的空白字符
    * @param [in] input 要处理的字符串
    * @return 返回去掉左侧空白字符后的新字符串（std::string 版本处理窄字符，std::wstring 版本处理宽字符）
    */
    static std::string TrimLeft(const char *input);
    /** 去掉字符串右侧的空白字符
    * @param [in] input 要处理的字符串
    * @return 返回去掉右侧空白字符后的新字符串
    */
    static std::string TrimRight(const char* input);
    /** 去掉字符串左右两侧的空白字符
    * @param [in] input 要处理的字符串
    * @return 返回去掉两侧空白字符后的新字符串
    */
    static std::string Trim(const char* input); /* both left and right */
    /** 去掉字符串左侧的空白字符（原地修改）
    * @param [in,out] input 要处理的字符串
    * @return 返回处理后的字符串引用（即 input 本身）
    */
    static std::string& TrimLeft(std::string& input);
    /** 去掉字符串右侧的空白字符（原地修改）
    * @param [in,out] input 要处理的字符串
    * @return 返回处理后的字符串引用（即 input 本身）
    */
    static std::string& TrimRight(std::string& input);
    /** 去掉字符串左右两侧的空白字符（原地修改）
    * @param [in,out] input 要处理的字符串
    * @return 返回处理后的字符串引用（即 input 本身）
    */
    static std::string& Trim(std::string& input); /* both left and right */
    /** 去掉字符串左侧的空白字符（宽字符版本）
    * @param [in] input 要处理的字符串
    * @return 返回去掉左侧空白字符后的新字符串
    */
    static std::wstring TrimLeft(const wchar_t* input);
    /** 去掉字符串右侧的空白字符（宽字符版本）
    * @param [in] input 要处理的字符串
    * @return 返回去掉右侧空白字符后的新字符串
    */
    static std::wstring TrimRight(const wchar_t* input);
    /** 去掉字符串左右两侧的空白字符（宽字符版本）
    * @param [in] input 要处理的字符串
    * @return 返回去掉两侧空白字符后的新字符串
    */
    static std::wstring Trim(const wchar_t* input); /* both left and right */
    /** 去掉字符串左侧的空白字符（宽字符版本，原地修改）
    * @param [in,out] input 要处理的字符串
    * @return 返回处理后的字符串引用（即 input 本身）
    */
    static std::wstring& TrimLeft(std::wstring& input);
    /** 去掉字符串右侧的空白字符（宽字符版本，原地修改）
    * @param [in,out] input 要处理的字符串
    * @return 返回处理后的字符串引用（即 input 本身）
    */
    static std::wstring& TrimRight(std::wstring& input);
    /** 去掉字符串左右两侧的空白字符（宽字符版本，原地修改）
    * @param [in,out] input 要处理的字符串
    * @return 返回处理后的字符串引用（即 input 本身）
    */
    static std::wstring& Trim(std::wstring& input); /* both left and right */

    /** 按分隔符集合拆分字符串，delimitor 中的任意一个字符都作为分隔符
    * @param [in] input 要拆分的字符串
    * @param [in] delimitor 分隔符集合，其中每个字符都视为一个分隔符
    * @return 返回拆分后的子串列表（std::string 版本处理窄字符，std::wstring 版本处理宽字符）
    */
    static std::list<std::string> Split(const std::string& input, const std::string& delimitor);
    static std::list<std::wstring> Split(const std::wstring& input, const std::wstring& delimitor);

    /** 比较两个字符串是否相等，比较时忽略大小写
    * @param [in] lhs 左值字符串
    * @param [in] rhs 右值字符串
    * @return 返回 true 表示忽略大小写后相等，否则返回 false
    *         （重载支持 std::wstring、std::string 以及对应的字符指针）
    */
    static bool IsEqualNoCase(const std::wstring& lhs, const std::wstring& rhs);
    static bool IsEqualNoCase(const wchar_t* lhs, const std::wstring& rhs);
    static bool IsEqualNoCase(const char* lhs, const std::string& rhs);
    static bool IsEqualNoCase(const std::wstring& lhs, const wchar_t* rhs);
    static bool IsEqualNoCase(const std::string& lhs, const std::string& rhs);
    static bool IsEqualNoCase(const std::string& lhs, const char* rhs);
    static bool IsEqualNoCase(const wchar_t* lhs, const wchar_t* rhs);
    static bool IsEqualNoCase(const char* lhs, const char* rhs);

    /** 比较字符串，区分大小写
    @param [in] lhs 左值字符串
    @param [in] rhs 右值字符串
    @return 返回值定义如下：
            0  : 相等
            1  : lhs > rhs
           -1  : lhs < rhs
    */
    static int32_t StringCompare(const std::wstring& lhs, const std::wstring& rhs);
    static int32_t StringCompare(const wchar_t* lhs, const wchar_t* rhs);
    static int32_t StringCompare(const std::string& lhs, const std::string& rhs);
    static int32_t StringCompare(const char* lhs, const char* rhs);

    /** 比较字符串，不区分大小写
    @param [in] lhs 左值字符串
    @param [in] rhs 右值字符串
    @return 返回值定义如下：
            0  : 相等
            1  : lhs > rhs
           -1  : lhs < rhs
    */
    static int32_t StringICompare(const std::wstring& lhs, const std::wstring& rhs);
    static int32_t StringICompare(const wchar_t* lhs, const wchar_t* rhs);
    static int32_t StringICompare(const std::string& lhs, const std::string& rhs);
    static int32_t StringICompare(const char* lhs, const char* rhs);

    /** 将整型数值转换为字符串
    * @param [in] value 要转换的整型数值
    * @return 返回转换后的字符串，各重载的差异如下：
    *         - UInt64/UInt32 前缀：无符号整数；Int64/Int32 前缀：有符号整数
    *         - W 后缀：返回 std::wstring（宽字符）；A 后缀：返回 std::string（窄字符）
    */
    static std::wstring UInt64ToStringW(uint64_t value);
    static std::wstring UInt32ToStringW(uint32_t value);
    static std::string UInt64ToStringA(uint64_t value);
    static std::string UInt32ToStringA(uint32_t value);

    /** 将有符号整型数值转换为指定编码的字符串
    * @param [in] value 要转换的整型数值
    * @return 返回转换后的字符串：W 后缀返回 std::wstring（宽字符），A 后缀返回 std::string（窄字符）
    */
    static std::wstring Int64ToStringW(int64_t value);
    static std::wstring Int32ToStringW(int32_t value);
    static std::string Int64ToStringA(int64_t value);
    static std::string Int32ToStringA(int32_t value);

    /** 将整型数值转换为字符串（返回值类型随编码模式变化）
    * @param [in] value 要转换的整型数值
    * @return 返回转换后的字符串：定义 DUILIB_UNICODE 时返回 std::wstring，否则返回 std::string
    */
#ifdef DUILIB_UNICODE
    static std::wstring UInt64ToString(uint64_t value);
    static std::wstring UInt32ToString(uint32_t value);
    static std::wstring Int64ToString(int64_t value);
    static std::wstring Int32ToString(int32_t value);
#else
    static std::string UInt64ToString(uint64_t value);
    static std::string UInt32ToString(uint32_t value);
    static std::string Int64ToString(int64_t value);
    static std::string Int32ToString(int32_t value);
#endif

    /** 将字符串转换成整型（十进制）
    * @param [in] str 要转换的字符串（std::wstring/std::string 以及对应的字符指针）
    * @return 返回转换后的32位整数，无法转换时返回0
    */
    static int32_t StringToInt32(const std::wstring& str);
    static int32_t StringToInt32(const std::wstring::value_type* str);
    static int32_t StringToInt32(const std::string& str);
    static int32_t StringToInt32(const std::string::value_type* str);

    /** 将字符串转换成整型（可指定进制）
    * @param [in] str 要转换的字符串
    * @param [out] pEndPtr 返回解析结束位置的字符指针，可为 nullptr（不需要时）
    * @param [in] nRadix 进制，取值范围通常为 2~36，10 表示十进制，16 表示十六进制
    * @return 返回转换后的整数（有符号/无符号取决于函数名），无法转换时返回0
    */
    static int32_t StringToInt32(const wchar_t* str, wchar_t** pEndPtr, int32_t nRadix);
    static int32_t StringToInt32(const char* str, char** pEndPtr, int32_t nRadix);
    static uint32_t StringToUInt32(const wchar_t* str, wchar_t** pEndPtr, int32_t nRadix);
    static uint32_t StringToUInt32(const char* str, char** pEndPtr, int32_t nRadix);

    /** 将字符串转换成64位整型（十进制）
    * @param [in] str 要转换的字符串（std::wstring/std::string 以及对应的字符指针）
    * @return 返回转换后的64位整数，无法转换时返回0
    */
    static int64_t StringToInt64(const std::wstring& str);
    static int64_t StringToInt64(const std::wstring::value_type* str);
    static int64_t StringToInt64(const std::string& str);
    static int64_t StringToInt64(const std::string::value_type* str);

    /** 将字符串转换成浮点型（十进制，double）
    * @param [in] str 要转换的字符串（std::wstring/std::string 以及对应的字符指针）
    * @return 返回转换后的双精度浮点数，无法转换时返回0
    */
    static double StringToDouble(const std::wstring& str);
    static double StringToDouble(const std::wstring::value_type* str);
    static double StringToDouble(const std::string& str);
    static double StringToDouble(const std::string::value_type* str);

    /** 将字符串转换成浮点型（十进制，float）
    * @param [in] str 要转换的字符串
    * @param [out] pEndPtr 返回解析结束位置的字符指针，可为 nullptr（不需要时）
    * @return 返回转换后的单精度浮点数，无法转换时返回0
    */
    static float StringToFloat(const wchar_t* str, wchar_t** pEndPtr);
    static float StringToFloat(const char* str, char** pEndPtr);
    static float StringToFloat(const wchar_t* str);
    static float StringToFloat(const char* str);

    /** 复制字符串（源字符串超长时按截断处理，并保证目标缓冲区以 '\0' 结尾）
    * @param [in,out] dest 目标缓冲区（数组版本会自动按数组长度传入 destSize）
    * @param [in] destSize 目标缓冲区的大小（字符数，含结尾的 '\0'）
    * @param [in] src 源字符串
    * @return 返回值仅作占位，始终为0
    */
    template <size_t _Size>
    static inline int32_t StringCopy(wchar_t(&dest)[_Size], const wchar_t* src)
    {
        return StringCopy(dest, _Size, src);
    }

    /** 复制字符串（宽字符版本，最多复制 destSize - 1 个字符）
    * @param [in,out] dest 目标缓冲区
    * @param [in] destSize 目标缓冲区的大小（字符数，含结尾的 '\0'）
    * @param [in] src 源字符串
    * @return 返回值仅作占位，始终为0
    */
    static int32_t StringCopy(wchar_t* dest, size_t destSize, const wchar_t* src);
    /** 复制字符串的指定长度（宽字符版本，最多复制 destSize - 1 个字符）
    * @param [in,out] dest 目标缓冲区
    * @param [in] destSize 目标缓冲区的大小（字符数，含结尾的 '\0'）
    * @param [in] src 源字符串
    * @param [in] srcSize 源字符串最多读取的字符数
    * @return 返回值仅作占位，始终为0
    */
    static int32_t StringNCopy(wchar_t* dest, size_t destSize, const wchar_t* src, size_t srcSize);

    template <size_t _Size>
    static inline int32_t StringCopy(char(&dest)[_Size], const char* src)
    {
        return StringCopy(dest, _Size, src);
    }

    /** 复制字符串（窄字符版本，最多复制 destSize - 1 个字符）
    * @param [in,out] dest 目标缓冲区
    * @param [in] destSize 目标缓冲区的大小（字节数，含结尾的 '\0'）
    * @param [in] src 源字符串
    * @return 返回值仅作占位，始终为0
    */
    static int32_t StringCopy(char* dest, size_t destSize, const char* src);
    /** 复制字符串的指定长度（窄字符版本，最多复制 destSize - 1 个字符）
    * @param [in,out] dest 目标缓冲区
    * @param [in] destSize 目标缓冲区的大小（字节数，含结尾的 '\0'）
    * @param [in] src 源字符串
    * @param [in] srcSize 源字符串最多读取的字节数
    * @return 返回值仅作占位，始终为0
    */
    static int32_t StringNCopy(char* dest, size_t destSize, const char* src, size_t srcSize);

    /** 计算字符串的长度（不含结尾的 '\0'）
    * @param [in] str 要计算的字符串，为 nullptr 时返回0
    * @return 返回字符串的字符数（std::wstring 版本为宽字符数，std::string 版本为窄字符数）
    */
    static size_t StringLen(const wchar_t* str);
    static size_t StringLen(const char* str);

    /** 判断字符串表示的布尔值是否为真
    * @param [in] value 要判断的字符串，取值为 "true" 或 "1" 时返回 true
    * @return 返回 true 表示为真，否则返回 false
    */
    static bool IsValueTrue(const DString& value);
};

}

#endif // UI_UTILS_STRINGUTIL_H_
