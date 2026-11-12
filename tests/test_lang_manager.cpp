// LangManager 的单元测试
//
// 语言文件取值的转义规则（单次扫描解析）：
//   "\\" -> 反斜杠，"\r" -> 回车符，"\n" -> 换行符，"\s" -> 空格，
//   其他转义序列（如"\P"、"\t"）保持原样。
// 本测试直接从内存加载语言表，不创建窗口、不依赖全局资源，可在命令行直接运行。

#include <crtdbg.h>

#include "duilib/Core/LangManager.h"

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace
{
int32_t g_failedCount = 0;
int32_t g_checkCount = 0;

void Check(bool condition, const std::function<std::string()>& message)
{
    ++g_checkCount;
    if (!condition) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << message() << std::endl;
    }
}

/** 直接使用常量字符串作为失败信息 */
void Check(bool condition, const char* message)
{
    Check(condition, [message]() { return std::string(message); });
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

/** 比较字符串是否相等，失败时打印用例名 */
void CheckText(const DString& actual, const DString& expected, const char* caseName)
{
    ++g_checkCount;
    if (actual != expected) {
        ++g_failedCount;
        std::cerr << "[FAIL] " << caseName << ": actual size=" << actual.size()
                  << ", expected size=" << expected.size() << std::endl;
    }
}

/** 加载语言表测试数据，返回加载结果 */
bool LoadTestTable(ui::LangManager& langManager)
{
    //注意：字符串字面量中的"C++转义"与"语言文件转义"是两层含义，
    //这里是语言文件（UTF8文本）的原始内容，例如文件中的"\\\\"表示两个反斜杠字符
    const std::string fileData =
        "STRID_LANG_BACKSLASH = C:\\\\Program Files\\\\nim_duilib\\\\bin\\\\label.xml\n"
        "STRID_LANG_NEWLINE = first\\nsecond\n"
        "STRID_LANG_CARRIAGE = a\\rb\n"
        "STRID_LANG_SPACE = \\sleading and trailing\\s\n"
        "STRID_LANG_UNKNOWN = \\P\\t\\d\n"
        "STRID_LANG_DOUBLE_ESCAPE = \\\\n\n"
        "STRID_LANG_UNC = \\\\\\\\server\\\\share\n"
        "STRID_LANG_PLAIN = plain text\n"
        "# 以#开头的是注释行，不会作为语言词条加载\n"
        "; 以;开头的是注释行，不会作为语言词条加载\n"
        "STRID_LANG_AFTER_COMMENT = value\n";

    std::vector<uint8_t> fileDataUtf8(fileData.begin(), fileData.end());
    return langManager.LoadStringTable(fileDataUtf8);
}

void TestEscapeCharacters()
{
    ui::LangManager langManager;
    CHECK(LoadTestTable(langManager));

    //"\\"：转义的反斜杠（Windows路径中的反斜杠需要双写）
    CheckText(langManager.GetStringByID(_T("STRID_LANG_BACKSLASH")),
              _T("C:\\Program Files\\nim_duilib\\bin\\label.xml"),
              "backslash: windows path");

    //"\n"：换行符
    CheckText(langManager.GetStringByID(_T("STRID_LANG_NEWLINE")), _T("first\nsecond"), "newline");

    //"\r"：回车符
    CheckText(langManager.GetStringByID(_T("STRID_LANG_CARRIAGE")), _T("a\rb"), "carriage return");

    //"\s"：空格（字符串两端的空格会被裁剪，所以用"\s"表示需要的空格）
    CheckText(langManager.GetStringByID(_T("STRID_LANG_SPACE")), _T(" leading and trailing "), "space");

    //其他转义序列保持原样，避免影响正常文本（如路径中的"\Program"、"\themes"）
    CheckText(langManager.GetStringByID(_T("STRID_LANG_UNKNOWN")), _T("\\P\\t\\d"), "unknown escape kept");

    //"\\n"：转义的反斜杠 + 字符'n'，不能被解析成换行符（单次扫描）
    CheckText(langManager.GetStringByID(_T("STRID_LANG_DOUBLE_ESCAPE")), _T("\\n"), "escaped backslash + n");

    //UNC路径：文件中的"\\\\"表示两个反斜杠，"\\"表示一个反斜杠
    CheckText(langManager.GetStringByID(_T("STRID_LANG_UNC")), _T("\\\\server\\share"), "unc path");

    //不含转义字符的文本原样加载
    CheckText(langManager.GetStringByID(_T("STRID_LANG_PLAIN")), _T("plain text"), "plain text");

    //注释行被跳过，注释之后的词条正常加载
    CheckText(langManager.GetStringByID(_T("STRID_LANG_AFTER_COMMENT")), _T("value"), "after comment");
}

void TestNotFound()
{
    ui::LangManager langManager;
    CHECK(LoadTestTable(langManager));
    CHECK(!langManager.HasStringByID(_T("STRID_LANG_NOT_EXISTS")));
    CheckText(langManager.GetStringByID(_T("STRID_LANG_NOT_EXISTS")), _T(""), "not exists: empty");
    CheckText(langManager.GetStringByID(_T("")), _T(""), "empty id: empty");
}

/** 覆盖已有词条后，取值按新的转义规则重新解析 */
void TestReload()
{
    ui::LangManager langManager;
    CHECK(LoadTestTable(langManager));

    const std::string fileData = "STRID_LANG_BACKSLASH = D:\\\\data\\\\file.txt\n";
    std::vector<uint8_t> fileDataUtf8(fileData.begin(), fileData.end());
    CHECK(langManager.LoadStringTable(fileDataUtf8));
    CheckText(langManager.GetStringByID(_T("STRID_LANG_BACKSLASH")), _T("D:\\data\\file.txt"), "reload");
}
}

int main()
{
#if defined(_WIN32) && defined(_DEBUG)
    //本测试不启动全局资源，断言只输出到调试器，避免弹出对话框导致测试挂起
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif
    TestEscapeCharacters();
    TestNotFound();
    TestReload();

    std::cout << "lang manager tests: " << g_checkCount << " checks, " << g_failedCount << " failure(s)" << std::endl;
    return (g_failedCount == 0) ? 0 : 1;
}
