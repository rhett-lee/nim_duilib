English | [简体中文](THIRD-PARTY-NOTICES.md)

> Last synced: 2026-10-29

# Third-Party Notices

nim_duilib uses or distributes the following third-party components with its SDK.
The full license texts live in `licenses/` in this repository and in `LICENSES/`
inside the released SDK.

| Component | Usage | License | License file |
| --- | --- | --- | --- |
| Skia | rendering engine | BSD-3-Clause | `licenses/skia.LICENSE.txt` |
| Chromium Embedded Framework (CEF) | embedded browser | BSD-3-Clause | `licenses/libcef.LICENSE.txt` |
| SDL3 | windowing and input (optional) | zlib | `licenses/SDL.LICENSE.txt` |
| zlib | compression | zlib | `licenses/zlib.LICENSE.txt` |
| libpng | PNG codec | libpng License v2 | `licenses/libpng.LICENSE.txt` |
| libjpeg-turbo | JPEG codec | IJG + BSD-3-Clause | `licenses/libjpeg-turbo.LICENSE.md` |
| libwebp | WebP codec | BSD-3-Clause | `licenses/libwebp.LICENSE.txt` |
| giflib | GIF codec | MIT | `licenses/giflib.LICENSE.txt` |
| expat | XML parser (Skia SVG) | MIT | `licenses/expat.LICENSE.txt` |
| pugixml | XML parser | MIT | `licenses/pugixml.LICENSE.txt` |
| nanosvg | SVG parser | zlib | `licenses/nanosvg.LICENSE.txt` |
| stb_image | image decoding | MIT or Public Domain | `licenses/stb_image.LICENSE.txt` |
| udis86 | disassembler | BSD-2-Clause | `licenses/udis86.LICENSE.txt` |
| CxImage | image processing | see license file | `licenses/cximage.LICENSE.txt` |
| libpag | animation rendering (optional) | see license file | `licenses/libpag.LICENSE.txt` |
| Microsoft.Web.WebView2 | WebView2 control (Windows, optional) | BSD-3-Clause | `licenses/Microsoft.Web.WebView2.LICENSE.txt` |
| Roboto Mono | monospace font asset | Apache-2.0 | `licenses/RobotoMono.LICENSE.txt` |
| LLVM (clang toolchain) | Skia build toolchain | Apache-2.0 WITH LLVM-exception | `licenses/llvm.LICENSE.txt` |
| duilib | upstream project | BSD-3-Clause | `licenses/duilib.LICENSE.txt` |
| NIM_Duilib_Framework | project base | MIT | `licenses/NIM_Duilib_Framework.LICENSE.txt` |

## Sample animation and image assets

The animation and image test assets under `bin/resources` are demo-only and grant
no license of their own:

- `themes/default/public/animation/loading1..6.json` and
  `loading_progress1/2.json` (Lottie); `loading5.json` and `loading6.json` carry
  `meta.g = LottieFiles AE` and were exported by LottieFiles;
- `themes/default/render/lottie.json` and `pag_logo.pag` (added in commit
  `020fcfe0` on 2025-10-22 without recorded provenance or license);
- `themes/default/render/{fan.gif,gif_test.gif,gif_test2.gif,jpg_test.jpg,bmp_test_24.bmp,webp_test*.webp}`
  are image codec test files.

The remaining control icons (SVG/PNG) were added with feature work and are
treated as project-authored assets covered by the root `LICENSE` (MIT); the 144
upstream files imported on 2019-04-19 are covered by
`licenses/NIM_Duilib_Framework.LICENSE.txt` and `licenses/duilib.LICENSE.txt`.
Example web pages load Bootstrap 3.3.7, Font Awesome Free 4.7.0 and Tailwind from
CDNs at run time; they are not redistributed. Verify the provenance of the demo
assets or replace them before redistributing them in a commercial product.

## Notes

- This file is a summary only and does not modify any component license; the
  texts under `LICENSES/` are authoritative.
- Optional components (CEF, SDL3, WebView2, libpag) apply only when enabled and
  shipped.
- When redistributing the SDK, keep `LICENSES/`, this file and the Chinese
  counterpart `THIRD-PARTY-NOTICES.md`.
