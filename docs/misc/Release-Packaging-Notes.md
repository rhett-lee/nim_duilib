# 发布打包工作记录（B-3）

本文档是 B-3「发布与打包流程」的**工作记录**：记录已产出的成果、做过的技术决策与依据、踩过的坑，以及后续跟进时需要知道的待办。

面向使用者的操作说明在 [发布与打包](../Release.md)。本文不重复那份内容，只记录**为什么这么做**和**还没做完什么**。

> 状态：功能已实现并通过 Windows 实测，**当前工作区未提交**（2026-10-20 决定暂缓提交，牵扯内容较多，待后续跟进）。下文「当前工作区状态」一节列出了提交时的完整文件清单。

## 一、当前工作区状态

改动**全部停留在工作区**，最后一次提交是 `5c836967`（FreeBSD 明确不支持 CEF）。

### 应当提交的文件

| 文件 | 类型 | 内容 |
| :--- | :--- | :--- |
| `VERSION` | 新增 | 版本号单一事实源，四段式 `1.0.2026.0416` |
| `build/pack.py` | 新增 | 打包脚本，纯标准库、跨平台、不参与编译 |
| `docs/Release.md` / `docs/Release.en.md` | 新增 | 面向使用者的打包与发布说明 |
| `docs/misc/Release-Packaging-Notes.md` / `.en.md` | 新增 | 本文档 |
| `docs/Summary.md` / `Summary.en.md` | 修改 | 索引登记 |
| `build/README.md` / `README.en.md` | 修改 | 新增「发布打包」章节 |
| `docs/misc/Development-Plan.md` | 修改 | B-3 条目改为已完成并指向本文 |
| `.gitignore` | 修改 | 新增 `/dist` |

### 不应提交的临时脚本

`build/` 目录下这一批是过程性脚本，**按项目纪律不纳入提交**：

```
add_dist_ignore.py          apply_a1_a2.py              apply_a4.py
build_a1a2_check.bat        build_a4_check.bat          build_chart_debug.bat
build_tabctrl_debug.bat     dedup_summary.py            fix_b3_spacing.py
fix_cef_comment.py          fix_spacing.py              fix_spacing2.py
mark_a1_a2_done.py          mark_a3_deferred.py         mark_a4_done.py
mark_b3_done.py             mark_b_deferred.py          register_release_docs.py
smoke_controls.bat          update_dev_plan.py          verify_cef_guard.py
verify_cef_macro.py         verify_overflow.py          verify_tabctrl_fixed.py
write_release_docs.py
```

`build/pack.py` 是**唯一**要提交的 `build/` 下新增文件（其余 `.py` / `.bat` 均为过程脚本）。

## 二、成果清单

### 产物契约

`pack.py` 强制的规则，全部有明确依据：

| 项 | 规则 | 依据 |
| :--- | :--- | :--- |
| 头文件路径 | 必须是 `include/duilib/xxx.h`，**不可压平** | 下游写法是 `#include "duilib/duilib.h"`（`examples/basic/MainForm.h:5`），压平则下游全部编译失败 |
| `RenderSkia/` | 不对外暴露 | `duilib.h` 不引用它，是渲染后端实现细节 |
| `third_party` 转发 | zlib / libpng / giflib / Microsoft.Web.WebView2 | 有 5 个对外头文件直接 include 了它们，不转发下游编译失败 |
| zlib / libpng | 整目录转发头文件，不逐文件筛选 | 存在条件分支的传递依赖，白名单会漏（见第四节） |
| Debug 库 | **默认排除**，需 `--allow-debug-libs` | Debug 库带调试 CRT 与 `/MTd`，混进 Release SDK 会让下游链接异常 |
| CEF wrapper 库 | 默认不打，需 `--with-cef-libs` | 单个 80–100 MB |
| `bin/resources.zip` | 一律排除 | 仅 748 字节的空壳文件 |
| 版本号 | 单一事实源为根目录 `VERSION` | 包名、包内 `VERSION`、`MANIFEST.txt` 全部由它派生 |

### `--with-examples` 的资源范围差异

这是打包时最容易搞错的一点。`resources/themes/default/` 下同时放着公共资源和各示例的私有主题目录：

| 模式 | `resources/` 收录范围 |
| :--- | :--- |
| 默认（SDK） | 仅 `themes/default/public/` 与 `themes/default/global.xml` |
| `--with-examples` | 整个 `resources/`，无排除项 |

`global.xml` 在 SDK 模式下**必须保留**。它是框架启动的必需文件，`GlobalManager.cpp:440` 在本地文件模式下找不到它会直接 `return false`，初始化直接失败。

SDK 模式下被排除的是 22 个示例私有主题目录：
`MultiLang`、`basic`、`cef`、`cef_browser`、`chat`、`child_window`、
`color_theme`、`controls`、`dpi_aware`、`layout`、`list_box`、`list_ctrl`、
`move_control`、`panel`、`render`、`rich_edit`、`threads`、`tree_view`、
`virtual_list_box`、`webview2`、`webview2_browser`、`xml_preview`。

### 命令行接口

```
python build/pack.py                      # SDK 包（不含示例）
python build/pack.py --with-examples      # SDK + 示例程序
python build/pack.py --with-cef-libs      # 额外带上 CEF wrapper 静态库
python build/pack.py --allow-debug-libs   # 允许打进仅有 Debug 版的库
python build/pack.py --list               # 只打印将收集的文件，不打包
python build/pack.py --format tgz         # 强制 tgz（默认 Windows 用 zip）
python build/pack.py --lib-dir lib/x64    # 指定库目录
python build/pack.py --out-dir dist       # 指定输出目录
```

产物输出到 `dist/`，包名格式 `nim_duilib-<版本>-<系统>-<架构>.zip`，内含 `VERSION`、`MANIFEST.txt`、`SHA256SUMS`、`LICENSE`、`include/duilib/`、`lib/`、`resources/`、`licenses/`。

## 三、关键技术决策与依据

### 1. 为什么不走 CPack / `install()`

原审计结论（零 CPack、零 `install()` 规则、零 NSIS/WiX/DEB/DMG）**仍然成立**，但**方案整体更换**。原因是架构不匹配，不是遗漏：

- **补 `install()` 规则 —— 无处可补**。本仓库**没有根 `CMakeLists.txt`**，`duilib/`、`examples/*/`、`duilib/third_party/*/` 各自独立 `project()`，由 `msvc_build.bat`、`linux_build.sh` 等脚本逐个 configure + build。`install()` 只能挂在单个项目里，没有任何单一 CMake 项目能覆盖全部产物。
- **CPack —— 需要顶层项目才能挂载**，同上。更关键的是 Windows 有两条构建路径（`build/*.sln` 共 36 个 `.vcxproj` 的 MSBuild 路径、`msvc_build.bat` 的 CMake 路径），产物混在同一 `lib/`、`bin/` 下，CPack 产物对 MSBuild 用户是空的。
- 因此改为「**独立打包脚本**」路线：与构建体系解耦，按文件名白名单收集，两条构建路径的产物都能收，也不需要新增顶层工程。

### 2. 头文件层级不可压平

包内头文件保持 `include/duilib/xxx.h` 的层级，与仓库根的相对路径一致。下游的引用写法是 `#include "duilib/duilib.h"`，因此 `include/` 下面必须还有一层 `duilib/`。

首版打包脚本把头文件压平为 `include/duilib.h`，下游立刻全部编译失败。

### 3. `third_party` 转发范围的判定链

这是本次工作中推导最细的一块，判定链路如下：

**第一步 —— 谁在引用。** 对外头文件（`duilib/` 下排除 `third_party/` 与 `RenderSkia/`）共 313 个（2026-10-20 实测）。逐个扫描 `#include`，只有 5 个引用了 `duilib/third_party/...`，共 9 条引用：

| 对外头文件 | 引用的第三方头文件 |
| :--- | :--- |
| `Core/ZipStreamIO.h:5` | `zlib/contrib/minizip/ioapi.h` |
| `Image/APngDecoder.h:5-9` | `zlib/zlib.h` + `libpng/{pngpriv,pngstruct,pnginfo,png}.h` |
| `Image/FrameSequence_gif.h:20` | `giflib/gif_lib.h` |
| `WebView2/WebView2Control.h:10` | `Microsoft.Web.WebView2/build/native/include/WebView2.h` |
| `WebView2/WebView2EnvironmentOptions.h:9` | 同上 |

关键在于这些 include 写的是 **`duilib/third_party/...` 全路径**（不是 `zlib.h` 这种裸名），所以包内必须原样落在 `include/duilib/third_party/<原路径>`。这与「头文件不可压平」是同一个约束的两面。

**第二步 —— 必需文件集。** 从那 5 个直接引用出发递归解析同目录 / 相对 include，得到最小闭包共 13 个文件：

```
zlib      必需 4：zlib.h → zconf.h；contrib/minizip/ioapi.h → ../../zlib.h, ints.h
libpng    必需 7：png.h → pnglibconf.h, pngconf.h
                  pngpriv.h → pnglibconf.h, png.h, pngstruct.h, pnginfo.h, pngdebug.h
                  pngstruct.h, pnginfo.h
giflib    必需 1：gif_lib.h（只依赖 <stdbool.h> / <stddef.h>）
WebView2  必需 1：WebView2.h
```

**第三步 —— 为什么 zlib / libpng 必须整树转发。** 这两个库的头文件之间是**同目录相对 include 链**，且带条件分支：

- `pngpriv.h:85` `#include "pngprefix.h"` —— 仅当 `PNG_PREFIX`
- `pngpriv.h:89` `#include "pngusr.h"` —— 仅当 `PNG_USER_CONFIG`
- `png.h:328` `#include "pnglibconf.h"` —— 仅当 `!PNGLCONF_H`
- `png.h:337` `#include "pngconf.h"` —— 仅当 `!PNG_VERSION_INFO_ONLY`
- `ioapi.h:74` `#include "mz64conf.h"` —— 仅当 `HAVE_MINIZIP64_CONF_H`（该文件本仓库根本没有）

按文件白名单转发，就得穷举所有宏组合；漏一个，下游换个编译开关就编译失败。整目录保形转发天然覆盖全部条件分支 —— **这才是「不现实，做整目录转发」的真正原因，不是文件数量问题**。

**第四步 —— giflib 与 WebView2 的特殊性。** `gif_lib.h` 只 include `<stdbool.h>` / `<stddef.h>`，无第三方相对依赖；`WebView2.h` 额外的 5 个依赖（`rpc.h` `rpcndr.h` `objidl.h` `oaidl.h` `EventToken.h`）**全部来自 Windows SDK**（`shared/` `um/` `winrt/`），不在 third_party 内，无需转发。WebView2 头文件位于 `third_party/Microsoft.Web.WebView2/build/native/include/`，路径深，所以需要多级路径分量元组。

**成本核算**（2026-10-20 实测）：

| 目录 | 整树转发的 .h 数 | 闭包中必需数 |
| :--- | :--- | :--- |
| zlib | 31 | 4 |
| libpng | 18 | 7 |
| giflib | 3 | 1 |
| Microsoft.Web.WebView2 | 1 | 1 |

多带 38 个头文件，全是几 KB 的文本文件，体积可忽略，换来不因宏组合或上游小版本升级而漏文件。

### 4. Debug 库判定：后缀 + 目录名双条件

Debug 来源有**两处**，只看一个都会漏：

- 文件名带 `_d` 后缀 —— CMake 路径的 `duilib_d.lib`
- 所在目录名为 `Debug` —— MSBuild 路径的 `lib/Debug/duilib.lib`（**无后缀**，只靠目录名区分，见 `msvc/PropertySheets/DuilibSettings.props`）

首版只认 `_d`，结果把 232 MB 的 Debug 库当成 Release 打进了包，压缩后体积从 4.8 MB 涨到 45.8 MB。现按双条件判定。

### 5. 版本号：单一事实源与已知缺口

版本号的单一事实源是仓库根的 `VERSION` 文件，格式为 `主版本.次版本.修订号.构建号`，例如 `1.0.2026.0416`。`pack.py` 从该文件读取版本，用于包名、包内 `VERSION` 与 `MANIFEST.txt`；格式非法（不是四段数字）时脚本报错退出，不会打出半成品包。

git tag 建议用 `v<版本号>` 格式。仓库现有 tag（`2026-06-08(main)` 等）是历史格式，含括号，不能直接用作包名或 CPack 版本号。

## 四、踩过的坑

| 坑 | 现象 | 根因 | 修法 |
| :--- | :--- | :--- | :--- |
| 头文件被压平 | 下游 `#include "duilib/duilib.h"` 全部找不到 | 首版脚本输出 `include/duilib.h` | 保持 `include/duilib/xxx.h` 层级 |
| Debug 库混入 | 包体积 4.8 MB → 45.8 MB | 只认 `_d` 后缀，漏了 MSBuild 的目录名判据 | 后缀 + 目录名双条件 |
| `resources.zip` 被打进 | 包里多一个 748 字节空壳 | 未显式排除 | 加入 `EXCLUDED_RESOURCE_FILES` |
| SDK 模式丢了 `global.xml` | 下游启动直接失败 | 资源排除时把 `global.xml` 一起排掉了 | 单独保留 `global.xml` 与 `public/` |
| `Summary.en.md` 重复登记 | 索引里「Release and Packaging」出现两次 | 登记脚本无存在性守卫 | 已修，登记前先查重 |
| README 章节层级断裂 | 「维护注意事项」变成空标题，5 个 `###` 子节被挂到「发布打包」下 | 新章节被插在 `##` 标题与其子节之间，且吃掉了标题后的空行 | 已修，章节整段移到父标题之前 |
| `pngpriv.h` 依赖链漏项 | 文档写「`pngpriv.h` → `pnglibconf.h`」，实际还有 `pngdebug.h` | 写文档时只看了前几处 include | 见第三节闭包，13 个文件已逐个核实 |

**另有一条顺序依赖**值得记住：`libpng/pngstruct.h:28-29` 是 `#ifndef ZLIB_H` 才 `#include "zlib.h"`，而 `libpng/` 目录下并没有 `zlib.h`。它能编过完全靠 `APngDecoder.h:5` 先包含了 `zlib/zlib.h` 把 `ZLIB_H` 定义掉。包内 `include/duilib/third_party/libpng/pngstruct.h` 仍保持这个顺序，闭包才自洽。

## 五、已知缺口与后续跟进

### 缺口

1. **版本号两处独立维护。** `msvc/dllRes/version.h` 的 `DUILIB_VER_*` 宏与 `VERSION` 文件目前各改各的，且该宏只被 `.vcxproj` 使用、CMake 侧读不到。升级版本须同时改两处。**后续应让 `version.h` 改为从 `VERSION` 派生。**

2. **只实测了 Windows。** 脚本本身跨平台（纯标准库，四平台共用同一份代码），但其它平台的库文件名后缀、静态库格式（`.a` vs `.lib`）差异可能需要按实际构建结果调整 `LIB_BASENAMES` 与格式判定。`docs/Release.md` 中已标注「未在本机验证」。

3. **签名与公证未自动化。** 本项目产出纯静态库（`add_library` 未指定 `SHARED`，`msvc/dllRes/dllmain.cpp` 只在 `.vcxproj` 中编译），SDK 包本身无需签名，文档只给手工流程。自动化签名等同 B-2 的 CI —— 需要证书管理，是长期维护负担。

4. **`pack.py` 两处常量命名有误导。** `THIRD_PARTY_INCLUDE_DIRS`（"指定子目录"）与 `THIRD_PARTY_INCLUDE_TREE_ROOTS`（"整树"）在实现上**完全等价** —— `pack.py:260-282` 两段代码都是 `iter_files(root)` 递归遍历，区别仅在路径拼接层数。`giflib` 放在 DIRS 里转发的是**整个 giflib 目录**（3 个 .h），并不是「只转发指定的那个文件」。两个常量真正承载的语义是「**单层快捷路径** vs **多级路径分量元组**」。`('giflib',)` 完全可以并入 TREE_ROOTS，行为零变化。**建议调整注释与常量名的措辞**（不是 bug，是表述偏差）。

5. **已放弃的子项。** NSIS/WiX（安装程序）与 DEB/dmg（系统包）不做 —— 静态库 SDK 用 zip / tgz 足够，制作安装程序对当前阶段是过度设计。

### 重新验收的方法

打包功能重新启用时，按这个顺序验收：

```
# 1. 列表模式先看收集范围
python build/pack.py --list

# 2. 打包
python build/pack.py

# 3. 完整性校验
cd dist && unzip -q nim_duilib-<版本>-<系统>-<架构>.zip -d /tmp/check
cd /tmp/check/nim_duilib-* && sha256sum -c SHA256SUMS

# 4. 下游编译验证（关键验收）
```

第 4 步是**最有效的验收手段**：解压产物后，在包外写一个 `probe.cpp`，**只加 `/Iinclude`**，然后：

```cpp
#include "duilib/duilib.h"
#include "duilib/Core/ZipStreamIO.h"
#include "duilib/Image/APngDecoder.h"
#include "duilib/Image/FrameSequence_gif.h"
#include "duilib/WebView2/WebView2Control.h"
```

期望 **EXITCODE=0**。这项证明「包外独立工程可编译」，比检查文件数量有效得多。

## 六、当前已有的验收结果

2026-10-20 在 Windows 上的实测结果：

- **下游编译验证**：`duilib/duilib.h` 与 4 个依赖 third_party 的头文件（`ZipStreamIO.h` / `APngDecoder.h` / `FrameSequence_gif.h` / `WebView2Control.h`）**全部 EXITCODE=0**。
- **资源排除精确性**：关闭示例时包内 `themes/default/` 下只有 `global.xml` 与 `public/`；打开时 389 个文件与仓库完全一致。
- **校验和**：`SHA256SUMS` 独立复算 587/587 全部一致。
- **包结构**：zip 与 tgz 两种格式均正确。
- **四平台差异**：脚本同构，但**产物正确性只有 Windows 被实测过**（见第五节缺口 2）。

## 七、相关文档

* [发布与打包](../Release.md) —— 面向使用者的完整操作说明
* [编译流程与脚本说明](../../build/README.md)
* [开发计划](Development-Plan.md) —— B-3 条目及整体路线图
* [项目审计报告](Project-Audit.md) —— B-3 的原始审计结论
