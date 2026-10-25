# 第三方组件声明 / Third-Party Notices

nim_duilib 使用或随 SDK 分发下列第三方组件。各组件的完整许可证文本位于
本仓库的 `licenses/` 目录，发布包中对应为 `LICENSES/`。

This product includes the following third-party components. The full license
texts live in `licenses/` in this repository and in `LICENSES/` inside the
released SDK.

| 组件 / Component | 用途 / Usage | 许可证 / License | 许可证文件 / License file |
| --- | --- | --- | --- |
| Skia | 渲染引擎 / rendering engine | BSD-3-Clause | `licenses/skia.LICENSE.txt` |
| Chromium Embedded Framework (CEF) | 内嵌浏览器 / embedded browser | BSD-3-Clause | `licenses/libcef.LICENSE.txt` |
| SDL3 | 跨平台窗口与输入 / windowing and input (optional) | zlib | `licenses/SDL.LICENSE.txt` |
| zlib | 压缩 / compression | zlib | `licenses/zlib.LICENSE.txt` |
| libpng | PNG 编解码 / PNG codec | libpng License v2 | `licenses/libpng.LICENSE.txt` |
| libjpeg-turbo | JPEG 编解码 / JPEG codec | IJG + BSD-3-Clause | `licenses/libjpeg-turbo.LICENSE.md` |
| libwebp | WebP 编解码 / WebP codec | BSD-3-Clause | `licenses/libwebp.LICENSE.txt` |
| giflib | GIF 编解码 / GIF codec | MIT | `licenses/giflib.LICENSE.txt` |
| expat | XML 解析 / XML parser (Skia SVG) | MIT | `licenses/expat.LICENSE.txt` |
| pugixml | XML 解析 / XML parser | MIT | `licenses/pugixml.LICENSE.txt` |
| nanosvg | SVG 解析 / SVG parser | zlib | `licenses/nanosvg.LICENSE.txt` |
| stb_image | 图像解码 / image decoding | MIT or Public Domain | `licenses/stb_image.LICENSE.txt` |
| udis86 | 反汇编 / disassembler | BSD-2-Clause | `licenses/udis86.LICENSE.txt` |
| CxImage | 图像处理 / image processing | 见许可证文件 / see file | `licenses/cximage.LICENSE.txt` |
| libpag | 动画渲染 / animation (optional) | 见许可证文件 / see file | `licenses/libpag.LICENSE.txt` |
| Microsoft.Web.WebView2 | WebView2 控件 / WebView2 control (Windows, optional) | BSD-3-Clause | `licenses/Microsoft.Web.WebView2.LICENSE.txt` |
| Roboto Mono | 等宽字体资源 / monospace font asset | Apache-2.0 | `licenses/RobotoMono.LICENSE.txt` |
| LLVM (clang toolchain) | Skia 构建工具链 / Skia build toolchain | Apache-2.0 WITH LLVM-exception | `licenses/llvm.LICENSE.txt` |
| duilib | 上游项目 / upstream project | BSD-3-Clause | `licenses/duilib.LICENSE.txt` |
| NIM_Duilib_Framework | 本项目基础 / project base | MIT | `licenses/NIM_Duilib_Framework.LICENSE.txt` |

## 说明 / Notes

- 本文件只做组件汇总，不改变任何组件的许可证条款；以 `LICENSES/` 中
  对应许可证文本为准。
- 可选的 CEF、SDL3、WebView2、libpag 仅在启用相应功能并随包分发时适用。
- 发布 SDK 时请同时保留 `LICENSES/` 与本文件，不得单独删除。

- This file is a summary only and does not modify any component license; the
  texts under `LICENSES/` are authoritative.
- Optional components (CEF, SDL3, WebView2, libpag) apply only when enabled and
  shipped.
- When redistributing the SDK, keep both `LICENSES/` and this file.
