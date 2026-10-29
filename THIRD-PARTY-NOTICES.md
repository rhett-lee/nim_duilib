简体中文 | [English](THIRD-PARTY-NOTICES.en.md)

# 第三方组件声明

nim_duilib 使用或随 SDK 分发下列第三方组件。各组件的完整许可证文本位于
本仓库的 `licenses/` 目录，发布包中对应为 `LICENSES/`。

| 组件 | 用途 | 许可证 | 许可证文件 |
| --- | --- | --- | --- |
| Skia | 渲染引擎 | BSD-3-Clause | `licenses/skia.LICENSE.txt` |
| Chromium Embedded Framework (CEF) | 内嵌浏览器 | BSD-3-Clause | `licenses/libcef.LICENSE.txt` |
| SDL3 | 跨平台窗口与输入（可选） | zlib | `licenses/SDL.LICENSE.txt` |
| zlib | 压缩 | zlib | `licenses/zlib.LICENSE.txt` |
| libpng | PNG 编解码 | libpng License v2 | `licenses/libpng.LICENSE.txt` |
| libjpeg-turbo | JPEG 编解码 | IJG + BSD-3-Clause | `licenses/libjpeg-turbo.LICENSE.md` |
| libwebp | WebP 编解码 | BSD-3-Clause | `licenses/libwebp.LICENSE.txt` |
| giflib | GIF 编解码 | MIT | `licenses/giflib.LICENSE.txt` |
| expat | XML 解析（Skia SVG） | MIT | `licenses/expat.LICENSE.txt` |
| pugixml | XML 解析 | MIT | `licenses/pugixml.LICENSE.txt` |
| nanosvg | SVG 解析 | zlib | `licenses/nanosvg.LICENSE.txt` |
| stb_image | 图像解码 | MIT or Public Domain | `licenses/stb_image.LICENSE.txt` |
| udis86 | 反汇编 | BSD-2-Clause | `licenses/udis86.LICENSE.txt` |
| CxImage | 图像处理 | 见许可证文件 | `licenses/cximage.LICENSE.txt` |
| libpag | 动画渲染（可选） | 见许可证文件 | `licenses/libpag.LICENSE.txt` |
| Microsoft.Web.WebView2 | WebView2 控件（Windows，可选） | BSD-3-Clause | `licenses/Microsoft.Web.WebView2.LICENSE.txt` |
| Roboto Mono | 等宽字体资源 | Apache-2.0 | `licenses/RobotoMono.LICENSE.txt` |
| LLVM (clang toolchain) | Skia 构建工具链 | Apache-2.0 WITH LLVM-exception | `licenses/llvm.LICENSE.txt` |
| duilib | 上游项目 | BSD-3-Clause | `licenses/duilib.LICENSE.txt` |
| NIM_Duilib_Framework | 本项目基础 | MIT | `licenses/NIM_Duilib_Framework.LICENSE.txt` |

## 示例动画与图片素材

`bin/resources` 中的动画与图片测试素材仅用于示例程序演示，不单独授予任何许可：

- `themes/default/public/animation/loading1..6.json`、`loading_progress1/2.json`
  （Lottie）；其中 `loading5.json`、`loading6.json` 的 `meta.g` 为 `LottieFiles AE`，
  由 LottieFiles 导出；
- `themes/default/render/lottie.json`、`pag_logo.pag`（提交 `020fcfe0`，2025-10-22
  加入时未记录来源与授权）；
- `themes/default/render/{fan.gif,gif_test.gif,gif_test2.gif,jpg_test.jpg,bmp_test_24.bmp,webp_test*.webp}`
  为图片编解码测试图。

其余控件图标（SVG/PNG）随功能开发提交，按项目自制素材处理，由根 `LICENSE`（MIT）
覆盖；上游初始资源（144 个文件，2019-04-19 导入）由
`licenses/NIM_Duilib_Framework.LICENSE.txt` 与 `licenses/duilib.LICENSE.txt` 覆盖。
示例网页通过 CDN 引用 Bootstrap 3.3.7、Font Awesome Free 4.7.0 与 Tailwind，
运行期由浏览器加载，不随包。若要在商业产品中再分发上述演示素材，
请先自行确认来源或替换为自有素材。

## 说明

- 本文件只做组件汇总，不改变任何组件的许可证条款；以 `LICENSES/` 中
  对应许可证文本为准。
- 可选的 CEF、SDL3、WebView2、libpag 仅在启用相应功能并随包分发时适用。
- 发布 SDK 时请同时保留 `LICENSES/`、本文件及英文版
  `THIRD-PARTY-NOTICES.en.md`，不得单独删除。
