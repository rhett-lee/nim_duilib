简体中文 | [English](README.en.md)

# nim_duilib MSVC 工程与编译脚本说明

本文档详细介绍 `nim_duilib\build` 目录中 Visual Studio 解决方案文件（*.sln）和编译脚本文件（*.bat/*.sh）的用途及使用方法，供维护人员参考。

## 目录文件概览

```
build/
├── 解决方案文件（Visual Studio）
│   ├── duilib.sln                   # 基础解决方案（含lib库，不含示例程序）
│   ├── duilib_no_cef.sln            # 基础解决方案（不含CEF）
│   ├── examples.sln                 # 完整解决方案（含lib库和示例程序）
│   └── examples_no_cef.sln          # 完整解决方案（不含CEF）
│
├── 编译脚本（Windows/Batch）
│   ├── detect_vs_version.bat         # 自动检测VS版本
│   ├── msvc_build.bat                # CMake + MSVC 编译脚本
│   ├── build_duilib_all_in_one.bat   # 一键编译脚本（MSVC + LLVM）
│   ├── gcc-mingw-w64_build.bat       # CMake + MinGW-w64 gcc 编译脚本
│   ├── llvm-mingw-w64_build.bat      # CMake + MinGW-w64 clang 编译脚本
│   └── build_duilib_all_in_one_mingw-w64.bat  # 一键编译脚本（MinGW-w64）
│
├── 编译脚本（Linux/macOS/其他平台）
│   ├── build_duilib_all_in_one.sh   # Linux/macOS/FreeBSD 一键编译脚本
│   ├── linux_build.sh               # Linux 系统编译脚本
│   ├── msys2_build.sh               # MSYS2 系统编译脚本
│   ├── macos_build.sh               # macOS 系统编译脚本
│   └── freebsd_build.sh             # FreeBSD 系统编译脚本
│
└── build.md                         # 项目编译相关文档
```

---

## 解决方案文件详解

### 1. duilib.sln

**功能：** Visual Studio 基础解决方案文件，包含 duilib 核心库工程。

**包含内容：**
- duilib 核心库工程
- 第三方依赖库工程（zlib、libpng、cximage、libwebp）

**特点：**
- 不包含示例程序
- 包含 CEF 和 WebView2 支持
- 需要预先下载并编译 Skia

**适用场景：**
- 仅使用 duilib 核心库进行开发
- 不需要参考示例程序

---

### 2. duilib_no_cef.sln

**功能：** 不包含 CEF 模块的基础解决方案。

**与 duilib.sln 的区别：**
- 不支持 CEF（libCEF）模块
- WebView2 支持仍可用

---

### 3. examples.sln

**功能：** 完整的示例程序解决方案，包含 duilib 库和所有示例程序。

**包含内容：**
- duilib 核心库
- 第三方依赖库
- 完整示例程序集：
  - `basic` - 基础示例
  - `controls` - 控件示例
  - `ColorPicker` - 颜色选择器
  - `DpiAware` - DPI 感知示例
  - `chat` - 聊天界面示例
  - `layout` - 布局示例
  - `ListBox` - 列表框示例
  - `ListCtrl` - 列表控件示例
  - `MoveControl` - 移动控件示例
  - `MultiLang` - 多语言示例
  - `render` - 渲染示例
  - `RichEdit` - 富文本编辑器示例
  - `VirtualListBox` - 虚拟列表框示例
  - `threads` - 多线程示例
  - `TreeView` - 树形视图示例
  - `cef` - CEF 浏览器示例
  - `CefBrowser` - CEF 浏览器完整示例
  - `WebView2` - WebView2 示例
  - `WebView2Browser` - WebView2 浏览器完整示例
  - `ChildWindow` - 子窗口示例
  - `XmlPreview` - XML 预览示例
  - `ColorTheme` - 主题颜色预览与生成示例

**注意：** CEF 模块需要手动下载配置，参见 [docs/CEF.md](../docs/CEF.md)。

---

### 4. examples_no_cef.sln

**功能：** 完整的示例程序解决方案，不包含 CEF 相关示例。

**与 examples.sln 的区别：**
- 不包含 `cef` 和 `CefBrowser` 示例程序
- 其他所有示例程序和功能保持一致

---

## 编译脚本详解

### 1. detect_vs_version.bat（VS 版本检测）

**功能：** 自动检测系统中安装的 Visual Studio 版本。

**检测优先级：**
1. **优先使用 vswhere.exe**：通过 VS 安装工具获取最新版本
2. **目录遍历备选**：按版本从高到低遍历常见安装目录

**检测的 VS 版本：**
- VS2026 (v145)
- VS2022 (v143)
- VS2019 (v142)
- VS2017 (v141)

**输出变量：**
| 变量名 | 说明 |
|--------|------|
| `VS_VERSION` | VS 版本标识（如 vs2022） |
| `VS_PATH` | VS 安装路径 |
| `VS_MAJOR_VERSION` | 主版本号（如 17） |

**使用方式：**
```batch
call detect_vs_version.bat
echo Detected: %VS_VERSION% (%VS_MAJOR_VERSION%)
```

---

### 2. msvc_build.bat（CMake + MSVC 编译）

**功能：** 使用 CMake 和 MSVC 编译器在命令行环境下编译项目。

**前置条件：**
- Visual Studio 2022 (17.0) 或更高版本
- CMake 3.21+（如使用 VS2026 则需 CMake 4.2+）
- 预先下载并编译 Skia

**使用方法：**
```batch
cd nim_duilib\build
msvc_build.bat
```

**编译内容：**
1. 第三方库：zlib、libpng、cximage、libwebp、libcef
2. duilib 核心库
3. 所有示例程序

**Skia 路径要求：**
- 期望 Skia 编译输出在 `..\..\skia\out\llvm.x64.release`

---

### 3. build_duilib_all_in_one.bat（一键编译 - MSVC）

**功能：** 全自动编译脚本，自动下载所有依赖项（包括 Skia）并完成编译。

**前置条件：**
- git.exe
- python3.exe
- LLVM Clang 编译器（位于 `C:\LLVM\bin`）
- Visual Studio 2022 (17.0) 或更高版本

**使用方法：**
```batch
cd nim_duilib\build
build_duilib_all_in_one.bat        # 静态运行库（/MT）
build_duilib_all_in_one.bat /MD    # 动态运行库（/MD）
```

**运行流程：**
1. 检测必要软件（git、python3、clang）
2. 检测 VS 版本
3. 克隆 nim_duilib 仓库（如不存在）
4. 克隆 skia_compile 工具仓库
5. 克隆 skia 仓库
6. 应用 Skia 补丁
7. 编译 Skia（Debug/Release x64/x86）
8. 设置 duilib 运行库模式
9. 使用 devenv 编译整个解决方案

**编译输出目录：**
| 平台 | 输出目录 |
|------|----------|
| x64 Release | `..\bin\` |
| x64 Debug | `..\bin\` |
| x86 Release | `..\bin\` |
| x86 Debug | `..\bin\` |

**Skia 编译选项：**
```
llvm.x64.debug
llvm.x64.release
llvm.x86.debug
llvm.x86.release
```

---

### 4. gcc-mingw-w64_build.bat（CMake + MinGW gcc）

**功能：** 使用 CMake 和 MinGW-w64 GCC 编译器编译项目。

**前置条件：**
- MinGW-w64 GCC/G++ 编译器
- CMake 3.21+

**使用方法：**
```batch
cd nim_duilib\build
gcc-mingw-w64_build.bat        # 不启用 SDL
gcc-mingw-w64_build.bat -sdl   # 启用 SDL
```

**特点：**
- 不支持 CEF 模块
- 支持 WebView2
- 支持 SDL 可选功能

---

### 5. llvm-mingw-w64_build.bat（CMake + MinGW clang）

**功能：** 使用 CMake 和 MinGW-w64 Clang/LLVM 编译器编译项目。

**前置条件：**
- MinGW-w64 Clang/LLVM 编译器
- CMake 3.21+

**使用方法：**
```batch
cd nim_duilib\build
llvm-mingw-w64_build.bat        # 不启用 SDL
llvm-mingw-w64_build.bat -sdl   # 启用 SDL
```

**特点：**
- 使用 Clang 编译器，性能更好
- 不支持 CEF 模块
- 支持 WebView2
- 支持 SDL 可选功能

---

### 6. build_duilib_all_in_one_mingw-w64.bat（一键编译 - MinGW）

**功能：** 全自动编译脚本，使用 MinGW-w64 编译器自动下载依赖并编译。

**前置条件：**
- git.exe
- python3.exe
- cmake.exe
- GCC/G++ 或 Clang/Clang++ 编译器（二选一）

**使用方法：**
```batch
cd nim_duilib\build
build_duilib_all_in_one_mingw-w64.bat      # 不启用 SDL
build_duilib_all_in_one_mingw-w64.bat -sdl # 启用 SDL
```

**运行流程：**
1. 检测必要软件（git、python3、cmake、gcc/clang）
2. 克隆 nim_duilib 仓库
3. 克隆 skia_compile 工具仓库
4. 克隆 skia 仓库
5. 应用 Skia 补丁
6. 编译 Skia（根据编译器选择 gcc 或 clang 版本）
7. 可选：克隆并编译 SDL3
8. 调用对应的 MinGW 编译脚本

---

## 编译配置对比

| 特性 | MSVC | MinGW GCC | MinGW Clang |
|------|------|-----------|-------------|
| 编译工具 | devenv.exe | mingw32-make.exe | mingw32-make.exe |
| CEF 支持 | ✅ 支持 | ❌ 不支持 | ❌ 不支持 |
| WebView2 支持 | ✅ 支持 | ✅ 支持 | ✅ 支持 |
| SDL 支持 | ✅ 可选 | ✅ 可选 | ✅ 可选 |
| 静态运行库 | ✅ /MT | ✅ | ✅ |
| 动态运行库 | ✅ /MD | ✅ | ✅ |
| 一键编译脚本 | ✅ | ❌ | ❌ |

---

## 快速开始

### 方式一：使用 Visual Studio IDE

1. **仅编译库**：
   ```
   双击打开 duilib.sln 或 duilib_no_cef.sln
   选择配置（Debug/Release）和平台（x64/Win32）
   生成 → 重新生成解决方案
   ```

2. **编译完整示例**：
   ```
   双击打开 examples.sln 或 examples_no_cef.sln
   选择配置和平台
   生成 → 重新生成解决方案
   ```

### 方式二：使用一键编译脚本（推荐新手）

```batch
cd nim_duilib\build
build_duilib_all_in_one.bat
```

脚本会自动处理：
- 下载所有依赖
- 编译 Skia
- 编译 duilib 和所有示例

### 方式三：使用 CMake 命令行编译

```batch
cd nim_duilib\build
msvc_build.bat
```

### 方式四：Linux / macOS / FreeBSD 编译脚本

```bash
./build/linux_build.sh                    # Release（默认）
DUILIB_BUILD_TYPE=Debug ./build/linux_build.sh   # Debug
```

`macos_build.sh`、`freebsd_build.sh` 用法相同。编译临时目录按配置分到
`build_temp/<编译器>_build/<Debug|Release>/`，Debug 与 Release 互不覆盖；
Skia 在 Unix 仍只使用 release 变体。

---

## 编译输出说明

### 输出目录结构

```
nim_duilib/
├── bin/                         # 编译输出目录
│   ├── duilib.dll               # duilib DLL文件
│   ├── basic.exe                # 示例程序
│   ├── controls.exe
│   └── ...
│
├── lib/                         # 静态库目录（配置由子目录表达）
│   ├── x64/
│   │   ├── Release/
│   │   │   ├── duilib.lib
│   │   │   ├── duilib-zlib.lib
│   │   │   └── ...
│   │   └── Debug/
│   │       └── ...
│   └── x86/
│       ├── Release/
│       └── Debug/
│
├── build_temp/                   # 编译临时目录（可安全删除）
│   ├── x64/                      # VS
│   ├── msvc/                     # CMake
│   └── ...
│
└── .vs/                          # VS 缓存目录（可定期清理）
```

Linux / macOS / FreeBSD 没有架构层：`lib/Debug/`、`lib/Release/`。

### 编译目标文件说明

| 文件类型 | 路径 | 说明 |
|----------|------|------|
| 动态库 | `bin\duilib.dll` | duilib 动态链接库 |
| 静态库 | `lib\x64\<Release\|Debug>\duilib.lib` | duilib 静态库 |
| 可执行程序 | `bin\*.exe` | 示例程序（Debug 带 `_d`） |
| 资源文件 | `bin\resources\` | UI 资源文件 |

---

## 常见问题排查

### Q1：编译报 "Visual Studio 2022 or newer is required"

**原因：** 检测到系统安装的 VS 版本低于 2022。

**解决方案：**
1. 安装 Visual Studio 2022 或更高版本
2. 确保使用 `detect_vs_version.bat` 正确检测到新版本

---

### Q2：编译报 "Skia not found"

**原因：** Skia 未编译或路径不正确。

**解决方案：**
1. 使用 `build_duilib_all_in_one.bat` 自动下载并编译 Skia
2. 或手动编译 Skia，确保输出在 `..\..\skia\out\llvm.x64.release`

---

### Q3：编译报 "clang.exe not found"

**原因：** LLVM Clang 未安装或不在 PATH 中。

**解决方案：**
1. 安装 LLVM Clang（推荐版本 16+）
2. 确保 `C:\LLVM\bin` 在系统 PATH 中
3. 或修改 `build_duilib_all_in_one.bat` 中的 LLVM 路径

---

### Q4：编译报 "CMake version is too old"

**原因：** CMake 版本低于要求。

**解决方案：**
- MSVC 编译需要 CMake 3.21+（VS2026 需要 4.2+）
- 下载最新 CMake：https://cmake.org/download/

---

### Q5：CEF 示例程序无法运行

**原因：** CEF 运行时文件未正确配置。

**解决方案：**
1. 手动下载 libCEF 并配置，参见 [docs/CEF.md](../docs/CEF.md)
2. 将 libcef.dll 和相关资源复制到 `bin\libcef_win` 或 `bin\libcef_win_109`

---

### Q6：Linux 下所有文本都不显示（字体列表为空）

**原因：** Skia 未启用 fontconfig 字体后端。`FontMgr_Skia.cpp` 在非Windows / 非 macOS 平台调用
`SkFontMgr_New_FontConfig`，若 Skia 构建时未开启该后端，会**静默返回空字体集**——不报错、不崩溃，
只是所有文字都不渲染，很容易被误判为布局或主题色问题。

`build_duilib_all_in_one.sh` 的 Linux 分支（clang 与 gcc 两条路径）已显式传入
`skia_enable_fontmgr_fontconfig=true skia_use_freetype=true`，不依赖 Skia 上游默认值。
若自行编译 Skia，必须带上这两个开关。

**排查方式：** 启动日志中检索 `[WARN] FontManager::GetFontNameList`。
出现该告警即确认是字体后端问题（Windows 下输出到调试器，Linux 下经 `SDL_Log` 输出到标准错误）。

---

## 发布打包

编译完成后用 `build/pack.py` 打包，脚本只收集已构建好的文件，不参与编译。
正式发布用 `build/release.py` 编排：它调用 `pack.py`、做版本与产物校验，
但不执行 commit / tag / push。

```
# 只打SDK（推荐，默认 Release）
python build/pack.py

# Debug SDK（包名带 -debug）
python build/pack.py --config Debug

# SDK + 示例程序（包名带 -examples）
python build/pack.py --with-examples

# 只看会收集哪些文件，不实际打包
python build/pack.py --list

# 指定 MSVC 运行库（默认 auto：DLL 模式 MD，否则 MT）
python build/pack.py --msvc-runtime md

# 发布：预检、打包、校验（产物输出到 dist/<版本>/）
python build/release.py check
python build/release.py package --config Release --arch x64 --dry-run
python build/release.py finalize
python build/release.py clean --list
```

直接运行 `pack.py` 时产物输出到 `dist/`；通过 `release.py package` 时固定输出到
`dist/<版本>/`（`dist/` 已在 .gitignore 中）。包名为
`nim_duilib-<版本>-<系统>-<架构>[-sdl][-cef|-cef109][-examples][-debug]`，
内含 `include/duilib/`、
`lib/`（duilib 5 个库 + Skia 全部库 + WebView2/JPEG 等链接库）、
`bin/`（运行期 DLL 与 `bin/resources/`）、`licenses/`、`docs/`（顶层使用文档，链接已按包内布局重写），
以及 `MANIFEST.txt` 与 `SHA256SUMS`。

包内还带**下游接入层**，不需要手动拼 include 路径、库列表和宏：

- `msvc/`（仅 Windows + MSVC）：`nim_duilib.props`、`nim_duilib.targets`、
  `nim_duilib.sdk.props` 与 `manifest/`（x64、x86 两份）。下游 `.vcxproj`
  末尾导入一次 `nim_duilib.targets` 即可完成编译、链接、manifest 与运行期
  DLL 复制。
- `lib/cmake/nim_duilib/`（全平台）：`find_package(nim_duilib CONFIG REQUIRED)`
  得到 `nim_duilib::duilib`，并提供 `nim_duilib_copy_runtime(<target> [RESOURCES])`。
- 两套机制都会校验架构/配置/工具链；发布包是单架构、单配置的，Debug 与
  Release 必须各用各的包。MSVC 运行库默认按包元数据强制对齐（可关闭）。

最小接入示例（完整示例与属性说明见
[发布与打包](../docs/Release.md#下游接入机制)）：

Visual Studio，写在 `.vcxproj` 末尾、`Microsoft.Cpp.targets` 之前：

```xml
<PropertyGroup>
  <NimDuilibDir>C:\SDK\nim_duilib-1.0.2026.1020-win-x64-debug\</NimDuilibDir>
  <NimDuilibCopyResources>1</NimDuilibCopyResources>
</PropertyGroup>
<Import Project="$(NimDuilibDir)msvc\nim_duilib.targets" />
```

CMake：

```cmake
list(APPEND CMAKE_PREFIX_PATH "C:/SDK/nim_duilib-1.0.2026.1020-win-x64-debug")
find_package(nim_duilib CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE nim_duilib::duilib)
nim_duilib_copy_runtime(my_app)
```

几个容易踩的点：

- 包内头文件必须保持 `include/duilib/xxx.h` 层级，下游写法是
  `#include "duilib/duilib.h"`；压平会导致下游全部编译失败。
- 图片解码实现头文件不打包（`Image/ImageDecoder_*.h`、`Image/Image_*.h`、
  `APngDecoder.h`、`FrameSequence_gif.h` 等），内部实现头文件
  `Core/ZipStreamIO.h`、`Core/ControlDropTarget*`、`Core/WindowDropTarget_*`、
  `Core/SDL_MacOS.h`，以及只被 `.cpp` 包含的 `Core/SDL_Linux.h`、
  `Core/MessageLoop_*`、`Utils/ProcessSingleton_*`、`Utils/BitmapHelper_*`、
  `Utils/ApiWrapper_Windows.h`、`Utils/Macros_Windows.h`、`Utils/DiskUtils_Windows.h`、
  `Utils/DllManager_Windows.h`、`Utils/InlineHook_Windows.h`、
  `Control/RichEdit*_Windows/_SDL` 均不打包；`third_party` 头文件只转发 WebView2
  （仅 Windows），`zlib` / `libpng` / `giflib` / `libjpeg-turbo` 的头文件
  均不再转发（对应静态库仍随包）。
- `Core/DragWindowFilter_SDL.h` 例外：非 Windows 必带；Windows 仅在
  `--with-sdl` 时打包（`ControlDragable.h` 在 `DUILIB_BUILD_FOR_SDL` 下包含它）。
- Windows 包在 `bin/WebView2Loader.dll` 自带动态 loader 的 DLL；
  `duilib` / `duilib-cximage` / `duilib-png` / `duilib-webp` / `duilib-zlib`
  任一缺失时打包直接失败，不再只告警。
- MinGW 构建的库为 `libduilib*.a`（静态）或 `libduilib*.dll.a`（导入库），
  Skia 目录为 `mingw64-gcc.<arch>.release` / `mingw64-llvm.<arch>.release`；
  打包脚本按库目录内容自动判断工具链（MSVC/MinGW），清单中链接写法为 `-lxxx`。
- DLL 模式（存在 `msvc/PropertySheets/duilib_dll.flag`）下 `bin/duilib*.dll`
  （x64/arm64 为 `duilib[_d].dll`，x86 为 `duilib32[_d].dll`）随包，
  `lib/` 中为对应的导入库；静态模式没有这些文件，打包时自动跳过。
- `--with-sdl` 时按实际产物收集 SDL3：MSVC 静态 `SDL3-static.lib`；
  动态导入库 `SDL3.lib` / `libSDL3.dll.a` 并随包 `bin/SDL3.dll`；
  MinGW 静态 `libSDL3.a`。
- 库目录只有一套布局：Windows `lib/<arch>/<Release|Debug>/`，
  Unix `lib/<Release|Debug>/`；配置由 `--config` 选择，不再靠 `_d` 判定。
- Debug 库带调试 CRT，只能给 Debug 下游使用，混进 Release SDK 会让链接异常。
- Windows 默认不打包 SDL3，需要时加 `--with-sdl`；CEF 开发包需要 `--with-cef`。

完整说明见 [docs/Release.md](../docs/Release.md)。

## 维护注意事项


### 1. 清理编译缓存

编译过程中产生的临时文件可以安全清理：

```batch
# 清理 VS 缓存
rmdir /s /q nim_duilib\build\.vs

# 清理编译临时文件
rmdir /s /q nim_duilib\build\build_temp

# 清理 CEF 运行时缓存
rmdir /s /q nim_duilib\cef_cache

# 清理 WebView2 缓存
rmdir /s /q nim_duilib\webview2_cache
```

### 2. 切换运行库模式

**静态运行库（/MT）：**
```batch
cd nim_duilib\msvc\PropertySheets
DuilibUseStaticRuntime.bat
```

**动态运行库（/MD）：**
```batch
cd nim_duilib\msvc\PropertySheets
DuilibUseDynamicRuntime.bat
```

**重要：** 切换后必须关闭 VS 并重新打开解决方案。

### 3. 添加新的示例程序

1. 在 `examples\` 目录下创建新的工程
2. 编辑示例的 `.vcxproj` 文件，添加属性表引用：
   ```xml
   <Import Project="$(SolutionDir)\..\msvc\PropertySheets\BinCommonSettings.props" />
   ```
3. 将新工程添加到 `examples.sln` 和 `examples_no_cef.sln`

### 4. Skia 编译版本要求

duilib 依赖特定版本的 Skia，编译输出目录必须为：
```
skia/out/
├── llvm.x64.debug/
├── llvm.x64.release/
├── llvm.x86.debug/
├── llvm.x86.release/
├── mingw64-gcc.x64.release/   # MinGW GCC 编译
├── mingw64-llvm.x64.release/  # MinGW Clang 编译
└── ...
```

### 5. 支持的编译器版本

| 编译器 | 最低版本 | 推荐版本 |
|--------|----------|----------|
| Visual Studio | VS2022 (17.0) | VS2022 / VS2026 |
| LLVM Clang    |      | 16.0+ |
| MinGW GCC     |      | 13.2+ |
| MinGW Clang   |      | 16.0+ |

备注：VS2017/VS2019在develop-cpp17分支代码中支持，但主分支不支持。

---

## 版本兼容性矩阵

| 功能特性 | MSVC + VS2022/2026 | MinGW GCC | MinGW Clang | Linux GCC/Clang | macOS Clang |
|----------|-------------------|-----------|-------------|-----------------|-------------|
| 核心库 | ✅ | ✅ | ✅ | ✅ | ✅ |
| Skia 渲染 | ✅ | ✅ | ✅ | ✅ | ✅ |
| CEF 浏览器 | ✅ | ❌ | ❌ | ✅ | ✅ |
| WebView2 | ✅ | ✅ | ✅ | ❌ | ❌ |
| SDL 输入 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 静态运行库 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 动态运行库 | ✅ | ❌ | ❌ | ❌ | ❌ |
| C++20 标准 | ✅ (VS2022+) | ✅ | ✅ | ✅ | ✅ |
| C++17 标准 | ✅ (VS2017/VS2019) | ✅ | ✅ | ✅ | ✅ |

备注：VS2017/VS2019在develop-cpp17分支代码中支持，但主分支不支持。
