# 发布与打包

本文档说明如何把 nim_duilib 打包成可分发的产物，以及发布时的版本与签名流程。

打包脚本为 `build/pack.py`，它**只收集已构建好的文件，不参与编译**。

## 为什么不用 CPack

本仓库没有根 `CMakeLists.txt`。`duilib/`、`examples/*/`、`duilib/third_party/*/`
各自是独立的 `project()`，由 `build/msvc_build.bat`、`build/linux_build.sh` 等
脚本逐个 configure + build。CPack 需要一个顶层项目才能挂载，
"补 `install()` 规则" 这条路在本仓库走不通。

此外 Windows 有两条构建路径 —— `build/*.sln`（MSBuild）与 `msvc_build.bat`
（CMake）—— 产物混在同一 `lib/`、`bin/` 下，任何单一构建系统都覆盖不全。
`pack.py` 按文件名白名单收集，两条路径的产物都能收。

## 快速开始

打包前必须先完成 Release 编译，脚本不会触发编译：

```
# Windows（MSBuild 路径）
build\duilib.sln      # 配置选 Release，平台按需

# Windows（CMake 路径）
build\msvc_build.bat

# Linux / macOS / FreeBSD
./build/linux_build.sh
./build/macos_build.sh
./build/freebsd_build.sh
```

编译完成后打包：

```
# 只打 SDK（推荐）
python build/pack.py

# SDK + 示例程序
python build/pack.py --with-examples
```

产物输出到 `dist/`，包名格式为 `nim_duilib-<版本>-<系统>-<架构>.zip`：

```
dist/nim_duilib-1.0.2026.0416-win-x64.zip
```

## 产物结构

```
nim_duilib-<版本>-<系统>-<架构>/
  VERSION               版本号，与仓库根 VERSION 一致
  MANIFEST.txt          包内容说明与注意事项
  SHA256SUMS            每个文件的 SHA256，可用 sha256sum -c 校验
  LICENSE               主许可证
  include/duilib/       对外头文件
  lib/                  静态库
  resources/            主题、语言、字体资源
  licenses/             第三方许可证
  bin/                  仅 --with-examples 时存在
```

### 头文件路径不可改动

包内头文件保持 `include/duilib/xxx.h` 的层级，与仓库根的相对路径一致。
下游的引用写法是：

```cpp
#include "duilib/duilib.h"
```

因此 `include/` 下面必须还有一层 `duilib/`。若压平成 `include/duilib.h`，
下游全部编译失败。

`RenderSkia/` 不对外暴露 —— 渲染后端是实现细节，`duilib.h` 不引用它。

### third_party 头文件会一并转发

有 5 个对外头文件直接 include 了 `third_party` 下的头文件，
必须随包转发，否则下游编译失败：

| 对外头文件 | 依赖 |
| :--- | :--- |
| `Core/ZipStreamIO.h` | `zlib` |
| `Image/APngDecoder.h` | `zlib`、`libpng` |
| `Image/FrameSequence_gif.h` | `giflib` |
| `WebView2/WebView2Control.h` | `Microsoft.Web.WebView2` |
| `WebView2/WebView2EnvironmentOptions.h` | `Microsoft.Web.WebView2` |

`zlib` 与 `libpng` 内部存在传递依赖（如 `pngpriv.h` 又 include
`pnglibconf.h`），因此这两个库按整目录转发头文件，不做逐文件筛选。

## --with-examples 的资源范围差异

这是打包时最容易搞错的一点。`resources/themes/default/` 下同时放着
公共资源和各示例的私有主题目录，两种模式收录范围不同：

| 模式 | `resources/` 收录范围 |
| :--- | :--- |
| 默认（SDK） | 仅 `themes/default/public/` 与 `themes/default/global.xml` |
| `--with-examples` | 整个 `resources/`，无排除项 |

`global.xml` 在 SDK 模式下**必须保留**。它是框架启动的必需文件，
`GlobalManager.cpp` 在本地文件模式下找不到它会直接 `return false`，
初始化直接失败。

SDK 模式下被排除的是各示例的私有主题目录，共 22 个：
`MultiLang`、`basic`、`cef`、`cef_browser`、`chat`、`child_window`、
`color_theme`、`controls`、`dpi_aware`、`layout`、`list_box`、`list_ctrl`、
`move_control`、`panel`、`render`、`rich_edit`、`threads`、`tree_view`、
`virtual_list_box`、`webview2`、`webview2_browser`、`xml_preview`。

`bin/resources.zip` 一律不打包 —— 它只有 748 字节，是个空壳文件。

## 命令行参数

| 参数 | 说明 |
| :--- | :--- |
| `--with-examples` | 包含示例程序与全部示例私有资源 |
| `--with-cef-libs` | 包含 CEF wrapper 静态库（单个 80–100 MB，默认不带） |
| `--allow-debug-libs` | 允许把仅有 Debug 版的库打进包（默认排除，见下） |
| `--format zip\|tgz\|tarbz2` | 压缩格式，默认 Windows 用 `zip`、其它用 `tgz` |
| `--lib-dir <路径>` | 指定静态库目录，默认自动探测 |
| `--out-dir <路径>` | 输出目录，默认 `dist/` |
| `--list` | 只打印将收集的文件清单，不实际打包 |

## Debug 库默认被排除

脚本会区分 Release 与 Debug 库，**默认只打 Release**。
Debug 库带调试 CRT 与 `/MTd` 运行时，混进 Release SDK 会让下游链接异常。

Debug 判定有两个来源，缺一不可：

- 文件名带 `_d` 后缀（CMake 路径的 `duilib_d.lib`）
- 所在目录名为 `Debug`（MSBuild 路径的 `lib/Debug/duilib.lib`）

只看文件名会漏掉第二种，把 200MB 以上的 Debug 库当成 Release 打进去。
确实需要时用 `--allow-debug-libs`。

若所有候选目录里都找不到 `duilib` 主库，脚本会明确告警 ——
此时打出的包不含可链接的 duilib，只能作为头文件与资源的参考。

## CEF 相关限制

CEF 运行时（`libcef.dll`、`*.pak`、`icudtl.dat`）**不在本仓库内**，
由使用者自行从 libcef 官方发布包获取。因此：

- 含 CEF 的示例程序（`cef`、`CefBrowser`）打包后**不能直接运行**
- `--with-examples` 的 `MANIFEST.txt` 会列出这些示例并标注该限制
- CEF wrapper 静态库默认不打，需要时加 `--with-cef-libs`

## 版本号

版本号的单一事实源是仓库根的 `VERSION` 文件，格式为
`主版本.次版本.修订号.构建号`，例如 `1.0.2026.0416`。

`pack.py` 从该文件读取版本，用于包名、包内 `VERSION` 与 `MANIFEST.txt`。
格式非法（不是四段数字）时脚本报错退出，不会打出半成品包。

`msvc/dllRes/version.h` 中的 `DUILIB_VER_*` 宏目前是**独立维护**的，
只被 `.vcxproj` 的版本资源使用，CMake 侧读不到。
升级版本时两处都要改 —— 这是当前的已知缺口，后续应让 `version.h`
改为从 `VERSION` 派生。

git tag 建议用 `v<版本号>` 格式，便于与包名对应。
仓库现有 tag（`2026-06-08(main)` 等）是历史格式，含括号，
不能直接用作包名或 CPack 版本号。

## 发布流程

1. 确认工作区干净，编译产物为 Release
2. 更新 `VERSION` 与 `msvc/dllRes/version.h`
3. 打包并校验：

   ```
   python build/pack.py --list
   python build/pack.py
   ```

4. 校验产物完整性：

   ```
   cd dist && unzip -q nim_duilib-<版本>-<系统>-<架构>.zip -d /tmp/check
   cd /tmp/check/nim_duilib-* && sha256sum -c SHA256SUMS
   ```

5. 打 tag：`git tag -a v<版本号> -m "release <版本号>"`
6. 推 tag 与产物到发布渠道

## 签名与公证

当前**未自动化**，以下为手工流程说明。

### Windows

只有 exe 与 dll 能签名，静态库 `.lib` 不需要。本项目产出的是纯静态库
（`duilib/CMakeLists.txt` 中 `add_library` 未指定 `SHARED`，
`msvc/dllRes/dllmain.cpp` 只在 `.vcxproj` 中参与编译），
因此 SDK 包本身无需签名。

若发布含示例程序的包，建议对 exe 做 Authenticode 签名：

```
signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 ^
    /f <证书文件>.pfx /p <密码> dist\extracted\bin\*.exe
```

### macOS

对 `.app` 与 `.dylib` 做签名与公证：

```
codesign --force --deep --sign "Developer ID Application: <团队名>" <产物>
xcrun notarytool submit <产物.zip> --keychain-profile <profile> --wait
xcrun stapler staple <产物.zip>
```

### Linux / FreeBSD

通常不做签名。若需 GPG 签名：

```
gpg --detach-sign --armor dist/nim_duilib-<版本>-<平台>-<架构>.tar.gz
```

发布后可用 `gpg --verify` 校验。

## 四平台差异

| 平台 | 默认格式 | 状态 |
| :--- | :--- | :--- |
| Windows | `zip` | 已实测 |
| Linux | `tgz` | 脚本同构，产物未在本机验证 |
| macOS | `tgz` | 脚本同构，产物未在本机验证 |
| FreeBSD | `tgz` | 脚本同构，产物未在本机验证 |

打包脚本本身跨平台（纯标准库，无外部依赖），四平台共用同一份代码。
但**产物正确性只有 Windows 被实测过** —— 其它平台的库文件名后缀、
静态库格式（`.a` vs `.lib`）差异可能需要按实际构建结果调整
`LIB_BASENAMES` 与格式判定。

## 相关文档

* [编译流程与脚本说明](../build/README.md)
* [快速上手](Getting-Started.md)
* [示例程序](Examples.md)
* [CEF 控件](CEF.md)
