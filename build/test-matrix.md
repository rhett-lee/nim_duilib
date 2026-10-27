# 构建与打包测试矩阵

本文档汇总 nim_duilib 统一构建入口（`build/build.py`）与打包脚本（`build/pack.py`）的
全部测试组合，供后续版本回归测试直接参照。矩阵按平台、工具链、架构、配置和功能开关
划分，并记录当前验证状态。

- 更新日期：2026-10-27
- 适用版本：新构建系统（`out/<arch>/<Config>` + `lib/<arch>/<Config>` + `bin/`）
- 命令示例中 Windows 用 `python`，Linux/macOS/FreeBSD 用 `python3`
- 每个组合出包后都必须执行 `pack.py --verify <包>` 校验条目数、文件 SHA 与文档链接

## 状态标记

| 标记 | 含义 |
| :--- | :--- |
| 通过 | 2026-10-26 ~ 27 本轮实际执行并通过 |
| 历史通过 | 新构建系统验证阶段（2026-10-23 ~ 25）执行并通过，本轮未复测 |
| 部分 | 已执行但存在已知限制，或仅完成构建/打包中的一项 |
| 未测 | 尚未验证 |
| 不适用 | 项目不支持该组合 |

## 0. 测试环境基线

| 项目 | 本轮环境 |
| :--- | :--- |
| 操作系统 | Windows 10/11 x64 |
| Visual Studio | 2022 17.14.37710.0（MSVC x64/x86；无 arm64 工具集） |
| CMake / Ninja | CMake 4.3.0，Ninja（VS 自带） |
| Python | 3.14.4 |
| MSYS2 | `E:\msys64`（MINGW64、UCRT64、CLANG64；MINGW32 缺 gcc/make） |
| 独立 MinGW | `C:\mingw64\llvm-mingw-20250430-ucrt-x86_64` |
| Skia | `6f559bafbed4c8323a899df4008aa073df4eccc6` |
| SDL3 | `release-3.4.18` |
| CEF | 默认 142；109 可选（Windows 7） |

MinGW 环境变量模板（每次新终端都要设置）：

```powershell
$env:MSYSTEM='MINGW64'
$env:PATH='E:\msys64\mingw64\bin;' + $env:PATH
$env:CC='E:\msys64\mingw64\bin\gcc.exe'
$env:CXX='E:\msys64\mingw64\bin\g++.exe'
```

独立 llvm-mingw 还需把 `E:\msys64\mingw64\bin` 放在 PATH 中提供 ninja/make，
`CC`/`CXX` 仍指向 llvm-mingw 的编译器。

## 1. Windows 构建矩阵

| 编号 | 工具链 | 架构/配置 | 关键开关 | 范围 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| W-B01 | MSVC | x64 / Debug | 默认（/MT、CEF 142、无 SDL） | 库 + 全部示例 | `python build/build.py build --config Debug` | 通过 | 861/861；过滤其他平台入口后增量构建为 21 次链接，新生成的 exe 均写入 `.build-config` |
| W-B02 | MSVC | x64 / Debug | `--cef-version 109` | 库 + 全部示例 | `... --config Debug --cef-version 109` | 通过 | 838/838，109 目标更少 |
| W-B03 | MSVC | x86 / Debug | 默认 | 库 + 全部示例 | `... --config Debug --arch x86` | 通过 | 861/861 |
| W-B04 | MSVC | x64 / Release | 默认 | 库 + 全部示例 | `... --config Release` | 通过 | 861/861 |
| W-B05 | MSVC | x86 / Release | 默认 | 库 + 全部示例 | `... --config Release --arch x86` | 通过 | 861/861 |
| W-B06 | MSVC | x64 / Release | `--cef-version 109` | 库 + 全部示例 | `... --config Release --cef-version 109` | 通过 | 537/537 |
| W-B07 | MSVC | x64 / Debug | `--md --sdl` | 库 + 全部示例 | `... --config Debug --md --sdl` | 通过 | 861/861；单配置生成器只构建/安装 Debug 版 SDL3（`/MDd`） |
| W-B08 | MSVC | x64 / Release | `--md` | 库 + 全部示例 | `... --config Release --md` | 通过 | 861/861（Skia 用 `.md` 变体） |
| W-B09 | MSVC | x64 / Release | `--sdl`（/MT） | 库 + 全部示例 | `... --config Release --sdl` | 通过 | 763/763，示例启动验证通过 |
| W-B10 | MSVC | x64 / Release | 关闭 CEF | 库 + 非 CEF 示例 | `... --config Release --cmake-arg=-DDUILIB_ENABLE_CEF=OFF` | 通过 | 328/328，21 个示例；缓存中残留的 cef/CefBrowser 已自动剔除 |
| W-B11 | MSVC | x64 / Debug | `--target duilib --no-examples --lang en` | 仅库 | `... --config Debug --target duilib --no-examples --lang en` | 通过 | 307/307，英文提示生效 |
| W-B12 | MSYS2 gcc | x64 / Release | 默认（CEF 不支持） | 库 + 非 CEF 示例 | `... --config Release --build-dir <临时目录>` | 通过 | 571/571 |
| W-B13 | llvm-mingw | x64 / Release | 默认 | 库 + 非 CEF 示例 | 同上，`CC/CXX` 指向独立 llvm-mingw | 通过 | 571/571，basic.exe 冒烟启动正常 |
| W-B14 | MSYS2 gcc | x64 / Release | `--sdl` | 库 + 非 CEF 示例 | `... --config Release --sdl` | 通过 | 571/571，SDL3 按 gcc 重建（CRT stamp 触发） |
| W-B15 | MSYS2 gcc | x64 / Release | `-DDUILIB_MINGW_STATIC=OFF` | 库 + 非 CEF 示例 | `... --config Release --cmake-arg=-DDUILIB_MINGW_STATIC=OFF --jobs 3` | 通过 | 571/571；objdump 确认示例依赖 libstdc++-6/libgcc_s_seh-1/libwinpthread-1.dll |
| W-B16 | MSYS2 gcc/llvm | x64 / Debug | 默认 | 库 | `... --config Debug --target duilib --no-examples` | 通过 | MINGW64 本轮实测 285/285（libduilib.a 333.2 MB）；UCRT64/CLANG64 历史通过；示例未复测 |
| W-B17 | MSYS2 MINGW32 | x86 / Release | 默认 | 库 + 示例 | — | 未测 | 环境缺 gcc/make，需先补齐工具链 |
| W-B18 | MSVC | arm64 / Debug/Release | 默认 | 库 + 示例 | `... --arch arm64` | 未测 | VS 未安装 arm64 工具集 |
| W-B19 | MSVC | x64 / Debug | `--dry-run` | 仅打印命令 | `... --config Debug --dry-run` | 通过 | 不执行配置/编译，不输出“编译完成” |
| W-B20 | MSVC | x64 / Debug | 复用构建目录 | 有状态选项回归 | 先 `--no-examples --md --sdl`，再默认构建 | 通过 | 默认值显式传入，不继承旧选项 |

构建矩阵执行要点：

- 在同一个 `out/<arch>/<Config>` 中切换工具链会被自动清理；但 `lib/<arch>/<Config>`、
  `bin/` 不会自动清理，切换工具链或架构前需手工清理，否则打包会报混合库/架构不一致。
- 手工清理 `bin/` 时只删 `*.exe`、`*.dll`、`*.build-config`；**`bin/resources.zip` 是仓库
  跟踪的 748 字节占位文件**（示例 `.rc` 通过 `IDR_THEME THEME "..\..\bin\resources.zip"`
  内嵌引用），误删会导致 windres 报 `can't open file '..\..\bin\resources.zip'`，
  需 `git checkout -- bin/resources.zip` 恢复。
- `--md`、`--sdl`、`--no-examples`、`--cef-version` 等是有状态选项，`build.py` 每次都会
  显式传入解析后的值；直接调用 `cmake` 复用目录时才需要手工清缓存。
- MinGW Debug 示例单个体积约 200–280 MB，全部示例约 10 GB；日常回归优先用 Release。
- MinGW 编译/链接内存不足时用 `--jobs 2` 或 `--jobs 3`，默认并行度可能触发
  `cc1plus/ld out of memory`。
- `DUILIB_MINGW_STATIC=OFF` 时示例依赖 `libstdc++-6.dll`、`libgcc_s_seh-1.dll`
  （x86 为 `libgcc_s_dw2-1.dll`）、`libwinpthread-1.dll`，构建不会自动复制到 `bin/`。

## 2. Windows 打包矩阵

打包前置：确认 `lib/<arch>/<Config>` 只有一种工具链的库（MSVC `.lib` 或 MinGW `.a`），
`bin/` 中只有目标架构的示例（Windows 各架构共用 `bin/`）。

| 编号 | 内容组合 | 命令示例 | 结果（条目 / 大小） | 状态 |
| :--- | :--- | :--- | :--- | :--- |
| W-P01 | x64 Debug + 示例 + CEF 142 | `python build/build.py pack --config Debug --with-examples --with-cef` | 1316 / 412.3 MB | 通过 |
| W-P02 | x64 Debug + 示例 + CEF 109 | `... --config Debug --with-examples --with-cef --cef-version 109` | 1134 / 373.5 MB | 通过 |
| W-P03 | x86 Debug + 示例 + CEF 142 | `... --config Debug --arch x86 --with-examples --with-cef` | 1314 / 368.1 MB | 通过 |
| W-P04 | x64 Release + 示例 + CEF 142 | `... --config Release --with-examples --with-cef` | 1316 / 328.1 MB | 通过 |
| W-P05 | x86 Release + 示例 + CEF 142 | `... --config Release --arch x86 --with-examples --with-cef` | 1314 / 287.8 MB | 通过 |
| W-P06 | x64 Release + 示例 + CEF 109 | `... --config Release --with-examples --with-cef --cef-version 109` | 1134 / 280.8 MB | 通过 |
| W-P07 | x64 Debug + 示例 + SDL + MD | `... --config Debug --md --with-examples --with-sdl --with-cef` | 1396 / 416.5 MB | 通过 |
| W-P08 | x64 Release + 示例 + MD | `... --config Release --md --with-examples --with-cef` | 1316 / 304.2 MB | 通过 |
| W-P09 | x64 Release + 示例 + SDL | `... --config Release --with-examples --with-sdl --with-cef` | 1396 / 352.8 MB | 通过 |
| W-P10 | x64 Release + 示例，无 CEF | `... --config Release --with-examples` | 762 / 121.8 MB | 通过 |
| W-P11 | x64 Debug，仅库无示例 | `... --config Debug` | 546 / 63.5 MB | 通过 |
| W-P12 | MinGW gcc Release + 示例（无 CEF） | `... --config Release --with-examples` | 757 / 208.2 MB | 通过 |
| W-P13 | llvm-mingw Release + 示例（无 CEF） | 同上（MinGW 环境） | 757 / 147.0 MB | 通过 |
| W-P14 | MinGW gcc Release + 示例 + SDL（无 CEF） | `... --config Release --with-examples --with-sdl` | 837 / 243.0 MB | 通过 |
| W-P15 | MinGW 动态链接 Release + 示例（无 CEF） | `... --config Release --with-examples`（源码关闭静态链接） | 757 / 195.4 MB | 通过，有限制 |
| W-P16 | x64 Debug，zip / tgz / tar.bz2 | `pack.py --config Debug --format zip\|tgz\|tarbz2` | 546 / 63.5 MB 各一份 | 通过 |
| W-P17 | 显式 MSVC 运行库 MT / MD | `pack.py --msvc-runtime mt` / `md` | 元数据与库一致 | 通过 |
| W-P18 | DLL 模式包 | `pack.py --verify dist/<旧 DLL 包>` | — | 部分 |

打包专项说明：

- W-P15：关闭静态链接后示例依赖 MinGW 运行库，当前构建与打包不会自动收集这些 DLL；
  分发前需手工复制到 exe 同级。包内 README 已提示。
- W-P18：新 CMake 工程不提供 duilib 的 DLL 构建目标；`bin/` 中出现 duilib DLL 时
  `pack.py` 仍可按 DLL 模式收集。仓库内旧 `dist/` DLL 包因历史文档链接失效未通过
  `--verify`，属于旧产物，不代表当前打包逻辑回归。
- `--with-examples` 未加 `--with-cef` 时会跳过 `cef.exe`/`CefBrowser.exe`，避免包内
  留下必然启动失败的二进制。
- 包名模式：`nim_duilib-<版本>-win-<arch>[-sdl][-cef|-cef109]-examples[-debug].zip`。

## 3. Linux 构建与打包矩阵

Unix 平台一次只构建一个配置，库输出到 `lib/<Config>`；`--md` 仅 MSVC 有效。

| 编号 | 平台 / 工具链 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| L-B01 | Linux gcc | x64 / Release | 默认（SDL 默认开启） | `python3 build/build.py --config Release` | 历史通过 | 构建 + 出包已验证 |
| L-B02 | Linux clang | x64 / Release | 默认 | 同上，`CC/CXX=clang/clang++` | 未测 | 预检与 Skia 工具链优先 clang |
| L-B03 | Linux | x64 / Debug | 默认 | `python3 build/build.py --config Debug` | 未测 | |
| L-B04 | Linux | arm64 / Release | 默认 | `--arch arm64` | 未测 | 缺真机/交叉工具链 |
| L-B05 | Linux gcc | x64 / Release | 关闭 SDL | `--no-sdl` | 未测 | |
| L-B06 | Linux | x64 / Release | `--with-cef` 打包 | `pack --with-cef` | 未测 | 代码路径有单元测试，缺实机包 |
| L-B07 | Linux | x64 / Release | 运行验证 | 解包运行示例 | 未测 | 依赖系统 X11/字体 |

## 4. FreeBSD 构建与打包矩阵

FreeBSD 复用 Linux/X11 分支；不支持 CEF，`--with-cef` 直接报错。

| 编号 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| F-B01 | x64 / Release | 默认 | `python3 build/build.py --config Release` | 历史通过 | 构建 + 出包 |
| F-B02 | x64 / Debug | 默认 | `--config Debug` | 未测 | |
| F-B03 | x64 / Release | `--with-cef` | `pack --with-cef` | 历史通过（负向） | 预期报错，CEF 不支持 |
| F-B04 | x64 / Release | 运行验证 | 解包运行示例 | 未测 | |

## 5. macOS 构建与打包矩阵

| 编号 | 架构/配置 | 关键开关 | 命令示例 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| M-B01 | x64 / Release | 默认 | `python3 build/build.py --config Release` | 部分 | VERSION/头文件、SDL3 dylib 等修复已合入，缺最终全量通过记录 |
| M-B02 | x64 / Debug | 默认 | `--config Debug` | 未测 | |
| M-B03 | arm64 / Release | 默认 | `--arch arm64` | 未测 | 缺真机验证 |
| M-B04 | x64 / Release | CEF 示例 / `.app` | 构建 `cef`、`CefBrowser` | 部分 | Helper/Framework/wrapper 修复已合入，实机 Bundle 待确认 |
| M-P01 | x64 / Release | 无 CEF 出包 | `pack --with-examples` | 未测 | 非 Windows 按可执行位识别示例 |
| M-P02 | x64 / Release | CEF `.app` 出包 | `pack --with-examples --with-cef` | 部分 | Bundle 收集逻辑与单元测试完成；不收集 CEF 开发文件/运行时 |

## 6. 跨平台通用命令矩阵

| 编号 | 命令 | 覆盖内容 | 状态 | 备注 |
| :--- | :--- | :--- | :--- | :--- |
| G-01 | `build.py doctor` | 工具链、版本清单、依赖目录、预检 | 通过 | Windows 本轮；Linux/FreeBSD/macOS 历史 |
| G-02 | `build.py deps --offline` | 离线采纳已有 Skia/CEF/SDL3；本地源码不在清单 ref 上时先切到本地已有的提交 | 通过 | 不访问网络，stamp 校验；Windows 本轮实测 SDL 由 `main` 切到 `release-3.4.18` |
| G-03 | `build.py clean` | 删除整个 `out/`；`--build-dir` 可只清理指定构建目录 | 通过 | dry-run 仅打印路径 |
| G-04 | `build.py clean --deps --dry-run` | 展示依赖清理范围 | 通过 | 不会删除源码克隆 |
| G-05 | `build.py build/pack/test --dry-run` | 只打印命令 | 通过 | build/clean 不再误报完成 |
| G-06 | `build.py test` | 构建/打包单元测试 | 通过 | 174 项全部通过 |
| G-07 | 负向校验 | 混合库、架构不一致、CEF 版本不匹配、错误 ref/archive | 通过 | 均在配置/打包阶段明确报错 |

## 7. 当前测试进展（2026-10-26）

| 范围 | 已完成 | 结果 |
| :--- | :--- | :--- |
| Windows P1（关键出包组合） | 7 / 7 | 全部通过 |
| Windows P2（参数分支与工程逻辑） | 9 / 9 | 全部通过 |
| Windows 自动测试 | 158 项 | 全部通过 |
| Linux x64 Release 构建与打包 | 历史验证 | 通过（运行时未验证） |
| FreeBSD x64 Release 构建与打包 | 历史验证 | 通过（CEF 不支持，运行时未验证） |
| macOS 构建与打包 | 修复已合入 | 部分，待最终全量确认 |
| arm64（Windows/Linux/macOS） | 0 | 未测 |

本轮验证过的代表性产物（中间包已清理，仅保留最终包）：

| 产物 | 条目 | 大小 |
| :--- | ---: | ---: |
| win-x64 Debug CEF 示例（最终保留） | 1316 | 420.2 MB |
| win-x64 Debug CEF109 示例 | 1134 | 373.5 MB |
| win-x86 Debug CEF 示例 | 1314 | 368.1 MB |
| win-x64 Release CEF 示例 | 1316 | 328.1 MB |
| win-x86 Release CEF 示例 | 1314 | 287.8 MB |
| win-x64 Release CEF109 示例 | 1134 | 280.8 MB |
| win-x64 Debug MD + SDL 示例 | 1396 | 421.1 MB |
| win-x64 Release MD 示例 | 1316 | 304.2 MB |
| win-x64 Release SDL 示例 | 1396 | 352.8 MB |
| win-x64 Release 无 CEF 示例 | 762 | 121.8 MB |
| win-x64 Debug 仅库 | 546 | 63.5 MB |
| win-x64 MinGW gcc Release 示例 | 757 | 214.6 MB |
| win-x64 llvm-mingw Release 示例 | 757 | 151.8 MB |
| win-x64 MinGW gcc Release SDL 示例 | 837 | 248.2 MB |
| win-x64 MinGW 动态链接 Release 示例 | 757 | 201.2 MB |

最终包位置：`E:\packtest\final\nim_duilib-1.0.2026.1023-win-x64-cef-examples-debug.zip`
（420.2 MB，1316 条目，`pack.py --verify` 全部匹配）。

### 本轮发现与修复

| 问题 | 处理 |
| :--- | :--- |
| 复用 `out/<arch>/<Config>` 时 CMake 有状态选项残留（`--no-examples`、`--md`、`--sdl`、`--cef-version`） | 每次显式传入解析后的值，新增 3 项回归测试 |
| SDL3 构建目录被其他工具链/运行库使用后仍复用旧缓存（MinGW 下调用 `cl.exe`） | 按 `.duilib-sdl` stamp 校验 CRT，不一致时清空重建 |
| `build`/`clean` 的 `--dry-run` 仍打印“编译完成/已删除” | dry-run 下不再输出完成提示，新增 1 项测试 |
| MinGW 禁用静态链接后示例缺运行库 DLL | 已验证构建/打包，文档补充运行库清单；不由构建自动收集 |

### 2026-10-27 依赖版本一致性（Windows 已验证）

| 问题 | 处理 | 验证 |
| :--- | :--- | :--- |
| `<deps-root>/SDL` 停在 `main`，清单为 `release-3.4.18`，脚本仍编译本地源码并复用旧 stamp | `deps` 先按清单切换 tag/commit（本地已有提交不联网，`--offline` 可用）；`.duilib-sdl` 记录源码提交，切换后旧产物重建 | `build.py deps --sdl --offline`：HEAD `3b2bc95` → `829a65d`，SDL3 重建，stamp 含 `829a65d...`；二次运行 `[OK] SDL3 is ready` |
| `skia_compile` 同名补丁归档被替换成旧版本后仍直接复用，Skia 产物不更新 | 先 `git status` 核对归档与当前提交，必要时 `git checkout -- <zip>` 恢复，再 `git pull`；补丁指纹变化时重新解压并重建 Skia | 篡改本地 zip 后 `doctor` 输出 `[WARN] skia patch differs from skia_compile HEAD`；`deps --offline` 恢复归档（`hash-object` 与 `HEAD:zip` 一致），Skia 未无谓重建 |
| `--skia-ref` 选中的 tag/branch 不检查本地工作区 | 与 commit ref 一样按清单对齐；branch 只要求当前在该分支，不追远端 | 临时仓库隔离验证：tag、branch 均可切换；离线缺 ref 时明确报错 |

### 已知缺口与风险

1. MinGW `DUILIB_MINGW_STATIC=OFF` 的包不包含 `libstdc++-6.dll` 等运行库，分发前需手工补齐。
2. 旧 `dist/` DLL 包未通过 `--verify`（历史文档链接失效）；新工程也没有 DLL 构建目标。
3. Linux/FreeBSD 仅验证 Release 构建与打包，Debug、解包运行、下游工程接入未验证。
4. macOS 修复已合入，但最终全量构建、`.app` 打包与运行仍待确认。
5. Windows/Linux/macOS 的 arm64 均未实测；Windows 缺 arm64 MSVC 工具集。
6. `bin/` 与 `lib/<arch>/<Config>` 被多架构/多工具链共用，切换后必须清理，否则打包
   会因 PE 架构或混合库校验失败（这是预期保护，不是缺陷）。
7. MinGW 默认并行度在内存较小的机器上可能 OOM，回归时优先 `--jobs 2`/`--jobs 3`。
8. Unix 的 SDL3 stamp 不再含配置分量（修复前切换 Debug/Release 会无谓重建 SDL3）；
   需在 Linux/macOS 真机验证：先 Release 后 Debug 各跑一次 `build.py deps`，第二次
   不应重新编译 SDL3。
9. Unix 的 Skia 静态库改为绝对路径链接（与 Windows 一致）；需在 Linux/macOS 真机
   验证：重新编译 Skia 后 `ninja` 能触发示例程序重新链接，且链接命令不再依赖
   `link_directories` 的搜索顺序。
10. `_duilib_cef_tracked_dir` 已按平台返回跟踪目录（修复前 Linux/macOS 错对
    `libcef_win` 的跟踪头）；需在 Linux/macOS 真机验证一次 `build.py deps` 的 CEF
    下载与迁移采纳路径。
11. SDL3 源码 ref 对齐与源码提交 stamp 仅在 Windows 验证（`main` → `release-3.4.18`
    切换并重建）；需在 Linux/macOS/FreeBSD 真机验证：本地 `SDL/` 不在清单 tag 时
    能离线切换，切换后 `.duilib-sdl` 记录新提交并触发一次重建，二次运行直接复用。
12. `skia_compile` 补丁归档刷新（旧归档恢复 + `git pull` + 补丁指纹触发重建）仅在
    Windows 验证；需在 Unix 真机验证 `git status` 少文件路径在子目录下的行为一致，
    以及本地归档缺失时 `deps` 仍能从仓库恢复而不联网。
13. Skia tag/branch ref 对齐在临时仓库中隔离验证；需在真实 `skia/` 工作区验证
    `--skia-ref` 选 branch 时不会误 fetch 或切到 detached HEAD。

## 8. 回归执行顺序建议

1. `build.py test`，确认构建/打包脚本自身无回归。
2. Windows 默认 x64 Debug 全量构建，作为基线。
3. 按 W-B02 ~ W-B15 逐项构建，每项后立即出包并 `pack.py --verify`。
4. 切换工具链/架构前清理 `bin/*.exe`、`bin/*.dll` 与 `lib/<arch>/<Config>`。
5. Windows 完成后执行 G-01 ~ G-05 通用命令分支。
6. Linux、FreeBSD、macOS 按各自矩阵执行，Release 优先；有条件再补 Debug 与运行验证。
7. 回归结束恢复默认 x64 Debug 构建，并重新生成最终包校验。
