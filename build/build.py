"""nim_duilib 统一构建入口。

用法：
  python3 build/build.py [--config Debug|Release]      # 默认：准备依赖 + 编译一个配置
  python3 build/build.py build [--no-deps] [--target X]
  python3 build/build.py deps [--force]
  python3 build/build.py doctor
  python3 build/build.py test
  python3 build/build.py pack [--with-cef]
  python3 build/build.py clean [--deps] [--build-dir DIR]

参数透传：--cmake-arg 追加给 cmake configure，--build-arg 追加给 cmake --build。
本脚本只做环境识别与编排，构建逻辑全部在根 CMake 工程。
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

from i18n import make_messages, set_console_utf8

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


REPO_ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = REPO_ROOT / "build"
CMAKE_FILE = REPO_ROOT / "CMakeLists.txt"
VERSIONS_FILE = REPO_ROOT / "cmake" / "third_party_versions.json"
PREFLIGHT_SCRIPT = REPO_ROOT / "cmake" / "duilib_preflight_tools.cmake"

VALID_CONFIGS = ("Debug", "Release")
VALID_ARCHES = ("auto", "x86", "x64", "arm64")
DEFAULT_CONFIG = "Release"
DEFAULT_DEPS_MODE = "prepare"
# Skia 补丁归档只随 skia_compile 仓库分发，版本清单里只有文件名。
SKIA_COMPILE_REPO = "https://github.com/rhett-lee/skia_compile.git"


class BuildError(Exception):
    def __init__(self, message: str, hint: str = "", code: int = 1):
        super().__init__(message)
        self.message = message
        self.hint = hint
        self.code = code


# ---------------------------------------------------------------------------
# 纯函数（供 test_build.py 覆盖）
# ---------------------------------------------------------------------------
def normalize_arch(value: str) -> str:
    value = (value or "").strip().lower()
    if value in ("amd64", "x86_64", "x64"):
        return "x64"
    if value in ("x86", "i386", "i686", "win32"):
        return "x86"
    if value in ("arm64", "aarch64"):
        return "arm64"
    return value


def resolve_cef_version(requested: str | None, manifest: dict) -> str:
    """把 --cef-version 归一为版本清单中的键。

    109 表示旧版；latest 或未指定时取清单 cef.default，清单升级后自动跟随，
    避免把当前版本号（如 142）硬编码进命令行与帮助文本。
    """
    cef = manifest.get("cef", {}) if isinstance(manifest, dict) else {}
    default = str(cef.get("default", ""))
    versions = cef.get("versions", {})
    if not requested or requested == "latest":
        return default
    if requested not in versions:
        raise BuildError(
            T('help.cef_unknown').format(requested=requested),
            T('help.cef_unknown_choices').format(
                default=default,
                versions=T('help.list_sep').join(versions)))
    return requested


def host_arch() -> str:
    if sys.platform == "win32":
        value = os.environ.get("PROCESSOR_ARCHITEW6432") or os.environ.get("PROCESSOR_ARCHITECTURE", "")
    else:
        value = platform.machine()
    arch = normalize_arch(value)
    return arch if arch in ("x86", "x64", "arm64") else "x64"


def resolve_arch(requested: str) -> str:
    return host_arch() if requested in ("", "auto") else requested


def preset_name(arch: str, config: str) -> str:
    return f"{arch}-{config.lower()}"


def preset_build_dir(repo: Path, arch: str, config: str) -> Path:
    return repo / "out" / arch / config


def resolve_build_dir(repo: Path, arch: str, config: str, override: str | None = None) -> Path:
    if override:
        path = Path(override).expanduser()
        if not path.is_absolute():
            path = Path.cwd() / path
        return path.resolve()
    return preset_build_dir(repo, arch, config)


def _expected_compiler(toolchain: "Toolchain") -> Path | None:
    env = toolchain.env
    name = env.get("CXX") or ("cl" if sys.platform == "win32" and not env.get("MSYSTEM") else "c++")
    found = shutil.which(name, path=env.get("PATH"))
    return Path(found).resolve() if found else None


def _cached_compiler(build_dir: Path) -> Path | None:
    cache = build_dir / "CMakeCache.txt"
    if not cache.is_file():
        return None
    try:
        text = cache.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None
    match = re.search(r"^CMAKE_CXX_COMPILER:[^=]*=(.+)$", text, re.MULTILINE)
    if not match or not match.group(1).strip():
        return None
    return Path(match.group(1).strip())


def reset_stale_build_dir(build_dir: Path, toolchain: "Toolchain", messages, dry_run: bool) -> None:
    """编译器切换后丢弃旧缓存。

    同一 out/<arch>/<Config> 在 MSVC 与 MinGW 之间切换时，旧 CMakeCache 仍指向
    原编译器：configure 会用新环境调用旧编译器（典型现象是 cl.exe 找不到
    kernel32.lib）。目标文件本身也不兼容，因此直接清空生成目录。
    """
    expected = _expected_compiler(toolchain)
    cached = _cached_compiler(build_dir)
    if not expected or not cached:
        return
    try:
        same = os.path.normcase(str(expected)) == os.path.normcase(str(cached.resolve()))
    except OSError:
        same = False
    if same:
        return
    print(messages.t("build.toolchain_changed").format(dir=build_dir, cached=cached, expected=expected),
          flush=True)
    if not dry_run:
        shutil.rmtree(build_dir)


STAMP_DIR_NAME = ".build-config"
LEGACY_STAMP_SUFFIX = ".build-config"


def _stamp_config(path: Path) -> str | None:
    """读取一个构建配置标记；空文件或不可读时返回 None。"""
    try:
        return path.read_text(encoding="utf-8", errors="replace").strip() or None
    except OSError:
        return None


def build_config_stamps(bin_dir: Path) -> dict[str, tuple[str, Path]]:
    """扫描 bin/ 下的构建配置标记，返回 {目标名: (构建配置, 标记文件)}。

    CMake 在链接后写 bin/.build-config/<目标>（见
    cmake/duilib_write_build_stamp.cmake）；旧版本写在
    bin/<目标>.build-config，仍兼容读取。同一目标两种标记并存时以新路径为准。
    """
    stamps: dict[str, tuple[str, Path]] = {}
    stamp_dir = bin_dir / STAMP_DIR_NAME
    if stamp_dir.is_dir():
        for path in sorted(stamp_dir.iterdir()):
            if not path.is_file():
                continue
            config = _stamp_config(path)
            if config:
                stamps[path.name] = (config, path)
    if bin_dir.is_dir():
        for path in sorted(bin_dir.glob("*" + LEGACY_STAMP_SUFFIX)):
            if not path.is_file():
                continue
            stem = path.name[:-len(LEGACY_STAMP_SUFFIX)]
            config = _stamp_config(path)
            if config and stem not in stamps:
                stamps[stem] = (config, path)
    return stamps


def target_bin_outputs(bin_dir: Path, target: str) -> list[Path]:
    """目标在共享 bin/ 下可能产生的产物（可执行文件或 macOS App Bundle）。"""
    candidates = [bin_dir / target,
                  bin_dir / (target + ".exe"),
                  bin_dir / (target + ".app")]
    return [path for path in candidates if path.exists() or path.is_symlink()]


def stale_build_outputs(bin_dir: Path, config: str,
                        target: str | None = None) -> list[tuple[str, str, Path, list[Path]]]:
    """找出 bin/ 中由其他配置链接出来的示例产物。

    Debug 与 Release 的示例输出同名，且都写到共享的 bin/；目标文件却按配置
    分别存放在 out/<arch>/<Config>。Ninja 只按修改时间判断链接步骤是否需要
    重跑：先构建 Debug 后，bin/ 中的文件比 Release 的目标文件新，Release 的
    链接步骤就被判为已是最新（ninja: no work to do），bin/ 一直是 Debug 产物。
    返回 [(目标, 实际配置, 标记文件, 待删除产物)]。
    """
    wanted = (target or "").strip()
    for suffix in (".exe", ".app"):
        if wanted.lower().endswith(suffix):
            wanted = wanted[:-len(suffix)]
            break
    if wanted.lower() == "all":
        wanted = ""
    stale = []
    for stem, (stamp_config, stamp_path) in sorted(build_config_stamps(bin_dir).items()):
        if wanted and stem != wanted:
            continue
        if stamp_config == config:
            continue
        outputs = target_bin_outputs(bin_dir, stem)
        if outputs:
            stale.append((stem, stamp_config, stamp_path, outputs))
    return stale


def _remove_stale_output(path: Path) -> None:
    """删除 bin/ 中另一个配置链接出来的产物，迫使当前配置重新链接。

    macOS App Bundle 不能整个删除：`Contents/Info.plist` 由 CMake 在 configure
    阶段生成，链接阶段只重建 `Contents/MacOS` 下的可执行文件。若把整个 `.app`
    删掉，重新链接后 bundle 缺少 Info.plist（没有 CFBundleIdentifier），
    LaunchServices 无法解析当前进程的 bundle，启动 SDL 时
    `+[UNUserNotificationCenter currentNotificationCenter]` 断言抛异常并把进程带崩。
    因此这里只删可执行文件，保留 Info.plist、PkgInfo 与资源目录。
    """
    if path.is_dir() and not path.is_symlink():
        if path.suffix == ".app":
            macos_dir = path / "Contents" / "MacOS"
            if macos_dir.is_dir():
                for child in sorted(macos_dir.iterdir()):
                    if child.is_dir() and not child.is_symlink():
                        shutil.rmtree(child)
                    else:
                        child.unlink()
                return
        shutil.rmtree(path)
    else:
        path.unlink()


def clear_stale_build_outputs(bin_dir: Path, config: str, target: str | None,
                              messages, dry_run: bool = False) -> list[str]:
    """删除 bin/ 中其他配置的示例产物，强制当前配置重新链接。

    只处理有配置标记的目标；没有标记时无法判断产物属于哪个配置，交由
    pack.py 的“无法确认”提示处理。--target 只清理该目标。
    """
    stale = stale_build_outputs(bin_dir, config, target)
    if not stale:
        return []
    other = ", ".join(sorted({entry[1] for entry in stale}))
    print(messages.t("build.config_switch").format(
        stale=other, count=len(stale), config=config), flush=True)
    if dry_run:
        return [entry[0] for entry in stale]
    for _stem, _stamp_config, stamp_path, outputs in stale:
        for path in outputs:
            try:
                _remove_stale_output(path)
            except FileNotFoundError:
                continue
            except OSError as exc:
                raise BuildError(
                    messages.t("build.clean_failed").format(path=path, error=exc),
                    messages.hint("build.clean_failed_hint"))
        try:
            stamp_path.unlink()
        except FileNotFoundError:
            pass
        except OSError as exc:
            raise BuildError(
                messages.t("build.clean_failed").format(path=stamp_path, error=exc),
                messages.hint("build.clean_failed_hint"))
    return [entry[0] for entry in stale]


BUILD_MODE_STAMP_NAME = ".duilib-build-mode"
MSVC_DLL_FLAG = REPO_ROOT / "msvc" / "PropertySheets" / "duilib_dll.flag"


def requested_build_mode(args) -> str:
    """本次构建的 CRT/DLL 模式；非 Windows MSVC 环境不参与切换检测。"""
    if sys.platform != "win32" or os.environ.get("MSYSTEM"):
        return "default"
    if getattr(args, "dll", False):
        return "dll"
    return "md" if getattr(args, "md", False) else "mt"


def _cached_build_mode(build_dir: Path) -> str | None:
    """从旧构建目录的 CMakeCache 推断上次模式（兼容升级前的构建目录）。"""
    cache = build_dir / "CMakeCache.txt"
    if not cache.is_file():
        return None
    try:
        text = cache.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None

    def cached_bool(name: str) -> bool:
        match = re.search(rf"^{name}:BOOL=(ON|OFF)$", text, re.MULTILINE)
        return bool(match and match.group(1) == "ON")

    if cached_bool("DUILIB_DLL"):
        return "dll"
    return "md" if cached_bool("DUILIB_MD") else "mt"


def reset_stale_build_mode(build_dir: Path, args, messages, dry_run: bool) -> str | None:
    """运行库/DLL 模式切换后清理构建树。

    MSVC 的 /MT 与 /MD 目标文件、CEF wrapper 的静态库互不兼容；旧构建目录
    继续复用时，只能靠命令行变化触发重编（Visual Studio 生成器不一定跟踪），
    容易出现旧的 /MT wrapper 被链进新的 DLL。这里在构建目录记录模式，
    切换时直接清理，保证 wrapper 与全部目标按新模式重新编译。
    返回被清理的旧模式（未切换时返回 None）。
    """
    mode = requested_build_mode(args)
    if mode == "default" or not build_dir.is_dir():
        return None
    stamp = build_dir / BUILD_MODE_STAMP_NAME
    previous = _stamp_config(stamp) or _cached_build_mode(build_dir)
    if previous is None or previous == mode:
        return None
    print(messages.t("build.mode_changed").format(
        dir=build_dir, previous=previous, mode=mode), flush=True)
    if not dry_run:
        shutil.rmtree(build_dir)
        if previous == "dll" and mode != "dll":
            # 切回静态库后，bin/ 里的旧 duilib DLL 会让 pack.py 误判为 DLL 包。
            for path in (REPO_ROOT / "bin").glob("duilib*.dll"):
                try:
                    path.unlink()
                except OSError:
                    pass
    return previous


def write_build_mode_stamp(build_dir: Path, args, dry_run: bool) -> None:
    mode = requested_build_mode(args)
    if mode == "default":
        return
    if dry_run:
        return
    try:
        build_dir.mkdir(parents=True, exist_ok=True)
        (build_dir / BUILD_MODE_STAMP_NAME).write_text(mode, encoding="utf-8")
    except OSError:
        # 标记只用于下次切换检测，写失败不应让构建失败。
        pass


def sync_msvc_dll_flag(enabled: bool, messages, dry_run: bool) -> None:
    """让 MSBuild 的属性表与本入口的 DLL 模式保持一致。

    CMake 构建（build.py）与 VS 工程共用 bin/、lib/；如果 flag 不跟随，切换
    入口后另一套构建会沿用旧模式。flag 本身被 gitignore。
    """
    if sys.platform != "win32" or os.environ.get("MSYSTEM"):
        return
    exists = MSVC_DLL_FLAG.is_file()
    if exists == enabled:
        return
    state = "ON" if enabled else "OFF"
    print(messages.t("build.dll_flag").format(state=state), flush=True)
    if dry_run:
        return
    try:
        if enabled:
            MSVC_DLL_FLAG.parent.mkdir(parents=True, exist_ok=True)
            MSVC_DLL_FLAG.write_text(
                "DLL mode marker for nim_duilib.\n"
                "Managed by build/build.py --dll; delete this file to switch "
                "the MSBuild projects back to static mode.\n",
                encoding="utf-8")
        else:
            MSVC_DLL_FLAG.unlink()
    except OSError as exc:
        raise BuildError(
            messages.t("build.clean_failed").format(path=MSVC_DLL_FLAG, error=exc),
            messages.hint("build.clean_failed_hint"))


def load_manifest(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as handle:
        data = json.load(handle)
    if data.get("schemaVersion") != 1:
        raise ValueError("unsupported schemaVersion")
    for section in ("skia", "sdl", "cef"):
        block = data.get(section)
        if not isinstance(block, dict) or not block.get("default"):
            raise ValueError(f"missing {section}.default")
    skia_default = data["skia"]["default"]
    if skia_default not in data["skia"].get("refs", {}):
        raise ValueError("skia.default not present in refs")
    sdl_default = data["sdl"]["default"]
    if sdl_default not in data["sdl"].get("refs", {}):
        raise ValueError("sdl.default not present in refs")
    cef_default = str(data["cef"]["default"])
    if cef_default not in data["cef"].get("versions", {}):
        raise ValueError("cef.default not present in versions")
    return data


def cef_archive_entry(manifest: dict, version: str, platform_name: str, arch: str) -> dict | None:
    versions = manifest.get("cef", {}).get("versions", {})
    return versions.get(str(version), {}).get("archives", {}).get(f"{platform_name}/{arch}")


def platform_name() -> str:
    if sys.platform == "win32":
        return "windows"
    if sys.platform == "darwin":
        return "macos"
    if sys.platform.startswith("freebsd"):
        return "freebsd"
    return "linux"


# ---------------------------------------------------------------------------
# 参数解析
# ---------------------------------------------------------------------------
def _platform_help(windows_key: str, other_key: str) -> str:
    """按当前平台选择帮助文案 key：只在有实际效果的平台上完整说明。"""
    return T(windows_key) if sys.platform == "win32" else T(other_key)


def _add_common_options(parser: argparse.ArgumentParser, suppress_defaults: bool = False) -> None:
    """公共选项。

    子命令解析器也定义同一批选项，以便 `build.py build --config Debug` 可用；
    子命令侧必须抑制默认值，否则 argparse 会用子命令的默认值覆盖写在子命令
    前面的选项（`build.py --config Debug build` 会变回 Release）。
    """

    def default(value):
        return argparse.SUPPRESS if suppress_defaults else value

    parser.add_argument("--config", choices=VALID_CONFIGS, default=default(DEFAULT_CONFIG),
                        help=T("help.config"))
    parser.add_argument("--arch", choices=VALID_ARCHES, default=default("auto"),
                        help=T("help.arch"))
    parser.add_argument("--lang", choices=("auto", "zh_CN", "en"), default=default("auto"),
                        help=T("help.lang"))
    parser.add_argument("--build-dir", default=default(None), help=T("help.build_dir"))
    parser.add_argument("--deps-root", default=default(None), help=T("help.deps_root"))
    parser.add_argument("--jobs", type=int, default=default(None),
                        help=T("help.jobs"))
    parser.add_argument("--offline", action="store_true", default=default(False),
                        help=T("help.offline"))
    parser.add_argument("--dry-run", action="store_true", default=default(False),
                        help=T("help.dry_run"))
    parser.add_argument("--cmake-arg", action="append", default=default([]),
                        help=T("help.cmake_arg"))
    parser.add_argument("--build-arg", action="append", default=default([]),
                        help=T("help.build_arg"))
    parser.add_argument("--vs-version", default=default(None),
                        help=_platform_help("help.vs_windows", "help.vs_other"))
    parser.add_argument("--cef-version", metavar="VERSION", default=default(None),
                        help=T("help.cef_version"))
    parser.add_argument("--skia-ref", default=default(None), help=T("help.skia_ref"))
    parser.add_argument("--sdl-ref", default=default(None), help=T("help.sdl_ref"))
    parser.add_argument("--sdl", dest="sdl", action="store_true", default=default(None),
                        help=_platform_help("help.sdl_windows", "help.sdl_other"))
    parser.add_argument("--no-sdl", dest="sdl", action="store_false", default=default(None),
                        help=_platform_help("help.no_sdl_windows", "help.no_sdl_other"))
    parser.add_argument("--md", action="store_true", default=default(False),
                        help=_platform_help("help.md_windows", "help.md_other"))
    parser.add_argument("--dll", action="store_true", default=default(False),
                        help=_platform_help("help.dll_windows", "help.dll_other"))
    parser.add_argument("--no-absolute-rpath", dest="rpath_absolute",
                        action="store_false", default=default(True),
                        help=_platform_help("help.rpath_windows", "help.rpath_other"))
    parser.add_argument("--cef-archive", default=default(None),
                        help=T("help.cef_archive"))


def build_parser() -> argparse.ArgumentParser:
    main_common = argparse.ArgumentParser(add_help=False)
    _add_common_options(main_common)
    sub_common = argparse.ArgumentParser(add_help=False)
    _add_common_options(sub_common, suppress_defaults=True)

    parser = argparse.ArgumentParser(
        prog="build.py",
        description=T("help.description"),
        epilog=T("help.epilog"),
        formatter_class=argparse.RawDescriptionHelpFormatter,
        parents=[main_common])
    sub = parser.add_subparsers(dest="command")

    build_parser_ = sub.add_parser("build", parents=[sub_common], help=T("help.cmd_build"))
    build_parser_.add_argument("--no-deps", action="store_true",
                               help=T("help.no_deps"))
    build_parser_.add_argument("--target", default=None,
                               help=T("help.target"))
    build_parser_.add_argument("--no-examples", action="store_true",
                               help=T("help.no_examples"))

    deps_parser = sub.add_parser("deps", parents=[sub_common],
                                 help=T("help.cmd_deps"))
    deps_parser.add_argument("--force", action="store_true",
                             help=T("help.force"))
    deps_parser.add_argument("--no-examples", action="store_true", help=argparse.SUPPRESS)

    sub.add_parser("doctor", parents=[sub_common], help=T("help.cmd_doctor"))

    sub.add_parser("test", parents=[sub_common], help=T("help.cmd_test"))

    pack_parser = sub.add_parser("pack", parents=[sub_common], help=T("help.cmd_pack"))
    pack_parser.add_argument("--with-cef", action="store_true",
                             help=T("help.with_cef"))
    pack_parser.add_argument("--with-sdl", action="store_true",
                             help=T("help.with_sdl"))
    pack_parser.add_argument("--with-examples", action="store_true",
                             help=T("help.with_examples"))
    pack_parser.add_argument("--out-dir", default=None, help=T("help.out_dir"))
    pack_parser.add_argument("--pack-arg", action="append", default=[],
                             help=T("help.pack_arg"))

    clean_parser = sub.add_parser("clean", parents=[sub_common],
                                  help=T("help.cmd_clean"))
    clean_parser.add_argument("--deps", action="store_true",
                              help=T("help.clean_deps"))

    return parser


def parse_args(argv=None) -> argparse.Namespace:
    parser = build_parser()
    args = parser.parse_args(argv)
    if args.command is None:
        args.command = "build"
        args.no_deps = False
        args.target = None
        args.no_examples = False
    # 子命令未提供的属性补默认，便于统一处理
    for name, default in (("force", False), ("with_cef", False), ("with_sdl", False),
                          ("with_examples", False), ("out_dir", None), ("pack_arg", []),
                          ("dll", False)):
        if not hasattr(args, name):
            setattr(args, name, default)
    if args.dll:
        if sys.platform != "win32" or os.environ.get("MSYSTEM"):
            parser.error(T("help.dll_platform"))
        # DLL 必须使用动态运行库，避免 wrapper/第三方库的 /MT 冲突。
        args.md = True
    return args


# ---------------------------------------------------------------------------
# 执行辅助
# ---------------------------------------------------------------------------
def _format_command(command) -> str:
    if sys.platform == "win32":
        return subprocess.list2cmdline([str(part) for part in command])
    import shlex
    return shlex.join([str(part) for part in command])


class Runner:
    def __init__(self, env: dict, dry_run: bool = False, verbose: bool = True):
        self.env = env
        self.dry_run = dry_run
        self.verbose = verbose

    def run(self, command, cwd: Path | None = None) -> int:
        if self.verbose:
            print("+ " + _format_command(command), flush=True)
        if self.dry_run:
            return 0
        completed = subprocess.run(
            [str(part) for part in command],
            cwd=str(cwd) if cwd else None,
            env=self.env)
        return completed.returncode


def _find_vswhere() -> Path | None:
    candidates = []
    for env_name in ("ProgramFiles(x86)", "ProgramFiles"):
        base = os.environ.get(env_name)
        if base:
            candidates.append(Path(base) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe")
    candidates.append(Path("vswhere.exe"))
    for candidate in candidates:
        if candidate.is_file():
            return candidate
        if candidate.name == "vswhere.exe":
            found = shutil.which("vswhere")
            if found:
                return Path(found)
    return None


def _vswhere_installation(vs_version: str | None) -> Path | None:
    vswhere = _find_vswhere()
    if not vswhere:
        return None
    command = [str(vswhere), "-latest", "-products", "*",
               "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
               "-property", "installationPath"]
    if vs_version:
        try:
            major = int(str(vs_version).split(".")[0])
            command[2:2] = ["-version", f"[{major}.0,{major + 1}.0)"]
        except ValueError:
            command[2:2] = ["-version", str(vs_version)]
    try:
        completed = subprocess.run(command, capture_output=True)
    except OSError:
        return None
    if completed.returncode != 0:
        return None
    text = completed.stdout.decode("mbcs" if sys.platform == "win32" else "utf-8",
                                   errors="replace").strip()
    if not text:
        return None
    path = Path(text.splitlines()[0].strip())
    return path if path.is_dir() else None


def _vcvars_platform(arch: str) -> str:
    """vcvarsall.bat 的平台参数（`主机_目标` 形式，单段表示原生工具）。

    arm64 需要区分主机：VS 默认只装 x64 主机的交叉工具（bin/Hostx64/arm64），
    直接传 `arm64` 会要求 Hostarm64 原生工具集，未安装时报
    "tools for arm64 are not installed"，与「已装 ARM64 生成工具」的实际状态不符。
    """
    if arch != "arm64":
        return arch
    return {"x64": "amd64_arm64", "x86": "x86_arm64", "arm64": "arm64"}[host_arch()]


def _msvc_environment(arch: str, vs_version: str | None) -> tuple[dict, Path | None]:
    install = _vswhere_installation(vs_version)
    if not install:
        return dict(os.environ), None
    vcvars = install / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat"
    if not vcvars.is_file():
        return dict(os.environ), install
    vs_arch = _vcvars_platform(arch)
    command = f'call "{vcvars}" {vs_arch} >nul && set'
    # 不能把带引号的批处理命令当成 argv 传给 cmd /c：Windows argv 转义会引入 \"；
    # 改用 shell=True 让 cmd 自己解析整条命令。
    completed = subprocess.run(command, shell=True, capture_output=True)
    if completed.returncode != 0:
        raise BuildError("[ERROR] failed to initialize the Visual Studio environment",
                         "run 'build.py doctor', or open a Developer Command Prompt and retry")
    env = dict(os.environ)
    text = completed.stdout.decode("mbcs" if sys.platform == "win32" else "utf-8",
                                   errors="replace")
    for line in text.splitlines():
        if "=" not in line or line.startswith("="):
            continue
        key, value = line.split("=", 1)
        # Windows 环境变量名不区分大小写，统一成大写，避免 "Path"/"PATH" 两份并存。
        key = key.upper()
        for existing in [name for name in env if name.upper() == key]:
            del env[existing]
        env[key] = value
    # 目标架构的 MSVC 工具集未安装时，vcvarsall 仍返回成功但 PATH 中没有 cl，
    # CMake 只会报 "No CMAKE_C_COMPILER could be found"；这里提前给出明确提示
    if not shutil.which("cl", path=env.get("PATH")):
        raise BuildError(
            f"[ERROR] Visual Studio C++ tools for {arch} are not installed",
            f"open the Visual Studio Installer and add the MSVC {arch} build tools component "
            "(Individual components > MSVC ... build tools), then reopen the terminal")
    return env, install


def _find_ninja(env: dict, vs_install: Path | None) -> Path | None:
    found = shutil.which("ninja", path=env.get("PATH"))
    if found:
        return Path(found)
    if vs_install:
        candidate = vs_install / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake" / "Ninja" / "ninja.exe"
        if candidate.is_file():
            return candidate
    return None


class Toolchain:
    def __init__(self, env: dict, generator: str, vs_install: Path | None, ninja: Path | None):
        self.env = env
        self.generator = generator
        self.vs_install = vs_install
        self.ninja = ninja

    @property
    def use_preset(self) -> bool:
        return self.generator == "Ninja"


def _is_uos() -> bool:
    """统信 UOS：clang++ 链接会失败，旧脚本固定改用 gcc。"""
    try:
        text = Path("/etc/os-release").read_text(encoding="utf-8", errors="replace")
    except OSError:
        return False
    return any(line.strip().strip('"') == "ID=uos" for line in text.splitlines())


def _compiler_triple(compiler: str, env: dict) -> str:
    found = shutil.which(compiler, path=env.get("PATH"))
    if not found:
        return ""
    try:
        result = subprocess.run([found, "-dumpmachine"], capture_output=True,
                                text=True, timeout=30)
    except (OSError, subprocess.SubprocessError):
        return ""
    return (result.stdout or "").strip() if result.returncode == 0 else ""


def _require_mingw_toolchain(env: dict) -> None:
    """MSYS2 下只接受目标为 mingw-w64 的编译器，否则会产出 MSYS 程序。"""
    env_cc, env_cxx = env.get("CC"), env.get("CXX")
    explicit = bool(env_cc and env_cxx)
    pairs = [(env_cc, env_cxx)] if explicit else [("clang", "clang++"), ("gcc", "g++")]
    for cc, cxx in pairs:
        if not cc or not cxx:
            continue
        triple = _compiler_triple(cc, env)
        if ("mingw" in triple or "w64-windows-gnu" in triple) and _compiler_triple(cxx, env):
            if not explicit:
                env["CC"], env["CXX"] = cc, cxx
            return
    raise BuildError(
        "[ERROR] no mingw-w64 targeted C/C++ compiler found",
        "run from an MSYS2 MINGW64/UCRT64/CLANG64 shell, where gcc/clang target *mingw*")


def detect_toolchain(arch: str, vs_version: str | None) -> Toolchain:
    env = dict(os.environ)
    vs_install = None
    if sys.platform == "win32" and not env.get("MSYSTEM"):
        env, vs_install = _msvc_environment(arch, vs_version)
        if vs_install is None and not shutil.which("cl", path=env.get("PATH")):
            raise BuildError(
                "[ERROR] Visual Studio 2022+ with the C++ workload was not found",
                "install Visual Studio 2022 (or newer) with 'Desktop development with C++'")
    elif sys.platform == "win32" and env.get("MSYSTEM"):
        _require_mingw_toolchain(env)
    elif sys.platform != "win32" and not env.get("CC") and not env.get("CXX") and _is_uos():
        # 与旧脚本一致：UOS 上 clang++ 链接会失败，未显式指定编译器时固定 gcc
        env["CC"], env["CXX"] = "gcc", "g++"
    ninja = _find_ninja(env, vs_install)
    if ninja:
        env["PATH"] = str(ninja.parent) + os.pathsep + env.get("PATH", "")
        generator = "Ninja"
    elif sys.platform == "win32" and not env.get("MSYSTEM"):
        generator = "Visual Studio 17 2022"
    else:
        generator = "Unix Makefiles"
    return Toolchain(env, generator, vs_install, ninja)


# ---------------------------------------------------------------------------
# CMake 参数
# ---------------------------------------------------------------------------
def _cmake_defines(args, arch: str, deps_mode: str, force: bool) -> list:
    manifest = load_manifest(VERSIONS_FILE)
    defines = [
        f"-DDUILIB_TARGET_ARCH={arch}",
        f"-DDUILIB_DEPS_MODE={deps_mode}",
        f"-DDUILIB_LANG={'zh_CN' if args.lang == 'zh_CN' else 'auto' if args.lang == 'auto' else 'en'}",
        f"-DDUILIB_OFFLINE={'ON' if args.offline else 'OFF'}",
        f"-DDUILIB_DEPS_FORCE={'ON' if force else 'OFF'}",
    ]
    if args.deps_root:
        defines.append(f"-DDUILIB_DEPS_ROOT={Path(args.deps_root).expanduser().resolve()}")

    # 以下选项在 CMake 缓存中是有状态的：只传 ON 或只在传入时传值，会让上一次
    # 调用的取值（如 --no-examples、--md、--sdl、--cef-version 109）残留在同一个
    # out/<arch>/<Config> 里，之后的“默认”构建实际仍沿用旧选项。这里每次都显式
    # 传入解析后的取值，保证每次调用的配置只由本次命令行决定。
    if args.sdl is not None:
        sdl_enabled = args.sdl
    else:
        # 与根工程默认一致：Windows（含 MinGW）默认关闭，其他平台强制开启。
        # DLL 模式也不隐式开启 SDL：Windows 上 SDL 体验不如原生实现，需要时
        # 显式 --sdl。DLL 模式只需 Skia 的 .md/.mdd 变体，SDL 关闭时
        # deps 不会 clone/build SDL3。
        sdl_enabled = sys.platform != "win32"
    defines.append(f"-DDUILIB_ENABLE_SDL={'ON' if sdl_enabled else 'OFF'}")
    defines.append(f"-DDUILIB_MD={'ON' if args.md else 'OFF'}")
    defines.append(f"-DDUILIB_DLL={'ON' if getattr(args, 'dll', False) else 'OFF'}")
    # Unix rpath 的绝对兜底默认开启；发布构建可用 --no-absolute-rpath。
    rpath_absolute = getattr(args, "rpath_absolute", True)
    defines.append(f"-DDUILIB_RPATH_ABSOLUTE={'ON' if rpath_absolute else 'OFF'}")
    defines.append(f"-DDUILIB_BUILD_EXAMPLES={'OFF' if getattr(args, 'no_examples', False) else 'ON'}")
    # CEF 是否编译进库：与根工程默认一致（Windows MSVC、Linux、macOS 开启；
    # MinGW/MSYS2 与 FreeBSD 关闭）。这也是有状态的缓存项：一旦某次用
    # `--cmake-arg=-DDUILIB_ENABLE_CEF=OFF` 关掉，后续不传就会一直沿用旧值。
    # 这里每次显式传入，用户仍可用 `--cmake-arg` 覆盖（它排在最后）。
    if sys.platform == "win32":
        cef_enabled = not os.environ.get("MSYSTEM")
    else:
        cef_enabled = sys.platform == "darwin" or sys.platform.startswith("linux")
    defines.append(f"-DDUILIB_ENABLE_CEF={'ON' if cef_enabled else 'OFF'}")
    defines.append(f"-DDUILIB_CEF_VERSION={resolve_cef_version(args.cef_version, manifest)}")
    defines.append(f"-DDUILIB_SKIA_REF={args.skia_ref or manifest['skia']['default']}")
    defines.append(f"-DDUILIB_SDL_REF={args.sdl_ref or manifest['sdl']['default']}")
    archive = Path(args.cef_archive).expanduser().resolve() if args.cef_archive else ""
    defines.append(f"-DDUILIB_CEF_ARCHIVE={archive}")

    defines.extend(args.cmake_arg)
    return defines, manifest


def _configure_command(args, toolchain: Toolchain, arch: str, config: str,
                       build_dir: Path, deps_mode: str, force: bool) -> list:
    defines, _ = _cmake_defines(args, arch, deps_mode, force)
    command = [shutil.which("cmake") or "cmake"]
    if toolchain.use_preset and not args.build_dir:
        command += ["--preset", preset_name(arch, config)]
    else:
        command += ["-S", str(REPO_ROOT), "-B", str(build_dir), "-G", toolchain.generator]
        if toolchain.generator.startswith("Visual Studio"):
            platform_map = {"x86": "Win32", "x64": "x64", "arm64": "ARM64"}
            command += ["-A", platform_map[arch]]
        if not toolchain.generator.startswith("Visual Studio"):
            command += [f"-DCMAKE_BUILD_TYPE={config}"]
    command += defines
    return command


def _build_command(args, toolchain: Toolchain, build_dir: Path, config: str,
                   target: str | None = None) -> list:
    cmake = shutil.which("cmake") or "cmake"
    command = [cmake, "--build", str(build_dir)]
    if toolchain.generator.startswith("Visual Studio"):
        command += ["--config", config]
    jobs = args.jobs
    if jobs is None:
        jobs = os.cpu_count() or 4
    command += ["--parallel", str(jobs)]
    if target:
        command += ["--target", target]
    command += args.build_arg
    return command


# ---------------------------------------------------------------------------
# 命令实现
# ---------------------------------------------------------------------------
def _require_cmake(messages) -> str:
    cmake = shutil.which("cmake")
    if not cmake:
        raise BuildError(messages.t("build.cmake_missing"), messages.hint("build.cmake_missing_hint"))
    return cmake


def _prepare(args, messages, runner: Runner, toolchain: Toolchain,
             arch: str, build_dir: Path, deps_mode: str, force: bool) -> None:
    print(messages.t("build.configuring") + f" (deps: {deps_mode})", flush=True)
    command = _configure_command(args, toolchain, arch, args.config, build_dir,
                                 deps_mode, force)
    if runner.run(command, cwd=REPO_ROOT) != 0:
        raise BuildError(messages.t("build.failed"), messages.hint("build.failed_hint"))
    print(messages.t("build.building") + " (deps)", flush=True)
    if runner.run(_build_command(args, toolchain, build_dir, args.config, "duilib_deps"),
                  cwd=REPO_ROOT) != 0:
        raise BuildError(messages.t("build.failed"), messages.hint("build.failed_hint"))
    write_build_mode_stamp(build_dir, args, runner.dry_run)


def _check_cef_archive(args, messages) -> None:
    """--cef-archive 指向不存在的文件时提前给出明确错误。"""
    if not args.cef_archive:
        return
    path = Path(args.cef_archive).expanduser().resolve()
    if not path.is_file():
        raise BuildError(messages.t("build.cef_archive_missing") + f": {path}",
                         messages.hint("build.cef_archive_missing_hint"))


def _deps_root(args) -> Path:
    return Path(args.deps_root).expanduser().resolve() if args.deps_root else REPO_ROOT.parent


def _git_capture(repo: Path, args: list[str]) -> tuple[int, str]:
    """执行只读 git 查询，返回 (退出码, 标准输出)。git 不可用时返回 (1, "")。"""
    try:
        completed = subprocess.run(["git", "-C", str(repo), *args],
                                   capture_output=True, text=True, errors="replace")
    except OSError:
        return 1, ""
    return completed.returncode, completed.stdout.strip()


def git_head(repo: Path) -> str:
    """仓库当前的提交哈希；不是 git 仓库时返回空串。"""
    if not (repo / ".git").is_dir():
        return ""
    code, output = _git_capture(repo, ["rev-parse", "HEAD"])
    return output if code == 0 else ""


def resolve_git_ref(repo: Path, ref: dict) -> str:
    """把清单 ref（commit/tag/branch）解析成本地仓库里的提交；解析不到返回空串。"""
    if not isinstance(ref, dict) or not (repo / ".git").is_dir():
        return ""
    kind = str(ref.get("type", ""))
    value = str(ref.get("value", ""))
    if not value:
        return ""
    specs = {
        "commit": [value],
        "tag": [f"refs/tags/{value}"],
        "branch": [f"refs/remotes/origin/{value}", f"refs/heads/{value}"],
    }.get(kind, [value])
    for spec in specs:
        code, output = _git_capture(repo, ["rev-parse", "--verify", "--quiet", spec])
        if code == 0 and output:
            return output
    return ""


def patch_archive_stale(patch: Path, repo_dir: Path) -> bool:
    """补丁归档与仓库当前提交不一致（被替换成旧版本或已删除）。

    同名归档可能在 skia_compile 里被重复提交（如 skia.2026-09-16.src.zip），
    只判断“文件存在”会让看起来相同的旧归档一直产生同一份 Skia 产物。
    """
    if not (repo_dir / ".git").is_dir():
        return False
    code, status = _git_capture(repo_dir, ["status", "--porcelain",
                                           "--untracked-files=no", "--", patch.name])
    return code == 0 and bool(status)


def _skia_patch_archive(manifest, args) -> Path | None:
    """当前 Skia ref 对应的补丁归档路径；该 ref 不需要补丁时返回 None。"""
    ref = getattr(args, "skia_ref", None) or manifest["skia"]["default"]
    patch_zip = manifest["skia"].get("refs", {}).get(ref, {}).get("patchZip", "")
    return _deps_root(args) / "skia_compile" / patch_zip if patch_zip else None


def _skia_patch_needed(args) -> bool:
    """已有 Skia 编译产物时由 CMake 判断是否复用，避免无谓的补丁下载。"""
    if getattr(args, "force", False):
        return True
    skia_out = _deps_root(args) / "skia" / "out"
    if not skia_out.is_dir():
        return True
    return not any((entry / name).is_file()
                   for entry in skia_out.iterdir()
                   for name in ("skia.lib", "libskia.a"))


def _retry_settings(manifest: dict) -> tuple[int, int]:
    """清单 retry 段：最大次数（0 = 无限）与重试间隔秒数。"""
    retry = manifest.get("retry", {}) if isinstance(manifest, dict) else {}
    try:
        attempts = int(retry.get("maxAttempts", 0))
    except (TypeError, ValueError):
        attempts = 0
    try:
        delay = int(retry.get("delaySeconds", 20))
    except (TypeError, ValueError):
        delay = 20
    return max(attempts, 0), max(delay, 0)


def _run_with_retry(runner: Runner, command, messages, what: str,
                    attempts: int, delay: int) -> bool:
    """执行网络命令；失败时按清单的统一参数重试（attempts=0 表示无限）。"""
    attempt = 1
    while True:
        if runner.run(command) == 0:
            return True
        if attempts and attempt >= attempts:
            return False
        print(messages.t("deps.retry").format(what=what, attempt=attempt, delay=delay),
              flush=True)
        time.sleep(delay)
        attempt += 1


def _restore_skia_patch(patch: Path, messages, runner: Runner, args) -> None:
    """把被替换成旧版本（或删掉）的补丁归档恢复成仓库当前提交里的版本。

    不联网：git checkout 只用本地对象。
    """
    if getattr(args, "dry_run", False) or not patch_archive_stale(patch, patch.parent):
        return
    print(messages.t("deps.skia_patch_restoring") + f" ({patch.name})", flush=True)
    if runner.run(["git", "-C", str(patch.parent), "checkout", "--", patch.name]) != 0:
        _warn_line(messages.t("deps.skia_patch_restore_failed") + f" ({patch})")


def _ensure_skia_patch(args, messages, runner: Runner) -> None:
    """确保 Skia 补丁归档与 skia_compile 的当前版本一致。

    归档已存在时也要刷新：同名归档可能被重复提交，本地旧文件会让旧的 Skia
    产物一直被复用。仓库不存在时按需克隆（已有 Skia 产物且归档齐备时不下载）。
    """
    try:
        manifest = load_manifest(VERSIONS_FILE)
    except (OSError, ValueError):
        # 清单损坏时交给 configure 阶段统一报错
        return
    patch = _skia_patch_archive(manifest, args)
    if patch is None:
        return
    repo_dir = patch.parent
    offline = getattr(args, "offline", False)

    if (repo_dir / ".git").is_dir():
        # 先恢复本地归档：带本地修改时 git pull 会被拒绝；恢复后 pull 才能取到
        # 最新提交（同名归档的新版本）。
        _restore_skia_patch(patch, messages, runner, args)
        if not offline:
            # 与旧脚本一致：更新失败只告警，继续使用本地已有内容
            print(messages.t("deps.skia_patch_updating") + f" ({repo_dir})", flush=True)
            if runner.run(["git", "-C", str(repo_dir), "pull", "--ff-only"]) != 0:
                _warn_line(messages.t("deps.skia_patch_update_failed"))

    if patch.is_file() or not _skia_patch_needed(args):
        return
    if offline:
        raise BuildError(messages.t("deps.skia_patch_missing") + f": {patch}",
                         messages.hint("deps.offline_missing_hint"))

    repo_url = manifest.get("skiaCompileRepo") or SKIA_COMPILE_REPO
    attempts, delay = _retry_settings(manifest)
    if (repo_dir / ".git").is_dir():
        # 仓库已存在但归档仍缺失（远端更新失败或离线拉取未成功）
        if not args.dry_run and not patch.is_file():
            raise BuildError(messages.t("deps.skia_patch_missing") + f": {patch}",
                             messages.hint("deps.skia_patch_fetch_failed_hint"))
        return
    if repo_dir.exists() and any(repo_dir.iterdir()):
        # 非 git 目录无法克隆进去，避免无限重试
        raise BuildError(messages.t("deps.skia_patch_dir_invalid") + f": {repo_dir}",
                         messages.hint("deps.skia_patch_dir_invalid_hint"))
    print(messages.t("deps.skia_patch_fetching") + f" ({repo_dir})", flush=True)
    repo_dir.parent.mkdir(parents=True, exist_ok=True)
    if not _run_with_retry(
            runner,
            ["git", "clone", "--depth", "1", repo_url, str(repo_dir)],
            messages, f"git clone {repo_url}", attempts, delay):
        raise BuildError(messages.t("deps.skia_patch_missing") + f": {patch}",
                         messages.hint("deps.skia_patch_fetch_failed_hint"))
    if not args.dry_run and not patch.is_file():
        raise BuildError(messages.t("deps.skia_patch_missing") + f": {patch}",
                         messages.hint("deps.skia_patch_fetch_failed_hint"))


def cmd_deps(args, messages) -> int:
    _require_cmake(messages)
    _check_cef_archive(args, messages)
    arch = resolve_arch(args.arch)
    build_dir = resolve_build_dir(REPO_ROOT, arch, args.config, args.build_dir)
    toolchain = detect_toolchain(arch, args.vs_version)
    reset_stale_build_dir(build_dir, toolchain, messages, args.dry_run)
    reset_stale_build_mode(build_dir, args, messages, args.dry_run)
    sync_msvc_dll_flag(getattr(args, "dll", False), messages, args.dry_run)
    runner = Runner(toolchain.env, args.dry_run)
    _ensure_skia_patch(args, messages, runner)
    _prepare(args, messages, runner, toolchain, arch, build_dir, "prepare", args.force)
    return 0


def cmd_build(args, messages) -> int:
    _require_cmake(messages)
    _check_cef_archive(args, messages)
    arch = resolve_arch(args.arch)
    build_dir = resolve_build_dir(REPO_ROOT, arch, args.config, args.build_dir)
    toolchain = detect_toolchain(arch, args.vs_version)
    reset_stale_build_dir(build_dir, toolchain, messages, args.dry_run)
    reset_stale_build_mode(build_dir, args, messages, args.dry_run)
    sync_msvc_dll_flag(getattr(args, "dll", False), messages, args.dry_run)
    runner = Runner(toolchain.env, args.dry_run)

    if not args.no_deps:
        _ensure_skia_patch(args, messages, runner)
        _prepare(args, messages, runner, toolchain, arch, build_dir, "prepare", False)

    print(messages.t("build.configuring") + f" ({arch}/{args.config})", flush=True)
    if runner.run(_configure_command(args, toolchain, arch, args.config, build_dir, "off", False),
                  cwd=REPO_ROOT) != 0:
        raise BuildError(messages.t("build.failed"), messages.hint("build.failed_hint"))
    write_build_mode_stamp(build_dir, args, args.dry_run)

    # Debug/Release 的示例都输出到共享的 bin/，但目标文件按配置分开；先删掉
    # 另一种配置的示例产物，否则 Ninja 会认为链接步骤已最新而跳过（bin/ 不会更新）。
    if not getattr(args, "no_examples", False):
        clear_stale_build_outputs(REPO_ROOT / "bin", args.config, args.target,
                                  messages, args.dry_run)

    print(messages.t("build.building") + f" ({build_dir})", flush=True)
    if runner.run(_build_command(args, toolchain, build_dir, args.config, args.target),
                  cwd=REPO_ROOT) != 0:
        raise BuildError(messages.t("build.failed"), messages.hint("build.failed_hint"))
    if not args.dry_run:
        print(messages.t("build.done"))
    return 0


def _ok_line(text: str) -> None:
    print(f"[OK]    {text}", flush=True)


def _warn_line(text: str) -> None:
    print(f"[WARN]  {text}", flush=True)


def _info_line(text: str) -> None:
    print(f"[INFO]  {text}", flush=True)


def _check_source_ref(label: str, src: Path, ref_key: str, refs: dict) -> None:
    """本地依赖源码不在清单指定的提交上时提示。

    deps 会先把工作区切到清单的 ref，再按新提交重新编译依赖；这里提前说明，
    避免用户以为编译用的是清单版本。
    """
    if not isinstance(refs, dict):
        return
    expected = resolve_git_ref(src, refs.get(ref_key, {}))
    actual = git_head(src)
    if expected and actual and actual != expected:
        _warn_line(f"{label} source at {actual[:12]}, manifest {ref_key} expects "
                   f"{expected[:12]}; build.py deps will switch it")


def cmd_doctor(args, messages) -> int:
    arch = resolve_arch(args.arch)
    host = platform_name()
    print(messages.t("doctor.title"), flush=True)
    _ok_line(f"platform {host}/{arch}")
    # 显示实际的解释器名字（Linux 上通常只有 python3，避免误以为存在 python）
    _ok_line(f"{Path(sys.executable).stem} {platform.python_version()} ({sys.executable})")
    _ok_line(f"repository {REPO_ROOT}")

    cmake = shutil.which("cmake")
    if not cmake:
        _warn_line(messages.t("doctor.cmake_missing"))
    else:
        completed = subprocess.run([cmake, "--version"], capture_output=True, text=True,
                                   errors="replace")
        _ok_line(f"cmake {completed.stdout.splitlines()[0] if completed.stdout else 'unknown'}")

    toolchain = None
    try:
        toolchain = detect_toolchain(arch, args.vs_version)
    except BuildError as error:
        _warn_line(error.message)
    if toolchain:
        _ok_line(f"generator {toolchain.generator}")
        _ok_line(f"ninja {toolchain.ninja}") if toolchain.ninja else _warn_line("ninja not found")
        if toolchain.vs_install:
            _ok_line(f"visual studio {toolchain.vs_install}")

    manifest = None
    try:
        manifest = load_manifest(VERSIONS_FILE)
        _ok_line(f"skia ref {manifest['skia']['default']}")
        _ok_line(f"sdl ref {manifest['sdl']['default']}")
        _ok_line(f"cef default {manifest['cef']['default']}")
    except (OSError, ValueError) as error:
        _warn_line(f"third_party_versions.json: {error}")

    deps_root = _deps_root(args)
    _ok_line(f"deps root {deps_root}")
    offline = getattr(args, "offline", False)
    missing_hint = "offline: cannot fetch it" if offline else "build.py deps will fetch it"

    # Skia 源码与补丁归档都由 build.py deps 自动获取。
    skia_root = deps_root / "skia"
    if skia_root.is_dir():
        _ok_line(f"skia source {skia_root}")
        if manifest:
            _check_source_ref("skia", skia_root, args.skia_ref or manifest["skia"]["default"],
                              manifest["skia"]["refs"])
    else:
        _warn_line(f"skia source missing: {skia_root} ({missing_hint})")

    patch = _skia_patch_archive(manifest, args) if manifest else None
    if patch is not None:
        if patch.is_file():
            if patch_archive_stale(patch, patch.parent):
                # 同名归档可能被重复提交，旧的本地副本会让 Skia 产物一直不更新
                _warn_line(f"skia patch differs from skia_compile HEAD: {patch.name} "
                           "(build.py deps will restore it)")
            else:
                _ok_line(f"skia patch {patch}")
        else:
            _warn_line(f"skia patch missing: {patch} ({missing_hint})")

    # SDL 在 Windows 默认关闭，其他平台强制开启；只报告当前配置实际需要的依赖。
    sdl_enabled = args.sdl if args.sdl is not None else host != "windows"
    sdl_src = deps_root / "SDL"
    sdl_root = deps_root / "SDL3"
    if not sdl_enabled:
        _info_line("sdl disabled (pass --sdl to enable)")
    else:
        if sdl_src.is_dir():
            _ok_line(f"sdl source {sdl_src}")
            if manifest:
                _check_source_ref("sdl", sdl_src, args.sdl_ref or manifest["sdl"]["default"],
                                  manifest["sdl"]["refs"])
        else:
            _warn_line(f"sdl source missing: {sdl_src} ({missing_hint})")
        if sdl_root.is_dir():
            _ok_line(f"sdl prefix {sdl_root}")
        else:
            # 首次构建前 SDL3 前缀必然不存在，由 deps 生成
            _info_line(f"sdl prefix not built yet: {sdl_root} ("
                       f"{'offline: cannot build it' if offline else 'build.py deps will build it'})")

    # CEF 运行时由 deps 下载；MinGW 与 FreeBSD 不支持 CEF。
    cef_supported = host in ("linux", "macos") or (host == "windows" and not os.environ.get("MSYSTEM"))
    if not cef_supported:
        _info_line("cef not supported in this environment")
    else:
        cef_version = getattr(args, "cef_version", None) or (manifest["cef"]["default"] if manifest else "")
        if manifest and cef_version == "latest":
            cef_version = manifest["cef"]["default"]
        if host == "windows":
            suffix = "_109" if cef_version == "109" else ""
            cef_arch = "Win32" if arch == "x86" else arch
            cef_dest, cef_marker = REPO_ROOT / "bin" / f"libcef_win{suffix}" / cef_arch, "libcef.dll"
        elif host == "linux":
            cef_dest, cef_marker = REPO_ROOT / "bin" / "libcef_linux", "libcef.so"
        else:
            cef_dest, cef_marker = deps_root / "cef_binary", Path("include") / "cef_version.h"
        if (cef_dest / cef_marker).is_file():
            _ok_line(f"cef runtime {cef_version} {cef_dest}")
        else:
            _info_line(f"cef runtime {cef_version} not prepared yet: {cef_dest} ("
                       f"{'offline: cannot download it' if offline else 'build.py deps will download it'})")

    if not cmake:
        return 1 if not shutil.which("git") else 0

    if toolchain is None:
        try:
            toolchain = detect_toolchain(arch, args.vs_version)
        except BuildError:
            toolchain = None
    env = toolchain.env if toolchain else dict(os.environ)
    # doctor 只做检查，preflight 脚本不会下载/安装依赖；用 prepare 让 gn 等
    # 依赖工具成为硬性检查项，而不是在 off 模式下只告警。
    command = [cmake,
               f"-DDUILIB_LANG={messages.language}",
               "-DDUILIB_DEPS_MODE=prepare",
               f"-DDUILIB_DEPS_ROOT={deps_root}",
               f"-DDUILIB_SKIA_ROOT={skia_root}",
               f"-DDUILIB_GN_EXECUTABLE={os.environ.get('DUILIB_GN_EXECUTABLE', '')}",
               f"-DDUILIB_GENERATOR={toolchain.generator if toolchain else 'Ninja'}",
               "-P", str(PREFLIGHT_SCRIPT)]
    print("+ " + _format_command(command), flush=True)
    if args.dry_run:
        return 0
    completed = subprocess.run(command, env=env)
    return completed.returncode


def cmd_test(args, messages) -> int:
    command = [sys.executable, "-m", "unittest", "discover", "-s", str(BUILD_DIR),
               "-p", "test_*.py", "-v"]
    print("+ " + _format_command(command), flush=True)
    if args.dry_run:
        return 0
    print(messages.t("test.start"), flush=True)
    completed = subprocess.run(command, cwd=str(REPO_ROOT))
    if completed.returncode != 0:
        raise BuildError(messages.t("test.failed"), messages.hint("test.failed_hint"))
    return 0


def cmd_pack(args, messages) -> int:
    arch = resolve_arch(args.arch)
    if args.sdl and not args.with_sdl:
        # --sdl 是编译开关；打包走 pack.py 的 --with-sdl，两个开关容易混淆
        print(messages.t("pack.sdl_flag_hint"), flush=True)
    command = [sys.executable, str(BUILD_DIR / "pack.py"),
               "--config", args.config, "--arch", arch]
    if args.with_cef:
        command.append("--with-cef")
        # pack.py 只区分 109 与最新版；latest/未指定/清单默认版本都不传。
        if args.cef_version == "109":
            command += ["--cef-version", "109"]
    if args.with_sdl:
        command.append("--with-sdl")
    if args.with_examples:
        command.append("--with-examples")
    if args.out_dir:
        command += ["--out-dir", args.out_dir]
    if args.md:
        command += ["--msvc-runtime", "md"]
    command += args.pack_arg
    print("+ " + _format_command(command), flush=True)
    if args.dry_run:
        return 0
    print(messages.t("pack.start"), flush=True)
    completed = subprocess.run(command, cwd=str(REPO_ROOT))
    if completed.returncode != 0:
        raise BuildError(messages.t("pack.failed"), messages.hint("pack.failed_hint"))
    return 0


def cmd_clean(args, messages) -> int:
    targets = []
    if args.build_dir:
        targets.append(resolve_build_dir(REPO_ROOT, resolve_arch(args.arch), args.config, args.build_dir))
    else:
        targets.append(REPO_ROOT / "out")
    if args.deps:
        deps_root = Path(args.deps_root).expanduser().resolve() if args.deps_root else REPO_ROOT.parent
        targets.extend([
            deps_root / "skia" / "out",
            deps_root / "SDL3",
            deps_root / "SDL3_build",
            deps_root / "cef_binary" / "downloads",
            deps_root / "cef_binary" / "extract",
        ])
    for target in targets:
        print(f"[clean] {target}", flush=True)
        if not args.dry_run and target.exists():
            try:
                shutil.rmtree(target)
            except OSError as exc:
                raise BuildError(
                    messages.t("build.clean_failed").format(path=target, error=exc),
                    messages.hint("build.clean_failed_hint"))
    if not args.dry_run:
        print(messages.t("build.clean_deps_done") if args.deps else messages.t("build.clean_done"))
    return 0


# ---------------------------------------------------------------------------
# 入口
# ---------------------------------------------------------------------------
def main(argv=None) -> int:
    # Console language must be ready before argparse: --help is output too.
    global MESSAGES
    argv = sys.argv[1:] if argv is None else list(argv)
    MESSAGES = make_messages(_requested_lang(argv))
    set_console_utf8()
    args = parse_args(argv)
    messages = MESSAGES

    if sys.version_info < (3, 8):
        print(messages.t("build.python_missing"))
        print(messages.hint("build.python_missing_hint"))
        return 1

    try:
        handlers = {
            "build": cmd_build,
            "deps": cmd_deps,
            "doctor": cmd_doctor,
            "test": cmd_test,
            "pack": cmd_pack,
            "clean": cmd_clean,
        }
        return handlers[args.command](args, messages)
    except BuildError as error:
        print(error.message, file=sys.stderr)
        if error.hint:
            print("  " + error.hint, file=sys.stderr)
        return error.code
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    sys.exit(main())
