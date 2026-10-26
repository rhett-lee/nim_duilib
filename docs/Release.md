# 发布与打包

本文档说明如何把 nim_duilib 打包成可分发的产物，以及发布时的版本与签名流程。

打包脚本为 `build/pack.py`，它**只收集已构建好的文件，不参与编译**。

## 为什么不用 CPack

本仓库的 SDK 不是普通的安装布局：除了库与头文件，还要带下游接入层
（`msvc/*.props/.targets`、`lib/cmake/nim_duilib/`）、运行期 DLL、`bin/resources`、
`LICENSES/` 与 `THIRD-PARTY-NOTICES.md`，并按单架构、单配置命名。这些内容由
`build/pack.py` 统一收集与校验，比在根工程里维护一套 `install()` 规则更可控。
根工程因此**不提供** `install()`，SDK 只由 `pack.py` 产出。

构建统一走 `python3 build/build.py`（旧的 `msvc_build.bat`、`linux_build.sh` 等已废弃）。
macOS/Linux/FreeBSD 通常没有 `python` 命令；Windows 上可将命令中的 `python3` 换成
`python`，`build.py` 也可直接运行 `build\build.bat`。
`pack.py` 只读取 `--config` 选中的一个库目录，不再合并候选目录或按优先级猜测。

| 平台 | 库目录 |
| :--- | :--- |
| Windows | `lib/<x64\|x86\|arm64>/<Release\|Debug>/` |
| Linux / macOS / FreeBSD | `lib/<Release\|Debug>/` |

配置只由目录表达，库名不再带 `_d`。约定**同一个 `<arch>|<Config>` 一次只由
一套体系完整构建**，后构建者覆盖前者（含 `.pdb`）。

## 快速开始

打包前必须先完成对应配置的编译，脚本不会触发编译：

```
# 推荐（macOS/Linux/FreeBSD 用 python3；Windows 可运行 build\build.bat）
python3 build/build.py --config Release

# 也可以先打开 Visual Studio 解决方案（先运行 build.py deps）
build\duilib.sln      # 配置选 Release 或 Debug，平台按需
```

编译完成后打包：

```
# 只打 SDK（推荐，默认 Release）
python3 build/pack.py

# Debug SDK（包名带 -debug）
python3 build/pack.py --config Debug

# SDK + 示例程序
python3 build/pack.py --with-examples
```

直接运行 `pack.py` 时产物输出到 `dist/`；通过 `build/release.py package`
执行时固定输出到 `dist/<版本>/`。

**DLL 模式**（仅 VS/MSBuild 路径）：在 `msvc/PropertySheets/` 下新建空文件
`duilib_dll.flag` 后重新构建，duilib 工程会产出 `bin/duilib*.dll`
（x86 为 `duilib32*.dll`，Debug 带 `_d`）与 `lib/<arch>/<Config>/duilib.lib`
导入库，其余库仍为静态库；删除该文件即回到静态模式。`build.py` 构建路径默认
静态库，加 `--md` 时使用动态运行库（/MD、/MDd）。
DLL 模式的 duilib 使用 `/MD`，因此 Skia 也必须使用 `/MD` 版本：
`SkiaSettings.props` 在检测到 `duilib_dll.flag` 时改用
`skia/out/llvm.<arch>.release.md`（Debug 为 `llvm.<arch>.debug.mdd`），
两个目录均需自行构建（参数与普通版本相同，仅把 `extra_cflags` 中的
`/MT` 换成 `/MD`、`/MTd` 换成 `/MDd`）。DLL 模式下 `pack.py` 会自动随包
收集运行期 DLL、选用同一 Skia 变体，包名追加 `-dll`。
DLL 模式的下游程序依赖 MSVC 动态运行库（`msvcp140.dll`、`vcruntime140.dll` 等），
目标机器需安装与编译工具版本匹配的 Visual C++ 2015-2022 Redistributable；
静态模式（`/MT`）没有该依赖。系统自带运行库过旧时（例如 14.29 配 14.4x 编译器），
需随程序附带新版运行库，否则可能出现启动崩溃。

包名格式为
`nim_duilib-<版本>-<系统>-<架构>[-mingw-gcc|-mingw-llvm][-sdl][-cef|-cef109][-examples][-dll][-debug]`，
同一版本的不同特性包可共存：

```
dist/1.0.2026.1023/nim_duilib-1.0.2026.1023-win-x64.zip
dist/1.0.2026.1023/nim_duilib-1.0.2026.1023-win-x64-debug.zip
dist/1.0.2026.1023/nim_duilib-1.0.2026.1023-win-x64-dll.zip
dist/1.0.2026.1023/nim_duilib-1.0.2026.1023-win-x64-mingw-gcc.zip
dist/1.0.2026.1023/nim_duilib-1.0.2026.1023-win-x64-mingw-llvm.zip
dist/1.0.2026.1023/nim_duilib-1.0.2026.1023-win-x64-cef109-examples-debug.zip
```

- `-mingw-gcc` / `-mingw-llvm`：库目录为 MinGW 产物时自动追加（也可用
  `--mingw-variant` 指定）；gcc 的静态库绑定 libstdc++，llvm-mingw 绑定 libc++，
  两者不可互换，Skia 目录也必须同源。MSYS2 的 MINGW64（msvcrt）与 UCRT64（ucrt）
  生成的包名同为 `-mingw-gcc`，但 C 运行库不同，两种包同样不可互换；
  两者不能同时归档到同一目录（同名覆盖），需要分目录保存；
- `-sdl`：仅 Windows 显式 `--with-sdl` 时追加；Unix 默认带 SDL，不加后缀；
- `-cef` / `-cef109`：`--with-cef` 时按 `--cef-version` 追加；
- `-examples`：`--with-examples` 时追加；
- `-dll`：`bin/` 下存在 `duilib*.dll`（DLL 模式）时自动追加，与静态包区分；
- `-debug`：始终在最后，兼容既有 Debug 包命名。

## 产物结构

```
nim_duilib-<版本>-<系统>-<架构>[-特性]/
  VERSION               版本号，来自 duilib/duilib_version.h
  MANIFEST.txt          包内容说明与注意事项
  SHA256SUMS            每个文件的 SHA256，校验方式见“校验发布包”
  LICENSE               主许可证
  include/duilib/       对外头文件（使用 SDL 时含 include/SDL3/）
  lib/                  静态库（duilib 5 个库 + Skia 全部库 + 第三方链接库）
  LICENSES/             第三方许可证
  THIRD-PARTY-NOTICES.md 第三方组件声明
  docs/                 使用文档（仅顶层；链接已按包内布局重写）
  msvc/                 下游接入：props/targets/manifest（x64/x86/arm64 均随包）
  lib/cmake/nim_duilib/ 下游接入：CMake find_package 配置
  bin/                  运行期 DLL；--with-examples 时还含示例程序
  bin/resources/        主题、语言、字体资源（与示例程序同级）
```

### SDK 使用契约

为保证下游能完成**编译、链接、运行**，`MANIFEST.txt` 固化以下内容：

1. **包含路径**：`include/`（`#include "duilib/duilib.h"`）。
   使用 CEF 时追加 `include/duilib/third_party/libcef/<libcef_win|libcef_win_109|libcef_linux>`。
2. **库路径**：`lib/`（平面结构）。
3. **链接列表**（MSVC 为 `xxx.lib`；MinGW 产物为 `.a` / `.dll.a`，写作 `-lxxx`）：
   - 基础：`duilib`、`duilib-cximage`、`duilib-webp`、`duilib-png`、`duilib-zlib`
   - Skia：`skia`、`svg`、`skshaper`、`skottie`、`sksg`、`jsonreader`
     （`bentleyottmann`、`expat`、`skcms`、`skresources` 随包，出现未解析符号时补链）
   - WebView2：MSVC 为 `WebView2LoaderStatic`（VS）或 `WebView2Loader.dll.lib`
     （CMake）；MinGW 只随包 `WebView2Loader.dll.lib`，写作 `-lWebView2Loader.dll`
     运行期均需 `WebView2Loader.dll`
   - JPEG（Windows MSVC）：`turbojpeg-static`；MinGW 包不包含该 MSVC 静态库
   - SDL（可选）：`SDL3-static`（MSVC 静态）/ `SDL3` 导入库 + `bin/SDL3.dll`
     （动态）/ `libSDL3.a`（MinGW 静态）；Unix 为 `SDL3`
   - CEF（可选）：Windows 为 `libcef.lib` + `libcef_dll_wrapper[_109].lib`；
     Linux 为 `-Lbin/libcef_linux -lcef -lcef_dll_wrapper`（`libcef.so` 随包在
     `bin/libcef_linux/`，不重复放入 `lib/`）；macOS 不在 SDK 包内，手动配置
4. **平台系统库**：Windows 启用 SDL 时补 `Version/Winmm/Imm32/Setupapi`
   （MinGW 写作 `-lversion -lwinmm -limm32 -lsetupapi`）；
   Unix 的 `X11`/`freetype`/`fontconfig` 由系统提供，不随包。
5. **运行期**：`bin/resources/` 与可执行文件同级（位于包内 `bin/` 下）；
   CEF 运行时在 `--with-cef` 时随包（Windows 保持 `bin/libcef_win[_109]/<架构>/`，
   Linux 为 `bin/libcef_linux/`），macOS 由下游手动部署；否则需自行配置；
   WebView2 只提供 loader，依赖系统安装的 Microsoft Edge WebView2 Evergreen
   Runtime；
   duilib 以 DLL 模式构建时 `bin/duilib*.dll` 需与可执行文件同级；
   Windows 动态 SDL3 需复制 `bin/SDL3.dll`；
   Unix 的 `libSDL3.so*` 需可被找到（`LD_LIBRARY_PATH` 等）；
   macOS 的 App Bundle 由 `duilib_cef_macos.cmake` 自动复制到
   `Contents/Frameworks/` 并设置相对 rpath（见“macOS：下游手动配置 CEF”）。
6. **配置一致性**：Debug 包只给 Debug 下游，Release 包只给 Release 下游，
   两套不可混用。MSVC 运行库随包固定：静态模式为 `/MT`（Debug 为 `/MTd`），
   DLL 模式为 `/MD`（Debug 为 `/MDd`），下游默认由接入层强制对齐。

### 下游接入机制

SDK 附带两套接入层，读取同一份包元数据（架构、配置、工具链、DLL 模式、
SDL/WebView2/JPEG/CEF、MSVC 运行库、运行期 DLL），无需手工抄写库列表。
包是单架构、单配置的：Debug 工程必须用 `-debug` 包，Release 工程用正式包。

#### Visual Studio（MSVC）

**最简接入**：在 `.vcxproj` 末尾（`Microsoft.Cpp.targets` 之前）导入一次：

```xml
<Import Project="C:\SDK\nim_duilib-1.0.2026.1020-win-x64-debug\msvc\nim_duilib.targets" />
```

属性表会自动完成：

- include：`include`（CEF 时再追加 wrapper 源码根）；
- 宏：`DUILIB_SDL/CEF/WEBVIEW2/JPEG_TURBO/LIB_PAG`、`UNICODE`、DLL 模式 `DUILIB_DLL`；
- 语言与编码：`stdcpp20`、`/utf-8`；
- 链接：静态库全套或 DLL 导入库 + 系统库；
- MSVC 运行库：按包元数据强制对齐；
- manifest：按工程 Platform 嵌入 `msvc/manifest/duilib.x64.manifest`、
  `duilib.x86.manifest` 或 `duilib.arm64.manifest`（三份均随包，x64 机器上也能
  开发 x86/arm64 程序；arm64 包结构已就绪，但未做真机编译/运行验证）；
- 构建后：复制 `bin/*.dll` 到 `$(OutDir)`。

**SDK 路径不固定时**，先定义属性再导入（属性必须在 Import 之前）：

```xml
<PropertyGroup>
  <NimDuilibDir>C:\SDK\nim_duilib-1.0.2026.1020-win-x64-debug\</NimDuilibDir>
  <NimDuilibCopyResources>1</NimDuilibCopyResources>
</PropertyGroup>
<Import Project="$(NimDuilibDir)msvc\nim_duilib.targets" />
```

`NimDuilibDir` 末尾必须带反斜杠；也可以放在 `Directory.Build.props` 或同名环境变量中。

**项目文件关键片段**（顺序很重要）：

```xml
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <!-- 常规的 ProjectConfigurations / Globals 等配置 -->

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />

  <!-- 自己的源文件与工程设置 -->
  <ItemGroup>
    <ClCompile Include="main.cpp" />
  </ItemGroup>

  <!-- nim_duilib 接入：放在 Microsoft.Cpp.targets 之前 -->
  <PropertyGroup>
    <NimDuilibDir>C:\SDK\nim_duilib-1.0.2026.1020-win-x64-debug\</NimDuilibDir>
  </PropertyGroup>
  <Import Project="$(NimDuilibDir)msvc\nim_duilib.targets" />

  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
```

**只要编译/链接设置、不要构建后复制**时，改为导入 `nim_duilib.props`：

```xml
<Import Project="$(NimDuilibDir)msvc\nim_duilib.props" />
```

可用属性：

| 属性 | 默认 | 说明 |
| :--- | :--- | :--- |
| `NimDuilibDir` | `<SDK根>` | SDK 根目录，末尾带 `\` |
| `NimDuilibCopyRuntime` | `1` | 构建后复制 `bin/*.dll` |
| `NimDuilibCopyResources` | `0` | 构建后复制 `bin/resources/` 到输出目录 `resources/` |
| `NimDuilibForceRuntimeLibrary` | `1` | 对齐 MSVC 运行库；`0` 可退出 |
| `NimDuilibSkipValidation` | `0` | 跳过工具链/架构/配置/库存在性校验 |

常见报错：

| 报错 | 原因 | 处理 |
| :--- | :--- | :--- |
| 找不到 `duilib\duilib.h` | `NimDuilibDir` 未指向包根目录，或末尾缺 `\` | 指向解压后的 `nim_duilib-...` 目录 |
| SDK 架构 / 配置不匹配 | 用了另一架构或另一配置的包 | 换对应包；Debug 用 `-debug` |
| 该 SDK 是 MinGW 产物 | MSVC 工程导入了 MinGW 包 | 换 MSVC 包 |
| LNK2038 / 运行库不匹配 | 工程自己覆盖了 `RuntimeLibrary` | 保留默认对齐，或 `NimDuilibForceRuntimeLibrary=0` 后自行保证一致 |

#### CMake（全平台）

**最简 CMakeLists.txt**：

```cmake
cmake_minimum_required(VERSION 3.18)
project(my_app LANGUAGES CXX)

# SDK 解压目录（包根，不是 lib/cmake 子目录）
list(APPEND CMAKE_PREFIX_PATH "C:/SDK/nim_duilib-1.0.2026.1020-win-x64-debug")
find_package(nim_duilib CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE nim_duilib::duilib)

# 构建后把 bin/*.dll 复制到可执行文件同级
nim_duilib_copy_runtime(my_app)
```

**配置与构建**：

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug ^
      -DCMAKE_PREFIX_PATH=C:/SDK/nim_duilib-1.0.2026.1020-win-x64-debug
cmake --build build
```

也可以直接指定配置文件所在目录：

```
cmake -S . -B build -Dnim_duilib_DIR=C:/SDK/nim_duilib-...-debug/lib/cmake/nim_duilib
```

**按需复制资源 / 指定输出目录**：

```cmake
# 同时复制 bin/resources/ 到输出目录的 resources/
# （会覆盖目标目录下已有的 resources/）
nim_duilib_copy_runtime(my_app RESOURCES)

# 多目标共享输出目录时，显式指定 DESTINATION
nim_duilib_copy_runtime(my_app DESTINATION "${CMAKE_BINARY_DIR}/bin")
nim_duilib_copy_runtime(my_tool RESOURCES DESTINATION "${CMAKE_BINARY_DIR}/bin")
```

**macOS App Bundle（SDL3 动态库）**：macOS 包内的 SDL3 是动态库，构建 `.app` 时
加 `FRAMEWORKS`，脚本会把 SDK 的 `lib/libSDL3*.dylib` 复制到
`.app/Contents/Frameworks`，并把可执行文件的 rpath 设为
`@executable_path/../Frameworks`（`BUILD_WITH_INSTALL_RPATH`，优先使用 bundle
内的库；消费者已显式设置的 rpath 条目会保留在其后，因此消费者自己的 rpath
属性应在调用 `nim_duilib_copy_runtime` 之前设置）：

```cmake
add_executable(my_app MACOSX_BUNDLE main.cpp)
target_link_libraries(my_app PRIVATE nim_duilib::duilib)
nim_duilib_copy_runtime(my_app RESOURCES FRAMEWORKS)
```

目标为 `MACOSX_BUNDLE` 且未指定 `DESTINATION` 时，`RESOURCES` 复制到
`my_app.app/Contents/Resources/duilib/`（对应
`GlobalManager::GetResourceRootPath(true)`），不会落到 `Contents/MacOS/resources`。
可直接参考 `build/sdk_test/bundle_main.cpp` 与 `build/sdk_test/CMakeLists.txt`
（发布包测试的 C11 场景）获取完整可运行的 bundle 接入示例。

Helper 可执行文件位于 `Contents/Frameworks/<Helper>.app/Contents/MacOS`，需要
自行设置 rpath `@executable_path/../../..`（脚本只处理主可执行文件）。若包内是
静态 SDL（`libSDL3.a`），`FRAMEWORKS` 只给出警告、不复制文件。

**校验与开关**：`find_package` 会校验平台、工具链、架构、配置与 MSVC 运行库，
不匹配默认直接 `FATAL_ERROR`：

| 变量 | 默认 | 说明 |
| :--- | :--- | :--- |
| `NIM_DUILIB_SET_MSVC_RUNTIME` | `ON` | 按包设置 `CMAKE_MSVC_RUNTIME_LIBRARY` |
| `NIM_DUILIB_ALLOW_ARCH_MISMATCH` | `OFF` | 架构不匹配只告警（不推荐） |
| `NIM_DUILIB_ALLOW_CONFIG_MISMATCH` | `OFF` | 配置不匹配只告警（不推荐） |

**多配置生成器（VS）注意**：发布包是单配置的。用 VS 生成器时 `CMAKE_BUILD_TYPE`
不生效，构建非包配置（例如 Release 包构建 Debug）会链接到不匹配的运行库。
建议每个配置使用独立的构建目录和对应的包：

```
cmake -S . -B build-debug   -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH=C:/SDK/nim_duilib-...-debug
cmake -S . -B build-release -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH=C:/SDK/nim_duilib-...
```

CEF 运行时（`libcef.dll`、`*.pak`、`icudtl.dat`）仍需自行放到程序目录；
`nim_duilib_copy_runtime` 只复制 SDK 的 `bin/*.dll` 与 `bin/resources/`
（macOS 的 `FRAMEWORKS` 额外复制 `lib/libSDL3*.dylib` 到 App Bundle；bundle 目标上
`RESOURCES` 的目标目录为 `Contents/Resources/duilib/`）。

### 头文件路径不可改动

包内头文件保持 `include/duilib/xxx.h` 的层级，与仓库根的相对路径一致。
下游的引用写法是：

```cpp
#include "duilib/duilib.h"
```

因此 `include/` 下面必须还有一层 `duilib/`。若压平成 `include/duilib.h`，
下游全部编译失败。

`RenderSkia/` 不对外暴露 —— 渲染后端是实现细节，`duilib.h` 不引用它。

### third_party 头文件只转发必需项

包内 `Image/` 只保留对外的基础接口头文件（`Image.h`、`ImageAttribute.h`、
`ImageInfo.h`、`ImageLoadParam.h`、`ImageDecoder.h`、`ImageDecoderFactory.h`、
`ImageUtil.h`、`StateImage.h`、`StateImageMap.h`、`ImagePlayer.h`）。

具体图片解码实现头文件**不随包**，因为它们只被库内 `.cpp` 使用，
`duilib.h` 的包含链不引用：

* `Image/ImageDecoder_*.h`（PNG / GIF / JPEG / WEBP / SVG / ICO / Icon /
  LOTTIE / PAG / Common）
* `Image/Image_*.h`（PNG / GIF / JPEG / WEBP / SVG / ICO / LOTTIE / PAG /
  Bitmap / Animation）
* `Image/APngDecoder.h`、`Image/FrameSequence_gif.h`、`Image/ImageDecoderUtil.h`

`Core/ZipStreamIO.h` 同样只在库内部使用（只被 `ZipManager.cpp` 包含，
`duilib.h` 与 `ZipManager.h` 都不引用），因此也不随包。

下列平台实现头文件只被库内 `.cpp` / `.mm` 包含，也不随包：

* `Core/ControlDropTargetImpl_SDL.h`
* `Core/ControlDropTargetImpl_Windows.h`
* `Core/ControlDropTargetUtils.h`
* `Core/SDL_MacOS.h`
* `Core/WindowDropTarget_SDL.h`
* `Core/WindowDropTarget_Windows.h`
* `Core/SDL_Linux.h`、`Core/MessageLoop_SDL.h`、`Core/MessageLoop_Windows.h`
* `Utils/ProcessSingleton_Linux.h`、`Utils/ProcessSingleton_MacOS.h`、`Utils/ProcessSingleton_Windows.h`
* `Utils/BitmapHelper_SDL.h`、`Utils/BitmapHelper_Windows.h`
* `Utils/ApiWrapper_Windows.h`、`Utils/Macros_Windows.h`
* `Utils/DiskUtils_Windows.h`、`Utils/DllManager_Windows.h`、`Utils/InlineHook_Windows.h`
* `Control/RichEditHost_Windows.h`、`Control/RichEditDragSource_Windows.h`、
  `Control/RichEditDropTarget_Windows.h`、`Control/RichEditDropTarget_SDL.h`

例外：`Core/DragWindowFilter_SDL.h` 被 `Core/ControlDragable.h` 在
`DUILIB_BUILD_FOR_SDL` 下条件包含。非 Windows 平台该宏始终开启，Unix 包会保留该头文件；
Windows 包未启用 `--with-sdl` 时不打包。

对外头文件真正依赖的 `third_party` 头文件只有一处，随包转发：

| 对外头文件 | 依赖 | 说明 |
| :--- | :--- | :--- |
| `WebView2/WebView2Control.h`、`WebView2/WebView2EnvironmentOptions.h` | `Microsoft.Web.WebView2` | 仅 Windows 转发 |

相应地，`zlib`、`libpng`、`giflib`、`libjpeg-turbo` 的头文件都不再转发。
其中 `duilib-zlib`、`turbojpeg-static` 静态库仍随包（链接需要），只是不提供头文件。

上述包含关系已逐个核对：包内 `duilib/` 头文件的 `#include "duilib/..."`
全部可以解析，下游只需 `#include "duilib/duilib.h"` 即可编译。

## --with-examples 的资源范围差异

这是打包时最容易搞错的一点。`bin/resources/themes/default/` 下同时放着
公共资源和各示例的私有主题目录，两种模式收录范围不同：

| 模式 | `bin/resources/` 收录范围 |
| :--- | :--- |
| 默认（SDK） | 仅 `themes/default/public/` 与 `themes/default/global.xml` |
| `--with-examples` | 整个 `bin/resources/`，无排除项 |

`global.xml` 在 SDK 模式下**必须保留**。它是框架启动的必需文件，
`GlobalManager.cpp` 在本地文件模式下找不到它会直接 `return false`，
初始化直接失败。

SDK 模式下被排除的是各示例的私有主题目录，共 22 个：
`MultiLang`、`basic`、`cef`、`cef_browser`、`chat`、`child_window`、
`color_theme`、`controls`、`dpi_aware`、`layout`、`list_box`、`list_ctrl`、
`move_control`、`panel`、`render`、`rich_edit`、`threads`、`tree_view`、
`virtual_list_box`、`webview2`、`webview2_browser`、`xml_preview`。

`bin/resources.zip` 一律不打包 —— 它只有 748 字节，是个空壳文件。

## 命令行参数

| 参数 | 说明 |
| :--- | :--- |
| `--config Release\|Debug` | 选择库配置，默认 `Release`（大小写不敏感） |
| `--arch x64\|x86\|arm64` | 库架构；Windows 决定库目录，所有平台影响包名 |
| `--lib-dir <路径>` | 完全显式指定库目录，覆盖 `--arch`/`--config` 的路径计算 |
| `--skia-lib-dir <路径>` | Skia 静态库目录，默认按平台/工具链探测（MSVC `../skia/out/llvm.<arch>.<config>`；MinGW 为 `mingw64-gcc\|llvm.<arch>.release`（独立 MinGW-w64）或 `gcc\|llvm.<arch>.release`（MSYS2），并按目录内是否存在 `.a` 校验） |
| `--sdl-dir <路径>` | SDL3 根目录，默认 `../SDL3` |
| `--with-sdl` | Windows 下额外打包 SDL3（Unix 始终打包） |
| `--with-cef` | 打包 CEF：头文件 + `libcef.lib` + wrapper + 运行时（体积大） |
| `--cef-version latest\|109` | 配合 `--with-cef` 选择版本，默认 `latest` |
| `--msvc-runtime auto\|mt\|md` | 写入包元数据的 MSVC 运行库；默认 `auto`（DLL 模式 MD，否则 MT） |
| `--with-examples` | 包含示例程序与全部示例私有资源 |
| `--format zip\|tgz\|tarbz2` | 压缩格式，默认 Windows 用 `zip`、其它用 `tgz` |
| `--out-dir <路径>` | 输出目录，默认 `dist/` |
| `--list` | 只打印将收集的文件清单，不实际打包 |
| `--allow-missing` | 关键产物缺失时仍继续出包（仅用于排查，发布流程不要使用） |
| `--verify <包路径>` | 只校验已有包：单根目录、`MANIFEST.txt`/`SHA256SUMS` 存在性、头文件与静态库布局、全部 SHA256、SDK 文档相对链接；通过返回 0，出错返回 1 |

## Debug 包

配置只由目录决定（`lib/<arch>/Debug` 或 `lib/Debug`），不再靠 `_d` 后缀判断。

- `--config Debug` 时包名追加 `-debug`，`MANIFEST.txt` 标注 `配置 : Debug`。
- Debug 库带调试 CRT（静态 `/MTd`，DLL 模式 `/MDd`），只能用于 Debug 下游，
  **不可混进 Release SDK**。
- `--with-examples` 时：Release 包跳过 `*_d.exe`；Debug 包因 CMake 的 Debug
  exe 与 Release 同名，无法区分，脚本会告警提示可能混有 Release 版本。
- 库目录不存在或主库 `duilib` 缺失时，脚本直接以退出码 1 失败，不会打出
  没有可链接库的半成品包。

## 归档语义

- 普通文件：tar 使用源文件权限（去掉组/其他可写位，保留可执行与 setuid 等特殊位），
  Windows 源没有 POSIX 权限位，按扩展名给 `0755`/`0644`；zip 通过 `external_attr`
  记录相同的权限，解包后同样可执行。
- 符号链接：zip 写成 `S_IFLNK` 条目（内容为链接目标），tar 写成 `SYMTYPE`，
  解包后还原 `libSDL3.so -> libSDL3.so.0 -> libSDL3.so.0.5.0` 这类链接链；
  目录型链接（如 macOS Framework 的 `Versions/Current`）也会保留。
- `SHA256SUMS` 只覆盖普通文件，符号链接不计入，避免 `sha256sum -c` 跟随链接后
  校验失败。

## 校验发布包

包内 `SHA256SUMS` 覆盖除符号链接外的每个文件。在**解压后的包根目录**执行：

```bash
sha256sum -c SHA256SUMS        # Linux / macOS / MSYS2
```

```powershell
# Windows PowerShell 5+：全部通过输出 OK，否则列出失败的文件
$bad = Get-Content SHA256SUMS | ForEach-Object {
    $h, $p = $_ -split '  ', 2
    if ((Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash -ne $h) { $p }
}
if ($bad) { $bad; exit 1 } else { 'OK' }
```

也可以对单个文件执行 `Get-FileHash -Algorithm SHA256 <文件>` 或
`certutil -hashfile <文件> SHA256`，再与 `SHA256SUMS` 中的值比对。

`release.py finalize` 还会在 `dist/<版本>/` 生成压缩包级 `SHA256SUMS.txt`，
格式相同，把上面的命令换成在 `dist/<版本>/` 目录执行即可。只校验单个压缩包时：

```powershell
Get-FileHash -Algorithm SHA256 .\nim_duilib-<版本>-win-x64.zip
```

## 可复现构建

设置 `SOURCE_DATE_EPOCH`（Unix 秒）后，所有归档条目、gzip 头与 `MANIFEST.txt`
里的“打包时间”统一使用该时间（按 UTC 渲染），同样的输入在不同机器上也能得到
一致的包：

```powershell
$env:SOURCE_DATE_EPOCH = 1700000000
python3 build/release.py package --config Release --arch x64
```

```bash
SOURCE_DATE_EPOCH=1700000000 python3 build/release.py package --config Release --arch x64
```

未设置时：普通条目取源文件 mtime，生成文件与 gzip 头取收集产物中的最新 mtime，
条目按包内路径排序——输入不变时连续两次打包的字节仍然完全一致。
环境变量不是整数秒时脚本直接以退出码 1 失败。

## CEF 打包

CEF 开发文件（头文件、`libcef.lib`/wrapper）仅在 `--with-cef` 时随包；
`--cef-version 109` 仅适用于 Windows。同一开关还会收集 CEF 运行时：

- Windows：从仓库 `bin/libcef_win[/_109]/{x64,Win32}/` 收集
  `libcef.dll`、`*.pak`、`icudtl.dat`、`v8_context_snapshot.bin`、`locales/` 等，
  包内保持 `bin/libcef_win[/_109]/<架构>/` 相对结构；
- Linux：从 `bin/libcef_linux/` 收集全部运行期文件（`libcef.so`、`*.pak`、
  `icudtl.dat`、`locales/`、`chrome-sandbox` 等），包内保持
  `bin/libcef_linux/` 扁平结构；`libcef.so` 不重复放进 `lib/`，链接时使用
  `-L<SDK>/bin/libcef_linux -lcef -lcef_dll_wrapper`
  （`find_package(nim_duilib)` 会按绝对路径自动链接），运行期把该目录内容
  复制到可执行文件同级；
- 运行时目录中的 `libcef.lib` 与 `cef_sandbox.lib` 不随包（前者已在 `lib/` 提供，
  后者本项目不使用沙箱且体积很大）；
- macOS 的 CEF 开发文件与运行时都不随包（运行时以 `.app` 内嵌 Framework
  提供），下游按下一节手动配置。

包内运行时按“可执行文件目录 + `libcef_win[/_109]/<架构>`”被加载
（`CefManager::AddCefDllToPath`），因此 `--with-examples` 时 `bin` 下的
`cef`、`CefBrowser` 可直接运行；下游程序需把该子目录整体复制到可执行文件同级。
Linux 程序把 `bin/libcef_linux/` 的内容复制到可执行文件同级，并确保
`libcef.so` 可被动态加载（同目录或 `LD_LIBRARY_PATH`）。

仓库中缺少对应运行时目录时脚本只告警、不阻断出包，`MANIFEST.txt` 会注明
CEF 运行时未随包、相关示例不能直接运行。

### macOS：下游手动配置 CEF

macOS 的 CEF 运行时以 `.app` 内嵌 Framework 的形式提供，无法像 Windows/Linux 那样
放进 `bin/`，因此 SDK 包不包含 macOS 的 CEF 开发文件与运行时（macOS 的
`--with-cef` 只控制 CEF 示例 Bundle 是否随包，不收集开发文件/运行时）。macOS 使用
CEF 请**从源码树构建**，`cmake/duilib_cef_macos.cmake` 已封装 wrapper、Framework、
Helper 与资源复制；SDK 的 CMake 接入层在 macOS 上不链接 CEF，仅使用 SDK 包无法
启用 CEF。

示例程序随包同样由 `--with-cef` 控制：`pack.py --with-examples --with-cef` 时
`bin/cef.app` 与 `bin/CefBrowser.app` 作为 App Bundle 整体随包（含
`Contents/Frameworks` 内的 CEF Framework 与 Helper），解包后可直接运行；只给
`--with-examples` 时这两个 Bundle 会被跳过并告警。

#### 1. 准备 CEF 二进制分发包

- 下载与目标架构一致的 CEF 二进制分发包（x64 或 arm64，不能混用），
  版本需与 `duilib/third_party/libcef/libcef_macos` 的 wrapper 源码匹配；
- `CEF_ROOT` 指向分发包根目录（其中含 `Release/Chromium Embedded
  Framework.framework`、`Resources/`、`cmake/FindCEF.cmake`、`include/`、
  `libcef_dll/`）。解析顺序：`-DCEF_ROOT=...` > 环境变量 `CEF_ROOT` >
  源码树同级的 `cef_binary/`；
- 注意与 `duilib/third_party/libcef/libcef_macos` 区分：后者只含
  `cmake/include/libcef_dll`，用于编译 `libcef_dll_wrapper`，不含运行期
  Framework 与 `Resources/`。

#### 2. 构建命令

```
# 仓库内构建（以 examples/cef 为例）
cd examples/cef
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DCEF_ROOT=<cef_binary 根目录>
cmake --build build
```

`DUILIB_ENABLE_CEF` 在 CEF 支持的平台默认 `ON`，`examples/cef` 与
`examples/CefBrowser` 会显式设置；下游工程需要在自己的 `CMakeLists.txt`
中于 `include(.../cmake/duilib_common.cmake)` 之前设置，并沿用同一套
`cmake/duilib_bin_macos.cmake` → `duilib_cef_macos.cmake` 路径。

#### 3. 必需的目标配置

1. 主程序必须是 `MACOSX_BUNDLE`，并提供 `Info.plist`（`CFBundleIdentifier`
   全局唯一，`CFBundleExecutable` 与目标名一致）；
2. 用 `COPY_MAC_FRAMEWORK(<target> <CEF_BINARY_DIR> <app>)` 把
   `Chromium Embedded Framework.framework` 复制进
   `<app>/Contents/Frameworks/`，并生成 Framework 内的版本符号链接；
3. 子进程 Helper 不能省略：`<App>_Helper` 目标编译
   `mac/process_helper_mac.cc`（其中 `CefScopedLibraryLoader::LoadInHelper()`
   与可选的 `CefScopedSandboxContext`），产物为 `<App> Helper.app`、
   `Helper (GPU)`、`Helper (Renderer)`、`Helper (Plugin)`，统一放在
   `Contents/Frameworks/`；
4. 主进程初始化前调用 `CefScopedLibraryLoader::LoadInMain()`
   （沙箱构建不允许直接链接 libcef），Helper 名单与 `CEF_HELPER_APP_SUFFIXES`
   保持一致，否则 CEF 子进程无法启动；
5. 默认启用沙箱（`CEF_USE_SANDBOX`）；确实不需要时在配置阶段加
   `-DUSE_SANDBOX=OFF`，并同步调整 Helper 与 entitlements。
6. macOS 的 SDL3 是动态库（`cmake/duilib_deps_run.cmake` 使用
   `SDL_SHARED=ON`），`duilib_cef_macos.cmake` 会在构建后把它复制进
   `<app>/Contents/Frameworks/`，并为主程序设置 rpath
   `@executable_path/../Frameworks`、为各 Helper 设置
   `@executable_path/../../..`（Helper 的 exe 位于
   `Contents/Frameworks/<Helper>.app/Contents/MacOS`，向上三级回到外层
   `Frameworks`）。不要手工把 dylib 放到 `Contents/MacOS/`：既不符合 bundle
   约定，打包与签名也要额外处理。

#### 4. 资源目录

CEF 应用的资源根由 `GlobalManager::GetResourceRootPath(true)` 决定：
存在 `<app>/Contents/Resources/duilib/` 时以它为根，主题目录为
`themes/default/<DUILIB_THEME_DIR_NAME>`，语言与字体分别在 `lang/`、`fonts/`。
`duilib_cef_macos.cmake` 已按此复制：

```
MyApp.app/Contents/Resources/duilib/
  themes/default/public/
  themes/default/<主题目录名>/
  themes/default/global.xml
  lang/
  fonts/
```

非 CEF 或非 bundle 的 macOS 程序使用 `GetResourceRootPath(false)`，资源放在
可执行文件同级的 `resources/`。

#### 5. 成品结构（缺一不可）

```
MyApp.app/Contents/
  MacOS/MyApp
  Frameworks/Chromium Embedded Framework.framework/   # libcef + Resources（icudtl.dat、*.pak）
  Frameworks/libSDL3*.dylib                           # SDL3 动态库（libSDL3 -> libSDL3.<abi>）
  Frameworks/MyApp Helper.app/
  Frameworks/MyApp Helper (GPU).app/
  Frameworks/MyApp Helper (Renderer).app/
  Frameworks/MyApp Helper (Plugin).app/
  Resources/duilib/                                   # 主题、语言、字体
```

运行期不要拆散 Framework，也不要把它移到 `Contents/MacOS/`；`@rpath` 与
Framework 的相对位置由 `COPY_MAC_FRAMEWORK` 保证，手工移动后需重新签名。

#### 6. 签名、公证与验证

1. 由内到外签名并启用 hardened runtime：Helper → Framework（含嵌套二进制）
   → 主 app；需要 JIT 或第三方框架时补
   `com.apple.security.cs.allow-jit`、`com.apple.security.cs.disable-library-validation`
   等 entitlements（见“签名与公证 / macOS”）；
2. 未签名或签名不完整的 `.app` 会被 Gatekeeper 拒绝启动；本地自测可用
   `codesign --deep --force --sign - MyApp.app` 做临时签名；
3. 分发前自检：

   ```
   codesign --verify --deep --strict --verbose=2 MyApp.app
   codesign -dv --verbose=4 MyApp.app          # 查看签名、entitlements、runtime
   spctl -a -vv MyApp.app                      # Gatekeeper 评估（需已公证）
   otool -L "MyApp.app/Contents/MacOS/MyApp"   # 确认未残留绝对路径依赖
   otool -L "MyApp.app/Contents/MacOS/MyApp" | grep SDL3   # 期望 @rpath/libSDL3*.dylib
   otool -l "MyApp.app/Contents/MacOS/MyApp" | grep -A2 LC_RPATH
                                               # 期望 @executable_path/../Frameworks
   otool -D "MyApp.app/Contents/Frameworks/libSDL3.0.dylib"
                                               # 期望 @rpath/libSDL3.0.dylib（不是绝对路径）
   DYLD_PRINT_LIBRARIES=1 "MyApp.app/Contents/MacOS/MyApp"
                                               # 加载路径应指向 Contents/Frameworks
   ```

   复制进 bundle 的 `libSDL3*.dylib` 与 CEF Framework 一样属于嵌套二进制，
   对外发布时必须逐个签名（`codesign --force --sign <身份> .../libSDL3*.dylib`），
   再由内到外签 Helper、Framework 与主 app。

#### 7. 常见问题

| 现象 | 原因 | 处理 |
| :--- | :--- | :--- |
| 启动即退出，无窗口 | Helper 缺失或名称与主程序不匹配 | 检查 `Contents/Frameworks/* Helper*.app`，且 `CFBundleExecutable` 用 `"<App> Helper"` 形式 |
| 白屏 / 页面不加载 | Framework 未随包或被移动 | 用 `COPY_MAC_FRAMEWORK` 复制，保持 `Contents/Frameworks/` 位置 |
| `Library not loaded: @rpath/libSDL3*.dylib` | SDL3 dylib 未放进 `Contents/Frameworks`，或 rpath 仍指向构建机 | 重新构建（脚本会自动复制并设置 rpath）；用 `otool -L`/`otool -l` 检查，必要时重新签名 |
| `libcef.dylib` 加载失败 | 主进程直接链接了 libcef，或 bundle id 不一致 | 主进程用 `CefScopedLibraryLoader::LoadInMain()`；Framework 与 Helper 的 bundle id 同一前缀 |
| 只显示空窗口或崩溃在 `CefInitialize` | 未初始化 sandbox / Helper 参数不全 | 保留 `process_helper_mac.cc` 的 sandbox 初始化，或统一改为 `USE_SANDBOX=OFF` |
| 找不到主题、字体 | 资源未按 `Contents/Resources/duilib/` 布局 | 检查 `DUILIB_THEME_DIR_NAME` 与复制目标路径 |
| 提示“无法打开，因为无法验证开发者” | 未签名 / 未公证 | 按第 6 节签名并公证，或本地临时签名自测 |
| Apple Silicon 上闪退 | CEF 分发包与目标架构不一致 | 换用 arm64 分发包（或改用 x64 分发包 + Rosetta 全链路） |

## WebView2 运行时

WebView2 仅在 Windows 提供。MSVC 包随包 `bin/WebView2Loader.dll` 与
`lib/WebView2LoaderStatic.lib`、`lib/WebView2Loader.dll.lib`；MinGW 包只随包
动态 loader 的导入库 `lib/WebView2Loader.dll.lib` 与 `bin/WebView2Loader.dll`
（`WebView2LoaderStatic.lib` 是 MSVC 静态库，MinGW 无法链接）。但 **loader 不是
浏览器内核**：WebView2 依赖系统中已安装的 Microsoft Edge WebView2 Evergreen
Runtime（安装了 Microsoft Edge 的 Windows 10/11 通常已具备）。

- 使用静态 loader（默认，仅 MSVC）时不需要 `WebView2Loader.dll`，但仍需要系统运行时；
- 使用动态 loader（`DUILIB_USE_WEBVIEW2_LOADER_DLL`）时，还需把
  `bin/WebView2Loader.dll` 复制到可执行文件同级——框架只在该目录查找；
- 需要离线或固定版本时，可使用 WebView2 Fixed Version Runtime，并通过
  `WebView2Manager::Initialize(..., browserExecutableFolder)` 指向其目录。

常见问题：

- 控件空白、初始化失败、错误码 `0x80070002`：系统未安装 Evergreen Runtime，
  从 <https://developer.microsoft.com/microsoft-edge/webview2/> 安装；
- 提示找不到 `WebView2Loader.dll`：使用了动态 loader，但未把包内
  `bin/WebView2Loader.dll` 放到可执行文件同级；
- 加载失败并返回 `0x8007000B`：loader 与进程架构不匹配，需使用同架构的 SDK 包。

`MANIFEST.txt` 的运行期章节会同步标注该依赖。

## 版本号

版本号的单一事实源是 `duilib/duilib_version.h` 中的四个数字宏：
`DUILIB_VER_MAJOR`、`DUILIB_VER_MINOR`、`DUILIB_VER_PATCH`、`DUILIB_VER_BUILD`，
例如 `1.0.2026.1023`。

`pack.py` 逐行解析该头文件，用于包名、包内 `VERSION` 与 `MANIFEST.txt`；
只匹配这四个宏，忽略注释、`#undef` 与其它宏，并校验每段范围 `0..65535`。
缺失、重复定义或越界时脚本报错退出，不会打出半成品包。

`DUILIB_VER_BUILD` 按 `MMDD` 书写，但**不要写前导零**：`0105` 在 C/RC 中
是八进制，`0108`、`0109` 会直接编译失败。1 月 5 日写 `105`，版本显示为
`1.0.2026.105`，全链路保持一致。

`msvc/dllRes/version.h` 只是 Windows 资源编译的适配层，引用该头文件并补充
字符串与产品信息宏，不再保存版本数字；CMake 构建不编译 RC，不受影响。

git tag 建议用 `v<版本号>` 格式，便于与包名对应。
仓库现有 tag（`2026-06-08(main)` 等）是历史格式，含括号，
不能直接用作包名或 CPack 版本号。

## 发布流程

完整的人工步骤见 [Release-Checklist.md](misc/Release-Checklist.md)（不随发布包分发）。
自动化部分由 `build/release.py` 提供：它只调用 `pack.py` 并校验产物，
**不执行 commit、tag、push**。

1. 确认目标配置的产物已编译；更新 `duilib/duilib_version.h` 与中英 CHANGELOG
   （`CHANGELOG.md` / `CHANGELOG.en.md`），提交后工作区保持干净。
2. 预检：

   ```
   python3 build/release.py check
   ```

3. 打包（先 `--dry-run` 核对命令与包名）：

   ```
   python3 build/release.py package --config Release --arch x64 --dry-run
   python3 build/release.py package --config Release --arch x64
   ```

   产物输出到 `dist/<版本>/`；其它特性包按需追加 `--with-sdl`、`--with-cef`、
   `--with-examples` 等参数。
4. 校验全部压缩包并生成发布说明：

   ```
   python3 build/release.py finalize
   ```

   校验 SHA256、必备文件与包内文档链接，生成 `SHA256SUMS.txt`、
   `RELEASE-NOTES.md` 与 `RELEASE-NOTES.en.md`。
5. 手动提交并打 tag：

   ```
   git commit -m "release: <版本>"
   git tag -a v<版本> -m "release <版本>"
   git push origin --tags
   ```

6. 上传 `dist/<版本>/` 下的产物；历史产物用 `release.py clean --list` 查看，
   删除需显式 `--version <旧版本> --yes`（脚本拒绝删除当前版本）。

仍可直接执行 `python3 build/pack.py`，但不会做发布预检与产物校验。

## 签名与公证

当前**未自动化**，以下为手工流程说明。

### Windows

只有 exe 与 dll 能签名，静态库 `.lib` 不需要。本项目产出的是纯静态库
（`duilib/CMakeLists.txt` 中 `add_library` 未指定 `SHARED`，
`msvc/dllRes/dllmain.cpp` 只在 `.vcxproj` 中参与编译），
因此 SDK 包本身无需签名。

若发布含示例程序的包，建议对 exe 做 Authenticode 签名：

```
signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 ^
    /f <证书文件>.pfx /p <密码> dist\extracted\bin\*.exe
```

### macOS

对 `.app` 与 `.dylib` 做签名与公证：

```
codesign --force --deep --sign "Developer ID Application: <团队名>" <产物>
xcrun notarytool submit <产物.zip> --keychain-profile <profile> --wait
xcrun stapler staple <产物.zip>
```

### Linux / FreeBSD

通常不做签名。若需 GPG 签名：

```
gpg --detach-sign --armor dist/<版本>/nim_duilib-<版本>-<系统>-<架构>[-特性].tar.gz
```

发布后可用 `gpg --verify` 校验。

## 四平台差异

| 平台 | 默认格式 | 状态 |
| :--- | :--- | :--- |
| Windows | `zip` | 已实测 |
| Linux | `tgz` | 已实测（x64 Release；默认 SDK、`--with-cef`、`--with-examples`） |
| macOS | `tgz` | 脚本同构，产物未在本机验证 |
| FreeBSD | `tgz` | 已实测（x64 Release；默认 SDK、`--with-examples`；不支持 CEF） |

打包脚本本身跨平台（纯标准库，无外部依赖），四平台共用同一份代码。
Windows 的 `.lib` 与 Unix 的 `.a` 均按目录白名单收集；Unix 的 `lib` 前缀
会自动归一化（如 `libduilib-zlib.a` → `duilib-zlib`）。目前 Windows（各工具链
组合）以及 Linux、FreeBSD（x64 Release）的产物已实测；Linux/FreeBSD 尚未覆盖 Debug
配置与解包后示例启动运行。FreeBSD 不支持 CEF，`--with-cef` 会直接报错。
macOS 首次打包后仍需核对库清单。

## 相关文档

* [编译参考手册](../build/build-reference.md)
* [快速上手](Getting-Started.md)
* [示例程序](Examples.md)
* [CEF 控件](CEF.md)
