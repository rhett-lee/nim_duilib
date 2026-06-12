#ifndef UI_CORE_MULTILANG_H_
#define UI_CORE_MULTILANG_H_

#include "duilib/Utils/FilePath.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace ui 
{

/** 多语言的支持
*/
class DUILIB_API LangManager
{
public:
    LangManager();
    ~LangManager();
    LangManager(const LangManager&) = delete;
    LangManager& operator = (const LangManager&) = delete;

public:
    /** 从本地文件加载所有语言映射表（文件格式按UTF8编码处理）
     * @param [in] strFilePath 语言文件的完整路径
     */
    bool LoadStringTable(const FilePath& strFilePath);

    /** 从内存中加载所有语言映射表（文件格式按UTF8编码处理）
     * @param [in] fileData 要加载的语言映射表的数据
     */
    bool LoadStringTable(const std::vector<uint8_t>& fileData);

    /** 清理多语言资源
    * @param [in] bBackup true表示备份原有的语言资源，可以使用RestoreStringTable函数恢复; false表示不备份
    */
    void ClearStringTable(bool bBackup);

    /** 恢复已备份的语言资源
    */
    void RestoreStringTable();

public:
    /** 根据ID获取指定语言的字符串
     * @param [in] id 指定字符串 ID
     * @return 返回 ID 对应的语言字符串
     */
    DString GetStringByID(const DString& id) const;

    /** 判断是否包含指定语言ID
    * @param [in] id 指定字符串 ID
    */
    bool HasStringByID(const DString& id) const;

private:
    /** 从内存中加载所有语言映射表
     * @param [in] utf8String 要加载的语言映射表的数据，UTF8格式的字符串，字符串以'\0'结尾
     */
    bool LoadStringTableFromFileData(const char* utf8String);

    /** 从内存中加载所有语言映射表（常规字符串）
     * @param [in] stringData 要加载的语言映射表的数据，常规字符串，字符串以'\0'结尾
     * @param [in] nStringSize 字符串的长度
     */
    bool LoadStringTableFromStringData(const DString::value_type* stringData, int32_t nStringSize = -1);

    /** 解析一行数据
    * @param [in] lineData 本行数据的字符串起始地址
    * @param [in] nLineDataLen 本行数据的长度
    */
    bool LoadStringTableLine(const DString::value_type* lineData, int32_t nLineDataLen);

private:
    /** 字符串的ID和取值映射表
    */
    std::unordered_map<DString, DString> m_stringTable;

    /** 字符串的ID和取值映射表(备份)
    */
    std::unordered_map<DString, DString> m_oldStringTable;
};

}
#endif //UI_CORE_MULTILANG_H_
