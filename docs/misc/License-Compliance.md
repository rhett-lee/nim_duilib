# 随包组件许可合规对照表

> 复核日期：2026-10-29（首次建立 2026-10-24，随后续组件与素材复核更新）
> 复核范围：`build/pack.py` 实际收集、随发布包以二进制形式分发的组件；
> 仅源码引用、默认未启用或未随包的组件单独注明。
> 依据：`python build/pack.py --config Debug --arch x64 --list` 的收集结果、
> Skia 构建参数（`skia/out/llvm.x64.*/args.gn`）与构建产物、
> `licenses/` 下的许可文本，以及 `bin/resources`（4.2 MB / 409 个文件）与
> `duilib/third_party/libpag/licenses/` 的来源核对。

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
| libjpeg-turbo | IJG + BSD-3-Clause | 是 | `lib/turbojpeg-static.lib`（静态模式）/ `bin/turbojpeg.dll`（DLL 模式）/ `LICENSES/libjpeg-turbo.LICENSE.md` | 仅 Windows MSVC 包随包（MinGW 不可链接，不收集），头文件不转发 |
| Roboto Mono（字体） | Apache-2.0 | 是 | `bin/resources/fonts/RobotoMono-*.ttf` / `LICENSES/RobotoMono.LICENSE.txt` | 字体 name 表声明 Apache-2.0；随示例资源分发 |
| giflib | MIT | 是 | 编译进 `lib/duilib.lib` / `LICENSES/giflib.LICENSE.txt` | GIF |
| stb_image | MIT 或公有领域（二选一） | 是 | 编译进 `lib/duilib.lib` / `LICENSES/stb_image.LICENSE.txt` | BMP、缩放 |
| nanosvg | zlib | 是 | 编译进 `lib/duilib.lib` / `LICENSES/nanosvg.LICENSE.txt` | SVG |
| pugixml | MIT | 是 | 编译进 `lib/duilib.lib` / `LICENSES/pugixml.LICENSE.txt` | XML 解析 |
| ConvertUTF（LLVM） | Apache-2.0 WITH LLVM-exception | 是 | 编译进 `lib/duilib.lib` / `LICENSES/llvm.LICENSE.txt` | UTF-8/16 转换 |
| udis86 | BSD-2-Clause | 是 | 编译进 `lib/duilib.lib` / `LICENSES/udis86.LICENSE.txt` | 指令长度计算 |
| CEF（含 Chromium） | BSD-3-Clause | 是 | `--with-cef` 时 `bin/libcef_win[_109]/<arch>/**`、`lib/libcef_dll_wrapper*.lib`、`lib/libcef*.lib` / `LICENSES/libcef.LICENSE.txt` | 运行时来自官方二进制分发包；未附 Chromium 组件级声明，见「遗留事项处理结果」 |
| WebView2 SDK（loader） | BSD-3-Clause（Microsoft） | 是 | `bin/WebView2Loader.dll`、`lib/WebView2Loader.dll.lib`、`include/duilib/WebView2/**` / `LICENSES/Microsoft.Web.WebView2.LICENSE.txt`；MSVC 包另有 `lib/WebView2LoaderStatic.lib` | loader 不是浏览器内核 |
| WebView2 Runtime | Microsoft 许可（系统组件） | 不适用 | 不随包 | 依赖系统 Evergreen Runtime |
| SDL3 | zlib | 是 | `-sdl` 时 `include/SDL3/**`、`lib/libSDL3*` / `bin/SDL3.dll`、SDL 许可 `LICENSES/SDL3/**` + `LICENSES/SDL.LICENSE.txt` | 跨平台窗口管理 |
| libpag | Apache-2.0（含大量第三方组件） | 是（需附带全部第三方许可） | 默认不编译、不随包 / `LICENSES/libpag.LICENSE.txt` | 默认关闭，SDK 包元数据固定 `DUILIB_LIB_PAG=0`；启用时须一并分发 `duilib/third_party/libpag/licenses/**`（含 `tgfx.LICENSE.txt`） |

## 复核要点

1. **Skia 实际启用的第三方只有 expat**：`args.gn` 显式关闭了
   libpng、zlib、libjpeg-turbo、libwebp、ICU，构建产物中
   `out/*/obj/third_party` 仅出现 expat；因此不需要额外附带这些组件的许可。
2. **宽松许可的义务是保留声明**：仓库 `licenses/` 已覆盖表中全部许可文本，
   打包脚本会把 `licenses/` 复制为包内 `LICENSES/`，并带上根 `LICENSE` 与中英文
   两份 `THIRD-PARTY-NOTICES`（`THIRD-PARTY-NOTICES.md` / `.en.md`），同时补充
   Skia 源码树的 `LICENSE`（仓库内缺失时）。
3. **CEF**：`--with-cef` 只收集运行时（排除 `.lib`）与 wrapper 开发库，
   许可文本随包；CEF 官方二进制分发包本身即按 BSD-3 再分发。
   Chromium 内含的第三方组件声明未单独附带（沿用 CEF 官方分发方式），
   默认不随包，需要更严格覆盖时的补充方式与体积评估见
   「遗留事项处理结果」第 4 条。
4. **WebView2**：仅分发 Microsoft 的 SDK loader（BSD-3），
   浏览器内核由系统 Evergreen Runtime 提供，不随包，不涉及再分发。
5. **libpag**：默认不编译、不随包，仅保留主体许可文本；
   若启用，必须同时附带其 `licenses/` 目录下的全部第三方许可
   （`libpag.LICENSE.txt`、`tgfx.LICENSE.txt`），且确认未使用 lz4（GPLv2）与 Qt（GPLv3）。
6. **资源内容**：`bin/resources/fonts/` 的 Roboto Mono 字体为 Apache-2.0，
   已附全文；控件图标（SVG/PNG）随功能开发提交，按项目自制素材处理；
   Lottie/PAG 动画与图片测试图的来源与处理见「遗留事项处理结果」第 3 条。
   示例网页仅通过 CDN 引用 Bootstrap 3.3.7（MIT）、Font Awesome Free 4.7.0
   （OFL-1.1 / MIT）与 Tailwind CDN（MIT），运行期由浏览器加载、未随包，不构成再分发。

## 遗留事项处理结果（2026-10-29）

- [x] **新增随包二进制组件或第三方资源时同步许可**：登记流程已写入
  [Release-Checklist.md](Release-Checklist.md) 第 1 节（准备）与第 5 节（finalize 校验）：
  1. 出包前执行 `python build/pack.py --config <Config> --arch <arch> [特性开关] --list`，
     把清单中的二进制与资源产物逐项对照本表；
  2. 新增组件必须同时补齐 `licenses/<组件>.LICENSE.*`、`THIRD-PARTY-NOTICES.md`
     与 `THIRD-PARTY-NOTICES.en.md` 中的同名行，以及本表的一行，并在本文件末尾的
     更新记录中注明来源；
  3. 未登记的组件不进入 `dist/<版本>/`（finalize 阶段由发布人按包内 `LICENSES/`
     与 `bin/resources/` 复核）。
  现状：2026-10-29 复核时，表中 25 个组件均能在 `licenses/` 的 20 个文件中找到对应
  许可文本（部分组件共用 Skia/duilib 的许可文件），无未登记项。
- [x] **libpag 第三方许可**：libpag 保持默认关闭，不编译、不随包。
  - SDK 包元数据固定 `DUILIB_LIB_PAG=0`（`packaging/cmake/nim_duilibConfig.cmake`、
    `packaging/msvc/nim_duilib.props`），包内不收集 `libpag.lib` / `libpag.dll`，
    下游无法误开；
  - 仓库只保留主体许可 `licenses/libpag.LICENSE.txt`（118,099 B，Apache-2.0）；
  - 本项目自行启用 libpag 时（非 SDK 包场景），必须随程序分发
    `duilib/third_party/libpag/licenses/` 下的全部文件：
    `libpag.LICENSE.txt`（118,099 B）、`tgfx.LICENSE.txt`（49,968 B）及其余 5 份说明文档；
    同时确认未使用 lz4（GPLv2）与 Qt（GPLv3），见
    `duilib/third_party/libpag/licenses/nim_duilib使用libpag说明.md`；
  - 若今后要在 SDK 包中开放 libpag，按「新增随包组件」流程处理：把上述许可
    复制到 `licenses/libpag/`，`DUILIB_LIB_PAG` 改为按包元数据取值，并同步本表。
- [x] **`bin/resources` 素材来源与授权**：共 4.2 MB / 409 个文件，按来源分三层处理
  （证据来自 `git log` 与文件内嵌元数据）：
  1. **上游初始导入（144 个文件）**：首个提交 `02c0f6e9`（2019-04-19，First commit）
     导入，属上游 NIM_Duilib_Framework 谱系，由仓库根 `LICENSE`（MIT）、
     `licenses/NIM_Duilib_Framework.LICENSE.txt` 与 `licenses/duilib.LICENSE.txt` 覆盖；
  2. **项目新增的控件/示例资源**：2019 年后随控件与示例开发提交的 XML、SVG、PNG
     与语言包（提交者以项目维护者为主，另有外部贡献者提交的聊天场景素材），
     提交记录中无第三方来源说明，按项目自制素材处理，由项目 MIT 许可覆盖；
  3. **Lottie/PAG 动画与图片测试素材（已注明风险）**：
     - `themes/default/public/animation/loading1..6.json`、`loading_progress1/2.json`
       （提交 `3a5ea36f`，2025-09-30）；其中 `loading5.json`、`loading6.json` 的
       `meta.g` 为 `LottieFiles AE`，可确认由 LottieFiles 导出；
     - `themes/default/render/lottie.json`、`pag_logo.pag`（提交 `020fcfe0`，2025-10-22），
       提交信息未记录来源与授权；
     - `themes/default/render/{fan.gif,gif_test.gif,gif_test2.gif,jpg_test.jpg,bmp_test_24.bmp,webp_test*.webp}`
       为图片解码/渲染测试图（2023 年加入，2025-10-22 替换为体积更小的版本）。
     结论：这些文件只用于示例程序的渲染演示；仓库内没有随附的第三方授权记录。
     已在包内 `THIRD-PARTY-NOTICES.md` 增加「示例动画与图片素材」说明，声明其仅作演示、
     不单独授权，下游二次分发前应自行确认来源或替换为自有素材。风险已记录，
     不作为当前出包的阻断项。
  - 其他：`fonts/RobotoMono-*.ttf` 为 Apache-2.0（已附全文），`lang/*.txt` 为项目自有；
    示例网页通过 CDN 引用 Bootstrap 3.3.7（MIT）、Font Awesome Free 4.7.0
    （OFL-1.1 / MIT）与 Tailwind CDN（MIT），运行期由浏览器加载、不随包，不构成再分发。
- [x] **CEF 的 Chromium 第三方声明**：评估完成，默认不随包。
  - 实测：CEF 官方二进制分发包（`cef_binary_109.1.18+gf1c41e4+chromium-109.0.5414.120`、
    142 系列，位于 `E:\develop\cef_binary`）只带 `LICENSE.txt` / `README.txt`，
    **不含 `LICENSES.chromium.html`**；本机未安装 Chrome/Chromium，网络不可用，
    未能取得该文件实测体积。
  - 体积评估（按 Chromium 生成方式估算，未实测）：该文件是 Chromium 全部第三方
    许可文本的 HTML 拼接，通常为 10 MB 量级（未压缩），zip 后约 1–2 MB；
    相对 CEF 运行时（Windows x64 约 200 MB）占比不足 1%，对下载体积影响可忽略。
  - 决策：沿用 CEF 官方分发方式，包内只随 `licenses/libcef.LICENSE.txt`（BSD-3）；
    Chromium 的组件级声明不进入默认包，避免引入与 CEF 版本不匹配的声明。
  - 可选增强：需要更严格覆盖时，从与 CEF 所用 Chromium 主版本一致的 Chrome/Chromium
    安装目录复制 `LICENSES.chromium.html` 到包内 `LICENSES/CEF/`，并在 `MANIFEST.txt`
    与本表登记。该步骤为可选，已在发布检查单中注明。

## 更新记录

| 日期 | 变更 |
| :--- | :--- |
| 2026-10-24 | 建立对照表，覆盖 25 个组件；确认随包组件均为宽松许可，无 GPL/AGPL/LGPL |
| 2026-10-29 | 4 条遗留事项全部处理：新增组件登记流程、libpag 启用许可、`bin/resources` 来源分层与演示素材说明、CEF `LICENSES.chromium.html` 评估与决策 |
