"""nim_duilib 统一构建入口。

用法（macOS/Linux/FreeBSD；Windows 可运行 build\\build.bat，参数相同）：
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
from pathlib import Path

from i18n import make_messages, set_console_utf8

REPO_ROOT = Path(__file__).resolve().parent.parent
BUILD_DIR = REPO_ROOT / "build"
CMAKE_FILE = REPO_ROOT / "CMakeLists.txt"
VERSIONS_FILE = REPO_ROOT / "cmake" / "third_party_versions.json"
PREFLIGHT_SCRIPT = REPO_ROOT / "cmake" / "duilib_preflight_tools.cmake"

VALID_CONFIGS = ("Debug", "Release")
VALID_ARCHES = ("auto", "x86", "x64", "arm64")
DEFAULT_CONFIG = "Release"
DEFAULT_DEPS_MODE = "prepare"


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
def _add_common_options(parser: argparse.ArgumentParser, suppress_defaults: bool = False) -> None:
    """公共选项。

    子命令解析器也定义同一批选项，以便 `build.py build --config Debug` 可用；
    子命令侧必须抑制默认值，否则 argparse 会用子命令的默认值覆盖写在子命令
    前面的选项（`build.py --config Debug build` 会变回 Release）。
    """

    def default(value):
        return argparse.SUPPRESS if suppress_defaults else value

    parser.add_argument("--config", choices=VALID_CONFIGS, default=default(DEFAULT_CONFIG),
                        help="构建配置（一次只构建一个，默认 Release）")
    parser.add_argument("--arch", choices=VALID_ARCHES, default=default("auto"),
                        help="目标架构（默认主机架构）")
    parser.add_argument("--lang", choices=("auto", "zh_CN", "en"), default=default("auto"),
                        help="交互语言（默认跟随系统）")
    parser.add_argument("--build-dir", default=default(None), help="覆盖构建目录（默认 out/<arch>/<Config>）")
    parser.add_argument("--deps-root", default=default(None), help="依赖源码/安装根目录（默认仓库同级）")
    parser.add_argument("--jobs", type=int, default=default(None), help="并行编译任务数")
    parser.add_argument("--offline", action="store_true", default=default(False),
                        help="只使用已缓存的依赖源码/归档")
    parser.add_argument("--dry-run", action="store_true", default=default(False), help="只打印将执行的命令")
    parser.add_argument("--cmake-arg", action="append", default=default([]),
                        help="追加给 cmake configure 的参数")
    parser.add_argument("--build-arg", action="append", default=default([]),
                        help="追加给 cmake --build 的参数")
    parser.add_argument("--vs-version", default=default(None),
                        help="显式指定 Visual Studio 主版本（如 17）")
    parser.add_argument("--cef-version", choices=("109", "142"), default=default(None),
                        help="CEF 版本（默认取 third_party_versions.json）")
    parser.add_argument("--skia-ref", default=default(None), help="Skia ref key（默认取清单）")
    parser.add_argument("--sdl-ref", default=default(None), help="SDL3 ref key（默认取清单）")
    parser.add_argument("--sdl", dest="sdl", action="store_true", default=default(None),
                        help="强制启用 SDL")
    parser.add_argument("--no-sdl", dest="sdl", action="store_false", default=default(None),
                        help="强制关闭 SDL")
    parser.add_argument("--md", action="store_true", default=default(False),
                        help="使用 MSVC 动态运行库 (/MD, /MDd)")
    parser.add_argument("--cef-archive", default=default(None),
                        help="本地 CEF 归档路径（跳过下载）")


def build_parser() -> argparse.ArgumentParser:
    main_common = argparse.ArgumentParser(add_help=False)
    _add_common_options(main_common)
    sub_common = argparse.ArgumentParser(add_help=False)
    _add_common_options(sub_common, suppress_defaults=True)

    parser = argparse.ArgumentParser(
        prog="build.py",
        description="nim_duilib 统一构建入口",
        epilog=("子命令专属参数需查看子命令帮助（build.py <command> --help）：\n"
                "  build.py build --help   # --no-deps / --target / --no-examples\n"
                "  build.py deps --help    # --force\n"
                "  build.py pack --help    # --with-cef / --with-sdl / --with-examples / --out-dir / --pack-arg\n"
                "  build.py clean --help   # --deps"),
        formatter_class=argparse.RawDescriptionHelpFormatter,
        parents=[main_common])
    sub = parser.add_subparsers(dest="command")

    build_parser_ = sub.add_parser("build", parents=[sub_common], help="准备依赖并编译一个配置")
    build_parser_.add_argument("--no-deps", action="store_true", help="跳过依赖准备")
    build_parser_.add_argument("--target", default=None, help="只构建指定目标")
    build_parser_.add_argument("--no-examples", action="store_true", help="不编译示例")

    deps_parser = sub.add_parser("deps", parents=[sub_common], help="只准备 Skia/SDL3/CEF")
    deps_parser.add_argument("--force", action="store_true", help="重建依赖")
    deps_parser.add_argument("--no-examples", action="store_true", help=argparse.SUPPRESS)

    sub.add_parser("doctor", parents=[sub_common], help="环境自检（无副作用）")

    sub.add_parser("test", parents=[sub_common], help="运行单元测试")

    pack_parser = sub.add_parser("pack", parents=[sub_common], help="调用 pack.py 出包")
    pack_parser.add_argument("--with-cef", action="store_true", help="包内包含 CEF")
    pack_parser.add_argument("--with-sdl", action="store_true", help="包内包含 SDL3")
    pack_parser.add_argument("--with-examples", action="store_true", help="包内包含示例")
    pack_parser.add_argument("--out-dir", default=None, help="包输出目录")
    pack_parser.add_argument("--pack-arg", action="append", default=[], help="追加给 pack.py 的参数")

    clean_parser = sub.add_parser("clean", parents=[sub_common], help="清理构建树")
    clean_parser.add_argument("--deps", action="store_true", help="同时清理依赖构建产物")

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
                          ("with_examples", False), ("out_dir", None), ("pack_arg", [])):
        if not hasattr(args, name):
            setattr(args, name, default)
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


def _msvc_environment(arch: str, vs_version: str | None) -> tuple[dict, Path | None]:
    install = _vswhere_installation(vs_version)
    if not install:
        return dict(os.environ), None
    vcvars = install / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat"
    if not vcvars.is_file():
        return dict(os.environ), install
    vs_arch = {"x86": "x86", "x64": "x64", "arm64": "arm64"}[arch]
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
            f"open the Visual Studio Installer and add the '{vs_arch}' MSVC build tools "
            "(Individual components > MSVC ... build tools)")
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


def detect_toolchain(arch: str, vs_version: str | None) -> Toolchain:
    env = dict(os.environ)
    vs_install = None
    if sys.platform == "win32" and not env.get("MSYSTEM"):
        env, vs_install = _msvc_environment(arch, vs_version)
        if vs_install is None and not shutil.which("cl", path=env.get("PATH")):
            raise BuildError(
                "[ERROR] Visual Studio 2022+ with the C++ workload was not found",
                "install Visual Studio 2022 (or newer) with 'Desktop development with C++'")
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
        # 与根工程默认一致：Windows（含 MinGW）默认关闭，其他平台强制开启
        sdl_enabled = sys.platform != "win32"
    defines.append(f"-DDUILIB_ENABLE_SDL={'ON' if sdl_enabled else 'OFF'}")
    defines.append(f"-DDUILIB_MD={'ON' if args.md else 'OFF'}")
    defines.append(f"-DDUILIB_BUILD_EXAMPLES={'OFF' if getattr(args, 'no_examples', False) else 'ON'}")
    defines.append(f"-DDUILIB_CEF_VERSION={args.cef_version or manifest['cef']['default']}")
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


def _check_cef_archive(args, messages) -> None:
    """--cef-archive 指向不存在的文件时提前给出明确错误。"""
    if not args.cef_archive:
        return
    path = Path(args.cef_archive).expanduser().resolve()
    if not path.is_file():
        raise BuildError(messages.t("build.cef_archive_missing") + f": {path}",
                         messages.hint("build.cef_archive_missing_hint"))


def cmd_deps(args, messages) -> int:
    _require_cmake(messages)
    _check_cef_archive(args, messages)
    arch = resolve_arch(args.arch)
    build_dir = resolve_build_dir(REPO_ROOT, arch, args.config, args.build_dir)
    toolchain = detect_toolchain(arch, args.vs_version)
    reset_stale_build_dir(build_dir, toolchain, messages, args.dry_run)
    runner = Runner(toolchain.env, args.dry_run)
    _prepare(args, messages, runner, toolchain, arch, build_dir, "prepare", args.force)
    return 0


def cmd_build(args, messages) -> int:
    _require_cmake(messages)
    _check_cef_archive(args, messages)
    arch = resolve_arch(args.arch)
    build_dir = resolve_build_dir(REPO_ROOT, arch, args.config, args.build_dir)
    toolchain = detect_toolchain(arch, args.vs_version)
    reset_stale_build_dir(build_dir, toolchain, messages, args.dry_run)
    runner = Runner(toolchain.env, args.dry_run)

    if not args.no_deps:
        _prepare(args, messages, runner, toolchain, arch, build_dir, "prepare", False)

    print(messages.t("build.configuring") + f" ({arch}/{args.config})", flush=True)
    if runner.run(_configure_command(args, toolchain, arch, args.config, build_dir, "off", False),
                  cwd=REPO_ROOT) != 0:
        raise BuildError(messages.t("build.failed"), messages.hint("build.failed_hint"))

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


def cmd_doctor(args, messages) -> int:
    arch = resolve_arch(args.arch)
    print(messages.t("doctor.title"), flush=True)
    _ok_line(f"platform {platform_name()}/{arch}")
    _ok_line(f"python {platform.python_version()} ({sys.executable})")
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

    try:
        manifest = load_manifest(VERSIONS_FILE)
        _ok_line(f"skia ref {manifest['skia']['default']}")
        _ok_line(f"sdl ref {manifest['sdl']['default']}")
        _ok_line(f"cef default {manifest['cef']['default']}")
    except (OSError, ValueError) as error:
        _warn_line(f"third_party_versions.json: {error}")

    deps_root = Path(args.deps_root).expanduser().resolve() if args.deps_root else REPO_ROOT.parent
    skia_root = deps_root / "skia"
    sdl_root = deps_root / "SDL3"
    _ok_line(f"skia source {skia_root}") if skia_root.is_dir() else _warn_line(f"skia source missing: {skia_root}")
    if sdl_root.is_dir():
        _ok_line(f"sdl prefix {sdl_root}")
    else:
        # 首次构建前 SDL3 前缀必然不存在，由 deps 生成，因此只作提示
        _info_line(f"sdl prefix not built yet: {sdl_root} (build.py deps will build it)")

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
    args = parse_args(argv)
    set_console_utf8()
    messages = make_messages(args.lang)

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
