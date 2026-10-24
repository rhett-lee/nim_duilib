# -*- coding: utf-8 -*-
"""build/pack.py 的最小回归测试：`--verify` 与包名规则。

不依赖已构建产物，可直接运行：

    python3 build/test_pack.py

覆盖：正/负向包体检（zip 与 tar.gz）、包名组合、SOURCE_DATE_EPOCH 校验。
"""
import hashlib
import os
import shutil
import sys
import tarfile
import tempfile
import unittest
import zipfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import pack  # noqa: E402


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
        self.assertTrue(pack.verify_package(self._make_zip(_payload())))

    def test_tgz_ok(self):
        self.assertTrue(pack.verify_package(self._make_tgz(_payload())))

    def test_missing_manifest(self):
        path = self._make_zip(_payload(), omit=('MANIFEST.txt',))
        self.assertFalse(pack.verify_package(path))

    def test_tampered_file(self):
        path = self._make_zip(_payload(),
                              tamper=('docs/Summary.md', b'changed\n'))
        self.assertFalse(pack.verify_package(path))

    def test_missing_file(self):
        self.assertFalse(
            pack.verify_package(os.path.join(self.tmp, 'not-exists.zip')))

    def test_corrupt_zip(self):
        path = os.path.join(self.tmp, 'broken.zip')
        with open(path, 'wb') as fp:
            fp.write(b'not a zip')
        self.assertFalse(pack.verify_package(path))

    def test_broken_doc_link(self):
        payload = _payload()
        payload['docs/Summary.md'] = b'[bad](missing.md)\n'
        self.assertFalse(pack.verify_package(self._make_zip(payload)))

    def test_broken_doc_link_tgz(self):
        payload = _payload()
        payload['docs/Summary.md'] = b'[bad](../build/missing.md)\n'
        self.assertFalse(pack.verify_package(self._make_tgz(payload)))


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

    def test_jpeg_arm64_is_skipped_with_warning(self):
        old_win = pack.is_windows_host
        try:
            pack.is_windows_host = lambda: True
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_jpeg_turbo(collector, 'arm64', 'msvc'), 0)
            self.assertEqual(collector.items, [])
            self.assertTrue(any('arm64' in msg for msg in collector.warnings))
        finally:
            pack.is_windows_host = old_win

    def test_cef_arm64_is_rejected(self):
        old_win = pack.is_windows_host
        try:
            pack.is_windows_host = lambda: True
            with self.assertRaises(pack.PackageError) as ctx:
                pack.collect_cef(pack.Collector(), 'arm64', 'latest')
            self.assertIn('arm64', str(ctx.exception))
            collector = pack.Collector()
            self.assertEqual(
                pack.collect_cef_runtime(collector, 'win', 'arm64', 'latest'), 0)
            self.assertTrue(any('arm64' in msg for msg in collector.warnings))
        finally:
            pack.is_windows_host = old_win

    def test_manifest_notes_arm64_scope(self):
        collector = self._collector()
        text, _sums = pack.write_manifest(
            collector, 'nim_duilib-1.0-win-arm64', '1.0', 'win', 'arm64',
            'Release', False, False, False, 'latest', toolchain='msvc',
            virtual_files={}, msvc_runtime='MT', package_time=0)
        self.assertIn('平台      : win-arm64', text)
        self.assertIn('arm64 说明', text)
        self.assertIn('未在 arm64 真机上做编译、打包、运行验证', text)
        self.assertIn('CEF 与 libjpeg-turbo', text)

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


if __name__ == '__main__':
    unittest.main()
