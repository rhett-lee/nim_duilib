# 发布与打包

本文档说明如何把 nim_duilib 打包成可分发的产物，以及发布时的版本与签名流程。

打包脚本为 `build/pack.py`，它**只收集已构建好的文件，不参与编译**。

## 为什么不用 CPack

本仓库没有根 `CMakeLists.txt`。`duilib/`、`examples/*/`、`duilib/third_party/*/`
各自是独立的 `project()`，由 `build/msvc_build.bat`、`build/linux_build.sh` 等
脚本逐个 configure + build。CPack 需要一个顶层项目才能挂载，
"补 `install()` 规则" 这条路在本仓库走不通。

此外 Windows 有两条构建路径 —— `build/*.sln`（MSBuild）与 `msvc_build.bat`
（CMake）。两者的库产物现在统一落在同一套目录：

| 平台 | 库目录 |
| :--- | :--- |
| Windows | `lib/<x64\|x86\|arm64>/<Release\|Debug>/` |
| Linux / macOS / FreeBSD | `lib/<Release\|Debug>/` |

配置只由目录表达，库名不再带 `_d`。约定**同一个 `<arch>|<Config>` 一次只由
一套体系完整构建**，后构建者覆盖前者（含 `.pdb`）。`pack.py` 只读取
`--config` 选中的一个目录，不再合并候选目录或按优先级猜测。

## 快速开始

打包前必须先完成对应配置的编译，脚本不会触发编译：

```
# Windows（MSBuild 路径）
build\duilib.sln      # 配置选 Release 或 Debug，平台按需

# Windows（CMake 路径）
build\msvc_build.bat

# Linux / macOS / FreeBSD
./build/linux_build.sh
./build/macos_build.sh
./build/freebsd_build.sh
```

编译完成后打包：

```
# 只打 SDK（推荐，默认 Release）
python build/pack.py

# Debug SDK（包名带 -debug）
python build/pack.py --config Debug

# SDK + 示例程序
python build/pack.py --with-examples
```

产物输出到 `dist/`，包名格式为
`nim_duilib-<版本>-<系统>-<架构>[-debug].zip`：

```
dist/nim_duilib-1.0.2026.0416-win-x64.zip
dist/nim_duilib-1.0.2026.0416-win-x64-debug.zip
```

## 产物结构

```
nim_duilib-<版本>-<系统>-<架构>/
  VERSION               版本号，与仓库根 VERSION 一致
  MANIFEST.txt          包内容说明与注意事项
  SHA256SUMS            每个文件的 SHA256，可用 sha256sum -c 校验
  LICENSE               主许可证
  include/duilib/       对外头文件（使用 SDL 时含 include/SDL3/）
  lib/                  静态库（duilib 5 个库 + Skia 全部库 + 第三方链接库）
  resources/            主题、语言、字体资源
  licenses/             第三方许可证
  bin/                  仅 --with-examples 时存在
```

### SDK 使用契约

为保证下游能完成**编译、链接、运行**，`MANIFEST.txt` 固化以下内容：

1. **包含路径**：`include/`（`#include "duilib/duilib.h"`）。
   使用 CEF 时追加 `include/duilib/third_party/libcef/<libcef_win|libcef_win_109>`。
2. **库路径**：`lib/`（平面结构）。
3. **链接列表**：
   - 基础：`duilib`、`duilib-cximage`、`duilib-webp`、`duilib-png`、`duilib-zlib`
   - Skia：`skia`、`svg`、`skshaper`、`skottie`、`sksg`、`jsonreader`
     （`bentleyottmann`、`expat`、`skcms`、`skresources` 随包，出现未解析符号时补链）
   - WebView2：`WebView2LoaderStatic`（VS）或 `WebView2Loader.dll`（CMake，运行期需 DLL）
   - JPEG（Windows）：`turbojpeg-static`
   - SDL（可选）：`SDL3-static`（Windows）/ `SDL3`（Unix）
   - CEF（可选）：`libcef` + `libcef_dll_wrapper[_109]`
4. **平台系统库**：Windows 启用 SDL 时补 `Version/Winmm/Imm32/Setupapi`；
   Unix 的 `X11`/`freetype`/`fontconfig` 由系统提供，不随包。
5. **运行期**：`resources/` 与可执行文件同级；CEF 运行时外置；
   Unix 的 `libSDL3.so*` 需可被找到（`LD_LIBRARY_PATH` 等）。
6. **配置一致性**：Debug 包只给 Debug 下游（`/MTd`），Release 包只给
   Release 下游（`/MT`），两套不可混用。

### 头文件路径不可改动

包内头文件保持 `include/duilib/xxx.h` 的层级，与仓库根的相对路径一致。
下游的引用写法是：

```cpp
#include "duilib/duilib.h"
```

因此 `include/` 下面必须还有一层 `duilib/`。若压平成 `include/duilib.h`，
下游全部编译失败。

`RenderSkia/` 不对外暴露 —— 渲染后端是实现细节，`duilib.h` 不引用它。

### third_party 头文件只转发必需项

包内 `Image/` 只保留对外的基础接口头文件（`Image.h`、`ImageAttribute.h`、
`ImageInfo.h`、`ImageLoadParam.h`、`ImageDecoder.h`、`ImageDecoderFactory.h`、
`ImageUtil.h`、`StateImage.h`、`StateImageMap.h`、`ImagePlayer.h`）。

具体图片解码实现头文件**不随包**，因为它们只被库内 `.cpp` 使用，
`duilib.h` 的包含链不引用：

* `Image/ImageDecoder_*.h`（PNG / GIF / JPEG / WEBP / SVG / ICO / Icon /
  LOTTIE / PAG / Common）
* `Image/Image_*.h`（PNG / GIF / JPEG / WEBP / SVG / ICO / LOTTIE / PAG /
  Bitmap / Animation）
* `Image/APngDecoder.h`、`Image/FrameSequence_gif.h`、`Image/ImageDecoderUtil.h`

`Core/ZipStreamIO.h` 同样只在库内部使用（只被 `ZipManager.cpp` 包含，
`duilib.h` 与 `ZipManager.h` 都不引用），因此也不随包。

下列平台实现头文件只被库内 `.cpp` / `.mm` 包含，也不随包：

* `Core/ControlDropTargetImpl_SDL.h`
* `Core/ControlDropTargetImpl_Windows.h`
* `Core/ControlDropTargetUtils.h`
* `Core/SDL_MacOS.h`
* `Core/WindowDropTarget_SDL.h`
* `Core/WindowDropTarget_Windows.h`

例外：`Core/DragWindowFilter_SDL.h` 被 `Core/ControlDragable.h` 在
`DUILIB_BUILD_FOR_SDL` 下条件包含。非 Windows 平台该宏始终开启，Unix 包会保留该头文件；
Windows 包未启用 `--with-sdl` 时不打包。

对外头文件真正依赖的 `third_party` 头文件只有一处，随包转发：

| 对外头文件 | 依赖 | 说明 |
| :--- | :--- | :--- |
| `WebView2/WebView2Control.h`、`WebView2/WebView2EnvironmentOptions.h` | `Microsoft.Web.WebView2` | 仅 Windows 转发 |

相应地，`zlib`、`libpng`、`giflib`、`libjpeg-turbo` 的头文件都不再转发。
其中 `duilib-zlib`、`turbojpeg-static` 静态库仍随包（链接需要），只是不提供头文件。

上述包含关系已逐个核对：包内 `duilib/` 头文件的 `#include "duilib/..."`
全部可以解析，下游只需 `#include "duilib/duilib.h"` 即可编译。

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
| `--config Release\|Debug` | 选择库配置，默认 `Release`（大小写不敏感） |
| `--arch x64\|x86\|arm64` | 库架构；Windows 决定库目录，所有平台影响包名 |
| `--lib-dir <路径>` | 完全显式指定库目录，覆盖 `--arch`/`--config` 的路径计算 |
| `--skia-lib-dir <路径>` | Skia 静态库目录，默认探测 `../skia/out/llvm.<arch>.<config>` |
| `--sdl-dir <路径>` | SDL3 根目录，默认 `../SDL3` |
| `--with-sdl` | Windows 下额外打包 SDL3（Unix 始终打包） |
| `--with-cef` | 打包 CEF 开发包：头文件 + `libcef.lib` + wrapper（体积大） |
| `--cef-version latest\|109` | 配合 `--with-cef` 选择版本，默认 `latest` |
| `--with-examples` | 包含示例程序与全部示例私有资源 |
| `--format zip\|tgz\|tarbz2` | 压缩格式，默认 Windows 用 `zip`、其它用 `tgz` |
| `--out-dir <路径>` | 输出目录，默认 `dist/` |
| `--list` | 只打印将收集的文件清单，不实际打包 |

## Debug 包

配置只由目录决定（`lib/<arch>/Debug` 或 `lib/Debug`），不再靠 `_d` 后缀判断。

- `--config Debug` 时包名追加 `-debug`，`MANIFEST.txt` 标注 `配置 : Debug`。
- Debug 库带调试 CRT 与 `/MTd`，只能用于 Debug 下游，**不可混进 Release SDK**。
- `--with-examples` 时：Release 包跳过 `*_d.exe`；Debug 包因 CMake 的 Debug
  exe 与 Release 同名，无法区分，脚本会告警提示可能混有 Release 版本。
- 库目录不存在或主库 `duilib` 缺失时，脚本直接以退出码 1 失败，不会打出
  没有可链接库的半成品包。

## CEF 相关限制

CEF 运行时（`libcef.dll`、`*.pak`、`icudtl.dat`）**不在本仓库内**，
由使用者自行从 libcef 官方发布包获取。因此：

- 含 CEF 的示例程序（`cef`、`CefBrowser`）打包后**不能直接运行**
- `--with-examples` 的 `MANIFEST.txt` 会列出这些示例并标注该限制
- CEF 开发包默认不打，需要时加 `--with-cef`（可用 `--cef-version 109` 选旧版）

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
Windows 的 `.lib` 与 Unix 的 `.a` 均按目录白名单收集；Unix 的 `lib` 前缀
会自动归一化（如 `libduilib-zlib.a` → `duilib-zlib`）。但**产物正确性目前
只有 Windows 被实测过**，其它平台首次打包后仍需核对库清单。

## 相关文档

* [编译流程与脚本说明](../build/README.md)
* [快速上手](Getting-Started.md)
* [示例程序](Examples.md)
* [CEF 控件](CEF.md)
