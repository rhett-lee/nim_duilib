# build 目录说明

本目录保存统一构建入口、出包/发布工具、单元测试与 Visual Studio 解决方案。
完整的命令、参数、依赖与排障说明见 [build-reference.md](build-reference.md)（英文版
[build-reference.en.md](build-reference.en.md)）。

## 目录文件概览

| 文件 | 说明 |
| :--- | :--- |
| `build.py` | **唯一命令行入口**：生成依赖、编译、测试、出包、清理 |
| `build.bat` | Windows 薄封装：定位 Python 3 后转调 `build.py` |
| `build-reference.md` / `.en.md` | 编译参考手册（命令、依赖、产物、排障） |
| `macos-app-bundle.md` / `.en.md` | 把示例编译为 macOS `.app` 的参考文档 |
| `pack.py` | 生成 SDK 包（只收集已构建产物，不参与编译） |
| `release.py` | 发布流程编排：预检、打包、校验（不做 commit/tag/push） |
| `autotest.py` | 本地自动化：环境检测 → 构建 → 打包 → 校验 → 结构断言 |
| `test_build.py` | 构建入口自身的单元测试 |
| `test_pack.py` / `test_autotest.py` | 打包与自动化测试 |
| `test_sdk_package.py` | SDK 包测试编排器的纯函数单元测试 |
| `sdk_package_test.py` | 发布 SDK 包测试编排器（结构校验 + 消费者编译运行 + 负向用例） |
| `sdk_test/` | SDK 消费者用例工程（仅依赖解包后的 SDK） |
| `sdk-package-test.md` | SDK 包测试方法与用途说明 |
| `duilib.sln` / `duilib_no_cef.sln` | Visual Studio 解决方案（只含库） |
| `examples.sln` / `examples_no_cef.sln` | Visual Studio 解决方案（库 + 示例） |
| `README.md` / `README.en.md` | 本文件 |

旧的分平台构建脚本已归档到 `legacy/`（不再参与构建，待全平台验证后删除；
Windows、Linux、FreeBSD 已验证，macOS 待验证），统一改用 `python3 build/build.py`。

## 前置依赖

- CMake 3.24+、Python 3（必需；Unix 上命令为 `python3`）、Git、Ninja。
- Windows：Visual Studio 2022+（C++ 工作负载），或 MSYS2 / MinGW-w64；可直接运行
  `build\build.bat`（参数与 `build.py` 相同，无需 `python3` 命令）。
- Linux/macOS/FreeBSD：gcc 10+/clang 12+，以及字体、GL、X11 等开发包；这些系统
  通常只提供 `python3`，没有 `python`。
- 首次构建前执行 `python3 build/build.py deps` 准备 Skia/SDL3/CEF。
- 下文命令中的 `python3` 在 Windows 上可换成 `python`；`build.py` 入口也可直接运行
  `build\build.bat`。

各平台系统包清单与版本要求见 [build-reference.md](build-reference.md) 第 1、8 节。

## Visual Studio 解决方案

解决方案不会自动准备依赖，打开前先运行：

```bash
python3 build/build.py deps
```

- `duilib.sln` / `examples.sln`：启用 CEF。
- `duilib_no_cef.sln` / `examples_no_cef.sln`：不编译 CEF，可缩短编译时间。
- 解决方案的中间目录仍是 `build/build_temp` 与 `build/.vs`，与新入口的
  `out/<arch>/<Config>` 互不影响。

## DLL 模式（Windows MSVC）

```bash
# duilib 编为 bin/duilib*.dll；自动准备 Skia .md/.mdd 与 SDL（/MD、/MDd）
python3 build/build.py build --dll --config Release

# Debug DLL（bin/duilib_d.dll，Skia 用 .mdd）
python3 build/build.py build --dll --config Debug

# 只有库，不编示例
python3 build/build.py build --dll --target duilib
```

`--dll` 隐式开启 `--md`，并同步 `msvc/PropertySheets/duilib_dll.flag`；不加
`--dll` 的构建会删除该 flag 并回到静态模式。切换模式时构建目录会自动清理，
CEF wrapper 与所有目标按新的 CRT 重新编译，无需手工删除 `build_temp/`。
DLL 模式包的 `pack.py` 会自动收集 `bin/duilib*.dll`，包名追加 `-dll`；
duilib DLL 实际引用 `turbojpeg.dll`（MSBuild 构建）时才随包该 DLL，且在 SDL
支持下打包必须带 `--with-sdl`。

## 构建产物与清理

| 路径 | 内容 | 是否可清理 |
| :--- | :--- | :--- |
| `out/<arch>/<Config>/` | 新入口的 CMake 构建树 | 可随时删除 |
| `lib/<arch>/<Config>/` | 编译出的库 | 重新编译生成 |
| `bin/` | 示例程序、`resources/`、CEF 运行时 | 运行必需 |
| `build/build_temp/` | VS 解决方案的中间目录 | 可清理 |
| `build/.vs/` | Visual Studio 缓存 | 可清理 |
| `cef_cache/`、`webview2_cache/` | CEF / WebView2 运行时网络缓存 | 可清理 |

```bash
python3 build/build.py clean          # 删除整个 out/
python3 build/build.py clean --deps   # 同时清理依赖构建产物
```

## 发布打包

SDK 只由 `pack.py` 生成，根 CMake 工程不提供 `install()` 目标。正式发布用
`release.py` 编排：它调用 `pack.py` 并做版本与产物校验，但不执行 commit/tag/push。

```bash
# 只打 SDK（默认 Release）
python3 build/pack.py

# Debug SDK（包名带 -debug）
python3 build/pack.py --config Debug

# SDK + 示例程序
python3 build/pack.py --with-examples

# 只看会收集哪些文件，不实际打包
python3 build/pack.py --list

# 指定 MSVC 运行库（默认 auto：DLL 模式 MD，否则 MT）
python3 build/pack.py --msvc-runtime md

# 仅排查：关键产物缺失时仍继续出包（发布流程不要使用）
python3 build/pack.py --allow-missing

# 发布：预检、打包、校验
python3 build/release.py check
python3 build/release.py package --config Release --arch x64 --dry-run
python3 build/release.py finalize
python3 build/release.py clean --list
```

也可以统一走新入口：

```bash
python3 build/build.py pack --with-cef
python3 build/build.py pack --config Debug --out-dir dist
```

包名为 `nim_duilib-<版本>-<系统>-<架构>[-sdl][-cef|-cef109][-examples][-debug]`，
内含 `include/duilib/`、`lib/<arch>/<Config>/`、`bin/`（运行期 DLL 与
`bin/resources/`）、`lib/cmake/nim_duilib/`、`LICENSES/`、`docs/`、`MANIFEST.txt`
与 `SHA256SUMS`。分发时必须保留 `LICENSES/` 与 `THIRD-PARTY-NOTICES.md`。

### 打包自动化（本地）

`autotest.py` 把「环境检测 →（可选）构建 → 打包 → 校验 → 结构断言 →（可选）冒烟」
串成可重复执行的任务，**不接入 CI**，默认不写仓库的 `bin/`、`lib/`、`dist/`，
输出与日志放在系统临时目录下的 `nim_duilib_autotest/<时间戳>/`（可用 `--out-dir` 覆盖）。

```bash
python3 build/autotest.py --preset unit                    # 单元测试
python3 build/autotest.py --preset pack-msvc-x64-release   # 用现有产物出包并体检
python3 build/autotest.py --preset arm64-synthetic         # 无 arm64 真机时验证包结构
python3 build/autotest.py --preset all-ready               # 全部前置条件满足的预设
python3 build/autotest.py --preset pack-msvc-x64-release --build  # 先构建再打包
python3 build/autotest.py --preset arm64-synthetic --dry-run      # 只打印命令
```

退出码：0 有预设通过且无失败；1 存在失败；2 全部跳过。`--list-presets` 可查看
预设与前置条件；报告写入 `report.json`，完整输出写入 `autotest.log`。

`arm64` 支持范围：接入文件与包结构已就绪，libjpeg-turbo 提供 arm64 预编译库；
但未在 arm64 真机上验证；CEF 上游没有 Windows arm64 预编译包，因此 arm64 包不含 CEF。

### 下游接入

包内自带下游接入层，不需要手动拼 include 路径、库列表和宏：

- `msvc/`（仅 Windows + MSVC）：`nim_duilib.props`、`nim_duilib.targets`、
  `nim_duilib.sdk.props` 与 `manifest/`。下游 `.vcxproj` 末尾导入一次
  `nim_duilib.targets` 即可完成编译、链接、manifest 与运行期 DLL 复制。
- `lib/cmake/nim_duilib/`（全平台）：`find_package(nim_duilib CONFIG REQUIRED)`
  得到 `nim_duilib::duilib`，并提供
  `nim_duilib_copy_runtime(<target> [RESOURCES] [FRAMEWORKS])`
  （`FRAMEWORKS` 仅 macOS App Bundle；bundle 目标上的 `RESOURCES` 复制到
  `Contents/Resources/duilib/resources/`）。可运行的 bundle 接入示例见
  `sdk_test/bundle_main.cpp` 与 `sdk_test/CMakeLists.txt`。
- 两套机制都会校验架构/配置/工具链；包是单架构、单配置的，Debug 与 Release
  必须各用各的包。MSVC 运行库默认按包元数据强制对齐（可关闭）。

最小接入示例与属性说明见 [docs/Release.md](../docs/Release.md#下游接入机制)。
几个容易踩的点：

- 包内头文件必须保持 `include/duilib/xxx.h` 层级，下游写法是
  `#include "duilib/duilib.h"`；压平会导致下游全部编译失败。
- 图片解码实现头文件、内部实现头文件不打包；`third_party` 头文件只转发 WebView2，
  `zlib` / `libpng` / `giflib` / `libjpeg-turbo` 的头文件不再转发（静态库仍随包）。
- `Core/DragWindowFilter_SDL.h` 非 Windows 必带；Windows 仅在 `--with-sdl` 时打包。
- Windows 包在 `bin/WebView2Loader.dll` 自带动态 loader；
  `duilib` / `duilib-cximage` / `duilib-png` / `duilib-webp` / `duilib-zlib`
  任一缺失时打包直接失败。
- MinGW 库为 `libduilib*.a`（静态）或 `libduilib*.dll.a`（导入库）；打包脚本按
  库目录内容自动判断工具链（MSVC/MinGW），清单中的链接写法为 `-lxxx`。
- DLL 模式（`bin/` 中存在 `duilib*.dll`，来自 `build.py --dll` 或
  `duilib_dll.flag`）下 `bin/duilib*.dll` 随包，`lib/` 中为导入库；包名自动追加
  `-dll`。DLL 模式使用 `/MD`，Skia 必须同 CRT。duilib DLL 以 SDL 编译时打包
  必须加 `--with-sdl`（`pack.py` 从 DLL 导出表检测，缺失时直接报错）；JPEG 只在
  该 DLL 实际引用 `turbojpeg.dll` 时收集（MSBuild 产物才有，CMake 当前未启用）。
- `--with-sdl` 时按实际产物收集 SDL3；Windows 默认不打包 SDL3。
- CEF 需要 `--with-cef`：Windows 运行时按 `bin/libcef_win[/_109]/<架构>/` 随包，
  Linux 按扁平 `bin/libcef_linux/` 随包；`--cef-version 109` 仅 Windows。
- 归档权限与符号链接：tar 保留源权限；zip 用 `S_IFLNK` 保存符号链接，符号链接
  不计入 `SHA256SUMS`。
- 可复现构建：`SOURCE_DATE_EPOCH` 统一条目时间、gzip 头与 MANIFEST 时间。

完整说明见 [docs/Release.md](../docs/Release.md)。

## 排障索引

| 现象 | 处理 |
| :--- | :--- |
| `Visual Studio not found` | 安装 VS 2022+ 与 C++ 工作负载，或用 `--vs-version` |
| `Skia not found` | 先运行 `python3 build/build.py deps` |
| `gn/ninja not found` | Windows 使用 VS 自带目录；其他平台安装 `ninja`、`gn` |
| CEF 示例无法运行 | 确认 `bin/libcef_win/<arch>` 或 `bin/libcef_linux` 含运行时 |
| Linux 文本不显示 | 安装 fontconfig 与中文字体，确认 `bin/resources/fonts` |
| 构建目录异常 | `python3 build/build.py clean` 后重试 |

## 维护注意事项

### 添加新的示例程序

1. 在 `examples/<名称>/` 下创建 `CMakeLists.txt` 与源码；
2. 在 `cmake/duilib_root_options.cmake` 的示例列表中登记名称；
3. 需要 CEF 的示例与 CEF 默认开关保持一致。

### Skia 变体与运行库

Skia 变体目录由架构、工具链与运行库共同决定（如 `llvm.x64.release`、
`llvm.x64.debug`、`mingw64-gcc.x64.release`）。切换 `--md` 或工具链时 `deps`
会生成对应变体，`pack.py` 按目录内容自动识别。

### 支持的编译器版本

MSVC 19.30+（VS 2022）、gcc 10+、clang 12+；MSVC 2017/2019 仅在
`develop-cpp17` 分支支持，且 VS 2017 不支持 CEF。
