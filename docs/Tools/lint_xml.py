#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""nim_duilib 布局 XML 静态检查工具（lint）。

手写 XML 时，很多错误只有在运行期才会暴露（Debug 版触发断言，Release 版静默失效），
例如节点名拼错、class 属性位置不对、颜色/字体/多语言 ID 不存在、Include 引用的文件不存在等。
本脚本在编辑阶段就可以把这些错误找出来，并给出 文件:行:列 的位置信息。

检查规则：
1. XML 语法：解析失败时给出 文件:行:列 和失败原因；
2. 未知节点名：对照 duilib/duilib_defs.h 里的 DUI_CTR_* 宏（自定义控件可用 --allow-node 加入白名单）；
3. class 属性必须写在所有属性的最前面（否则 class 里的同名属性会覆盖显式属性）；
4. 同一个 XML 文件内 name 属性的值不能重复（FindControl 依赖 name 唯一）；
5. text_id 是否存在：对照 bin/resources/lang/*.txt 的全部字符串 ID；
6. font 是否存在：对照各主题 global.xml 里的 <Font id="...">；
7. 颜色值是否合法：语义色名、Alias 别名、标准 CSS 色名，或者 #RRGGBB / #AARRGGBB；
8. <Include> 节点的 src/source 引用的文件是否存在，count 是否为正整数；
9. 图片文件是否存在：属性名以 _image 结尾（含 bkimage/image），支持 file='...' 语法与 ${VAR} 变量。

退出码：0 表示没有错误（仅警告也返回 0），1 表示存在错误；加 --strict 时警告也算失败。

用法示例：
    # 检查整个主题资源目录（默认行为）
    python docs/Tools/lint_xml.py

    # 检查指定文件或目录
    python docs/Tools/lint_xml.py bin/resources/themes/default/layout/layout.xml
    python docs/Tools/lint_xml.py bin/resources/themes/default/layout

    # 把警告也当作失败（适合接入 CI），或者跳过图片检查
    python docs/Tools/lint_xml.py --strict
    python docs/Tools/lint_xml.py --no-image

    # 指定仓库根目录（默认自动识别脚本所在的仓库）
    python docs/Tools/lint_xml.py --repo .

    # 允许自定义控件节点名（避免“未知的节点名”警告，可重复指定）
    python docs/Tools/lint_xml.py --allow-node RenderTest1 --allow-node ColorPreviewLabel
"""

import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from xml.parsers import expat

# 脚本位于 <repo>/docs/Tools/lint_xml.py
DEFAULT_REPO = Path(__file__).resolve().parent.parent.parent

# 默认检查的目录（相对仓库根目录）
DEFAULT_PATHS = ("bin/resources/themes",)

# 额外的 XML 节点名（不是控件，由 WindowBuilder 单独解析或直接忽略）
SPECIAL_NODE_NAMES = frozenset({
    "Window", "Global", "Include",
    # IsIgnoreNodeName 中的节点
    "DefaultFontFamilyNames", "FallbackFontFamilyNames",
    "Font", "FontFile", "FontResource",
    "Class", "TextColor", "ThemeColor", "ThemeMeta", "Theme", "Alias", "Var",
    # ParseXmlNodeChildren 中单独处理的节点
    "Event", "BubbledEvent",
    "PropertyGridGroup", "CheckComboText",
    "ListCtrlHeaderItem", "ListCtrlItem", "ListCtrlSubItem",
    "PropertyGridTextProperty", "PropertyGridComboProperty",
    "PropertyGridFontProperty", "PropertyGridFontSizeProperty",
    "PropertyGridColorProperty", "PropertyGridDateTimeProperty",
    "PropertyGridIPAddressProperty", "PropertyGridHotKeyProperty",
    "PropertyGridFileProperty", "PropertyGridDirectoryProperty",
    # ThemeMeta 下的元信息节点
    "Property",
})

# 颜色属性名：bkcolor / bkcolor2 / fgcolor / bgcolor / xxx_color
COLOR_ATTR_RE = re.compile(r"^(?:[A-Za-z0-9]+_)*(?:bkcolor2?|fgcolor|bgcolor|color)$")

# 图片属性名（其余含 image 的属性，如 expand_image_class，是类名而不是图片路径）
IMAGE_ATTR_NAMES = frozenset({"image", "bkimage"})
IMAGE_EXT_RE = re.compile(r"\.(png|jpg|jpeg|gif|bmp|ico|svg|webp|json|pag|lottie)$", re.I)

# ${VAR} 变量
VAR_RE = re.compile(r"\$\{([A-Za-z0-9_]+)\}")
# 图片属性值中的 file='...'
FILE_IN_IMAGE_RE = re.compile(r"file\s*=\s*(['\"])(.*?)\1", re.I)

# 合法的十六进制颜色：ColorMap::HexToColor 只接受 #RRGGBB 和 #AARRGGBB
HEX_COLOR_RE = re.compile(r"^#[0-9a-fA-F]{6}$|^#[0-9a-fA-F]{8}$")

# 看起来像字符串 ID 的 text_id 值（其余值可能是运行期生成的字面量文本）
TEXT_ID_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_.]*$")

# 主题 global.xml 中需要忽略的元信息 ID（不是 text_id）
META_TEXT_IDS = frozenset({"INCLUDE_LANGUAGE_FILES"})


class Node(object):
    """XML 元素（保留属性顺序、行列号，便于定位 class 位置问题）"""

    __slots__ = ("name", "attrs", "line", "column", "children", "parent")

    def __init__(self, name, attrs, line, column, parent=None):
        self.name = name
        self.attrs = attrs          # list[(name, value)]，按 XML 文档中的顺序
        self.line = line
        self.column = column
        self.children = []
        self.parent = parent

    def get(self, attr_name, default=None):
        for name, value in self.attrs:
            if name == attr_name:
                return value
        return default

    def iter(self):
        yield self
        for child in self.children:
            for node in child.iter():
                yield node


class Issue(object):
    __slots__ = ("level", "path", "line", "column", "message")

    def __init__(self, level, path, line, column, message):
        self.level = level
        self.path = path
        self.line = line
        self.column = column
        self.message = message

    def format(self):
        return "%s:%d:%d: [%s] %s" % (self.path, self.line, self.column, self.level, self.message)


def parse_xml(path):
    """解析 XML 文件，返回 (根节点, ExpatError 或 None)。

    使用 expat 是为了拿到元素的行号、列号和属性的原始顺序（ElementTree 会丢失属性顺序）。
    """
    try:
        data = path.read_bytes()
    except OSError as error:
        return None, error

    parser = expat.ParserCreate()
    parser.ordered_attributes = True
    stack = []
    roots = []

    def start_element_handler(name, attrs):
        pairs = [(attrs[i], attrs[i + 1]) for i in range(0, len(attrs), 2)]
        parent = stack[-1] if stack else None
        node = Node(name, pairs, parser.CurrentLineNumber, parser.CurrentColumnNumber + 1, parent)
        if parent is not None:
            parent.children.append(node)
        else:
            roots.append(node)
        stack.append(node)

    def end_element_handler(_name):
        if stack:
            stack.pop()

    parser.StartElementHandler = start_element_handler
    parser.EndElementHandler = end_element_handler
    try:
        parser.Parse(data, True)
    except expat.ExpatError as error:
        return None, error
    return (roots[0] if roots else None), None


def read_text(path):
    for encoding in ("utf-8-sig", "utf-16", "gbk"):
        try:
            return path.read_text(encoding=encoding)
        except (UnicodeError, OSError):
            continue
    return path.read_text(encoding="utf-8", errors="replace")


def collect_node_names(repo):
    """从 duilib/duilib_defs.h 的 DUI_CTR_* 宏收集合法的节点名。

    其中有 8 个小写宏（window_title_bar、btn_window_close 等）是控件 name 的取值而不是节点名，
    需要过滤掉。
    """
    defs_file = repo / "duilib" / "duilib_defs.h"
    names = set()
    if defs_file.is_file():
        text = read_text(defs_file)
        for value in re.findall(r"#define\s+DUI_CTR_[A-Z0-9_]+\s+\(_T\(\"([^\"]+)\"\)\)", text):
            if value[:1].isupper():
                names.add(value)
    return names


def collect_theme_globals(repo):
    """收集各主题的 global.xml 文件"""
    themes_dir = repo / "bin" / "resources" / "themes"
    if not themes_dir.is_dir():
        return []
    return sorted(p for p in themes_dir.glob("*/global.xml") if p.is_file())


def collect_colors(repo):
    """收集语义色名（ThemeColor/TextColor 的 name）与 Alias 别名（新旧名称都算合法）"""
    names = set()
    for global_xml in collect_theme_globals(repo):
        try:
            root = ET.parse(str(global_xml)).getroot()
        except ET.ParseError:
            continue
        for child in root:
            tag = child.tag
            if tag in ("ThemeColor", "TextColor"):
                name = (child.get("name") or "").strip()
                if name:
                    names.add(name)
            elif tag == "Alias":
                alias_name = (child.get("name") or "").strip()
                alias_value = (child.get("value") or "").strip()
                if alias_name:
                    names.add(alias_name)
                if alias_value:
                    names.add(alias_value)
    return names


def collect_standard_colors(repo):
    """收集标准 CSS 色名（大小写不敏感，与 StandardColorMap 一致）"""
    colors_file = repo / "duilib" / "Core" / "UiColors.cpp"
    names = set()
    if colors_file.is_file():
        text = read_text(colors_file)
        for name in re.findall(r"\{\s*_T\(\"([A-Za-z0-9]+)\"\)\s*,", text):
            names.add(name.lower())
    return names


def collect_fonts(repo):
    """收集全局字体 ID（各主题 global.xml 的 <Font id="...">）"""
    fonts = set()
    for global_xml in collect_theme_globals(repo):
        try:
            root = ET.parse(str(global_xml)).getroot()
        except ET.ParseError:
            continue
        for child in root.iter("Font"):
            font_id = (child.get("id") or "").strip()
            if font_id:
                fonts.add(font_id)
    return fonts


def collect_text_ids(repo):
    """收集语言文件中的全部字符串 ID"""
    lang_dir = repo / "bin" / "resources" / "lang"
    text_ids = set()
    if not lang_dir.is_dir():
        return text_ids
    for lang_file in lang_dir.glob("*.txt"):
        text = read_text(lang_file)
        for line in text.splitlines():
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key = line.split("=", 1)[0].strip()
            if key and key not in META_TEXT_IDS:
                text_ids.add(key)
    return text_ids


def collect_global_vars(repo):
    """收集各主题 global.xml 中 <Var name value> 定义的变量（用于展开 ${VAR}）"""
    values = {}
    for global_xml in collect_theme_globals(repo):
        try:
            root = ET.parse(str(global_xml)).getroot()
        except ET.ParseError:
            continue
        for child in root.iter("Var"):
            name = (child.get("name") or "").strip()
            value = (child.get("value") or "").strip()
            if name and value:
                values.setdefault(name, value)
    return values


def is_rich_text_markup(node):
    """判断节点是否是 RichText 的内容标记（<b>、<font>、<br> 等，不是控件节点）"""
    parent = node.parent
    while parent is not None:
        if parent.name == "RichText":
            return True
        parent = parent.parent
    return False


def expand_vars(value, variables):
    """反复展开 ${VAR}，直到无法继续展开"""
    for _ in range(5):
        new_value = VAR_RE.sub(lambda match: variables.get(match.group(1), match.group(0)), value)
        if new_value == value:
            break
        value = new_value
    return value


def split_image_items(value):
    """按不在引号内的 '|' 切分图片属性值（多帧图片用 '|' 连接多组参数）"""
    items = []
    current = []
    quote = None
    for ch in value:
        if quote:
            current.append(ch)
            if ch == quote:
                quote = None
        elif ch in ("'", '"'):
            quote = ch
            current.append(ch)
        elif ch == "|":
            items.append("".join(current))
            current = []
        else:
            current.append(ch)
    items.append("".join(current))
    return [item.strip() for item in items if item.strip()]


def extract_image_paths(value):
    """从图片属性值中提取图片文件路径。

    支持两种写法：
        normal_image="user_1.png"
        normal_image="file='${PATH_BUTTON}/icon.svg' width='16' ..."
    """
    paths = []
    for item in split_image_items(value):
        match = FILE_IN_IMAGE_RE.search(item)
        if match:
            path = match.group(2).strip()
            if path:
                paths.append(path)
            continue
        if ("=" not in item) and not any(ch.isspace() for ch in item):
            paths.append(item)
    return paths


class Linter(object):
    """XML 布局检查器"""

    def __init__(self, repo, check_images=True, check_text_ids=True, check_colors=True, allow_nodes=()):
        self.repo = repo
        self.check_images = check_images
        self.check_text_ids = check_text_ids
        self.check_colors = check_colors
        # 用户指定的自定义控件节点名（不当作未知节点）
        self.allow_nodes = set(allow_nodes)

        self.node_names = collect_node_names(repo) | SPECIAL_NODE_NAMES
        self.control_node_names = collect_node_names(repo)
        self.semantic_colors = collect_colors(repo)
        self.standard_colors = collect_standard_colors(repo)
        self.fonts = collect_fonts(repo)
        self.text_ids = collect_text_ids(repo)
        self.global_vars = collect_global_vars(repo)

        themes_dir = repo / "bin" / "resources" / "themes"
        self.theme_dirs = sorted(p for p in themes_dir.iterdir() if p.is_dir()) if themes_dir.is_dir() else []

        self.issues = []
        self.visited = set()

    # ------------------------------------------------------------------ 输出

    def add_issue(self, level, path, line, column, message):
        self.issues.append(Issue(level, self.display_path(path), line, max(1, column), message))

    def display_path(self, path):
        try:
            return path.resolve().relative_to(self.repo).as_posix()
        except (ValueError, OSError):
            return path.as_posix()

    # ------------------------------------------------------------------ 资源搜索

    def search_dirs(self, xml_path):
        """返回查找图片/Include 相对路径时使用的候选目录。

        与运行期的资源查找顺序接近：先按窗口皮肤目录（XML 所在目录），再按各主题根目录。
        """
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
        # 去重
        unique_dirs = []
        for directory in dirs:
            if directory not in unique_dirs:
                unique_dirs.append(directory)
        return unique_dirs

    def resolve_resource(self, xml_path, resource_path, search_dirs):
        """把资源相对路径解析为存在的磁盘路径，找不到返回 None"""
        if resource_path.startswith("$"):
            return None
        if resource_path.startswith("/") or resource_path.startswith("\\"):
            candidate = Path(resource_path)
            return candidate if candidate.is_file() else None
        if re.match(r"^[A-Za-z]:", resource_path):
            candidate = Path(resource_path)
            return candidate if candidate.is_file() else None
        normalized = resource_path.replace("\\", "/")
        for directory in search_dirs:
            candidate = directory / normalized
            if candidate.is_file():
                return candidate
        return None

    # ------------------------------------------------------------------ 文件级检查

    def lint_file(self, path):
        path = path.resolve()
        if path in self.visited:
            return
        self.visited.add(path)

        if path.suffix.lower() != ".xml":
            return
        root, error = parse_xml(path)
        if error is not None:
            if isinstance(error, expat.ExpatError):
                self.add_issue("error", path, error.lineno, error.offset + 1,
                               "XML 语法错误：%s" % expat.ErrorString(error.code))
            else:
                self.add_issue("error", path, 1, 1, "读取文件失败：%s" % error)
            return
        if root is None:
            self.add_issue("error", path, 1, 1, "XML 内容为空")
            return

        local = self.collect_local_definitions(root)
        self.check_tree(path, root, local)
        self.visited.add(path)

    def collect_local_definitions(self, root):
        """收集当前文件内定义的变量、字体、语义色（窗口内定义优先于全局定义）"""
        variables = dict(self.global_vars)
        fonts = set(self.fonts)
        colors = set(self.semantic_colors)
        for node in root.iter():
            if node.name == "Var":
                name = (node.get("name") or "").strip()
                value = (node.get("value") or "").strip()
                if name and value:
                    variables[name] = value
            elif node.name == "Font":
                font_id = (node.get("id") or "").strip()
                if font_id:
                    fonts.add(font_id)
            elif node.name in ("ThemeColor", "TextColor"):
                name = (node.get("name") or "").strip()
                if name:
                    colors.add(name)
        return {"vars": variables, "fonts": fonts, "colors": colors}

    def check_tree(self, path, root, local):
        search_dirs = self.search_dirs(path)
        name_map = {}
        for node in root.iter():
            is_markup = is_rich_text_markup(node)

            # 1. 节点名（RichText 的子节点是富文本标记，不是控件）
            if (not is_markup) and (node.parent is not None) and \
                    (node.name not in self.node_names) and (node.name not in self.allow_nodes):
                self.add_issue("warning", path, node.line, node.column,
                               "未知的节点名 <%s>（对照 duilib_defs.h 的 DUI_CTR_* 宏；自定义控件可用 --allow-node 加入白名单）" % node.name)

            # 2. class 必须在所有属性最前面
            attr_names = [name for name, _value in node.attrs]
            if (not is_markup) and (node.name in self.control_node_names) and \
                    ("class" in attr_names) and (attr_names.index("class") != 0):
                self.add_issue("error", path, node.line, node.column,
                               "属性 class 必须写在所有属性的最前面，否则 class 中的同名属性会覆盖显式属性")

            # 3. name 不能重复（只检查控件节点：Class/Font 等节点的 name 不是控件名）
            node_name = node.get("name")
            if node_name and (node.name in self.control_node_names) and not is_markup:
                node_name = node_name.strip()
                if node_name:
                    if node_name in name_map:
                        self.add_issue("warning", path, node.line, node.column,
                                       "name=\"%s\" 重复（首次出现于第 %d 行），FindControl 只能取到其中一个"
                                       % (node_name, name_map[node_name]))
                    else:
                        name_map[node_name] = node.line

            # 4. Include 节点
            if node.name == "Include":
                self.check_include(path, node, search_dirs, local)

            # 5. 属性级检查
            for attr_name, attr_value in node.attrs:
                value = attr_value.strip()
                if not value:
                    continue
                if self.check_text_ids and (attr_name.endswith("text_id") or attr_name.endswith("textid")):
                    self.check_text_id(path, node, attr_name, value)
                elif attr_name.endswith("font") or attr_name.endswith("font_id") or attr_name.endswith("fontid"):
                    self.check_font(path, node, attr_name, value, local)
                elif self.check_colors and COLOR_ATTR_RE.match(attr_name):
                    self.check_color(path, node, attr_name, value, local)
                elif self.check_images and ((attr_name in IMAGE_ATTR_NAMES) or attr_name.endswith("_image")):
                    self.check_image(path, node, attr_name, value, search_dirs, local)

    def check_text_id(self, path, node, attr_name, value):
        if not TEXT_ID_RE.match(value):
            # 值不像是字符串 ID（可能是直接写的文本），不做检查
            return
        if value not in self.text_ids:
            self.add_issue("warning", path, node.line, node.column,
                           "%s=\"%s\" 未在语言文件（bin/resources/lang/*.txt）中定义" % (attr_name, value))

    def check_font(self, path, node, attr_name, value, local):
        if value not in local["fonts"]:
            self.add_issue("warning", path, node.line, node.column,
                           "%s=\"%s\" 未在主题 global.xml 的 <Font id> 中定义" % (attr_name, value))

    def check_color(self, path, node, attr_name, value, local):
        color_value = expand_vars(value, local["vars"])
        if "$" in color_value:
            return
        if color_value.startswith("#"):
            if not HEX_COLOR_RE.match(color_value):
                self.add_issue("warning", path, node.line, node.column,
                               "%s=\"%s\" 不是合法的十六进制颜色（只支持 #RRGGBB 或 #AARRGGBB）"
                               % (attr_name, value))
            return
        if "|" in color_value or "," in color_value or " " in color_value:
            return
        if color_value in local["colors"]:
            return
        if color_value.lower() in self.standard_colors:
            return
        self.add_issue("warning", path, node.line, node.column,
                       "%s=\"%s\" 不是已定义的语义色或标准颜色名（见 bin/resources/themes/color_light/global.xml）"
                       % (attr_name, value))

    def check_include(self, path, node, search_dirs, local):
        include_path = node.get("src") or node.get("source") or ""
        include_path = expand_vars(include_path.strip(), local["vars"])
        count_value = (node.get("count") or "").strip()
        if count_value:
            if not count_value.isdigit() or int(count_value) <= 0:
                self.add_issue("error", path, node.line, node.column,
                               "Include 的 count 必须是正整数，当前值：%s" % count_value)
        if not include_path:
            self.add_issue("error", path, node.line, node.column,
                           "Include 节点缺少 src（或 source）属性，无法加载被包含的 XML 文件")
            return
        if "$" in include_path:
            return
        resolved = self.resolve_resource(path, include_path, search_dirs)
        if resolved is None:
            self.add_issue("error", path, node.line, node.column,
                           "Include 引用的文件不存在：%s" % include_path)
            return
        self.lint_file(resolved)

    def check_image(self, path, node, attr_name, value, search_dirs, local):
        for image_path in extract_image_paths(value):
            image_path = expand_vars(image_path, local["vars"]).strip()
            if not image_path or "$" in image_path:
                continue
            if image_path.startswith("http://") or image_path.startswith("https://"):
                continue
            if not IMAGE_EXT_RE.search(image_path):
                continue
            if self.resolve_resource(path, image_path, search_dirs) is None:
                self.add_issue("warning", path, node.line, node.column,
                               "%s 引用的图片不存在：%s（若为运行期动态注册的资源可忽略）" % (attr_name, image_path))


def collect_files(repo, paths):
    files = []
    for raw_path in paths:
        path = Path(raw_path)
        if not path.is_absolute():
            path = repo / path
        if not path.exists():
            continue
        if path.is_dir():
            files.extend(sorted(p for p in path.rglob("*.xml") if p.is_file()))
        elif path.suffix.lower() == ".xml":
            files.append(path)
    return files


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="nim_duilib 布局 XML 静态检查工具（节点名/class 位置/name 重复/颜色/字体/text_id/Include/图片）",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__.split("检查规则：", 1)[0].strip())
    parser.add_argument("paths", nargs="*", help="要检查的 XML 文件或目录（默认检查 %s）" % (DEFAULT_PATHS,))
    parser.add_argument("--repo", default=str(DEFAULT_REPO), help="nim_duilib 仓库根目录（默认自动识别）")
    parser.add_argument("--strict", action="store_true", help="把警告也视为失败（退出码为 1）")
    parser.add_argument("--no-image", action="store_true", help="不检查图片文件是否存在")
    parser.add_argument("--no-text-id", action="store_true", help="不检查多语言 text_id 是否存在")
    parser.add_argument("--no-color", action="store_true", help="不检查颜色值是否合法")
    parser.add_argument("--allow-node", action="append", default=[], metavar="NAME",
                        help="自定义控件节点名（可重复指定），如 --allow-node RenderTest1")
    args = parser.parse_args(argv)

    repo = Path(args.repo).resolve()
    if not repo.is_dir():
        print("错误：找不到仓库根目录：%s" % repo, file=sys.stderr)
        return 2

    linter = Linter(repo,
                    check_images=not args.no_image,
                    check_text_ids=not args.no_text_id,
                    check_colors=not args.no_color,
                    allow_nodes=args.allow_node)
    files = collect_files(repo, args.paths if args.paths else DEFAULT_PATHS)
    if not files:
        print("错误：没有找到可检查的 XML 文件", file=sys.stderr)
        return 2

    for xml_file in files:
        linter.lint_file(xml_file)

    for issue in sorted(linter.issues, key=lambda item: (item.path, item.line, item.column)):
        print(issue.format())

    errors = sum(1 for issue in linter.issues if issue.level == "error")
    warnings = sum(1 for issue in linter.issues if issue.level == "warning")
    print()
    print("检查完成：%d 个文件，%d 个错误，%d 个警告" % (len(linter.visited), errors, warnings))
    if errors:
        return 1
    if args.strict and warnings:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
