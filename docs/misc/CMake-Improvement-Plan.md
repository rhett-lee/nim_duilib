# CMake 构建脚本改进计划（CMake Improvement Plan）

> 本文档是 2026-10-20 对 `nim_duilib` 仓库**全部 CMake 脚本**的审核结论，作为后续**逐项实施**的跟踪入口。
> 基线日期：**2026-10-20**　｜　当前状态：**第 1~3 批已完成（2026-10-20）：第 1 批 CM-1、CM-4、CM-6、CM-7、CM-9、CM-12、CM-16（CM-14 按反馈退回、不处理）；第 2 批 CM-2、CM-8、CM-19；第 3 批 CM-3、CM-15（后两项的 macOS 实机验证待补）。第 4 批：CM-20 已完成、CM-5 本轮暂不实施。见第九节实施记录，其余各项未开始**
> 更新规则：每完成一项把 `- [ ]` 改为 `- [x]` 并填写「完成日期」；新增项追加到对应小节末尾，不要重排既有编号。
> 关联文档：[Development-Plan.md](Development-Plan.md)（整体开发规划）、[Project-Audit.md](Project-Audit.md)（项目盘点）
> 前置工作：C-7（`DUILIB_SRC_MACOS` 平台判定脆弱）已于 2026-10-20 完成，见 [Development-Plan.md](Development-Plan.md) 的 C-7 小节。

## 扫描范围与方法

| 项 | 说明 |
| :--- | :--- |
| 扫描对象 | 全仓 `CMakeLists.txt`（55 个）与 `*.cmake`（43 个） |
| 第一方文件 | `duilib/CMakeLists.txt`、`examples/*/CMakeLists.txt`（23 个）、`cmake/*.cmake`（8 个）、skill 模板（2 个）、需要本仓库自行编译的第三方库（5 个）—— **共约 39 个** |
| 第三方上游文件 | `duilib/third_party/zlib/contrib/**`、`libpng/contrib/**`、`libcef/**`、`libjpeg-turbo/**` 等自带文件，不逐行审、不作为改动目标 |
| 方法 | 逐文件阅读 + 跨文件一致性对比（`Grep` 全仓核对变量设置位置与引用位置）+ 本机 CMake（4.3.0）实际配置验证 + `--warn-uninitialized` 检查 + 产物目录实测 |

### 已实测验证的事实（本机 Windows + cmake 4.3.0）

| 事实 | 证据 |
| :--- | :--- |
| CMake 路径的库产物**无 `_d` 后缀**，多配置生成器下按配置分子目录；手写 `.vcxproj` 路径的产物**有 `_d` 后缀**且集中在 `lib/x64/` | `lib/Debug/` 下为 CMake 产物（`duilib.lib`、`duilib-zlib.lib`、`duilib-png.lib`、`duilib-webp.lib`、`duilib-cximage.lib`，均无 `_d`）；`lib/x64/` 下为手写 vcxproj 产物（`duilib_d.lib`、`zlib_d.lib`、`libpng_d.lib`、`libwebp_d.lib`、`cximage_d.lib`） |
| `build/*.sln`（`examples.sln` / `duilib.sln`）是**手写 `.vcxproj` 工程，不经过 CMake** | `examples.sln` 内的项目路径为 `..\examples\basic\basic.vcxproj` |
| 走 CMake 的入口只有 `build/msvc_build.bat`、`build/linux_build.sh`、`build/macos_build.sh`、`build/freebsd_build.sh` | 脚本内均为 `cmake -S <duilib|examples/xxx> -B ...` |
| `cmake --warn-uninitialized` 全量配置仅报 1 个未初始化变量：`DUILIB_MD`（已在 CM-6 修复） | 审核时配置输出；修复后复测第一方项目为 0 条（`libpng` 仍有若干条，但全部来自其上游自带脚本，属本计划「非目标」） |

---

## 一、缺陷（CM-1 ~ CM-5，建议最先处理）

### CM-1 `duilib_common.cmake` 变量名中多了一个逗号 ✅ 已完成（2026-10-20）

- **位置**：[cmake/duilib_common.cmake:73](../..//cmake/duilib_common.cmake#L73) → `set(DUILIB_SYSTEM_PROCESSOR "unknown")`（修复前为 `set(DUILIB_SYSTEM_PROCESSOR, "unknown")`）
- **问题**：CMake 的参数分隔符是空白与分号，**逗号不是分隔符**，因此该语句实际创建的是名为 `DUILIB_SYSTEM_PROCESSOR,`（含逗号）的变量，真正的 `DUILIB_SYSTEM_PROCESSOR` **未被赋值**。
- **影响**：命中"未知 CPU 架构"分支时（前面分支均未匹配），`DUILIB_SYSTEM_PROCESSOR` 为空 → Skia 库路径、WebView2 路径、链接库名都会拼成错误路径（如 `out/llvm..release`）。当前 x64/arm64/x86 都能命中前序分支，属**潜在缺陷**。
- **动作**：改为 `set(DUILIB_SYSTEM_PROCESSOR "unknown")`。
- **验收**：该分支被触发时（可临时构造匹配失败的 `CMAKE_SYSTEM_PROCESSOR`）变量值为 `unknown`，且 `--warn-uninitialized` 不再提示。
- **工作量**：极低　**依赖**：无

### CM-2 `libpng` 中硬编码的 `ZLIB_LIBRARY` 路径与扩展名均错误 ✅ 已完成（2026-10-20）

- **位置**：[duilib/third_party/libpng/CMakeLists.txt:139-150](..//..//duilib/third_party/libpng/CMakeLists.txt#L139-L150)（修复前 `set(ZLIB_LIBRARY "${DUILIB_SRC_ROOT_DIR}/duilib/lib/libduilib-zlib.a")`）
- **问题**：两处错——
  1. 路径**多了一层 `duilib/`**：zlib 的产物实际输出到 `${DUILIB_SRC_ROOT_DIR}/lib/`（见 [zlib/CMakeLists.txt:15-18](..//..//duilib/third_party/zlib/CMakeLists.txt#L15-L18) 与 [libpng/CMakeLists.txt:726](..//..//duilib/third_party/libpng/CMakeLists.txt#L726)）；
  2. 扩展名 `.a` 是 Unix 静态库格式，Windows/MSVC 下 zlib 产物是 `duilib-zlib.lib`（实测位于 `lib/Debug/`，VS 多配置生成器按配置分子目录）。
- **影响**：该"提示路径"永远不存在，`find_package(ZLIB)` 会静默转去搜索**系统 zlib**，导致链接到哪个 zlib 不可控（Windows 上尤其危险）；属从 Linux 构建脚本移植时的残留。
- **动作**（已实施）：去掉多余的 `duilib/` 层级；库名前缀/后缀改用 `CMAKE_STATIC_LIBRARY_PREFIX` / `CMAKE_STATIC_LIBRARY_SUFFIX`；配置目录用 `CMAKE_CFG_INTDIR` 拼接（多配置生成器展开为 `$(Configuration)`，单配置为 `.`）。
- **验收**：Windows/Linux 下配置 libpng 后，`ZLIB_LIBRARY` 指向真实存在的文件（`EXISTS` 为真）；libpng 构建日志中不再有落到系统 zlib 的迹象。
- **实测结论**：配置后 `Found ZLIB: C:/develop/nim_duilib/lib/$(Configuration)/duilib-zlib.lib (found version "1.3.2")`，且 `ZLIB_INCLUDE_DIR` 为仓库内 `duilib/third_party/zlib`；libpng 编译通过（exit 0）。
- **注意**：`$<CONFIG>` **不能**用在这里——实测它会被原样写入 `IMPORTED_LOCATION`→vcxproj 的 `AdditionalDependencies`（`$<CONFIG>\x.lib`），链接器无法识别。
- **工作量**：低　**依赖**：无

### CM-3 `duilib_cef_macos.cmake` 引用了未定义的 `${target}` ✅ 已完成（2026-10-20）

- **位置**：修复前 [cmake/duilib_cef_macos.cmake:58-63](..//..//cmake/duilib_cef_macos.cmake#L58-L63) → `set_target_properties(${target} PROPERTIES CLANG_ENABLE_OBJC_ARC "YES")`；修正后位于 [同文件:113-119](..//..//cmake/duilib_cef_macos.cmake#L113-L119)。本文件内目标变量名为 `CEF_TARGET`（[:46](..//..//cmake/duilib_cef_macos.cmake#L46)）。
- **问题**：`target` 从未在本文件内定义（只在 CEF 上游示例的 `foreach(target ...)` 循环里出现过），此处应为 `${CEF_TARGET}`。
- **影响**：`OPTION_USE_ARC` 默认为 `ON`，因此 **macOS + CEF 构建时会在配置阶段报错**（`set_target_properties` 参数错误），或该 ARC 属性根本没设上。
- **动作**（已实施）：
  1. 改为 `${CEF_TARGET}`；
  2. 该代码块整体下移到「`CEF_COMPILER_FLAGS` 被整体赋值之后 + `add_executable(${CEF_TARGET} ...)` 之后」——原位置在 `add_executable` 之前，`set_target_properties` 会因目标不存在而失败；同时原位置的 `list(APPEND CEF_COMPILER_FLAGS -fobjc-arc)` 会被随后第 72 行的 `set(CEF_COMPILER_FLAGS ...)` 整体覆盖、导致 `-fobjc-arc` 静默丢失，下移后两处一并修正。
  3. 核实结果：该文件**仍在用**——仅由 [cmake/duilib_bin_macos.cmake:15-17](..//..//cmake/duilib_bin_macos.cmake#L15-L17) 在 `macOS + DUILIB_ENABLE_CEF` 时 `include`，与 CM-15 的 `CEF_ROOT` 问题是同一文件。
- **验收**：macOS 上配置 `examples/cef`（或 `CefBrowser`）无报错，且目标确实带 `CLANG_ENABLE_OBJC_ARC` 属性。
- **实测结论**：本机为 Windows，无法实机配置 macOS 分支；已做**静态核实**——`CEF_TARGET` 在同文件 L46 赋值、`add_executable` 在 L69 创建目标，修正后的代码块（L113-119）在二者之后，`${CEF_TARGET}` 与 `CEF_COMPILER_FLAGS` 均已成定义。**macOS 实机验证待补**。
- **工作量**：极低　**依赖**：需 macOS 环境核实

### CM-4 两份 skill 模板缺 `DUILIB_WINRES_FILE_NAME`，新建示例会漏资源 ✅ 已完成（2026-10-20）

- **位置**：[.trae/skills/nim-duilib/assets/templates/CMakeLists.txt](..//..//.trae/skills/nim-duilib/assets/templates/CMakeLists.txt)、[.workbuddy/skills/nim-duilib/assets/templates/CMakeLists.txt](..//..//.workbuddy/skills/nim-duilib/assets/templates/CMakeLists.txt)
- **问题**：23 个 examples 的 CMakeLists **全部**包含 `set(DUILIB_WINRES_FILE_NAME "${PROJECT_NAME}.rc")`（控制在 `include(duilib_common.cmake)` 之前），两份模板都缺这一行；模板也没有 CEF / SDL / WebView2 开关与 `DUILIB_SRC_SUB_DIRS` 的示例注释。
- **影响**：按模板新建的示例拿不到 `.rc`（图标、manifest、版本信息），且后续维护者可能继续复制缺失项。
- **动作**：模板补齐 `set(DUILIB_WINRES_FILE_NAME "${PROJECT_NAME}.rc")`（含注释），并补充"可选开关"的注释示例（`DUILIB_ENABLE_SDL` / `DUILIB_ENABLE_CEF` / `DUILIB_WEBVIEW2_EXE` / `DUILIB_SRC_SUB_DIRS` / `DUILIB_THEME_DIR_NAME`），并注明**开关必须写在 `include(duilib_common.cmake)` 之前**。
- **验收**：用模板新建一个示例，Windows 下产物带图标与 manifest；`DUILIB_SRC_SUB_DIRS` 等开关按注释填写后生效。
- **工作量**：极低　**依赖**：无

### CM-5 多配置生成器下 `DUILIB_BUILD_TYPE` 恒为 release（且 Debug/Release 产物同名互相覆盖）

- **位置**：[cmake/duilib_common.cmake:83-87](..//..//cmake/duilib_common.cmake#L83-L87)（用 `CMAKE_BUILD_TYPE` 推导）、[cmake/duilib_common.cmake:94-106](..//..//cmake/duilib_common.cmake#L94-L106)（Windows 下据此拼 Skia 库子目录）
- **问题**：
  1. VS / Xcode 这类**多配置生成器**下 `CMAKE_BUILD_TYPE` 恒为空 → `DUILIB_BUILD_TYPE` 永远是 `release`。在 VS 里把配置切成 Debug 重新生成时，Skia 库路径仍是 `llvm.x64.release`，与 Debug 版 Skia 不匹配；目前只是被 `msvc_build.bat` 固定传 `-DCMAKE_BUILD_TYPE=Release` 掩盖。
  2. CMake 路径下各库（zlib / libpng / libwebp / cximage / duilib）只设置了 `CMAKE_ARCHIVE_OUTPUT_DIRECTORY`，**没有 `CMAKE_DEBUG_POSTFIX`** → 产物名不带 `_d`，与手写 `.vcxproj` 的命名（`duilib_d.lib`）不统一；且在**单配置生成器**（Linux/macOS 的 Ninja/Makefile）下 Debug 与 Release 会写入同一个 `${root}/lib/` 目录、同名互相覆盖（多配置生成器 VS 会自动分到 `lib/Debug`、`lib/Release`，不冲突）。已实测：`lib/Debug/` 为 CMake 产物（无 `_d`），`lib/x64/` 为 vcxproj 产物（有 `_d`）。
- **动作**：二选一或都做——
  1. 用 `$<CONFIG>` 生成表达式替代 `CMAKE_BUILD_TYPE`（例如 `llvm.${ARCH}.$<IF:$<CONFIG:Debug>,debug,release>`），使多配置生成器自动切换；
  2. 给第三方面向仓库的产物统一加 `CMAKE_DEBUG_POSTFIX "_d"`，并把 `DUILIB_LIBS` 的引用方式改为按配置后缀取值（或改用 imported target）。
- **验收**：在 VS 生成器下分别生成 Debug / Release，Skia 与 SDL 库路径各自正确；两种配置的产物不再互相覆盖（文件可同时存在）。
- **实测证据（2026-10-20 批次 2 期间）**：`build/msvc_build.bat` 只用 `-DCMAKE_BUILD_TYPE=Release` + 多配置生成器，并显式传 `-DDUILIB_SKIA_LIB_SUBPATH=llvm.<ARCH>.release` 规避本问题；若**不走**该脚本、直接在 VS 生成器下构建 `Debug` 配置，`examples/ChildWindow` 实测报 **1694 个 LNK2038**（`_ITERATOR_DEBUG_LEVEL` 0 vs 2、`RuntimeLibrary` `MT_StaticRelease` vs `MTd_StaticDebug`）+ `LNK1319`——即 Debug 配置链接到了 Release 版 Skia/第三方库。修复后此项应消失。
- **工作量**：低 ~ 中（涉及 `duilib_common.cmake` + 5 个第三方 CMakeLists）　**依赖**：需先确定 Debug/Release 命名规则（见 CM-17）
- **实施状态（2026-10-20）**：本轮按用户决定**暂不实施**（两项动作风险差异大，留待 CM-17/CM-21 定命名约定后一起做）。已用探针工程验证「动作 1」技术可行：`link_directories` 支持 `$<CONFIG>` 生成表达式（VS 多配置下 Debug 生成 `.../dbg`、Release 生成 `.../rel`）；`c:\develop\skia\out\` 下同时存在 `llvm.x64.debug` 与 `llvm.x64.release`，路径切换后即可消除 Debug 链接 Release 库的 LNK2038。

---

## 二、未定义变量与静默行为（CM-6 ~ CM-9）

### CM-6 `DUILIB_MD` 被当作已定义变量使用（实际出现 6 处）✅ 已完成（2026-10-20）

- **位置**（审核时误记为 4 处，实际 6 处，全部同样写法）：
  - [duilib/CMakeLists.txt:10](..//..//duilib/CMakeLists.txt#L10)
  - [cmake/duilib_bin_windows.cmake:8](..//..//cmake/duilib_bin_windows.cmake#L8)
  - [duilib/third_party/zlib/CMakeLists.txt:6](..//..//duilib/third_party/zlib/CMakeLists.txt#L6)
  - [duilib/third_party/libpng/CMakeLists.txt:22](..//..//duilib/third_party/libpng/CMakeLists.txt#L22)（审核时遗漏）
  - [duilib/third_party/libwebp/CMakeLists.txt:10](..//..//duilib/third_party/libwebp/CMakeLists.txt#L10)
  - [duilib/third_party/cximage/CMakeLists.txt:10](..//..//duilib/third_party/cximage/CMakeLists.txt#L10)（审核时遗漏）
  - 另有两份文档中的同款示例：[cmake/README.md:176](..//..//cmake/README.md#L176)、[cmake/README.en.md:179](..//..//cmake/README.en.md#L179)
- **问题**：写法都是 `if("${DUILIB_MD}" STREQUAL "ON")`。`DUILIB_MD` 是外部可传的开关，未定义时会被 `--warn-uninitialized` 报出（本机实测：全量配置仅此一条警告）。
- **影响**：功能上等价于"未设置则走 else（MT）"，但语义隐晦，且是 CMake 层面唯一的未初始化变量告警。
- **动作**：统一改为 `if(DUILIB_MD)`（6 处 + 2 份文档示例保持同一种写法）；如需保留"仅 ON 生效"的语义，可改为 `if(DUILIB_MD STREQUAL "ON")`（不引用 `${}` 即可避免告警）。
- **验收**：`cmake --warn-uninitialized` 配置输出中**0 条**未初始化变量告警。
- **工作量**：极低　**依赖**：无

### CM-7 `duilib/CMakeLists.txt` 中的死变量与冗余条件 ✅ 已完成（2026-10-20）

- **位置**（审核时的行号）：
  - `duilib/CMakeLists.txt:94` → `list(APPEND PROJECT_SOURCES ${CEF_CONTROL_SRC} ${CEF_CONTROL_INTERNAL_SRC})`（已删除）
  - [duilib/CMakeLists.txt:67](..//..//duilib/CMakeLists.txt#L67) → `if(CEF_LIB_DIR)`（已简化，原为 `if(CEF_LIB_DIR AND NOT CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")`）
  - `cmake/duilib_bin_windows.cmake:105` → Windows 专用文件内重复判断 `CMAKE_SYSTEM_NAME STREQUAL "Windows"`（已删除）
- **问题**：`PROJECT_SOURCES` 全仓**没有任何地方读取**（`Grep` 仅命中定义处）→ 该 `list(APPEND)` 无任何作用；`:68` 的后半条件冗余（FreeBSD 分支已把 `CEF_LIB_DIR` 置空）。
- **影响**：读代码时产生误导（以为 CEF 源通过该变量传递）；无功能影响。
- **动作**：删除 `PROJECT_SOURCES` 行；`if(CEF_LIB_DIR AND NOT ... FreeBSD)` 简化为 `if(CEF_LIB_DIR)`；删除 `duilib_bin_windows.cmake:105` 的重复判断。
- **验收**：全仓 `Grep "PROJECT_SOURCES"` 无命中；Windows 与 FreeBSD 的 CMake 配置行为不变。
- **工作量**：极低　**依赖**：无

### CM-8 未定义变量被直接链接（静默空展开）✅ 已完成（2026-10-20）

- **位置**：[cmake/duilib_bin_windows.cmake:126](..//..//cmake/duilib_bin_windows.cmake#L126)（`${DUILIB_SDL_LIBS}`、`${DUILIB_CEF_LIBS}`）、[cmake/duilib_bin_linux.cmake:24](..//..//cmake/duilib_bin_linux.cmake#L24)、[cmake/duilib_bin_macos.cmake:68](..//..//cmake/duilib_bin_macos.cmake#L68)
- **问题**：`DUILIB_SDL_LIBS` 仅在 `DUILIB_ENABLE_SDL` 为真时定义（[duilib_common.cmake:169-183](..//..//cmake/duilib_common.cmake#L169-L183)），`DUILIB_CEF_LIBS` 仅在 `DUILIB_ENABLE_CEF` 为真时定义（[L119-157](..//..//cmake/duilib_common.cmake#L119-L157)）；关闭开关后它们在链接语句中静默展开为空。
- **影响**：无功能影响，但与 C-7 已消除的"静默操作未定义变量"属同一类问题，口径应统一。
- **动作**（已实施）：在 `duilib_common.cmake` 的两个条件块**之前**给出默认空值（[L117-118](..//..//cmake/duilib_common.cmake#L117-L118)）：`set(DUILIB_CEF_LIBS "")` / `set(DUILIB_SDL_LIBS "")`，开启开关后再填充。不改变任何链接行为。
- **验收**：关闭 SDL / CEF 后配置并检查链接命令，不含空展开痕迹；`--warn-uninitialized` 0 告警（与 CM-6 一并验收）。
- **实测结论**：`duilib`、`examples/basic`、`examples/ChildWindow`、`examples/CefBrowser`、`examples/WebView2Browser` 五项配置 exit 0，`--warn-uninitialized` 均为 **0 条**（libpng 剩余的 10 条全部来自其上游脚本，属非目标）。
- **工作量**：极低　**依赖**：无

### CM-9 `libwebp` 对纯 C 库定义 `UNICODE`/`_UNICODE` ✅ 已完成（2026-10-20）

- **位置**：[duilib/third_party/libwebp/CMakeLists.txt:21-24](..//..//duilib/third_party/libwebp/CMakeLists.txt#L21-L24)
- **问题**：libwebp 是纯 C 库、对外 API 使用 `char`，加 `-DUNICODE -D_UNICODE` 只会影响 Windows 头文件的 `TCHAR` 映射，对构建无收益。
- **影响**：无直接破坏，但属无意义的全局宏污染；同时它是各平台编译选项差异中不必要的噪声。
- **动作**：删除该段（`/utf-8` 建议保留）。
- **验收**：libwebp 仍能编译通过，产物不变。
- **工作量**：极低　**依赖**：无

---

## 三、可维护性与一致性（CM-10 ~ CM-16）

### CM-10 macOS 编译参数存在三份重复

- **位置**：
  - [cmake/duilib_bin_macos.cmake:20-57](..//..//cmake/duilib_bin_macos.cmake#L20-L57)
  - [cmake/duilib_cef_macos.cmake:76-115](..//..//cmake/duilib_cef_macos.cmake#L76-L115)
  - [duilib/CMakeLists.txt:262-306](..//..//duilib/CMakeLists.txt#L262-L306)
- **问题**：`-fno-strict-aliasing` / `-fvisibility=hidden` / `-Wno-...` 等约 15~20 行内容在三个文件里各写一遍（注释也重复），任何一项调整都要同步三处。
- **影响**：漂移风险高（曾经就出现过参数不一致）。
- **动作**：抽成 `cmake/duilib_compiler_flags.cmake`（一个 `DUILIB_COMPILER_FLAGS` / `DUILIB_CXX_COMPILER_FLAGS` 的公共定义），三处改为 `include()` 引用。
- **验收**：三处参数来源唯一；macOS（含 CEF）与 Linux 构建参数与改动前一致（对比配置日志 / 编译命令行）。
- **工作量**：低　**依赖**：需 macOS / Linux 环境验证

### CM-11 23 个示例的 CMakeLists 是同一份 21 行样板

- **位置**：`examples/*/CMakeLists.txt`（23 个，[examples/basic/CMakeLists.txt](..//..//examples/basic/CMakeLists.txt) 为例）
- **问题**：除 3~6 行差异（开关、子目录、主题目录名）外，其余 15 行完全重复；`cmake_minimum_required` / `DUILIB_SRC_ROOT_DIR` / `DUILIB_PROJECT_SRC_DIR` / `DUILIB_WINRES_FILE_NAME` / 两次 `include` 都逐字复制。
- **影响**：改一处公共约定要动 23 个文件；新示例的写法靠"抄"，缺少约束（本次 CM-4 就暴露了模板缺项）。
- **动作**：抽出一个 `cmake/duilib_example.cmake`（传入 `DUILIB_PROJECT_NAME` 与可选开关），示例文件缩减为"声明 + 开关 + include"数行；保留现有文件路径不变（`-S examples/xxx` 仍可用）。
- **验收**：23 个示例的构建产物、开关生效性与改动前一致（可用 `DUILIB_LOG` 输出对比）。
- **工作量**：中　**依赖**：建议在 CM-10 之后做

### CM-12 平台判据不统一（`APPLE` 与 `CMAKE_SYSTEM_NAME STREQUAL "Darwin"` 并存）✅ 已完成（2026-10-20）

- **位置**：[cmake/duilib_common.cmake:6](..//..//cmake/duilib_common.cmake#L6)（原为 `elseif(APPLE)`，已改为 `elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")`）对比 [duilib/CMakeLists.txt:43](..//..//duilib/CMakeLists.txt#L43)、[cmake/duilib_bin.cmake:44-56](..//..//cmake/duilib_bin.cmake#L44-L56)（用 `CMAKE_SYSTEM_NAME`）
- **问题**：`APPLE` 为真同时涵盖 iOS / tvOS 等，而其余代码按 `Darwin` 判定；两套判据在新增苹果平台时会分叉。
- **影响**：潜在误判（例如未来支持 iOS 时行为不一致）。
- **动作**：统一为 `CMAKE_SYSTEM_NAME STREQUAL "Darwin"`（C-7 已在 `duilib/CMakeLists.txt` 内做过这一步）。
- **验收**：macOS 下 `DUILIB_OS_MACOS` 仍被设置；`Grep "if(APPLE)"` 在 `cmake/` 下无命中。
- **工作量**：极低　**依赖**：无

### CM-13 Linux 与 FreeBSD 的 freetype / fontconfig 处理方式不一致

- **位置**：[cmake/duilib_bin_linux.cmake:21](..//..//cmake/duilib_bin_linux.cmake#L21)（裸名 `freetype fontconfig`）对比 [cmake/duilib_bin_freebsd.cmake:15-20](..//..//cmake/duilib_bin_freebsd.cmake#L15-L20)（`find_package(... REQUIRED)` + imported target），另见 [duilib/CMakeLists.txt:235-239](..//..//duilib/CMakeLists.txt#L235-L239)
- **问题**：Linux 用裸库名依赖链接器搜索路径，FreeBSD 用 imported target；同一份依赖在库与示例之间也各写一遍。
- **影响**：Linux 上换发行版/交叉编译时容易找不到库且报错信息不明确。
- **动作**：Linux 侧同样改用 `find_package(Freetype/Fontconfig/X11 REQUIRED)` + imported target，并把公共依赖集中到 `cmake/` 下的公共文件。
- **验收**：Linux 下配置与链接不再依赖裸库名搜索；FreeBSD 行为不变。
- **工作量**：低　**依赖**：需 Linux 环境验证

### CM-14 示例个例修补 SDL include 路径，未统一 ✗ 不处理（2026-10-20 按反馈退回，保持现状）

- **位置**：`examples/ChildWindow/CMakeLists.txt:22-25` → `include_directories("${DUILIB_SDL_SRC_ROOT_DIR}/include")`（**保持现状**，未做改动）
- **审核结论（已作废）**：曾建议把该 include 上收到 `duilib_bin.cmake` 的 `if(DUILIB_ENABLE_SDL)` 分支内统一添加。
- **退回原因（用户裁定）**：**SDL 的 include 目录不开放给各个示例程序**——仅 duilib 库内部使用 SDL，属内部实现范畴；`ChildWindow` 是**有意的例外**，该示例包含系统原生子窗口，需要直接使用 SDL 渲染子窗口内容，因此它单独显式包含 SDL 头文件目录是正确写法。
- **结论**：本项**不处理**，审核结论作废；`cmake/duilib_bin.cmake` 只保留 SDL 的 `link_directories`，不添加 SDL 的 `include_directories`。
- **工作量**：—　**依赖**：无

### CM-15 硬编码路径集中清理 ✅ 已完成（2026-10-20）

- **位置与问题**：
  | 硬编码 | 位置 | 说明 |
  | :--- | :--- | :--- |
  | WebView2 库目录 `duilib/third_party/Microsoft.Web.WebView2/build/native/${DUILIB_SYSTEM_PROCESSOR}` | [duilib_bin_windows.cmake:45](..//..//cmake/duilib_bin_windows.cmake#L45)、[117](..//..//cmake/duilib_bin_windows.cmake#L117) | 目录名与 `DUILIB_SYSTEM_PROCESSOR` 取值（`x64`/`x86`）强耦合，且未做存在性检查 |
  | manifest 文件 `msvc/manifest/duilib.x64.manifest` / `.x86.manifest` | [duilib_bin_windows.cmake:67-69](..//..//cmake/duilib_bin_windows.cmake#L67-L69) | 与 `DUILIB_BITS_64` 手工配对，且**同一段代码在文件内重复定义两次**（L55-59 与 L67-69） |
  | CEF 根目录 `../../cef_binary` | [duilib_cef_macos.cmake:9](..//..//cmake/duilib_cef_macos.cmake#L9) | 解析结果为**仓库之外**的目录；审核时判断为"与 `third_party/libcef/libcef_macos` 的存放方式不一致、疑似脱节" |
- **动作**（已实施）：
  1. **WebView2**：新增 `DUILIB_WEBVIEW2_NATIVE_DIR`（[duilib_bin_windows.cmake:29-31](..//..//cmake/duilib_bin_windows.cmake#L29-L31)），`link_directories` 与 DLL 复制两处改用它；在 `if(DUILIB_WEBVIEW2_EXE)` 分支内加 `EXISTS` 校验，缺失时报 `FATAL_ERROR` 并给出完整路径。
  2. **manifest**：`DUILIB_WIN_MANIFEST` 的定义上移并**去掉重复定义**（[L33-41](..//..//cmake/duilib_bin_windows.cmake#L33-L41)），加 `EXISTS` 校验（缺失时报 `FATAL_ERROR`）；MSVC / MinGW 两处使用点不变。
  3. **macOS CEF_ROOT**：**核实结论与审核判断不同**——`third_party/libcef/libcef_macos` 只有 `cmake/ include/ libcef_dll/`（用于编译 `libcef_dll_wrapper`），**不含** `Release/Chromium Embedded Framework.framework` 与 `Resources/`；而 `COPY_MAC_FRAMEWORK` 需要从 `CEF_BINARY_DIR`（= `${CEF_ROOT}/$<CONFIGURATION>`）拷贝框架，因此不能指向仓库内目录。`../../cef_binary` 与 [build/build_duilib_all_in_one.sh:369-403](../../build/build_duilib_all_in_one.sh#L369-L403) 的"下载并解压到 `nim_duilib` 同级 `cef_binary`"约定一致，**默认值保持不变**；改为：允许 `-DCEF_ROOT=...` 或环境变量 `CEF_ROOT` 覆盖（`FindCEF.cmake` 本身优先采用外部值），并在目录不存在时给出明确的 `FATAL_ERROR`（原来会在 `find_package(CEF)` 处报模糊错误）。
- **验收**：三条路径均来自单一变量定义处；macOS + CEF 的配置能定位到真实存在的目录。
- **实测结论（Windows 部分）**：`examples/basic`、`examples/CefBrowser`、`examples/WebView2Browser` 三项配置 exit 0、0 告警；生成的 `WebView2Browser.vcxproj` 中复制命令为 `.../Microsoft.Web.WebView2/build/native/x64/WebView2Loader.dll`，`basic.vcxproj` 中为 `AdditionalManifestFiles = C:\develop\nim_duilib\msvc\manifest\duilib.x64.manifest`（路径均正确、且来自单一变量）。
- **macOS 部分**：本机无法实机验证，仅做路径与 `COPY_MAC_FRAMEWORK`/`FindCEF.cmake` 的静态核对。**macOS 实机验证待补**。
- **工作量**：低（含核实）　**依赖**：需 macOS 环境核实 CEF 分支

### CM-16 小项合集（6 条全部处理）✅ 已完成（2026-10-20）

| # | 位置（审核时） | 问题 | 动作 | 状态 |
| :--- | :--- | :--- | :--- | :--- |
| 1 | [cmake/duilib_compiles.cmake:6,16](..//..//cmake/duilib_compiles.cmake#L6-L16) | `set(CMAKE_REQUIRED_FLAGS "-mavx")` 为**覆盖**而非追加；未使用 `CMakePushCheckState` | 追加写法 + push/pop，避免污染外层检测 | ✅ 已按动作实施（用法补充：首次检测前 `CMAKE_REQUIRED_FLAGS` 可能未定义，故用 `${CMAKE_REQUIRED_FLAGS} -mavx` 追加是安全的空展开） |
| 2 | [duilib_common.cmake:18-19,42-43](..//..//cmake/duilib_common.cmake#L18-L43) | `option()` 定义在 `if(MINGW)` / `if(DUILIB_OS_WINDOWS)` 内部 → 条件外该缓存变量不存在，行为隐晦 | 提到文件顶层统一声明 | ✅ 已实施（`DUILIB_MINGW_STATIC`、`DUILIB_WEBVIEW2_EXE` 移到与其它 `option()` 同一区块） |
| 3 | [duilib_common.cmake:21](..//..//cmake/duilib_common.cmake#L21) | 注释写"此处不声明"，但下方实际以 `set(... CACHE STRING ...)` 声明了 `DUILIB_SKIA_LIB_SUBPATH` | 修正注释 | ✅ 已实施（注释改为"最终以 CACHE 变量 DUILIB_SKIA_LIB_SUBPATH 声明"） |
| 4 | [duilib_common.cmake:183-184](..//..//cmake/duilib_common.cmake#L183-L184) | 79 行 `DUILIB_LOG` 日志块占文件近 1/3 | 抽成 `duilib_log_variables.cmake` | ✅ 已实施（新增 [cmake/duilib_log_variables.cmake](..//..//cmake/duilib_log_variables.cmake)，原文件末尾改为 `include(...)`） |
| 5 | [libwebp/CMakeLists.txt:1](..//..//duilib/third_party/libwebp/CMakeLists.txt#L1) | `cmake_minimum_required` 为 3.16，其余为 3.18 | 统一到 3.18 | ✅ 已实施 |
| 6 | [duilib_bin.cmake:1-3](..//..//cmake/duilib_bin.cmake#L1-L3) | in-tree 构建检查写在这个被每个示例 include 的公共文件里 | 保留但确认语义 | ✅ 已确认（`CMAKE_CURRENT_SOURCE_DIR`/`CMAKE_CURRENT_BINARY_DIR` 在 include 处取的是**示例所在目录**，语义正确，保留原位；顶层聚合见 CM-22） |

- **工作量**：低（分条独立）　**依赖**：无

---

## 四、现代化与结构性（CM-17 ~ CM-19）

### CM-17 作用域化：全局 `include_directories` / `link_directories` → `target_*`

- **位置**：[cmake/duilib_bin.cmake:15-26](../..//cmake/duilib_bin.cmake#L15-L26)、[cmake/duilib_common.cmake:14-26](../..//cmake/duilib_common.cmake#L14-L26)、[duilib/CMakeLists.txt:63-70](..//..//duilib/CMakeLists.txt#L63-L70)、`duilib_bin_windows.cmake:48,51,56`、`duilib_bin_linux.cmake:8,11`
- **问题**：全部使用目录级/全局形式的 `include_directories()` 与 `link_directories()`；后者的搜索顺序还依赖调用顺序（顺序敏感）。
- **影响**：作用域污染、顺序脆弱；仓库多套构建（库 / 示例）之间难以复用。
- **动作**：改为 `target_include_directories` / `target_link_directories` / `target_link_libraries`（配合 `PRIVATE`/`PUBLIC`）。
- **验收**：配置与编译结果不变，`Grep` 第一方文件中不再有全局 `include_directories` / `link_directories`。
- **工作量**：中　**依赖**：建议在 CM-11（示例收敛）之后做，收益更大

### CM-18 `aux_source_directory` → 显式源文件列表

- **位置**：[duilib/CMakeLists.txt:80-89](..//..//duilib/CMakeLists.txt#L80-L89)、[cmake/duilib_bin.cmake:32-41](../..//cmake/duilib_bin.cmake#L32-L41)、[libwebp/CMakeLists.txt:33-38](..//..//duilib/third_party/libwebp/CMakeLists.txt#L33-L38)、`cximage` 等
- **问题**：CMake 官方不推荐 `aux_source_directory`——新增源文件不会触发重新配置（需手动重跑），且会把目录下不需要的文件一并纳入。
- **影响**：新增 `duilib/Control/X.cpp` 后忘记重配置会"莫名没编进去"；平台专属文件要靠事后 `list(REMOVE_ITEM)` 剔除（C-7 处理的就是这类问题）。
- **动作**：duilib 库与示例改为按平台分组显式列举源文件（可用 `file(GLOB_RECURSE CONFIGURE_DEPENDS ...)` 作为过渡，它能在新增文件后自动重配置）。
- **验收**：新增/删除一个源文件后，重新配置即可正确参与构建（无需清理缓存）；平台专属文件（`.mm` 等）不再依赖 `REMOVE_ITEM` 补丁。
- **工作量**：中 ~ 高　**依赖**：无

### CM-19 链接选项写法过时 ✅ 已完成（2026-10-20）

- **位置**：[duilib_bin_windows.cmake:35-44](..//..//cmake/duilib_bin_windows.cmake#L35-L44)（原 `set(CMAKE_EXE_LINKER_FLAGS ...)`，MinGW 的 `-mwindows` / `-static`）、[duilib_bin_windows.cmake:77-97](..//..//cmake/duilib_bin_windows.cmake#L77-L97)（原 `set_target_properties(... LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup")`）
- **问题**：用全局变量覆盖链接参数会**同时影响**该目录下所有 target 及后续子目录；`LINK_FLAGS` 属性已是过时写法；显式 `/ENTRY:wWinMainCRTStartup` 一般不需要（`/SUBSYSTEM:WINDOWS` 会自动推导入口）。
- **动作**（已实施）：删除原文件顶部 `set(CMAKE_EXE_LINKER_FLAGS ...)` 全局覆盖块；MSVC 的 `/SUBSYSTEM:WINDOWS` + `/ENTRY:wWinMainCRTStartup` + `/DELAYLOAD:libcef.dll` 改为 `target_link_options(${PROJECT_NAME} PRIVATE ...)`；MinGW 的 `-mwindows` / `-static` 移入新增的 `elseif(DUILIB_MINGW)` 分支，同样用 `target_link_options`（不再影响其它目标）。
- **验收**：示例产物仍为 Windows 子系统、可正常启动（含 MinGW 构建）；注入的全局 `CMAKE_EXE_LINKER_FLAGS` 不再被覆盖。
- **实测结论**：`examples/ChildWindow` 生成的 vcxproj 含 `<SubSystem>Windows</SubSystem>` 与 `<EntryPointSymbol>wWinMainCRTStartup</EntryPointSymbol>`，且 vcxproj 中已无 `CMAKE_EXE_LINKER_FLAGS` 的痕迹；链接错误仅剩既有的 LNK2038 运行库不匹配（属 CM-5 范围，非本次改动引入）。MinGW 分支本机无工具链，未做实机验证。
- **`/ENTRY` 评估结论：保留**——`examples/*/main_windows.cpp` 同时定义了 `WinMain` 与 `wWinMain`，UNICODE 下入口为 `wWinMain`；显式指定入口点与现有手写 `.vcxproj` 行为一致，去掉会改变既有行为，故保留。
- **工作量**：低　**依赖**：无

---

## 五、结构性建议（CM-20 ~ CM-22，改动面较大，需先决策）

### CM-20 `duilib` 库层缺少 `option(DUILIB_ENABLE_CEF)` ✅ 已完成（2026-10-20）

- **位置**：修复前 [duilib/CMakeLists.txt:42-72](..//..//duilib/CMakeLists.txt#L42-L72)（include 路径）与 [duilib/CMakeLists.txt:103-119](..//..//duilib/CMakeLists.txt#L103-L119)、[247-259](..//..//duilib/CMakeLists.txt#L247-L259)（源文件收集与追加）
- **问题**：CEF 源文件在所有非 FreeBSD 平台**无条件编译**，没有库层开关；而仓库存在 `build/duilib_no_cef.sln` / `build/examples_no_cef.sln`，说明"不编译 CEF"是真实需求，却只有手写工程通道能满足。
- **动作**（已实施）：
  1. `duilib/CMakeLists.txt` 在平台判定之后新增 `option(DUILIB_ENABLE_CEF "Enable CEF" ON)`（非 Windows/Linux/macOS 即 `CEF_LIB_DIR` 为空时默认 `OFF`）；若外部强制 `ON` 但平台不支持，则降级为 `OFF` 并给出 `WARNING`；
  2. CEF include 路径、CEF 源文件收集、CEF 源文件追加三处均改由 `if(DUILIB_ENABLE_CEF)` 控制，原来各自的 `NOT FreeBSD` 判断随之简化；
  3. 关闭时给 duilib 目标加 `DUILIB_NO_CEF=1` 编译宏，并在 [duilib.cpp:6](..//..//duilib.cpp#L6) 增加该宏的判断——否则核心文件 `duilib.cpp` 仍会无条件 `#include "duilib_cef.h"` 导致编译失败（实测发现，见下）；
  4. `examples` 侧保持原有语义：`examples/cef`、`examples/CefBrowser` 在各自的 `CMakeLists.txt` 中 `set(DUILIB_ENABLE_CEF ON)`，与库层开关联动。
- **验收**：`-DDUILIB_ENABLE_CEF=OFF` 下 duilib 不编译 CEF 源、不依赖 libcef 头文件；`ON` 时行为与现状一致。
- **实测结论**：
  | 场景 | 结果 |
  | :--- | :--- |
  | 配置 `-DDUILIB_ENABLE_CEF=ON`（默认） | 输出 `CEF include path: .../libcef_win`、`CEF sources enabled (Windows)`；`duilib.vcxproj` 中 `CEFControl` 相关条目 **22** 个 |
  | 配置 `-DDUILIB_ENABLE_CEF=OFF` | 输出 `CEF includes disabled` / `CEF sources disabled`；`duilib.vcxproj` 中 `CEFControl` 相关条目 **0** 个 |
  | 构建 `-DDUILIB_ENABLE_CEF=OFF` 的 Debug x64 | 首次失败：`CefManager.h(10,14): error C1083: 无法打开包括文件 include/cef_app.h`（来源是 `duilib.cpp` 无条件包含 `duilib_cef.h`）；补上 `DUILIB_NO_CEF` 判断后**编译成功、0 error / 0 warning** |
- **工作量**：低 ~ 中　**依赖**：需与 `examples` 的 CEF 开关语义统一（已保持原语义）

### CM-21 第三方库定制方式规范化（升级可维护性）

- **位置**：`duilib/third_party/` 下 5 个库
- **问题**：四个库的定制风格各不相同——
  | 库 | 定制方式 |
  | :--- | :--- |
  | zlib | [OUTPUT_NAME "duilib-zlib"](..//..//duilib/third_party/zlib/CMakeLists.txt#L18) |
  | libpng | [set_target_properties(OUTPUT_NAME "duilib-png")](..//..//duilib/third_party/libpng/CMakeLists.txt#L745)，且并存 `png_shared`/`png_static`/`png_framework` |
  | libwebp | [add_library(duilib-webp STATIC ...)](..//..//duilib/third_party/libwebp/CMakeLists.txt#L40) |
  | cximage | [add_library(duilib-cximage STATIC ...)](..//..//duilib/third_party/cximage/CMakeLists.txt#L64) |
  | libcef | 只改 `CMAKE_ARCHIVE_OUTPUT_DIRECTORY`（[libcef_win:213](..//..//duilib/third_party/libcef/libcef_win/CMakeLists.txt#L213)） |
  另外所有改动只用中文注释 `# duilib` 标记，**没有 patch 文件或升级清单**，第三方库升级时改动会丢失。
- **动作**：统一"输出到 `<root>/lib/` + 产物名 `duilib-*` + 静态库 + 统一 `_d` 后缀（配合 CM-5）"的约定；把改动点整理成 `duilib/third_party/<lib>/README-duilib.md` 或 patch 目录，写清"改了哪几处、升级时如何重放"。
- **验收**：5 个库的定制方式一致；升级任一第三方库时，按文档能在不影响其它库的前提下重放改动。
- **工作量**：中　**依赖**：CM-2、CM-5

### CM-22 顶层 `CMakeLists.txt` 聚合（构建体系结构）

- **位置**：仓库根（当前**没有**顶层 CMakeLists）
- **问题**：每个第三方库、每个示例都要单独 `cmake -S ... -B ...`，依赖顺序与清单硬编码在 `build/msvc_build.bat`、`build/linux_build.sh`、`build/macos_build.sh`、`build/freebsd_build.sh` 里；没有统一的 `add_subdirectory` 依赖图，也没有 `install()`/`export()` 规则（产物全靠 `CMAKE_*_OUTPUT_DIRECTORY` 写进 `lib/`，与 [pack.py](../../build/pack.py) 只能靠目录约定耦合）。
- **动作（需先决策范围）**：
  1. 增加顶层 `CMakeLists.txt`：`option(DUILIB_BUILD_LIBS/EXAMPLES/TESTS)` + `add_subdirectory`，让 `cmake -S . -B build` 一把配置；
  2. 视情况增加 `install()`/`export()` 规则，与打包流程解耦；
  3. 各平台 `*_build.sh` 改为薄封装（保持命令行兼容）。
- **验收**：`cmake -S . -B <dir>` 能配置出全部目标；`cmake --build <dir>` 按依赖顺序完成；现有 4 个构建脚本行为不变或等价。
- **工作量**：中 ~ 高　**依赖**：建议在 CM-18 / CM-21 之后；需先确认是否保留"逐库单独配置"的用法

---

## 六、明确的非目标

| 项 | 说明 |
| :--- | :--- |
| 第三方上游自带的 CMake 文件 | `third_party/zlib/contrib/**`、`libpng/contrib/**`、`libcef/**`、`libjpeg-turbo/**` 的 CMake 文件保持原样，不纳入改进范围 |
| 手写 `.vcxproj` / `.sln` 工程 | `build/*.sln` 与 `examples/*/*.vcxproj` 是另一套构建通道，本文档不涉及（但 CM-5 / CM-21 的命名约定需与其保持一致） |
| macOS / Linux / FreeBSD 上的实机验证 | 相关项在对应平台标记"需平台核实"，本机（Windows）只做配置级与逻辑级验证 |
| 不追求 CMake 版本升级 | 保持 `cmake_minimum_required` 现状（3.18），不引入仅高版本可用的特性 |

---

## 七、执行顺序建议

| 阶段 | 项 | 说明 |
| :--- | :--- | :--- |
| 第 1 步 ✅ 已完成（2026-10-20） | **CM-1、CM-4、CM-6、CM-7、CM-9、CM-12、CM-16**（CM-14 已按反馈退回、不处理） | 全是极低工作量、零行为风险或纯注释类，可一批完成，一次配置 + 一次 Debug 编译即可验收 |
| 第 2 步 ✅ 已完成（2026-10-20） | **CM-2、CM-8、CM-19** | 低风险但会改动实际构建行为（链接/路径），需要一次 Windows 完整配置 + 编译验证 |
| 第 3 步 ✅ 已完成（2026-10-20） | **CM-3、CM-15** | 需 macOS（或对应平台）核实；本机仅做静态核实 + Windows 侧行为验证，macOS 实机验证待补 |
| 第 4 步 ⚠ 部分完成（2026-10-20） | **CM-5、CM-20** | CM-20 已完成；CM-5 **本轮按用户决定暂不实施**（两项动作的风险差异大，等 CM-17/CM-21 的命名约定确定后再做） |
| 第 5 步 | **CM-10、CM-11、CM-13、CM-17、CM-18、CM-21** | 重构类，改动面大，建议每项单独提交、单独验证 |
| 第 6 步 | **CM-22** | 结构性改造，需先确认是否保留逐库单独配置用法 |

> 每完成一步，把对应项在第八节打勾，并在此表标记阶段完成情况。

---

## 八、跟踪清单

> 说明：`[ ]` 未开始，`[~]` 进行中 / 已决策暂缓，`[x]` 已完成并填写日期。

**一、缺陷**

- [x] **CM-1** `duilib_common.cmake` 变量名含逗号　｜ 极低 ｜ 完成日期：2026-10-20
- [x] **CM-2** `libpng` 的 `ZLIB_LIBRARY` 路径与扩展名错误　｜ 低 ｜ 完成日期：2026-10-20
- [x] **CM-3** `duilib_cef_macos.cmake` 的 `${target}` 未定义　｜ 极低（macOS 实机验证待补）｜ 完成日期：2026-10-20
- [x] **CM-4** skill 模板缺 `DUILIB_WINRES_FILE_NAME`　｜ 极低 ｜ 完成日期：2026-10-20
- [ ] **CM-5** 多配置生成器下 `DUILIB_BUILD_TYPE` 恒为 release（含 Debug/Release 产物同名）　｜ 低~中 ｜ 依赖：CM-17/CM-21 的命名约定 ｜ 完成日期：______（2026-10-20 按用户决定**本轮暂不实施**）

**二、未定义变量与静默行为**

- [x] **CM-6** `DUILIB_MD` 未定义变量（实际 6 处 + 2 份文档）　｜ 极低 ｜ 完成日期：2026-10-20
- [x] **CM-7** `PROJECT_SOURCES` 死变量 + 冗余条件（3 处）　｜ 极低 ｜ 完成日期：2026-10-20
- [x] **CM-8** 未定义变量被链接（SDL / CEF 库变量）　｜ 极低 ｜ 完成日期：2026-10-20
- [x] **CM-9** `libwebp` 对纯 C 库加 `UNICODE` 宏　｜ 极低 ｜ 完成日期：2026-10-20

**三、可维护性与一致性**

- [ ] **CM-10** macOS 编译参数三份重复收敛　｜ 低 ｜ 完成日期：______
- [ ] **CM-11** 23 个示例的样板代码收敛　｜ 中 ｜ 完成日期：______
- [x] **CM-12** 平台判据统一（`APPLE` → `Darwin`）　｜ 极低 ｜ 完成日期：2026-10-20
- [ ] **CM-13** Linux / FreeBSD 的 freetype+fontconfig 处理统一　｜ 低 ｜ 完成日期：______
- [~] **CM-14** SDL include 路径上收到公共模块　｜ 不处理（2026-10-20 按反馈退回，保持现状，见 CM-14 小节）
- [x] **CM-15** 硬编码路径清理（WebView2 / manifest / macOS CEF_ROOT）　｜ 低（macOS 实机验证待补）｜ 完成日期：2026-10-20
- [x] **CM-16** 小项合集（6 条）　｜ 低 ｜ 完成日期：2026-10-20

**四、现代化**

- [ ] **CM-17** 全局 `include_directories` / `link_directories` 作用域化　｜ 中 ｜ 完成日期：______
- [ ] **CM-18** `aux_source_directory` → 显式源文件列表　｜ 中~高 ｜ 完成日期：______
- [x] **CM-19** 链接选项写法过时（全局 flags / `LINK_FLAGS`）　｜ 低 ｜ 完成日期：2026-10-20

**五、结构性**

- [x] **CM-20** `duilib` 库层增加 `option(DUILIB_ENABLE_CEF)`　｜ 低~中 ｜ 完成日期：2026-10-20
- [ ] **CM-21** 第三方库定制方式规范化（含升级清单）　｜ 中 ｜ 完成日期：______
- [ ] **CM-22** 顶层 `CMakeLists.txt` 聚合与 install/export　｜ 中~高 ｜ 完成日期：______

---

## 九、实施记录

### 批次 1（2026-10-20）：CM-1、CM-4、CM-6、CM-7、CM-9、CM-12、CM-16（CM-14 实施后按反馈退回、不处理）

**改动清单**

| 文件 | 改动 |
| :--- | :--- |
| `cmake/duilib_common.cmake` | CM-1 去掉变量名中的逗号；CM-12 `APPLE` → `CMAKE_SYSTEM_NAME STREQUAL "Darwin"`；CM-16-2 把 `DUILIB_MINGW_STATIC`、`DUILIB_WEBVIEW2_EXE` 的 `option()` 提到顶层；CM-16-3 修正 Skia 子目录注释；CM-16-4 日志块外移为 `include(...)` |
| `cmake/duilib_log_variables.cmake`（新增） | CM-16-4 承载原 `duilib_common.cmake` 末尾的 `DUILIB_LOG` 日志块 |
| `cmake/duilib_bin.cmake` | ~~CM-14 在 `if(DUILIB_ENABLE_SDL)` 分支内统一添加 SDL 头文件目录~~ → **已按用户反馈退回**，该文件仅保留 SDL 的 `link_directories` |
| `cmake/duilib_compiles.cmake` | CM-16-1 `include(CMakePushCheckState)` + 追加式 `CMAKE_REQUIRED_FLAGS` + `push/pop` |
| `cmake/duilib_bin_windows.cmake` | CM-6 `if(DUILIB_MD)`；CM-7 删除 Windows 专用文件内重复的平台判断 |
| `duilib/CMakeLists.txt` | CM-6 `if(DUILIB_MD)`；CM-7 删除 `PROJECT_SOURCES` 行、`if(CEF_LIB_DIR) ` 简化；顺带消除 C-7 引入的重复 CEF 日志（同义 `message` 出现两次） |
| `duilib/third_party/zlib|libpng|libwebp|cximage` 的 `CMakeLists.txt` | CM-6 `if(DUILIB_MD)`（6 处之一）；libwebp 另含 CM-9（删 `-DUNICODE -D_UNICODE`）与 CM-16-5（`cmake_minimum_required` 3.16 → 3.18） |
| `examples/ChildWindow/CMakeLists.txt` | ~~CM-14 删除个例的 SDL include 写法~~ → **已按用户反馈退回**，保留原有的个例 `include_directories` |
| `cmake/README.md`、`cmake/README.en.md` | CM-6 同步文档中的同款示例代码块 |
| `.trae/skills/nim-duilib/assets/templates/CMakeLists.txt`、`.workbuddy/skills/nim-duilib/assets/templates/CMakeLists.txt` | CM-4 补 `DUILIB_WINRES_FILE_NAME` 与「开关 / 可选变量」注释 |

**验收证据（本机：Windows 10 + cmake 4.3.0 + VS 2022 17.14，生成器 `-G"Visual Studio 17 2022"` + `-A x64`）**

| 项 | 结果 |
| :--- | :--- |
| `cmake --warn-uninitialized` 配置：`duilib`、`examples/basic`、`zlib`、`libwebp`、`cximage` | 5 个项目全部配置成功（exit 0），**0 条**未初始化变量告警 → CM-6 验收达标 |
| 同上：`libpng` | 配置成功（exit 0），但仍有未初始化告警，全部来自 libpng 上游自带脚本（`CMakeLists.txt:423 / 616 / 1040 / 1115`）与 CMP194 策略提示；属本计划「非目标」，不计入 CM-6 达标口径 |
| `duilib` Debug x64 编译 | exit 0，产出 `duilib.lib`，**0 error / 0 warning**（仅 2 条 MSB8029 环境提示：输出目录位于临时目录） |
| `libwebp` Debug x64 编译 | exit 0，产出 `duilib-webp.lib`，**0 error**（3 条 MSB8029 环境提示）→ CM-9「产物不变」达标 |
| CEF / macOS 分支日志 | `duilib` 配置日志中 `CEF support enabled (Windows)`、`CEF macOS-only sources removed (Windows is not Darwin)`、`macOS-only sources removed (Windows is not Darwin)` 各出现**一次**，无重复 |

**批次内额外发现与修正**

1. CM-6 实际命中 **6 处**（审核时记为 4 处，遗漏 `libpng/CMakeLists.txt:22`、`cximage/CMakeLists.txt:10`），另加 2 份 README 示例；正文已更正为 6 处。
2. 「已实测验证的事实」表原记录「CMake 产物在 `lib/x64/` 且无 `_d`」有误，实为：**CMake 产物在 `lib/Debug/`**（VS 多配置生成器按配置自动分子目录），**`lib/x64/` 是手写 vcxproj 的产物目录**；CM-5 第 2 点的「同名同目录互相覆盖」仅在**单配置生成器**（Linux/macOS）下成立。已更正。
3. CM-2 新增证据：`find_package(ZLIB)` 在 `ZLIB_LIBRARY` 被预置为**不存在的路径**时仍输出 `Found ZLIB: .../duilib/lib/libduilib-zlib.a (found version "1.3.2")`，不报任何错误——即该错误路径是**静默**生效的，CM-2 的判定成立（配置日志实测）。

### 批次 2（2026-10-20）：CM-2、CM-8、CM-19（CM-14 按反馈退回、不处理）

**改动清单**

| 文件 | 改动 |
| :--- | :--- |
| `duilib/third_party/libpng/CMakeLists.txt` | CM-2 重建 `ZLIB_LIBRARY`：去掉多余的 `duilib/` 层级；库名前缀/后缀改用 `CMAKE_STATIC_LIBRARY_PREFIX/SUFFIX`；配置目录用 `CMAKE_CFG_INTDIR` 拼接（多配置生成器展开为 `$(Configuration)`，单配置为 `.`） |
| `cmake/duilib_common.cmake` | CM-8 在 CEF / SDL 条件块**之前**新增 `set(DUILIB_CEF_LIBS "")` 与 `set(DUILIB_SDL_LIBS "")`，使链接处不再引用未定义变量 |
| `cmake/duilib_bin_windows.cmake` | CM-19 删除顶部 `set(CMAKE_EXE_LINKER_FLAGS ...)` 全局覆盖块；MSVC 的 `/SUBSYSTEM:WINDOWS`、`/ENTRY:wWinMainCRTStartup`、`/DELAYLOAD:libcef.dll` 改为 `target_link_options(... PRIVATE)`；MinGW 的 `-mwindows` / `-static` 移入 `elseif(DUILIB_MINGW)` 分支并改为 `target_link_options` |
| `cmake/duilib_bin.cmake`、`examples/ChildWindow/CMakeLists.txt` | CM-14 **保持现状**（未改动，已按用户反馈退回） |

**验收证据（本机：Windows 10 + cmake 4.3.0 + VS 2022 17.14，生成器 `-G"Visual Studio 17 2022"` + `-A x64`）**

| 项 | 结果 |
| :--- | :--- |
| CM-2：`libpng` 配置输出 | `ZLIB_LIBRARY: C:/develop/nim_duilib/lib/$(Configuration)/duilib-zlib.lib`；`Found ZLIB: C:/develop/nim_duilib/lib/$(Configuration)/duilib-zlib.lib (found version "1.3.2")`；`ZLIB_INCLUDE_DIR` 为仓库内 `duilib/third_party/zlib`（不再落到系统 zlib）→ CM-2 验收达标 |
| CM-2：`libpng` Release 编译 | exit 0 |
| CM-8：`--warn-uninitialized` 配置 `duilib`、`examples/basic`、`examples/ChildWindow`、`examples/CefBrowser`、`examples/WebView2Browser` | 5 项全部 exit 0，**0 条**未初始化变量告警 → CM-8 验收达标 |
| CM-19：`examples/ChildWindow` 生成的 vcxproj | 含 `<SubSystem>Windows</SubSystem>` 与 `<EntryPointSymbol>wWinMainCRTStartup</EntryPointSymbol>`；vcxproj 中已无 `CMAKE_EXE_LINKER_FLAGS` 覆盖痕迹 |
| CM-19：`examples/ChildWindow` Debug 链接 | 仅剩既有的 LNK2038 运行库不匹配（`MT_StaticRelease` vs `MTd_StaticDebug`）+ 少量 LNK4098 警告，属 CM-5 范围（VS 多配置下 `DUILIB_BUILD_TYPE` 恒为 release → 链接到 Skia release 库），**非本次改动引入** |
| CM-19：MinGW 分支 | 本机无 MinGW 工具链，未做实机验证（仅人工审查 + 分支逻辑检查） |

**批次内额外发现与修正**

1. **`$<CONFIG>` 在 `IMPORTED_LOCATION` 中不会被展开**（实测）：先用 `$<CONFIG>` 的方案，被原样写入 vcxproj 的 `AdditionalDependencies`（形如 `$<CONFIG>\x.lib`，XML 转义为 `&lt;CONFIG&gt;`），链接器无法识别；改用 `CMAKE_CFG_INTDIR` 后实测写出 `...\lib\$(Configuration)\duilib-zlib.lib`，MSBuild 可展开。已在 CM-2 小节记录该注意点。
2. `build/msvc_build.bat` 仅编译 **Release**，且第三方库走**多配置生成器**（`-G"Visual Studio 17 2022"`）→ 第三方库产物落 `lib/Release/`；脚本通过显式传 `-DDUILIB_SKIA_LIB_SUBPATH=llvm.<ARCH>.release` 规避 CM-5 的 Debug/Release 路径问题。
3. 裸 VS 生成器下构建 Debug 配置（不走 `msvc_build.bat`）实测 `examples/ChildWindow` 出现 **1694 个 LNK2038**（`_ITERATOR_DEBUG_LEVEL` 0 vs 2、`RuntimeLibrary` `MT_StaticRelease` vs `MTd_StaticDebug`）+ `LNK1319`；根因是 CM-5（多配置下 `DUILIB_BUILD_TYPE` 恒为 release）+ MT/MTd 运行库不一致，属 CM-5 处理范围。该新证据已补入 CM-5 小节潜在影响。

### 批次 3（2026-10-20）：CM-3、CM-15

**改动清单**

| 文件 | 改动 |
| :--- | :--- |
| `cmake/duilib_cef_macos.cmake` | **CM-3** `${target}` → `${CEF_TARGET}`，并把 ARC 代码块下移到 `add_executable` 之后、`CEF_COMPILER_FLAGS` 整体赋值之后（顺带修复 `-fobjc-arc` 被覆盖丢失）；**CM-15-3** `CEF_ROOT` 允许 `-DCEF_ROOT` / 环境变量覆盖，并在目录不存在时给出明确的 `FATAL_ERROR`（默认值仍为 `../../cef_binary`） |
| `cmake/duilib_bin_windows.cmake` | **CM-15-1** 新增 `DUILIB_WEBVIEW2_NATIVE_DIR`，`link_directories` 与 `WebView2Loader.dll` 复制两处改用它，并加 `EXISTS` 校验；**CM-15-2** `DUILIB_WIN_MANIFEST` 定义上移、删除重复定义、加 `EXISTS` 校验 |

**验收证据（本机：Windows 10 + cmake 4.3.0 + VS 2022，生成器 `-G"Visual Studio 17 2022"` + `-A x64`）**

| 项 | 结果 |
| :--- | :--- |
| 配置 `examples/basic`、`examples/CefBrowser`、`examples/WebView2Browser`（`--warn-uninitialized`） | 3 项全部 exit 0，**0 条**未初始化告警、0 错误 |
| `WebView2Browser.vcxproj` 中的 DLL 复制命令 | `cmake -E copy C:/develop/nim_duilib/duilib/third_party/Microsoft.Web.WebView2/build/native/x64/WebView2Loader.dll ...` → 路径来自 `DUILIB_WEBVIEW2_NATIVE_DIR` |
| `basic.vcxproj` 中的 manifest | `<AdditionalManifestFiles>C:\develop\nim_duilib\msvc\manifest\duilib.x64.manifest;</AdditionalManifestFiles>` → 路径来自 `DUILIB_WIN_MANIFEST` |
| macOS 分支（CM-3、CM-15-3） | 本机无 macOS，未实机验证；仅静态核对了 `CEF_TARGET` 定义顺序、`add_executable` 位置、`COPY_MAC_FRAMEWORK` 对 `CEF_BINARY_DIR` 的依赖与 `FindCEF.cmake` 的 `CEF_ROOT` 优先级。**待 macOS 实机验证** |

**批次内额外发现与修正**

1. **CM-15-3 的审核判断有误**：`third_party/libcef/libcef_macos` 只是 `libcef_dll_wrapper` 的源码/配置来源（含 `cmake/ include/ libcef_dll/`），**不含** `Release/Chromium Embedded Framework.framework` 与 `Resources/`；而 `COPY_MAC_FRAMEWORK` 必须从 `CEF_BINARY_DIR`（`${CEF_ROOT}/$<CONFIGURATION>`）拷贝框架。因此不能改为指向仓库内目录，实际需要的是「CEF 二进制分发包」根目录（`build_duilib_all_in_one.sh` 解压到 `nim_duilib` 同级 `cef_binary`）。已改为保留默认值 + 支持外部覆盖 + 缺失时明确报错。
2. **CM-3 连带发现两处同源缺陷**：原 `set_target_properties` 写在 `add_executable` **之前**（目标尚不存在），且 `list(APPEND CEF_COMPILER_FLAGS -fobjc-arc)` 会被随后的 `set(CEF_COMPILER_FLAGS ...)` 整体覆盖。已随 CM-3 一并修正。
3. **`DUILIB_WIN_MANIFEST` 在 `duilib_bin_windows.cmake` 内被重复定义两次**（原 L55-59 与 L67-69，内容完全相同），已合并为一处。

### 批次 4（2026-10-20）：CM-20（CM-5 按用户决定暂不实施）

**范围决策**：CM-5 的两项动作风险差异大，用户决定**本轮不做**——「动作 1」（`$<CONFIG>` 多配置 Skia 路径）虽小但会改变库路径解析逻辑；「动作 2」（`CMAKE_DEBUG_POSTFIX=_d`）波及 5 个第三方库产物命名与打包流程，且计划中本就依赖 CM-17（命名约定，第 5 步）。故本轮只实施 CM-20，CM-5 保持原状、留待 CM-17/CM-21 一同决策。

**改动清单**

| 文件 | 改动 |
| :--- | :--- |
| `duilib/CMakeLists.txt` | **CM-20** 新增 `option(DUILIB_ENABLE_CEF "Enable CEF" ON)`（平台不支持时默认 `OFF`，外部强制 `ON` 时降级并 `WARNING`）；CEF include 路径、源文件收集、源文件追加三处改由该开关控制；关闭时给目标加 `DUILIB_NO_CEF=1` |
| `duilib/duilib.cpp` | **CM-20** `#include "duilib_cef.h"` 的条件由 `!__FreeBSD__` 改为 `!__FreeBSD__ && !DUILIB_NO_CEF` |

**验收证据（本机：Windows 10 + cmake 4.3.0 + VS 2022，`-G"Visual Studio 17 2022"` + `-A x64`）**

| 项 | 结果 |
| :--- | :--- |
| `duilib` 配置（默认 `DUILIB_ENABLE_CEF=ON`） | `CEF include path: .../libcef_win`、`CEF sources enabled (Windows)`；`duilib.vcxproj` 中 `CEFControl` 条目 **22** 个（与改动前一致） |
| `duilib` 配置（`-DDUILIB_ENABLE_CEF=OFF`） | `CEF includes disabled` / `CEF sources disabled`；vcxproj 中 `CEFControl` 条目 **0** 个、无 libcef include 路径 |
| `duilib` Debug x64 编译（`DUILIB_ENABLE_CEF=OFF`） | 编译成功，**0 error / 0 warning** |
| `examples/cef` 语义 | 保持原样：示例在自身 `CMakeLists.txt` 中 `set(DUILIB_ENABLE_CEF ON)`，不受库层默认值影响 |

**批次内额外发现与修正**

1. **关闭 CEF 后核心源码仍会引用 CEF 头文件**：`duilib/duilib.cpp` 只按 `__FreeBSD__` 判断是否 `#include "duilib_cef.h"`，因此在 `DUILIB_ENABLE_CEF=OFF` 的首次编译实测失败（`CefManager.h(10,14): error C1083: 无法打开包括文件 include/cef_app.h`）。已新增 `DUILIB_NO_CEF` 宏并在该处判断，这是 CM-20 能真正落地的必要配套改动。
2. **`link_directories` 支持生成表达式（探针实测，供 CM-5 后续使用）**：在 VS 多配置生成器下 `link_directories("$<IF:$<CONFIG:Debug>,C:/tmp/dbg,C:/tmp/rel>")` 生成了 Debug 配置 `C:/tmp/dbg`、其它配置 `C:/tmp/rel`，说明 CM-5「动作 1」技术可行、不需要改用 `target_link_directories`。
