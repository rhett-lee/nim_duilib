"""文档一致性测试：维护范围内的 Markdown 相对链接与中英文配对（标准库 unittest）。

运行：
  python3 build/build.py test
  python3 -m unittest build.test_docs -v

范围（`docs/misc/` 下的历史计划与 spec 属于历史记录，不在此列）：
  - 仓库根目录 `*.md`
  - `build/*.md`
  - `docs/*.md`
  - `.trae/**/*.md`、`.workbuddy/**/*.md`（技能文档）

检查两件事：
  1. 相对链接（`](path)` / `](path#anchor)`）指向的文件或目录确实存在；
  2. `docs/` 顶层的每份文档都有对应的中英文版本（`X.md` ↔ `X.en.md` 双向检查）。

代码块与行内代码里出现的 `](...)` 常常是 C++ 签名示例，扫描前先剔除，
避免把示例当链接。
"""

from __future__ import annotations

import re
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

DOC_PATTERNS = ("*.md", "build/*.md", "docs/*.md",
                ".trae/**/*.md", ".workbuddy/**/*.md")

_FENCE = re.compile(r"```.*?```", re.S)
_INLINE_CODE = re.compile(r"`[^`\n]*`")
_LINK = re.compile(r"\]\(([^)\s]+)\)")
_URL_PREFIXES = ("http://", "https://", "mailto:", "#")


def maintained_docs() -> list[Path]:
    """需要保持链接有效的 Markdown 文件。"""
    docs: list[Path] = []
    for pattern in DOC_PATTERNS:
        docs.extend(REPO_ROOT.glob(pattern))
    return sorted(path for path in docs if path.is_file())


def relative_link_targets(text: str) -> list[str]:
    """取出正文中的相对链接目标（去掉代码块、行内代码与外部链接）。"""
    text = _FENCE.sub("", text)
    text = _INLINE_CODE.sub("", text)
    targets: list[str] = []
    for target in _LINK.findall(text):
        if target.startswith(_URL_PREFIXES):
            continue
        if Path(target).is_absolute():
            continue
        targets.append(target)
    return targets


class DocLinkTests(unittest.TestCase):
    def test_relative_links_exist(self):
        problems: list[str] = []
        for doc in maintained_docs():
            content = doc.read_text(encoding="utf-8")
            for target in relative_link_targets(content):
                relative = target.split("#", 1)[0]
                if not relative:
                    continue
                if not (doc.parent / relative).exists():
                    problems.append(f"{doc.relative_to(REPO_ROOT).as_posix()} -> {target}")
        self.assertFalse(problems,
                         "Markdown 相对链接指向不存在的文件：\n" + "\n".join(problems))


class DocLanguagePairTests(unittest.TestCase):
    """docs/ 顶层文档的中英文必须成对：`X.md` 与 `X.en.md` 同时存在（双向检查）。"""

    def test_top_level_docs_have_english_counterpart(self):
        docs_dir = REPO_ROOT / "docs"
        problems: list[str] = []
        for doc in sorted(docs_dir.glob("*.md")):
            if doc.name.endswith(".en.md"):
                chinese = doc.with_name(doc.name[:-len(".en.md")] + ".md")
                if not chinese.is_file():
                    problems.append(f"{doc.name}（缺少 {chinese.name}）")
            else:
                english = doc.with_name(doc.name[:-len(".md")] + ".en.md")
                if not english.is_file():
                    problems.append(f"{doc.name}（缺少 {english.name}）")
        self.assertFalse(problems, "中英文文档不成对：\n" + "\n".join(problems))


if __name__ == "__main__":
    unittest.main(verbosity=2)
