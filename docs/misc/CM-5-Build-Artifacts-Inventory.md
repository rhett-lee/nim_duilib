# CM-5 前置整理：构建产物与静态库使用现状

> 本文档为 **CM-5**（多配置生成器下 `DUILIB_BUILD_TYPE` 恒为 release / Debug 与 Release 产物同名）的**现状盘点**，只做事实整理，不含实施改动。
> 盘点环境：Windows 10 + VS 2022 (v143) + CMake 4.3.0 + x64；仓库 `c:\develop\nim_duilib`，Skia `c:\develop\skia`，SDL3 `c:\develop\SDL3`。
> 盘点日期：2026-10-21。

---

## 一、两套并行的构建体系

| 体系 | 入口 | 目标范围 | 输出根 | 配置方式 |
| :--- | :--- | :--- | :--- | :--- |
| **CMake**（逐目标独立配置） | `build/msvc_build.bat`、`build/linux_build.sh`、`build/macos_build.sh`、`build/freebsd_build.sh`、`build/msys2_build.sh`、`build/{llvm,gcc}-mingw-w64_build.bat` | 每个第三方库、`duilib`、每个示例**各自一个 build 目录**（无顶层 CMakeLists） | `lib/`（库）、`bin/`（exe） | 脚本固定 `-DCMAKE_BUILD_TYPE=Release` |
| **Visual Studio 解决方案** | `build/duilib.sln`、`examples.sln`、`*_no_cef.sln` | 第三方库 vcxproj + `duilib.vcxproj` + 23 个示例 vcxproj | `lib/$(Platform)`（库）、`bin/`（exe） | VS 多配置：`Debug`/`Release` × `Win32`/`x64` |

两套体系**互不复用产物**，各自产出同名不同路径的库文件；`lib/x64` 与 `lib/Debug` 的并存即由此而来。

---

## 二、产物现状矩阵（Windows / x64，实测）

### 2.1 静态库

| 库 | 谁产出 | 实际路径 | 实际文件名 | 配置覆盖 | CRT 标识（实测） |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Skia（`skia/svg/skshaper/skottie/sksg/jsonreader`） | gn + ninja（clang） | `skia/out/llvm.x64.release`、`skia/out/llvm.x64.debug` | `skia.lib` 等，**无后缀** | **Debug + Release 都有**（Windows） | release→`libcmt`；debug→`libcmtd` |
| SDL3 | 预编译，放在 `SDL3/lib` | `SDL3/lib/SDL3-static.lib` | `SDL3-static.lib` | **仅 Release** | `libcmt` |
| zlib | CMake | `lib/Debug/duilib-zlib.lib` | `duilib-zlib` | 仅 Debug（本机）；Release 会进 `lib/Release/` | MT/MTd 由 `DUILIB_MD` 决定 |
| zlib | VS vcxproj | `lib/x64/zlib.lib`、`zlib_d.lib` | `zlib` / `zlib_d` | Debug + Release | 由属性表决定（默认 MT/MTd） |
| libpng | CMake | `lib/Debug/duilib-png.lib` | `duilib-png` | 同上 | 同上 |
| libpng | VS vcxproj | `lib/x64/libpng.lib`、`libpng_d.lib` | `libpng` / `libpng_d` | Debug + Release | 同上 |
| libwebp | CMake | `lib/Debug/duilib-webp.lib` | `duilib-webp` | 同上 | 同上 |
| libwebp | VS vcxproj | `lib/x64/libwebp.lib`、`libwebp_d.lib` | `libwebp` / `libwebp_d` | Debug + Release | 同上 |
| cximage | CMake | `lib/Debug/duilib-cximage.lib` | `duilib-cximage` | 同上 | 同上 |
| cximage | VS vcxproj | `lib/x64/cximage.lib`、`cximage_d.lib` | `cximage` / `cximage_d` | Debug + Release | 同上 |
| CEF wrapper | CMake（`third_party/libcef/libcef_win`） | `lib/<Config>/libcef_dll_wrapper.lib` | `libcef_dll_wrapper`，**无后缀** | 按需构建 | 跟随 `DUILIB_MD` |
| CEF wrapper | VS vcxproj | `lib/x64/libcef_dll_wrapper.lib`、`_d.lib`（109 版同样） | 有 `_d` | Debug + Release | 属性表 |
| libcef（导入库） | CEF 二进制分发包 | `duilib/third_party/libcef/libcef_win/lib/x64/libcef.lib` | `libcef.lib` | 仅一份 | 无 CRT 依赖（C API） |
| libjpeg-turbo | 预编译（**当前两套构建均未链接**） | `third_party/libjpeg-turbo/libjpeg-turbo6-win-vc-x64/lib/turbojpeg-static.lib` | `turbojpeg-static` | **仅 Release** | `libcmt` |
| libpag | 预编译（当前**无库文件**） | `third_party/libpag/windows/lib-vc-x64`（空） | `libpag.lib` | 未启用 | — |
| WebView2 | 预编译 | `third_party/Microsoft.Web.WebView2/build/native/x64` | CMake：`WebView2Loader.dll.lib`；VS：`WebView2LoaderStatic.lib` | 各一份 | — |

### 2.2 `nim_duilib` 库本身

| 产出方 | 路径 | 文件名 | 配置 | 备注 |
| :--- | :--- | :--- | :--- | :--- |
| CMake | `lib/Debug/duilib.lib`（Release 为 `lib/Release/duilib.lib`） | **无 `_d`** | Debug 已实测存在（2026/10/20 22:23） | `CMAKE_ARCHIVE_OUTPUT_DIRECTORY = <root>/lib/`，多配置生成器自动加配置子目录 |
| VS | `lib/x64/duilib.lib`、`duilib_d.lib` | 有 `_d` | Debug / Release 都存在 | 由 `\[CommonSettings.props\]` 的 `TargetName=$(ProjectName)_d` 决定 |
| VS（DLL 模式） | `bin/duilib.dll` / `duilib_d.dll` | — | 未启用 | 由 `msvc/PropertySheets/duilib_dll.flag` 控制，当前**该文件不存在** → 静态库模式 |

### 2.3 可执行程序

| 产出方 | 输出目录 | 文件名 | 配置 | 实测 |
| :--- | :--- | :--- | :--- | :--- |
| CMake | `<root>/bin/`（**不按配置分目录**） | `basic.exe`（`TargetName=PROJECT_NAME`，**无 `_d`**） | Debug / Release **同名** | 生成的 `basic.vcxproj` 中 Debug 与 Release 的 `OutDir` 均为 `C:\develop\nim_duilib\bin\`，`TargetName` 均为 `basic` |
| VS | `<root>/bin/` | Release：`basic.exe`；Debug：`basic_d.exe` | 有 `_d` 区分 | `\[BinOutSettings.props\]` |

> 实测 `bin/` 当前只有 23 个 `*_d.exe`（VS Debug），以及 CMake 构建遗留的 `basic.pdb`（无 `_d`，来自 Debug 尝试，exe 因 LNK2038 未生成）。

---

## 三、构建脚本的配置固定情况

| 脚本 | 平台 | 固定构建类型 | Skia 子目录 |
| :--- | :--- | :--- | :--- |
| `build/msvc_build.bat` | Windows/MSVC | `Release` | `llvm.%CPU_ARCH%.release`（代码里有 Debug 分支，但 `DUILIB_BUILD_TYPE` 被固定为 Release，分支实际不生效） |
| `build/gcc-mingw-w64_build.bat` | Windows/MinGW-gcc | `Release` | `mingw64-gcc.<arch>.release` |
| `build/llvm-mingw-w64_build.bat` | Windows/MinGW-clang | `Release` | `mingw64-llvm.<arch>.release` |
| `build/msys2_build.sh` | Windows/MSYS2 | `Release` | `llvm.<arch>.release` 或 `gcc.<arch>.release` |
| `build/linux_build.sh` | Linux | `Release` | `llvm.<arch>.release`（默认）/ `gcc.<arch>.release` |
| `build/macos_build.sh` | macOS | `Release` | `llvm.<arch>.release` |
| `build/freebsd_build.sh` | FreeBSD | `Release` | `llvm.<arch>.release` |
| `build/build_duilib_all_in_one.bat` | Windows | Skia 生成 **x64/x86 × debug/release 四份**；nim_duilib 用 **VS Debug 解决方案**构建 | `llvm.<arch>.{debug,release}` |
| `build/build_duilib_all_in_one.sh` | Linux/macOS/FreeBSD | Skia **只生成 release**；SDL 也**只按 Release 编译** | `llvm.<arch>.release` |

**结论**：现状下除 Windows 的 `build_duilib_all_in_one.bat` 外，所有脚本都只走 Release；Debug 目前只有 VS 解决方案这一条可用路径。**按 D-3 的目标**，`nim_duilib` 在所有平台都要提供 Debug 配置，因此 Unix 侧脚本需放开配置限制（Skia/SDL 仍只保留 Release 构建，见第四节）。

### 3.1 pack.py 的现状（两套产物共用同一个打包入口）

| 维度 | 当前实现 |
| :--- | :--- |
| 候选库目录 | Windows：`lib/x64`、`lib/Debug`、`lib/Release`、`lib`；非 Windows：`lib`、`lib/Release`、`lib/Debug`（`default_lib_dirs()`） |
| 目录合并 | **合并所有存在的目录**，同名库由高优先级目录胜出（`collect_libs()`） |
| 库名白名单 | 同时列出两套命名：`duilib`、`duilib-cximage/png/webp/zlib`（CMake）与 `cximage`、`libpng`、`libwebp`、`zlib`（MSBuild） |
| Debug 处理 | 同名同时存在时 **Release 优先**；仅有 Debug 版的库默认排除，需 `--allow-debug-libs` 才纳入 |
| CEF wrapper | 仅在 `--with-cef-libs` 时收集（`libcef_dll_wrapper[_109]`） |
| 已知缺陷 | 第 93~94 行注释把两套命名**写反了**（实际 CMake 产 `lib/Release/duilib-zlib.lib`、MSBuild 产 `lib/x64/zlib.lib`） |

**因此**：当前 `pack.py` 能"同时吃下两套产物"，但无法判断某个库来自哪套体系；两套产物若停留在同一目录且同名（如改名前的 `duilib.lib`），只能靠目录优先级猜测。D-1 实施后 4 个第三方库名会统一，但 `duilib.lib` / `duilib_d.lib` 仍两套同名，这一歧义依然存在。

> **P4（2026-10-21）已按上表完成改造**：`pack.py` 只读取 `--config` 选中的唯一目录，删除多目录合并、双套白名单与 `_d` 判定；同时补入 Skia、WebView2、turbojpeg、SDL3、CEF 的链接库。详见 [CM-5 P4 打包方案](CM-5-P4-Pack-Plan.md)。

---

## 四、Debug 能力边界（按 D-3 决策）

| 组件 | Windows | Linux / macOS / FreeBSD |
| :--- | :--- | :--- |
| **Skia** | **不提供"真正的 Debug"**，但**必须编译两个变体**：`llvm.<arch>.debug` 与 `llvm.<arch>.release`。两者都是 release 模式代码（`is_debug=false`），仅运行库不同（`/MTd` vs `/MT`）。原因：Skia 是 C++ 库，Debug 目标若链接 `/MT` 版本会因运行库不匹配而链接失败 | **不提供 Debug，只编译一个 release 版本** |
| **SDL3** | 不提供 Debug，只有一个 release 版本 | 同左 |
| **nim_duilib（库与示例）** | **提供 Debug** | **提供 Debug**（链接 release 版 Skia/SDL；Unix 无 MSVC 的 CRT 标记机制，不产生 `LNK2038`） |

> 现状与目标的差距：Windows 已有 `llvm.x64.debug`，符合 D-3；Unix 侧 `build_duilib_all_in_one.sh` 目前的 Skia/SDL 只建 release，**这也符合 D-3**——需要补的是"允许 `nim_duilib` 自身在 Unix 上按 Debug 配置构建"，而不是补建 Skia/SDL 的 debug 版。

### 关键实测证据

1. **CMake 下 Debug 配置实际链接到 Release 版 Skia**：`duilib_bin_windows.cmake` 的链接目录生成结果为 `C:/develop/skia/out/llvm.x64.release`（`basic.vcxproj` 第 130 行），与 Debug 配置无关——因为 `duilib_common.cmake` 用 `CMAKE_BUILD_TYPE` 推导 `DUILIB_BUILD_TYPE`，而 VS 生成器下该变量为空。
2. **Debug 链接 Release 库的具体表现**：**CMake 配置**的 `examples/ChildWindow` Debug 构建（仅链接目录指向 release Skia）报 1694 个 `LNK2038`（`_ITERATOR_DEBUG_LEVEL` 0 vs 2、`RuntimeLibrary` `MT_StaticRelease` vs `MTd_StaticDebug`）+ `LNK1319`；同一示例用 **VS 解决方案 Debug** 构建（Debug Skia + Release SDL）成功产出 `bin/ChildWindow_d.exe`——说明这批错误来自 Skia 路径（I-1），SDL 的 Release 库可与 Debug 目标正常链接。
3. **CRT 标识扫描**（读取 `.lib` 内 `drectve` 段字符串）：
   - `skia/out/llvm.x64.debug/skia.lib` → `libcmtd / libcpmtd`
   - `skia/out/llvm.x64.release/skia.lib` → `libcmt / libcpmt`
   - `SDL3/lib/SDL3-static.lib` → `libcmt`（277 处）
   - `libjpeg-turbo/.../turbojpeg-static.lib` → `libcmt`
4. **CMake 多配置的输出目录行为**：
   - 库：`lib/Debug/`、`lib/Release/`（CMake 自动加配置子目录），文件名**不带 `_d`**；
   - 可执行程序：`bin/`（**不加配置子目录**），文件名**不带 `_d`**。
   - 生成的 `duilib.vcxproj`：Debug `OutDir=...\lib\Debug\`、`TargetName=duilib`；Release `OutDir=...\lib\Release\`、`TargetName=duilib`。

---

## 五、链接侧的两套命名约定

| 位置 | 约定 |
| :--- | :--- |
| CMake `cmake/duilib_common.cmake` | `DUILIB_LIBS = duilib duilib-cximage duilib-webp duilib-png duilib-zlib`（**全部无配置后缀**）；`DUILIB_SDL_LIBS = SDL3-static.lib`；`DUILIB_SKIA_LIBS = svg skshaper skottie sksg jsonreader skia` |
| CMake 链接目录 | `lib` + 自动追加的 `lib/$(Configuration)`（实测 `AdditionalLibraryDirectories` 含 `lib` 与 `lib/$(Configuration)`） |
| VS 属性表 `CommonSettings.props` | `DuilibThirdLibs`：Debug=`zlib_d.lib;cximage_d.lib;libpng_d.lib;libwebp_d.lib`，Release=无后缀；`DuilibMainLib`：`duilib_d.lib` / `duilib.lib` |
| VS 链接目录 | `lib\$(Platform)`（即 `lib/x64`，Debug/Release **同目录**，靠 `_d` 区分） |
| CEF | CMake：`libcef` + `libcef_dll_wrapper`（无 `_d`）；VS：`libcef.lib` + `libcef_dll_wrapper[_109][_d].lib` |
| WebView2 | CMake：`WebView2Loader.dll.lib` + 宏 `DUILIB_USE_WEBVIEW2_LOADER_DLL=1`；VS：`WebView2LoaderStatic.lib`（静态） |

> 注：上表为**现状**。VS 侧 4 个第三方库的名称将在 D-1 实施后变为 `duilib-*`（见 5.2）。

### 5.1 为什么 `duilib-` 前缀在 Linux / macOS / FreeBSD 上不能去掉

Unix 系统普遍预装 `libz`、`libpng`、`libwebp`，裸名链接存在三类"静默选错库"的路径：

1. **链接期回退**：`ld` 会先搜 `-L` 指定目录，但本地文件缺失/改名时会**静默回退**到 `/usr/lib`、`/usr/local/lib` 的系统库，不报错。前缀让"本地库不存在"变成 `cannot find -lduilib-zlib` 的硬错误。
2. **同目录内的 `.so` 优先**：若本地同时存在 `libz.so` 与 `libz.a`，`ld` 默认选 `.so`；且生成的 `NEEDED` 只记录 `libz.so.1`，**运行时仍会加载系统库**（除非额外做 rpath/`$ORIGIN` 或强制静态链接）。前缀使运行时依赖名也独有（`libduilib-zlib.so.1`）。
3. **CMake 的 `find_package` 绕过 `-L` 顺序**：`find_package(ZLIB)` 直接给出系统路径的 `ZLIB::ZLIB`，目标级链接完全不看 `-L` 顺序。libpng 已通过 CM-2 显式设置 `ZLIB_LIBRARY` 指向 `<root>/lib/.../libduilib-zlib.a` 规避，但这是"逐处修补"，不是通用保障。

**结论**：前缀必须保留在 Unix 侧。Windows 无系统同名库冲突，但为两套体系产物一致，**已确认（2026-10-21）把 VS 侧 `zlib`/`libpng`/`libwebp`/`cximage` 的输出名也改成 `duilib-*`**，详见 5.2 与第十节决策记录。

**可选加固**（与是否保留前缀无关，二者叠加最稳）：

- CMake 侧把 `${DUILIB_LIBS}` 的裸名改为 imported target 或绝对路径（`IMPORTED_LOCATION "${DUILIB_LIB_PATH}/libduilib-zlib.a"`），使"库不存在"在配置期就报错，`find_package` 也无法介入；
- 保持静态库（当前 Unix 侧即 `.a`），彻底消除运行时 soname 冲突。

### 5.2 命名统一决策（已确认，2026-10-21）

VS 侧 4 个第三方库的输出名改为与 CMake 一致的 `duilib-*`。**D-2（方案 B）确定后不再使用 `_d` 后缀**：同名库放在 `Debug` / `Release` 目录中区分（见 5.3）。

| 库 | 现状（VS 产物名） | 目标（VS 产物名，Debug 与 Release 同名） | 目标位置 |
| :--- | :--- | :--- | :--- |
| zlib | `zlib.lib` / `zlib_d.lib` | `duilib-zlib.lib` | `lib/$(Platform)/Debug|Release/` |
| libpng | `libpng.lib` / `libpng_d.lib` | `duilib-png.lib` | 同上 |
| libwebp | `libwebp.lib` / `libwebp_d.lib` | `duilib-webp.lib` | 同上 |
| cximage | `cximage.lib` / `cximage_d.lib` | `duilib-cximage.lib` | 同上 |

改名后 `CommonSettings.props` 的 `DuilibThirdLibs` 在两个配置下使用同一组名字：

```text
Debug   : duilib-zlib.lib;duilib-cximage.lib;duilib-png.lib;duilib-webp.lib
Release : duilib-zlib.lib;duilib-cximage.lib;duilib-png.lib;duilib-webp.lib
```

连带需要更新的位置（实施时逐项核对）：

| 文件 | 需要改动的内容 |
| :--- | :--- |
| `duilib/third_party/zlib/win32/zlib.vcxproj` | `Microsoft.Cpp.props` 之后设置常量 `<TargetName>duilib-zlib</TargetName>`（不再区分配置） |
| `duilib/third_party/libpng/projects/vstudio/libpng/libpng.vcxproj` | 同上（`duilib-png`） |
| `duilib/third_party/libwebp/libwebp.vcxproj` | 同上（`duilib-webp`） |
| `duilib/third_party/cximage/cximage.vcxproj` | 同上（`duilib-cximage`） |
| `msvc/PropertySheets/CommonSettings.props` | ① `OutDir` 改为 `..\lib\$(Platform)\$(Configuration)\`（第 8 行）；② 删除 `TargetName=$(ProjectName)_d` 的两个配置块（第 11~16 行，exe 由 `DuilibSettings.props` / `BinOutSettings.props` 自行覆盖，不受影响）；③ `DuilibThirdLibs` 两行（第 87、88 行）改为同一组名字 |
| `msvc/PropertySheets/README.md`、`README.en.md`、`readme_cn.md` | 产物清单里的库名与目录（各 4 行 + 路径说明） |
| `build/pack.py` | 改为只认一套布局（见 D-7）：候选目录 `lib/<arch>/Debug|Release`（Windows）、`lib/Debug|Release`（Unix）；`LIB_BASENAMES` 删除旧名；移除 `_d` 判定；第 93~94、331~332 行注释把 CMake 与 MSBuild 命名**写反了**，一并修正 |

**影响面**：下游 VS 项目若写死 `zlib.lib` / `libpng.lib` / `libwebp.lib` / `cximage.lib` 或路径 `lib\x64\duilib_d.lib`，会同时受改名与目录调整影响（用户已确认接受）。`duilib` 主库名不变（Debug 下由目录区分，不再产出 `duilib_d.lib`）。

### 5.3 统一布局决策（D-2，方案 B，2026-10-21）

**规则：配置只由目录表达，库文件名不再带 `_d`。**

| 构建体系 | Windows | Linux / macOS / FreeBSD |
| :--- | :--- | :--- |
| CMake | `lib/<arch>/Debug|Release/`（多配置自动追加配置层；单配置用 `CMAKE_*_OUTPUT_DIRECTORY_<CONFIG>` 显式指定） | `lib/Debug|Release/`（单配置同上） |
| VS / MSBuild | `lib/$(Platform)/$(Configuration)/`（`CommonSettings.props` 的 `OutDir`） | 不适用 |

- `<arch>`：CMake 侧 `x64` / `x86`；VS 侧 `x64` / `Win32`，实施时增加一个平台名映射（`Win32 → x86`），使两套体系落到同一目录。
- **exe 保持现状**：VS 仍输出 `bin\` + `_d`（`DuilibSettings.props` / `BinOutSettings.props` 未改）；CMake 的 exe 输出沿用现状。这是有意的例外——exe 不是链接输入，迁移成本高于收益。
- CEF wrapper 也进入配置目录，且**不再加 `_d`**（与 D-5 的名称不变部分不冲突；D-5 只约束前缀）。

---

## 六、功能特性上的差异（与产物强相关）

| 特性 | CMake | VS |
| :--- | :--- | :--- |
| libjpeg-turbo | **不参与构建、不链接**（无 `DUILIB_JPEG_TURBO` 宏） | **同样未链接**：`JpegTurboSettings.props` 虽默认 `JpegTurboEnabled=1`，但**没有任何 vcxproj 导入该属性表**（全仓库仅有 3 份 README 与本属性表自身提到它），因此 `DUILIB_JPEG_TURBO` 宏与 `turbojpeg-static.lib` 均不生效；只有项目显式导入后才启用 |
| libpag | 不参与 | **未链接**：`LibPagSettings.props` 默认 `LibPagEnabled=0`，且**没有任何 vcxproj 导入该属性表**；同时 `third_party/libpag/windows/lib-vc-x64` 下无库文件 |
| WebView2 | 动态 Loader（复制 `WebView2Loader.dll` 到输出目录） | 静态 Loader |
| SDL（Windows） | `DUILIB_ENABLE_SDL` 默认 `OFF` | `SDLEnabled=1`（默认开启） |
| CEF（Windows） | `DUILIB_ENABLE_CEF` 默认 `ON`（库层），示例按需 | `LibCefEnabled=1` |
| MSVC 运行库 | `DUILIB_MD` 开关（默认 MT/MTd） | `duilib_dll.flag` 是否存在（不存在→MT/MTd） |

---

## 七、冲突与风险清单（供方案设计）

| 编号 | 问题 | 触发条件 | 后果 |
| :--- | :--- | :--- | :--- |
| **I-1** | CMake 的 Skia 路径由 `CMAKE_BUILD_TYPE` 推导，VS 生成器下为空 → 恒为 `release` | 用 VS/Ninja Multi-Config 生成器直接构建 Debug | Debug 目标链接 Release Skia，`LNK2038`（CM-5 主体问题）。**已解决（P2，2026-10-21）**：Windows 用 `$<IF:$<CONFIG:Debug>,debug,release>` 按配置选择，Unix 固定 release；VS 生成器 Debug 可直接构建 |
| **I-2** | CMake 可执行程序 `bin/<name>` 不区分配置、无 `_d` | 单配置生成器下分别构建 Debug 与 Release | 后构建者覆盖前者。**D-2 明确 exe 保持现状**，本项为已知遗留，不在 CM-5 范围（VS 用 `_d` 区分，不受影响） |
| **I-3** | CMake 库输出到 `lib/<Config>/` 且无 `_d`；VS 输出到 `lib/x64/` 且有 `_d` | 两套体系混用同一 `lib/` 根 | **已定（D-2，方案 B）解决**：两套统一为 `lib/<arch>/<Configuration>/`，库名不再带 `_d`。CMake 侧已实施（P1）；VS 侧已实施（P3，2026-10-21） |
| **I-4** | 第三方库命名两套并存（`duilib-zlib` vs `zlib[_d]`） | 修改任一命名 | 需同步 `duilib_common.cmake`（`DUILIB_LIBS`）与 `CommonSettings.props`（`DuilibThirdLibs`），遗漏即链接失败。**已解决（P1 + P3，2026-10-21）**：两套均使用 `duilib-*` 且无 `_d`，`DuilibThirdLibs` 已同步 |
| **I-5** | SDL3 只有 Release 版（不提供 Debug 版） | Windows Debug 构建启用 SDL 的目标 | **不是阻塞项**：实测 VS Debug 已正常链接并产出 `bin/ChildWindow_d.exe`。**已定（D-4b）：保持现状，不补 Debug 版** |
| **I-6** | libjpeg-turbo 只有 Release 预编译库（`/MT`），且当前无项目导入其属性表 | 项目显式导入 `JpegTurboSettings.props` 后做 Debug 构建 | 链接该 Release 静态库会 `LNK2038`。**已定（D-4a）：不提供 Debug 版，保持现状**；即 Debug 下不启用该特性 |
| **I-7** | 非 Windows 平台 Skia 仅 Release | 在这些平台构建 Debug | **按 D-3 不是阻塞项**：`nim_duilib` 提供 Debug，直接链接 release 版 Skia/SDL（Unix 无 MSVC CRT 标记机制）。需要放开脚本里固定 `Release` 的限制 |
| **I-8** | 单配置生成器（Ninja/Makefile）下 Debug/Release 都写 `lib/` 根 | Linux/macOS/FreeBSD 分别构建两种配置 | 同名库互相覆盖。**已定（D-2，方案 B）解决**：单配置显式设置 `CMAKE_*_OUTPUT_DIRECTORY_<CONFIG>`，落到 `lib/Debug`、`lib/Release`。**已实施（P1，2026-10-21）** |

---

## 八、方案设计前需要确认的决策点

1. **目录与 `_d` 策略：已定（D-2，2026-10-21，方案 B）**
   - 统一为 **CMake 风格**：`lib/<arch>/<Configuration>/`（Windows）/ `lib/<Configuration>/`（Unix），库文件名不带 `_d`；
   - VS 侧改 `CommonSettings.props` 的 `OutDir` 并移除 `TargetName` 的 `_d`；`DuilibThirdLibs` 两配置使用同一组名字；
   - **exe 保持现状**（VS `bin\` + `_d`；CMake 沿用现状），作为有意例外；
   - 详细布局与连带改动见 5.2 / 5.3。
2. **Debug 的适用范围（已定 D-3，2026-10-21）**：
   - **Skia**：Windows 下不提供"真正的 Debug"，但**必须同时编译 debug / release 两个变体**（两者均为 release 模式代码，仅 CRT 不同：`/MTd` vs `/MT`），否则 Debug 目标链接失败；Linux/macOS/FreeBSD 只编译一个 release 版本；
   - **SDL**：所有系统都只编译一个 release 版本（见 D-4b）；
   - **nim_duilib（库与示例）**：Windows / Linux / macOS / FreeBSD **均提供 Debug**。
3. **SDL 的 Debug 版本**：**已定（D-4b，2026-10-21）不提供**，只保留现有 Release 版（Windows `SDL3-static.lib`，Unix 侧现有 `.so`/`.dylib`），保持现状；Debug 构建直接使用该 Release 库（VS Debug 实测可行）。
   **libjpeg-turbo 已定（D-4a，2026-10-21）**：不提供 Debug 版，只保留现有 Release 预编译库，保持现状；Debug 构建下不启用该特性。
4. **libcef wrapper 的命名**：**已定（D-5，2026-10-21）不纳入 `duilib-*` 前缀，保持名称 `libcef_dll_wrapper[_109]`**。各平台 `libcef_*/CMakeLists.txt` 只保留输出目录定制（按 D-2 落到 `lib/<Configuration>/`）；`_d` 后缀随 D-2 取消；CEF 导入库 `libcef.lib` 的链接名不变。
5. **是否保留两套体系**：**已定（D-6，2026-10-21）长期并存**。保留 VS 解决方案的原因是大量 Windows 开发者习惯 VS 而非 CMake；CM-22 的顶层聚合只服务于 CMake 入口，不替代 VS 工程。
6. **两套并存下的 pack 方案：已定（D-7，2026-10-21，方案 C）**——布局统一后 `pack.py` 只认一套规则：按 `lib/<arch>/<Configuration>/` 或 `lib/<Configuration>/` 取库，默认 Release；删除双套命名白名单、`_d` 判定与目录优先级猜测。现状分析见 3.1。

---

## 九、相关文件索引

- 构建参数与库名：[cmake/duilib_common.cmake](../../cmake/duilib_common.cmake)
- 示例可执行程序链接：[cmake/duilib_bin_windows.cmake](../../cmake/duilib_bin_windows.cmake)
- duilib 库输出目录/源码：[duilib/CMakeLists.txt](../../duilib/CMakeLists.txt)
- VS 公共属性：[msvc/PropertySheets/CommonSettings.props](../../msvc/PropertySheets/CommonSettings.props)、[SkiaSettings.props](../../msvc/PropertySheets/SkiaSettings.props)、[SDLSettings.props](../../msvc/PropertySheets/SDLSettings.props)、[CEFSettings.props](../../msvc/PropertySheets/CEFSettings.props)、[JpegTurboSettings.props](../../msvc/PropertySheets/JpegTurboSettings.props)、[WebView2Settings.props](../../msvc/PropertySheets/WebView2Settings.props)、[BinOutSettings.props](../../msvc/PropertySheets/BinOutSettings.props)
- 构建脚本：[build/msvc_build.bat](../../build/msvc_build.bat)、[build/build_duilib_all_in_one.bat](../../build/build_duilib_all_in_one.bat)、[build/build_duilib_all_in_one.sh](../../build/build_duilib_all_in_one.sh)
- 跟踪文档：[CMake-Improvement-Plan.md](CMake-Improvement-Plan.md)

---

## 十、已确认决策记录

| 编号 | 日期 | 决策 | 落点 |
| :--- | :--- | :--- | :--- |
| **D-1** | 2026-10-21 | 第三方静态库统一使用 `duilib-` 前缀：Unix 侧保持不变（前缀不可取消，理由见 5.1）；**VS 侧 4 个库的 `TargetName` 改为 `duilib-zlib` / `duilib-png` / `duilib-webp` / `duilib-cximage`**；同步更新 `DuilibThirdLibs`。`duilib` 主库名不改。接受下游写死旧库名的兼容性影响。**Debug 不再用 `_d` 后缀（由 D-2 的目录区分）**。 | 5.2 节（含连带改动清单）；实施归入 CM-5 |
| **D-2** | 2026-10-21 | **布局采用方案 B（CMake 风格）**：库输出统一为 `lib/<arch>/<Configuration>/`（Windows，arch 为 x64/x86）与 `lib/<Configuration>/`（Linux/macOS/FreeBSD），**文件名不再带 `_d`**。CMake 多配置天然如此，单配置用 `CMAKE_*_OUTPUT_DIRECTORY_<CONFIG>`；VS 改 `CommonSettings.props` 的 `OutDir` 并移除 `TargetName` 的 `_d`（库工程覆盖，exe 仍用 `DuilibSettings.props` / `BinOutSettings.props` 的 `bin\` + `_d`）。 | 5.3 节；解决 I-3 / I-8（I-2 作为 exe 例外保留） |
| **D-7** | 2026-10-21 | **pack 采用方案 C（只认一套布局）**：按 D-2 的配置目录取库，默认 Release；删除双套命名白名单、`_d` 判定与目录优先级猜测，并修正 `pack.py` 中写反的注释。 | 3.1 节；实施归入 CM-5 |
| **D-3** | 2026-10-21 | **Debug 能力边界**：① Skia——Windows 不提供真正的 Debug，但必须同时编译 debug/release 两个变体（均为 release 模式代码，仅 CRT 不同：`/MTd` vs `/MT`）；Linux/macOS/FreeBSD 只编译一个 release 版。② SDL——所有系统只编译一个 release 版。③ nim_duilib（库与示例）——四个平台均提供 Debug。 | 第四节；Unix 侧需放开脚本的固定 `Release` 限制（Skia/SDL 仍只建 release） |
| **D-6** | 2026-10-21 | **CMake 与 VS 解决方案长期并存**（保留 VS 是因为大量 Windows 开发者习惯用 VS，不习惯 CMake）。CM-22 顶层聚合只覆盖 CMake 入口，不替代 VS 工程；两套体系需在产物命名（D-1）与 Debug 边界（D-3）上保持一致。 | 第八节决策点 5；pack 方案另见 D-7 |
| **D-5** | 2026-10-21 | `libcef_dll_wrapper` **不纳入** `duilib-*` 前缀，名称保持 `libcef_dll_wrapper[_109]`。`_d` 后缀随 D-2 取消，产物进入配置目录；各平台 `libcef_*/CMakeLists.txt` 只保留输出目录定制。 | CM-21 遗留项关闭；`duilib/third_party/README-duilib.md` 同步 |
| **D-4a** | 2026-10-21 | **libjpeg-turbo 不提供 Debug 版本**，只保留现有 Release 预编译库（`turbojpeg-static.lib`，`/MT`），保持现状；Debug 构建下不启用该特性，也不补建 Debug 版。 | 第七节 I-6 定性关闭；不涉及代码改动 |
| **D-4b** | 2026-10-21 | **SDL 不提供 Debug 版本**，只保留现有 Release 版（Windows `SDL3-static.lib`；Unix 侧现有 `.so`/`.dylib`），保持现状；Debug 构建直接链接该 Release 库（VS Debug 实测可行）。各构建脚本不新增 SDL Debug 构建。 | 第七节 I-5 定性关闭；不涉及代码改动 |

**决策状态：D-1 ~ D-7 已全部确认（2026-10-21），无待决策项。** 下一步为 CM-5 的分阶段实施（方案见 [CMake-Improvement-Plan.md](CMake-Improvement-Plan.md) 的 CM-5 小节）。
