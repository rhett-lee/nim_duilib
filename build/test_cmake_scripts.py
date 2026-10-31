# -*- coding: utf-8 -*-
"""构建系统 CMake 侧脚本的回归测试。

运行：
  python3 build/build.py test
  python3 -m unittest build.test_cmake_scripts -v

覆盖：
  * cmake/duilib_skia_variants.cmake 的候选目录名（并与 build/pack.py 打包侧比对，
    防止两侧再次漂移）；
  * cmake/duilib_sdl_variants.cmake 的变体前缀目录名（MSVC 按架构/配置/运行库，
    MinGW 复用 Skia 的 release 变体；同样与 build/pack.py 逐项比对）；
  * cmake/duilib_deps.cmake 的 off 模式逐配置检查（含 MinGW 只认 .a）；
  * cmake/duilib_deps_run.cmake 的多配置 Skia 准备（Debug/Release 各一份）；
  * cmake/duilib_deps_run.cmake 的 SDL3 变体安装，含“安装后写 stamp → 再次校验”
    的构建成功路径（用桩替换真实构建，防止漏传变体参数而误报失败）；
  * _duilib_sdl_ready 调用点的参数个数静态校验；
  * auto 模式的 DUILIB_SDL_AUTOBUILD 作用域传递，以及根工程解析 SDL3 变体时
    自己定义 DUILIB_MINGW 的静态校验；
  * build.py find_cmake() 的定位顺序（测试不再依赖 PATH 上一定有 cmake）；
  * 真实 configure 冒烟（prepare 模式，验证 CMake 改动能被完整解析/生成）。

夹具 build/tests/skia_variants_check.cmake 通过 `cmake -P` 执行，结果取 stdout
中的 "RESULT:" 行。
"""
import importlib.util
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

BUILD_DIR = Path(__file__).resolve().parent
REPO_ROOT = BUILD_DIR.parent
if str(BUILD_DIR) not in sys.path:
    sys.path.insert(0, str(BUILD_DIR))

import pack  # noqa: E402

# 按路径加载 build.py：仓库根目录下同名目录会被当作命名空间包，不能直接 import build。
_spec = importlib.util.spec_from_file_location("duilib_build_entry", BUILD_DIR / "build.py")
entry = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(entry)

HARNESS = BUILD_DIR / "tests" / "skia_variants_check.cmake"

# 统一入口：PATH 上没有 cmake 时回退到 Visual Studio 自带的 CMake（build.py 同款逻辑）
CMAKE = shutil.which("cmake") or entry.find_cmake()


def _require_cmake(case):
    if not CMAKE:
        case.skipTest("cmake not found (PATH / DUILIB_CMAKE_EXECUTABLE / Visual Studio)")


def _run_harness(mode, timeout=120, process_env=None, **defines):
    """执行 CMake 夹具，返回 (returncode, stdout+stderr)。"""
    command = [CMAKE, "-DDUILIB_TEST_MODE=%s" % mode]
    for key, value in defines.items():
        if value is None:
            continue
        command.append("-D%s=%s" % (key, value))
    command += ["-P", str(HARNESS)]
    result = subprocess.run(
        command, cwd=str(REPO_ROOT), stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, encoding="utf-8", errors="replace", timeout=timeout, env=process_env)
    return result.returncode, result.stdout


def _result(output):
    for line in output.splitlines():
        if "RESULT:" in line:
            return line.split("RESULT:", 1)[1].strip()
    return ""


class SkiaVariantCandidateTests(unittest.TestCase):
    """候选目录名规则（cmake/duilib_skia_variants.cmake）。"""

    CASES = [
        # MSVC / clang-cl：llvm.<arch>.<debug|release>，/MD 时追加 .mdd / .md
        dict(name="msvc-debug", config="Debug", platform="windows", arch="x64",
             mingw="0", md="0", gcc="0",
             expected="llvm.x64.debug"),
        dict(name="msvc-release", config="Release", platform="windows", arch="x64",
             mingw="0", md="0", gcc="0",
             expected="llvm.x64.release"),
        dict(name="msvc-debug-md", config="Debug", platform="windows", arch="x64",
             mingw="0", md="1", gcc="0",
             expected="llvm.x64.debug.mdd"),
        dict(name="msvc-release-md", config="Release", platform="windows", arch="x64",
             mingw="0", md="1", gcc="0",
             expected="llvm.x64.release.md"),
        dict(name="msvc-x86-debug", config="Debug", platform="windows", arch="x86",
             mingw="0", md="0", gcc="0",
             expected="llvm.x86.debug"),
        dict(name="msvc-arm64-release", config="Release", platform="windows", arch="arm64",
             mingw="0", md="0", gcc="0",
             expected="llvm.arm64.release"),
        # 独立 MinGW-w64：本工具链前缀优先，另一组前缀回退
        dict(name="mingw-gcc-debug", config="Debug", platform="windows", arch="x64",
             mingw="1", md="0", gcc="1", msys2_env="", mingw_name="mingw64-",
             expected="mingw64-gcc.x64.debug;mingw64-gcc.x64.release;"
                      "gcc.x64.debug;gcc.x64.release"),
        dict(name="mingw-gcc-release", config="Release", platform="windows", arch="x64",
             mingw="1", md="0", gcc="1", msys2_env="", mingw_name="mingw64-",
             expected="mingw64-gcc.x64.release;gcc.x64.release"),
        dict(name="mingw-llvm-debug", config="Debug", platform="windows", arch="x64",
             mingw="1", md="0", gcc="0", msys2_env="", mingw_name="mingw64-",
             expected="mingw64-llvm.x64.debug;mingw64-llvm.x64.release;"
                      "llvm.x64.debug;llvm.x64.release"),
        dict(name="mingw-llvm-release", config="Release", platform="windows", arch="x64",
             mingw="1", md="0", gcc="0", msys2_env="", mingw_name="mingw64-",
             expected="mingw64-llvm.x64.release;llvm.x64.release"),
        # MSYS2 MINGW64：msvcrt 变体优先（与 UCRT64 的 gcc 产物不兼容），无前缀优先
        dict(name="msys2-mingw64-gcc-debug", config="Debug", platform="windows", arch="x64",
             mingw="1", md="0", gcc="1", msys2_env="mingw64", mingw_name="",
             expected="gcc.msvcrt.x64.release;gcc.x64.debug;gcc.x64.release;"
                      "mingw64-gcc.x64.debug;mingw64-gcc.x64.release"),
        dict(name="msys2-mingw64-llvm-debug", config="Debug", platform="windows", arch="x64",
             mingw="1", md="0", gcc="0", msys2_env="mingw64", mingw_name="",
             expected="llvm.x64.debug;llvm.x64.release;"
                      "mingw64-llvm.x64.debug;mingw64-llvm.x64.release"),
        # MSYS2 UCRT64：没有 msvcrt 变体
        dict(name="mingw-gcc-ucrt64-debug", config="Debug", platform="windows", arch="x64",
             mingw="1", md="0", gcc="1", msys2_env="ucrt64", mingw_name="",
             expected="gcc.x64.debug;gcc.x64.release;"
                      "mingw64-gcc.x64.debug;mingw64-gcc.x64.release"),
        dict(name="mingw-gcc-ucrt64-release", config="Release", platform="windows", arch="x64",
             mingw="1", md="0", gcc="1", msys2_env="ucrt64", mingw_name="",
             expected="gcc.x64.release;mingw64-gcc.x64.release"),
    ]

    def test_candidates(self):
        _require_cmake(self)
        for case in self.CASES:
            with self.subTest(case=case["name"]):
                rc, output = _run_harness(
                    "candidates",
                    DUILIB_TEST_CONFIG=case["config"],
                    DUILIB_TEST_PLATFORM=case["platform"],
                    DUILIB_TEST_ARCH=case["arch"],
                    DUILIB_TEST_MINGW=case["mingw"],
                    DUILIB_TEST_MD=case["md"],
                    DUILIB_TEST_GCC=case["gcc"],
                    DUILIB_TEST_MSYS2_ENV=case.get("msys2_env", ""),
                    DUILIB_TEST_MINGW_NAME=case.get("mingw_name", ""))
                self.assertEqual(rc, 0, output)
                self.assertEqual(_result(output), case["expected"], output)


class SkiaCandidateConsistencyTests(unittest.TestCase):
    """cmake/duilib_skia_variants.cmake 与 build/pack.py 的候选目录名必须一致。"""

    def _pack_names(self, case):
        return pack._skia_candidate_names(
            case["config"], case["arch"],
            toolchain="mingw" if case["mingw"] == "1" else "msvc",
            mingw_variant=("gcc" if case["gcc"] == "1" else "llvm")
            if case["mingw"] == "1" else None,
            md_mode=case["md"] == "1",
            msys2_env=case.get("msys2_env", ""), windows=True)

    def test_cmake_matches_pack(self):
        _require_cmake(self)
        for case in SkiaVariantCandidateTests.CASES:
            if case["platform"] != "windows":
                continue
            with self.subTest(case=case["name"]):
                rc, output = _run_harness(
                    "candidates",
                    DUILIB_TEST_CONFIG=case["config"],
                    DUILIB_TEST_PLATFORM=case["platform"],
                    DUILIB_TEST_ARCH=case["arch"],
                    DUILIB_TEST_MINGW=case["mingw"],
                    DUILIB_TEST_MD=case["md"],
                    DUILIB_TEST_GCC=case["gcc"],
                    DUILIB_TEST_MSYS2_ENV=case.get("msys2_env", ""),
                    DUILIB_TEST_MINGW_NAME=case.get("mingw_name", ""))
                self.assertEqual(rc, 0, output)
                cmake_names = _result(output).split(";")
                pack_names = self._pack_names(case)
                # 打包侧唯一允许的差异：推断不出 MSYS2 环境（env 为空）且编译器是
                # gcc 时，末尾追加 gcc.msvcrt.<arch>.release 兜底——CMake 侧依据编译器
                # 安装位置可能正好装成了该变体。其余顺序必须逐项相同。
                extra = pack_names[len(cmake_names):]
                self.assertEqual(
                    pack_names[:len(cmake_names)], cmake_names,
                    "CMake 与 pack.py 的 Skia 候选目录名不一致（%s）:\n"
                    "  cmake: %s\n  pack : %s"
                    % (case["name"], cmake_names, pack_names))
                if extra:
                    allowed = (case["mingw"] == "1" and case["gcc"] == "1"
                               and not case.get("msys2_env", ""))
                    if not allowed:
                        self.fail("pack.py 多出了意料之外的候选（%s）：%s"
                                  % (case["name"], extra))
                    self.assertEqual(extra,
                                     ["gcc.msvcrt.%s.release" % case["arch"]],
                                     "pack.py 的兜底候选只允许是 gcc.msvcrt（%s）"
                                     % case["name"])

    def test_non_windows_cmake_is_pack_subset(self):
        """非 Windows：CMake 按工具链给出 llvm/gcc，pack.py 两个都备选。"""
        _require_cmake(self)
        rc, output = _run_harness(
            "candidates", DUILIB_TEST_CONFIG="Release", DUILIB_TEST_PLATFORM="other",
            DUILIB_TEST_ARCH="x64", DUILIB_TEST_MINGW="0", DUILIB_TEST_GCC="0")
        self.assertEqual(rc, 0, output)
        cmake_names = _result(output).split(";")
        pack_names = pack._skia_candidate_names("Release", "x64", windows=False)
        self.assertTrue(set(cmake_names) <= set(pack_names),
                        "CMake 候选不在 pack.py 的候选集合中：%s vs %s"
                        % (cmake_names, pack_names))


class SdlVariantNameTests(unittest.TestCase):
    """SDL3 变体前缀目录名（cmake/duilib_sdl_variants.cmake）。"""

    CASES = [
        # MSVC：msvc.<arch>.<debug|release>，/MD（--md / --dll）时追加 .mdd / .md
        dict(name="msvc-release-mt", config="Release", platform="windows", arch="x64",
             mingw="0", md="0", gcc="0",
             expected="msvc.x64.release"),
        dict(name="msvc-debug-mt", config="Debug", platform="windows", arch="x64",
             mingw="0", md="0", gcc="0",
             expected="msvc.x64.debug"),
        dict(name="msvc-release-md", config="Release", platform="windows", arch="x64",
             mingw="0", md="1", gcc="0",
             expected="msvc.x64.release.md"),
        dict(name="msvc-debug-md", config="Debug", platform="windows", arch="x64",
             mingw="0", md="1", gcc="0",
             expected="msvc.x64.debug.mdd"),
        dict(name="msvc-x86-debug", config="Debug", platform="windows", arch="x86",
             mingw="0", md="0", gcc="0",
             expected="msvc.x86.debug"),
        dict(name="msvc-arm64-release", config="Release", platform="windows", arch="arm64",
             mingw="0", md="0", gcc="0",
             expected="msvc.arm64.release"),
        # MinGW：与 Skia 的 release 变体同名，Debug 也复用 release（CRT 由工具链决定）
        dict(name="mingw-gcc-release", config="Release", platform="windows", arch="x64",
             mingw="1", md="0", gcc="1", msys2_env="", mingw_name="mingw64-",
             expected="mingw64-gcc.x64.release"),
        dict(name="mingw-gcc-debug", config="Debug", platform="windows", arch="x64",
             mingw="1", md="0", gcc="1", msys2_env="", mingw_name="mingw64-",
             expected="mingw64-gcc.x64.release"),
        dict(name="mingw-llvm-release", config="Release", platform="windows", arch="x64",
             mingw="1", md="0", gcc="0", msys2_env="", mingw_name="mingw64-",
             expected="mingw64-llvm.x64.release"),
        # MSYS2 MINGW64 使用 msvcrt，目录名带 .msvcrt 后缀（与 UCRT64 不兼容）
        dict(name="msys2-mingw64-gcc-release", config="Release", platform="windows",
             arch="x64", mingw="1", md="0", gcc="1", msys2_env="mingw64", mingw_name="",
             expected="gcc.msvcrt.x64.release"),
        dict(name="msys2-mingw64-gcc-debug", config="Debug", platform="windows",
             arch="x64", mingw="1", md="0", gcc="1", msys2_env="mingw64", mingw_name="",
             expected="gcc.msvcrt.x64.release"),
        dict(name="msys2-ucrt64-gcc-release", config="Release", platform="windows",
             arch="x64", mingw="1", md="0", gcc="1", msys2_env="ucrt64", mingw_name="",
             expected="gcc.x64.release"),
        dict(name="msys2-clang64-llvm-release", config="Release", platform="windows",
             arch="x64", mingw="1", md="0", gcc="0", msys2_env="clang64", mingw_name="",
             expected="llvm.x64.release"),
        # 非 Windows 不做变体（沿用 <deps-root>/SDL3 单前缀）
        dict(name="non-windows", config="Release", platform="other", arch="x64",
             mingw="0", md="0", gcc="0",
             expected=""),
    ]

    def test_variant_names(self):
        _require_cmake(self)
        for case in self.CASES:
            with self.subTest(case=case["name"]):
                rc, output = _run_harness(
                    "sdl_variant",
                    DUILIB_TEST_CONFIG=case["config"],
                    DUILIB_TEST_PLATFORM=case["platform"],
                    DUILIB_TEST_ARCH=case["arch"],
                    DUILIB_TEST_MINGW=case["mingw"],
                    DUILIB_TEST_MD=case["md"],
                    DUILIB_TEST_GCC=case["gcc"],
                    DUILIB_TEST_MSYS2_ENV=case.get("msys2_env", ""),
                    DUILIB_TEST_MINGW_NAME=case.get("mingw_name", ""))
                self.assertEqual(rc, 0, output)
                self.assertEqual(_result(output), case["expected"], output)


class SdlVariantConsistencyTests(unittest.TestCase):
    """cmake/duilib_sdl_variants.cmake 与 build/pack.py 的变体目录名必须一致。"""

    def test_cmake_matches_pack(self):
        _require_cmake(self)
        for case in SdlVariantNameTests.CASES:
            is_windows = case["platform"] == "windows"
            mingw = case["mingw"] == "1"
            with self.subTest(case=case["name"]):
                rc, output = _run_harness(
                    "sdl_variant",
                    DUILIB_TEST_CONFIG=case["config"],
                    DUILIB_TEST_PLATFORM=case["platform"],
                    DUILIB_TEST_ARCH=case["arch"],
                    DUILIB_TEST_MINGW=case["mingw"],
                    DUILIB_TEST_MD=case["md"],
                    DUILIB_TEST_GCC=case["gcc"],
                    DUILIB_TEST_MSYS2_ENV=case.get("msys2_env", ""),
                    DUILIB_TEST_MINGW_NAME=case.get("mingw_name", ""))
                self.assertEqual(rc, 0, output)
                cmake_name = _result(output)
                if not is_windows:
                    expected_pack = ""
                else:
                    expected_pack = pack._sdl_variant_name(
                        case["config"], case["arch"],
                        toolchain="mingw" if mingw else "msvc",
                        mingw_variant=("gcc" if case["gcc"] == "1" else "llvm")
                        if mingw else None,
                        md_mode=case["md"] == "1",
                        msys2_env=case.get("msys2_env", ""), windows=True)
                self.assertEqual(cmake_name, expected_pack,
                                 "CMake 与 pack.py 的 SDL3 变体目录名不一致（%s）:\n"
                                 "  cmake: %r\n  pack : %r"
                                 % (case["name"], cmake_name, expected_pack))


class SdlIncludeDirTests(unittest.TestCase):
    """SDL3 头文件目录：多配置生成器下必须与链接的变体一样按配置切换。"""

    CASES = [
        # Debug/Release 指向不同变体（MSVC 多配置）：用生成器表达式
        dict(name="different-prefixes",
             debug="/root/msvc.x64.debug", release="/root/msvc.x64.release",
             expected="$<$<CONFIG:Debug>:/root/msvc.x64.debug/include>"
                      "$<$<NOT:$<CONFIG:Debug>>:/root/msvc.x64.release/include>"),
        # MinGW：Debug/Release 共用 release 变体，退化为普通目录
        dict(name="same-prefix",
             debug="/root/mingw64-gcc.x64.release",
             release="/root/mingw64-gcc.x64.release",
             expected="/root/mingw64-gcc.x64.release/include"),
        dict(name="debug-only", debug="/root/msvc.x64.debug", release="",
             expected="/root/msvc.x64.debug/include"),
        dict(name="release-only", debug="", release="/root/msvc.x64.release",
             expected="/root/msvc.x64.release/include"),
        dict(name="no-prefix", debug="", release="", expected=""),
    ]

    def test_include_dir(self):
        _require_cmake(self)
        for case in self.CASES:
            with self.subTest(case=case["name"]):
                rc, output = _run_harness(
                    "sdl_include",
                    DUILIB_TEST_SDL_DEBUG_PREFIX=case["debug"],
                    DUILIB_TEST_SDL_RELEASE_PREFIX=case["release"])
                self.assertEqual(rc, 0, output)
                self.assertEqual(_result(output), case["expected"], output)


class _SdlRootMixin:
    """在临时目录里造 SDL3 安装根与依赖目录。"""

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="duilib_sdl_test_")
        self.sdl_root = os.path.join(self.tmp, "SDL3")
        self.deps_root = os.path.join(self.tmp, "deps")
        os.makedirs(self.sdl_root, exist_ok=True)
        os.makedirs(self.deps_root, exist_ok=True)

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def make_prefix(self, variant, lib_name="SDL3-static.lib", lib_subdir="lib",
                    header=True, lib=True):
        prefix = os.path.join(self.sdl_root, variant) if variant else self.sdl_root
        os.makedirs(os.path.join(prefix, "include", "SDL3"), exist_ok=True)
        os.makedirs(os.path.join(prefix, lib_subdir), exist_ok=True)
        if header:
            with open(os.path.join(prefix, "include", "SDL3", "SDL.h"), "wb") as fp:
                fp.write(b"")
        if lib:
            with open(os.path.join(prefix, lib_subdir, lib_name), "wb") as fp:
                fp.write(b"")
        return prefix

    def read_stamp(self, prefix):
        with open(os.path.join(prefix, ".duilib-sdl"), "r", encoding="utf-8") as fp:
            return fp.read()

    def _norm(self, path):
        return os.path.normcase(os.path.normpath(path))


class SdlPrefixResolutionTests(_SdlRootMixin, unittest.TestCase):
    """SDL3 安装前缀解析：变体目录优先、<root> 作为单前缀回退。"""

    def _resolve(self, config="Release", platform="windows", arch="x64",
                 mingw="0", md="0", gcc="0", msys2_env="", mingw_name="mingw64-"):
        return _run_harness(
            "sdl_prefix",
            DUILIB_TEST_SDL_ROOT=self.sdl_root,
            DUILIB_TEST_CONFIG=config,
            DUILIB_TEST_PLATFORM=platform,
            DUILIB_TEST_ARCH=arch,
            DUILIB_TEST_MINGW=mingw,
            DUILIB_TEST_MD=md,
            DUILIB_TEST_GCC=gcc,
            DUILIB_TEST_MSYS2_ENV=msys2_env,
            DUILIB_TEST_MINGW_NAME=mingw_name)

    def _assert_resolved(self, output, expected_prefix, expected_lib_dir, canonical):
        rc_ok = _result(output).split("|")
        self.assertEqual(len(rc_ok), 3, output)
        self.assertEqual(self._norm(rc_ok[0]), self._norm(expected_prefix), output)
        self.assertEqual(self._norm(rc_ok[1]), self._norm(expected_lib_dir), output)
        self.assertEqual(rc_ok[2], canonical, output)

    def test_variant_prefix_wins_over_root(self):
        """变体目录完整时优先使用它（同一根下的旧单前缀只作回退）。"""
        _require_cmake(self)
        self.make_prefix("msvc.x64.release")
        self.make_prefix("")
        rc, output = self._resolve(config="Release")
        self.assertEqual(rc, 0, output)
        variant = os.path.join(self.sdl_root, "msvc.x64.release")
        self._assert_resolved(output, variant,
                              os.path.join(variant, "lib"), "SDL3-static.lib")

    def test_debug_resolves_debug_variant(self):
        _require_cmake(self)
        self.make_prefix("msvc.x64.debug")
        self.make_prefix("msvc.x64.release")
        rc, output = self._resolve(config="Debug")
        self.assertEqual(rc, 0, output)
        variant = os.path.join(self.sdl_root, "msvc.x64.debug")
        self._assert_resolved(output, variant,
                              os.path.join(variant, "lib"), "SDL3-static.lib")

    def test_fallback_to_single_prefix(self):
        """没有变体目录时回退到 <root>（兼容显式传入的自备单前缀）。"""
        _require_cmake(self)
        self.make_prefix("")
        rc, output = self._resolve(config="Debug")
        self.assertEqual(rc, 0, output)
        self._assert_resolved(output, self.sdl_root,
                              os.path.join(self.sdl_root, "lib"), "SDL3-static.lib")

    def test_variant_without_library_falls_back(self):
        """变体目录只有头文件不算可用，继续尝试 <root>。"""
        _require_cmake(self)
        self.make_prefix("msvc.x64.release", lib=False)
        self.make_prefix("")
        rc, output = self._resolve(config="Release")
        self.assertEqual(rc, 0, output)
        self._assert_resolved(output, self.sdl_root,
                              os.path.join(self.sdl_root, "lib"), "SDL3-static.lib")

    def test_mingw_uses_lib_sdl3_a(self):
        _require_cmake(self)
        self.make_prefix("mingw64-gcc.x64.release", lib_name="libSDL3.a")
        rc, output = self._resolve(config="Debug", mingw="1", gcc="1")
        self.assertEqual(rc, 0, output)
        variant = os.path.join(self.sdl_root, "mingw64-gcc.x64.release")
        self._assert_resolved(output, variant,
                              os.path.join(variant, "lib"), "libSDL3.a")

    def test_non_windows_uses_root_and_lib64(self):
        """非 Windows 保持单前缀；CMake 的安装可能落在 lib64。"""
        _require_cmake(self)
        self.make_prefix("", lib_name="libSDL3.so", lib_subdir="lib64")
        rc, output = self._resolve(config="Release", platform="other", arch="x64")
        self.assertEqual(rc, 0, output)
        self._assert_resolved(output, self.sdl_root,
                              os.path.join(self.sdl_root, "lib64"),
                              "SDL3-static.lib")

    def test_nothing_found(self):
        _require_cmake(self)
        rc, output = self._resolve(config="Release")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output).split("|")[0], "<none>", output)


class DependenciesOffModeSdlTests(_SdlRootMixin, unittest.TestCase):
    """cmake/duilib_deps.cmake 的 off 模式：逐配置检查 SDL3 变体前缀。"""

    def _check(self, config=None, config_types=None, mingw="0", gcc="0",
               msys2_env="", mingw_name="mingw64-", windows="1", arch="x64", md="0"):
        return _run_harness(
            "deps_check_sdl",
            DUILIB_TEST_SDL_ROOT=self.sdl_root,
            DUILIB_TEST_CONFIG=config or "Release",
            DUILIB_TEST_CONFIGURATION_TYPES=config_types or "",
            DUILIB_TEST_WINDOWS=windows,
            DUILIB_TEST_ARCH=arch,
            DUILIB_TEST_MINGW=mingw,
            DUILIB_TEST_GCC=gcc,
            DUILIB_TEST_MD=md,
            DUILIB_TEST_MSYS2_ENV=msys2_env,
            DUILIB_TEST_MINGW_NAME=mingw_name)

    def test_multi_config_requires_both_variants(self):
        _require_cmake(self)
        self.make_prefix("msvc.x64.debug")
        self.make_prefix("msvc.x64.release")
        rc, output = self._check(config_types="Debug;Release")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "OK")

    def test_multi_config_missing_debug_reports_that_config(self):
        _require_cmake(self)
        self.make_prefix("msvc.x64.release")
        rc, output = self._check(config_types="Debug;Release")
        self.assertNotEqual(rc, 0, output)
        self.assertIn("no usable SDL3 installation prefix", output)
        self.assertIn("msvc.x64.debug", output)

    def test_single_prefix_fallback_ok(self):
        _require_cmake(self)
        self.make_prefix("")
        rc, output = self._check(config="Release")
        self.assertEqual(rc, 0, output)

    def test_mingw_debug_accepts_release_variant(self):
        """MinGW 的 Debug/Release 共用 release 变体（CRT 由工具链决定）。"""
        _require_cmake(self)
        self.make_prefix("mingw64-gcc.x64.release", lib_name="libSDL3.a")
        rc, output = self._check(config="Debug", mingw="1", gcc="1")
        self.assertEqual(rc, 0, output)

    def test_mingw_requires_a_static_library(self):
        """MinGW 只认 .a：只有 .lib 的目录不可用。"""
        _require_cmake(self)
        self.make_prefix("mingw64-gcc.x64.release", lib_name="SDL3-static.lib")
        rc, output = self._check(config="Release", mingw="1", gcc="1")
        self.assertNotEqual(rc, 0, output)
        self.assertIn("mingw64-gcc.x64.release", output)
        self.assertIn("no usable SDL3 installation prefix", output)

    def test_missing_everything_fails(self):
        _require_cmake(self)
        rc, output = self._check(config="Release")
        self.assertNotEqual(rc, 0, output)
        self.assertIn("no usable SDL3 installation prefix", output)

    def test_non_windows_single_prefix(self):
        _require_cmake(self)
        self.make_prefix("", lib_name="libSDL3.so", lib_subdir="lib64")
        rc, output = self._check(config="Release", windows="0")
        self.assertEqual(rc, 0, output)


class DepsAutoModeTests(_SdlRootMixin, unittest.TestCase):
    """auto 模式：DUILIB_SDL_AUTOBUILD 必须能从函数作用域传到父作用域。

    `duilib_configure_dependencies()` 是函数，函数内的 `set()` 不会传出去；
    子目录里的 duilib_common.cmake 依赖该变量跳过硬校验，否则首次配置
    （SDL3 变体尚未安装）会误报“未找到可用的 SDL3 安装前缀”。
    """

    def test_autobuild_flag_reaches_parent_scope(self):
        _require_cmake(self)
        rc, output = _run_harness(
            "deps_auto",
            DUILIB_TEST_DEPS_ROOT=self.deps_root,
            DUILIB_TEST_SKIA_ROOT=os.path.join(self.tmp, "skia"),
            DUILIB_TEST_SDL_ROOT=self.sdl_root)
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "ON", output)


class RootOptionsSdlWiringTests(unittest.TestCase):
    """静态检查：根工程解析 SDL3 变体前必须自己定义并使用 DUILIB_MINGW。

    `duilib_root_options.cmake` 在 add_subdirectory 之前执行，此时
    `duilib_common.cmake` 里的 DUILIB_MINGW 还不存在；直接引用会把 MinGW 当成
    MSVC，把头文件/库前缀解析成 msvc.<arch>.*（MinGW 构建曾因此指向 MSVC 变体）。
    """

    def test_mingw_flag_defined_before_sdl_variant_calls(self):
        text = (REPO_ROOT / "cmake" / "duilib_root_options.cmake").read_text(
            encoding="utf-8")
        defined_at = text.find("set(DUILIB_MINGW FALSE)")
        self.assertNotEqual(defined_at, -1, "根工程没有定义 DUILIB_MINGW")
        self.assertIn("set(DUILIB_MINGW TRUE)", text, "MinGW 分支没有置位 DUILIB_MINGW")
        call_at = text.find("duilib_sdl_variant_name(")
        self.assertNotEqual(call_at, -1, "根工程没有解析 SDL3 变体前缀")
        self.assertLess(defined_at, call_at, "DUILIB_MINGW 必须在解析变体之前定义")
        self.assertIn('"${DUILIB_MINGW}"', text[call_at:call_at + 400],
                      "解析 SDL3 变体时没有传入 DUILIB_MINGW")


class SdlReadyStampTests(_SdlRootMixin, unittest.TestCase):
    """_duilib_sdl_ready：stamp 与变体绑定，Release 产物不能被视为 Debug 就绪。"""

    def _ready(self, ready_variant, other_variant, override="",
               with_header="1", with_lib="1"):
        return _run_harness(
            "sdl_ready",
            DUILIB_TEST_SDL_ROOT=self.sdl_root,
            DUILIB_TEST_DEPS_ROOT=self.deps_root,
            DUILIB_TEST_SDL_SRC=os.path.join(self.tmp, "SDL"),
            DUILIB_TEST_ARCH="x64",
            DUILIB_TEST_CONFIG="Release",
            DUILIB_TEST_MULTI_CONFIG="1",
            DUILIB_TEST_MINGW="0",
            DUILIB_TEST_MD="0",
            DUILIB_TEST_GCC="0",
            DUILIB_TEST_MSYS2_ENV="",
            DUILIB_TEST_MINGW_NAME="mingw64-",
            DUILIB_TEST_CRT_ID="msvc-mt",
            DUILIB_TEST_READY_VARIANT=ready_variant,
            DUILIB_TEST_OTHER_VARIANT=other_variant,
            DUILIB_TEST_STAMP_OVERRIDE=override,
            DUILIB_TEST_READY_HEADER=with_header,
            DUILIB_TEST_READY_LIB=with_lib)

    def test_release_prefix_not_reused_for_debug(self):
        _require_cmake(self)
        rc, output = self._ready("msvc.x64.release", "msvc.x64.debug")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "TRUE;FALSE", output)

    def test_stale_stamp_is_not_ready(self):
        _require_cmake(self)
        rc, output = self._ready("msvc.x64.release", "msvc.x64.debug",
                                 override="stale|stamp")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "FALSE;FALSE", output)

    def test_missing_library_is_not_ready(self):
        _require_cmake(self)
        rc, output = self._ready("msvc.x64.release", "msvc.x64.debug",
                                 with_lib="0")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "FALSE;FALSE", output)

    def test_missing_header_is_not_ready(self):
        _require_cmake(self)
        rc, output = self._ready("msvc.x64.release", "msvc.x64.debug",
                                 with_header="0")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "FALSE;FALSE", output)


class PrepareSdlConfigTests(_SdlRootMixin, unittest.TestCase):
    """cmake/duilib_deps_run.cmake：多配置下 Debug/Release 安装到各自的变体前缀。"""

    def _prepare(self, config, multi_config, mingw="0", gcc="0", md="0",
                 msys2_env="", mingw_name="mingw64-"):
        return _run_harness(
            "prepare_sdl",
            DUILIB_TEST_SDL_ROOT=self.sdl_root,
            DUILIB_TEST_DEPS_ROOT=self.deps_root,
            DUILIB_TEST_SDL_SRC=os.path.join(self.tmp, "SDL"),
            DUILIB_TEST_CONFIG=config,
            DUILIB_TEST_MULTI_CONFIG=multi_config,
            DUILIB_TEST_ARCH="x64",
            DUILIB_TEST_MINGW=mingw,
            DUILIB_TEST_GCC=gcc,
            DUILIB_TEST_MD=md,
            DUILIB_TEST_MSYS2_ENV=msys2_env,
            DUILIB_TEST_MINGW_NAME=mingw_name,
            DUILIB_TEST_CRT_ID="msvc-md" if md == "1" else "msvc-mt")

    def test_multi_config_installs_two_distinct_prefixes(self):
        _require_cmake(self)
        rc, output = self._prepare("Release", "1")
        self.assertEqual(rc, 0, output)
        variants = _result(output).split(";")
        self.assertEqual(variants, ["msvc.x64.debug", "msvc.x64.release"], output)
        debug_prefix = os.path.join(self.sdl_root, "msvc.x64.debug")
        release_prefix = os.path.join(self.sdl_root, "msvc.x64.release")
        # 两份产物互不覆盖：各自的库与 stamp 都保留
        for prefix, variant in ((debug_prefix, "msvc.x64.debug"),
                                (release_prefix, "msvc.x64.release")):
            self.assertTrue(os.path.isfile(
                os.path.join(prefix, "lib", "SDL3-static.lib")), prefix)
            self.assertTrue(os.path.isfile(
                os.path.join(prefix, "include", "SDL3", "SDL.h")), prefix)
            self.assertIn(variant, self.read_stamp(prefix))
            self.assertTrue(os.path.isfile(os.path.join(prefix, ".duilib-crt")), prefix)
        self.assertIn("multi-config generator", output)
        ready_lines = [line for line in output.splitlines()
                       if "[OK] SDL3 is ready" in line]
        self.assertEqual(len(ready_lines), 2, output)
        self.assertIn("msvc.x64.debug", "\n".join(ready_lines))
        self.assertIn("msvc.x64.release", "\n".join(ready_lines))

    def test_single_config_debug_only_prepares_debug(self):
        _require_cmake(self)
        rc, output = self._prepare("Debug", "0")
        self.assertEqual(rc, 0, output)
        ready_lines = [line for line in output.splitlines()
                       if "[OK] SDL3 is ready" in line]
        self.assertEqual(len(ready_lines), 1, output)
        self.assertIn("msvc.x64.debug", ready_lines[0])
        # 单配置只处理当前配置：Release 前缀不会被“顺手”准备
        self.assertNotIn("msvc.x64.release", "\n".join(ready_lines))

    def test_mingw_multi_config_shares_release_variant(self):
        _require_cmake(self)
        rc, output = self._prepare("Release", "1", mingw="1", gcc="1")
        self.assertEqual(rc, 0, output)
        variants = _result(output).split(";")
        self.assertEqual(variants,
                         ["mingw64-gcc.x64.release", "mingw64-gcc.x64.release"],
                         output)
        # 共用同一前缀：整个 SDL3/ 下只有一个变体目录
        subdirs = sorted(name for name in os.listdir(self.sdl_root)
                         if os.path.isdir(os.path.join(self.sdl_root, name)))
        self.assertEqual(subdirs, ["mingw64-gcc.x64.release"], output)
        self.assertTrue(os.path.isfile(os.path.join(
            self.sdl_root, "mingw64-gcc.x64.release", "lib", "libSDL3.a")))


class SdlReadyCallSiteTests(unittest.TestCase):
    """静态检查：`_duilib_sdl_ready` 的每个调用点都必须传满 3 个参数。

    该函数签名是 (prefix variant out_ready)，漏传变体在运行期才报
    “invoked with incorrect arguments”；这里提前在源码层拦住。
    """

    ARITY = 3
    FUNCTION = "_duilib_sdl_ready("

    def test_all_call_sites_pass_variant(self):
        files = [REPO_ROOT / "cmake" / "duilib_deps_run.cmake",
                 HARNESS]
        problems = []
        for path in files:
            text = path.read_text(encoding="utf-8")
            for lineno, line in enumerate(text.splitlines(), 1):
                if self.FUNCTION not in line:
                    continue
                if line.lstrip().startswith("function("):
                    continue
                inner = line.split(self.FUNCTION, 1)[1]
                if not inner.rstrip().endswith(")"):
                    problems.append("%s:%d 调用没有写在同一行，无法静态校验"
                                    % (path.name, lineno))
                    continue
                args = inner.rstrip()[:-1].split()
                if len(args) != self.ARITY:
                    problems.append("%s:%d 只有 %d 个参数（应为 %d）"
                                    % (path.name, lineno, len(args), self.ARITY))
        self.assertEqual(problems, [], "\n".join(problems))


class PrepareSdlBuildPathTests(_SdlRootMixin, unittest.TestCase):
    """安装后的再次校验：构建成功路径同样必须按变体判定就绪。

    回归：`_duilib_sdl_ready` 曾在“写 stamp 之后”的调用点漏传变体参数，
    导致安装成功仍报失败；`prepare_sdl_build` 夹具不预置假前缀，会走到该路径。
    """

    def _prepare_build(self, config="Release", multi_config="1", mingw="0",
                       gcc="0", md="0"):
        return _run_harness(
            "prepare_sdl_build",
            DUILIB_TEST_SDL_ROOT=self.sdl_root,
            DUILIB_TEST_DEPS_ROOT=self.deps_root,
            DUILIB_TEST_SDL_SRC=os.path.join(self.tmp, "SDL"),
            DUILIB_TEST_CONFIG=config,
            DUILIB_TEST_MULTI_CONFIG=multi_config,
            DUILIB_TEST_ARCH="x64",
            DUILIB_TEST_MINGW=mingw,
            DUILIB_TEST_GCC=gcc,
            DUILIB_TEST_MD=md,
            DUILIB_TEST_MSYS2_ENV="",
            DUILIB_TEST_MINGW_NAME="mingw64-",
            DUILIB_TEST_CRT_ID="msvc-md" if md == "1" else "msvc-mt")

    def test_post_install_check_uses_variant(self):
        _require_cmake(self)
        rc, output = self._prepare_build()
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output).split(";"),
                         ["msvc.x64.debug", "msvc.x64.release"], output)
        ready_lines = [line for line in output.splitlines()
                       if "[OK] SDL3 is ready" in line]
        self.assertEqual(len(ready_lines), 2, output)
        for variant, config in (("msvc.x64.debug", "Debug"),
                                ("msvc.x64.release", "Release")):
            prefix = os.path.join(self.sdl_root, variant)
            self.assertTrue(os.path.isfile(
                os.path.join(prefix, "include", "SDL3", "SDL.h")), prefix)
            self.assertTrue(os.path.isfile(
                os.path.join(prefix, "lib", "SDL3-static.lib")), prefix)
            self.assertIn(variant, self.read_stamp(prefix))
            self.assertIn(variant, "\n".join(ready_lines))
            self.assertIn("fake SDL3 install (%s)" % config, output)

    def test_single_config_build_installs_only_that_variant(self):
        _require_cmake(self)
        rc, output = self._prepare_build(config="Debug", multi_config="0")
        self.assertEqual(rc, 0, output)
        prefix = os.path.join(self.sdl_root, "msvc.x64.debug")
        ready_lines = [line for line in output.splitlines()
                       if "[OK] SDL3 is ready" in line]
        self.assertEqual(len(ready_lines), 1, output)
        self.assertIn("msvc.x64.debug", ready_lines[0])
        self.assertFalse(os.path.isdir(os.path.join(self.sdl_root, "msvc.x64.release")),
                         output)
        self.assertIn("msvc.x64.debug", self.read_stamp(prefix))


class _SkiaRootMixin:
    """在临时目录里造 skia/out/<variant> 假产物。"""

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix="duilib_skia_test_")
        self.skia_root = os.path.join(self.tmp, "skia")
        os.makedirs(os.path.join(self.skia_root, "out"), exist_ok=True)

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def make_variant(self, name, lib_name="skia.lib"):
        path = os.path.join(self.skia_root, "out", name)
        os.makedirs(path, exist_ok=True)
        with open(os.path.join(path, lib_name), "wb") as fp:
            fp.write(b"")
        return path


class DependenciesOffModeTests(_SkiaRootMixin, unittest.TestCase):
    """cmake/duilib_deps.cmake 的 off 模式：逐配置检查 Skia。"""

    def _check(self, config=None, config_types=None, mingw="0", gcc="0",
               msys2_env="", mingw_name="mingw64-", windows="1", arch="x64", md="0"):
        return _run_harness(
            "deps_check",
            DUILIB_TEST_SKIA_ROOT=self.skia_root,
            DUILIB_TEST_CONFIG=config or "Release",
            DUILIB_TEST_CONFIGURATION_TYPES=config_types or "",
            DUILIB_TEST_WINDOWS=windows,
            DUILIB_TEST_ARCH=arch,
            DUILIB_TEST_MINGW=mingw,
            DUILIB_TEST_GCC=gcc,
            DUILIB_TEST_MD=md,
            DUILIB_TEST_MSYS2_ENV=msys2_env,
            DUILIB_TEST_MINGW_NAME=mingw_name)

    def test_single_config_release_ok(self):
        _require_cmake(self)
        self.make_variant("llvm.x64.release")
        rc, output = self._check(config="Release")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "OK")

    def test_single_config_debug_accepts_debug_variant(self):
        """回归：只有 Debug 变体时不能再报 'no usable Skia library directory'。"""
        _require_cmake(self)
        self.make_variant("llvm.x64.debug")
        rc, output = self._check(config="Debug")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "OK")

    def test_single_config_debug_requires_debug_variant(self):
        _require_cmake(self)
        self.make_variant("llvm.x64.release")
        rc, output = self._check(config="Debug")
        self.assertNotEqual(rc, 0, output)
        self.assertIn("no usable Skia library directory", output)
        self.assertIn("llvm.x64.debug", output)

    def test_multi_config_checks_each_config(self):
        _require_cmake(self)
        self.make_variant("llvm.x64.debug")
        self.make_variant("llvm.x64.release")
        rc, output = self._check(config_types="Debug;Release")
        self.assertEqual(rc, 0, output)

    def test_multi_config_missing_debug_reports_that_config(self):
        _require_cmake(self)
        self.make_variant("llvm.x64.release")
        rc, output = self._check(config_types="Debug;Release")
        self.assertNotEqual(rc, 0, output)
        self.assertIn("debug:", output)
        self.assertIn("llvm.x64.debug", output)

    def test_mingw_requires_a_static_library(self):
        """MinGW 只接受包含 .a 的目录：.lib 目录跳过并回退到 release 变体。"""
        _require_cmake(self)
        self.make_variant("mingw64-gcc.x64.debug", lib_name="skia.lib")
        self.make_variant("mingw64-gcc.x64.release", lib_name="libskia.a")
        rc, output = self._check(config="Debug", mingw="1", gcc="1")
        self.assertEqual(rc, 0, output)

    def test_mingw_reports_lib_only_directory(self):
        _require_cmake(self)
        self.make_variant("mingw64-gcc.x64.debug", lib_name="skia.lib")
        rc, output = self._check(config="Debug", mingw="1", gcc="1")
        self.assertNotEqual(rc, 0, output)
        self.assertIn("缺少 .a 静态库", output)

    def test_non_windows_fallback_to_gcc(self):
        _require_cmake(self)
        self.make_variant("gcc.x64.release", lib_name="libskia.a")
        rc, output = self._check(config="Release", windows="0")
        self.assertEqual(rc, 0, output)


class PrepareSkiaConfigTests(_SkiaRootMixin, unittest.TestCase):
    """cmake/duilib_deps_run.cmake：多配置生成器下 Debug/Release 两份都要准备。"""

    def _prepare(self, config, multi_config, mingw="0", gcc="0", md="0",
                 msys2_env="", mingw_name="mingw64-"):
        deps_root = os.path.join(self.tmp, "deps")
        os.makedirs(deps_root, exist_ok=True)
        return _run_harness(
            "prepare_skia",
            DUILIB_TEST_SKIA_ROOT=self.skia_root,
            DUILIB_TEST_DEPS_ROOT=deps_root,
            DUILIB_TEST_CONFIG=config,
            DUILIB_TEST_MULTI_CONFIG=multi_config,
            DUILIB_TEST_ARCH="x64",
            DUILIB_TEST_MINGW=mingw,
            DUILIB_TEST_GCC=gcc,
            DUILIB_TEST_MD=md,
            DUILIB_TEST_MSYS2_ENV=msys2_env,
            DUILIB_TEST_MINGW_NAME=mingw_name,
            DUILIB_TEST_CRT_ID="msvc-md" if md == "1" else "msvc-mt")

    def test_multi_config_prepares_debug_and_release(self):
        _require_cmake(self)
        rc, output = self._prepare("Release", "1")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "OK")
        # 两份变体都被“采纳”，并给出多配置提示
        self.assertIn("llvm.x64.debug", output)
        self.assertIn("llvm.x64.release", output)
        self.assertIn("preparing Skia for both Debug and Release", output)

    def test_single_config_debug_only(self):
        _require_cmake(self)
        rc, output = self._prepare("Debug", "0")
        self.assertEqual(rc, 0, output)
        self.assertIn("llvm.x64.debug", output)
        # 不允许顺手准备 Release（单配置只处理当前配置）
        ready_lines = [line for line in output.splitlines() if "(llvm.x64." in line]
        self.assertTrue(ready_lines)
        self.assertNotIn("(llvm.x64.release)", "\n".join(ready_lines))

    def test_single_config_release_only(self):
        _require_cmake(self)
        rc, output = self._prepare("Release", "0")
        self.assertEqual(rc, 0, output)
        self.assertIn("llvm.x64.release", output)
        self.assertNotIn("(llvm.x64.debug)", output)


class FindCmakeTests(unittest.TestCase):
    """build.py find_cmake()：测试/构建不再要求 PATH 上一定有 cmake。"""

    def test_explicit_override_wins(self):
        tmp = tempfile.mkdtemp(prefix="duilib_find_cmake_")
        try:
            fake = os.path.join(tmp, "cmake-override.exe")
            with open(fake, "wb") as fp:
                fp.write(b"")
            env = {"DUILIB_CMAKE_EXECUTABLE": fake,
                   "PATH": os.path.join(tmp, "empty")}
            self.assertEqual(entry.find_cmake(env=env, vs_install=Path(tmp)), fake)
        finally:
            shutil.rmtree(tmp, ignore_errors=True)

    def test_falls_back_to_path(self):
        tmp = tempfile.mkdtemp(prefix="duilib_find_cmake_")
        try:
            name = "cmake.exe" if sys.platform == "win32" else "cmake"
            fake = os.path.join(tmp, name)
            with open(fake, "wb") as fp:
                fp.write(b"")
            if sys.platform != "win32":
                os.chmod(fake, 0o755)
            env = {"PATH": tmp}
            found = entry.find_cmake(env=env, vs_install=Path(tmp))
            # Windows 上 shutil.which 返回的扩展名大小写可能不同（.EXE / .exe）
            self.assertEqual(os.path.normcase(found), os.path.normcase(fake))
        finally:
            shutil.rmtree(tmp, ignore_errors=True)

    def test_returns_none_when_nothing_found(self):
        tmp = tempfile.mkdtemp(prefix="duilib_find_cmake_")
        try:
            env = {"DUILIB_CMAKE_EXECUTABLE": "", "PATH": os.path.join(tmp, "empty")}
            # 显式传入不存在的 VS 安装目录，避免落到 vswhere 找到真实 cmake
            self.assertIsNone(entry.find_cmake(env=env, vs_install=Path(tmp)))
        finally:
            shutil.rmtree(tmp, ignore_errors=True)


class CommonCandidateTests(_SkiaRootMixin, unittest.TestCase):
    """cmake/duilib_common.cmake 的作用域映射与 autobuild 路径解析。"""

    def _common(self, **defines):
        defines.setdefault("DUILIB_TEST_CRT_DIR", "")
        return _run_harness(
            "common",
            DUILIB_TEST_SKIA_ROOT=self.skia_root,
            DUILIB_TEST_ARCH="x64",
            DUILIB_TEST_AUTOBUILD="1",
            **defines)

    def test_msvc_release_subpath(self):
        _require_cmake(self)
        rc, output = self._common(
            DUILIB_TEST_WINDOWS="1", DUILIB_TEST_MSVC="1", DUILIB_TEST_MINGW="0",
            DUILIB_TEST_MD="0", DUILIB_TEST_COMPILER_ID="MSVC",
            DUILIB_TEST_CONFIG="Release", DUILIB_TEST_CONFIGURATION_TYPES="")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "llvm.x64.release")

    def test_multi_config_generator_expression(self):
        _require_cmake(self)
        rc, output = self._common(
            DUILIB_TEST_WINDOWS="1", DUILIB_TEST_MSVC="1", DUILIB_TEST_MINGW="0",
            DUILIB_TEST_MD="0", DUILIB_TEST_COMPILER_ID="MSVC",
            DUILIB_TEST_CONFIG="", DUILIB_TEST_CONFIGURATION_TYPES="Debug;Release")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output),
                         "$<IF:$<CONFIG:Debug>,llvm.x64.debug,llvm.x64.release>")

    def test_msys2_ucrt64_debug_subpath(self):
        """MSYS2 检测（编译器位于 <root>/<env>/bin 且存在 usr/bin/pacman.exe）。"""
        _require_cmake(self)
        msys_root = os.path.join(self.tmp, "msys2")
        os.makedirs(os.path.join(msys_root, "usr", "bin"), exist_ok=True)
        os.makedirs(os.path.join(msys_root, "ucrt64", "bin"), exist_ok=True)
        with open(os.path.join(msys_root, "usr", "bin", "pacman.exe"), "wb") as fp:
            fp.write(b"")
        compiler = os.path.join(msys_root, "ucrt64", "bin", "g++.exe")
        with open(compiler, "wb") as fp:
            fp.write(b"")
        rc, output = self._common(
            DUILIB_TEST_WINDOWS="1", DUILIB_TEST_MSVC="0", DUILIB_TEST_MINGW="1",
            DUILIB_TEST_MD="0", DUILIB_TEST_COMPILER_ID="GNU",
            DUILIB_TEST_CXX_COMPILER=compiler,
            DUILIB_TEST_CONFIG="Debug", DUILIB_TEST_CONFIGURATION_TYPES="")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "gcc.x64.debug")

    def _common_with_crt_stamp(self, stamp):
        stamp_dir = os.path.join(self.tmp, "dep")
        os.makedirs(stamp_dir, exist_ok=True)
        with open(os.path.join(stamp_dir, ".duilib-crt"), "w", encoding="utf-8") as fp:
            fp.write(stamp)
        return self._common(
            DUILIB_TEST_WINDOWS="1", DUILIB_TEST_MSVC="1", DUILIB_TEST_MINGW="0",
            DUILIB_TEST_MD="0", DUILIB_TEST_COMPILER_ID="MSVC",
            DUILIB_TEST_CONFIG="Release", DUILIB_TEST_CONFIGURATION_TYPES="",
            DUILIB_TEST_CRT_DIR=stamp_dir)

    def test_crt_stamp_match(self):
        _require_cmake(self)
        rc, output = self._common_with_crt_stamp("msvc-mt")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "llvm.x64.release")

    def test_crt_stamp_mismatch(self):
        """依赖目录的 .duilib-crt 与当前工具链不一致时必须报错（共用 duilib_crt_id）。"""
        _require_cmake(self)
        rc, output = self._common_with_crt_stamp("msvc-md")
        self.assertNotEqual(rc, 0, output)
        self.assertIn("CRT mismatch", output)


class CrtIdTests(unittest.TestCase):
    """cmake/duilib_crt.cmake 是 CRT 标识的唯一推导点。"""

    def _crt(self, msvc="0", mingw="0", md="0", msystem=None, compiler=None):
        process_env = dict(os.environ)
        process_env.pop("MSYSTEM", None)
        if msystem is not None:
            process_env["MSYSTEM"] = msystem
        return _run_harness(
            "crt", process_env=process_env,
            DUILIB_TEST_MSVC=msvc, DUILIB_TEST_MINGW=mingw, DUILIB_TEST_MD=md,
            DUILIB_TEST_CXX_COMPILER=compiler)

    def _toolchain_tree(self, env_name):
        """伪造 MSYS2 安装树，返回 <root>/<env>/bin/g++.exe；env_name 为空时返回独立 MinGW。"""
        root = tempfile.mkdtemp(prefix="duilib_crt_")
        self.addCleanup(shutil.rmtree, root, True)
        env_dir = os.path.join(root, env_name) if env_name else os.path.join(root, "mingw64")
        os.makedirs(os.path.join(env_dir, "bin"), exist_ok=True)
        if env_name:
            os.makedirs(os.path.join(root, "usr", "bin"), exist_ok=True)
            with open(os.path.join(root, "usr", "bin", "pacman.exe"), "wb") as fp:
                fp.write(b"")
        compiler = os.path.join(env_dir, "bin", "g++.exe")
        with open(compiler, "wb") as fp:
            fp.write(b"")
        return compiler

    def test_msvc(self):
        _require_cmake(self)
        rc, output = self._crt(msvc="1", md="1")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "msvc-md")
        rc, output = self._crt(msvc="1", md="0")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "msvc-mt")

    def test_mingw(self):
        """没有编译器路径时（脱离配置直接调用）才回退 MSYSTEM。"""
        _require_cmake(self)
        rc, output = self._crt(mingw="1", msystem="MINGW64")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "mingw64-msvcrt")
        rc, output = self._crt(mingw="1", msystem="UCRT64")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "ucrt64")
        rc, output = self._crt(mingw="1")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "mingw-unknown")

    def test_mingw_compiler_location_wins_over_msystem(self):
        """有编译器路径时以安装位置为准：同一构建在 cmd / Git Bash / MSYS2 下结果一致。"""
        _require_cmake(self)
        # Git Bash 也设置 MSYSTEM=MINGW64，但编译器不在 MSYS2 里 → 独立 MinGW-w64
        compiler = self._toolchain_tree("")
        rc, output = self._crt(mingw="1", msystem="MINGW64", compiler=compiler)
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "mingw-unknown")
        # MSYSTEM 未设置（从 cmd 配置），编译器位于 <root>/ucrt64/bin → ucrt64
        compiler = self._toolchain_tree("ucrt64")
        rc, output = self._crt(mingw="1", compiler=compiler)
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "ucrt64")
        # MSYS2 MINGW64 目录：msvcrt 需要单独标识
        compiler = self._toolchain_tree("mingw64")
        rc, output = self._crt(mingw="1", compiler=compiler)
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "mingw64-msvcrt")
        # 编译器在 MSYS2 里，但 MSYSTEM 指向别的环境：仍以编译器为准
        compiler = self._toolchain_tree("clang64")
        rc, output = self._crt(mingw="1", msystem="MINGW64", compiler=compiler)
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "clang64")

    def test_other(self):
        _require_cmake(self)
        rc, output = self._crt()
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "system")


class ConfigureSmokeTests(unittest.TestCase):
    """真实 configure 冒烟：prepare 模式不依赖 Skia，可验证 CMake 改动能否被完整解析。"""

    def test_configure_prepare_mode(self):
        cmake = shutil.which("cmake") or entry.find_cmake()
        if not cmake:
            self.skipTest("cmake not found (PATH / DUILIB_CMAKE_EXECUTABLE / Visual Studio)")
        tmp = tempfile.mkdtemp(prefix="duilib_configure_")
        try:
            build_dir = os.path.join(tmp, "build")
            deps_root = os.path.join(tmp, "deps")
            os.makedirs(deps_root, exist_ok=True)
            command = [
                cmake, "-S", str(REPO_ROOT), "-B", build_dir,
                "-DDUILIB_DEPS_MODE=prepare",
                "-DDUILIB_DEPS_ROOT=" + deps_root,
                "-DDUILIB_BUILD_EXAMPLES=OFF",
                "-DDUILIB_ENABLE_SDL=OFF",
                "-DDUILIB_ENABLE_CEF=OFF",
                "-DDUILIB_LANG=en",
                "-DDUILIB_OFFLINE=ON",
            ]
            result = subprocess.run(
                command, cwd=str(REPO_ROOT), stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT, text=True, encoding="utf-8",
                errors="replace", timeout=300)
            output = result.stdout
            if result.returncode != 0:
                environment_missing = (
                    "No CMAKE_C_COMPILER could be found",
                    "No CMAKE_CXX_COMPILER could be found",
                    "could not find any instance of Visual Studio",
                    "CMAKE_CXX_COMPILER not set",
                )
                if any(marker in output for marker in environment_missing):
                    self.skipTest("no usable compiler/generator in this environment")
                self.fail("configure failed (rc=%d):\n%s" % (result.returncode, output))
            self.assertTrue(os.path.isfile(os.path.join(build_dir, "CMakeCache.txt")))
            with open(os.path.join(build_dir, "CMakeCache.txt"), "r",
                      encoding="utf-8", errors="replace") as fp:
                cache = fp.read()
            self.assertIn("DUILIB_DEPS_MODE:STRING=prepare", cache)
        finally:
            shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    unittest.main(verbosity=2)
