# P2-1 打包自动化 与 P2-4 arm64 接入支持

## 1. 背景与目标

待办来源：[Packaging-Follow-Up-Items.md](../../Packaging-Follow-Up-Items.md) 的 P2-1、P2-4。

- **P2-1**：为 `build/pack.py` 的关键逻辑补自动化测试，并提供可重复执行的
  「检测环境 →（可选）编译 → 打包 → `--verify` →（可选）冒烟」自动化脚本；
  **不接入 CI**（用户明确不做 CI 部分）。
- **P2-4**：支持 arm64 的**接入文件与产物结构**（manifest、`pack.py` 收集、
  VS props、CMake 架构识别、Skia 目录映射、文档支持范围）；**不做真机编译/打包/
  运行验证**（无设备）。缺 arm64 预编译依赖时给出明确错误，而不是静默出坏包。

约束：

- 改动仅保存在本地，不提交（用户自行提交）。
- 不修改用户现有 `bin/`、`lib/` 产物；自动化默认只读现有产物，输出到临时目录。
- 中间产物、日志、报告写入 `C:\develop\temp\autotest\`。
- 文档与注释使用中文。

## 2. P2-4：arm64 支持范围

最终支持范围（写入文档与 MANIFEST）：

| 项 | arm64 状态 |
| :--- | :--- |
| 头文件 / 基础静态库 / Skia / CMake config / VS props / targets | 结构已就绪；需要 arm64 构建产物 |
| Common Controls manifest | 本次补齐 `duilib.arm64.manifest`，随包并在 VS 工程中嵌入 |
| CMake / MSBuild 架构识别 | 本次补齐 ARM64 识别与 manifest 选择 |
| Skia 目录 | 映射 `llvm.arm64.release` / `llvm.arm64.debug`（DLL 模式 `.md` / `.mdd`）；目录缺失时报错 |
| CEF | 上游无 Windows arm64 预编译包，`--with-cef --arch arm64` 明确报错退出 |
| libjpeg-turbo | 无 arm64 预编译静态库，打包告警并跳过 |
| WebView2 / SDL3 | 沿用现有按架构探测；缺产物时保持现有告警/跳过语义 |
| 真机编译、打包、运行验证 | **未做**（无测试设备），发布时不得声称已验证 |

### 2.1 具体改动

1. **新增** `msvc/manifest/duilib.arm64.manifest`
   - 以 `duilib.x64.manifest` 为模板；
   - `assemblyIdentity` 与 Common-Controls 依赖的
     `processorArchitecture` 改为 `ARM64`；
   - 保留全部 `supportedOS` 与文件头。
2. **`build/pack.py`**
   - 新增模块常量 `WINDOWS_MANIFEST_ARCHES = ('x64', 'x86', 'arm64')`；
   - `collect_integration_files()` 的 manifest 循环改用该常量（约 1525 行），
     使 Windows 包同时收集三份 manifest；
   - `collect_jpeg_turbo()`：arm64 保持告警跳过，告警文案明确「上游无 arm64 预编译库」；
   - `collect_cef()`：arm64 报错文案明确「CEF 上游无 Windows arm64 预编译包」；
   - `write_manifest()`：在 Windows 包的「接入文件」段落注明 arm64 支持范围
     （结构就绪、未做真机验证；CEF/JPEG 不适用）。
3. **`packaging/msvc/nim_duilib.props`**
   - 新增 `NimDuilibManifestArch` 小写映射（x64 / x86 / arm64）；
   - Manifest `ItemGroup` 条件改为 `'$(NimDuilibManifestArch)'!=''`，
     Include 使用 `duilib.$(NimDuilibManifestArch).manifest`；
   - 更新顶部注释（arm64 已配套 manifest）。
4. **`packaging/cmake/nim_duilibConfig.cmake`**
   - Windows 架构识别：`CMAKE_GENERATOR_PLATFORM` 优先；无该变量时先按
     `CMAKE_SYSTEM_PROCESSOR` 匹配 `aarch64|arm64`，再按 `CMAKE_SIZEOF_VOID_P`
     判 x86/x64（修正 Ninja/ARM64 主机误判为 x64）。
5. **`msvc/PropertySheets/SkiaSettings.props`**
   - 新增 `Release|ARM64`、`Debug|ARM64` 的 `SkiaLibDir`；
   - 新增 DLL 模式 `llvm.arm64.release.md`、`llvm.arm64.debug.mdd`。
6. **`cmake/duilib_common.cmake`**
   - `DUILIB_SKIA_CPU_ARCH` 在无 generator platform 时，先按
     `CMAKE_SYSTEM_PROCESSOR` 识别 arm64，再回退指针宽度。
7. **`packaging/msvc/nim_duilib.targets`**
   - 仅当其中存在架构白名单/校验时补 arm64；不改变其他行为。
8. **文档**
   - `docs/misc/Packaging-Follow-Up-Items.md`：更新 P2-1、P2-4 状态与更新记录；
   - `build/README.md` / `build/README.en.md`：补充 `autotest.py` 用法；
   - `docs/Release.md` / `Release.en.md`：包名/架构说明补 arm64 支持范围（如已有架构表）。

## 3. P2-1：测试与自动化

### 3.1 `build/test_pack.py` 扩展（不依赖真实构建产物）

新增用例分组：

1. **包名 / 元数据**
   - arm64（Windows、MinGW、DLL）包名；
   - `--msvc-runtime auto/mt/md` 元数据推断（用临时假库目录）。
2. **MANIFEST 生成**
   - `write_manifest()` 使用临时 collector + 固定版本/平台/SHA，
     断言包含版本、平台、配置、SHA256SUMS 说明、arm64 支持范围说明；
   - `SOURCE_DATE_EPOCH` 固定时 MANIFEST 时间稳定。
3. **接入文件生成**
   - `collect_integration_files()` 输出包含 `msvc/manifest/duilib.x64.manifest`、
     `duilib.x86.manifest`、`duilib.arm64.manifest`、`msvc/nim_duilib.props`、
     `msvc/nim_duilib.targets`、`cmake/nim_duilibConfig.cmake`、
     `nim_duilib.sdk.props`；
   - 生成文件中的版本/架构/配置/工具链与传入元数据一致。
4. **arm64 相关**
   - `duilib.arm64.manifest` 内容含 `processorArchitecture="ARM64"`；
   - `resolve_skia_lib_dir()`：临时 `llvm.arm64.release` + 假 `.lib` 可解析；
     缺失时错误信息包含 arm64 与候选目录；
   - `collect_jpeg_turbo()` 对 arm64 产生告警且不收集；
   - `collect_cef()` / `collect_cef_runtime()` 对 arm64 的行为（报错/告警）。
5. **链接重写 / 文档相对链接**（沿用现有 `verify_package` 覆盖）
   - 构造含 `docs/Summary.md` 相对链接的包，正确链接通过、断链失败；
   - 新增 zip/tgz 两种格式的断链负向用例。
6. **防回归**
   - `WINDOWS_MANIFEST_ARCHES` 与三份 manifest 文件实际存在性一致。

保持 `unittest` 单文件、零第三方依赖；全部用例可在 Windows 工作区 Python 与
MSYS2 Python 下运行。

### 3.2 新增 `build/autotest.py`

**定位**：本地可重复执行的打包自动化，不做 CI；默认只读，输出隔离。

CLI：

```
python build/autotest.py --list-presets
python build/autotest.py --preset unit
python build/autotest.py --preset pack-msvc-x64-release [--with-examples]
python build/autotest.py --preset arm64-synthetic
python build/autotest.py --preset all-ready
python build/autotest.py --preset pack-msvc-x64-release --build
python build/autotest.py --preset ... --dry-run
python build/autotest.py --preset ... --out-dir C:\develop\temp\autotest\run1
```

预设（`PRESETS` 表：名称 → lib 架构/配置/工具链、pack 参数、是否需要构建）：

| 预设 | 内容 | 前置条件 |
| :--- | :--- | :--- |
| `unit` | 运行 `test_pack.py` 与 `test_autotest.py` | Python |
| `pack-msvc-x64-release` | 用现有 `lib/x64/Release` 打包 + 体检 | `lib/x64/Release`、Skia |
| `pack-msvc-x64-debug` | `lib/x64/Debug` | 同上 |
| `pack-msvc-x86-release` | `lib/x86/Release` | 同上 |
| `pack-mingw-x64-release` | MSYS2/MinGW 现有产物 | `lib/x64/Release` + gcc/llvm Skia |
| `arm64-synthetic` | 临时假库 + `--arch arm64` 出包，校验 arm64 manifest/接入文件 | 无（不需要真机） |
| `all-ready` | 依次运行所有前置条件满足的预设，缺条件记为 skipped | — |

单次执行流水线：

1. **环境检测**：OS/架构、Python 版本、`MSBuild`/`cl`、`gcc`/`clang`、
   仓库版本、`lib/<arch>/<config>`、Skia 候选目录；
2. **可选构建**（仅 `--build`）：按预设调用现有脚本
   （MSVC：`build/msvc_build.bat`；MSYS2：`build/msys2_build.sh`），
   记录命令、超时（默认 3600 s）与退出码；失败立即终止并保留日志；
3. **打包**：调用 `pack.py`（`--out-dir` 指向本次运行目录），先 `--list`
   再实际打包；
4. **验证**：对产物调用 `pack.py --verify`；额外做结构断言
   （`MANIFEST.txt`、`SHA256SUMS`、`lib/cmake/nim_duilib/nim_duilibConfig.cmake`、
   Windows 包三份 manifest；arm64 包断言 manifest 架构为 ARM64）；
5. **冒烟**（可选，`--smoke`）：解包到临时目录，若 `bin/basic.exe` 存在且
   架构匹配，启动并等待 8 s，检查进程存活/退出码，然后结束进程；
6. **报告**：输出 `report.json`（环境、命令、耗时、包路径/大小、verify 结果、
   断言、跳过原因）与 `autotest.log`；控制台打印摘要；
   退出码：0=全部通过，1=有失败，2=仅跳过/前置条件不足。

安全约束：

- 默认不构建、不写 `lib/`、`bin/`、`dist/`；包输出到 `--out-dir`（默认
  `C:\develop\temp\autotest\<时间戳>\dist`）；
- `--dry-run` 只打印将执行的命令；
- 不自动提交、不修改版本号、不删除现有产物（临时目录可选 `--keep` 保留）。

### 3.3 `build/test_autotest.py`

纯单元测试（不构建、不打包）：预设表完整性、命令拼装、前置条件判定、
`--dry-run` 不执行外部命令、报告字段、退出码映射、arm64 合成库创建内容。

## 4. 验收标准

1. `python build/test_pack.py`、MSYS2 Python 同命令全部通过；
2. `python build/test_autotest.py` 全部通过；
3. `python build/autotest.py --preset unit` 退出码 0；
4. `python build/autotest.py --preset arm64-synthetic` 退出码 0：
   产出 zip 内含 `msvc/manifest/duilib.arm64.manifest` 且架构为 ARM64，
   `--verify` 通过；
5. 前置条件满足的 `pack-*` 预设：`--verify` 通过，报告记录包大小与 SHA 校验结果；
6. 文档明确 arm64「结构就绪、未真机验证」与 CEF/JPEG 不适用；
7. 不产生 Release 构建要求；Debug 编译不受影响（本任务不要求全量编译）。

## 5. 待批准事项

- 本 spec 覆盖 P2-1（不含 CI）与 P2-4（不含真机验证）；
- 是否按上述文件清单实施；
- 是否接受新增 `docs/misc/specs/P2-Packaging-Automation-Arm64/` 下的
  `spec.md`、`tasks.md`、`checklist.md`。

无人值守模式下按用户已下达的任务授权直接实施，不等待交互式确认；
实施完成后在最终汇报中列出审批点与结果。
