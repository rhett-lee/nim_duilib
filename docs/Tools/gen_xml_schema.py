#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""根据文档与源码生成布局 XML 的 XSD 架构文件。

XML 驱动界面的编辑器（VS Code、Visual Studio、Notepad++、QtCreator 等）可以借助 XSD 提供：
1. 节点名补全与非法节点提示；
2. 属性名补全与悬浮说明（用途取自 docs 的属性表）；
3. 结构校验（子节点、属性名拼写）。

数据来源：
1. 节点名：docs/XmlNode.md 的节点名称表 + duilib/duilib_defs.h 的 DUI_CTR_* 宏 + WindowBuilder 支持的特殊节点；
2. 属性：docs/Control.md、Box.md、Window.md、Global.md、Theme.md、ThemeColor.md 中的属性表，
   继承关系取自文档中的“继承了 `X` 属性”“与`X`是基于相同模板的类”等说明，
   以及 docs/XmlNode.md 的节点说明列中的“继承X”；
3. 富文本标记：WindowBuilder::ParseRichTextXmlNode 支持的 <a>/<b>/<i>/<u>/<s>/<del>/<strike>/<ins>/<bgcolor>/<font>/<br>。

解析约定：
1. 子章节（如“### XML属性”“### Font 所有可用属性”）没有类名时，沿用上级章节的目标类；
2. 属性名称单元格支持别名写法（如“src（或 source）”“font / font_id”），两种写法都会生成；
3. 属性表中“说明/描述/用途”列作为 XSD 的 documentation，其他列（默认值、类型、示例值）忽略。

生成的 XSD 对属性采取宽松策略（每个节点都带 xs:anyAttribute），
因此不会因为用户自定义属性或框架新增属性导致校验失败；需要严格检查属性名时请使用 lint_xml.py。

用法：
    # 重新生成（默认输出到 docs/schema/nim_duilib.xsd）
    python docs/Tools/gen_xml_schema.py

    # 指定输出文件或仓库根目录
    python docs/Tools/gen_xml_schema.py --output docs/schema/nim_duilib.xsd
    python docs/Tools/gen_xml_schema.py --repo .

    # 额外检查：属性名是否在 C++ 源码中出现过（用于发现文档里的笔误）
    python docs/Tools/gen_xml_schema.py --verify

退出码：0 表示生成成功；1 表示生成失败（文档缺失、XSD 格式非法等）。
"""

import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from xml.sax.saxutils import escape

# 脚本位于 <repo>/docs/Tools/gen_xml_schema.py
DEFAULT_REPO = Path(__file__).resolve().parent.parent.parent
DEFAULT_OUTPUT = Path("docs/schema/nim_duilib.xsd")

# 属性数据来源（按顺序读取，先读到的属性说明优先使用）
ATTRIBUTE_DOCS = (
    "docs/Control.md",
    "docs/Box.md",
    "docs/Window.md",
    "docs/Global.md",
    "docs/Theme.md",
    "docs/ThemeColor.md",
)

# 非控件的特殊节点（与 lint_xml.py 保持一致）
SPECIAL_NODE_NAMES = (
    "Window", "Global", "Include",
    "DefaultFontFamilyNames", "FallbackFontFamilyNames",
    "Font", "FontFile", "FontResource",
    "Class", "TextColor", "ThemeColor", "ThemeMeta", "Theme", "Alias", "Var",
    "Event", "BubbledEvent",
    "PropertyGridGroup", "CheckComboText",
    "ListCtrlHeaderItem", "ListCtrlItem", "ListCtrlSubItem",
    "PropertyGridTextProperty", "PropertyGridComboProperty",
    "PropertyGridFontProperty", "PropertyGridFontSizeProperty",
    "PropertyGridColorProperty", "PropertyGridDateTimeProperty",
    "PropertyGridIPAddressProperty", "PropertyGridHotKeyProperty",
    "PropertyGridFileProperty", "PropertyGridDirectoryProperty",
    "Property",
)

# 不是控件节点，但会作为 XML 根节点被 WindowBuilder 或资源管理加载
TEMPLATE_ROOT_NAMES = ("Loading", "ChildWindowTemplate")

# <Global> 节点下允许出现的节点
GLOBAL_CHILD_NODES = (
    "Theme", "ThemeMeta", "ThemeColor", "TextColor", "Alias", "Var",
    "Font", "FontFile", "FontResource",
    "DefaultFontFamilyNames", "FallbackFontFamilyNames",
    "Class", "Include", "Property",
)

# RichText 家族节点（子节点可以是富文本标记）
RICH_TEXT_NODE_NAMES = ("RichText", "RichTextBox", "RichTextHBox", "RichTextVBox")

# RichText 节点支持的富文本标记（见 WindowBuilder::ParseRichTextXmlNode）
MARKUP_NODE_NAMES = ("a", "b", "i", "u", "s", "del", "strike", "ins", "bgcolor", "font", "br")

# 富文本标记的属性（见 WindowBuilder::ParseRichTextXmlNode）
MARKUP_ATTRIBUTES = ("href", "color", "face", "size")

# 文档中有属性表、但不是 XML 节点的类（布局类、模板类）
LAYOUT_CLASS_NAMES = (
    "Layout", "HLayout", "VLayout", "HFlowLayout", "VFlowLayout",
    "HTileLayout", "VTileLayout", "GridLayout",
    "VirtualHLayout", "VirtualVLayout", "VirtualHTileLayout", "VirtualVTileLayout",
)
TEMPLATE_CLASS_NAMES = ("ControlDragableT", "ControlMovableT", "ControlResizableT")

# 文档中有属性表、但不对应 XML 节点名的类名（作为父类参与属性继承）
# 例如 ListBox/VirtualListBox 是 C++ 类名，XML 中实际使用 HListBox/VListBox 等节点
EXTRA_CLASS_NAMES = ("ListBox", "VirtualListBox")

# 没有独立属性表、但根据源码定义可以确定的父类（文档中没有写明继承关系）
NODE_PARENT_HINTS = {
    "ListBoxItemH": ("ListBoxItem", "HBox"),   # ListBoxItemTemplate<HBox>
    "ListBoxItemV": ("ListBoxItem", "VBox"),   # ListBoxItemTemplate<VBox>
    "MenuItem": ("ListBoxItem",),              # Menu.h: class MenuItem : public ListBoxItem
    "SubMenu": ("ListBoxItem",),               # Menu.cpp: class SubMenu : public ListBoxItem
}

# 没有文档属性表、但确实存在的属性（补充定义）
BUILTIN_ATTRIBUTES = {
    "DefaultFontFamilyNames": {"windows": "Windows 系统使用的字体名称列表，逗号分隔",
                               "macos": "macOS 系统使用的字体名称列表，逗号分隔",
                               "linux": "Linux 系统使用的字体名称列表，逗号分隔",
                               "freebsd": "FreeBSD 系统使用的字体名称列表，逗号分隔"},
    "FallbackFontFamilyNames": {
        "windows": "Windows 系统的候选字体（默认字体缺少字形时依次回退），逗号分隔",
        "macos": "macOS 系统的候选字体，逗号分隔",
        "linux": "Linux 系统的候选字体，逗号分隔",
        "freebsd": "FreeBSD 系统的候选字体，逗号分隔",
    },
}

# 属性表表头（第一列）
ATTRIBUTE_HEADER_RE = re.compile(r"^(属性名(?:称)?|属性|参数名(?:称)?|名称|attribute|attribute name)(?:\s*[\(（][^\)）]*[\)）])?$", re.I)
# 属性表中的说明列
DESCRIPTION_HEADER_RE = re.compile(r"^(说明|描述|用途|注释|备注|description)$", re.I)
# 表格分隔行，如 | :--- | :---: |
TABLE_SEPARATOR_RE = re.compile(r"^:?-{2,}:?$")
# 属性名称单元格（允许用反引号包裹）
ATTRIBUTE_CELL_RE = re.compile(r"^`?([A-Za-z_][A-Za-z0-9_]*)`?$")
# 属性名称单元格的别名写法：src（或 source）
ATTRIBUTE_ALT_CELL_RE = re.compile(r"^`?([A-Za-z_][A-Za-z0-9_]*)`?（或\s*`?([A-Za-z_][A-Za-z0-9_]*)`?）")
# 属性名称单元格的别名写法：font / font_id
ATTRIBUTE_SLASH_CELL_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)\s*/\s*([A-Za-z_][A-Za-z0-9_]*)$")
# 章节标题中的编号前缀
TITLE_NUMBER_RE = re.compile(r"^[\s0-9一二三四五六七八九十\.、]*")
# 章节标题中的英文标识符
TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
# 继承说明：关键字后面到标点符号之前的部分（“不继承”除外）
PARENT_HINT_RE = re.compile(r"(?<!不)(?:继承(?:自|了)?|相同模板|请参考)([^，。；：（）\n]*)")


def read_text(path):
    """读取 UTF-8 文本（兼容 BOM）"""
    return path.read_text(encoding="utf-8-sig")


def parse_xml_node_names(repo):
    """从 docs/XmlNode.md 的表格中提取节点名"""
    node_names = set()
    path = repo / "docs" / "XmlNode.md"
    if not path.is_file():
        return node_names
    for line in read_text(path).splitlines():
        line = line.strip()
        if not line.startswith("|"):
            continue
        cells = split_table_row(line)
        if len(cells) < 2:
            continue
        # 第二列是 "NodeName" 形式的节点名
        match = re.match(r'"([A-Za-z_][A-Za-z0-9_]*)"', cells[1].strip())
        if match:
            node_names.add(match.group(1))
    return node_names


def parse_dui_ctr_node_names(repo):
    """从 duilib/duilib_defs.h 提取 DUI_CTR_* 宏中的节点名（忽略小写的控件 name）"""
    node_names = set()
    path = repo / "duilib" / "duilib_defs.h"
    if not path.is_file():
        return node_names
    for line in read_text(path).splitlines():
        match = re.search(r'#define\s+DUI_CTR_[A-Z0-9_]*\s+\(_T\("([^"]+)"\)\)', line)
        if not match:
            continue
        name = match.group(1)
        # 小写开头的是控件 name（如 window_title_bar、btn_window_close），不是节点名
        if name[:1].isupper():
            node_names.add(name)
    return node_names


def split_table_row(line):
    """拆分 Markdown 表格的一行，返回单元格文本列表"""
    text = line.strip()
    if text.startswith("|"):
        text = text[1:]
    if text.endswith("|"):
        text = text[:-1]
    return [cell.strip() for cell in text.split("|")]


def split_sections(text):
    """按 Markdown 的一级到六级标题拆分章节，返回 [(标题级别, 标题, [正文行])]"""
    sections = []
    current = None
    for line in text.splitlines():
        match = re.match(r"^(#{1,6})\s+(.+?)\s*$", line)
        if match:
            current = (len(match.group(1)), match.group(2), [])
            sections.append(current)
        elif current is not None:
            current[2].append(line)
    return sections


def is_table_header(body_lines, index):
    """判断第 index 行是否是 Markdown 表格的表头（下一行是分隔行）"""
    if index + 1 >= len(body_lines):
        return False
    line = body_lines[index].strip()
    next_line = body_lines[index + 1].strip()
    if (not line.startswith("|")) or (not next_line.startswith("|")):
        return False
    cells = split_table_row(next_line)
    return bool(cells) and all((not cell) or TABLE_SEPARATOR_RE.match(cell) for cell in cells)


def parse_attribute_tables(body_lines):
    """从章节正文中提取属性表，返回 [(属性名, 说明)]（按出现顺序）

    属性表允许被说明文字（或注释块）中断：例如 Control 的属性表在中间插入了“注意”说明，
    后续行仍然是同一张表的延续，这里按列数判断并继续读取。
    """
    attributes = []
    columns = 0
    description_column = -1
    index = 0
    while index < len(body_lines):
        line = body_lines[index].strip()
        if not line.startswith("|"):
            index += 1
            continue
        cells = split_table_row(line)
        if is_table_header(body_lines, index):
            columns = len(cells) if (cells and ATTRIBUTE_HEADER_RE.match(cells[0])) else 0
            description_column = -1
            if columns:
                # 说明列优先取“说明/描述/用途”列，否则取最后一列（如“用途”“示例值”）
                for column, cell in enumerate(cells):
                    if DESCRIPTION_HEADER_RE.match(cell.strip()):
                        description_column = column
                        break
                if description_column < 0:
                    description_column = len(cells) - 1
            index += 2
            continue
        if cells and all((not cell) or TABLE_SEPARATOR_RE.match(cell) for cell in cells):
            index += 1
            continue
        if columns and (len(cells) == columns):
            names = parse_attribute_cell(cells[0])
            description = clean_description(cells[description_column]) if description_column >= 1 else ""
            attributes.extend((name, description) for name in names)
        index += 1
    return attributes


def parse_attribute_cell(cell):
    """解析属性名称单元格，返回属性名列表（支持 src（或 source）、font / font_id 的写法）"""
    cell = cell.strip().replace("`", "")
    match = ATTRIBUTE_ALT_CELL_RE.match(cell)
    if match:
        return [match.group(1), match.group(2)]
    match = ATTRIBUTE_SLASH_CELL_RE.match(cell)
    if match:
        return [match.group(1), match.group(2)]
    match = ATTRIBUTE_CELL_RE.match(cell)
    if match:
        return [match.group(1)]
    return []


def clean_description(text):
    """清理属性说明中的 Markdown 标记"""
    text = re.sub(r"<br\s*/?>", " ", text, flags=re.I)
    text = text.replace("`", "").strip()
    text = re.sub(r"[|]", " ", text)
    text = re.sub(r"\s+", " ", text)
    for mark in ("**", "__"):
        text = text.replace(mark, "")
    return text


def title_tokens(title):
    """提取标题中的类名候选（去掉编号前缀后的英文标识符）"""
    title = TITLE_NUMBER_RE.sub("", title).strip()
    return TOKEN_RE.findall(title)


def match_section_targets(title, known_names):
    """根据章节标题找出该属性表对应的节点/类名"""
    # 中文标题的显式映射
    if title.strip() == "窗口的属性":
        return ["Window"]

    tokens = title_tokens(title)
    if not tokens:
        return []

    # "RichEdit/RichEdit2的属性"、"Panel/PanelHBox/PanelVBox的属性" 这类写法
    if re.search(r"[A-Za-z0-9_]+(/[A-Za-z0-9_]+)+", title):
        targets = [name for name in known_names
                   if re.search(r"(?<![A-Za-z0-9_])%s(?![A-Za-z0-9_])" % re.escape(name), title)]
        if targets:
            return sorted(targets, key=lambda name: title.index(name))

    # 其余情况取第一个已知的类名（标题里出现的父类名不算本章节的目标）
    for token in tokens:
        if token in known_names:
            return [token]
    return []


def match_parents(title, body_lines, known_names):
    """从标题和正文中提取父类/模板类名称

    文档中的继承说明比较固定，例如：
    - “X 控件继承了 `Y` 属性，更多可用属性请参考`Y`的属性”
    - “X 控件继承了`A`、`B`和`C`属性”
    - “X 控件继承自Y，……”、“X与Y是基于相同模板的类，请参考 `Y`的属性”
    这里只解析继承关键字所在的短句（到标点符号为止），避免把同一句/表格中提到的
    其他类名误当成父类。
    """
    parents = []
    for line in [title] + body_lines:
        line = line.strip()
        if line.startswith("|"):
            # 表格行（属性表或节点说明表）不参与继承关系的解析
            continue
        for match in PARENT_HINT_RE.finditer(line):
            for token in TOKEN_RE.findall(match.group(1)):
                if (token in known_names) and (token not in parents):
                    parents.append(token)
    return parents


def parse_xml_node_parents(repo, known_names):
    """从 docs/XmlNode.md 的说明列中提取“继承X”的父类名，返回 {节点名: [父类名]}"""
    parents = {}
    path = repo / "docs" / "XmlNode.md"
    if not path.is_file():
        return parents
    for line in read_text(path).splitlines():
        line = line.strip()
        if not line.startswith("|"):
            continue
        cells = split_table_row(line)
        if len(cells) < 4:
            continue
        match = re.match(r'"([A-Za-z_][A-Za-z0-9_]*)"', cells[1].strip())
        if not match:
            continue
        node_name = match.group(1)
        description = cells[3]
        # 只取“继承X”中紧跟其后的类名，避免把说明里的其他类名（如“由 Menu::CreateControl 创建”）当成父类
        for parent_match in re.finditer(r"继承(?:自)?\s*`?([A-Za-z_][A-Za-z0-9_]*)", description):
            token = parent_match.group(1)
            if (token in known_names) and (token != node_name):
                parents.setdefault(node_name, []).append(token)
    return parents


def collect_records(repo, known_names):
    """解析文档，返回 {类名: {"attributes": [(属性名, 说明)], "parents": [父类名]}}"""
    records = {}

    def record_of(name):
        if name not in records:
            records[name] = {"attributes": [], "parents": [], "source": ""}
        return records[name]

    # docs/XmlNode.md 中说明列的“继承X”，用于补充没有独立章节的节点（如 MenuListBox）
    node_parents = parse_xml_node_parents(repo, known_names)
    for name, parent_names in NODE_PARENT_HINTS.items():
        node_parents.setdefault(name, []).extend(parent_names)
    for name, parent_names in node_parents.items():
        record = record_of(name)
        for parent in parent_names:
            if (parent != name) and (parent not in record["parents"]):
                record["parents"].append(parent)

    for doc_file in ATTRIBUTE_DOCS:
        path = repo / doc_file
        if not path.is_file():
            continue
        text = read_text(path)
        # 子章节（如“XML属性”）通常没有类名，若标题未命中则沿用上级章节的目标类
        section_stack = []  # [(标题级别, [目标类名], 是否由标题直接匹配)]
        for level, title, body_lines in split_sections(text):
            while section_stack and (section_stack[-1][0] >= level):
                section_stack.pop()
            targets = match_section_targets(title, known_names)
            directly_matched = bool(targets)
            if (not targets) and section_stack and section_stack[-1][2]:
                # 只从“标题直接指明类名”的上级章节继承，避免把例如 <Global> 的属性扩散到无关子章节
                targets = section_stack[-1][1]
            section_stack.append((level, targets, directly_matched))
            if not targets:
                continue
            attributes = parse_attribute_tables(body_lines)
            parents = match_parents(title, body_lines, known_names)
            for target in targets:
                record = record_of(target)
                for name, description in attributes:
                    if name not in [item[0] for item in record["attributes"]]:
                        record["attributes"].append((name, description))
                for parent in parents + node_parents.get(target, []):
                    if (parent != target) and (parent not in record["parents"]):
                        record["parents"].append(parent)
                if attributes and not record["source"]:
                    record["source"] = doc_file
    return records


def layout_class_for_node(node_name):
    """根据容器节点名推断使用的布局类（用于补充布局属性）"""
    if node_name.startswith("Virtual"):
        tile = "Tile" in node_name
        if "H" in node_name:
            return "VirtualHTileLayout" if tile else "VirtualHLayout"
        if "V" in node_name:
            return "VirtualVTileLayout" if tile else "VirtualVLayout"
    if "Grid" in node_name:
        return "GridLayout"
    if "Flow" in node_name:
        return "HFlowLayout" if node_name.startswith("H") else "VFlowLayout"
    if "Tile" in node_name:
        return "HTileLayout" if node_name.startswith("H") else "VTileLayout"
    if node_name.startswith("H"):
        return "HLayout"
    if node_name.startswith("V"):
        return "VLayout"
    return "Layout"


def attribute_chain(node_name, records):
    """返回节点的属性来源链（自身、布局类、父类依次排列）"""
    chain = []
    seen = set()

    def add(name, stack):
        if (name in seen) or (name not in records) or (name in stack):
            return
        seen.add(name)
        chain.append(name)
        for parent in records[name]["parents"]:
            add(parent, stack + [name])

    add(node_name, [])
    if "Box" in chain:
        # 容器还需要布局类的属性（如 justify、scale_down、grid_width 等）
        add(layout_class_for_node(node_name), [])
    return chain


def collect_attribute_groups(node_names, records):
    """为每个节点计算属性组引用

    属性组按“来源类名 + 属性子集”去重：同一个类只声明一次属性，
    子类通过引用父类的属性组来实现继承，避免 XSD 体积膨胀。
    返回 (属性组字典, 节点属性组引用字典)
    """
    groups = {}      # 组名 -> (来源类名, [(属性名, 说明)])
    node_groups = {}  # 节点名 -> [组名]
    group_names = {}  # (来源类名, 属性签名) -> 组名

    def ensure_group(owner, attributes):
        signature = (owner, tuple(sorted(name for name, _description in attributes)))
        if signature in group_names:
            return group_names[signature]
        group_name = "%sAttrs" % owner
        suffix = 2
        while group_name in groups:
            group_name = "%sAttrs%d" % (owner, suffix)
            suffix += 1
        group_names[signature] = group_name
        groups[group_name] = (owner, attributes)
        return group_name

    for node_name in sorted(node_names):
        included = set()
        refs = []
        for owner in attribute_chain(node_name, records):
            attributes = [(name, description) for name, description in records[owner]["attributes"]
                          if name not in included]
            if not attributes:
                continue
            included.update(name for name, _description in attributes)
            refs.append(ensure_group(owner, attributes))
        attributes = [(name, description) for name, description in BUILTIN_ATTRIBUTES.get(node_name, {}).items()
                      if name not in included]
        if attributes:
            refs.append(ensure_group(node_name, attributes))
        node_groups[node_name] = refs
    return groups, node_groups


def xsd_attribute(attr_name, description):
    """生成一条 XSD 属性声明"""
    if not description:
        return '    <xs:attribute name="%s" type="xs:string"/>' % escape(attr_name)
    return ('    <xs:attribute name="%s" type="xs:string">\n'
            '      <xs:annotation>\n'
            '        <xs:documentation>%s</xs:documentation>\n'
            '      </xs:annotation>\n'
            '    </xs:attribute>') % (escape(attr_name), escape(description))


def build_schema(node_names, node_groups, groups):
    """生成 XSD 文本"""
    lines = []
    lines.append('<?xml version="1.0" encoding="UTF-8"?>')
    lines.append('<!-- 本文件由 docs/Tools/gen_xml_schema.py 自动生成，请勿手工修改。 -->')
    lines.append('<!-- 生成方法：python docs/Tools/gen_xml_schema.py -->')
    lines.append('<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema" elementFormDefault="qualified">')
    lines.append('  <xs:annotation>')
    lines.append('    <xs:documentation>')
    lines.append('      nim_duilib 布局 XML 的结构定义（XML 皮肤 + C++ 逻辑的桌面 UI 框架）。')
    lines.append('      节点名来自 docs/XmlNode.md 与 duilib/duilib_defs.h 的 DUI_CTR_* 宏；属性来自 docs 的属性表。')
    lines.append('      为兼容自定义控件属性与框架新增属性，每个节点都允许未声明的属性（xs:anyAttribute）；')
    lines.append('      需要严格检查属性名、颜色、字体、text_id、Include、图片等，请使用 docs/Tools/lint_xml.py。')
    lines.append('      自定义控件节点（通过 WindowBuilder 的控件工厂注册）不在本文件中，可在自己的扩展 XSD 中声明。')
    lines.append('    </xs:documentation>')
    lines.append('  </xs:annotation>')
    lines.append('')

    # 富文本标记
    lines.append('  <!-- 富文本标记（RichText 的子节点，不是控件） -->')
    lines.append('  <xs:complexType name="RichTextMarkupType" mixed="true">')
    lines.append('    <xs:group ref="RichTextContent" minOccurs="0" maxOccurs="unbounded"/>')
    for attr_name in MARKUP_ATTRIBUTES:
        lines.append('    <xs:attribute name="%s" type="xs:string"/>' % attr_name)
    lines.append('    <xs:anyAttribute processContents="skip"/>')
    lines.append('  </xs:complexType>')
    lines.append('')

    # 子节点分组
    lines.append('  <!-- 任意控件节点都可以作为容器的子节点（框架按 XML 顺序递归创建） -->')
    lines.append('  <xs:group name="AnyNode">')
    lines.append('    <xs:choice>')
    for node_name in sorted(node_names):
        lines.append('      <xs:element ref="%s"/>' % node_name)
    lines.append('    </xs:choice>')
    lines.append('  </xs:group>')
    lines.append('')
    lines.append('  <!-- 富文本内容：除控件节点外，还可以是富文本标记 -->')
    lines.append('  <xs:group name="RichTextContent">')
    lines.append('    <xs:choice>')
    for node_name in sorted(node_names):
        lines.append('      <xs:element ref="%s"/>' % node_name)
    for markup_name in MARKUP_NODE_NAMES:
        lines.append('      <xs:element ref="%s"/>' % markup_name)
    lines.append('    </xs:choice>')
    lines.append('  </xs:group>')
    lines.append('')
    lines.append('  <!-- 全局资源节点的子节点（如 <ThemeMeta> 下的 <Property>） -->')
    lines.append('  <xs:group name="GlobalContent">')
    lines.append('    <xs:choice>')
    for node_name in GLOBAL_CHILD_NODES:
        lines.append('      <xs:element ref="%s"/>' % node_name)
    lines.append('    </xs:choice>')
    lines.append('  </xs:group>')
    lines.append('')

    # 属性组
    lines.append('  <!-- 属性组：每个类只声明自己的属性，子类通过引用父类属性组实现继承 -->')
    for group_name, (owner, attributes) in groups.items():
        lines.append('  <xs:attributeGroup name="%s">' % group_name)
        lines.append('    <!-- 来源：%s -->' % escape(owner))
        for attr_name, description in attributes:
            lines.append(xsd_attribute(attr_name, description))
        lines.append('  </xs:attributeGroup>')
    lines.append('')

    # 节点类型
    lines.append('  <!-- 节点类型 -->')
    for node_name in sorted(node_names):
        lines.append('  <xs:complexType name="%sType" mixed="true">' % node_name)
        if node_name == "Global":
            # <Global> 下可以放全局资源节点，也可以放控件模板（例如 property_grid.xml 中的 <PropertyGrid>）
            lines.append('    <xs:group ref="AnyNode" minOccurs="0" maxOccurs="unbounded"/>')
        elif node_name in RICH_TEXT_NODE_NAMES:
            lines.append('    <xs:group ref="RichTextContent" minOccurs="0" maxOccurs="unbounded"/>')
        elif node_name in TEMPLATE_ROOT_NAMES:
            lines.append('    <xs:group ref="AnyNode" minOccurs="0" maxOccurs="unbounded"/>')
        elif node_name in GLOBAL_CHILD_NODES:
            # 全局资源节点（Font/FontFile/ThemeColor/Class/Include/Var/Alias/Property/Theme/ThemeMeta 等）
            if node_name in ("ThemeMeta",):
                lines.append('    <xs:group ref="GlobalContent" minOccurs="0" maxOccurs="unbounded"/>')
        else:
            lines.append('    <xs:group ref="AnyNode" minOccurs="0" maxOccurs="unbounded"/>')
        for group_name in node_groups.get(node_name, []):
            lines.append('    <xs:attributeGroup ref="%s"/>' % group_name)
        lines.append('    <xs:anyAttribute processContents="skip"/>')
        lines.append('  </xs:complexType>')
    lines.append('')

    # 富文本标记元素
    lines.append('  <!-- 富文本标记元素 -->')
    for markup_name in MARKUP_NODE_NAMES:
        lines.append('  <xs:element name="%s" type="RichTextMarkupType"/>' % markup_name)
    lines.append('')

    # 全局元素
    lines.append('  <!-- 全局元素：任何一个节点都可以作为 XML 的根节点（窗口、Global、模板等） -->')
    for node_name in sorted(node_names):
        lines.append('  <xs:element name="%s" type="%sType"/>' % (node_name, node_name))
    lines.append('</xs:schema>')
    return "\n".join(lines) + "\n"


def verify_attributes(repo, groups):
    """检查属性名是否在 C++ 源码中出现过，返回疑似笔误的属性名"""
    source_text = []
    for path in sorted((repo / "duilib").rglob("*")):
        if path.suffix.lower() in (".h", ".hpp", ".cpp", ".c", ".cc"):
            try:
                source_text.append(read_text(path))
            except (OSError, UnicodeDecodeError):
                continue
    blob = "\n".join(source_text)
    missing = {}
    for owner, attributes in groups.values():
        for attr_name, _description in attributes:
            if attr_name in missing:
                continue
            if re.search(r'"%s"' % re.escape(attr_name), blob) is None:
                missing[attr_name] = owner
    return missing


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="根据 docs 的属性表生成布局 XML 的 XSD",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__.split("用法：", 1)[0].strip())
    parser.add_argument("--repo", default=str(DEFAULT_REPO), help="nim_duilib 仓库根目录（默认自动识别）")
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT), help="输出的 XSD 文件（默认 %s）" % DEFAULT_OUTPUT)
    parser.add_argument("--verify", action="store_true", help="检查属性名是否在 C++ 源码中出现过")
    args = parser.parse_args(argv)

    repo = Path(args.repo).resolve()
    if not repo.is_dir():
        print("错误：找不到仓库根目录：%s" % repo, file=sys.stderr)
        return 2

    node_names = set(parse_xml_node_names(repo))
    node_names |= parse_dui_ctr_node_names(repo)
    node_names |= set(SPECIAL_NODE_NAMES)
    node_names |= set(TEMPLATE_ROOT_NAMES)
    if not node_names:
        print("错误：没有解析到任何节点名（docs/XmlNode.md 或 duilib_defs.h 缺失？）", file=sys.stderr)
        return 1

    known_names = set(node_names)
    known_names |= set(LAYOUT_CLASS_NAMES)
    known_names |= set(TEMPLATE_CLASS_NAMES)
    known_names |= set(EXTRA_CLASS_NAMES)
    records = collect_records(repo, known_names)

    groups, node_groups = collect_attribute_groups(node_names, records)
    schema_text = build_schema(node_names, node_groups, groups)
    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = repo / output_path
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(schema_text, encoding="utf-8", newline="\n")

    # 校验生成的 XSD 是合法 XML
    try:
        ET.parse(str(output_path))
    except ET.ParseError as error:
        print("错误：生成的 XSD 不是合法的 XML：%s" % error, file=sys.stderr)
        return 1

    attributes = set()
    for _owner, attr_list in groups.values():
        attributes.update(attr_name for attr_name, _description in attr_list)
    attributes.update(MARKUP_ATTRIBUTES)
    nodes_without_attributes = [node for node in node_names if not node_groups.get(node)]

    print("已生成：%s" % output_path.as_posix())
    print("节点：%d 个，属性组：%d 个，属性名：%d 个，无属性的节点：%d 个"
          % (len(node_names), len(groups), len(attributes), len(nodes_without_attributes)))
    if nodes_without_attributes:
        print("无属性的节点（文档中尚未提供属性表）：%s" % ", ".join(sorted(nodes_without_attributes)))

    if args.verify:
        missing = verify_attributes(repo, groups)
        if missing:
            print()
            print("以下属性名在 C++ 源码中没有找到（可能是文档笔误，或者是文档中已定义、暂未实现的规范字段）：")
            for attr_name in sorted(missing):
                print("  %s（首次出现在 %s）" % (attr_name, missing[attr_name]))
        else:
            print("属性名检查通过：全部属性名都在 C++ 源码中出现过")
    return 0


if __name__ == "__main__":
    sys.exit(main())
