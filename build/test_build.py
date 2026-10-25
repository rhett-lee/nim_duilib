"""build/build.py 与 build/i18n.py 的单元测试（标准库 unittest）。

运行：
  python build/build.py test
  python -m unittest discover -s build -p test_*.py -v
"""

from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path

BUILD_DIR = Path(__file__).resolve().parent
REPO_ROOT = BUILD_DIR.parent
if str(BUILD_DIR) not in sys.path:
    sys.path.insert(0, str(BUILD_DIR))

from i18n import Messages, resolve_language  # noqa: E402

# 按路径加载 build.py：仓库根目录下同名目录会被当作命名空间包，不能直接 import build。
_spec = importlib.util.spec_from_file_location("duilib_build_entry", BUILD_DIR / "build.py")
entry = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(entry)


class ArchTests(unittest.TestCase):
    def test_normalize_arch_aliases(self):
        self.assertEqual(entry.normalize_arch("AMD64"), "x64")
        self.assertEqual(entry.normalize_arch("x86_64"), "x64")
        self.assertEqual(entry.normalize_arch("x64"), "x64")
        self.assertEqual(entry.normalize_arch("i686"), "x86")
        self.assertEqual(entry.normalize_arch("Win32"), "x86")
        self.assertEqual(entry.normalize_arch("aarch64"), "arm64")
        self.assertEqual(entry.normalize_arch(" arm64 "), "arm64")

    def test_normalize_arch_unknown_passthrough(self):
        self.assertEqual(entry.normalize_arch("mips"), "mips")
        self.assertEqual(entry.normalize_arch(""), "")

    def test_resolve_arch(self):
        for requested in ("", "auto"):
            self.assertIn(entry.resolve_arch(requested), ("x86", "x64", "arm64"))
        self.assertEqual(entry.resolve_arch("x86"), "x86")

    def test_host_arch_is_supported(self):
        self.assertIn(entry.host_arch(), ("x86", "x64", "arm64"))


class LayoutTests(unittest.TestCase):
    def test_preset_name(self):
        self.assertEqual(entry.preset_name("x64", "Debug"), "x64-debug")
        self.assertEqual(entry.preset_name("arm64", "Release"), "arm64-release")

    def test_preset_build_dir(self):
        self.assertEqual(entry.preset_build_dir(Path("C:/repo"), "x64", "Debug"),
                         Path("C:/repo/out/x64/Debug"))

    def test_resolve_build_dir_default(self):
        self.assertEqual(entry.resolve_build_dir(Path("C:/repo"), "x64", "Release"),
                         Path("C:/repo/out/x64/Release"))

    def test_resolve_build_dir_override_absolute(self):
        override = Path("C:/tmp/custom")
        self.assertEqual(entry.resolve_build_dir(Path("C:/repo"), "x64", "Release", str(override)),
                         override.resolve())

    def test_resolve_build_dir_override_relative(self):
        resolved = entry.resolve_build_dir(Path("C:/repo"), "x64", "Release", "rel/out")
        self.assertEqual(resolved, (Path.cwd() / "rel/out").resolve())


class ManifestTests(unittest.TestCase):
    def setUp(self):
        self.manifest = entry.load_manifest(entry.VERSIONS_FILE)

    def test_manifest_defaults(self):
        self.assertEqual(self.manifest["schemaVersion"], 1)
        self.assertIn(self.manifest["skia"]["default"], self.manifest["skia"]["refs"])
        self.assertIn(self.manifest["sdl"]["default"], self.manifest["sdl"]["refs"])
        self.assertEqual(str(self.manifest["cef"]["default"]), self.manifest["cef"]["default"])

    def test_cef_archive_entry_known(self):
        archive = entry.cef_archive_entry(self.manifest, "142", "windows", "x64")
        self.assertIsNotNone(archive)
        self.assertIn("file", archive)

    def test_cef_archive_entry_unknown(self):
        self.assertIsNone(entry.cef_archive_entry(self.manifest, "999", "windows", "x64"))
        self.assertIsNone(entry.cef_archive_entry(self.manifest, "142", "plan9", "x64"))

    def test_load_manifest_rejects_bad_schema(self):
        import json
        import tempfile

        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "versions.json"
            path.write_text(json.dumps({"schemaVersion": 2}), encoding="utf-8")
            with self.assertRaises(ValueError):
                entry.load_manifest(path)


class ArgumentTests(unittest.TestCase):
    def test_default_command_is_build(self):
        args = entry.parse_args([])
        self.assertEqual(args.command, "build")
        self.assertEqual(args.config, "Release")
        self.assertEqual(args.arch, "auto")
        self.assertFalse(args.no_deps)

    def test_explicit_build_options(self):
        args = entry.parse_args(["build", "--config", "Debug", "--arch", "x64",
                                 "--no-deps", "--target", "duilib"])
        self.assertEqual(args.command, "build")
        self.assertEqual(args.config, "Debug")
        self.assertEqual(args.arch, "x64")
        self.assertTrue(args.no_deps)
        self.assertEqual(args.target, "duilib")

    def test_options_before_subcommand_are_kept(self):
        args = entry.parse_args(["--config", "Debug", "--arch", "x86", "--jobs", "4",
                                 "build", "--no-deps"])
        self.assertEqual(args.command, "build")
        self.assertEqual(args.config, "Debug")
        self.assertEqual(args.arch, "x86")
        self.assertEqual(args.jobs, 4)
        self.assertTrue(args.no_deps)

    def test_deps_options(self):
        args = entry.parse_args(["deps", "--force"])
        self.assertEqual(args.command, "deps")
        self.assertTrue(args.force)

    def test_invalid_config_rejected(self):
        with self.assertRaises(SystemExit):
            entry.parse_args(["build", "--config", "RelWithDebInfo"])

    def test_clean_accepts_deps(self):
        args = entry.parse_args(["clean", "--deps"])
        self.assertEqual(args.command, "clean")
        self.assertTrue(args.deps)

    def test_cmake_and_build_args_append(self):
        args = entry.parse_args(["build", "--cmake-arg=-DFOO=1",
                                 "--build-arg=-v"])
        self.assertEqual(args.cmake_arg, ["-DFOO=1"])
        self.assertEqual(args.build_arg, ["-v"])


class StaleCacheTests(unittest.TestCase):
    class _Messages:
        @staticmethod
        def t(_key):
            return "{dir}: {cached} -> {expected}"

    def _make_dir(self, compiler):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        root = Path(temp.name)
        (root / "CMakeCache.txt").write_text(
            f"# cache\nCMAKE_CXX_COMPILER:FILEPATH={compiler}\n", encoding="utf-8")
        return root

    def test_cached_compiler_reads_cache(self):
        cached = Path("C:/tools/compiler.exe")
        root = self._make_dir(str(cached))
        self.assertEqual(entry._cached_compiler(root), cached)

    def test_cached_compiler_without_cache(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.assertIsNone(entry._cached_compiler(Path(tmp)))

    def test_reset_on_compiler_change(self):
        root = self._make_dir(Path("C:/tools/old-compiler.exe"))
        toolchain = entry.Toolchain({"CXX": sys.executable}, "Ninja", None, None)
        entry.reset_stale_build_dir(root, toolchain, self._Messages(), dry_run=False)
        self.assertFalse(root.exists())

    def test_keep_when_compiler_matches(self):
        root = self._make_dir(sys.executable)
        toolchain = entry.Toolchain({"CXX": sys.executable}, "Ninja", None, None)
        entry.reset_stale_build_dir(root, toolchain, self._Messages(), dry_run=False)
        self.assertTrue(root.exists())

    def test_dry_run_keeps_directory(self):
        root = self._make_dir(Path("C:/tools/old-compiler.exe"))
        toolchain = entry.Toolchain({"CXX": sys.executable}, "Ninja", None, None)
        entry.reset_stale_build_dir(root, toolchain, self._Messages(), dry_run=True)
        self.assertTrue(root.exists())


class LanguageTests(unittest.TestCase):
    def test_cli_wins(self):
        self.assertEqual(resolve_language(cli="zh_CN", env={"DUILIB_LANG": "en"}, system="en-US"),
                         "zh_CN")
        self.assertEqual(resolve_language(cli="en", env={"DUILIB_LANG": "zh_CN"}, system="zh-CN"),
                         "en")

    def test_env_used_when_cli_auto(self):
        self.assertEqual(resolve_language(cli="auto", env={"DUILIB_LANG": "zh_CN"}, system="en-US"),
                         "zh_CN")

    def test_system_fallback(self):
        self.assertEqual(resolve_language(cli=None, env={}, system="zh_CN"), "zh_CN")
        self.assertEqual(resolve_language(cli=None, env={}, system="en_US"), "en")
        self.assertEqual(resolve_language(cli=None, env={}, system=""), "en")

    def test_messages_lookup_and_fallback(self):
        messages = Messages("zh_CN", {"k": {"en": "hello", "zh_CN": "你好"},
                                      "only_en": {"en": "only"}})
        self.assertEqual(messages.t("k"), "你好")
        self.assertEqual(messages.t("only_en"), "only")
        self.assertEqual(messages.t("missing"), "missing")


if __name__ == "__main__":
    unittest.main()
