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
class UILIB_API LangManager
{
public:
    LangManager();
    ~LangManager();
    LangManager(const LangManager&) = delete;
    LangManager& operator = (const LangManager&) = delete;

public:
    /** 从本地文件加载所有语言映射表
     * @param[in] strFilePath 语言文件的完整路径
     */
    bool LoadStringTable(const FilePath& strFilePath);

    /** 从内存中加载所有语言映射表
     * @param[in] fileData 要加载的语言映射表的数据
     */
    bool LoadStringTable(const std::vector<uint8_t>& fileData);

    /** 清理多语言资源
    */
    void ClearStringTable();

public:
    /** 根据ID获取指定语言的字符串
     * @param[in] id 指定字符串 ID
     * @return 返回 ID 对应的语言字符串
     */
    DString GetStringViaID(const DString& id);

private:
    /** 分析语言映射表内容
     * @param[in] list 读取出来的映射表内容列表
     */
    bool AnalyzeStringTable(const std::vector<DString>& list);

private:
    /** 字符串的ID和取值映射表
    */
    std::unordered_map<DString, DString> m_stringTable;
};

}
#endif //UI_CORE_MULTILANG_H_
