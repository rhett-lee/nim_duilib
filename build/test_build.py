"""build/build.py 与 build/i18n.py 的单元测试（标准库 unittest）。

运行：
  python build/build.py test
  python -m unittest discover -s build -p test_*.py -v
"""

from __future__ import annotations

import contextlib
import importlib.util
import io
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

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

    def test_cef_archives_are_minimal(self):
        for version, block in self.manifest["cef"]["versions"].items():
            for key, archive in block["archives"].items():
                self.assertTrue(archive["file"].endswith("_minimal.tar.bz2"),
                                f"{version}/{key}: {archive['file']}")

    def test_retry_defaults(self):
        self.assertEqual(self.manifest["retry"]["maxAttempts"], 0)
        self.assertEqual(self.manifest["retry"]["delaySeconds"], 20)
        self.assertTrue(self.manifest["skiaCompileRepo"].endswith("skia_compile.git"))

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
        # argparse 会把用法和错误写到 stderr，这里属于预期失败，不打印到测试输出
        with contextlib.redirect_stderr(io.StringIO()):
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


class CmakeDefinePersistenceTests(unittest.TestCase):
    """复用同一构建目录时，有状态的选项必须每次显式传入，避免上次取值残留。"""

    def _defines(self, argv):
        args = entry.parse_args(argv)
        defines, _manifest = entry._cmake_defines(args, 'x64', 'prepare', False)
        return defines

    def test_defaults_are_explicit(self):
        defines = self._defines([])
        manifest = entry.load_manifest(entry.VERSIONS_FILE)
        self.assertIn('-DDUILIB_BUILD_EXAMPLES=ON', defines)
        self.assertIn('-DDUILIB_MD=OFF', defines)
        expected_sdl = ('-DDUILIB_ENABLE_SDL=OFF' if sys.platform == 'win32'
                        else '-DDUILIB_ENABLE_SDL=ON')
        self.assertIn(expected_sdl, defines)
        self.assertIn('-DDUILIB_CEF_VERSION=%s' % manifest['cef']['default'],
                      defines)
        self.assertIn('-DDUILIB_SKIA_REF=%s' % manifest['skia']['default'],
                      defines)
        self.assertIn('-DDUILIB_CEF_ARCHIVE=', defines)

    def test_explicit_flags_override(self):
        defines = self._defines(['build', '--no-examples', '--md', '--sdl',
                                 '--cef-version', '109'])
        self.assertIn('-DDUILIB_BUILD_EXAMPLES=OFF', defines)
        self.assertIn('-DDUILIB_MD=ON', defines)
        self.assertIn('-DDUILIB_ENABLE_SDL=ON', defines)
        self.assertIn('-DDUILIB_CEF_VERSION=109', defines)

    def test_cmake_arg_comes_last(self):
        defines = self._defines(['build', '--md',
                                 '--cmake-arg=-DDUILIB_MD=OFF'])
        self.assertEqual(defines[-1], '-DDUILIB_MD=OFF')


class StaleCacheTests(unittest.TestCase):
    # 与平台无关的“另一个编译器”路径：测试只比较路径是否变化
    OLD_COMPILER = Path(tempfile.gettempdir()) / "duilib-old-compiler"

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

    def _reset(self, root, dry_run=False):
        """执行清理逻辑并捕获它打印的提示。"""
        stream = io.StringIO()
        toolchain = entry.Toolchain({"CXX": sys.executable}, "Ninja", None, None)
        with contextlib.redirect_stdout(stream):
            entry.reset_stale_build_dir(root, toolchain, self._Messages(), dry_run=dry_run)
        return stream.getvalue()

    def test_cached_compiler_reads_cache(self):
        root = self._make_dir(self.OLD_COMPILER)
        self.assertEqual(entry._cached_compiler(root), self.OLD_COMPILER)

    def test_cached_compiler_without_cache(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.assertIsNone(entry._cached_compiler(Path(tmp)))

    def test_reset_on_compiler_change(self):
        root = self._make_dir(self.OLD_COMPILER)
        output = self._reset(root)
        self.assertIn(str(self.OLD_COMPILER), output)
        self.assertFalse(root.exists())

    def test_keep_when_compiler_matches(self):
        root = self._make_dir(sys.executable)
        output = self._reset(root)
        self.assertEqual(output, "")
        self.assertTrue(root.exists())

    def test_dry_run_keeps_directory(self):
        root = self._make_dir(self.OLD_COMPILER)
        output = self._reset(root, dry_run=True)
        self.assertIn(str(self.OLD_COMPILER), output)
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


class DryRunOutputTests(unittest.TestCase):
    def test_clean_dry_run_does_not_report_deletion(self):
        with tempfile.TemporaryDirectory() as root:
            target = Path(root) / "tree"
            target.mkdir()
            (target / "keep.txt").write_text("x", encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                code = entry.main(["--lang", "zh_CN", "clean",
                                   "--build-dir", str(target), "--dry-run"])
            self.assertEqual(code, 0)
            self.assertTrue((target / "keep.txt").exists())
            lines = [line for line in output.getvalue().splitlines() if line.strip()]
            self.assertEqual(len(lines), 1)
            self.assertIn(str(target), lines[0])


class ToolchainSelectionTests(unittest.TestCase):
    def test_uos_detected_from_os_release(self):
        with mock.patch.object(entry.Path, "read_text", return_value='NAME="UOS"\nID=uos\n'):
            self.assertTrue(entry._is_uos())

    def test_non_uos_os_release(self):
        with mock.patch.object(entry.Path, "read_text", return_value="ID=ubuntu\nID_LIKE=debian\n"):
            self.assertFalse(entry._is_uos())

    def test_mingw_compiler_pair_is_selected(self):
        env = {"PATH": ""}

        def triple(name, _env):
            if name in ("clang", "clang++"):
                return "x86_64-w64-windows-gnu"
            return "x86_64-pc-msys"

        with mock.patch.object(entry, "_compiler_triple", side_effect=triple):
            entry._require_mingw_toolchain(env)
        self.assertEqual((env["CC"], env["CXX"]), ("clang", "clang++"))

    def test_non_mingw_compilers_are_rejected(self):
        env = {"PATH": ""}
        with mock.patch.object(entry, "_compiler_triple", return_value="x86_64-pc-msys"):
            with self.assertRaises(entry.BuildError):
                entry._require_mingw_toolchain(env)
        self.assertNotIn("CC", env)


class SkiaPatchTests(unittest.TestCase):
    def _messages(self):
        return entry.make_messages("zh_CN")

    def test_offline_reports_missing_patch(self):
        with tempfile.TemporaryDirectory() as root:
            args = entry.parse_args(["deps", "--deps-root", root, "--offline"])
            with self.assertRaises(entry.BuildError):
                entry._ensure_skia_patch(args, self._messages(), mock.Mock())
            self.assertFalse((Path(root) / "skia_compile").exists())

    def test_dry_run_prints_clone(self):
        with tempfile.TemporaryDirectory() as root:
            args = entry.parse_args(["deps", "--deps-root", root, "--dry-run"])
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                entry._ensure_skia_patch(args, self._messages(), entry.Runner({}, True))
            text = output.getvalue()
            self.assertIn("git clone", text)
            self.assertIn(entry.SKIA_COMPILE_REPO, text)
            self.assertFalse((Path(root) / "skia_compile").exists())

    def test_clone_writes_archive(self):
        with tempfile.TemporaryDirectory() as root:
            args = entry.parse_args(["deps", "--deps-root", root])
            patch = entry._skia_patch_archive(
                entry.load_manifest(entry.VERSIONS_FILE), args)
            calls = []

            def fake_run(command, cwd=None):
                calls.append(command)
                patch.parent.mkdir(parents=True, exist_ok=True)
                patch.write_bytes(b"x")
                return 0

            runner = mock.Mock()
            runner.run.side_effect = fake_run
            entry._ensure_skia_patch(args, self._messages(), runner)
            self.assertEqual(calls[0][:5], ["git", "clone", "--depth", "1",
                                            entry.SKIA_COMPILE_REPO])
            self.assertTrue(patch.is_file())

    def test_clone_retries_until_success(self):
        with tempfile.TemporaryDirectory() as root:
            args = entry.parse_args(["deps", "--deps-root", root])
            patch = entry._skia_patch_archive(
                entry.load_manifest(entry.VERSIONS_FILE), args)
            calls = []

            def fake_run(command, cwd=None):
                calls.append(command)
                if len(calls) == 1:
                    return 128
                patch.parent.mkdir(parents=True, exist_ok=True)
                patch.write_bytes(b"x")
                return 0

            runner = mock.Mock()
            runner.run.side_effect = fake_run
            with mock.patch.object(entry.time, "sleep") as sleep:
                entry._ensure_skia_patch(args, self._messages(), runner)
            self.assertEqual(len(calls), 2)
            sleep.assert_called_once()
            self.assertTrue(patch.is_file())

    def test_non_git_directory_is_rejected(self):
        with tempfile.TemporaryDirectory() as root:
            repo = Path(root) / "skia_compile"
            repo.mkdir()
            (repo / "stale.txt").write_text("x", encoding="utf-8")
            args = entry.parse_args(["deps", "--deps-root", root])
            with self.assertRaises(entry.BuildError):
                entry._ensure_skia_patch(args, self._messages(), mock.Mock())

    def test_existing_repo_is_pulled(self):
        with tempfile.TemporaryDirectory() as root:
            repo = Path(root) / "skia_compile"
            (repo / ".git").mkdir(parents=True)
            args = entry.parse_args(["deps", "--deps-root", root])
            patch = entry._skia_patch_archive(
                entry.load_manifest(entry.VERSIONS_FILE), args)
            calls = []

            def fake_run(command, cwd=None):
                calls.append(command)
                patch.write_bytes(b"x")
                return 0

            runner = mock.Mock()
            runner.run.side_effect = fake_run
            entry._ensure_skia_patch(args, self._messages(), runner)
            # build.py 的 deps root 会 resolve() 成真实路径；macOS 上 /var 是
            # /private/var 的符号链接，这里同样按真实路径比较。
            self.assertEqual(calls[0],
                             ["git", "-C", str(repo.resolve()), "pull", "--ff-only"])
            self.assertTrue(patch.is_file())

    def test_built_artifacts_skip_clone(self):
        with tempfile.TemporaryDirectory() as root:
            variant = Path(root) / "skia" / "out" / "llvm.x64.release"
            variant.mkdir(parents=True)
            (variant / "skia.lib").write_text("x", encoding="utf-8")
            args = entry.parse_args(["deps", "--deps-root", root])
            runner = mock.Mock()
            entry._ensure_skia_patch(args, self._messages(), runner)
            runner.run.assert_not_called()
            self.assertFalse((Path(root) / "skia_compile").exists())


if __name__ == "__main__":
    unittest.main()
