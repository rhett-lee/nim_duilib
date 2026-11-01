# -*- coding: utf-8 -*-
"""build/release.py 的最小回归测试：包名预判、归档发现与 CHANGELOG 提取。

不依赖已构建产物，可直接运行：

    python3 build/test_release.py

覆盖：DLL 模式预判（包名是否追加 `-dll`）、出包命令的语言转发、
输出目录里的归档筛选、`git status --porcelain` 的路径解析与 `dist/` 忽略规则、
发布说明条目提取。
"""
import contextlib
import io
import os
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import release  # noqa: E402

# 固定简体中文：断言基于中文文案，不能随宿主系统语言变化。
release.MESSAGES = release.make_messages('zh_CN')


class _TempDirTest(unittest.TestCase):

    def setUp(self):
        self._temp = tempfile.TemporaryDirectory()
        self.temp_dir = self._temp.name

    def tearDown(self):
        self._temp.cleanup()

    def _touch(self, name, content=b''):
        path = os.path.join(self.temp_dir, name)
        with open(path, 'wb') as handle:
            handle.write(content)
        return path


class ExpectedRuntimeDllTest(_TempDirTest):
    """包名预判必须与 pack.py 的 DLL 收集规则一致（Debug 只认 `_d`）。"""

    def _expect(self, sys_tag, arch_tag, config, files):
        # 每个场景都从空的 bin/ 开始，避免上一组文件残留影响判定。
        for name in os.listdir(self.temp_dir):
            os.remove(os.path.join(self.temp_dir, name))
        for name in files:
            self._touch(name)
        with mock.patch.object(release.pack, 'BIN_DIR', self.temp_dir):
            found = release._expected_runtime_dll(sys_tag, arch_tag, config)
        return os.path.basename(found) if found else None

    def test_non_windows_has_no_dll_mode(self):
        with mock.patch.object(release.pack, 'BIN_DIR', self.temp_dir):
            self._touch('duilib.dll')
            self.assertIsNone(release._expected_runtime_dll('linux', 'x64', 'Release'))
            self.assertIsNone(release._expected_runtime_dll('macos', 'arm64', 'Debug'))

    def test_x64_release_and_debug(self):
        self.assertEqual(self._expect('win', 'x64', 'Release', ['duilib.dll']),
                         'duilib.dll')
        self.assertEqual(self._expect('win', 'x64', 'Debug', ['duilib_d.dll']),
                         'duilib_d.dll')

    def test_config_does_not_fall_back_to_other_config_dll(self):
        # Debug 包只收集 duilib_d.dll；bin/ 里只有 Release 的 duilib.dll 时
        # 包名不能追加 -dll（pack.py 此时也不会收集该 DLL）。
        self.assertIsNone(self._expect('win', 'x64', 'Debug', ['duilib.dll']))
        self.assertIsNone(self._expect('win', 'x64', 'Release', ['duilib_d.dll']))

    def test_x86_uses_duilib32_names(self):
        self.assertEqual(self._expect('win', 'x86', 'Release', ['duilib32.dll']),
                         'duilib32.dll')
        self.assertEqual(self._expect('win', 'x86', 'Debug', ['duilib32_d.dll']),
                         'duilib32_d.dll')
        self.assertIsNone(self._expect('win', 'x86', 'Release', ['duilib.dll']))

    def test_dll_mode_name_gets_suffix(self):
        # 预判结果直接喂给 build_package_name：DLL 模式包名必须追加 -dll。
        with mock.patch.object(release.pack, 'BIN_DIR', self.temp_dir):
            self._touch('duilib.dll')
            is_dll = release._expected_runtime_dll('win', 'x64', 'Release') is not None
        name = release.pack.build_package_name(
            '1.0', 'win', 'x64', 'Release', False, False, 'latest', False,
            is_dll=is_dll)
        self.assertEqual(name, 'nim_duilib-1.0-win-x64-dll')


class FindArchivesTest(_TempDirTest):
    """release.py 只用输出目录里的压缩包做校验，其它文件必须忽略。"""

    def test_only_archives_are_listed(self):
        self._touch('a.zip')
        self._touch('b.tar.gz')
        self._touch('c.tar.bz2')
        self._touch('SHA256SUMS.txt')
        self._touch('RELEASE-NOTES.md')
        found = [os.path.basename(path) for path in release._find_archives(self.temp_dir)]
        self.assertEqual(found, ['a.zip', 'b.tar.gz', 'c.tar.bz2'])

    def test_empty_directory(self):
        self.assertEqual(release._find_archives(self.temp_dir), [])


class PorcelainPathTest(unittest.TestCase):
    """git status --porcelain 的路径解析与 dist/ 忽略规则。"""

    def test_status_columns_are_skipped(self):
        self.assertEqual(release._porcelain_path(' M docs/Release.md'),
                         'docs/Release.md')
        # 不能 strip 整行后取 [3:]：首行的状态列以空格开头
        self.assertEqual(release._porcelain_path(' D  build/pack.py'),
                         'build/pack.py')

    def test_rename_takes_destination(self):
        self.assertEqual(release._porcelain_path('R  old.md -> new.md'), 'new.md')

    def test_quoted_path_is_unquoted(self):
        self.assertEqual(release._porcelain_path('?? "docs/a b.md"'), 'docs/a b.md')

    def test_short_line_is_tolerated(self):
        self.assertEqual(release._porcelain_path(''), '')
        self.assertEqual(release._porcelain_path(' M '), '')

    def test_dist_directory_is_ignored(self):
        self.assertTrue(release._is_ignored_path('dist'))
        self.assertTrue(release._is_ignored_path('dist/1.0/a.zip'))
        self.assertTrue(release._is_ignored_path('dist\\1.0\\a.zip'))
        # 只是名字里带 dist 的普通路径不能被忽略
        self.assertFalse(release._is_ignored_path('build/dist'))
        self.assertFalse(release._is_ignored_path('docs/distribution.md'))


class ChangelogSectionTest(_TempDirTest):
    """RELEASE-NOTES 由 CHANGELOG 中当前版本的条目生成。"""

    TEXT = (
        '# 更新日志\n\n'
        '## [Unreleased]\n\n'
        '### 新增\n\n'
        '- 未发布内容\n\n'
        '## [1.0.2026.1023]\n\n'
        '### 修复\n\n'
        '- 修复甲\n'
        '- 修复乙\n\n'
        '## [1.0.2026.1020]\n\n'
        '### 修复\n\n'
        '- 旧版本内容\n'
    )

    def _write(self):
        path = os.path.join(self.temp_dir, 'CHANGELOG.md')
        with open(path, 'w', encoding='utf-8', newline='\n') as handle:
            handle.write(self.TEXT)
        return path

    def test_section_stops_at_next_version(self):
        section = release._extract_changelog_section(self._write(), '1.0.2026.1023')
        self.assertIsNotNone(section)
        self.assertIn('修复甲', section)
        self.assertIn('修复乙', section)
        self.assertNotIn('旧版本内容', section)
        self.assertNotIn('未发布内容', section)

    def test_last_section_runs_to_end_of_file(self):
        section = release._extract_changelog_section(self._write(), '1.0.2026.1020')
        self.assertEqual(section.strip(), '### 修复\n\n- 旧版本内容')

    def test_missing_version_returns_none(self):
        self.assertIsNone(release._extract_changelog_section(self._write(),
                                                           '1.0.2026.1101'))


class PackageLanguageTest(unittest.TestCase):
    """调用 pack.py 出包时必须带上解析后的语言（pack.py 不继承 --lang）。"""

    def test_pack_language_follows_release_language(self):
        args = release.build_parser().parse_args(['package', '--dry-run'])
        old = release.MESSAGES
        release.MESSAGES = release.make_messages('en')
        output = io.StringIO()
        try:
            with contextlib.redirect_stdout(output), \
                    mock.patch.object(release.pack, 'read_version',
                                      return_value='1.0.0'):
                code = release.cmd_package(args)
        finally:
            release.MESSAGES = old
        self.assertEqual(code, 0)
        self.assertIn('--lang en', output.getvalue())


if __name__ == '__main__':
    unittest.main()
