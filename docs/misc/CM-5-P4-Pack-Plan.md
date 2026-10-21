# CM-5 / P4：pack.py 适配统一布局与 SDK 可交付性（方案设计）

> 状态：**Q1~Q14 已全部确认，代码与文档已实施（2026-10-21）**。Debug 侧自测通过；Release 包与最小消费工程验收已由用户确认（2026-10-21），构建系统的系统性测试后续单独形成计划。
> 依据：[CM-5 产物盘点](CM-5-Build-Artifacts-Inventory.md) 的 D-2（方案 B，统一布局）与 D-7（pack 采用方案 C，只认一套布局）。
> 范围：只改 `build/pack.py` 与描述其 CLI 的文档；不改构建脚本、CMake、vcxproj、产物布局。
> 日期：2026-10-21。

---

## 一、目标

P1（CMake 侧）与 P3（VS 侧）已把库产物统一到：

| 平台 | 库目录 |
| :--- | :--- |
| Windows | `lib/<x64\|x86>/<Release\|Debug>/` |
| Linux / macOS / FreeBSD | `lib/<Release\|Debug>/` |

P4 的目标：

1. 让 `pack.py` **只认这一套布局**：默认只收 `Release`，配置由目录决定，不再靠 `_d` 或候选目录猜测；
2. 删除历史遗留的双套命名（`zlib` / `libpng` / `libwebp` / `cximage`）与多目录合并逻辑；
3. 修复 Unix 侧 `.a` 静态库无法收集、`lib` 前缀匹配不上的缺陷（P4-B1/B2）；
4. **让打出的 SDK 真正可交付**：`include/` + `lib/` 能支撑应用层完成编译、链接、运行（P4-B3，新增要求）。

---

## 二、现状与问题

### 2.1 当前实现（[pack.py](../../build/pack.py)）

| 维度 | 现状 | 位置 |
| :--- | :--- | :--- |
| 候选库目录 | Windows：`lib/x64`、`lib/Debug`、`lib/Release`、`lib`；Unix：`lib`、`lib/Release`、`lib/Debug` | `default_lib_dirs()` |
| 目录合并 | 合并**所有存在**的目录，同名库由高优先级目录胜出 | `collect_libs()` |
| 库名白名单 | 同时列两套：`duilib`、`duilib-*` 与 `cximage`、`libpng`、`libwebp`、`zlib` | `LIB_BASENAMES` |
| Debug 判定 | 文件 `_d` 后缀 + 目录名 `Debug` | `_scan_lib_dir()` / `_is_debug_dir()` |
| Debug 开关 | `--allow-debug-libs` | `argparse` |
| 候选目录注释 | 第 93~94 行把 CMake / MSBuild 的命名**写反了** | 文件头注释 |

### 2.2 布局统一后已失效的部分

1. `lib/x64` 与 `lib/Debug` 这类候选目录已不存在（P3 后 VS 输出到 `lib/x64/Release`，P1 后 CMake Debug 输出到 `lib/x64/Debug`），旧候选列表只会命中迁移前的残留产物。
2. 白名单中的 `zlib` / `libpng` / `libwebp` / `cximage` 已不再产出（D-1）。
3. `_d` 判定已无意义（D-2：库名不带 `_d`）；新规则下"配置只由目录表达"。
4. 多目录合并会把 `Debug` 与 `Release` 一起扫，再靠优先级二选一，与新规则冲突。

### 2.3 Unix 侧缺陷（已确认修复：P4-B1 / P4-B2）

| 编号 | 问题 | 后果 |
| :--- | :--- | :--- |
| **P4-B1** | `EXCLUDE_LIB_SUFFIXES` 包含 `.a`，而 `_scan_lib_dir()` 又只接受 `.lib` / `.a` | 两者矛盾：**Unix 静态库全部被排除**，打出的 Unix 包没有任何库 |
| **P4-B2** | 扫描键是文件名去扩展名（`libduilib-zlib.a` → `libduilib-zlib`），白名单是 `duilib-zlib`，未去掉 `lib` 前缀 | 即使修掉 B1，Unix 库仍匹配不上白名单 |

> Unix 静态库由 CMake 生成，默认带 `lib` 前缀：`libduilib.a`、`libduilib-zlib.a`、`libduilib-png.a`、`libduilib-webp.a`、`libduilib-cximage.a`；CEF wrapper 通过 `PREFIX ""` 输出为 `libcef_dll_wrapper.a`，恰好与白名单同名。

### 2.4 第三方链接库缺失（P4-B3，已确认 Q6~Q9）

| 组件 | 现状 | 影响 |
| :--- | :--- | :--- |
| Skia | `lib/` 中**没有** Skia 静态库 | `duilib.lib` 中有大量 Skia 符号未解析，下游只拿 `lib/` 里的 5 个库**无法链接** |
| SDL3 | `lib/` 中**没有** SDL3 库与头文件 | Unix 默认 `DUILIB_ENABLE_SDL=ON`，下游链接缺 `SDL3`，且使用 SDL 的代码缺头文件 |
| 许可证 | 未带 Skia / SDL 的许可证 | 再分发不合规 |

**实测事实**：

- Skia Windows Release 目录 `skia/out/llvm.x64.release/` 有 **10 个** `.lib`：`skia`(20.7 MB)、`skottie`(8.3)、`svg`(7.3)、`sksg`(6.7)、`bentleyottmann`(0.2)、`expat`(0.3)、`jsonreader`(0.1)、`skcms`(0.1)、`skresources`(0.1)、`skshaper`(0.1)；Debug 目录同名（`llvm.x64.debug`）。
- `DUILIB_SKIA_LIBS` 只列了 6 个（`svg skshaper skottie sksg jsonreader skia`），其余 4 个是 Skia 的传递依赖；**打包应带上全部 10 个**，下游按需链接。
- Windows SDL3：`SDL3/lib/SDL3-static.lib`（15 MB），静态库，无运行时 DLL。
- Unix SDL3：`build_duilib_all_in_one.sh` 以 `SDL_SHARED=ON` 安装到 `SDL3/lib64`（或 `lib`），即 **动态库 `libSDL3.so*`**，运行时需要随包分发并设置库路径。
- SDL3 头文件：`SDL3/include/SDL3/**`；许可证：`SDL3/licenses/SDL3`。
- Skia 许可证：`skia/LICENSE`。

---

## 三、目标规则

### 3.1 库目录解析（唯一，不再合并）

| 平台 | 解析结果 | 说明 |
| :--- | :--- | :--- |
| Windows | `<root>/lib/<arch>/<Config>` | `arch` 默认取主机架构（`AMD64→x64`、`x86→x86`、`ARM64→arm64`），可用 `--arch` 覆盖 |
| Unix | `<root>/lib/<Config>` | 目录不含架构层（D-2） |

- `<Config>` 取值 `Release` / `Debug`，默认 `Release`，大小写不敏感（统一规范化为 `Release` / `Debug`）。
- **只读取选中的一个目录**，不合并、不按优先级回退。
- `--lib-dir` 保留，作为完全显式指定（如 `--lib-dir lib/x86/Debug`）；此时 `--arch` / `--config` 不参与路径计算，但仍决定包名与示例过滤。
- 目录不存在 → **直接失败（退出码 1）**，提示应检查的路径与构建命令。
- 主库 `duilib`（`duilib.lib` / `libduilib.a`）缺失 → **失败**；4 个 `duilib-*` 库缺失 → 告警（允许只打头文件/资源的场景）。

### 3.2 库名白名单

```text
duilib
duilib-cximage
duilib-png
duilib-webp
duilib-zlib
```

- 删除 `cximage` / `libpng` / `libwebp` / `zlib`。
- CEF wrapper 保持 `libcef_dll_wrapper`、`libcef_dll_wrapper_109`，仍只在 `--with-cef` 时收集。
- 匹配规则：
  - Windows：仅接受 `.lib`，文件名去扩展名后与白名单精确相等；
  - Unix：仅接受 `.a`；去扩展名后若以 `lib` 开头且去掉后命中白名单，则视为命中（`libduilib-zlib.a` → `duilib-zlib`）；`libcef_dll_wrapper.a` 去掉后为 `cef_dll_wrapper`（不在白名单），按原名匹配。
- 排除扩展名：`.pdb`、`.ilk`、`.exp`、`.idb`、`.obj`、`.o`、`.so`、`.dylib`；**移除 `.a`**。

### 3.3 配置（Debug）语义

- 配置只由目录决定，**删除** `_is_debug_dir()`、`_scan_lib_dir()` 的 Debug/Release 分流以及 `--allow-debug-libs`。
- 新增 `--config {Release,Debug}`，默认 `Release`。
- `--config Debug` 时：
  - 只读 `Debug` 目录；
  - 包名追加 `-debug`：`nim_duilib-<版本>-<系统>-<架构>-debug.zip`；
  - `MANIFEST.txt` 增加 `配置 : Debug`，并提示下游必须使用 Debug 运行库；
  - 示例不做 `_d` 过滤（CMake 的 Debug exe 与 Release 同名，无法区分），输出告警说明可能混有 Release exe。

### 3.4 示例程序（I-2 例外，仅做最小过滤）

- `--with-examples` 语义不变：收集 `bin/*.exe` 与 `WebView2Loader.dll`。
- `--config Release` 跳过以 `_d.exe` 结尾的程序（VS Debug 产物）。
- `--config Debug` 全部收集（见 3.3 告警）。
- 不改 `bin/` 布局与 exe 命名。

### 3.5 CLI 变更

| 参数 | 变更 | 说明 |
| :--- | :--- | :--- |
| `--config {Release,Debug}` | **新增**，默认 `Release` | 选择 `lib/.../Release` 或 `lib/.../Debug` |
| `--arch {x64,x86,arm64}` | **新增**，默认主机架构 | Windows 影响库目录；所有平台影响包名 |
| `--allow-debug-libs` | **移除** | 由 `--config Debug` 取代，不留别名 |
| `--lib-dir` | 保留 | 完全显式指定 duilib 库目录 |
| `--skia-lib-dir` | **新增**（P4-B3） | 直接指定 Skia 的 `out/<变体>` 目录；默认自动探测 |
| `--sdl-dir` | **新增**（P4-B3） | 指定 SDL3 根目录；默认 `<repo>/../SDL3` |
| `--with-sdl` | **新增**（P4-B4），仅 Windows，默认**关闭** | 启用时打包 `SDL3-static.lib` 与 `include/SDL3/`；Unix 始终打包，无需该参数 |
| `--with-cef` | **新增**（P4-B4），默认**关闭** | 一次带齐 CEF 头文件、`libcef.lib`、wrapper 库与许可证；替代原 `--with-cef-libs` |
| `--cef-version {latest,109}` | **新增**（P4-B4），默认 `latest` | 选择 `libcef_win` + `libcef_dll_wrapper` 或 `libcef_win_109` + `libcef_dll_wrapper_109` |
| 其余参数 | 保持不变 | `--with-examples`、`--format`、`--out-dir`、`--list` |

### 3.6 包名与清单

| 项 | Release | Debug |
| :--- | :--- | :--- |
| 包名 | `nim_duilib-<版本>-<系统>-<架构>` | `nim_duilib-<版本>-<系统>-<架构>-debug` |
| MANIFEST | `配置 : Release` | `配置 : Debug` |
| 包内库 | `lib/<文件名>`，平面结构 | 同左 |

### 3.7 Skia 库（P4-B3）

**目录解析**

| 平台 | 规则 |
| :--- | :--- |
| 显式 | `--skia-lib-dir <path>` 优先，不随配置变化 |
| Windows | `<repo>/../skia/out/llvm.<arch>.<debug\|release>`：Release → `llvm.x64.release`，Debug → `llvm.x64.debug`（D-3：两者均为 release 模式代码，仅 CRT 不同） |
| Unix | `<repo>/../skia/out/{llvm,gcc}.<arch>.release`：按顺序探测；都不存在时列出 `out/` 下的候选目录并失败 |

**收集规则**

- 收集该目录下**全部** `*.lib`（Windows）/ `*.a`（Unix），包内落到 `lib/<原文件名>`。
- 不收集 Skia 头文件：`RenderSkia/` 不对外暴露（`duilib.h` 不引用），下游无需 Skia 头文件。
- 目录缺失 → **失败**（Skia 是 duilib 的必选链接依赖）。
- Skia 的许可证 `<repo>/../skia/LICENSE` → 包内 `licenses/skia/LICENSE`。

**下游链接列表（写入 MANIFEST）**

```text
Windows: skia.lib;svg.lib;skshaper.lib;skottie.lib;sksg.lib;jsonreader.lib
         （bentleyottmann / expat / skcms / skresources 为 Skia 传递依赖，一并随包，
           出现未解析符号时补链）
Unix   : -lskia -lsvg -lskshaper -lskottie -lsksg -ljsonreader
```

### 3.8 SDL3 与头文件（P4-B3）

| 平台 | 收集条件 | 收集内容 | 包内位置 |
| :--- | :--- | :--- | :--- |
| Windows | **仅 `--with-sdl`**，默认不打包 | `SDL3/lib/SDL3-static.lib`（静态） | `lib/` |
| Unix | 始终打包 | `SDL3/lib64/` 或 `SDL3/lib/` 下的 `libSDL3.so*` / `libSDL3*.dylib` | `lib/` |
| 全平台 | 与上一列一致 | `SDL3/include/SDL3/**` | `include/SDL3/` |
| 全平台 | 与上一列一致 | `SDL3/licenses/**` | `licenses/SDL3/` |

- Windows 默认不打包 SDL（`SDLEnabled=0`）；使用 `--with-sdl` 显式开启。目录缺失时：开启则**失败**，未开启则忽略。
- Unix 默认启用 SDL（`DUILIB_ENABLE_SDL`），始终打包；目录缺失 → **失败**（缺库无法链接）。
- Windows 启用 `--with-sdl` 时，MANIFEST 提示：下游需定义 `DUILIB_SDL=1`（`duilib_config.h` 依据该宏启用 SDL 分支），并补链系统库 `Version.lib;Winmm.lib;Imm32.lib;Setupapi.lib`。
- Unix 为动态库：MANIFEST 提示运行期需让 `libSDL3.so*` 可被找到（复制到 exe 同级目录，或设置 `LD_LIBRARY_PATH` / macOS `DYLD_LIBRARY_PATH`）。

### 3.9 SDK 使用契约（写入 MANIFEST）

为满足"编译、链接、运行均能通过"，`MANIFEST.txt` 增加以下内容：

1. **包含路径**：`include/`（`#include "duilib/duilib.h"`；使用 SDL 时 `<SDL3/SDL.h>` 也由该路径覆盖；使用 CEF 时额外加 `include/duilib/third_party/libcef/<libcef_win|libcef_win_109>`）。
2. **库路径**：`lib/`（平面结构）。
3. **链接列表**：
   - 基础：`duilib` + `duilib-cximage` + `duilib-webp` + `duilib-png` + `duilib-zlib`；
   - Skia：`skia` + `svg` + `skshaper` + `skottie` + `sksg` + `jsonreader`（传递依赖缺失时补 `bentleyottmann` / `expat` / `skcms` / `skresources`）；
   - WebView2：`WebView2LoaderStatic`（VS）或 `WebView2Loader.dll`（CMake）；
   - JPEG（Windows）：`turbojpeg-static`；
   - SDL（可选）：`SDL3-static`（Windows）/ `SDL3`（Unix）；
   - CEF（可选）：`libcef` + `libcef_dll_wrapper[_109]`。
4. **平台系统库**：Windows 的 SDL 依赖（`Version`/`Winmm`/`Imm32`/`Setupapi`）、Unix 的 `X11`/`freetype`/`fontconfig`（系统提供，不随包）。
5. **运行期资源**：`resources/` 与 exe 同级；CEF 运行时不在包内（`libcef.dll`、`*.pak`、`icudtl.dat`、`locales/`）；WebView2 已带 `WebView2Loader.dll`。
6. **配置一致性**：Debug 包仅用于 Debug 下游（`/MTd`），Release 包用于 Release 下游（`/MT`）；两套不可混用。

### 3.10 third_party 全面梳理（P4-B4，待确认）

对 `duilib/third_party/` 逐目录核对后的结论：

| 类别 | 目录 | 产物形态 | 打包处理 |
| :--- | :--- | :--- | :--- |
| 已随包（默认） | `zlib` / `libpng` / `libwebp` / `cximage` | 独立静态库 + 头文件 | 已纳入：`duilib-zlib` / `duilib-png` / `duilib-webp` / `duilib-cximage` |
| **新增默认随包** | `Microsoft.Web.WebView2` | 头文件（已转发）+ loader 库 + DLL | 增加 `WebView2Loader.dll.lib`、`WebView2LoaderStatic.lib`（按架构）到 `lib/`；`WebView2Loader.dll` 已在 `bin/` |
| **新增默认随包** | `libjpeg-turbo` | 预编译静态库 + 头文件 | Windows 按架构增加 `turbojpeg-static.lib` 与 `include/duilib/third_party/libjpeg-turbo/*.h`；VS 默认 `JpegTurboEnabled=1`，`duilib.lib` 有 turbojpeg 符号，**必须随包** |
| 可选（`--with-cef`） | `libcef/libcef_win`、`libcef_win_109` | CEF 头文件 + `libcef.lib` 导入库 + wrapper 库 | 默认关闭；开启时按 `--cef-version` 带齐三者与 `LICENSE.txt`；运行库（`libcef.dll`、`*.pak`、`icudtl.dat`）仍外置 |
| 可选（`--with-sdl`） | SDL3（仓库同级，非 third_party） | Windows 静态库 / Unix 动态库 + 头文件 | 见 3.8：Windows 默认关闭，Unix 始终随包 |
| 源码内嵌，无需单独打包 | `giflib`（头文件已转发）、`stb_image`、`svg`(nanosvg)、`xml`(pugixml)、`convert_utf`、`libudis86` | 源码直接编入 `duilib.lib` | 仅转发对外头文件；不产生独立库 |
| 不适用 | `libpag` | 目录内只有许可证，无二进制 | 不打包 |
| 需记录（非 P4 处理） | `third_party/apng` | `duilib/CMakeLists.txt` 引用该目录，但仓库中不存在 | `aux_source_directory` 结果为空，属现有状态；只在文档记录 |

**许可证**：仓库根 `licenses/` 已包含 `skia.LICENSE.txt`、`SDL.LICENSE.txt`、`libjpeg-turbo.LICENSE.md`、`Microsoft.Web.WebView2.LICENSE.txt`、`libcef.LICENSE.txt` 等全部相关条目；`pack.py` 现有规则会整目录复制，**无需新增许可收集逻辑**。

**WebView2 loader 选择**（写入 MANIFEST）：

| 构建方式 | 需要的 loader | 包内文件 |
| :--- | :--- | :--- |
| VS 工程 | 静态 loader | `lib/WebView2LoaderStatic.lib` |
| CMake | 动态 loader | `lib/WebView2Loader.dll.lib` + `bin/WebView2Loader.dll` |

---

## 四、代码改动清单（`build/pack.py`）

| # | 位置 | 改动 |
| :--- | :--- | :--- |
| 1 | 文件头注释（15~18、35~40 行） | 删除"两条构建路径产物混在一起"的描述；说明唯一布局、`--config` 与第三方库来源 |
| 2 | `LIB_BASENAMES`（92~105 行） | 只保留 5 个 `duilib-*` 名称；修正写反的命名注释 |
| 3 | `EXCLUDE_LIB_SUFFIXES`（114~116 行） | 移除 `.a`（P4-B1） |
| 4 | `default_lib_dirs()`（597~609 行） | 改为返回**单个**目录：Windows `lib/<arch>/<Config>`、Unix `lib/<Config>` |
| 5 | `_is_debug_dir()` | 删除 |
| 6 | `_scan_lib_dir()` | 删除 Debug/Release 分流；增加 Unix `lib` 前缀归一化（P4-B2）；返回 `{基名: 实际文件名}` |
| 7 | `collect_libs()` | 去掉多目录合并与 `allow_debug_libs`；目录/主库缺失改为失败；保留第三方缺失告警与 CEF 体积提示 |
| 8 | 新增 `collect_skia_libs()` | 解析并收集 Skia 全部静态库（3.7） |
| 9 | 新增 `collect_sdl()` | Unix 始终收集 SDL3；Windows 仅在 `--with-sdl` 时收集库与头文件（3.8） |
| 10 | 新增 `collect_webview2_loader()` | 按架构收集 `WebView2Loader.dll.lib` 与 `WebView2LoaderStatic.lib`（3.10） |
| 11 | 新增 `collect_jpeg_turbo()` | Windows 按架构收集 `turbojpeg-static.lib` 与头文件（3.10） |
| 12 | 新增 `collect_cef()` | `--with-cef` 时按 `--cef-version` 收集 CEF 头文件、`libcef.lib`、wrapper 与许可证（3.10） |
| 13 | `collect_examples()` | Release 下跳过 `*_d.exe`；Debug 下输出"可能混合"告警 |
| 14 | `write_manifest()` | 增加 `配置`、Skia/SDL/WebView2/TurboJPEG/CEF 清单与 3.9 的使用契约 |
| 15 | `main()` / `argparse` | 新增 `--config`、`--arch`、`--skia-lib-dir`、`--sdl-dir`、`--with-sdl`、`--with-cef`、`--cef-version`；删除 `--allow-debug-libs` 与 `--with-cef-libs`；包名加 `-debug` |
| 16 | 底部用法示例与 `--help` | 同步新参数 |

---

## 五、文档同步清单（P4 内完成）

| 文件 | 需要修改的内容 |
| :--- | :--- |
| [docs/Release.md](../Release.md) | 参数表（删 `--allow-debug-libs` / `--with-cef-libs`，增 `--config`/`--arch`/`--skia-lib-dir`/`--sdl-dir`/`--with-sdl`/`--with-cef`/`--cef-version`）；Debug 规则改写为按目录；包名补 `-debug`；新增"SDK 使用契约"（包含路径、链接列表、运行期依赖） |
| [docs/Release.en.md](../Release.en.md) | 同上（英文） |
| [build/README.md](../../build/README.md) | 发布打包小节补 `--config` 与第三方库打包说明 |
| [build/README.en.md](../../build/README.en.md) | 同上（英文） |
| [docs/misc/Release-Packaging-Notes.md](Release-Packaging-Notes.md) | 契约表与 CLI 段更新；原文保留为历史并加日期注记；补 Skia/SDL 决策 |
| [docs/misc/Release-Packaging-Notes.en.md](Release-Packaging-Notes.en.md) | 同上（英文） |
| [docs/misc/Development-Plan.md](Development-Plan.md)（B-3 记录） | 只加"2026-10-21 CLI 与第三方库变更"注记，不改写历史 |
| [CMake-Improvement-Plan.md](CMake-Improvement-Plan.md) | P4 状态与实施记录 |
| [CM-5-Build-Artifacts-Inventory.md](CM-5-Build-Artifacts-Inventory.md) | 3.1 节更新为 P4 后规则；补 B3 结论 |

---

## 六、本次不做的事

1. 不改库的输出目录、文件名、构建脚本（属 P1/P3/P5）。
2. 不改 `bin/` 布局与 exe 命名（I-2 为有意例外）。
3. 不改头文件、`resources/`、`licenses/` 的既有收集规则（只**增加** SDL3 / CEF / libjpeg-turbo 头文件；许可证已由根 `licenses/` 覆盖）。
4. 不打包 Skia 头文件（`RenderSkia/` 不对外）；CEF 头文件仅在 `--with-cef` 时打包；CEF 运行库始终外置。
5. 不做 CPack / `install()`。
6. 不清理旧产物（`lib/x64/*.lib`、`lib/Debug/`）；P4 后不会被默认收集，清理另行确认。

---

## 七、验收标准

> 本机当前只有 `lib/x64/Debug` 与 `lib/x86/Debug/duilib-zlib.lib`，没有 Release 库。因此 P4 开发期用 Debug 目录做功能验证，**Release 的最终验收由用户构建后执行**。

| # | 命令 / 操作 | 期望 |
| :--- | :--- | :--- |
| 1 | `python build/pack.py --list` | 目录不存在 → 退出码 1，提示 `lib/x64/Release` 与构建命令 |
| 2 | `python build/pack.py --config Debug --list` | 命中 5 个 `duilib-*` 库；含 10 个 Skia Debug 库、`WebView2Loader*.lib`、`turbojpeg-static.lib`；**不含 SDL**（Windows 默认关闭）；不出现 `_d.lib` / `.pdb` |
| 3 | `python build/pack.py --config debug --list` | 同上（大小写不敏感） |
| 4 | `python build/pack.py --lib-dir lib/x86/Debug --list` | 只命中 `duilib-zlib.lib`；告警缺其余库；主库缺失 → 退出码 1 |
| 5 | `python build/pack.py --config Debug --skia-lib-dir <指定目录> --list` | 使用外部指定的 Skia 目录，不再自动探测 |
| 6 | `python build/pack.py --config Release --with-examples --list`（用户构建 Release 后） | 只含 Release 库；不出现 `*_d.exe`；头文件/资源数量与旧版一致 |
| 7 | `python build/pack.py --config Debug --with-examples --list` | 含全部 exe，并输出"可能混合配置"告警 |
| 8 | 实际打包（Debug） | 包名带 `-debug`；MANIFEST 含配置、Skia/WebView2/TurboJPEG（及可选的 SDL/CEF）清单与使用契约；`SHA256SUMS` 可校验 |
| 9 | **最小消费工程链接测试** | 在仓库外临时目录建最小工程，`include += <包>/include`、`lib += <包>/lib`，链接 3.9 的完整列表（含 Skia、WebView2 loader、turbojpeg），**成功产出 exe** |
| 10 | **运行测试** | 将 `basic_d.exe`（或 Release 版）与 `resources/` 放入包根；若 Unix/`--with-sdl`，SDL3 动态库可被找到；程序能启动并正常显示窗口（用户手动确认） |
| 11 | Unix（待环境） | 包内含 `lib/libduilib*.a`、`lib/libskia*.a` 等、`lib/libSDL3.so*`、`include/SDL3/` |
| 12 | `python build/pack.py --config Debug --with-sdl --list` | 额外包含 `SDL3-static.lib` 与 `include/SDL3/`；缺少 SDL3 目录 → 退出码 1 |
| 13 | `python build/pack.py --config Debug --with-cef --list` | 包含 CEF 头文件、`libcef.lib`、`libcef_dll_wrapper.lib`；MANIFEST 标注运行库外置 |
| 14 | `python build/pack.py --with-cef --cef-version 109 --list` | 使用 `libcef_win_109` 头文件与 `libcef_dll_wrapper_109.lib` |
| 15 | `--list` 架构一致性 | `--arch x86` 时 WebView2 / TurboJPEG / libcef 均取 x86（Win32）目录，包名架构为 `x86` |

---

## 八、风险与兼容性

| 风险 | 影响 | 处理 |
| :--- | :--- | :--- |
| 移除 `--allow-debug-libs` | 旧命令失效 | 仓库内部工具，文档同步；旧命令得到 argparse 明确报错 |
| 目录不存在时直接失败 | 旧流程"打空包"变为报错 | 期望行为：避免把迁移前旧产物误打进包 |
| `--config Debug` 包被误当 Release | 下游 CRT 不匹配 | 包名加 `-debug`、MANIFEST 标注、CLI 默认 Release |
| Unix 收集规则变化 | 首次真正收集 `.a` | 前缀归一化；用 `--list` 核对 |
| Skia/SDL 目录外置（仓库同级） | 打包依赖本机目录布局 | 提供 `--skia-lib-dir` / `--sdl-dir` 显式指定；缺失时报错并给出候选路径 |
| Unix SDL 为动态库 | 运行期找不到库 | 包内带上、MANIFEST 说明 `LD_LIBRARY_PATH`；后续可考虑静态 SDL |
| TurboJPEG 只有 release 版静态库 | Debug 下游若 CRT/宏与构建时不一致，可能 LNK2038 | MANIFEST 提示：仅使用同配置构建的 SDK；VS 默认 `JpegTurboEnabled=1`，该库必须随包 |
| CEF 仅带开发包 | 下游缺运行库无法启动 | `--with-cef` 的 MANIFEST 明确列出需外部准备的 `libcef.dll`、`*.pak`、`icudtl.dat`、`locales/` |
| 包体积大幅增加 | Debug 包含 247 MB 的 `duilib.lib` + Skia + SDL | 属预期；`--with-cef` 仍默认关闭 |
| SDK 构建选项（`DUILIB_MD`/SDL/CEF/WebView2/JPEG）无法从产物推断 | 下游宏或 CRT 不一致 | MANIFEST 记录本次实际打包的第三方库，并给出宏提示；完整方案见 CM-22（install/export） |

---

## 九、已确认决策（2026-10-21）

| 编号 | 决策 |
| :--- | :--- |
| **Q1** | 目录不存在 / 主库缺失 → 直接失败（退出码 1） |
| **Q2** | 删除 `--allow-debug-libs`，改用 `--config Debug` |
| **Q3** | 新增 `--arch`，默认主机架构 |
| **Q4** | Debug 包名追加 `-debug`，MANIFEST 标注 |
| **Q5** | P4-B1（`.a` 被排除）与 P4-B2（`lib` 前缀）纳入 P4 修复 |
| **Q6** | Skia 整目录全打包（10 个），不只打 `DUILIB_SKIA_LIBS` 的 6 个 |
| **Q7** | 打包 SDL3 头文件（`include/SDL3/`）与许可证 |
| **Q8** | Unix 的 `libSDL3.so*` 动态库随包分发 |
| **Q9** | MANIFEST 固化"包含路径 + 链接列表 + 运行期依赖"的使用契约（3.9） |
| **Q10** | Windows 的 SDL 做成可选项 `--with-sdl`，**默认关闭**；Unix 始终打包 |
| **Q11** | WebView2 的两种 loader **都默认随包**（按 `--arch` 取目录） |
| **Q12** | `turbojpeg-static.lib` 与头文件 **默认随包**（Windows） |
| **Q13** | CEF 改为 `--with-cef` 一次带齐头文件 + `libcef.lib` + wrapper，并新增 `--cef-version`（默认 `latest`，可选 `109`） |
| **Q14** | giflib / stb_image / nanosvg / pugixml / convert_utf / libudis86 不单独打包；`libpag`（无二进制）与不存在的 `apng` 不处理 |

---

## 十、实施记录（2026-10-21）

代码与文档已按 Q1~Q14 完成：

1. `build/pack.py`：目录解析、白名单、Debug 语义、Skia/SDL/WebView2/TurboJPEG/CEF 收集、
   MANIFEST 使用契约、CLI 全部落地；删除 `default_lib_dirs()`、`--allow-debug-libs`、
   `--with-cef-libs` 与 `_d` 判定。
2. 自测（Debug，Windows）：`--list` 命中 5 个 duilib 库、10 个 Skia Debug 库、
   2 个 WebView2 loader、`turbojpeg-static.lib` + 5 个头文件，未含 SDL；
   `--with-sdl` 增加 79 个文件；`--with-cef` 增加 288 个文件；
   `--arch x86 --lib-dir lib/x64/Debug --skia-lib-dir ...` 分支正常；
   `--config Release` 在 Release 目录缺失时退出码 1；
   实际生成 Debug zip（606 个收集文件 + 3 个元数据文件），包名带 `-debug`。
3. 文档同步：`docs/Release.md/.en.md`、`build/README.md/.en.md`、
   `docs/misc/Release-Packaging-Notes.md/.en.md`、`Development-Plan.md`、
   `CMake-Improvement-Plan.md`、`CM-5-Build-Artifacts-Inventory.md`。

验收：**已完成（用户确认，2026-10-21）**——Release 包验收、最小消费工程链接测试与运行测试均已确认通过。构建系统（含四平台打包、Release 产物）的**系统性测试后续单独形成计划**，不在本方案内展开。
