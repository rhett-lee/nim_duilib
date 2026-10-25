#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""打包自动化：环境检测 ->（可选）构建 -> 打包 -> 体检 ->（可选）冒烟 -> 报告。

定位：本地可重复执行的打包自动化，不接入 CI。默认只读现有产物，
不写仓库的 bin/、lib/、dist/；所有输出放在 --out-dir（默认
C:\\develop\\temp\\autotest\\<时间戳>）。

用法：
    python build/autotest.py --list-presets
    python build/autotest.py --preset unit
    python build/autotest.py --preset pack-msvc-x64-release
    python build/autotest.py --preset arm64-synthetic
    python build/autotest.py --preset all-ready
    python build/autotest.py --preset pack-msvc-x64-release --build
    python build/autotest.py --preset ... --dry-run

退出码：0 有预设通过且无失败；1 存在失败；2 全部跳过（前置条件不足）。
"""

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
import time
import zipfile

BUILD_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(BUILD_DIR)
sys.path.insert(0, BUILD_DIR)

import pack  # noqa: E402

if os.name == 'nt':
    DEFAULT_OUT_ROOT = os.path.join('C:\\develop\\temp', 'autotest')
else:
    DEFAULT_OUT_ROOT = os.path.join('/tmp', 'autotest')

PRESETS = {
    'unit': {
        'kind': 'unit',
        'description': '运行 test_pack.py 与 test_autotest.py',
    },
    'pack-msvc-x64-release': {
        'kind': 'pack',
        'arch': 'x64',
        'config': 'Release',
        'toolchain': 'msvc',
        'lib_dir': os.path.join('lib', 'x64', 'Release'),
        'description': 'MSVC x64 Release：现有产物打包 + 体检',
        'build': 'msvc',
    },
    'pack-msvc-x64-debug': {
        'kind': 'pack',
        'arch': 'x64',
        'config': 'Debug',
        'toolchain': 'msvc',
        'lib_dir': os.path.join('lib', 'x64', 'Debug'),
        'description': 'MSVC x64 Debug：现有产物打包 + 体检',
    },
    'pack-msvc-x86-release': {
        'kind': 'pack',
        'arch': 'x86',
        'config': 'Release',
        'toolchain': 'msvc',
        'lib_dir': os.path.join('lib', 'x86', 'Release'),
        'description': 'MSVC x86 Release：现有产物打包 + 体检',
    },
    'pack-mingw-x64-release': {
        'kind': 'pack',
        'arch': 'x64',
        'config': 'Release',
        'toolchain': 'mingw',
        'lib_dir': os.path.join('lib', 'x64', 'Release'),
        'description': 'MinGW x64 Release：现有产物打包 + 体检',
        'build': 'mingw',
    },
    'arm64-synthetic': {
        'kind': 'synthetic-arm64',
        'description': '用临时假库产出 arm64 包，校验 manifest/接入文件（无需真机）',
    },
}

ALL_PRESETS = [name for name in PRESETS]


def _now_stamp():
    return time.strftime('%Y%m%d_%H%M%S')


def _which(*names):
    for name in names:
        path = shutil.which(name)
        if path:
            return path
    return ''


def detect_environment():
    sys_tag, host_arch = pack.detect_platform_tag()
    return {
        'platform': platform.platform(),
        'machine': platform.machine(),
        'host_system': sys_tag,
        'host_arch': host_arch,
        'python': sys.version.split()[0],
        'python_exe': sys.executable,
        'repo_root': REPO_ROOT,
        'version': pack.read_version() or '',
        'tools': {
            'cmake': _which('cmake'),
            'cl': _which('cl'),
            'gcc': _which('gcc'),
            'clang': _which('clang'),
            'bash': _which('bash'),
            'cmd': _which('cmd'),
        },
    }


def _lib_ext(toolchain):
    return ('.lib',) if toolchain == 'msvc' else ('.a', '.dll.a')


def check_prereq(preset):
    """返回 (ok, reason)。只检查，不创建任何文件。"""
    kind = preset['kind']
    if kind in ('unit', 'synthetic-arm64'):
        return True, ''

    lib_dir = os.path.join(REPO_ROOT, preset['lib_dir'])
    if not os.path.isdir(lib_dir):
        return False, '库目录不存在：%s' % preset['lib_dir']

    names = {name.lower() for name in os.listdir(lib_dir)}
    exts = _lib_ext(preset['toolchain'])
    for base in pack.LIB_BASENAMES:
        if not any(name.startswith(base.lower()) and name.endswith(exts)
                   for name in names):
            return False, '库目录缺少 %s 的 %s' % (base, '/'.join(exts))

    try:
        toolchain = pack.detect_windows_toolchain(lib_dir)
        variant = None
        if toolchain == 'mingw':
            variant = pack.resolve_mingw_variant(lib_dir, 'auto')
        pack.resolve_skia_lib_dir(
            None, preset['config'], preset['arch'], toolchain, variant,
            dll_mode=(toolchain == 'msvc' and pack.detect_dll_mode()))
    except pack.PackageError as exc:
        return False, str(exc)
    return True, ''


def _pack_command(preset, out_dir, args):
    cmd = [
        sys.executable, os.path.join(BUILD_DIR, 'pack.py'),
        '--arch', preset['arch'],
        '--config', preset['config'],
        '--out-dir', out_dir,
    ]
    lib_dir = os.path.join(REPO_ROOT, preset['lib_dir'])
    if preset['toolchain'] == 'mingw':
        variant = pack.resolve_mingw_variant(lib_dir, 'auto')
        cmd += ['--mingw-variant', variant]
    if args.with_sdl:
        cmd.append('--with-sdl')
    if args.with_examples:
        cmd.append('--with-examples')
    return cmd


def _synthetic_inputs(work_dir, args):
    """创建 arm64 合成库目录，返回 pack 命令。"""
    lib_dir = os.path.join(work_dir, 'fake', 'lib')
    skia_dir = os.path.join(work_dir, 'fake', 'skia')
    out_dir = os.path.join(work_dir, 'dist')
    if not args.dry_run:
        for path in (lib_dir, skia_dir, out_dir):
            if not os.path.isdir(path):
                os.makedirs(path)
        for base in pack.LIB_BASENAMES:
            open(os.path.join(lib_dir, base + '.lib'), 'wb').close()
        for base in tuple(pack.SKIA_LINK_ORDER) + tuple(pack.SKIA_EXTRA_LIBS):
            open(os.path.join(skia_dir, base + '.lib'), 'wb').close()
    return lib_dir, skia_dir, out_dir


def _synthetic_command(work_dir, args):
    lib_dir, skia_dir, out_dir = _synthetic_inputs(work_dir, args)
    return [
        sys.executable, os.path.join(BUILD_DIR, 'pack.py'),
        '--arch', 'arm64',
        '--config', 'Release',
        '--lib-dir', lib_dir,
        '--skia-lib-dir', skia_dir,
        '--out-dir', out_dir,
    ]


def _build_command(preset, args):
    kind = preset.get('build')
    if not kind:
        return None, '该预设不支持自动构建'
    command = [sys.executable, os.path.join(BUILD_DIR, 'build.py'),
               '--config', preset.get('config', 'Release')]
    if preset.get('arch'):
        command += ['--arch', preset['arch']]
    return command, ''


def _run_step(name, cmd, log_fp, timeout, dry_run=False, env=None):
    step = {
        'name': name,
        'command': cmd,
        'status': 'passed',
        'returncode': 0,
        'duration': 0.0,
    }
    if dry_run:
        step['status'] = 'planned'
        log_fp.write('[DRY-RUN] %s: %s\n' % (name, ' '.join(cmd)))
        return step

    log_fp.write('\n=== %s ===\n%s\n' % (name, ' '.join(cmd)))
    log_fp.flush()
    start = time.time()
    try:
        proc = subprocess.run(cmd, cwd=REPO_ROOT, env=env,
                              capture_output=True, text=True,
                              timeout=timeout)
        output = (proc.stdout or '') + (proc.stderr or '')
        log_fp.write(output)
        step['returncode'] = proc.returncode
        if proc.returncode != 0:
            step['status'] = 'failed'
    except subprocess.TimeoutExpired as exc:
        log_fp.write((exc.stdout or '') + (exc.stderr or ''))
        step['status'] = 'failed'
        step['returncode'] = -1
        step['error'] = '超时（%d 秒）' % timeout
    except OSError as exc:
        step['status'] = 'failed'
        step['returncode'] = -1
        step['error'] = str(exc)
    step['duration'] = round(time.time() - start, 2)
    log_fp.write('--> %s（退出码 %s，%.2fs）\n'
                 % (step['status'], step['returncode'], step['duration']))
    log_fp.flush()
    return step


def _new_package(out_dir, before):
    if not os.path.isdir(out_dir):
        return ''
    names = sorted(set(os.listdir(out_dir)) - before)
    packages = [name for name in names
                if name.lower().endswith(('.zip', '.tgz', '.tar.bz2'))]
    return os.path.join(out_dir, packages[-1]) if packages else ''


def _zip_members(package):
    with zipfile.ZipFile(package) as zf:
        return [info.filename for info in zf.infolist() if not info.is_dir()]


def inspect_package(package, arch):
    """结构断言，返回 (ok, errors)。"""
    errors = []
    try:
        members = _zip_members(package)
    except (OSError, zipfile.BadZipFile) as exc:
        return False, ['无法读取包：%s' % exc]
    if not members:
        return False, ['包为空']

    roots = {name.split('/', 1)[0] for name in members}
    if len(roots) != 1:
        errors.append('包内根目录不唯一：%s' % ', '.join(sorted(roots)))
        rels = set(members)
    else:
        root = roots.pop()
        rels = {name[len(root) + 1:] for name in members
                if name.startswith(root + '/')}

    for required in ('MANIFEST.txt', 'SHA256SUMS',
                     'lib/cmake/nim_duilib/nim_duilibConfig.cmake'):
        if required not in rels:
            errors.append('缺少 %s' % required)
    if 'win' in package.lower() or any(
            rel.startswith('msvc/') for rel in rels):
        for manifest_arch in pack.WINDOWS_MANIFEST_ARCHES:
            if 'msvc/manifest/duilib.%s.manifest' % manifest_arch not in rels:
                errors.append('缺少 %s manifest' % manifest_arch)
    if arch == 'arm64':
        name = 'msvc/manifest/duilib.arm64.manifest'
        # 直接按后缀查找，避免依赖根目录名
        with zipfile.ZipFile(package) as zf:
            member = next((n for n in zf.namelist()
                           if n.endswith('/' + name)), '')
            if not member:
                errors.append('缺少 arm64 manifest')
            elif b'processorArchitecture="ARM64"' not in zf.read(member):
                errors.append('arm64 manifest 的架构不是 ARM64')
    return not errors, errors


def _smoke(package, work_dir, host_arch, log_fp):
    """解包并启动 bin/basic.exe；仅当架构与主机一致时执行。"""
    extract_dir = os.path.join(work_dir, 'extract')
    if os.path.isdir(extract_dir):
        shutil.rmtree(extract_dir)
    os.makedirs(extract_dir)
    with zipfile.ZipFile(package) as zf:
        zf.extractall(extract_dir)

    exe = ''
    for dirpath, _dirnames, filenames in os.walk(extract_dir):
        for name in filenames:
            if name.lower() == 'basic.exe':
                exe = os.path.join(dirpath, name)
                break
        if exe:
            break
    if not exe:
        return 'skipped', '包内没有 bin/basic.exe（未包含示例）'

    machine = pack.pe_machine(exe)
    expected = pack.PE_MACHINE_BY_ARCH.get(host_arch)
    if machine is not None and expected is not None and machine != expected:
        return 'skipped', '示例架构（0x%04X）与主机 %s 不一致' % (machine, host_arch)

    log_fp.write('\n=== smoke ===\n%s\n' % exe)
    log_fp.flush()
    proc = subprocess.Popen([exe], cwd=os.path.dirname(exe))
    time.sleep(8)
    if proc.poll() is None:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
        return 'passed', '启动后 8 秒仍在运行'
    return ('passed', '退出码 %d' % proc.returncode) if proc.returncode == 0 \
        else ('failed', '启动后退出码 %d' % proc.returncode)


def _run_pack_preset(name, preset, args, work_dir, log_fp):
    record = {'name': name, 'status': 'skipped', 'reason': '',
              'steps': [], 'assertions': [], 'package': {}}
    ok, reason = check_prereq(preset)
    if not ok:
        record['reason'] = reason
        return record

    if args.build:
        build_cmd, build_reason = _build_command(preset, args)
        if not build_cmd:
            record['status'] = 'skipped'
            record['reason'] = build_reason
            return record
        record['steps'].append(_run_step(
            'build', build_cmd, log_fp, args.build_timeout,
            dry_run=args.dry_run,
            env=dict(os.environ, DUILIB_BUILD_TYPE=preset['config'])))

    out_dir = os.path.join(work_dir, 'dist')
    before = set(os.listdir(out_dir)) if os.path.isdir(out_dir) else set()
    cmd = _pack_command(preset, out_dir, args)
    pack_step = _run_step('pack', cmd, log_fp, 1800, dry_run=args.dry_run)
    record['steps'].append(pack_step)
    if pack_step['status'] == 'failed':
        record['status'] = 'failed'
        record['reason'] = '打包失败'
        return record
    if args.dry_run:
        record['status'] = 'planned'
        return record

    package = _new_package(out_dir, before)
    if not package:
        record['status'] = 'failed'
        record['reason'] = '未找到新生成的包'
        return record
    record['package'] = {
        'path': package,
        'size': os.path.getsize(package),
    }

    verify = _run_step('verify',
                       [sys.executable, os.path.join(BUILD_DIR, 'pack.py'),
                        '--verify', package],
                       log_fp, 600)
    record['steps'].append(verify)
    if verify['status'] == 'failed':
        record['status'] = 'failed'
        record['reason'] = '包体检未通过'
        return record

    inspect_ok, errors = inspect_package(package, preset['arch'])
    record['assertions'] = errors or ['结构断言通过']
    if not inspect_ok:
        record['status'] = 'failed'
        record['reason'] = '结构断言未通过'
        return record

    if args.smoke and preset['config'] == 'Release':
        smoke_status, smoke_reason = _smoke(
            package, work_dir, detect_environment()['host_arch'], log_fp)
        record['assertions'].append('冒烟：%s' % smoke_reason)
        if smoke_status == 'failed':
            record['status'] = 'failed'
            record['reason'] = smoke_reason
            return record

    record['status'] = 'passed'
    return record


def _run_synthetic_arm64(name, args, work_dir, log_fp):
    record = {'name': name, 'status': 'skipped', 'reason': '',
              'steps': [], 'assertions': [], 'package': {}}
    if not os.path.isfile(os.path.join(pack.MSVC_MANIFEST_DIR,
                                       'duilib.arm64.manifest')):
        record['reason'] = '缺少 duilib.arm64.manifest'
        return record

    out_dir = os.path.join(work_dir, 'dist')
    before = set(os.listdir(out_dir)) if os.path.isdir(out_dir) else set()
    cmd = _synthetic_command(work_dir, args)
    pack_step = _run_step('pack-arm64', cmd, log_fp, 1800, dry_run=args.dry_run)
    record['steps'].append(pack_step)
    if pack_step['status'] == 'failed':
        record['status'] = 'failed'
        record['reason'] = 'arm64 打包失败'
        return record
    if args.dry_run:
        record['status'] = 'planned'
        return record

    package = _new_package(out_dir, before)
    if not package:
        record['status'] = 'failed'
        record['reason'] = '未找到新生成的 arm64 包'
        return record
    record['package'] = {'path': package, 'size': os.path.getsize(package)}

    verify = _run_step('verify-arm64',
                       [sys.executable, os.path.join(BUILD_DIR, 'pack.py'),
                        '--verify', package], log_fp, 600)
    record['steps'].append(verify)
    inspect_ok, errors = inspect_package(package, 'arm64')
    record['assertions'] = errors or ['结构断言通过']
    if verify['status'] == 'failed' or not inspect_ok:
        record['status'] = 'failed'
        record['reason'] = 'arm64 结构/体检未通过'
        return record
    record['status'] = 'passed'
    return record


def _run_unit(name, args, log_fp):
    record = {'name': name, 'status': 'passed', 'reason': '',
              'steps': [], 'assertions': [], 'package': {}}
    for script in ('test_pack.py', 'test_autotest.py'):
        step = _run_step(
            script,
            [sys.executable, os.path.join(BUILD_DIR, script)],
            log_fp, 600, dry_run=args.dry_run)
        record['steps'].append(step)
        if step['status'] == 'failed':
            record['status'] = 'failed'
            record['reason'] = '%s 未通过' % script
            return record
    if args.dry_run:
        record['status'] = 'planned'
    return record


def run_preset(name, args, work_dir, log_fp):
    preset = PRESETS[name]
    if preset['kind'] == 'unit':
        return _run_unit(name, args, log_fp)
    if preset['kind'] == 'synthetic-arm64':
        return _run_synthetic_arm64(name, args, work_dir, log_fp)
    return _run_pack_preset(name, preset, args, work_dir, log_fp)


def _print_summary(records):
    passed = sum(1 for r in records if r['status'] in ('passed', 'planned'))
    failed = sum(1 for r in records if r['status'] == 'failed')
    skipped = sum(1 for r in records if r['status'] == 'skipped')
    for record in records:
        line = '%-26s %s' % (record['name'], record['status'])
        if record.get('reason'):
            line += '（%s）' % record['reason']
        if record.get('package'):
            line += '  包：%s' % record['package']['path']
        print(line)
    print('-' * 60)
    print('通过 %d，失败 %d，跳过 %d' % (passed, failed, skipped))
    if failed:
        return 1
    if not passed:
        return 2
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(
        description='nim_duilib 打包自动化（不接入 CI）')
    parser.add_argument('--list-presets', action='store_true',
                        help='列出全部预设后退出')
    parser.add_argument('--preset', default='unit',
                        choices=ALL_PRESETS + ['all-ready'],
                        help='要执行的预设（默认 unit；all-ready 运行所有可运行项）')
    parser.add_argument('--out-dir', default=None,
                        help='运行输出目录（默认 C:\\develop\\temp\\autotest\\<时间戳>）')
    parser.add_argument('--build', action='store_true',
                        help='先调用现有构建脚本（仅部分预设支持）')
    parser.add_argument('--smoke', action='store_true',
                        help='解包并启动 bin/basic.exe（需包内含示例）')
    parser.add_argument('--with-sdl', action='store_true',
                        help='打包时附加 --with-sdl')
    parser.add_argument('--with-examples', action='store_true',
                        help='打包时附加 --with-examples')
    parser.add_argument('--build-timeout', type=int, default=3600,
                        help='构建超时秒数（默认 3600）')
    parser.add_argument('--dry-run', action='store_true',
                        help='只打印将执行的命令，不运行')
    args = parser.parse_args(argv)

    if args.list_presets:
        for name in ALL_PRESETS:
            print('%-26s %s' % (name, PRESETS[name]['description']))
        print('%-26s %s' % ('all-ready', '运行所有前置条件满足的预设'))
        return 0

    run_dir = args.out_dir or os.path.join(DEFAULT_OUT_ROOT, _now_stamp())
    if not os.path.isdir(run_dir):
        os.makedirs(run_dir)
    log_path = os.path.join(run_dir, 'autotest.log')
    report_path = os.path.join(run_dir, 'report.json')

    env = detect_environment()
    names = ALL_PRESETS if args.preset == 'all-ready' else [args.preset]
    records = []
    started = time.strftime('%Y-%m-%d %H:%M:%S')

    with open(log_path, 'w', encoding='utf-8') as log_fp:
        log_fp.write('nim_duilib 打包自动化\n')
        log_fp.write(json.dumps(env, ensure_ascii=False, indent=2) + '\n')
        for name in names:
            work_dir = os.path.join(run_dir, name)
            if not args.dry_run and not os.path.isdir(work_dir):
                os.makedirs(work_dir)
            print('>>> %s' % name)
            record = run_preset(name, args, work_dir, log_fp)
            records.append(record)

    report = {
        'schema': 1,
        'started': started,
        'finished': time.strftime('%Y-%m-%d %H:%M:%S'),
        'dry_run': args.dry_run,
        'environment': env,
        'presets': records,
        'summary': {
            'passed': sum(1 for r in records
                          if r['status'] in ('passed', 'planned')),
            'failed': sum(1 for r in records if r['status'] == 'failed'),
            'skipped': sum(1 for r in records if r['status'] == 'skipped'),
        },
    }
    if not args.dry_run:
        with open(report_path, 'w', encoding='utf-8') as fp:
            json.dump(report, fp, ensure_ascii=False, indent=2)
    print('日志：%s' % log_path)
    if not args.dry_run:
        print('报告：%s' % report_path)
    return _print_summary(records)


if __name__ == '__main__':
    sys.exit(main())
