# -*- coding: utf-8 -*-
"""build/pack.py 的最小回归测试：`--verify` 与包名规则。

不依赖已构建产物，可直接运行：

    python3 build/test_pack.py

覆盖：正/负向包体检（zip 与 tar.gz）、包名组合、SOURCE_DATE_EPOCH 校验。
"""
import contextlib
import hashlib
import io
import os
import shutil
import struct
import sys
import tarfile
import tempfile
import unittest
import zipfile
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import pack  # noqa: E402

# 固定简体中文：采集/体检的告警断言基于中文，不能随宿主系统语言变化。
pack.MESSAGES = pack.make_messages('zh_CN')


def _payload():
    return {
        'include/duilib/duilib.h': b'#pragma once\n',
        'lib/libduilib.a': b'!<arch>\n',
        'MANIFEST.txt': b'Version : 1.0.0\n',
        'docs/Summary.md': b'[header](../include/duilib/duilib.h)\n',
    }


def _sums(payload):
    lines = []
    for rel, data in sorted(payload.items()):
        if rel in ('MANIFEST.txt', 'VERSION'):
            continue
        lines.append('%s  %s' % (hashlib.sha256(data).hexdigest(), rel))
    return ('\n'.join(lines) + '\n').encode('utf-8')


class VerifyPackageTest(unittest.TestCase):

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix='pack_verify_')

    def tearDown(self):
        for name in os.listdir(self.tmp):
            os.unlink(os.path.join(self.tmp, name))
        os.rmdir(self.tmp)

    def _verify(self, path):
        """执行体检并捕获输出；负向用例的报错属于预期，不应污染测试输出。"""
        stream = io.StringIO()
        with contextlib.redirect_stdout(stream), contextlib.redirect_stderr(stream):
            ok = pack.verify_package(path)
        return ok, stream.getvalue()

    def _make_zip(self, payload, name='pkg.zip', omit=(), tamper=None):
        path = os.path.join(self.tmp, name)
        with zipfile.ZipFile(path, 'w') as zf:
            for rel, data in payload.items():
                if rel in omit:
                    continue
                if tamper and rel == tamper[0]:
                    data = tamper[1]
                zf.writestr('pkg-1.0/' + rel, data)
            zf.writestr('pkg-1.0/SHA256SUMS', _sums(payload))
        return path

    def _make_tgz(self, payload):
        path = os.path.join(self.tmp, 'pkg.tar.gz')
        with tarfile.open(path, 'w:gz') as tf:
            for rel, data in payload.items():
                info = tarfile.TarInfo('pkg-1.0/' + rel)
                info.size = len(data)
                tf.addfile(info, _file_like(data))
            sums = _sums(payload)
            info = tarfile.TarInfo('pkg-1.0/SHA256SUMS')
            info.size = len(sums)
            tf.addfile(info, _file_like(sums))
        return path

    def test_zip_ok(self):
        ok, output = self._verify(self._make_zip(_payload()))
        self.assertTrue(ok)
        self.assertIn('[OK]', output)

    def test_tgz_ok(self):
        ok, output = self._verify(self._make_tgz(_payload()))
        self.assertTrue(ok)
        self.assertIn('[OK]', output)

    def test_missing_manifest(self):
        ok, output = self._verify(self._make_zip(_payload(), omit=('MANIFEST.txt',)))
        self.assertFalse(ok)
        self.assertIn('[FAIL]', output)

    def test_tampered_file(self):
        ok, output = self._verify(
            self._make_zip(_payload(), tamper=('docs/Summary.md', b'changed\n')))
        self.assertFalse(ok)
        self.assertIn('[FAIL]', output)

    def test_missing_file(self):
        ok, output = self._verify(os.path.join(self.tmp, 'not-exists.zip'))
        self.assertFalse(ok)
        self.assertIn('[ERROR]', output)

    def test_corrupt_zip(self):
        path = os.path.join(self.tmp, 'broken.zip')
        with open(path, 'wb') as fp:
            fp.write(b'not a zip')
        ok, output = self._verify(path)
        self.assertFalse(ok)
        self.assertIn('[ERROR]', output)

    def test_broken_doc_link(self):
        payload = _payload()
        payload['docs/Summary.md'] = b'[bad](missing.md)\n'
        ok, output = self._verify(self._make_zip(payload))
        self.assertFalse(ok)
        self.assertIn('[FAIL]', output)

    def test_broken_doc_link_tgz(self):
        payload = _payload()
        payload['docs/Summary.md'] = b'[bad](../build/missing.md)\n'
        ok, output = self._verify(self._make_tgz(payload))
        self.assertFalse(ok)
        self.assertIn('[FAIL]', output)


def _file_like(data):
    """tarfile.addfile 需要一个可读的文件对象。"""
    import io
    return io.BytesIO(data)


class PackageNameTest(unittest.TestCase):

    def test_windows_variants(self):
        name = pack.build_package_name('1.0', 'win', 'x64', 'Release',
                                       False, False, 'latest', False,
                                       mingw_variant='gcc')
        self.assertEqual(name, 'nim_duilib-1.0-win-x64-mingw-gcc')
        name = pack.build_package_name('1.0', 'win', 'x64', 'Debug',
                                       True, False, 'latest', True,
                                       mingw_variant='llvm')
        self.assertEqual(name,
                         'nim_duilib-1.0-win-x64-mingw-llvm-sdl-examples-debug')

    def test_msvc_and_cef(self):
        name = pack.build_package_name('1.0', 'win', 'x86', 'Release',
                                       False, True, '109', False)
        self.assertEqual(name, 'nim_duilib-1.0-win-x86-cef109')

    def test_unix_has_no_sdl_suffix(self):
        name = pack.build_package_name('1.0', 'linux', 'x64', 'Release',
                                       True, False, 'latest', False)
        self.assertEqual(name, 'nim_duilib-1.0-linux-x64')

    def test_dll_mode_suffix(self):
        name = pack.build_package_name('1.0', 'win', 'x64', 'Release',
                                       False, False, 'latest', False,
                                       is_dll=True)
        self.assertEqual(name, 'nim_duilib-1.0-win-x64-dll')
        name = pack.build_package_name('1.0', 'win', 'x64', 'Debug',
                                       False, False, 'latest', True,
                                       is_dll=True)
        self.assertEqual(name, 'nim_duilib-1.0-win-x64-examples-dll-debug')

    def test_arm64_variants(self):
        name = pack.build_package_name('1.0', 'win', 'arm64', 'Release',
                                       False, False, 'latest', False)
        self.assertEqual(name, 'nim_duilib-1.0-win-arm64')
        name = pack.build_package_name('1.0', 'win', 'arm64', 'Debug',
                                       True, False, 'latest', True,
                                       mingw_variant='gcc', is_dll=True)
        self.assertEqual(
            name, 'nim_duilib-1.0-win-arm64-mingw-gcc-sdl-examples-dll-debug')


class SourceDateEpochTest(unittest.TestCase):

    def test_invalid_value(self):
        old = os.environ.get('SOURCE_DATE_EPOCH')
        os.environ['SOURCE_DATE_EPOCH'] = 'not-a-number'
        try:
            with self.assertRaises(pack.PackageError):
                pack.read_source_date_epoch()
        finally:
            if old is None:
                os.environ.pop('SOURCE_DATE_EPOCH', None)
            else:
                os.environ['SOURCE_DATE_EPOCH'] = old

    def test_negative_value(self):
        old = os.environ.get('SOURCE_DATE_EPOCH')
        os.environ['SOURCE_DATE_EPOCH'] = '-1'
        try:
            with self.assertRaises(pack.PackageError):
                pack.read_source_date_epoch()
        finally:
            if old is None:
                os.environ.pop('SOURCE_DATE_EPOCH', None)
            else:
                os.environ['SOURCE_DATE_EPOCH'] = old


class _TempDirTest(unittest.TestCase):

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix='pack_extra_')

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _touch(self, rel, data=b''):
        path = os.path.join(self.tmp, *rel.split('/'))
        parent = os.path.dirname(path)
        if parent and not os.path.isdir(parent):
            os.makedirs(parent)
        with open(path, 'wb') as fp:
            fp.write(data)
        return path


class ManifestFilesTest(_TempDirTest):

    def test_all_arch_manifests_exist(self):
        for arch in pack.WINDOWS_MANIFEST_ARCHES:
            path = os.path.join(pack.MSVC_MANIFEST_DIR,
                                'duilib.%s.manifest' % arch)
            self.assertTrue(os.path.isfile(path), path)

    def test_arm64_manifest_architecture(self):
        path = os.path.join(pack.MSVC_MANIFEST_DIR, 'duilib.arm64.manifest')
        with open(path, 'r', encoding='utf-8') as fp:
            text = fp.read()
        self.assertEqual(text.count('processorArchitecture="ARM64"'), 2)
        self.assertIn('Microsoft.Windows.Common-Controls', text)
        self.assertIn('supportedOS', text)

    def test_x64_manifest_architecture(self):
        path = os.path.join(pack.MSVC_MANIFEST_DIR, 'duilib.x64.manifest')
        with open(path, 'r', encoding='utf-8') as fp:
            text = fp.read()
        self.assertIn('processorArchitecture="AMD64"', text)


class IntegrationFilesTest(_TempDirTest):

    def _collector(self, with_dll=False):
        collector = pack.Collector()
        collector.add('lib/duilib.lib', self._touch('lib/duilib.lib'))
        if with_dll:
            collector.add('bin/duilib.dll', self._touch('bin/duilib.dll'))
        return collector

    def _meta(self, arch='x64', collector=None):
        collector = collector or self._collector()
        return pack.build_package_meta(
            collector, '1.0.0', 'win', arch, 'Release', 'msvc',
            False, 'latest', 'auto')

    def test_manifests_and_virtual_files(self):
        collector = self._collector()
        count, virtual = pack.collect_integration_files(collector, self._meta())
        self.assertEqual(count, len(pack.WINDOWS_MANIFEST_ARCHES))
        rels = {rel for rel, _abs in collector.items}
        for arch in pack.WINDOWS_MANIFEST_ARCHES:
            self.assertIn('msvc/manifest/duilib.%s.manifest' % arch, rels)
        for rel in (
                'msvc/nim_duilib.props',
                'msvc/nim_duilib.targets',
                'msvc/nim_duilib.sdk.props',
                'lib/cmake/nim_duilib/nim_duilibConfig.cmake',
                'lib/cmake/nim_duilib/nim_duilibConfigVersion.cmake',
                'lib/cmake/nim_duilib/nim_duilibPackageInfo.cmake'):
            self.assertIn(rel, virtual)

    def test_cmake_config_skips_dlls_outside_bin_root(self):
        # 旧包的元数据会把 CEF 运行时 DLL 摊平列出；生成的 Config 必须按存在性过滤，
        # 否则 nim_duilib_copy_runtime 会复制不存在的 bin/<name>。
        _count, virtual = pack.collect_integration_files(self._collector(),
                                                         self._meta())
        config = virtual['lib/cmake/nim_duilib/nim_duilibConfig.cmake']
        self.assertIn('NIM_DUILIB_PACKAGE_RUNTIME_DLLS', config)
        self.assertIn('EXISTS "${nim_duilib_ROOT}/bin/${_nim_dll}"', config)

    def test_arm64_metadata_in_generated_files(self):
        meta = self._meta(arch='arm64')
        _count, virtual = pack.collect_integration_files(self._collector(), meta)
        sdk_props = virtual['msvc/nim_duilib.sdk.props']
        self.assertIn('<NimDuilibPackageArch>arm64</NimDuilibPackageArch>',
                      sdk_props)
        package_info = virtual[
            'lib/cmake/nim_duilib/nim_duilibPackageInfo.cmake']
        self.assertIn('set(NIM_DUILIB_PACKAGE_ARCH "arm64")', package_info)

    def test_cmake_config_has_arm64_detection(self):
        _count, virtual = pack.collect_integration_files(self._collector(),
                                                         self._meta(arch='arm64'))
        config = virtual['lib/cmake/nim_duilib/nim_duilibConfig.cmake']
        self.assertIn('CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64"',
                      config)

    def test_runtime_dlls_only_from_bin_root(self):
        # CEF 运行时在 bin/libcef_win/<arch>/ 下，不能摊平成 bin/<name> 列进元数据，
        # 否则 CEF 包的消费者执行 nim_duilib_copy_runtime 会复制失败。
        collector = self._collector(with_dll=True)
        collector.add('bin/WebView2Loader.dll',
                      self._touch('bin/WebView2Loader.dll'))
        collector.add('bin/libcef_win/x64/chrome_elf.dll',
                      self._touch('bin/libcef_win/x64/chrome_elf.dll'))
        collector.add('bin/libcef_win/x64/libcef.dll',
                      self._touch('bin/libcef_win/x64/libcef.dll'))
        meta = self._meta(collector=collector)
        self.assertEqual(meta['runtime_dlls'],
                         ['WebView2Loader.dll', 'duilib.dll'])

    def test_msvc_runtime_auto(self):
        self.assertEqual(self._meta()['msvc_runtime'], 'MT')
        meta = self._meta(collector=self._collector(with_dll=True))
        self.assertEqual(meta['msvc_runtime'], 'MD')
        self.assertTrue(meta['is_dll'])

    def test_msvc_runtime_explicit(self):
        collector = self._collector()
        meta = pack.build_package_meta(
            collector, '1.0.0', 'win', 'x64', 'Debug', 'msvc',
            False, 'latest', 'md')
        self.assertEqual(meta['msvc_runtime'], 'MD')

    def test_dll_sdl_feature_requires_with_sdl(self):
        # DLL 以 SDL 编译但打包没加 --with-sdl：下游缺 SDL 头文件、宏与 DLL
        # 的类布局不一致，必须直接报错，不能静默出包。
        collector = pack.Collector()
        collector.add('bin/duilib.dll',
                      self._touch('bin/duilib.dll',
                                  pack.SDL_EXPORT_MARKER.encode('ascii')))
        with self.assertRaises(pack.PackageError):
            pack.build_package_meta(
                collector, '1.0.0', 'win', 'x64', 'Release', 'msvc',
                False, 'latest', 'auto')

    def test_dll_sdl_feature_matches_with_sdl(self):
        collector = pack.Collector()
        collector.add('bin/duilib.dll',
                      self._touch('bin/duilib.dll',
                                  pack.SDL_EXPORT_MARKER.encode('ascii')))
        collector.add('lib/SDL3-static.lib',
                      self._touch('lib/SDL3-static.lib'))
        meta = pack.build_package_meta(
            collector, '1.0.0', 'win', 'x64', 'Release', 'msvc',
            False, 'latest', 'auto', with_sdl=True)
        self.assertTrue(meta['has_sdl'])
        self.assertEqual(meta['sdl_kind'], 'static')

    def test_dll_without_sdl_ignores_with_sdl(self):
        # DLL 本身没有 SDL 时，即使包内带 SDL 库也要按 HAS_SDL=0 生成元数据。
        collector = pack.Collector()
        collector.add('bin/duilib.dll', self._touch('bin/duilib.dll', b'MZ'))
        collector.add('lib/SDL3-static.lib',
                      self._touch('lib/SDL3-static.lib'))
        meta = pack.build_package_meta(
            collector, '1.0.0', 'win', 'x64', 'Release', 'msvc',
            False, 'latest', 'auto', with_sdl=True)
        self.assertFalse(meta['has_sdl'])
        self.assertEqual(meta['sdl_kind'], '')


class Arm64SupportTest(_TempDirTest):

    def _collector(self):
        collector = pack.Collector()
        collector.add('include/duilib/duilib.h',
                      self._touch('include/duilib/duilib.h', b'#pragma once\n'))
        return collector

    def test_skia_dir_detection(self):
        old_root = pack.SKIA_SRC_ROOT
        old_win = pack.is_windows_host
        try:
            pack.SKIA_SRC_ROOT = self.tmp
            pack.is_windows_host = lambda: True
            out_dir = os.path.join(self.tmp, 'out')
            target = os.path.join(out_dir, 'llvm.arm64.release')
            os.makedirs(target)
            self._touch('out/llvm.arm64.release/skia.lib')
            self.assertEqual(
                pack.resolve_skia_lib_dir(None, 'Release', 'arm64', 'msvc'),
                target)
            shutil.rmtree(target)
            with self.assertRaises(pack.PackageError) as ctx:
                pack.resolve_skia_lib_dir(None, 'Release', 'arm64', 'msvc')
            self.assertIn('arm64', str(ctx.exception))
            self.assertIn('llvm.arm64.release', str(ctx.exception))
        finally:
            pack.SKIA_SRC_ROOT = old_root
            pack.is_windows_host = old_win

    def test_jpeg_arm64_is_collected(self):
        old_win = pack.is_windows_host
        old_dir = pack.JPEG_TURBO_DIR
        try:
            pack.is_windows_host = lambda: True
            pack.JPEG_TURBO_DIR = self.tmp
            self._touch('libjpeg-turbo6-win-vc-arm64/lib/turbojpeg-static.lib')
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_jpeg_turbo(collector, 'arm64', 'msvc'), 1)
            self.assertEqual([rel for rel, _abs in collector.items],
                             ['lib/turbojpeg-static.lib'])
            self.assertEqual(collector.warnings, [])
            # MinGW 仍不收集（MSVC 静态库无法链接）。
            self.assertEqual(
                pack.collect_jpeg_turbo(pack.Collector(), 'arm64', 'mingw'), 0)
        finally:
            pack.is_windows_host = old_win
            pack.JPEG_TURBO_DIR = old_dir

    def test_cef_arm64_is_supported(self):
        # 上游已发布 Windows arm64 预编译包（清单默认版本与 109 都有），
        # collect_cef 按 lib/arm64 收集导入库，collect_cef_runtime 按 arm64 子目录。
        old_win = pack.is_windows_host
        old_dir = pack.CEF_DIR
        try:
            pack.is_windows_host = lambda: True
            pack.CEF_DIR = self.tmp
            self._touch('libcef_win/lib/arm64/libcef.lib', b'ARM64LIB')
            self._touch('libcef_win_109/lib/arm64/libcef.lib', b'ARM64LIB109')
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_cef(collector, 'win', 'arm64', 'latest'), 1)
            self.assertEqual([rel for rel, _abs in collector.items],
                             ['lib/libcef.lib'])
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_cef(collector, 'win', 'arm64', '109'), 1)
            self.assertEqual(collector.warnings, [])
            # 运行时：bin/libcef_win/arm64/ 目录存在时按 CEF_RUNTIME_ARCH_DIRS
            # 的 arm64 映射收集，不再告警跳过。
            old_bin = pack.BIN_DIR
            pack.BIN_DIR = self.tmp
            try:
                self._touch('libcef_win/arm64/libcef.dll', b'MZ')
                collector = pack.Collector()
                self.assertEqual(
                    pack.collect_cef_runtime(collector, 'win', 'arm64',
                                             'latest'), 1)
                self.assertEqual(
                    [rel for rel, _abs in collector.items],
                    ['bin/libcef_win/arm64/libcef.dll'])
                self.assertEqual(collector.warnings, [])
            finally:
                pack.BIN_DIR = old_bin
        finally:
            pack.is_windows_host = old_win
            pack.CEF_DIR = old_dir

    def test_manifest_notes_arm64_scope(self):
        collector = self._collector()
        text, _sums = pack.write_manifest(
            collector, 'nim_duilib-1.0-win-arm64', '1.0', 'win', 'arm64',
            'Release', False, False, False, 'latest', toolchain='msvc',
            virtual_files={}, msvc_runtime='MT', package_time=0)
        self.assertIn('平台      : win-arm64', text)
        self.assertIn('arm64 说明', text)
        self.assertIn('未在 arm64 真机上做编译、打包、运行验证', text)
        self.assertIn('CEF 上游提供 Windows arm64 预编译包（%s 与 109）'
                      % pack.cef_default_version(), text)
        self.assertIn('需要 CEF 时加 --with-cef 重新出包', text)
        # --with-cef 出包时说明改为“已收集 CEF 开发文件与运行时”
        text, _sums = pack.write_manifest(
            self._collector(), 'nim_duilib-1.0-win-arm64-cef', '1.0', 'win',
            'arm64', 'Release', False, False, True, 'latest', toolchain='msvc',
            virtual_files={}, msvc_runtime='MT', package_time=0)
        self.assertIn('本包已按 --with-cef 收集 CEF 开发文件与运行时', text)
        self.assertNotIn('需要 CEF 时加 --with-cef 重新出包', text)

    def test_manifest_time_is_deterministic(self):
        collector = self._collector()
        text, _sums = pack.write_manifest(
            collector, 'pkg', '1.0', 'win', 'x64', 'Release',
            False, False, False, 'latest', toolchain='msvc',
            virtual_files={}, msvc_runtime='MT', package_time=0)
        self.assertIn('打包时间  : 1970-01-01 00:00:00 UTC', text)

    def test_manifest_and_sums_cover_virtual_files(self):
        collector = self._collector()
        virtual = {'msvc/nim_duilib.props': 'hello'}
        _text, sums = pack.write_manifest(
            collector, 'pkg', '1.0', 'win', 'x64', 'Release',
            False, False, False, 'latest', toolchain='msvc',
            virtual_files=virtual, msvc_runtime='MT', package_time=0)
        digest = hashlib.sha256(b'hello').hexdigest()
        self.assertIn('%s  msvc/nim_duilib.props' % digest, sums)


class CriticalArtifactsTest(_TempDirTest):

    def _empty_dir(self, name='empty'):
        path = os.path.join(self.tmp, name)
        if not os.path.isdir(path):
            os.makedirs(path)
        return path

    def test_collector_critical_default(self):
        collector = pack.Collector()
        collector.warn('missing', critical=True)
        self.assertEqual(collector.critical, ['missing'])
        self.assertIn('missing', collector.warnings)

    def test_collector_allow_missing_downgrades(self):
        collector = pack.Collector(allow_missing=True)
        collector.warn('missing', critical=True)
        self.assertEqual(collector.critical, [])
        self.assertIn('missing', collector.warnings)

    def test_missing_core_header(self):
        old = pack.DUIBIB_SRC_DIR
        try:
            pack.DUIBIB_SRC_DIR = self._empty_dir()
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_headers(collector, False, False, True), 0)
            self.assertTrue(any('duilib.h' in item
                                for item in collector.critical))
        finally:
            pack.DUIBIB_SRC_DIR = old

    def test_missing_resources(self):
        old = pack.RESOURCES_DIR
        try:
            pack.RESOURCES_DIR = os.path.join(self.tmp, 'no-resources')
            collector = pack.Collector()
            self.assertEqual(pack.collect_resources(collector, False), 0)
            self.assertTrue(any('资源目录' in item
                                for item in collector.critical))
        finally:
            pack.RESOURCES_DIR = old

    def test_missing_manifest(self):
        old = pack.MSVC_MANIFEST_DIR
        try:
            pack.MSVC_MANIFEST_DIR = self._empty_dir()
            collector = pack.Collector()
            collector.add('lib/duilib.lib', self._touch('lib/duilib.lib'))
            meta = pack.build_package_meta(
                collector, '1.0', 'win', 'x64', 'Release', 'msvc',
                False, 'latest', 'auto')
            pack.collect_integration_files(collector, meta)
            self.assertTrue(any('manifest' in item
                                for item in collector.critical))
        finally:
            pack.MSVC_MANIFEST_DIR = old

    def test_missing_sdl_runtime_dll(self):
        old_root = pack.SDL3_SRC_ROOT
        old_win = pack.is_windows_host
        try:
            pack.SDL3_SRC_ROOT = self.tmp
            pack.is_windows_host = lambda: True
            self._touch('lib/SDL3.lib')
            collector = pack.Collector()
            pack.collect_sdl(collector, None, 'Release', True)
            self.assertTrue(any('SDL3' in item and 'DLL' in item
                                for item in collector.critical))
        finally:
            pack.SDL3_SRC_ROOT = old_root
            pack.is_windows_host = old_win

    def test_missing_cef_runtime(self):
        old = pack.BIN_DIR
        try:
            pack.BIN_DIR = self._empty_dir()
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_cef_runtime(collector, 'win', 'x64', 'latest'), 0)
            self.assertTrue(any('CEF 运行时' in item
                                for item in collector.critical))
        finally:
            pack.BIN_DIR = old

    def test_missing_examples(self):
        old = pack.BIN_DIR
        try:
            pack.BIN_DIR = self._empty_dir()
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_examples(collector, 'Release', False), 0)
            self.assertTrue(any('示例程序' in item
                                for item in collector.critical))
        finally:
            pack.BIN_DIR = old

    def test_real_repo_has_no_critical_missing(self):
        collector = pack.Collector()
        pack.collect_headers(collector, False, False, True)
        pack.collect_resources(collector, False)
        collector.add('lib/duilib.lib', self._touch('lib/duilib.lib'))
        meta = pack.build_package_meta(
            collector, '1.0', 'win', 'x64', 'Release', 'msvc',
            False, 'latest', 'auto')
        pack.collect_integration_files(collector, meta)
        self.assertEqual(collector.critical, [])


class CefLinuxPackagingTest(_TempDirTest):
    """Linux 的 CEF 随包：头文件 + wrapper + bin/libcef_linux 运行时。"""

    def _setup_cef(self):
        old_cef, old_bin = pack.CEF_DIR, pack.BIN_DIR
        pack.CEF_DIR = os.path.join(self.tmp, 'cef')
        pack.BIN_DIR = os.path.join(self.tmp, 'bin')
        self._touch('cef/libcef_linux/include/cef_version.h',
                    b'#define CEF_VERSION "1"\n')
        self._touch('bin/libcef_linux/libcef.so')
        self._touch('bin/libcef_linux/icudtl.dat')
        self._touch('bin/libcef_linux/locales/zh-CN.pak')
        return old_cef, old_bin

    def test_collect_headers_and_runtime(self):
        old_cef, old_bin = self._setup_cef()
        try:
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_cef(collector, 'linux', 'x64', 'latest'), 1)
            collector.add('lib/libcef_dll_wrapper.a',
                          self._touch('libcef_dll_wrapper.a'))
            self.assertEqual(
                pack.collect_cef_runtime(collector, 'linux', 'x64', 'latest'), 3)
            rels = [rel for rel, _abs in collector.items]
            self.assertIn('include/duilib/third_party/libcef/libcef_linux/'
                          'include/cef_version.h', rels)
            self.assertNotIn('lib/libcef.so', rels)
            self.assertIn('bin/libcef_linux/libcef.so', rels)
            self.assertIn('bin/libcef_linux/locales/zh-CN.pak', rels)
            self.assertEqual(collector.critical, [])
        finally:
            pack.CEF_DIR, pack.BIN_DIR = old_cef, old_bin

    def test_missing_libcef_so(self):
        old_cef, old_bin = self._setup_cef()
        try:
            os.remove(os.path.join(pack.BIN_DIR, 'libcef_linux', 'libcef.so'))
            with self.assertRaises(pack.PackageError) as ctx:
                pack.collect_cef(pack.Collector(), 'linux', 'x64', 'latest')
            self.assertIn('libcef.so', str(ctx.exception))
        finally:
            pack.CEF_DIR, pack.BIN_DIR = old_cef, old_bin

    def test_macos_collects_nothing(self):
        """macOS 的 CEF 开发文件/运行时不随包，--with-cef 只控制示例 Bundle。"""
        collector = pack.Collector()
        self.assertEqual(pack.collect_cef(collector, 'macos', 'x64', 'latest'), 0)
        self.assertEqual(list(collector.items), [])
        self.assertTrue(any('--with-cef' in msg for msg in collector.warnings))

    def test_package_meta_marks_cef(self):
        collector = pack.Collector()
        collector.add('lib/libcef_dll_wrapper.a',
                      self._touch('libcef_dll_wrapper.a'))
        collector.add('bin/libcef_linux/libcef.so', self._touch('libcef.so'))
        meta = pack.build_package_meta(
            collector, '1.0', 'linux', 'x64', 'Release', None,
            True, 'latest', 'auto')
        self.assertTrue(meta['has_cef'])
        self.assertEqual(meta['cef_src'], 'libcef_linux')
        self.assertEqual(meta['cef_wrapper'], 'libcef_dll_wrapper')

    def test_manifest_documents_linux_runtime(self):
        collector = pack.Collector()
        collector.add('include/duilib/duilib.h', self._touch('duilib.h'))
        collector.add('lib/duilib.a', self._touch('libduilib.a'))
        collector.add('lib/libcef_dll_wrapper.a',
                      self._touch('libcef_dll_wrapper.a'))
        collector.add('bin/libcef_linux/libcef.so', self._touch('libcef.so'))
        text, _sums = pack.write_manifest(
            collector, 'pkg', '1.0', 'linux', 'x64', 'Release',
            False, False, True, 'latest', package_time=0)
        self.assertIn('bin/libcef_linux', text)
        self.assertIn('-Lbin/libcef_linux -lcef', text)
        self.assertIn('LD_LIBRARY_PATH', text)

    def test_wrapper_base_name_without_lib_prefix(self):
        """Unix 的 wrapper 产物被 CMake 去掉 lib 前缀（PREFIX ""），
        文件名为 cef_dll_wrapper.a；_lib_base_name 必须归一为白名单名。"""
        wanted = set(pack.LIB_BASENAMES) | set(pack.CEF_WRAPPER_BY_VERSION.values())
        self.assertEqual(pack._lib_base_name('cef_dll_wrapper', wanted),
                         'libcef_dll_wrapper')
        self.assertEqual(pack._lib_base_name('cef_dll_wrapper_109', wanted),
                         'libcef_dll_wrapper_109')
        # 带前缀与基础库的既有行为不回归
        self.assertEqual(pack._lib_base_name('libcef_dll_wrapper', wanted),
                         'libcef_dll_wrapper')
        self.assertEqual(pack._lib_base_name('libduilib', wanted), 'duilib')
        self.assertEqual(pack._lib_base_name('duilib', wanted), 'duilib')

    def test_collect_libs_accepts_prefixless_wrapper(self):
        """Linux lib 目录中是 cef_dll_wrapper.a 时 --with-cef 收集不报错。"""
        lib_dir = os.path.join(self.tmp, 'lib', 'Release')
        for base in pack.LIB_BASENAMES:
            self._touch('lib/Release/lib%s.a' % base)
        # 真实产物：无 lib 前缀（libcef_dll 的 CMakeLists.txt 设 PREFIX ""）
        self._touch('lib/Release/cef_dll_wrapper.a')
        collector = pack.Collector()
        count = pack.collect_libs(collector, lib_dir, 'Release',
                                  True, 'latest', 'linux')
        self.assertEqual(count, len(pack.LIB_BASENAMES) + 1)
        rels = [rel for rel, _abs in collector.items]
        self.assertIn('lib/cef_dll_wrapper.a', rels)
        self.assertEqual(collector.critical, [])


class CefMacosPackagingTest(_TempDirTest):
    """macOS 不随包 CEF 开发文件；-cef 只表示 CEF 示例 Bundle 随包。"""

    def test_headers_exclude_cef_on_macos(self):
        collector = pack.Collector()
        pack.collect_headers(collector, True, False, False, is_macos=True)
        rels = [rel for rel, _abs in collector.items]
        self.assertIn('include/duilib/duilib.h', rels)
        self.assertNotIn('include/duilib/duilib_cef.h', rels)
        self.assertFalse(any(rel.startswith('include/duilib/CEFControl/')
                             for rel in rels))

    def test_headers_include_cef_elsewhere(self):
        collector = pack.Collector()
        pack.collect_headers(collector, True, False, False, is_macos=False)
        rels = [rel for rel, _abs in collector.items]
        self.assertIn('include/duilib/duilib_cef.h', rels)

    def test_meta_never_declares_cef_on_macos(self):
        collector = pack.Collector()
        collector.add('include/duilib/duilib_cef.h',
                      self._touch('duilib_cef.h'))
        collector.add('include/duilib/CEFControl/CefManager.h',
                      self._touch('CefManager.h'))
        collector.add('lib/libduilib.a', self._touch('libduilib.a'))
        meta = pack.build_package_meta(
            collector, '1.0', 'macos', 'x64', 'Release', None,
            True, 'latest', 'auto')
        self.assertFalse(meta['has_cef'])
        self.assertEqual(meta['cef_src'], '')
        self.assertEqual(meta['cef_wrapper'], '')

    def test_name_cef_follows_example_bundle(self):
        item = ('bin/cef.app/Contents/Frameworks/'
                'Chromium Embedded Framework.framework/Chromium Embedded Framework',
                '/abs/ignored')
        self.assertTrue(pack.has_macos_cef_examples([item]))
        self.assertFalse(pack.has_macos_cef_examples(
            [('bin/basic.app/Contents/MacOS/basic', '/abs/ignored')]))
        self.assertFalse(pack.has_macos_cef_examples([]))


class DocLinkRewriteTest(unittest.TestCase):

    def test_rewrite_and_drop(self):
        packaged = {'include/duilib/duilib.h', 'docs/Summary.md'}
        text = ('[h](../duilib/duilib.h) [s](./Summary.md) '
                '[gone](../build/missing.md) [bad](Nope.md)')
        result = pack._rewrite_doc_links(text, packaged)
        self.assertIn('(../include/duilib/duilib.h)', result)
        self.assertIn('[s](./Summary.md)', result)
        self.assertNotIn('../build/missing.md', result)
        self.assertNotIn('[bad](Nope.md)', result)
        self.assertIn('bad', result)


class CefExampleSelectionTest(_TempDirTest):

    def _collect(self, with_cef, is_macos=False):
        bin_dir = os.path.join(self.tmp, 'bin')
        os.makedirs(bin_dir, exist_ok=True)
        collector = pack.Collector()
        with mock.patch.object(pack, 'BIN_DIR', bin_dir):
            count = pack.collect_examples(collector, 'Release', False, with_cef,
                                          is_macos=is_macos)
        return count, [rel for rel, _abs in collector.items], collector.warnings

    def _make_bin(self):
        names = ('basic', 'cef', 'CefBrowser', 'libSDL3.so', 'resources.zip')
        for name in names:
            path = self._touch('bin/' + name)
            os.chmod(path, 0o755 if name in ('basic', 'cef', 'CefBrowser')
                     else 0o644)

    def _make_bin_app(self):
        basic = self._touch('bin/basic')
        os.chmod(basic, 0o755)
        self._touch('bin/cef.app/Contents/MacOS/cef')
        self._touch('bin/cef.app/Contents/Info.plist')
        self._touch('bin/CefBrowser.app/Contents/MacOS/CefBrowser')

    def test_is_cef_example(self):
        for name in ('cef', 'CefBrowser', 'cef.exe', 'CefBrowser.exe',
                     'cef_d.exe', 'CefBrowser_d.exe', 'cef.app',
                     'CefBrowser.app'):
            self.assertTrue(pack.is_cef_example(name), name)
        for name in ('basic', 'basic.exe', 'cefclient', 'CefBrowser2',
                     'basic.app'):
            self.assertFalse(pack.is_cef_example(name), name)

    def test_skip_cef_examples_without_cef(self):
        self._make_bin()
        count, items, warnings = self._collect(False)
        self.assertEqual(count, 1)
        self.assertEqual(items, ['bin/basic'])
        self.assertTrue(any('CEF' in msg for msg in warnings))

    def test_include_cef_examples_with_cef(self):
        self._make_bin()
        count, items, _warnings = self._collect(True)
        self.assertEqual(count, 3)
        self.assertEqual(items, ['bin/CefBrowser', 'bin/basic', 'bin/cef'])

    def test_collect_macos_app_bundles(self):
        """macOS 的 CEF 示例是 .app 目录：整个 Bundle 逐文件收集。"""
        self._make_bin_app()
        count, items, _warnings = self._collect(True, is_macos=True)
        self.assertEqual(count, 4)
        self.assertEqual(sorted(items), [
            'bin/CefBrowser.app/Contents/MacOS/CefBrowser',
            'bin/basic',
            'bin/cef.app/Contents/Info.plist',
            'bin/cef.app/Contents/MacOS/cef',
        ])

    def test_skip_macos_app_bundles_without_cef(self):
        self._make_bin_app()
        count, items, warnings = self._collect(False, is_macos=True)
        self.assertEqual(count, 1)
        self.assertEqual(items, ['bin/basic'])
        self.assertTrue(any('cef.app' in msg for msg in warnings))

    def test_collect_plain_macos_app_bundle(self):
        """普通示例的 .app（如 basic.app）不是 CEF 示例，应整体收集。"""
        self._make_bin_app()
        self._touch('bin/basic.app/Contents/MacOS/basic')
        self._touch('bin/basic.app/Contents/Resources/duilib/resources/themes/default/global.xml')
        count, items, warnings = self._collect(False, is_macos=True)
        self.assertEqual(count, 3)
        self.assertEqual(sorted(items), [
            'bin/basic',
            'bin/basic.app/Contents/MacOS/basic',
            'bin/basic.app/Contents/Resources/duilib/resources/themes/default/global.xml',
        ])
        self.assertTrue(any('cef.app' in msg for msg in warnings))
        self.assertFalse(any('basic.app' in msg for msg in warnings))

    def test_manifest_documents_macos_app_bundles(self):
        collector = pack.Collector()
        collector.add('include/duilib/duilib.h', self._touch('duilib.h'))
        collector.add('lib/duilib.a', self._touch('libduilib.a'))
        collector.add('bin/cef.app/Contents/MacOS/cef',
                      self._touch('cef.app/Contents/MacOS/cef'))
        text, _sums = pack.write_manifest(
            collector, 'pkg', '1.0', 'macos', 'x64', 'Release',
            True, False, True, 'latest', package_time=0)
        self.assertIn('bin/*.app', text)
        self.assertNotIn('未打包', text)

        text, _sums = pack.write_manifest(
            collector, 'pkg', '1.0', 'macos', 'x64', 'Release',
            True, False, False, 'latest', package_time=0)
        self.assertNotIn('bin/*.app', text)
        self.assertIn('未打包', text)


class PeArchCheckTest(_TempDirTest):
    """Windows 的 bin/ 跨架构共用：PE 架构不符的产物被跳过并阻止出包。"""

    @staticmethod
    def _write_pe(path, machine):
        data = bytearray(0x60)
        data[0:2] = b'MZ'
        data[0x3C:0x40] = struct.pack('<I', 0x40)
        data[0x40:0x44] = b'PE\0\0'
        data[0x44:0x46] = struct.pack('<H', machine)
        with open(path, 'wb') as fp:
            fp.write(data)

    def test_pe_machine(self):
        path = self._touch('x64.exe')
        self._write_pe(path, pack.PE_MACHINE_BY_ARCH['x64'])
        self.assertEqual(pack.pe_machine(path),
                         pack.PE_MACHINE_BY_ARCH['x64'])
        self.assertTrue(pack.pe_arch_matches(path, 'x64'))
        self.assertFalse(pack.pe_arch_matches(path, 'x86'))
        self.assertIsNone(pack.pe_machine(self._touch('plain.exe')))

    def test_collect_examples_skips_other_arch(self):
        bin_dir = os.path.join(self.tmp, 'bin')
        os.makedirs(bin_dir)
        self._write_pe(os.path.join(bin_dir, 'basic.exe'),
                       pack.PE_MACHINE_BY_ARCH['x86'])
        self._write_pe(os.path.join(bin_dir, 'layout.exe'),
                       pack.PE_MACHINE_BY_ARCH['x64'])
        self._write_pe(os.path.join(bin_dir, 'WebView2Loader.dll'),
                       pack.PE_MACHINE_BY_ARCH['x64'])
        collector = pack.Collector()
        with mock.patch.object(pack, 'BIN_DIR', bin_dir):
            count = pack.collect_examples(collector, 'Release', True,
                                          arch_tag='x86')
        self.assertEqual([rel for rel, _abs in collector.items],
                         ['bin/basic.exe'])
        self.assertEqual(count, 1)
        self.assertTrue(any('架构' in msg for msg in collector.critical))


class ExampleBuildConfigTest(_TempDirTest):
    """Debug/Release 同名示例：按 CMake 写入的 .build-config 标记判定配置。"""

    def _bin_with(self, entries):
        bin_dir = os.path.join(self.tmp, 'bin')
        stamp_dir = os.path.join(bin_dir, '.build-config')
        os.makedirs(stamp_dir, exist_ok=True)
        for name, stamp in entries:
            self._touch('bin/' + name)
            if stamp is not None:
                # 标记按 CMake 目标名保存（basic.exe -> basic，cef.app -> cef）
                stem = name
                if name.endswith('.app') or name.lower().endswith('.exe'):
                    stem = name[:-4]
                with open(os.path.join(stamp_dir, stem), 'w',
                          encoding='utf-8') as fp:
                    fp.write(stamp + '\n')
        return bin_dir

    def _collect(self, config, bin_dir):
        collector = pack.Collector()
        with mock.patch.object(pack, 'BIN_DIR', bin_dir):
            count = pack.collect_examples(collector, config, True, True)
        return count, [rel for rel, _abs in collector.items], collector.warnings

    def test_debug_skips_examples_built_for_release(self):
        bin_dir = self._bin_with([('basic.exe', 'Release'),
                                  ('layout.exe', 'Debug')])
        count, items, warnings = self._collect('Debug', bin_dir)
        self.assertEqual(items, ['bin/layout.exe'])
        self.assertEqual(count, 1)
        self.assertTrue(any('另一配置' in msg for msg in warnings))

    def test_release_skips_examples_built_for_debug(self):
        bin_dir = self._bin_with([('basic.exe', 'Debug')])
        count, items, warnings = self._collect('Release', bin_dir)
        self.assertEqual(count, 0)
        self.assertEqual(items, [])
        self.assertTrue(any('另一配置' in msg for msg in warnings))

    def test_debug_example_without_stamp_warns(self):
        bin_dir = self._bin_with([('basic.exe', None)])
        count, items, warnings = self._collect('Debug', bin_dir)
        self.assertEqual(count, 1)
        self.assertEqual(items, ['bin/basic.exe'])
        self.assertTrue(any('无法确认' in msg for msg in warnings))

    def test_release_example_without_stamp_no_warning(self):
        bin_dir = self._bin_with([('basic.exe', None)])
        _count, items, warnings = self._collect('Release', bin_dir)
        self.assertEqual(items, ['bin/basic.exe'])
        self.assertFalse(any('无法确认' in msg for msg in warnings))

    def test_stamp_directory_is_not_collected(self):
        """bin/.build-config 目录本身不能被当作示例程序收集。"""
        bin_dir = os.path.join(self.tmp, 'bin')
        stamp_dir = os.path.join(bin_dir, '.build-config')
        os.makedirs(stamp_dir, exist_ok=True)
        self._touch('bin/basic.exe')
        with open(os.path.join(stamp_dir, 'basic'), 'w',
                  encoding='utf-8') as fp:
            fp.write('Release\n')
        count, items, _warnings = self._collect('Release', bin_dir)
        self.assertEqual(count, 1)
        self.assertEqual(items, ['bin/basic.exe'])

    def test_windows_exe_uses_target_stamp(self):
        """CMake 按目标名写标记（.build-config/basic），而 bin 中的文件是 basic.exe。"""
        bin_dir = os.path.join(self.tmp, 'bin')
        stamp_dir = os.path.join(bin_dir, '.build-config')
        os.makedirs(stamp_dir, exist_ok=True)
        self._touch('bin/basic.exe')
        with open(os.path.join(stamp_dir, 'basic'), 'w',
                  encoding='utf-8') as fp:
            fp.write('Release\n')
        count, items, warnings = self._collect('Debug', bin_dir)
        self.assertEqual(count, 0)
        self.assertEqual(items, [])
        self.assertTrue(any('另一配置' in msg for msg in warnings))

    def test_legacy_inline_stamp_is_still_read(self):
        """升级前写在 bin/<目标>.build-config 的标记仍能识别配置。"""
        bin_dir = os.path.join(self.tmp, 'bin')
        os.makedirs(bin_dir, exist_ok=True)
        self._touch('bin/basic.exe')
        with open(os.path.join(bin_dir, 'basic.build-config'), 'w',
                  encoding='utf-8') as fp:
            fp.write('Release\n')
        count, items, warnings = self._collect('Debug', bin_dir)
        self.assertEqual(count, 0)
        self.assertEqual(items, [])
        self.assertTrue(any('另一配置' in msg for msg in warnings))

    def test_macos_app_reads_target_stamp(self):
        bin_dir = os.path.join(self.tmp, 'bin')
        stamp_dir = os.path.join(bin_dir, '.build-config')
        os.makedirs(stamp_dir, exist_ok=True)
        self._touch('bin/cef.app/Contents/MacOS/cef')
        with open(os.path.join(stamp_dir, 'cef'), 'w',
                  encoding='utf-8') as fp:
            fp.write('Release\n')
        collector = pack.Collector()
        with mock.patch.object(pack, 'BIN_DIR', bin_dir):
            count = pack.collect_examples(collector, 'Debug', False, True,
                                          is_macos=True)
        self.assertEqual(count, 0)
        self.assertEqual([rel for rel, _abs in collector.items], [])
        self.assertTrue(any('另一配置' in msg for msg in collector.warnings))


class ConsoleLanguageTest(unittest.TestCase):
    """英文模式下控制台文案必须是纯 ASCII（非中文系统不会乱码）。"""

    def _messages(self):
        return [
            pack.T('pkg.resources_missing') % 'res',
            pack.T('pkg.cef_runtime_missing') % 'bin/libcef_win',
            pack.T('pkg.examples_missing') % 'bin',
            pack.T('pkg.headers_missing'),
            pack.T('pkg.static_libs_missing'),
        ]

    def test_english_messages_are_ascii(self):
        old = pack.MESSAGES
        pack.MESSAGES = pack.make_messages('en')
        try:
            texts = self._messages()
        finally:
            pack.MESSAGES = old
        self.assertIn('resources directory does not exist', texts[0])
        self.assertIn('CEF runtime directory does not exist', texts[1])
        self.assertIn('no example binaries were found', texts[2])
        for text in texts:
            self.assertEqual([ch for ch in text if ord(ch) > 127], [], text)

    def test_simplified_chinese_messages_stay_chinese(self):
        old = pack.MESSAGES
        pack.MESSAGES = pack.make_messages('zh_CN')
        try:
            texts = self._messages()
        finally:
            pack.MESSAGES = old
        self.assertIn('资源目录', texts[0])
        self.assertIn('CEF 运行时', texts[1])
        self.assertIn('示例程序', texts[2])


class Msys2EnvDetectionTests(_TempDirTest):
    """MSYS2 环境判定：优先依据编译器安装位置，MSYSTEM 只在无法探测时兜底。

    与 CMake 侧 duilib_detect_msys2 同一口径（<root>/<env>/bin 且存在
    <root>/usr/bin/pacman.exe）；Git Bash 也会设置 MSYSTEM=MINGW64，不能照搬。
    """

    def _msys_tree(self, env_name):
        """伪造 MSYS2 安装树，返回 <root>/<env>/bin/g++.exe 绝对路径。"""
        root = os.path.join(self.tmp, 'msys2')
        env = os.path.join(root, env_name, 'bin')
        os.makedirs(env, exist_ok=True)
        os.makedirs(os.path.join(root, 'usr', 'bin'), exist_ok=True)
        self._touch(os.path.relpath(os.path.join(root, 'usr', 'bin', 'pacman.exe'),
                                    self.tmp).replace(os.sep, '/'))
        compiler = os.path.join(env, 'g++.exe')
        self._touch(os.path.relpath(compiler, self.tmp).replace(os.sep, '/'))
        return compiler

    def _standalone_compiler(self):
        compiler = os.path.join(self.tmp, 'mingw64', 'bin', 'g++.exe')
        self._touch(os.path.relpath(compiler, self.tmp).replace(os.sep, '/'))
        return compiler

    def test_compiler_in_msys2_env(self):
        self.assertEqual(
            pack._msys2_env_from_compiler(self._msys_tree('ucrt64')), 'ucrt64')

    def test_compiler_outside_msys2(self):
        self.assertEqual(pack._msys2_env_from_compiler(self._standalone_compiler()), '')
        self.assertEqual(pack._msys2_env_from_compiler(''), '')

    def test_env_name_prefers_compiler_over_msystem(self):
        old = dict(os.environ)
        old_which = pack.shutil.which
        old_exe = pack.sys.executable

        def _restore():
            os.environ.clear()
            os.environ.update(old)
            pack.shutil.which = old_which
            pack.sys.executable = old_exe

        self.addCleanup(_restore)
        try:
            os.environ['MSYSTEM'] = 'MINGW64'
            # 1) CC 指向 MSYS2 里的编译器：以编译器为准
            os.environ['CC'] = self._msys_tree('mingw64')
            self.assertEqual(pack._msys2_env_name(), 'mingw64')
            # 2) CC 指向独立 MinGW-w64（Git Bash 的 MSYSTEM 不可信）：空串
            os.environ['CC'] = self._standalone_compiler()
            self.assertEqual(pack._msys2_env_name(), '')
        finally:
            os.environ.clear()
            os.environ.update(old)

    def test_env_name_falls_back_to_msystem(self):
        """探测不到任何编译器时才用 MSYSTEM，且只认已知的 MSYS2 环境名。"""
        old = dict(os.environ)
        old_which = pack.shutil.which
        old_exe = pack.sys.executable

        def _restore():
            os.environ.clear()
            os.environ.update(old)
            pack.shutil.which = old_which
            pack.sys.executable = old_exe

        self.addCleanup(_restore)
        try:
            os.environ.pop('CC', None)
            os.environ.pop('CXX', None)
            os.environ['MSYSTEM'] = 'MINGW64'
            pack.shutil.which = lambda name, *args, **kwargs: None
            pack.sys.executable = os.path.join(self.tmp, 'python.exe')
            self.assertEqual(pack._msys2_env_name(), 'mingw64')
            # 非 MSYS2 的 MSYSTEM 值（如 MSYS / 未知）不当成环境名
            os.environ['MSYSTEM'] = 'MSYS'
            self.assertEqual(pack._msys2_env_name(), '')
        finally:
            os.environ.clear()
            os.environ.update(old)


class Msys2FallbackCandidateTests(_TempDirTest):
    """推断不出 MSYS2 环境时，末尾兜底的 gcc.msvcrt 变体仍能被解析到。"""

    def test_skia_candidates_end_with_msvcrt_fallback(self):
        names = pack._skia_candidate_names('Release', 'x64', 'mingw', 'gcc',
                                           msys2_env='', windows=True)
        self.assertEqual(names[-1], 'gcc.msvcrt.x64.release')
        # 兜底只追加在末尾：前面的候选顺序不受影响（独立 MinGW/UCRT64 优先）
        self.assertEqual(names[0], 'mingw64-gcc.x64.release')
        self.assertEqual(names[1], 'gcc.x64.release')

    def test_msys2_env_known_keeps_msvcrt_first(self):
        names = pack._skia_candidate_names('Release', 'x64', 'mingw', 'gcc',
                                           msys2_env='mingw64', windows=True)
        self.assertEqual(names[0], 'gcc.msvcrt.x64.release')
        self.assertNotIn('gcc.msvcrt.x64.release', names[1:])

    def test_resolve_skia_msvcrt_variant_when_env_unknown(self):
        old_root = pack.SKIA_SRC_ROOT
        old_env = pack._msys2_env_name
        try:
            pack.SKIA_SRC_ROOT = self.tmp
            pack._msys2_env_name = lambda: ''
            self._touch('out/gcc.msvcrt.x64.release/libSkia.a')
            self.assertEqual(
                pack.resolve_skia_lib_dir(None, 'Release', 'x64', 'mingw', 'gcc'),
                os.path.join(self.tmp, 'out', 'gcc.msvcrt.x64.release'))
        finally:
            pack.SKIA_SRC_ROOT = old_root
            pack._msys2_env_name = old_env

    def test_resolve_sdl_msvcrt_variant_when_env_unknown(self):
        old_root = pack.SDL3_SRC_ROOT
        old_env = pack._msys2_env_name
        try:
            pack.SDL3_SRC_ROOT = self.tmp
            pack._msys2_env_name = lambda: ''
            variant = 'gcc.msvcrt.x64.release'
            self._touch(variant + '/lib/libSDL3.a')
            self.assertEqual(
                pack.resolve_sdl_prefix(None, 'Release', 'x64', 'mingw', 'gcc'),
                os.path.join(self.tmp, variant))
        finally:
            pack.SDL3_SRC_ROOT = old_root
            pack._msys2_env_name = old_env


class DepsRootTests(_TempDirTest):
    """`--deps-root` 把 Skia / SDL3 的探测目录改到该根下（与 build.py 一致）。"""

    def _apply(self, root):
        old = (pack.SKIA_SRC_ROOT, pack.SDL3_SRC_ROOT)
        self.addCleanup(setattr, pack, 'SKIA_SRC_ROOT', old[0])
        self.addCleanup(setattr, pack, 'SDL3_SRC_ROOT', old[1])
        pack.set_deps_root(root)

    def test_absolute_root(self):
        self._apply(self.tmp)
        self.assertEqual(pack.SKIA_SRC_ROOT, os.path.join(self.tmp, 'skia'))
        self.assertEqual(pack.SDL3_SRC_ROOT, os.path.join(self.tmp, 'SDL3'))

    def test_relative_root_is_cwd_relative(self):
        # 相对路径按当前工作目录解析（与 build.py --deps-root 相同）；在临时目录下
        # 运行才能证明不是“相对仓库根”，否则仓库根恰好等于 cwd 时两种实现都通过。
        old_cwd = os.getcwd()
        self.addCleanup(os.chdir, old_cwd)
        os.chdir(self.tmp)
        self._apply('custom_deps')
        expected = os.path.join(self.tmp, 'custom_deps')
        self.assertEqual(pack.SKIA_SRC_ROOT, os.path.join(expected, 'skia'))
        self.assertEqual(pack.SDL3_SRC_ROOT, os.path.join(expected, 'SDL3'))


class MsvcMdModeTests(unittest.TestCase):
    """`--msvc-runtime md` 也必须选 .md/.mdd 变体，不能只看是否有 duilib DLL。"""

    def test_static_md_runtime_selects_md(self):
        self.assertTrue(pack.resolve_md_mode('msvc', False, 'md'))
        self.assertTrue(pack.resolve_md_mode('msvc', False, 'MD'))

    def test_dll_forces_md(self):
        self.assertTrue(pack.resolve_md_mode('msvc', True, 'auto'))
        self.assertTrue(pack.resolve_md_mode('msvc', True, 'mt'))

    def test_mt_runtime_without_dll_is_mt(self):
        self.assertFalse(pack.resolve_md_mode('msvc', False, 'mt'))
        self.assertFalse(pack.resolve_md_mode('msvc', False, 'auto'))

    def test_mingw_never_uses_md_variant(self):
        self.assertFalse(pack.resolve_md_mode('mingw', True, 'md'))
        # windows=True / msys2_env='' 显式声明目标平台与环境：测试在任意宿主
        # （含 MSYS2 MINGW64，否则会推断成 msvcrt 目录）上都应得到同一结果
        self.assertEqual(
            pack._sdl_variant_name('Release', 'x64', 'mingw',
                                   mingw_variant='gcc', md_mode=True,
                                   msys2_env='', windows=True),
            'mingw64-gcc.x64.release')

    def test_skia_candidates_follow_md_mode(self):
        self.assertEqual(
            pack._skia_candidate_names('Release', 'x64', md_mode=True,
                                       msys2_env='', windows=True)[0],
            'llvm.x64.release.md')
        self.assertEqual(
            pack._skia_candidate_names('Debug', 'x64', md_mode=True,
                                       msys2_env='', windows=True)[0],
            'llvm.x64.debug.mdd')


class SdlPrefixSelectionTests(_TempDirTest):
    """SDL3 安装前缀解析：Windows 按变体子目录，显式 --sdl-dir 视为单前缀。"""

    def _resolve(self, config='Release', toolchain='msvc', mingw_variant=None,
                 md_mode=False, windows=True, explicit=None):
        old_root = pack.SDL3_SRC_ROOT
        old_win = pack.is_windows_host
        try:
            pack.SDL3_SRC_ROOT = self.tmp
            pack.is_windows_host = lambda: windows
            return pack.resolve_sdl_prefix(explicit, config, 'x64', toolchain,
                                           mingw_variant, md_mode)
        finally:
            pack.SDL3_SRC_ROOT = old_root
            pack.is_windows_host = old_win

    def _prefix(self, variant, lib_name='SDL3-static.lib'):
        prefix = os.path.join(self.tmp, variant) if variant else self.tmp
        for rel in (('lib', lib_name), ('include', 'SDL3', 'SDL.h')):
            self._touch(os.path.relpath(os.path.join(prefix, *rel), self.tmp)
                        .replace(os.sep, '/'))
        return prefix

    def test_variant_prefix_wins_over_root(self):
        self._prefix('')
        variant = self._prefix('msvc.x64.release')
        self.assertEqual(self._resolve(config='Release'), variant)

    def test_debug_uses_debug_variant(self):
        self._prefix('msvc.x64.release')
        variant = self._prefix('msvc.x64.debug')
        self.assertEqual(self._resolve(config='Debug'), variant)

    def test_md_suffix_follows_md_mode(self):
        variant = self._prefix('msvc.x64.release.md')
        self.assertEqual(self._resolve(config='Release', md_mode=True), variant)

    def test_fallback_to_single_prefix(self):
        self._prefix('')
        self.assertEqual(self._resolve(config='Debug'), self.tmp)

    def test_mingw_shares_release_variant(self):
        variant = self._prefix('mingw64-gcc.x64.release', lib_name='libSDL3.a')
        self.assertEqual(
            self._resolve(config='Debug', toolchain='mingw', mingw_variant='gcc'),
            variant)

    def test_non_windows_uses_root(self):
        self._prefix('')
        self.assertEqual(self._resolve(config='Release', windows=False), self.tmp)

    def test_explicit_dir_is_single_prefix(self):
        explicit = os.path.join(self.tmp, 'custom')
        os.makedirs(explicit, exist_ok=True)
        self.assertEqual(self._resolve(explicit=explicit), explicit)

    def test_missing_prefix_raises(self):
        with self.assertRaises(pack.PackageError) as ctx:
            self._resolve(config='Debug')
        self.assertIn('msvc.x64.debug', str(ctx.exception))

    def test_toolchain_library_filter(self):
        """MinGW 只认 .a：只有 .lib 的变体目录不可用，回退 <root>。"""
        self._prefix('mingw64-gcc.x64.release', lib_name='SDL3-static.lib')
        self._prefix('', lib_name='libSDL3.a')
        self.assertEqual(
            self._resolve(config='Release', toolchain='mingw', mingw_variant='gcc'),
            self.tmp)

    def test_mingw_msys2_guess_mismatch_still_resolves(self):
        """打包侧的 MSYS2 判定与 CMake 不同时，仍按候选顺序找到已安装变体。

        Python 侧只能依据 sys.executable / MSYSTEM 推断 MSYS2 环境（Git Bash
        也会设置 MSYSTEM），CMake 侧依据编译器安装位置；两侧不一致时规范变体名
        可能指向 CMake 并未安装的目录（如 gcc.msvcrt.*），此时应继续尝试同组
        的其他候选，而不是直接判定为缺少 SDL3。
        """
        variant = self._prefix('mingw64-gcc.x64.release', lib_name='libSDL3.a')
        old_env = pack._msys2_env_name
        pack._msys2_env_name = lambda: 'mingw64'
        try:
            self.assertNotEqual(
                pack._sdl_variant_name('Release', 'x64', 'mingw', 'gcc',
                                       windows=True),
                'mingw64-gcc.x64.release')
            self.assertEqual(
                self._resolve(config='Release', toolchain='mingw',
                              mingw_variant='gcc'),
                variant)
        finally:
            pack._msys2_env_name = old_env


class CollectSdlVariantTests(_TempDirTest):
    """collect_sdl 从选中的变体前缀收集库、头文件与许可证。"""

    def _collect(self, config='Debug'):
        old_root = pack.SDL3_SRC_ROOT
        old_win = pack.is_windows_host
        try:
            pack.SDL3_SRC_ROOT = self.tmp
            pack.is_windows_host = lambda: True
            collector = pack.Collector()
            pack.collect_sdl(collector, None, config, True, arch_tag='x64',
                             toolchain='msvc')
            return collector
        finally:
            pack.SDL3_SRC_ROOT = old_root
            pack.is_windows_host = old_win

    def test_debug_variant_library_is_packaged(self):
        prefix = 'msvc.x64.debug'
        self._touch(prefix + '/lib/SDL3-static.lib')
        self._touch(prefix + '/include/SDL3/SDL.h')
        self._touch(prefix + '/share/licenses/SDL3/LICENSE.txt')
        collector = self._collect(config='Debug')
        rels = [rel for rel, _abs in collector.items]
        self.assertIn('lib/SDL3-static.lib', rels)
        self.assertIn('include/SDL3/SDL.h', rels)
        self.assertIn('LICENSES/SDL3/LICENSE.txt', rels)
        abs_lib = dict(collector.items)['lib/SDL3-static.lib']
        self.assertIn(os.path.normcase(os.path.join(self.tmp, prefix)),
                      os.path.normcase(abs_lib))

    def test_dynamic_import_library_requires_dll(self):
        prefix = 'msvc.x64.release'
        self._touch(prefix + '/lib/SDL3.lib')
        collector = self._collect(config='Release')
        self.assertTrue(any('SDL3' in item and 'DLL' in item
                            for item in collector.critical))

    def test_dynamic_import_library_with_dll(self):
        prefix = 'msvc.x64.release'
        self._touch(prefix + '/lib/SDL3.lib')
        self._touch(prefix + '/bin/SDL3.dll')
        collector = self._collect(config='Release')
        rels = [rel for rel, _abs in collector.items]
        self.assertIn('lib/SDL3.lib', rels)
        self.assertIn('bin/SDL3.dll', rels)
        self.assertEqual(collector.critical, [])


class SdlVariantCallSiteTests(unittest.TestCase):
    """调用 `pack._sdl_variant_name` 的关键字参数必须与函数签名一致。

    回归：`build.py doctor --sdl` 曾用 `dll_mode=` 调用（签名已经改成 md_mode），
    运行期直接 TypeError；这里在源码层提前拦住同类问题。
    """

    def test_keywords_match_signature(self):
        import ast
        import inspect
        signature = inspect.signature(pack._sdl_variant_name)
        build_dir = os.path.dirname(os.path.abspath(__file__))
        problems = []
        for name in ('build.py', 'test_pack.py', 'test_cmake_scripts.py'):
            path = os.path.join(build_dir, name)
            with open(path, 'r', encoding='utf-8') as fp:
                tree = ast.parse(fp.read(), filename=path)
            for node in ast.walk(tree):
                if not isinstance(node, ast.Call):
                    continue
                func = node.func
                if not (isinstance(func, ast.Attribute)
                        and func.attr == '_sdl_variant_name'):
                    continue
                for keyword in node.keywords:
                    if keyword.arg and keyword.arg not in signature.parameters:
                        problems.append(
                            '%s:%d 关键字 %s 不在签名（%s）中'
                            % (name, node.lineno, keyword.arg,
                               ', '.join(signature.parameters)))
        self.assertEqual(problems, [], '\n'.join(problems))


if __name__ == '__main__':
    unittest.main()
