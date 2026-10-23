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
| 已验证 | `--list`、XML 解析、CMake 配置、SHA/断链检查、真实 zip 抽查 |

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

- [ ] 用真实发布包跑通 VS 与 CMake 的最小工程，而不仅是配置阶段。
- 待验证：
  - MSBuild：导入 `msvc/nim_duilib.targets`，Debug/x86 与 Debug/x64 各编译、链接、运行一次；
  - CMake：Windows 与 Linux 各编译、运行一次；
  - DLL 模式：`bin/*.dll` 复制、`bin/resources` 相对路径、manifest 嵌入效果；
  - `NimDuilibCopyResources=1` / `nim_duilib_copy_runtime(... RESOURCES)` 的复制结果。
- 验收：生成的可执行程序能启动，并能加载主题资源；失败项在文档中记录原因。

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

- [ ] tar 内文件权限按类型设置，不再统一 `0o644`。
- [ ] 符号链接按 `lstat` 处理，避免 zip 展开 `.so` 版本链产生重复实体。
- 现状：`--with-examples` 的 Unix 示例程序解包后不可执行；zip 与 tar 对符号链接行为不一致。
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

#### P1-2 可复现构建

- [ ] 支持 `SOURCE_DATE_EPOCH`，固定 zip 条目时间与 tar mtime/gzip 头。
- 现状：条目时间取当前时间，同一输入两次打包 SHA256 不同。
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
- [ ] 打包脚本按上述结构收集对应平台/架构的运行时，保持相对结构放入包内 `bin/`。
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
- 现状：仅打包 CEF 头文件与 `libcef.lib`，运行时尚未随包。
- 验收：带 CEF 的 Windows/Linux 示例程序在干净机器上无需额外拷贝即可启动；
  `chrome-sandbox` 的可执行权限正确；macOS 按手动配置流程验证。

#### P1-4 WebView2 运行时说明

- [ ] 在 `MANIFEST.txt` 与接入文档中明确“依赖系统 Evergreen Runtime”。
- 现状：只带 `WebView2Loader.dll`，容易被误认为开箱即用。
- 验收：文档与 MANIFEST 均有说明，并有对应的常见问题条目。

#### P1-5 校验方式对 Windows 友好

- [ ] 提供 `verify.py` 或 PowerShell `Get-FileHash` 示例。
- 现状：文档只写 `sha256sum -c`，Windows 默认没有该命令。
- 验收：Windows 用户可按文档一步完成全包校验。

#### P1-6 许可合规复查

- [ ] 逐项确认随包分发的第三方组件许可：Skia 及其第三方（expat、skcms、bentleyottmann、skresources 等）、CEF、WebView2、SDL、libjpeg-turbo、libwebp、libpng、zlib。
- 现状：`licenses/` 已覆盖主要组件，但未逐项核对分发条款。
- 验收：形成“组件 → 许可证 → 随包分发是否允许 → 包内文件”的对照表。

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
