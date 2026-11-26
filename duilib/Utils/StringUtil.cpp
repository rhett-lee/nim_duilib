#include "StringUtil.h"
#include <filesystem>
#include <cstdlib>
#include <cstdarg>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdint>
#include <cwchar>
#include <algorithm>

#ifndef DUILIB_BUILD_FOR_WIN
#include <strings.h>
#endif

namespace ui
{

#define COUNT_OF(array)            (sizeof(array)/sizeof(array[0]))

namespace
{

template<typename CharType>
int StringTokenizeT(const std::basic_string<CharType> &input,
                    const std::basic_string<CharType> &delimitor,
                    std::list<std::basic_string<CharType> > &output)
{
    size_t token_begin;
    size_t token_end;

    output.clear();

    for (token_begin = token_end = 0; token_end != std::basic_string<CharType>::npos;)
    {
        token_begin = input.find_first_not_of(delimitor, token_begin);
        if (token_begin == std::basic_string<CharType>::npos)
            break;
        token_end = input.find_first_of(delimitor, token_begin + 1);
        output.push_back(input.substr(token_begin, token_end - token_begin));
        token_begin = token_end + 1;
    }

    return static_cast<int>(output.size());
}

template<typename CharType>
size_t StringReplaceAllT(const std::basic_string<CharType> &find,
                         const std::basic_string<CharType> &replace,
                         std::basic_string<CharType> &output)
{
    size_t find_length = find.size();
    size_t replace_length = replace.size();
    size_t offset = 0, endpos = 0;
    size_t target = 0, found_pos = 0;
    size_t replaced = 0;
    CharType *data_ptr = nullptr;

    if (find.empty())
        return 0;

    /*
     * to avoid extra memory reallocating,
     * we use two passes to finish the task in the case that replace.size() is greater find.size()
     */

    if (find_length < replace_length)
    {
        /* the first pass, count all available 'find' to be replaced  */
        for (;;)
        {
            offset = output.find(find, offset);
            if (offset == std::basic_string<CharType>::npos)
                break;
            replaced++;
            offset += find_length;
        }

        if (replaced == 0)
            return 0;

        size_t newsize = output.size() + replaced * (replace_length - find_length);

        /* we apply for more memory to hold the content to be replaced */
        endpos = newsize;
        offset = newsize - output.size();
        output.resize(newsize);
        data_ptr = &output[0];

        memmove((void*)(data_ptr + offset),
                (void*)data_ptr,
                (output.size() - offset) * sizeof(CharType));

        //第一遍仅用于统计，第二遍会重新统计实际替换次数，这里清零避免返回值被重复累加
        replaced = 0;
    }
    else
    {
        endpos = output.size();
        offset = 0;
        data_ptr = (CharType*)(&output[0]);
    }

    /* the second pass,  the replacement */
    while (offset < endpos)
    {
        found_pos = output.find(find, offset);
        if (found_pos != std::basic_string<CharType>::npos)
        {
            /* move the content between two targets */
            if (target != found_pos)
                memmove((void*)(data_ptr + target),
                        (void*)(data_ptr + offset),
                        (found_pos - offset) * sizeof(CharType));

            target += found_pos - offset;

            /* replace */
            memcpy(data_ptr + target,
                   replace.data(),
                   replace_length * sizeof(CharType));

            target += replace_length;
            offset = find_length + found_pos;
            replaced++;
        }
        else
        {
            /* ending work  */
            if (target != offset)
                memcpy((void*)(data_ptr + target),
                       (void*)(data_ptr + offset),
                       (endpos - offset) * sizeof(CharType));
            break;
        }
    }

    if (replace_length < find_length)
        output.resize(output.size() - replaced * (find_length - replace_length));

    return replaced;
}

inline int vsnprintfT(char *dst, size_t count, const char *format, va_list ap)
{
    return vsnprintf(dst, count, format, ap);
}

inline int vsnprintfT(wchar_t *dst, size_t count, const wchar_t *format, va_list ap)
{
#ifdef DUILIB_BUILD_FOR_WIN
    //不能直接在缓冲区不足时调用 _vsnwprintf_s：发生截断时会触发无效参数处理器
    //(调试版弹出断言对话框、发布版直接终止进程)。这里先计算所需长度：
    //缓冲区足够时正常格式化；不足时返回所需长度，由调用方扩容后重试
    va_list ap_length;
    va_copy(ap_length, ap);
    const int needLength = ::_vscwprintf(format, ap_length);
    va_end(ap_length);
    if (needLength < 0) {
        return -1;
    }
    if (static_cast<size_t>(needLength) < count) {
        return ::_vsnwprintf_s(dst, count, count, format, ap);
    }
    return needLength;
#else
    return vswprintf(dst, count, format, ap);
#endif
}

template<typename CharType>
void StringAppendVT(const CharType *format, va_list ap, std::basic_string<CharType> &output)
{
    CharType stack_buffer[1024] = {0, };

    /* first, we try to finish the task using a fixed-size buffer in the stack */
    va_list ap_copy;
    va_copy(ap_copy, ap);

    int result = vsnprintfT(stack_buffer, COUNT_OF(stack_buffer), format, ap_copy);
    va_end(ap_copy);
    if (result >= 0 && result < static_cast<int>(COUNT_OF(stack_buffer)))
    {
        /* It fits */
        output.append(stack_buffer, result);
        return;
    }

    std::basic_string<CharType> heap_buffer;

    /* MSVC 的 vsnprintf(char版) 遵循 C99 标准：缓冲区不足时返回"所需长度"而不是 -1。
       原逻辑在 result != -1 时直接 ASSERT 并返回，导致格式化结果超过 1023 字符时返回空字符串。
       这里按返回的所需长度一次性重新格式化，避免该问题 */
    if (result >= 0) {
        const int needSize = result + 1;
        heap_buffer.resize(needSize);
        va_copy(ap_copy, ap);
        const int result2 = vsnprintfT(&heap_buffer[0], needSize, format, ap_copy);
        va_end(ap_copy);
        if ((result2 >= 0) && (result2 < needSize)) {
            output.append(heap_buffer.data(), result2);
        }
        return;
    }

    /* then, we have to repeatedly increase buffer size until it fits. */
    int buffer_size = COUNT_OF(stack_buffer);
    for (;;)
    {
        if (result != -1)
        {
            ASSERT(0);
            return; /* not expected, result should be -1 here */
        }
        // 检查 buffer_size 翻倍是否溢出（防御性）
        if (buffer_size > (INT_MAX / 2)) {
            ASSERT(0);
            return;    /* too long, would overflow */
        }
        buffer_size <<= 1; /* try doubling the buffer size */
        if (buffer_size > 32 * 1024 * 1024)
        {
            ASSERT(0);
            return;    /* too long */
        }
        /* resize */
        heap_buffer.resize(buffer_size);
        /*
         * NOTE: You can only use a va_list once.  Since we're in a while loop, we
         * need to make a new copy each time so we don't use up the original.
         */
        va_copy(ap_copy, ap);
        result = vsnprintfT(&heap_buffer[0], buffer_size, format, ap_copy);
        va_end(ap_copy);

        if ((result >= 0) && (result < buffer_size)) {
            /* It fits */
            output.append(&heap_buffer[0], result);
            return;
        }
    }
}

#define NOT_SPACE(x) ((x) != 0x20 && ((x) < 0 || 0x1d < (x)))

template<typename CharType>
void StringTrimT(std::basic_string<CharType> &output)
{
    if (output.empty()) {
        return;
    }
    size_t bound1 = 0;
    size_t bound2 = output.length();
    const CharType *src = output.data();
    if (src == nullptr) {
        return;
    }

    for (; bound2 > 0; bound2--)
        if (NOT_SPACE(src[bound2-1]))
            break;

    for (; bound1 < bound2; bound1++)
        if (NOT_SPACE(src[bound1]))
            break;

    if (bound1 < bound2) {
        memmove((void *)src,
            src + bound1,
            sizeof(CharType) * (bound2 - bound1));
    }

    output.resize(bound2 - bound1);
}

template<typename CharType>
void StringTrimLeftT(std::basic_string<CharType> &output)
{
    size_t check = 0;
    size_t length = output.length();
    const CharType *src = output.data();
    if (src == nullptr) {
        return;
    }

    for (; check < length; check++)
        if (NOT_SPACE(src[check]))
            break;

    output.erase(0, check);
}

template<typename CharType>
void StringTrimRightT(std::basic_string<CharType> &output)
{
    size_t length = output.length();
    const CharType *src = output.data();
    if (src == nullptr) {
        return;
    }

    for (; length > 0; length--)
        if (NOT_SPACE(src[length-1]))
            break;

    output.resize(length);
}

/** 判断字符是否为空白字符（规则与 Trim 保持一致：空格及 ASCII 控制字符）
*/
inline bool StringIsSpaceChar(char ch)
{
    const unsigned char value = static_cast<unsigned char>(ch);
    return (value == 0x20) || (value <= 0x1d);
}

inline bool StringIsSpaceChar(wchar_t ch)
{
    const uint32_t value = static_cast<uint32_t>(ch);
    return (value == 0x20) || (value <= 0x1d);
}

/** 忽略大小写比较指定长度的字符串
*/
inline int StringNICmp(const char* lhs, const char* rhs, size_t count)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return ::_strnicmp(lhs, rhs, count);
#else
    return ::strncasecmp(lhs, rhs, count);
#endif
}

inline int StringNICmp(const wchar_t* lhs, const wchar_t* rhs, size_t count)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return ::_wcsnicmp(lhs, rhs, count);
#else
    return ::wcsncasecmp(lhs, rhs, count);
#endif
}

template<typename CharType>
bool StringStartsWithT(const std::basic_string<CharType>& str, const std::basic_string<CharType>& prefix)
{
    if (prefix.size() > str.size()) {
        return false;
    }
    return str.compare(0, prefix.size(), prefix) == 0;
}

template<typename CharType>
bool StringStartsWithNoCaseT(const std::basic_string<CharType>& str, const std::basic_string<CharType>& prefix)
{
    if (prefix.size() > str.size()) {
        return false;
    }
    return StringNICmp(str.c_str(), prefix.c_str(), prefix.size()) == 0;
}

template<typename CharType>
bool StringEndsWithT(const std::basic_string<CharType>& str, const std::basic_string<CharType>& suffix)
{
    if (suffix.size() > str.size()) {
        return false;
    }
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

template<typename CharType>
bool StringEndsWithNoCaseT(const std::basic_string<CharType>& str, const std::basic_string<CharType>& suffix)
{
    if (suffix.size() > str.size()) {
        return false;
    }
    return StringNICmp(str.c_str() + (str.size() - suffix.size()), suffix.c_str(), suffix.size()) == 0;
}

template<typename CharType>
bool StringContainsNoCaseT(const std::basic_string<CharType>& str, const std::basic_string<CharType>& sub)
{
    if (sub.empty()) {
        return true;
    }
    if (sub.size() > str.size()) {
        return false;
    }
    for (size_t i = 0; i + sub.size() <= str.size(); ++i) {
        if (StringNICmp(str.c_str() + i, sub.c_str(), sub.size()) == 0) {
            return true;
        }
    }
    return false;
}

template<typename CharType>
std::basic_string<CharType> StringJoinT(const std::vector<std::basic_string<CharType>>& parts,
                                        const std::basic_string<CharType>& separator)
{
    std::basic_string<CharType> output;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            output += separator;
        }
        output += parts[i];
    }
    return output;
}

template<typename CharType>
std::vector<std::basic_string<CharType>> StringSplitByStringT(const std::basic_string<CharType>& input,
                                                              const std::basic_string<CharType>& delimiter,
                                                              bool keepEmpty)
{
    std::vector<std::basic_string<CharType>> output;
    if (delimiter.empty()) {
        output.push_back(input);
        return output;
    }
    const size_t npos = std::basic_string<CharType>::npos;
    size_t start = 0;
    for (;;) {
        const size_t pos = input.find(delimiter, start);
        if (pos == npos) {
            if (keepEmpty || (start < input.size())) {
                output.push_back(input.substr(start));
            }
            break;
        }
        if (keepEmpty || (pos > start)) {
            output.push_back(input.substr(start, pos - start));
        }
        start = pos + delimiter.size();
    }
    return output;
}

template<typename CharType>
std::vector<std::basic_string<CharType>> StringSplitLinesT(const std::basic_string<CharType>& input)
{
    std::vector<std::basic_string<CharType>> output;
    const size_t length = input.size();
    if (length == 0) {
        return output;
    }
    size_t start = 0;
    for (size_t i = 0; i < length; ++i) {
        const CharType ch = input[i];
        if ((ch == static_cast<CharType>('\n')) || (ch == static_cast<CharType>('\r'))) {
            output.push_back(input.substr(start, i - start));
            if ((ch == static_cast<CharType>('\r')) && (i + 1 < length) && (input[i + 1] == static_cast<CharType>('\n'))) {
                ++i;
            }
            start = i + 1;
        }
    }
    if (start < length) {
        output.push_back(input.substr(start));
    }
    return output;
}

template<typename CharType>
std::basic_string<CharType> StringMakeReplaceAllT(const std::basic_string<CharType>& input,
                                                  const std::basic_string<CharType>& find,
                                                  const std::basic_string<CharType>& replace)
{
    if (input.empty() || find.empty()) {
        return input;
    }
    std::basic_string<CharType> output = input;
    StringReplaceAllT<CharType>(find, replace, output);
    return output;
}

template<typename CharType>
std::basic_string<CharType> StringMakeReplaceAllNoCaseT(const std::basic_string<CharType>& input,
                                                        const std::basic_string<CharType>& find,
                                                        const std::basic_string<CharType>& replace)
{
    if (input.empty() || find.empty()) {
        return input;
    }
    std::basic_string<CharType> output;
    output.reserve(input.size());
    size_t lastPos = 0;
    size_t pos = 0;
    while (pos + find.size() <= input.size()) {
        if (StringNICmp(input.c_str() + pos, find.c_str(), find.size()) == 0) {
            output.append(input, lastPos, pos - lastPos);
            output += replace;
            pos += find.size();
            lastPos = pos;
        }
        else {
            ++pos;
        }
    }
    output.append(input, lastPos, std::basic_string<CharType>::npos);
    return output;
}

inline long long StringStrToInt64(const char* str, char** pEndPtr)
{
    return ::strtoll(str, pEndPtr, 10);
}

inline long long StringStrToInt64(const wchar_t* str, wchar_t** pEndPtr)
{
    return ::wcstoll(str, pEndPtr, 10);
}

inline double StringStrToDouble(const char* str, char** pEndPtr)
{
    return ::strtod(str, pEndPtr);
}

inline double StringStrToDouble(const wchar_t* str, wchar_t** pEndPtr)
{
    return ::wcstod(str, pEndPtr);
}

/** 解析64位整数：仅当整个字符串（去掉首尾空白后）都是合法十进制整数时返回 true
*/
template<typename CharType>
bool StringParseInt64T(const std::basic_string<CharType>& str, int64_t& value)
{
    if (str.empty()) {
        return false;
    }
    const CharType* begin = str.c_str();
    CharType* end = nullptr;
    errno = 0;
    const long long parsed = StringStrToInt64(begin, &end);
    if ((errno == ERANGE) || (end == begin)) {
        return false;
    }
    while ((*end != 0) && StringIsSpaceChar(*end)) {
        ++end;
    }
    if (*end != 0) {
        return false;
    }
    value = static_cast<int64_t>(parsed);
    return true;
}

/** 解析双精度浮点数：仅当整个字符串（去掉首尾空白后）都是合法浮点数时返回 true
*/
template<typename CharType>
bool StringParseDoubleT(const std::basic_string<CharType>& str, double& value)
{
    if (str.empty()) {
        return false;
    }
    const CharType* begin = str.c_str();
    CharType* end = nullptr;
    errno = 0;
    const double parsed = StringStrToDouble(begin, &end);
    if ((errno == ERANGE) || (end == begin)) {
        return false;
    }
    while ((*end != 0) && StringIsSpaceChar(*end)) {
        ++end;
    }
    if (*end != 0) {
        return false;
    }
    value = parsed;
    return true;
}

} // anonymous namespace


std::wstring StringUtil::Printf(const wchar_t *format, ...)
{
    va_list args;
    va_start(args, format);
    std::wstring output;
    StringAppendVT<wchar_t>(format, args, output);
    va_end(args);
    return output;
}

std::string StringUtil::Printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::string output;
    StringAppendVT<char>(format, args, output);
    va_end(args);
    return output;
}

size_t StringUtil::ReplaceAll(const std::wstring& find, const std::wstring& replace, std::wstring& output)
{
    if (output.empty()) {
        return 0;
    }
    return StringReplaceAllT<wchar_t>(find, replace, output);
}

size_t StringUtil::ReplaceAll(const std::string& find, const std::string& replace, std::string& output)
{
    if (output.empty())    {
        return 0;
    }
    return StringReplaceAllT<char>(find, replace, output);
}

std::wstring StringUtil::MakeReplaceAll(const std::wstring& input, const std::wstring& find, const std::wstring& replace)
{
    return StringMakeReplaceAllT<wchar_t>(input, find, replace);
}

std::string StringUtil::MakeReplaceAll(const std::string& input, const std::string& find, const std::string& replace)
{
    return StringMakeReplaceAllT<char>(input, find, replace);
}

std::wstring StringUtil::MakeReplaceAllNoCase(const std::wstring& input, const std::wstring& find, const std::wstring& replace)
{
    return StringMakeReplaceAllNoCaseT<wchar_t>(input, find, replace);
}

std::string StringUtil::MakeReplaceAllNoCase(const std::string& input, const std::string& find, const std::string& replace)
{
    return StringMakeReplaceAllNoCaseT<char>(input, find, replace);
}

void StringUtil::LowerString(std::string& str)
{
    if (str.empty()) {
        return;
    }
    char* start = str.data();
    char* end = start + str.size();
    for (; start < end; start++) {
        if (*start >= 'A' && *start <= 'Z') {
            *start += 'a' - 'A';
        }
    }
}

void StringUtil::LowerString(std::wstring& str)
{
    if (str.empty()) {
        return;
    }
    wchar_t* start = str.data();
    wchar_t* end = start + str.size();
    for (; start < end; start++) {
        if (*start >= L'A' && *start <= L'Z') {
            *start += L'a' - L'A';
        }
    }
}

void StringUtil::UpperString(std::string& str)
{
    if (str.empty()) {
        return;
    }
    char* start = str.data();
    char* end = start + str.size();
    for (; start < end; start++) {
        if (*start >= 'a' && *start <= 'z') {
            *start -= 'a' - 'A';
        }
    }
}

void StringUtil::UpperString(std::wstring& str)
{
    if (str.empty()) {
        return;
    }
    wchar_t* start = str.data();
    wchar_t* end = start + str.size();
    for (; start < end; start++) {
        if (*start >= L'a' && *start <= L'z') {
            *start -= L'a' - L'A';
        }
    }
}

std::wstring StringUtil::MakeLowerString(const std::wstring&str)
{
    std::wstring resStr = str;
    if (resStr.empty()) {
        return L"";
    }
    wchar_t *start = resStr.data();
    wchar_t *end = start + resStr.size();
    for (; start < end; start++) {
        if (*start >= L'A' && *start <= L'Z') {
            *start += L'a' - L'A';
        }
    }    
    return resStr;
}

std::string StringUtil::MakeLowerString(const std::string& str)
{
    std::string resStr = str;
    if (resStr.empty()) {
        return "";
    }
    char* start = resStr.data();
    char* end = start + resStr.size();
    for (; start < end; start++) {
        if (*start >= 'A' && *start <= 'Z') {
            *start += 'a' - 'A';
        }
    }
    return resStr;
}

std::wstring StringUtil::MakeUpperString(const std::wstring& str)
{
    std::wstring resStr = str;
    if (resStr.empty()) {
        return L"";
    }
    wchar_t *start = resStr.data();
    wchar_t *end = start + resStr.size();
    for (; start < end; ++start) {
        if (*start >= L'a' && *start <= L'z') {
            *start -= L'a' - L'A';
        }
    }
    return resStr;
}

std::string StringUtil::MakeUpperString(const std::string& str)
{
    std::string resStr = str;
    if (resStr.empty()) {
        return "";
    }
    char* start = resStr.data();
    char* end = start + resStr.size();
    for (; start < end; ++start) {
        if (*start >= 'a' && *start <= 'z') {
            *start -= 'a' - 'A';
        }
    }
    return resStr;
}

std::string StringUtil::TrimLeft(const char *input)
{
    std::string output = (input != nullptr) ? std::string(input) : std::string();
    TrimLeft(output);
    return output;
}

std::string StringUtil::TrimRight(const char *input)
{
    std::string output = (input != nullptr) ? std::string(input) : std::string();
    TrimRight(output);
    return output;
}

std::string StringUtil::Trim(const char *input) /* both left and right */
{
    std::string output = (input != nullptr) ? std::string(input) : std::string();
    Trim(output);
    return output;
}

std::string& StringUtil::TrimLeft(std::string &input)
{
    StringTrimLeftT<char>(input);
    return input;
}

std::string& StringUtil::TrimRight(std::string &input)
{
    StringTrimRightT<char>(input);
    return input;
}

std::string& StringUtil::Trim(std::string &input) /* both left and right */
{
    StringTrimT<char>(input);
    return input;
}

std::wstring StringUtil::TrimLeft(const wchar_t *input)
{
    std::wstring output = (input != nullptr) ? std::wstring(input) : std::wstring();
    TrimLeft(output);
    return output;
}

std::wstring StringUtil::TrimRight(const wchar_t *input)
{
    std::wstring output = (input != nullptr) ? std::wstring(input) : std::wstring();
    TrimRight(output);
    return output;
}

std::wstring StringUtil::Trim(const wchar_t *input) /* both left and right */
{
    std::wstring output = (input != nullptr) ? std::wstring(input) : std::wstring();
    Trim(output);
    return output;
}

std::wstring& StringUtil::TrimLeft(std::wstring&input)
{
    StringTrimLeftT<wchar_t>(input);
    return input;
}

std::wstring& StringUtil::TrimRight(std::wstring&input)
{
    StringTrimRightT<wchar_t>(input);
    return input;
}

std::wstring& StringUtil::Trim(std::wstring&input) /* both left and right */
{
    StringTrimT<wchar_t>(input);
    return input;
}

bool StringUtil::IsNullOrEmpty(const wchar_t* str)
{
    return (str == nullptr) || (*str == 0);
}

bool StringUtil::IsNullOrEmpty(const char* str)
{
    return (str == nullptr) || (*str == 0);
}

bool StringUtil::IsNullOrEmpty(const std::wstring& str)
{
    return str.empty();
}

bool StringUtil::IsNullOrEmpty(const std::string& str)
{
    return str.empty();
}

bool StringUtil::IsNullOrWhiteSpace(const wchar_t* str)
{
    if (str == nullptr) {
        return true;
    }
    for (; *str != 0; ++str) {
        if (!StringIsSpaceChar(*str)) {
            return false;
        }
    }
    return true;
}

bool StringUtil::IsNullOrWhiteSpace(const char* str)
{
    if (str == nullptr) {
        return true;
    }
    for (; *str != 0; ++str) {
        if (!StringIsSpaceChar(*str)) {
            return false;
        }
    }
    return true;
}

bool StringUtil::IsNullOrWhiteSpace(const std::wstring& str)
{
    for (wchar_t ch : str) {
        if (!StringIsSpaceChar(ch)) {
            return false;
        }
    }
    return true;
}

bool StringUtil::IsNullOrWhiteSpace(const std::string& str)
{
    for (char ch : str) {
        if (!StringIsSpaceChar(ch)) {
            return false;
        }
    }
    return true;
}


std::list<std::string> StringUtil::Split(const std::string& input, const std::string& delimitor)
{
    std::list<std::string> output;
    std::string input2(input);

    if (input2.empty())
        return output;

    char* context = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    char *token = strtok_s(input2.data(), delimitor.c_str(), &context);
#else
    char* token = strtok_r(input2.data(), delimitor.c_str(), &context);
#endif
    while (token != nullptr) {
        output.push_back(token);
#ifdef DUILIB_BUILD_FOR_WIN
        token = strtok_s(nullptr, delimitor.c_str(), &context);
#else
        token = strtok_r(nullptr, delimitor.c_str(), &context);
#endif
    }
    return output;
}

std::list<std::wstring> StringUtil::Split(const std::wstring& input, const std::wstring& delimitor)
{
    std::list<std::wstring> output;
    std::wstring input2(input);

    if (input2.empty()) {
        return output;
    }

    wchar_t* context = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    wchar_t* token = wcstok_s(input2.data(), delimitor.c_str(), &context);
#else
    wchar_t* token = wcstok(input2.data(), delimitor.c_str(), &context);
#endif
    while (token != nullptr) {
        output.push_back(token);
#ifdef DUILIB_BUILD_FOR_WIN
        token = wcstok_s(nullptr, delimitor.c_str(), &context);
#else
        token = wcstok(nullptr, delimitor.c_str(), &context);
#endif
    }
    return output;
}

std::vector<std::string> StringUtil::SplitByString(const std::string& input, const std::string& delimiter, bool keepEmpty)
{
    return StringSplitByStringT<char>(input, delimiter, keepEmpty);
}

std::vector<std::wstring> StringUtil::SplitByString(const std::wstring& input, const std::wstring& delimiter, bool keepEmpty)
{
    return StringSplitByStringT<wchar_t>(input, delimiter, keepEmpty);
}

std::vector<std::string> StringUtil::SplitLines(const std::string& input)
{
    return StringSplitLinesT<char>(input);
}

std::vector<std::wstring> StringUtil::SplitLines(const std::wstring& input)
{
    return StringSplitLinesT<wchar_t>(input);
}

std::string StringUtil::Join(const std::vector<std::string>& parts, const std::string& separator)
{
    return StringJoinT<char>(parts, separator);
}

std::wstring StringUtil::Join(const std::vector<std::wstring>& parts, const std::wstring& separator)
{
    return StringJoinT<wchar_t>(parts, separator);
}

static bool IsEqualNoCasePrivate(const wchar_t* lhs, const wchar_t* rhs)
{
    //空指针统一按空字符串处理，保证各重载的空值语义一致
    static const wchar_t kEmptyStr[] = L"";
    if (lhs == nullptr) {
        lhs = kEmptyStr;
    }
    if (rhs == nullptr) {
        rhs = kEmptyStr;
    }
    if (lhs == rhs) {
        return true;
    }
    for (;;) {
        if (*lhs == *rhs) {
            if (*lhs++ == 0) {
                return true;
            }
            rhs++;
            continue;
        }
        if (towupper(*lhs++) == towupper(*rhs++)) {
            continue;
        }
        return false;
    }
}

static bool IsEqualNoCasePrivate(const char* lhs, const char* rhs)
{
    //空指针统一按空字符串处理，保证各重载的空值语义一致
    static const char kEmptyStr[] = "";
    if (lhs == nullptr) {
        lhs = kEmptyStr;
    }
    if (rhs == nullptr) {
        rhs = kEmptyStr;
    }
    if (lhs == rhs) {
        return true;
    }
    for (;;) {
        if (*lhs == *rhs) {
            if (*lhs++ == 0) {
                return true;
            }
            rhs++;
            continue;
        }
        if (toupper(*lhs++) == toupper(*rhs++)) {
            continue;
        }
        return false;
    }
}

bool StringUtil::IsEqualNoCase(const std::wstring& lhs, const std::wstring& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    return IsEqualNoCasePrivate(lhs.c_str(), rhs.c_str());
}

bool StringUtil::IsEqualNoCase(const wchar_t* lhs, const std::wstring& rhs)
{
    return IsEqualNoCasePrivate(lhs, rhs.c_str());
}

bool StringUtil::IsEqualNoCase(const char* lhs, const std::string& rhs)
{
    return IsEqualNoCasePrivate(lhs, rhs.c_str());
}

bool StringUtil::IsEqualNoCase(const std::wstring& lhs, const wchar_t* rhs)
{
    return IsEqualNoCasePrivate(lhs.c_str(), rhs);
}

bool StringUtil::IsEqualNoCase(const std::string& lhs, const std::string& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    return IsEqualNoCasePrivate(lhs.c_str(), rhs.c_str());
}

bool StringUtil::IsEqualNoCase(const std::string& lhs, const char* rhs)
{
    return IsEqualNoCasePrivate(lhs.c_str(), rhs);
}

bool StringUtil::IsEqualNoCase(const wchar_t* lhs, const wchar_t* rhs)
{
    return IsEqualNoCasePrivate(lhs, rhs);
}

bool StringUtil::IsEqualNoCase(const char* lhs, const char* rhs)
{
    return IsEqualNoCasePrivate(lhs, rhs);
}

bool StringUtil::StartsWith(const std::wstring& str, const std::wstring& prefix)
{
    return StringStartsWithT<wchar_t>(str, prefix);
}

bool StringUtil::StartsWith(const std::string& str, const std::string& prefix)
{
    return StringStartsWithT<char>(str, prefix);
}

bool StringUtil::StartsWithNoCase(const std::wstring& str, const std::wstring& prefix)
{
    return StringStartsWithNoCaseT<wchar_t>(str, prefix);
}

bool StringUtil::StartsWithNoCase(const std::string& str, const std::string& prefix)
{
    return StringStartsWithNoCaseT<char>(str, prefix);
}

bool StringUtil::EndsWith(const std::wstring& str, const std::wstring& suffix)
{
    return StringEndsWithT<wchar_t>(str, suffix);
}

bool StringUtil::EndsWith(const std::string& str, const std::string& suffix)
{
    return StringEndsWithT<char>(str, suffix);
}

bool StringUtil::EndsWithNoCase(const std::wstring& str, const std::wstring& suffix)
{
    return StringEndsWithNoCaseT<wchar_t>(str, suffix);
}

bool StringUtil::EndsWithNoCase(const std::string& str, const std::string& suffix)
{
    return StringEndsWithNoCaseT<char>(str, suffix);
}

bool StringUtil::ContainsNoCase(const std::wstring& str, const std::wstring& sub)
{
    return StringContainsNoCaseT<wchar_t>(str, sub);
}

bool StringUtil::ContainsNoCase(const std::string& str, const std::string& sub)
{
    return StringContainsNoCaseT<char>(str, sub);
}

int32_t StringUtil::StringCompare(const std::wstring& lhs, const std::wstring& rhs)
{
    return ::wcscmp(lhs.c_str(), rhs.c_str());
}

int32_t StringUtil::StringCompare(const wchar_t* lhs, const wchar_t* rhs)
{
    if ((lhs == nullptr) && (rhs == nullptr)) {
        return 0;
    }
    else if (lhs == nullptr) {
        return -1;
    }
    else if (rhs == nullptr) {
        return 1;
    }
    else {
        return ::wcscmp(lhs, rhs);
    }
}

int32_t StringUtil::StringCompare(const std::string& lhs, const std::string& rhs)
{
    return ::strcmp(lhs.c_str(), rhs.c_str());
}

int32_t StringUtil::StringCompare(const char* lhs, const char* rhs)
{
    if ((lhs == nullptr) && (rhs == nullptr)) {
        return 0;
    }
    else if (lhs == nullptr) {
        return -1;
    }
    else if (rhs == nullptr) {
        return 1;
    }
    else {
        return ::strcmp(lhs, rhs);
    }
}

int32_t StringUtil::StringICompare(const std::wstring& lhs, const std::wstring& rhs)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return ::_wcsicmp(lhs.c_str(), rhs.c_str());
#else
    return ::wcscasecmp(lhs.c_str(), rhs.c_str());
#endif
}

int32_t StringUtil::StringICompare(const wchar_t* lhs, const wchar_t* rhs)
{
    if ((lhs == nullptr) && (rhs == nullptr)) {
        return 0;
    }
    else if (lhs == nullptr) {
        return -1;
    }
    else if (rhs == nullptr) {
        return 1;
    }
    else {
#ifdef DUILIB_BUILD_FOR_WIN
        return ::_wcsicmp(lhs, rhs);
#else
        return ::wcscasecmp(lhs, rhs);
#endif
    }
}

int32_t StringUtil::StringICompare(const std::string& lhs, const std::string& rhs)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return ::_stricmp(lhs.c_str(), rhs.c_str());
#else
    return ::strcasecmp(lhs.c_str(), rhs.c_str());
#endif
}

int32_t StringUtil::StringICompare(const char* lhs, const char* rhs)
{
    if ((lhs == nullptr) && (rhs == nullptr)) {
        return 0;
    }
    else if (lhs == nullptr) {
        return -1;
    }
    else if (rhs == nullptr) {
        return 1;
    }
    else {
#ifdef DUILIB_BUILD_FOR_WIN
        return ::_stricmp(lhs, rhs);
#else
        return ::strcasecmp(lhs, rhs);
#endif
    }
}


std::wstring StringUtil::UInt64ToStringW(uint64_t value)
{
    wchar_t temp[32] = {0, };
    int pos = 0;
    do {
        temp[pos++] = (wchar_t)(L'0' + (int)(value % 10));
        value /= 10;
    } while (value != 0);

    //将低位在前的数字反转为正常顺序，然后一次性构造字符串，避免逐字符 += 触发多次内存分配
    for (int i = 0, j = pos - 1; i < j; ++i, --j) {
        const wchar_t ch = temp[i];
        temp[i] = temp[j];
        temp[j] = ch;
    }
    return std::wstring(temp, (size_t)pos);
}

std::wstring StringUtil::UInt32ToStringW(uint32_t value)
{
    return UInt64ToStringW(value);
}

std::string StringUtil::UInt64ToStringA(uint64_t value)
{
    char temp[32] = { 0 };
    int pos = 0;
    do {
        temp[pos++] = (char)('0' + (int)(value % 10));
        value /= 10;
    } while (value != 0);

    //将低位在前的数字反转为正常顺序，然后一次性构造字符串，避免逐字符 += 触发多次内存分配
    for (int i = 0, j = pos - 1; i < j; ++i, --j) {
        const char ch = temp[i];
        temp[i] = temp[j];
        temp[j] = ch;
    }
    return std::string(temp, (size_t)pos);
}

std::string StringUtil::UInt32ToStringA(uint32_t value)
{
    return UInt64ToStringA(value);
}

std::wstring StringUtil::Int64ToStringW(int64_t value)
{
#if defined (DUILIB_BUILD_FOR_WIN)
    return StringUtil::Printf(L"%I64d", value);
#else
    return StringUtil::Printf(L"%lld", value);
#endif
}

std::wstring StringUtil::Int32ToStringW(int32_t value)
{
    return StringUtil::Printf(L"%d", value);
}

std::string StringUtil::Int64ToStringA(int64_t value)
{
#if defined (DUILIB_BUILD_FOR_WIN)
    return StringUtil::Printf("%I64d", value);
#else
    return StringUtil::Printf("%lld", value);
#endif
}

std::string StringUtil::Int32ToStringA(int32_t value)
{
    return StringUtil::Printf("%d", value);
}

#ifdef DUILIB_UNICODE
std::wstring StringUtil::UInt64ToString(uint64_t value)
{
    return UInt64ToStringW(value);
}

std::wstring StringUtil::UInt32ToString(uint32_t value)
{
    return UInt32ToStringW(value);
}

std::wstring StringUtil::Int64ToString(int64_t value)
{
    return Int64ToStringW(value);
}

std::wstring StringUtil::Int32ToString(int32_t value)
{
    return Int32ToStringW(value);
}
#else

std::string StringUtil::UInt64ToString(uint64_t value)
{
    return UInt64ToStringA(value);
}

std::string StringUtil::UInt32ToString(uint32_t value)
{
    return UInt32ToStringA(value);
}

std::string StringUtil::Int64ToString(int64_t value)
{
    return Int64ToStringA(value);
}

std::string StringUtil::Int32ToString(int32_t value)
{
    return Int32ToStringA(value);
}
#endif

int32_t StringUtil::StringToInt32(const std::wstring& str)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return ::_wtoi(str.c_str());
#else
    return wcstol(str.c_str(), nullptr, 10);
#endif
}

int32_t StringUtil::StringToInt32(const std::wstring::value_type* str)
{
    ASSERT(str != nullptr);
    if (str != nullptr) {
#ifdef DUILIB_BUILD_FOR_WIN
        return ::_wtoi(str);
#else
        return wcstol(str, nullptr, 10);
#endif
    }
    else {
        return 0;
    }
}

int32_t StringUtil::StringToInt32(const std::string& str)
{
    return ::atoi(str.c_str());
}

int32_t StringUtil::StringToInt32(const std::string::value_type* str)
{
    ASSERT(str != nullptr);
    if (str != nullptr) {
        return ::atoi(str);
    }
    else {
        return 0;
    }
}

int32_t StringUtil::StringToInt32(const wchar_t* str, wchar_t** pEndPtr, int32_t nRadix)
{
    if (str == nullptr) {
        return 0;
    }
    return ::wcstol(str, pEndPtr, nRadix);
}

int32_t StringUtil::StringToInt32(const char* str, char** pEndPtr, int32_t nRadix)
{
    if (str == nullptr) {
        return 0;
    }
    return ::strtol(str, pEndPtr, nRadix);
}

uint32_t StringUtil::StringToUInt32(const wchar_t* str, wchar_t** pEndPtr, int32_t nRadix)
{
    if (str == nullptr) {
        return 0;
    }
    return ::wcstoul(str, pEndPtr, nRadix);
}

uint32_t StringUtil::StringToUInt32(const char* str, char** pEndPtr, int32_t nRadix)
{
    if (str == nullptr) {
        return 0;
    }
    return ::strtoul(str, pEndPtr, nRadix);
}

int64_t StringUtil::StringToInt64(const std::wstring& str)
{
    //返回值是有符号类型，应使用有符号转换函数，避免负数依赖无符号回绕的实现定义行为
    return ::wcstoll(str.c_str(), nullptr, 10);
}

int64_t StringUtil::StringToInt64(const std::wstring::value_type* str)
{
    ASSERT(str != nullptr);
    if (str != nullptr) {
        return ::wcstoll(str, nullptr, 10);
    }
    else {
        return 0;
    }
}

int64_t StringUtil::StringToInt64(const std::string& str)
{
    //返回值是有符号类型，应使用有符号转换函数，避免负数依赖无符号回绕的实现定义行为
    return ::strtoll(str.c_str(), nullptr, 10);
}

int64_t StringUtil::StringToInt64(const std::string::value_type* str)
{
    ASSERT(str != nullptr);
    if (str != nullptr) {
        return ::strtoll(str, nullptr, 10);
    }
    else {
        return 0;
    }
}


double StringUtil::StringToDouble(const std::wstring& str)
{
    return ::wcstod(str.c_str(), nullptr);
}

double StringUtil::StringToDouble(const std::wstring::value_type* str)
{
    ASSERT(str != nullptr);
    if (str != nullptr) {
        return ::wcstod(str, nullptr);
    }
    else {
        return 0;
    }
}

double StringUtil::StringToDouble(const std::string& str)
{
    return ::strtod(str.c_str(), nullptr);
}

double StringUtil::StringToDouble(const std::string::value_type* str)
{
    ASSERT(str != nullptr);
    if (str != nullptr) {
        return ::strtod(str, nullptr);
    }
    else {
        return 0;
    }
}

int32_t StringUtil::StringToInt32(const std::wstring& str, bool* pbOk)
{
    int64_t value = 0;
    bool ok = StringParseInt64T(str, value);
    if (ok && ((value < INT32_MIN) || (value > INT32_MAX))) {
        ok = false;
    }
    if (pbOk != nullptr) {
        *pbOk = ok;
    }
    return ok ? static_cast<int32_t>(value) : 0;
}

int32_t StringUtil::StringToInt32(const std::string& str, bool* pbOk)
{
    int64_t value = 0;
    bool ok = StringParseInt64T(str, value);
    if (ok && ((value < INT32_MIN) || (value > INT32_MAX))) {
        ok = false;
    }
    if (pbOk != nullptr) {
        *pbOk = ok;
    }
    return ok ? static_cast<int32_t>(value) : 0;
}

int64_t StringUtil::StringToInt64(const std::wstring& str, bool* pbOk)
{
    int64_t value = 0;
    const bool ok = StringParseInt64T(str, value);
    if (pbOk != nullptr) {
        *pbOk = ok;
    }
    return ok ? value : 0;
}

int64_t StringUtil::StringToInt64(const std::string& str, bool* pbOk)
{
    int64_t value = 0;
    const bool ok = StringParseInt64T(str, value);
    if (pbOk != nullptr) {
        *pbOk = ok;
    }
    return ok ? value : 0;
}

double StringUtil::StringToDouble(const wchar_t* str, wchar_t** pEndPtr)
{
    if (str == nullptr) {
        return 0;
    }
    return ::wcstod(str, pEndPtr);
}

double StringUtil::StringToDouble(const char* str, char** pEndPtr)
{
    if (str == nullptr) {
        return 0;
    }
    return ::strtod(str, pEndPtr);
}

double StringUtil::StringToDouble(const std::wstring& str, bool* pbOk)
{
    double value = 0;
    const bool ok = StringParseDoubleT(str, value);
    if (pbOk != nullptr) {
        *pbOk = ok;
    }
    return ok ? value : 0;
}

double StringUtil::StringToDouble(const std::string& str, bool* pbOk)
{
    double value = 0;
    const bool ok = StringParseDoubleT(str, value);
    if (pbOk != nullptr) {
        *pbOk = ok;
    }
    return ok ? value : 0;
}

float StringUtil::StringToFloat(const wchar_t* str, wchar_t** pEndPtr)
{
    if (str == nullptr) {
        return 0;
    }
    return ::wcstof(str, pEndPtr);
}

float StringUtil::StringToFloat(const char* str, char** pEndPtr)
{
    if (str == nullptr) {
        return 0;
    }
    return ::strtof(str, pEndPtr);
}

float StringUtil::StringToFloat(const wchar_t* str)
{
    return StringToFloat(str, nullptr);
}

float StringUtil::StringToFloat(const char* str)
{
    return StringToFloat(str, nullptr);
}

int32_t StringUtil::StringCopy(wchar_t* dest, size_t destSize, const wchar_t* src)
{
    if ((dest == nullptr) || (destSize == 0) || (src == nullptr)) {
        return 0;
    }
    //最多拷贝 destSize - 1 个字符，给结尾的'\0'预留空间，避免写入 dest[destSize] 造成越界。
    //Windows平台原来使用 wcscpy_s：源字符串超长时会触发无效参数处理器(调试版断言、发布版终止进程)，
    //而多数调用方传入的是系统结构体中的定长数组(内容长度不可控)，因此各平台统一按截断处理
    size_t nLen = std::min((size_t)wcslen(src), destSize - 1);
    memcpy(dest, src, nLen * sizeof(wchar_t));
    dest[nLen] = L'\0';
    return 0;
}

int32_t StringUtil::StringNCopy(wchar_t* dest, size_t destSize, const wchar_t* src, size_t srcSize)
{
    if ((dest == nullptr) || (destSize == 0) || (src == nullptr) || (srcSize == 0)) {
        return 0;
    }
    //最多拷贝 destSize - 1 个字符，给结尾的'\0'预留空间，避免写入 dest[destSize] 造成越界
    size_t nLen = std::min(srcSize, destSize - 1);
    nLen = std::min(nLen, (size_t)wcsnlen(src, nLen));
    memcpy(dest, src, nLen * sizeof(wchar_t));
    dest[nLen] = L'\0';
    return 0;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

int32_t StringUtil::StringCopy(char* dest, size_t destSize, const char* src)
{
    if ((dest == nullptr) || (destSize == 0) || (src == nullptr)) {
        return 0;
    }
    //最多拷贝 destSize - 1 个字符，给结尾的'\0'预留空间，避免写入 dest[destSize] 造成越界。
    //与宽字符版本一致：源字符串超长时按截断处理，不触发无效参数处理器
    size_t nLen = std::min(destSize - 1, strlen(src));
    memcpy(dest, src, nLen);
    dest[nLen] = '\0';
    return 0;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

int32_t StringUtil::StringNCopy(char* dest, size_t destSize, const char* src, size_t srcSize)
{
    if ((dest == nullptr) || (destSize == 0) || (src == nullptr) || (srcSize == 0)) {
        return 0;
    }
    //最多拷贝 destSize - 1 个字符，给结尾的'\0'预留空间，避免写入 dest[destSize] 造成越界
    size_t nLen = std::min(srcSize, destSize - 1);
    nLen = std::min(nLen, strnlen(src, nLen));
    memcpy(dest, src, nLen);
    dest[nLen] = '\0';
    return 0;
}

size_t StringUtil::StringLen(const wchar_t* str)
{
    if (str == nullptr) {
        return 0;
    }
    else {
        return ::wcslen(str);
    }
}

size_t StringUtil::StringLen(const char* str)
{
    if (str == nullptr) {
        return 0;
    }
    else {
        return ::strlen(str);
    }
}

bool StringUtil::IsValueTrue(const DString& value)
{
    return StringToBool(value, false);
}

bool StringUtil::StringToBool(const std::wstring& value, bool defaultValue)
{
    size_t begin = 0;
    size_t end = value.size();
    while ((begin < end) && StringIsSpaceChar(value[begin])) {
        ++begin;
    }
    while ((end > begin) && StringIsSpaceChar(value[end - 1])) {
        --end;
    }
    if (begin >= end) {
        return defaultValue;
    }
    const std::wstring token = MakeLowerString(value.substr(begin, end - begin));
    if ((token == L"true") || (token == L"1") || (token == L"yes") || (token == L"on")) {
        return true;
    }
    if ((token == L"false") || (token == L"0") || (token == L"no") || (token == L"off")) {
        return false;
    }
    return defaultValue;
}

bool StringUtil::StringToBool(const std::string& value, bool defaultValue)
{
    size_t begin = 0;
    size_t end = value.size();
    while ((begin < end) && StringIsSpaceChar(value[begin])) {
        ++begin;
    }
    while ((end > begin) && StringIsSpaceChar(value[end - 1])) {
        --end;
    }
    if (begin >= end) {
        return defaultValue;
    }
    const std::string token = MakeLowerString(value.substr(begin, end - begin));
    if ((token == "true") || (token == "1") || (token == "yes") || (token == "on")) {
        return true;
    }
    if ((token == "false") || (token == "0") || (token == "no") || (token == "off")) {
        return false;
    }
    return defaultValue;
}

} // namespace ui
