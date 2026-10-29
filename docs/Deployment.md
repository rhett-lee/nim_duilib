简体中文 | [English](Deployment.en.md)

# SDK 使用说明：安装、升级、迁移与常见问题

本文面向**发布包（SDK）的使用者**：拿到 `nim_duilib-<版本>-<系统>-<架构>...zip/tar.gz`
之后如何接入工程、部署运行期文件、升级到新版本，以及从旧做法迁移过来。

> 包内自带 `MANIFEST.txt`，记录了该包的实际内容、链接列表、宏定义与运行期要求；
> 本文是通用说明，遇到包与本文不一致时以 `MANIFEST.txt` 为准。

## 一、包内目录结构

| 目录/文件 | 说明 |
| :--- | :--- |
| `include/duilib/**.h` | 对外头文件，保持仓库相对路径，`#include "duilib/duilib.h"` |
| `include/duilib/third_party/libcef/<libcef_win\|libcef_win_109>/include/*.h` | `--with-cef` 包额外包含的 CEF 头文件；接入层把该父目录加入包含路径，按 `#include "include/cef_xxx.h"` 引用 |
| `lib/` | 静态库/导入库（`.lib` / `.a`，平面结构） |
| `bin/*.dll` | 运行期 DLL（DLL 模式为 `duilib.dll`、`turbojpeg.dll` 等） |
| `bin/resources/` | 主题、语言、字体资源，**必须与可执行文件同级** |
| `bin/libcef_win[_109]/<arch>/` | `--with-cef` 包的 CEF 运行时（`libcef.dll`、`*.pak`、`icudtl.dat`、`locales/`） |
| `bin/libcef_linux/` | Linux 包的 CEF 运行时（`libcef.so` 等） |
| `bin/*.exe`、`bin/*.app` | `-examples` 包的示例程序（macOS 为 `.app`） |
| `msvc/` | Windows + MSVC 接入：`nim_duilib.props`、`nim_duilib.targets`、manifest |
| `lib/cmake/nim_duilib/` | 全平台 CMake 接入：`find_package(nim_duilib CONFIG)` |
| `docs/` | 随包使用文档（相对链接已按包内布局重写） |
| `LICENSES/`、`THIRD-PARTY-NOTICES.md` | 第三方组件许可证与声明 |
| `VERSION`、`MANIFEST.txt`、`SHA256SUMS` | 版本号、包清单、逐文件校验和 |

包名各段含义：`-mingw-gcc` / `-mingw-llvm`（MinGW 的两个 C++ 运行时变体）、
`-sdl`、`-cef` / `-cef109`、`-examples`、`-dll`、`-debug`。
**发布包是单架构、单配置的**：Debug 与 Release、x64/x86/arm64 各用各的包，不能混用。
平台差异：Windows/Linux/macOS 包可按需带 CEF；**FreeBSD 包不含 CEF**
（上游无 Chromium 构建，`pack.py --with-cef` 直接报错），其余内容与 Linux 包同构。

## 二、安装

### 1. 解包与校验

```powershell
# Windows（PowerShell）
Expand-Archive nim_duilib-<版本>-win-x64-examples.zip -DestinationPath .
```

```bash
# Linux / macOS / FreeBSD
tar -xzf nim_duilib-<版本>-linux-x64-examples.tar.gz
cd nim_duilib-<版本>-linux-x64
sha256sum -c SHA256SUMS          # Windows 用 MANIFEST.txt 里的 PowerShell 校验片段
```

SDK 是**免安装**的：解压到任意目录即可，不需要写注册表、不需要改 `PATH`。
可以把包目录放在工程旁边（例如 `third_party/nim_duilib-sdk`），也可以放系统共享目录。

### 2. Windows + Visual Studio（MSVC）

在 `.vcxproj` 末尾导入接入文件（它会自动导入 `nim_duilib.props`）：

```xml
<Import Project="<SDK根>\msvc\nim_duilib.targets" />
```

接入文件会完成：

1. 加入 `include/`（CEF 包另加 CEF 头文件根）与 `lib/`；
2. 按包的能力写入宏：`DUILIB_SDL`、`DUILIB_CEF`、`DUILIB_WEBVIEW2`、
   `DUILIB_JPEG_TURBO`、`DUILIB_LIB_PAG=0`，DLL 包再加 `DUILIB_DLL`；
3. 写入链接列表（静态包含 Skia、`duilib-*`、可选 SDL/JPEG/WebView2/CEF，
   DLL 包只链导入库 + 系统库）与 `libcef.dll` 的延迟加载；
4. 按包元数据把 `RuntimeLibrary` 对齐到 `/MT` 或 `/MD`（`NimDuilibForceRuntimeLibrary=0` 可关闭）；
5. 按 `Platform` 嵌入包内的 Common Controls manifest；
6. 构建后把 `bin/*.dll` 复制到输出目录（`NimDuilibCopyRuntime=0` 可关闭），
   `NimDuilibCopyResources=1` 时连同 `bin/resources/` 一起复制。

导入时若架构、配置、工具链或产物不匹配，会在 `PrepareForBuild` 阶段直接报错，
不会拖到链接期才出现 `LNK2019` / `LNK2038`。只想设置编译/链接、不要复制运行期文件时，
可以只导入 `msvc\nim_duilib.props`。

### 3. CMake（全平台）

```cmake
find_package(nim_duilib CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE nim_duilib::duilib)

# 构建后复制运行期文件（Windows 复制 bin/*.dll）
nim_duilib_copy_runtime(my_app)
# 需要框架默认资源时（应用自己不带主题资源）
nim_duilib_copy_runtime(my_app RESOURCES)
```

`find_package` 找不到包时，用 `-DCMAKE_PREFIX_PATH=<SDK根>` 或
`-Dnim_duilib_DIR=<SDK根>/lib/cmake/nim_duilib` 指定。可选开关（在 `find_package`
之前设置）：

| 开关 | 作用 |
| :--- | :--- |
| `NIM_DUILIB_SET_MSVC_RUNTIME=OFF` | 不按包元数据强制 MSVC 运行库 |
| `NIM_DUILIB_ALLOW_ARCH_MISMATCH=ON` | 架构不匹配只告警（链接很可能失败） |
| `NIM_DUILIB_ALLOW_CONFIG_MISMATCH=ON` | 配置不匹配只告警 |

macOS 的 `.app` 目标可加 `FRAMEWORKS`，把 `lib/libSDL3*.dylib` 复制到
`Contents/Frameworks` 并写入 `@executable_path/../Frameworks` rpath；
`RESOURCES` 在 `.app` 目标上复制到 `Contents/Resources/duilib/resources/`
（`GlobalManager::GetResourceRootPath(true)` 定位的标准目录）。

## 三、运行期部署

| 文件/目录 | 是否必需 | 说明 |
| :--- | :--- | :--- |
| `<exe 同级>/resources/` | **必需**（应用未自带资源时） | 框架启动即读取 `global.xml` 与主题；从 `bin/` 取程序时请连同 `bin/resources/` 一起复制 |
| `WebView2Loader.dll` | 仅动态 loader 的包 | 静态 loader（`WebView2LoaderStatic.lib`）无需 DLL；WebView2 还要求系统装有 Edge WebView2 Evergreen Runtime |
| `duilib*.dll`、`turbojpeg.dll`、`SDL3.dll` | 仅 DLL / 动态 SDL 包 | 放在可执行文件同级；`lib/` 中的同名库是导入库 |
| `bin/libcef_win[_109]/<arch>/` | `--with-cef` 且用 CEF 时 | 整体复制到可执行文件同级，**不要摊平**：框架按“可执行文件目录 + 该子目录”定位 `libcef.dll` |
| MSVC 运行库 | `/MD` 构建 | 目标机需安装对应版本的 VC++ Redistributable；`/MT` 包不需要 |
| MinGW 运行库 | 下游未用 `-static` | gcc：`libstdc++-6.dll`、`libgcc_s_*.dll`、`libwinpthread-1.dll`；llvm-mingw：`libc++.dll` 等 |
| `libSDL3.so*` / `libSDL3*.dylib` | Unix / macOS | 包内位于 `lib/`；示例已写入相对 rpath，下游程序需自行设置 rpath 或 `LD_LIBRARY_PATH` / `DYLD_LIBRARY_PATH` |

## 四、升级到新版本

1. **核对矩阵**：先确认新包的系统、架构、配置、工具链、特性（SDL/CEF/CEF109/DLL）
   与当前工程一致；包名不同即不是同一个包，不能拿来直接替换。
2. **替换目录**：把新包解压到新目录，工程里的 `NimDuilibDir` / `CMAKE_PREFIX_PATH`
   指向新目录（推荐保留版本子目录，便于回滚）。
3. **清理旧产物**：删除上次构建复制到输出目录的 `bin/*.dll` 与旧 `resources/`，
   避免新旧 DLL/资源混用；CMake 工程改路径后建议清理构建目录（缓存的
   `nim_duilib_DIR` 指向旧包）。
4. **重新编译并链接**：不要只替换 DLL——头文件、库、宏定义与运行库都要同批更新。
5. **回归检查**：程序能否启动（资源是否就位）、图片/WebView2/CEF 页面是否正常、
   Debug 与 Release 是否都在用对应包。
6. 若新版本调整了包布局或库名，`MANIFEST.txt` 的“目录结构/链接列表/运行期”章节
   会明确写出差异。

## 五、从旧做法迁移

| 迁移场景 | 需要做的事 |
| :--- | :--- |
| 直接引用源码树 → 使用 SDK 包 | 头文件路径改为 `<SDK根>/include`，链接列表按 `MANIFEST.txt` 取；宏定义（`DUILIB_SDL`/`DUILIB_CEF`/…）不要手写漏项，交给 `nim_duilib.targets` 或 CMake config |
| 静态包 → DLL 包（`-dll`） | 换用 `-dll` 包并在工程里定义 `DUILIB_DLL`（接入层自动完成）；运行期随程序分发 `bin/duilib*.dll` 与 `bin/turbojpeg.dll`；`/MT` 与 `/MD` 不能混用 |
| CEF 142 包 → CEF 109 包 | 用 `-cef109` 包整体替换；封装库名由 `libcef_dll_wrapper` 变为 `libcef_dll_wrapper_109`，两者不可混链；运行时目录也是 `bin/libcef_win_109/<arch>` |
| 从源码树构建 → 消费 SDK 包 | 不再需要自己构建 Skia/SDL/CEF；但项目若用第三方依赖（WebView2、CEF）需按包能力启用对应宏 |
| 单配置工程 → Debug/Release 双配置 | 准备两份包（`-debug` 与默认 Release），按 `Configuration` 分别导入；CMake 下可把两份包放到不同前缀目录分别 configure |
| x64 → arm64（Windows） | 换 arm64 包；arm64 包不带 SDL 与 libpag（上游无预编译库）；CEF 包与 142/109 均已提供 |

## 六、常见问题（FAQ）

| 现象 | 原因 | 处理 |
| :--- | :--- | :--- |
| 程序启动即崩溃、日志提示读不到 `global.xml` / 主题 | `resources/` 没有放在可执行文件同级 | 复制 `bin/resources/` 到 `<exe 同级>/resources/`，或 `nim_duilib_copy_runtime(... RESOURCES)` / `NimDuilibCopyResources=1` |
| 启动报 `0xc000007b`（应用程序无法正常启动） | 可执行文件与 DLL 架构不一致（x86/x64/arm64 混用） | 用同一架构的包与工具链；检查输出目录里残留的旧 DLL |
| `LNK2038`（`RuntimeLibrary` / `_ITERATOR_DEBUG_LEVEL` 不匹配） | 工程与 SDK 的运行库（`/MT` vs `/MD`）或配置（Debug vs Release）不一致 | 换对应配置/运行库的包；保留接入层的强制对齐，或把工程设置改成与包一致 |
| `LNK4098`（默认库与其他库冲突） | Debug 工程链到了 Release 版第三方库，或混用了其他运行库 | 使用配套的 Debug 包；静态 Debug 下接入层已加 `/NODEFAULTLIB:"libcmt.lib"` |
| `LNK2019`：大量 `duilib-*` / Skia 符号未解析 | 只链了 `duilib.lib`，漏掉 `MANIFEST.txt` 中的 Skia 与第三方库 | 按链接列表补全，或直接使用 `nim_duilib.targets` / `nim_duilib::duilib` |
| WebView2 创建失败（`0x80070002` 等） | 系统未安装 Edge WebView2 Evergreen Runtime，或用了动态 loader 但没带 `WebView2Loader.dll` | 安装 WebView2 Runtime；把 `bin/WebView2Loader.dll` 放到可执行文件同级 |
| CEF 页面白屏 / `CefManager` 初始化失败 | CEF 运行时目录缺失、被摊平，或 142/109 文件混用 | 把 `bin/libcef_win[_109]/<arch>/` 整个目录复制到可执行文件同级；同一进程只保留一个 CEF 版本 |
| Unix 上运行示例报找不到 `libSDL3.so.0` | rpath 被改写或未设置库搜索路径 | 用包内相对 rpath（`$ORIGIN/../lib`），或设置 `LD_LIBRARY_PATH`（macOS 为 `DYLD_LIBRARY_PATH`） |
| 打包/接入时提示“SDK 是 MinGW 产物，不能用于 VS” | 混用了 MinGW 包与 MSVC 工程 | 换 MSVC 包，或改用 CMake + MinGW 工具链 |
| `sdk_package_test.py` 里 C01 显示 SKIP | 在 x64 主机上交叉验证 arm64 包，产物无法在本机运行 | 属预期；运行验证需对应架构的环境 |

## 七、相关文档

* [包内文档索引（Summary.md）](Summary.md)
* [CEF 控件使用说明（CEF.md）](CEF.md)
* 包内 `MANIFEST.txt`：目录结构、链接列表、运行期要求、校验命令
