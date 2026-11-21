// UrlBar 地址栏控件单元测试
//
// 测试对象（不创建窗口、不加载主题资源、不启动浏览器运行时）：
//  1. UrlBarUtil 纯函数：协议解析、主机名解析、网址判定、file:// URL 生成、
//     百分号编码、输入归一化（补全协议、本地路径、搜索模板）；
//  2. UrlBar 控件在未初始化（无窗口）状态下的默认行为：
//     控件类型、网址读写、安全状态、搜索模板、历史记录管理、参数边界处理。
//
// 说明：UrlBar 的子控件在 OnInit（加入窗口）时才创建，因此本测试只覆盖
// 不依赖窗口的接口；带窗口的交互行为由示例程序进行人工验证。

#if defined(_WIN32)
#include <crtdbg.h>
#endif

#include "duilib/Control/UrlBar.h"
#include "duilib/Control/UrlBarUtil.h"
#include "duilib/Utils/StringConvert.h"

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

/** 将 DString 转换为 std::string，用于输出断言信息
*/
std::string ToDisplayString(const DString& text)
{
#ifdef DUILIB_BUILD_FOR_WIN
    return ui::StringConvert::TToMBCS(text);
#else
    return text;
#endif
}

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })
#define CHECK_EQ(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual) + ": expected <" + ToDisplayString(DString(expected)) \
             + ">, actual <" + ToDisplayString(DString(actual)) + ">"; \
    })
#define CHECK_INT(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual) + ": expected " + std::to_string(static_cast<int64_t>(expected)) \
             + ", actual " + std::to_string(static_cast<int64_t>(actual)); \
    })

void TestGetScheme()
{
    CHECK_EQ(ui::UrlGetScheme(_T("http://example.com")), _T("http"));
    CHECK_EQ(ui::UrlGetScheme(_T("HTTPS://example.com")), _T("https"));
    CHECK_EQ(ui::UrlGetScheme(_T("file:///C:/dir/file.txt")), _T("file"));
    CHECK_EQ(ui::UrlGetScheme(_T("about:blank")), _T("about"));
    CHECK_EQ(ui::UrlGetScheme(_T("mailto:user@example.com")), _T("mailto"));
    CHECK_EQ(ui::UrlGetScheme(_T("data:text/plain,abc")), _T("data"));
    //协议名前后空白应被忽略
    CHECK_EQ(ui::UrlGetScheme(_T("  https://example.com  ")), _T("https"));
    //Windows盘符不是协议
    CHECK_EQ(ui::UrlGetScheme(_T("C:\\dir\\file.txt")), _T(""));
    CHECK_EQ(ui::UrlGetScheme(_T("C:/dir/file.txt")), _T(""));
    //没有协议
    CHECK_EQ(ui::UrlGetScheme(_T("example.com")), _T(""));
    CHECK_EQ(ui::UrlGetScheme(_T("")), _T(""));
    //协议名不合法：以数字开头、长度不足、包含非法字符
    CHECK_EQ(ui::UrlGetScheme(_T("1http://example.com")), _T(""));
    CHECK_EQ(ui::UrlGetScheme(_T("a:b")), _T(""));
    CHECK_EQ(ui::UrlGetScheme(_T("ht~tp:x")), _T(""));
    CHECK_EQ(ui::UrlGetScheme(_T("ht tp:x")), _T(""));

    CHECK(ui::UrlHasScheme(_T("https://example.com")));
    CHECK(ui::UrlHasScheme(_T("mailto:user@example.com")));
    CHECK(!ui::UrlHasScheme(_T("C:\\dir")));
    CHECK(!ui::UrlHasScheme(_T("example.com")));
    CHECK(!ui::UrlHasScheme(_T("")));
}

void TestGetHost()
{
    CHECK_EQ(ui::UrlGetHost(_T("https://Example.COM:8080/path")), _T("example.com"));
    CHECK_EQ(ui::UrlGetHost(_T("https://user:pass@www.example.com/x")), _T("www.example.com"));
    CHECK_EQ(ui::UrlGetHost(_T("https://example.com")), _T("example.com"));
    CHECK_EQ(ui::UrlGetHost(_T("https://example.com?q=1")), _T("example.com"));
    CHECK_EQ(ui::UrlGetHost(_T("https://example.com#fragment")), _T("example.com"));
    CHECK_EQ(ui::UrlGetHost(_T("http://[::1]:8080/index.html")), _T("[::1]"));
    //file:/// 没有主机名
    CHECK_EQ(ui::UrlGetHost(_T("file:///C:/dir/file.txt")), _T(""));
    //没有"://"的协议无法解析主机名
    CHECK_EQ(ui::UrlGetHost(_T("about:blank")), _T(""));
    CHECK_EQ(ui::UrlGetHost(_T("example.com")), _T(""));
    CHECK_EQ(ui::UrlGetHost(_T("")), _T(""));
}

void TestIsLikelyUrl()
{
    CHECK(ui::UrlIsLikelyUrl(_T("example.com")));
    CHECK(ui::UrlIsLikelyUrl(_T("www.example.com/path?q=1")));
    CHECK(ui::UrlIsLikelyUrl(_T("localhost:8080")));
    CHECK(ui::UrlIsLikelyUrl(_T("localhost")));
    CHECK(ui::UrlIsLikelyUrl(_T("127.0.0.1")));
    CHECK(ui::UrlIsLikelyUrl(_T("192.168.1.1:8080")));
    CHECK(ui::UrlIsLikelyUrl(_T("[::1]:8080")));
    CHECK(ui::UrlIsLikelyUrl(_T("//example.com/path")));
    CHECK(ui::UrlIsLikelyUrl(_T("server:8080")));

    CHECK(!ui::UrlIsLikelyUrl(_T("")));
    CHECK(!ui::UrlIsLikelyUrl(_T("   ")));
    CHECK(!ui::UrlIsLikelyUrl(_T("hello")));
    CHECK(!ui::UrlIsLikelyUrl(_T("hello world")));
    CHECK(!ui::UrlIsLikelyUrl(_T("hello\tworld")));
    CHECK(!ui::UrlIsLikelyUrl(_T("/home/user/file.txt")));
    CHECK(!ui::UrlIsLikelyUrl(_T("C:\\dir\\file.txt")));
}

void TestMakeFileUrl()
{
#ifdef DUILIB_BUILD_FOR_WIN
    CHECK_EQ(ui::UrlMakeFileUrl(_T("C:\\dir\\file.txt")), _T("file:///C:/dir/file.txt"));
    CHECK_EQ(ui::UrlMakeFileUrl(_T("C:/dir/file.txt")), _T("file:///C:/dir/file.txt"));
    CHECK_EQ(ui::UrlMakeFileUrl(_T("C:\\dir\\")), _T("file:///C:/dir/"));
    CHECK_EQ(ui::UrlMakeFileUrl(_T("\\\\server\\share\\dir\\file.txt")), _T("file://server/share/dir/file.txt"));
#else
    CHECK_EQ(ui::UrlMakeFileUrl(_T("/home/user/file.txt")), _T("file:///home/user/file.txt"));
#endif
}

void TestPercentEncode()
{
    //保留字符
    CHECK_EQ(ui::UrlPercentEncode(_T("abcXYZ0189-_.~")), _T("abcXYZ0189-_.~"));
    //空格编码为%20
    CHECK_EQ(ui::UrlPercentEncode(_T("a b")), _T("a%20b"));
    //保留字符之外的符号
    CHECK_EQ(ui::UrlPercentEncode(_T("a/b?c=d&e")), _T("a%2Fb%3Fc%3Dd%26e"));
    //UTF-8多字节字符：中文 → 每个字节百分号编码
    CHECK_EQ(ui::UrlPercentEncode(_T("中文")), _T("%E4%B8%AD%E6%96%87"));
    //空字符串
    CHECK_EQ(ui::UrlPercentEncode(_T("")), _T(""));
}

void TestNormalizeInput()
{
    //空输入
    CHECK_EQ(ui::UrlNormalizeInput(_T(""), _T("")), _T(""));
    CHECK_EQ(ui::UrlNormalizeInput(_T("   "), _T("")), _T(""));

    //已有协议：原样返回（仅去首尾空白）
    CHECK_EQ(ui::UrlNormalizeInput(_T("https://Example.com/a"), _T("")), _T("https://Example.com/a"));
    CHECK_EQ(ui::UrlNormalizeInput(_T("  about:blank  "), _T("")), _T("about:blank"));
    CHECK_EQ(ui::UrlNormalizeInput(_T("mailto:user@example.com"), _T("")), _T("mailto:user@example.com"));
    CHECK_EQ(ui::UrlNormalizeInput(_T("file:///C:/dir/file.txt"), _T("")), _T("file:///C:/dir/file.txt"));

    //协议相对地址：补全https
    CHECK_EQ(ui::UrlNormalizeInput(_T("//example.com/a"), _T("")), _T("https://example.com/a"));

    //形如网址：补全https
    CHECK_EQ(ui::UrlNormalizeInput(_T("example.com"), _T("")), _T("https://example.com"));
    CHECK_EQ(ui::UrlNormalizeInput(_T("example.com/path?q=1"), _T("")), _T("https://example.com/path?q=1"));
    CHECK_EQ(ui::UrlNormalizeInput(_T("localhost:8080"), _T("")), _T("https://localhost:8080"));
    CHECK_EQ(ui::UrlNormalizeInput(_T("example.com:8080/path"), _T("")), _T("https://example.com:8080/path"));

    //含空白且没有搜索模板：保持原样
    CHECK_EQ(ui::UrlNormalizeInput(_T("hello world"), _T("")), _T("hello world"));

    //搜索模板：百分号编码后替换%s
    CHECK_EQ(ui::UrlNormalizeInput(_T("hello world"), _T("https://www.bing.com/search?q=%s")),
             _T("https://www.bing.com/search?q=hello%20world"));
    CHECK_EQ(ui::UrlNormalizeInput(_T("中文"), _T("https://www.bing.com/search?q=%s")),
             _T("https://www.bing.com/search?q=%E4%B8%AD%E6%96%87"));
    //模板中没有%s时忽略模板
    CHECK_EQ(ui::UrlNormalizeInput(_T("hello world"), _T("https://www.bing.com/search")),
             _T("hello world"));

    //单个词且没有搜索模板：补全https
    CHECK_EQ(ui::UrlNormalizeInput(_T("hello"), _T("")), _T("https://hello"));

#ifdef DUILIB_BUILD_FOR_WIN
    //本地绝对路径：转换为file:// URL
    CHECK_EQ(ui::UrlNormalizeInput(_T("C:\\dir\\file.txt"), _T("")), _T("file:///C:/dir/file.txt"));
    CHECK_EQ(ui::UrlNormalizeInput(_T("\\\\server\\share\\dir"), _T("")), _T("file://server/share/dir"));
#endif
}

void TestUrlBarDefaults()
{
    ui::UrlBar urlBar(nullptr);

    //控件类型
    CHECK_EQ(urlBar.GetType(), DUI_CTR_URL_BAR);
    //未加入窗口时，内部编辑框尚未创建
    CHECK(urlBar.GetEditControl() == nullptr);

    //默认状态
    CHECK_EQ(urlBar.GetUrl(), _T(""));
    CHECK_EQ(urlBar.GetEditText(), _T(""));
    CHECK_EQ(urlBar.GetSearchUrlTemplate(), _T(""));
    CHECK(!urlBar.IsSecure());
    CHECK(urlBar.IsHistoryEnabled());
    CHECK_INT(urlBar.GetMaxHistory(), 20);
    CHECK_INT(urlBar.GetMaxVisible(), 8);
    CHECK_EQ(urlBar.GetHistoryFile(), _T(""));
    CHECK(urlBar.GetHistory().empty());

    //收藏按钮与清除按钮的开关：默认开启，可通过接口关闭
    CHECK(urlBar.IsFavoriteButtonEnabled());
    CHECK(urlBar.IsClearButtonEnabled());
    urlBar.SetFavoriteButtonEnabled(false);
    CHECK(!urlBar.IsFavoriteButtonEnabled());
    urlBar.SetFavoriteButtonEnabled(true);
    CHECK(urlBar.IsFavoriteButtonEnabled());
    urlBar.SetClearButtonEnabled(false);
    CHECK(!urlBar.IsClearButtonEnabled());
    urlBar.SetClearButtonEnabled(true);
    CHECK(urlBar.IsClearButtonEnabled());

    //网址读写：未初始化时缓存在内部，并同步安全状态
    urlBar.SetUrl(_T("https://example.com"));
    CHECK_EQ(urlBar.GetUrl(), _T("https://example.com"));
    CHECK_EQ(urlBar.GetEditText(), _T("https://example.com"));
    CHECK(urlBar.IsSecure());
    urlBar.SetUrl(_T("http://example.com"));
    CHECK_EQ(urlBar.GetUrl(), _T("http://example.com"));
    CHECK(!urlBar.IsSecure());

    //安全状态可直接设置
    urlBar.SetSecure(true);
    CHECK(urlBar.IsSecure());
    urlBar.SetSecure(false);
    CHECK(!urlBar.IsSecure());

    //搜索模板
    urlBar.SetSearchUrlTemplate(_T("https://www.bing.com/search?q=%s"));
    CHECK_EQ(urlBar.GetSearchUrlTemplate(), _T("https://www.bing.com/search?q=%s"));
    urlBar.SetSearchUrlTemplate(_T(""));
    CHECK_EQ(urlBar.GetSearchUrlTemplate(), _T(""));

    //未初始化时提交输入不产生任何效果，也不应崩溃
    urlBar.Submit();
    CHECK(true);
}

void TestUrlBarHistory()
{
    ui::UrlBar urlBar(nullptr);
    CHECK(urlBar.IsHistoryEnabled());

    //MRU顺序：最近添加的排在最前
    urlBar.AddHistory(_T("https://b.com"));
    urlBar.AddHistory(_T("https://a.com"));
    urlBar.AddHistory(_T("https://b.com"));
    std::vector<DString> history = urlBar.GetHistory();
    CHECK(history.size() == 2);
    if (history.size() == 2) {
        CHECK_EQ(history[0], _T("https://b.com"));
        CHECK_EQ(history[1], _T("https://a.com"));
    }

    //空记录应被忽略
    urlBar.AddHistory(_T(""));
    urlBar.AddHistory(_T("   "));
    CHECK(urlBar.GetHistory().size() == 2);

    //历史条数上限
    urlBar.SetMaxHistory(1);
    CHECK_INT(urlBar.GetMaxHistory(), 1);
    history = urlBar.GetHistory();
    CHECK(history.size() == 1);
    if (history.size() == 1) {
        CHECK_EQ(history[0], _T("https://b.com"));
    }

    //边界值处理
    urlBar.SetMaxHistory(0);
    CHECK_INT(urlBar.GetMaxHistory(), 1);
    urlBar.SetMaxHistory(5000);
    CHECK_INT(urlBar.GetMaxHistory(), 1000);
    urlBar.SetMaxHistory(20);
    CHECK_INT(urlBar.GetMaxHistory(), 20);

    //联想下拉可见行数
    urlBar.SetMaxVisible(0);
    CHECK_INT(urlBar.GetMaxVisible(), 1);
    urlBar.SetMaxVisible(1000);
    CHECK_INT(urlBar.GetMaxVisible(), 100);
    urlBar.SetMaxVisible(8);
    CHECK_INT(urlBar.GetMaxVisible(), 8);

    //清空历史
    urlBar.ClearHistory();
    CHECK(urlBar.GetHistory().empty());

    //禁用历史后不再记录
    urlBar.SetHistoryEnabled(false);
    CHECK(!urlBar.IsHistoryEnabled());
    urlBar.AddHistory(_T("https://c.com"));
    CHECK(urlBar.GetHistory().empty());

    //历史持久化文件（仅校验属性读写，不写磁盘）
    urlBar.SetHistoryFile(_T("user/url_bar_test_history.txt"));
    CHECK_EQ(urlBar.GetHistoryFile(), _T("user/url_bar_test_history.txt"));

    //切换为空文件
    urlBar.SetHistoryFile(_T(""));
    CHECK_EQ(urlBar.GetHistoryFile(), _T(""));
}

}//namespace

int main()
{
#ifdef _WIN32
    //未初始化时不应触发调试断言（MFALLOC/CRT断言输出会干扰测试结果）
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif

    TestGetScheme();
    TestGetHost();
    TestIsLikelyUrl();
    TestMakeFileUrl();
    TestPercentEncode();
    TestNormalizeInput();
    TestUrlBarDefaults();
    TestUrlBarHistory();

    std::cout << "UrlBar tests: " << (g_checkCount - g_failedCount)
              << "/" << g_checkCount << " checks passed" << std::endl;
    return g_failedCount == 0 ? 0 : 1;
}
