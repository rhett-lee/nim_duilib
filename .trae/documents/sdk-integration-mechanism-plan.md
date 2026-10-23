# SDK 消费方集成机制设计（VS + CMake）

## Context

`build/pack.py` 已能生成单架构、单配置的发布包（`include/`、`lib/`、`bin/`、`resources/`、`licenses/`、`MANIFEST.txt`），但下游应用只能照着 `docs/Release.md` 手工拼 include 路径、库列表、宏、系统库和运行期 DLL。本次为发布包增加一套“接入层”：

- VS 工程：导入 property sheet/targets，自动完成 include、库、宏、系统库、DPI manifest、运行期 DLL 复制；
- CMake 工程：`find_package(nim_duilib CONFIG)` 得到 `nim_duilib::duilib`，并提供运行期复制辅助函数；
- 两套机制都由包内元数据驱动，能对配置/架构/工具链不匹配给出明确错误。

已确认的用户选择：

- CMake 包名 `nim_duilib`，target `nim_duilib::duilib`；
- 下游 MSVC 运行库默认强制对齐 SDK（可开关退出）；
- DLL 默认自动复制，`resources/` 按需复制（默认关闭）；
- CMake 覆盖 Windows（MSVC/MinGW）、Linux、macOS、FreeBSD；CEF 在 Windows 完整支持，macOS 的 CEF 框架/Helper 仍需用户手动配置。

## 包内新增文件

```
msvc/
  nim_duilib.props              # 手写模板：属性、include/lib、宏、依赖库
  nim_duilib.targets            # 手写模板：Import props + 校验 + 运行期复制
  nim_duilib.sdk.props          # pack.py 生成：包元数据
  manifest/duilib.x64|duilib.x86.manifest   # 按架构复制仓库 msvc/manifest（arm64 不带）
lib/cmake/nim_duilib/
  nim_duilibConfig.cmake        # 手写模板：校验、开关、imported target
  nim_duilibHelpers.cmake       # 手写模板：nim_duilib_copy_runtime()
  nim_duilibPackageInfo.cmake   # pack.py 生成：包元数据
  nim_duilibConfigVersion.cmake # pack.py 生成：版本兼容
```

模板存放在仓库 `packaging/msvc/` 与 `packaging/cmake/`，由 `pack.py` 读取并渲染，不额外落盘。

## 包元数据（两套格式，同一份数据）

`nim_duilibPackageInfo.cmake` 与 `nim_duilib.sdk.props` 生成以下键：

| 键 | 说明 |
|---|---|
| VERSION / CONFIG / ARCH | 完整版本、`Debug`/`Release`、`x64`/`x86`/`arm64` |
| TOOLCHAIN | `msvc` / `mingw` |
| IS_DLL | DLL 模式（包内有 `bin/duilib*.dll`） |
| HAS_SDL / SDL_KIND | 是否带 SDL3；`static`（`SDL3-static.lib`/`libSDL3.a`）或 `shared`（`SDL3.lib`/`libSDL3.dll.a`） |
| HAS_WEBVIEW2 / HAS_JPEG | 是否带 loader / turbojpeg-static |
| HAS_CEF / CEF_WRAPPER | 是否带 CEF；wrapper 名（`libcef_dll_wrapper[_109]`） |
| MSVC_RUNTIME | `MT` 或 `MD`（仅 MSVC） |
| RUNTIME_DLLS | 实际随包的 `bin/*.dll` 文件名列表 |

`IS_DLL` 由收集结果推断；`MSVC_RUNTIME` 由 `--msvc-runtime {auto,mt,md}` 控制，默认 `auto`：DLL 模式 → `MD`，否则 `MT`（因为仓库支持 `/MD` 静态构建，必须允许显式指定）。

## VS 机制要点

`nim_duilib.props`：

- `NimDuilibDir` 默认 `$(MSBuildThisFileDirectory)..\`，允许导入前覆盖；派生 include/lib 目录。
- `NimDuilibPlatform`：`Win32` → `x86`，其余取 `$(Platform)`。
- 宏全部显式写死，避免 `duilib_config.h` 在 Windows 上“未定义 `DUILIB_CEF` 时默认开启”的陷阱：
  `DUILIB_SDL`、`DUILIB_CEF`、`DUILIB_WEBVIEW2`、`DUILIB_JPEG_TURBO`、`DUILIB_LIB_PAG=0`；
  DLL 模式追加 `DUILIB_DLL`；Windows 追加 `UNICODE;_UNICODE`。
- include：`$(NimDuilibDir)include`；启用 CEF 时追加 `include\duilib\third_party\libcef\<src>`（CEF 头按 `"include/cef_xxx.h"` 引用）。
- 依赖库：
  - 静态：Skia 6 库 + `bentleyottmann/expat/skcms/skresources`（存在才加）、SDL3、`turbojpeg-static`、duilib 5 库、WebView2 静态 loader、CEF（`libcef` + wrapper）；
  - DLL：只链 `duilib.lib` 导入库 + 系统库；
  - 系统库：`Comctl32;Imm32;Opengl32;User32;shlwapi`；SDL 追加 `Version;Winmm;Setupapi`；WebView2 追加 `WinInet`；CEF 追加 `/DELAYLOAD:libcef.dll`。
- `LanguageStandard=stdcpp20`、`/utf-8`。
- `RuntimeLibrary` 按元数据设置（`NimDuilibForceRuntimeLibrary=0` 可退出）。
- `Manifest` item 可选加入 `msvc\manifest\duilib.<arch>.manifest`（`NimDuilibUseManifest=0` 关闭）。

`nim_duilib.targets`：

- 顶部 Import `nim_duilib.props`，下游只需在 `.vcxproj` 末尾导入一次：
  `<Import Project="$(NimDuilibDir)\msvc\nim_duilib.targets" />`。
- `BeforeTargets="PrepareForBuild"` 校验并 `<Error>` 中止：
  SDK 根/`include\duilib\duilib.h` 缺失；工具链为 MinGW（提示改用 CMake）；`NimDuilibPlatform` 与包架构不一致；`$(Configuration)` 与包配置不一致；请求的 CEF/SDL 库缺失。
- `AfterTargets="Build"` 复制 `bin\*.dll` 到 `$(OutDir)`（`SkipUnchangedFiles=true`）；`NimDuilibCopyResources=1` 时把 `resources\**` 复制到 `$(OutDir)resources`（默认关闭，避免覆盖应用资源）。
- 只追加 `%(...)`，不设置 `OutDir/TargetName/ConfigurationType`。

## CMake 机制要点

`nim_duilibConfig.cmake`：

- 包位于 `<prefix>/lib/cmake/nim_duilib/`，`nim_duilib_ROOT = 配置目录/../../..`。
- 校验：编译器工具链与包一致（MSVC/MinGW）；架构（`CMAKE_GENERATOR_PLATFORM`/`CMAKE_SIZEOF_VOID_P`）与包一致；单配置构建的 `CMAKE_BUILD_TYPE` 与包一致。不匹配默认 `FATAL_ERROR` 并给出修复命令，可用 `NIM_DUILIB_ALLOW_CONFIG_MISMATCH` / `NIM_DUILIB_ALLOW_ARCH_MISMATCH` 放行；多配置生成器给出“包为单配置”的警告。
- MSVC 运行库：`NIM_DUILIB_SET_MSVC_RUNTIME=ON`（默认）时，在 `CMAKE_MSVC_RUNTIME_LIBRARY` 未设置的情况下按元数据设置；已设置且不一致则报错。
- `add_library(nim_duilib::duilib INTERFACE IMPORTED GLOBAL)`：
  - `INTERFACE_INCLUDE_DIRECTORIES=<sdk>/include`；
  - `INTERFACE_COMPILE_FEATURES=cxx_std_20`；
  - `INTERFACE_COMPILE_DEFINITIONS` 同 VS（显式 0/1 + `DUILIB_DLL`）；
  - `INTERFACE_LINK_LIBRARIES` 使用**绝对库路径**（MSVC `.lib`；MinGW `lib*.a` / `lib*.dll.a`；Unix `lib*.a`、`libSDL3.so*`），按仓库契约顺序：duilib 5 库 → SDL3 → Skia → CEF → 系统库，规避 `-l` 搜索与顺序问题；
  - 平台系统库：Windows 同 VS；Linux `X11::X11` + `Freetype::Freetype` + `Fontconfig::Fontconfig` + `pthread dl`；macOS `Accelerate/CoreFoundation/CoreText/CoreGraphics` + `pthread dl` + `-framework AppKit/Foundation/Metal/Cocoa`；FreeBSD 同 Linux 的 X11/字体部分；
  - 暴露 `nim_duilib_FOUND`、`nim_duilib_ROOT`、`NIM_DUILIB_VERSION`、`NIM_DUILIB_HAS_*`、`NIM_DUILIB_MANIFEST`（Windows x64/x86 存在时）。
- CEF：Windows 完整（`libcef` + wrapper + `/DELAYLOAD`）；macOS 只链接 wrapper/`libcef`（若在包内），运行期框架/Helper 复制由文档提示用户自行处理；CEF 运行库本就不在仓库内。

`nim_duilibHelpers.cmake`：

- `nim_duilib_copy_runtime(<target> [RESOURCES] [DESTINATION <dir>])`：POST_BUILD 复制元数据中的 `runtime_dlls`；`RESOURCES` 时复制 `resources/` 到目标目录下的 `resources/`。

## `build/pack.py` 改动

1. 新增 `PACKAGING_DIR`、模板路径常量。
2. 新增 `_render_template(rel, mapping)`：读 UTF-8、替换 `@KEY@`、残留占位符即 `PackageError`。
3. 新增 `build_package_meta(...)`：汇总配置/架构/工具链/DLL/SDL/CEF/运行库/运行期 DLL。
4. 新增 `collect_integration_files(collector, meta)`：把 manifest 实体文件加入 collector，返回虚拟文件字典（props/targets/config/helpers/package-info/version）。
5. `_write_virtual(..., bom=False)`：props 写 UTF-8 BOM，其余无 BOM。
6. `write_manifest(..., virtual_files=None)`：新增“下游接入”章节；`SHA256SUMS` 覆盖虚拟文件。
7. `build_package(..., virtual_files)`：循环写入虚拟文件。
8. CLI 新增 `--msvc-runtime {auto,mt,md}`；主流程打印 `接入文件 : N`，`--list` 以 `[gen]` 前缀列出虚拟文件。
9. 文件头注释补充“下游接入层”约束。

## 文档同步

- `build/README.md` / `.en.md`：新增“SDK 下游接入”小节。
- `docs/Release.md` / `.en.md`：在“使用发布包”处给出 VS 与 CMake 的最小示例。
- `docs/misc/Release-Packaging-Notes.md` / `.en.md`：契约表新增“下游接入文件”“MSVC 运行库”“运行期复制”行，并补充本轮验证记录。
- 包内 `MANIFEST.txt`：新增“下游接入”章节（VS Import 片段、`find_package` 片段、单配置/CRT/CEF 运行期限制）。

## 实施顺序

1. 创建 `packaging/msvc/nim_duilib.props`、`packaging/msvc/nim_duilib.targets`。
2. 创建 `packaging/cmake/nim_duilibConfig.cmake`、`packaging/cmake/nim_duilibHelpers.cmake`。
3. 改造 `build/pack.py`（元数据、渲染、虚拟文件、清单、CLI）。
4. 同步上述文档。
5. 验证。

## 验证

1. `python -m py_compile build/pack.py`。
2. `python build/pack.py --list --arch x64 --config Debug`：`[gen]` 文件齐全、`接入文件` 计数正确、原有文件数不减少；再跑 `--with-sdl`、`--with-cef` 变体。
3. 伪造 MinGW 库目录：元数据为 `mingw`，不生成 `msvc/` 文件，`PackageInfo.cmake` 正确；伪造 DLL/SDL/CEF 包验证 `IS_DLL`、`SDL_KIND`、`CEF_WRAPPER`、`MSVC_RUNTIME`。
4. 用临时脚本把渲染结果写到 `%TEMP%`，用 `[xml]` 校验两份 props XML；有 `msbuild` 时对临时 `.vcxproj` 执行 `/pp` 检查 Import 链与宏展开。
5. 临时 CMake consumer：伪造 SDK 根（最小 `include/` + 空库文件 + 生成的 config），`cmake -S ... -B ... -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=...` 只配置不编译，确认 `nim_duilib::duilib` 及其属性；再分别制造架构/配置不匹配，确认报错信息。
6. 不编译 Release，不提交代码（按用户约定）。

## 风险与取舍

- **单配置包**：一个 VS 解决方案同时构建 Debug/Release 时，只能为一套配置导入对应包；校验会提前报错，文档给出按配置分别 Import 的写法。
- **MSVC 运行库**：默认强制对齐，可能覆盖下游既有设置；提供显式退出开关并记录元数据来源（`--msvc-runtime`）。
- **CEF 运行期**：`libcef.dll`、`*.pak`、`icudtl.dat`、`locales/` 不在包内，任何机制都无法自动复制；macOS 的框架/Helper 仍需手动配置。
- **资源复制**：默认关闭，避免覆盖应用自己的主题；开启时只覆盖不删除文件。
- **MinGW**：VS 不可用（校验直接报错），CMake 用绝对路径避免 `-l` 搜索差异；`-static`、libstdc++ 选择仍由用户决定。
