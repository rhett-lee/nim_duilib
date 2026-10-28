# arm64 测试计划（Windows）

目标：给出一份可直接执行、结果可直接回填 `build/test-matrix.md` 的 arm64 验证清单。

**范围决策（2026-10-28）**：本次 arm64 支持只验证 Windows；macOS / Linux / FreeBSD 均无
真机环境，**不安排测试**，相应矩阵项标记为「不安排」而不是「未测」。

## 0. 背景与现状

| 项目 | 现状 | 影响 |
| :--- | :--- | :--- |
| 脚本支持 | `build.py --arch auto\|x86\|x64\|arm64`；库目录 `lib/<arch>/<Config>` | 构建入口已就绪 |
| Windows 工具链 | VS 2022 17.14.37710.0 已装 MSVC 14.44.35207「ARM64 生成工具」；`Hostx64\arm64\cl.exe` 与 Windows SDK 10.0.26100.0 的 `um/arm64`、`ucrt/arm64` 齐全；`python build/build.py doctor --arch arm64` 通过（Ninja + VS + gn + curl 全 OK） | Windows arm64 可交叉编译 |
| vcvars 平台参数 | 原先对 `vcvarsall.bat` 传 `arm64`，要求 x64 主机也装 `Hostarm64` 原生工具集，导致误报 `Visual Studio C++ tools for arm64 are not installed`；已改为按主机选择 `amd64_arm64` / `x86_arm64` / `arm64`（2026-10-28 修复） | 交叉编译不再需要额外组件 |
| 打包支持 | `pack.py --arch arm64`；库目录映射 `lib/arm64/...`；PE 架构校验（`0xAA64`）；随包三份 manifest（含 `duilib.arm64.manifest`）；`NIM_DUILIB_PACKAGE_ARCH=arm64` | 已生成 Release 无 CEF、Release CEF142 与 Debug CEF142 三个包并通过校验 |
| 无真机校验 | `autotest.py --preset arm64-synthetic` 用临时假库出包并体检 | 不依赖 arm64 机器即可回归包结构 |
| Windows + CEF | 上游提供 Windows arm64 预编译包（142 与 109）；`third_party_versions.json` 两个版本都登记了 `windows/arm64`；CMake 按 `lib/arm64` 取 `libcef.lib`，`pack.py` 按 `bin/libcef_win[_109]/arm64` 收集运行时 | CEF 构建、出包与包测试都纳入 Windows 矩阵 |
| Skia | `deps` 已按 GN `target_cpu="arm64"` + clang 构建出 `skia/out/llvm.arm64.release` 与 `llvm.arm64.debug`（各 1078 个目标，约 6 分钟）；DLL 模式的 `.md`/`.mdd` 尚未构建 | Release/Debug 构建已可用；DLL 模式首次仍需先构建对应 Skia 目录 |
| SDL3 | Windows 默认 `DUILIB_ENABLE_SDL=OFF`；只有 `--sdl` 才需要 arm64 SDL3 | 默认组合不依赖 SDL3 |
| 现有矩阵 | W-B18/W-B18b（arm64 Release/Debug 构建）、W-P23/W-P24（arm64 出包）已回填为「通过」；Debug 出包、CEF109、示例启动仍「未测」；L-B04、M-B03 因无真机改为「不安排」 | 见文末「本轮执行结果」 |

## 1. 范围

**做**：Windows x64 主机交叉编译 arm64（Release + Debug）、DLL 模式、出包
（含 `--with-examples`）、`pack.py --verify`、`sdk_package_test.py`（编译/结构场景）、
CEF 142 与 109 的构建与出包；有 Windows arm64 运行环境时再补示例启动。

**不做**：macOS / Linux / FreeBSD 的 arm64（无真机，不安排测试）；FreeBSD arm64
（上游也不在本项目支持范围）；x86/arm64 混包；性能基准；签名/公证。

## 2. 前置条件

| 项目 | 要求 | 校验命令 |
| :--- | :--- | :--- |
| VS 工具链 | VS 2022 + 「MSVC v143 - VS 2022 C++ ARM64/ARM64EC 生成工具」+ Windows SDK arm64 库 | `python3 build/build.py doctor --arch arm64`（应显示 `generator Ninja`、`visual studio ...`，无 arm64 告警） |
| Skia | `skia/out/llvm.arm64.release`（Debug 用 `llvm.arm64.debug`；DLL 模式 `.md`/`.mdd`）；缺失时由 `deps` 按 GN `target_cpu="arm64"` 自动构建 | `ls skia/out/llvm.arm64.*` |
| CEF | `deps` 按 `windows/arm64` 下载并解压到 `<deps>/cef_binary/extract/{142,109}/arm64/`，再复制到 `bin/libcef_win[_109]/arm64/` | `dumpbin /headers bin\libcef_win\arm64\libcef.dll` 看 `AA64` |
| 架构切换 | 必须先清理 `bin/` 与 `lib/<arch>/<Config>`，否则打包会因混合架构失败（预期保护）；切 CEF 版本时清理对应 `bin/libcef_win[_109]` 目录 | `git status`、`dumpbin` 抽查 |

通用：

- 构建时显式传 `--arch arm64`；`--arch auto` 只跟随主机（x64 主机会得到 x64）。
- 磁盘充足，包测试用 `--out-dir ./temp`（见 `sdk-package-test.md` 排错表）。
- 仅交叉编译时，产物无法在 x64 主机运行；启动验证（R-01/R-04、W-A09/W-A10）需要
  Windows on ARM 设备或 arm64 虚拟机，没有则把这几项标记为「待真机」，不要当作通过。

## 3. Phase 0：不需 arm64 环境的前置回归（0.5 小时）

```bash
python3 build/build.py test
python3 build/test_build.py
python3 build/test_pack.py
python3 build/autotest.py --preset arm64-synthetic
```

期望：

- 单元测试全部通过（当前基线：227 项通过、2 项按平台跳过；其中新增 4 项
  `_vcvars_platform` 映射用例与 5 项 `_can_run_arch` 用例）。
- `arm64-synthetic` 生成一个 arm64 包且 `inspect_package` 校验通过：包名含 `-arm64`、
  `lib/arm64/<Config>/`、三份 manifest 齐全且 `duilib.arm64.manifest` 为 `ARM64`、
  `nim_duilibConfig.cmake` 含 `CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64"`。

Phase 0 不通过则先修脚本，不要继续。

## 4. Windows arm64 测试矩阵（W-A）

| 编号 | 内容 | 命令 | 通过标准 |
| :--- | :--- | :--- | :--- |
| W-A01 | Release 库 + 示例构建 | `python3 build/build.py --arch arm64` | 构建成功；`bin\<目标>.exe` 的 PE machine 为 `AA64`；`lib/arm64/Release` 生成 |
| W-A02 | Debug 库 + 示例构建 | `python3 build/build.py --arch arm64 --config Debug` | 同上，库在 `lib/arm64/Debug` |
| W-A03 | DLL 模式 | `python3 build/build.py --arch arm64 --dll` | 产出 `duilib.dll` / `duilib_d.dll`，架构为 AA64（`.md`/`.mdd` Skia 存在） |
| W-A04 | Release 出包 | `python3 build/pack.py --arch arm64 [--with-examples]` | 生成 `nim_duilib-1.0.2026.1023-win-arm64[-examples].zip`；`pack.py --verify` 通过 |
| W-A05 | Debug 出包 | `python3 build/pack.py --arch arm64 --config Debug --with-examples` | 包名含 `-debug`；`--verify` 通过 |
| W-A06 | CEF 142 构建 + 出包 | `python3 build/build.py --arch arm64`（默认 CEF 142）；`python3 build/pack.py --arch arm64 --with-examples --with-cef` | 包名含 `-cef`；包内 `bin/libcef_win/arm64/libcef.dll` 与 `lib/libcef.lib` 齐全；PE 架构 AA64；`--verify` 通过 |
| W-A07 | CEF 109 构建 + 出包 | 同 W-A06，换成 `--cef-version 109` | 包名含 `-cef109`；`bin/libcef_win_109/arm64/libcef.dll` 为 AA64；`--verify` 通过 |
| W-A08 | 包测试（结构 + 编译校验） | `python3 build/sdk_package_test.py --package dist/<W-A04 产物> --out-dir ./temp` | P01–P08 通过；C01 用 VS ARM64 工具链编译运行（x64 主机可编译、不可运行，C10 会 SKIP）；C04 用 `-A x64` 构造相反架构并被拒绝 |
| W-A09 | 启动示例（需 Windows arm64 环境） | 同上加 `--run-examples` | `[PASS] C10 examples.smoke`；无环境时记录为「待真机」 |
| W-A10 | CEF 示例启动（需 Windows arm64 环境） | 解包 W-A06 / W-A07 产物后运行 `bin\cef.exe`、`bin\CefBrowser.exe` | 窗口正常、页面加载，进程不崩溃（运行时在 `bin/libcef_win[_109]/arm64/`） |

W-A08 的 C01 若因工具链缺失 SKIP，说明 arm64 工具集未装好，回到 §2 检查；不要当作通过。

## 5. 运行与功能验证

| 编号 | 检查 | 方式 | 通过标准 |
| :--- | :--- | :--- | :--- |
| R-01 | 示例启动 | `sdk_package_test.py --run-examples`（C10）或直接运行 `bin\<示例>.exe` | Windows arm64 上进程启动 3 秒后仍在运行 |
| R-02 | 资源加载 | 打开 `basic` 主界面 | 主题、字体、语言资源正常，无缺图/乱码 |
| R-03 | 资源热更新 | 修改 `bin/resources/themes/default/basic/*.xml` 后重建（Windows 直接读 `bin/resources`） | 新样式生效，无需删产物 |
| R-04 | CEF 示例 | 打开 `cef` / `CefBrowser` | 页面加载正常，退出不崩溃 |

## 6. 依赖与打包专项检查

| 检查项 | 命令/方法 | 失败含义 |
| :--- | :--- | :--- |
| PE 架构 | `dumpbin /headers bin\<目标>.exe`、`dumpbin /headers lib\arm64\Release\duilib.lib` | 混入 x64 产物，需清理 `bin/` 与 `lib/` 重来 |
| Skia 目录 | `ls skia/out/llvm.arm64.release`（Debug 用 `.debug`；DLL 模式 `.md`/`.mdd`） | 缺 arm64 Skia，链接报找不到库 |
| CEF 运行时架构 | `dumpbin /headers bin\libcef_win[_109]\arm64\libcef.dll` | 混入 x64 CEF，示例启动即失败 |
| 包内架构一致性 | `pack.py --verify`、`sdk_package_test.py` 的 P04/P06 | 包名架构与库架构不一致 |
| arm64 manifest | 包内 `msvc/manifest/duilib.arm64.manifest` 为 `processorArchitecture="ARM64"` | 下游接入选错 manifest |
| 接入层 | 解包后按 `msvc/manifest/` 选 arm64 manifest 编译一个最小工程 | manifest 与库架构错配 |

## 7. 预期问题与处置

| 现象 | 原因 | 处置 |
| :--- | :--- | :--- |
| `Visual Studio C++ tools for arm64 are not installed` | VS 未装 ARM64 生成工具；或 `vcvarsall` 参数与主机不匹配（早期版本固定传 `arm64`，要求 Hostarm64 原生工具集） | 确认 `build.py` 已含 `_vcvars_platform`（x64 主机 → `amd64_arm64`）；VS Installer 勾选「MSVC v143 ARM64/ARM64EC 生成工具」后重开终端 |
| 链接报找不到 Skia 库 | `skia/out/llvm.arm64.release` 不存在 | 用与 x64 相同的 GN 参数构建 arm64 Skia，或 `--skia-lib-dir` 显式指定 |
| `pack.py --arch arm64 --with-cef` 告警「CEF 运行时目录不存在：bin/libcef_win/arm64」 | `deps` 之前按 x64 取包，`bin/libcef_win/arm64/` 里只有导入库或为空 | 删除 `bin/libcef_win/arm64` 后 `python3 build/build.py deps --arch arm64 --force` 重下 `windows/arm64` 归档 |
| CEF 示例闪退/加载 `libcef.dll` 失败 | `bin/libcef_win[_109]/arm64/libcef.dll` 仍是 x64 的（手工拷贝或旧缓存） | `dumpbin /headers ...\libcef.dll` 确认 AA64，必要时删除后重下 |
| 链接/打包报架构不匹配 | `bin/`、`lib/` 混有另一架构或另一配置 | 按 §2 清理后重跑 |
| x64 主机上示例无法启动 | 产物是 ARM64，x64 Windows 不能直接执行 | 换 Windows on ARM 设备/虚拟机，或将 R-01/R-04 标为「待真机」 |

## 8. 验收标准

1. Phase 0 全部通过（脚本层不回归）。
2. Windows arm64 Release 与 Debug 构建成功，产物架构经 `dumpbin` 确认为 `AA64`。
3. 至少一个 arm64 包 `pack.py --verify` 通过；`sdk_package_test.py` 中可执行的场景全 PASS
   （SKIP 需注明原因；C01 不得因缺工具链而 SKIP）；带 `--with-cef` 的包（W-A06/W-A07）
   包含 AA64 的 CEF 运行时。
4. 有 Windows arm64 运行环境时，示例启动（C10）与 CEF 示例各通过一次；没有则明确记录
   「待真机」，不得记为通过。
5. 每个结论附可复现命令与关键输出（`report.json` 的 PASS/FAIL/SKIP 数、`dumpbin` 片段）。

## 9. 结果回填

| 位置 | 回填内容 |
| :--- | :--- |
| `build/test-matrix.md` | W-B18、W-P23、W-P24 按结果更新（通过 / 部分 / 待真机）；§0 基线补 arm64 工具链；L-B04、M-B03 改为「不安排（无真机）」；§7 进展表与已知缺口同步 |
| `build/README.md` / `README.en.md` | arm64 支持范围注明「仅 Windows 实测，其他平台无真机」 |
| `CHANGELOG.md` / `.en.md` | 工具链探测、Skia 路径等修复按「修复」条目记录 |
| `build/test-matrix.md` §7 | 本轮新发现的问题按「本轮发现与修复」表格记录 |

## 10. 执行顺序与耗时估算

```text
Phase 0（任意 x64 机器）                 0.5 h
构建 arm64 Skia（llvm.arm64.release/debug）约 6 min/变体（实测 8 核）
Windows arm64 构建 + 出包 + 包测试        1–2 h
示例启动（需 Windows arm64 环境）          0.5 h
结果回填                                 0.5 h
```

顺序：Phase 0 → 构建 arm64 Skia → Release 构建 → Debug 构建 → 出包 →
`pack.py --verify` → `sdk_package_test.py`（先不带 `--run-examples`，再带）→
运行验证（有环境时）→ 回填。

## 11. 本轮执行结果（2026-10-28，x64 主机交叉编译）

| 编号 | 结果 | 关键数据 |
| :--- | :--- | :--- |
| Phase 0 | 通过 | `python -m unittest build.test_build build.test_pack build.test_sdk_package build.test_autotest`：227 项通过、2 项按平台跳过 |
| W-A01 | 通过 | `build.py --arch arm64`：25/25 目标、21 个示例链接完成；`bin/*.exe` 23 个全部 PE `0xAA64`；`lib/arm64/Release` 6 个库 237 MB |
| W-A02 | 通过 | `deps` 自动构建 `skia/out/llvm.arm64.debug`（1078 目标，约 6 分钟）；802/802 目标、23 个示例链接完成；`bin/*.exe` 均为 `0xAA64`，清单为 `processorArchitecture="ARM64"`；`lib/arm64/Debug` 6 个库 409.4 MB |
| W-A03 | 未测 | DLL 模式需 `.md`/`.mdd` 的 arm64 Skia |
| W-A04 | 通过 | `nim_duilib-1.0.2026.1023-win-arm64-examples.zip`：760 条目 / 109.0 MB，`--verify` 760 条目 SHA 全匹配 |
| W-A05 | 通过 | `...-win-arm64-cef-examples-debug.zip`：1314 条目 / 394.5 MB，`--verify` 全匹配；SDK 测试 PASS=14 FAIL=0 SKIP=5。WebView2 无 arm64 loader，按预期跳过 |
| W-A06 | 通过 | `...-win-arm64-cef-examples.zip`：1314 条目 / 311.7 MB；CEF 开发文件 288 项 + 运行时 238 项；包内 `libcef.dll` 为 `0xAA64`；`--verify` 1314 条目全匹配 |
| W-A07 | 未测 | CEF109 arm64 未构建/出包（清单条目已就绪） |
| W-A08 | 通过（C01 按架构 SKIP） | 三个包均 `PASS=14 FAIL=0 SKIP=5`；C01 已完成配置/编译/链接，因 x64 主机不能执行 ARM64 程序而 SKIP；C02 由 C01 覆盖，C06 缺 g++，C10 未开 `--run-examples`，C11 仅 macOS |
| W-A09 / W-A10 / R-01 ~ R-04 | 待真机 | 需 Windows on ARM 设备；x64 主机运行 ARM64 程序报 `WinError 216` |

本轮修复（详见 `build/test-matrix.md` §7）：Skia ARM64 无 WGL（`Render_Skia_Windows`
固定 CPU 绘制）、arm64 manifest 选错、`nim_duilibConfig.cmake` 交叉架构识别、
`_can_run_arch` 使 C01 按架构 SKIP。
