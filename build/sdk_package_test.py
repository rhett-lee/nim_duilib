#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""nim_duilib 发布 SDK 包测试编排器。

以“解包后的成品包”为唯一输入，覆盖三类用例：
  1. P*：包结构、校验和、元数据、接入文件、负向包；
  2. C01/C02：真实 CMake 消费者工程完成配置、编译、链接、运行；
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
import subprocess
import sys
import tarfile
import time
import traceback
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

BUILD_DIR = Path(__file__).resolve().parent
REPO_ROOT = BUILD_DIR.parent
SDK_TEST_DIR = BUILD_DIR / "sdk_test"
PACK_PY = BUILD_DIR / "pack.py"

if os.name == "nt":
    DEFAULT_OUT_ROOT = Path(r"C:\develop\temp\sdk_test")
else:
    import tempfile
    DEFAULT_OUT_ROOT = Path(tempfile.gettempdir()) / "nim_duilib_sdk_test"

# 场景登记表：(编号, 名称, 说明, 是否需要编译)
SCENARIO_ORDER = [
    ("P01", "package.extract", "解包与包根目录唯一", False),
    ("P02", "package.layout", "必需目录与文件齐全", False),
    ("P03", "package.checksums", "SHA256SUMS 完整且哈希匹配", False),
    ("P04", "package.metadata", "包名/版本/清单/CMake 元数据/实际文件一致", False),
    ("P05", "package.integration", "CMake 与 MSVC 接入文件可用", False),
    ("P06", "package.forbidden", "不含调试文件、沙箱库等禁止内容", False),
    ("P07", "package.verify-cli", "pack.py --verify 通过", False),
    ("P08", "package.docs", "文档与许可证齐全、运行资源存在", False),
    ("C01", "consumer.static", "静态库消费者：配置/编译/链接/运行", True),
    ("C02", "consumer.dll", "DLL 消费者：导入库与运行期 DLL", True),
    ("C03", "negative.config", "配置不匹配必须拒绝", True),
    ("C04", "negative.arch", "架构不匹配必须拒绝", True),
    ("C05", "negative.msvc-runtime", "MSVC 运行库不匹配必须拒绝", True),
    ("C06", "negative.toolchain", "工具链不匹配必须拒绝", True),
    ("C07", "negative.incomplete", "缺库/缺头文件必须被 --verify 发现", False),
    ("C08", "negative.tampered", "文件被篡改必须被 --verify 发现", False),
    ("C09", "package.tar-formats", "tgz/tarbz2 重新打包后仍可通过校验", False),
    ("C10", "examples.smoke", "示例程序冒烟启动（可选）", True),
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
    has_jpeg = is_msvc_like and any(name.lower().startswith("turbojpeg")
                                    for name in lib_names)

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
    if cef_src == "libcef_win":
        has_cef = "libcef.lib" in lib_names and bool(wrapper)
    elif cef_src == "libcef_win_109":
        has_cef = "libcef.lib" in lib_names and bool(wrapper)
    elif cef_src == "libcef_linux":
        has_cef = bool(wrapper) and "bin/libcef_linux/libcef.so" in files
    else:
        has_cef = False

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
        "cef_src": cef_src,
        "cef_wrapper": wrapper,
        "has_examples": has_examples,
        "has_resources": "bin/resources/themes/default/global.xml" in files,
    }


def select_archive_root(names: list[str]) -> str:
    """发布包必须只有一个顶层目录。"""
    roots = {name.split("/", 1)[0] for name in names if name and name.strip("/")}
    if not roots:
        raise ValueError("归档为空")
    if len(roots) != 1:
        raise ValueError("包根目录不唯一：%s" % ", ".join(sorted(roots)[:5]))
    return roots.pop()


def scenario_applicability(sid: str, caps: dict, meta: dict, *,
                           no_build: bool = False, run_examples: bool = False,
                           build_block_reason: str = "") -> tuple[bool, str]:
    """判断构建类场景是否适用；包级场景始终适用。"""
    if sid not in BUILD_SCENARIOS:
        return True, ""
    if no_build:
        return False, "指定了 --no-build"
    if sid in ("C01", "C02"):
        if sid == "C01" and caps.get("is_dll"):
            return False, "DLL 包由 C02 覆盖"
        if sid == "C02" and not caps.get("is_dll"):
            return False, "静态包由 C01 覆盖"
        if build_block_reason:
            return False, build_block_reason
        return True, ""
    if sid == "C10":
        if not run_examples:
            return False, "未指定 --run-examples"
        if not caps.get("has_examples"):
            return False, "包内不含示例程序"
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

    def meta(self, key: str, default: str = "") -> str:
        return self.info.get("NIM_DUILIB_PACKAGE_" + key.upper(), default)

    def archive_stem(self) -> str:
        return strip_archive_ext(self.archive.name)


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


def extract_package(archive: Path, dest: Path) -> Path:
    dest.mkdir(parents=True, exist_ok=True)
    shutil.unpack_archive(str(archive), str(dest))
    roots = [item for item in dest.iterdir() if item.is_dir()]
    if len(roots) != 1:
        raise AssertionError("包根目录不唯一：%s" % ", ".join(item.name for item in roots[:5]))
    return roots[0]


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
    select_archive_root(names)
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
        "lib/cmake/nim_duilib/nim_duilibConfig.cmake",
        "lib/cmake/nim_duilib/nim_duilibHelpers.cmake",
        "lib/cmake/nim_duilib/nim_duilibPackageInfo.cmake",
        "lib/cmake/nim_duilib/nim_duilibConfigVersion.cmake",
    ]
    problems = [item for item in required if not (info.root / item).is_file()]
    if problems:
        raise AssertionError("缺少必需文件：" + ", ".join(problems))

    stems = info.caps["lib_stems"]
    if "duilib" not in stems:
        raise AssertionError("lib/ 下缺少 duilib 主库")
    if not info.caps["is_dll"]:
        for stem in ("duilib-cximage", "duilib-webp", "duilib-png", "duilib-zlib", "skia"):
            if stem not in stems:
                raise AssertionError("静态包 lib/ 下缺少 %s" % stem)
    if not info.caps["has_resources"]:
        raise AssertionError("缺少运行资源 bin/resources/themes/default/global.xml")
    return "必需文件 %d 项齐全，库 %d 个" % (len(required), len(stems))


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
            problems.append("无法解析行：%s" % line[:60])
            continue
        sums[match.group(2)] = match.group(1).lower()
    if problems:
        raise AssertionError("；".join(problems[:5]))

    virtual = {"MANIFEST.txt", "VERSION"}
    actual = {item for item in info.files
              if item not in virtual and item != "SHA256SUMS"}
    missing = sorted(actual - set(sums))
    extra = sorted(set(sums) - actual)
    if missing:
        raise AssertionError("未登记到 SHA256SUMS：%s" % ", ".join(missing[:5]))
    if extra:
        raise AssertionError("SHA256SUMS 登记了不存在的文件：%s" % ", ".join(extra[:5]))

    mismatches = []
    for rel, digest in sums.items():
        path = info.root / rel
        if not path.is_file():
            continue
        if hashlib.sha256(path.read_bytes()).hexdigest().lower() != digest:
            mismatches.append(rel)
    if mismatches:
        raise AssertionError("SHA256 不匹配：%s" % ", ".join(mismatches[:5]))
    return "校验和覆盖 %d 个文件且全部匹配" % len(sums)


def check_metadata(info: PackageInfo) -> str:
    parts = info.name_parts
    if not parts:
        raise AssertionError("包名无法解析：%s" % info.archive.name)

    problems = []

    def expect(actual, expected, label):
        if str(actual).strip() != str(expected).strip():
            problems.append("%s：期望 '%s'，实际 '%s'" % (label, expected, actual))

    expect(parts["version"], info.meta("VERSION"), "包名版本 vs CMake 版本")
    expect(parts["version"], info.version_text, "包名版本 vs VERSION")
    expect(parts["version"], info.manifest.get("版本", ""), "包名版本 vs MANIFEST")
    expect(parts["system"], info.meta("SYSTEM"), "包名平台 vs CMake 平台")
    expect("%s-%s" % (parts["system"], parts["arch"]),
           info.manifest.get("平台", ""), "包名平台 vs MANIFEST")
    expect(parts["arch"], info.meta("ARCH"), "包名架构 vs CMake 架构")
    expect(parts["config"], info.meta("CONFIG"), "包名配置 vs CMake 配置")
    expect(parts["config"], info.manifest.get("配置", ""), "包名配置 vs MANIFEST")

    if parts["system"] == "win":
        expect(parts["toolchain"], info.meta("TOOLCHAIN"), "包名工具链 vs CMake 工具链")
        toolchain_label = info.manifest.get("工具链", "")
        expected_label = "MSVC" if parts["toolchain"] == "msvc" else "MinGW"
        if toolchain_label and not toolchain_label.startswith(expected_label):
            problems.append("MANIFEST 工具链 '%s' 与包名 '%s' 不一致"
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
    for key, name_value, file_value, label in checks:
        meta_value = flag_value(info.meta(key))
        if name_value != file_value or meta_value != file_value:
            problems.append("%s 不一致：包名=%s, CMake=%s, 实际文件=%s"
                            % (label, name_value, meta_value, file_value))

    if parts["system"] == "win":
        expect(parts["has_examples"], info.manifest.get("包含示例") == "是",
               "包名示例标志 vs MANIFEST")
        if info.meta("TOOLCHAIN") == "mingw":
            expect(parts["mingw_variant"], info.meta("MINGW_VARIANT"),
                   "包名 MinGW 变体 vs CMake")
    expect(parts["has_examples"], caps["has_examples"], "包名示例标志 vs 实际文件")

    if info.meta("SYSTEM") == "win" and info.meta("TOOLCHAIN") == "msvc":
        runtime = info.meta("MSVC_RUNTIME").upper()
        if runtime not in ("MT", "MD"):
            problems.append("MSVC 运行库元数据非法：'%s'" % runtime)
        if caps["is_dll"] and runtime != "MD":
            problems.append("DLL 包必须使用 /MD，实际元数据为 %s" % runtime)
    else:
        if info.meta("MSVC_RUNTIME"):
            problems.append("非 Windows/MSVC 包不应声明 MSVC 运行库")

    expect(info.manifest.get("包名", ""), info.root.name, "MANIFEST 包名 vs 包根目录")

    if problems:
        raise AssertionError("；".join(problems[:8]))
    return "包名/版本/清单/CMake 元数据与实际文件一致"


def check_integration(info: PackageInfo) -> str:
    cmake_dir = info.root / "lib/cmake/nim_duilib"
    problems = []
    for path in cmake_dir.glob("*.cmake"):
        text = _read_text(path)
        leftover = re.findall(r"@[A-Z0-9_]+@", text)
        if leftover:
            problems.append("%s 存在未替换占位符 %s"
                            % (path.name, ", ".join(sorted(set(leftover)))))
    version_file = cmake_dir / "nim_duilibConfigVersion.cmake"
    if not version_file.is_file() or "PACKAGE_VERSION" not in _read_text(version_file):
        problems.append("nim_duilibConfigVersion.cmake 缺少 PACKAGE_VERSION")

    system = info.meta("SYSTEM")
    if system == "win":
        for arch in ("x64", "x86", "arm64"):
            manifest = info.root / ("msvc/manifest/duilib.%s.manifest" % arch)
            if not manifest.is_file():
                problems.append("缺少 msvc/manifest/duilib.%s.manifest" % arch)
        if info.meta("TOOLCHAIN") == "msvc":
            for rel in ("msvc/nim_duilib.props", "msvc/nim_duilib.targets",
                        "msvc/nim_duilib.sdk.props"):
                if not (info.root / rel).is_file():
                    problems.append("缺少 %s" % rel)
            props = info.root / "msvc/nim_duilib.sdk.props"
            if props.is_file():
                text = _read_text(props)
                for key in ("NimDuilibPackageVersion", "NimDuilibPackageArch",
                            "NimDuilibPackageConfig", "NimDuilibPackageIsDll"):
                    if key not in text:
                        problems.append("nim_duilib.sdk.props 缺少 %s" % key)
        # 所有 Windows 包的 manifest 必须是合法 XML（带 BOM 也能解析）。
        import xml.etree.ElementTree as ET
        for path in (info.root / "msvc/manifest").glob("*.manifest"):
            try:
                ET.parse(str(path))
            except ET.ParseError as exc:
                problems.append("%s XML 非法：%s" % (path.name, exc))

    if problems:
        raise AssertionError("；".join(problems[:8]))
    return "CMake/MSVC 接入文件完整且可解析"


def check_forbidden(info: PackageInfo) -> str:
    bad_ext = (".pdb", ".ilk", ".exp", ".obj", ".o")
    problems = []
    for item in info.files:
        lower = item.lower()
        if lower.endswith(bad_ext):
            problems.append("禁止的调试/中间文件：%s" % item)
        if lower.endswith("cef_sandbox.lib"):
            problems.append("不应随包的沙箱库：%s" % item)
        if lower.endswith(".cpp") and item.startswith("include/"):
            problems.append("include/ 下不应有 .cpp：%s" % item)
        if item == "bin/resources.zip":
            problems.append("不应随包 resources.zip")
    if not info.caps["has_cef"]:
        for prefix in ("include/duilib/duilib_cef.h", "include/duilib/CEFControl/"):
            if any(item == prefix or item.startswith(prefix) for item in info.files):
                problems.append("未声明 CEF 却包含 %s" % prefix)
                break
    if not info.caps["has_sdl"]:
        if any(item.startswith("include/SDL3/") for item in info.files):
            problems.append("未声明 SDL 却包含 include/SDL3/")
    if problems:
        raise AssertionError("；".join(problems[:8]))
    return "未发现禁止内容"


def run_verify_cli(archive: Path) -> subprocess.CompletedProcess:
    return subprocess.run(
        [sys.executable, str(PACK_PY), "--verify", str(archive)],
        capture_output=True, text=True, encoding="utf-8", errors="replace")


def check_verify_cli(info: PackageInfo) -> str:
    proc = run_verify_cli(info.archive)
    if proc.returncode != 0:
        tail = (proc.stdout or "")[-1200:]
        raise AssertionError("pack.py --verify 失败（rc=%d）：\n%s" % (proc.returncode, tail))
    return "pack.py --verify 返回 0"


def check_docs(info: PackageInfo) -> str:
    docs = [item for item in info.files if item.startswith("docs/") and item.endswith(".md")]
    licenses = [item for item in info.files if item.startswith("LICENSES/")]
    problems = []
    if not docs:
        problems.append("docs/ 下没有使用文档")
    if not licenses:
        problems.append("LICENSES/ 为空")
    if not (info.root / "THIRD-PARTY-NOTICES.md").is_file():
        problems.append("缺少 THIRD-PARTY-NOTICES.md")
    if not info.caps["has_resources"]:
        problems.append("缺少 bin/resources/themes/default/global.xml")
    if problems:
        raise AssertionError("；".join(problems))
    return "文档 %d 份，许可证 %d 份，运行资源存在" % (len(docs), len(licenses))


def _make_tar(root: Path, dest: Path, mode: str) -> Path:
    dest.parent.mkdir(parents=True, exist_ok=True)
    with tarfile.open(dest, mode) as tf:
        tf.add(str(root), arcname=root.name)
    return dest


def check_tar_formats(info: PackageInfo, work_dir: Path) -> str:
    results = []
    for suffix, mode in ((".tgz", "w:gz"), (".tar.bz2", "w:bz2")):
        dest = work_dir / ("repack" + suffix)
        _make_tar(info.root, dest, mode)
        proc = run_verify_cli(dest)
        if proc.returncode != 0:
            raise AssertionError("%s 校验失败：%s" % (suffix, (proc.stdout or "")[-800:]))
        results.append(suffix)
    return "重新打包 %s 后均通过 --verify" % " / ".join(results)


def _is_library_file(rel: str) -> bool:
    return rel.startswith("lib/") and rel.count("/") == 1 and \
        rel.lower().endswith((".lib", ".a"))


def _make_tar_from_root(root: Path, dest: Path, mode: str, *,
                        skip=None, override_rel: str = "",
                        override_data: bytes = b"") -> Path:
    """由解包目录直接生成归档，可选跳过文件或替换某个文件的内容。

    直接在原目录上生成归档，避免为负向用例复制整包（大包会耗尽临时空间）。
    """
    import io

    dest.parent.mkdir(parents=True, exist_ok=True)
    with tarfile.open(dest, mode) as tf:
        for dirpath, dirnames, filenames in os.walk(root, followlinks=False):
            dirnames[:] = [name for name in dirnames
                           if not os.path.islink(os.path.join(dirpath, name))]
            for name in filenames:
                full = os.path.join(dirpath, name)
                rel = os.path.relpath(full, root).replace(os.sep, "/")
                if skip and skip(rel):
                    continue
                info = tf.gettarinfo(full, arcname=root.name + "/" + rel)
                if os.path.islink(full):
                    tf.addfile(info)
                elif rel == override_rel:
                    info.size = len(override_data)
                    tf.addfile(info, io.BytesIO(override_data))
                else:
                    with open(full, "rb") as handle:
                        tf.addfile(info, handle)
    return dest


def check_incomplete(info: PackageInfo, work_dir: Path) -> str:
    cases = [
        ("缺库", lambda rel: _is_library_file(rel)),
        ("缺头文件", lambda rel: rel.startswith("include/duilib/")),
    ]
    verified = []
    for index, (label, skip) in enumerate(cases):
        archive = _make_tar_from_root(
            info.root, work_dir / ("bad-%d.tgz" % index), "w:gz", skip=skip)
        proc = run_verify_cli(archive)
        if proc.returncode == 0:
            raise AssertionError("%s 的包本应被 --verify 拒绝，实际通过" % label)
        verified.append(label)
    return "%s 均被 --verify 拒绝" % " / ".join(verified)


def check_tampered(info: PackageInfo, work_dir: Path) -> str:
    checksum_file = _pick_checksum_target(info)
    original = (info.root / checksum_file).read_bytes()
    data = original + b"\n# sdk-package-test tampered\n"
    archive = _make_tar_from_root(
        info.root, work_dir / "tampered.tgz", "w:gz",
        override_rel=checksum_file, override_data=data)
    proc = run_verify_cli(archive)
    if proc.returncode == 0:
        raise AssertionError("篡改 %s 后 --verify 本应失败，实际通过" % checksum_file)
    if "SHA256" not in (proc.stdout or ""):
        raise AssertionError("失败原因不是 SHA256 不匹配：%s" % (proc.stdout or "")[-500:])
    return "篡改 %s 被 --verify 以 SHA256 不匹配拒绝" % checksum_file


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
        raise AssertionError("SHA256SUMS 中没有可篡改的文件")
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
        return "包平台 %s 与本机 %s 不同" % (system, host_system())
    if toolchain == "msvc" and sys.platform != "win32":
        return "MSVC 包只能在 Windows 编译"
    if toolchain == "mingw" and not os.environ.get("MSYSTEM"):
        return "MinGW 包需要 MSYS2/MinGW 环境"
    if toolchain == "mingw" and info.meta("MINGW_VARIANT") == "llvm":
        compilers = (os.environ.get("CC", "") + os.environ.get("CXX", "")).lower()
        if "clang" not in compilers:
            return "mingw-llvm 包需要在 CC/CXX 指向 clang 的环境中编译"
    tc, reason = detect_toolchain(info)
    if tc is None:
        return "工具链不可用（%s）" % reason
    if toolchain == "mingw" and tc.generator == "Visual Studio 17 2022":
        return "当前环境没有 MinGW 编译器"
    return ""


def cmake_exe() -> str:
    found = shutil.which("cmake")
    if not found:
        raise ScenarioSkipped("找不到 cmake")
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


def build_command(build_dir: Path, toolchain, jobs: int, config: str | None) -> list[str]:
    command = [cmake_exe(), "--build", str(build_dir), "--parallel", str(jobs)]
    if toolchain.generator.startswith("Visual Studio") and config:
        command += ["--config", config]
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


def check_consumer(info: PackageInfo, work_dir: Path, log_dir: Path, options: Options) -> str:
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped("工具链不可用（%s）" % reason)
    if info.meta("SYSTEM") != host_system():
        raise ScenarioSkipped("包平台与本机不同")

    build_dir = work_dir / "consumer"
    env = toolchain.env
    log = log_dir / "consumer.log"
    log.parent.mkdir(parents=True, exist_ok=True)
    if log.is_file():
        log.unlink()

    configure = configure_command(info, build_dir, toolchain)
    rc, output = run_command(configure, cwd=REPO_ROOT, env=env, log_path=log,
                             dry_run=options.dry_run)
    if options.dry_run:
        raise ScenarioSkipped("dry-run：已打印配置/编译命令")
    if rc != 0:
        raise AssertionError("配置失败（rc=%d）：\n%s" % (rc, _tail(output)))

    build = build_command(build_dir, toolchain, options.jobs, info.meta("CONFIG"))
    rc, output = run_command(build, cwd=REPO_ROOT, env=env, log_path=log,
                             dry_run=False, timeout=3600)
    if rc != 0:
        raise AssertionError("编译失败（rc=%d）：\n%s" % (rc, _tail(output)))

    exe = find_consumer_executable(build_dir)
    if exe is None:
        raise AssertionError("未找到测试程序 sdk_consumer")
    exe_dir = exe.parent
    if not (exe_dir / "resources/themes/default/global.xml").is_file():
        raise AssertionError("运行资源未复制到可执行文件同级：%s" % exe_dir)
    if info.caps["is_dll"]:
        missing = [name for name in info.caps["bin_dlls"]
                   if not (exe_dir / name).is_file()]
        if missing:
            raise AssertionError("DLL 未复制到可执行文件同级：%s" % ", ".join(missing))

    try:
        proc = subprocess.run([str(exe)], cwd=str(exe_dir), env=env,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              timeout=180)
    except subprocess.TimeoutExpired:
        raise AssertionError("运行超时（180 秒）")
    run_output = proc.stdout.decode("utf-8", errors="replace")
    _log_command(log, [str(exe)], run_output, proc.returncode)

    if proc.returncode != 0 or "SDK_TEST_RESULT=ok" not in run_output:
        raise AssertionError("运行失败（rc=%d）：\n%s"
                             % (proc.returncode, _tail(run_output)))

    features = _parse_features(run_output)
    expected = {
        "sdl": "1" if info.caps["has_sdl"] else "0",
        "cef": "1" if info.caps["has_cef"] else "0",
        "webview2": "1" if info.caps["has_webview2"] else "0",
        "jpeg": "1" if info.caps["has_jpeg"] else "0",
        "dll": "1" if info.caps["is_dll"] else "0",
    }
    wrong = ["%s=%s（期望 %s）" % (key, features.get(key, "?"), value)
             for key, value in expected.items() if features.get(key) != value]
    if wrong:
        raise AssertionError("运行期编译特性与元数据不一致：%s" % ", ".join(wrong))
    return "配置/编译/运行通过，特性开关与元数据一致"


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
        raise ScenarioSkipped("dry-run：已打印配置命令")
    if rc == 0:
        raise AssertionError("配置本应失败，实际成功")
    if expected_text not in output:
        raise AssertionError("配置失败原因不含 '%s'：\n%s"
                             % (expected_text, _tail(output, 1500)))
    return "按预期被拒绝（%s）" % expected_text


def check_negative_config(info: PackageInfo, work_dir: Path, log_dir: Path,
                          options: Options) -> str:
    blocked = build_block_reason(info)
    if blocked:
        raise ScenarioSkipped(blocked)
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped("工具链不可用（%s）" % reason)
    if toolchain.generator.startswith("Visual Studio"):
        raise ScenarioSkipped("多配置生成器无法通过 CMAKE_BUILD_TYPE 触发配置校验")
    config = info.meta("CONFIG") or "Release"
    opposite = "Release" if config == "Debug" else "Debug"
    command = configure_command(info, work_dir / "neg-config", toolchain,
                                build_type=opposite)
    return _expect_configure_failure(
        info, work_dir, log_dir, options, log_name="neg-config.log",
        command=command, env=toolchain.env, expected_text="配置为")


def check_negative_arch(info: PackageInfo, work_dir: Path, log_dir: Path,
                        options: Options) -> str:
    blocked = build_block_reason(info)
    if blocked:
        raise ScenarioSkipped(blocked)
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped("工具链不可用（%s）" % reason)
    if info.meta("SYSTEM") != "win":
        if info.meta("ARCH") == host_arch():
            raise ScenarioSkipped("包架构与主机一致，且当前平台无法交叉切换架构")
        command = configure_command(info, work_dir / "neg-arch", toolchain)
        return _expect_configure_failure(
            info, work_dir, log_dir, options, log_name="neg-arch.log",
            command=command, env=toolchain.env, expected_text="架构为")

    package_arch = info.meta("ARCH")
    opposite = {"x64": "x86", "x86": "x64", "arm64": "x64"}.get(package_arch, "x86")
    if toolchain.vs_install is not None:
        command = configure_command(info, work_dir / "neg-arch", toolchain,
                                    generator="Visual Studio 17 2022",
                                    arch=opposite)
        return _expect_configure_failure(
            info, work_dir, log_dir, options, log_name="neg-arch.log",
            command=command, env=toolchain.env, expected_text="架构为")
    if package_arch != host_arch():
        command = configure_command(info, work_dir / "neg-arch", toolchain)
        return _expect_configure_failure(
            info, work_dir, log_dir, options, log_name="neg-arch.log",
            command=command, env=toolchain.env, expected_text="架构为")
    raise ScenarioSkipped("未安装 Visual Studio，且包架构与主机一致")


def check_negative_msvc_runtime(info: PackageInfo, work_dir: Path, log_dir: Path,
                                options: Options) -> str:
    if info.meta("SYSTEM") != "win" or info.meta("TOOLCHAIN") != "msvc":
        raise ScenarioSkipped("仅 Windows MSVC 包适用")
    runtime = info.meta("MSVC_RUNTIME").upper()
    if runtime not in ("MT", "MD"):
        raise ScenarioSkipped("包未声明 MSVC 运行库")
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped("工具链不可用（%s）" % reason)
    opposite = ("MultiThreaded$<$<CONFIG:Debug>:Debug>"
                if runtime == "MD"
                else "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    command = configure_command(info, work_dir / "neg-runtime", toolchain,
                                extra=["-DCMAKE_MSVC_RUNTIME_LIBRARY=%s" % opposite])
    return _expect_configure_failure(
        info, work_dir, log_dir, options, log_name="neg-runtime.log",
        command=command, env=toolchain.env, expected_text="运行库")


def check_negative_toolchain(info: PackageInfo, work_dir: Path, log_dir: Path,
                             options: Options) -> str:
    package_toolchain = info.meta("TOOLCHAIN")
    if package_toolchain not in ("msvc", "mingw"):
        raise ScenarioSkipped("非 Windows 包不存在 MSVC/MinGW 工具链对立面")
    toolchain, reason = detect_toolchain(info)
    if toolchain is None:
        raise ScenarioSkipped("工具链不可用（%s）" % reason)

    if package_toolchain == "msvc":
        gxx = shutil.which("g++")
        if not gxx:
            raise ScenarioSkipped("未找到 g++，无法模拟 MinGW 工具链")
        generator = "Ninja" if toolchain.ninja else None
        if generator is None:
            raise ScenarioSkipped("未找到 Ninja，无法切换编译器")
        command = configure_command(info, work_dir / "neg-toolchain", toolchain,
                                    generator=generator,
                                    extra=["-DCMAKE_CXX_COMPILER=%s" % gxx])
    else:
        if toolchain.vs_install is None or not shutil.which("cl", path=toolchain.env.get("PATH")):
            raise ScenarioSkipped("未安装 MSVC，无法模拟 MSVC 工具链")
        command = configure_command(info, work_dir / "neg-toolchain", toolchain)
    return _expect_configure_failure(
        info, work_dir, log_dir, options, log_name="neg-toolchain.log",
        command=command, env=toolchain.env, expected_text="该 nim_duilib SDK 是")


def check_examples_smoke(info: PackageInfo, work_dir: Path, log_dir: Path,
                         options: Options) -> str:
    candidates = [
        info.root / "bin/basic.exe",
        info.root / "bin/basic",
        info.root / "bin/basic.app/Contents/MacOS/basic",
    ]
    exe = next((path for path in candidates if path.is_file()), None)
    if exe is None:
        raise ScenarioSkipped("未找到 bin/basic 示例程序")
    proc = subprocess.Popen([str(exe)], cwd=str(exe.parent),
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        time.sleep(3)
        code = proc.poll()
        if code is not None:
            raise AssertionError("示例程序启动后立即退出（rc=%s）" % code)
    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=10)
            except subprocess.TimeoutExpired:
                proc.kill()
    return "bin/basic 启动 3 秒后仍在运行"


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


def _scenario_runner(sid: str, info: PackageInfo, work_dir: Path,
                     log_dir: Path, options: Options):
    if sid == "P01":
        return "解包成功：%d 个文件，包根 %s" % (len(info.files), info.root.name)
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
    raise AssertionError("未知场景：%s" % sid)


def run_package(info: PackageInfo, package_out: Path, options: Options) -> list[Result]:
    work_dir = package_out / "work"
    log_dir = package_out / "logs"
    work_dir.mkdir(parents=True, exist_ok=True)
    log_dir.mkdir(parents=True, exist_ok=True)

    block_reason = build_block_reason(info)
    results = []
    for sid in [item[0] for item in SCENARIO_ORDER if item[0] in options.selected]:
        name = SCENARIO_NAMES[sid]
        if options.dry_run and sid in BUILD_SCENARIOS:
            results.append(Result(sid, name, "SKIP", "dry-run：未执行", ""))
            continue
        applicable, reason = scenario_applicability(
            sid, info.caps, info.info, no_build=options.no_build,
            run_examples=options.run_examples, build_block_reason=block_reason)
        if not applicable:
            results.append(Result(sid, name, "SKIP", reason, ""))
            continue
        try:
            detail = _scenario_runner(sid, info, work_dir, log_dir, options)
            results.append(Result(sid, name, "PASS", detail, ""))
        except ScenarioSkipped as exc:
            results.append(Result(sid, name, "SKIP", str(exc), ""))
        except AssertionError as exc:
            results.append(Result(sid, name, "FAIL", str(exc), _existing_log(log_dir)))
        except Exception as exc:  # 未预期异常也计为失败，但不中断其它场景
            detail = "%s: %s\n%s" % (type(exc).__name__, exc, traceback.format_exc(limit=3))
            results.append(Result(sid, name, "FAIL", detail, _existing_log(log_dir)))
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
            raise SystemExit("未知场景：%s（用 --list-scenarios 查看）" % item)
        selected.add(item)
    if not selected:
        raise SystemExit("--scenarios 不能为空")
    return selected


def print_scenarios() -> None:
    print("%-4s %-24s %-8s %s" % ("编号", "名称", "类型", "说明"))
    print("-" * 78)
    for sid, name, desc, needs_build in SCENARIO_ORDER:
        print("%-4s %-24s %-8s %s" % (sid, name, "需编译" if needs_build else "包级", desc))


def print_results(package: PackageInfo, results: list[Result]) -> None:
    print("\n== %s" % package.archive.name)
    for item in results:
        print("[%-4s] %s %-24s %s" % (item.status, item.scenario_id, item.name, item.detail))
        if item.log:
            print("       日志：%s" % item.log)


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
    parser = argparse.ArgumentParser(
        prog="sdk_package_test.py",
        description="nim_duilib 发布 SDK 包测试（结构校验 + 消费者编译运行 + 负向用例）")
    parser.add_argument("--package", action="append", default=[], metavar="ARCHIVE",
                        help="待测试的发布包，可重复指定")
    parser.add_argument("--scenarios", default="all",
                        help="场景编号列表（逗号分隔）或 all")
    parser.add_argument("--no-build", action="store_true", help="只执行包级场景")
    parser.add_argument("--run-examples", action="store_true", help="额外冒烟启动示例程序")
    parser.add_argument("--out-dir", default=None, help="临时目录与报告输出目录")
    parser.add_argument("--keep", action="store_true", help="保留解包与构建目录")
    parser.add_argument("--jobs", type=int, default=None, help="并行编译任务数")
    parser.add_argument("--dry-run", action="store_true", help="只打印将执行的命令")
    parser.add_argument("--list-scenarios", action="store_true", help="列出场景后退出")
    args = parser.parse_args(argv)

    if args.list_scenarios:
        print_scenarios()
        return 0

    packages = [Path(item).expanduser().resolve() for item in args.package]
    if not packages:
        parser.error("需要 --package <发布包>（可重复指定）")
    for path in packages:
        if not path.is_file():
            parser.error("包不存在：%s" % path)

    out_root = Path(args.out_dir).expanduser().resolve() if args.out_dir else \
        DEFAULT_OUT_ROOT / time.strftime("%Y%m%d-%H%M%S")
    out_root.mkdir(parents=True, exist_ok=True)

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
        package_out = out_root / ("%02d-%s" % (index + 1, archive.stem))
        package_out.mkdir(parents=True, exist_ok=True)
        try:
            info = load_package(archive, package_out / "extract")
        except Exception as exc:
            print("[FAIL] %s\n       无法加载发布包：%s: %s"
                  % (archive.name, type(exc).__name__, exc))
            entries.append({
                "archive": str(archive), "package": archive.name,
                "results": [{"id": "P00", "name": "package.load", "status": "FAIL",
                             "detail": "无法加载发布包：%s: %s" % (type(exc).__name__, exc),
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
    print("\n结果：PASS=%d FAIL=%d SKIP=%d" % (total["PASS"], total["FAIL"], total["SKIP"]))
    print("报告：%s" % report_path)
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
