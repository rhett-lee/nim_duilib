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
  仓库已存在时先核对本地归档与当前提交（同名归档可能被重复提交），把被替换成旧版本
  的文件恢复后再 `git pull` 更新，失败时继续使用本地内容（与旧脚本一致）；
  `--offline` 下不联网，只做本地恢复，离线使用需提前自备。直接用 CMake 构建
  （`DUILIB_DEPS_MODE=auto` 或 `duilib_deps` 目标）时也会自动完成同样的处理。
- 网络操作统一重试：`git clone/fetch`、CEF 下载共用版本清单 `retry` 段
  （`maxAttempts=0` 无限重试、`delaySeconds=20`）。
- 双语构建参考手册 `build/build-reference.md` / `.en.md`；`build/README.md` /
  `.en.md` 改为前置依赖与发布打包索引。
- `build/test_build.py`：构建入口与 i18n 的标准库单元测试。
- 合规：`THIRD-PARTY-NOTICES.md`，SDK 包内许可证目录统一为 `LICENSES/`。
- SDK 接入层（macOS App Bundle）：`nim_duilib_copy_runtime` 新增 `FRAMEWORKS`
  选项（目标须为 `MACOSX_BUNDLE`），把包内 `lib/libSDL3*.dylib` 复制到
  `<target>.app/Contents/Frameworks`，并把可执行文件的 rpath 设为
  `@executable_path/../Frameworks`（`BUILD_WITH_INSTALL_RPATH`，保证 bundle
  内的库优先，避免构建树的 SDK 绝对路径抢先命中）；`nim_duilibConfig.cmake`
  登记 SDL3 dylib 文件名，包内 README 与接入文档同步说明。
- SDK 接入层（macOS App Bundle 资源）：`nim_duilib_copy_runtime` 的 `RESOURCES`
  在 `MACOSX_BUNDLE` 目标上（未指定 `DESTINATION` 时）复制到
  `Contents/Resources/duilib/resources/`，与普通程序的 `<exe 目录>/resources/`
  同构，也是 `GlobalManager::GetResourceRootPath(true)` 定位的标准目录。
- 发布包测试新增 macOS App Bundle 消费者用例 C11：`build/sdk_test/bundle_main.cpp`
  与 `nim_duilib_sdk_bundle` 目标用 SDK 包生成 `sdk_bundle.app`，校验
  `Contents/Frameworks/libSDL3*.dylib`、`@rpath`/`LC_RPATH`、
  `Contents/Resources/duilib/resources`，并把 `.app` 复制到构建树外运行。
- 普通示例的 macOS App Bundle 支持（不修改 C++ 代码）：示例可设置
  `DUILIB_EXAMPLE_MACOS_BUNDLE=ON`，或用根选项
  `DUILIB_MACOS_BUNDLE_EXAMPLES=<目标名列表>` 指定；`examples/basic` 已默认开启。
  Bundle 把资源同步到 `Contents/Resources/duilib/resources`（与普通程序
  同构，资源清单按示例名自动推导），把 SDL3 动态库复制到 `Contents/Frameworks` 并设置
  `@executable_path/../Frameworks` rpath，自动清理切换模式后残留的另一种产物，
  并从 `duilib_version.h` 生成 Bundle 版本号。资源同步由每次构建都会运行的
  `<目标>_bundle_resources` 目标完成（`cmake/duilib_copy_bundle_resources.cmake`
  按内容比较、只复制变化的文件并清理清单外的旧文件），因此只改 XML/图片等资源、
  不改 C++ 源码时 Bundle 内容也会更新，不需要强制重新链接。
- macOS Bundle 资源按示例自动裁剪：资源清单由示例的目标名（`project()` 名称）
  推导，只复制示例自己的主题目录、默认主题公共资源、字体与语言文件，
  示例的 `CMakeLists.txt` 不需要配置；目标名与资源目录名不一致时（如
  `ChildWindow`→`child_window`）也可显式设置 `DUILIB_THEME_DIR_NAME`，显式值优先，
  写错目录会在配置阶段直接报错；`themes/default` 下其他示例的目录不再进包，
  Bundle 资源体积由约 4 MB 降到约 1.3 MB。需要额外资源时可用
  `DUILIB_MACOS_BUNDLE_RESOURCES` 覆盖默认清单。
- `GlobalManager::GetResourceRootPath(bool bMacOsAppBundle = true)` 在 macOS 下按
  参数选择资源根：`true`（默认）返回 `<App>.app/Contents/Resources/duilib/resources/`，
  `false` 返回可执行文件同目录的 `resources/`；非 Bundle 程序仍回退到后者。
  `GetResourceZipPath(bool)` 的 zip 模式对应 `.../duilib/resources.zip`。
- macOS App Bundle 参考文档 `build/macos-app-bundle.md` / `.en.md`，以及
  `Info.plist` 模板与可选的 `.icns` 图标配置。
- Windows MSVC DLL 模式：`python build/build.py build --dll` 把 duilib 编为
  `bin/duilib*.dll`（x86 为 `duilib32*.dll`，Debug 带 `_d`，导入库统一为
  `lib/<arch>/<Config>/duilib.lib`）；`--dll` 隐含 `--md`，自动准备 Skia
  `.md`/`.mdd`，同时同步 `msvc/PropertySheets/duilib_dll.flag`。Windows 上 SDL
  默认关闭，DLL 模式也不自动开启；关闭时 `deps` 不下载 SDL 源码、不构建 SDL3，
  需要时显式 `--sdl`。静态 ↔ DLL 模式切换由
  `out/<arch>/<Config>/.duilib-build-mode` 检测，自动清理构建树（含
  `libcef_dll_wrapper` 旧 `/MT` 目标文件）；DLL → 静态还会删除 `bin/duilib*.dll`。
- 发布包 DLL 支持：自动收集 `bin/duilib*.dll`，包名追加 `-dll`，元数据
  `IS_DLL=1`、`MSVC_RUNTIME=MD`；`pack.py` 通过 DLL 导出表确认 SDL 能力
  （DLL 以 SDL 构建时打包必须加 `--with-sdl`），并按 DLL 的实际导入决定是否
  随包 `bin/turbojpeg.dll`。

### 变更

- 统一入口一次只构建一个 `--config`（Debug/Release），需要两者时调用两次；
  `build/autotest.py` 的构建步骤改为调用 `build.py`。
- CEF 运行时按版本与架构输出到 `bin/libcef_win[/_109]/<arch>` 或 `bin/libcef_linux`，
  编译使用仓库跟踪的头文件与导入库。所有平台统一使用 `_minimal` 发行包（体积约为
  标准包的一半，含 `include/`、`Release/`、`Resources/`）。
- 依赖准备（CEF）：下载的归档按 CEF 官方发布的 `<下载地址>.sha1` 校验完整性；
  哈希文件与归档一起缓存、离线可复用。拿不到时回退版本清单中的 `sha256`，
  两者都没有时只告警并跳过校验。
- 命令行帮助去歧义：`--sdl`/`--no-sdl`、`--md`、`--vs-version` 在非 Windows 平台上
  标注为仅 Windows 有效；`--no-absolute-rpath` 在 Windows 上标注为仅 Unix。
  补充 `--deps-root`（Skia/SDL3/CEF 的安装位置）、`--jobs`、`--skia-ref`、
  `--sdl-ref` 等参数的用途说明。`--cef-version` 不再硬编码 142，接受 `109` 或
  `latest`（跟随版本清单默认值，清单升级后自动跟随）；`pack.py`、`release.py`、
  `autotest.py`、`sdk_package_test.py` 的同类参数说明同步完善。
- `doctor` 的依赖状态报告更完整：分别列出 Skia 源码与补丁归档、SDL 源码与前缀、
  CEF 运行时，缺失时说明由 `deps` 获取，`--offline` 下则提示无法获取；解释器名称
  按实际命令显示（Linux 上为 `python3`）。
- 依赖版本（SDL3）：清单默认 ref 由 `release-3.4.18` tag 改为 `main` 分支
  （该 tag 的源码当前编译报错）；`release-3.4.18` 仍保留在 `refs` 中，需要时可用
  `--sdl-ref release-3.4.18` 临时切换。

### 移除

- 删除 `build/build.md`，构建说明合并到 `build/build-reference.md`。
- 旧分平台构建脚本归档到 `build/legacy/`（不再参与构建，全平台验证后移除）。

### 修复

- 构建（切换 Debug/Release）：示例程序都输出到共享的 `bin/`，目标文件却按配置
  分开存放；先编译 Debug 再编译 Release 时，`bin/` 中的文件比 Release 的目标
  文件新，Ninja 判定链接步骤已是最新（`ninja: no work to do`），`bin/` 一直是
  Debug 产物。现在 `build.py build` 在链接前根据 `bin/.build-config/<目标>`
  （兼容旧版 `bin/<目标>.build-config`）删除其他配置的示例产物及标记，强制当前
  配置重新链接；`--target X` 只清理该目标，`--no-examples` 跳过清理。
- 发布包测试（macOS P04/P06 误报）：`sdk_package_test.py` 原先只在
  `bin/libcef_win*`、`bin/libcef_linux` 中查找 CEF 运行时，macOS 的 CEF 位于示例
  `.app/Contents/Frameworks/Chromium Embedded Framework.framework`，导致包名 `-cef`
  与实际能力不一致，且把随包的 CEF 头文件当成“未声明 CEF”。现在 macOS 的 `-cef`
  只表示 CEF 示例 Bundle 随包，`HAS_CEF` 按“SDK 是否提供 CEF 开发文件”判定。
- 打包（macOS CEF 头文件）：`--with-cef` 此前在 macOS 也转发
  `include/duilib/duilib_cef.h`、`include/duilib/CEFControl/`，但这些头文件依赖
  未随包的 `third_party/libcef` 头文件，无法用于下游编译，且与元数据 `HAS_CEF=0`
  和发布文档矛盾。现在 macOS 不转发 CEF 头文件；包名中的 `-cef` 只在
  cef/CefBrowser 示例 App Bundle 确实进包时追加。
- 发布包测试（C07/C08/C09 耗时）：三个场景此前把整个解包目录重新压缩成 tgz /
  tar.bz2，Debug CEF 包可达数 GB，单场景会持续十几分钟且 Python 占满一个核
  （并非死循环）。现在只从真实包挑选最小头文件、最小静态库与运行资源生成代表性
  小归档，并重建 `SHA256SUMS`；判定逻辑不变，耗时与包体积无关，完整包仍由
  P01–P03、P07 覆盖。
- 依赖准备（SDL3）：本地 `<deps-root>/SDL` 源码停在别的分支/标签（例如 `main`）时，
  脚本此前直接用它编译，完成标记却只记录清单里的 ref 名称，导致用错版本编译的库被
  当成清单版本的产物复用。现在 `deps` 先按清单把工作区切到指定 tag/commit（本地
  已有该提交时不联网，`--offline` 同样可用），并在 `.duilib-sdl` 中记录源码提交；
  切换后旧产物自动重建（升级到本版本时会重建一次 SDL3）。
- 依赖准备（Skia 补丁）：本地补丁归档此前只要文件名存在就直接复用，而
  skia_compile 中同名归档可能被重复提交（如 `skia.2026-09-16.src.zip`），旧版本
  补丁会让旧的 Skia 产物一直不更新。现在 `deps` 核对归档与仓库当前提交，把被替换
  （或删除）的归档恢复成仓库版本，再 `git pull` 取最新提交；补丁指纹变化时重新
  解压并编译 Skia。`--offline` 只做本地恢复；`doctor` 会提示归档与仓库不一致。
- 依赖准备（Skia 源码）：`--skia-ref` 选中的 tag/branch 此前不会检查本地工作区，
  现在与 commit ref 一样按清单对齐（branch 只要求当前就在该分支上，不追远端新提交）。
- 构建（Windows 链接依赖）：Skia 与 SDL3 的静态库此前只以库名（配合
  `link_directories`）传给 `target_link_libraries`，Ninja 不会把它们记为链接依赖，
  依赖重新编译后示例程序不会重新链接，包里可能留下用旧库生成的二进制。现在
  各平台统一按库文件绝对路径链接静态库（MSVC 的 `SDL3-static[d].lib`、MinGW 的
  `libSDL3.a`、各平台的 Skia `.a`/`.lib`）；共享库（Unix 的 SDL3 `.so`/`.dylib`）
  仍保留库名由 `-l` 解析，避免把构建机绝对路径写进 dylib 的 `install_name`/rpath。
- 构建（Windows 示例）：Debug 默认开启增量链接，并行链接多个示例时偶发
  `LNK1000: Internal error during IncrBuildImage`，旧 `.ilk` 损坏后还要手工删除；
  示例程序统一加 `/INCREMENTAL:NO`，链接耗时代价很小、构建结果稳定。
- 构建（Windows DLL / libjpeg-turbo）：DLL 模式改用动态 `turbojpeg.lib` 导入库，
  构建后把 `turbojpeg.dll` 复制到 `bin/`，不再链接按 `/MT` 编译的
  `turbojpeg-static.lib`，消除 `LNK4098`（`/MT` 与 `/MD` 目标文件冲突）。
- 构建（MSBuild 运行库切换）：`CommonSettings.props` 在每个工程 `IntDir` 记录
  `duilib_crt_mode.txt`，切换 `duilib_dll.flag` 后自动删除旧 `.obj/.pch`
  （含 CEF wrapper），修掉 VS 增量构建不跟踪编译选项、仍链接旧 `/MT` 目标文件的问题。
- 发布包测试（Windows 长路径 / x86 运行库）：解包时去掉归档顶层包目录、包工作
  目录改用短序号名，避免 CEF 头文件与 MSBuild `.tlog` 超过 `MAX_PATH`
  （`FileNotFoundError`、`MSB6003`）；MSVC `/MD` 包在运行消费者前把 VS 2022
  运行库复制到 exe 同级，避开系统 `SysWOW64` 旧运行库导致的 x86 崩溃。
- 发布包测试（DLL 包 JPEG 误判）：`infer_capabilities()` 只看 `lib/`，DLL 包
  实际只有运行期 `bin/turbojpeg.dll`，导致 P04 报“元数据 HAS_JPEG=1、实际文件
  为 0”，并连带 C02 消费者按错误能力配置失败；现在静态包看
  `lib/turbojpeg*.lib`、DLL 包看 `bin/turbojpeg.dll`，新增对应单元测试。
- 打包（Debug/Release 同名示例）：CMake 链接后在隐藏目录
  `bin/.build-config/<目标>` 记录构建配置（升级前写在 `bin/<目标>.build-config`，
  `pack.py` 仍兼容读取，重新链接时会自动删除旧文件）；打 Debug 包时按标记跳过
  Release 构建的示例，缺少标记时提示无法确认，不再笼统告警“可能混有 Release 版本”。
- 依赖准备（SDL3）：Debug 与 Release 生成的 `SDL3-static.lib` 同名且安装到同一个
  前缀，原先两个配置都构建、Release 最后安装，Debug 构建实际链接的是 Release 版
  静态库（`/MTd` 与 `/MT`、`/MDd` 与 `/MD` 混用）。现在单配置生成器（`build.py`
  默认的 Ninja）只构建并安装当前配置，完成标记也记录配置，切换 Debug/Release 时
  自动重建；多配置生成器（Visual Studio）仍需构建两份，会打印提示说明 Debug
  链接的是 Release 静态库。
- 构建（CMake 示例列表）：`DUILIB_EXAMPLES` 把推导结果写进缓存，关闭
  `DUILIB_ENABLE_CEF` 后再配置时仍沿用上次含 `cef`/`CefBrowser` 的旧列表，
  链接阶段报 `CefManager::GetInstance` 等符号无法解析。现在每次配置都重新推导
  平台默认列表（只有与默认值都不同、且非上次推导结果时才视为用户显式覆盖），
  根工程另加一道保护：`DUILIB_ENABLE_CEF=OFF` 时直接跳过 CEF 示例并提示。
- `build.py`：`DUILIB_ENABLE_CEF` 也是有状态的缓存项，用
  `--cmake-arg=-DDUILIB_ENABLE_CEF=OFF` 关掉一次后，后续不带该参数的构建仍沿用
  旧值。现在与其他开关一样每次显式传入（Windows MSVC、Linux、macOS 为 ON，
  MinGW/MSYS2 与 FreeBSD 为 OFF），用户仍可用 `--cmake-arg` 覆盖。
- 打包（SDK 接入）：`NimDuilibPackageRuntimeDlls` 之前把
  `bin/libcef_win[_109]/<arch>/` 下的 CEF 运行时 DLL 文件名摊平列入，下游用
  `nim_duilib_copy_runtime` 时会尝试复制不存在的 `bin/<name>` 而构建失败。
  现在元数据只登记 bin 根目录下的 DLL；`nim_duilibConfig.cmake` 也会按文件是否
  存在过滤，旧包同样可用。CEF 运行时仍按文档由下游整体复制对应子目录。
- 构建（macOS CEF 示例）：`cef.app`/`CefBrowser.app` 未把 SDL3 动态库放进
  `Contents/Frameworks`，也没有指向该目录的相对 rpath，只能靠构建机的 SDL3
  绝对路径运行。现在构建后由 `duilib_copy_sdl_bundle.cmake` 复制
  `libSDL3*.dylib`（保留版本链接），主程序 rpath 设为
  `@executable_path/../Frameworks`（Helper 为 `@executable_path/../../..`）；
  自备静态 SDL3 时自动跳过复制。
- 构建（macOS）：CEF 二进制分发包通常只带 Release 框架，Debug 构建复制
  `Debug/Chromium Embedded Framework.framework` 时会失败。现在配置期检测
  `Debug/` 与 `Release/`，Debug 框架缺失时自动回退到 Release 并打印提示；
  两者都不存在时在配置期直接报错，提示检查 `CEF_ROOT`。
- 构建工具：`autotest.py`、`sdk_package_test.py` 的默认输出目录改用系统临时目录
  （`tempfile.gettempdir()`），移除写死的 `C:\develop\temp`、`/tmp`，换机器无需改代码；
  `README`、`README.en.md`、`sdk-package-test.md` 中的相关示例同步更新。
- 依赖准备（Windows）：Skia 使用的 LLVM 根目录不再写死 `C:/LLVM`，改为按
  `-DDUILIB_LLVM_ROOT` → `LLVM_ROOT` 环境变量 → `C:/LLVM` → `Program Files/LLVM`
  → Visual Studio 自带 LLVM 的顺序探测。
- 构建（macOS）：示例程序链接时 ld64 报 “ignoring duplicate libraries”（`-lpthread`、
  `-lSDL3`、Skia 库等被重复列出，链接器本身会忽略重复项）。现在在链接器支持时添加
  `-Wl,-no_warn_duplicate_libraries`，消除该告警。
- 示例（controls）：`TabCtrlForm.h`/`TabCtrlForm.cpp` 缺少文件末尾换行、
  `TimelineForm.cpp` 存在只自增未使用的 `nIndex`，分别触发 `-Wnewline-eof` 与
  `-Wunused-but-set-variable`；已补齐换行并删除无用变量。
- 构建（Windows / CEF）：主工程使用 C++20，而 `libcef_dll_wrapper` 按上游固定使用
  C++17，编译命令行同时带 `/std:c++20` 与 `/std:c++17`，产生 D9025 警告。现在为
  wrapper 目标（含 109 变体）单独设置 `CXX_STANDARD 17`，不再覆盖全局标准。
- 依赖准备（UOS）：与旧脚本一致，统信 UOS 上固定使用 `gcc/g++`（Skia 与主工程一致），
  避免 clang++ 在该平台的链接错误。
- 依赖准备：Skia 补丁归档增加“已应用”校验与内容指纹（`skia/.duilib-patch`），补丁与
  基线提交未变化时不再重复解压，避免刷新源码 mtime 导致整棵 Skia 重编。
- 依赖准备（CEF）：缓存归档 sha256 不匹配时自动删除并重新下载；通过
  `DUILIB_CEF_ARCHIVE` 指定的本地归档仍直接报错，避免误删用户文件。
- 依赖准备：curl 下载增加 `--speed-limit 100 --speed-time 120`，连接后长时间低速
  传输会中断本次尝试并按统一参数重试，避免无限重试下永久挂起。
- `build.py`（MSYS2）：校验编译器目标为 mingw-w64（优先 clang、其次 gcc），否则给出
  明确错误，避免误用 MSYS 目标编译器产出非 Windows 程序。
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
- 打包（Linux）：CEF wrapper 的 Unix 产物被上游 CMake 去掉 `lib` 前缀
  （`PREFIX ""`），文件名为 `cef_dll_wrapper.a`，不在 `pack.py` 的收集白名单中，
  `--with-cef` 时报“缺少 libcef_dll_wrapper”。现在归一化时把无前缀名映射回
  白名单名，包内文件保持原名（下游 `nim_duilibConfig.cmake` 两种写法都接受）。
- 依赖准备（Unix）：SDL3 的完成标记包含目标配置，而 Unix 上 SDL3 固定按 Release
  构建共享库、与 duilib 的 Debug/Release 无关，切换配置会无谓地重新编译 SDL3；
  现在配置分量仅 Windows 保留。
- 依赖准备（CEF）：`_duilib_cef_tracked_dir` 原先不区分平台，恒返回
  `libcef_win[_109]`，Linux/macOS 的版本比对与迁移采纳错对 Windows 的跟踪头；
  现在按平台返回 `libcef_linux` / `libcef_macos` / `libcef_win[_109]`。
- 打包支持 macOS 的 CEF 示例 App Bundle（`bin/cef.app`、`bin/CefBrowser.app`）：
  `--with-examples` 时与其他平台一致由 `--with-cef` 联动控制——未指定则跳过并告警，
  指定后整个目录（含 `Contents/Frameworks` 内的 CEF Framework、Helper 与符号链接）
  按原结构收集。macOS 的 `--with-cef` 只控制示例，不收集 CEF 开发文件与运行时
  （Bundle 自带 Framework），原“直接报错”的行为移除。
- CEF 示例 App Bundle（macOS）：程序链接的是动态 SDL3（构建时 `SDL_SHARED=ON`），
  但 `duilib_cef_macos.cmake` 既没有把 `libSDL3*.dylib` 放进 `Contents/Frameworks`，
  也没有设置指向它的 rpath，能运行只是因为 rpath 中残留构建机的 SDL3 绝对路径，
  换目录/换机器即加载失败。现在构建后自动把 dylib 复制进 Bundle，并为主程序设置
  rpath `@executable_path/../Frameworks`、为 Helper 设置
  `@executable_path/../../..`（Helper 的 exe 在
  `Contents/Frameworks/<Helper>.app/Contents/MacOS`），打包时随 Bundle 一并收集。
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
- SDK 测试消费者工程（`build/sdk_test`）：`NIM_DUILIB_SDK_DIR` 支持省略与相对路径。
  解包后把 `sdk_test` 放进 SDK 根目录并执行
  `cmake -S ./sdk_test -B ./build -DNIM_DUILIB_SDK_DIR=./` 时，原先的相对路径被按构建
  目录解析，`find_package(nim_duilib CONFIG)` 找不到
  `lib/cmake/nim_duilib/nim_duilibConfig.cmake`。现在以 `sdk_test` 自身、它的
  上一级/上两级、构建目录及其上一级/上两级为基准解析相对路径（取第一个命中），
  `sdk_test` 与解包目录并列时写 `-DNIM_DUILIB_SDK_DIR=./nim_duilib-<版本>-<平台>`
  也能直接工作；省略时按常见布局自动探测；找不到时报错并列出所有尝试过的目录。
- 单元测试（macOS）：`test_build.SkiaPatchTests.test_existing_repo_is_pulled` 用临时
  目录的原始路径做比较，而 `build.py` 的 deps root 会 `resolve()` 成真实路径；macOS 上
  `/var` 是 `/private/var` 的符号链接，两者不相等导致测试失败。断言改为按
  `Path.resolve()` 后的路径比较，Windows/Linux 结果不变。

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
