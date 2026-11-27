// StringUtil::Split 优化前后的性能对比基准
//
// 对比三种实现：
//   1) SplitLegacy：优化前的实现（std::list + strtok，需要整串拷贝）；
//   2) Split：优化后的实现（std::vector + 单遍扫描，不拷贝输入）；
//   3) SplitByString：按完整分隔字符串切分的参考实现。
//
// 覆盖场景：大量短字段、少量长字段、常规配置串、宽字符版本、多字符分隔符（仅SplitByString支持）。
//
// 评测请使用 Release 配置：
//   python build/build.py build --offline --target bench_string_split
//   out/x64/Release/tests/bench_string_split.exe --iterations 1
//
// --iterations N 用于整体放大迭代次数（默认 1），便于在高性能机器上获得更稳定的结果。

#include "duilib/Utils/StringUtil.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace
{
using Clock = std::chrono::steady_clock;

struct BenchResult
{
    double totalMs = 0.0;       // 总耗时（毫秒）
    double usPerOp = 0.0;       // 单次调用耗时（微秒）
    double mbPerSec = 0.0;      // 吞吐量（按输入字节数计算）
    size_t partCount = 0;       // 返回的字段数量（用于结果一致性校验）
};

/** 执行基准：预热后连续调用 iterations 次，重复多轮并取中位数，降低测量噪声
* @param [in] inputBytes 单次调用的输入字节数，用于计算吞吐量
* @param [in] iterations 每轮迭代次数
* @param [in] func 被测函数，返回切分结果的字段数量
*/
template<typename Func>
BenchResult Measure(size_t inputBytes, int iterations, Func func)
{
    const int warmupCount = 3;
    for (int i = 0; i < warmupCount; ++i) {
        const size_t partCount = func();
        (void)partCount;
    }

    constexpr int kRoundCount = 5;
    std::vector<double> roundMs;
    roundMs.reserve(kRoundCount);
    size_t partCount = 0;
    for (int round = 0; round < kRoundCount; ++round) {
        const auto start = Clock::now();
        size_t totalParts = 0;
        for (int i = 0; i < iterations; ++i) {
            totalParts += func();
        }
        const auto finish = Clock::now();
        roundMs.push_back(std::chrono::duration<double, std::milli>(finish - start).count());
        partCount = (iterations > 0) ? (totalParts / static_cast<size_t>(iterations)) : 0;
    }
    std::sort(roundMs.begin(), roundMs.end());

    BenchResult result;
    result.totalMs = roundMs[kRoundCount / 2];
    result.usPerOp = (iterations > 0) ? (result.totalMs * 1000.0 / iterations) : 0.0;
    const double seconds = result.totalMs / 1000.0;
    result.mbPerSec = (seconds > 0.0)
        ? (static_cast<double>(inputBytes) * iterations / seconds / (1024.0 * 1024.0))
        : 0.0;
    result.partCount = partCount;
    return result;
}

/** 用相同的字段和分隔符构造测试字符串
*/
std::string MakeStringFields(const std::string& token, char delimiter, size_t count)
{
    std::string input;
    input.reserve(count * (token.size() + 1));
    for (size_t i = 0; i < count; ++i) {
        input += token;
        if ((i + 1) < count) {
            input.push_back(delimiter);
        }
    }
    return input;
}

/** 用相同的字段和分隔符构造宽字符测试字符串
*/
std::wstring MakeWideFields(const std::wstring& token, wchar_t delimiter, size_t count)
{
    std::wstring input;
    input.reserve(count * (token.size() + 1));
    for (size_t i = 0; i < count; ++i) {
        input += token;
        if ((i + 1) < count) {
            input.push_back(delimiter);
        }
    }
    return input;
}

/** 优化前的Split实现（std::list + strtok），作为"修改前"基线
*/
std::list<std::string> SplitLegacy(const std::string& input, const std::string& delimitor)
{
    std::list<std::string> output;
    std::string input2(input);
    if (input2.empty()) {
        return output;
    }
    char* context = nullptr;
#ifdef DUILIB_BUILD_FOR_WIN
    char* token = strtok_s(input2.data(), delimitor.c_str(), &context);
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

/** 优化前的Split实现（宽字符版本）
*/
std::list<std::wstring> SplitLegacyWide(const std::wstring& input, const std::wstring& delimitor)
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

/** 打印一组对比结果
*/
void PrintCompare(const std::string& caseName, size_t inputBytes,
                  const BenchResult& legacyResult,
                  const BenchResult& currentResult,
                  const BenchResult& byStringResult)
{
    std::cout << "[" << caseName << "] 输入 " << std::fixed << std::setprecision(1)
              << (static_cast<double>(inputBytes) / 1024.0) << " KB，字段数 "
              << currentResult.partCount << std::endl;
    std::cout << "  Split(修改前)  : " << std::setprecision(3) << legacyResult.usPerOp
              << " us/op, " << std::setprecision(1) << legacyResult.mbPerSec << " MB/s" << std::endl;
    std::cout << "  Split(修改后)  : " << std::setprecision(3) << currentResult.usPerOp
              << " us/op, " << std::setprecision(1) << currentResult.mbPerSec << " MB/s" << std::endl;
    if (currentResult.usPerOp > 0.0) {
        std::cout << "  性能提升       : " << std::setprecision(2)
                  << (legacyResult.usPerOp / currentResult.usPerOp) << "x" << std::endl;
    }
    std::cout << "  SplitByString  : " << std::setprecision(3) << byStringResult.usPerOp
              << " us/op, " << std::setprecision(1) << byStringResult.mbPerSec << " MB/s" << std::endl;
    if (legacyResult.partCount != currentResult.partCount) {
        std::cout << "  [WARN] 优化前后的字段数不一致: " << legacyResult.partCount
                  << " vs " << currentResult.partCount << std::endl;
    }
    if (currentResult.partCount != byStringResult.partCount) {
        std::cout << "  [WARN] 与SplitByString的字段数不一致: " << currentResult.partCount
                  << " vs " << byStringResult.partCount << std::endl;
    }
    std::cout << std::endl;
}

/** 单字符分隔符场景：对比优化前后，并以SplitByString作为参考
*/
void BenchOneDelimiter(const std::string& caseName, const std::string& input,
                       int iterations, const std::string& delimiter)
{
    const BenchResult legacyResult = Measure(input.size(), iterations, [&input, &delimiter]() {
        return SplitLegacy(input, delimiter).size();
    });
    const BenchResult currentResult = Measure(input.size(), iterations, [&input, &delimiter]() {
        return ui::StringUtil::Split(input, delimiter).size();
    });
    const BenchResult byStringResult = Measure(input.size(), iterations, [&input, &delimiter]() {
        return ui::StringUtil::SplitByString(input, delimiter, false).size();
    });
    PrintCompare(caseName, input.size(), legacyResult, currentResult, byStringResult);
}

/** 宽字符单字符分隔符场景
*/
void BenchWideOneDelimiter(const std::string& caseName, const std::wstring& input,
                           int iterations, const std::wstring& delimiter)
{
    const size_t inputBytes = input.size() * sizeof(wchar_t);
    const BenchResult legacyResult = Measure(inputBytes, iterations, [&input, &delimiter]() {
        return SplitLegacyWide(input, delimiter).size();
    });
    const BenchResult currentResult = Measure(inputBytes, iterations, [&input, &delimiter]() {
        return ui::StringUtil::Split(input, delimiter).size();
    });
    const BenchResult byStringResult = Measure(inputBytes, iterations, [&input, &delimiter]() {
        return ui::StringUtil::SplitByString(input, delimiter, false).size();
    });
    PrintCompare(caseName, inputBytes, legacyResult, currentResult, byStringResult);
}

/** 多字符分隔符场景：只有 SplitByString 支持
*/
void BenchMultiDelimiter(const std::string& caseName, const std::string& input,
                         int iterations, const std::string& delimiter)
{
    const BenchResult byStringResult = Measure(input.size(), iterations, [&input, &delimiter]() {
        return ui::StringUtil::SplitByString(input, delimiter, false).size();
    });
    std::cout << "[" << caseName << "] 输入 " << std::fixed << std::setprecision(1)
              << (static_cast<double>(input.size()) / 1024.0) << " KB，字段数 "
              << byStringResult.partCount << std::endl;
    std::cout << "  SplitByString : " << std::setprecision(3) << byStringResult.usPerOp
              << " us/op, " << std::setprecision(1) << byStringResult.mbPerSec << " MB/s" << std::endl;
    std::cout << "  (Split 不支持多字符分隔符，未参与对比)" << std::endl << std::endl;
}

/** 解析命令行参数：--iterations N
*/
int GetIterationsScale(int argc, char* argv[])
{
    int scale = 1;
    for (int i = 1; i < argc; ++i) {
        if ((std::string(argv[i]) == "--iterations") && ((i + 1) < argc)) {
            const int value = std::atoi(argv[i + 1]);
            if (value > 0) {
                scale = value;
            }
            ++i;
        }
    }
    return scale;
}

} // namespace

int main(int argc, char* argv[])
{
    const int scale = GetIterationsScale(argc, argv);
    std::cout << "StringUtil Split 性能基准（Release 构建，迭代倍率 " << scale << "）" << std::endl << std::endl;

    // 1) 大量短字段（约120KB，20000个字段，字段可走SSO）
    {
        const std::string input = MakeStringFields(std::string("abcd"), ',', 20000);
        BenchOneDelimiter("大量短字段", input, 50 * scale, std::string(","));
    }

    // 2) 少量长字段（约256KB，2000个字段，每个字段128字节，需要堆分配）
    {
        const std::string input = MakeStringFields(std::string(128, 'x'), ',', 2000);
        BenchOneDelimiter("少量长字段", input, 100 * scale, std::string(","));
    }

    // 3) 常规配置串（字段数很少，反映典型调用场景）
    {
        const std::string input = MakeStringFields(std::string("value"), ',', 10);
        BenchOneDelimiter("常规配置串", input, 20000 * scale, std::string(","));
    }

    // 4) 宽字符版本（约240KB，20000个字段）
    {
        const std::wstring input = MakeWideFields(std::wstring(L"abcd"), L',', 20000);
        BenchWideOneDelimiter("大量短字段(宽字符)", input, 30 * scale, std::wstring(L","));
    }

    // 5) 多字符分隔符（约90KB，5000个字段）
    {
        const std::string input = MakeStringFields(std::string("value"), ',', 5000);
        std::string multiInput;
        multiInput.reserve(input.size() + 5000);
        for (char ch : input) {
            if (ch == ',') {
                multiInput += "||";
            }
            else {
                multiInput.push_back(ch);
            }
        }
        BenchMultiDelimiter("多字符分隔符", multiInput, 100 * scale, std::string("||"));
    }

    return 0;
}
