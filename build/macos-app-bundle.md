# 把示例编译为 macOS App Bundle

本文说明如何在不修改示例 C++ 代码的前提下，把 macOS 下的普通示例程序编译成
`.app`（App Bundle）。Windows、Linux、FreeBSD 完全不受影响。

`examples/basic` 已默认开启，执行一次正常构建即可得到 `bin/basic.app`：

```bash
python3 build/build.py --config Release
open bin/basic.app
```

其他示例只需在 `examples/<名称>/CMakeLists.txt` 中加一行：

```cmake
set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)
```

位置必须在 `include(.../duilib_example.cmake)` 之前。示例源码、资源初始化代码都不用改。

macOS 下 `bin/basic` 现在变成 `bin/basic.app`，可执行文件位于
`bin/basic.app/Contents/MacOS/basic`。自动化脚本若直接调用 `bin/basic`，需要改用
这个路径或使用 `open bin/basic.app`。

## 1. 两种开启方式

### 方式一：示例内单行开关（推荐）

```cmake
# examples/chat/CMakeLists.txt
cmake_minimum_required(VERSION 3.18)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
project(chat CXX)

set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)   # 仅 macOS 生效

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/duilib_example.cmake")
```

优点：配置跟随示例，换构建目录、换机器都有效；示例内显式设置的优先级最高。

### 方式二：命令行指定目标列表

不修改示例文件，通过新增的 CMake 缓存变量选择目标，名称为 CMake 目标名，
用分号或逗号分隔，`all` / `*` 表示全部示例：

```bash
# 只把 chat 编译为 .app
python3 build/build.py --cmake-arg=-DDUILIB_MACOS_BUNDLE_EXAMPLES=chat

# 多个目标
python3 build/build.py --cmake-arg=-DDUILIB_MACOS_BUNDLE_EXAMPLES=chat,panel

# 全部示例
python3 build/build.py --cmake-arg=-DDUILIB_MACOS_BUNDLE_EXAMPLES=all
```

直接用 CMake 时：

```bash
cmake -S . -B out/x64/Release -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DDUILIB_MACOS_BUNDLE_EXAMPLES="chat;panel"
cmake --build out/x64/Release
```

关闭某个示例的 Bundle：把该示例 CMakeLists 里的开关删掉或设为 `OFF`；若之前用
命令行列表开启，传入空列表即可：

```bash
python3 build/build.py --cmake-arg=-DDUILIB_MACOS_BUNDLE_EXAMPLES=
```

> CEF 示例（`cef`、`CefBrowser`）本来就使用 App Bundle，由
> `cmake/duilib_cef_macos.cmake` 处理，不受本开关影响。

## 2. 产物结构

以 `basic` 为例：

```text
bin/basic.app/
  Contents/
    Info.plist
    MacOS/
      basic                       # 可执行文件
    Resources/
      duilib/
        resources/                # 标准 Bundle 资源目录（与 exe 同级 resources/ 同构）
          themes/
            default/
              global.xml
              public/             # 默认主题的公共样式与图片
              basic/              # basic 示例自己的 XML
            color_light/
              global.xml
            color_dark/
              global.xml
          fonts/                  # 框架启动时加载
          lang/                   # text_id 文本
      basic.icns                  # 仅在配置了图标时存在
    Frameworks/
      libSDL3*.dylib              # SDL3 动态库（静态 SDL3 时不存在）
```

Bundle 内的资源清单由公共脚本按示例名自动推导（见 §4），只复制该示例需要的部分，
`themes/default` 下其他示例的目录（`chat`、`cef`、`controls` 等）不会进入 Bundle。
因此示例的 `CMakeLists.txt` 不需要配置任何资源列表。

资源采用 macOS 标准布局 `Contents/Resources/duilib/resources/`，其内部结构与普通程序的
`<可执行文件目录>/resources/` 完全一致，框架的路径拼接逻辑不需要区分两种形态。
`GlobalManager::GetResourceRootPath(bool bMacOsAppBundle)` 在 macOS 下的行为：

1. `bMacOsAppBundle == true`（默认值）：返回
   `<App>.app/Contents/Resources/duilib/resources/`（zip 模式为同目录的
   `resources.zip`）；
2. `bMacOsAppBundle == false`：返回“可执行文件同目录 + `resources/`”，
   兼容非 Bundle 程序和手动放置资源的场景。

因此示例代码保持 `GetResourceRootPath()` 或写 `GetResourceRootPath(true)` 都能正确加载；
SDK 消费者使用 `nim_duilib_copy_runtime(... RESOURCES)` 时也复制到相同布局。

## 3. 构建时的自动处理

`cmake/duilib_macos_bundle.cmake` 在链接后执行以下操作：

1. 把资源复制到 `Contents/Resources/duilib/resources`（标准布局）：按示例名自动
   推导的清单只复制需要的子目录/文件（basic 当前约 1.3 MB，整份 `bin/resources`
   约 4 MB）。复制前会清理旧的 `resources/` 目录，避免残留旧文件；
2. 若启用 SDL3（macOS 上始终启用），把 `libSDL3*.dylib` 及其版本软链接链复制到
   `Contents/Frameworks`；
3. 把主程序 rpath 设为 `@executable_path/../Frameworks`，避免加载构建机上的
   SDL3 绝对路径；
4. 删除可能残留的非 Bundle 产物 `bin/<目标>`；反方向切回普通程序时会删除
   `bin/<目标>.app`；
5. 从 `duilib/duilib_version.h` 读取版本，写入 `CFBundleVersion` 与
   `CFBundleShortVersionString`。

资源复制挂在目标的 `POST_BUILD` 上，只有目标重新链接时才会执行。如果只修改了
资源文件，而目标没有重新链接，请先删除 `bin/<目标>.app` 再构建，避免 Bundle
里仍是旧资源。

Bundle 只会在 macOS 上生成，其他平台的构建逻辑没有任何变化。

## 4. 自定义 Bundle 信息

在示例的 `CMakeLists.txt` 中、`include(duilib_example.cmake)` 之前可选设置：

```cmake
# Bundle Identifier，默认 com.nimduilib.<目标名>
set(DUILIB_MACOS_BUNDLE_IDENTIFIER "com.example.chat")

# Finder 中显示的名称，默认目标名
set(DUILIB_MACOS_BUNDLE_NAME "Chat Demo")

# .icns 图标（绝对路径，或相对示例源码目录）
set(DUILIB_MACOS_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/chat.icns")
```

### 默认资源清单（示例无需配置）

Bundle 内的资源清单由 `cmake/duilib_macos_bundle.cmake` 按示例的目标名
（`project(<名称> CXX)`）自动推导，示例的 `CMakeLists.txt` 不需要写任何资源列表：

| 条目 | 说明 |
| --- | --- |
| `themes/default/global.xml` | 全局变量、字体配置与公共控件样式（启动必需） |
| `themes/default/public` | `global.xml` 中 `${PATH_...}` 引用的公共图片/样式 |
| `themes/default/<示例资源目录>` | 示例自己的 XML 与图片 |
| `themes/color_light/global.xml`、`themes/color_dark/global.xml` | 语义色值，启动时按系统深浅色加载（必需），标题栏主题切换也使用 |
| `fonts` | `global.xml` 的 `FontFile` 在启动时加载 |
| `lang` | `text_id` 以及中英文切换所需文本 |

示例资源目录的匹配规则：把目标名和 `themes/default` 下的目录名都转成小写、
去掉下划线后比较，所以 `basic→basic`、`ChildWindow→child_window`、
`MultiLang→MultiLang`、`WebView2Browser→webview2_browser` 都能自动对应。
找不到对应目录时（如 `ColorPicker` 不使用 XML）只复制公共资源，不会报错。
也可以像 `cef`、`CefBrowser` 那样显式声明 `DUILIB_THEME_DIR_NAME`。

只有在需要额外资源或去掉默认条目时，才设置 `DUILIB_MACOS_BUNDLE_RESOURCES`
覆盖默认清单（相对 `bin/resources`，分号或逗号分隔）：

```cmake
# 可选：覆盖默认资源清单；目录递归复制，单个文件按原路径复制。
# 例如去掉颜色主题（不建议，界面会缺少语义色值）：
set(DUILIB_MACOS_BUNDLE_RESOURCES
    "themes/default/global.xml"
    "themes/default/public"
    "themes/default/chat"
    "fonts"
    "lang")
```

显式清单里的路径不存在会在配置阶段直接报错并打印缺失路径，便于定位。
无论使用默认清单还是显式清单，复制前都会清空 `Contents/Resources/duilib/resources`，
不会残留上一次的旧主题目录。

图标要求：

- 必须是 `.icns`；Windows 的 `.ico` 不能直接使用。
- 建议包含 16、32、128、256、512 以及对应的 `@2x` 尺寸。
- 可用“图标工厂”或 `iconutil` 生成：

```bash
mkdir -p Chat.iconset
sips -z 16 16     chat.png --out Chat.iconset/icon_16x16.png
sips -z 32 32     chat.png --out Chat.iconset/icon_16x16@2x.png
sips -z 128 128   chat.png --out Chat.iconset/icon_128x128.png
sips -z 256 256   chat.png --out Chat.iconset/icon_128x128@2x.png
sips -z 512 512   chat.png --out Chat.iconset/icon_512x512.png
sips -z 1024 1024 chat.png --out Chat.iconset/icon_512x512@2x.png
iconutil -c icns Chat.iconset -o chat.icns
```

`Info.plist` 模板在 `cmake/macos/Info.plist.in`，包含
`NSHighResolutionCapable`、`NSPrincipalClass` 与 `LSMinimumSystemVersion`。
如需添加权限声明、URL Scheme 等，可在示例目录提供自己的模板并设置
`MACOSX_BUNDLE_INFO_PLIST`（需在 `project()` 之后、创建目标之前）。

## 5. 验证

```bash
# Info.plist 语法与内容
plutil -lint bin/basic.app/Contents/Info.plist
plutil -p bin/basic.app/Contents/Info.plist | grep -E 'CFBundle(Identifier|Version|ShortVersionString)'

# SDL3 依赖与 rpath
otool -L bin/basic.app/Contents/MacOS/basic | grep SDL3
# 期望：@rpath/libSDL3*.dylib

otool -l bin/basic.app/Contents/MacOS/basic | grep -A2 LC_RPATH
# 期望：path @executable_path/../Frameworks

otool -D bin/basic.app/Contents/Frameworks/libSDL3.0.dylib
# 期望：@rpath/libSDL3.0.dylib

# 资源与启动
test -f bin/basic.app/Contents/Resources/duilib/resources/themes/default/global.xml && echo resources-ok
test ! -e bin/basic.app/Contents/Resources/duilib/resources/themes/default/chat && echo resources-trimmed
DYLD_PRINT_LIBRARIES=1 bin/basic.app/Contents/MacOS/basic
open bin/basic.app
```

如果启动后找不到主题或字体，先确认
`Contents/Resources/duilib/resources/themes/default/public`、`themes/default/basic`、
`themes/default/global.xml`、`themes/color_light/global.xml`、`themes/color_dark/global.xml`、
`lang/`、`fonts/` 都在 `bin/basic.app/Contents/Resources/duilib/resources/` 下。
构建脚本只复制默认清单里的资源，其他示例的目录本来就不应出现。

## 6. 签名与公证

本地自测可以用 ad-hoc 签名；对外分发必须使用 Developer ID，并按 Apple 要求
公证。签名顺序必须是“先改内容、后签名”，资源或 dylib 有变化后要重新签名。

```bash
# 本地自测
codesign --force --deep --sign - bin/basic.app
codesign --verify --deep --strict --verbose=2 bin/basic.app

# 对外分发（示例；实际使用自己的证书和 profile）
codesign --force --deep --options runtime \
         --sign "Developer ID Application: <团队名>" bin/basic.app
ditto -c -k --keepParent bin/basic.app basic.zip
xcrun notarytool submit basic.zip --keychain-profile <profile> --wait
xcrun stapler staple bin/basic.app
spctl -a -vv bin/basic.app
```

`--deep` 适合快速自测；正式发布建议由内到外逐个签名（`Contents/Frameworks` 下的
`libSDL3*.dylib` → 主可执行文件 → `.app`），避免遗漏嵌套二进制。

CEF 示例的签名、Helper 与 Framework 细节见
[docs/Release.md 的 macOS 章节](../docs/Release.md#签名与公证)。

## 7. 打包与包测试

`build/pack.py` 会自动把 `bin/` 下的 `.app` 当作目录整体收集，不需要额外参数：

```bash
python3 build/build.py --config Release
python3 build/pack.py --config Release --with-examples
```

产物位于 `dist/`，包内路径为 `bin/basic.app/...`。`build/sdk_package_test.py`
的 C10 用例已支持 `bin/basic.app/Contents/MacOS/basic`：

```bash
python3 build/sdk_package_test.py \
    --package dist/nim_duilib-<版本>-macos-x64-examples.tar.gz
```

Debug 与 Release 的 `.app` 同名，仍由 `bin/.build-config/<目标>` 标记区分。
切换配置时 `build.py` 会删除另一配置的同名产物，避免包内混入错误配置。

## 8. 常见问题

| 现象 | 原因 | 处理 |
| :--- | :--- | :--- |
| 仍然是 `bin/basic`，没有 `.app` | 开关没生效 | 确认 `set(DUILIB_EXAMPLE_MACOS_BUNDLE ON)` 位于 include 之前；本开关仅 macOS 生效 |
| `bin/basic.app` 中没有 `Resources/duilib/resources/` | 资源未复制或源目录缺失 | 确认仓库有 `bin/resources`，重新构建；查看构建日志中的 `Copying ... into basic.app` |
| 启动后提示找不到主题/字体 | 资源不在标准目录，或示例资源目录名与目标名不匹配 | 检查 `Contents/Resources/duilib/resources/themes/default/global.xml`；确认 `themes/default/<示例目录>` 存在，必要时用 `DUILIB_THEME_DIR_NAME` 指定目录名后再构建 |
| `themes/default` 下缺少其他示例的目录 | Bundle 默认按示例裁剪资源 | 正常现象；需要额外资源时用 `DUILIB_MACOS_BUNDLE_RESOURCES` 覆盖默认清单 |
| `Library not loaded: @rpath/libSDL3*.dylib` | SDL3 未复制或 rpath 被改动 | 检查 `Contents/Frameworks` 与 `LC_RPATH`；重新构建，必要时重新签名 |
| 双击提示“无法验证开发者” | 未签名/未公证 | 本地自测先 `codesign --force --deep --sign -`，分发按第 6 节处理 |
| 切换开关后 `bin/` 里同时有 `basic` 和 `basic.app` | 上一次产物残留 | 重新链接目标（构建脚本会删除反向产物），或手工删除旧产物 |
| 打包时收集了旧的 `.app` | 关闭 Bundle 后没有重新链接 | 手工 `rm -rf bin/<目标>.app` 后重新构建 |
| 修改资源后 Bundle 没变化 | 资源复制在 `POST_BUILD`，目标未重新链接 | 先 `rm -rf bin/<目标>.app`，再重新构建 |

## 9. 相关文件

| 文件 | 作用 |
| :--- | :--- |
| `examples/basic/CMakeLists.txt` | 已开启 Bundle；资源清单按示例名自动推导，无需配置 |
| `cmake/duilib_example.cmake` | 解析 `DUILIB_EXAMPLE_MACOS_BUNDLE` 与目标列表 |
| `cmake/duilib_bin_macos.cmake` | macOS 分支：`MACOSX_BUNDLE` 目标与普通目标二选一 |
| `cmake/duilib_macos_bundle.cmake` | Bundle 属性、资源、图标、SDL3、rpath |
| `cmake/macos/Info.plist.in` | Info.plist 模板 |
| `cmake/duilib_copy_sdl_bundle.cmake` | 复制 SDL3 动态库到 `Contents/Frameworks` |
