# 编译脚本简化方案

本文针对「build 目录文件多、入口分散、新手必须先读大量文档」的问题给出优化方案。
目标：**一个入口、一条命令、一屏文档**。本方案与
`docs/misc/Root-CMake-Build-Plan.md`（根 CMake 工程）配套实施。

## 1. 现状盘点

`build/` 下已纳入版本控制的文件共 22 个，按用途可分为四类：

| 类别 | 文件 | 问题 |
| :--- | :--- | :--- |
| 一键脚本 | `build_duilib_all_in_one.sh`、`build_duilib_all_in_one.bat`、`build_duilib_all_in_one_mingw-w64.bat` | 三个名字相近，平台/编译器组合交叉 |
| 单平台脚本 | `linux_build.sh`、`macos_build.sh`、`freebsd_build.sh`、`msys2_build.sh`、`msvc_build.bat`、`gcc-mingw-w64_build.bat`、`llvm-mingw-w64_build.bat`、`detect_vs_version.bat` | 8 个入口，参数与默认值各不相同 |
| IDE 工程 | `duilib.sln`、`duilib_no_cef.sln`、`examples.sln`、`examples_no_cef.sln` | 合理保留，但对命令行用户是干扰 |
| 工具/文档 | `pack.py`、`release.py`、`autotest.py`、`test_*.py`、`README.md`、`README.en.md`、`build.md` | `README` 约 30 KB，`build.md` 与之部分重复 |

结论：文件绝对数量不大，**真正的问题在入口分散、命名相似、文档过长**。

常见新手困惑：

1. “一键”和“单平台”该用哪个？
2. `build_duilib_all_in_one.sh` 与 `msys2_build.sh` 在 Windows 上是什么关系？
3. Skia/SDL/CEF 到底要不要先准备？哪个脚本会自动准备？
4. 出错后该看 `README.md` 的哪一节？4 个 `.sln` 又该用哪个？

## 2. 目标与原则

1. **单一入口**：命令行构建只有一个命令，平台与编译器自动识别；
2. **默认即可用**：默认配置能直接跑通（依赖首次自动准备、构建库与示例）；
3. **渐进披露**：README 第一屏只保留“三条命令 + 一张决策表”，细节放到参考手册；
4. **不留历史包袱**：新入口跑通并验证后，直接删除旧脚本，不保留转发层；
5. **不重复实现**：依赖准备与构建逻辑下沉到根 CMake，脚本只做“识别环境 + 调 CMake”。

## 3. 方案对比

| 方案 | 做法 | 优点 | 缺点 |
| :--- | :--- | :--- | :--- |
| A. 只整理文档 | 合并 README/build.md，加决策表 | 成本最低 | 入口仍分散，治标不治本 |
| B. 单入口 + 根 CMake（推荐） | 根 CMake 承担构建，新增 `build/build.py` 子命令入口 | 一处逻辑、跨平台一致、易扩展 | 依赖 `Root-CMake-Build-Plan` 先落地 |
| C. 只用 CMake Presets | 删除全部脚本，直接 `cmake --preset` | 无自定义脚本 | 依赖准备需多步；新手仍要理解 preset |

推荐 **B**：以根 CMake 为底座，Python 入口负责环境识别与子命令编排。
Python 已是 `pack.py` / `release.py` / `autotest.py` 的前置条件，不引入新的运行时依赖。

## 4. 目标目录结构

```
build/
  build.py              # 唯一命令行入口（子命令）
  build.bat             # Windows 薄封装：寻找 py/python 后转调 build.py
  pack.py               # 出包（保持现状）
  release.py            # 发布流程（保持现状）
  autotest.py           # 自动化（保持现状）
  test_pack.py
  test_autotest.py
  test_build.py         # 入口自身测试（标准库 unittest）
  duilib*.sln           # IDE 方式，保留
  examples*.sln
  README.md             # build 目录说明、各平台前置依赖、发布打包与排障索引（保留）
  README.en.md
  build-reference.md    # 新入口完整参考手册（替代 build.md）
  build-reference.en.md # 英文版参考手册（与 docs/ 的 *.en.md 规则一致）
```

旧的 11 个构建脚本（3 个 all-in-one、8 个单平台）在新入口验证通过后直接删除，
`build/build.md`（旧脚本对照表）同时删除，不作为长期并存入口。`build/README.md`
与 `.en.md` 保留，但删除其中的旧脚本表，改为指向 `build-reference.md`。

## 5. 入口设计

### 5.1 子命令

| 命令 | 作用 |
| :--- | :--- |
| `python build/build.py [--config Release\|Debug]` | 默认：自动确保依赖就绪 + 构建**一个**配置（默认 Release）+ 库与示例 |
| `build.py deps [--config ...] [--force]` | 只准备当前配置所需的 Skia/SDL/CEF |
| `build.py build [--config ...] [--no-deps] [--target ...]` | 构建一个配置；默认先确保依赖，`--no-deps` 跳过检查 |
| `build.py test [--config ...]` | 调用 `test_pack.py` / `test_autotest.py` |
| `build.py pack [--config ...] [--with-cef]` | 调用 `pack.py`，收集对应配置的产物，并生成 `LICENSES/` 与 `THIRD-PARTY-NOTICES.md` |
| `build.py doctor` | 环境自检（复用根 CMake 两阶段预检；无 CMake 时 Python 内置探测，见 `Root-CMake-Build-Plan.md` 1.3） |
| `build.py clean [--build-dir ...] [--deps]` | 清理构建树：默认删除整个 `out/`（所有架构/配置）；`--build-dir` 可收窄；默认**不清理**依赖缓存，`--deps` 才清理依赖产物 |

说明：不再提供 `--debug` 别名，统一使用 `--config Debug|Release`；一次调用只构建
一个配置，需要两种配置时调用两次（见 `Root-CMake-Build-Plan.md` 10.4 第 14 条）。
**Python 3 是硬性要求**（Skia 的构建流程依赖 `python3`），没有 Python 3 时
`build.bat` / `build.py` 直接报错并给出安装提示；不提供绕过 Python 的脚本。
SDK 只由 `pack.py` 产出，根工程**不新增 `install()` 目标**。

### 5.2 自动识别

- 平台：`Windows` / `Linux` / `macOS` / `FreeBSD` / `MSYS2/ MinGW`；
- 架构：Windows `x86` / `x64` / `arm64`，其他平台 `x64` / `arm64`；交叉编译可用
  `--arch` 显式指定（Windows + Ninja 编译 arm64 时必须指定）；
- 编译器：MSVC（自动检测 VS 版本）、MinGW gcc/clang、Unix gcc/clang；
- 依赖目录：仓库同级的 `skia/`、`SDL3/`、`cef_binary/`（可 `--deps-root` 覆盖）；
- 依赖版本：`--cef-version 109|142`、`--skia-ref`、`--sdl-ref`，默认取
  `cmake/third_party_versions.json` 中对应段的 `default`；
- Windows 下 SDL 默认关闭（可选），Linux/macOS/FreeBSD 强制启用；显式
  `--sdl` / `DUILIB_ENABLE_SDL=ON` 时，Windows 的 SDL 也由 `deps` 自动构建
  （Debug+Release，无需手工准备 `../SDL3`）；
- 参数透传：`--cmake-arg`、`--build-arg`，避免为新需求不断加参数；
- 构建目录：默认 `out/<arch>/<Config>`（如 `out/x64/Release`），与 `build/` 平级，
  一次只构建一个架构+配置；`--build-dir <dir>` 可覆盖（允许放到仓库外）；
  `build/build_temp` 仅属于 `build/*.sln` 的既有 VS 工程，新入口不写入、不清理；
  可执行文件/运行时始终输出到仓库根 `bin/`。

### 5.3 输出与错误

- 每个阶段打印“正在做什么、预计下一步”；
- **交互语言**：默认跟随系统（简体中文 → 中文，否则英文），`--lang en|zh_CN` 可覆盖；
  规范见 `Root-CMake-Build-Plan.md` 第 1.2 节；
- 失败时给出**单一可执行建议**（格式 `[ERROR]` + `Hint:`，见 `Root-CMake-Build-Plan.md` 1.2），例如：
  - `[ERROR] gn/ninja not found` + `Hint: install them, or run 'build.py deps --skia-use-bundled'`
  - `[ERROR] Visual Studio not found` + `Hint: install VS 2022 with the C++ workload`
  - `[ERROR] failed to download CEF` + `Hint: use '--cef-archive <local file>'`

## 6. 旧脚本移除

1. **前置条件**：`build.py` 在全部目标平台完成一轮真实验证（依赖准备、构建、示例运行），确认无功能缺口；
2. **一次性删除** 11 个旧构建脚本：`build_duilib_all_in_one.sh/.bat`、`build_duilib_all_in_one_mingw-w64.bat`、`linux_build.sh`、`macos_build.sh`、`freebsd_build.sh`、`msys2_build.sh`、`msvc_build.bat`、`gcc-mingw-w64_build.bat`、`llvm-mingw-w64_build.bat`、`detect_vs_version.bat`；
3. **能力吸收**：`detect_vs_version.bat` 的 VS 版本检测逻辑内联进 `build.py`（`--vs-version` 可显式覆盖）；各旧脚本中的平台默认值（如 MSYS2 静态 SDL）固化为 `build.py` 的默认行为或显式参数；`autotest.py` / `release.py` 若原本一次构建 Release+Debug，改为分别调用 `build.py --config Debug` 与 `build.py --config Release` 两次；
4. **引用清理**：删除前全仓库搜索旧脚本名，更新 `README`、`README.en.md`、
   `build-reference.md`、`build-reference.en.md`、`autotest.py` 预设、`.sln` 说明
   及 CI 配置中的调用；
5. **记录变更**：在 CHANGELOG 写一条“构建入口统一为 `python build/build.py`”，不再提供旧命令。

## 7. 文档重构

### 7.1 README 第一屏（目标 ≤ 40 行）

```
# 编译

环境要求：CMake 3.24+、Python 3.8+、Git；Windows 另需 Visual Studio 2022+（或 Ninja + clang）。

## 快速开始
python build/build.py                 # 首次自动准备 Skia/SDL/CEF，构建 Release 库与示例
python build/build.py --config Debug  # Debug（与 Release 使用不同构建目录）
python build/build.py doctor          # 环境自检
python build/build.py pack            # 出包

## 我该用哪个方式？
| 场景 | 做法 |
| 只想跑起来 | python build/build.py |
| 用 Visual Studio | 打开 build/examples.sln（需先准备依赖：build.py deps） |
| 只编库 | python build/build.py build --target duilib |
| 出发布包 | python build/build.py pack --with-cef |
| 完整参数 | 见 build-reference.md / build-reference.en.md |
```

### 7.2 参考手册

- 同时维护**中文** `build-reference.md` 与**英文** `build-reference.en.md`，
  命名与 `docs/` 下其他 `*.md` / `*.en.md` 规则一致；两份内容必须对应，
  修改中文版时同步英文版；
- 合并现根 `README.md` 的平台矩阵、变量表、故障排查；`build/build.md` 的旧脚本
  对照表不再迁移（脚本已删除），其中仍有效的产物/运行说明保留在 `build/README.md`；
- 删除 `build/build.md` 时同步更新引用：根 `README.md` / `README.en.md`、
  `WORKBUDDY.md`、`TRAE.md`、`CLAUDE.md`、`docs/Release*.md` 等；
- `build/README.md` / `.en.md` 保留，定位为“各平台前置依赖 + 发布打包 + 排障索引”，
  删除旧脚本表并指向 `build-reference.md`；
- 明确 SDK 产物只由 `pack.py` 生成（`include/`、`lib/<arch>/<Config>`、`bin/`、
  `lib/cmake/nim_duilib/`、`LICENSES/`），根工程不提供 `install()` 目标；
- 只保留**一张**“平台 × 编译器 × 依赖 × 可选模块”矩阵；
- 删除已由 `build.py --help` 覆盖的参数说明，文档只写“为什么/什么时候用”。

## 8. 实施阶段

| 阶段 | 内容 | 前置 |
| :--- | :--- | :--- |
| S0 | 文档收口：README 一屏 + `build-reference.md`/`.en.md`；删除 `build.md` 并更新引用；`build/README.md` 改为前置依赖与打包索引；增加决策表 | 无 |
| S1 | 根 CMake P0 落地（`off` 模式），`build.py build/deps/doctor` | Root-CMake-Build-Plan P0 |
| S2 | 依赖自动化（`prepare`/`auto`）+ 默认命令一键跑通 | S1 |
| S3 | 全平台真实验证后一次性删除旧脚本；`pack/test` 子命令；全新引用清理 | S2 |
| S4 | 收尾：CHANGELOG、清理无用的本地调试脚本与日志 | S3 |

## 9. 验收标准

1. 全新机器严格按 README 第一屏操作可完成：依赖准备 → 库 → 示例运行；
2. 构建入口从 11 个降为 1 个（另保留 4 个 `.sln`），旧构建脚本已删除且无残留引用；
3. `build.py doctor` 能准确指出缺失项，且不产生副作用；
4. 旧脚本中的平台默认行为（如 MSYS2 静态 SDL、VS 版本检测）已在新入口中保留；
   Windows 的 SDL 在显式启用时可由 `deps` 自动构建（Debug+Release）；
5. `build-reference.md` 与 `build-reference.en.md` 均存在且内容对应；
6. `--config Debug|Release` 一次只构建一个配置（默认 Release），构建树位于
   `out/<arch>/<Config>`；两次调用分别产出 Debug/Release；`build` 默认自动确保依赖、
   `--no-deps` 可跳过；
7. `pack.py`、`release.py`、`autotest.py` 行为不变（需要两种配置时分两次调用），
   测试全绿；
8. `python -m unittest build/test_build.py` 通过（标准库 unittest，无 pytest 依赖）；
9. `pack.py` 产物包含 `LICENSES/` 与 `THIRD-PARTY-NOTICES.md`；
10. `build/build.md` 已删除，全仓库无指向它的链接；根 README 首屏只出现
    `python build/build.py` 与 `python build/build.py --config Debug`，不出现
    `--debug` 或 `build` 子命令形式。

## 10. 风险

| 风险 | 缓解 |
| :--- | :--- |
| 新手机器无 Python | Python 3 是硬性要求（Skia 构建依赖）；`build.bat` 依次尝试 `py -3` / `python3`，缺失时报错并给出安装提示；不提供仅调 CMake 的备用脚本 |
| 旧文档/个人脚本仍引用旧路径 | 删除前全仓库搜索旧脚本名并更新；发布说明给出新命令；不保留转发层 |
| 入口与 CMake 参数重复 | 所有参数尽量透传，入口不实现构建逻辑 |
| 旧脚本的隐式默认值丢失 | 删除前逐个比对默认参数，差异点写入 `build.py` 默认值或 `build-reference.md` |

入口侧的其余决策（`--config` 语义、`doctor` 自举、Presets 与入口的唯一事实源、
Python 3 硬性依赖、macOS 两阶段自动化、SDK 仅由 `pack.py` 产出、构建树
`out/<arch>/<Config>` 与 `.gitignore`）统一见
`docs/misc/Root-CMake-Build-Plan.md` 第 10 节；所有决策项已确认，可进入实施。
