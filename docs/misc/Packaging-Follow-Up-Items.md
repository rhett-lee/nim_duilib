# 发布打包待办事项（Follow-up）

本文件记录发布打包流程已经完成的部分、尚未完成的工作及验收标准，供后续逐项跟进。

> 本文件位于 `docs/misc/`，不随发布包分发。

## 一、当前基线

已完成并验证的内容（Windows x64 Debug 静态包）：

| 领域 | 状态 |
| :--- | :--- |
| 产物收集 | 头文件、库、Skia、SDL、CEF、WebView2、JPEG、运行期 DLL、资源、许可证、文档 |
| 包内布局 | `include/`、`lib/`、`bin/`（含 `bin/resources/`）、`docs/`、`licenses/`、`msvc/`、`lib/cmake/` |
| 元数据与校验 | `MANIFEST.txt`（含文档清单）、`SHA256SUMS`、`VERSION` |
| 下游接入 | VS props/targets、CMake Config/Helper、x64 + x86 manifest |
| 文档处理 | 只打顶层使用文档，源码链接按包布局重写，无断链 |
| 已验证 | `--list`、XML 解析、CMake 配置、SHA/断链检查、真实 zip 抽查；VS/CMake 端到端（x64 Debug） |

尚未覆盖：Release、x86/arm64、MinGW、DLL 模式、Linux/macOS/FreeBSD、SDL/CEF/示例程序组合。

### 平台与特性约束

| 平台 | 架构 | CEF | 说明 |
| :--- | :--- | :--- | :--- |
| Windows | x64 / x86 | 支持 | CEF 二进制按版本与架构位于 `bin/libcef_win/{x64,Win32}/`、`bin/libcef_win_109/{x64,Win32}/` |
| Linux | 仅 x64（不支持 x86） | 支持 | CEF 运行时位于 `bin/libcef_linux/`，与 Windows 的分架构结构不同 |
| macOS | 待确认 | 以 `.app` 内嵌 | CEF 随示例生成 `cef.app`、`CefBrowser.app`；框架与 Helper 位于 bundle 内，SDK 包暂不自动收集 |
| FreeBSD | 待确认 | 不支持 | 目录结构参考 Linux，但包内不包含 CEF |

说明：上表是平台能力与目录约束，包内最终布局以打包脚本为准；CEF 运行时的收集与落包见 P1-3。

## 二、状态说明

- `[ ]` 未开始
- `[~]` 进行中
- `[x]` 已完成
- `[-]` 不处理（明确关闭，并记录理由）
- 优先级：P0 发布阻断，P1 发布质量，P2 维护与体验

## 三、待办清单

### P0：发布前必须处理

#### P0-1 打包矩阵实测

- [ ] 覆盖“平台 × 配置 × 模式 × 特性”组合，每个组合至少执行一次 `--list` 与解包检查。
- 现状：只验证了 Windows x64 Debug 静态包。
- 待覆盖：
  - 配置：Debug / Release；
  - 架构：Windows x64/x86；Linux 仅 x64（不支持 x86）；arm64 待定；
  - 工具链：MSVC / MinGW；
  - 模式：静态 / DLL（`EnableDuilibDll`）；
  - 平台：Windows / Linux / macOS / FreeBSD（FreeBSD 无 CEF）；
  - 特性：`--with-sdl`、`--with-cef`（Windows/Linux/macOS）、`--with-examples`。
- 验收：矩阵中每个格子都有记录（命令、文件数、关键文件存在性、告警），异常项有结论。

#### P0-2 关键产物缺失时阻止出包

- [-] **不处理**（2026-10-23 决策：必要性不大）。
- 理由：产物是否齐全由构建流程负责；打包脚本保持“告警后继续出包”的宽松行为，便于缺项时先出包排查。
- 说明：缺失项仍会在输出中逐条告警，但不作为失败条件，不引入 `--strict`。

#### P0-3 接入层端到端验证

- [~] Windows x64（Debug、静态、MSVC）已完成（2026-10-23）；其余组合待补。
- 已完成：
  - MSBuild：导入 `msvc/nim_duilib.targets` 的 Debug|x64 工程编译、链接、运行通过；
    `NimDuilibCopyResources=1` 复制 `resources/`，`bin/*.dll` 复制 `WebView2Loader.dll`；
    嵌入 manifest 已验证（Common Controls 6.0.0.0、AMD64、supportedOS 列表）。
  - CMake：`find_package(nim_duilib CONFIG REQUIRED)` +
    `nim_duilib_copy_runtime(... RESOURCES)` 的 Debug|x64 工程编译、链接、运行通过。
  - 运行验证：`GlobalManager::Startup` 成功加载 `resources/themes/default/global.xml`，
    程序输出 `duilib version: 1.0.2026.1023`。
  - 负向验证：x64 包 + x86 工程在 CMake 配置阶段按架构不匹配拒绝，并给出明确提示。
- 发现并修复：接入层 Debug 未屏蔽 release CRT，下游链接出现 LNK4098；
  已在 `packaging/msvc/nim_duilib.props` 与 `packaging/cmake/nim_duilibConfig.cmake`
  中补 `/NODEFAULTLIB:"libcmt.lib"`（与仓库内 `DuilibSettings.props` /
  `BinCommonSettings.props` 一致）。修复后 VS 与 CMake 两条路径均无 LNK4098/LNK2038。
- 未覆盖及原因：
  - Windows x86：本机 Skia 只有 `llvm.x64.debug` / `llvm.x64.release`，缺 x86 产物，无法出包；
  - DLL 模式：当前构建为静态，`bin/` 下没有 `duilib*.dll`；
  - Linux/macOS/FreeBSD：无对应环境。
- 验收（Windows x64）：可执行程序启动并成功加载主题资源，链接无 LNK2038/LNK4098。

#### P0-4 签名与公证

- [-] **不处理**（2026-10-23 决策：不具备条件）。
- 理由：当前没有 Windows 代码签名证书与 macOS 公证环境；SDK 以头文件和静态库/导入库形式分发，通常不需要对库本身签名。
- 说明：如后续需要对外分发可执行示例或安装包，再单独准备证书与公证流程。

#### P0-5 发布流程闭环

> 方案见 [Release-Process-Closure-Plan.md](Release-Process-Closure-Plan.md)，已于 2026-10-23 实施。
> 版本号单一源为 `duilib/duilib_version.h`；`BUILD` 段不补零。

- [x] 版本递增检查、CHANGELOG / Release Notes、git tag、产物归档。
- [x] `dist/` 旧产物清理策略。
- 现状：`build/release.py` 提供 check / package / finalize / clean；中英
  `CHANGELOG.md` / `CHANGELOG.en.md` 与 `docs/misc/Release-Checklist.md` 已建立；
  包名追加 `-sdl/-cef/-cef109/-examples/-debug` 特性后缀，产物按 `dist/<版本>/` 归档。
- 验收：按 checklist 可从干净工作区产出唯一版本包，步骤可复现。

### P1：发布质量

#### P1-1 Unix 归档语义

- [x] tar 内文件权限按类型设置，不再统一 `0o644`。
- [x] 符号链接按 `lstat` 处理，避免 zip 展开 `.so` 版本链产生重复实体。
- 现状（2026-10-23 已实施）：`_tar_add_file()` 用 `lstat` 生成条目，普通文件权限取自
  源文件（清除组/其他可写位，保留可执行与 setuid 等特殊位）；Windows 源没有 POSIX
  权限位，按扩展名给 `0755`/`0644`。`_zip_add_file()` 把符号链接写成 `S_IFLNK`
  条目（内容为链接目标），普通文件按权限写 `external_attr`，解包端可还原权限。
  `iter_files()` 额外产出目录型符号链接（如 macOS Framework 的 `Versions/Current`），
  否则这类链接会丢失。符号链接不计入 `SHA256SUMS`，避免 `sha256sum -c` 跟随链接后
  校验失败。
- 实测结构（Linux SDL3）：`libSDL3.so -> libSDL3.so.0 -> libSDL3.so.0.5.0`，
  打包需保留完整符号链接链，不能只留实体文件。
- 其它受影响文件：Linux `bin/` 下的示例程序（无扩展名，可执行）、
  macOS `bin/` 下的示例程序与 `.app` bundle 内的可执行文件、
  CEF 的 `chrome-sandbox`（需要可执行权限）。
- macOS `bin/` 的示例为无扩展名可执行文件（`-rwxr-xr-x`），CEF 示例为
  `cef.app`、`CefBrowser.app`；打包需保留 bundle 目录结构与内部符号链接。
- 依据：`build/pack.py` 的 `build_package()` 与 `iter_files()`。
- 验收：Linux/macOS 包内示例程序与 `chrome-sandbox` 可直接执行；
  `libSDL3.so*` 等链接结构与构建目录一致。
  Windows 侧已回归：Debug x64 zip 539 个文件，`SHA256SUMS` 全部匹配，
  `.dll` 条目 `0755`、`.lib` 条目 `0644`，`zip` 可完整解压；
  Unix 侧因无环境未实跑，链接与权限逻辑以构造用例验证
  （zip 写 `S_IFLNK` 条目、tar 写 `SYMTYPE`，链接目标与模式均正确）。

#### P1-2 可复现构建

- [x] 支持 `SOURCE_DATE_EPOCH`，固定 zip 条目时间与 tar mtime/gzip 头。
- 现状（2026-10-23 已实施）：`read_source_date_epoch()` 读取并校验环境变量；
  设置后所有归档条目、gzip 头与 `MANIFEST.txt` 的“打包时间”统一使用该值
  （按 UTC 渲染）。未设置时普通条目取源文件 mtime，生成文件与 gzip 头取收集
  产物中的最新 mtime；条目按包内路径排序。`--format tarbz2` 无时间戳头，
  同样稳定。
- 实测：Debug x64 包连续两次打包，zip 摘要
  `616D5FDB…36FAD36`、tgz 摘要 `C78FAF5F…0B171E` 两次一致；
  `SOURCE_DATE_EPOCH=1700000000` 时 zip 摘要 `BEEF0E1A…0A9EDC`、tgz 摘要
  `2DBDFB19…C52473` 两次一致，条目时间全部为 `2023-11-14 22:13:20`，
  gzip 头 mtime 同为 `1700000000` 且不写文件名；非法值退出码 1。
- 验收：同一输入、同一环境连续两次打包，`SHA256SUMS` 与包文件摘要一致。

#### P1-3 CEF 运行时打包

- [x] Windows：运行时二进制位于 `bin/libcef_win/{x64,Win32}/`、
  `bin/libcef_win_109/{x64,Win32}/`，含 `libcef.dll`、`*.pak`、`icudtl.dat`、
  `v8_context_snapshot.bin`、`locales/`。
- [x] Linux：运行时位于 `bin/libcef_linux/`（扁平，不再分架构），含
  `libcef.so`、`libEGL.so`、`libGLESv2.so`、`libvk_swiftshader.so`、
  `libvulkan.so.1`、`chrome-sandbox`、`vk_swiftshader_icd.json`、
  `resources.pak`、`chrome_100_percent.pak`、`chrome_200_percent.pak`、
  `icudtl.dat`、`v8_context_snapshot.bin`、`locales/*.pak`。
- [~] 打包脚本按上述结构收集对应平台/架构的运行时，保持相对结构放入包内 `bin/`。
  - [x] Windows（2026-10-23）：`--with-cef` 时从 `bin/libcef_win[_109]/{x64,Win32}/`
    收集运行时，包内保持 `bin/libcef_win[_109]/<架构>/` 结构；
    `libcef.lib`、`cef_sandbox.lib` 等库文件不随包（前者在 `lib/` 已提供，
    后者本项目不使用沙箱且体积 80 MB 级）。
  - [ ] Linux：`--with-cef` 目前仅支持 Windows 出包（`collect_cef` 直接报错），
    待放开后再接入 `bin/libcef_linux/`。
  - macOS 按既定设计不自动收集（见下条）。
- 现状：Windows 运行时已随包；Linux 待 CEF 出包支持；macOS 仍由下游手动配置。
- [x] macOS：结构已确认，CEF 随示例以 `.app` 形式生成：
  `Contents/MacOS/<exe>`、`Contents/Frameworks/Chromium Embedded Framework.framework`
  （`Versions/A/...` 及顶层符号链接）、`Contents/Frameworks/<name> Helper*.app`
  （Helper / GPU / Plugin / Renderer）、`Contents/Resources/`。
  按既定设计，SDK 包不自动收集 macOS 的框架与 Helper，由下游手动配置。
- [x] macOS 的 CEF 二进制分发包位于与仓库平级的 `cef_binary/`（可用 `-DCEF_ROOT`
  或环境变量 `CEF_ROOT` 覆盖），内含 `include/`、`libcef_dll/`、`cmake/`、
  `Release/Chromium Embedded Framework.framework`；构建时由 `COPY_MAC_FRAMEWORK`
  把框架与 Helper 拷入 `.app` 的 `Contents/Frameworks/`。
  注意：仓库内 `duilib/third_party/libcef/libcef_macos` 只用于编译 wrapper，
  不是运行时分发包，两者不能混用。
- [x] FreeBSD：不支持 CEF，包内不出现任何 CEF 内容。
- 验收：带 CEF 的 Windows/Linux 示例程序在干净机器上无需额外拷贝即可启动；
  `chrome-sandbox` 的可执行权限正确；macOS 按手动配置流程验证。
  Windows 侧已满足运行时随包（`--with-cef --list`：latest 235 个、109 个 69 个
  运行时文件，且不含任何 `.lib`）；示例程序尚未构建，运行验证待补。

#### P1-4 WebView2 运行时说明

- [x] 在 `MANIFEST.txt` 与接入文档中明确“依赖系统 Evergreen Runtime”。
- 现状（2026-10-23 已实施）：`write_manifest()` 的“运行期”章节在包内含
  WebView2 loader（`WebView2LoaderStatic.lib`、`WebView2Loader.dll.lib` 或
  `bin/WebView2Loader.dll`）时，注明依赖系统已安装的 Microsoft Edge WebView2
  Evergreen Runtime、本包不含浏览器内核、`0x80070002` 错误与安装地址。
  `docs/Release.md` / `Release.en.md` 新增“WebView2 运行时”章节，说明
  静态/动态 loader 的差异、Fixed Version Runtime 方式与常见问题。
- 验收：Windows 包的 `MANIFEST.txt` 与文档均有说明，常见问题覆盖运行时缺失、
  `WebView2Loader.dll` 缺失与架构不匹配三种情况。

#### P1-5 校验方式对 Windows 友好

- [x] 提供 `verify.py` 或 PowerShell `Get-FileHash` 示例。
- 现状（2026-10-23 已实施）：`MANIFEST.txt` 的“校验”章节与
  `docs/Release.md` / `Release.en.md` 的“校验发布包”章节给出 PowerShell 5+
  校验 `SHA256SUMS` 的脚本（全部通过输出 `OK`，否则列出失败项并 `exit 1`），
  并补充 `Get-FileHash` / `certutil -hashfile` 的单文件用法；
  `release.py finalize` 生成的压缩包级 `SHA256SUMS.txt` 格式相同，可在
  `dist/<版本>/` 直接复用同一脚本。
- 验收：Windows 用户无需 `sha256sum`，在包根目录执行文档中的命令即可完成
  全包校验；已在解压后的实测包上执行该脚本，输出 `OK`。

#### P1-6 许可合规复查

- [x] 逐项确认随包分发的第三方组件许可：Skia 及其第三方（expat、skcms、bentleyottmann、skresources 等）、CEF、WebView2、SDL、libjpeg-turbo、libwebp、libpng、zlib。
- 现状（2026-10-24 已实施）：新增 [License-Compliance.md](License-Compliance.md)，
  按“组件 → 许可证 → 是否允许随包分发 → 包内文件”逐项列出 25 个组件/模块
  （含字体资源）。随包组件均为 MIT / BSD-2/3 / zlib / libpng / Apache-2.0
  （含 LLVM 例外）等宽松许可，允许二进制分发；未发现 GPL/AGPL/LGPL。
- 复查发现两处许可文本缺失：Skia 静态链接的 expat（MIT）、资源字体
  Roboto Mono（Apache-2.0）。已新增 `licenses/expat.LICENSE.txt` 与
  `licenses/RobotoMono.LICENSE.txt`，并在 README 第三方库表中登记；
  打包脚本会随 `licenses/` 一并收集。
- 确认 Skia 实际仅启用 expat 一个 third_party（`args.gn` 关闭了 libpng、zlib、
  libjpeg-turbo、libwebp、ICU），因此无需补充其余 Skia 第三方许可。
- 验收：对照表覆盖全部随包二进制与许可文本；`--list` 中 `lib/expat.lib`、
  `licenses/expat.LICENSE.txt` 与 `licenses/RobotoMono.LICENSE.txt` 均已随包。

### P2：维护与体验

#### P2-1 pack.py 自动化测试与 CI

- [ ] 为链接重写、MANIFEST、SHA256SUMS、元数据推断、接入文件生成编写测试。
- [ ] 接入 CI，至少在 Windows 与一个 Unix 平台运行。
- 现状：此前验证依赖临时脚本，已删除，无回归保护。
- 验收：测试可在本地与 CI 稳定运行，覆盖主要分支。

#### P2-2 包体检命令

- [ ] 增加 `pack.py --verify <包路径>`：校验布局、SHA、文档断链、头文件完整性。
- 验收：损坏包与缺文件包能给出明确错误；正常包返回 0。

#### P2-3 文档补充

- [ ] 安装、升级、迁移说明。
- [ ] 常见问题（运行期 DLL 缺失、资源目录、架构/配置不匹配、CEF 运行时）。
- [ ] 发布 checklist。
- 验收：新用户按文档可从下载走到可运行程序。

#### P2-4 arm64 策略

- [ ] 明确 arm64 的支持范围，或补齐 manifest 与相关产物。
- 现状：`--arch arm64` 允许出包，但没有对应 manifest，部分依赖可能缺失。
- 验收：文档明确“支持/不支持”；若支持，矩阵中有对应验证记录。

## 四、建议推进顺序

1. P0-1 全矩阵与 P0-3 接入层冒烟，同步修复 P1-1 归档语义；
2. P0-5 发布 checklist；
3. P1 其余项（优先 P1-3 CEF 运行时）、P2 长期项按版本节奏推进。

> P0-2 与 P0-4 已明确关闭，不再排期。

## 五、更新记录

| 日期 | 变更 |
| :--- | :--- |
| 2026-10-23 | 创建，整理当前基线与待办清单 |
| 2026-10-23 | P0-2、P0-4 明确关闭；P1-3 补充 CEF 二进制目录现状与平台范围 |
| 2026-10-23 | 补充平台约束（Linux 无 x86、FreeBSD 无 CEF）；记录 Windows/Linux CEF 运行时结构与 SDL3 符号链接结构 |
| 2026-10-23 | 修复 macOS 编译错误：仓库根 `VERSION` 重命名为 `VERSION.txt`，避免与标准头 `<version>` 冲突 |
| 2026-10-23 | 记录 macOS `bin/` 结构：示例为可执行文件，CEF 示例为 `cef.app`/`CefBrowser.app`（含 Framework 与 Helper）；明确 macOS CEF 仍由下游手动配置 |
| 2026-10-23 | 记录 macOS CEF 分发包位置与结构：同级 `cef_binary/`，含 `Release/Chromium Embedded Framework.framework`；与仓库内 wrapper 目录用途不同 |
| 2026-10-23 | P0-5 方案定稿并存入 [Release-Process-Closure-Plan.md](Release-Process-Closure-Plan.md)，暂不实施 |
| 2026-10-23 | 确定版本号单一源方案：`duilib/duilib_version.h` 为唯一编辑点，删除 `VERSION.txt`，`msvc/dllRes/version.h` 改为适配层，`BUILD` 段不补零 |
| 2026-10-23 | 版本号单一源已实施：`pack.py` 解析新头文件，`--list` 版本为 `1.0.2026.1023`；`VERSION.txt` 已删除 |
| 2026-10-23 | P1-3 Windows 运行时已随包：`collect_cef_runtime()` 收集 `bin/libcef_win[_109]/<架构>/`，排除 `.lib`；Linux 待 `--with-cef` 支持，macOS 按设计手动 |
| 2026-10-23 | P1-1 已实施：tar 按源权限写模式（保留可执行/setuid），zip 用 `S_IFLNK` 条目保存符号链接，`iter_files()` 补目录链接，符号链接不计入 `SHA256SUMS` |
| 2026-10-23 | P1-2 已实施：支持 `SOURCE_DATE_EPOCH`，统一归档条目/gzip 头/MANIFEST 时间；未设置时用源文件与产物最新 mtime，条目排序，两次打包摘要一致 |
| 2026-10-23 | P1-4 已实施：MANIFEST 运行期与 Release 文档标注 WebView2 依赖系统 Evergreen Runtime，补充 loader 用法与常见问题 |
| 2026-10-23 | P1-5 已实施：MANIFEST 与 Release 文档补充 PowerShell `Get-FileHash` 全包校验脚本及 `SHA256SUMS.txt` 用法 |
| 2026-10-24 | P1-6 已实施：新增许可合规对照表；补充 expat（MIT）与 Roboto Mono 字体（Apache-2.0）许可文本并在 README 登记；确认随包组件均为宽松许可、无 GPL/AGPL/LGPL |
