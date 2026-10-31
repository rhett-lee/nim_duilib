"""构建系统 CMake 脚本的自动化测试（标准库 unittest）。

运行：
  python3 build/build.py test
  python3 -m unittest build/test_cmake_scripts.py -v

覆盖 build/tests/skia_variants_check.cmake 驱动的三类逻辑（它们此前只能靠手工
在 Windows 上跑 build.py 验证，因此单配置生成器的 bug“Debug 被当成 Release”
一直没有被测到）：

1. cmake/duilib_skia_variants.cmake：Skia 变体目录名规则（MSVC Debug/Release
   × /MT、/MD，MinGW 独立版/MSYS2 × gcc、llvm，非 Windows）。
2. cmake/duilib_deps.cmake 的 DUILIB_DEPS_MODE=off 依赖检查：单配置/多配置
   生成器、Debug 变体缺失、MinGW 拒绝 MSVC 产物。
3. cmake/duilib_deps_run.cmake 的 duilib_prepare_skia：多配置生成器必须同时
   准备 Debug/Release 两个变体（否则 VS 的 Debug 配置阶段报缺 Skia 目录）。
"""

from __future__ import annotations

import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

BUILD_DIR = Path(__file__).resolve().parent
REPO_ROOT = BUILD_DIR.parent
HARNESS = BUILD_DIR / "tests" / "skia_variants_check.cmake"
CMAKE = shutil.which("cmake")

# 候选目录的期望值（顺序即优先级）
EXPECTED_CANDIDATES = {
    "msvc-debug-mt": ["llvm.x64.debug"],
    "msvc-debug-md": ["llvm.x64.debug.mdd"],
    "msvc-release-mt": ["llvm.x64.release"],
    "msvc-release-md": ["llvm.x64.release.md"],
    "msvc-debug-x86": ["llvm.x86.debug"],
    "msvc-unknown-config": ["llvm.x64.release"],
    "mingw-gcc-debug": ["mingw64-gcc.x64.debug", "mingw64-gcc.x64.release",
                        "gcc.x64.debug", "gcc.x64.release"],
    "mingw-llvm-release": ["mingw64-llvm.x64.release", "llvm.x64.release"],
    "mingw-llvm-ucrt64-release": ["llvm.x64.release", "mingw64-llvm.x64.release"],
    "mingw-gcc-mingw64-debug": ["gcc.msvcrt.x64.release", "gcc.x64.debug",
                                "gcc.x64.release", "mingw64-gcc.x64.debug",
                                "mingw64-gcc.x64.release"],
}


def _run(arguments: list[str]) -> subprocess.CompletedProcess:
    return subprocess.run([CMAKE, *arguments], cwd=str(REPO_ROOT),
                          capture_output=True, text=True, errors="replace", timeout=120)


@unittest.skipUnless(CMAKE, "cmake is not available")
class SkiaVariantCandidateTests(unittest.TestCase):
    """cmake/duilib_skia_variants.cmake 的规则（一次 cmake -P 跑完整矩阵）。"""

    @classmethod
    def setUpClass(cls):
        completed = _run([f"-DDUILIB_REPO_ROOT={REPO_ROOT}",
                          "-DDUILIB_TEST_MODE=candidates", "-P", str(HARNESS)])
        if completed.returncode != 0:
            raise AssertionError(completed.stdout + completed.stderr)
        cls.cases = {}
        for line in (completed.stdout + completed.stderr).splitlines():
            if "CASE|" not in line:
                continue
            _, name, values = line.strip().split("|", 2)
            cls.cases[name] = values.split(";") if values else []

    def test_all_cases_reported(self):
        expected = set(EXPECTED_CANDIDATES) | {"unix-release"}
        self.assertEqual(expected, set(self.cases))

    def test_msvc_variants(self):
        for name in ("msvc-debug-mt", "msvc-debug-md", "msvc-release-mt",
                     "msvc-release-md", "msvc-debug-x86", "msvc-unknown-config"):
            with self.subTest(case=name):
                self.assertEqual(EXPECTED_CANDIDATES[name], self.cases[name])

    def test_mingw_variants(self):
        for name in ("mingw-gcc-debug", "mingw-llvm-release",
                     "mingw-llvm-ucrt64-release", "mingw-gcc-mingw64-debug"):
            with self.subTest(case=name):
                self.assertEqual(EXPECTED_CANDIDATES[name], self.cases[name])

    def test_unix_only_release_variants(self):
        values = self.cases["unix-release"]
        self.assertIn(len(values), (1, 2), values)
        # 最后一定是 gcc 回退（clang 版优先），且没有任何 debug 变体
        self.assertEqual("gcc.x64.release", values[-1])
        for value in values:
            self.assertTrue(value.endswith(".release"), value)
            self.assertNotIn(".debug", value)


@unittest.skipUnless(CMAKE, "cmake is not available")
class DependenciesOffModeTests(unittest.TestCase):
    """cmake/duilib_deps.cmake 的 off 模式依赖检查（每个用例一次 cmake -P）。"""

    def _check(self, case: str, variants: dict) -> subprocess.CompletedProcess:
        """variants 形如 {"llvm.x64.debug": None, "mingw64-gcc.x64.release": "libskia.a"}。"""
        with tempfile.TemporaryDirectory(prefix="duilib_skia_check_") as temp:
            skia_root = Path(temp) / "skia"
            out_dir = skia_root / "out"
            out_dir.mkdir(parents=True)
            for name, library in variants.items():
                variant_dir = out_dir / name
                variant_dir.mkdir()
                if library:
                    (variant_dir / library).write_text("", encoding="utf-8")
            return _run([f"-DDUILIB_REPO_ROOT={REPO_ROOT}",
                         "-DDUILIB_TEST_MODE=deps_check",
                         f"-DDUILIB_TEST_CASE={case}",
                         f"-DDUILIB_TEST_SKIA_ROOT={skia_root}",
                         "-P", str(HARNESS)])

    def _assert_ok(self, completed: subprocess.CompletedProcess):
        output = completed.stdout + completed.stderr
        self.assertEqual(completed.returncode, 0, output)
        self.assertIn("RESULT|ok", output)

    def _assert_missing(self, completed: subprocess.CompletedProcess, *needles: str):
        output = completed.stdout + completed.stderr
        self.assertNotEqual(completed.returncode, 0, "expected the check to fail")
        self.assertIn("no usable Skia library directory", output)
        for needle in needles:
            self.assertIn(needle, output)

    def test_msvc_debug_accepts_debug_variant(self):
        # 回归用例：旧实现把 Debug 配置当成 Release，只有 debug 变体时会误报
        self._assert_ok(self._check("msvc-debug-default-rt", {"llvm.x64.debug": None}))

    def test_msvc_debug_md_accepts_mdd_variant(self):
        self._assert_ok(self._check("msvc-debug-md", {"llvm.x64.debug.mdd": None}))

    def test_msvc_debug_rejects_release_variant(self):
        self._assert_missing(self._check("msvc-debug-missing", {"llvm.x64.release": None}),
                             "llvm.x64.debug")

    def test_msvc_release_accepts_release_variant(self):
        self._assert_ok(self._check("msvc-release", {"llvm.x64.release": None}))

    def test_mingw_debug_falls_back_to_release(self):
        self._assert_ok(self._check("mingw-debug-fallback",
                                    {"mingw64-gcc.x64.release": "libskia.a"}))

    def test_mingw_rejects_msvc_variant(self):
        # llvm.x64.debug 与 MSVC 产物同名，但没有 .a，不能用于 MinGW 构建
        self._assert_missing(self._check("mingw-rejects-msvc-dir",
                                         {"llvm.x64.debug": "skia.lib"}),
                             "llvm.x64.debug")

    def test_multi_config_requires_both_variants(self):
        self._assert_ok(self._check("multi-config-both", {"llvm.x64.debug": None,
                                                          "llvm.x64.release": None}))

    def test_multi_config_reports_missing_debug_variant(self):
        self._assert_missing(self._check("multi-config-missing-debug",
                                         {"llvm.x64.release": None}),
                             "debug", "llvm.x64.debug")

    def test_unix_accepts_gcc_fallback(self):
        self._assert_ok(self._check("unix-gcc-fallback", {"gcc.x64.release": "libskia.a"}))


@unittest.skipUnless(CMAKE, "cmake is not available")
class PrepareSkiaConfigTests(unittest.TestCase):
    """duilib_prepare_skia 在“依赖已就绪”时选择了哪些 Skia 变体。

    假 Skia 目录里放好库文件与匹配的 .duilib-ref，让准备函数直接走“已就绪”
    分支，不触发真正的 gn/ninja 编译，从而可以只测配置选择逻辑。
    """

    # 与 build/tests/skia_variants_check.cmake 里 prepare_skia 模式的 -D 值一致
    STAMP = "test-ref|test-value|msvc-mt|x64"

    def _run_case(self, case: str, variants: list[str]) -> subprocess.CompletedProcess:
        with tempfile.TemporaryDirectory(prefix="duilib_skia_prepare_") as temp:
            root = Path(temp)
            skia_out = root / "skia" / "out"
            skia_out.mkdir(parents=True)
            for name in variants:
                variant_dir = skia_out / name
                variant_dir.mkdir()
                (variant_dir / "skia.lib").write_text("", encoding="utf-8")
                # 注意：不能带 BOM，_duilib_prepare_skia_config 是逐字比较 stamp
                (variant_dir / ".duilib-ref").write_text(self.STAMP, encoding="utf-8")
            return _run([f"-DDUILIB_REPO_ROOT={REPO_ROOT}",
                         "-DDUILIB_TEST_MODE=prepare_skia",
                         f"-DDUILIB_TEST_CASE={case}",
                         f"-DDUILIB_TEST_SKIA_ROOT={root / 'skia'}",
                         f"-DDUILIB_TEST_DEPS_ROOT={root / 'deps'}",
                         "-P", str(HARNESS)])

    def _variants_in(self, completed: subprocess.CompletedProcess, *needles: str):
        return all(needle in completed.stdout + completed.stderr for needle in needles)

    def test_multi_config_prepares_debug_and_release(self):
        # 回归用例：旧实现只准备 Release，VS 的 Debug 配置会在配置期失败
        completed = self._run_case("multi-config", ["llvm.x64.debug", "llvm.x64.release"])
        output = completed.stdout + completed.stderr
        self.assertEqual(completed.returncode, 0, output)
        self.assertIn("RESULT|ok", output)
        self.assertTrue(self._variants_in(completed, "(llvm.x64.debug)"), output)
        self.assertTrue(self._variants_in(completed, "(llvm.x64.release)"), output)

    def test_multi_config_does_not_silently_accept_missing_debug(self):
        # 只有 Release 变体时必须失败（继续准备 Debug 会因假目录不是 git checkout 而报错）
        completed = self._run_case("multi-config", ["llvm.x64.release"])
        output = completed.stdout + completed.stderr
        self.assertNotEqual(completed.returncode, 0, output)
        self.assertNotIn("(llvm.x64.debug)", output)
        self.assertNotIn("RESULT|ok", output)

    def test_single_config_debug_only_prepares_debug(self):
        completed = self._run_case("single-config-debug", ["llvm.x64.debug"])
        output = completed.stdout + completed.stderr
        self.assertEqual(completed.returncode, 0, output)
        self.assertTrue(self._variants_in(completed, "(llvm.x64.debug)"), output)
        self.assertNotIn("(llvm.x64.release)", output)

    def test_single_config_release_only_prepares_release(self):
        completed = self._run_case("single-config-release", ["llvm.x64.release"])
        output = completed.stdout + completed.stderr
        self.assertEqual(completed.returncode, 0, output)
        self.assertTrue(self._variants_in(completed, "(llvm.x64.release)"), output)
        self.assertNotIn("(llvm.x64.debug)", output)


if __name__ == "__main__":
    unittest.main()
