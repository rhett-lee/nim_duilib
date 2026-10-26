# -*- coding: utf-8 -*-
"""build/sdk_package_test.py 的纯函数单元测试（不需要真实发布包）。"""

import importlib.util
import subprocess
import sys
import unittest
from pathlib import Path
from unittest import mock

MODULE_PATH = Path(__file__).resolve().parent / "sdk_package_test.py"
_SPEC = importlib.util.spec_from_file_location("sdk_package_test_under_test", MODULE_PATH)
sdk = importlib.util.module_from_spec(_SPEC)
# dataclasses 需要模块先注册到 sys.modules 才能解析类型注解。
sys.modules[_SPEC.name] = sdk
_SPEC.loader.exec_module(sdk)


class PackageNameTest(unittest.TestCase):
    def test_release_static(self):
        parts = sdk.parse_package_name("nim_duilib-1.0.2026.1023-win-x64.zip")
        self.assertEqual(parts["version"], "1.0.2026.1023")
        self.assertEqual(parts["system"], "win")
        self.assertEqual(parts["arch"], "x64")
        self.assertEqual(parts["config"], "Release")
        self.assertEqual(parts["toolchain"], "msvc")
        self.assertFalse(parts["is_dll"])
        self.assertFalse(parts["has_sdl"])
        self.assertFalse(parts["has_cef"])
        self.assertFalse(parts["has_examples"])

    def test_debug_sdl_cef_examples(self):
        parts = sdk.parse_package_name(
            "nim_duilib-1.0.2026.1023-win-x64-sdl-cef109-examples-debug.zip")
        self.assertEqual(parts["config"], "Debug")
        self.assertTrue(parts["has_sdl"])
        self.assertTrue(parts["has_cef"])
        self.assertEqual(parts["cef_version"], "109")
        self.assertTrue(parts["has_examples"])

    def test_dll_md(self):
        parts = sdk.parse_package_name(
            "nim_duilib-1.0.2026.1023-win-x64-dll-debug.zip")
        self.assertTrue(parts["is_dll"])

    def test_mingw_variants(self):
        gcc = sdk.parse_package_name(
            "nim_duilib-1.0.2026.1023-win-x64-mingw-gcc.zip")
        llvm = sdk.parse_package_name(
            "nim_duilib-1.0.2026.1023-win-x64-mingw-llvm.zip")
        self.assertEqual(gcc["toolchain"], "mingw")
        self.assertEqual(gcc["mingw_variant"], "gcc")
        self.assertEqual(llvm["mingw_variant"], "llvm")

    def test_unix_package_has_sdl_by_default(self):
        parts = sdk.parse_package_name(
            "nim_duilib-1.0.2026.1023-linux-x64.tar.bz2")
        self.assertEqual(parts["system"], "linux")
        self.assertTrue(parts["has_sdl"])
        self.assertEqual(parts["toolchain"], "")

    def test_unknown_name(self):
        self.assertEqual(sdk.parse_package_name("random.zip"), {})

    def test_strip_archive_ext(self):
        self.assertEqual(sdk.strip_archive_ext("a.tar.bz2"), "a")
        self.assertEqual(sdk.strip_archive_ext("a.tgz"), "a")
        self.assertEqual(sdk.strip_archive_ext("a.zip"), "a")


class MetadataParsingTest(unittest.TestCase):
    def test_parse_cmake_info(self):
        text = '\n'.join([
            'set(NIM_DUILIB_PACKAGE_VERSION "1.2.3")',
            'set(NIM_DUILIB_PACKAGE_IS_DLL 1)',
            'set(NIM_DUILIB_PACKAGE_RUNTIME_DLLS "duilib.dll;SDL3.dll")',
        ])
        info = sdk.parse_cmake_info(text)
        self.assertEqual(info["NIM_DUILIB_PACKAGE_VERSION"], "1.2.3")
        self.assertEqual(info["NIM_DUILIB_PACKAGE_IS_DLL"], "1")
        self.assertEqual(info["NIM_DUILIB_PACKAGE_RUNTIME_DLLS"],
                         "duilib.dll;SDL3.dll")

    def test_parse_manifest_fields(self):
        text = '\n'.join([
            'nim_duilib SDK',
            '=' * 60,
            '版本      : 1.0.2026.1023',
            '包名      : nim_duilib-1.0.2026.1023-win-x64',
            '平台      : win-x64',
            '配置      : Debug',
            '工具链    : MSVC',
            '包含示例  : 是',
            '',
            '目录结构',
            '-' * 60,
            '  include/duilib/**.h   对外头文件',
            '  用法：#include "duilib/duilib.h"',
        ])
        fields = sdk.parse_manifest_fields(text)
        self.assertEqual(fields["版本"], "1.0.2026.1023")
        self.assertEqual(fields["平台"], "win-x64")
        self.assertEqual(fields["配置"], "Debug")
        self.assertEqual(fields["包含示例"], "是")
        self.assertNotIn("include/duilib/**.h", fields)

    def test_flag_value(self):
        self.assertTrue(sdk.flag_value("1"))
        self.assertTrue(sdk.flag_value("ON"))
        self.assertFalse(sdk.flag_value("0"))
        self.assertFalse(sdk.flag_value(""))


class CapabilityTest(unittest.TestCase):
    MSVC_STATIC_CEF = [
        "include/duilib/duilib.h",
        "lib/duilib.lib",
        "lib/duilib-cximage.lib",
        "lib/duilib-webp.lib",
        "lib/duilib-png.lib",
        "lib/duilib-zlib.lib",
        "lib/skia.lib",
        "lib/libcef.lib",
        "lib/libcef_dll_wrapper.lib",
        "lib/SDL3-static.lib",
        "lib/WebView2LoaderStatic.lib",
        "lib/turbojpeg-static.lib",
        "bin/libcef_win_109/x64/libcef.dll",
        "bin/libcef_win_109/x64/locales/en-US.pak",
        "bin/resources/themes/default/global.xml",
        "bin/basic.exe",
    ]

    def test_msvc_static_cef(self):
        caps = sdk.infer_capabilities(self.MSVC_STATIC_CEF, "msvc")
        self.assertFalse(caps["is_dll"])
        self.assertTrue(caps["has_sdl"])
        self.assertEqual(caps["sdl_kind"], "static")
        self.assertTrue(caps["has_cef"])
        self.assertEqual(caps["cef_src"], "libcef_win_109")
        self.assertTrue(caps["has_webview2"])
        self.assertTrue(caps["has_jpeg"])
        self.assertTrue(caps["has_examples"])
        self.assertTrue(caps["has_resources"])

    def test_dll_without_optional(self):
        files = [
            "include/duilib/duilib.h",
            "lib/duilib.lib",
            "lib/WebView2LoaderStatic.lib",
            "bin/duilib.dll",
            "bin/resources/themes/default/global.xml",
        ]
        caps = sdk.infer_capabilities(files, "msvc")
        self.assertTrue(caps["is_dll"])
        self.assertFalse(caps["has_sdl"])
        self.assertFalse(caps["has_cef"])
        self.assertFalse(caps["has_jpeg"])
        self.assertFalse(caps["has_examples"])

    def test_linux_sdl_shared_and_cef(self):
        files = [
            "include/duilib/duilib.h",
            "include/SDL3/SDL.h",
            "lib/libduilib.a",
            "lib/libduilib-cximage.a",
            "lib/libskia.a",
            "lib/libcef_dll_wrapper.a",
            "lib/libSDL3.so",
            "bin/libcef_linux/libcef.so",
            "bin/resources/themes/default/global.xml",
            "bin/basic",
        ]
        caps = sdk.infer_capabilities(files, "")
        self.assertFalse(caps["is_dll"])
        self.assertTrue(caps["has_sdl"])
        self.assertEqual(caps["sdl_kind"], "shared")
        self.assertTrue(caps["has_cef"])
        self.assertEqual(caps["cef_src"], "libcef_linux")
        self.assertTrue(caps["has_examples"])

    def test_mingw_sdl_import_lib(self):
        files = [
            "lib/libduilib.a",
            "lib/libSDL3.dll.a",
            "bin/SDL3.dll",
            "bin/resources/themes/default/global.xml",
        ]
        caps = sdk.infer_capabilities(files, "mingw")
        self.assertTrue(caps["has_sdl"])
        self.assertEqual(caps["sdl_kind"], "shared")
        self.assertFalse(caps["is_dll"])

    def test_lib_stems(self):
        stems = sdk._package_lib_stems(
            {"duilib.lib", "libskia.a", "libSDL3.dll.a", "libcef_dll_wrapper.a"})
        self.assertEqual(stems, {"duilib", "skia", "SDL3", "cef_dll_wrapper"})


class ArchiveRootTest(unittest.TestCase):
    def test_single_root(self):
        self.assertEqual(
            sdk.select_archive_root(["pkg/MANIFEST.txt", "pkg/lib/a.lib"]), "pkg")

    def test_multiple_roots(self):
        with self.assertRaises(ValueError):
            sdk.select_archive_root(["a/x", "b/y"])

    def test_empty(self):
        with self.assertRaises(ValueError):
            sdk.select_archive_root([])


class ApplicabilityTest(unittest.TestCase):
    STATIC = {"is_dll": False, "has_examples": True}
    DLL = {"is_dll": True, "has_examples": False}

    def test_package_scenarios_always_run(self):
        for sid in ("P01", "P07", "C09"):
            ok, reason = sdk.scenario_applicability(
                sid, self.STATIC, {}, no_build=True)
            self.assertTrue(ok, reason)

    def test_static_consumer(self):
        ok, reason = sdk.scenario_applicability("C01", self.STATIC, {})
        self.assertTrue(ok, reason)
        ok, reason = sdk.scenario_applicability("C02", self.STATIC, {})
        self.assertFalse(ok)
        self.assertIn("C01", reason)

    def test_dll_consumer(self):
        ok, _ = sdk.scenario_applicability("C02", self.DLL, {})
        self.assertTrue(ok)
        ok, reason = sdk.scenario_applicability("C01", self.DLL, {})
        self.assertFalse(ok)
        self.assertIn("C02", reason)

    def test_no_build(self):
        ok, reason = sdk.scenario_applicability(
            "C01", self.STATIC, {}, no_build=True)
        self.assertFalse(ok)
        self.assertIn("--no-build", reason)

    def test_examples_gate(self):
        ok, reason = sdk.scenario_applicability("C10", self.STATIC, {})
        self.assertFalse(ok)
        self.assertIn("--run-examples", reason)
        ok, _ = sdk.scenario_applicability(
            "C10", self.STATIC, {}, run_examples=True)
        self.assertTrue(ok)

    def test_build_block_reason(self):
        ok, reason = sdk.scenario_applicability(
            "C01", self.STATIC, {}, build_block_reason="平台不同")
        self.assertFalse(ok)
        self.assertEqual(reason, "平台不同")

    def test_macos_bundle_requires_macos(self):
        caps = {"is_dll": False, "has_sdl": True, "sdl_kind": "shared"}
        with mock.patch.object(sdk, "host_system", return_value="linux"):
            ok, reason = sdk.scenario_applicability("C11", caps, {})
        self.assertFalse(ok)
        self.assertIn("macOS", reason)

    def test_macos_bundle_requires_shared_sdl(self):
        with mock.patch.object(sdk, "host_system", return_value="macos"):
            for caps, expected in (
                    ({"is_dll": False, "has_sdl": False, "sdl_kind": ""}, False),
                    ({"is_dll": False, "has_sdl": True, "sdl_kind": "static"}, False),
                    ({"is_dll": False, "has_sdl": True, "sdl_kind": "shared"}, True)):
                ok, reason = sdk.scenario_applicability("C11", caps, {})
                self.assertEqual(ok, expected, reason)
                if not expected:
                    self.assertIn("SDL3", reason)

    def test_macos_bundle_build_blocked(self):
        caps = {"is_dll": False, "has_sdl": True, "sdl_kind": "shared"}
        with mock.patch.object(sdk, "host_system", return_value="macos"):
            ok, reason = sdk.scenario_applicability(
                "C11", caps, {}, build_block_reason="平台不同")
        self.assertFalse(ok)
        self.assertEqual(reason, "平台不同")

    def test_macos_bundle_no_build(self):
        caps = {"is_dll": False, "has_sdl": True, "sdl_kind": "shared"}
        with mock.patch.object(sdk, "host_system", return_value="macos"):
            ok, reason = sdk.scenario_applicability(
                "C11", caps, {}, no_build=True)
        self.assertFalse(ok)
        self.assertIn("--no-build", reason)


class SelectionTest(unittest.TestCase):
    def test_all(self):
        self.assertEqual(sdk.parse_selection("all"),
                         {item[0] for item in sdk.SCENARIO_ORDER})

    def test_subset(self):
        self.assertEqual(sdk.parse_selection("p01, c01"), {"P01", "C01"})

    def test_unknown(self):
        with self.assertRaises(SystemExit):
            sdk.parse_selection("X99")


def _print_help() -> None:
    """打印本测试脚本的用途与用法。

    直接交给 unittest.main() 时，--help 只显示 unittest 自己的参数说明，
    看不出本脚本测什么、不测什么，因此这里单独输出。
    """
    print("build/test_sdk_package.py：sdk_package_test.py 的纯函数单元测试")
    print()
    print("覆盖范围：")
    print("  - 包名解析，CMake 元数据与 MANIFEST 字段解析")
    print("  - 能力推断（静态/DLL，SDL/CEF/WebView2/JPEG）")
    print("  - 归档根选择、场景适用性、场景编号选择")
    print()
    print("不做什么：不读取真实 SDK 包，不调用 CMake，也不编译、链接或运行任何 C++ 代码。")
    print("要验证真实 SDK 包（含消费者编译/链接/运行），请使用：")
    print("  python3 build/sdk_package_test.py --package <发布包>")
    print("查看可执行场景：")
    print("  python3 build/sdk_package_test.py --list-scenarios")
    print()
    print("其余参数仍由 unittest 解析：-v、-q、-k <模式>、<测试类或测试方法>。")
    print("若误把编排器参数（如 --package）传给本脚本，会自动转交给")
    print("build/sdk_package_test.py。")


# 只有 sdk_package_test.py（用例编排器）认识的选项。两个脚本名字相近，用户很容易
# 把两者看混；这里检测到编排器参数就转交，而不是让 unittest 报 unrecognized arguments。
ORCHESTRATOR_OPTIONS = {
    "--package", "--scenarios", "--no-build", "--run-examples",
    "--out-dir", "--keep", "--jobs", "--dry-run", "--list-scenarios",
}


def _is_orchestrator_invocation(argv: list) -> bool:
    for arg in argv:
        if arg.split("=", 1)[0] in ORCHESTRATOR_OPTIONS:
            return True
    return False


def _run_orchestrator(argv: list) -> int:
    orchestrator = Path(__file__).resolve().with_name("sdk_package_test.py")
    print("build/test_sdk_package.py 只是纯函数单元测试，正在转交给用例编排器：")
    print("  python3 %s %s" % (orchestrator, " ".join(argv)))
    sys.stdout.flush()
    try:
        return subprocess.call([sys.executable, str(orchestrator), *argv])
    except OSError as exc:
        print("无法启动 %s：%s" % (orchestrator, exc))
        return 2


if __name__ == "__main__":
    _argv = sys.argv[1:]
    if any(arg in ("-h", "--help") for arg in _argv):
        _print_help()
    elif _is_orchestrator_invocation(_argv):
        sys.exit(_run_orchestrator(_argv))
    else:
        unittest.main()
