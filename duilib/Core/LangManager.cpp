#include "LangManager.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/PerformanceUtil.h"

namespace ui 
{
LangManager::LangManager()
{
}

LangManager::~LangManager()
{
    m_stringTable.clear();
}

bool LangManager::LoadStringTable(const FilePath& strFilePath)
{
    PerformanceStat perfStat(_T("LangManager::LoadStringTable"));
    std::vector<uint8_t> fileData;
    FileUtil::ReadFileData(strFilePath, fileData);
    ASSERT(!fileData.empty());
    if (fileData.empty()) {
        return false;
    }
    fileData.push_back('\0');
    return LoadStringTable(fileData);
}

bool LangManager::LoadStringTable(const std::vector<uint8_t>& fileData)
{
    std::vector<DString> string_list;
    if (fileData.empty()) {
        return false;
    }
    size_t bomSize = 0;
    if ((fileData.size() >= 3)   &&
        (fileData.at(0) == 0xEF) &&
        (fileData.at(1) == 0xBB) &&
        (fileData.at(2) == 0xBF) ) {
        //跳过UTF8的BOM头
        bomSize = 3;
    }
    if (fileData[fileData.size() - 1] == '\0') {
        //数据有尾0，无需拷贝
        return LoadStringTableFromFileData((const char*)fileData.data() + bomSize);
    }
    else {
        //数据无尾0, 拷贝一份再处理
        std::vector<uint8_t> fileDataStr(fileData);
        fileDataStr.push_back('\0');
        return LoadStringTableFromFileData((const char*)fileDataStr.data() + bomSize);
    }
}

bool LangManager::LoadStringTableFromFileData(const char* utf8String)
{
    if (utf8String == nullptr) {
        return false;
    }
#ifdef DUILIB_UNICODE
    //Unicode字符串(需要先转码)
    DString stringData = StringConvert::UTF8ToT(utf8String, StringUtil::StringLen(utf8String));
    return LoadStringTableFromStringData(stringData.c_str(), (int32_t)stringData.size());
#else
    //UTF8字符串, 无需转码
    return LoadStringTableFromStringData(utf8String, -1);
#endif
}

bool LangManager::LoadStringTableFromStringData(const DString::value_type* stringData, int32_t nStringSize)
{
    if ((stringData == nullptr) || (*stringData == _T('\0'))) {
        return false;
    }
    if (nStringSize < 0) {
        nStringSize = (int32_t)StringUtil::StringLen(stringData);
    }
    if (nStringSize <= 0) {
        return true;
    }
    // 直接操作原始字符数组指针，减少开销
    const DString::value_type* ptr = stringData;
    const DString::value_type* start = ptr;       // 当前行的起始位置
    const DString::value_type* const end = ptr + nStringSize; // 文本结束位置（const避免意外修改）

    while (ptr < end) {
        // 遇到换行符时处理当前行
        if (*ptr == _T('\r') || *ptr == _T('\n')) {
            // 计算当前行长度，仅当长度>0时添加（过滤空行）
            const size_t line_len = ptr - start;
            if (line_len > 0) {
                LoadStringTableLine(start, (int32_t)line_len);
            }

            // 跳过当前换行符，并处理Windows风格的\r\n
            ++ptr;
            if (ptr < end && *ptr == _T('\n') && *(ptr - 1) == _T('\r')) {
                ++ptr; // 跳过\r后的\n，避免重复分割
            }
            // 更新下一行的起始位置
            start = ptr;
        }
        else {
            // 非换行符，指针后移
            ++ptr;
        }
    }

    // 处理文本末尾无换行符的最后一行（过滤空行）
    const size_t last_line_len = ptr - start;
    if (last_line_len > 0) {
        LoadStringTableLine(start, (int32_t)last_line_len);
    }
    return true;
}

// 自定义返回结构体：存储分割后的子串指针和长度
struct SplitEqualsResult
{
    const DString::value_type* first_ptr;   // 等号前部分指针
    int32_t first_len;                      // 等号前部分长度
    const DString::value_type* second_ptr;  // 等号后部分指针
    int32_t second_len;                     // 等号后部分长度

    // 构造函数：初始化空值（避免野指针）
    SplitEqualsResult():
        first_ptr(nullptr),
        first_len(0),
        second_ptr(nullptr),
        second_len(0)
    {
    }

    //判断是否为合法的资源字符串
    bool IsValidLanguageString() const
    {
        return (first_ptr != nullptr) && (*first_ptr != _T('\0')) && (first_len > 0) &&
               (second_ptr != nullptr) && (*second_ptr != _T('\0')) && (second_len > 0);
    }
};

/**
 * @brief 将指定长度的字符串按'='分割为两个子串，返回各自的指针和长度
 * @param lineData 输入字符串的起始指针（非空）
 * @param nLineDataLen 输入字符串的有效长度（>=0）
 * @return SplitEqualsResult：包含前后两部分的指针+长度
 *         边界规则：
 *         - 无等号：前半部分=完整字符串（指针+总长度），后半部分=nullptr+0
 *         - 等号在开头：前半部分=nullptr+0，后半部分=等号后指针+剩余长度
 *         - 等号在结尾：前半部分=完整字符串（除等号）指针+长度，后半部分=nullptr+0
 *         - 空输入：双nullptr+双0
 */
SplitEqualsResult split_string_by_equals(const DString::value_type* lineData, int32_t nLineDataLen)
{
    SplitEqualsResult result; // 初始化默认空值

    // 空输入处理：指针为空 或 长度<=0
    if (lineData == nullptr || nLineDataLen <= 0) {
        return result;
    }

    // 遍历字符串，寻找第一个'='
    int32_t equal_index = -1; // 等号的下标（-1表示无等号）
    for (int32_t i = 0; i < nLineDataLen; ++i) {
        if (lineData[i] == _T('=')) {
            equal_index = i;
            break; // 只取第一个等号
        }
    }

    if (equal_index == -1) {
        // 无等号：前半部分=完整字符串，后半部分为空
        result.first_ptr = lineData;
        result.first_len = nLineDataLen;
    }
    else {
        // 有等号：分割前半部分和后半部分
        if (equal_index > 0) {
            // 等号前有内容
            result.first_ptr = lineData;
            result.first_len = equal_index;
        }
        if (equal_index < nLineDataLen - 1) {
            // 等号后有内容
            result.second_ptr = lineData + equal_index + 1;
            result.second_len = nLineDataLen - (equal_index + 1);
        }
    }
    return result;
}

// 自定义返回结构体：trim后的字符串指针+长度
struct TrimStringResult {
    const DString::value_type* ptr;  // trim后的起始指针
    int32_t len;                     // trim后的有效长度

    // 默认构造：空值初始化，避免野指针
    TrimStringResult() : ptr(nullptr), len(0) {}

    // 便捷构造：直接初始化指针和长度
    TrimStringResult(const DString::value_type* p, int32_t l) : ptr(p), len(l) {}

    //是否为有效的字符串
    bool IsValidString() const
    {
        return (ptr != nullptr) && (*ptr != _T('\0')) && (len > 0);
    }
};

/**
 * @brief 高性能trim函数：仅去除字符串首尾空格（无拷贝，仅指针/长度计算）
 * @param lineData 原始字符串指针（非空）
 * @param nLineDataLen 原始字符串有效长度（>=0）
 * @return TrimResult：trim后的起始指针 + 有效长度
 *         边界处理：全空格/空字符串返回 ptr=nullptr + len=0
 */
TrimStringResult trim_lang_string(const DString::value_type* lineData, int32_t nLineDataLen)
{
    // 1. 空输入处理：指针为空 或 长度<=0
    if (lineData == nullptr || nLineDataLen <= 0) {
        return TrimStringResult();
    }

    // 2. 跳过开头的所有空格
    int32_t start = 0;
    while (start < nLineDataLen) {
        if (lineData[start] != _T(' ')) {
            break;
        }
        ++start;
    }

    // 3. 跳过结尾的所有空格
    int32_t end = nLineDataLen - 1;
    while (end >= start) {
        if (lineData[end] != _T(' ')) {
            break;
        }
        --end;
    }

    // 4. 计算有效长度：全空格/无有效字符时返回空
    int32_t valid_len = end - start + 1;
    if (valid_len <= 0) {
        return TrimStringResult();
    }

    // 5. 返回trim后的指针（偏移start）和有效长度
    return TrimStringResult(lineData + start, valid_len);
}

bool LangManager::LoadStringTableLine(const DString::value_type* lineData, int32_t nLineDataLen)
{
    if ((lineData == nullptr) || (*lineData == _T('\0')) || (nLineDataLen <= 0)) {
        return true;
    }
    if ((*lineData == _T('#')) || *lineData == _T(';')) {
        //以";" 或者 "#" 开头为注释，跳过
        return true;
    }

    SplitEqualsResult splitResult = split_string_by_equals(lineData, nLineDataLen);
    bool bValidLanguageString = splitResult.IsValidLanguageString();
    if (!bValidLanguageString) {
        bool bEmptyLine = true;
#ifdef _DEBUG
        for (int32_t i = 0; i < nLineDataLen; ++i) {
            if ((lineData[i] != _T(' ')) && (lineData[i] != _T('\t'))) {
                bEmptyLine = false;
                break;
            }
        }
#endif
        if (bEmptyLine) {
            //改行为空行，跳过
            return true;
        }
        ASSERT(!"LangManager::LoadStringTableLine failed: invalid line data");
        return false;
    }

    TrimStringResult stringIdTrim = trim_lang_string(splitResult.first_ptr, splitResult.first_len);
    TrimStringResult stringValueTrim = trim_lang_string(splitResult.second_ptr, splitResult.second_len);
    if (!stringIdTrim.IsValidString()) {
        ASSERT(!"LangManager::LoadStringTableLine failed: invalid line data");
        return false;
    }
    if (stringValueTrim.IsValidString()) {
        //有字符串ID，有合法的取值
        //替换取值字符串中的转义字符: //将\n和\r替换为真实的换行符、回车符，将\s替换为空格
        bool bEscapeCharacters = false;
        for (int32_t i = 0; i < stringValueTrim.len; ++i) {
            if (stringValueTrim.ptr[i] == _T('\\')) {
                bEscapeCharacters = true; //可能含有转义字符
                break;
            }
        }
        if (bEscapeCharacters) {
            //可能含有转义字符，检测并替换转义字符
            DString stringValue = DString(stringValueTrim.ptr, stringValueTrim.len);
            StringUtil::ReplaceAll(_T("\\r"), _T("\r"), stringValue);
            StringUtil::ReplaceAll(_T("\\n"), _T("\n"), stringValue);
            StringUtil::ReplaceAll(_T("\\s"), _T(" "), stringValue);
            m_stringTable[std::move(DString(stringIdTrim.ptr, stringIdTrim.len))] = std::move(stringValue);
        }
        else {
            //无转义字符
            m_stringTable[std::move(DString(stringIdTrim.ptr, stringIdTrim.len))] = std::move(DString(stringValueTrim.ptr, stringValueTrim.len));
        }
    }
    else {
        //有字符串ID，但值为空
        m_stringTable[std::move(DString(stringIdTrim.ptr, stringIdTrim.len))] = std::move(DString());
    }
    return true;
}

void LangManager::ClearStringTable(bool bBackup)
{
    m_oldStringTable.clear();
    if (bBackup) {
        m_oldStringTable.swap(m_stringTable);
    }
    else {
        m_stringTable.clear();
    }    
}

void LangManager::RestoreStringTable()
{
    m_oldStringTable.swap(m_stringTable);
}

DString LangManager::GetStringByID(const DString& id) const
{
    DString text;
    if (id.empty()) {
        return text;
    }
    auto it = m_stringTable.find(id);
    if (it == m_stringTable.end()) {
        ASSERT(!"LangManager::GetStringByID failed!");
        return text;
    }
    else {
        text = it->second;
    }
    return text;
}

bool LangManager::HasStringByID(const DString& id) const
{
    auto it = m_stringTable.find(id);
    return it != m_stringTable.end();
}

}//namespace ui 
