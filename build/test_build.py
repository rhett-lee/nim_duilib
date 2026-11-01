"""build/build.py 与 build/i18n.py 的单元测试（标准库 unittest）。

运行：
  python3 build/build.py test
  python3 -m unittest discover -s build -p test_*.py -v
"""

from __future__ import annotations

import contextlib
import importlib.util
import io
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path
from types import SimpleNamespace
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

# 固定简体中文：帮助文本断言基于中文，不能随宿主系统语言变化。
entry.MESSAGES = entry.make_messages("zh_CN")


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


class MsvcVcvarsTests(unittest.TestCase):
    """vcvarsall 平台参数：x64 主机上的 arm64 目标走交叉工具，不要求 Hostarm64。"""

    def _platform(self, arch, host):
        with mock.patch.object(entry, "host_arch", return_value=host):
            return entry._vcvars_platform(arch)

    def test_native_platforms(self):
        self.assertEqual(self._platform("x64", "x64"), "x64")
        self.assertEqual(self._platform("x86", "x64"), "x86")

    def test_arm64_cross_from_x64(self):
        self.assertEqual(self._platform("arm64", "x64"), "amd64_arm64")

    def test_arm64_cross_from_x86(self):
        self.assertEqual(self._platform("arm64", "x86"), "x86_arm64")

    def test_arm64_native(self):
        self.assertEqual(self._platform("arm64", "arm64"), "arm64")


class VsGeneratorTests(unittest.TestCase):
    """没有 Ninja 时的 VS 生成器名：必须随实际安装的 VS 版本变化。"""

    def test_install_layouts(self):
        root = Path("C:/Program Files/Microsoft Visual Studio")
        cases = {
            "2017": "Visual Studio 15 2017",
            "2019": "Visual Studio 16 2019",
            "2022": "Visual Studio 17 2022",
            "2026": "Visual Studio 18 2026",
            "15": "Visual Studio 15 2017",
            "17": "Visual Studio 17 2022",
            "18": "Visual Studio 18 2026",
        }
        for folder, expected in cases.items():
            install = root / folder / "Community"
            self.assertEqual(entry.vs_generator_name(install), expected, folder)

    def test_unknown_install_falls_back_to_2022(self):
        self.assertEqual(entry.vs_generator_name(None), "Visual Studio 17 2022")
        self.assertEqual(entry.vs_generator_name(Path("C:/VS/BuildTools")),
                         "Visual Studio 17 2022")

    def test_windows_without_ninja_uses_installed_vs_generator(self):
        install = Path("C:/Program Files/Microsoft Visual Studio/18/Community")
        with mock.patch.object(entry.sys, "platform", "win32"), \
                mock.patch.object(entry, "_msvc_environment",
                                  return_value=({"PATH": ""}, install)), \
                mock.patch.object(entry, "_find_ninja", return_value=None):
            toolchain = entry.detect_toolchain("x64", None)
        self.assertEqual(toolchain.generator, "Visual Studio 18 2026")
        self.assertEqual(toolchain.vs_install, install)


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


class NinjaDiscoveryTests(unittest.TestCase):
    """ninja 查找顺序：PATH → Visual Studio → skia/bin → 补丁归档自带的副本。

    某些 Visual Studio 安装不含 ninja（可选组件），而 Skia 补丁归档自带
    bin/ninja.exe：此时仍应使用 Ninja 生成器与预设，而不是退回 MSBuild。
    """

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = Path(tmp.name)

    def _fake_ninja(self, path: Path) -> Path:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(b"")
        return path

    def test_env_override(self):
        expected = self._fake_ninja(self.tmp / "override" / "ninja.exe")
        env = {"PATH": "", "DUILIB_NINJA_EXECUTABLE": str(expected)}
        with mock.patch.object(entry.shutil, "which", return_value=None):
            self.assertEqual(entry._find_ninja(env, None), expected)

    def test_path_hit(self):
        expected = self._fake_ninja(self.tmp / "path" / "ninja.exe")
        with mock.patch.object(entry.shutil, "which", return_value=str(expected)):
            self.assertEqual(entry._find_ninja({"PATH": ""}, None), expected)

    def test_visual_studio_hit(self):
        vs = self.tmp / "VS"
        expected = self._fake_ninja(
            vs / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake" / "Ninja" / "ninja.exe")
        with mock.patch.object(entry.shutil, "which", return_value=None):
            self.assertEqual(entry._find_ninja({"PATH": ""}, vs), expected)

    def test_skia_bin_used_when_not_on_path(self):
        deps = self.tmp / "deps"
        name = "ninja.exe" if sys.platform == "win32" else "ninja"
        expected = self._fake_ninja(deps / "skia" / "bin" / name)
        args = SimpleNamespace(deps_root=str(deps), skia_ref=None)
        with mock.patch.object(entry.shutil, "which", return_value=None):
            self.assertEqual(entry._find_ninja({"PATH": ""}, None, args), expected)

    def test_missing_everywhere(self):
        with mock.patch.object(entry.shutil, "which", return_value=None):
            self.assertIsNone(entry._find_ninja({"PATH": ""}, None))

    @unittest.skipUnless(sys.platform == "win32", "归档内的 ninja 是 Windows 程序")
    def test_archive_extract_when_nothing_else(self):
        deps = self.tmp / "deps"
        target = self.tmp / "tools" / "bin" / "ninja.exe"
        args = SimpleNamespace(deps_root=str(deps), skia_ref=None)
        with mock.patch.object(entry.shutil, "which", return_value=None), \
                mock.patch.object(entry, "_bundled_ninja_path", return_value=target):
            self.assertEqual(entry._find_ninja({"PATH": ""}, None, args), target)
        self.assertTrue(target.is_file())

    @unittest.skipUnless(sys.platform == "win32", "归档内的 ninja 是 Windows 程序")
    def test_archive_missing_returns_none(self):
        deps = self.tmp / "deps"
        target = self.tmp / "tools" / "bin" / "ninja.exe"
        args = SimpleNamespace(deps_root=str(deps), skia_ref=None)
        with mock.patch.object(entry.shutil, "which", return_value=None), \
                mock.patch.object(entry, "_bundled_ninja_path", return_value=target), \
                mock.patch.object(entry, "_skia_patch_archive", return_value=None):
            self.assertIsNone(entry._find_ninja({"PATH": ""}, None, args))
        self.assertFalse(target.is_file())


class ManifestTests(unittest.TestCase):
    def setUp(self):
        self.manifest = entry.load_manifest(entry.VERSIONS_FILE)

    def test_manifest_defaults(self):
        self.assertEqual(self.manifest["schemaVersion"], 1)
        self.assertIn(self.manifest["skia"]["default"], self.manifest["skia"]["refs"])
        self.assertIn(self.manifest["sdl"]["default"], self.manifest["sdl"]["refs"])
        self.assertEqual(str(self.manifest["cef"]["default"]), self.manifest["cef"]["default"])

    def test_cef_archive_entry_known(self):
        archive = entry.cef_archive_entry(self.manifest, "156", "windows", "x64")
        self.assertIsNotNone(archive)
        self.assertIn("file", archive)

    def test_cef_archive_entry_unknown(self):
        self.assertIsNone(entry.cef_archive_entry(self.manifest, "999", "windows", "x64"))
        self.assertIsNone(entry.cef_archive_entry(self.manifest, "156", "plan9", "x64"))

    def test_cef_archives_are_minimal(self):
        for version, block in self.manifest["cef"]["versions"].items():
            for key, archive in block["archives"].items():
                self.assertTrue(archive["file"].endswith("_minimal.tar.bz2"),
                                f"{version}/{key}: {archive['file']}")

    def test_retry_defaults(self):
        self.assertEqual(self.manifest["retry"]["maxAttempts"], 0)
        self.assertEqual(self.manifest["retry"]["delaySeconds"], 20)
        self.assertEqual(self.manifest["skia"]["patchDir"], "build/skia_patch")
        self.assertNotIn("skiaCompileRepo", self.manifest)

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
        self.assertIn('-DDUILIB_RPATH_ABSOLUTE=ON', defines)
        expected_sdl = ('-DDUILIB_ENABLE_SDL=OFF' if sys.platform == 'win32'
                        else '-DDUILIB_ENABLE_SDL=ON')
        self.assertIn(expected_sdl, defines)
        # CEF 是否编译进库同样有状态：MSVC/Linux/macOS 默认开启，MinGW/FreeBSD 关闭
        if sys.platform == 'win32':
            expected_cef = ('-DDUILIB_ENABLE_CEF=OFF' if os.environ.get('MSYSTEM')
                            else '-DDUILIB_ENABLE_CEF=ON')
        elif sys.platform == 'darwin' or sys.platform.startswith('linux'):
            expected_cef = '-DDUILIB_ENABLE_CEF=ON'
        else:
            expected_cef = '-DDUILIB_ENABLE_CEF=OFF'
        self.assertIn(expected_cef, defines)
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

    @unittest.skipUnless(sys.platform == 'win32' and not os.environ.get('MSYSTEM'),
                         '--dll 仅在 Windows MSVC 下可用')
    def test_dll_does_not_enable_sdl(self):
        """Windows 上 SDL 始终默认关闭，DLL 模式也不例外。"""
        defines = self._defines(['build', '--dll'])
        self.assertIn('-DDUILIB_DLL=ON', defines)
        self.assertIn('-DDUILIB_MD=ON', defines)
        self.assertIn('-DDUILIB_ENABLE_SDL=OFF', defines)

    def test_no_absolute_rpath(self):
        defines = self._defines(['build', '--no-absolute-rpath'])
        self.assertIn('-DDUILIB_RPATH_ABSOLUTE=OFF', defines)

    def test_cmake_arg_comes_last(self):
        defines = self._defines(['build', '--md',
                                 '--cmake-arg=-DDUILIB_MD=OFF'])
        self.assertEqual(defines[-1], '-DDUILIB_MD=OFF')

    def test_cef_off_via_cmake_arg(self):
        """`--cmake-arg` 关闭 CEF 时应覆盖显式传入的默认值，并排在最后生效。"""
        defines = self._defines(['build',
                                 '--cmake-arg=-DDUILIB_ENABLE_CEF=OFF'])
        self.assertEqual(defines[-1], '-DDUILIB_ENABLE_CEF=OFF')
        self.assertTrue(any(d.startswith('-DDUILIB_ENABLE_CEF=')
                            for d in defines[:-1]))


class CefVersionTests(unittest.TestCase):
    """--cef-version：109 为旧版，latest 跟随版本清单，不硬编码当前版本号。"""

    def setUp(self):
        self.manifest = entry.load_manifest(entry.VERSIONS_FILE)

    def test_latest_follows_manifest_default(self):
        default = self.manifest["cef"]["default"]
        self.assertEqual(entry.resolve_cef_version("latest", self.manifest), default)
        self.assertEqual(entry.resolve_cef_version(None, self.manifest), default)

    def test_109_and_concrete_version(self):
        self.assertEqual(entry.resolve_cef_version("109", self.manifest), "109")
        default = self.manifest["cef"]["default"]
        self.assertEqual(entry.resolve_cef_version(default, self.manifest), default)

    def test_unknown_version_raises(self):
        with self.assertRaises(entry.BuildError):
            entry.resolve_cef_version("999", self.manifest)

    def test_define_uses_manifest_default_for_latest(self):
        args = entry.parse_args(["build", "--cef-version", "latest"])
        defines, _ = entry._cmake_defines(args, "x64", "prepare", False)
        self.assertIn("-DDUILIB_CEF_VERSION=%s" % self.manifest["cef"]["default"],
                      defines)

    def test_help_marks_windows_only_options(self):
        """非 Windows 平台应把 --sdl/--md/--vs-version 标注为仅 Windows。"""
        with mock.patch.object(entry.sys, "platform", "linux"):
            parser = entry.argparse.ArgumentParser(add_help=False)
            entry._add_common_options(parser)
            text = parser.format_help()
        self.assertIn("仅 Windows 有效；其他平台 SDL 始终开启", text)
        self.assertIn("仅 Windows（MSVC）有效", text)
        self.assertIn("仅 Windows 有效；其他平台忽略该参数", text)
        self.assertIn("产物只写相对 rpath", text)

    def test_help_switches_to_english(self):
        """--lang en 时帮助文案必须全英文且为纯 ASCII。"""
        with mock.patch.object(entry.sys, "platform", "linux"):
            old = entry.MESSAGES
            entry.MESSAGES = entry.make_messages("en")
            try:
                parser = entry.argparse.ArgumentParser(add_help=False)
                entry._add_common_options(parser)
                text = parser.format_help()
            finally:
                entry.MESSAGES = old
        self.assertIn("Windows only; SDL is always enabled", text)
        self.assertIn("Windows (MSVC) only", text)
        self.assertIn("write only relative rpath", text)
        self.assertEqual([ch for ch in text if ord(ch) > 127], [])


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
        """执行清理逻辑并捕获它打印的提示，返回 (输出, 是否检测到切换)。"""
        stream = io.StringIO()
        toolchain = entry.Toolchain({"CXX": sys.executable}, "Ninja", None, None)
        with contextlib.redirect_stdout(stream):
            switched = entry.reset_stale_build_dir(root, toolchain, self._Messages(), dry_run=dry_run)
        return stream.getvalue(), switched

    def test_cached_compiler_reads_cache(self):
        root = self._make_dir(self.OLD_COMPILER)
        self.assertEqual(entry._cached_compiler(root), self.OLD_COMPILER)

    def test_cached_compiler_without_cache(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.assertIsNone(entry._cached_compiler(Path(tmp)))

    def test_reset_on_compiler_change(self):
        root = self._make_dir(self.OLD_COMPILER)
        output, switched = self._reset(root)
        self.assertTrue(switched)
        self.assertIn(str(self.OLD_COMPILER), output)
        self.assertFalse(root.exists())

    def test_keep_when_compiler_matches(self):
        root = self._make_dir(sys.executable)
        output, switched = self._reset(root)
        self.assertFalse(switched)
        self.assertEqual(output, "")
        self.assertTrue(root.exists())

    def test_dry_run_keeps_directory(self):
        root = self._make_dir(self.OLD_COMPILER)
        output, switched = self._reset(root, dry_run=True)
        self.assertTrue(switched)
        self.assertIn(str(self.OLD_COMPILER), output)
        self.assertTrue(root.exists())


class CompilerOutputTests(unittest.TestCase):
    """切换编译器后清理 lib/、build_temp 与 bin/ 中的产物，保留运行时资源。"""

    class _Messages:
        @staticmethod
        def t(key):
            return "[" + key + "]"

        @staticmethod
        def hint(key):
            return "Hint: [" + key + "]"

    RUNTIME_FILES = ("resources.zip", "WebView2Loader.dll")
    RUNTIME_DIRS = ("resources", "libcef_win", "libcef_win_109", "cef_cache", "webview2_cache")

    def _make_repo(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        root = Path(temp.name)
        for relative in ("lib/x64/Release", "build/build_temp/x64"):
            (root / relative).mkdir(parents=True)
        (root / "lib/x64/Release/duilib.lib").write_bytes(b"lib")
        (root / "lib/x64/Release/libcef_dll_wrapper.lib").write_bytes(b"lib")
        (root / "build/build_temp/x64/duilib.obj").write_bytes(b"obj")
        bin_dir = root / "bin"
        bin_dir.mkdir()
        for name in ("basic.exe", "basic.pdb", "duilib_d.ilk", "duilib_d.pdb", "CefBrowser_d.ilk"):
            (bin_dir / name).write_bytes(b"bin")
        (bin_dir / "duilib.dll").write_bytes(b"dll")
        for name in self.RUNTIME_FILES:
            (bin_dir / name).write_bytes(b"runtime")
        for name in self.RUNTIME_DIRS:
            (bin_dir / name).mkdir()
            (bin_dir / name / "keep.txt").write_text("runtime", encoding="utf-8")
        stamp_dir = bin_dir / entry.STAMP_DIR_NAME
        stamp_dir.mkdir()
        (stamp_dir / "basic").write_text("Release\n", encoding="utf-8")
        (bin_dir / "basic.build-config").write_text("Release\n", encoding="utf-8")
        return root

    def _clean(self, root, dry_run=False):
        stream = io.StringIO()
        with contextlib.redirect_stdout(stream):
            removed = entry.clean_compiler_outputs(root, self._Messages(), dry_run)
        return removed, stream.getvalue()

    def test_paths_exclude_runtime_assets(self):
        root = self._make_repo()
        names = {path.name for path in entry.compiler_output_paths(root)}
        self.assertIn("lib", names)
        self.assertIn("build_temp", names)
        self.assertIn("basic.exe", names)
        self.assertIn("duilib.dll", names)
        for name in self.RUNTIME_FILES + self.RUNTIME_DIRS:
            self.assertNotIn(name, names)

    def test_clean_removes_compiler_outputs(self):
        root = self._make_repo()
        removed, output = self._clean(root)
        self.assertIn("[build.compiler_outputs]", output)
        self.assertFalse((root / "lib").exists())
        self.assertFalse((root / "build/build_temp").exists())
        bin_dir = root / "bin"
        for name in ("basic.exe", "basic.pdb", "duilib_d.ilk", "duilib_d.pdb",
                     "CefBrowser_d.ilk", "duilib.dll", "basic.build-config"):
            self.assertFalse((bin_dir / name).exists(), name)
        self.assertFalse((bin_dir / entry.STAMP_DIR_NAME).exists())
        for name in self.RUNTIME_FILES:
            self.assertTrue((bin_dir / name).is_file(), name)
        for name in self.RUNTIME_DIRS:
            self.assertTrue((bin_dir / name / "keep.txt").is_file(), name)
        self.assertGreater(len(removed), 0)

    def test_dry_run_keeps_outputs(self):
        root = self._make_repo()
        removed, output = self._clean(root, dry_run=True)
        self.assertIn("[build.compiler_outputs]", output)
        self.assertTrue((root / "lib/x64/Release/duilib.lib").is_file())
        self.assertTrue((root / "build/build_temp/x64/duilib.obj").is_file())
        self.assertTrue((root / "bin/basic.exe").is_file())
        self.assertTrue((root / "bin/.build-config/basic").is_file())


class DepsPlanPrintTests(unittest.TestCase):
    """build.py _print_deps_plan：deps --dry-run 额外打印 Skia/SDL3 的实际命令。"""

    class _Messages:
        @staticmethod
        def t(key):
            return "[" + key + "]"

        @staticmethod
        def hint(key):
            return "Hint: [" + key + "]"

    def _run(self, build_dir, runner_returns=0, md=False, dll=False,
             reset_pending=False):
        """调用 _print_deps_plan 并返回它打印的内容；Runner 用桩替代，不真的执行 cmake。"""
        args = SimpleNamespace(jobs=2, config="Release", build_arg=[],
                               md=md, dll=dll, sdl=None, cmake_arg=[])
        toolchain = entry.Toolchain({"CXX": sys.executable}, "Ninja", None, None)
        stream = io.StringIO()
        calls = []

        class _StubRunner:
            def __init__(self, _env, dry_run):
                pass

            def run(self, command, cwd=None):
                calls.append(command)
                return runner_returns

        with mock.patch.object(entry, "Runner", _StubRunner), \
                contextlib.redirect_stdout(stream):
            entry._print_deps_plan(args, self._Messages(), toolchain, build_dir,
                                   reset_pending)
        return stream.getvalue(), calls

    def test_reset_pending_skips_plan(self):
        """构建树要重建（编译器切换/运行库模式切换）时，不按旧缓存展开计划命令。"""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "CMakeCache.txt").write_text("# cache\n", encoding="utf-8")
            output, calls = self._run(root, reset_pending=True)
        self.assertIn("[deps.plan_reset]", output)
        self.assertIn("Hint: [deps.plan_reset_hint]", output)
        self.assertNotIn("[deps.plan_cache_stale]", output)
        self.assertEqual(calls, [])

    def test_unconfigured_build_dir_prints_hint(self):
        """构建目录还没配置时只给提示，不能在 dry-run 里触发真正的配置。"""
        with tempfile.TemporaryDirectory() as tmp:
            output, calls = self._run(Path(tmp))
        self.assertIn("[deps.plan_unconfigured]", output)
        self.assertIn("Hint: [deps.plan_unconfigured_hint]", output)
        self.assertEqual(calls, [])

    def test_configured_build_dir_runs_plan_target(self):
        """已配置的构建目录：执行只读的 duilib_deps_plan 目标。"""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "CMakeCache.txt").write_text("# cache\n", encoding="utf-8")
            output, calls = self._run(root)
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][-1], "duilib_deps_plan")
        self.assertNotIn("[deps.plan_unconfigured]", output)

    def test_stale_cache_options_print_warning(self):
        """缓存里的 MD/DLL 与本次参数不同：提示计划命令反映的是上次配置。"""
        cache = ("DUILIB_MD:BOOL=ON\nDUILIB_DLL:BOOL=ON\n"
                 "DUILIB_ENABLE_SDL:UNINITIALIZED=ON\n")
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "CMakeCache.txt").write_text(cache, encoding="utf-8")
            output, _ = self._run(root)
        self.assertIn("[deps.plan_cache_stale]", output)
        self.assertIn("Hint: [deps.plan_cache_stale_hint]", output)
        self.assertIn("DUILIB_MD=ON (now OFF)", output)
        self.assertIn("DUILIB_DLL=ON (now OFF)", output)
        if sys.platform == "win32":
            # Windows 默认关闭 SDL；非 Windows 默认开启，因此只在 Windows 上断言
            self.assertIn("DUILIB_ENABLE_SDL=ON (now OFF)", output)

    def test_matching_cache_options_print_no_warning(self):
        """缓存与本次参数一致时不要误报（例如 --dll 与缓存 DUILIB_DLL=ON）。"""
        cache = "DUILIB_MD:BOOL=ON\nDUILIB_DLL:BOOL=ON\n"
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "CMakeCache.txt").write_text(cache, encoding="utf-8")
            output, _ = self._run(root, md=True, dll=True)
        self.assertNotIn("[deps.plan_cache_stale]", output)

    def test_plan_target_failure_prints_hint(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "CMakeCache.txt").write_text("# cache\n", encoding="utf-8")
            output, calls = self._run(root, runner_returns=1)
        self.assertIn("[deps.plan_failed]", output)
        self.assertIn("Hint: [deps.plan_failed_hint]", output)


class ConfigSwitchTests(unittest.TestCase):
    """切换 Debug/Release 时，共享 bin/ 中另一种配置的示例产物要被清理。

    Debug 与 Release 的示例同名且都写到 bin/，目标文件却按配置分开；不清理时
    Ninja 会认为链接步骤已是最新（ninja: no work to do），bin/ 一直不更新。
    """

    class _Messages:
        @staticmethod
        def t(key):
            return {
                "build.config_switch": "switch {stale}/{count}/{config}",
                "build.clean_failed": "failed {path}: {error}",
            }.get(key, key)

        @staticmethod
        def hint(_key):
            return "hint"

    def _bin(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        bin_dir = Path(temp.name) / "bin"
        (bin_dir / ".build-config").mkdir(parents=True)
        return bin_dir

    def _stamp(self, bin_dir, target, config):
        (bin_dir / ".build-config" / target).write_text(config + "\n", encoding="utf-8")

    def _touch(self, path):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("x", encoding="utf-8")

    def _clear(self, bin_dir, config, target=None, dry_run=False):
        stream = io.StringIO()
        with contextlib.redirect_stdout(stream):
            removed = entry.clear_stale_build_outputs(
                bin_dir, config, target, self._Messages(), dry_run=dry_run)
        return removed, stream.getvalue()

    def test_stale_output_detected_for_other_config(self):
        bin_dir = self._bin()
        self._stamp(bin_dir, "basic", "Debug")
        self._touch(bin_dir / "basic")
        stale = entry.stale_build_outputs(bin_dir, "Release")
        self.assertEqual([item[0] for item in stale], ["basic"])
        self.assertEqual(stale[0][1], "Debug")
        self.assertEqual(stale[0][3], [bin_dir / "basic"])
        # 标记目录本身不能当成旧版标记文件
        self.assertNotIn(".build-config", entry.build_config_stamps(bin_dir))

    def test_current_config_is_kept(self):
        bin_dir = self._bin()
        self._stamp(bin_dir, "basic", "Release")
        self._touch(bin_dir / "basic")
        self.assertEqual(entry.stale_build_outputs(bin_dir, "Release"), [])
        removed, output = self._clear(bin_dir, "Release")
        self.assertEqual(removed, [])
        self.assertEqual(output, "")
        self.assertTrue((bin_dir / "basic").exists())

    def test_clear_removes_output_and_stamp(self):
        bin_dir = self._bin()
        self._stamp(bin_dir, "basic", "Debug")
        self._touch(bin_dir / "basic")
        removed, output = self._clear(bin_dir, "Release")
        self.assertEqual(removed, ["basic"])
        self.assertIn("Debug/1/Release", output)
        self.assertFalse((bin_dir / "basic").exists())
        self.assertFalse((bin_dir / ".build-config" / "basic").exists())

    def test_windows_exe_is_removed(self):
        bin_dir = self._bin()
        self._stamp(bin_dir, "basic", "Debug")
        self._touch(bin_dir / "basic.exe")
        removed, _ = self._clear(bin_dir, "Release")
        self.assertEqual(removed, ["basic"])
        self.assertFalse((bin_dir / "basic.exe").exists())

    def test_macos_bundle_keeps_info_plist(self):
        """只删 .app 内的可执行文件：Info.plist 由 configure 生成，删掉不会再生。

        用 CEF 示例名（CefBrowser）验证：CEF 的 .app 结构与普通示例一致，
        且额外带 Contents/Frameworks（CEF framework、Helper.app、SDL3 动态库），
        这些都由链接后的 POST_BUILD 复制，清理时不能整目录删除。
        """
        bin_dir = self._bin()
        self._stamp(bin_dir, "CefBrowser", "Debug")
        bundle = bin_dir / "CefBrowser.app"
        self._touch(bundle / "Contents" / "Info.plist")
        self._touch(bundle / "Contents" / "MacOS" / "CefBrowser")
        self._touch(bundle / "Contents" / "Resources" / "duilib" / "resources" / "themes" / "global.xml")
        framework = bundle / "Contents" / "Frameworks" / "Chromium Embedded Framework.framework"
        self._touch(framework / "Versions" / "A" / "Chromium Embedded Framework")
        helper_exe = (bundle / "Contents" / "Frameworks" / "CefBrowser Helper.app"
                      / "Contents" / "MacOS" / "CefBrowser Helper")
        self._touch(helper_exe)
        self._touch(bundle / "Contents" / "Frameworks" / "libSDL3.dylib")
        removed, _ = self._clear(bin_dir, "Release")
        self.assertEqual(removed, ["CefBrowser"])
        self.assertFalse((bundle / "Contents" / "MacOS" / "CefBrowser").exists())
        self.assertTrue((bundle / "Contents" / "Info.plist").exists())
        self.assertTrue((bundle / "Contents" / "Resources" / "duilib" / "resources" / "themes" / "global.xml").exists())
        self.assertTrue((framework / "Versions" / "A" / "Chromium Embedded Framework").exists())
        self.assertTrue(helper_exe.exists())
        self.assertTrue((bundle / "Contents" / "Frameworks" / "libSDL3.dylib").exists())

    def test_non_bundle_directory_is_removed(self):
        bin_dir = self._bin()
        self._stamp(bin_dir, "basic", "Debug")
        self._touch(bin_dir / "basic" / "nested" / "file")
        removed, _ = self._clear(bin_dir, "Release")
        self.assertEqual(removed, ["basic"])
        self.assertFalse((bin_dir / "basic").exists())

    def test_legacy_inline_stamp_is_read_and_removed(self):
        bin_dir = self._bin()
        legacy = bin_dir / "basic.build-config"
        legacy.write_text("Debug\n", encoding="utf-8")
        self._touch(bin_dir / "basic")
        stale = entry.stale_build_outputs(bin_dir, "Release")
        self.assertEqual(len(stale), 1)
        self.assertEqual(stale[0][2], legacy)
        self._clear(bin_dir, "Release")
        self.assertFalse(legacy.exists())
        self.assertFalse((bin_dir / "basic").exists())

    def test_directory_stamp_wins_over_legacy(self):
        bin_dir = self._bin()
        self._stamp(bin_dir, "basic", "Release")
        (bin_dir / "basic.build-config").write_text("Debug\n", encoding="utf-8")
        self._touch(bin_dir / "basic")
        # 新路径说 Release，说明产物已是当前配置；旧标记不应把它当成 Debug
        self.assertEqual(entry.stale_build_outputs(bin_dir, "Release"), [])

    def test_stale_stamp_without_output_is_ignored(self):
        bin_dir = self._bin()
        self._stamp(bin_dir, "basic", "Debug")
        self.assertEqual(entry.stale_build_outputs(bin_dir, "Release"), [])
        self.assertEqual(entry.build_config_stamps(bin_dir)["basic"][0], "Debug")

    def test_target_filter_limits_cleanup(self):
        bin_dir = self._bin()
        for name in ("basic", "chat"):
            self._stamp(bin_dir, name, "Debug")
            self._touch(bin_dir / name)
        removed, _ = self._clear(bin_dir, "Release", target="basic")
        self.assertEqual(removed, ["basic"])
        self.assertFalse((bin_dir / "basic").exists())
        self.assertTrue((bin_dir / "chat").exists())
        self.assertTrue((bin_dir / ".build-config" / "chat").exists())

    def test_target_accepts_output_name(self):
        bin_dir = self._bin()
        for name in ("basic", "chat"):
            self._stamp(bin_dir, name, "Debug")
            self._touch(bin_dir / name)
        removed, _ = self._clear(bin_dir, "Release", target="basic.app")
        self.assertEqual(removed, ["basic"])

    def test_all_targets_clean_whole_bin(self):
        for target in (None, "all"):
            bin_dir = self._bin()
            for name in ("basic", "chat"):
                self._stamp(bin_dir, name, "Debug")
                self._touch(bin_dir / name)
            removed, _ = self._clear(bin_dir, "Release", target=target)
            self.assertEqual(removed, ["basic", "chat"])
            self.assertFalse((bin_dir / "basic").exists())
            self.assertFalse((bin_dir / "chat").exists())

    def test_dry_run_keeps_outputs_and_stamps(self):
        bin_dir = self._bin()
        self._stamp(bin_dir, "basic", "Debug")
        self._touch(bin_dir / "basic")
        removed, output = self._clear(bin_dir, "Release", dry_run=True)
        self.assertEqual(removed, ["basic"])
        self.assertIn("Debug/1/Release", output)
        self.assertTrue((bin_dir / "basic").exists())
        self.assertTrue((bin_dir / ".build-config" / "basic").exists())


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

    def test_traditional_chinese_falls_back_to_english(self):
        """繁体区域一律英文，与 cmake/duilib_i18n.cmake 的判定保持一致。"""
        for system in ("zh_TW", "zh-HK", "zh_MO", "zh_Hant", "zh_Hant_TW"):
            self.assertEqual(resolve_language(cli=None, env={}, system=system), "en", system)

    def test_simplified_chinese_variants(self):
        for system in ("zh_CN", "zh-CN", "zh_Hans", "zh_Hans_CN", "zh"):
            self.assertEqual(resolve_language(cli=None, env={}, system=system), "zh_CN", system)

    def test_messages_lookup_and_fallback(self):
        messages = Messages("zh_CN", {"k": {"en": "hello", "zh_CN": "你好"},
                                      "only_en": {"en": "only"}})
        self.assertEqual(messages.t("k"), "你好")
        self.assertEqual(messages.t("only_en"), "only")
        self.assertEqual(messages.t("missing"), "missing")

    def test_help_honors_lang_before_argparse(self):
        """--help 在 argparse 之前解析 --lang，英文输出必须是纯 ASCII。"""
        out = io.StringIO()
        old = entry.MESSAGES
        try:
            with contextlib.redirect_stdout(out):
                with self.assertRaises(SystemExit) as ctx:
                    entry.main(["--help", "--lang", "en"])
        finally:
            entry.MESSAGES = old
        self.assertEqual(ctx.exception.code, 0)
        text = out.getvalue()
        self.assertIn("nim_duilib unified build entry", text)
        self.assertEqual([ch for ch in text if ord(ch) > 127], [])


class MessageCatalogTests(unittest.TestCase):
    """cmake/i18n/messages.json 的键必须与代码里的引用一致。

    两侧语言目录查不到键时会把键名原样输出（CMake 的 duilib_t/duilib_msg/
    duilib_warn/duilib_error/duilib_tf 与 Python 的 T()/Messages.t()/hint()），
    用户看到的是一串英文键名。这里静态扫描调用点，把“引用了不存在的键”
    或“只提供一种语言”挡在提交前。
    """

    # 键名形如 `区域.用途`，区域是 messages.json 里已有的前缀
    KEY_RE = re.compile(
        r"^(?:sdk|pkg|rel|deps|auto|help|preflight|build|options|pack|test|doctor|lang"
        r"|cmake|bundle)\.[a-z0-9_.]+$")
    CMAKE_CALL_RE = (
        re.compile(r'duilib_t\(\s*"([^"]+)"'),
        # duilib_tf 的第一个参数是输出变量，键名在第二个位置
        re.compile(r'duilib_tf\(\s*[^"\s]+\s*\n?\s*"([^"]+)"'),
        re.compile(r'duilib_msg\(\s*"([^"]+)"'),
        re.compile(r'duilib_warn\(\s*"([^"]+)"'),
        re.compile(r'duilib_error\(\s*"([^"]+)"\s+"([^"]+)"'),
    )
    PYTHON_CALL_RE = (
        re.compile(r"T\(\s*'([^']+)'\s*[%),]"),
        re.compile(r'T\(\s*"([^"]+)"\s*[%),]'),
        re.compile(r"\.t\(\s*'([^']+)'\s*\)"),
        re.compile(r'\.t\(\s*"([^"]+)"\s*\)'),
        re.compile(r"\.hint\(\s*'([^']+)'\s*\)"),
        re.compile(r'\.hint\(\s*"([^"]+)"\s*\)'),
    )

    @staticmethod
    def catalog():
        data = json.loads((REPO_ROOT / "cmake" / "i18n" / "messages.json")
                          .read_text(encoding="utf-8"))
        return data["messages"]

    def _sources(self):
        """CMake 脚本、根 CMakeLists 与构建入口（不含单元测试本身）。"""
        files = sorted((REPO_ROOT / "cmake").rglob("*.cmake"))
        for name in ("CMakeLists.txt", "duilib/CMakeLists.txt"):
            path = REPO_ROOT / name
            if path.is_file():
                files.append(path)
        files += [path for path in sorted(BUILD_DIR.glob("*.py"))
                  if path.name != "test_build.py" and not path.name.startswith("test_")]
        return files

    def _referenced_keys(self):
        """扫描调用点，返回 {键名: 出现位置}。"""
        found = {}
        for path in self._sources():
            text = path.read_text(encoding="utf-8", errors="replace")
            patterns = (self.CMAKE_CALL_RE if path.suffix == ".cmake"
                        or path.name == "CMakeLists.txt" else self.PYTHON_CALL_RE)
            for pattern in patterns:
                for match in pattern.finditer(text):
                    for group in match.groups():
                        if not group or not self.KEY_RE.match(group):
                            continue
                        line = text.count("\n", 0, match.start()) + 1
                        found.setdefault(group, "%s:%d" % (path.relative_to(REPO_ROOT), line))
        return found

    def test_catalog_is_bilingual(self):
        catalog = self.catalog()
        self.assertTrue(catalog)
        missing = []
        for key, entry in catalog.items():
            if not self.KEY_RE.match(key):
                missing.append("%s: 键名不符合 `区域.用途`" % key)
                continue
            for language in ("en", "zh_CN"):
                if not isinstance(entry, dict) or not str(entry.get(language, "")).strip():
                    missing.append("%s: 缺少 %s" % (key, language))
        self.assertEqual(missing, [], "消息目录不完整：\n" + "\n".join(missing))

    def test_placeholders_single_style(self):
        """同一条消息不要混用两种插值风格（%s 与 {name}）。"""
        problems = []
        for key, entry in self.catalog().items():
            for language, text in entry.items():
                if "%s" in text and re.search(r"\{[a-z0-9_]+\}", text):
                    problems.append("%s/%s: %s" % (key, language, text))
        self.assertEqual(problems, [], "同一条消息混用了 %s 与 {name}：\n" + "\n".join(problems))

    def test_referenced_keys_exist(self):
        catalog = self.catalog()
        referenced = self._referenced_keys()
        self.assertTrue(referenced, "没有扫描到任何消息键引用，检查扫描规则")
        missing = ["%s (%s)" % (key, where) for key, where in sorted(referenced.items())
                   if key not in catalog]
        self.assertEqual(missing, [],
                         "代码引用了消息目录中不存在的键：\n" + "\n".join(missing))

    def test_placeholders_substituted(self):
        """占位符必须真的被替换：duilib_t/msg/warn/error 不替换，duilib_tf 要传值。

        `duilib_tf` 只替换调用方给出的 `name=value`；漏传时用户会看到字面量
        `{source}`（in-tree 报错曾这样）。反过来，不带占位符替换的接口一旦遇到
        含 `{name}` 的文案，也会原样输出。
        """
        catalog = self.catalog()
        placeholder_re = re.compile(r"\{([a-z0-9_]+)\}")
        provided_re = re.compile(r'"([a-z0-9_]+)=')
        # duilib_tf 的参数可能跨行，取到本次调用的右括号为止
        tf_re = re.compile(r'duilib_tf\(\s*[^"\s]+\s*\n?\s*"([^"]+)"(.*?)\)',
                           re.S)
        plain_re = (
            re.compile(r'duilib_t\(\s*"([^"]+)"'),
            re.compile(r'duilib_msg\(\s*"([^"]+)"'),
            re.compile(r'duilib_warn\(\s*"([^"]+)"'),
            re.compile(r'duilib_error\(\s*"([^"]+)"'),
        )

        def text_of(key, language):
            entry = catalog.get(key) or {}
            return str(entry.get(language, entry.get("en", "")))

        problems = []
        for path in self._sources():
            if path.suffix != ".cmake" and path.name != "CMakeLists.txt":
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            for match in tf_re.finditer(text):
                key = match.group(1)
                if key not in catalog:
                    continue
                needed = set(placeholder_re.findall(text_of(key, "en")))
                needed |= set(placeholder_re.findall(text_of(key, "zh_CN")))
                given = set(provided_re.findall(match.group(2)))
                for missing in sorted(needed - given):
                    line = text.count("\n", 0, match.start()) + 1
                    problems.append("%s:%d: %s 缺少参数 %s=" % (
                        path.relative_to(REPO_ROOT), line, key, missing))
            for pattern in plain_re:
                for match in pattern.finditer(text):
                    key = match.group(1)
                    if key not in catalog:
                        continue
                    for language in ("en", "zh_CN"):
                        if placeholder_re.search(text_of(key, language)):
                            line = text.count("\n", 0, match.start()) + 1
                            problems.append("%s:%d: %s 含占位符但用了不替换的接口" % (
                                path.relative_to(REPO_ROOT), line, key))
                            break
        self.assertEqual(problems, [], "占位符没有被正确替换：\n" + "\n".join(problems))

    MARKER_RE = re.compile(r"^\[[A-Za-z0-9]+\]")

    def test_markers_not_duplicated(self):
        """状态标记只出现一次。

        `_ok_line` / `_warn_line` / `_info_line` 自己会加 `[OK]`/`[WARN]`/`[INFO]`
        前缀，CMake 里的 `message(STATUS "[WARN]  ${_t}")` 同理：这类调用里的文案
        不能再以标记开头，否则用户会看到 `[WARN]  [WARN] ...`（doctor 的
        cmake_missing 曾这样）。
        """
        catalog = self.catalog()
        problems = []

        def text_of(key, language):
            entry = catalog.get(key) or {}
            return str(entry.get(language, ""))

        def check(key, where):
            for language in ("en", "zh_CN"):
                if self.MARKER_RE.match(text_of(key, language)):
                    problems.append("%s: %s 的文案自带 %s 标记" % (
                        where, key, language))
                    return

        # Python：包装函数会加前缀
        py_wrap_re = re.compile(r"_(?:ok|warn|info)_line\(")
        py_key_re = re.compile(r"messages\.t\(\s*\"([^\"]+)\"\s*\)")
        for path in sorted(BUILD_DIR.glob("*.py")):
            if path.name.startswith("test_"):
                continue
            for number, line in enumerate(path.read_text(encoding="utf-8",
                                                         errors="replace").splitlines(), 1):
                if not py_wrap_re.search(line):
                    continue
                for key in py_key_re.findall(line):
                    if key in catalog:
                        check(key, "%s:%d" % (path.relative_to(REPO_ROOT), number))

        # CMake：`message(STATUS "[TOKEN]  ${var}")`，var 取自上一条 duilib_t/duilib_tf
        cmake_msg_re = re.compile(
            r'message\s*\(\s*(?:STATUS|WARNING)\s*"[^"]*\[[A-Za-z0-9]+\][^"]*\$\{([A-Za-z0-9_]+)\}"')
        for path in sorted((REPO_ROOT / "cmake").rglob("*.cmake")):
            text = path.read_text(encoding="utf-8", errors="replace")
            for match in cmake_msg_re.finditer(text):
                var = match.group(1)
                before = text[:match.start()]
                assigns = list(re.finditer(
                    r'duilib_t\(\s*"([^"]+)"\s*%s\)|duilib_tf\(\s*%s\s*"([^"]+)"' % (var, var),
                    before))
                if not assigns:
                    continue
                key = assigns[-1].group(1) or assigns[-1].group(2)
                if key in catalog:
                    line = text.count("\n", 0, match.start()) + 1
                    check(key, "%s:%d" % (path.relative_to(REPO_ROOT), line))
        self.assertEqual(problems, [], "状态标记被重复添加：\n" + "\n".join(problems))

    def test_no_orphan_keys(self):
        """目录里的每个键都必须在源码中出现，避免留下无人引用的死文案。

        这里只要求键名作为字符串出现过（键名也可能作为数据传递，例如
        `SCENARIO_ORDER` 里的 `sdk.scn_p01`），因此不能复用“调用点”正则：
        文案被删除时调用点也会一起消失，检查仍然有效。
        """
        catalog = self.catalog()
        texts = [path.read_text(encoding="utf-8", errors="replace")
                 for path in self._sources()]
        orphans = [key for key in catalog if not any(key in text for text in texts)]
        self.assertEqual(orphans, [], "消息目录中有未被引用的键：\n" + "\n".join(orphans))


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


class PackCommandTests(unittest.TestCase):
    """`build.py pack` 必须把依赖根转发给 pack.py。"""

    def _command(self, argv):
        captured = []

        def fake_run(command, **_kwargs):
            captured.append([str(part) for part in command])
            return SimpleNamespace(returncode=0)

        with mock.patch.object(entry.subprocess, "run", side_effect=fake_run), \
                contextlib.redirect_stdout(io.StringIO()):
            code = entry.main(["--lang", "en"] + argv)
        self.assertEqual(code, 0)
        self.assertTrue(captured)
        return captured[-1]

    def test_default_deps_root_is_forwarded(self):
        command = self._command(["pack"])
        self.assertIn("--deps-root", command)
        index = command.index("--deps-root")
        self.assertEqual(command[index + 1], str(entry.REPO_ROOT.parent.resolve()))

    def test_explicit_deps_root_is_forwarded(self):
        with tempfile.TemporaryDirectory() as root:
            command = self._command(["pack", "--deps-root", root])
            index = command.index("--deps-root")
            self.assertEqual(command[index + 1], str(Path(root).resolve()))

    def test_language_is_forwarded_to_pack(self):
        """pack.py 不会继承 --lang，必须显式转发解析后的语言。"""
        command = self._command(["pack"])
        index = command.index("--lang")
        self.assertEqual(command[index + 1], "en")
        # 命令行里后出现的 --lang 生效，转发的是解析后的值而不是字面量 auto
        command = self._command(["pack", "--lang", "zh_CN"])
        index = command.index("--lang")
        self.assertEqual(command[index + 1], "zh_CN")


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
    """Skia 补丁归档随仓库分发：路径、缺失处理与 SHA1.txt 校验。"""

    def _messages(self):
        return entry.make_messages("zh_CN")

    def test_manifest_patch_dir(self):
        manifest = entry.load_manifest(entry.VERSIONS_FILE)
        self.assertEqual(manifest["skia"]["patchDir"], entry.DEFAULT_SKIA_PATCH_DIR)

    def test_patch_archive_is_in_repository(self):
        manifest = entry.load_manifest(entry.VERSIONS_FILE)
        args = entry.parse_args(["deps"])
        patch = entry._skia_patch_archive(manifest, args)
        self.assertIsNotNone(patch)
        self.assertEqual(patch.parent,
                         (entry.REPO_ROOT / entry.DEFAULT_SKIA_PATCH_DIR).resolve())
        self.assertTrue(patch.is_file(), patch)

    def test_shipped_patch_matches_manifest(self):
        manifest = entry.load_manifest(entry.VERSIONS_FILE)
        args = entry.parse_args(["deps"])
        patch = entry._skia_patch_archive(manifest, args)
        ref = entry._skia_patch_ref(manifest, args)
        self.assertEqual(entry.skia_patch_sha1(patch), manifest["skia"]["default"].lower())
        self.assertTrue(entry.skia_patch_matches(patch, ref))
        mismatch = dict(ref, value="0" * 40)
        self.assertFalse(entry.skia_patch_matches(patch, mismatch))

    def test_missing_patch_raises_without_artifacts(self):
        with tempfile.TemporaryDirectory() as root:
            args = entry.parse_args(["deps", "--deps-root", root])
            missing = Path(root) / "skia.2026-10-30.src.zip"
            with mock.patch.object(entry, "_skia_patch_archive", return_value=missing):
                with self.assertRaises(entry.BuildError):
                    entry._ensure_skia_patch(args, self._messages(), mock.Mock())

    def test_missing_patch_is_only_a_warning_with_artifacts(self):
        with tempfile.TemporaryDirectory() as root:
            variant = Path(root) / "skia" / "out" / "llvm.x64.release"
            variant.mkdir(parents=True)
            (variant / "skia.lib").write_text("x", encoding="utf-8")
            args = entry.parse_args(["deps", "--deps-root", root])
            missing = Path(root) / "skia.2026-10-30.src.zip"
            with mock.patch.object(entry, "_skia_patch_archive", return_value=missing):
                # 已有 Skia 产物：缺归档只告警，不中断构建
                stream = io.StringIO()
                with contextlib.redirect_stdout(stream):
                    entry._ensure_skia_patch(args, self._messages(), mock.Mock())
            output = stream.getvalue()
            self.assertEqual(output.count("[WARN]"), 1)
            # 告警路径不能复用 [ERROR] 文案，否则会输出 “[WARN]  [ERROR] …”
            self.assertNotIn("[ERROR]", output)

    def test_sha1_mismatch_raises(self):
        with tempfile.TemporaryDirectory() as root:
            patch = Path(root) / "skia-patch.zip"
            with zipfile.ZipFile(patch, "w") as archive:
                archive.writestr("SHA1.txt", "SHA-1:0000000000")
            args = entry.parse_args(["deps", "--deps-root", root])
            with mock.patch.object(entry, "_skia_patch_archive", return_value=patch):
                with self.assertRaises(entry.BuildError):
                    entry._ensure_skia_patch(args, self._messages(), mock.Mock())


class GitRefTests(unittest.TestCase):
    """依赖源码与清单 ref 的一致性检查（需要 git）。"""

    def setUp(self):
        if not shutil.which("git"):
            self.skipTest("git not available")

    def _git(self, repo: Path, *args: str) -> str:
        completed = subprocess.run(
            ["git", "-C", str(repo), "-c", "user.name=test", "-c", "user.email=test@example.com",
             *args], capture_output=True, text=True, check=True)
        return completed.stdout.strip()

    def _repo(self, root: Path) -> Path:
        repo = root / "repo"
        repo.mkdir()
        self._git(repo, "init", "-q")
        (repo / "sample.txt").write_bytes(b"first")
        self._git(repo, "add", ".")
        self._git(repo, "commit", "-q", "-m", "sample")
        return repo

    def test_resolve_git_ref_commit_and_tag(self):
        with tempfile.TemporaryDirectory() as root:
            repo = self._repo(Path(root))
            head = entry.git_head(repo)
            self.assertTrue(head)
            missing = {"type": "tag", "value": "release-3.4.18"}
            self.assertEqual(entry.resolve_git_ref(repo, missing), "")
            self._git(repo, "tag", "release-3.4.18")
            self.assertEqual(entry.resolve_git_ref(repo, missing), head)
            self.assertEqual(entry.resolve_git_ref(repo, {"type": "commit", "value": head}), head)

    def test_resolve_git_ref_branch(self):
        with tempfile.TemporaryDirectory() as root:
            repo = self._repo(Path(root))
            head = entry.git_head(repo)
            branch = self._git(repo, "rev-parse", "--abbrev-ref", "HEAD")
            self.assertEqual(entry.resolve_git_ref(repo, {"type": "branch", "value": branch}), head)
            self.assertEqual(entry.resolve_git_ref(repo, {"type": "branch", "value": "nope"}), "")

    def test_git_head_ignores_plain_directory(self):
        with tempfile.TemporaryDirectory() as root:
            self.assertEqual(entry.git_head(Path(root)), "")


if __name__ == "__main__":
    unittest.main()
