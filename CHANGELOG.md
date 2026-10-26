# 更新日志

本项目的所有重要变更记录在此文件中，格式参考
[Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)。

版本号由 `duilib/duilib_version.h` 统一维护，格式为
`MAJOR.MINOR.PATCH.BUILD`，其中 `BUILD` 按 `MMDD` 书写且不补零。

## [Unreleased]

### 新增

- 根 CMake 工程与统一构建入口 `python build/build.py`（Windows 可用
  `build/build.bat`）：`build`（默认）、`deps`、`test`、`pack`、`doctor`、`clean`；
  构建树统一为 `out/<arch>/<Config>`。
- 依赖自动准备：版本清单 `cmake/third_party_versions.json` 锁定 Skia、SDL3 与 CEF，
  支持下载缓存、sha256 校验、断点续传与并发锁；已有产物可按 stamp 直接采纳。
- 缺少 Skia 补丁归档时，`build.py deps` / `build` 自动克隆整个
  [skia_compile](https://github.com/rhett-lee/skia_compile) 仓库到 `skia_compile/`；
  仓库已存在则 `git pull` 更新，失败时继续使用本地内容（与旧脚本一致）；
  `--offline` 下不联网，需提前自备。直接用 CMake 构建（`DUILIB_DEPS_MODE=auto`
  或 `duilib_deps` 目标）时也会自动完成同样的克隆。
- 网络操作统一重试：`git clone/fetch`、CEF 下载共用版本清单 `retry` 段
  （`maxAttempts=0` 无限重试、`delaySeconds=20`）。
- 双语构建参考手册 `build/build-reference.md` / `.en.md`；`build/README.md` /
  `.en.md` 改为前置依赖与发布打包索引。
- `build/test_build.py`：构建入口与 i18n 的标准库单元测试。
- 合规：`THIRD-PARTY-NOTICES.md`，SDK 包内许可证目录统一为 `LICENSES/`。

### 变更

- 统一入口一次只构建一个 `--config`（Debug/Release），需要两者时调用两次；
  `build/autotest.py` 的构建步骤改为调用 `build.py`。
- CEF 运行时按版本与架构输出到 `bin/libcef_win[/_109]/<arch>` 或 `bin/libcef_linux`，
  编译使用仓库跟踪的头文件与导入库。所有平台统一使用 `_minimal` 发行包（体积约为
  标准包的一半，含 `include/`、`Release/`、`Resources/`）。
- `doctor` 的依赖状态报告更完整：分别列出 Skia 源码与补丁归档、SDL 源码与前缀、
  CEF 运行时，缺失时说明由 `deps` 获取，`--offline` 下则提示无法获取；解释器名称
  按实际命令显示（Linux 上为 `python3`）。

### 移除

- 删除 `build/build.md`，构建说明合并到 `build/build-reference.md`。
- 旧分平台构建脚本归档到 `build/legacy/`（不再参与构建，全平台验证后移除）。

### 修复

- 依赖准备：Skia 源码仓库改为与旧脚本一致的 `https://github.com/google/skia.git`，
  原先的 `skia.googlesource.com` 在部分网络下无法访问；Skia/SDL 的克隆地址统一从
  版本清单读取，不再在脚本里硬编码。
- `build.py`：写在子命令之前的选项（如 `--config Debug build`）会被子命令默认值覆盖，
  现已在子命令侧抑制默认值。
- `build.py`：同一 `out/<arch>/<Config>` 切换编译器（MSVC ↔ MinGW）时复用旧 CMake
  缓存导致配置失败；现在检测到编译器变化会自动清空该构建目录。
- 依赖准备：Unix 上会误选 Skia 目录里的 Windows 版 `gn.exe`/`ninja.exe`；且回退到
  PATH 查找时结果变量已被置空，`find_program` 直接跳过搜索，最终报“找不到 gn”。
  现在只在 Windows 选用 `.exe`，Unix 使用本机工具并用独立临时变量查找 PATH。
- `build.py`：`vcvarsall` 输出的 `Path` 与继承的 `PATH` 并存，导致工具查找读到旧值。
- `build.py doctor`：预检以 `DUILIB_DEPS_MODE=off` 运行时整体跳过依赖工具检查，输出的
  环境摘要里没有 `gn`。现在 `gn` 始终检查，doctor 按 `prepare` 严格校验（缺失即失败）；
  `build --no-deps` 的 `off` 模式下缺失只告警，不阻断复用已有依赖的构建。
- 预检：非 Windows 平台新增编译器检查，要求 `gcc/g++` 或 `clang/clang++` 至少一组成对
  可用；有 `CC`/`CXX` 时按环境变量检查，否则先查 gcc、再查 clang。
- 依赖准备：Skia 的构建脚本固定调用 `python3`（不认 `python` / `py -3`），而预检此前
  接受 `python` 或 `py -3`，导致问题到编译 Skia 时才暴露。现在缺少 `python3` 时预检
  给出告警，并在实际编译 Skia 前报错终止。
- 文档与命令示例：macOS/Linux/FreeBSD 通常只有 `python3`，按原文档执行
  `python build/build.py` 会报 command not found；示例已统一为
  `python3 build/build.py`，Windows 仍可直接运行 `build\build.bat`。
- 依赖准备（macOS）：SDL3 已成功编译安装后仍报 `SDL3 build failed`；就绪检查只认
  `libSDL3.so`，而 macOS 的安装产物是 `libSDL3.dylib`。现在 macOS 平台按 dylib 判断。
- CEF 示例（macOS）：各 Helper 子目标未应用 duilib 的 include/link 目录，编译时报
  `fatal error: 'duilib/duilib.h' file not found`；现在 Helper 目标与主目标使用相同的
  目录设置和链接依赖。
- 根 CMake 工程（macOS）：CEF wrapper（`libcef_dll_wrapper`）未随工程编译，而 CEF 示例
  链接时需要它；现在 macOS 也会构建 wrapper，产物输出到 `lib/<Config>`。
- `build.py build --arch arm64`：目标架构的 MSVC 工具集未安装时，vcvarsall 仍返回成功，
  CMake 只会报“C compiler identification is unknown”；现在初始化环境后检查 `cl`，
  缺失时直接提示 `Visual Studio C++ tools for arm64 are not installed` 并给出安装位置。
- `build.py clean`：删除构建树遇到文件被占用时抛出 Python 异常堆栈；现在输出
  带路径的明确错误，并提示关闭占用程序后重试。
- `build.py build --md`（MSVC）：Skia 目录候选没有区分 MSVC 运行库，`--md` 仍选到
  `/MT` 编译的 `llvm.<arch>.debug`，配置期报 CRT mismatch；现在 Debug 选
  `llvm.<arch>.debug.mdd`、Release 选 `llvm.<arch>.release.md`，`deps` 的候选命名同步修正。
- `--md`（MSVC）生成的运行库表达式错误：`MultiThreadedDLL$<$<CONFIG:Debug>:Debug>`
  在 Debug 展开为非法值 `MultiThreadedDLLDebug`，Generate 阶段直接失败；改为
  `MultiThreaded$<$<CONFIG:Debug>:Debug>DLL`（根工程、duilib、zlib、libpng、libwebp、
  cximage 与文档示例同步修正）。
- Skia 目录选择（MSYS2 CLANG64）：`llvm.<arch>.debug` 这类目录名与 MSVC 产物重名，
  但其中只有 `.lib`；现在 MinGW 只接受包含 `.a` 的候选目录，会按顺序回退到
  `mingw64-llvm.<arch>.release`，避免配置期误判后报“没有 .a 静态库”。
- CEF 示例（macOS/Linux）：wrapper 的链接名 `cef_dll_wrapper` 与目标名
  `libcef_dll_wrapper` 不一致，`target_link_libraries` 不会为它建立构建依赖，并行构建
  时 Helper 会在 wrapper 生成前链接，报 `ld: library 'cef_dll_wrapper' not found`；
  现在 wrapper 目标额外提供同名 ALIAS `cef_dll_wrapper`，链接侧按其引用即可获得绝对
  路径与构建依赖（链接名保持不变，不影响打包）。
- 根 CMake 工程：libpng/zlib 的目标名由 `png_static`/`zlibstatic` 改为 `duilib-png`/
  `duilib-zlib`，与库文件名一致。此前 `DUILIB_LIBS` 按库文件名链接时不会自动建立构建
  依赖，并行构建可能在库文件生成前就执行链接，报 `library 'duilib-png' not found`；
  改名后 CMake 会自动使用库文件的绝对路径并建立依赖。libpng 以内嵌方式构建时关闭
  安装导出（`SKIP_INSTALL_ALL`），避免其链接非导出目标 `duilib-zlib` 导致生成失败。
- Skia 编译工具链与主工程解耦：非 Windows 平台优先使用 `clang/clang++`（GCC 版 Skia
  运行性能明显较差），仅当 clang 不可用时回退 `gcc/g++`；构建产物目录相应为
  `out/llvm.<arch>.release` 或 `out/gcc.<arch>.release`。
- 修复首次构建 SDL3 成功后仍报 `SDL3 build failed`：完成标记（`.duilib-sdl`）原先在
  就绪检查之后写入，首次安装时校验必然失败；现在先写入再校验。
- Unix 示例程序改用相对 rpath（`$ORIGIN` / `@loader_path`，含 `../lib` 与
  `libcef_linux`），打包后优先加载包内 SDL3/CEF，不再复用构建机的绝对路径；
  打包脚本按可执行位识别 Unix 示例程序（原先只认 `.exe`）。
- 打包未包含 CEF 模块（未指定 `--with-cef`）时，自动跳过 `cef` 与 `CefBrowser`
  示例程序（Windows 为 `cef.exe`/`CefBrowser.exe`），避免包内留下必然启动失败的
  二进制；包内 README 同步说明。
- 打包支持 macOS 的 CEF 示例 App Bundle（`bin/cef.app`、`bin/CefBrowser.app`）：
  `--with-examples` 时与其他平台一致由 `--with-cef` 联动控制——未指定则跳过并告警，
  指定后整个目录（含 `Contents/Frameworks` 内的 CEF Framework、Helper 与符号链接）
  按原结构收集。macOS 的 `--with-cef` 只控制示例，不收集 CEF 开发文件与运行时
  （Bundle 自带 Framework），原“直接报错”的行为移除。
- CEF 只由 `cef` 与 `CefBrowser` 示例链接：此前根工程开启 `DUILIB_ENABLE_CEF`
  时所有示例都链接 `libcef`（Linux 下 `basic` 等程序的 `ldd` 出现 `libcef.so`）。
  现在由示例级 `DUILIB_EXAMPLE_CEF` 控制，`DUILIB_ENABLE_CEF` 只决定库是否编译
  CEF 控件源码。

- CEF 109（Windows CMake）：根工程固定编译 `libcef_win`（最新版）的 wrapper，
  且 109 的 wrapper 目标名仍为 `libcef_dll_wrapper`，与链接方期望的
  `libcef_dll_wrapper_109` 不一致（会链接失败或误用最新版源码）；现在根工程按
  `DUILIB_CEF_VERSION` 选择源码目录，109 的 wrapper 目标名改为
  `libcef_dll_wrapper_109`。
- CEF 109（Windows CMake，Debug）：`USE_SANDBOX=ON` 时 wrapper 会定义
  `_HAS_ITERATOR_DEBUGGING=0`，与 Debug 主工程默认的 `_ITERATOR_DEBUG_LEVEL=2`
  冲突（LNK2038）；本项目不链接 `cef_sandbox.lib`，现在 109 配置期强制
  `USE_SANDBOX=OFF`。
- `build.py pack --sdl`：`--sdl` 只是编译开关，包内不会包含 SDL3（需要
  `--with-sdl`）；现在给出提示，避免误以为 SDL3 已随包。
- `build.py --dry-run`：`pack` 与 `test` 子命令此前忽略该选项、直接真实执行；
  现在与其他子命令一致，只打印将要执行的命令。
- `pack.py`：同一 `lib/<arch>/<Config>` 中同时存在 MSVC `.lib` 与 MinGW `.a`
  时，工具链判断会误选（切换工具链后旧库不会被自动清理）；现在打包直接报错，
  提示先清理该目录，避免打出错误的包。
- MinGW 示例链接：Debug 示例单个体积可达 200 MB 以上，并行链接多个示例时
  `ld` 峰值内存很高（8 GB 机器上易报 `out of memory`）；现在 GNU ld 追加
  `-Wl,--no-keep-memory` 降低峰值内存（内存仍紧张时可用 `--jobs 2`）；
  clang/CLANG64 使用的 lld 不识别该参数，保持原样。
- 参数校验：`--skia-ref` / `--sdl-ref` 传入清单中不存在的键时，不再笼统报
  “third_party_versions.json 内容无效”，而是指出具体键与修正方式；
  `--cef-archive` 指向不存在的文件时立即报错，不再等到 CMake 配置阶段。
- `pack.py --with-cef`：库目录缺少所选版本的 wrapper 时（常见于编译用了
  `--cef-version 109`、打包时漏加同一参数），错误信息会指出库目录中实际的
  CEF 版本，并提示打包时应使用的 `--cef-version`。
- `pack.py --with-examples`（Windows）：`bin/` 由多个架构共用，打包 x86 SDK 时
  会把残留的 x64 示例程序一并收进包；现在按 PE 头的 Machine 字段校验架构，
  不一致的 exe/dll 会被跳过并阻止出包（可清理 `bin/` 或加 `--allow-missing`）。
- `build.py`：`DUILIB_MD`、`DUILIB_BUILD_EXAMPLES`、`DUILIB_ENABLE_SDL`、
  `DUILIB_CEF_VERSION`、`DUILIB_SKIA_REF` 等有状态选项此前只在传入时设置，
  复用同一个 `out/<arch>/<Config>` 时旧取值会残留在 CMake 缓存中（例如先跑
  `--no-examples` 后，默认构建不再编译示例）；现在每次调用都显式传入本次解析
  后的取值，构建结果只由当前命令行决定。
- 依赖准备：SDL3 的构建目录按“架构-配置”共享，与工具链无关。切换工具链或运行库
  （MSVC ↔ MinGW、/MT ↔ /MD）后会复用旧 CMake 缓存并继续调用上次的编译器，
  例如 MinGW 下 `--sdl` 仍调用 `cl.exe` 并因缺少 MSVC 环境而失败。现在按
  `.duilib-sdl` stamp 判断，CRT 不一致时先清空 SDL3 构建目录再重新配置。
- `build.py --dry-run`：`build` 与 `clean` 在只打印命令、未实际执行的情况下仍输出
  “编译完成/已删除”；现在 dry-run 不再打印完成提示（与 `pack`/`test` 一致）。

## [1.0.2026.1023] - 2026-10-23

### 新增

- 发布打包脚本 `build/pack.py`：按 SDK 契约收集已构建产物，生成 zip/tgz、
  `MANIFEST.txt`、`SHA256SUMS` 与包内 `VERSION`。
- 发布流程编排脚本 `build/release.py`：提供 `check`、`package`、`finalize`、`clean`
  四个子命令，不执行 git 写操作。
- 包内接入层：VS 的 `msvc/nim_duilib.props`、`.targets` 与 x64/x86 manifest；
  CMake 的 `lib/cmake/nim_duilib/*.cmake`，提供 `nim_duilib::duilib`。
- 顶层使用文档随 SDK 打包，并按包内布局重写 Markdown 链接。
- `--with-cef` 同时打包 CEF 运行时：保持 `bin/libcef_win[_109]/<架构>/` 结构，
  含 `libcef.dll`、`*.pak`、`icudtl.dat`、`locales/` 等（Windows）。
- `MANIFEST.txt` 与发布文档标注 WebView2 依赖系统安装的 Microsoft Edge
  WebView2 Evergreen Runtime，SDK 仅提供 loader；补充动态/静态 loader 与
  常见问题说明。
- 发布文档与 `MANIFEST.txt` 增加 Windows 下的包校验命令
  （PowerShell `Get-FileHash`、`certutil -hashfile`），无需安装 `sha256sum`。
- 许可合规：补充 `licenses/expat.LICENSE.txt`（Skia 静态链接的 expat，MIT）
  与 `licenses/RobotoMono.LICENSE.txt`（资源字体，Apache-2.0），并新增
  `docs/misc/License-Compliance.md` 随包组件许可对照表。
- 发布相关文档：`docs/Release.md`、`docs/misc/Release-Checklist.md` 与打包工作记录。

### 变更

- 版本号统一到 `duilib/duilib_version.h`，`msvc/dllRes/version.h` 改为 RC 适配层。
- 资源目录调整为 `bin/resources/`，与示例程序同级。
- 包名追加特性后缀：`-sdl`、`-cef` / `-cef109`、`-examples`、`-debug`，
  同一版本的不同特性包可共存于 `dist/`。
- `docs/` 只打包顶层使用文档，`misc/`、`specs/` 等子目录不随包。
- 归档语义：tar 按源文件权限写入（保留可执行与 setuid 位），zip 用 `S_IFLNK`
  条目保存符号链接；Unix 包解包后 `.so` 链接链与示例程序的可执行权限得以保留，
  符号链接不再计入 `SHA256SUMS`。
- 支持可复现构建：设置 `SOURCE_DATE_EPOCH` 后统一归档条目、gzip 头与
  `MANIFEST.txt` 的打包时间；未设置时按源文件/产物 mtime 生成并排序条目，
  输入不变时连续两次打包摘要一致。

### 修复

- 修复 macOS 上根目录 `VERSION` 与标准头 `<version>` 冲突导致的编译失败。
- SDK 接入层（VS 属性表与 CMake Config）在 Debug 下补充
  `/NODEFAULTLIB:"libcmt.lib"`，消除下游链接的 LNK4098 告警。

### 移除

- 移除根目录 `VERSION.txt`，避免版本数据出现第二份来源。
