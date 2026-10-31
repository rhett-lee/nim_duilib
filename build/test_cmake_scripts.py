# -*- coding: utf-8 -*-
"""构建系统 CMake 侧脚本的回归测试。

运行：
  python3 build/build.py test
  python3 -m unittest build.test_cmake_scripts -v

覆盖：
  * cmake/duilib_skia_variants.cmake 的候选目录名（并与 build/pack.py 打包侧比对，
    防止两侧再次漂移）；
  * cmake/duilib_deps.cmake 的 off 模式逐配置检查（含 MinGW 只认 .a）；
  * cmake/duilib_deps_run.cmake 的多配置 Skia 准备（Debug/Release 各一份）；
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
            dll_mode=case["md"] == "1",
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
                self.assertEqual(cmake_names, pack_names,
                                 "CMake 与 pack.py 的 Skia 候选目录名不一致（%s）:\n"
                                 "  cmake: %s\n  pack : %s"
                                 % (case["name"], cmake_names, pack_names))

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

    def _crt(self, msvc="0", mingw="0", md="0", msystem=None):
        process_env = dict(os.environ)
        process_env.pop("MSYSTEM", None)
        if msystem is not None:
            process_env["MSYSTEM"] = msystem
        return _run_harness(
            "crt", process_env=process_env,
            DUILIB_TEST_MSVC=msvc, DUILIB_TEST_MINGW=mingw, DUILIB_TEST_MD=md)

    def test_msvc(self):
        _require_cmake(self)
        rc, output = self._crt(msvc="1", md="1")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "msvc-md")
        rc, output = self._crt(msvc="1", md="0")
        self.assertEqual(rc, 0, output)
        self.assertEqual(_result(output), "msvc-mt")

    def test_mingw(self):
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
