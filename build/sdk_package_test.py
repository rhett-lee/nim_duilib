#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""nim_duilib 发布 SDK 包测试编排器。

以“解包后的成品包”为唯一输入，覆盖三类用例：
  1. P*：包结构、校验和、元数据、接入文件、负向包；
  2. C01/C02/C11：真实 CMake 消费者工程完成配置、编译、链接、运行
     （C11 专测 macOS App Bundle 的 Frameworks/rpath/资源布局）；
  3. C03-C06：配置/架构/运行库/工具链不匹配时必须被 SDK 拒绝。

用法：
  python build/sdk_package_test.py --list-scenarios
  python build/sdk_package_test.py --package dist/nim_duilib-...zip
  python build/sdk_package_test.py --package dist/xxx.zip --no-build
  python build/sdk_package_test.py --package dist/xxx.zip --scenarios P01,C01 --keep

退出码：0 全部通过（允许 SKIP）、1 存在 FAIL、2 全部 SKIP。
"""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import os
import platform
import re
import shutil
import stat
import subprocess
import sys
import tarfile
import tempfile
import time
import traceback
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

BUILD_DIR = Path(__file__).resolve().parent
REPO_ROOT = BUILD_DIR.parent
SDK_TEST_DIR = BUILD_DIR / "sdk_test"
PACK_PY = BUILD_DIR / "pack.py"

if str(BUILD_DIR) not in sys.path:
    sys.path.insert(0, str(BUILD_DIR))

from i18n import make_messages, set_console_utf8  # noqa: E402 - 复用 build/i18n.py 的取词逻辑

# 控制台文案：英文在脚本里按 key 引用，中文在 cmake/i18n/messages.json。
# 语言顺序：--lang / DUILIB_LANG -> LC_ALL/LC_MESSAGES/LANG -> 系统区域 -> en。
MESSAGES = make_messages()


def T(key):
    """取控制台文案：简体中文系统显示中文，其余（含 zh_TW/zh_HK）显示英文。"""
    return MESSAGES.t(key)


def _requested_lang(argv):
    """预扫描 --lang：让 --help 与参数错误也按所选语言输出。"""
    for index, arg in enumerate(argv):
        if arg.startswith('--lang='):
            return arg.split('=', 1)[1]
        if arg == '--lang' and index + 1 < len(argv):
            return argv[index + 1]
    return 'auto'


DEFAULT_OUT_ROOT = Path(tempfile.gettempdir()) / "nim_duilib_sdk_test"

# 场景登记表：(编号, 名称, 说明, 是否需要编译)
SCENARIO_ORDER = [
    ("P01", "package.extract", "sdk.scn_p01", False),
    ("P02", "package.layout", "sdk.scn_p02", False),
    ("P03", "package.checksums", "sdk.scn_p03", False),
    ("P04", "package.metadata", "sdk.scn_p04", False),
    ("P05", "package.integration", "sdk.scn_p05", False),
    ("P06", "package.forbidden", "sdk.scn_p06", False),
    ("P07", "package.verify-cli", "sdk.scn_p07", False),
    ("P08", "package.docs", "sdk.scn_p08", False),
    ("C01", "consumer.static", "sdk.scn_c01", True),
    ("C02", "consumer.dll", "sdk.scn_c02", True),
    ("C03", "negative.config", "sdk.scn_c03", True),
    ("C04", "negative.arch", "sdk.scn_c04", True),
    ("C05", "negative.msvc-runtime", "sdk.scn_c05", True),
    ("C06", "negative.toolchain", "sdk.scn_c06", True),
    ("C07", "negative.incomplete", "sdk.scn_c07", False),
    ("C08", "negative.tampered", "sdk.scn_c08", False),
    ("C09", "package.tar-formats", "sdk.scn_c09", False),
    ("C10", "examples.smoke", "sdk.scn_c10", True),
    ("C11", "consumer.macos-bundle", "sdk.scn_c11", True),
]
SCENARIO_NAMES = {sid: name for sid, name, _desc, _build in SCENARIO_ORDER}
BUILD_SCENARIOS = {sid for sid, _name, _desc, needs_build in SCENARIO_ORDER if needs_build}

_PACKAGE_NAME_RE = re.compile(
    r"^nim_duilib-(?P<version>[0-9][0-9A-Za-z.]*)-(?P<system>win|linux|macos|freebsd)-"
    r"(?P<arch>x64|x86|arm64)(?P<flags>(?:-[A-Za-z0-9]+)*)$")
_CMAKE_SET_RE = re.compile(
    r'^\s*set\(\s*([A-Za-z0-9_]+)\s+(?:"([^"]*)"|([^)\s]*))\s*\)', re.MULTILINE)
_SHA_LINE_RE = re.compile(r"^([0-9a-fA-F]{64})\s+\*?(.+)$")


class ScenarioSkipped(Exception):
    """环境不满足，跳过而不是失败。"""


# ---------------------------------------------------------------------------
# 纯函数（供 test_sdk_package.py 覆盖）
# ---------------------------------------------------------------------------
def strip_archive_ext(name: str) -> str:
    """去掉归档扩展名，得到包名/包根目录名。"""
    lower = name.lower()
    for ext in (".tar.bz2", ".tar.gz", ".tar.xz", ".tgz", ".tbz2", ".zip", ".tar"):
        if lower.endswith(ext):
            return name[: -len(ext)]
    return name


def parse_package_name(name: str) -> dict:
    """拆分发布包名。

    规则见 build/pack.py:build_package_name：
      nim_duilib-<版本>-<系统>-<架构>[-mingw-gcc|-mingw-llvm][-sdl][-cef|-cef109]
                                       [-examples][-dll][-debug]
    无法识别时返回空 dict。
    """
    stem = strip_archive_ext(Path(str(name)).name)
    match = _PACKAGE_NAME_RE.match(stem)
    if not match:
        return {}
    system = match.group("system")
    flags = [item for item in match.group("flags").split("-") if item]
    toolchain = ""
    mingw_variant = ""
    if system == "win":
        if "mingw" in flags:
            toolchain = "mingw"
            mingw_variant = "llvm" if "llvm" in flags else "gcc"
        else:
            toolchain = "msvc"
    return {
        "stem": stem,
        "version": match.group("version"),
        "system": system,
        "arch": match.group("arch"),
        "flags": flags,
        "config": "Debug" if "debug" in flags else "Release",
        "toolchain": toolchain,
        "mingw_variant": mingw_variant,
        "is_dll": "dll" in flags,
        # Unix 包默认携带 SDL，包名不追加 -sdl。
        "has_sdl": ("sdl" in flags) or system != "win",
        "has_cef": ("cef" in flags) or ("cef109" in flags),
        "cef_version": "109" if "cef109" in flags else ("142" if "cef" in flags else ""),
        "has_examples": "examples" in flags,
    }


def parse_cmake_info(text: str) -> dict:
    """解析 nim_duilibPackageInfo.cmake 中的 set(KEY value)。"""
    values = {}
    for match in _CMAKE_SET_RE.finditer(text):
        value = match.group(2) if match.group(2) is not None else match.group(3)
        values[match.group(1)] = value.strip()
    return values


def parse_manifest_fields(text: str) -> dict:
    """解析 MANIFEST.txt 顶部的「键 : 值」字段。"""
    fields = {}
    for raw in text.splitlines():
        line = raw.strip()
        if not line or ":" not in line:
            continue
        if line.startswith(("-", "=")) or line.startswith("#"):
            continue
        key, value = line.split(":", 1)
        key = key.strip()
        # 目录结构/文档章节的正文行键名含空格，不是字段。
        if not key or any(ch.isspace() for ch in key):
            continue
        fields[key] = value.strip()
    return fields


def _package_lib_names(files: list[str]) -> set[str]:
    return {item.split("/")[-1] for item in files
            if item.startswith("lib/") and item.count("/") == 1}


def _package_lib_stems(lib_names: set[str]) -> set[str]:
    stems = set()
    for name in lib_names:
        if name.lower().endswith(".a"):
            stem = name[:-2]
            if stem.lower().endswith(".dll"):
                stem = stem[:-4]
            if stem.startswith("lib"):
                stem = stem[3:]
        else:
            stem = os.path.splitext(name)[0]
        stems.add(stem)
    return stems


CEF_FRAMEWORK_MARK = (
    ".app/Contents/Frameworks/Chromium Embedded Framework.framework/")


def has_cef_example_bundle(files: list[str]) -> bool:
    """包内是否有内嵌 CEF Framework 的 macOS 示例 Bundle。

    macOS 不随包 CEF 开发文件/运行时，包名里的 -cef 只表示这类示例随包，
    因此 P04 用它校验包名，而不是 HAS_CEF。
    """
    return any(item.startswith("bin/") and CEF_FRAMEWORK_MARK in item
               for item in files)


def infer_capabilities(files: list[str], toolchain: str = "") -> dict:
    """由包内实际文件推断能力开关，规则与 build/pack.py 保持一致。"""
    lib_names = _package_lib_names(files)
    stems = _package_lib_stems(lib_names)
    bin_top = {item.split("/", 1)[1] for item in files
               if item.startswith("bin/") and item.count("/") == 1}
    bin_dlls = {name for name in bin_top if name.lower().endswith(".dll")}
    is_dll = any(name.lower().startswith("duilib") for name in bin_dlls)

    sdl_static = ("SDL3-static.lib" in lib_names) or ("libSDL3.a" in lib_names)
    sdl_shared = (
        ("SDL3.lib" in lib_names)
        or ("libSDL3.dll.a" in lib_names)
        or any(name.lower().startswith("libsdl3")
               and (".so" in name.lower() or name.lower().endswith(".dylib"))
               for name in lib_names))
    sdl_kind = "static" if sdl_static else ("shared" if sdl_shared else "")

    is_msvc_like = toolchain == "msvc" or (
        not toolchain and any(name.lower().endswith(".lib") for name in lib_names))
    if is_msvc_like:
        has_webview2 = ("WebView2LoaderStatic.lib" in lib_names
                        or "WebView2Loader.dll.lib" in lib_names)
    else:
        has_webview2 = ("WebView2Loader.dll.lib" in lib_names
                        or "WebView2Loader.dll" in bin_dlls)
    # 静态包随 turbojpeg-static.lib；DLL 包只随运行期 bin/turbojpeg.dll，
    # lib/ 中只有导入库 duilib.lib，因此必须同时看 bin。
    has_jpeg = is_msvc_like and (
        any(name.lower().startswith("turbojpeg") for name in lib_names)
        or "turbojpeg.dll" in bin_dlls)

    cef_src = ""
    for prefix in ("bin/libcef_win_109", "bin/libcef_win", "bin/libcef_linux"):
        if any(item.startswith(prefix + "/") for item in files):
            cef_src = prefix.split("/")[-1]
            break
    wrapper = ""
    for stem in stems:
        if stem.startswith("libcef_dll_wrapper"):
            wrapper = stem
            break
        if stem.startswith("cef_dll_wrapper"):
            wrapper = "lib" + stem
            break
    cef_headers = ("include/duilib/duilib_cef.h" in files
                   or any(item.startswith("include/duilib/CEFControl/")
                          for item in files))
    if cef_src in ("libcef_win", "libcef_win_109"):
        has_cef = "libcef.lib" in lib_names and bool(wrapper)
    elif cef_src == "libcef_linux":
        has_cef = bool(wrapper) and "bin/libcef_linux/libcef.so" in files
    else:
        # 未收集到 bin/libcef_win[_109]/ 运行库目录时（Windows 允许缺运行时出包），
        # pack.py 仍以“头文件 + libcef.lib + wrapper”声明 CEF；macOS 不随包
        # CEF 开发文件，恒为 False，其 CEF 能力由 has_cef_examples 单独表示。
        has_cef = cef_headers and bool(wrapper) and "libcef.lib" in lib_names

    has_examples = any(
        (item.startswith("bin/") and item.count("/") == 1
         and (item.lower().endswith(".exe") or "." not in item.split("/")[-1]))
        or (item.startswith("bin/") and ".app/Contents/MacOS/" in item)
        for item in files)

    return {
        "lib_names": lib_names,
        "lib_stems": stems,
        "bin_dlls": sorted(bin_dlls),
        "is_dll": is_dll,
        "has_sdl": sdl_static or sdl_shared,
        "sdl_kind": sdl_kind,
        "has_webview2": has_webview2,
        "has_jpeg": has_jpeg,
        "has_cef": has_cef,
        "cef_headers": cef_headers,
        "has_cef_examples": has_cef_example_bundle(files),
        "cef_src": cef_src,
        "cef_wrapper": wrapper,
        "has_examples": has_examples,
        "has_resources": "bin/resources/themes/default/global.xml" in files,
    }


def select_archive_root(names: list[str]) -> str:
    """发布包必须只有一个顶层目录。"""
    roots = {name.split("/", 1)[0] for name in names if name and name.strip("/")}
    if not roots:
        raise ValueError(T('sdk.archive_empty'))
    if len(roots) != 1:
        raise ValueError(T('sdk.root_not_unique') % ", ".join(sorted(roots)[:5]))
    return roots.pop()


def scenario_applicability(sid: str, caps: dict, meta: dict, *,
                           no_build: bool = False, run_examples: bool = False,
                           build_block_reason: str = "") -> tuple[bool, str]:
    """判断构建类场景是否适用；包级场景始终适用。"""
    if sid not in BUILD_SCENARIOS:
        return True, ""
    if no_build:
        return False, T('sdk.no_build')
    if sid in ("C01", "C02"):
        if sid == "C01" and caps.get("is_dll"):
            return False, T('sdk.dll_covered')
        if sid == "C02" and not caps.get("is_dll"):
            return False, T('sdk.static_covered')
        if build_block_reason:
            return False, build_block_reason
        return True, ""
    if sid == "C10":
        if not run_examples:
            return False, T('sdk.no_run_examples')
        if not caps.get("has_examples"):
            return False, T('sdk.no_examples')
        if build_block_reason:
            return False, build_block_reason
        return True, ""
    if sid == "C11":
        if host_system() != "macos":
            return False, T('sdk.macos_only')
        if not caps.get("has_sdl") or caps.get("sdl_kind") != "shared":
            return False, T('sdk.no_shared_sdl')
        if build_block_reason:
            return False, build_block_reason
        return True, ""
    # C03-C06 只需“相反工具链可用”，不需要能正常编译本包；
    # 具体环境在场景内部判定，不满足时返回 SKIP。
    return True, ""


def host_system() -> str:
    if sys.platform == "win32":
        return "win"
    if sys.platform == "darwin":
        return "macos"
    if sys.platform.startswith("freebsd"):
        return "freebsd"
    return "linux"


def normalize_arch(value: str) -> str:
    value = (value or "").strip().lower()
    if value in ("amd64", "x86_64", "x64"):
        return "x64"
    if value in ("x86", "i386", "i686", "win32"):
        return "x86"
    if value in ("arm64", "aarch64"):
        return "arm64"
    return value


def host_arch() -> str:
    if sys.platform == "win32":
        value = (os.environ.get("PROCESSOR_ARCHITEW6432")
                 or os.environ.get("PROCESSOR_ARCHITECTURE", ""))
    else:
        value = platform.machine()
    arch = normalize_arch(value)
    return arch if arch in ("x86", "x64", "arm64") else "x64"


def flag_value(value) -> bool:
    return str(value).strip().lower() in ("1", "on", "true", "yes")


# ---------------------------------------------------------------------------
# 包加载与结构校验
# ---------------------------------------------------------------------------
@dataclass
class PackageInfo:
    archive: Path
    root: Path
    files: list[str]
    name_parts: dict
    info: dict
    manifest: dict
    caps: dict
    version_text: str
    # 归档顶层包目录名；解包后其内容已落到 root，仅 P01/P04 与重打包使用。
    # 单测可省略，此时按归档文件名推导。
    root_name: str = ""

    def meta(self, key: str, default: str = "") -> str:
        return self.info.get("NIM_DUILIB_PACKAGE_" + key.upper(), default)

    def archive_stem(self) -> str:
        return strip_archive_ext(self.archive.name)

    def root_dir_name(self) -> str:
        return self.root_name or self.archive_stem()


@dataclass
class Options:
    out_root: Path
    selected: set[str]
    no_build: bool
    run_examples: bool
    keep: bool
    jobs: int
    dry_run: bool
    package_paths: list[Path] = field(default_factory=list)
    tmp_dir: Path | None = None


def _scenario_env(base_env: dict, options: Options) -> dict:
    """给配置/编译/运行注入本场景的临时目录。

    编译器的临时文件默认写到系统临时目录：Linux 上 `/tmp` 常常是小容量 tmpfs
    （例如 1.9 GiB），写满后 gcc 会报 "error closing ...s: 设备上没有空间"，
    进而被误报成「C++ 编译器不可用」「配置失败原因不含 ...」。把 TMPDIR/TEMP/TMP
    指到 `--out-dir` 下的 `tmp/`（与解包目录同一块磁盘），既不依赖系统临时目录的
    剩余空间，也不把编译中间文件散落到系统目录。
    """
    env = dict(base_env)
    if options.tmp_dir is not None:
        for name in ("TMPDIR", "TEMP", "TMP"):
            env[name] = str(options.tmp_dir)
    return env


def _warn_disk_space(packages: list[Path], out_root: Path) -> None:
    """启动前估算磁盘需求：解包、编译与重打包都会占用 `--out-dir` 所在磁盘。"""
    try:
        free = shutil.disk_usage(out_root).free
        archive_bytes = sum(path.stat().st_size for path in packages)
    except OSError:
        return
    need = archive_bytes * 2 + 512 * 1024 * 1024
    if free < need:
        print(T('sdk.disk_space')
              % (out_root, free / 2 ** 30, need / 2 ** 30), flush=True)


def _read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def collect_files(root: Path) -> list[str]:
    """收集包内普通文件（不跟随符号链接），返回 POSIX 相对路径。"""
    result = []
    for dirpath, dirnames, filenames in os.walk(root, followlinks=False):
        dirnames[:] = [name for name in dirnames
                       if not os.path.islink(os.path.join(dirpath, name))]
        for name in filenames:
            full = os.path.join(dirpath, name)
            if os.path.islink(full):
                continue
            result.append(os.path.relpath(full, root).replace(os.sep, "/"))
    return sorted(result)


def _restore_entry_mode(path: Path, mode: int) -> None:
    """按归档条目权限位恢复解包文件的模式。

    `open(..., "wb")` 建出来的文件是 0644，Unix 上丢掉了可执行位：
    macOS 的 `bin/basic.app/Contents/MacOS/basic` 解包后无法执行，C10 会直接
    报 `PermissionError: [Errno 13]`。Windows 没有 POSIX 权限位，chmod 只影响
    只读位，因此跳过。
    """
    if os.name == "nt" or not mode:
        return
    try:
        os.chmod(path, stat.S_IMODE(mode))
    except OSError:
        pass


def extract_package(archive: Path, dest: Path) -> Path:
    """解包，并把归档顶层包目录的内容直接落到 dest。

    Windows 未启用长路径支持时，“解包目录/包名/包内深路径”很容易超过
    MAX_PATH（CEF 头文件最长的相对路径约 120 字符，包名本身约 70~80 字符），
    shutil.unpack_archive 会在创建深目录时直接抛 FileNotFoundError。这里去掉
    归档的顶层包目录，避免把包名重复叠加进解包路径。包名本身仍由
    select_archive_root 校验，P01/P04 使用 root_name 对照。
    """
    dest.mkdir(parents=True, exist_ok=True)
    root_name = select_archive_root(archive_member_names(archive))
    prefix = root_name + "/"
    if archive.name.lower().endswith(".zip"):
        with zipfile.ZipFile(archive) as zf:
            for member in zf.infolist():
                name = member.filename
                if member.is_dir() or not name.startswith(prefix):
                    continue
                rel = name[len(prefix):]
                if not rel:
                    continue
                target = dest / rel
                target.parent.mkdir(parents=True, exist_ok=True)
                with zf.open(member) as src, open(target, "wb") as dst:
                    shutil.copyfileobj(src, dst)
                _restore_entry_mode(target, member.external_attr >> 16)
    else:
        with tarfile.open(archive, "r:*") as tf:
            for member in tf.getmembers():
                name = member.name
                if not name.startswith(prefix) or \
                        not (member.isfile() or member.issym()):
                    continue
                rel = name[len(prefix):]
                target = dest / rel
                target.parent.mkdir(parents=True, exist_ok=True)
                if member.issym():
                    if target.exists() or target.is_symlink():
                        target.unlink()
                    os.symlink(member.linkname, target)
                elif member.isfile():
                    with tf.extractfile(member) as src, open(target, "wb") as dst:
                        shutil.copyfileobj(src, dst)
                    _restore_entry_mode(target, member.mode)
    return dest


def archive_member_names(archive: Path) -> list[str]:
    if archive.name.lower().endswith(".zip"):
        with zipfile.ZipFile(archive) as zf:
            return [info.filename for info in zf.infolist() if not info.is_dir()]
    with tarfile.open(archive, "r:*") as tf:
        return [member.name for member in tf.getmembers()
                if member.isfile() or member.issym()]


def load_package(archive: Path, extract_dir: Path) -> PackageInfo:
    """解包并读取包内元数据。"""
    names = archive_member_names(archive)
    root_name = select_archive_root(names)
    root = extract_package(archive, extract_dir)
    files = collect_files(root)

    cmake_path = root / "lib/cmake/nim_duilib/nim_duilibPackageInfo.cmake"
    info = parse_cmake_info(_read_text(cmake_path)) if cmake_path.is_file() else {}
    manifest_path = root / "MANIFEST.txt"
    manifest = parse_manifest_fields(_read_text(manifest_path)) if manifest_path.is_file() else {}
    version_path = root / "VERSION"
    version_text = _read_text(version_path).strip() if version_path.is_file() else ""

    toolchain = info.get("NIM_DUILIB_PACKAGE_TOOLCHAIN", "")
    return PackageInfo(
        archive=archive,
        root=root,
        root_name=root_name,
        files=files,
        name_parts=parse_package_name(archive.name),
        info=info,
        manifest=manifest,
        caps=infer_capabilities(files, toolchain),
        version_text=version_text,
    )


def check_layout(info: PackageInfo) -> str:
    required = [
        "include/duilib/duilib.h",
        "MANIFEST.txt",
        "SHA256SUMS",
        "VERSION",
        "LICENSE",
        "THIRD-PARTY-NOTICES.md",
        "THIRD-PARTY-NOTICES.en.md",
        "lib/cmake/nim_duilib/nim_duilibConfig.cmake",
        "lib/cmake/nim_duilib/nim_duilibHelpers.cmake",
        "lib/cmake/nim_duilib/nim_duilibPackageInfo.cmake",
        "lib/cmake/nim_duilib/nim_duilibConfigVersion.cmake",
    ]
    problems = [item for item in required if not (info.root / item).is_file()]
    if problems:
        raise AssertionError(T('sdk.required_missing') + ", ".join(problems))

    stems = info.caps["lib_stems"]
    if "duilib" not in stems:
        raise AssertionError(T('sdk.main_lib_missing'))
    if not info.caps["is_dll"]:
        for stem in ("duilib-cximage", "duilib-webp", "duilib-png", "duilib-zlib", "skia"):
            if stem not in stems:
                raise AssertionError(T('sdk.static_lib_missing') % stem)
    if not info.caps["has_resources"]:
        raise AssertionError(T('sdk.resource_missing'))
    return T('sdk.layout_ok') % (len(required), len(stems))


def check_checksums(info: PackageInfo) -> str:
    sums_path = info.root / "SHA256SUMS"
    sums = {}
    problems = []
    for line in _read_text(sums_path).splitlines():
        line = line.strip()
        if not line:
            continue
        match = _SHA_LINE_RE.match(line)
        if not match:
            problems.append(T('sdk.line_unparsable') % line[:60])
            continue
        sums[match.group(2)] = match.group(1).lower()
    if problems:
        raise AssertionError(T('sdk.sep').join(problems[:5]))

    virtual = {"MANIFEST.txt", "VERSION"}
    actual = {item for item in info.files
              if item not in virtual and item != "SHA256SUMS"}
    missing = sorted(actual - set(sums))
    extra = sorted(set(sums) - actual)
    if missing:
        raise AssertionError(T('sdk.not_listed') % ", ".join(missing[:5]))
    if extra:
        raise AssertionError(T('sdk.listed_missing') % ", ".join(extra[:5]))

    mismatches = []
    for rel, digest in sums.items():
        path = info.root / rel
        if not path.is_file():
            continue
        if hashlib.sha256(path.read_bytes()).hexdigest().lower() != digest:
            mismatches.append(rel)
    if mismatches:
        raise AssertionError(T('sdk.sha_mismatch') % ", ".join(mismatches[:5]))
    return T('sdk.checksums_ok') % len(sums)


def check_metadata(info: PackageInfo) -> str:
    parts = info.name_parts
    if not parts:
        raise AssertionError(T('sdk.name_unparsable') % info.archive.name)

    problems = []

    def expect(actual, expected, label):
        if str(actual).strip() != str(expected).strip():
            problems.append(T('sdk.expect') % (label, expected, actual))

    expect(parts["version"], info.meta("VERSION"), T('sdk.lbl_version_cmake'))
    expect(parts["version"], info.version_text, T('sdk.lbl_version_text'))
    expect(parts["version"], info.manifest.get("版本", ""), T('sdk.lbl_version_manifest'))
    expect(parts["system"], info.meta("SYSTEM"), T('sdk.lbl_platform_cmake'))
    expect("%s-%s" % (parts["system"], parts["arch"]),
           info.manifest.get("平台", ""), T('sdk.lbl_platform_manifest'))
    expect(parts["arch"], info.meta("ARCH"), T('sdk.lbl_arch_cmake'))
    expect(parts["config"], info.meta("CONFIG"), T('sdk.lbl_config_cmake'))
    expect(parts["config"], info.manifest.get("配置", ""), T('sdk.lbl_config_manifest'))

    if parts["system"] == "win":
        expect(parts["toolchain"], info.meta("TOOLCHAIN"), T('sdk.lbl_toolchain_cmake'))
        toolchain_label = info.manifest.get("工具链", "")
        expected_label = "MSVC" if parts["toolchain"] == "msvc" else "MinGW"
        if toolchain_label and not toolchain_label.startswith(expected_label):
            problems.append(T('sdk.toolchain_label_mismatch')
                            % (toolchain_label, parts["toolchain"]))

    caps = info.caps
    checks = [
        ("IS_DLL", parts["is_dll"], caps["is_dll"], "DLL"),
        ("HAS_SDL", parts["has_sdl"], caps["has_sdl"], "SDL"),
        ("HAS_CEF", parts["has_cef"], caps["has_cef"], "CEF"),
        ("HAS_WEBVIEW2", flag_value(info.meta("HAS_WEBVIEW2")),
         caps["has_webview2"], "WebView2"),
        ("HAS_JPEG", flag_value(info.meta("HAS_JPEG")), caps["has_jpeg"], "JPEG"),
    ]
    if parts["system"] == "macos":
        # macOS 不随包 CEF 开发文件与运行时，HAS_CEF 必须为 0；包名的 -cef
        # 只表示 CEF 示例 App Bundle（内嵌 Framework）确实随包。
        if flag_value(info.meta("HAS_CEF")):
            problems.append(T('sdk.macos_has_cef'))
        expect(parts["has_cef"], caps["has_cef_examples"],
               T('sdk.lbl_cef_bundle'))
        checks = [item for item in checks if item[0] != "HAS_CEF"]
    for key, name_value, file_value, label in checks:
        meta_value = flag_value(info.meta(key))
        if name_value != file_value or meta_value != file_value:
            problems.append(T('sdk.cap_mismatch')
                            % (label, name_value, meta_value, file_value))

    if parts["system"] == "win":
        expect(parts["has_examples"], info.manifest.get("包含示例") == "是",
               T('sdk.lbl_examples_manifest'))
        if info.meta("TOOLCHAIN") == "mingw":
            expect(parts["mingw_variant"], info.meta("MINGW_VARIANT"),
                   T('sdk.lbl_mingw_variant'))
    expect(parts["has_examples"], caps["has_examples"], T('sdk.lbl_examples_files'))

    if info.meta("SYSTEM") == "win" and info.meta("TOOLCHAIN") == "msvc":
        runtime = info.meta("MSVC_RUNTIME").upper()
        if runtime not in ("MT", "MD"):
            problems.append(T('sdk.bad_msvc_runtime') % runtime)
        if caps["is_dll"] and runtime != "MD":
            problems.append(T('sdk.dll_needs_md') % runtime)
    else:
        if info.meta("MSVC_RUNTIME"):
            problems.append(T('sdk.runtime_not_allowed'))

    expect(info.manifest.get("包名", ""), info.root_dir_name(), T('sdk.lbl_name_root'))

    if problems:
        raise AssertionError(T('sdk.sep').join(problems[:8]))
    return T('sdk.metadata_ok')


def check_integration(info: PackageInfo) -> str:
    cmake_dir = info.root / "lib/cmake/nim_duilib"
    problems = []
    for path in cmake_dir.glob("*.cmake"):
        text = _read_text(path)
        leftover = re.findall(r"@[A-Z0-9_]+@", text)
        if leftover:
            problems.append(T('sdk.placeholder')
                            % (path.name, ", ".join(sorted(set(leftover)))))
    version_file = cmake_dir / "nim_duilibConfigVersion.cmake"
    if not version_file.is_file() or "PACKAGE_VERSION" not in _read_text(version_file):
        problems.append(T('sdk.config_version_missing'))

    system = info.meta("SYSTEM")
    if system == "win":
        for arch in ("x64", "x86", "arm64"):
            manifest = info.root / ("msvc/manifest/duilib.%s.manifest" % arch)
            if not manifest.is_file():
                problems.append(T('sdk.manifest_file_missing') % arch)
        if info.meta("TOOLCHAIN") == "msvc":
            for rel in ("msvc/nim_duilib.props", "msvc/nim_duilib.targets",
                        "msvc/nim_duilib.sdk.props"):
                if not (info.root / rel).is_file():
                    problems.append(T('sdk.missing') % rel)
            props = info.root / "msvc/nim_duilib.sdk.props"
            if props.is_file():
                text = _read_text(props)
                for key in ("NimDuilibPackageVersion", "NimDuilibPackageArch",
                            "NimDuilibPackageConfig", "NimDuilibPackageIsDll"):
                    if key not in text:
                        problems.append(T('sdk.props_missing_key') % key)
        # 所有 Windows 包的 manifest 必须是合法 XML（带 BOM 也能解析）。
        import xml.etree.ElementTree as ET
        for path in (info.root / "msvc/manifest").glob("*.manifest"):
            try:
                ET.parse(str(path))
            except ET.ParseError as exc:
                problems.append(T('sdk.xml_invalid') % (path.name, exc))

    if problems:
        raise AssertionError(T('sdk.sep').join(problems[:8]))
    return T('sdk.integration_ok')


def check_forbidden(info: PackageInfo) -> str:
    bad_ext = (".pdb", ".ilk", ".exp", ".obj", ".o")
    problems = []
    for item in info.files:
        lower = item.lower()
        if lower.endswith(bad_ext):
            problems.append(T('sdk.forbidden_debug') % item)
        if lower.endswith("cef_sandbox.lib"):
            problems.append(T('sdk.forbidden_sandbox') % item)
        if lower.endswith(".cpp") and item.startswith("include/"):
            problems.append(T('sdk.forbidden_cpp') % item)
        if item == "bin/resources.zip":
            problems.append(T('sdk.forbidden_zip'))
    if not info.caps["has_cef"]:
        for prefix in ("include/duilib/duilib_cef.h", "include/duilib/CEFControl/"):
            if any(item == prefix or item.startswith(prefix) for item in info.files):
                message = T('sdk.undeclared_cef') % prefix
                if info.name_parts.get("system") == "macos":
                    message += (T('sdk.macos_cef_hint'))
                problems.append(message)
                break
    if not info.caps["has_sdl"]:
        if any(item.startswith("include/SDL3/") for item in info.files):
            problems.append(T('sdk.undeclared_sdl'))
    if problems:
        raise AssertionError(T('sdk.sep').join(problems[:8]))
    return T('sdk.forbidden_ok')


def run_verify_cli(archive: Path) -> subprocess.CompletedProcess:
    # Same language as this process: verify messages are matched verbatim.
    return subprocess.run(
        [sys.executable, str(PACK_PY), "--verify", str(archive),
         "--lang", MESSAGES.language],
        capture_output=True, text=True, encoding="utf-8", errors="replace")


def check_verify_cli(info: PackageInfo) -> str:
    proc = run_verify_cli(info.archive)
    if proc.returncode != 0:
        tail = (proc.stdout or "")[-1200:]
        raise AssertionError(T('sdk.verify_failed') % (proc.returncode, tail))
    return T('sdk.verify_ok')


def check_docs(info: PackageInfo) -> str:
    docs = [item for item in info.files if item.startswith("docs/") and item.endswith(".md")]
    licenses = [item for item in info.files if item.startswith("LICENSES/")]
    problems = []
    if not docs:
        problems.append(T('sdk.no_docs'))
    if not licenses:
        problems.append(T('sdk.licenses_empty'))
    for notices_name in ("THIRD-PARTY-NOTICES.md", "THIRD-PARTY-NOTICES.en.md"):
        if not (info.root / notices_name).is_file():
            problems.append(T('sdk.notices_missing') % notices_name)
    if not info.caps["has_resources"]:
        problems.append(T('sdk.global_xml_missing'))
    if problems:
        raise AssertionError(T('sdk.sep').join(problems))
    return T('sdk.docs_ok') % (len(docs), len(licenses))


def _tar_open(dest: Path, mode: str):
    """打开用于重打包测试的归档。

    这里生成的归档只用于校验逻辑（格式、缺文件、篡改），压缩率不影响结论；
    归档里只有代表性小文件（见 _make_sample_package），级别 1 已足够。
    """
    kwargs = {}
    if mode.startswith("w:"):
        kwargs["compresslevel"] = 1
    dest.parent.mkdir(parents=True, exist_ok=True)
    return tarfile.open(dest, mode, **kwargs)


def _sample_symlinks(info: PackageInfo, limit: int = 3):
    """返回包内前若干个符号链接（相对路径、链接目标），用于格式重打包用例。"""
    found = []
    for dirpath, dirnames, filenames in os.walk(info.root, followlinks=False):
        for name in dirnames + filenames:
            full = os.path.join(dirpath, name)
            if not os.path.islink(full):
                continue
            rel = os.path.relpath(full, info.root).replace(os.sep, "/")
            found.append((rel, os.readlink(full)))
            if len(found) >= limit:
                return found
    return found


def _pick_smallest(info: PackageInfo, predicate) -> str:
    """在包内挑选满足条件且体积最小的文件，控制测试归档大小。"""
    candidates = [rel for rel in info.files if predicate(rel)]
    if not candidates:
        return ""
    return min(candidates, key=lambda rel: (info.root / rel).stat().st_size)


def _make_sample_package(info: PackageInfo, dest: Path, mode: str,
                         rels, *, override=None, hash_from=None) -> Path:
    """从真实包生成只含代表性文件的小归档，供 C07/C08/C09 使用。

    整包重新 gzip/bz2 在 Debug CEF 包（可达数 GB）上单次就要十几分钟，而这三个
    用例只验证 `--verify` 对缺文件/篡改的判定以及 tgz/tar.bz2 的写出；完整包
    的校验已由 P07 覆盖。这里保留真实的 MANIFEST/VERSION、调用方指定的文件、
    一个符号链接，并按实际内容重建 SHA256SUMS。

    override 用指定内容替换某个文件；hash_from 指定该文件参与 SHA256SUMS 的
    基准内容（C08 用它模拟“文件被篡改而校验和未更新”）。
    """
    import io

    override = override or {}
    hash_from = hash_from or {}
    wanted = list(dict.fromkeys(list(rels) + list(override)))
    payload = {}
    for rel in wanted:
        data = override.get(rel)
        payload[rel] = data if data is not None else (info.root / rel).read_bytes()
    for rel in ("MANIFEST.txt", "VERSION"):
        path = info.root / rel
        if path.is_file():
            payload[rel] = path.read_bytes()

    lines = []
    for rel, data in sorted(payload.items()):
        if rel in ("MANIFEST.txt", "VERSION"):
            continue
        basis = hash_from.get(rel, data)
        lines.append("%s  %s" % (hashlib.sha256(basis).hexdigest(), rel))
    payload["SHA256SUMS"] = ("\n".join(lines) + "\n").encode("utf-8")

    dest.parent.mkdir(parents=True, exist_ok=True)
    with _tar_open(dest, mode) as tf:
        for rel in sorted(payload):
            data = payload[rel]
            entry = tarfile.TarInfo(info.root_dir_name() + "/" + rel)
            entry.size = len(data)
            entry.mtime = 0
            entry.mode = 0o644
            tf.addfile(entry, io.BytesIO(data))
        for rel, target in _sample_symlinks(info):
            entry = tarfile.TarInfo(info.root_dir_name() + "/" + rel)
            entry.type = tarfile.SYMTYPE
            entry.linkname = target
            entry.mtime = 0
            tf.addfile(entry)
    return dest


def check_tar_formats(info: PackageInfo, work_dir: Path) -> str:
    header = _pick_smallest(info, lambda rel: rel.startswith("include/duilib/"))
    lib = _pick_smallest(info, _is_library_file)
    if not header or not lib:
        raise AssertionError(T('sdk.repack_need_files'))
    rels = [header, lib]
    runtime = "bin/resources/themes/default/global.xml"
    if (info.root / runtime).is_file():
        rels.append(runtime)

    results = []
    for suffix, mode in ((".tgz", "w:gz"), (".tar.bz2", "w:bz2")):
        dest = work_dir / ("repack" + suffix)
        _make_sample_package(info, dest, mode, rels)
        proc = run_verify_cli(dest)
        if proc.returncode != 0:
            raise AssertionError(T('sdk.repack_failed') % (suffix, (proc.stdout or "")[-800:]))
        results.append(suffix)
    return T('sdk.repack_ok') % " / ".join(results)


def _is_library_file(rel: str) -> bool:
    return rel.startswith("lib/") and rel.count("/") == 1 and \
        rel.lower().endswith((".lib", ".a"))


def check_incomplete(info: PackageInfo, work_dir: Path) -> str:
    header = _pick_smallest(info, lambda rel: rel.startswith("include/duilib/"))
    lib = _pick_smallest(info, _is_library_file)
    if not header or not lib:
        raise AssertionError(T('sdk.negative_need_files'))
    cases = [
        (T('sdk.label_missing_lib'), T('pkg.static_libs_missing'), [header]),
        (T('sdk.label_missing_header'), T('pkg.headers_missing'), [lib]),
    ]
    verified = []
    for index, (label, expected, rels) in enumerate(cases):
        archive = _make_sample_package(info, work_dir / ("bad-%d.tgz" % index),
                                       "w:gz", rels)
        proc = run_verify_cli(archive)
        if proc.returncode == 0:
            raise AssertionError(T('sdk.should_reject') % label)
        if expected not in (proc.stdout or ""):
            raise AssertionError(T('sdk.unexpected_reason')
                                 % (label, expected, (proc.stdout or "")[-500:]))
        verified.append(label)
    return T('sdk.rejected_ok') % " / ".join(verified)


def check_tampered(info: PackageInfo, work_dir: Path) -> str:
    rel = _pick_checksum_target(info)
    original = (info.root / rel).read_bytes()
    tampered = original + b"\n# sdk-package-test tampered\n"
    # 同时带上头文件与静态库，避免“缺文件”错误掩盖 SHA256 不匹配。
    rels = [rel]
    for extra in (_pick_smallest(info, lambda item: item.startswith("include/duilib/")),
                  _pick_smallest(info, _is_library_file)):
        if extra and extra not in rels:
            rels.append(extra)
    archive = _make_sample_package(
        info, work_dir / "tampered.tgz", "w:gz", rels,
        override={rel: tampered}, hash_from={rel: original})
    proc = run_verify_cli(archive)
    if proc.returncode == 0:
        raise AssertionError(T('sdk.tamper_should_fail') % rel)
    if "SHA256" not in (proc.stdout or ""):
        raise AssertionError(T('sdk.tamper_reason') % (proc.stdout or "")[-500:])
    return T('sdk.tamper_ok') % rel


def _pick_checksum_target(info: PackageInfo) -> str:
    preferred = "bin/resources/themes/default/global.xml"
    if (info.root / preferred).is_file():
        return preferred
    sums = set()
    for line in _read_text(info.root / "SHA256SUMS").splitlines():
        match = _SHA_LINE_RE.match(line.strip())
        if match:
            sums.add(match.group(2))
    candidates = [item for item in info.files if item in sums]
    if not candidates:
        raise AssertionError(T('sdk.no_tamper_target'))
    return min(candidates, key=lambda item: (info.root / item).stat().st_size)


# ---------------------------------------------------------------------------
# 编译与运行
# ---------------------------------------------------------------------------
_BUILDER_MODULE = None


def load_builder_module():
    """复用 build.py 的工具链检测（vcvars/Ninja/编译器）。"""
    global _BUILDER_MODULE
    if _BUILDER_MODULE is not None:
        return _BUILDER_MODULE
    if str(BUILD_DIR) not in sys.path:
        sys.path.insert(0, str(BUILD_DIR))
    spec = importlib.util.spec_from_file_location("nim_duilib_build_entry", BUILD_DIR / "build.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    _BUILDER_MODULE = module
    return module


def detect_toolchain(info: PackageInfo):
    try:
        builder = load_builder_module()
        return builder.detect_toolchain(info.meta("ARCH") or host_arch(), None), ""
    except Exception as exc:  # BuildError 或环境缺失
        return None, "%s: %s" % (type(exc).__name__, exc)


def build_block_reason(info: PackageInfo) -> str:
    """C01/C02/C10 的前置条件：平台与工具链必须能在本机使用。"""
    system = info.meta("SYSTEM")
    toolchain = info.meta("TOOLCHAIN")
    if system != host_system():
        return T('sdk.platform_differs') % (system, host_system())
    if toolchain == "msvc" and sys.platform != "win32":
        return T('sdk.msvc_windows_only')
    if toolchain == "mingw" and not os.environ.get("MSYSTEM"):
        return T('sdk.mingw_env_required')
    if toolchain == "mingw" and info.meta("MINGW_VARIANT") == "llvm":
        compilers = (os.environ.get("CC", "") + os.environ.get("CXX", "")).lower()
        if "clang" not in compilers:
            return T('sdk.mingw_llvm_env')
    tc, reason = detect_toolchain(info)
    if tc is None:
        return T('sdk.toolchain_unavailable') % reason
    if toolchain == "mingw" and tc.generator == "Visual Studio 17 2022":
        return T('sdk.no_mingw_compiler')
    return ""


def cmake_exe() -> str:
    found = shutil.which("cmake")
    if not found:
        raise ScenarioSkipped(T('sdk.no_cmake'))
    return found


def expectation_args(info: PackageInfo) -> list[str]:
    mapping = {
        "VERSION": info.meta("VERSION"),
        "SYSTEM": info.meta("SYSTEM"),
        "ARCH": info.meta("ARCH"),
        "CONFIG": info.meta("CONFIG"),
        "TOOLCHAIN": info.meta("TOOLCHAIN"),
        "IS_DLL": "1" if info.caps["is_dll"] else "0",
        "HAS_SDL": "1" if info.caps["has_sdl"] else "0",
        "HAS_CEF": "1" if info.caps["has_cef"] else "0",
        "HAS_WEBVIEW2": "1" if info.caps["has_webview2"] else "0",
        "HAS_JPEG": "1" if info.caps["has_jpeg"] else "0",
        "MSVC_RUNTIME": info.meta("MSVC_RUNTIME"),
    }
    return ["-DSDK_EXPECT_%s=%s" % (key, value) for key, value in mapping.items()]


def configure_command(info: PackageInfo, build_dir: Path, toolchain, *,
                     generator: str | None = None,
                     build_type: str | None = None,
                     arch: str | None = None,
                     extra: list[str] | None = None) -> list[str]:
    generator = generator or toolchain.generator
    command = [cmake_exe(), "-S", str(SDK_TEST_DIR), "-B", str(build_dir),
               "-G", generator,
               "-DNIM_DUILIB_SDK_DIR=%s" % info.root]
    if generator.startswith("Visual Studio"):
        target_arch = arch or info.meta("ARCH") or host_arch()
        platform_map = {"x86": "Win32", "x64": "x64", "arm64": "ARM64"}
        command += ["-A", platform_map.get(target_arch, "x64")]
    else:
        command += ["-DCMAKE_BUILD_TYPE=%s" % (build_type or info.meta("CONFIG") or "Release")]
    command += expectation_args(info)
    if extra:
        command += extra
    return command


def build_command(build_dir: Path, toolchain, jobs: int, config: str | None,
                  target: str | None = None) -> list[str]:
    command = [cmake_exe(), "--build", str(build_dir), "--parallel", str(jobs)]
    if toolchain.generator.startswith("Visual Studio") and config:
        command += ["--config", config]
    if target:
        command += ["--target", target]
    return command


def _log_command(log_path: Path, command: list[str], output: str = "", rc: int | None = None) -> None:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    lines = ["$ " + " ".join(command)]
    if rc is not None:
        lines.append("")
        lines.append("rc=%d" % rc)
    if output:
        lines.append("")
        lines.append(output)
    with log_path.open("a", encoding="utf-8", errors="replace") as handle:
        handle.write("\n".join(lines) + "\n\n")


def run_command(command: list[str], *, cwd: Path, env: dict, log_path: Path,
                dry_run: bool, timeout: int = 1800) -> tuple[int, str]:
    if dry_run:
        _log_command(log_path, command, "(dry-run)")
        return 0, ""
    try:
        proc = subprocess.run(command, cwd=str(cwd), env=env, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, timeout=timeout)
        output = proc.stdout.decode("utf-8", errors="replace")
        _log_command(log_path, command, output, proc.returncode)
        return proc.returncode, output
    except subprocess.TimeoutExpired:
        _log_command(log_path, command, "(timeout)", -1)
        return -1, "(timeout)"


def _tail(text: str, limit: int = 2000) -> str:
    text = (text or "").strip()
    return text[-limit:]


def find_consumer_executable(build_dir: Path) -> Path | None:
    for name in ("sdk_consumer.exe", "sdk_consumer"):
        for path in build_dir.rglob(name):
            if path.is_file():
                return path
    return None


def deploy_msvc_runtime(exe_dir: Path, info: PackageInfo) -> None:
    """为 /MD 的 MSVC 消费者部署 app-local CRT。

    测试机上的 x86 VC 运行库常常比 VS 2022 自带版本旧（例如 SysWOW64 里的
    MSVCP140 仍是 14.29），直接运行会在 MSVCP140.dll 里崩溃；而 app-local
    部署本身就是官方支持的发布方式。仅对 MD 包复制，/MT 包无需运行库。
    """
    if info.meta("TOOLCHAIN") != "msvc" or info.meta("MSVC_RUNTIME") != "MD":
        return
    crt = _msvc_crt_dir(info.meta("ARCH"))
    if crt is None:
        return
    for dll in crt.glob("*.dll"):
        shutil.copy2(dll, exe_dir / dll.name)


def _msvc_crt_dir(arch: str) -> Path | None:
    if os.name != "nt":
        return None
    candidates: list[tuple[str, Path]] = []
    for var in ("ProgramFiles", "ProgramFiles(x86)"):
        base = os.environ.get(var)
        if not base:
            continue
        for ver in Path(base).glob("Microsoft Visual Studio/*/*/VC/Redist/MSVC/*"):
            for crt in (ver / arch).glob("Microsoft.VC*.CRT"):
                if (crt / "msvcp140.dll").is_file():
                    candidates.append((ver.name, crt))
    if not candidates:
        return None
    candidates.sort(key=lambda item: item[0], reverse=True)
    return candidates[0][1]

def _can_run_arch(arch: str, host: str) -> bool:
    """本机能否直接运行该架构的程序（x64 主机可通过 WOW64 运行 x86）。"""
    if arch == host:
        return True
    return arch == "x86" and host == "x64"


def check_consumer(info: PackageInfo, work_dir: Path, log_dir: Path, options: Options) -> str:
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped(T('sdk.toolchain_unavailable') % reason)
    if info.meta("SYSTEM") != host_system():
        raise ScenarioSkipped(T('sdk.host_platform_differs'))

    build_dir = work_dir / "consumer"
    env = _scenario_env(toolchain.env, options)
    log = log_dir / "consumer.log"
    log.parent.mkdir(parents=True, exist_ok=True)
    if log.is_file():
        log.unlink()

    configure = configure_command(info, build_dir, toolchain)
    rc, output = run_command(configure, cwd=REPO_ROOT, env=env, log_path=log,
                             dry_run=options.dry_run)
    if options.dry_run:
        raise ScenarioSkipped(T('sdk.dry_run_configure'))
    if rc != 0:
        raise AssertionError(T('sdk.configure_failed') % (rc, _tail(output)))

    # 只构建可执行文件消费者目标；macOS 上同一工程还登记了 bundle 目标，
    # 由 C11 单独构建，避免每个场景都多链接一次静态库。
    build = build_command(build_dir, toolchain, options.jobs, info.meta("CONFIG"),
                          target="nim_duilib_sdk_consumer")
    rc, output = run_command(build, cwd=REPO_ROOT, env=env, log_path=log,
                             dry_run=False, timeout=3600)
    if rc != 0:
        raise AssertionError(T('sdk.build_failed') % (rc, _tail(output)))

    exe = find_consumer_executable(build_dir)
    if exe is None:
        raise AssertionError(T('sdk.consumer_not_found'))
    exe_dir = exe.parent
    if not (exe_dir / "resources/themes/default/global.xml").is_file():
        raise AssertionError(T('sdk.resources_not_copied') % exe_dir)
    if info.caps["is_dll"]:
        missing = [name for name in info.caps["bin_dlls"]
                   if not (exe_dir / name).is_file()]
        if missing:
            raise AssertionError(T('sdk.dlls_not_copied') % ", ".join(missing))

    # /MD 消费者的运行库可能与测试机系统目录版本不一致，按官方 app-local
    # 方式把 VS 自带的 CRT 复制到 exe 同级，避免误判为 SDK 问题。
    deploy_msvc_runtime(exe_dir, info)

    package_arch = info.meta("ARCH") or host_arch()
    if not _can_run_arch(package_arch, host_arch()):
        # 交叉编译产物无法在本机执行（如 x64 主机上构建 arm64 消费者）：
        # 配置与链接已验证，运行验证需对应架构的机器。
        raise ScenarioSkipped(
            T('sdk.cross_arch_skip') % (host_arch(), package_arch, package_arch))

    try:
        proc = subprocess.run([str(exe)], cwd=str(exe_dir), env=env,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              timeout=180)
    except subprocess.TimeoutExpired:
        raise AssertionError(T('sdk.run_timeout'))
    run_output = proc.stdout.decode("utf-8", errors="replace")
    _log_command(log, [str(exe)], run_output, proc.returncode)

    if proc.returncode != 0 or "SDK_TEST_RESULT=ok" not in run_output:
        raise AssertionError(T('sdk.run_failed')
                             % (proc.returncode, _tail(run_output)))

    _check_features(info, run_output)
    return T('sdk.consumer_ok')


def _check_features(info: PackageInfo, output: str) -> None:
    """核对程序打印的 SDK_TEST_FEATURES 与包能力。"""
    features = _parse_features(output)
    expected = {
        "sdl": "1" if info.caps["has_sdl"] else "0",
        "cef": "1" if info.caps["has_cef"] else "0",
        "webview2": "1" if info.caps["has_webview2"] else "0",
        "jpeg": "1" if info.caps["has_jpeg"] else "0",
        "dll": "1" if info.caps["is_dll"] else "0",
    }
    wrong = [T('sdk.feature_value') % (key, features.get(key, "?"), value)
             for key, value in expected.items() if features.get(key) != value]
    if wrong:
        raise AssertionError(T('sdk.features_mismatch') % ", ".join(wrong))


def _parse_features(output: str) -> dict:
    for line in output.splitlines():
        if line.startswith("SDK_TEST_FEATURES"):
            result = {}
            for item in line.split()[1:]:
                if "=" in item:
                    key, value = item.split("=", 1)
                    result[key] = value
            return result
    return {}


def _expect_configure_failure(info: PackageInfo, work_dir: Path, log_dir: Path,
                              options: Options, *, log_name: str,
                              command: list[str], env: dict,
                              expected_text: str) -> str:
    log = log_dir / log_name
    log.parent.mkdir(parents=True, exist_ok=True)
    if log.is_file():
        log.unlink()
    rc, output = run_command(command, cwd=REPO_ROOT, env=env, log_path=log,
                             dry_run=options.dry_run, timeout=600)
    if options.dry_run:
        raise ScenarioSkipped(T('sdk.dry_run_configure_only'))
    if rc == 0:
        raise AssertionError(T('sdk.configure_should_fail'))
    if expected_text not in output:
        raise AssertionError(T('sdk.configure_reason')
                             % (expected_text, _tail(output, 1500)))
    return T('sdk.rejected_expected') % expected_text


def check_negative_config(info: PackageInfo, work_dir: Path, log_dir: Path,
                          options: Options) -> str:
    blocked = build_block_reason(info)
    if blocked:
        raise ScenarioSkipped(blocked)
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped(T('sdk.toolchain_unavailable') % reason)
    if toolchain.generator.startswith("Visual Studio"):
        raise ScenarioSkipped(T('sdk.multiconfig_hint'))
    config = info.meta("CONFIG") or "Release"
    opposite = "Release" if config == "Debug" else "Debug"
    command = configure_command(info, work_dir / "neg-config", toolchain,
                                build_type=opposite)
    return _expect_configure_failure(
        info, work_dir, log_dir, options, log_name="neg-config.log",
        command=command, env=_scenario_env(toolchain.env, options), expected_text="配置为")


def check_negative_arch(info: PackageInfo, work_dir: Path, log_dir: Path,
                        options: Options) -> str:
    blocked = build_block_reason(info)
    if blocked:
        raise ScenarioSkipped(blocked)
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped(T('sdk.toolchain_unavailable') % reason)
    if info.meta("SYSTEM") != "win":
        if info.meta("ARCH") == host_arch():
            raise ScenarioSkipped(T('sdk.arch_same_host'))
        command = configure_command(info, work_dir / "neg-arch", toolchain)
        return _expect_configure_failure(
            info, work_dir, log_dir, options, log_name="neg-arch.log",
            command=command, env=_scenario_env(toolchain.env, options), expected_text="架构为")

    package_arch = info.meta("ARCH")
    opposite = {"x64": "x86", "x86": "x64", "arm64": "x64"}.get(package_arch, "x86")
    if toolchain.vs_install is not None:
        command = configure_command(info, work_dir / "neg-arch", toolchain,
                                    generator="Visual Studio 17 2022",
                                    arch=opposite)
        return _expect_configure_failure(
            info, work_dir, log_dir, options, log_name="neg-arch.log",
            command=command, env=_scenario_env(toolchain.env, options), expected_text="架构为")
    if package_arch != host_arch():
        command = configure_command(info, work_dir / "neg-arch", toolchain)
        return _expect_configure_failure(
            info, work_dir, log_dir, options, log_name="neg-arch.log",
            command=command, env=_scenario_env(toolchain.env, options), expected_text="架构为")
    raise ScenarioSkipped(T('sdk.no_vs_arch_same'))


def check_negative_msvc_runtime(info: PackageInfo, work_dir: Path, log_dir: Path,
                                options: Options) -> str:
    if info.meta("SYSTEM") != "win" or info.meta("TOOLCHAIN") != "msvc":
        raise ScenarioSkipped(T('sdk.msvc_only'))
    runtime = info.meta("MSVC_RUNTIME").upper()
    if runtime not in ("MT", "MD"):
        raise ScenarioSkipped(T('sdk.no_runtime_declared'))
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped(T('sdk.toolchain_unavailable') % reason)
    opposite = ("MultiThreaded$<$<CONFIG:Debug>:Debug>"
                if runtime == "MD"
                else "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    command = configure_command(info, work_dir / "neg-runtime", toolchain,
                                extra=["-DCMAKE_MSVC_RUNTIME_LIBRARY=%s" % opposite])
    return _expect_configure_failure(
        info, work_dir, log_dir, options, log_name="neg-runtime.log",
        command=command, env=_scenario_env(toolchain.env, options), expected_text="运行库")


def check_negative_toolchain(info: PackageInfo, work_dir: Path, log_dir: Path,
                             options: Options) -> str:
    package_toolchain = info.meta("TOOLCHAIN")
    if package_toolchain not in ("msvc", "mingw"):
        raise ScenarioSkipped(T('sdk.non_windows_no_counterpart'))
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped(T('sdk.toolchain_unavailable') % reason)

    if package_toolchain == "msvc":
        gxx = shutil.which("g++")
        if not gxx:
            raise ScenarioSkipped(T('sdk.no_gxx'))
        generator = "Ninja" if toolchain.ninja else None
        if generator is None:
            raise ScenarioSkipped(T('sdk.no_ninja'))
        command = configure_command(info, work_dir / "neg-toolchain", toolchain,
                                    generator=generator,
                                    extra=["-DCMAKE_CXX_COMPILER=%s" % gxx])
    else:
        if toolchain.vs_install is None or not shutil.which("cl", path=toolchain.env.get("PATH")):
            raise ScenarioSkipped(T('sdk.no_msvc'))
        command = configure_command(info, work_dir / "neg-toolchain", toolchain)
    return _expect_configure_failure(
        info, work_dir, log_dir, options, log_name="neg-toolchain.log",
        command=command, env=_scenario_env(toolchain.env, options), expected_text="该 nim_duilib SDK 是")


def check_examples_smoke(info: PackageInfo, work_dir: Path, log_dir: Path,
                         options: Options) -> str:
    candidates = [
        info.root / "bin/basic.exe",
        info.root / "bin/basic",
        info.root / "bin/basic.app/Contents/MacOS/basic",
    ]
    exe = next((path for path in candidates if path.is_file()), None)
    if exe is None:
        raise ScenarioSkipped(T('sdk.no_basic'))
    proc = subprocess.Popen([str(exe)], cwd=str(exe.parent),
                            env=_scenario_env(os.environ, options),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        time.sleep(3)
        code = proc.poll()
        if code is not None:
            raise AssertionError(T('sdk.example_exited') % code)
    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=10)
            except subprocess.TimeoutExpired:
                proc.kill()
    return T('sdk.example_running')


# ---------------------------------------------------------------------------
# 场景调度
# ---------------------------------------------------------------------------
@dataclass
class Result:
    scenario_id: str
    name: str
    status: str
    detail: str
    log: str = ""


def find_macos_bundle(build_dir: Path) -> Path | None:
    """在构建目录中定位 sdk_bundle.app（兼容多配置生成器的 <Config>/ 子目录）。"""
    for path in sorted(build_dir.rglob("sdk_bundle.app")):
        if (path / "Contents/MacOS/sdk_bundle").is_file():
            return path
    return None


def _otool_path() -> str | None:
    found = shutil.which("otool")
    if found:
        return found
    fallback = Path("/usr/bin/otool")
    return str(fallback) if fallback.is_file() else None


def _run_otool(arguments: list[str], log_path: Path, env: dict) -> str:
    otool = _otool_path()
    if otool is None:
        raise ScenarioSkipped(T('sdk.no_otool'))
    rc, output = run_command([otool, *arguments], cwd=REPO_ROOT, env=env,
                             log_path=log_path, dry_run=False, timeout=120)
    if rc != 0:
        raise AssertionError(T('sdk.otool_failed')
                             % (" ".join(arguments), rc, _tail(output)))
    return output


def check_macos_bundle(info: PackageInfo, work_dir: Path, log_dir: Path,
                       options: Options) -> str:
    """C11：用 SDK 包生成 macOS App Bundle，验证 Frameworks/rpath/资源布局。"""
    if sys.platform != "darwin":
        raise ScenarioSkipped(T('sdk.macos_only'))
    if not info.caps.get("has_sdl") or info.caps.get("sdl_kind") != "shared":
        raise ScenarioSkipped(T('sdk.no_shared_sdl'))
    blocked = build_block_reason(info)
    if blocked:
        raise ScenarioSkipped(blocked)
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped(T('sdk.toolchain_unavailable') % reason)

    build_dir = work_dir / "consumer_bundle"
    env = _scenario_env(toolchain.env, options)
    log = log_dir / "bundle.log"
    log.parent.mkdir(parents=True, exist_ok=True)
    if log.is_file():
        log.unlink()

    configure = configure_command(info, build_dir, toolchain)
    rc, output = run_command(configure, cwd=REPO_ROOT, env=env, log_path=log,
                             dry_run=options.dry_run)
    if options.dry_run:
        raise ScenarioSkipped(T('sdk.dry_run_configure'))
    if rc != 0:
        raise AssertionError(T('sdk.configure_failed') % (rc, _tail(output)))

    build = build_command(build_dir, toolchain, options.jobs, info.meta("CONFIG"),
                          target="nim_duilib_sdk_bundle")
    rc, output = run_command(build, cwd=REPO_ROOT, env=env, log_path=log,
                             dry_run=False, timeout=3600)
    if rc != 0:
        raise AssertionError(T('sdk.build_failed') % (rc, _tail(output)))

    app = find_macos_bundle(build_dir)
    if app is None:
        raise AssertionError(T('sdk.bundle_not_found') % build_dir)

    exe = app / "Contents/MacOS/sdk_bundle"
    dylibs = sorted((app / "Contents/Frameworks").glob("libSDL3*.dylib"))
    if not dylibs:
        raise AssertionError(T('sdk.no_framework_dylib')
                             % (app / "Contents/Frameworks"))
    global_xml = app / "Contents/Resources/duilib/resources/themes/default/global.xml"
    if not global_xml.is_file():
        raise AssertionError(T('sdk.bundle_resources_missing')
                             % global_xml)

    # rpath 与 install_name：确认引用 @rpath，且不残留 SDK/构建机的绝对路径。
    linked = _run_otool(["-L", str(exe)], log, env)
    if "@rpath/libSDL3" not in linked:
        raise AssertionError(T('sdk.no_rpath_ref')
                             % _tail(linked))
    if str(info.root) in linked:
        raise AssertionError(T('sdk.absolute_sdk_ref')
                             % (info.root, _tail(linked)))
    load_commands = _run_otool(["-l", str(exe)], log, env)
    if "@executable_path/../Frameworks" not in load_commands:
        raise AssertionError(T('sdk.missing_lc_rpath')
                             % _tail(load_commands))
    install_name = _run_otool(["-D", str(dylibs[0])], log, env)
    if not any(line.strip().startswith("@rpath/")
               for line in install_name.splitlines()):
        raise AssertionError(T('sdk.sdl_install_name')
                             % _tail(install_name))

    # 把 .app 复制到构建树之外再运行，确保不依赖构建目录的绝对 rpath。
    relocated_root = work_dir / "bundle-relocated"
    if relocated_root.exists():
        shutil.rmtree(relocated_root)
    relocated_root.mkdir(parents=True, exist_ok=True)
    relocated_app = relocated_root / app.name
    shutil.copytree(app, relocated_app, symlinks=True)
    relocated_exe = relocated_app / "Contents/MacOS/sdk_bundle"

    # DYLD_PRINT_LIBRARIES 让 dyld 打印实际加载的路径：既能证明 Frameworks 生效，
    # 也能发现“仍在从 SDK 的绝对路径加载”的假通过。
    run_env = dict(env)
    run_env["DYLD_PRINT_LIBRARIES"] = "1"
    try:
        proc = subprocess.run([str(relocated_exe)], cwd=str(relocated_exe.parent),
                              env=run_env, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, timeout=180)
    except subprocess.TimeoutExpired:
        raise AssertionError(T('sdk.relocated_timeout'))
    run_output = proc.stdout.decode("utf-8", errors="replace")
    _log_command(log, [str(relocated_exe)], run_output, proc.returncode)
    if proc.returncode != 0 or "SDK_TEST_RESULT=ok" not in run_output:
        raise AssertionError(T('sdk.relocated_failed')
                             % (proc.returncode, _tail(run_output)))
    if "Contents/Frameworks/libSDL3" not in run_output:
        raise AssertionError(T('sdk.dyld_not_framework')
                             % _tail(run_output))
    if "/lib/libSDL3" in run_output:
        raise AssertionError(T('sdk.dyld_sdk_lib') % _tail(run_output))
    resource_line = next((line for line in run_output.splitlines()
                          if line.startswith("SDK_TEST_RESOURCE_ROOT=")), "")
    if "Contents/Resources/duilib/resources" not in resource_line:
        raise AssertionError(T('sdk.resource_root')
                             % (resource_line or T('sdk.resource_root_missing')))
    sdl_version = 0
    for line in run_output.splitlines():
        if line.startswith("SDK_TEST_SDL_VERSION="):
            try:
                sdl_version = int(line.split("=", 1)[1])
            except ValueError:
                sdl_version = 0
            break
    if sdl_version <= 0:
        raise AssertionError(T('sdk.no_sdl_version')
                             % _tail(run_output))
    _check_features(info, run_output)
    return (T('sdk.bundle_ok')
            % sdl_version)


def _scenario_runner(sid: str, info: PackageInfo, work_dir: Path,
                     log_dir: Path, options: Options):
    if sid == "P01":
        return T('sdk.extracted') % (len(info.files), info.root_dir_name())
    if sid == "P02":
        return check_layout(info)
    if sid == "P03":
        return check_checksums(info)
    if sid == "P04":
        return check_metadata(info)
    if sid == "P05":
        return check_integration(info)
    if sid == "P06":
        return check_forbidden(info)
    if sid == "P07":
        return check_verify_cli(info)
    if sid == "P08":
        return check_docs(info)
    if sid in ("C01", "C02"):
        return check_consumer(info, work_dir, log_dir, options)
    if sid == "C03":
        return check_negative_config(info, work_dir, log_dir, options)
    if sid == "C04":
        return check_negative_arch(info, work_dir, log_dir, options)
    if sid == "C05":
        return check_negative_msvc_runtime(info, work_dir, log_dir, options)
    if sid == "C06":
        return check_negative_toolchain(info, work_dir, log_dir, options)
    if sid == "C07":
        return check_incomplete(info, work_dir)
    if sid == "C08":
        return check_tampered(info, work_dir)
    if sid == "C09":
        return check_tar_formats(info, work_dir)
    if sid == "C10":
        return check_examples_smoke(info, work_dir, log_dir, options)
    if sid == "C11":
        return check_macos_bundle(info, work_dir, log_dir, options)
    raise AssertionError(T('sdk.unknown_scenario') % sid)


def run_package(info: PackageInfo, package_out: Path, options: Options) -> list[Result]:
    work_dir = package_out / "work"
    log_dir = package_out / "logs"
    work_dir.mkdir(parents=True, exist_ok=True)
    log_dir.mkdir(parents=True, exist_ok=True)
    # 编译器的临时文件放到本包目录下（见 _scenario_env）：不依赖系统临时目录余量。
    options.tmp_dir = work_dir / "tmp"
    options.tmp_dir.mkdir(parents=True, exist_ok=True)

    block_reason = build_block_reason(info)
    results = []

    def record(result: Result) -> None:
        """记录结果并立即输出进度：大包用例（重打包/缺文件）单场景可能要几分钟，
        无人值守运行时需要能及时看到当前进度，避免误判为卡死。"""
        results.append(result)
        print("     [%s] %s %s" % (result.status, result.scenario_id, result.name),
              flush=True)

    for sid in [item[0] for item in SCENARIO_ORDER if item[0] in options.selected]:
        name = SCENARIO_NAMES[sid]
        if options.dry_run and sid in BUILD_SCENARIOS:
            record(Result(sid, name, "SKIP", T('sdk.dry_run_skipped'), ""))
            continue
        print("  >> %s %s ..." % (sid, name), flush=True)
        applicable, reason = scenario_applicability(
            sid, info.caps, info.info, no_build=options.no_build,
            run_examples=options.run_examples, build_block_reason=block_reason)
        if not applicable:
            record(Result(sid, name, "SKIP", reason, ""))
            continue
        try:
            detail = _scenario_runner(sid, info, work_dir, log_dir, options)
            record(Result(sid, name, "PASS", detail, ""))
        except ScenarioSkipped as exc:
            record(Result(sid, name, "SKIP", str(exc), ""))
        except AssertionError as exc:
            record(Result(sid, name, "FAIL", str(exc), _existing_log(log_dir)))
        except Exception as exc:  # 未预期异常也计为失败，但不中断其它场景
            detail = "%s: %s\n%s" % (type(exc).__name__, exc, traceback.format_exc(limit=3))
            record(Result(sid, name, "FAIL", detail, _existing_log(log_dir)))
    return results


def _existing_log(log_dir: Path) -> str:
    logs = sorted(log_dir.glob("*.log"), key=lambda item: item.stat().st_mtime, reverse=True)
    return str(logs[0]) if logs else ""


def parse_selection(value: str) -> set[str]:
    if not value or value.strip().lower() == "all":
        return {item[0] for item in SCENARIO_ORDER}
    selected = set()
    for raw in value.split(","):
        item = raw.strip().upper()
        if not item:
            continue
        if item not in SCENARIO_NAMES:
            raise SystemExit(T('sdk.unknown_scenario_hint') % item)
        selected.add(item)
    if not selected:
        raise SystemExit(T('sdk.scenarios_empty'))
    return selected


def print_scenarios() -> None:
    # Descriptions are catalog keys so that --lang still applies.
    print("%-4s %-24s %-8s %s" % (T('sdk.hdr_id'), T('sdk.hdr_name'),
                                   T('sdk.hdr_kind'), T('sdk.hdr_desc')))
    print("-" * 78)
    for sid, name, desc, needs_build in SCENARIO_ORDER:
        print("%-4s %-24s %-8s %s" % (sid, name,
                                      T('sdk.kind_build') if needs_build
                                      else T('sdk.kind_package'), T(desc)))


def print_results(package: PackageInfo, results: list[Result]) -> None:
    print("\n== %s" % package.archive.name)
    for item in results:
        print("[%-4s] %s %-24s %s" % (item.status, item.scenario_id, item.name, item.detail))
        if item.log:
            print(T('sdk.log_line') % item.log)


def write_report(out_root: Path, entries: list[dict]) -> Path:
    report = {
        "generatedAt": time.strftime("%Y-%m-%d %H:%M:%S"),
        "host": {"system": host_system(), "arch": host_arch(), "python": sys.version},
        "packages": entries,
    }
    path = out_root / "report.json"
    out_root.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
    return path


def main(argv: list[str] | None = None) -> int:
    # Console language must be ready before argparse: --help is output too.
    global MESSAGES
    argv = sys.argv[1:] if argv is None else list(argv)
    MESSAGES = make_messages(_requested_lang(argv))
    set_console_utf8()
    parser = argparse.ArgumentParser(
        prog="sdk_package_test.py",
        description=T('sdk.help_description'))
    parser.add_argument("--package", action="append", default=[], metavar="ARCHIVE",
                        help=T('sdk.help_package'))
    parser.add_argument("--scenarios", default="all",
                        help=T('sdk.help_scenarios'))
    parser.add_argument("--no-build", action="store_true", help=T('sdk.help_no_build'))
    parser.add_argument("--run-examples", action="store_true", help=T('sdk.help_run_examples'))
    parser.add_argument("--out-dir", default=None,
                        help=T('sdk.help_out_dir'))
    parser.add_argument("--keep", action="store_true", help=T('sdk.help_keep'))
    parser.add_argument("--jobs", type=int, default=None,
                        help=T('sdk.help_jobs'))
    parser.add_argument("--dry-run", action="store_true", help=T('sdk.help_dry_run'))
    parser.add_argument("--list-scenarios", action="store_true", help=T('sdk.help_list_scenarios'))
    parser.add_argument("--lang", choices=('auto', 'zh_CN', 'en'), default='auto',
                        help=T('sdk.help_lang'))
    args = parser.parse_args(argv)

    if args.list_scenarios:
        print_scenarios()
        return 0

    packages = [Path(item).expanduser().resolve() for item in args.package]
    if not packages:
        parser.error(T('sdk.package_required'))
    for path in packages:
        if not path.is_file():
            parser.error(T('sdk.package_missing') % path)

    out_root = Path(args.out_dir).expanduser().resolve() if args.out_dir else \
        DEFAULT_OUT_ROOT / time.strftime("%Y%m%d-%H%M%S")
    out_root.mkdir(parents=True, exist_ok=True)
    _warn_disk_space(packages, out_root)

    options = Options(
        out_root=out_root,
        selected=parse_selection(args.scenarios),
        no_build=args.no_build,
        run_examples=args.run_examples,
        keep=args.keep,
        jobs=args.jobs or (os.cpu_count() or 4),
        dry_run=args.dry_run,
        package_paths=packages,
    )

    entries = []
    total = {"PASS": 0, "FAIL": 0, "SKIP": 0}
    for index, archive in enumerate(packages):
        # 目录名只用序号：Windows 未启用长路径时，包名+work+CMakeScratch
        # 很容易超过 MAX_PATH（MSBuild 的 .tlog 路径甚至会直接打不开）。
        package_out = out_root / ("%02d" % (index + 1))
        package_out.mkdir(parents=True, exist_ok=True)
        try:
            info = load_package(archive, package_out / "extract")
        except Exception as exc:
            print(T('sdk.load_failed_print')
                  % (archive.name, type(exc).__name__, exc))
            entries.append({
                "archive": str(archive), "package": archive.name,
                "results": [{"id": "P00", "name": "package.load", "status": "FAIL",
                             "detail": T('sdk.load_failed') % (type(exc).__name__, exc),
                             "log": ""}],
            })
            total["FAIL"] += 1
            continue

        results = run_package(info, package_out, options)
        print_results(info, results)
        for item in results:
            total[item.status] = total.get(item.status, 0) + 1
        entries.append({
            "archive": str(archive),
            "package": archive.name,
            "metadata": {
                "version": info.meta("VERSION"),
                "system": info.meta("SYSTEM"),
                "arch": info.meta("ARCH"),
                "config": info.meta("CONFIG"),
                "toolchain": info.meta("TOOLCHAIN"),
                "is_dll": info.caps["is_dll"],
                "has_sdl": info.caps["has_sdl"],
                "has_cef": info.caps["has_cef"],
                "has_examples": info.caps["has_examples"],
                "msvc_runtime": info.meta("MSVC_RUNTIME"),
            },
            "results": [
                {"id": item.scenario_id, "name": item.name, "status": item.status,
                 "detail": item.detail, "log": item.log}
                for item in results
            ],
        })
        if not options.keep and not options.dry_run:
            shutil.rmtree(package_out / "extract", ignore_errors=True)
            shutil.rmtree(package_out / "work", ignore_errors=True)

    report_path = write_report(out_root, entries)
    print(T('sdk.result_line') % (total["PASS"], total["FAIL"], total["SKIP"]))
    print(T('sdk.report_path') % report_path)
    if not options.keep and not options.dry_run:
        for item in out_root.iterdir():
            if item.is_dir() and item.name[:2].isdigit():
                shutil.rmtree(item, ignore_errors=True)

    if total["FAIL"]:
        return 1
    if total["PASS"] == 0:
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
