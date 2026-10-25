# 随包组件许可合规对照表

> 复核日期：2026-10-24
> 复核范围：`build/pack.py` 实际收集、随发布包以二进制形式分发的组件；
> 仅源码引用、默认未启用或未随包的组件单独注明。
> 依据：`python build/pack.py --config Debug --arch x64 --list` 的收集结果、
> Skia 构建参数（`skia/out/llvm.x64.*/args.gn`）与构建产物，以及
> `licenses/` 下的许可文本。

## 结论

- 随包的第三方组件全部为宽松许可（MIT、BSD-2/3、zlib、libpng、
  Apache-2.0 with LLVM exception 等），**允许以二进制形式随包分发**，
  前提是保留版权声明与许可文本。
- 未发现 GPL、AGPL、LGPL 等要求开源衍生作品或动态链接隔离的组件。
- WebView2 浏览器内核来自系统安装的 Evergreen Runtime，不随包分发，
  不涉及再分发许可；包内只有 BSD-3 的 SDK loader。
- 需要补充的许可文本有两处：Skia 静态链接的 expat（MIT）与资源字体
  Roboto Mono（Apache-2.0），均已补充到 `licenses/`。

## 对照表

| 组件 | 许可证 | 允许随包分发 | 包内文件（组件 / 许可文本） | 备注 |
| :--- | :--- | :--- | :--- | :--- |
| nim_duilib（本项目） | MIT | 是 | `LICENSE`（包根） | 本项目主体许可 |
| duilib（上游） | BSD-2-Clause | 是 | `lib/duilib.lib`、`bin/duilib*.dll`（DLL 模式）/ `licenses/duilib.LICENSE.txt` | 本项目在其基础上开发 |
| NIM_Duilib_Framework | MIT | 是 | 同上 / `LICENSES/NIM_Duilib_Framework.LICENSE.txt` | 上游框架许可 |
| Skia | BSD-3-Clause | 是 | `lib/skia.lib`、`svg.lib`、`skottie.lib`、`sksg.lib`、`skshaper.lib`、`jsonreader.lib` / `LICENSES/skia.LICENSE.txt` | 渲染引擎；源码不随包 |
| skcms | BSD-3-Clause | 是 | `lib/skcms.lib` / 由 Skia 许可覆盖 | Skia 内置色彩管理模块 |
| bentleyottmann | BSD-3-Clause | 是 | `lib/bentleyottmann.lib` / 由 Skia 许可覆盖 | Skia 内置几何模块 |
| skresources | BSD-3-Clause | 是 | `lib/skresources.lib` / 由 Skia 许可覆盖 | Skia 内置资源模块 |
| expat | MIT | 是 | `lib/expat.lib` / `LICENSES/expat.LICENSE.txt` | Skia SVG 解析依赖，静态链接 |
| zlib | zlib | 是 | `lib/duilib-zlib.lib` / `LICENSES/zlib.LICENSE.txt` | PNG/APNG、Zip 解压 |
| libpng | libpng-2.0（PNG Reference Library License v2） | 是 | `lib/duilib-png.lib` / `LICENSES/libpng.LICENSE.txt` | PNG/APNG |
| libwebp | BSD-3-Clause | 是 | `lib/duilib-webp.lib` / `LICENSES/libwebp.LICENSE.txt` | WebP |
| cximage | CxImage 自定义 MIT 风格 | 是 | `lib/duilib-cximage.lib` / `LICENSES/cximage.LICENSE.txt` | ICO 等图片格式 |
| libjpeg-turbo | IJG + BSD-3-Clause | 是 | `lib/turbojpeg-static.lib` / `LICENSES/libjpeg-turbo.LICENSE.md` | 仅 Windows MSVC 包随包（MinGW 不可链接，不收集），头文件不转发 |
| Roboto Mono（字体） | Apache-2.0 | 是 | `bin/resources/fonts/RobotoMono-*.ttf` / `LICENSES/RobotoMono.LICENSE.txt` | 字体 name 表声明 Apache-2.0；随示例资源分发 |
| giflib | MIT | 是 | 编译进 `lib/duilib.lib` / `LICENSES/giflib.LICENSE.txt` | GIF |
| stb_image | MIT 或公有领域（二选一） | 是 | 编译进 `lib/duilib.lib` / `LICENSES/stb_image.LICENSE.txt` | BMP、缩放 |
| nanosvg | zlib | 是 | 编译进 `lib/duilib.lib` / `LICENSES/nanosvg.LICENSE.txt` | SVG |
| pugixml | MIT | 是 | 编译进 `lib/duilib.lib` / `LICENSES/pugixml.LICENSE.txt` | XML 解析 |
| ConvertUTF（LLVM） | Apache-2.0 WITH LLVM-exception | 是 | 编译进 `lib/duilib.lib` / `LICENSES/llvm.LICENSE.txt` | UTF-8/16 转换 |
| udis86 | BSD-2-Clause | 是 | 编译进 `lib/duilib.lib` / `LICENSES/udis86.LICENSE.txt` | 指令长度计算 |
| CEF（含 Chromium） | BSD-3-Clause | 是 | `--with-cef` 时 `bin/libcef_win[_109]/<arch>/**`、`lib/libcef_dll_wrapper*.lib`、`lib/libcef*.lib` / `LICENSES/libcef.LICENSE.txt` | 运行时来自官方二进制分发包 |
| WebView2 SDK（loader） | BSD-3-Clause（Microsoft） | 是 | `bin/WebView2Loader.dll`、`lib/WebView2Loader.dll.lib`、`include/duilib/WebView2/**` / `LICENSES/Microsoft.Web.WebView2.LICENSE.txt`；MSVC 包另有 `lib/WebView2LoaderStatic.lib` | loader 不是浏览器内核 |
| WebView2 Runtime | Microsoft 许可（系统组件） | 不适用 | 不随包 | 依赖系统 Evergreen Runtime |
| SDL3 | zlib | 是 | `-sdl` 时 `include/SDL3/**`、`lib/libSDL3*` / `bin/SDL3.dll`、SDL 许可 `LICENSES/SDL3/**` + `LICENSES/SDL.LICENSE.txt` | 跨平台窗口管理 |
| libpag | Apache-2.0（含大量第三方组件） | 是（需附带全部第三方许可） | 默认未随包 / `LICENSES/libpag.LICENSE.txt` | 默认关闭；启用时须一并分发 `duilib/third_party/libpag/licenses/**` |

## 复核要点

1. **Skia 实际启用的第三方只有 expat**：`args.gn` 显式关闭了
   libpng、zlib、libjpeg-turbo、libwebp、ICU，构建产物中
   `out/*/obj/third_party` 仅出现 expat；因此不需要额外附带这些组件的许可。
2. **宽松许可的义务是保留声明**：仓库 `licenses/` 已覆盖表中全部许可文本，
   打包脚本会把 `licenses/` 复制为包内 `LICENSES/`，并带上根 `LICENSE` 与
   `THIRD-PARTY-NOTICES.md`，同时补充 Skia 源码树的 `LICENSE`（仓库内缺失时）。
3. **CEF**：`--with-cef` 只收集运行时（排除 `.lib`）与 wrapper 开发库，
   许可文本随包；CEF 官方二进制分发包本身即按 BSD-3 再分发。
   Chromium 内含的第三方组件声明未单独附带（沿用 CEF 官方分发方式），
   如需更严格，可在发布目录额外附 `LICENSES.chromium.html`。
4. **WebView2**：仅分发 Microsoft 的 SDK loader（BSD-3），
   浏览器内核由系统 Evergreen Runtime 提供，不随包，不涉及再分发。
5. **libpag**：默认不编译、不随包，仅保留主体许可文本；
   若启用，必须同时附带其 `licenses/` 目录下的全部第三方许可。
6. **资源内容**：`bin/resources/fonts/` 的 Roboto Mono 字体为 Apache-2.0，
   已附全文；示例网页仅通过 CDN 引用 Bootstrap（MIT），未随包，不涉及分发。

## 遗留事项

- [ ] 每次新增随包二进制组件或第三方资源（字体、图片、网页库）时，
  同步补充 `licenses/` 与上表；建议在发布检查单中加入
  “新增组件是否已列入许可对照表”。
- [ ] 如启用 libpag，补充其第三方许可并更新本表。
- [ ] 核对 `bin/resources` 中图片、Lottie/PAG 示例素材的来源与授权；
  当前未发现第三方许可声明，若来自外部素材库需补充署名或替换。
- [ ] 如需完整覆盖 CEF 的 Chromium 第三方声明，评估附带
  `LICENSES.chromium.html` 的体积影响。
