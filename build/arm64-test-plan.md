# arm64 测试计划

目标：在没有 arm64 测试环境的现状下，给出一份可直接执行、结果可直接回填
`build/test-matrix.md` 的 arm64 验证清单。适用 Windows / Linux / macOS 的 x64 → arm64
（以及 Apple Silicon 原生）；FreeBSD arm64 不在当前支持范围。

## 0. 背景与现状

| 项目 | 现状 | 影响 |
| :--- | :--- | :--- |
| 脚本支持 | `build.py --arch auto\|x86\|x64\|arm64`；库目录 `lib/<arch>/<Config>`；Windows 用 `vcvarsall` 的 ARM64 工具集，缺失时直接报 `Visual Studio C++ tools for arm64 are not installed` | 代码与构建入口已就绪，缺的是真机验证 |
| 打包支持 | `pack.py --arch arm64`；库目录映射 `lib/arm64/...`；PE 架构校验（`0xAA64`）；随包三份 manifest（含 `duilib.arm64.manifest`）；`NIM_DUILIB_PACKAGE_ARCH=arm64` | 包结构与接入文件已就绪，尚未生成过真实 arm64 包 |
| 无真机校验 | `autotest.py --preset arm64-synthetic` 用临时假库出包并体检 | 不依赖 arm64 机器即可回归包结构 |
| Windows 工具链 | 本轮 VS 2022 未安装 ARM64 生成工具（W-B18 未测） | Windows arm64 构建的前置条件 |
| Windows + CEF | 上游提供 Windows arm64 预编译包（142 与 109）；`third_party_versions.json` 两个版本都登记了 `windows/arm64` 归档，CMake 按 `lib/arm64` 取 `libcef.lib`，`pack.py` 按 `bin/libcef_win[_109]/arm64` 收集运行时 | Windows arm64 的 CEF 构建、出包与包测试都纳入矩阵 |
| Linux / macOS + CEF | CEF 提供 `linuxarm64`、`macosarm64` 分发包（见 `docs/CEF.md`） | Linux/macOS arm64 可覆盖 CEF 示例 |
| Skia | 预编译库目录按架构命名：`llvm.arm64.release` / `llvm.arm64.debug`（DLL 模式 `.md` / `.mdd`），目录缺失时链接报错 | 需确认或自行构建 arm64 Skia |
| libjpeg-turbo | 提供 arm64 预编译库（Windows 包按架构收集） | 无需额外处理 |
| SDL3 | 由 `deps` 按目标平台构建；macOS 为动态库，需随 Bundle 分发 | arm64 上需重新构建（不能复用 x64 产物） |
| 现有矩阵 | W-B18、L-B04、M-B03 均为「未测（无 arm64 测试环境）」 | 本计划完成后可回填为「通过」 |

## 1. 范围

**做**：Windows / Linux / macOS 的 arm64 Release + Debug 构建、出包、包测试（含
`--run-examples`）、示例启动；macOS 额外覆盖 App Bundle（C11）；三平台的 CEF 示例
（Windows 142 与 109、Linux `linuxarm64`、macOS `macosarm64`）。

**不做**：FreeBSD arm64；x86/arm64 混包；性能基准；签名/公证流程（分发另议）。

## 2. 前置条件

| 平台 | 环境 | 依赖准备 | 校验命令 |
| :--- | :--- | :--- | :--- |
| Windows | Windows 11 arm64 真机，或 x64 主机 + VS 2022「MSVC v143 ARM64 生成工具」交叉编译 | Skia `skia/out/llvm.arm64.release`（Debug 用 `llvm.arm64.debug`；DLL 模式 `.md`/`.mdd`）；arm64 版 SDL3 由 `deps` 构建；CEF 由 `deps` 按 `windows/arm64` 下载（142 / 109 均有预编译包） | `python3 build/build.py doctor`；`dumpbin /headers bin\<目标>.exe` 看 `machine (AA64)` |
| Linux | arm64 真机或 arm64 虚拟机（推荐原生，脚本未提供 Linux 交叉工具链参数） | arm64 Skia `.a`；SDL3 在 arm64 上重新 `deps` | `python3 build/build.py doctor`；`file bin/<目标>`、`readelf -h` |
| macOS | Apple Silicon（M 系列）；不建议 Rosetta 混跑 | CEF 换 `macosarm64` 分发包；Skia `llvm.arm64.*`；SDL3 dylib 需 arm64 | `python3 build/build.py doctor`；`file bin/<目标>`、`lipo -archs` |

通用：

- 磁盘 ≥ 8 GiB，包测试使用 `--out-dir ./temp`（避免 `/tmp` 空间不足，见
  `sdk-package-test.md` 排错表）。
- 切换架构前清理 `bin/` 与 `lib/<arch>/<Config>`，否则打包会因混合架构失败（预期保护）。
- CEF 运行期目录也要按架构清理：Linux 的 `bin/libcef_linux/` 与 macOS 的
  `<deps>/cef_binary/` 不区分架构，从 x64 换到 arm64 时先删除，再
  `python3 build/build.py deps --force` 重新按 `linux/arm64`、`macos/arm64` 取包；
  Windows 的 `bin/libcef_win/<arch>/`（含 `arm64`）本身按架构分目录，无需删除；
  但 109 与 142 的运行时也分目录（`libcef_win_109` / `libcef_win`），切换版本时
  注意用 `--cef-version` 指定正确的一套。
- Windows arm64 交叉构建时必须显式 `--arch arm64`（`--arch auto` 只跟随主机）。

## 3. Phase 0：无 arm64 真机也能做的回归（0.5 小时）

在任何 x64 机器上先跑：

```bash
python3 build/build.py test
python3 build/test_build.py
python3 build/test_pack.py
python3 build/autotest.py --preset arm64-synthetic
```

期望：

- 单元测试全部通过（当前基线：218 项通过、2 项按平台跳过）。
- `arm64-synthetic` 生成一个 arm64 包，`inspect_package` 校验通过：包名含 `-arm64`、
  `lib/arm64/<Config>/`、三份 manifest 齐全且 `duilib.arm64.manifest` 为 `ARM64`、
  `nim_duilibConfig.cmake` 含 `CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64"`。

Phase 0 不通过则先修脚本，不要继续后面的真机步骤。

## 4. 测试矩阵

### 4.1 Windows arm64（W-A）

| 编号 | 内容 | 命令 | 通过标准 |
| :--- | :--- | :--- | :--- |
| W-A01 | Release 库 + 示例构建 | `python3 build/build.py --arch arm64` | 构建成功；`bin\<目标>.exe` 的 PE machine 为 `AA64`；`lib/arm64/Release` 生成 |
| W-A02 | Debug 库 + 示例构建 | `python3 build/build.py --arch arm64 --config Debug` | 同上，库在 `lib/arm64/Debug` |
| W-A03 | DLL 模式 | `python3 build/build.py --arch arm64 --dll` | 产出 `duilib.dll` / `duilib_d.dll`，架构为 AA64 |
| W-A04 | Release 出包 | `python3 build/pack.py --arch arm64`（可选 `--with-examples`） | 生成 `nim_duilib-1.0.2026.1023-win-arm64[-examples].zip`；`--verify` 通过 |
| W-A05 | Debug 出包 | `python3 build/pack.py --arch arm64 --config Debug --with-examples` | 包名含 `-debug`；`--verify` 通过 |
| W-A06 | CEF 142 构建 + 出包 | `python3 build/build.py --arch arm64 --with-cef`；`python3 build/pack.py --arch arm64 --with-examples --with-cef` | 包名含 `-cef`；包内 `bin/libcef_win/arm64/libcef.dll` 与 `lib/libcef.lib` 齐全；PE 架构为 AA64；`--verify` 通过 |
| W-A07 | CEF 109 构建 + 出包 | 同 W-A06，命令加 `--cef-version 109` | 包名含 `-cef109`；`bin/libcef_win_109/arm64/libcef.dll` 为 AA64；`--verify` 通过 |
| W-A08 | 包测试 | `python3 build/sdk_package_test.py --package dist/<W-A04 产物> --out-dir ./temp` | P01–P08 通过；C01 用 arm64 工具链编译运行；DLL 包另验 C02；C04 用 `-A x64` 构造相反架构并被拒绝 |
| W-A09 | 启动示例 | `... --package <W-A04 产物> --run-examples` | `[PASS] C10 examples.smoke` |
| W-A10 | CEF 示例启动 | 解包 W-A06 / W-A07 产物后运行 `bin\cef.exe`、`bin\CefBrowser.exe` | 窗口正常、页面加载，进程不崩溃（运行时在 `bin/libcef_win[_109]/arm64/`） |

如果 W-A08 的 C01 因工具链缺失 SKIP，说明 arm64 工具集未装好，回到 §2 检查；不要把它
当作通过。

### 4.2 Linux arm64（L-A）

| 编号 | 内容 | 命令 | 通过标准 |
| :--- | :--- | :--- | :--- |
| L-A01 | Release 构建 | `python3 build/build.py --arch arm64` | 构建成功；`file bin/<目标>` 显示 `ARM aarch64` |
| L-A02 | Debug 构建 | `python3 build/build.py --arch arm64 --config Debug` | 同上 |
| L-A03 | Release 出包 | `python3 build/pack.py --arch arm64 --with-examples` | `nim_duilib-1.0.2026.1023-linux-arm64-examples.tar.gz`；`--verify` 通过 |
| L-A04 | Release CEF 出包 | 先 `python3 build/build.py deps --arch arm64 --force`（按 `linux/arm64` 取 `linuxarm64` 归档，离线时用 `--cef-archive` 指定），再 `python3 build/pack.py --arch arm64 --with-examples --with-cef` | `bin/libcef_linux/libcef.so` 为 aarch64；包内含 CEF 资源；`--verify` 通过 |
| L-A05 | Debug 出包 | `python3 build/pack.py --arch arm64 --config Debug --with-examples [--with-cef]` | 包名含 `-debug`；`--verify` 通过 |
| L-A06 | 包测试 | `python3 build/sdk_package_test.py --package dist/<产物> --out-dir ./temp` | P01–P08、C01、C03、C07–C09 通过；C02/C05/C06/C11 按平台 SKIP；C04 因主机与包架构一致 SKIP（均属预期） |
| L-A07 | 启动示例 | 同上加 `--run-examples` | `[PASS] C10 examples.smoke`（含 CEF 包时同时确认 `cef` 示例能起窗口） |

### 4.3 macOS arm64（M-A）

| 编号 | 内容 | 命令 | 通过标准 |
| :--- | :--- | :--- | :--- |
| M-A01 | Release 构建 | `python3 build/build.py --arch arm64` | 构建成功；`file bin/basic.app/Contents/MacOS/basic` 为 `arm64`；`lib/arm64/Release` 生成 |
| M-A02 | Debug 构建 | `python3 build/build.py --arch arm64 --config Debug` | 同上；切换到 Debug 后首次启动不崩溃（`Contents/Info.plist` 保留） |
| M-A03 | Bundle 结构 | `plutil -p bin/basic.app/Contents/Info.plist`；`lipo -archs bin/basic.app/Contents/Frameworks/libSDL3*.dylib` | plist 存在且 `CFBundleIdentifier` 有值；dylib 为 arm64 |
| M-A04 | 资源增量同步 | 修改 `bin/resources/themes/default/basic/*.xml` 后 `python3 build/build.py --arch arm64` | 日志出现 `Syncing duilib resources into basic.app`；启动即看到新样式（等价 M-B07） |
| M-A05 | Release 出包 | 先删除 `<deps>/cef_binary` 并 `python3 build/build.py deps --arch arm64 --force`（取 `macos/arm64`），再 `python3 build/pack.py --arch arm64 --with-examples --with-cef` | `nim_duilib-1.0.2026.1023-macos-arm64-cef-examples.tar.gz`；`--verify` 通过；Bundle 内 CEF Framework 为 arm64 |
| M-A06 | Debug 出包 | `python3 build/pack.py --arch arm64 --config Debug --with-examples` | 包名含 `-debug`；`--verify` 通过 |
| M-A07 | 包测试 | `python3 build/sdk_package_test.py --package dist/<产物> --out-dir ./temp` | P01–P08、C01、C03、C09、C11 通过（C11 校验 Frameworks/rpath/资源与迁移后运行）；C02/C05/C06 SKIP、C04 因主机与包架构一致 SKIP（均属预期） |
| M-A08 | 启动示例 | 同上加 `--run-examples` | `[PASS] C10 examples.smoke` |
| M-A09 | CEF 示例 | `open bin/CefBrowser.app`（或 `.../cef.app`） | 窗口正常、无「CEF 分发包与目标架构不一致」闪退 |

macOS arm64 说明：链接器会自动为 arm64 可执行文件生成 ad-hoc 签名，直接运行不需要额外
签名；对外分发需开发者签名与公证，属本计划范围外。

## 5. 运行与功能验证（三平台共用）

| 编号 | 检查 | 方式 | 通过标准 |
| :--- | :--- | :--- | :--- |
| R-01 | 示例启动 | `sdk_package_test.py --run-examples`（C10） | 进程启动 3 秒后仍在运行 |
| R-02 | 资源加载 | 打开 `basic` 主界面 | 主题、字体、语言资源正常，无缺图/乱码 |
| R-03 | 资源热更新 | 修改主题 XML 后重建（macOS 走 Bundle 同步；Windows/Linux 直接读 `bin/resources`） | 新样式生效，无需删产物 |
| R-04 | CEF 示例（Linux/macOS arm64） | 打开 `cef` / `CefBrowser` | 页面加载正常，退出不崩溃 |
| R-05 | Bundle 迁移（macOS） | C11 场景自动执行 | 迁移到其他目录后仍能加载 `Contents/Frameworks` 与资源 |

## 6. 依赖与打包专项检查

| 检查项 | 命令/方法 | 失败含义 |
| :--- | :--- | :--- |
| Windows PE 架构 | `dumpbin /headers bin\<目标>.exe`、`dumpbin /headers lib\arm64\Release\duilib.lib` | 混入 x64 产物，需清理 `bin/` 与 `lib/` 重来 |
| Linux ELF 架构 | `file bin/<目标>`、`readelf -h bin/<目标>` | 同上 |
| macOS Mach-O 架构 | `file`、`lipo -archs`（含 `Contents/Frameworks` 内 dylib 与 CEF framework） | 同上，CEF 分发包架构不匹配 |
| Skia 目录 | `ls skia/out/llvm.arm64.release`（Debug 用 `.debug`；DLL 模式 `.md`/`.mdd`） | 缺 arm64 Skia，链接阶段报找不到库 |
| SDL3 | `deps` 在 arm64 上重新构建；macOS 检查 Bundle 内 dylib | 复用 x64 产物会链接/启动失败 |
| CEF 运行时架构 | Windows `dumpbin /headers bin\libcef_win[_109]\arm64\libcef.dll`；Linux `file bin/libcef_linux/libcef.so`；macOS `lipo -archs` 查 Bundle 内 Framework | 混入 x64 CEF，示例启动即失败 |
| 包内架构一致性 | `pack.py --verify`、`sdk_package_test.py` 的 P04/P06 | 包名架构与库架构不一致 |
| arm64 manifest | 包内 `msvc/manifest/duilib.arm64.manifest` 为 `processorArchitecture="ARM64"` | 下游接入选错 manifest |

## 7. 预期问题与处置

| 现象 | 原因 | 处置 |
| :--- | :--- | :--- |
| `Visual Studio C++ tools for arm64 are not installed` | VS 未装 ARM64 生成工具（`vcvarsall` 仍返回成功，脚本已显式拦截） | VS Installer 勾选「MSVC v143 ARM64 生成工具」后重开终端 |
| 链接报找不到 `skia.lib` | `skia/out/llvm.arm64.release` 不存在 | 用同样的 GN 参数构建 arm64 Skia，或 `--skia-lib-dir` 显式指定 |
| `pack.py --arch arm64 --with-cef` 告警「CEF 运行时目录不存在：bin/libcef_win/arm64」 | `deps` 之前按 x64 取包，`bin/libcef_win/arm64/` 里只有导入库或为空 | 删除 `bin/libcef_win/arm64` 后 `python3 build/build.py deps --arch arm64 --force` 重下 `windows/arm64` 归档 |
| Windows arm64 上 CEF 示例闪退/加载 `libcef.dll` 失败 | `bin/libcef_win[_109]/arm64/libcef.dll` 仍是 x64 的（手工拷贝或旧缓存） | `dumpbin /headers ...\libcef.dll` 确认 AA64，必要时删除后重下 |
| Linux / macOS arm64 上 CEF 示例起不来、加载 `libcef` 失败 | 仍在使用 x64 的 CEF 分发包（`bin/libcef_linux`、`<deps>/cef_binary` 不区分架构） | 删除对应目录后 `deps --arch arm64 --force` 重取 `linux/arm64`、`macos/arm64` 归档，再清理 `bin/` 重链 |
| macOS 启动崩溃、栈顶 `+[UNUserNotificationCenter currentNotificationCenter]` | `Contents/Info.plist` 缺失（历史问题已修复） | 确认使用当前 `build.py`；必要时 `rm -rf bin/basic.app` 重构建 |
| 链接/打包报架构不匹配 | `bin/`、`lib/` 混有另一架构或另一配置 | 按矩阵回退步骤清理后重跑 |
| Linux 交叉编译 arm64 失败 | 脚本未提供 Linux 交叉工具链参数，默认按主机架构 | 优先 arm64 真机/虚拟机原生构建 |

## 8. 验收标准

1. Phase 0 全部通过（脚本层不回归）。
2. Windows、Linux、macOS 的 arm64 Release 与 Debug 构建均成功，产物架构经上述命令确认。
3. 每平台至少一个 arm64 包 `pack.py --verify` 通过，`sdk_package_test.py` 中可执行的场景
   全 PASS（SKIP 需注明原因，且 C01 不得因缺工具链而 SKIP）；带 `--with-cef` 的包还需
   包含对应架构的 CEF 运行时（W-A06/W-A07、L-A04、M-A05）。
4. 示例启动验证（C10）在 Release 与 Debug 包上各通过一次。
5. macOS 额外满足：Bundle `Info.plist` 存在、Frameworks 与 rpath 正确、资源同步生效、
   C11 迁移后仍可运行。
6. 每个结论附可复现命令与关键输出（`report.json` 的 PASS/FAIL/SKIP 数、`file` /
   `lipo` / `dumpbin` 输出片段）。

## 9. 结果回填

| 位置 | 回填内容 |
| :--- | :--- |
| `build/test-matrix.md` | W-B18、L-B04、M-B03 改为「通过」并补备注；§3/§4/§5 增补 arm64 的包与包测试编号（可仿 3.2/4.2/5.2 的形式）；§7 进展表「arm64」一行按平台拆分；§7 已知缺口第 5 条相应删除或改写 |
| `build/README.md` / `README.en.md` | 更新 arm64 支持范围说明（三平台均可含 CEF arm64；真机结论待补） |
| `CHANGELOG.md` / `.en.md` | 如有修复（工具链探测、Skia 路径、交叉构建参数）按「修复」条目记录 |
| `build/test-matrix.md` §7 | 本轮新发现的问题按「本轮发现与修复」表格记录 |

## 10. 执行顺序与耗时估算

```text
Phase 0（任意 x64 机器）          0.5 h
Windows arm64（含装工具集）        2–4 h
Linux arm64                       1–2 h（若需先构建 arm64 Skia，另加数小时）
macOS arm64                       1–2 h
结果回填                          0.5 h
```

顺序：Phase 0 → Windows → Linux → macOS。每个平台内部：Release 构建 → Debug 构建 →
出包 → `pack.py --verify` → `sdk_package_test.py`（先不带 `--run-examples`，再带）→
运行验证 → 回填。
