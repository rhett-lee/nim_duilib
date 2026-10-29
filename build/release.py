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
    python3 build/release.py check
    python3 build/release.py package --config Release --arch x64 --with-cef --dry-run
    python3 build/release.py package --config Release --arch x64 --with-cef
    python3 build/release.py finalize
    python3 build/release.py clean --list
    python3 build/release.py clean --version 1.0.2026.1015 --yes

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

from i18n import make_messages, set_console_utf8  # noqa: E402 - 同上

# 控制台文案：英文在脚本里按 key 引用，中文在 cmake/i18n/messages.json。
# 语言顺序：--lang / DUILIB_LANG -> LC_ALL/LC_MESSAGES/LANG -> 系统区域 -> en。
MESSAGES = make_messages()


def T(key):
    """取控制台文案：简体中文系统显示中文，其余（含 zh_TW/zh_HK）显示英文。"""
    return MESSAGES.t(key)


def _requested_lang(argv):
    """预扫描 --lang：让 --help 与参数错误也按所选语言输出。"""
    for index, arg in enumerate(argv):
        if arg.startswith('--lang='):
            return arg.split('=', 1)[1]
        if arg == '--lang' and index + 1 < len(argv):
            return argv[index + 1]
    return 'auto'


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

    print(T('rel.version') % version)
    failures = []

    # 1. 工作区无已跟踪文件的未提交修改。dist/ 是产物目录，不参与检查。
    code, out = run_git(['status', '--porcelain', '--untracked-files=no'])
    dirty = []
    workspace_ok = True
    if code is None or code != 0:
        workspace_ok = False
        failures.append(T('rel.git_status_failed') % out)
    else:
        for line in out.splitlines():
            if not line.strip():
                continue
            path = _porcelain_path(line)
            if not _is_ignored_path(path):
                dirty.append('%s  %s' % (line[:2], path))
        if dirty:
            workspace_ok = False
            failures.append(T('rel.workspace_dirty') +
                            '\n'.join('    ' + item for item in dirty))
    print(T('rel.workspace_check') % ('OK' if workspace_ok else 'FAIL'))

    # 2. 版本号解析（read_version 已校验缺失、重复与范围）。
    print(T('rel.version_parsed') % version)

    # 3. 当前版本 tag 不存在，且相对最近的 v* tag 严格递增。
    code, out = run_git(['tag', '--list', 'v*'])
    if code is None or code != 0:
        failures.append(T('rel.git_tag_failed') % out)
        print(T('rel.tag_check_fail'))
    else:
        current_tag = 'v' + version
        parsed = []
        for name in out.splitlines():
            match = _VERSION_TAG_RE.match(name.strip())
            if match:
                parsed.append((tuple(int(x) for x in match.groups()), name.strip()))
        if current_tag in [name for _key, name in parsed]:
            failures.append(T('rel.tag_exists') % current_tag)
            print(T('rel.tag_check_fail'))
        elif parsed:
            latest_key, latest_name = max(parsed, key=lambda kv: kv[0])
            if _version_tuple(version) <= latest_key:
                failures.append(T('rel.version_not_increased')
                                % (version, latest_name))
                print(T('rel.tag_check_fail'))
            else:
                print(T('rel.tag_check_ok') % latest_name)
        else:
            print(T('rel.tag_check_empty'))

    # 4. 中英 CHANGELOG 都包含当前版本条目，且版本标题一致。
    changelog_failures = []
    changelog_sets = {}
    for name in CHANGELOG_FILES:
        path = os.path.join(REPO_ROOT, name)
        headings = _changelog_headings(path)
        if headings is None:
            changelog_failures.append(T('rel.changelog_missing') % name)
            continue
        changelog_sets[name] = set(h for h in headings if h.lower() != 'unreleased')
        if version not in headings:
            changelog_failures.append(T('rel.changelog_no_entry')
                                      % (name, version))
    if len(changelog_sets) == len(CHANGELOG_FILES):
        if changelog_sets[CHANGELOG_FILES[0]] != changelog_sets[CHANGELOG_FILES[1]]:
            changelog_failures.append(T('rel.changelog_heading_mismatch')
                                      % (CHANGELOG_FILES[0], CHANGELOG_FILES[1]))
    failures.extend(changelog_failures)
    print(T('rel.changelog_check') % ('OK' if not changelog_failures else 'FAIL'))

    if failures:
        print('')
        eprint(T('rel.check_failed'))
        for item in failures:
            eprint('  - %s' % item)
        return 1

    print('')
    print(T('rel.check_passed'))
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

    print(T('rel.version') % version)
    print(T('rel.platform') % (sys_tag, arch_tag))
    print(T('rel.config') % args.config)
    print(T('rel.output_dir') % os.path.relpath(out_dir, REPO_ROOT))
    print(T('rel.expected_artifact') % os.path.relpath(out_path, REPO_ROOT))
    print(T('rel.command') % format_command(cmd))
    if args.dry_run:
        print(T('rel.dry_run'))
        return 0

    print('-' * 60)
    code = subprocess.call(cmd, cwd=REPO_ROOT)
    if code != 0:
        eprint(T('rel.pack_exit') % code)
        return code
    # 出包后立即对归档做一次体检（布局 / SHA / 文档链接），坏包不进入发布目录
    if not pack.verify_package(out_path):
        eprint(T('rel.pack_check_failed') % os.path.relpath(out_path, REPO_ROOT))
        return 1
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
        return [T('rel.sums_missing')]
    failures = []
    with open(sums_path, 'r', encoding='utf-8') as f:
        for lineno, line in enumerate(f, 1):
            line = line.rstrip('\r\n')
            if not line.strip():
                continue
            parts = line.split('  ', 1)
            if len(parts) != 2:
                failures.append(T('rel.sums_line_invalid') % lineno)
                continue
            digest, rel = parts
            path = os.path.join(pkg_root, rel.replace('/', os.sep))
            if not os.path.isfile(path):
                failures.append(T('rel.sums_file_absent') % rel)
            elif sha256_of(path).lower() != digest.lower():
                failures.append(T('rel.sha_mismatch') % rel)
    return failures


def _strip_inline_code(line):
    return re.sub(r'`[^`]*`', '', line)


def _check_doc_links(pkg_root):
    """检查包内 Markdown 的相对链接是否都能在包内找到目标。"""
    failures = []
    docs_root = os.path.join(pkg_root, 'docs')
    if not os.path.isdir(docs_root):
        return [T('rel.docs_missing')]
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
                            failures.append(T('rel.doc_link_broken')
                                            % (rel_md, lineno, target))
    return failures


def _verify_package(pkg_root, version):
    failures = []
    for rel in REQUIRED_PACKAGE_FILES:
        path = os.path.join(pkg_root, rel.replace('/', os.sep))
        if not os.path.isfile(path):
            failures.append(T('rel.required_missing') % rel)

    version_path = os.path.join(pkg_root, 'VERSION')
    if os.path.isfile(version_path):
        actual = read_text(version_path).strip()
        if actual != version:
            failures.append(T('rel.version_content') % (actual, version))

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
        eprint(T('rel.dir_missing') % os.path.relpath(out_dir, REPO_ROOT))
        return 1
    archives = _find_archives(out_dir)
    if not archives:
        eprint(T('rel.archive_missing') %
               os.path.relpath(out_dir, REPO_ROOT))
        return 1

    print(T('rel.version') % version)
    print(T('rel.dir') % os.path.relpath(out_dir, REPO_ROOT))

    archive_hashes = {}
    failed = False
    for archive in archives:
        name = os.path.basename(archive)
        print('-' * 60)
        print(T('rel.verify') % name)
        temp_dir = tempfile.mkdtemp(prefix='nim_duilib_finalize_')
        try:
            _extract_archive(archive, temp_dir)
            pkg_root = _package_root(temp_dir)
            failures = _verify_package(pkg_root, version)
        except Exception as exc:  # noqa: BLE001 - 解压失败按校验失败处理
            failures = [T('rel.verify_exception') % exc]
        finally:
            shutil.rmtree(temp_dir, ignore_errors=True)

        if failures:
            failed = True
            for item in failures:
                print('  [FAIL] %s' % item)
        else:
            archive_hashes[name] = sha256_of(archive)
            print(T('rel.verify_ok'))

    if failed:
        eprint(T('rel.not_verified'))
        return 1

    # 压缩包级哈希
    sums_lines = ['%s  %s' % (digest, name)
                  for name, digest in sorted(archive_hashes.items())]
    sums_path = os.path.join(out_dir, 'SHA256SUMS.txt')
    with open(sums_path, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(sums_lines) + '\n')
    print('-' * 60)
    print(T('rel.generated') % os.path.relpath(sums_path, REPO_ROOT))

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
        print(T('rel.generated') % os.path.relpath(path, REPO_ROOT))
    if missing:
        eprint(T('rel.notes_missing')
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
        print(T('rel.dist_missing'))
        return 0

    if args.version:
        if not args.yes:
            eprint(T('rel.clean_needs_yes')
                   % args.version)
            return 1
        if args.version == version:
            eprint(T('rel.clean_current_refused') % args.version)
            return 1
        target = os.path.join(DIST_DIR, args.version)
        if not os.path.isdir(target):
            eprint(T('rel.clean_version_missing')
                   % os.path.relpath(target, REPO_ROOT))
            return 1
        shutil.rmtree(target)
        print(T('rel.deleted') % os.path.relpath(target, REPO_ROOT))
        return 0

    # --list 与不带参数行为一致：只列出，不删除。
    entries = sorted(name for name in os.listdir(DIST_DIR)
                     if os.path.isdir(os.path.join(DIST_DIR, name)))
    if not entries:
        print(T('rel.dist_empty'))
        return 0

    print(T('rel.dist_versions'))
    for name in entries:
        directory = os.path.join(DIST_DIR, name)
        archives = _find_archives(directory)
        mark = T('rel.current_marker') if name == version else ''
        print(T('rel.dist_line')
              % (name, len(archives), _human_size(_dir_size(directory)), mark))
    print('')
    print(T('rel.clean_hint'))
    print(T('rel.clean_command'))
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
    raise argparse.ArgumentTypeError(T('rel.config_invalid') % value)


def _add_lang_option(parser, suppress_default=False):
    """--lang 同时挂在主命令与子命令上，写在子命令之后也能生效。"""
    parser.add_argument('--lang', choices=('auto', 'zh_CN', 'en'),
                        default=argparse.SUPPRESS if suppress_default else 'auto',
                        help=T('rel.help_lang'))


def build_parser():
    parser = argparse.ArgumentParser(
        description=T('rel.help_description'),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    _add_lang_option(parser)
    sub = parser.add_subparsers(dest='command')
    sub.required = True

    check = sub.add_parser('check', help=T('rel.help_check'))
    _add_lang_option(check, suppress_default=True)
    check.set_defaults(func=cmd_check)

    package = sub.add_parser('package', help=T('rel.help_package'))
    _add_lang_option(package, suppress_default=True)
    package.add_argument('--config', type=_parse_config, default='Release',
                         metavar='{Release,Debug}', help=T('rel.help_config'))
    package.add_argument('--arch', choices=('x64', 'x86', 'arm64'), default=None,
                         help=T('rel.help_arch'))
    package.add_argument('--with-sdl', action='store_true',
                         help=T('rel.help_with_sdl'))
    package.add_argument('--with-cef', action='store_true',
                         help=T('rel.help_with_cef'))
    package.add_argument('--cef-version', choices=('latest', '109'),
                         default='latest',
                         help=T('rel.help_cef_version'))
    package.add_argument('--with-examples', action='store_true',
                         help=T('rel.help_with_examples'))
    package.add_argument('--msvc-runtime', choices=('auto', 'mt', 'md'),
                         default='auto',
                         help=T('rel.help_msvc_runtime'))
    package.add_argument('--mingw-variant', choices=('auto', 'gcc', 'llvm'),
                         default='auto',
                         help=T('rel.help_mingw_variant'))
    package.add_argument('--format', choices=('zip', 'tgz', 'tarbz2'),
                         default=None, help=T('rel.help_format'))
    package.add_argument('--dry-run', action='store_true',
                         help=T('rel.help_dry_run'))
    package.set_defaults(func=cmd_package)

    finalize = sub.add_parser('finalize', help=T('rel.help_verify'))
    _add_lang_option(finalize, suppress_default=True)
    finalize.set_defaults(func=cmd_finalize)

    clean = sub.add_parser('clean', help=T('rel.help_clean'))
    _add_lang_option(clean, suppress_default=True)
    clean.add_argument('--list', action='store_true',
                       help=T('rel.help_list'))
    clean.add_argument('--version', default=None,
                       help=T('rel.help_version'))
    clean.add_argument('--yes', action='store_true',
                       help=T('rel.help_yes'))
    clean.set_defaults(func=cmd_clean)

    return parser


def main(argv=None):
    # Console language must be ready before build_parser(): --help is output too.
    global MESSAGES
    argv = sys.argv[1:] if argv is None else list(argv)
    MESSAGES = make_messages(_requested_lang(argv))
    set_console_utf8()

    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return args.func(args)
    except KeyboardInterrupt:
        eprint(T('rel.interrupted'))
        return 2
    except Exception as exc:  # noqa: BLE001 - 顶层兜底，给出可读错误
        eprint('[ERROR] %s' % exc)
        return 2


if __name__ == '__main__':
    sys.exit(main())
