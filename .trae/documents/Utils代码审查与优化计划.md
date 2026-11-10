# duilib/Utils 代码审查与优化计划

## Context（背景）

用户要求对 `duilib/Utils` 目录下全部代码做 Review：修复错误、优化可改进点、分析并改进性能，且**不做大改、不引入新 bug**；本轮只改本地代码，仅保证 Debug 编译通过。

已完成源码通读（StringUtil / StringConvert / StringCharset / FilePath / FilePathUtil / FileUtil / FileTime / LogUtil / PerformanceUtil / AttributeUtil / ProcessSingleton / DllManager / DiskUtils / SystemUtil / MonitorUtil / Clipboard / BitmapHelper / ScreenCapture / WinImplBase / Macros）。以下问题均为静态分析可直接确认的缺陷，修复范围小、行为变化明确。

## 一、明确的缺陷修复（按文件）

### 1. StringUtil.cpp
- **`StringAppendVT` 长字符串返回空串（严重）**：MSVC 的 `vsnprintf` 遵循 C99，截断时返回"所需长度"而不是 -1；当前代码在 `result != -1` 时 `ASSERT(0)` 并直接 return，导致 `StringUtil::Printf` 格式化超过 1023 字符时返回空字符串。
  修复：若 `result >= 0 && result >= buffer_size`，直接按 `result + 1` 一次性格式化；仍保留 -1 时倍增逻辑。
- **`StringCopy` / `StringNCopy`（非 Windows 分支）缓冲区越界**：`nLen = min(len, destSize)` 后写 `dest[nLen] = 0`，当源串超长时写到 `dest[destSize]`（越界）。
  修复：改为 `min(len, destSize - 1)`。
- **`StringToInt64` 使用无符号解析**：`wcstoull/strtoull` 解析到 `int64_t`，负数依赖实现定义的回绕。改为 `wcstoll/strtoll`。
- **`UInt64ToStringW/A` 性能**：逐字符 `str += temp[--pos]` 会触发多次扩容。改为先填充栈缓冲、再按区间一次性构造字符串。

### 2. StringConvert.cpp
- **性能**：每个转换函数都固定 `vector.resize(8192)`（UTF16→ 16KB、UTF32→ 32KB），短字符串也做一次 16KB 级零初始化，且输出串多次 append 扩容。
  修复：按输入长度计算输出上界一次性分配（UTF8→UTF16 上界 = 字节数；UTF16→UTF8 = 3×单元数；UTF8→UTF32 = 字节数；UTF32→UTF8 = 4×单元数；UTF16→UTF32 = 单元数；UTF32→UTF16 = 2×单元数），并对输出字符串 `reserve`，保留原有循环以兼容边界情况。行为不变，仅减少分配与零填充。
- 为 `UTF8ToUTF32` / `UTF32ToUTF8` 补充与其他函数一致的空指针/零长度保护。

### 3. StringCharset.cpp
- **显式指定编码时的误判**：无 BOM 且数据为纯 ASCII 时 `GetDataCharset` 返回 ANSI，导致显式指定 UTF16_LE/BE 或 UTF8 时被判定"编码不匹配"而返回 false（例如 `"A\0B\0"` 明确按 UTF16LE 读取会失败）。
  修复：当检测结果为 ANSI（ASCII 子集，缺乏区分度）时，信任调用方指定的编码，不再判定失败。

### 4. FilePath.cpp
- **`TrimRightPathSeparator` 破坏根目录语义**：`"C:\"` 会变成 `"C:"`（从"根目录"变成"驱动器当前目录"）。
  修复：当路径为"根名 + 分隔符"形式时保留原样（POSIX 的 `"/"` 已处理）。
- **`operator+=` 自拼接未防护**：`/=`、`JoinFilePath` 都判断了 `&rightPath == this`，`+=` 没有，自拼接为未定义行为。修复：同一对象时先拷贝右侧路径再拼接。

### 5. FilePathUtil.cpp
- **`GetCurrentModuleDirectory`（Windows）**：固定 1024 缓冲，超长路径被静默截断。改为在返回值等于缓冲区大小时扩容重试（`ERROR_INSUFFICIENT_BUFFER`）。
- **`GetFileExtension`**：先整体拷贝路径字符串；且路径以分隔符结尾时可能把目录名中的"."当成扩展名并带上分隔符。改为按下标直接解析（仍返回大写扩展名，不包含"."）。

### 6. FileUtil.cpp
- **空文件时输出残留**：`ReadFileData` / `ReadFileHeaderData` 在文件大小为 0 时不清理输出容器，返回 true 且保留旧数据。修复：进入函数先 `clear()`。
- **32 位文件大小**：`ftell` 在 Windows 上为 32 位，>2GB 文件失败。改用 `_ftelli64` / `ftello`。
- **`WriteFileData(const DStringW&)` 硬编码元素大小 2**：改为 `sizeof(DStringW::value_type)`（非 Windows 平台 wchar_t 为 4 字节时会写坏数据）。

### 7. LogUtil.cpp
- **`GetTimeStamp` 时间显示错误（明确 bug）**：分钟 = 总分钟数、秒 = 总秒数，没有对 60 取模。修复为 `(total/60)%60`、`total%60`。

### 8. PerformanceUtil.cpp
- **`GetQPCFrequency` 惰性静态变量无同步**：多线程首次调用存在数据竞争。改用函数内 `static const int64_t`（C++11 线程安全初始化）。

### 9. AttributeUtil.cpp
- **`ValidateWindowSize` 工作区为空时把窗口尺寸压成 0**：显示器查询失败（`rcWork` 为空）时 `width > 0` 恒成立。修复：仅在工作区非空时裁剪。

### 10. ProcessSingleton_Windows.h
- **监听端丢弃数据（严重）**：管道以 `FILE_FLAG_OVERLAPPED` 打开，`ReadFile` 通常返回 `ERROR_IO_PENDING`，当前条件判断把这种情况当成读取失败，导致新进程参数被静默丢弃。
  修复：`ERROR_IO_PENDING` 时等待完成并取回字节数；复用 `WaitForPipe`（增加可选的输出字节数参数）。
- **发送端未校验完成结果**：等待 pending 写完成后未调用 `GetOverlappedResult`，失败也返回 true。修复：等待后取回 `dwWritten` 并校验。
- **`GetUserSpecificName` 直接按字节扩展 UTF-8 应用名**：非 ASCII 名称会乱码。改用 `StringConvert::UTF8ToWString`。

### 11. DiskUtils_Windows.cpp
- **`unique_ptr<T>` 管理 `new[]`（UB）**：`GetLogicalDriveList` 用 `std::unique_ptr<value_type>` 托管数组，析构走 `delete` 而非 `delete[]`。改为 `std::unique_ptr<value_type[]>`。

### 12. Clipboard_Windows.cpp
- **`SetClipboardText` 未检查 `GlobalLock` 返回值**：失败时空指针 memcpy。补充判空并释放内存。
- `GetClipboardText`：用 `GlobalSize` 构造再截断的写法多余，改为直接按 C 字符串赋值（行为不变，少一次分配）。

### 13. MonitorUtil_Windows.cpp
- **`GetPrimaryMonitorDisplayScale` 未检查 `GetDC(nullptr)`**：空 DC 传给 `GetDeviceCaps`。补充判空，失败时回退 96 DPI。

### 14. BitmapHepler_Windows.cpp / ScreenCapture_Windows.cpp
- 宽高校验 `== 0` 改为 `<= 0`（负数时 DIB 创建行为未定义）。
- `BitmapHelper::CreateRenderObject(HDC...)`：补充 `CreateCompatibleDC` / `CreateGDIBitmap` 失败处理，避免把空 `pBits` 传给 `bitmap->Init` 并释放已分配对象。

## 二、其余文件（第二阶段，仅修明确缺陷）

通读剩余文件后只对可确证的缺陷做最小修复，不做结构调整：`Dialog.cpp`、`MessageBoxWnd.cpp`、`ToastWnd.cpp`、`TrayIcon_Windows.cpp`、`FileDialog_Windows.cpp`、`InlineHook_Windows.cpp`、`ApiWrapper_Windows.cpp`、`WinImplBase.cpp`。若发现的问题需要改结构或涉及平台差异，仅记录在最终报告中，不改代码。

## 三、性能分析结论（报告项）

- 字符串转换的固定 8K 缓冲是最高频的无效开销（已列入修复）。
- `StringUtil::Split` 使用 `strtok`，会修改副本并丢失空 token（保持现状，属 API 语义）。
- `AttributeUtil::ParseAttributeList` 逐字符 `+=` 构造 name/value（低收益，暂不改，避免语义风险）。
- `ProcessSingletonData::DeserializeData` 的 `erase` 循环为 O(n²)，但协议上限 4KB，影响可忽略。
- `PerformanceUtilHelper::m_stat` 无锁：OnPaint 等跨线程统计存在竞争；属设计级问题，只报告不建议本轮修改。

## 四、验证

1. Debug 增量编译（仅 duilib target），必须零错误：
   ```
   $msvc="C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231"; $sdk="C:\Program Files (x86)\Windows Kits\10"; $sdkver="10.0.26100.0"; $env:INCLUDE="..."; $env:LIB="...";
   cmake --build "E:\develop\nim_duilib\out\x64\Debug" --target duilib
   ```
2. 重点自检：`StringUtil::Printf` 超长字符串、`FileUtil` 读空文件、`GetFileExtension` 带尾分隔符路径、`LogUtil` 时间戳、`FileUtil` 写宽字符串。
3. 最终输出：修复清单（问题 → 修改 → 影响面）+ 未修改项及理由 + 性能收益说明。
