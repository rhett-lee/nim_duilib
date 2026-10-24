# 构建/打包脚本控制台输出英文化

## 目标

在纯英文运行环境中，`build/` 下的编译、打包、发布脚本不得向控制台输出中文，
避免出现乱码或无法显示的字符。范围限定为**运行期控制台输出**（stdout / stderr、
告警、错误、进度、`--help` 文本）；**注释与 docstring 保留中文**，
包内生成的 `MANIFEST.txt` 等文件内容保持现有中文不变。

## 约束

- 所有控制台消息改为 ASCII 英文；不改变逻辑、退出码、文件收集行为。
- 中文注释、文档字符串、`MANIFEST.txt`、`RELEASE-NOTES`（来自 CHANGELOG）不改。
- 输出标签保持原有对齐风格（如 `Version   : `）。
- 不新增/删除命令行参数；`--help` 语义与参数说明保持一致。

## 改动文件与要点

### 1. `build/pack.py`

**函数级改动（仅输出字符串）：**

| 位置 | 内容 |
| :--- | :--- |
| `read_version` | `[ERROR] 找不到版本头文件` → `[ERROR] version header not found`；`读取版本头文件失败` → `failed to read version header`；`版本宏重复定义` → `duplicate version macros`；`版本宏缺失` → `missing version macros`；`版本宏超出范围` → `version macro out of range (0..65535)` |
| `collect_headers` | `third_party/... 不存在，头文件可能不完整` → `third_party/... is missing; headers may be incomplete`；`缺少核心头文件 ...` → `missing core header include/duilib/duilib.h (source dir: ...)` |
| `collect_libs` | `启用了 --with-cef，但库目录中缺少` → `--with-cef was requested but %s is missing from the library directory`；`存在 CEF 库 ...，未打包；需要请加 --with-cef` → `CEF library %s (%s) found but not packaged; add --with-cef to include it`；`缺少基础库` → `missing base libraries: %s (dir: %s); build %s first` |
| `collect_skia_libs` | `Skia 目录中没有 ... 静态库` → `no %s static libraries in Skia directory: %s` |
| `resolve_skia_lib_dir` | `--skia-lib-dir 不存在` → `--skia-lib-dir does not exist: %s`；`未找到 Skia 库目录（尝试过：...；可用：...）` → `Skia library directory not found (tried: %s; available: %s); use --skia-lib-dir` |
| `collect_sdl` | `SDL3 目录不存在` → `SDL3 directory does not exist: %s (use --sdl-dir)`；`启用了 --with-sdl，但 ... 下没有...` → `--with-sdl was requested but no SDL3 static/import library exists under %s (tried: %s)`；`缺少 SDL3 运行期 DLL` → `missing SDL3 runtime DLL: %s (downstream programs will fail to run)`；`SDL3 动态库不存在` → `SDL3 dynamic library not found (looked in %s)` |
| `collect_webview2_loader` | `WebView2 没有 ... 架构的 loader，已跳过` → `WebView2 has no loader for %s; skipped`；`缺少 WebView2 loader` → `missing WebView2 loader: %s` |
| `collect_duilib_runtime_dll` | `bin/...dll 的架构（0x....）与目标 ... 不匹配，已跳过` → `bin/%s.dll architecture (0x%04X) does not match target %s; skipped` |
| `collect_jpeg_turbo` | `libjpeg-turbo 上游没有 Windows ... 预编译库，已跳过` → `libjpeg-turbo has no prebuilt Windows %s library upstream; skipped`；`缺少 turbojpeg-static.lib` → `missing turbojpeg-static.lib: %s` |
| `collect_cef` | `--with-cef 目前仅支持 Windows 打包` → `--with-cef is currently supported on Windows only`；`CEF 上游没有 Windows ... 预编译包` → `CEF has no prebuilt Windows %s package upstream`；`缺少 %s` → `missing %s` |
| `collect_cef_runtime` | `CEF 运行时没有 ... 架构的目录，已跳过` → `no CEF runtime directory for %s; skipped`；`CEF 运行时目录不存在：...（本包只有头文件与库，带 CEF 的示例程序无法直接运行）` → `CEF runtime directory does not exist: %s (this package ships headers and libraries only; CEF examples cannot run directly)` |
| `collect_resources` | `资源目录不存在：...（框架启动必需）` → `resources directory does not exist: %s (required at startup)` |
| `collect_licenses` | `许可证目录不存在` → `licenses directory does not exist: %s`；`缺少 Skia 许可证` → `missing Skia license: %s` |
| `collect_docs` | `文档目录不存在` → `docs directory does not exist: %s` |
| `collect_examples` | `bin 目录不存在` → `bin directory does not exist: %s`；`Debug 包中的示例程序可能混有 Release 版本...` → `Debug package may contain Release example binaries (CMake reuses names); verify before distributing`；`已跳过 Debug 示例程序` → `skipped Debug example binaries: %s`；`启用了 --with-examples，但 ... 下没有可打包的示例程序` → `--with-examples was requested but no example binaries were found under %s` |
| `_render_template` | `接入模板不存在：...（应位于仓库 packaging/ 目录）` → `integration template not found: %s (expected under packaging/)`；`接入模板 ... 存在未替换的占位符` → `integration template %s has unreplaced placeholders: %s` |
| `collect_integration_files` | `缺少 manifest：...（VS 工程不会自动嵌入 Common Controls v6）` → `missing manifest: %s (VS projects will not embed Common Controls v6)` |
| `read_source_date_epoch` | `SOURCE_DATE_EPOCH 必须是整数秒` → `SOURCE_DATE_EPOCH must be an integer number of seconds: %r`；`不能为负数` → `SOURCE_DATE_EPOCH must not be negative: %d` |
| `resolve_lib_dir` | `--lib-dir 不存在` → `--lib-dir does not exist: %s`；`库目录不存在：...（请先完成 ... 的构建）` → `library directory does not exist: %s (build %s|%s first)` |
| `verify_package` | `包不存在` → `package does not exist: %s`；`无法打开包` → `cannot open package %s: %s`；`包根目录不唯一` → `multiple package root directories: %s`；`缺少 %s` → `missing %s`；`缺少 include/duilib/ 头文件` → `missing include/duilib/ headers`；`缺少 lib/ 静态库（.a / .lib）` → `missing static libraries under lib/ (.a / .lib)`；`SHA256SUMS 行格式异常` → `malformed SHA256SUMS line: %s`；`未登记到 SHA256SUMS` → `file not listed in SHA256SUMS: %s`；`SHA256SUMS 登记了包内不存在的文件` → `SHA256SUMS lists a file missing from the package: %s`；`SHA256 不匹配` → `SHA256 mismatch: %s`；`…（已发现 %d 处不匹配，不再逐个列出）` → `... (%d mismatches found; stopping listing)`；`文档链接失效` → `broken document link: %s -> %s`；`包内含 %d 个符号链接（未计入 SHA256SUMS）` → `package contains %d symbolic links (not included in SHA256SUMS)`；`[FAIL] ...：%d 个条目...` → `[FAIL] %s: %d entries, %d errors, %d warnings`；`[OK] ...：%d 个条目（%d 文件 / %d 链接），%d 个头文件，%d 个库，SHA 全部匹配` → `[OK] %s: %d entries (%d files / %d links), %d headers, %d libraries, all SHA256 match` |
| `_parse_config` | `配置只能是 Release 或 Debug` → `config must be Release or Debug: %r` |
| `main`（argparse） | `description='nim_duilib 发布打包脚本'` → `'nim_duilib release packaging script'`；全部 `help=` 文本改英文（`--config/--arch/--lib-dir/--skia-lib-dir/--mingw-variant/--sdl-dir/--with-sdl/--with-cef/--cef-version/--msvc-runtime/--with-examples/--format/--out-dir/--list/--allow-missing/--verify`） |
| `main`（摘要） | 标签改为 `Version / Platform / Config / Toolchain / Lib dir / Skia dir / Format / Examples / Headers / Libraries / Skia libs / SDL3 / WebView2 / Runtime DLLs / JPEG / CEF / CEF runtime / Resources / Licenses / Docs / Examples / Integration / Total`；`是/否` → `yes/no`；`工具链 MinGW（llvm-mingw）/MinGW（gcc）` → `MinGW (llvm-mingw)/MinGW (gcc)`；`告警：` → `Warnings:`；`合计 ... 个文件` → `Total     : %d files, %s`；`接入文件 ...（生成 ...）` → `Integration: %d (%d generated)` |
| `main`（关键产物阻断） | `关键产物缺失，已阻止出包` → `critical artifacts are missing; packaging aborted`；`仅需先出包排查时可加 --allow-missing（不要用于发布）。` → `use --allow-missing only for troubleshooting (never for releases).` |
| `main`（失败/产物） | `打包失败` → `packaging failed`；`产物      : %s（%s）` → `Artifact  : %s (%s)` |

**保持中文**：所有注释、docstring、`write_manifest` 生成的 `MANIFEST.txt` 文本。

### 2. `build/release.py`

- `cmd_check`：`版本/工作区检查/版本号解析/tag 检查/CHANGELOG 检查/检查未通过/检查通过。以下命令由维护者手动执行：` 及其失败原因（`执行 git status 失败`、`工作区有已跟踪文件的未提交修改`、`执行 git tag 失败`、`tag ... 已存在，不能重复发布`、`版本未严格递增`、`缺少 CHANGELOG`、`没有当前版本 [...] 的条目`、`中英 CHANGELOG 的版本标题不一致`）全部改英文。
- `cmd_package`：`版本/平台/配置/输出目录/预计产物/执行命令`、`（--dry-run：未执行打包）`、`pack.py 退出码`、`包体检未通过` 改英文。
- `cmd_finalize`：`目录不存在`、`未找到压缩包`、`版本/目录/校验`、`SHA256、必备文件、文档链接`、`存在未通过校验的压缩包，未生成发布说明`、`已生成`、`以下文件缺少版本 [...] 条目` 改英文；`发布说明`/`Release Notes` 标题属于生成文件内容，保留。
- 校验辅助函数：`_verify_sha256sums`、`_check_doc_links`、`_verify_package` 返回的失败字符串改英文（`缺少 SHA256SUMS`、`行格式错误`、`引用的文件不存在`、`SHA256 不匹配`、`缺少 docs/ 目录`、`链接目标不存在`、`缺少必备文件`、`VERSION 内容为 ...，期望 ...`、`解压或校验异常`）。
- `cmd_clean`：`dist/ 不存在，无需清理`、`删除需要显式 --yes`、`拒绝删除当前版本目录`、`版本目录不存在`、`已删除`、`dist/ 下没有版本目录`、`dist/ 版本目录：`、`（当前版本）`、`默认不删除任何内容；如需删除旧版本：`、`python ... <版本> --yes` 改英文。
- `_parse_config`、`build_parser`：description 与全部 `help=` 改英文；`已中断` 改英文。
- 保持中文：注释、docstring、`RELEASE-NOTES` 内容。

### 3. `build/autotest.py`

- `PRESETS` 的 `description`、`--list-presets` 末行（`运行所有前置条件满足的预设`）改英文。
- `check_prereq`、`_build_command`、`_run_step`（超时）、`inspect_package`、`_smoke` 的失败/跳过原因改英文。
- `_run_pack_preset` / `_run_synthetic_arm64` / `_run_unit` 的 `reason`、`assertions` 文本改英文（`打包失败`、`未找到新生成的包`、`包体检未通过`、`结构断言通过/未通过`、`冒烟：`、`缺少 duilib.arm64.manifest` 等）。
- `_print_summary`：`通过 %d，失败 %d，跳过 %d` → `passed %d, failed %d, skipped %d`；`包：` → `package: `。
- `main`：argparse description/help、日志首行 `nim_duilib 打包自动化`、`日志：`、`报告：` 改英文。
- 保持中文：注释、docstring。

### 4. `build/build_duilib_all_in_one.sh`（11 行）

`忽略 gcc/clang：目标不是 mingw-w64` → `Ignoring gcc/clang: target is not mingw-w64 (...)`；
`未找到目标为 mingw-w64 的 gcc/clang` → `No gcc/clang targeting mingw-w64 found`；
`请在 MSYS2 的 MINGW64/UCRT64/CLANG64 环境下运行` → `Please run inside an MSYS2 MINGW64/UCRT64/CLANG64 environment`；
`nim_duilib/skia/skia_compile/SDL 更新失败，继续使用本地代码` → `... update failed; continuing with the local copy`；
`... 获取失败（已尝试 N 次）` → `... fetch failed (after N attempts)`。

### 5. `build/msys2_build.sh`（5 行）

`忽略 clang/gcc：目标不是 mingw-w64`、`需要 mingw-w64 目标的编译器（...）`、`未找到 ninja/make/mingw32-make：请先安装其中任意一个构建工具`、`无效的 DUILIB_BUILD_TYPE：...（仅支持 Debug / Release）` 改为英文对应文本。

### 6. 测试同步

- `build/test_pack.py`：`CriticalArtifactsTest` 中依赖中文告警的断言改为英文子串
  （`资源目录` → `resources directory`，`CEF 运行时` → `CEF runtime`，`示例程序` → `example binaries`）。
  `MANIFEST` 相关中文断言保持不变。
- `build/test_autotest.py`：执行时检查是否依赖中文 reason/assertion；若有则同步。

### 7. 不改动

- `build/*.bat`（中文仅出现在 `@REM` 注释中，控制台输出已是英文）。
- `MANIFEST.txt` 生成逻辑、`RELEASE-NOTES`。
- 一次性维护脚本（`apply_*.py`、`mark_*.py`、`fix_*.py`、`verify_*.py`、
  `write_release_docs.py` 等），不属于编译/打包流程。

## 验收

1. `python build/test_pack.py`（39 用例）全部通过。
2. `python build/test_autotest.py`（17 用例）全部通过。
3. 设置 `PYTHONIOENCODING=ascii` 后运行以下命令，stdout/stderr 均为纯 ASCII：
   - `python build/pack.py --help`
   - `python build/pack.py --list`（需已构建产物；否则以版本头文件/库缺失错误验证）
   - `python build/release.py --help`
   - `python build/release.py check`
   - `python build/autotest.py --help`
   - `python build/autotest.py --list-presets`
   - `python build/autotest.py --preset unit`
4. 用脚本扫描 `pack.py` / `release.py` / `autotest.py` 中 `print(`、`eprint(`、
   `warn(`、`PackageError(`、`help=`、`description=`、`assertions`、`reason` 等
   输出上下文，确认不再含 CJK；`write_manifest` 区域与注释除外。
5. `build_duilib_all_in_one.sh`、`msys2_build.sh` 的 `echo` 行不含 CJK。
6. 不提交代码；只保证 Debug 构建与上述测试通过。
