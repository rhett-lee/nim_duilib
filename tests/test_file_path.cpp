// duilib/Utils 文件路径与文件读写相关类的单元测试
//
// 覆盖 FilePath（路径解析、拼接、规范化、父子目录判断）、FilePathUtil（拼接、规范化）、
// FileUtil（二进制读写，含空内容）以及 FileTime（FILETIME 往返与格式化）。
// 使用系统临时目录，测试结束后自动清理。

#include "duilib/Utils/FilePath.h"
#include "duilib/Utils/FilePathUtil.h"
#include "duilib/Utils/FileUtil.h"
#include "duilib/Utils/FileTime.h"
#include <cstdint>
#include <filesystem>
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

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })

#define CHECK_INT(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual " == " #expected) + ": expected " + \
               std::to_string(static_cast<int64_t>(expected)) + ", actual " + \
               std::to_string(static_cast<int64_t>(actual)); \
    })

#define CHECK_WSTR(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual " == " #expected); \
    })

void TestFilePathBasic()
{
    const ui::FilePath emptyPath;
    CHECK(emptyPath.IsEmpty());
    CHECK(emptyPath.GetFileName().empty());
    CHECK(emptyPath.GetFileExtension().empty());
    CHECK(emptyPath.ToStringW().empty());
    CHECK_INT(emptyPath.GetFileSize(), 0);
    CHECK(!emptyPath.IsExistsPath());
    CHECK(!emptyPath.IsExistsFile());
    CHECK(!emptyPath.IsExistsDirectory());

#ifdef DUILIB_BUILD_FOR_WIN
    const ui::FilePath filePath(std::wstring(L"C:\\abc\\def.txt"));
    CHECK(!filePath.IsEmpty());
    CHECK(filePath.IsAbsolutePath());
    CHECK(!filePath.IsRelativePath());
    CHECK_WSTR(filePath.GetFileName(), DString(L"def.txt"));
    CHECK_WSTR(filePath.GetFileExtension(), DString(L".txt"));
    CHECK_WSTR(filePath.GetParentPath().ToStringW(), DString(L"C:\\abc"));

    // 无扩展名
    const ui::FilePath noExt(std::wstring(L"C:\\abc\\README"));
    CHECK(noExt.GetFileExtension().empty());

    // 相对路径
    const ui::FilePath relativePath(std::string("abc/def.txt"));
    CHECK(relativePath.IsRelativePath());
    CHECK(!relativePath.IsAbsolutePath());
    CHECK_WSTR(relativePath.GetFileName(), DString(L"def.txt"));
#endif
}

void TestFilePathJoinAndNormalize()
{
#ifdef DUILIB_BUILD_FOR_WIN
    // 路径拼接
    ui::FilePath joinPath(std::wstring(L"C:\\abc"));
    joinPath.JoinFilePath(ui::FilePath(std::wstring(L"def\\test.txt")));
    CHECK_WSTR(joinPath.ToStringW(), DString(L"C:\\abc\\def\\test.txt"));

    ui::FilePath operatorJoin(std::wstring(L"C:\\abc"));
    operatorJoin /= ui::FilePath(std::wstring(L"def"));
    CHECK_WSTR(operatorJoin.ToStringW(), DString(L"C:\\abc\\def"));

    // 按字符串拼接：不自动添加分隔符
    ui::FilePath appendPath(std::wstring(L"C:\\abc\\"));
    appendPath += ui::FilePath(std::wstring(L"def"));
    CHECK_WSTR(appendPath.ToStringW(), DString(L"C:\\abc\\def"));

    // 规范化：去除"."和".."，并统一分隔符为反斜杠
    ui::FilePath normalizePath(std::wstring(L"C:\\abc\\.\\def\\..\\test.txt"));
    normalizePath.NormalizeFilePath();
    CHECK_WSTR(normalizePath.ToStringW(), DString(L"C:\\abc\\test.txt"));

    // 目录规范化：结尾添加分隔符
    ui::FilePath dirPath(std::wstring(L"C:\\abc\\def"));
    dirPath.NormalizeDirectoryPath();
    CHECK_WSTR(dirPath.ToStringW(), DString(L"C:\\abc\\def\\"));

    // 相对路径：将"/"替换为"\\"
    ui::FilePath relativePath(std::wstring(L"abc/def"));
    relativePath.NormalizeFilePath();
    CHECK_WSTR(relativePath.ToStringW(), DString(L"abc\\def"));
#endif
}

void TestFilePathSeparators()
{
#ifdef DUILIB_BUILD_FOR_WIN
    // 去除结尾分隔符
    ui::FilePath path(std::wstring(L"C:\\abc\\def\\"));
    path.TrimRightPathSeparator();
    CHECK_WSTR(path.ToStringW(), DString(L"C:\\abc\\def"));

    // 根目录不应被裁剪成盘符相对路径
    ui::FilePath rootPath(std::wstring(L"C:\\"));
    rootPath.TrimRightPathSeparator();
    CHECK_WSTR(rootPath.ToStringW(), DString(L"C:\\"));

    // 确保以分隔符结尾
    ui::FilePath dir(std::wstring(L"C:\\abc"));
    dir.FormatPathAsDirectory();
    CHECK_WSTR(dir.ToStringW(), DString(L"C:\\abc\\"));

    // 已经是目录形式时不应重复添加分隔符
    dir.FormatPathAsDirectory();
    CHECK_WSTR(dir.ToStringW(), DString(L"C:\\abc\\"));

    // 删除文件名，保留目录
    ui::FilePath withFile(std::wstring(L"C:\\abc\\def.txt"));
    withFile.RemoveFileName();
    CHECK_WSTR(withFile.ToStringW(), DString(L"C:\\abc\\"));
#endif
}

void TestFilePathSubDirectory()
{
#ifdef DUILIB_BUILD_FOR_WIN
    const ui::FilePath parent(std::wstring(L"C:\\abc"));
    CHECK(ui::FilePath(std::wstring(L"C:\\abc\\def")).IsSubDirectory(parent));
    CHECK(ui::FilePath(std::wstring(L"C:\\abc\\def\\ghi")).IsSubDirectory(parent));
    CHECK(!ui::FilePath(std::wstring(L"C:\\abcd")).IsSubDirectory(parent));
    CHECK(!parent.IsSubDirectory(parent));
    CHECK(!ui::FilePath().IsSubDirectory(parent));
    CHECK(!ui::FilePath(std::wstring(L"C:\\abc\\def")).IsSubDirectory(ui::FilePath()));

    // 父目录列表（从根目录开始）
    std::vector<ui::FilePath> parentList;
    ui::FilePath(std::wstring(L"C:\\abc\\def\\test.txt")).GetParentPathList(parentList);
    CHECK_INT((int)parentList.size(), 3);
    if (parentList.size() == 3) {
        CHECK_WSTR(parentList[0].ToStringW(), DString(L"C:\\"));
        CHECK_WSTR(parentList[1].ToStringW(), DString(L"C:\\abc"));
        CHECK_WSTR(parentList[2].ToStringW(), DString(L"C:\\abc\\def"));
    }

    // 路径比较与Hash：对规范化后的绝对路径使用
    ui::FilePath path1(std::wstring(L"C:\\abc\\def.txt"));
    ui::FilePath path2(std::wstring(L"C:\\abc\\def.txt"));
    CHECK(path1 == path2);
    CHECK(!(path1 != path2));
    CHECK_INT((int)(path1.HashValue() == path2.HashValue()), 1);
#endif
}

void TestFilePathUtil()
{
#ifdef DUILIB_BUILD_FOR_WIN
    // 拼接路径
    const ui::FilePath joinPath = ui::FilePathUtil::JoinFilePath(
        ui::FilePath(std::wstring(L"C:\\abc")), ui::FilePath(std::wstring(L"def\\test.txt")));
    CHECK_WSTR(joinPath.ToStringW(), DString(L"C:\\abc\\def\\test.txt"));

    // 字符串形式的规范化
    CHECK_WSTR(ui::FilePathUtil::NormalizeFilePath(DString(L"C:\\abc\\.\\def\\..\\test.txt")),
               DString(L"C:\\abc\\test.txt"));

    // 相对路径的规范化
    CHECK_WSTR(ui::FilePathUtil::NormalizeFilePath(DString(L"abc/./def")), DString(L"abc\\def"));

    // 扩展名
    CHECK_WSTR(ui::FilePathUtil::GetFileExtension(DString(L"C:\\abc\\def.xml")), DString(L"XML"));
    CHECK(ui::FilePathUtil::GetFileExtension(DString(L"C:\\abc\\def")).empty());
    // 目录名中的点不应被当作扩展名
    CHECK(ui::FilePathUtil::GetFileExtension(DString(L"C:\\a.b\\def")).empty());

    // 程序所在目录不应为空，且为绝对路径
    const ui::FilePath moduleDir = ui::FilePathUtil::GetCurrentModuleDirectory();
    CHECK(!moduleDir.IsEmpty());
    CHECK(moduleDir.IsAbsolutePath());
#endif
}

void TestFileReadWrite()
{
    // 在系统临时目录下创建独立的测试目录
    std::error_code ec;
    const std::filesystem::path tempDir =
        std::filesystem::temp_directory_path(ec) / "duilib_utils_test";
    if (ec) {
        std::cerr << "[SKIP] 无法获取临时目录: " << ec.message() << std::endl;
        return;
    }
    std::filesystem::remove_all(tempDir, ec);
    std::filesystem::create_directories(tempDir, ec);
    if (ec) {
        std::cerr << "[SKIP] 无法创建临时目录: " << ec.message() << std::endl;
        return;
    }
#ifdef DUILIB_BUILD_FOR_WIN
    const ui::FilePath testFile(std::wstring(tempDir.wstring() + L"\\test.bin"));
#else
    const ui::FilePath testFile(std::string(tempDir.string() + "/test.bin"));
#endif

    // 写入普通内容
    const std::string content = "hello duilib";
    CHECK(ui::FileUtil::WriteFileData(testFile, DStringA(content)));
    CHECK(testFile.IsExistsFile());
    CHECK(!testFile.IsExistsDirectory());
    CHECK_INT(testFile.GetFileSize(), content.size());

    // 读取全部内容
    std::vector<uint8_t> fileData;
    CHECK(ui::FileUtil::ReadFileData(testFile, fileData));
    CHECK_INT((int)fileData.size(), content.size());
    CHECK(std::string(fileData.begin(), fileData.end()) == content);

    // 只读取头部若干字节
    std::vector<uint8_t> headerData;
    CHECK(ui::FileUtil::ReadFileHeaderData(testFile, 5, headerData));
    CHECK_INT((int)headerData.size(), 5);
    CHECK(std::string(headerData.begin(), headerData.end()) == "hello");

    // 空内容写入：文件被创建/截断，应视为成功
    CHECK(ui::FileUtil::WriteFileData(testFile, DStringA()));
    CHECK(testFile.IsExistsFile());
    CHECK_INT(testFile.GetFileSize(), 0);

    // 读取不存在的文件应失败
    std::vector<uint8_t> missingData;
#ifdef DUILIB_BUILD_FOR_WIN
    const ui::FilePath missingFile(std::wstring(tempDir.wstring() + L"\\not_exists.bin"));
#else
    const ui::FilePath missingFile(std::string(tempDir.string() + "/not_exists.bin"));
#endif
    CHECK(!ui::FileUtil::ReadFileData(missingFile, missingData));
    CHECK(missingData.empty());

    // 清理
    std::filesystem::remove_all(tempDir, ec);
    CHECK(!std::filesystem::exists(tempDir));
}

#ifdef DUILIB_BUILD_FOR_WIN
void TestFileTime()
{
    // 构造一个确定的UTC时间：2020-05-06 07:08:09
    SYSTEMTIME systemTime = { 0 };
    systemTime.wYear = 2020;
    systemTime.wMonth = 5;
    systemTime.wDay = 6;
    systemTime.wHour = 7;
    systemTime.wMinute = 8;
    systemTime.wSecond = 9;
    FILETIME fileTimeValue = { 0 };
    CHECK(::SystemTimeToFileTime(&systemTime, &fileTimeValue) != FALSE);

    ui::FileTime fileTime;
    fileTime.FromFileTime(fileTimeValue);
    const uint64_t expectedValue =
        (static_cast<uint64_t>(fileTimeValue.dwHighDateTime) << 32) | fileTimeValue.dwLowDateTime;
    CHECK(fileTime.GetValue() == expectedValue);

    // 往返转换
    const FILETIME roundTrip = fileTime.ToFileTime();
    CHECK(roundTrip.dwLowDateTime == fileTimeValue.dwLowDateTime);
    CHECK(roundTrip.dwHighDateTime == fileTimeValue.dwHighDateTime);

    // 字符串格式："YYYY-MM-DD HH:MM:SS"
    const DString text = fileTime.ToString();
    CHECK_INT((int)text.size(), 19);
    if (text.size() == 19) {
        CHECK((text[4] == L'-') && (text[7] == L'-'));
        CHECK(text[10] == L' ');
        CHECK((text[13] == L':') && (text[16] == L':'));
        // 本地时区不会改变日期所在的年份（仅做基本范围检查）
        CHECK((text[0] == L'2') && (text[1] == L'0'));
    }

    // 默认值(0)也应能安全格式化，不出现未初始化数据
    const ui::FileTime zeroTime;
    CHECK_INT((int)zeroTime.GetValue(), 0);
    CHECK_INT((int)zeroTime.ToString().size(), 19);
}
#endif

} // namespace

int main()
{
    TestFilePathBasic();
    TestFilePathJoinAndNormalize();
    TestFilePathSeparators();
    TestFilePathSubDirectory();
    TestFilePathUtil();
    TestFileReadWrite();
#ifdef DUILIB_BUILD_FOR_WIN
    TestFileTime();
#endif

    if (g_failedCount == 0) {
        std::cout << "All tests passed (" << g_checkCount << " checks)." << std::endl;
        return 0;
    }
    std::cerr << g_failedCount << " of " << g_checkCount << " checks failed." << std::endl;
    return 1;
}
