#include "duilib/Control/AddressBarPathUtil.h"
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

#define CHECK(condition) Check((condition), []() { return std::string(#condition); })
#define CHECK_EQ(actual, expected) \
    Check((actual) == (expected), [&]() { \
        return std::string(#actual) + ": expected <" + ui::StringConvert::TToMBCS(DString(expected)) \
             + ">, actual <" + ui::StringConvert::TToMBCS(DString(actual)) + ">"; \
    })

/** 断言路径段的显示名、完整路径与根标记
*/
void CheckSegment(const ui::AddressBarPathSegment& segment,
                  const DString& displayName,
                  const DString& fullPath,
                  bool bRoot,
                  bool bDrive)
{
    CHECK_EQ(segment.m_displayName, displayName);
    CHECK_EQ(segment.m_fullPath, fullPath);
    CHECK(segment.m_bRoot == bRoot);
    CHECK(segment.m_bDrive == bDrive);
    CHECK(segment.m_bComputer == false);
}

void TestEmptyPath()
{
    std::vector<ui::AddressBarPathSegment> segments;
    ui::BuildAddressBarPathSegments(_T(""), _T(""), segments);
    CHECK(segments.empty());

    ui::BuildAddressBarPathSegments(_T(""), _T("Computer"), segments);
    CHECK(segments.size() == 1);
    if (segments.size() == 1) {
        CHECK(segments[0].m_bComputer);
        CHECK_EQ(segments[0].m_displayName, _T("Computer"));
    }
}

void TestDrivePath()
{
#ifdef DUILIB_BUILD_FOR_WIN
    std::vector<ui::AddressBarPathSegment> segments;
    ui::BuildAddressBarPathSegments(_T("C:/LLVM/include"), _T("此电脑"), segments);
    CHECK(segments.size() == 3);
    if (segments.size() == 3) {
        //根段的显示名可能是磁盘卷名（如“本地磁盘 (C:)”），只校验类型与完整路径
        CHECK(segments[0].m_bRoot);
        CHECK(segments[0].m_bDrive);
        CHECK(!segments[0].m_displayName.empty());
        CHECK_EQ(segments[0].m_fullPath, _T("C:\\"));
        CheckSegment(segments[1], _T("LLVM"), _T("C:\\LLVM"), false, false);
        CheckSegment(segments[2], _T("include"), _T("C:\\LLVM\\include"), false, false);
    }

    //盘符根
    ui::BuildAddressBarPathSegments(_T("C:\\"), _T("此电脑"), segments);
    CHECK(segments.size() == 1);
    if (segments.size() == 1) {
        CHECK(segments[0].m_bRoot);
        CHECK(segments[0].m_bDrive);
        CHECK_EQ(segments[0].m_fullPath, _T("C:\\"));
    }

    //重复、混用的分隔符与尾部多余分隔符应被规范化
    ui::BuildAddressBarPathSegments(_T("C:\\\\LLVM////include\\\\"), _T("此电脑"), segments);
    CHECK(segments.size() == 3);
    if (segments.size() == 3) {
        CheckSegment(segments[1], _T("LLVM"), _T("C:\\LLVM"), false, false);
        CheckSegment(segments[2], _T("include"), _T("C:\\LLVM\\include"), false, false);
    }
#endif
}

void TestUncPath()
{
#ifdef DUILIB_BUILD_FOR_WIN
    std::vector<ui::AddressBarPathSegment> segments;
    ui::BuildAddressBarPathSegments(_T("\\\\server\\share\\dir"), _T("此电脑"), segments);
    CHECK(segments.size() == 2);
    if (segments.size() == 2) {
        CHECK(segments[0].m_bRoot);
        CHECK(!segments[0].m_bDrive);
        CHECK_EQ(segments[0].m_displayName, _T("\\\\server\\share"));
        CHECK_EQ(segments[0].m_fullPath, _T("\\\\server\\share\\"));
        CheckSegment(segments[1], _T("dir"), _T("\\\\server\\share\\dir"), false, false);
    }

    //扩展长度路径：\\?\C:\...
    ui::BuildAddressBarPathSegments(_T("\\\\?\\C:\\LLVM"), _T("此电脑"), segments);
    CHECK(segments.size() == 2);
    if (segments.size() == 2) {
        CHECK(segments[0].m_bRoot);
        CHECK(segments[0].m_bDrive);
        //盘符根的显示名可能是磁盘卷名（如“本地磁盘 (C:)”）
        CHECK(!segments[0].m_displayName.empty());
        CHECK_EQ(segments[0].m_fullPath, _T("\\\\?\\C:\\"));
        CheckSegment(segments[1], _T("LLVM"), _T("\\\\?\\C:\\LLVM"), false, false);
    }

    //扩展长度UNC路径：\\?\UNC\server\share\...
    ui::BuildAddressBarPathSegments(_T("\\\\?\\UNC\\server\\share\\dir"), _T("此电脑"), segments);
    CHECK(segments.size() == 2);
    if (segments.size() == 2) {
        CHECK(segments[0].m_bRoot);
        CHECK(!segments[0].m_bDrive);
        CHECK_EQ(segments[0].m_displayName, _T("\\\\server\\share"));
        CheckSegment(segments[1], _T("dir"), _T("\\\\?\\UNC\\server\\share\\dir"), false, false);
    }
#endif
}

void TestRelativePath()
{
    std::vector<ui::AddressBarPathSegment> segments;
    ui::BuildAddressBarPathSegments(_T("a\\b\\c"), _T("此电脑"), segments);
    CHECK(segments.size() == 3);
    if (segments.size() == 3) {
        CheckSegment(segments[0], _T("a"), _T("a"), false, false);
        CheckSegment(segments[1], _T("b"), _T("a\\b"), false, false);
        CheckSegment(segments[2], _T("c"), _T("a\\b\\c"), false, false);
    }

    //单段相对路径
    ui::BuildAddressBarPathSegments(_T("folder"), _T("此电脑"), segments);
    CHECK(segments.size() == 1);
    if (segments.size() == 1) {
        CheckSegment(segments[0], _T("folder"), _T("folder"), false, false);
    }
}

void TestExpandUserPath()
{
    //非"~"开头的路径保持不变
    CHECK_EQ(ui::ExpandAddressBarUserPath(_T("C:\\LLVM")), _T("C:\\LLVM"));
    //普通文件名不应被当作主目录
    CHECK_EQ(ui::ExpandAddressBarUserPath(_T("~abc")), _T("~abc"));

    const DString home = ui::ExpandAddressBarUserPath(_T("~"));
    CHECK(!home.empty());
    CHECK(home[0] != _T('~'));
    //"~"后面带分隔符时，展开结果保留后续路径
    CHECK_EQ(ui::ExpandAddressBarUserPath(_T("~/docs")), home + DString(_T("/docs")));
}

}//namespace

int main()
{
    TestEmptyPath();
    TestDrivePath();
    TestUncPath();
    TestRelativePath();
    TestExpandUserPath();

    std::cout << "AddressBar path tests: " << (g_checkCount - g_failedCount)
              << "/" << g_checkCount << " checks passed" << std::endl;
    return g_failedCount == 0 ? 0 : 1;
}
