#include "StringUtil.h"
#include <filesystem>
#include <cstdlib>
#include <cstdarg>
#include <vector>

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
    return _vsnwprintf_s(dst, count, count, format, ap);
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

    /* then, we have to repeatedly increase buffer size until it fits. */
    int buffer_size = COUNT_OF(stack_buffer);
    std::basic_string<CharType> heap_buffer;
    for (;;)
    {
        if (result != -1)
        {
            ASSERT(0);
            return; /* not expected, result should be -1 here */
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
    std::string output = input;
    TrimLeft(output);
    return output;
}

std::string StringUtil::TrimRight(const char *input)
{
    std::string output = input;
    TrimRight(output);
    return output;
}

std::string StringUtil::Trim(const char *input) /* both left and right */
{
    std::string output = input;
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
    std::wstring output = input;
    TrimLeft(output);
    return output;
}

std::wstring StringUtil::TrimRight(const wchar_t *input)
{
    std::wstring output = input;
    TrimRight(output);
    return output;
}

std::wstring StringUtil::Trim(const wchar_t *input) /* both left and right */
{
    std::wstring output = input;
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

static bool IsEqualNoCasePrivate(const wchar_t* lhs, const wchar_t* rhs)
{
    if ((lhs == nullptr) || (rhs == nullptr)) {
        return true;
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
    if ((lhs == nullptr) || (rhs == nullptr)) {
        return true;
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
    if (lhs == nullptr) {
        return false;
    }
    return IsEqualNoCasePrivate(lhs, rhs.c_str());
}

bool StringUtil::IsEqualNoCase(const char* lhs, const std::string& rhs)
{
    if (lhs == nullptr) {
        return false;
    }
    return IsEqualNoCasePrivate(lhs, rhs.c_str());
}

bool StringUtil::IsEqualNoCase(const std::wstring& lhs, const wchar_t* rhs)
{
    if (rhs == nullptr) {
        return false;
    }
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
    if (rhs == nullptr) {
        return false;
    }
    return IsEqualNoCasePrivate(lhs.c_str(), rhs);
}

bool StringUtil::IsEqualNoCase(const wchar_t* lhs, const wchar_t* rhs)
{
    if (lhs == nullptr) {
        return (rhs == nullptr) ? true : false;
    }
    else if (rhs == nullptr) {
        return true;
    }
    return IsEqualNoCasePrivate(lhs, rhs);
}

bool StringUtil::IsEqualNoCase(const char* lhs, const char* rhs)
{
    if (lhs == nullptr) {
        return (rhs == nullptr) ? true : false;
    }
    else if (rhs == nullptr) {
        return true;
    }
    return IsEqualNoCasePrivate(lhs, rhs);
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

    std::wstring str;
    do {
        str += temp[--pos];
    } while (pos > 0);
    return str;
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

    std::string str;
    do {
        str += temp[--pos];
    } while (pos > 0);
    return str;
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
    return ::wcstoull(str.c_str(), nullptr, 10);
}

int64_t StringUtil::StringToInt64(const std::wstring::value_type* str)
{
    ASSERT(str != nullptr);
    if (str != nullptr) {
        return ::wcstoull(str, nullptr, 10);
    }
    else {
        return 0;
    }
}

int64_t StringUtil::StringToInt64(const std::string& str)
{
    return ::strtoull(str.c_str(), nullptr, 10);
}

int64_t StringUtil::StringToInt64(const std::string::value_type* str)
{
    ASSERT(str != nullptr);
    if (str != nullptr) {
        return ::strtoull(str, nullptr, 10);
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

int32_t StringUtil::StringCopy(wchar_t* dest, size_t destSize, const wchar_t* src)
{
    if ((dest == nullptr) || (destSize == 0) || (src == nullptr)) {
        return 0;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    return ::wcscpy_s(dest, destSize, src);
#else
    size_t nLen = std::min((size_t)wcslen(src), destSize);
    ::wcsncpy(dest, src, nLen);
    dest[nLen] = L'\0';
    return 0;
#endif
}

int32_t StringUtil::StringNCopy(wchar_t* dest, size_t destSize, const wchar_t* src, size_t srcSize)
{
    if ((dest == nullptr) || (destSize == 0) || (src == nullptr) || (srcSize == 0)) {
        return 0;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    return ::wcsncpy_s(dest, destSize, src, srcSize);
#else
    size_t nLen = std::min(srcSize, destSize);
    ::wcsncpy(dest, src, nLen);
    dest[nLen] = L'\0';
    return 0;
#endif
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
#ifdef DUILIB_BUILD_FOR_WIN
    return ::strcpy_s(dest, destSize, src);
#else
    size_t nLen = std::min(destSize, strlen(src));
    ::strncpy(dest, src, nLen);
    dest[nLen] = '\0';
    return 0;
#endif
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

int32_t StringUtil::StringNCopy(char* dest, size_t destSize, const char* src, size_t srcSize)
{
    if ((dest == nullptr) || (destSize == 0) || (src == nullptr) || (srcSize == 0)) {
        return 0;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    return ::strncpy_s(dest, destSize, src, srcSize);
#else
    size_t nLen = std::min(srcSize, destSize);
    ::strncpy(dest, src, nLen);
    dest[nLen] = '\0';
    return 0;
#endif
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

} // namespace ui
