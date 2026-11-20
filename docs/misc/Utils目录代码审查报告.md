# duilib/Utils 目录代码审查报告

## 1. 审查范围

- 目录：`duilib/Utils`
- 平台重点：Windows（Debug x64，`DUILIB_DLL=OFF` 静态库模式），同时关注跨平台分支的明显问题
- 审查方式：逐文件静态审查 + 关键纯逻辑的单元测试验证
- 修改原则：只做必要的小范围修复，不调整对外接口、不重构架构；需要调整约定或影响范围较大的问题仅记录在案

本次覆盖的文件：

| 分类 | 文件 |
| --- | --- |
| 字符串/编码 | `StringUtil.*`、`StringConvert.*`、`StringCharset.*` |
| 路径/文件 | `FilePath.*`、`FilePathUtil.*`、`FileUtil.*`、`FileTime.*` |
| 属性/配置 | `AttributeUtil.*` |
| 系统能力 | `SystemUtil_*`、`DiskUtils_Windows.*`、`DllManager_Windows.*`、`MonitorUtil_*`、`Clipboard_*`、`PerformanceUtil.*`、`LogUtil.*` |
| 窗口辅助 | `WinImplBase.*`、`Dialog.*`、`MessageBoxWnd.*`、`ToastWnd.*`、`TrayIcon*`、`FileDialog*`、`SystemNotification*` |
| 进程/平台 | `ProcessSingleton*`、`ScreenCapture*`、`BitmapHel*`、`ApiWrapper_Windows.*`、`InlineHook_Windows.*`、`Macros_Windows.h` |

## 2. 审查结论

整体代码结构清晰，平台差异通过 `*_Windows.cpp` / `*_SDL.cpp` / `*_Linux.cpp` 等文件隔离，错误处理大多完整。本次发现的问题主要集中在三类：

1. **MSVC 安全 CRT 的截断行为**：`*_s` 系列函数在缓冲区不足时会触发无效参数处理器，调试版弹出断言对话框、发布版可能直接终止进程。这类问题在字符串复制与格式化路径上确实存在。
2. **边界与返回值语义**：扩容型字符串替换的替换计数、空内容写文件、空指针比较、盘符相对路径解析等。
3. **防御性初始化**：文件时间转换为本地时间失败时读取了未初始化的 `SYSTEMTIME`。

对确认的低风险缺陷已做最小修复（见第 3 节），其余需要评估调用方约定或影响范围较大的项仅记录建议（见第 4 节）。

## 3. 已修复问题

### 3.1 `StringUtil::Printf` 宽字符格式化超长时触发断言/终止（高）

- 文件：`duilib/Utils/StringUtil.cpp`（`vsnprintfT(wchar_t*)`）
- 问题：Windows 下使用 `_vsnwprintf_s`，当结果超过 1023 个字符发生截断时，会触发 CRT 无效参数处理器。调试版表现为进程卡在断言对话框（单元测试首次运行时即复现），发布版可能直接终止进程。
- 修复：先用 `_vscwprintf` 计算所需长度；缓冲区足够时正常格式化，不足时返回所需长度，由 `StringAppendVT` 已有的扩容逻辑重试。行为与 char 版 C99 语义保持一致。

### 3.2 `StringUtil::ReplaceAll` 扩容路径返回值重复计数（中）

- 文件：`duilib/Utils/StringUtil.cpp`（`StringReplaceAllT`）
- 问题：`replace` 比 `find` 长时采用两遍扫描，第一遍统计次数后未清零，第二遍又累加一次，导致返回值约为实际替换数的 2 倍（例如 `"a-a-a"` 替换 `a → abc` 返回 6 而不是 3）。
- 修复：进入第二遍前重置计数器。

### 3.3 `StringUtil::StringCopy` / `StringNCopy` 超长源串导致进程终止（高）

- 文件：`duilib/Utils/StringUtil.cpp`
- 问题：Windows 下使用 `wcscpy_s` / `wcsncpy_s` / `strcpy_s` / `strncpy_s`。不少调用方把用户可控文本写入系统结构体的定长字符数组（如托盘图标提示文本），源串超长时 `_s` 函数会触发无效参数处理器；而非 Windows 分支是安全截断，两个平台行为不一致。
- 修复：所有平台统一按 `destSize - 1` 截断并补 `'\0'`；`StringNCopy` 同时受 `srcSize` 与源串长度约束，避免越界读取。

### 3.4 `StringUtil::IsEqualNoCase` 单空指针返回“相等”（中）

- 文件：`duilib/Utils/StringUtil.cpp`
- 问题：`IsEqualNoCase(const wchar_t*, const wchar_t*)` 与 char 版本在 `lhs` 非空、`rhs == nullptr` 时返回 `true`，与其它重载（字符串 vs 空指针返回 `false`）语义矛盾。
- 修复：仅当两个指针都为 `nullptr` 时才视为相等。

### 3.5 `FileTime::ToString` 使用未初始化时间结构（中）

- 文件：`duilib/Utils/FileTime.cpp`
- 问题：`SYSTEMTIME stUTC/stLocal` 未初始化，`FileTimeToSystemTime` / `SystemTimeToTzSpecificLocalTime` 失败时仍读取其字段（默认构造的 `FileTime` 值为 0，属于会被实际触达的路径）；非 Windows 分支未判断 `localtime_r` 返回值。
- 修复：结构体零初始化并检查返回值；转换失败时返回空字符串（本地时区转换失败则回退为 UTC）。

### 3.6 `DiskUtils::GetDriveFromDirectoryPath` 盘符相对路径解析错误（中）

- 文件：`duilib/Utils/DiskUtils_Windows.cpp`
- 问题：固定取前三个字符，`"C:"` 返回 `"C:"`，`"C:abc"` 返回 `"C:a"`，都不是合法盘符，导致后续 `GetVolumeInformationW` 等调用失败。
- 修复：第三个字符为分隔符时取前三字符；否则返回 `"C:\\"`，保证结果是盘符根路径。

### 3.7 `FileUtil::WriteFileData` 空内容返回失败（低）

- 文件：`duilib/Utils/FileUtil.cpp`（三个重载）
- 问题：文件已成功创建/截断，但内容为空时 `isWriteOk` 保持 `false`，语义上不成立。
- 修复：文件成功打开即视为成功，只有实际写入长度不符时才返回失败。

### 3.8 `StringConvert` 转换失败判断无效（低）

- 文件：`duilib/Utils/StringConvert.cpp`
- 问题：`MultiByteToWideChar` / `WideCharToMultiByte` 失败返回 0，代码判断的是 `length < 0`，条件永远不成立。
- 修复：改为 `length <= 0`，失败时按空字符串处理。

## 4. 未修改项与后续建议

以下问题经复核确实存在，但修复需要调整对外约定、跨平台行为或涉及较大范围，本轮仅记录：

1. **`StringCharset::GetDataAsString`（UTF-16BE）**：`wchar_t` 为 32 位的平台只做字节序反转，未合并代理对，非 BMP 字符会得到错误的码点。建议按 `WCHAR_T_IS_UTF16` 分支处理。
2. **`StringCharset::GetDataCharsetByBOM`**：`FF FE 00 00` 且长度恰为 4 时返回 `UTF16_LE`，属于兼容性特例；UTF-32LE 空文件会被当作一个 `U+0000`。建议后续明确 UTF-32 支持策略。
3. **无 BOM 的 UTF-16 纯 ASCII 数据**：字节全部小于 0x80，会被识别为 ANSI（含 0 字节也算 ASCII）。调用方需要显式传入编码类型，属于已知限制，建议在文档中说明。
4. **`DllManager::LoadDll`**：使用 TCHAR 版 `LoadLibrary`，非 Unicode Windows 构建下 UTF-8 路径会传给 ANSI 版 API。建议统一使用 `LoadLibraryW`。
5. **`Clipboard::GetClipboardText`**：没有文本格式时仍返回 `true`（只反映了打开剪贴板成功），建议区分“操作成功”和“无文本数据”。另外直接 `text = buf` 依赖剪贴板数据以 `'\0'` 结尾，来自外部进程的数据不保证这一点。
6. **`ProcessSingletonData`**：以 `'\x1E'` 作为参数分隔符，参数本身不允许包含该字符；`dwDataLength` 只校验了上限，没有与实际包长度互相校验（当前 `substr` 会截断，不会越界）。
7. **`PerformanceUtil`**：非 Windows 平台不支持嵌套统计（只有一个开始时间）；`EndStat` 在计数为 0 时仅断言，建议增加运行时保护。
8. **`AttributeUtil::ParseString` / `ParseWindowSize`**：当配置为 `"0%"` 或 `"0"` 时会触发 `GetValidPercent` 的断言（配置非法时调试中断）。建议对非法百分比返回安全默认值。
9. **`AttributeUtil::ParseWindowSize` 的 `pScaledCX/pScaledCY`**：百分比场景未做 DPI 缩放，但传入窗口指针时仍返回“已缩放”，与注释语义不符，建议确认调用方预期后统一。
10. **`FilePath::operator<`**：Debug 下对未规范化的绝对路径有断言约束。属于设计约定，使用方需保证先调用 `NormalizeFilePath`。
11. **`NormalizeFilePath` 的分隔符转换**：依赖 `std::filesystem::lexically_normal` 在 Windows 上输出反斜杠（MSVC 已验证）；其它标准库实现不保证这一点，若需跨标准库移植建议显式 `make_preferred`。

## 5. 新增单元测试

新增 4 个独立测试目标，均已注册到 `tests/CMakeLists.txt`：

| 测试目标 | 测试名 | 覆盖内容 | 断言数 |
| --- | --- | --- | --- |
| `test_string_util` | `string_util` | 字符串替换（等长/变长/变短/未命中/空串）、大小写转换、Trim、Split、比较与空指针语义、数值与字符串互转、复制截断、超长格式化、布尔值判断 | 91 |
| `test_string_convert` | `string_convert` | UTF-8/UTF-16/UTF-32/WString 互转与往返、4 字节 UTF-8 字符、MBCS 转换、BOM 检测、ASCII/GBK/UTF-8/UTF-16 流校验、字节流到字符串转换（含显式编码与 BOM 冲突） | 76 |
| `test_file_path` | `file_path` | FilePath 解析/拼接/规范化/分隔符处理/父子目录判断/父目录列表/比较与 Hash、FilePathUtil 拼接与规范化、扩展名、文件读写往返（含空内容与不存在文件）、FileTime 往返与格式化 | 77 |
| `test_attribute_util` | `attribute_util` | Size/Point/Rect/Padding/Margin 解析、百分比与非百分比解析、属性列表（单/双引号、花括号、名称裁剪）、盘符提取、进程单例序列化/反序列化（正常、空参数、损坏包头、超长参数） | 66 |

测试代码只使用公开接口，不创建窗口、不加载主题资源；文件读写测试使用系统临时目录并在结束后清理。

## 6. 验证结果

- 构建：`python build\build.py build --config Debug --arch x64 --offline --target <target>`（`DUILIB_DLL=OFF`）全部成功。
- 运行：

```
ctest --test-dir out\x64\Debug -R "string_util|string_convert|file_path|attribute_util" --output-on-failure

1/4 Test  string_util .......   Passed
2/4 Test  string_convert ....   Passed
3/4 Test  file_path .........   Passed
4/4 Test  attribute_util ....   Passed

100% tests passed, 0 tests failed out of 4
```

- 测试过程中发现并修复了 2 个真实缺陷（宽字符格式化截断挂起、替换计数翻倍），均已在上表用例中固定下来，防止回归；另修正了 1 处测试预期（`FilePath::GetFileExtension` 保留小写且带点，与 `FilePathUtil` 的“去点并大写”不同）。
