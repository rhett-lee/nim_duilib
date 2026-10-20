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

2. Windows 有两条构建路径，产物混在同一 lib/ 与 bin/ 下：
   - MSBuild ：build/*.sln + duilib/与 examples/ 下的 .vcxproj
   - CMake   ：build/msvc_build.bat
   本脚本只按文件名白名单收集，不关心是哪条路径产出的。

3. 头文件必须保持 `duilib/xxx.h` 的仓库根相对路径，因为下游工程的
   include 写法是 `#include "duilib/duilib.h"`（见 examples/basic/MainForm.h:5）。
   若改成 include/xxx.h，下游全部编译失败。

4. 5 个对外头文件直接 include 了 third_party 下的头文件，必须一并转发，
   否则下游编译失败：
   - Core/ZipStreamIO.h            -> zlib/contrib/minizip/ioapi.h
   - Image/APngDecoder.h           -> zlib/zlib.h、libpng/*.h
   - Image/FrameSequence_gif.h     -> giflib/gif_lib.h
   - WebView2/*.h                  -> Microsoft.Web.WebView2/.../WebView2.h

5. CEF 运行时（libcef.dll、*.pak、icudtl.dat）**不在本仓库内**，由使用者
   自行放入。因此含 CEF 的示例程序打包后不能直接运行，脚本会在清单中标注。

用法：
    python build/pack.py                      # SDK 包（不含示例）
    python build/pack.py --with-examples      # SDK + 示例程序
    python build/pack.py --list               # 只打印将收集的文件，不打包
    python build/pack.py --format tgz         # 强制 tgz（默认 Windows 用 zip）
    python build/pack.py --lib-dir lib/x64    # 指定库目录
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
RESOURCES_DIR = os.path.join(REPO_ROOT, 'bin', 'resources')
LICENSES_DIR = os.path.join(REPO_ROOT, 'licenses')
BIN_DIR = os.path.join(REPO_ROOT, 'bin')

# --------------------------------------------------------------------------
# 收集规则
# --------------------------------------------------------------------------

# 对外头文件需要一并转发的 third_party 子树（见文件头约束 4）。
# 每项都是「相对 third_party 的路径分量元组」—— 注意不要写成单个字符串，
# os.path.join('giflib') 会返回 str，迭代它得到的是单个字符。
THIRD_PARTY_INCLUDE_DIRS = (
    ('giflib',),
    ('Microsoft.Web.WebView2', 'build', 'native', 'include'),
)

# zlib / libpng 的头文件被多个对外头文件直接 include，且彼此有传递依赖
# （pngpriv.h -> pnglibconf.h / png.h / pngstruct.h / pnginfo.h），
# 按文件白名单转发不现实，直接整目录转发头文件。
THIRD_PARTY_INCLUDE_TREE_ROOTS = (
    'zlib',
    'libpng',
)

# 静态库白名单：基名 -> 说明。缺失时告警但不中断（CEF 是可选的）。
#
# 两套构建路径的命名不同，都列出：
#   CMake   ：zlib.lib / libpng.lib / libwebp.lib / cximage.lib（Debug 加 _d 后缀）
#   MSBuild ：duilib-zlib.lib / duilib-png.lib / ...（Debug 放在 lib/Debug/ 子目录）
LIB_BASENAMES = (
    'duilib',
    'duilib-cximage',
    'duilib-png',
    'duilib-webp',
    'duilib-zlib',
    'cximage',
    'libpng',
    'libwebp',
    'zlib',
)

# CEF wrapper 库：仅 --with-cef-libs 时收集，默认不带（体积巨大）。
CEF_LIB_BASENAMES = (
    'libcef_dll_wrapper',
    'libcef_dll_wrapper_109',
)

# 明确排除的文件扩展名：调试符号、中间产物、导入库。
EXCLUDE_LIB_SUFFIXES = (
    '.pdb', '.ilk', '.exp', '.idb', '.obj', '.o', '.a', '.so', '.dylib',
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


def collect_headers(collector):
    """收集对外头文件 + 必需的 third_party 头文件。

    路径前缀必须是 `include/duilib/`，不能是 `include/`——
    下游的写法是 `#include "duilib/duilib.h"`（见 examples/basic/MainForm.h:5），
    所以包内必须保留仓库根的相对路径层级。少了这一层下游直接编译失败。
    """
    count = 0
    for abs_path, rel in iter_files(DUIBIB_SRC_DIR):
        if not rel.endswith(('.h', '.hpp', '.inl')):
            continue
        if rel.startswith('third_party/'):
            continue
        # RenderSkia 是渲染后端实现细节，duilib.h 不引用它，不对外暴露。
        if rel.startswith('RenderSkia/'):
            continue
        if collector.add('include/duilib/' + rel, abs_path):
            count += 1

    # third_party 整树转发（zlib / libpng）
    for sub in THIRD_PARTY_INCLUDE_TREE_ROOTS:
        root = os.path.join(DUIBIB_SRC_DIR, 'third_party', sub)
        if not os.path.isdir(root):
            collector.warn('third_party/%s 不存在，头文件可能不完整' % sub)
            continue
        for abs_path, rel in iter_files(root):
            if not rel.endswith(('.h', '.hpp', '.inl')):
                continue
            if collector.add('include/duilib/third_party/%s/%s' % (sub, rel), abs_path):
                count += 1

    # third_party 指定子目录转发（giflib / WebView2）
    for parts in THIRD_PARTY_INCLUDE_DIRS:
        root = os.path.join(DUIBIB_SRC_DIR, 'third_party', *parts)
        if not os.path.isdir(root):
            collector.warn('third_party/%s 不存在，头文件可能不完整' % os.path.join(*parts))
            continue
        prefix = 'include/duilib/third_party/' + '/'.join(parts)
        for abs_path, rel in iter_files(root):
            if not rel.endswith(('.h', '.hpp', '.inl')):
                continue
            if collector.add('%s/%s' % (prefix, rel), abs_path):
                count += 1

    return count


def _is_debug_dir(lib_dir):
    """判断目录本身是否代表 Debug 构建。

    MSBuild 路径的 Debug 库**没有 _d 后缀**，只靠放在 lib/Debug/ 区分
    （见 msvc/PropertySheets/DuilibSettings.props：ImportLibrary 指向
    lib/$(Platform)/$(ProjectName).lib，Debug/Release 由目录区分）。
    只看文件名后缀会漏掉这一半，把 232MB 的 Debug 库当成 Release 打进去。
    """
    return os.path.basename(os.path.normpath(lib_dir)).lower() == 'debug'


def _scan_lib_dir(lib_dir):
    """扫描一个目录，返回 (release_map, debug_only_map)，均为 {基名: 文件名}。

    Debug 判定有两个来源，缺一不可：
      1. 文件名带 _d 后缀（CMake 路径）
      2. 目录名是 Debug（MSBuild 路径）
    同基名同时存在 Release 与 Debug 时 Release 胜出；只有 Debug 版的
    单独返回，交给调用方决定 —— **Debug 库绝不能进 Release SDK**
    （下游会链到调试 CRT / /MTd），见 collect_libs。
    """
    dir_is_debug = _is_debug_dir(lib_dir)
    release = {}
    debug_only = {}
    for name in os.listdir(lib_dir):
        full = os.path.join(lib_dir, name)
        if not os.path.isfile(full):
            continue
        if name.lower().endswith(EXCLUDE_LIB_SUFFIXES):
            continue
        stem, ext = os.path.splitext(name)
        if ext.lower() not in ('.lib', '.a'):
            continue
        if dir_is_debug or stem.endswith('_d'):
            debug_only.setdefault(stem[:-2] if stem.endswith('_d') else stem, name)
        else:
            release.setdefault(stem, name)
    return release, debug_only


def collect_libs(collector, lib_dirs, with_cef_libs, allow_debug_libs=False):
    """按白名单收集静态库。

    lib_dirs 是按优先级排列的候选目录列表。**合并**所有存在的目录 ——
    两条构建路径的命名不同（CMake 产 lib/x64/zlib.lib，MSBuild 产
    lib/Debug/duilib-zlib.lib），只取第一个目录会漏掉另一半。
    同名库由高优先级目录胜出。

    只有 Debug 版的库默认排除：Debug 库带调试 CRT 符号与 /MTd 运行时，
    混进 Release SDK 会让下游链接失败或体积暴涨。需要时用
    --allow-debug-libs 显式打开。
    """
    existing = [d for d in lib_dirs if d and os.path.isdir(d)]
    if not existing:
        collector.warn('未找到任何库目录（尝试过：%s），请先完成编译'
                       % ', '.join('lib/x64', 'lib/Debug', 'lib'))
        return 0

    # 低优先级先合入，高优先级后合入覆盖 —— dict.update 语义
    release_map = {}
    debug_map = {}
    for lib_dir in reversed(existing):
        rel_map, dbg_map = _scan_lib_dir(lib_dir)
        release_map.update(rel_map)
        debug_map.update(dbg_map)

    available = dict(debug_map)
    available.update(release_map)

    def locate(name):
        return next(d for d in existing if os.path.isfile(os.path.join(d, name)))

    wanted = list(LIB_BASENAMES)
    if with_cef_libs:
        wanted += list(CEF_LIB_BASENAMES)

    count = 0
    missing = []
    skipped_debug = []
    for base in wanted:
        name = available.get(base)
        if name is None:
            if base.startswith('libcef'):
                continue  # CEF wrapper 缺失属正常，未开 --with-cef-libs
            missing.append(base)
            continue
        # 只有 Debug 版、且未显式允许 —— 跳过
        if base not in release_map and not allow_debug_libs:
            skipped_debug.append('%s（%s，%s）' % (
                name, base, human_size(os.path.getsize(os.path.join(locate(name), name)))))
            continue
        if collector.add('lib/' + name, os.path.join(locate(name), name)):
            count += 1

    if skipped_debug:
        collector.warn('已排除仅有 Debug 版的库（%d 个）：%s'
                       % (len(skipped_debug), '；'.join(skipped_debug)))
        if not allow_debug_libs:
            collector.warn('  -> Release SDK 不应包含 Debug 库。需要时加 --allow-debug-libs')

    if missing:
        collector.warn('缺少库：%s（已搜索：%s）'
                       % (', '.join(missing),
                          ', '.join(os.path.relpath(d, REPO_ROOT).replace(os.sep, '/')
                                    for d in existing)))
        if 'duilib' in missing:
            collector.warn('  -> 未找到 duilib 主库，本包不含可链接的 duilib，'
                           '仅可用于资源/头文件参考。请先完成 Release 编译。')

    # CEF wrapper 即使不在白名单里，只要存在就提示体积，避免用户以为漏了
    if not with_cef_libs:
        for base in CEF_LIB_BASENAMES:
            name = available.get(base)
            if name is not None:
                full = os.path.join(locate(name), name)
                collector.warn('存在 CEF 库 %s（%s），未打包；需要请加 --with-cef-libs'
                               % (name, human_size(os.path.getsize(full))))
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


def collect_licenses(collector):
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
    return count


def collect_examples(collector):
    """收集示例程序可执行文件与运行时 dll。"""
    if not os.path.isdir(BIN_DIR):
        collector.warn('bin 目录不存在：%s' % BIN_DIR)
        return 0
    count = 0
    exec_exts = ('.exe',)
    for name in sorted(os.listdir(BIN_DIR)):
        full = os.path.join(BIN_DIR, name)
        if not os.path.isfile(full):
            continue
        low = name.lower()
        if low.endswith('.dll'):
            if name in INCLUDE_RUNTIME_DLLS:
                if collector.add('bin/' + name, full):
                    count += 1
            continue
        if not low.endswith(exec_exts):
            continue
        if collector.add('bin/' + name, full):
            count += 1
    return count


# --------------------------------------------------------------------------
# 清单与打包
# --------------------------------------------------------------------------

CEF_EXAMPLES = ('cef.exe', 'cef_d.exe', 'CefBrowser.exe', 'CefBrowser_d.exe')


def write_manifest(collector, pkg_root_name, version, sys_tag, arch_tag, with_examples):
    """生成包内 MANIFEST.txt 与 SHA256SUMS。"""
    lines = []
    lines.append('nim_duilib SDK')
    lines.append('=' * 60)
    lines.append('版本      : %s' % version)
    lines.append('平台      : %s-%s' % (sys_tag, arch_tag))
    lines.append('打包时间  : %s' % time.strftime('%Y-%m-%d %H:%M:%S'))
    lines.append('包含示例  : %s' % ('是' if with_examples else '否'))
    lines.append('文件总数  : %d' % len(collector.items))
    lines.append('')
    lines.append('目录结构')
    lines.append('-' * 60)
    lines.append('  include/duilib/**.h   对外头文件，保持仓库根相对路径')
    lines.append('                         用法：#include "duilib/duilib.h"')
    lines.append('  lib/                  静态库（.lib / .a）')
    lines.append('  resources/            主题、语言、字体资源')
    lines.append('  licenses/             第三方许可证')
    lines.append('  VERSION               版本号')
    lines.append('')
    if with_examples:
        lines.append('示例程序')
        lines.append('-' * 60)
        lines.append('  bin/*.exe             示例可执行文件，需与 resources/ 同级')
        lines.append('')
        missing_cef = [e for e in CEF_EXAMPLES
                       if any(r.endswith('/' + e) for r, _ in collector.items)]
        if missing_cef:
            lines.append('  [注意] 以下示例依赖 CEF 运行时，本仓库不含该运行时，')
            lines.append('         直接运行会失败。需自行从 libcef 发布包放入：')
            for e in missing_cef:
                lines.append('           %s' % e)
            lines.append('')
    else:
        lines.append('本包不含示例程序资源。resources/themes/ 下只保留了 public/ 与')
        lines.append('global.xml；各示例的私有主题目录未包含。')
        lines.append('')
    lines.append('校验')
    lines.append('-' * 60)
    lines.append('  SHA256SUMS 可用 sha256sum -c SHA256SUMS 校验')
    lines.append('')
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

def default_lib_dirs():
    """返回按优先级排列的库目录候选列表。

    两条构建路径的产物位置不同：
      CMake   —— lib/x64（Windows 多配置生成器会再加一层 Debug/Release）
      MSBuild —— lib/Debug、lib/Release
    因此这里给出多个候选，由 collect_libs 取第一个存在的。
    非Windows 平台只有 lib/。
    """
    rels = ['lib/x64', 'lib/Debug', 'lib/Release', 'lib']
    if not platform.system().lower().startswith('win'):
        rels = ['lib', 'lib/Release', 'lib/Debug']
    return [os.path.join(REPO_ROOT, *rel.split('/')) for rel in rels]


def main(argv=None):
    parser = argparse.ArgumentParser(
        description='nim_duilib 发布打包脚本',
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument('--with-examples', action='store_true',
                        help='包含示例程序与全部示例私有资源（默认只打 SDK）')
    parser.add_argument('--with-cef-libs', action='store_true',
                        help='包含 CEF wrapper 静态库（体积巨大，默认不带）')
    parser.add_argument('--allow-debug-libs', action='store_true',
                        help='允许把仅有 Debug 版的库打进包（默认排除，'
                             '因Debug 库带调试 CRT，混进 Release SDK 会让下游链接异常）')
    parser.add_argument('--format', choices=('zip', 'tgz', 'tarbz2'), default=None,
                        help='压缩格式，默认 Windows 用 zip、其它用 tgz')
    parser.add_argument('--lib-dir', default=None,
                        help='静态库目录，默认按构建路径自动探测 lib/x64 等')
    parser.add_argument('--out-dir', default=DEFAULT_OUT_DIR, help='输出目录，默认 dist/')
    parser.add_argument('--list', action='store_true',
                        help='只打印将收集的文件清单，不实际打包')
    args = parser.parse_args(argv)

    version = read_version()
    if version is None:
        return 1

    sys_tag, arch_tag = detect_platform_tag()
    fmt = args.format
    if fmt is None:
        fmt = 'zip' if sys_tag == 'win' else 'tgz'

    lib_dirs = [args.lib_dir] if args.lib_dir else default_lib_dirs()
    lib_dirs = [d for d in lib_dirs if d]

    collector = Collector()
    n_headers = collect_headers(collector)
    n_libs = collect_libs(collector, lib_dirs, args.with_cef_libs,
                          args.allow_debug_libs)
    n_res = collect_resources(collector, args.with_examples)
    n_lic = collect_licenses(collector)
    n_exe = collect_examples(collector) if args.with_examples else 0
    collector.sort()

    lib_dir_label = ', '.join(os.path.relpath(d, REPO_ROOT).replace(os.sep, '/')
                              for d in lib_dirs if os.path.isdir(d)) or '(无)'

    print('版本      : %s' % version)
    print('平台      : %s-%s' % (sys_tag, arch_tag))
    print('库目录    : %s' % lib_dir_label)
    print('格式      : %s' % fmt)
    print('包含示例  : %s' % ('是' if args.with_examples else '否'))
    print('-' * 60)
    print('头文件    : %d' % n_headers)
    print('静态库    : %d' % n_libs)
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
    manifest_text, sums_text = write_manifest(
        collector, pkg_root_name, version, sys_tag, arch_tag, args.with_examples)

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