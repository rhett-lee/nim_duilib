#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""校验 nim-duilib skill 文档中引用的标识符是否真实存在于 nim_duilib 源码。

设计目的：防止文档与代码漂移。本项目历史上出现过 26 处文档与源码不一致
（错误的事件枚举名、不存在的字体 ID、写死的错误色值等），靠人工审计成本很高。
本脚本把高置信度的标识符检查自动化，改动 references/ 后跑一次即可。

检查项：
  - DUI_CTR_*      控件/容器节点名宏
  - kEvent*        事件枚举
  - ui::Xxx        C++ 类名
  - system_*_*     字体 ID
  - bg_/text_/color_/border_*  语义色名

用法：
    python scripts/verify_docs.py --repo C:/develop/nim_duilib
    python scripts/verify_docs.py --repo .            # 当前目录就是仓库根
    python scripts/verify_docs.py --repo . --quiet    # 只打印汇总

    # 校验 skill 自带的文档/模板（默认行为）
    # 追加校验自己写出来的代码与 XML（目录会递归，也可直接给文件）
    python scripts/verify_docs.py --repo . \
        --extra examples/hello_button bin/resources/themes/default/hello_button

文档内可用注释声明"此处提到的名字本就应该是缺失的"（例如讲解"不存在的旧名"时）：
    <!-- verify:allow-missing kEventResize kEventChecked arial_12 -->

退出码：0 = 全部命中；1 = 发现疑似漂移。
"""

import argparse
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

# ---------------------------------------------------------------------------
# 标识符提取规则
# ---------------------------------------------------------------------------

# 文档中会出现的、长得像颜色名但其实不是颜色的东西（XML 属性名 / 目录名 / 事件名）
EXCLUDE_IDENTS = {
    # XML 属性名
    "border_size", "border_color", "border_round",
    "text_align", "text_padding", "text_id", "text_changed", "text_color",
    # 目录名 / 分类值
    "color_theme", "color_light", "color_dark", "bg_color", "color_picker",
    # 事件字符串（XML type）
    "value_changed", "visible_changed",
}

PATTERNS = [
    ("DUI_CTR 宏",   re.compile(r"\bDUI_CTR_[A-Z0-9_]+\b")),
    ("事件枚举",      re.compile(r"\bkEvent[A-Za-z]+\b")),
    ("C++ 类名",     re.compile(r"\bui::([A-Z][A-Za-z0-9]*)\b")),
    ("字体 ID",      re.compile(r"\bsystem_(?:regular|bold|underline|italic|strikeout|fullstyle)_(?:12|14|16|18|20|22)\b"
                                r"|\bsystem_(?:12|14|16|18|20|22)\b")),
    ("语义色名",      re.compile(r"\b(?:bg|text|color|border)_[a-z0-9_]+\b")),
    # XML 属性形式：assets/templates 与文档代码块里的实际引用
    ("Class 样式名",  re.compile(r'\bclass="([A-Za-z_]\w*)"')),
    ("XML 字体引用",  re.compile(r'\bfont="([A-Za-z_]\w*)"')),
]

# ---------------------------------------------------------------------------
# 源码索引
# ---------------------------------------------------------------------------


def _read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return ""


def build_index(repo: Path) -> dict:
    """从 nim_duilib 源码中提取各类标识符的真实集合。"""
    idx = {"DUI_CTR 宏": set(), "事件枚举": set(), "C++ 类名": set(),
           "字体 ID": set(), "语义色名": set(),
           "Class 样式名": set(), "XML 字体引用": set()}

    duilib = repo / "duilib"
    if not duilib.is_dir():
        return idx

    headers = list(duilib.rglob("*.h")) + list(duilib.rglob("*.cpp"))
    for h in headers:
        text = _read(h)
        if not text:
            continue
        idx["DUI_CTR 宏"].update(re.findall(r"#define\s+(DUI_CTR_[A-Z0-9_]+)", text))
        idx["事件枚举"].update(re.findall(r"\bkEvent[A-Za-z]+\b", text))
        # class DUILIB_API Xxx / class Xxx / typedef ... Xxx; / using Xxx =
        idx["C++ 类名"].update(re.findall(r"class\s+(?:DUILIB_API\s+)?([A-Za-z_]\w*)", text))
        idx["C++ 类名"].update(re.findall(r"typedef\s+[^;]*?\b([A-Za-z_]\w*)\s*;", text))
        idx["C++ 类名"].update(re.findall(r"using\s+([A-Za-z_]\w*)\s*=", text))
        # 自由函数 / 函数模板（如 auto UiBind(F&&...)）——不是类但同样可调用
        idx["C++ 类名"].update(re.findall(r"\bauto\s+([A-Za-z_]\w*)\s*\(", text))
        # 注意：这里刻意不收集普通 #define 宏名——include guard 等会让集合膨胀到数万，
        # 使检查失去敏感度。宏形式的公开 API（DUI_CTR_*）已由单独一项覆盖。

    # 字体 ID / 色名 / Class 样式名：所有主题的 global.xml
    for g in (repo / "bin" / "resources" / "themes").rglob("global.xml"):
        text = _read(g)
        idx["字体 ID"].update(re.findall(r'<Font\s+id="([^"]+)"', text))
        idx["XML 字体引用"].update(re.findall(r'<Font\s+id="([^"]+)"', text))
        idx["语义色名"].update(re.findall(r'<ThemeColor\s+name="([^"]+)"', text))
        idx["语义色名"].update(re.findall(r'<TextColor\s+name="([^"]+)"', text))
        idx["语义色名"].update(re.findall(r'<Alias\s+name="([^"]+)"', text))
        # <Alias name="旧名" value="新名"/> 中的 value 也是合法色名
        idx["语义色名"].update(re.findall(r'<Alias\s+name="[^"]+"\s+value="([^"]+)"', text))
        idx["Class 样式名"].update(re.findall(r'<Class\s+name="([^"]+)"', text))

    # 内置颜色名（white/gray/blue 等），定义在 UiColors.cpp
    uic = repo / "duilib" / "Core" / "UiColors.cpp"
    if uic.is_file():
        idx["语义色名"].update(re.findall(r'_T\("([A-Za-z_][A-Za-z_0-9]*)"\)', _read(uic)))

    return idx


def probe_paths(repo: Path) -> list:
    """返回用于存在性兜底判断的源码文本（命中即视为存在）。"""
    texts = []
    for rel in ("duilib/duilib_defs.h",
                "bin/resources/themes/default/global.xml",
                "bin/resources/themes/color_light/global.xml",
                "bin/resources/themes/color_dark/global.xml"):
        p = repo / rel
        if p.is_file():
            texts.append(_read(p))
    return texts


# ---------------------------------------------------------------------------
# 文档扫描
# ---------------------------------------------------------------------------

ALLOW_MISSING_RE = re.compile(r"<!--\s*verify:allow-missing\s+([^>]+?)-->")


def scan_file(md: Path, idx: dict, fallback: list) -> list:
    """扫描单个 markdown，返回 [(行号, 标识符, 类别)] 缺失列表。"""
    text = _read(md)
    allow = set()
    for m in ALLOW_MISSING_RE.finditer(text):
        allow.update(m.group(1).split())

    missing = []
    for lineno, line in enumerate(text.splitlines(), 1):
        for kind, pat in PATTERNS:
            for m in pat.finditer(line):
                name = m.group(1) if pat.groups else m.group(0)
                if not name or name in EXCLUDE_IDENTS or name in allow:
                    continue
                if name in idx[kind]:
                    continue
                # 兜底：在关键源码文件里能找到字面量也算存在
                if any(name in t for t in fallback):
                    continue
                missing.append((lineno, name, kind))
    return missing


def main() -> int:
    ap = argparse.ArgumentParser(description="校验 nim-duilib skill 文档与源码的一致性")
    ap.add_argument("--repo", default=".", help="nim_duilib 仓库根目录（含 duilib/ 与 bin/resources/）")
    ap.add_argument("--skill", default=None, help="skill 目录（默认取本脚本的上级目录）")
    ap.add_argument("--extra", nargs="*", default=[], metavar="PATH",
                    help="额外要校验的文件或目录（目录递归取 *.md/*.xml/*.h/*.cpp），"
                         "用于校验自己写出的应用代码与布局")
    ap.add_argument("--quiet", action="store_true", help="只打印汇总，不逐条列出")
    args = ap.parse_args()

    repo = Path(args.repo).resolve()
    if not (repo / "duilib").is_dir():
        print(f"错误：{repo} 下没有 duilib/ 目录，请用 --repo 指定 nim_duilib 仓库根目录。")
        return 2

    skill = Path(args.skill).resolve() if args.skill else Path(__file__).resolve().parent.parent
    targets = []
    if (skill / "SKILL.md").is_file():
        targets.append(skill / "SKILL.md")
    targets += sorted((skill / "references").glob("*.md"))
    # assets 模板同样要校验：模板里 class/font/颜色引用错会导致生成的窗口静默没样式
    for pat in ("*.xml", "*.h", "*.cpp"):
        targets += sorted((skill / "assets").rglob(pat))

    # 额外目标：用户自己写出的应用源码与布局
    extra_missing_paths = []
    for raw in args.extra:
        p = Path(raw)
        if not p.exists():
            extra_missing_paths.append(raw)
            continue
        if p.is_dir():
            for pat in ("*.md", "*.xml", "*.h", "*.cpp"):
                targets += sorted(p.rglob(pat))
        else:
            targets.append(p)
    for raw in extra_missing_paths:
        print(f"警告：--extra 路径不存在，已跳过：{raw}")

    if not targets:
        print(f"错误：{skill} 下没有找到 references/*.md")
        return 2

    idx = build_index(repo)
    fallback = probe_paths(repo)

    total_missing = 0
    per_file = {}
    for md in targets:
        miss = scan_file(md, idx, fallback)
        if miss:
            per_file[md] = miss
            total_missing += len(miss)

    if not args.quiet:
        for md, miss in per_file.items():
            print(f"\n{md.name}:")
            for lineno, name, kind in miss:
                print(f"  行 {lineno:<5} [{kind}]  {name}")

    print("\n" + "=" * 60)
    print(f"扫描文档 {len(targets)} 个，源码索引："
          f"{len(idx['DUI_CTR 宏'])} 个宏 / {len(idx['事件枚举'])} 个事件 / "
          f"{len(idx['C++ 类名'])} 个类名 / {len(idx['字体 ID'])} 个字体 / "
          f"{len(idx['语义色名'])} 个色名")
    if total_missing == 0:
        print("结果：全部命中，未发现文档漂移。")
        return 0
    print(f"结果：发现 {total_missing} 处疑似漂移（文档引用了源码中不存在的名字）。")
    print("提示：若某处是刻意举例（如讲解'不存在'的旧名），在该文件加注释：")
    print("      <!-- verify:allow-missing 名字1 名字2 -->")
    return 1


if __name__ == "__main__":
    sys.exit(main())
