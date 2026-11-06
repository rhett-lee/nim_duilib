# Linux 编译警告修复方案

## Context

用户在 Linux（GCC 12，CMake，Release）构建 `nit_duilib` 时出现 8 类警告，分布在自有代码与第三方代码中。目标是消除这些警告，同时不改变运行时行为；本机为 Windows，无法直接复现 Linux 构建，最终由用户在 Linux 侧重新构建验证。按项目约定，本方案只修改源码/构建脚本，不执行任何 git 提交。

警告来源：
- `duilib/Control/AddressBarPathUtil.cpp:29`：`StartsWithNoCase` 未使用（仅 Windows 分支调用）
- `duilib/Core/ClickThrough_Linux.cpp:24/62`：`GetWindowPID`/`GetWindowTitle` 未使用（仅被注释的 TEST 代码引用）
- `duilib/RenderSkia/SkUTF.cpp:321`：`tmp` 可能未初始化、写入 8 字节到 4 字节缓冲区
- `-Wrestrict` ×4：`AddressBarPathUtil.cpp:295`、`DirectoryTreeImpl_Unix.cpp:461`、`ThemeManager.cpp:831`、`SkTextBox.cpp:302`（均为 libstdc++ `_M_replace` 的 GCC 12 误报）
- `duilib/third_party/svg/nanosvg.h:3211`：`size == -1l` 有符号/无符号比较
- giflib（`dgif_lib.c`/`egif_lib.c`/`quantize.c`）：6 处有符号/无符号比较
- Skia `tools/window/WindowContext.cpp`、`GLWindowContext.cpp`：`clang::reinitializes` 属性在 GCC 下被忽略（`-Wattributes`）

## 修改内容

### 1. duilib/Control/AddressBarPathUtil.cpp（第 27-36 行）

只把 `StartsWithNoCase` 的定义包进 `#ifdef DUILIB_BUILD_FOR_WIN ... #endif`（保留其上方注释在宏外或宏内均可），该函数只在 Windows 分支（190/210/259 行）使用。其他匿名命名空间函数不动。

### 2. duilib/Core/ClickThrough_Linux.cpp（第 24、62 行）

给 `GetWindowPID`、`GetWindowTitle` 加 `[[maybe_unused]]`，保留调试辅助代码（仅被 190-210 行注释的 TEST 代码引用）。

### 3. duilib/RenderSkia/SkUTF.cpp（第 299-326 行）

重写 `SkUTF::ToUTF8` 的字节写回逻辑，保持语义等价：
- `char tmp[SkUTF::kMaxBytesInUTF8Sequence] = { 0 };`（消除 maybe-uninitialized）
- 用下标写入代替 `*--utf8 = *p++`（消除 stringop-overflow 误报）
- `utf8[0] = (char)(~(0xFF >> count) | uni);`，`utf8[i] = tmp[count - 1 - i]`（`i` 从 1 到 `count-1`）
- `count` 最大为 4，与 `SkUTF.h:62` 的缓冲区长度一致

### 4. 第三方/系统性警告：duilib/CMakeLists.txt（第 410-420 行 GNU 分支）

- 对 giflib 全部 C 源码定向关闭符号比较警告：
  `set_source_files_properties(${GIFLIB_SRCS} PROPERTIES COMPILE_OPTIONS "-Wno-sign-compare")`
- Skia 两个 tools 源文件追加 `-Wno-attributes`（建议用 `set_property(SOURCE ... APPEND PROPERTY COMPILE_OPTIONS ...)`，避免覆盖已有的 `-mavx`/`-mavx2`）：
  - `WindowContext.cpp`：`-Wno-attributes`
  - `GLWindowContext.cpp`：`-Wno-attributes`，再按 `DUILIB_HAVE_AVX2`/`DUILIB_HAVE_AVX` 追加 `-mavx2`/`-mavx`

### 5. duilib/Image/ImageDecoder_SVG.cpp（第 11-14 行）

在现有 GCC `pragma GCC diagnostic push` 区内，追加一行
`#pragma GCC diagnostic ignored "-Wsign-compare"`，覆盖 `nanosvg.h:3211`。

### 6. cmake/duilib_compiler_flags.cmake（GNU 分支，第 36-40 行）

给 `DUILIB_CXX_COMPILER_FLAGS` 追加 `-Wno-restrict`，并注明：GCC 12 的 `-Wrestrict` 对 libstdc++ `basic_string::_M_replace` 存在误报（日志中报出 2^63 级别字节数），四处告警均不可操作，故定向关闭（仅 C++）。

## 验证

1. 用户在 Linux 上按原命令重新构建：`cmake --build out/x64/Release --parallel 4`。
2. 确认日志中不再出现上述 8 类警告（搜索 `警告` 与 `[-W`）。
3. 确认 `duilib` 库与示例链接、运行正常（尤其是 SVG 解码、GIF 解码、文本省略号显示、地址栏路径分段）；本机 Windows 仅回归确认修改文件可正常编译。
