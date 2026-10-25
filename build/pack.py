#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""nim_duilib 发布打包脚本。

从已构建好的产物目录收集文件，按SDK 契约打包为压缩包，不参与编译。

设计约束（改动前务必先读）：

1. 根 CMake 工程存在但没有 install()：SDK 还需带上游接入层、运行期 DLL、
   资源与合规文件，并按单架构、单配置命名，统一由本脚本收集与校验。
   因此这里不走 CPack，也不要试图"补 install() 规则"来替代本脚本。

2. 库目录只有一套布局（CM-5 D-2 方案 B），配置由目录表达，不再靠 `_d`
   后缀或多目录合并猜测：
   - Windows：lib/<x64|x86|arm64>/<Release|Debug>/
   - Unix   ：lib/<Release|Debug>/
   只读取 --config 选中的一个目录，不合并、不按优先级回退。
   VS 与 CMake 写入同一目录，约定"同一个 <arch>|<Config> 一次只由一套
   体系完整构建"，后构建者覆盖前者（含 .pdb）。

3. 头文件必须保持 `duilib/xxx.h` 的仓库根相对路径，因为下游工程的
   include 写法是 `#include "duilib/duilib.h"`（见 examples/basic/MainForm.h:5）。
   若改成 include/xxx.h，下游全部编译失败。

4. 对外头文件对 third_party 的依赖只有一处，必须一并转发，否则下游编译失败：
   - WebView2/*.h                  -> Microsoft.Web.WebView2/.../WebView2.h

   以下头文件只在库内部使用，duilib.h 的包含链不引用，因此不转发：
   - 图片解码实现（Image/ImageDecoder_*.h、Image_*.h、APngDecoder.h、
     FrameSequence_gif.h 等）；
   - 内部实现头文件（Core/ZipStreamIO.h、Core/ControlDropTarget*、
     Core/WindowDropTarget_*、Core/SDL_MacOS.h、MessageLoop_*、
     ProcessSingleton_*、RichEdit*_Windows 等）。
   随之 libpng / giflib / zlib / libjpeg-turbo 的头文件也不再转发。

   例外：Core/DragWindowFilter_SDL.h 被 Core/ControlDragable.h 在
   DUILIB_BUILD_FOR_SDL 下包含，非 Windows 平台必须转发（该宏恒为开），
   Windows 仅 --with-sdl 时转发。

   注意两类头文件按条件转发：
   - WebView2 只有 Windows 支持，非 Windows 平台不转发 WebView2/、
     duilib_webview2.h 与 third_party/Microsoft.Web.WebView2 的头文件；
   - CEF 相关头文件（CEFControl/、duilib_cef.h）仅在 --with-cef 时转发。

5. SDK 必须能独立支撑下游编译、链接、运行，因此除 5 个 duilib 库外还随包：
   - Skia 全部静态库（duilib 的必选链接依赖，整目录打包）；
   - WebView2 loader 库（Windows：MSVC 带静态 loader + 动态 loader 导入库 + DLL；
     MinGW 只带动态 loader 的导入库 + DLL，MSVC 静态 loader 不可链接）；
   - turbojpeg-static（仅 Windows MSVC：VS 默认启用 libjpeg-turbo，duilib.lib 有符号）；
   - SDL3（Unix 始终；Windows 需 --with-sdl；动态库时同时带运行期 DLL）；
   - CEF 开发包（仅 --with-cef：头文件 + libcef.lib/wrapper；Linux 无导入库，
     链接时直接使用 bin/libcef_linux/libcef.so）；
   - DLL 模式运行库（Windows：bin/duilib*.dll，lib/ 中为对应的导入库）。
   --with-cef 时同时收集 CEF 运行时（libcef.dll/libcef.so、*.pak、icudtl.dat、
   locales/）：Windows 保持 bin/libcef_win[_109]/<arch>/ 结构，Linux 为
   bin/libcef_linux/ 扁平结构；仓库中缺少该目录时只告警，包内只有开发文件，
   含 CEF 的示例程序不能直接运行，清单中会标注。macOS 的 CEF 运行时以
   .app 内嵌 Framework 提供，不自动收集，下游按发布文档手动部署。

6. Windows 工具链按库目录内容自动判断：存在 duilib.lib 为 MSVC，
   存在 libduilib*.a 为 MinGW。两者产物不同，不能混用：
   - MSVC ：库为 .lib，Skia 目录 llvm.<arch>.<release|debug>；
   - MinGW：静态库 .a、导入库 .dll.a，Skia 目录为仅有的 release 变体
            mingw64-gcc.<arch>.release / mingw64-llvm.<arch>.release（独立 MinGW-w64），
            或 gcc.<arch>.release / llvm.<arch>.release（MSYS2）。
   MinGW 的 Skia 目录按内容（存在 .a）判定：MSYS2 CLANG64 的 llvm.<arch>.release
   与 MSVC 目录同名，只看目录名无法区分，也会把 MSVC 的 .lib 误当 MinGW 库。
   因此 MinGW 的清单链接写法是 -lxxx，MSVC 是 xxx.lib。
   MinGW 还要区分 gcc（libstdc++）与 llvm-mingw（libc++），两者 C++ 运行时不兼容，
   库与 Skia 必须同源；包名追加 -mingw-gcc / -mingw-llvm，避免互相覆盖。

7. 包内还要提供“下游接入层”，由 packaging/ 下的模板渲染生成（不手工落盘）：
   - Windows+MSVC：msvc/nim_duilib.props、nim_duilib.targets、nim_duilib.sdk.props；
   - 全平台：lib/cmake/nim_duilib/*.cmake（find_package(nim_duilib)）。
   元数据（架构/配置/工具链/DLL/SDL/CEF/运行库/运行期 DLL）必须与实际
   收集结果一致，否则下游会链接到错误产物。

8. docs/ 只打包顶层的使用文档（Summary、Control、Theme、CEF 等）；
   misc/、specs/ 等子目录一律不打包。Release.*（发布流程）、
   Getting-Started.* / Examples.*（库内部文档）与感谢名单.txt 也不打包。
   Summary.* 作为包内文档索引，不包含指向包外目录的链接。
   文档按实际收集结果重写链接：../duilib/x.h -> ../include/duilib/x.h；../bin/resources/ 与包内一致
   （资源位于 bin/ 下），../../bin/resources/ 改为 ../bin/resources/；
   指向未随包内容的链接退化为纯文字。
   新增顶层文档默认随包，如需排除请加入 DOC_EXCLUDE_FILES。

包名：nim_duilib-<版本>-<系统>-<架构>[-mingw-<变体>][-sdl][-cef|-cef109][-examples][-dll][-debug]，
同一版本的不同特性包可共存于同一目录。
DLL 模式（运行期目录 bin/ 中有 duilib*.dll）自动追加 -dll，避免与静态包同名覆盖。

用法：
    python build/pack.py                      # Release SDK 包（不含示例）
    python build/pack.py --config Debug       # Debug SDK 包（包名带 -debug）
    python build/pack.py --with-examples      # SDK + 示例程序（包名带 -examples）
    python build/pack.py --list               # 只打印将收集的文件，不打包
    python build/pack.py --format tgz         # 强制 tgz（默认 Windows 用 zip）
    python build/pack.py --lib-dir lib/x86/Debug  # 显式指定库目录
    python build/pack.py --with-sdl           # Windows 额外打包 SDL3（包名带 -sdl）
    python build/pack.py --with-cef           # 打包 CEF 开发文件 + 运行时（包名带 -cef）
    python build/pack.py --msvc-runtime md    # 指定 MSVC 运行库（默认按模式推断）
    python build/pack.py --out-dir dist       # 指定输出目录
    python build/pack.py --allow-missing      # 关键产物缺失时仍出包（仅排查用）

发布流程请使用 build/release.py（check / package / finalize / clean）。

可复现构建：设置环境变量 SOURCE_DATE_EPOCH（秒）后，所有归档条目与 gzip 头
统一使用该时间；未设置时条目用源文件 mtime、生成文件用收集产物中的最新 mtime，
输入不变时连续两次打包的字节一致。

退出码：0 成功；1 参数或产物缺失；2 打包异常。
"""

import argparse
import gzip
import hashlib
import io
import os
import platform
import re
import stat
import struct
import sys
import tarfile
import time
import zipfile

# --------------------------------------------------------------------------
# 路径
# --------------------------------------------------------------------------

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
VERSION_HEADER = os.path.join(REPO_ROOT, 'duilib', 'duilib_version.h')
DEFAULT_OUT_DIR = os.path.join(REPO_ROOT, 'dist')

DUIBIB_SRC_DIR = os.path.join(REPO_ROOT, 'duilib')
THIRD_PARTY_DIR = os.path.join(DUIBIB_SRC_DIR, 'third_party')
WEBVIEW2_NATIVE_DIR = os.path.join(
    THIRD_PARTY_DIR, 'Microsoft.Web.WebView2', 'build', 'native')
JPEG_TURBO_DIR = os.path.join(THIRD_PARTY_DIR, 'libjpeg-turbo')
CEF_DIR = os.path.join(THIRD_PARTY_DIR, 'libcef')
RESOURCES_DIR = os.path.join(REPO_ROOT, 'bin', 'resources')
LICENSES_DIR = os.path.join(REPO_ROOT, 'licenses')
DOCS_DIR = os.path.join(REPO_ROOT, 'docs')
BIN_DIR = os.path.join(REPO_ROOT, 'bin')
PACKAGING_DIR = os.path.join(REPO_ROOT, 'packaging')
MSVC_MANIFEST_DIR = os.path.join(REPO_ROOT, 'msvc', 'manifest')

# 外部依赖（与仓库同级），可用命令行参数覆盖
SKIA_SRC_ROOT = os.path.join(os.path.dirname(REPO_ROOT), 'skia')
SDL3_SRC_ROOT = os.path.join(os.path.dirname(REPO_ROOT), 'SDL3')

# --------------------------------------------------------------------------
# 收集规则
# --------------------------------------------------------------------------

# 内部实现头文件（相对 duilib/），不对外转发。
# 判定依据：只被库内 .cpp / .mm 包含，任何对外头文件都不引用。
# - Core/ZipStreamIO.h 直接 include zlib 的 minizip 头文件，只被 ZipManager.cpp 使用；
# - DropTarget / SDL_MacOS 系列是 Windows / SDL 平台的具体实现，只被 .cpp / .mm 使用；
# - MessageLoop_* / NativeWindow 之外的平台适配（SDL_Linux、ProcessSingleton_*、
#   BitmapHelper_*、ApiWrapper_Windows、Macros_Windows、DiskUtils/DllManager/
#   InlineHook_Windows、RichEdit*_Windows/_SDL）同样只被 .cpp 包含。
INTERNAL_HEADER_FILES = (
    'Control/RichEditDragSource_Windows.h',
    'Control/RichEditDropTarget_SDL.h',
    'Control/RichEditDropTarget_Windows.h',
    'Control/RichEditHost_Windows.h',
    'Core/ControlDropTargetImpl_SDL.h',
    'Core/ControlDropTargetImpl_Windows.h',
    'Core/ControlDropTargetUtils.h',
    'Core/MessageLoop_SDL.h',
    'Core/MessageLoop_Windows.h',
    'Core/SDL_Linux.h',
    'Core/SDL_MacOS.h',
    'Core/WindowDropTarget_SDL.h',
    'Core/WindowDropTarget_Windows.h',
    'Core/ZipStreamIO.h',
    'Utils/ApiWrapper_Windows.h',
    'Utils/BitmapHelper_SDL.h',
    'Utils/BitmapHelper_Windows.h',
    'Utils/DiskUtils_Windows.h',
    'Utils/DllManager_Windows.h',
    'Utils/InlineHook_Windows.h',
    'Utils/Macros_Windows.h',
    'Utils/ProcessSingleton_Linux.h',
    'Utils/ProcessSingleton_MacOS.h',
    'Utils/ProcessSingleton_Windows.h',
)

# SDL 拖动过滤器：Core/ControlDragable.h（duilib.h 的包含链）在
# DUILIB_BUILD_FOR_SDL 下包含它。非 Windows 平台该宏始终定义（duilib_config.h），
# 因此必须转发；Windows 仅 --with-sdl 时转发。
SDL_HEADER_FILES = (
    'Core/DragWindowFilter_SDL.h',
)

# 随 SDK 打包的顶层文档：docs/ 下的一级文件，子目录（misc/、specs/）一律不打包。
# 以下文件属于库自身的维护文档，不适合发给下游：
# - Release.*：发布与打包流程（版本、签名、公证、构建脚本），面向维护者；
# - Getting-Started.* / Examples.*：库内部的快速上手与示例程序说明；
# - 感谢名单.txt：项目致谢，不是 SDK 使用文档。
# Summary.* 是文档索引，已移除指向包外目录的链接，随包提供。
DOC_EXCLUDE_FILES = frozenset((
    'Release.md',
    'Release.en.md',
    'Getting-Started.md',
    'Getting-Started.en.md',
    'Examples.md',
    'Examples.en.md',
    '感谢名单.txt',
))

# 图片解码实现相关的头文件（相对 duilib/）：只在库内部使用，不对外转发。
# duilib.h 只引用 Image/Image.h、ImageDecoder.h、ImageDecoderFactory.h 等基础
# 接口，包含链不涉及下列头文件（ImageDecoderFactory.h -> ImageUtil.h 属工具类，
# 不在此列表）。
IMAGE_DECODER_HEADER_FILES = (
    'Image/APngDecoder.h',
    'Image/FrameSequence_gif.h',
    'Image/Image_Animation.h',
    'Image/Image_Bitmap.h',
    'Image/Image_GIF.h',
    'Image/Image_ICO.h',
    'Image/Image_JPEG.h',
    'Image/Image_LOTTIE.h',
    'Image/Image_PAG.h',
    'Image/Image_PNG.h',
    'Image/Image_Svg.h',
    'Image/Image_WEBP.h',
    'Image/ImageDecoderUtil.h',
    'Image/ImageDecoder_Common.h',
    'Image/ImageDecoder_GIF.h',
    'Image/ImageDecoder_ICO.h',
    'Image/ImageDecoder_Icon.h',
    'Image/ImageDecoder_JPEG.h',
    'Image/ImageDecoder_LOTTIE.h',
    'Image/ImageDecoder_PAG.h',
    'Image/ImageDecoder_PNG.h',
    'Image/ImageDecoder_SVG.h',
    'Image/ImageDecoder_WEBP.h',
)

# WebView2 仅在 Windows 下支持，其他平台不转发该子树。
WEBVIEW2_INCLUDE_DIR = ('Microsoft.Web.WebView2', 'build', 'native', 'include')

# 平台/模块相关的对外头文件（相对 duilib/ 的路径）：
# WebView2 仅 Windows 转发；CEF 头文件仅 --with-cef 时转发。
WEBVIEW2_HEADER_DIRS = ('WebView2/',)
WEBVIEW2_HEADER_FILES = ('duilib_webview2.h',)
CEF_HEADER_DIRS = ('CEFControl/',)
CEF_HEADER_FILES = ('duilib_cef.h',)

# 静态库白名单（D-1/D-2 统一命名后的唯一一套）。
LIB_BASENAMES = (
    'duilib',
    'duilib-cximage',
    'duilib-png',
    'duilib-webp',
    'duilib-zlib',
)

# MinGW 的两种编译器：静态库绑定不同 C++ 运行时，产物不可互换。
MINGW_VARIANTS = ('gcc', 'llvm')

# CEF wrapper / 源码目录：--with-cef 时按版本只收集一套。
CEF_WRAPPER_BY_VERSION = {
    'latest': 'libcef_dll_wrapper',
    '109': 'libcef_dll_wrapper_109',
}
CEF_SRC_BY_VERSION = {
    'latest': 'libcef_win',
    '109': 'libcef_win_109',
}


def cef_src_name(sys_tag, cef_version):
    """CEF 源码目录名：Windows 按版本区分，Linux 只有一套。"""
    if sys_tag == 'linux':
        return 'libcef_linux'
    return CEF_SRC_BY_VERSION[cef_version]


# CEF 运行时目录：仓库 bin/<源码目录>/<VS 平台目录>/（Windows）。
# CefManager 按“可执行文件目录 + 该子目录”加载 libcef.dll
# （见 duilib/CEFControl/CefManager_Windows.cpp: AddCefDllToPath），
# 所以包内必须保持同样的相对结构，不能把运行时摊平到 bin/。
CEF_RUNTIME_ARCH_DIRS = {'x64': 'x64', 'x86': 'Win32'}

# 随 Windows 包分发的 VS manifest 架构。x64 主机上也可能开发 x86/arm64 程序，
# 三份都随包，由 props 按 Platform 选择。
WINDOWS_MANIFEST_ARCHES = ('x64', 'x86', 'arm64')

# CEF 运行时目录里混有开发用文件：libcef.lib 导入库已在 lib/ 下提供，
# 109 版本的 cef_sandbox.lib 是 80 MB 级的链接库且本项目不使用沙箱，均不随包。
CEF_RUNTIME_EXCLUDE_SUFFIXES = (
    '.lib', '.pdb', '.ilk', '.exp', '.obj', '.o',
)

# 下游链接 Skia 的推荐顺序（与 example 工程一致）；其余 4 个是 Skia 的
# 传递依赖，整目录随包，出现未解析符号时补链。
SKIA_LINK_ORDER = ('skia', 'svg', 'skshaper', 'skottie', 'sksg', 'jsonreader')
SKIA_EXTRA_LIBS = ('bentleyottmann', 'expat', 'skcms', 'skresources')

# 明确排除的文件扩展名：调试符号、中间产物、动态库。
# 注意：.a 是 Unix 静态库，不能排除（P4-B1）。
EXCLUDE_LIB_SUFFIXES = (
    '.pdb', '.ilk', '.exp', '.idb', '.obj', '.o', '.so', '.dylib',
)

# 运行时 dll：示例程序运行必需（非 CEF 部分）。
INCLUDE_RUNTIME_DLLS = (
    'WebView2Loader.dll',
)

# bin/resources 的排除项。resources.zip 是 748 字节的空壳，不打包。
EXCLUDED_RESOURCE_FILES = (
    'resources.zip',
)

# --with-examples 关闭时，bin/resources/themes/default/ 下只保留 public 子目录
# 与 global.xml（global.xml 是框架启动必需，见 GlobalManager.cpp:440，
# 缺失会直接 return false 导致初始化失败）。
EXAMPLE_THEME_DIR = os.path.join('themes', 'default')
EXAMPLE_THEME_KEEP_FILES = ('global.xml',)
EXAMPLE_THEME_KEEP_DIRS = ('public',)


# --------------------------------------------------------------------------
# 工具
# --------------------------------------------------------------------------

def eprint(*args):
    sys.stderr.write(' '.join(str(a) for a in args) + '\n')


_VERSION_MACROS = ('MAJOR', 'MINOR', 'PATCH', 'BUILD')
_VERSION_RE = re.compile(
    r'^\s*#\s*define\s+DUILIB_VER_(MAJOR|MINOR|PATCH|BUILD)\s+(\d+)\b')


def read_version():
    """从 duilib/duilib_version.h 解析版本号，返回 MAJOR.MINOR.PATCH.BUILD。

    该头文件是版本的唯一编辑点，后续可能加入版权、ABI 等其它内容，
    因此只逐行匹配四个版本宏，忽略注释、#undef 与其它宏。
    BUILD 段按 MMDD 语义书写但不补零（0105 在 C/RC 中是八进制）。
    """
    if not os.path.isfile(VERSION_HEADER):
        eprint('[ERROR] 找不到版本头文件：%s' % VERSION_HEADER)
        return None
    values = {}
    duplicated = set()
    try:
        with io.open(VERSION_HEADER, 'r', encoding='utf-8-sig') as f:
            for line in f:
                match = _VERSION_RE.match(line)
                if match is None:
                    continue
                name, value = match.group(1), int(match.group(2))
                if name in values:
                    duplicated.add(name)
                else:
                    values[name] = value
    except OSError as exc:
        eprint('[ERROR] 读取版本头文件失败：%s' % exc)
        return None
    if duplicated:
        eprint('[ERROR] 版本宏重复定义：%s（%s）'
               % (', '.join('DUILIB_VER_' + name for name in sorted(duplicated)),
                  VERSION_HEADER))
        return None
    missing = [name for name in _VERSION_MACROS if name not in values]
    if missing:
        eprint('[ERROR] 版本宏缺失：%s（%s）'
               % (', '.join('DUILIB_VER_' + name for name in missing),
                  VERSION_HEADER))
        return None
    for name in _VERSION_MACROS:
        if values[name] > 65535:
            eprint('[ERROR] 版本宏超出范围（0..65535）：DUILIB_VER_%s=%s'
                   % (name, values[name]))
            return None
    return '.'.join(str(values[name]) for name in _VERSION_MACROS)


def is_windows_host():
    """当前 Python 是否运行在 Windows 上（含 MSYS2 / Cygwin 的 POSIX 模拟层）。

    MSYS2 的 MSYS Python 中 platform.system() 返回 MSYS_NT-*（sys.platform
    为 msys），只判断 'win' 开头会把打包链路误判为 Linux；原生 Windows
    Python 与 mingw-w64 Python 返回 Windows。
    """
    return platform.system().lower().startswith(('win', 'msys', 'cygwin'))


def detect_dll_mode():
    """bin/ 中存在 duilib*.dll 时，本次构建为 DLL 模式（MSVC /MD 动态运行库）。

    DLL 模式需要同样使用 /MD 编译的 Skia，目录名带 .md（Release）/ .mdd（Debug）。
    """
    if not os.path.isdir(BIN_DIR):
        return False
    for name in os.listdir(BIN_DIR):
        if name.lower().startswith('duilib') and name.lower().endswith('.dll'):
            return True
    return False


def detect_platform_tag():
    """返回 (系统标签, 架构标签)，用于包名。"""
    sysname = platform.system().lower()
    if is_windows_host():
        sys_tag = 'win'
    elif sysname == 'darwin':
        sys_tag = 'macos'
    elif sysname == 'freebsd':
        sys_tag = 'freebsd'
    else:
        # Linux 发行版名不做细分：Skia 静态库按 llvm.x64.release 编译，
        # 跨发行版可用的前提是 glibc 版本兼容，写死发行版名反而误导。
        sys_tag = 'linux'

    machine = platform.machine().lower()
    if machine in ('amd64', 'x86_64'):
        arch_tag = 'x64'
    elif machine in ('i386', 'i686', 'x86'):
        arch_tag = 'x86'
    elif machine in ('aarch64', 'arm64'):
        arch_tag = 'arm64'
    else:
        arch_tag = machine or 'unknown'
    return sys_tag, arch_tag


def build_package_name(version, sys_tag, arch_tag, config,
                       with_sdl, with_cef, cef_version, with_examples,
                       mingw_variant=None, is_dll=False):
    """包名：nim_duilib-<版本>-<系统>-<架构>[-mingw-<变体>][-sdl][-cef|-cef109][-examples][-dll][-debug]。

    同一版本的不同特性包必须能共存于 dist/，否则会互相覆盖。
    MinGW 的 gcc 与 llvm-mingw 产物绑定不同 C++ 运行时（libstdc++ / libc++），
    不可互换，包名必须带 -mingw-gcc / -mingw-llvm 以便同时发布。
    DLL 模式与静态库模式（duilib.dll + 导入库 vs 纯静态库）同样不可互换，
    包名追加 -dll。
    Unix 默认带 SDL，不追加 -sdl，避免改变既有 Unix 包名。
    """
    name = 'nim_duilib-%s-%s-%s' % (version, sys_tag, arch_tag)
    if sys_tag == 'win' and mingw_variant:
        name += '-mingw-%s' % mingw_variant
    if with_sdl and sys_tag == 'win':
        name += '-sdl'
    if with_cef:
        name += '-cef109' if cef_version == '109' else '-cef'
    if with_examples:
        name += '-examples'
    if is_dll:
        name += '-dll'
    if config == 'Debug':
        name += '-debug'
    return name


def human_size(num_bytes):
    for unit in ('B', 'KB', 'MB', 'GB'):
        if abs(num_bytes) < 1024.0 or unit == 'GB':
            return '%.1f %s' % (num_bytes, unit)
        num_bytes /= 1024.0
    return '%.1f GB' % num_bytes


def sha256_of(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


# PE 文件头里的 Machine 字段值（IMAGE_FILE_MACHINE_*）。
PE_MACHINE_BY_ARCH = {'x86': 0x014c, 'x64': 0x8664, 'arm64': 0xaa64}


def pe_machine(path):
    """读取 PE 文件的 Machine 字段；不是 PE 或读取失败时返回 None。"""
    try:
        with open(path, 'rb') as fp:
            if fp.read(2) != b'MZ':
                return None
            fp.seek(0x3C)
            offset = struct.unpack('<I', fp.read(4))[0]
            fp.seek(offset)
            if fp.read(4) != b'PE\0\0':
                return None
            return struct.unpack('<H', fp.read(2))[0]
    except (OSError, struct.error):
        return None


def iter_files(root):
    """遍历 root 下的文件与符号链接，返回 (绝对路径, 相对 root 的路径)。

    os.walk 默认不跟随目录链接，链接目录本身也不会出现在结果里；
    这里显式补上，否则 macOS 的 `Versions/Current` 之类链接目录会丢失。
    """
    for dirpath, dirnames, filenames in os.walk(root):
        for name in dirnames:
            abs_path = os.path.join(dirpath, name)
            if os.path.islink(abs_path):
                yield abs_path, os.path.relpath(abs_path, root).replace(os.sep, '/')
        for name in filenames:
            abs_path = os.path.join(dirpath, name)
            yield abs_path, os.path.relpath(abs_path, root).replace(os.sep, '/')


# --------------------------------------------------------------------------
# 收集
# --------------------------------------------------------------------------

class Collector(object):
    """收集结果容器。items 为 (包内相对路径, 磁盘绝对路径)。"""

    def __init__(self, allow_missing=False):
        self.items = []
        self._seen = set()
        self.warnings = []
        # 关键产物缺失：默认阻止出包；allow_missing 仅供排查时降级为告警。
        self.allow_missing = allow_missing
        self.critical = []

    def add(self, rel_path, abs_path):
        rel = rel_path.replace(os.sep, '/').lstrip('/')
        if rel in self._seen:
            return False
        if not (os.path.isfile(abs_path) or os.path.islink(abs_path)):
            return False
        self._seen.add(rel)
        self.items.append((rel, abs_path))
        return True

    def warn(self, msg, critical=False):
        self.warnings.append(msg)
        if critical and not self.allow_missing:
            self.critical.append(msg)

    def total_size(self):
        # 符号链接取链接本身的大小，不能用 getsize（会跟随到目标，重复计数）
        return sum(os.lstat(p).st_size for _rel, p in self.items)

    def sort(self):
        self.items.sort(key=lambda kv: kv[0])


def collect_headers(collector, with_cef, with_sdl, is_windows):
    """收集对外头文件 + 必需的 third_party 头文件。

    路径前缀必须是 `include/duilib/`，不能是 `include/`——
    下游的写法是 `#include "duilib/duilib.h"`（见 examples/basic/MainForm.h:5），
    所以包内必须保留仓库根的相对路径层级。少了这一层下游直接编译失败。

    WebView2 头文件仅 Windows 转发；CEF 头文件仅 --with-cef 时转发；
    图片解码实现头文件与内部实现头文件（IMAGE_DECODER_HEADER_FILES /
    INTERNAL_HEADER_FILES）不转发；SDL_HEADER_FILES 仅在 Windows 未启用
    --with-sdl 时排除。
    """
    excluded_dirs = []
    excluded_files = list(INTERNAL_HEADER_FILES) + list(IMAGE_DECODER_HEADER_FILES)
    if is_windows and not with_sdl:
        excluded_files.extend(SDL_HEADER_FILES)
    if not is_windows:
        excluded_dirs.extend(WEBVIEW2_HEADER_DIRS)
        excluded_files.extend(WEBVIEW2_HEADER_FILES)
    if not with_cef:
        excluded_dirs.extend(CEF_HEADER_DIRS)
        excluded_files.extend(CEF_HEADER_FILES)

    count = 0
    for abs_path, rel in iter_files(DUIBIB_SRC_DIR):
        if not rel.endswith(('.h', '.hpp', '.inl')):
            continue
        if rel.startswith('third_party/'):
            continue
        # RenderSkia 是渲染后端实现细节，duilib.h 不引用它，不对外暴露。
        if rel.startswith('RenderSkia/'):
            continue
        if rel in excluded_files or any(rel.startswith(d) for d in excluded_dirs):
            continue
        if collector.add('include/duilib/' + rel, abs_path):
            count += 1

    # third_party 头文件：只有 WebView2 需要转发（且仅 Windows）
    if is_windows:
        parts = WEBVIEW2_INCLUDE_DIR
        root = os.path.join(DUIBIB_SRC_DIR, 'third_party', *parts)
        if not os.path.isdir(root):
            collector.warn('third_party/%s 不存在，头文件可能不完整'
                           % os.path.join(*parts))
        else:
            prefix = 'include/duilib/third_party/' + '/'.join(parts)
            for abs_path, rel in iter_files(root):
                if not rel.endswith(('.h', '.hpp', '.inl')):
                    continue
                if collector.add('%s/%s' % (prefix, rel), abs_path):
                    count += 1

    if not any(rel == 'include/duilib/duilib.h'
               for rel, _abs in collector.items):
        collector.warn('缺少核心头文件 include/duilib/duilib.h（源目录：%s）'
                       % _rel(DUIBIB_SRC_DIR), critical=True)

    return count


class PackageError(Exception):
    """打包前置条件不满足（目录/库缺失等），由 main 统一转为退出码 1。"""


def _rel(path):
    """转成相对仓库根的展示路径。"""
    try:
        return os.path.relpath(path, REPO_ROOT).replace(os.sep, '/')
    except ValueError:
        return path


def _lib_base_name(stem, wanted):
    """归一化库文件名主干后匹配白名单。

    - Unix/MinGW：libduilib-zlib.a -> duilib-zlib；
    - MinGW 导入库：libduilib.dll.a -> duilib；
    - MSVC：duilib.lib -> duilib。
    不在白名单中的名字原样返回，由调用方忽略。
    """
    names = [stem]
    if stem.lower().endswith('.dll'):
        names.append(stem[:-4])
    for name in names:
        if name.startswith('lib') and name[3:] in wanted:
            return name[3:]
        if name in wanted:
            return name
    return stem


def detect_windows_toolchain(lib_dir):
    """根据库目录内容判断 Windows 工具链：'msvc' / 'mingw' / None。

    用于区分 .lib（MSVC）与 .a（MinGW）产物，并据此选择 Skia 输出目录。
    """
    if not is_windows_host():
        return None
    if os.path.isfile(os.path.join(lib_dir, 'duilib.lib')):
        return 'msvc'
    for name in os.listdir(lib_dir):
        low = name.lower()
        if low.startswith('libduilib') and low.endswith('.a'):
            return 'mingw'
    return None


def detect_mingw_variant(lib_dir):
    """判断 MinGW 静态库由 gcc 还是 llvm-mingw 编译：'gcc' / 'llvm' / None。

    gcc 的 MinGW 使用 libstdc++（符号含 _ZNSt7__cxx11 / _ZNSt6vector），
    llvm-mingw 默认使用 libc++（符号含 _ZNSt3__1）。两者 ABI 不兼容，
    打包时必须与 Skia 库使用同一套工具链。
    """
    if not os.path.isdir(lib_dir):
        return None
    for name in sorted(os.listdir(lib_dir)):
        low = name.lower()
        if not (low.startswith('libduilib') and low.endswith('.a')):
            continue
        with open(os.path.join(lib_dir, name), 'rb') as f:
            data = f.read()
        if b'_ZNSt3__1' in data:
            return 'llvm'
        if (b'_ZNSt7__cxx11' in data or b'_ZNSt6vector' in data
                or b'_ZNSt12basic_string' in data):
            return 'gcc'
    return None


def resolve_mingw_variant(lib_dir, requested='auto'):
    """确定 MinGW 变体；显式指定与库内特征冲突时直接失败。"""
    detected = detect_mingw_variant(lib_dir)
    if requested and requested != 'auto':
        if detected and detected != requested:
            raise PackageError(
                '库目录 %s 看起来是 %s 编译的，与 --mingw-variant=%s 不一致'
                % (_rel(lib_dir), 'llvm-mingw' if detected == 'llvm' else 'gcc',
                   requested))
        return requested
    if detected is None:
        raise PackageError(
            '无法从 %s 判断 MinGW 编译器（gcc / llvm-mingw），'
            '请用 --mingw-variant 指定' % _rel(lib_dir))
    return detected


def _scan_lib_dir(lib_dir):
    """扫描唯一库目录，返回 {基名: 文件名}。

    配置只由目录决定（Release/Debug），文件名不再参与判断（D-2）。
    Windows 同时接受 MSVC 的 .lib 与 MinGW 的 .a（含 .dll.a 导入库）；
    Unix 只接受 .a。
    """
    is_windows = is_windows_host()
    allowed_exts = ('.lib', '.a') if is_windows else ('.a',)
    wanted = set(LIB_BASENAMES) | set(CEF_WRAPPER_BY_VERSION.values())
    available = {}
    for name in sorted(os.listdir(lib_dir)):
        full = os.path.join(lib_dir, name)
        if not os.path.isfile(full):
            continue
        if name.lower().endswith(EXCLUDE_LIB_SUFFIXES):
            continue
        stem, ext = os.path.splitext(name)
        ext = ext.lower()
        if ext not in allowed_exts:
            continue
        base = _lib_base_name(stem, wanted)
        if base in wanted and base not in available:
            available[base] = name
    return available


def collect_libs(collector, lib_dir, config, with_cef, cef_version):
    """按白名单收集唯一库目录中的静态库。

    5 个基础库（LIB_BASENAMES）任一缺失直接失败：缺少任何一个，
    下游链接 duilib 时都会失败，不应产出残缺 SDK。
    配置由目录决定，不再做 _d 后缀或多目录合并判断。
    """
    available = _scan_lib_dir(lib_dir)

    if with_cef:
        wrapper = CEF_WRAPPER_BY_VERSION[cef_version]
        if wrapper not in available:
            raise PackageError('启用了 --with-cef，但库目录中缺少 %s（目录：%s）'
                               % (wrapper, _rel(lib_dir)))
    else:
        for base in CEF_WRAPPER_BY_VERSION.values():
            name = available.get(base)
            if name is not None:
                full = os.path.join(lib_dir, name)
                collector.warn('存在 CEF 库 %s（%s），未打包；需要请加 --with-cef'
                               % (name, human_size(os.path.getsize(full))))

    wanted = list(LIB_BASENAMES)
    if with_cef:
        wanted.append(CEF_WRAPPER_BY_VERSION[cef_version])

    missing_base = [b for b in LIB_BASENAMES if b not in available]
    if missing_base:
        raise PackageError('缺少基础库：%s（目录：%s），请先完成 %s 编译'
                           % (', '.join(missing_base), _rel(lib_dir), config))

    count = 0
    for base in wanted:
        name = available.get(base)
        if name is None:
            continue
        if collector.add('lib/' + name, os.path.join(lib_dir, name)):
            count += 1
    return count


def _collect_tree(collector, src_root, dst_prefix):
    """把 src_root 下所有文件按相对路径转发到 dst_prefix。"""
    count = 0
    if not os.path.isdir(src_root):
        return count
    for abs_path, rel in iter_files(src_root):
        if collector.add('%s/%s' % (dst_prefix, rel), abs_path):
            count += 1
    return count


def _skia_lib_exts(toolchain=None):
    """Skia 目录中可接受的静态库扩展名。

    MinGW 只能用 .a，MSVC 只能用 .lib；工具链未知时 Windows 两者都接受
    （与历史行为一致），Unix 只接受 .a。
    """
    if toolchain == 'mingw':
        return ('.a',)
    if toolchain == 'msvc':
        return ('.lib',)
    if is_windows_host():
        return ('.lib', '.a')
    return ('.a',)


def _skia_dir_has_libs(path, toolchain=None):
    """目录存在且包含该工具链可接受的静态库文件。"""
    if not os.path.isdir(path):
        return False
    exts = _skia_lib_exts(toolchain)
    for name in os.listdir(path):
        if os.path.isfile(os.path.join(path, name)) and name.lower().endswith(exts):
            return True
    return False


def collect_skia_libs(collector, skia_lib_dir, toolchain=None):
    """收集 Skia 全部静态库（P4-B3：整目录打包）。

    按工具链过滤扩展名：MinGW 只收 .a，MSVC 只收 .lib。MSYS2 CLANG64 的
    llvm.<arch>.release 与 MSVC 目录同名，若两者文件混放也不会打错库。
    """
    exts = _skia_lib_exts(toolchain)
    count = 0
    for name in sorted(os.listdir(skia_lib_dir)):
        full = os.path.join(skia_lib_dir, name)
        if not os.path.isfile(full) or not name.lower().endswith(exts):
            continue
        if collector.add('lib/' + name, full):
            count += 1
    if count == 0:
        raise PackageError('Skia 目录中没有 %s 静态库：%s'
                           % ('/'.join(exts), _rel(skia_lib_dir)))
    return count


def _msys2_env_name():
    """当前运行的 Python 所属 MSYS2 环境名（mingw64/ucrt64/clang64/...）。

    MSYS2 的 MINGW64 使用 msvcrt，而 UCRT64 使用 ucrt：两者编译出的 gcc 静态库
    引用的 CRT 符号不同，不能混用。MINGW64 的 Skia 目录因此带 .msvcrt 后缀。
    非 MSYS2 环境返回空字符串。
    """
    prefix = os.path.basename(os.path.dirname(os.path.dirname(sys.executable)))
    name = prefix.lower()
    if name in ('mingw64', 'ucrt64', 'clang64', 'clangarm64'):
        return name
    return os.environ.get('MSYSTEM', '').lower()


def resolve_skia_lib_dir(explicit_dir, config, arch_tag, toolchain=None,
                         mingw_variant=None, dll_mode=False):
    """解析 Skia 库目录。显式指定优先，否则按平台/配置/工具链探测。

    MinGW 的 gcc 与 llvm-mingw（以及 MSYS2 的 gcc/clang）各自使用不同的 Skia
    静态库，不能混用。目录名分两种前缀：
      独立 MinGW-w64 : mingw64-gcc.<arch>.release / mingw64-llvm.<arch>.release
      MSYS2          : gcc.<arch>.release / llvm.<arch>.release
      MSYS2 MINGW64  : gcc.msvcrt.<arch>.release（msvcrt，与 UCRT64 的 gcc 产物不兼容）
    MSVC DLL 模式使用 /MD 动态运行库，必须链接同 CRT 的 Skia：
      llvm.<arch>.release.md / llvm.<arch>.debug.mdd
    MSYS2 的 llvm.<arch>.release 与 MSVC 目录同名，因此除目录名外还要检查目录
    内容（.a / .lib），否则会把 MSVC 的 Skia 打进 MinGW 包。
    """
    if explicit_dir:
        path = explicit_dir if os.path.isabs(explicit_dir) \
            else os.path.join(REPO_ROOT, explicit_dir)
        if not os.path.isdir(path):
            raise PackageError('--skia-lib-dir 不存在：%s' % path)
        if toolchain in ('mingw', 'msvc') and not _skia_dir_has_libs(path, toolchain):
            raise PackageError(
                '--skia-lib-dir 中没有 %s 静态库，与 %s 工具链不匹配：%s'
                % ('/'.join(_skia_lib_exts(toolchain)), toolchain, path))
        if toolchain == 'mingw' and mingw_variant:
            base = os.path.basename(os.path.normpath(path))
            match = re.match(r'(?:mingw64-)?(gcc|llvm)\.', base)
            if match and match.group(1) != mingw_variant:
                raise PackageError(
                    '--skia-lib-dir 是 %s 的产物，与 MinGW 编译器 %s 不匹配：%s'
                    % (match.group(1), mingw_variant, path))
        return path

    out_dir = os.path.join(SKIA_SRC_ROOT, 'out')
    is_windows = is_windows_host()
    if is_windows:
        # MSVC：llvm.<arch>.debug / release（MSYS2 CLANG64 也用这个目录名，按内容区分）
        msvc_name = 'llvm.%s.%s' % (
            arch_tag, 'debug' if config == 'Debug' else 'release')
        if dll_mode:
            # DLL 模式：duilib 用 /MD，必须匹配同 CRT 的 Skia 变体
            msvc_name += '.mdd' if config == 'Debug' else '.md'
        # MinGW：Skia 只有 release 变体；独立 MinGW-w64 带 mingw64- 前缀，MSYS2 无前缀
        mingw_names = []
        for variant in MINGW_VARIANTS:
            mingw_names.append('mingw64-%s.%s.release' % (variant, arch_tag))
            mingw_names.append('%s.%s.release' % (variant, arch_tag))
        # MSYS2 MINGW64 的 gcc 产物使用 msvcrt，目录名带 .msvcrt 后缀
        msvcrt_name = 'gcc.msvcrt.%s.release' % arch_tag
        if toolchain == 'mingw':
            if mingw_variant in MINGW_VARIANTS:
                # 同一变体的独立 MinGW 与 MSYS2 目录可能同时存在，优先独立 MinGW 命名；
                # 需要强制使用 MSYS2 产物时用 --skia-lib-dir 指定
                names = ['mingw64-%s.%s.release' % (mingw_variant, arch_tag),
                         '%s.%s.release' % (mingw_variant, arch_tag)]
                if mingw_variant == 'gcc':
                    # MINGW64 环境下优先 msvcrt 目录；其他环境放在最后，
                    # 只有前两个目录都不存在时才会选中，避免误用 msvcrt 库
                    if _msys2_env_name() == 'mingw64':
                        names.insert(0, msvcrt_name)
                    else:
                        names.append(msvcrt_name)
            else:
                names = mingw_names + [msvcrt_name]
        elif toolchain == 'msvc':
            names = [msvc_name]
        else:
            names = [msvc_name] + mingw_names + [msvcrt_name]
        candidates = [os.path.join(out_dir, name) for name in names]
    else:
        candidates = [
            os.path.join(out_dir, 'llvm.%s.release' % arch_tag),
            os.path.join(out_dir, 'gcc.%s.release' % arch_tag),
        ]

    # 按目录内容过滤：目录名相同但库类型不符的候选（如 MSVC 的 .lib）直接跳过
    for path in candidates:
        if _skia_dir_has_libs(path, toolchain):
            return path

    available = []
    if os.path.isdir(out_dir):
        available = sorted(d for d in os.listdir(out_dir)
                           if os.path.isdir(os.path.join(out_dir, d)))
    raise PackageError('未找到 Skia 库目录（尝试过：%s；可用：%s）。'
                       '可用 --skia-lib-dir 指定'
                       % (', '.join(candidates),
                          ', '.join(available) if available else '无'))


def collect_sdl(collector, sdl_dir, config, with_sdl):
    """收集 SDL3 库、头文件与许可证（P4-B3/B4）。

    Windows 默认不打包，仅 --with-sdl 时收集；Unix 始终收集。
    """
    is_windows = is_windows_host()
    if is_windows and not with_sdl:
        return 0

    root = sdl_dir if sdl_dir else SDL3_SRC_ROOT
    if sdl_dir and not os.path.isabs(sdl_dir):
        root = os.path.join(REPO_ROOT, sdl_dir)
    if not os.path.isdir(root):
        raise PackageError('SDL3 目录不存在：%s（可用 --sdl-dir 指定）' % root)

    count = 0
    if is_windows:
        # 按工具链产物依次探测：MSVC 静态 SDL3-static.lib；MSVC 动态 SDL3.lib；
        # MinGW 动态导入库 libSDL3.dll.a；MinGW 静态库 libSDL3.a。
        lib_root = os.path.join(root, 'lib')
        candidates = (
            ('SDL3-static.lib', False),
            ('SDL3.lib', True),
            ('libSDL3.dll.a', True),
            ('libSDL3.a', False),
        )
        chosen = None
        for name, needs_dll in candidates:
            full = os.path.join(lib_root, name)
            if os.path.isfile(full):
                chosen = (name, needs_dll, full)
                break
        if chosen is None:
            raise PackageError('启用了 --with-sdl，但 %s 下没有 SDL3 静态库或导入库'
                               '（尝试过：%s）'
                               % (_rel(lib_root),
                                  ', '.join(n for n, _ in candidates)))
        name, needs_dll, full = chosen
        if collector.add('lib/' + name, full):
            count += 1
        # 动态导入库必须有运行期 DLL，否则下游能链接但运行失败。
        if needs_dll:
            dll = os.path.join(root, 'bin', 'SDL3.dll')
            if os.path.isfile(dll):
                if collector.add('bin/SDL3.dll', dll):
                    count += 1
            else:
                collector.warn('缺少 SDL3 运行期 DLL：%s（下游运行会失败）'
                               % _rel(dll), critical=True)
    else:
        lib_dirs = [os.path.join(root, 'lib64'), os.path.join(root, 'lib')]
        found = False
        for lib_dir in lib_dirs:
            if not os.path.isdir(lib_dir):
                continue
            for name in sorted(os.listdir(lib_dir)):
                full = os.path.join(lib_dir, name)
                if not os.path.isfile(full):
                    continue
                low = name.lower()
                if not (low.startswith('libsdl3') and
                        ('.so' in low or low.endswith('.dylib'))):
                    continue
                if collector.add('lib/' + name, full):
                    count += 1
                found = True
            if found:
                break
        if not found:
            raise PackageError('SDL3 动态库不存在（查找 %s）' % ', '.join(lib_dirs))

    count += _collect_tree(collector, os.path.join(root, 'include', 'SDL3'),
                           'include/SDL3')
    # SDL3 的 CMake 安装把许可证放在 share/licenses/SDL3；兼容旧布局 licenses/SDL3
    lic_root = os.path.join(root, 'licenses')
    if not os.path.isdir(os.path.join(lic_root, 'SDL3')):
        alt_root = os.path.join(root, 'share', 'licenses')
        if os.path.isdir(os.path.join(alt_root, 'SDL3')):
            lic_root = alt_root
    count += _collect_tree(collector, os.path.join(lic_root, 'SDL3'),
                           'LICENSES/SDL3')
    return count


def collect_webview2_loader(collector, arch_tag, toolchain=None):
    """收集 WebView2 loader 库（默认随包，P4-B4/Q11）。

    MinGW 只收动态 loader 的导入库（WebView2Loader.dll.lib，GNU ld 可用）；
    WebView2LoaderStatic.lib 是 MSVC 静态库，MinGW 无法链接，不随包。
    """
    if not is_windows_host():
        return 0
    arch_dir = {'x64': 'x64', 'x86': 'x86'}.get(arch_tag)
    count = 0
    if arch_dir is None:
        collector.warn('WebView2 没有 %s 架构的 loader，已跳过' % arch_tag)
        return count
    root = os.path.join(WEBVIEW2_NATIVE_DIR, arch_dir)
    lib_names = (('WebView2Loader.dll.lib',) if toolchain == 'mingw'
                 else ('WebView2Loader.dll.lib', 'WebView2LoaderStatic.lib'))
    for name in lib_names:
        full = os.path.join(root, name)
        if os.path.isfile(full):
            if collector.add('lib/' + name, full):
                count += 1
        else:
            collector.warn('缺少 WebView2 loader：%s' % _rel(full))
    # 动态 loader 的运行期 DLL：SDK-only 包也必须自带，否则 CMake 构建的下游
    # 在运行期找不到 WebView2Loader.dll。
    dll = os.path.join(root, 'WebView2Loader.dll')
    if os.path.isfile(dll):
        if collector.add('bin/WebView2Loader.dll', dll):
            count += 1
    else:
        collector.warn('缺少 WebView2 loader：%s' % _rel(dll))
    return count


def collect_duilib_runtime_dll(collector, arch_tag, config):
    """收集 DLL 模式下的 duilib 运行库（bin/duilib*.dll）。

    命名由 msvc/PropertySheets/DuilibSettings.props 决定：
    x64/arm64 为 duilib[_d].dll，x86（VS 的 Win32）为 duilib32[_d].dll。
    此时 lib/ 下打包的是同名导入库，下游运行必须带上这个 DLL。
    静态库模式（含 CMake 构建）没有该文件，返回 0，不告警。
    """
    if not is_windows_host():
        return 0
    base = 'duilib32' if arch_tag == 'x86' else 'duilib'
    if config == 'Debug':
        base += '_d'
    full = os.path.join(BIN_DIR, base + '.dll')
    if not os.path.isfile(full):
        return 0
    # 交叉出包（例如在 x64 主机的 bin/ 里已有 DLL 时打 arm64 包）必须校验
    # 实际架构，否则会把主机架构的 DLL 静默放进目标架构的包。
    machine = pe_machine(full)
    expected = PE_MACHINE_BY_ARCH.get(arch_tag)
    if machine is not None and expected is not None and machine != expected:
        collector.warn('bin/%s.dll 的架构（0x%04X）与目标 %s 不匹配，已跳过'
                       % (base, machine, arch_tag))
        return 0
    return 1 if collector.add('bin/%s.dll' % base, full) else 0


def collect_jpeg_turbo(collector, arch_tag, toolchain=None):
    """收集 libjpeg-turbo 静态库（仅 Windows MSVC，P4-B4/Q12）。

    头文件只在库内部使用，不对外转发（图片解码头文件不打包）。
    预编译的 turbojpeg-static.lib 是 MSVC 静态库，MinGW 无法链接；MinGW
    构建也不使用它，因此 MinGW 包不收集，避免给下游一个不可用的库。
    """
    if not is_windows_host():
        return 0
    if toolchain == 'mingw':
        return 0
    arch_dir = {'x64': 'x64', 'x86': 'x86', 'arm64': 'arm64'}.get(arch_tag)
    count = 0
    if arch_dir is None:
        collector.warn('libjpeg-turbo 上游没有 Windows %s 预编译库，已跳过' % arch_tag)
        return count
    root = os.path.join(JPEG_TURBO_DIR, 'libjpeg-turbo6-win-vc-%s' % arch_dir)
    lib = os.path.join(root, 'lib', 'turbojpeg-static.lib')
    if os.path.isfile(lib):
        if collector.add('lib/turbojpeg-static.lib', lib):
            count += 1
    else:
        collector.warn('缺少 turbojpeg-static.lib：%s' % _rel(lib))
    return count


def collect_cef(collector, sys_tag, arch_tag, cef_version):
    """收集 CEF 开发包：头文件 + 链接库（wrapper 在 collect_libs 中）。

    Windows：`libcef.lib` 导入库放入 lib/，运行时由 collect_cef_runtime 收集。
    Linux：`libcef.so` 同时是链接库与运行库，只在 bin/libcef_linux/ 保留一份，
    避免重复打包上百 MB；下游链接时把该目录加入库搜索路径（见 MANIFEST）。
    macOS：运行时以 .app 内嵌 Framework 提供，不随 SDK 包自动收集。
    """
    if sys_tag == 'macos':
        raise PackageError(
            '--with-cef 不在 macOS 上自动收集 CEF；请按发布文档手动部署 '
            '.app 内的 Framework')
    if sys_tag not in ('win', 'linux'):
        raise PackageError('--with-cef 仅支持 Windows / Linux')

    src_name = cef_src_name(sys_tag, cef_version)
    src_root = os.path.join(CEF_DIR, src_name)
    if not os.path.isdir(src_root):
        raise PackageError('缺少 CEF 源码目录：%s' % _rel(src_root))

    count = 0
    if sys_tag == 'win':
        arch_dir = {'x64': 'x64', 'x86': 'Win32'}.get(arch_tag)
        if arch_dir is None:
            raise PackageError('CEF 上游没有 Windows %s 预编译包，无法随包' % arch_tag)
        lib = os.path.join(src_root, 'lib', arch_dir, 'libcef.lib')
        if not os.path.isfile(lib):
            raise PackageError('缺少 %s' % _rel(lib))
        if collector.add('lib/libcef.lib', lib):
            count += 1
    else:
        so = os.path.join(BIN_DIR, src_name, 'libcef.so')
        if not os.path.isfile(so):
            raise PackageError(
                '缺少 %s：请把 CEF 二进制分发包的 Release/ 与 Resources/ '
                '内容复制到 bin/%s/' % (_rel(so), src_name))

    count += _collect_tree(collector, os.path.join(src_root, 'include'),
                           'include/duilib/third_party/libcef/%s/include' % src_name)
    return count


def collect_cef_runtime(collector, sys_tag, arch_tag, cef_version):
    """收集 CEF 运行时（--with-cef 时随包）。

    运行时按“可执行文件目录 + 固定子目录”被加载：
      Windows：<exe>/libcef_win[/_109]/{x64,Win32}/
      Linux：  <exe>/libcef_linux/（扁平结构，libcef.so 与资源文件同级）
    包内保持仓库 bin/ 下的同一相对结构（bin/libcef_win[_109]/<arch>/、
    bin/libcef_linux/），示例程序放在 bin/ 时可直接启动，下游把该子目录
    整体复制到可执行文件同级即可。
    macOS 的运行时以 .app 内嵌 Framework 提供，SDK 包不自动收集（下游手动配置）。
    """
    if sys_tag == 'macos':
        return 0
    if sys_tag not in ('win', 'linux'):
        return 0

    src_name = cef_src_name(sys_tag, cef_version)
    if sys_tag == 'linux':
        src_root = os.path.join(BIN_DIR, src_name)
        dst_prefix = 'bin/%s/' % src_name
    else:
        arch_dir = CEF_RUNTIME_ARCH_DIRS.get(arch_tag)
        if arch_dir is None:
            collector.warn('CEF 运行时没有 %s 架构的目录，已跳过' % arch_tag,
                           critical=True)
            return 0
        src_root = os.path.join(BIN_DIR, src_name, arch_dir)
        dst_prefix = 'bin/%s/%s/' % (src_name, arch_dir)

    if not os.path.isdir(src_root):
        collector.warn('CEF 运行时目录不存在：%s（本包只有头文件与库，'
                       '带 CEF 的示例程序无法直接运行）' % _rel(src_root),
                       critical=True)
        return 0

    count = 0
    for abs_path, rel in iter_files(src_root):
        if os.path.splitext(rel)[1].lower() in CEF_RUNTIME_EXCLUDE_SUFFIXES:
            continue
        if collector.add(dst_prefix + rel, abs_path):
            count += 1
    return count


def _resource_rel_ok(rel, with_examples):
    """判断 resources 内的相对路径是否应打包。"""
    base = rel.rsplit('/', 1)[-1]
    if base in EXCLUDED_RESOURCE_FILES:
        return False
    if with_examples:
        return True
    # 关闭时：themes/default/ 下只保留 public/ 与 global.xml
    prefix = EXAMPLE_THEME_DIR.replace(os.sep, '/') + '/'
    if rel.startswith(prefix):
        sub = rel[len(prefix):]
        if '/' in sub:
            return sub.split('/', 1)[0] in EXAMPLE_THEME_KEEP_DIRS
        return sub in EXAMPLE_THEME_KEEP_FILES
    return True


def collect_resources(collector, with_examples):
    if not os.path.isdir(RESOURCES_DIR):
        collector.warn('资源目录不存在：%s（框架启动必需）' % RESOURCES_DIR,
                       critical=True)
        return 0
    count = 0
    for abs_path, rel in iter_files(RESOURCES_DIR):
        if not _resource_rel_ok(rel, with_examples):
            continue
        if collector.add('bin/resources/' + rel, abs_path):
            count += 1
    return count


def collect_licenses(collector, skia_lib_dir):
    if not os.path.isdir(LICENSES_DIR):
        collector.warn('许可证目录不存在：%s' % LICENSES_DIR)
        return 0
    count = 0
    for abs_path, rel in iter_files(LICENSES_DIR):
        if collector.add('LICENSES/' + rel, abs_path):
            count += 1
    # 仓库根的 LICENSE 也放进包
    root_license = os.path.join(REPO_ROOT, 'LICENSE')
    if os.path.isfile(root_license):
        collector.add('LICENSE', root_license)
        count += 1
    # 第三方声明汇总（合规要求：包内必须带 THIRD-PARTY-NOTICES.md）
    notices = os.path.join(REPO_ROOT, 'THIRD-PARTY-NOTICES.md')
    if os.path.isfile(notices):
        if collector.add('THIRD-PARTY-NOTICES.md', notices):
            count += 1
    else:
        collector.warn('缺少第三方声明：%s' % notices)
    # Skia 的许可证不在本仓库内（Skia 源码树同级）；仓库 licenses/ 已有时不重复
    skia_root = os.path.dirname(os.path.dirname(skia_lib_dir))
    skia_license = os.path.join(skia_root, 'LICENSE')
    has_skia_license = any(r.startswith('LICENSES/skia') for r, _ in collector.items)
    if not has_skia_license and os.path.isfile(skia_license):
        if collector.add('LICENSES/skia/LICENSE', skia_license):
            count += 1
    elif not has_skia_license:
        collector.warn('缺少 Skia 许可证：%s' % skia_license)
    return count


def collect_docs(collector):
    """把 docs/ 顶层文档渲染为包内虚拟文件，并修正指向源码的链接。

    只取一级文件，跳过 DOC_EXCLUDE_FILES 中的维护文档。文档里有大量
    指向源码树的相对链接，打包后源码不存在，因此按实际收集结果改写：
      - ../duilib/xxx      -> ../include/duilib/xxx（该头文件随包时）
      - ../../bin/resources -> ../bin/resources（资源位于包内 bin/ 下）
    目标不在包内的链接（.cpp、examples/、build/、未打包的内部头文件）
    去掉链接，只保留显示文字，避免包内出现断链。
    """
    if not os.path.isdir(DOCS_DIR):
        collector.warn('文档目录不存在：%s' % DOCS_DIR)
        return {}

    doc_names = []
    for name in sorted(os.listdir(DOCS_DIR)):
        if name in DOC_EXCLUDE_FILES:
            continue
        if os.path.isfile(os.path.join(DOCS_DIR, name)):
            doc_names.append(name)

    # 链接目标是否在包内：已收集的实体文件 + 即将生成的文档本身。
    packaged = {rel for rel, _abs in collector.items}
    packaged.update('docs/' + name for name in doc_names)

    virtual = {}
    for name in doc_names:
        text = _read_text(os.path.join(DOCS_DIR, name))
        virtual['docs/' + name] = _rewrite_doc_links(text, packaged)
    return virtual


_DOC_LINK_RE = re.compile(r'\[([^\]]*)\]\(([^)\s]+)(\s+"[^"]*")?\)')


def _read_text(path):
    """按 UTF-8 读取文本，去掉 BOM 并统一换行，保证 zip/tar 的摘要一致。"""
    with open(path, 'rb') as f:
        data = f.read()
    return data.decode('utf-8-sig').replace('\r\n', '\n').replace('\r', '\n')


def _resolve_doc_link(url, packaged):
    """把文档中的相对链接映射到包内路径。

    返回 (动作, 新地址)：keep 原样保留；rewrite 改写；drop 删除链接。
    """
    target, sep, fragment = url.partition('#')
    suffix = ('#' + fragment) if sep else ''
    if not target or url.startswith(('http://', 'https://', 'mailto:')):
        return 'keep', url

    if target.startswith('../duilib/'):
        rest = target[len('../'):]
        pkg = 'include/' + rest
        new_url = '../include/' + rest + suffix
    elif target.startswith('../bin/resources/'):
        # 包内资源同样位于 bin/ 下，链接保持不变。
        pkg = target[len('../'):]
        new_url = url
    elif target.startswith('../../bin/resources/'):
        rest = target[len('../../bin/resources/'):]
        pkg = 'bin/resources/' + rest
        new_url = '../bin/resources/' + rest + suffix
    elif target.startswith('../'):
        # 源码树中的其它目录（examples/、build/、README），包内不存在。
        return 'drop', url
    elif target.startswith('./'):
        pkg = 'docs/' + target[2:]
        new_url = url
    elif target.endswith('.md'):
        pkg = 'docs/' + target
        new_url = url
    else:
        return 'keep', url

    if pkg not in packaged:
        return 'drop', url
    return ('keep', url) if new_url == url else ('rewrite', new_url)


def _rewrite_doc_links(text, packaged):
    """改写文档中的 Markdown 链接；目标缺失时退化为纯文字。"""
    def repl(match):
        label, url, title = match.group(1), match.group(2), match.group(3) or ''
        action, new_url = _resolve_doc_link(url, packaged)
        if action == 'keep':
            return match.group(0)
        if action == 'rewrite':
            return '[%s](%s%s)' % (label, new_url, title)
        return label

    return _DOC_LINK_RE.sub(repl, text)


def collect_examples(collector, config, is_windows):
    """收集示例程序可执行文件与运行时 dll。

    Release 包跳过 VS 的 Debug 可执行文件（*_d.exe）；Debug 包因 CMake 的
    Debug exe 与 Release 同名，无法区分，全部收集并告警。
    运行时 dll 仅 Windows 收集（此处只有 WebView2Loader.dll；
    duilib 与 SDL3 的运行的 DLL 由各自 collect 函数收集）。
    """
    if not os.path.isdir(BIN_DIR):
        collector.warn('bin 目录不存在：%s' % BIN_DIR, critical=True)
        return 0
    count = 0
    exe_count = 0
    skipped_debug = []
    exec_exts = ('.exe',)
    for name in sorted(os.listdir(BIN_DIR)):
        full = os.path.join(BIN_DIR, name)
        if not os.path.isfile(full):
            continue
        low = name.lower()
        if low.endswith('.dll'):
            if is_windows and name in INCLUDE_RUNTIME_DLLS:
                if collector.add('bin/' + name, full):
                    count += 1
            continue
        if not low.endswith(exec_exts):
            continue
        if config == 'Release' and low.endswith('_d.exe'):
            skipped_debug.append(name)
            continue
        if collector.add('bin/' + name, full):
            count += 1
            exe_count += 1
    if config == 'Debug' and exe_count:
        collector.warn('Debug 包中的示例程序可能混有 Release 版本'
                       '（CMake 的 Debug exe 与 Release 同名），请确认后再分发')
    if skipped_debug:
        collector.warn('已跳过 Debug 示例程序：%s' % ', '.join(skipped_debug))
    if exe_count == 0:
        collector.warn('启用了 --with-examples，但 %s 下没有可打包的示例程序'
                       % _rel(BIN_DIR), critical=True)
    return count


# --------------------------------------------------------------------------
# 下游接入文件（VS property sheet / CMake package）
# --------------------------------------------------------------------------

def _render_template(rel_path, mapping):
    """读取 packaging/ 下的模板并替换 @KEY@ 占位符。

    模板缺失、或替换后仍有占位符，都视为打包错误：宁可失败，
    也不要把带 @...@ 的半成品 property sheet / CMake 文件发给下游。
    """
    path = os.path.join(PACKAGING_DIR, rel_path.replace('/', os.sep))
    if not os.path.isfile(path):
        raise PackageError('接入模板不存在：%s（应位于仓库 packaging/ 目录）'
                           % _rel(path))
    with io.open(path, 'r', encoding='utf-8-sig') as f:
        text = f.read()
    for key, value in mapping.items():
        text = text.replace('@%s@' % key, str(value))
    leftover = re.findall(r'@[A-Z0-9_]+@', text)
    if leftover:
        raise PackageError('接入模板 %s 存在未替换的占位符：%s'
                           % (rel_path, ', '.join(sorted(set(leftover)))))
    return text


def _with_bom(text):
    """VS 读取 UTF-8 属性表时带 BOM 最稳妥（中文注释不会变乱码）。"""
    return u'\ufeff' + text


def _package_lib_names(collector):
    return set(rel.rsplit('/', 1)[-1] for rel, _abs in collector.items
               if rel.startswith('lib/'))


def _package_lib_stems(lib_names):
    """库文件名主干：duilib.lib -> duilib，libskia.a -> skia，libSDL3.dll.a -> SDL3。

    只对 .a（Unix/MinGW）去掉 lib 前缀：MSVC 的 libcef_dll_wrapper.lib
    本身就带 lib 前缀，不能误删。
    """
    stems = set()
    for name in lib_names:
        if name.lower().endswith('.a'):
            stem = name[:-2]
            if stem.lower().endswith('.dll'):
                stem = stem[:-4]
            if stem.startswith('lib'):
                stem = stem[3:]
        else:
            stem = os.path.splitext(name)[0]
        stems.add(stem)
    return stems


def build_package_meta(collector, version, sys_tag, arch_tag, config,
                       toolchain, with_cef, cef_version, msvc_runtime,
                       mingw_variant=None):
    """汇总发布包的接入元数据（VS props 与 CMake package info 共用）。

    所有特性均以“实际进了包的文件”为准，而不是命令行开关，
    避免声明了 SDL/CEF 但包里没有对应库。
    """
    lib_names = _package_lib_names(collector)
    stems = _package_lib_stems(lib_names)
    bin_dlls = sorted(rel.rsplit('/', 1)[-1] for rel, _abs in collector.items
                      if rel.startswith('bin/') and rel.lower().endswith('.dll'))
    is_dll = any(name.lower().startswith('duilib') for name in bin_dlls)

    sdl_static = ('SDL3-static.lib' in lib_names) or ('libSDL3.a' in lib_names)
    sdl_shared = (('SDL3.lib' in lib_names) or ('libSDL3.dll.a' in lib_names)
                  or any(name.lower().startswith('libsdl3') and
                         ('.so' in name.lower() or name.lower().endswith('.dylib'))
                         for name in lib_names))
    has_sdl = sdl_static or sdl_shared
    sdl_kind = 'static' if sdl_static else ('shared' if sdl_shared else '')

    # WebView2：MSVC 可用静态或动态 loader；MinGW 只随包动态 loader 的导入库。
    # JPEG：预编译 turbojpeg-static.lib 仅 MSVC 可用，MinGW 包不收集。
    is_msvc = toolchain == 'msvc'
    if is_msvc:
        has_webview2 = ('WebView2LoaderStatic.lib' in lib_names or
                        'WebView2Loader.dll.lib' in lib_names)
    else:
        has_webview2 = (toolchain == 'mingw' and
                        'WebView2Loader.dll.lib' in lib_names)
    has_jpeg = is_msvc and any(name.lower().startswith('turbojpeg')
                               for name in lib_names)

    cef_wrapper = ''
    cef_src = ''
    has_cef = False
    if with_cef and is_msvc:
        wrapper = CEF_WRAPPER_BY_VERSION[cef_version]
        if 'libcef.lib' in lib_names and wrapper in stems:
            has_cef = True
            cef_wrapper = wrapper
            cef_src = CEF_SRC_BY_VERSION[cef_version]
    elif with_cef and sys_tag == 'linux':
        wrapper = CEF_WRAPPER_BY_VERSION[cef_version]
        # Linux 没有单独的导入库：以运行期 libcef.so 与 wrapper 都在包内为准。
        cef_so = any(rel == 'bin/libcef_linux/libcef.so'
                     for rel, _abs in collector.items)
        # _lib_stems 会去掉 Unix 静态库的 lib 前缀，两种写法都接受。
        if cef_so and (wrapper in stems or wrapper[3:] in stems):
            has_cef = True
            cef_wrapper = wrapper
            cef_src = 'libcef_linux'

    if sys_tag == 'win' and toolchain == 'msvc':
        if msvc_runtime == 'auto':
            msvc_runtime = 'MD' if is_dll else 'MT'
        else:
            msvc_runtime = msvc_runtime.upper()
    else:
        msvc_runtime = ''

    return {
        'version': version,
        'system': sys_tag,
        'arch': arch_tag,
        'config': config,
        'toolchain': toolchain or '',
        'mingw_variant': mingw_variant or '',
        'is_dll': is_dll,
        'has_sdl': has_sdl,
        'sdl_kind': sdl_kind,
        'has_webview2': has_webview2,
        'has_jpeg': has_jpeg,
        'has_cef': has_cef,
        'cef_src': cef_src,
        'cef_wrapper': cef_wrapper,
        'msvc_runtime': msvc_runtime,
        'runtime_dlls': bin_dlls,
    }


def _build_sdk_props(meta):
    """生成 nim_duilib.sdk.props（MSBuild 可读的包元数据）。"""
    def flag(value):
        return '1' if value else '0'

    lines = [
        '<?xml version="1.0" encoding="utf-8"?>',
        '<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">',
        '    <!-- 由 build/pack.py 生成，勿手工修改。 -->',
        '    <PropertyGroup Label="NimDuilibPackage">',
        '        <NimDuilibPackageVersion>%s</NimDuilibPackageVersion>' % meta['version'],
        '        <NimDuilibPackageConfig>%s</NimDuilibPackageConfig>' % meta['config'],
        '        <NimDuilibPackageArch>%s</NimDuilibPackageArch>' % meta['arch'],
        '        <NimDuilibPackageToolchain>%s</NimDuilibPackageToolchain>' % meta['toolchain'],
        '        <NimDuilibPackageMingwVariant>%s</NimDuilibPackageMingwVariant>' % meta['mingw_variant'],
        '        <NimDuilibPackageIsDll>%s</NimDuilibPackageIsDll>' % flag(meta['is_dll']),
        '        <NimDuilibPackageHasSdl>%s</NimDuilibPackageHasSdl>' % flag(meta['has_sdl']),
        '        <NimDuilibPackageSdlKind>%s</NimDuilibPackageSdlKind>' % meta['sdl_kind'],
        '        <NimDuilibPackageHasWebView2>%s</NimDuilibPackageHasWebView2>' % flag(meta['has_webview2']),
        '        <NimDuilibPackageHasJpeg>%s</NimDuilibPackageHasJpeg>' % flag(meta['has_jpeg']),
        '        <NimDuilibPackageHasCef>%s</NimDuilibPackageHasCef>' % flag(meta['has_cef']),
        '        <NimDuilibPackageCefSrc>%s</NimDuilibPackageCefSrc>' % meta['cef_src'],
        '        <NimDuilibPackageCefWrapper>%s</NimDuilibPackageCefWrapper>' % meta['cef_wrapper'],
        '        <NimDuilibPackageMsvcRuntime>%s</NimDuilibPackageMsvcRuntime>' % meta['msvc_runtime'],
        '        <NimDuilibPackageRuntimeDlls>%s</NimDuilibPackageRuntimeDlls>' % ';'.join(meta['runtime_dlls']),
        '    </PropertyGroup>',
        '</Project>',
        '',
    ]
    return '\n'.join(lines)


def _build_package_info_cmake(meta):
    """生成 nim_duilibPackageInfo.cmake（CMake 可读的包元数据）。"""
    def flag(value):
        return '1' if value else '0'

    lines = [
        '# 由 build/pack.py 生成，勿手工修改。',
        'set(NIM_DUILIB_PACKAGE_VERSION "%s")' % meta['version'],
        'set(NIM_DUILIB_PACKAGE_SYSTEM "%s")' % meta['system'],
        'set(NIM_DUILIB_PACKAGE_ARCH "%s")' % meta['arch'],
        'set(NIM_DUILIB_PACKAGE_CONFIG "%s")' % meta['config'],
        'set(NIM_DUILIB_PACKAGE_TOOLCHAIN "%s")' % meta['toolchain'],
        'set(NIM_DUILIB_PACKAGE_MINGW_VARIANT "%s")' % meta['mingw_variant'],
        'set(NIM_DUILIB_PACKAGE_IS_DLL %s)' % flag(meta['is_dll']),
        'set(NIM_DUILIB_PACKAGE_HAS_SDL %s)' % flag(meta['has_sdl']),
        'set(NIM_DUILIB_PACKAGE_SDL_KIND "%s")' % meta['sdl_kind'],
        'set(NIM_DUILIB_PACKAGE_HAS_WEBVIEW2 %s)' % flag(meta['has_webview2']),
        'set(NIM_DUILIB_PACKAGE_HAS_JPEG %s)' % flag(meta['has_jpeg']),
        'set(NIM_DUILIB_PACKAGE_HAS_CEF %s)' % flag(meta['has_cef']),
        'set(NIM_DUILIB_PACKAGE_CEF_SRC "%s")' % meta['cef_src'],
        'set(NIM_DUILIB_PACKAGE_CEF_WRAPPER "%s")' % meta['cef_wrapper'],
        'set(NIM_DUILIB_PACKAGE_MSVC_RUNTIME "%s")' % meta['msvc_runtime'],
        'set(NIM_DUILIB_PACKAGE_RUNTIME_DLLS "%s")' % ';'.join(meta['runtime_dlls']),
        '',
    ]
    return '\n'.join(lines)


def _build_config_version(meta):
    """生成 nim_duilibConfigVersion.cmake：同主版本、且不高于包版本即兼容。"""
    version = meta['version']
    major = version.split('.')[0]
    lines = [
        '# 由 build/pack.py 生成，勿手工修改。',
        'set(PACKAGE_VERSION "%s")' % version,
        '',
        'if(PACKAGE_FIND_VERSION)',
        '    if(PACKAGE_FIND_VERSION VERSION_GREATER PACKAGE_VERSION)',
        '        set(PACKAGE_VERSION_COMPATIBLE FALSE)',
        '    else()',
        '        set(PACKAGE_VERSION_COMPATIBLE TRUE)',
        '        if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)',
        '            set(PACKAGE_VERSION_EXACT TRUE)',
        '        endif()',
        '        set(_nim_package_major "%s")' % major,
        '        set(_nim_find_major "${PACKAGE_FIND_VERSION_MAJOR}")',
        '        if(NOT _nim_find_major STREQUAL _nim_package_major)',
        '            set(PACKAGE_VERSION_COMPATIBLE FALSE)',
        '        endif()',
        '    endif()',
        'else()',
        '    set(PACKAGE_VERSION_COMPATIBLE TRUE)',
        'endif()',
        '',
    ]
    return '\n'.join(lines)


def collect_integration_files(collector, meta):
    """收集/生成下游接入文件。

    实体文件（Windows manifest）直接进 collector；其余是内存中生成的
    虚拟文件，返回 {包内相对路径: 文本}，由 build_package 写入压缩包。
    """
    count = 0
    virtual = {}

    if meta['system'] == 'win':
        # x64 机器上也能开发 x86/arm64 程序，三份 manifest 都随包，供下游按
        # Platform 选择（props 里按 NimDuilibManifestArch 取对应的那个）。
        for manifest_arch in WINDOWS_MANIFEST_ARCHES:
            manifest_name = 'duilib.%s.manifest' % manifest_arch
            manifest = os.path.join(MSVC_MANIFEST_DIR, manifest_name)
            if os.path.isfile(manifest):
                if collector.add('msvc/manifest/' + manifest_name, manifest):
                    count += 1
            else:
                collector.warn('缺少 manifest：%s（VS 工程不会自动嵌入 Common Controls v6）'
                               % _rel(manifest), critical=True)

    if meta['system'] == 'win' and meta['toolchain'] == 'msvc':
        virtual['msvc/nim_duilib.props'] = _with_bom(
            _render_template('msvc/nim_duilib.props', {}))
        virtual['msvc/nim_duilib.targets'] = _with_bom(
            _render_template('msvc/nim_duilib.targets', {}))
        virtual['msvc/nim_duilib.sdk.props'] = _with_bom(_build_sdk_props(meta))

    cmake_dir = 'lib/cmake/nim_duilib/'
    virtual[cmake_dir + 'nim_duilibConfig.cmake'] = _render_template(
        'cmake/nim_duilibConfig.cmake', {})
    virtual[cmake_dir + 'nim_duilibHelpers.cmake'] = _render_template(
        'cmake/nim_duilibHelpers.cmake', {})
    virtual[cmake_dir + 'nim_duilibPackageInfo.cmake'] = _build_package_info_cmake(meta)
    virtual[cmake_dir + 'nim_duilibConfigVersion.cmake'] = _build_config_version(meta)

    return count, virtual


# --------------------------------------------------------------------------
# 清单与打包
# --------------------------------------------------------------------------

CEF_EXAMPLES = ('cef.exe', 'cef_d.exe', 'CefBrowser.exe', 'CefBrowser_d.exe')


def _lib_stems(collector):
    """包内 lib/ 下所有库的文件名主干（Unix/MinGW 去掉 lib 前缀与 .dll 中缀）。"""
    stems = set()
    for rel, _abs in collector.items:
        if not rel.startswith('lib/'):
            continue
        stem = os.path.splitext(rel.rsplit('/', 1)[-1])[0]
        if stem.startswith('lib'):
            stem = stem[3:]
        if stem.lower().endswith('.dll'):
            stem = stem[:-4]
        stems.add(stem)
    return stems


def _lib_filenames(collector):
    """包内 lib/ 下所有库的完整文件名（用于区分静态库与导入库）。"""
    return set(rel.rsplit('/', 1)[-1] for rel, _abs in collector.items
               if rel.startswith('lib/'))


def write_manifest(collector, pkg_root_name, version, sys_tag, arch_tag,
                   config, with_examples, with_sdl, with_cef, cef_version,
                   toolchain=None, virtual_files=None, msvc_runtime=None,
                   package_time=None, mingw_variant=None):
    """生成包内 MANIFEST.txt 与 SHA256SUMS，并固化 SDK 使用契约。

    package_time 为 MANIFEST 里“打包时间”使用的 Unix 时间；None 表示取当前时间。
    调用方应传入与归档一致的确定性时间，否则包摘要不可复现。
    """
    if package_time is None:
        package_time = int(time.time())
    is_windows = sys_tag == 'win'
    is_mingw = toolchain == 'mingw'
    virtual_files = virtual_files or {}
    stems = _lib_stems(collector)
    lib_names = _lib_filenames(collector)
    bin_dlls = [r.split('/', 1)[1] for r, _abs in collector.items
                if r.startswith('bin/') and r.lower().endswith('.dll')]
    has_webview2 = (('WebView2LoaderStatic' in stems) or
                    ('WebView2Loader.dll' in stems) or
                    ('WebView2Loader.dll' in bin_dlls))
    # SDL3 静态库：MSVC 的 SDL3-static.lib 或 MinGW 的 libSDL3.a；
    # 动态库：Windows 的导入库（SDL3.lib / libSDL3.dll.a）或 Unix 的 libSDL3.so*。
    sdl_static = ('SDL3-static.lib' in lib_names) or ('libSDL3.a' in lib_names)
    sdl_dynamic = (('SDL3.lib' in lib_names) or ('libSDL3.dll.a' in lib_names) or
                   any(n.startswith('libSDL3.so') or n.startswith('libSDL3.')
                       and n.endswith('.dylib') for n in lib_names))
    # CEF 运行时保持 bin/libcef_win[_109]/<arch>/ 或 bin/libcef_linux/ 结构，
    # 见 collect_cef_runtime。
    cef_runtime_rel = ''
    if with_cef and is_windows and not is_mingw:
        cef_arch_dir = CEF_RUNTIME_ARCH_DIRS.get(arch_tag)
        if cef_arch_dir:
            cef_runtime_rel = 'bin/%s/%s' % (
                CEF_SRC_BY_VERSION[cef_version], cef_arch_dir)
    elif with_cef and sys_tag == 'linux':
        cef_runtime_rel = 'bin/libcef_linux'
    cef_runtime_present = bool(cef_runtime_rel) and any(
        rel.startswith(cef_runtime_rel + '/') for rel, _abs in collector.items)
    lines = []

    def section(title):
        lines.append('')
        lines.append(title)
        lines.append('-' * 60)

    def fmt_libs(names):
        # MinGW 产物是 libxxx.a / libxxx.dll.a，链接写法与 Unix 一致。
        if is_windows and not is_mingw:
            return ';'.join('%s.lib' % n for n in names)
        return ' '.join('-l%s' % n for n in names)

    lines.append('nim_duilib SDK')
    lines.append('=' * 60)
    lines.append('版本      : %s' % version)
    lines.append('包名      : %s' % pkg_root_name)
    lines.append('平台      : %s-%s' % (sys_tag, arch_tag))
    lines.append('配置      : %s' % config)
    if is_windows and toolchain:
        if is_mingw:
            toolchain_label = ('MinGW（llvm-mingw）' if mingw_variant == 'llvm'
                               else 'MinGW（gcc）')
        else:
            toolchain_label = 'MSVC'
        lines.append('工具链    : %s' % toolchain_label)
    lines.append('打包时间  : %s UTC'
                 % time.strftime('%Y-%m-%d %H:%M:%S',
                                 time.gmtime(package_time)))
    lines.append('包含示例  : %s' % ('是' if with_examples else '否'))
    lines.append('文件总数  : %d' % (len(collector.items) + len(virtual_files)))

    section('目录结构')
    lines.append('  include/duilib/**.h   对外头文件，保持仓库根相对路径')
    lines.append('                         用法：#include "duilib/duilib.h"')
    lines.append('  lib/                  静态库（.lib / .a），平面结构')
    lines.append('  bin/                  运行期 DLL；--with-examples 时还含示例程序')
    lines.append('  bin/resources/        主题、语言、字体资源（与示例程序同级）')
    if cef_runtime_present:
        if sys_tag == 'linux':
            lines.append('  %s/  CEF 运行时（libcef.so、*.pak、icudtl.dat、'
                         'locales/）' % cef_runtime_rel)
        else:
            lines.append('  %s/  CEF 运行时（libcef.dll、*.pak、icudtl.dat、'
                         'locales/）' % cef_runtime_rel)
    lines.append('  LICENSES/             第三方许可证')
    lines.append('  THIRD-PARTY-NOTICES.md 第三方组件声明')
    lines.append('  docs/                 使用文档（顶层；清单见“文档”章节）')
    lines.append('  msvc/                 VS 接入：props/targets/manifest（仅 Windows+MSVC）')
    lines.append('  lib/cmake/nim_duilib/ CMake 接入：find_package(nim_duilib)')
    lines.append('  VERSION               版本号')

    section('文档')
    doc_files = sorted(rel for rel in virtual_files if rel.startswith('docs/'))
    if doc_files:
        lines.append('  随包使用文档（相对链接已按包内布局重写）：')
        for rel in doc_files:
            lines.append('    %s' % rel)
    else:
        lines.append('  本包不含使用文档。')

    section('开发接入')
    lines.append('1) 包含路径：include/')
    if with_cef:
        src_name = cef_src_name(sys_tag, cef_version)
        lines.append('   使用 CEF 时追加：include/duilib/third_party/libcef/%s'
                     % src_name)
    if with_sdl or not is_windows:
        lines.append('   使用 SDL 时 <SDL3/SDL.h> 也由 include/ 覆盖')
    lines.append('2) 库路径  ：lib/')
    if is_mingw:
        variant_label = 'llvm-mingw（libc++）' if mingw_variant == 'llvm' \
            else 'gcc（libstdc++）'
        lines.append('   本包由 MinGW %s编译，lib/ 静态库与 Skia 均绑定该工具链的'
                     % variant_label)
        lines.append('   C++ 运行时；不要与另一 MinGW 变体的包混用。')
    lines.append('3) 链接列表（按需取用，顺序仅供参考）')

    base_links = [n for n in LIB_BASENAMES if n in stems]
    lines.append('   基础    : %s' % fmt_libs(base_links))

    skia_links = [n for n in SKIA_LINK_ORDER if n in stems]
    if skia_links:
        lines.append('   Skia    : %s' % fmt_libs(skia_links))
        extra = [n for n in SKIA_EXTRA_LIBS if n in stems]
        if extra:
            lines.append('             随包的传递依赖：%s'
                         '（出现未解析符号时补链）' % ', '.join(extra))

    if 'WebView2LoaderStatic' in stems:
        lines.append('   WebView2: WebView2LoaderStatic.lib（静态 loader，无需 DLL）')
    if 'WebView2Loader.dll' in stems:
        if is_mingw:
            lines.append('   WebView2: -lWebView2Loader.dll（动态 loader 导入库，'
                         '运行期需 WebView2Loader.dll）')
        else:
            lines.append('   WebView2: WebView2Loader.dll.lib（动态 loader，'
                         '运行期需 WebView2Loader.dll）')
    if 'turbojpeg-static' in stems:
        lines.append('   JPEG    : turbojpeg-static.lib')
    if sdl_static:
        sdl_static_name = ('SDL3-static.lib' if 'SDL3-static.lib' in lib_names
                           else 'libSDL3.a')
        lines.append('   SDL3    : %s（静态）' % sdl_static_name)
    elif sdl_dynamic:
        lines.append('   SDL3    : -lSDL3（动态库，见运行期）')
    if with_cef:
        wrapper = CEF_WRAPPER_BY_VERSION[cef_version]
        if sys_tag == 'linux':
            lines.append('   CEF     : -Lbin/libcef_linux -lcef -l%s'
                         '（libcef.so 随包在 bin/libcef_linux/）' % wrapper)
        else:
            lines.append('   CEF     : libcef.lib;%s.lib' % wrapper)

    section('下游接入')
    if is_windows and not is_mingw:
        lines.append('  VS（MSVC）：在 .vcxproj 末尾加入（属性表会带上 include/lib/宏/')
        lines.append('  系统库/manifest，并在构建后复制 bin/*.dll）：')
        lines.append('    <Import Project="<SDK根>\\msvc\\nim_duilib.targets" />')
        lines.append('  只想设置编译/链接、不要复制时，可只导入 msvc\\nim_duilib.props。')
        lines.append('  可用属性：NimDuilibCopyRuntime=0、NimDuilibCopyResources=1、')
        lines.append('            NimDuilibForceRuntimeLibrary=0、NimDuilibSkipValidation=1')
    lines.append('  CMake（全平台）：')
    lines.append('    find_package(nim_duilib CONFIG REQUIRED)')
    lines.append('    target_link_libraries(<target> PRIVATE nim_duilib::duilib)')
    lines.append('    nim_duilib_copy_runtime(<target> [RESOURCES])')
    lines.append('  nim_duilib_copy_runtime 在构建后复制 bin/*.dll；RESOURCES 额外复制')
    lines.append('  bin/resources 到输出目录的 resources/（默认不复制，避免覆盖应用自己的主题）。')
    lines.append('  可设置 NIM_DUILIB_SET_MSVC_RUNTIME=OFF 退出运行库强制对齐；')
    lines.append('  NIM_DUILIB_ALLOW_{ARCH,CONFIG}_MISMATCH=ON 放行不匹配（不推荐）。')
    lines.append('  注意：发布包是单架构、单配置的，Debug/Release 需各用各的包。')
    if is_windows and arch_tag == 'arm64':
        lines.append('')
        lines.append('  arm64 说明：manifest / props / CMake config 等接入文件与包结构')
        lines.append('  已就绪，但未在 arm64 真机上做编译、打包、运行验证；')
        lines.append('  CEF 上游没有 Windows arm64 预编译包，本包不含 CEF。')

    section('运行期')
    lines.append('  bin/resources/ 必须与可执行文件同级（框架启动必需）；从 bin/ 取用')
    lines.append('  程序时请连同 bin/resources/ 一起复制。')
    if is_windows:
        if has_webview2:
            loader_files = []
            if 'WebView2LoaderStatic.lib' in lib_names:
                loader_files.append('WebView2LoaderStatic.lib')
            if 'WebView2Loader.dll.lib' in lib_names:
                loader_files.append('WebView2Loader.dll.lib')
            if 'WebView2Loader.dll' in bin_dlls:
                loader_files.append('WebView2Loader.dll')
            lines.append('  WebView2 依赖系统已安装的 Microsoft Edge WebView2 Evergreen')
            lines.append('  Runtime；本包只带 loader（%s），'
                         % ' / '.join(loader_files))
            lines.append('  不含浏览器内核。未安装时创建环境失败（常见 0x80070002），')
            lines.append('  安装：https://developer.microsoft.com/microsoft-edge/webview2/')
        if 'WebView2Loader.dll' in bin_dlls:
            lines.append('  WebView2Loader.dll 随包（bin/）；使用动态 loader 时复制到')
            lines.append('  可执行文件同级。')
        duilib_dlls = [n for n in bin_dlls if n.lower().startswith('duilib')]
        if duilib_dlls:
            lines.append('  duilib 为 DLL 模式：%s 随包（bin/），需与下游可执行文件'
                         % ', '.join(sorted(duilib_dlls)))
            lines.append('  放在同级目录；lib/ 中的 duilib 库为导入库。')
        if sdl_dynamic:
            lines.append('  SDL3 为动态库：lib/ 中为导入库，SDL3.dll 随包（bin/），')
            lines.append('  需复制到可执行文件同级；下游需定义 DUILIB_SDL=1。')
        elif sdl_static:
            sys_libs = ('-lversion -lwinmm -limm32 -lsetupapi' if is_mingw
                        else 'Version.lib;Winmm.lib;Imm32.lib;Setupapi.lib')
            lines.append('  SDL3 为静态库，无需额外 DLL；下游需定义 DUILIB_SDL=1，')
            lines.append('  并补链系统库：%s' % sys_libs)
        if with_cef:
            if cef_runtime_present:
                lines.append('  CEF 运行时随包（%s/）。框架启动时按“可执行文件目录 +'
                             % cef_runtime_rel)
                lines.append('  该子目录”加载 libcef.dll（CefManager::AddCefDllToPath），')
                lines.append('  下游请把该子目录整体复制到可执行文件同级，不要摊平。')
            else:
                lines.append('  CEF 运行时不在本包内，需从 libcef 发布包放入可执行文件同级：')
                lines.append('  libcef.dll、icudtl.dat、*.pak、locales/ 等。')
        if is_mingw:
            if mingw_variant == 'llvm':
                runtime_dlls = 'libc++.dll、libc++abi.dll、libunwind.dll'
            elif arch_tag == 'x86':
                runtime_dlls = ('libstdc++-6.dll、libgcc_s_dw2-1.dll、'
                                'libwinpthread-1.dll')
            else:
                runtime_dlls = ('libstdc++-6.dll、libgcc_s_seh-1.dll、'
                                'libwinpthread-1.dll')
            lines.append('  MinGW 运行库：下游未用 -static 链接时，需随程序提供 %s'
                         % runtime_dlls)
            lines.append('  （从所用编译器环境的 bin/ 目录获取）；本包示例程序已用 -static')
            lines.append('  链接，可独立运行。')
    else:
        lines.append('  Skia 为静态库；SDL3 为动态库，需让 libSDL3.so* 可被找到：')
        lines.append('  复制到可执行文件同级，或设置 LD_LIBRARY_PATH'
                     '（macOS：DYLD_LIBRARY_PATH）。')
        lines.append('  X11 / freetype / fontconfig 由系统提供，不随包。')
        if with_cef and cef_runtime_present:
            lines.append('  CEF 运行时随包（%s/）：把该目录全部内容复制到可执行文件'
                         % cef_runtime_rel)
            lines.append('  同级（libcef.so、*.pak、icudtl.dat、locales/ 等）；')
            lines.append('  libcef.so 需可被动态加载（同目录或 LD_LIBRARY_PATH）。')

    section('配置一致性')
    if is_windows and msvc_runtime:
        if msvc_runtime == 'MD':
            runtime_flag = '/MDd' if config == 'Debug' else '/MD'
        else:
            runtime_flag = '/MTd' if config == 'Debug' else '/MT'
        lines.append('  本包是 %s 版本，仅用于 %s 构建的下游工程（%s），'
                     % (config, config, runtime_flag))
        lines.append('  MSVC 运行库为 %s；VS/CMake 接入默认会强制对齐，'
                     % msvc_runtime)
        lines.append('  不一致会触发 LNK2038。与另一配置的包不可混用。')
    else:
        lines.append('  本包是 %s 版本，仅用于 %s 构建的下游工程，'
                     % (config, config))
        lines.append('  必须使用对应配置；与另一配置的包不可混用。')

    if is_mingw and mingw_variant == 'gcc':
        lines.append('  MSYS2 的 MINGW64（msvcrt）与 UCRT64（ucrt）都会生成')
        lines.append('  -mingw-gcc 包，但 C 运行库不同，两者不可互换；请记录构建环境。')

    if with_examples:
        section('示例程序')
        lines.append('  bin/*.exe             示例可执行文件（bin/resources/ 已在其同级）')
        missing_cef = [e for e in CEF_EXAMPLES
                       if any(r.endswith('/' + e) for r, _ in collector.items)]
        if missing_cef and not cef_runtime_present:
            lines.append('')
            lines.append('  [注意] 以下示例依赖 CEF 运行时，本包未包含该运行时，')
            lines.append('         直接运行会失败。需自行从 libcef 发布包放入：')
            for e in missing_cef:
                lines.append('           %s' % e)
    else:
        section('资源说明')
        lines.append('  本包不含示例程序资源。bin/resources/themes/ 下只保留了 public/ 与')
        lines.append('  global.xml；各示例的私有主题目录未包含。')

    section('校验')
    lines.append('  SHA256SUMS 覆盖包内全部普通文件（符号链接不计入，解包时按链接还原）。')
    lines.append('  Unix 包保留源文件权限与 .so 链接链；Windows 包条目权限无实际作用。')
    lines.append('  Unix/macOS：在包根目录执行 sha256sum -c SHA256SUMS')
    lines.append('  Windows PowerShell（在包根目录执行，全部通过输出 OK，否则列出失败项）：')
    lines.append("    $bad = Get-Content SHA256SUMS | ForEach-Object {")
    lines.append("        $h, $p = $_ -split '  ', 2")
    lines.append("        if ((Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash -ne $h) { $p }")
    lines.append("    }")
    lines.append("    if ($bad) { $bad; exit 1 } else { 'OK' }")
    lines.append('  也可用 certutil -hashfile <文件> SHA256 逐个比对。')

    manifest_text = '\n'.join(lines).replace('\n', '\r\n')

    sums = []
    for rel, abs_path in sorted(collector.items, key=lambda kv: kv[0]):
        # 符号链接在归档里是链接条目（tar 的 SYMTYPE / zip 的 S_IFLNK），
        # 没有独立内容；写进 SHA256SUMS 会让 sha256sum -c 跟随链接后校验失败。
        if os.path.islink(abs_path):
            continue
        sums.append('%s  %s' % (sha256_of(abs_path), rel))
    for rel in sorted(virtual_files):
        digest = hashlib.sha256(virtual_files[rel].encode('utf-8')).hexdigest()
        sums.append('%s  %s' % (digest, rel))
    sums_text = '\n'.join(sums) + '\n'

    return manifest_text, sums_text


def read_source_date_epoch():
    """读取 SOURCE_DATE_EPOCH（秒）；未设置返回 None，非法值直接失败。"""
    value = os.environ.get('SOURCE_DATE_EPOCH', '').strip()
    if not value:
        return None
    try:
        epoch = int(value)
    except ValueError:
        raise PackageError('SOURCE_DATE_EPOCH 必须是整数秒：%r' % value)
    if epoch < 0:
        raise PackageError('SOURCE_DATE_EPOCH 不能为负数：%d' % epoch)
    return epoch


def _default_archive_time(collector):
    """未设置 SOURCE_DATE_EPOCH 时，取收集文件中最新的 mtime 作为生成时间。

    连续两次打包（输入不变）得到完全相同的字节，不再依赖墙钟时间。
    """
    latest = 0
    for _rel, abs_path in collector.items:
        try:
            latest = max(latest, int(os.lstat(abs_path).st_mtime))
        except OSError:
            pass
    return latest or int(time.time())


def _zip_datetime(timestamp):
    """zip 条目时间：按 UTC 归一，且不早于 1980-01-01（zip 格式下限）。"""
    return time.gmtime(max(int(timestamp), 315532800))[:6]


def _write_virtual(zip_obj, tar_obj, arcname, text, timestamp):
    """把内存中的文本同时（或分别）写入 zip / tar。

    zip_obj 与 tar_obj 只有一个非 None —— 调用方按目标格式二选一。
    两者都写入会因None 崩溃，且早期版本正是这么写的。
    """
    if zip_obj is not None:
        data = text.encode('utf-8')
        zip_obj.writestr(_zip_info(arcname, len(data), timestamp), data)
    if tar_obj is not None:
        raw = text.replace('\r\n', '\n').encode('utf-8')
        tinfo = tarfile.TarInfo(arcname)
        tinfo.size = len(raw)
        tinfo.mtime = int(timestamp)
        tinfo.mode = 0o644
        tar_obj.addfile(tinfo, io.BytesIO(raw))


def _zip_info(arcname, size, timestamp):
    info = zipfile.ZipInfo(arcname, date_time=_zip_datetime(timestamp))
    info.create_system = 3  # Unix，解包端才会按 external_attr 建链接/权限
    info.compress_type = zipfile.ZIP_DEFLATED
    info.external_attr = (stat.S_IFREG | 0o644) << 16
    return info


def _archive_file_mode(abs_path):
    """归档条目的权限：按源文件权限位，保留可执行与特殊位（如 chrome-sandbox）。

    Windows 源没有 POSIX 权限位，按扩展名给 0755/0644，保证 tar 包在 Unix
    解包后示例程序可执行。
    """
    if os.name == 'nt':
        ext = os.path.splitext(abs_path)[1].lower()
        return 0o755 if ext in ('.exe', '.dll', '.bat', '.cmd', '.ps1') else 0o644
    mode = stat.S_IMODE(os.lstat(abs_path).st_mode) & ~0o022
    return mode or 0o644


def _zip_add_file(zf, abs_path, arc, timestamp=None):
    """按条目类型写入 zip。

    符号链接必须写成 S_IFLNK 条目（内容为链接目标），否则 zipfile.write
    会跟随链接，把 `libSDL3.so -> libSDL3.so.0 -> ...` 展开成多份实体文件。
    timestamp 为 None 时条目时间取源文件 mtime。
    """
    st = os.lstat(abs_path)
    entry_time = st.st_mtime if timestamp is None else timestamp
    info = zipfile.ZipInfo(arc, date_time=_zip_datetime(entry_time))
    info.create_system = 3
    if stat.S_ISLNK(st.st_mode):
        info.external_attr = (stat.S_IFLNK | 0o777) << 16
        info.compress_type = zipfile.ZIP_STORED
        zf.writestr(info, os.readlink(abs_path).encode('utf-8'))
        return
    info.external_attr = (stat.S_IFREG | _archive_file_mode(abs_path)) << 16
    info.compress_type = zipfile.ZIP_DEFLATED
    with zf.open(info, 'w') as dst, open(abs_path, 'rb') as src:
        for chunk in iter(lambda: src.read(1024 * 1024), b''):
            dst.write(chunk)


def _tar_add_file(tf, abs_path, arc, timestamp=None):
    """按条目类型写入 tar：符号链接保留目标路径，普通文件按类型设置权限。

    timestamp 为 None 时条目时间取源文件 mtime。
    """
    st = os.lstat(abs_path)
    info = tarfile.TarInfo(arc)
    info.mtime = int(st.st_mtime if timestamp is None else timestamp)
    info.uid = info.gid = 0
    info.uname = info.gname = 'root'
    if stat.S_ISLNK(st.st_mode):
        info.type = tarfile.SYMTYPE
        info.linkname = os.readlink(abs_path)
        info.mode = 0o777
        tf.addfile(info)
        return
    info.type = tarfile.REGTYPE
    info.mode = _archive_file_mode(abs_path)
    info.size = st.st_size
    with open(abs_path, 'rb') as f:
        tf.addfile(info, f)


def build_package(collector, staging_root, pkg_root_name, version, manifest_text,
                  sums_text, fmt, out_path, virtual_files=None,
                  source_date_epoch=None, generated_time=None):
    """把收集到的文件写成压缩包。staging_root 用于统一路径前缀。

    source_date_epoch 非 None 时，所有条目与 gzip 头统一使用该时间（可复现构建）。
    为 None 时普通条目用源文件 mtime，生成的文件用 generated_time
    （默认取收集产物中的最新 mtime），因此输入不变时连续两次打包字节一致。
    """
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    virtual_files = virtual_files or {}
    version_text = version + '\n'
    # 条目顺序影响压缩结果，按包内路径排序，避免文件系统遍历顺序差异
    items = sorted(collector.items, key=lambda kv: kv[0])
    if generated_time is None:
        generated_time = (source_date_epoch if source_date_epoch is not None
                          else _default_archive_time(collector))

    def write_tar(tf):
        for rel, abs_path in items:
            _tar_add_file(tf, abs_path, '%s/%s' % (pkg_root_name, rel),
                          source_date_epoch)
        for rel in sorted(virtual_files):
            _write_virtual(None, tf, '%s/%s' % (pkg_root_name, rel),
                           virtual_files[rel], generated_time)
        _write_virtual(None, tf, '%s/MANIFEST.txt' % pkg_root_name, manifest_text,
                       generated_time)
        _write_virtual(None, tf, '%s/SHA256SUMS' % pkg_root_name, sums_text,
                       generated_time)
        _write_virtual(None, tf, '%s/VERSION' % pkg_root_name, version_text,
                       generated_time)

    if fmt == 'zip':
        with zipfile.ZipFile(out_path, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as zf:
            for rel, abs_path in items:
                _zip_add_file(zf, abs_path, '%s/%s' % (pkg_root_name, rel),
                              source_date_epoch)
            for rel in sorted(virtual_files):
                _write_virtual(zf, None, '%s/%s' % (pkg_root_name, rel),
                               virtual_files[rel], generated_time)
            _write_virtual(zf, None, '%s/MANIFEST.txt' % pkg_root_name, manifest_text,
                           generated_time)
            _write_virtual(zf, None, '%s/SHA256SUMS' % pkg_root_name, sums_text,
                           generated_time)
            _write_virtual(zf, None, '%s/VERSION' % pkg_root_name, version_text,
                           generated_time)
    elif fmt == 'tgz':
        # gzip 头里的 mtime 默认取墙钟时间，必须显式指定，否则包摘要不稳定；
        # filename 置空避免写入本机绝对路径。
        with open(out_path, 'wb') as raw:
            with gzip.GzipFile(fileobj=raw, mode='wb', compresslevel=9,
                               mtime=generated_time, filename='') as gz:
                with tarfile.open(fileobj=gz, mode='w|') as tf:
                    write_tar(tf)
    else:
        with tarfile.open(out_path, 'w:bz2') as tf:
            write_tar(tf)

    return out_path


# --------------------------------------------------------------------------
# 主流程
# --------------------------------------------------------------------------

def resolve_lib_dir(lib_dir_opt, config, arch_tag):
    """解析唯一的 duilib 库目录（CM-5 D-2 方案 B）。

    显式 --lib-dir 优先；否则 Windows 为 lib/<arch>/<Config>、
    Unix 为 lib/<Config>。目录不存在直接失败，不在多个候选之间回退。
    """
    if lib_dir_opt:
        path = lib_dir_opt if os.path.isabs(lib_dir_opt) \
            else os.path.join(REPO_ROOT, lib_dir_opt)
        if not os.path.isdir(path):
            raise PackageError('--lib-dir 不存在：%s' % path)
        return path

    if is_windows_host():
        parts = ['lib', arch_tag, config]
    else:
        parts = ['lib', config]
    path = os.path.join(REPO_ROOT, *parts)
    if not os.path.isdir(path):
        raise PackageError('库目录不存在：%s（请先完成 %s|%s 的构建）'
                           % (_rel(path), arch_tag, config))
    return path


def verify_package(path):
    """校验已有发布包：布局、MANIFEST、SHA256SUMS、文档内相对链接。

    返回 True 表示通过（无错误），False 表示存在错误；告警不影响返回值。
    """
    if not os.path.isfile(path):
        eprint('[ERROR] 包不存在：%s' % path)
        return False

    errors = []
    warnings = []
    try:
        if path.lower().endswith('.zip'):
            archive = zipfile.ZipFile(path)
            members = {info.filename: info for info in archive.infolist()
                       if not info.is_dir()}

            def read(name):
                with archive.open(name) as fp:
                    return fp.read()

            def is_symlink(name):
                return stat.S_ISLNK((members[name].external_attr >> 16) & 0xFFFF)
        else:
            archive = tarfile.open(path, 'r:*')
            members = {m.name: m for m in archive.getmembers()
                       if m.isfile() or m.issym()}

            def read(name):
                fp = archive.extractfile(name)
                return fp.read() if fp is not None else b''

            def is_symlink(name):
                return members[name].issym()
    except (OSError, zipfile.BadZipFile, tarfile.TarError) as exc:
        eprint('[ERROR] 无法打开包 %s：%s' % (path, exc))
        return False

    try:
        files = [name for name in members if not is_symlink(name)]
        symlinks = [name for name in members if is_symlink(name)]
        roots = sorted({name.split('/', 1)[0] for name in members})
        if len(roots) != 1:
            errors.append('包根目录不唯一：%s' % ', '.join(roots[:5]))
            root = ''
        else:
            root = roots[0] + '/'

        def has(rel):
            return root + rel in members

        for required in ('MANIFEST.txt', 'SHA256SUMS'):
            if not has(required):
                errors.append('缺少 %s' % required)

        headers = [name for name in files
                   if name.startswith(root + 'include/duilib/')]
        if not headers:
            errors.append('缺少 include/duilib/ 头文件')
        libs = [name for name in files
                if name.startswith(root + 'lib/')
                and (name.endswith('.a') or name.endswith('.lib'))]
        if not libs:
            errors.append('缺少 lib/ 静态库（.a / .lib）')

        sums = {}
        if has('SHA256SUMS'):
            text = read(root + 'SHA256SUMS').decode('utf-8', 'replace')
            for line in text.splitlines():
                match = re.match(r'^([0-9a-fA-F]{64})\s+\*?(.+)$', line)
                if not match:
                    if line.strip():
                        warnings.append('SHA256SUMS 行格式异常：%s' % line[:60])
                    continue
                sums[match.group(2)] = match.group(1).lower()

        # MANIFEST.txt / VERSION 等虚拟文件按设计不写入 SHA256SUMS
        virtual = {root + 'MANIFEST.txt', root + 'VERSION'}
        listed = {root + rel for rel in sums}
        actual = set(files) - {root + 'SHA256SUMS'} - virtual
        for name in sorted(actual - listed):
            warnings.append('未登记到 SHA256SUMS：%s' % name[len(root):])
        for name in sorted(listed - actual):
            errors.append('SHA256SUMS 登记了包内不存在的文件：%s' % name[len(root):])

        mismatches = 0
        for rel, digest in sums.items():
            name = root + rel
            if name not in files:
                continue
            if hashlib.sha256(read(name)).hexdigest().lower() != digest:
                mismatches += 1
                errors.append('SHA256 不匹配：%s' % rel)
                if mismatches >= 20:
                    errors.append('…（已发现 %d 处不匹配，不再逐个列出）' % mismatches)
                    break

        # 文档相对链接：只检查 SDK 文档（跳过 LICENSES/ 下逐字复制的上游文本）
        for name in [item for item in files if item.endswith('.md')
                     and not item.startswith(root + 'LICENSES/')]:
            text = read(name).decode('utf-8', 'replace')
            base = os.path.dirname(name)
            for target in re.findall(r'\[[^\]]*\]\(([^)\s]+)\)', text):
                if target.startswith(('#', 'http://', 'https://', 'mailto:')):
                    continue
                link = target.split('#', 1)[0]
                if not link or link.startswith('/'):
                    continue
                dest = os.path.normpath(os.path.join(base, link)).replace('\\', '/')
                if dest not in members and not any(
                        item.startswith(dest + '/') for item in members):
                    errors.append('文档链接失效：%s -> %s'
                                  % (name[len(root):], target))

        if symlinks:
            warnings.append('包内含 %d 个符号链接（未计入 SHA256SUMS）' % len(symlinks))
    finally:
        archive.close()

    for item in warnings:
        sys.stdout.write('[WARN] %s\n' % item)
    for item in errors:
        sys.stdout.write('[ERROR] %s\n' % item)

    if errors:
        sys.stdout.write('[FAIL] %s：%d 个条目，%d 个错误，%d 个告警\n'
                         % (path, len(members), len(errors), len(warnings)))
        return False
    sys.stdout.write('[OK] %s：%d 个条目（%d 文件 / %d 链接），%d 个头文件，'
                     '%d 个库，SHA 全部匹配\n'
                     % (path, len(members), len(files), len(symlinks),
                        len(headers), len(libs)))
    return True


def _parse_config(value):
    """--config 大小写不敏感，统一规范化为 Release / Debug。"""
    norm = value.strip().lower()
    if norm == 'release':
        return 'Release'
    if norm == 'debug':
        return 'Debug'
    raise argparse.ArgumentTypeError('配置只能是 Release 或 Debug：%r' % value)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description='nim_duilib 发布打包脚本',
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument('--config', type=_parse_config, default='Release',
                        metavar='{Release,Debug}',
                        help='库配置，决定读取 lib/.../Release 或 lib/.../Debug'
                             '（默认 Release）')
    parser.add_argument('--arch', choices=('x64', 'x86', 'arm64'), default=None,
                        help='库架构；Windows 影响库目录，所有平台影响包名'
                             '（默认主机架构）')
    parser.add_argument('--lib-dir', default=None,
                        help='显式指定 duilib 库目录，此时 --arch/--config 不再参与'
                             '路径计算，但仍决定包名与示例过滤')
    parser.add_argument('--skia-lib-dir', default=None,
                        help='Skia 静态库目录（默认按平台/工具链探测，'
                             '如 ../skia/out/llvm.<arch>.<config>、'
                             'mingw64-gcc.<arch>.release（独立 MinGW-w64）或 '
                             'gcc.<arch>.release（MSYS2）；'
                             'DLL 模式为 llvm.<arch>.release.md；'
                             'MinGW 会按目录内是否存在 .a 校验）')
    parser.add_argument('--mingw-variant', choices=('auto', 'gcc', 'llvm'),
                        default='auto',
                        help='MinGW 编译器变体（默认从静态库符号自动判断）；'
                             'gcc 与 llvm-mingw 产物绑定不同 C++ 运行时，'
                             '包名会追加 -mingw-gcc / -mingw-llvm'
                             '（MSYS2 CLANG64 使用 libc++，归为 llvm）')
    parser.add_argument('--sdl-dir', default=None,
                        help='SDL3 根目录（默认 ../SDL3）')
    parser.add_argument('--with-sdl', action='store_true',
                        help='Windows 下额外打包 SDL3（Unix 始终打包，无需该参数）')
    parser.add_argument('--with-cef', action='store_true',
                        help='打包 CEF：头文件 + libcef.lib/wrapper + 运行时'
                             '（Windows/Linux；macOS 不自动收集，按发布文档手动部署；'
                             '体积巨大，默认不带）')
    parser.add_argument('--cef-version', choices=('latest', '109'), default='latest',
                        help='配合 --with-cef 选择 CEF 版本（默认 latest）')
    parser.add_argument('--msvc-runtime', choices=('auto', 'mt', 'md'), default='auto',
                        help='MSVC 运行库：auto（DLL 模式 MD，否则 MT）/ mt / md；'
                             '仅影响包内元数据与下游校验，不改变已构建产物')
    parser.add_argument('--with-examples', action='store_true',
                        help='包含示例程序与全部示例私有资源（默认只打 SDK）')
    parser.add_argument('--format', choices=('zip', 'tgz', 'tarbz2'), default=None,
                        help='压缩格式，默认 Windows 用 zip、其它用 tgz')
    parser.add_argument('--out-dir', default=DEFAULT_OUT_DIR, help='输出目录，默认 dist/')
    parser.add_argument('--list', action='store_true',
                        help='只打印将收集的文件清单，不实际打包')
    parser.add_argument('--allow-missing', action='store_true',
                        help='关键产物缺失时仍继续出包（仅用于排查，'
                             '发布流程不应使用）')
    parser.add_argument('--verify', metavar='PACKAGE', default=None,
                        help='只校验已有包：布局、MANIFEST、SHA256SUMS 与文档相对链接，'
                             '不做打包；通过返回 0，发现错误返回 1')
    args = parser.parse_args(argv)

    if args.verify:
        return 0 if verify_package(args.verify) else 1

    version = read_version()
    if version is None:
        return 1

    try:
        source_date_epoch = read_source_date_epoch()
    except PackageError as exc:
        eprint('[ERROR] %s' % exc)
        return 1

    sys_tag, host_arch = detect_platform_tag()
    arch_tag = args.arch or host_arch
    config = args.config
    fmt = args.format
    if fmt is None:
        fmt = 'zip' if sys_tag == 'win' else 'tgz'

    try:
        lib_dir = resolve_lib_dir(args.lib_dir, config, arch_tag)
        toolchain = detect_windows_toolchain(lib_dir) if sys_tag == 'win' else None
        mingw_variant = None
        if toolchain == 'mingw':
            mingw_variant = resolve_mingw_variant(lib_dir, args.mingw_variant)
        if toolchain == 'mingw' and args.with_cef:
            raise PackageError('--with-cef 仅支持 MSVC；MinGW（含 MSYS2）不支持 CEF')
        if args.with_cef and sys_tag != 'win' and args.cef_version != 'latest':
            raise PackageError('--cef-version 109 仅适用于 Windows；'
                               'Linux 只有 latest')
        skia_dir = resolve_skia_lib_dir(args.skia_lib_dir, config, arch_tag,
                                        toolchain, mingw_variant,
                                        dll_mode=(toolchain == 'msvc' and detect_dll_mode()))

        collector = Collector(allow_missing=args.allow_missing)
        n_headers = collect_headers(collector, args.with_cef, args.with_sdl, sys_tag == 'win')
        n_libs = collect_libs(collector, lib_dir, config,
                              args.with_cef, args.cef_version)
        n_skia = collect_skia_libs(collector, skia_dir, toolchain)
        n_sdl = collect_sdl(collector, args.sdl_dir, config, args.with_sdl)
        n_wv2 = collect_webview2_loader(collector, arch_tag, toolchain)
        n_dll = collect_duilib_runtime_dll(collector, arch_tag, config)
        n_jpeg = collect_jpeg_turbo(collector, arch_tag, toolchain)
        n_cef = 0
        n_cef_rt = 0
        if args.with_cef:
            n_cef = collect_cef(collector, sys_tag, arch_tag, args.cef_version)
            n_cef_rt = collect_cef_runtime(collector, sys_tag, arch_tag,
                                           args.cef_version)
    except PackageError as exc:
        eprint('[ERROR] %s' % exc)
        return 1

    n_res = collect_resources(collector, args.with_examples)
    n_lic = collect_licenses(collector, skia_dir)
    doc_files = collect_docs(collector)
    n_docs = len(doc_files)
    n_exe = collect_examples(collector, config, sys_tag == 'win') \
        if args.with_examples else 0

    try:
        meta = build_package_meta(collector, version, sys_tag, arch_tag, config,
                                  toolchain, args.with_cef, args.cef_version,
                                  args.msvc_runtime, mingw_variant)
        n_integration, gen_files = collect_integration_files(collector, meta)
        n_generated = len(gen_files)
        virtual_files = dict(gen_files)
        virtual_files.update(doc_files)
    except PackageError as exc:
        eprint('[ERROR] %s' % exc)
        return 1
    collector.sort()

    print('版本      : %s' % version)
    print('平台      : %s-%s' % (sys_tag, arch_tag))
    print('配置      : %s' % config)
    if sys_tag == 'win' and toolchain:
        if toolchain == 'mingw':
            toolchain_text = ('MinGW（llvm-mingw）' if mingw_variant == 'llvm'
                              else 'MinGW（gcc）')
        else:
            toolchain_text = 'MSVC'
        print('工具链    : %s' % toolchain_text)
    print('库目录    : %s' % _rel(lib_dir))
    print('Skia 目录 : %s' % _rel(skia_dir))
    print('格式      : %s' % fmt)
    print('包含示例  : %s' % ('是' if args.with_examples else '否'))
    print('-' * 60)
    print('头文件    : %d' % n_headers)
    print('静态库    : %d' % n_libs)
    print('Skia 库   : %d' % n_skia)
    print('SDL3      : %d' % n_sdl)
    print('WebView2  : %d' % n_wv2)
    print('DLL运行库 : %d' % n_dll)
    print('JPEG      : %d' % n_jpeg)
    print('CEF       : %d' % n_cef)
    print('CEF运行时 : %d' % n_cef_rt)
    print('资源文件  : %d' % n_res)
    print('许可证    : %d' % n_lic)
    print('文档      : %d' % n_docs)
    print('示例程序  : %d' % n_exe)
    print('接入文件  : %d（生成 %d）'
          % (n_integration + n_generated, n_generated))
    print('合计      : %d 个文件，%s'
          % (len(collector.items) + len(virtual_files),
             human_size(collector.total_size())))
    if collector.warnings:
        print('-' * 60)
        print('告警：')
        for w in collector.warnings:
            print('  ! %s' % w)

    if args.list:
        print('-' * 60)
        for rel, _abs in collector.items:
            print('  %s' % rel)
        for rel in sorted(virtual_files):
            if rel.startswith('docs/'):
                print('  %s' % rel)
            else:
                print('  [gen] %s' % rel)
        return 0

    if collector.critical:
        eprint('[ERROR] 关键产物缺失，已阻止出包：')
        for msg in collector.critical:
            eprint('  - %s' % msg)
        eprint('  仅需先出包排查时可加 --allow-missing（不要用于发布）。')
        return 1

    pkg_root_name = build_package_name(
        version, sys_tag, arch_tag, config,
        args.with_sdl, args.with_cef, args.cef_version, args.with_examples,
        mingw_variant=mingw_variant, is_dll=meta['is_dll'])
    package_time = (source_date_epoch if source_date_epoch is not None
                    else _default_archive_time(collector))
    manifest_text, sums_text = write_manifest(
        collector, pkg_root_name, version, sys_tag, arch_tag, config,
        args.with_examples, args.with_sdl, args.with_cef, args.cef_version,
        toolchain, virtual_files=virtual_files,
        msvc_runtime=meta['msvc_runtime'], package_time=package_time,
        mingw_variant=mingw_variant)

    ext = {'zip': '.zip', 'tgz': '.tar.gz', 'tarbz2': '.tar.bz2'}[fmt]
    out_path = os.path.join(args.out_dir, pkg_root_name + ext)

    try:
        build_package(collector, REPO_ROOT, pkg_root_name, version, manifest_text,
                      sums_text, fmt, out_path, virtual_files=virtual_files,
                      source_date_epoch=source_date_epoch,
                      generated_time=package_time)
    except Exception as exc:  # noqa: BLE001 - 顶层兜底，避免半成品包
        if os.path.exists(out_path):
            os.remove(out_path)
        eprint('[ERROR] 打包失败：%s' % exc)
        return 2

    print('-' * 60)
    print('产物      : %s（%s）' % (out_path, human_size(os.path.getsize(out_path))))
    return 0


if __name__ == '__main__':
    sys.exit(main())