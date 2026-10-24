# -*- coding: utf-8 -*-
"""nim_duilib 发布流程编排脚本。

本脚本只做编排与校验，不替代 build/pack.py，也不执行任何 git 写操作：
commit / tag / push 只在检查结束后打印建议命令，由维护者手动执行。

子命令：
    check      版本、工作区、tag、中英 CHANGELOG 一致性检查
    package    调用 pack.py 打包到 dist/<版本>/
    finalize   校验 dist/<版本>/ 下压缩包的 SHA256、必备文件与文档链接，
               生成压缩包级 SHA256SUMS.txt 与中英 RELEASE-NOTES
    clean      列出或手动删除 dist/<版本>/，默认不删除任何内容

用法：
    python build/release.py check
    python build/release.py package --config Release --arch x64 --with-cef --dry-run
    python build/release.py package --config Release --arch x64 --with-cef
    python build/release.py finalize
    python build/release.py clean --list
    python build/release.py clean --version 1.0.2026.1015 --yes

退出码：0 成功；1 检查或校验失败；2 执行异常。
"""

import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
import urllib.parse
import zipfile

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
DIST_DIR = os.path.join(REPO_ROOT, 'dist')
PACK_SCRIPT = os.path.join(SCRIPT_DIR, 'pack.py')

if SCRIPT_DIR not in sys.path:
    sys.path.insert(0, SCRIPT_DIR)

import pack  # noqa: E402 - 复用版本解析与包名规则，必须先把脚本目录加入 sys.path

CHANGELOG_FILES = ('CHANGELOG.md', 'CHANGELOG.en.md')
REQUIRED_PACKAGE_FILES = ('MANIFEST.txt', 'VERSION',
                          'include/duilib/duilib.h')
_ARCHIVE_EXTS = ('.zip', '.tar.gz', '.tar.bz2')
_VERSION_TAG_RE = re.compile(r'^v(\d+)\.(\d+)\.(\d+)\.(\d+)$')
_CHANGELOG_HEADING_RE = re.compile(r'^##\s*\[([^\]]+)\]', re.M)
_MD_LINK_RE = re.compile(r'\[[^\]]*\]\(([^)]+)\)')


def eprint(*args):
    sys.stderr.write(' '.join(str(a) for a in args) + '\n')


def run_git(args):
    """返回 (退出码, 标准输出)；git 不可用时退出码为 None。"""
    try:
        proc = subprocess.run(['git'] + args, cwd=REPO_ROOT,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                              universal_newlines=True)
    except OSError as exc:
        return None, str(exc)
    # 不能 strip() 整个输出：git status --porcelain 的首行以状态列空格开头，
    # 去掉后 _porcelain_path 会把路径首字符一起吃掉。
    return proc.returncode, proc.stdout.replace('\r\n', '\n').strip('\n')


def read_text(path):
    with open(path, 'r', encoding='utf-8-sig') as f:
        return f.read()


def sha256_of(path):
    digest = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def format_command(cmd):
    if os.name == 'nt':
        return subprocess.list2cmdline(cmd)
    return ' '.join(cmd)


def _porcelain_path(line):
    """从 git status --porcelain 的一行中取出路径。"""
    path = line[3:] if len(line) > 3 else ''
    if ' -> ' in path:
        path = path.split(' -> ', 1)[1]
    return path.strip().strip('"')


def _is_ignored_path(path):
    norm = path.replace('\\', '/')
    return norm == 'dist' or norm.startswith('dist/')


def _version_tuple(version):
    return tuple(int(part) for part in version.split('.'))


def _changelog_headings(path):
    if not os.path.isfile(path):
        return None
    return _CHANGELOG_HEADING_RE.findall(read_text(path))


# --------------------------------------------------------------------------
# check
# --------------------------------------------------------------------------

def cmd_check(args):
    del args
    version = pack.read_version()
    if version is None:
        return 1

    print('版本      : %s' % version)
    failures = []

    # 1. 工作区无已跟踪文件的未提交修改。dist/ 是产物目录，不参与检查。
    code, out = run_git(['status', '--porcelain', '--untracked-files=no'])
    dirty = []
    workspace_ok = True
    if code is None or code != 0:
        workspace_ok = False
        failures.append('执行 git status 失败：%s' % out)
    else:
        for line in out.splitlines():
            if not line.strip():
                continue
            path = _porcelain_path(line)
            if not _is_ignored_path(path):
                dirty.append('%s  %s' % (line[:2], path))
        if dirty:
            workspace_ok = False
            failures.append('工作区有已跟踪文件的未提交修改：\n' +
                            '\n'.join('    ' + item for item in dirty))
    print('[%s] 工作区检查' % ('OK' if workspace_ok else 'FAIL'))

    # 2. 版本号解析（read_version 已校验缺失、重复与范围）。
    print('[OK] 版本号解析：%s' % version)

    # 3. 当前版本 tag 不存在，且相对最近的 v* tag 严格递增。
    code, out = run_git(['tag', '--list', 'v*'])
    if code is None or code != 0:
        failures.append('执行 git tag 失败：%s' % out)
        print('[FAIL] tag 检查')
    else:
        current_tag = 'v' + version
        parsed = []
        for name in out.splitlines():
            match = _VERSION_TAG_RE.match(name.strip())
            if match:
                parsed.append((tuple(int(x) for x in match.groups()), name.strip()))
        if current_tag in [name for _key, name in parsed]:
            failures.append('tag %s 已存在，不能重复发布' % current_tag)
            print('[FAIL] tag 检查')
        elif parsed:
            latest_key, latest_name = max(parsed, key=lambda kv: kv[0])
            if _version_tuple(version) <= latest_key:
                failures.append('版本未严格递增：当前 %s 不大于最近 tag %s'
                                % (version, latest_name))
                print('[FAIL] tag 检查')
            else:
                print('[OK] tag 检查：最近 tag 为 %s' % latest_name)
        else:
            print('[OK] tag 检查：尚无 v* 版本 tag')

    # 4. 中英 CHANGELOG 都包含当前版本条目，且版本标题一致。
    changelog_failures = []
    changelog_sets = {}
    for name in CHANGELOG_FILES:
        path = os.path.join(REPO_ROOT, name)
        headings = _changelog_headings(path)
        if headings is None:
            changelog_failures.append('缺少 CHANGELOG：%s' % name)
            continue
        changelog_sets[name] = set(h for h in headings if h.lower() != 'unreleased')
        if version not in headings:
            changelog_failures.append('%s 没有当前版本 [%s] 的条目'
                                      % (name, version))
    if len(changelog_sets) == len(CHANGELOG_FILES):
        if changelog_sets[CHANGELOG_FILES[0]] != changelog_sets[CHANGELOG_FILES[1]]:
            changelog_failures.append('中英 CHANGELOG 的版本标题不一致：%s / %s'
                                      % (CHANGELOG_FILES[0], CHANGELOG_FILES[1]))
    failures.extend(changelog_failures)
    print('[%s] CHANGELOG 检查' % ('OK' if not changelog_failures else 'FAIL'))

    if failures:
        print('')
        eprint('检查未通过：')
        for item in failures:
            eprint('  - %s' % item)
        return 1

    print('')
    print('检查通过。以下命令由维护者手动执行：')
    print('  git add CHANGELOG.md CHANGELOG.en.md duilib/duilib_version.h')
    print('  git commit -m "release: %s"' % version)
    print('  git tag -a v%s -m "release %s"' % (version, version))
    print('  git push origin --tags')
    return 0


# --------------------------------------------------------------------------
# package
# --------------------------------------------------------------------------

def cmd_package(args):
    version = pack.read_version()
    if version is None:
        return 1

    sys_tag, host_arch = pack.detect_platform_tag()
    arch_tag = args.arch or host_arch
    out_dir = os.path.join(DIST_DIR, version)

    cmd = [sys.executable, PACK_SCRIPT,
           '--config', args.config,
           '--arch', arch_tag,
           '--out-dir', out_dir]
    if args.with_sdl:
        cmd.append('--with-sdl')
    if args.with_cef:
        cmd.extend(['--with-cef', '--cef-version', args.cef_version])
    if args.with_examples:
        cmd.append('--with-examples')
    if args.msvc_runtime != 'auto':
        cmd.extend(['--msvc-runtime', args.msvc_runtime])
    if args.mingw_variant != 'auto':
        cmd.extend(['--mingw-variant', args.mingw_variant])
    if args.format:
        cmd.extend(['--format', args.format])

    mingw_variant = None
    if sys_tag == 'win':
        if args.mingw_variant != 'auto':
            mingw_variant = args.mingw_variant
        else:
            # dry-run 时库可能尚未构建，探测失败则按无后缀包名展示
            try:
                lib_dir = pack.resolve_lib_dir(None, args.config, arch_tag)
                if pack.detect_windows_toolchain(lib_dir) == 'mingw':
                    mingw_variant = pack.resolve_mingw_variant(lib_dir, 'auto')
            except pack.PackageError:
                mingw_variant = None

    fmt = args.format or ('zip' if sys_tag == 'win' else 'tgz')
    ext = {'zip': '.zip', 'tgz': '.tar.gz', 'tarbz2': '.tar.bz2'}[fmt]
    pkg_name = pack.build_package_name(
        version, sys_tag, arch_tag, args.config,
        args.with_sdl, args.with_cef, args.cef_version, args.with_examples,
        mingw_variant=mingw_variant)
    out_path = os.path.join(out_dir, pkg_name + ext)

    print('版本      : %s' % version)
    print('平台      : %s-%s' % (sys_tag, arch_tag))
    print('配置      : %s' % args.config)
    print('输出目录  : %s' % os.path.relpath(out_dir, REPO_ROOT))
    print('预计产物  : %s' % os.path.relpath(out_path, REPO_ROOT))
    print('执行命令  : %s' % format_command(cmd))
    if args.dry_run:
        print('（--dry-run：未执行打包）')
        return 0

    print('-' * 60)
    code = subprocess.call(cmd, cwd=REPO_ROOT)
    if code != 0:
        eprint('[ERROR] pack.py 退出码 %d' % code)
        return code
    return 0


# --------------------------------------------------------------------------
# finalize：校验压缩包并生成发布说明
# --------------------------------------------------------------------------

def _find_archives(directory):
    result = []
    for name in sorted(os.listdir(directory)):
        lower = name.lower()
        if lower.endswith(_ARCHIVE_EXTS):
            result.append(os.path.join(directory, name))
    return result


def _extract_archive(archive, dest):
    if archive.lower().endswith('.zip'):
        with zipfile.ZipFile(archive) as zf:
            zf.extractall(dest)
    else:
        with tarfile.open(archive, 'r:*') as tf:
            try:
                tf.extractall(dest, filter='data')
            except TypeError:  # Python < 3.12 没有 extraction filter
                tf.extractall(dest)


def _package_root(extract_dir):
    entries = [os.path.join(extract_dir, name)
               for name in os.listdir(extract_dir)]
    dirs = [p for p in entries if os.path.isdir(p)]
    if len(dirs) == 1:
        return dirs[0]
    return extract_dir


def _verify_sha256sums(pkg_root):
    sums_path = os.path.join(pkg_root, 'SHA256SUMS')
    if not os.path.isfile(sums_path):
        return ['缺少 SHA256SUMS']
    failures = []
    with open(sums_path, 'r', encoding='utf-8') as f:
        for lineno, line in enumerate(f, 1):
            line = line.rstrip('\r\n')
            if not line.strip():
                continue
            parts = line.split('  ', 1)
            if len(parts) != 2:
                failures.append('SHA256SUMS 第 %d 行格式错误' % lineno)
                continue
            digest, rel = parts
            path = os.path.join(pkg_root, rel.replace('/', os.sep))
            if not os.path.isfile(path):
                failures.append('SHA256SUMS 引用的文件不存在：%s' % rel)
            elif sha256_of(path).lower() != digest.lower():
                failures.append('SHA256 不匹配：%s' % rel)
    return failures


def _strip_inline_code(line):
    return re.sub(r'`[^`]*`', '', line)


def _check_doc_links(pkg_root):
    """检查包内 Markdown 的相对链接是否都能在包内找到目标。"""
    failures = []
    docs_root = os.path.join(pkg_root, 'docs')
    if not os.path.isdir(docs_root):
        return ['缺少 docs/ 目录']
    for current, _dirs, files in os.walk(docs_root):
        for name in sorted(files):
            if not name.lower().endswith('.md'):
                continue
            md_path = os.path.join(current, name)
            rel_md = os.path.relpath(md_path, pkg_root).replace('\\', '/')
            in_fence = False
            with open(md_path, 'r', encoding='utf-8-sig') as f:
                for lineno, line in enumerate(f, 1):
                    stripped = line.lstrip()
                    if stripped.startswith('```') or stripped.startswith('~~~'):
                        in_fence = not in_fence
                        continue
                    if in_fence:
                        continue
                    for target in _MD_LINK_RE.findall(_strip_inline_code(line)):
                        target = target.strip()
                        if target.startswith('<') and target.endswith('>'):
                            target = target[1:-1].strip()
                        if not target or target.startswith('#'):
                            continue
                        if re.match(r'^[a-zA-Z][a-zA-Z0-9+.-]*:', target):
                            continue
                        path_part = urllib.parse.unquote(target.split('#', 1)[0])
                        if not path_part:
                            continue
                        resolved = os.path.normpath(
                            os.path.join(current, path_part.replace('/', os.sep)))
                        if not os.path.exists(resolved):
                            failures.append('%s:%d 链接目标不存在：%s'
                                            % (rel_md, lineno, target))
    return failures


def _verify_package(pkg_root, version):
    failures = []
    for rel in REQUIRED_PACKAGE_FILES:
        path = os.path.join(pkg_root, rel.replace('/', os.sep))
        if not os.path.isfile(path):
            failures.append('缺少必备文件：%s' % rel)

    version_path = os.path.join(pkg_root, 'VERSION')
    if os.path.isfile(version_path):
        actual = read_text(version_path).strip()
        if actual != version:
            failures.append('VERSION 内容为 %r，期望 %r' % (actual, version))

    failures.extend(_verify_sha256sums(pkg_root))
    failures.extend(_check_doc_links(pkg_root))
    return failures


def _extract_changelog_section(path, version):
    text = read_text(path)
    match = re.search(r'^##\s*\[' + re.escape(version) + r'\][^\n]*$',
                      text, re.M)
    if match is None:
        return None
    start = match.end()
    following = re.search(r'^##\s*\[', text[start:], re.M)
    end = start + following.start() if following else len(text)
    return text[start:end].strip()


def cmd_finalize(args):
    del args
    version = pack.read_version()
    if version is None:
        return 1

    out_dir = os.path.join(DIST_DIR, version)
    if not os.path.isdir(out_dir):
        eprint('[ERROR] 目录不存在：%s' % os.path.relpath(out_dir, REPO_ROOT))
        return 1
    archives = _find_archives(out_dir)
    if not archives:
        eprint('[ERROR] 未找到压缩包：%s' %
               os.path.relpath(out_dir, REPO_ROOT))
        return 1

    print('版本      : %s' % version)
    print('目录      : %s' % os.path.relpath(out_dir, REPO_ROOT))

    archive_hashes = {}
    failed = False
    for archive in archives:
        name = os.path.basename(archive)
        print('-' * 60)
        print('校验      : %s' % name)
        temp_dir = tempfile.mkdtemp(prefix='nim_duilib_finalize_')
        try:
            _extract_archive(archive, temp_dir)
            pkg_root = _package_root(temp_dir)
            failures = _verify_package(pkg_root, version)
        except Exception as exc:  # noqa: BLE001 - 解压失败按校验失败处理
            failures = ['解压或校验异常：%s' % exc]
        finally:
            shutil.rmtree(temp_dir, ignore_errors=True)

        if failures:
            failed = True
            for item in failures:
                print('  [FAIL] %s' % item)
        else:
            archive_hashes[name] = sha256_of(archive)
            print('  [OK] SHA256、必备文件、文档链接')

    if failed:
        eprint('[ERROR] 存在未通过校验的压缩包，未生成发布说明')
        return 1

    # 压缩包级哈希
    sums_lines = ['%s  %s' % (digest, name)
                  for name, digest in sorted(archive_hashes.items())]
    sums_path = os.path.join(out_dir, 'SHA256SUMS.txt')
    with open(sums_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(sums_lines) + '\n')
    print('-' * 60)
    print('已生成    : %s' % os.path.relpath(sums_path, REPO_ROOT))

    # 中英发布说明
    notes = (('CHANGELOG.md', 'RELEASE-NOTES.md', '发布说明'),
             ('CHANGELOG.en.md', 'RELEASE-NOTES.en.md', 'Release Notes'))
    missing = []
    for source, target, title in notes:
        section = _extract_changelog_section(
            os.path.join(REPO_ROOT, source), version)
        if section is None:
            missing.append(source)
            continue
        text = '# nim_duilib %s %s\n\n%s\n' % (version, title, section)
        path = os.path.join(out_dir, target)
        with open(path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(text)
        print('已生成    : %s' % os.path.relpath(path, REPO_ROOT))
    if missing:
        eprint('[ERROR] 以下文件缺少版本 [%s] 条目：%s'
               % (version, ', '.join(missing)))
        return 1
    return 0


# --------------------------------------------------------------------------
# clean：默认只列出，不删除
# --------------------------------------------------------------------------

def _dir_size(path):
    total = 0
    for current, _dirs, files in os.walk(path):
        for name in files:
            try:
                total += os.path.getsize(os.path.join(current, name))
            except OSError:
                pass
    return total


def _human_size(num_bytes):
    value = float(num_bytes)
    for unit in ('B', 'KB', 'MB', 'GB'):
        if abs(value) < 1024.0 or unit == 'GB':
            return '%.1f %s' % (value, unit)
        value /= 1024.0
    return '%.1f GB' % value


def cmd_clean(args):
    version = pack.read_version()
    if version is None:
        return 1

    if not os.path.isdir(DIST_DIR):
        print('dist/ 不存在，无需清理')
        return 0

    if args.version:
        if not args.yes:
            eprint('[ERROR] 删除需要显式 --yes：'
                   'python build/release.py clean --version %s --yes'
                   % args.version)
            return 1
        if args.version == version:
            eprint('[ERROR] 拒绝删除当前版本目录：%s' % args.version)
            return 1
        target = os.path.join(DIST_DIR, args.version)
        if not os.path.isdir(target):
            eprint('[ERROR] 版本目录不存在：%s'
                   % os.path.relpath(target, REPO_ROOT))
            return 1
        shutil.rmtree(target)
        print('已删除    : %s' % os.path.relpath(target, REPO_ROOT))
        return 0

    # --list 与不带参数行为一致：只列出，不删除。
    entries = sorted(name for name in os.listdir(DIST_DIR)
                     if os.path.isdir(os.path.join(DIST_DIR, name)))
    if not entries:
        print('dist/ 下没有版本目录')
        return 0

    print('dist/ 版本目录：')
    for name in entries:
        directory = os.path.join(DIST_DIR, name)
        archives = _find_archives(directory)
        mark = '（当前版本）' if name == version else ''
        print('  %-16s %2d 个包  %8s %s'
              % (name, len(archives), _human_size(_dir_size(directory)), mark))
    print('')
    print('默认不删除任何内容；如需删除旧版本：')
    print('  python build/release.py clean --version <版本> --yes')
    return 0


# --------------------------------------------------------------------------
# 命令行
# --------------------------------------------------------------------------

def _parse_config(value):
    norm = value.strip().lower()
    if norm == 'release':
        return 'Release'
    if norm == 'debug':
        return 'Debug'
    raise argparse.ArgumentTypeError('配置只能是 Release 或 Debug：%r' % value)


def build_parser():
    parser = argparse.ArgumentParser(
        description='nim_duilib 发布流程编排（不执行 git 写操作）',
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    sub = parser.add_subparsers(dest='command')
    sub.required = True

    check = sub.add_parser('check', help='版本、工作区、tag、CHANGELOG 检查')
    check.set_defaults(func=cmd_check)

    package = sub.add_parser('package', help='调用 pack.py 打包到 dist/<版本>/')
    package.add_argument('--config', type=_parse_config, default='Release',
                         metavar='{Release,Debug}', help='库配置（默认 Release）')
    package.add_argument('--arch', choices=('x64', 'x86', 'arm64'), default=None,
                         help='目标架构（默认主机架构）')
    package.add_argument('--with-sdl', action='store_true',
                         help='Windows 下额外打包 SDL3')
    package.add_argument('--with-cef', action='store_true',
                         help='额外打包 CEF 开发包')
    package.add_argument('--cef-version', choices=('latest', '109'),
                         default='latest', help='CEF 版本（默认 latest）')
    package.add_argument('--with-examples', action='store_true',
                         help='包含示例程序')
    package.add_argument('--msvc-runtime', choices=('auto', 'mt', 'md'),
                         default='auto', help='MSVC 运行库（默认 auto）')
    package.add_argument('--mingw-variant', choices=('auto', 'gcc', 'llvm'),
                         default='auto',
                         help='MinGW 编译器变体（默认自动判断），'
                              '包名追加 -mingw-gcc / -mingw-llvm')
    package.add_argument('--format', choices=('zip', 'tgz', 'tarbz2'),
                         default=None, help='压缩格式（默认 Windows zip、其它 tgz）')
    package.add_argument('--dry-run', action='store_true',
                         help='只打印命令与最终包名，不执行打包')
    package.set_defaults(func=cmd_package)

    finalize = sub.add_parser('finalize', help='校验 dist/<版本>/ 下压缩包')
    finalize.set_defaults(func=cmd_finalize)

    clean = sub.add_parser('clean', help='列出或手动删除 dist/<版本>/')
    clean.add_argument('--list', action='store_true',
                       help='只列出版本目录与占用（默认行为）')
    clean.add_argument('--version', default=None,
                       help='要删除的版本目录名，必须同时给 --yes')
    clean.add_argument('--yes', action='store_true',
                       help='确认删除')
    clean.set_defaults(func=cmd_clean)

    return parser


def main(argv=None):
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return args.func(args)
    except KeyboardInterrupt:
        eprint('[ERROR] 已中断')
        return 2
    except Exception as exc:  # noqa: BLE001 - 顶层兜底，给出可读错误
        eprint('[ERROR] %s' % exc)
        return 2


if __name__ == '__main__':
    sys.exit(main())
