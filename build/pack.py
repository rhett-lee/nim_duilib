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
     Core/WindowDropTarget_*、Core/SDL_MacOS.h）。
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
   - WebView2 loader 库（Windows：静态 loader + 动态 loader 导入库）；
   - turbojpeg-static（Windows：VS 默认启用 libjpeg-turbo，duilib.lib 有符号）；
   - SDL3（Unix 始终；Windows 需 --with-sdl）；
   - CEF 开发包（仅 --with-cef：头文件 + libcef.lib + wrapper）。
   CEF 运行时（libcef.dll、*.pak、icudtl.dat、locales/）**不在本仓库内**，
   由使用者自行放入，含 CEF 的示例程序打包后不能直接运行，清单中会标注。

用法：
    python build/pack.py                      # Release SDK 包（不含示例）
    python build/pack.py --config Debug       # Debug SDK 包（包名带 -debug）
    python build/pack.py --with-examples      # SDK + 示例程序
    python build/pack.py --list               # 只打印将收集的文件，不打包
    python build/pack.py --format tgz         # 强制 tgz（默认 Windows 用 zip）
    python build/pack.py --lib-dir lib/x86/Debug  # 显式指定库目录
    python build/pack.py --with-sdl           # Windows 额外打包 SDL3
    python build/pack.py --with-cef           # 额外打包 CEF 开发包
    python build/pack.py --out-dir dist       # 指定输出目录

退出码：0 成功；1 参数或产物缺失；2 打包异常。
"""

import argparse
import hashlib
import io
import os
import platform
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

# 外部依赖（与仓库同级），可用命令行参数覆盖
SKIA_SRC_ROOT = os.path.join(os.path.dirname(REPO_ROOT), 'skia')
SDL3_SRC_ROOT = os.path.join(os.path.dirname(REPO_ROOT), 'SDL3')

# --------------------------------------------------------------------------
# 收集规则
# --------------------------------------------------------------------------

# 内部实现头文件（相对 duilib/），不对外转发。
# 判定依据：只被库内 .cpp / .mm 包含，任何对外头文件都不引用。
# - Core/ZipStreamIO.h 直接 include zlib 的 minizip 头文件，只被 ZipManager.cpp 使用；
# - DropTarget / SDL_MacOS 系列是 Windows / SDL 平台的具体实现，只被 .cpp / .mm 使用。
INTERNAL_HEADER_FILES = (
    'Core/ControlDropTargetImpl_SDL.h',
    'Core/ControlDropTargetImpl_Windows.h',
    'Core/ControlDropTargetUtils.h',
    'Core/SDL_MacOS.h',
    'Core/WindowDropTarget_SDL.h',
    'Core/WindowDropTarget_Windows.h',
    'Core/ZipStreamIO.h',
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
    """Unix 静态库去掉 lib 前缀后匹配白名单（P4-B2）。

    libduilib-zlib.a -> duilib-zlib；libcef_dll_wrapper.a 去掉后为
    cef_dll_wrapper（不在白名单），保留原名匹配。
    """
    if stem.startswith('lib') and stem[3:] in wanted:
        return stem[3:]
    return stem


def _scan_lib_dir(lib_dir):
    """扫描唯一库目录，返回 {基名: 文件名}。

    配置只由目录决定（Release/Debug），文件名不再参与判断（D-2）。
    Windows 只接受 .lib，Unix 只接受 .a。
    """
    is_windows = platform.system().lower().startswith('win')
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
        if is_windows and ext != '.lib':
            continue
        if not is_windows and ext != '.a':
            continue
        base = _lib_base_name(stem, wanted)
        if base in wanted and base not in available:
            available[base] = name
    return available


def collect_libs(collector, lib_dir, config, with_cef, cef_version):
    """按白名单收集唯一库目录中的静态库。

    目录或主库 duilib 缺失直接失败（Q1）；其余第三方库缺失只告警。
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

    missing = [b for b in wanted if b not in available]
    if 'duilib' in missing:
        raise PackageError('未找到 duilib 主库（目录：%s），请先完成 %s 编译'
                           % (_rel(lib_dir), config))

    count = 0
    for base in wanted:
        name = available.get(base)
        if name is None:
            continue
        if collector.add('lib/' + name, os.path.join(lib_dir, name)):
            count += 1

    optional_missing = [b for b in missing if b != 'duilib']
    if optional_missing:
        collector.warn('缺少库：%s（目录：%s）'
                       % (', '.join(optional_missing), _rel(lib_dir)))
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
    """收集 Skia 全部静态库（P4-B3：整目录打包）。"""
    is_windows = platform.system().lower().startswith('win')
    ext = '.lib' if is_windows else '.a'
    count = 0
    for name in sorted(os.listdir(skia_lib_dir)):
        full = os.path.join(skia_lib_dir, name)
        if not os.path.isfile(full) or not name.lower().endswith(ext):
            continue
        if collector.add('lib/' + name, full):
            count += 1
    if count == 0:
        raise PackageError('Skia 目录中没有 %s 静态库：%s'
                           % (ext, _rel(skia_lib_dir)))
    return count


def resolve_skia_lib_dir(explicit_dir, config, arch_tag):
    """解析 Skia 库目录。显式指定优先，否则按平台/配置探测。"""
    if explicit_dir:
        path = explicit_dir if os.path.isabs(explicit_dir) \
            else os.path.join(REPO_ROOT, explicit_dir)
        if not os.path.isdir(path):
            raise PackageError('--skia-lib-dir 不存在：%s' % path)
        return path

    out_dir = os.path.join(SKIA_SRC_ROOT, 'out')
    is_windows = platform.system().lower().startswith('win')
    if is_windows:
        variant = 'llvm.%s.%s' % (
            arch_tag, 'debug' if config == 'Debug' else 'release')
        candidates = [os.path.join(out_dir, variant)]
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
        lib = os.path.join(root, 'lib', 'SDL3-static.lib')
        if not os.path.isfile(lib):
            raise PackageError('启用了 --with-sdl，但缺少 %s' % lib)
        if collector.add('lib/SDL3-static.lib', lib):
            count += 1
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
    return count


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
    运行时 dll 仅 Windows 收集（当前只有 WebView2Loader.dll）。
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
# 清单与打包
# --------------------------------------------------------------------------

CEF_EXAMPLES = ('cef.exe', 'cef_d.exe', 'CefBrowser.exe', 'CefBrowser_d.exe')


def _lib_stems(collector):
    """包内 lib/ 下所有库的文件名主干（Unix 去掉 lib 前缀）。"""
    stems = set()
    for rel, _abs in collector.items:
        if not rel.startswith('lib/'):
            continue
        stem = os.path.splitext(rel.rsplit('/', 1)[-1])[0]
        if stem.startswith('lib'):
            stem = stem[3:]
        stems.add(stem)
    return stems


def write_manifest(collector, pkg_root_name, version, sys_tag, arch_tag,
                   config, with_examples, with_sdl, with_cef, cef_version):
    """生成包内 MANIFEST.txt 与 SHA256SUMS，并固化 SDK 使用契约。"""
    is_windows = sys_tag == 'win'
    stems = _lib_stems(collector)
    lines = []

    def section(title):
        lines.append('')
        lines.append(title)
        lines.append('-' * 60)

    def fmt_libs(names):
        if is_windows:
            return ';'.join('%s.lib' % n for n in names)
        return ' '.join('-l%s' % n for n in names)

    lines.append('nim_duilib SDK')
    lines.append('=' * 60)
    lines.append('版本      : %s' % version)
    lines.append('包名      : %s' % pkg_root_name)
    lines.append('平台      : %s-%s' % (sys_tag, arch_tag))
    lines.append('配置      : %s' % config)
    lines.append('打包时间  : %s' % time.strftime('%Y-%m-%d %H:%M:%S'))
    lines.append('包含示例  : %s' % ('是' if with_examples else '否'))
    lines.append('文件总数  : %d' % len(collector.items))

    section('目录结构')
    lines.append('  include/duilib/**.h   对外头文件，保持仓库根相对路径')
    lines.append('                         用法：#include "duilib/duilib.h"')
    lines.append('  lib/                  静态库（.lib / .a），平面结构')
    lines.append('  resources/            主题、语言、字体资源')
    lines.append('  licenses/             第三方许可证')
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
    if 'SDL3-static' in stems:
        lines.append('   SDL3    : SDL3-static.lib（静态）')
    elif 'SDL3' in stems:
        lines.append('   SDL3    : -lSDL3（动态库，见运行期）')
    if with_cef:
        wrapper = CEF_WRAPPER_BY_VERSION[cef_version]
        lines.append('   CEF     : libcef.lib;%s.lib' % wrapper)

    section('运行期')
    lines.append('  resources/ 必须放在可执行文件同级目录（框架启动必需）。')
    if is_windows:
        if 'WebView2Loader.dll' in stems:
            lines.append('  动态 WebView2 loader 需要 WebView2Loader.dll 与可执行文件')
            lines.append('  同级；可从 bin/ 或 third_party 同名文件复制。')
        if 'SDL3-static' in stems:
            lines.append('  SDL3 为静态库，无需额外 DLL；下游需定义 DUILIB_SDL=1，')
            lines.append('  并补链系统库：Version.lib;Winmm.lib;Imm32.lib;Setupapi.lib')
        if with_cef:
            lines.append('  CEF 运行时不在本包内，需从 libcef 发布包放入可执行文件同级：')
            lines.append('  libcef.dll、icudtl.dat、*.pak、locales/ 等。')
    else:
        lines.append('  Skia 为静态库；SDL3 为动态库，需让 libSDL3.so* 可被找到：')
        lines.append('  复制到可执行文件同级，或设置 LD_LIBRARY_PATH'
                     '（macOS：DYLD_LIBRARY_PATH）。')
        lines.append('  X11 / freetype / fontconfig 由系统提供，不随包。')

    section('配置一致性')
    if config == 'Debug':
        lines.append('  本包是 Debug 版本，仅用于 Debug 构建的下游工程（/MTd），')
        lines.append('  必须使用 Debug 运行库；与 Release 包不可混用。')
    else:
        lines.append('  本包是 Release 版本，仅用于 Release 构建的下游工程（/MT），')
        lines.append('  与 Debug 包不可混用。')

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
                  fmt, out_path):
    """把收集到的文件写成压缩包。staging_root 用于统一路径前缀。"""
    os.makedirs(os.path.dirname(out_path), exist_ok=True)

    if fmt == 'zip':
        with zipfile.ZipFile(out_path, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as zf:
            for rel, abs_path in collector.items:
                arc = '%s/%s' % (pkg_root_name, rel)
                zf.write(abs_path, arc)
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
                        help='Skia 静态库目录（默认探测 ../skia/out/llvm.<arch>.<config>）')
    parser.add_argument('--sdl-dir', default=None,
                        help='SDL3 根目录（默认 ../SDL3）')
    parser.add_argument('--with-sdl', action='store_true',
                        help='Windows 下额外打包 SDL3（Unix 始终打包，无需该参数）')
    parser.add_argument('--with-cef', action='store_true',
                        help='打包 CEF 开发包：头文件 + libcef.lib + wrapper'
                             '（体积巨大，默认不带）')
    parser.add_argument('--cef-version', choices=('latest', '109'), default='latest',
                        help='配合 --with-cef 选择 CEF 版本（默认 latest）')
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
        skia_dir = resolve_skia_lib_dir(args.skia_lib_dir, config, arch_tag)

        collector = Collector()
        n_headers = collect_headers(collector, args.with_cef, args.with_sdl, sys_tag == 'win')
        n_libs = collect_libs(collector, lib_dir, config,
                              args.with_cef, args.cef_version)
        n_skia = collect_skia_libs(collector, skia_dir)
        n_sdl = collect_sdl(collector, args.sdl_dir, config, args.with_sdl)
        n_wv2 = collect_webview2_loader(collector, arch_tag)
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
    collector.sort()

    print('版本      : %s' % version)
    print('平台      : %s-%s' % (sys_tag, arch_tag))
    print('配置      : %s' % config)
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
    print('JPEG      : %d' % n_jpeg)
    print('CEF       : %d' % n_cef)
    print('资源文件  : %d' % n_res)
    print('许可证    : %d' % n_lic)
    print('示例程序  : %d' % n_exe)
    print('合计      : %d 个文件，%s' % (len(collector.items),
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
        return 0

    pkg_root_name = 'nim_duilib-%s-%s-%s' % (version, sys_tag, arch_tag)
    if config == 'Debug':
        pkg_root_name += '-debug'
    manifest_text, sums_text = write_manifest(
        collector, pkg_root_name, version, sys_tag, arch_tag, config,
        args.with_examples, args.with_sdl, args.with_cef, args.cef_version)

    ext = {'zip': '.zip', 'tgz': '.tar.gz', 'tarbz2': '.tar.bz2'}[fmt]
    out_path = os.path.join(args.out_dir, pkg_root_name + ext)

    try:
        build_package(collector, REPO_ROOT, pkg_root_name, manifest_text,
                      sums_text, fmt, out_path)
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