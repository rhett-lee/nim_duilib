# -*- coding: utf-8 -*-
"""build/pack.py 的最小回归测试：`--verify` 与包名规则。

不依赖已构建产物，可直接运行：

    python3 build/test_pack.py

覆盖：正/负向包体检（zip 与 tar.gz）、包名组合、SOURCE_DATE_EPOCH 校验。
"""
import hashlib
import os
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


if __name__ == '__main__':
    unittest.main()
