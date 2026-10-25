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
- 双语构建参考手册 `build/build-reference.md` / `.en.md`；`build/README.md` /
  `.en.md` 改为前置依赖与发布打包索引。
- `build/test_build.py`：构建入口与 i18n 的标准库单元测试。
- 合规：`THIRD-PARTY-NOTICES.md`，SDK 包内许可证目录统一为 `LICENSES/`。

### 变更

- 统一入口一次只构建一个 `--config`（Debug/Release），需要两者时调用两次；
  `build/autotest.py` 的构建步骤改为调用 `build.py`。
- CEF 运行时按版本与架构输出到 `bin/libcef_win[/_109]/<arch>` 或 `bin/libcef_linux`，
  编译使用仓库跟踪的头文件与导入库。

### 移除

- 删除 `build/build.md`，构建说明合并到 `build/build-reference.md`。
- 旧分平台构建脚本归档到 `build/legacy/`（不再参与构建，全平台验证后移除）。

### 修复

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
