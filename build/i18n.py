"""交互语言与消息目录（build 入口共用）。

单一维护点：cmake/i18n/messages.json。
语言判定顺序与 cmake/duilib_i18n.cmake 保持一致：
  --lang / DUILIB_LANG -> LC_ALL/LC_MESSAGES/LANG -> 系统区域 -> en
"""

from __future__ import annotations

import json
import os
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
MESSAGES_FILE = REPO_ROOT / "cmake" / "i18n" / "messages.json"


def _system_locale() -> str:
    if sys.platform == "win32":
        try:
            import winreg

            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Control Panel\International") as key:
                return str(winreg.QueryValueEx(key, "LocaleName")[0])
        except OSError:
            return ""
    for name in ("LC_ALL", "LC_MESSAGES", "LANG"):
        value = os.environ.get(name, "")
        if value:
            return value
    return ""


def _normalize_language(tag: str) -> str:
    """把区域标签归一为 zh_CN / en。

    只有简体中文（zh、zh_CN、zh_Hans，含 zh_Hans_CN 之类）显示中文；
    zh_TW / zh_HK / zh_MO / zh_Hant 等繁体区域一律显示英文，
    与 cmake/duilib_i18n.cmake 的 ^zh([-_]CN|[-_]Hans)?$ 判定保持一致。
    """
    normalized = (tag or "").replace("-", "_").lower()
    if not normalized.startswith("zh"):
        return "en"
    if normalized.startswith(("zh_tw", "zh_hk", "zh_mo", "zh_hant")):
        return "en"
    return "zh_CN"


def resolve_language(cli=None, env=None, system=None) -> str:
    """纯函数：返回 zh_CN 或 en。"""
    env = os.environ if env is None else env
    if system is None:
        system = _system_locale()

    candidate = cli if cli and cli != "auto" else env.get("DUILIB_LANG", "")
    if candidate and candidate != "auto":
        return _normalize_language(candidate)

    return _normalize_language(system)


def _load_messages() -> dict:
    with MESSAGES_FILE.open("r", encoding="utf-8") as handle:
        return json.load(handle).get("messages", {})


class Messages:
    def __init__(self, language: str, messages: dict | None = None):
        self.language = language if language in ("zh_CN", "en") else "en"
        self._messages = _load_messages() if messages is None else messages

    def t(self, key: str) -> str:
        entry = self._messages.get(key)
        if not isinstance(entry, dict):
            return key
        text = entry.get(self.language)
        if not text:
            text = entry.get("en", key)
        return text

    def hint(self, key: str) -> str:
        return "Hint: " + self.t(key)


def set_console_utf8() -> bool:
    """Windows 控制台切到 UTF-8；返回是否成功（非 Windows 视为成功）。"""
    if sys.platform != "win32":
        return True
    try:
        import ctypes

        kernel32 = ctypes.windll.kernel32
        kernel32.SetConsoleOutputCP(65001)
        kernel32.SetConsoleCP(65001)
        for stream in (sys.stdout, sys.stderr):
            if hasattr(stream, "reconfigure"):
                stream.reconfigure(encoding="utf-8", errors="replace")
        os.environ["DUILIB_CONSOLE_UTF8"] = "1"
        return True
    except (AttributeError, OSError):
        return False


def make_messages(cli_lang: str | None = None) -> Messages:
    return Messages(resolve_language(cli=cli_lang))
