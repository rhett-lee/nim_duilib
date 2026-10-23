#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""nim_duilib 发布打包脚本。

从已构建好的产物目录收集文件，按SDK 契约打包为压缩包，不参与编译。

设计约束（改动前务必先读）：

1. 本仓库**没有根 CMakeLists.txt**，25 个目录各自独立 project()，靠
   build/msvc_build.bat、build/linux_build.sh 等脚本逐个 configure + build。
   CPack 需要一个顶层项目才能挂载，因此这里不走 CPack，改用本脚本。
   不要试图"补 install() 规则"来替代本脚本 —— 没有任何单一 CMake 项目
   能覆盖全部产物。

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
   - WebView2 loader 库（Windows：静态 loader + 动态 loader 导入库 + DLL）；
   - turbojpeg-static（Windows：VS 默认启用 libjpeg-turbo，duilib.lib 有符号）；
   - SDL3（Unix 始终；Windows 需 --with-sdl；动态库时同时带运行期 DLL）；
   - CEF 开发包（仅 --with-cef：头文件 + libcef.lib + wrapper）；
   - DLL 模式运行库（Windows：bin/duilib*.dll，lib/ 中为对应的导入库）。
   CEF 运行时（libcef.dll、*.pak、icudtl.dat、locales/）**不在本仓库内**，
   由使用者自行放入，含 CEF 的示例程序打包后不能直接运行，清单中会标注。

6. Windows 工具链按库目录内容自动判断：存在 duilib.lib 为 MSVC，
   存在 libduilib*.a 为 MinGW。两者产物不同，不能混用：
   - MSVC ：库为 .lib，Skia 目录 llvm.<arch>.<release|debug>；
   - MinGW：静态库 .a、导入库 .dll.a，Skia 目录为仅有的 release 变体
            mingw64-gcc.<arch>.release / mingw64-llvm.<arch>.release。
   因此 MinGW 的清单链接写法是 -lxxx，MSVC 是 xxx.lib。

7. 包内还要提供“下游接入层”，由 packaging/ 下的模板渲染生成（不手工落盘）：
   - Windows+MSVC：msvc/nim_duilib.props、nim_duilib.targets、nim_duilib.sdk.props；
   - 全平台：lib/cmake/nim_duilib/*.cmake（find_package(nim_duilib)）。
   元数据（架构/配置/工具链/DLL/SDL/CEF/运行库/运行期 DLL）必须与实际
   收集结果一致，否则下游会链接到错误产物。

用法：
    python build/pack.py                      # Release SDK 包（不含示例）
    python build/pack.py --config Debug       # Debug SDK 包（包名带 -debug）
    python build/pack.py --with-examples      # SDK + 示例程序
    python build/pack.py --list               # 只打印将收集的文件，不打包
    python build/pack.py --format tgz         # 强制 tgz（默认 Windows 用 zip）
    python build/pack.py --lib-dir lib/x86/Debug  # 显式指定库目录
    python build/pack.py --with-sdl           # Windows 额外打包 SDL3
    python build/pack.py --with-cef           # 额外打包 CEF 开发包
    python build/pack.py --msvc-runtime md    # 指定 MSVC 运行库（默认按模式推断）
    python build/pack.py --out-dir dist       # 指定输出目录

退出码：0 成功；1 参数或产物缺失；2 打包异常。
"""

import argparse
import hashlib
import io
import os
import platform
import re
import stat
import sys
import tarfile
import time
import zipfile

# --------------------------------------------------------------------------
# 路径
# --------------------------------------------------------------------------

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
VERSION_FILE = os.path.join(REPO_ROOT, 'VERSION')
DEFAULT_OUT_DIR = os.path.join(REPO_ROOT, 'dist')

DUIBIB_SRC_DIR = os.path.join(REPO_ROOT, 'duilib')
THIRD_PARTY_DIR = os.path.join(DUIBIB_SRC_DIR, 'third_party')
WEBVIEW2_NATIVE_DIR = os.path.join(
    THIRD_PARTY_DIR, 'Microsoft.Web.WebView2', 'build', 'native')
JPEG_TURBO_DIR = os.path.join(THIRD_PARTY_DIR, 'libjpeg-turbo')
CEF_DIR = os.path.join(THIRD_PARTY_DIR, 'libcef')
RESOURCES_DIR = os.path.join(REPO_ROOT, 'bin', 'resources')
LICENSES_DIR = os.path.join(REPO_ROOT, 'licenses')
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

# CEF wrapper / 源码目录：--with-cef 时按版本只收集一套。
CEF_WRAPPER_BY_VERSION = {
    'latest': 'libcef_dll_wrapper',
    '109': 'libcef_dll_wrapper_109',
}
CEF_SRC_BY_VERSION = {
    'latest': 'libcef_win',
    '109': 'libcef_win_109',
}

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

# --with-examples 关闭时，resources/themes/default/ 下只保留 public 子目录
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


def read_version():
    """读取根目录 VERSION 文件。格式：MAJOR.MINOR.PATCH.BUILD"""
    if not os.path.isfile(VERSION_FILE):
        eprint('[ERROR] 找不到版本文件：%s' % VERSION_FILE)
        return None
    with io.open(VERSION_FILE, 'r', encoding='utf-8-sig') as f:
        raw = f.read().strip()
    parts = raw.split('.')
    if len(parts) != 4 or not all(p.isdigit() for p in parts):
        eprint('[ERROR] VERSION 格式非法（应为 MAJOR.MINOR.PATCH.BUILD）：%r' % raw)
        return None
    return raw


def detect_platform_tag():
    """返回 (系统标签, 架构标签)，用于包名。"""
    sysname = platform.system().lower()
    if sysname.startswith('win'):
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


def iter_files(root):
    """遍历 root 下所有文件，返回 (绝对路径, 相对 root 的路径)。"""
    for dirpath, _dirnames, filenames in os.walk(root):
        for name in filenames:
            abs_path = os.path.join(dirpath, name)
            yield abs_path, os.path.relpath(abs_path, root).replace(os.sep, '/')


# --------------------------------------------------------------------------
# 收集
# --------------------------------------------------------------------------

class Collector(object):
    """收集结果容器。items 为 (包内相对路径, 磁盘绝对路径)。"""

    def __init__(self):
        self.items = []
        self._seen = set()
        self.warnings = []

    def add(self, rel_path, abs_path):
        rel = rel_path.replace(os.sep, '/').lstrip('/')
        if rel in self._seen:
            return False
        if not os.path.isfile(abs_path):
            return False
        self._seen.add(rel)
        self.items.append((rel, abs_path))
        return True

    def warn(self, msg):
        self.warnings.append(msg)

    def total_size(self):
        return sum(os.path.getsize(p) for _rel, p in self.items)

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
            collector.warn('third_party/%s 不存在，头文件可能不完整' % os.path.join(*parts))
        else:
            prefix = 'include/duilib/third_party/' + '/'.join(parts)
            for abs_path, rel in iter_files(root):
                if not rel.endswith(('.h', '.hpp', '.inl')):
                    continue
                if collector.add('%s/%s' % (prefix, rel), abs_path):
                    count += 1

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
    if not platform.system().lower().startswith('win'):
        return None
    if os.path.isfile(os.path.join(lib_dir, 'duilib.lib')):
        return 'msvc'
    for name in os.listdir(lib_dir):
        low = name.lower()
        if low.startswith('libduilib') and low.endswith('.a'):
            return 'mingw'
    return None


def _scan_lib_dir(lib_dir):
    """扫描唯一库目录，返回 {基名: 文件名}。

    配置只由目录决定（Release/Debug），文件名不再参与判断（D-2）。
    Windows 同时接受 MSVC 的 .lib 与 MinGW 的 .a（含 .dll.a 导入库）；
    Unix 只接受 .a。
    """
    is_windows = platform.system().lower().startswith('win')
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


def collect_skia_libs(collector, skia_lib_dir):
    """收集 Skia 全部静态库（P4-B3：整目录打包）。

    Windows 同时接受 MSVC 的 .lib 与 MinGW 的 .a，Unix 只接受 .a。
    """
    is_windows = platform.system().lower().startswith('win')
    exts = ('.lib', '.a') if is_windows else ('.a',)
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


def resolve_skia_lib_dir(explicit_dir, config, arch_tag, toolchain=None):
    """解析 Skia 库目录。显式指定优先，否则按平台/配置/工具链探测。"""
    if explicit_dir:
        path = explicit_dir if os.path.isabs(explicit_dir) \
            else os.path.join(REPO_ROOT, explicit_dir)
        if not os.path.isdir(path):
            raise PackageError('--skia-lib-dir 不存在：%s' % path)
        return path

    out_dir = os.path.join(SKIA_SRC_ROOT, 'out')
    is_windows = platform.system().lower().startswith('win')
    if is_windows:
        # MSVC：llvm.<arch>.debug / release
        msvc_variant = os.path.join(
            out_dir, 'llvm.%s.%s' % (
                arch_tag, 'debug' if config == 'Debug' else 'release'))
        # MinGW：Skia 只有 release 变体，目录名带 mingw64 前缀
        mingw_variants = [
            os.path.join(out_dir, 'mingw64-gcc.%s.release' % arch_tag),
            os.path.join(out_dir, 'mingw64-llvm.%s.release' % arch_tag),
        ]
        if toolchain == 'mingw':
            candidates = mingw_variants
        elif toolchain == 'msvc':
            candidates = [msvc_variant]
        else:
            candidates = [msvc_variant] + mingw_variants
    else:
        candidates = [
            os.path.join(out_dir, 'llvm.%s.release' % arch_tag),
            os.path.join(out_dir, 'gcc.%s.release' % arch_tag),
        ]

    for path in candidates:
        if os.path.isdir(path):
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
    is_windows = platform.system().lower().startswith('win')
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
                               % _rel(dll))
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
    count += _collect_tree(collector, os.path.join(root, 'licenses'),
                           'licenses/SDL3')
    return count


def collect_webview2_loader(collector, arch_tag):
    """收集 WebView2 loader 库（默认随包，P4-B4/Q11）。"""
    if not platform.system().lower().startswith('win'):
        return 0
    arch_dir = {'x64': 'x64', 'x86': 'x86'}.get(arch_tag)
    count = 0
    if arch_dir is None:
        collector.warn('WebView2 没有 %s 架构的 loader，已跳过' % arch_tag)
        return count
    root = os.path.join(WEBVIEW2_NATIVE_DIR, arch_dir)
    for name in ('WebView2Loader.dll.lib', 'WebView2LoaderStatic.lib'):
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
    if not platform.system().lower().startswith('win'):
        return 0
    base = 'duilib32' if arch_tag == 'x86' else 'duilib'
    if config == 'Debug':
        base += '_d'
    full = os.path.join(BIN_DIR, base + '.dll')
    if not os.path.isfile(full):
        return 0
    return 1 if collector.add('bin/%s.dll' % base, full) else 0


def collect_jpeg_turbo(collector, arch_tag):
    """收集 libjpeg-turbo 静态库（Windows 默认随包，P4-B4/Q12）。

    头文件只在库内部使用，不对外转发（图片解码头文件不打包）。
    """
    if not platform.system().lower().startswith('win'):
        return 0
    arch_dir = {'x64': 'x64', 'x86': 'x86'}.get(arch_tag)
    count = 0
    if arch_dir is None:
        collector.warn('libjpeg-turbo 没有 %s 架构的预编译库，已跳过' % arch_tag)
        return count
    root = os.path.join(JPEG_TURBO_DIR, 'libjpeg-turbo6-win-vc-%s' % arch_dir)
    lib = os.path.join(root, 'lib', 'turbojpeg-static.lib')
    if os.path.isfile(lib):
        if collector.add('lib/turbojpeg-static.lib', lib):
            count += 1
    else:
        collector.warn('缺少 turbojpeg-static.lib：%s' % _rel(lib))
    return count


def collect_cef(collector, arch_tag, cef_version):
    """收集 CEF 开发包：头文件 + libcef.lib（wrapper 在 collect_libs 中）。"""
    if not platform.system().lower().startswith('win'):
        raise PackageError('--with-cef 目前仅支持 Windows 打包')
    src_name = CEF_SRC_BY_VERSION[cef_version]
    arch_dir = {'x64': 'x64', 'x86': 'Win32'}.get(arch_tag)
    if arch_dir is None:
        raise PackageError('CEF 没有 %s 架构的库' % arch_tag)

    src_root = os.path.join(CEF_DIR, src_name)
    lib = os.path.join(src_root, 'lib', arch_dir, 'libcef.lib')
    if not os.path.isfile(lib):
        raise PackageError('缺少 %s' % _rel(lib))

    count = 0
    if collector.add('lib/libcef.lib', lib):
        count += 1
    count += _collect_tree(collector, os.path.join(src_root, 'include'),
                           'include/duilib/third_party/libcef/%s/include' % src_name)
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
        collector.warn('资源目录不存在：%s' % RESOURCES_DIR)
        return 0
    count = 0
    for abs_path, rel in iter_files(RESOURCES_DIR):
        if not _resource_rel_ok(rel, with_examples):
            continue
        if collector.add('resources/' + rel, abs_path):
            count += 1
    return count


def collect_licenses(collector, skia_lib_dir):
    if not os.path.isdir(LICENSES_DIR):
        collector.warn('许可证目录不存在：%s' % LICENSES_DIR)
        return 0
    count = 0
    for abs_path, rel in iter_files(LICENSES_DIR):
        if collector.add('licenses/' + rel, abs_path):
            count += 1
    # 仓库根的 LICENSE 也放进包
    root_license = os.path.join(REPO_ROOT, 'LICENSE')
    if os.path.isfile(root_license):
        collector.add('LICENSE', root_license)
        count += 1
    # Skia 的许可证不在本仓库内（Skia 源码树同级）；仓库 licenses/ 已有时不重复
    skia_root = os.path.dirname(os.path.dirname(skia_lib_dir))
    skia_license = os.path.join(skia_root, 'LICENSE')
    has_skia_license = any(r.startswith('licenses/skia') for r, _ in collector.items)
    if not has_skia_license and os.path.isfile(skia_license):
        if collector.add('licenses/skia/LICENSE', skia_license):
            count += 1
    elif not has_skia_license:
        collector.warn('缺少 Skia 许可证：%s' % skia_license)
    return count


def collect_examples(collector, config, is_windows):
    """收集示例程序可执行文件与运行时 dll。

    Release 包跳过 VS 的 Debug 可执行文件（*_d.exe）；Debug 包因 CMake 的
    Debug exe 与 Release 同名，无法区分，全部收集并告警。
    运行时 dll 仅 Windows 收集（此处只有 WebView2Loader.dll；
    duilib 与 SDL3 的运行的 DLL 由各自 collect 函数收集）。
    """
    if not os.path.isdir(BIN_DIR):
        collector.warn('bin 目录不存在：%s' % BIN_DIR)
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
                       toolchain, with_cef, cef_version, msvc_runtime):
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

    # WebView2 / JPEG / CEF 的预编译产物只有 MSVC 可用的 .lib；
    # MinGW 包即使随包带了这些文件，也不能声明为可用特性。
    is_msvc = toolchain == 'msvc'
    has_webview2 = is_msvc and ('WebView2LoaderStatic.lib' in lib_names or
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

    if meta['system'] == 'win' and meta['arch'] in ('x64', 'x86'):
        manifest_name = 'duilib.%s.manifest' % meta['arch']
        manifest = os.path.join(MSVC_MANIFEST_DIR, manifest_name)
        if os.path.isfile(manifest):
            if collector.add('msvc/manifest/' + manifest_name, manifest):
                count += 1
        else:
            collector.warn('缺少 manifest：%s（VS 工程不会自动嵌入 Common Controls v6）'
                           % _rel(manifest))

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
                   toolchain=None, virtual_files=None, msvc_runtime=None):
    """生成包内 MANIFEST.txt 与 SHA256SUMS，并固化 SDK 使用契约。"""
    is_windows = sys_tag == 'win'
    is_mingw = toolchain == 'mingw'
    virtual_files = virtual_files or {}
    stems = _lib_stems(collector)
    lib_names = _lib_filenames(collector)
    bin_dlls = [r.split('/', 1)[1] for r, _abs in collector.items
                if r.startswith('bin/') and r.lower().endswith('.dll')]
    # SDL3 静态库：MSVC 的 SDL3-static.lib 或 MinGW 的 libSDL3.a；
    # 动态库：Windows 的导入库（SDL3.lib / libSDL3.dll.a）或 Unix 的 libSDL3.so*。
    sdl_static = ('SDL3-static.lib' in lib_names) or ('libSDL3.a' in lib_names)
    sdl_dynamic = (('SDL3.lib' in lib_names) or ('libSDL3.dll.a' in lib_names) or
                   any(n.startswith('libSDL3.so') or n.startswith('libSDL3.')
                       and n.endswith('.dylib') for n in lib_names))
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
        lines.append('工具链    : %s' % ('MinGW' if is_mingw else 'MSVC'))
    lines.append('打包时间  : %s' % time.strftime('%Y-%m-%d %H:%M:%S'))
    lines.append('包含示例  : %s' % ('是' if with_examples else '否'))
    lines.append('文件总数  : %d' % (len(collector.items) + len(virtual_files)))

    section('目录结构')
    lines.append('  include/duilib/**.h   对外头文件，保持仓库根相对路径')
    lines.append('                         用法：#include "duilib/duilib.h"')
    lines.append('  lib/                  静态库（.lib / .a），平面结构')
    lines.append('  resources/            主题、语言、字体资源')
    lines.append('  licenses/             第三方许可证')
    lines.append('  msvc/                 VS 接入：props/targets/manifest（仅 Windows+MSVC）')
    lines.append('  lib/cmake/nim_duilib/ CMake 接入：find_package(nim_duilib)')
    lines.append('  VERSION               版本号')

    section('开发接入')
    lines.append('1) 包含路径：include/')
    if with_cef:
        src_name = CEF_SRC_BY_VERSION[cef_version]
        lines.append('   使用 CEF 时追加：include/duilib/third_party/libcef/%s'
                     % src_name)
    if with_sdl or not is_windows:
        lines.append('   使用 SDL 时 <SDL3/SDL.h> 也由 include/ 覆盖')
    lines.append('2) 库路径  ：lib/')
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
    lines.append('  resources/（默认不复制，避免覆盖应用自己的主题）。')
    lines.append('  可设置 NIM_DUILIB_SET_MSVC_RUNTIME=OFF 退出运行库强制对齐；')
    lines.append('  NIM_DUILIB_ALLOW_{ARCH,CONFIG}_MISMATCH=ON 放行不匹配（不推荐）。')
    lines.append('  注意：发布包是单架构、单配置的，Debug/Release 需各用各的包。')

    section('运行期')
    lines.append('  resources/ 必须放在可执行文件同级目录（框架启动必需）。')
    if is_windows:
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
            lines.append('  CEF 运行时不在本包内，需从 libcef 发布包放入可执行文件同级：')
            lines.append('  libcef.dll、icudtl.dat、*.pak、locales/ 等。')
    else:
        lines.append('  Skia 为静态库；SDL3 为动态库，需让 libSDL3.so* 可被找到：')
        lines.append('  复制到可执行文件同级，或设置 LD_LIBRARY_PATH'
                     '（macOS：DYLD_LIBRARY_PATH）。')
        lines.append('  X11 / freetype / fontconfig 由系统提供，不随包。')

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

    if with_examples:
        section('示例程序')
        lines.append('  bin/*.exe             示例可执行文件，需与 resources/ 同级')
        missing_cef = [e for e in CEF_EXAMPLES
                       if any(r.endswith('/' + e) for r, _ in collector.items)]
        if missing_cef:
            lines.append('')
            lines.append('  [注意] 以下示例依赖 CEF 运行时，本仓库不含该运行时，')
            lines.append('         直接运行会失败。需自行从 libcef 发布包放入：')
            for e in missing_cef:
                lines.append('           %s' % e)
    else:
        section('资源说明')
        lines.append('  本包不含示例程序资源。resources/themes/ 下只保留了 public/ 与')
        lines.append('  global.xml；各示例的私有主题目录未包含。')

    section('校验')
    lines.append('  SHA256SUMS 可用 sha256sum -c SHA256SUMS 校验')

    manifest_text = '\n'.join(lines).replace('\n', '\r\n')

    sums = []
    for rel, abs_path in sorted(collector.items, key=lambda kv: kv[0]):
        sums.append('%s  %s' % (sha256_of(abs_path), rel))
    for rel in sorted(virtual_files):
        digest = hashlib.sha256(virtual_files[rel].encode('utf-8')).hexdigest()
        sums.append('%s  %s' % (digest, rel))
    sums_text = '\n'.join(sums) + '\n'

    return manifest_text, sums_text


def _write_virtual(zip_obj, tar_obj, arcname, text):
    """把内存中的文本同时（或分别）写入 zip / tar。

    zip_obj 与 tar_obj 只有一个非 None —— 调用方按目标格式二选一。
    两者都写入会因None 崩溃，且早期版本正是这么写的。
    """
    if zip_obj is not None:
        data = text.encode('utf-8')
        zip_obj.writestr(_zip_info(arcname, len(data)), data)
    if tar_obj is not None:
        raw = text.replace('\r\n', '\n').encode('utf-8')
        tinfo = tarfile.TarInfo(arcname)
        tinfo.size = len(raw)
        tinfo.mtime = int(time.time())
        tinfo.mode = 0o644
        tar_obj.addfile(tinfo, io.BytesIO(raw))


def _zip_info(arcname, size):
    info = zipfile.ZipInfo(arcname, date_time=time.localtime()[:6])
    info.compress_type = zipfile.ZIP_DEFLATED
    info.external_attr = 0o644 << 16
    return info


def build_package(collector, staging_root, pkg_root_name, manifest_text, sums_text,
                  fmt, out_path, virtual_files=None):
    """把收集到的文件写成压缩包。staging_root 用于统一路径前缀。"""
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    virtual_files = virtual_files or {}

    if fmt == 'zip':
        with zipfile.ZipFile(out_path, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as zf:
            for rel, abs_path in collector.items:
                arc = '%s/%s' % (pkg_root_name, rel)
                zf.write(abs_path, arc)
            for rel in sorted(virtual_files):
                _write_virtual(zf, None, '%s/%s' % (pkg_root_name, rel),
                               virtual_files[rel])
            _write_virtual(zf, None, '%s/MANIFEST.txt' % pkg_root_name, manifest_text)
            _write_virtual(zf, None, '%s/SHA256SUMS' % pkg_root_name, sums_text)
            _write_virtual(zf, None, '%s/VERSION' % pkg_root_name,
                           open(VERSION_FILE, 'r', encoding='utf-8-sig').read().strip() + '\n')
    else:
        gz_mode = 'w:gz' if fmt == 'tgz' else 'w:bz2'
        with tarfile.open(out_path, gz_mode) as tf:
            for rel, abs_path in collector.items:
                arc = '%s/%s' % (pkg_root_name, rel)
                info = tf.gettarinfo(abs_path, arcname=arc)
                info.uid = info.gid = 0
                info.uname = info.gname = 'root'
                info.mode = 0o644
                with open(abs_path, 'rb') as f:
                    tf.addfile(info, f)
            for rel in sorted(virtual_files):
                _write_virtual(None, tf, '%s/%s' % (pkg_root_name, rel),
                               virtual_files[rel])
            _write_virtual(None, tf, '%s/MANIFEST.txt' % pkg_root_name, manifest_text)
            _write_virtual(None, tf, '%s/SHA256SUMS' % pkg_root_name, sums_text)
            _write_virtual(None, tf, '%s/VERSION' % pkg_root_name,
                           open(VERSION_FILE, 'r', encoding='utf-8-sig').read().strip() + '\n')

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

    if platform.system().lower().startswith('win'):
        parts = ['lib', arch_tag, config]
    else:
        parts = ['lib', config]
    path = os.path.join(REPO_ROOT, *parts)
    if not os.path.isdir(path):
        raise PackageError('库目录不存在：%s（请先完成 %s|%s 的构建）'
                           % (_rel(path), arch_tag, config))
    return path


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
                             '如 ../skia/out/llvm.<arch>.<config> 或 '
                             'mingw64-gcc.<arch>.release）')
    parser.add_argument('--sdl-dir', default=None,
                        help='SDL3 根目录（默认 ../SDL3）')
    parser.add_argument('--with-sdl', action='store_true',
                        help='Windows 下额外打包 SDL3（Unix 始终打包，无需该参数）')
    parser.add_argument('--with-cef', action='store_true',
                        help='打包 CEF 开发包：头文件 + libcef.lib + wrapper'
                             '（体积巨大，默认不带）')
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
    args = parser.parse_args(argv)

    version = read_version()
    if version is None:
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
        skia_dir = resolve_skia_lib_dir(args.skia_lib_dir, config, arch_tag,
                                        toolchain)

        collector = Collector()
        n_headers = collect_headers(collector, args.with_cef, args.with_sdl, sys_tag == 'win')
        n_libs = collect_libs(collector, lib_dir, config,
                              args.with_cef, args.cef_version)
        n_skia = collect_skia_libs(collector, skia_dir)
        n_sdl = collect_sdl(collector, args.sdl_dir, config, args.with_sdl)
        n_wv2 = collect_webview2_loader(collector, arch_tag)
        n_dll = collect_duilib_runtime_dll(collector, arch_tag, config)
        n_jpeg = collect_jpeg_turbo(collector, arch_tag)
        n_cef = 0
        if args.with_cef:
            n_cef = collect_cef(collector, arch_tag, args.cef_version)
    except PackageError as exc:
        eprint('[ERROR] %s' % exc)
        return 1

    n_res = collect_resources(collector, args.with_examples)
    n_lic = collect_licenses(collector, skia_dir)
    n_exe = collect_examples(collector, config, sys_tag == 'win') \
        if args.with_examples else 0

    try:
        meta = build_package_meta(collector, version, sys_tag, arch_tag, config,
                                  toolchain, args.with_cef, args.cef_version,
                                  args.msvc_runtime)
        n_integration, virtual_files = collect_integration_files(collector, meta)
    except PackageError as exc:
        eprint('[ERROR] %s' % exc)
        return 1
    collector.sort()

    print('版本      : %s' % version)
    print('平台      : %s-%s' % (sys_tag, arch_tag))
    print('配置      : %s' % config)
    if sys_tag == 'win' and toolchain:
        print('工具链    : %s' % ('MinGW' if toolchain == 'mingw' else 'MSVC'))
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
    print('资源文件  : %d' % n_res)
    print('许可证    : %d' % n_lic)
    print('示例程序  : %d' % n_exe)
    print('接入文件  : %d（生成 %d）'
          % (n_integration + len(virtual_files), len(virtual_files)))
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
            print('  [gen] %s' % rel)
        return 0

    pkg_root_name = 'nim_duilib-%s-%s-%s' % (version, sys_tag, arch_tag)
    if config == 'Debug':
        pkg_root_name += '-debug'
    manifest_text, sums_text = write_manifest(
        collector, pkg_root_name, version, sys_tag, arch_tag, config,
        args.with_examples, args.with_sdl, args.with_cef, args.cef_version,
        toolchain, virtual_files=virtual_files,
        msvc_runtime=meta['msvc_runtime'])

    ext = {'zip': '.zip', 'tgz': '.tar.gz', 'tarbz2': '.tar.bz2'}[fmt]
    out_path = os.path.join(args.out_dir, pkg_root_name + ext)

    try:
        build_package(collector, REPO_ROOT, pkg_root_name, manifest_text,
                      sums_text, fmt, out_path, virtual_files=virtual_files)
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