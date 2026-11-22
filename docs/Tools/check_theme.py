#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""nim_duilib 主题资源校验工具。

与 lint_xml.py（布局 XML 静态检查）互补：本工具关注主题体系自身的一致性，
以及 svg_replace_colors 与 SVG 文件内容的匹配关系。

校验规则：
T1 主题文件可解析：default/global.xml、color_dark/global.xml、color_light/global.xml。
T2 颜色矩阵一致：color_dark 与 color_light 的 ThemeColor 名称集合必须完全相同，
   同一文件内颜色名不得重复。
T3 ThemeColor 字段合法：
   - value 必须是 #RRGGBB 或 #AARRGGBB；
   - derived_from 引用的颜色必须已定义；
   - contrast_bg 引用的颜色必须已定义（警告，该属性仅用于对比度评估）；
   - adjust 语法合法：lightness/saturation/hue/alpha/invert，运算符支持 + - * =（警告）。
T4 Alias 的目标可解析为颜色名、Class 名或标准 CSS 色名（警告）。
T5 svg_replace_colors 校验：
   - 每个条目格式为 “源串|目标色名”，多组用分号分隔；
   - 源串必须在对应 SVG 文件内容中按字符串存在（错误）。运行期使用区分大小写的
     字符串替换，源串不存在时替换会静默失效，图标颜色不会跟随主题变化；
   - 目标色名必须能解析为颜色（警告）。解析失败时运行期会退化为纯文本替换；
   - file='...' 指向的文件必须存在（错误），且必须是 SVG 文件（否则警告）；
   - ${VAR} 必须能展开为具体值（错误）。

退出码：0 表示没有错误（允许警告），1 表示存在错误。

用法示例：
    # 检查仓库内的全部主题资源（默认行为）
    python docs/Tools/check_theme.py

    # 指定仓库根目录
    python docs/Tools/check_theme.py --repo .
"""

import argparse
import re
import sys
from pathlib import Path

# 复用 lint_xml.py 中的 XML 解析（带行列号）、变量展开与标准色收集
sys.path.insert(0, str(Path(__file__).resolve().parent))
import lint_xml  # noqa: E402

# 脚本位于 <repo>/docs/Tools/check_theme.py
DEFAULT_REPO = Path(__file__).resolve().parent.parent.parent

# 需要检查的主题文件（相对 bin/resources/themes）
THEME_FILES = ("default", "color_dark", "color_light")
# 颜色主题：两者的颜色矩阵必须完全一致
COLOR_THEMES = ("color_dark", "color_light")

# 合法的十六进制颜色（与 ColorMap::HexToColor 一致）
HEX_COLOR_RE = re.compile(r"^#[0-9a-fA-F]{6}$|^#[0-9a-fA-F]{8}$")
# ThemeGenerator::ApplyAdjustments 支持的属性与运算符
ADJUST_PROPERTIES = frozenset({"lightness", "saturation", "hue", "alpha", "invert"})
ADJUST_VALUE_RE = re.compile(r"^[+\-*=]?\s*-?\d+(?:\.\d+)?$")

# 图片属性值内部的 svg_replace_colors='...' 与 file='...'
SVG_MAP_RE = re.compile(r"svg_replace_colors\s*=\s*(['\"])(.*?)\1", re.I | re.S)
FILE_RE = re.compile(r"file\s*=\s*(['\"])(.*?)\1", re.I | re.S)


class ThemeChecker(object):
    """主题资源检查器"""

    def __init__(self, repo):
        self.repo = repo
        self.themes_root = repo / "bin" / "resources" / "themes"
        self.theme_dirs = sorted(p for p in self.themes_root.iterdir() if p.is_dir()) \
            if self.themes_root.is_dir() else []

        self.issues = []
        # 主题级的定义汇总
        self.theme_paths = {}       # 主题名 -> global.xml 路径
        self.theme_colors = {}      # 主题名 -> {颜色名: 节点}
        self.semantic_colors = set()  # 全部 ThemeColor 名称
        self.text_colors = set()      # 全部 TextColor 名称
        self.aliases = {}             # Alias name -> value
        self.classes = set()          # Class name
        self.global_vars = lint_xml.collect_global_vars(repo)
        self.standard_colors = lint_xml.collect_standard_colors(repo)

        # 统计信息
        self.xml_count = 0
        self.mapping_count = 0

    # ------------------------------------------------------------------ 输出

    def add_issue(self, level, path, line, column, message):
        self.issues.append(lint_xml.Issue(level, self.display_path(path), line, max(1, column), message))

    def display_path(self, path):
        try:
            return path.resolve().relative_to(self.repo).as_posix()
        except (ValueError, OSError):
            return path.as_posix()

    # ------------------------------------------------------------------ 公共入口

    def run(self):
        self.load_theme_files()
        self.check_color_matrix()
        self.check_theme_color_fields()
        self.check_aliases()
        self.check_svg_replace_colors()

    # ------------------------------------------------------------------ T1 主题文件

    def load_theme_files(self):
        for theme in THEME_FILES:
            path = (self.themes_root / theme / "global.xml").resolve()
            if not path.is_file():
                self.add_issue("error", path, 1, 1, "主题文件不存在：%s" % path)
                continue
            root, error = lint_xml.parse_xml(path)
            if error is not None:
                self.add_issue("error", path, getattr(error, "lineno", 1),
                               getattr(error, "offset", 0) + 1,
                               "主题文件解析失败：%s" % error)
                continue
            if root is None:
                self.add_issue("error", path, 1, 1, "主题文件内容为空")
                continue

            self.theme_paths[theme] = path
            colors = self.theme_colors.setdefault(theme, {})
            for node in root.iter():
                if node.name == "ThemeColor":
                    name = (node.get("name") or "").strip()
                    if name:
                        if name in colors:
                            self.add_issue("error", path, node.line, node.column,
                                           "颜色名 %s 重复定义（首次出现在第 %d 行）"
                                           % (name, colors[name].line))
                        else:
                            colors[name] = node
                        self.semantic_colors.add(name)
                elif node.name == "TextColor":
                    name = (node.get("name") or "").strip()
                    if name:
                        self.text_colors.add(name)
                elif node.name == "Alias":
                    name = (node.get("name") or "").strip()
                    value = (node.get("value") or "").strip()
                    if name and value:
                        self.aliases.setdefault(name, value)
                elif node.name == "Class":
                    name = (node.get("name") or "").strip()
                    if name:
                        self.classes.add(name)
                elif node.name == "Var":
                    name = (node.get("name") or "").strip()
                    value = (node.get("value") or "").strip()
                    if name and value:
                        self.global_vars.setdefault(name, value)

    # ------------------------------------------------------------------ T2 颜色矩阵

    def check_color_matrix(self):
        if len(COLOR_THEMES) != 2:
            return
        first, second = COLOR_THEMES
        first_colors = self.theme_colors.get(first, {})
        second_colors = self.theme_colors.get(second, {})
        if not first_colors or not second_colors:
            return

        for name in sorted(set(first_colors) - set(second_colors)):
            self.add_issue("error", self.theme_paths.get(second, self.themes_root), 1, 1,
                           "颜色 %s 只在 %s 中定义（第 %d 行），%s 缺少该颜色"
                           % (name, first, first_colors[name].line, second))
        for name in sorted(set(second_colors) - set(first_colors)):
            self.add_issue("error", self.theme_paths.get(first, self.themes_root), 1, 1,
                           "颜色 %s 只在 %s 中定义（第 %d 行），%s 缺少该颜色"
                           % (name, second, second_colors[name].line, first))

    # ------------------------------------------------------------------ T3 ThemeColor 字段

    def check_theme_color_fields(self):
        for theme in THEME_FILES:
            path = self.theme_paths.get(theme)
            if path is None:
                continue
            for name, node in self.theme_colors.get(theme, {}).items():
                value = (node.get("value") or "").strip()
                if not value:
                    self.add_issue("error", path, node.line, node.column,
                                   "颜色 %s 缺少 value 属性" % name)
                elif not HEX_COLOR_RE.match(value):
                    self.add_issue("error", path, node.line, node.column,
                                   "颜色 %s 的 value=%s 不是合法的十六进制颜色（只支持 #RRGGBB 或 #AARRGGBB）"
                                   % (name, value))

                derived_from = (node.get("derived_from") or "").strip()
                if derived_from and derived_from not in self.semantic_colors:
                    self.add_issue("error", path, node.line, node.column,
                                   "颜色 %s 的 derived_from=%s 未定义" % (name, derived_from))

                for contrast in self.split_names(node.get("contrast_bg")):
                    if not self.is_color(contrast):
                        self.add_issue("warning", path, node.line, node.column,
                                       "颜色 %s 的 contrast_bg 引用了未定义的颜色 %s"
                                       % (name, contrast))

                for adjustment in self.split_adjust(node.get("adjust")):
                    self.check_adjust_item(path, node, name, adjustment)

    @staticmethod
    def split_names(value):
        if not value:
            return []
        return [item.strip() for item in re.split(r"[,;]", value) if item.strip()]

    @staticmethod
    def split_adjust(value):
        if not value:
            return []
        return [item.strip() for item in value.split(",") if item.strip()]

    def check_adjust_item(self, path, node, color_name, adjustment):
        if ":" in adjustment:
            prop, raw_value = adjustment.split(":", 1)
            prop = prop.strip()
            raw_value = raw_value.strip()
        else:
            prop, raw_value = adjustment, ""

        if prop not in ADJUST_PROPERTIES:
            self.add_issue("warning", path, node.line, node.column,
                           "颜色 %s 的 adjust 使用了未知属性 %s（运行期会被忽略）" % (color_name, prop))
            return
        if prop == "invert":
            if raw_value:
                self.add_issue("warning", path, node.line, node.column,
                               "颜色 %s 的 adjust 中 invert 不接受数值，当前写法：%s"
                               % (color_name, adjustment))
            return
        if not raw_value or not ADJUST_VALUE_RE.match(raw_value):
            self.add_issue("warning", path, node.line, node.column,
                           "颜色 %s 的 adjust 数值不合法：%s（运行期会被忽略）"
                           % (color_name, adjustment))

    # ------------------------------------------------------------------ T4 Alias

    def check_aliases(self):
        path = self.theme_paths.get("default", self.themes_root)
        if not self.aliases:
            return
        for name, value in sorted(self.aliases.items()):
            if value.startswith("#"):
                if not HEX_COLOR_RE.match(value):
                    self.add_issue("warning", path, 1, 1,
                                   "Alias %s 的 value=%s 不是合法的十六进制颜色" % (name, value))
            elif value in self.aliases:
                # 运行期只做一次映射，不解析别名链
                self.add_issue("warning", path, 1, 1,
                               "Alias %s 指向了另一个别名 %s，运行期不会继续解析（只做一次映射）"
                               % (name, value))
            elif (value not in self.semantic_colors) and (value not in self.text_colors) and \
                    (value not in self.classes) and (value.lower() not in self.standard_colors):
                self.add_issue("warning", path, 1, 1,
                               "Alias %s 的目标 %s 无法解析为颜色名、Class 名或标准色名" % (name, value))

    # ------------------------------------------------------------------ T5 svg_replace_colors

    def check_svg_replace_colors(self):
        if not self.themes_root.is_dir():
            return
        for xml_path in sorted(self.themes_root.rglob("*.xml")):
            path = xml_path.resolve()
            root, error = lint_xml.parse_xml(path)
            if (error is not None) or (root is None):
                # 语法错误由 lint_xml.py 负责报告，这里不重复报告
                continue
            self.xml_count += 1

            local_vars = dict(self.global_vars)
            for node in root.iter():
                if node.name == "Var":
                    name = (node.get("name") or "").strip()
                    value = (node.get("value") or "").strip()
                    if name and value:
                        local_vars[name] = value
                elif node.name == "Class":
                    name = (node.get("name") or "").strip()
                    if name:
                        self.classes.add(name)

            search_dirs = self.search_dirs(path)
            for node in root.iter():
                for attr_name, attr_value in node.attrs:
                    if attr_name == "svg_replace_colors":
                        self.add_issue("warning", path, node.line, node.column,
                                       "svg_replace_colors 必须写在图片属性值内部（如 normal_image=\"file='...' "
                                       "svg_replace_colors='...'\"），作为独立属性不会被解析")
                        continue
                    if "svg_replace_colors" not in attr_value.lower():
                        continue
                    self.check_svg_mapping(path, node, attr_name, attr_value, search_dirs, local_vars)

    def check_svg_mapping(self, path, node, attr_name, attr_value, search_dirs, local_vars):
        for match in SVG_MAP_RE.finditer(attr_value):
            mapping = match.group(2).strip()
            if not mapping:
                continue

            mapping = lint_xml.expand_vars(mapping, local_vars)
            if lint_xml.VAR_RE.search(mapping):
                self.add_issue("error", path, node.line, node.column,
                               "%s 中的 svg_replace_colors 含有无法展开的变量：%s" % (attr_name, mapping))
                continue

            file_match = FILE_RE.search(attr_value)
            if file_match is None:
                self.add_issue("warning", path, node.line, node.column,
                               "%s 配置了 svg_replace_colors，但无法定位对应的图片文件（缺少 file='...'）"
                               % attr_name)
                continue
            image_path = lint_xml.expand_vars(file_match.group(2).strip(), local_vars)
            if lint_xml.VAR_RE.search(image_path):
                self.add_issue("error", path, node.line, node.column,
                               "%s 的 file 路径含有无法展开的变量：%s" % (attr_name, image_path))
                continue
            if image_path.startswith("$"):
                self.add_issue("warning", path, node.line, node.column,
                               "%s 引用了运行期动态注册的图片（%s），无法静态校验 svg_replace_colors"
                               % (attr_name, image_path))
                continue

            svg_file = self.resolve_resource(path, image_path, search_dirs)
            if svg_file is None:
                self.add_issue("error", path, node.line, node.column,
                               "%s 引用的图片不存在，无法校验 svg_replace_colors：%s"
                               % (attr_name, image_path))
                continue
            if svg_file.suffix.lower() != ".svg":
                self.add_issue("warning", path, node.line, node.column,
                               "%s 引用的文件不是 SVG（%s），svg_replace_colors 不会生效"
                               % (attr_name, image_path))
                continue

            try:
                svg_data = svg_file.read_bytes()
            except OSError as error:
                self.add_issue("error", path, node.line, node.column,
                               "读取 SVG 文件失败：%s（%s）" % (svg_file, error))
                continue
            self.check_svg_pairs(path, node, attr_name, mapping, svg_file, svg_data)

    def check_svg_pairs(self, path, node, attr_name, mapping, svg_file, svg_data):
        seen_sources = set()
        for pair in mapping.split(";"):
            pair = pair.strip()
            if not pair:
                continue
            self.mapping_count += 1
            parts = [part.strip() for part in pair.split("|")]
            if (len(parts) != 2) or (not parts[0]) or (not parts[1]):
                self.add_issue("error", path, node.line, node.column,
                               "%s 的 svg_replace_colors 条目格式错误：%s（应为“源串|目标色名”）"
                               % (attr_name, pair))
                continue

            source, target = parts
            if source in seen_sources:
                self.add_issue("warning", path, node.line, node.column,
                               "%s 的 svg_replace_colors 中源串 %s 重复配置" % (attr_name, source))
            seen_sources.add(source)

            source_bytes = source.encode("utf-8")
            if source_bytes not in svg_data:
                hint = ""
                if source_bytes.lower() in svg_data.lower():
                    hint = "（SVG 中存在大小写不同的写法；运行期按大小写敏感替换，同样不会生效）"
                self.add_issue("error", path, node.line, node.column,
                               "%s 的 svg_replace_colors 源色 %s 在 %s 中不存在%s"
                               % (attr_name, source, self.display_path(svg_file), hint))

            if not self.is_color(target):
                self.add_issue("warning", path, node.line, node.column,
                               "%s 的 svg_replace_colors 目标色名 %s 无法解析为颜色（运行期会按纯文本替换处理）"
                               % (attr_name, target))

    # ------------------------------------------------------------------ 工具函数

    def is_color(self, name):
        """判断名称能否解析为颜色（与 Control::PrivateGetUiColor 的解析顺序一致）"""
        if not name:
            return False
        if name.startswith("#"):
            return bool(HEX_COLOR_RE.match(name))
        if (name in self.semantic_colors) or (name in self.text_colors):
            return True
        if name.lower() in self.standard_colors:
            return True
        alias_value = self.aliases.get(name)
        if alias_value:
            if alias_value.startswith("#"):
                return bool(HEX_COLOR_RE.match(alias_value))
            return (alias_value in self.semantic_colors) or (alias_value.lower() in self.standard_colors)
        return False

    def search_dirs(self, xml_path):
        """返回查找图片相对路径时使用的候选目录（与 lint_xml.py 的查找顺序一致）"""
        dirs = [xml_path.parent]
        window_res_path = ""
        for theme_dir in self.theme_dirs:
            try:
                relative = xml_path.resolve().relative_to(theme_dir.resolve())
                if len(relative.parts) > 1:
                    window_res_path = relative.parts[0]
            except (ValueError, OSError):
                continue
        for theme_dir in self.theme_dirs:
            if window_res_path:
                dirs.append(theme_dir / window_res_path)
            dirs.append(theme_dir)
        unique_dirs = []
        for directory in dirs:
            if directory not in unique_dirs:
                unique_dirs.append(directory)
        return unique_dirs

    def resolve_resource(self, xml_path, resource_path, search_dirs):
        """把资源相对路径解析为存在的磁盘路径，找不到返回 None"""
        if resource_path.startswith("$"):
            return None
        if resource_path.startswith("/") or resource_path.startswith("\\") or \
                re.match(r"^[A-Za-z]:", resource_path):
            candidate = Path(resource_path)
            return candidate if candidate.is_file() else None
        normalized = resource_path.replace("\\", "/")
        for directory in search_dirs:
            candidate = directory / normalized
            if candidate.is_file():
                return candidate
        return None


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="nim_duilib 主题资源校验工具（颜色矩阵/ThemeColor 字段/Alias/svg_replace_colors）",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__.split("校验规则：", 1)[0].strip())
    parser.add_argument("--repo", default=str(DEFAULT_REPO), help="nim_duilib 仓库根目录（默认自动识别）")
    args = parser.parse_args(argv)

    repo = Path(args.repo).resolve()
    themes_root = repo / "bin" / "resources" / "themes"
    if not themes_root.is_dir():
        print("错误：找不到主题资源目录：%s" % themes_root, file=sys.stderr)
        return 2

    checker = ThemeChecker(repo)
    checker.run()

    for issue in sorted(checker.issues, key=lambda item: (item.path, item.line, item.column)):
        print(issue.format())

    errors = sum(1 for issue in checker.issues if issue.level == "error")
    warnings = sum(1 for issue in checker.issues if issue.level == "warning")
    print()
    print("检查完成：主题文件 %d 个，XML 文件 %d 个，svg_replace_colors 条目 %d 个，"
          "错误 %d 个，警告 %d 个"
          % (len(checker.theme_paths), checker.xml_count, checker.mapping_count, errors, warnings))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
