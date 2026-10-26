# -*- coding: utf-8 -*-
"""build/sdk_package_test.py 的纯函数单元测试（不需要真实发布包）。"""

import importlib.util
import sys
import unittest
from pathlib import Path

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


class SelectionTest(unittest.TestCase):
    def test_all(self):
        self.assertEqual(sdk.parse_selection("all"),
                         {item[0] for item in sdk.SCENARIO_ORDER})

    def test_subset(self):
        self.assertEqual(sdk.parse_selection("p01, c01"), {"P01", "C01"})

    def test_unknown(self):
        with self.assertRaises(SystemExit):
            sdk.parse_selection("X99")


if __name__ == "__main__":
    unittest.main()
