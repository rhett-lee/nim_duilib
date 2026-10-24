# -*- coding: utf-8 -*-
"""build/autotest.py 的纯单元测试：不构建、不实际打包。"""

import argparse
import contextlib
import io
import os
import shutil
import sys
import tempfile
import unittest
import zipfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import autotest  # noqa: E402


class PresetTableTest(unittest.TestCase):

    def test_all_presets_declared(self):
        for name in autotest.ALL_PRESETS:
            self.assertIn(name, autotest.PRESETS)
            self.assertIn('kind', autotest.PRESETS[name])
            self.assertIn('description', autotest.PRESETS[name])

    def test_pack_presets_have_required_fields(self):
        for name, preset in autotest.PRESETS.items():
            if preset['kind'] != 'pack':
                continue
            for key in ('arch', 'config', 'toolchain', 'lib_dir'):
                self.assertIn(key, preset, '%s 缺少 %s' % (name, key))
            self.assertIn(preset['arch'], ('x64', 'x86', 'arm64'))
            self.assertIn(preset['config'], ('Release', 'Debug'))
            self.assertIn(preset['toolchain'], ('msvc', 'mingw'))

    def test_list_presets(self):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            self.assertEqual(autotest.main(['--list-presets']), 0)
        text = out.getvalue()
        self.assertIn('arm64-synthetic', text)
        self.assertIn('pack-msvc-x64-release', text)


class PrereqTest(unittest.TestCase):

    def test_unit_and_synthetic_are_ready(self):
        for name in ('unit', 'arm64-synthetic'):
            ok, reason = autotest.check_prereq(autotest.PRESETS[name])
            self.assertTrue(ok, reason)

    def test_missing_lib_dir_is_skipped(self):
        preset = dict(autotest.PRESETS['pack-msvc-x64-release'])
        preset['lib_dir'] = os.path.join('lib', 'no-such-arch', 'Release')
        ok, reason = autotest.check_prereq(preset)
        self.assertFalse(ok)
        self.assertIn('库目录不存在', reason)


class PackCommandTest(unittest.TestCase):

    def _args(self, sdl=False, examples=False):
        return argparse.Namespace(with_sdl=sdl, with_examples=examples)

    def test_command_contains_arch_and_output(self):
        preset = autotest.PRESETS['pack-msvc-x64-release']
        cmd = autotest._pack_command(preset, 'C:/tmp/dist', self._args())
        self.assertIn('--arch', cmd)
        self.assertIn('x64', cmd)
        self.assertIn('--config', cmd)
        self.assertIn('Release', cmd)
        self.assertIn('--out-dir', cmd)
        self.assertNotIn('--with-sdl', cmd)

    def test_feature_switches(self):
        preset = autotest.PRESETS['pack-msvc-x64-release']
        cmd = autotest._pack_command(
            preset, 'out', self._args(sdl=True, examples=True))
        self.assertIn('--with-sdl', cmd)
        self.assertIn('--with-examples', cmd)

    def test_synthetic_command_uses_fake_dirs(self):
        args = self._args()
        args.dry_run = True
        with tempfile.TemporaryDirectory() as tmp:
            cmd = autotest._synthetic_command(tmp, args)
        self.assertIn('arm64', cmd)
        self.assertIn('--lib-dir', cmd)
        self.assertIn('--skia-lib-dir', cmd)
        self.assertTrue(any(str(item).endswith(os.path.join('fake', 'lib'))
                            for item in cmd))
        self.assertTrue(any(str(item).endswith(os.path.join('fake', 'skia'))
                            for item in cmd))


class PackageInspectionTest(unittest.TestCase):

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix='autotest_inspect_')

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _make_zip(self, name='nim_duilib-1.0-win-x64.zip', omit=(),
                  arm64_text='processorArchitecture="ARM64"'):
        path = os.path.join(self.tmp, name)
        root = 'nim_duilib-1.0-win-x64'
        members = {
            'MANIFEST.txt': b'x',
            'SHA256SUMS': b'x',
            'lib/cmake/nim_duilib/nim_duilibConfig.cmake': b'x',
            'msvc/manifest/duilib.x64.manifest': b'x',
            'msvc/manifest/duilib.x86.manifest': b'x',
            'msvc/manifest/duilib.arm64.manifest':
                arm64_text.encode('utf-8'),
        }
        with zipfile.ZipFile(path, 'w') as zf:
            for rel, data in members.items():
                if rel in omit:
                    continue
                zf.writestr('%s/%s' % (root, rel), data)
        return path

    def test_complete_package_passes(self):
        ok, errors = autotest.inspect_package(self._make_zip(), 'x64')
        self.assertTrue(ok, errors)

    def test_missing_arm64_manifest_fails(self):
        path = self._make_zip(omit=('msvc/manifest/duilib.arm64.manifest',))
        ok, errors = autotest.inspect_package(path, 'x64')
        self.assertFalse(ok)
        self.assertTrue(any('arm64' in item for item in errors))

    def test_arm64_manifest_content_checked(self):
        path = self._make_zip(arm64_text='processorArchitecture="AMD64"')
        ok, errors = autotest.inspect_package(path, 'arm64')
        self.assertFalse(ok)
        self.assertTrue(any('ARM64' in item for item in errors))

    def test_missing_manifest_txt_fails(self):
        path = self._make_zip(omit=('MANIFEST.txt',))
        ok, errors = autotest.inspect_package(path, 'x64')
        self.assertFalse(ok)
        self.assertTrue(any('MANIFEST.txt' in item for item in errors))


class ReportAndDryRunTest(unittest.TestCase):

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix='autotest_run_')

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def test_new_package_detection(self):
        before = set()
        path = os.path.join(self.tmp, 'a.zip')
        open(path, 'wb').close()
        found = autotest._new_package(self.tmp, before)
        self.assertEqual(found, path)
        self.assertEqual(autotest._new_package(self.tmp, {os.path.basename(path)}), '')

    def test_summary_exit_codes(self):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            self.assertEqual(
                autotest._print_summary([{'name': 'a', 'status': 'passed'}]),
                0)
            self.assertEqual(
                autotest._print_summary([{'name': 'a', 'status': 'failed'}]),
                1)
            self.assertEqual(
                autotest._print_summary([{'name': 'a', 'status': 'skipped'}]),
                2)

    def test_unit_dry_run(self):
        with contextlib.redirect_stdout(io.StringIO()):
            code = autotest.main(['--preset', 'unit', '--dry-run',
                                  '--out-dir', self.tmp])
        self.assertEqual(code, 0)
        self.assertTrue(os.path.isfile(os.path.join(self.tmp, 'autotest.log')))

    def test_arm64_synthetic_dry_run(self):
        with contextlib.redirect_stdout(io.StringIO()):
            code = autotest.main(['--preset', 'arm64-synthetic', '--dry-run',
                                  '--out-dir', self.tmp])
        self.assertEqual(code, 0)


class EnvironmentTest(unittest.TestCase):

    def test_environment_fields(self):
        env = autotest.detect_environment()
        for key in ('platform', 'host_system', 'host_arch', 'python',
                    'repo_root', 'tools'):
            self.assertIn(key, env)
        self.assertIn('cmake', env['tools'])


if __name__ == '__main__':
    unittest.main()
