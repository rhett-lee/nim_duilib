#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""controls 示例页面的静态校验（跨平台版本）。

用 Python3 重写 tests/test_controls_pages.ps1，Windows/Linux/macOS 均可执行。
脚本不创建窗口、不加载主题，只解析资源文件，因此可直接在命令行与 CI 中运行。

校验规则：
  R1  每个页面都是格式正确的 XML。
  R2  元素名必须是合法控件节点（来自 duilib_defs.h 的 DUI_CTR_*）。
      另外允许 "Window" 和 "Event" 两个非控件节点；RichText/RichText*
      内部的子树（富文本标记，例如 <a href="...">）不参与校验。
  R3  每个 class 引用都必须有对应的 <Class name="..."> 定义。
  R4  每个具名颜色都必须由 ThemeColor/Alias/Color/Var 定义，
      或来自 UiColors.cpp 的内置标准色表。
  R5  同一页面内控件 name 唯一。
  R6  每个 *_id（text_id、prompt_text_id、title_id 等）引用的 STRID
      都必须同时存在于中文和英文字符串文件中。检查范围为 controls 页面，
      以及 chat/render/tree_view/list_ctrl 主题目录下的全部 XML。
  R7  Form 中未限定对象的 FindControl(_T("name")) 必须能在该 Form 的
      GetSkinFile() 返回的 XML 中找到。
  R8  controls.xml 中的每个 btn_gallery_* 按钮都必须在 ControlForm.cpp
      中通过 bindOpen() 绑定；反之每个 bindOpen() 名称也必须存在于
      controls.xml。
  R9  Form 的 GetSkinFile() 返回的 XML 文件必须存在。
  R10 拒绝框架不支持的属性（expanded=、child_spacing=），
      这两个属性曾导致控件测试页出现真实回归。
  R11 test.xml 中"标签 + 线条"行的高度与标签宽度必须足够，
      避免 Line 前面的文字被纵向裁切或横向截断。
  R12 多行 RichEdit 必须开启垂直滚动条，且富文本示例行数足够多
      （否则看不到滚动条效果）。
  R13 disabled_weekdays 的取值必须是合法星期；datetime.xml 的"禁用周末"
      示例必须同时禁用周六与周日，且 Calendar/CalendarFlyout/DateTime
      的属性转发链路保持完整。
  R14 Mask 的 highlight_control 必须指向同页控件；当高亮目标位于滚动容器
      内部而 Mask 在其外部时，Mask.cpp 必须补偿滚动偏移。
  R15 控件的 UI 文本属性（text、prompt_text、title、x_axis_title、
      y_axis_title、tooltip_text）禁止硬编码中文，必须改用对应的 *_id，
      否则切换语言后文本不会跟随变化。检查范围同 R6。

退出码：全部通过为 0，存在失败项为 1。
"""

import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

FAILURES = []
CHECKS = 0


def add_failure(rule, message):
    FAILURES.append("[{}] {}".format(rule, message))


def read_text(path):
    """按 UTF-8 读取文本，并兼容 BOM（语言文件带 BOM）。"""
    return Path(path).read_text(encoding="utf-8-sig")


def iter_page_elements(element):
    """遍历页面元素，跳过 RichText/RichText* 的富文本内容子树。"""
    yield element
    if element.tag.startswith("RichText"):
        return
    for child in element:
        yield from iter_page_elements(child)


def is_color_attribute(name):
    return (name.startswith("bkcolor") or
            name.startswith("bordercolor") or
            name.startswith("fgcolor") or
            name.startswith("bgcolor") or
            name == "color" or
            name.endswith("_color"))


def collect_language_keys(files):
    keys = set()
    for path in files:
        if not path.is_file():
            continue
        for match in re.finditer(r"(?m)^\s*(STRID_[A-Z0-9_]+)\s*=", read_text(path)):
            keys.add(match.group(1))
    return keys


def to_int(value):
    """把属性值解析为整数，非数字（auto/stretch/百分比）返回 None。"""
    if value is None:
        return None
    try:
        return int(value.strip())
    except ValueError:
        return None


def parse_font_size(font):
    """从字体名（如 system_regular_12）中提取字号。"""
    if not font:
        return None
    match = re.search(r"_(\d+)$", font.strip())
    return int(match.group(1)) if match else None


def is_true(value):
    """与框架 StringUtil::IsValueTrue 的常见取值保持一致。"""
    return value is not None and value.strip().lower() in ("true", "1", "yes", "on")


def build_parent_map(root):
    parents = {}
    for parent in root.iter():
        for child in parent:
            parents[child] = parent
    return parents


def is_inside(element, parents, tags):
    """判断元素是否存在标签属于 tags 的祖先。"""
    current = element
    while current in parents:
        current = parents[current]
        if current.tag in tags:
            return True
    return False


def count_sample_text_lines(source_text, control_name):
    """统计某个 FindControl 之后第一次 SetText 调用中的换行数量。

    源码中的换行是转义后的字面量 \\r\\n，因此按字符计数即可。
    """
    anchor = 'FindControl(_T("{}"))'.format(control_name)
    start = source_text.find(anchor)
    if start < 0:
        return -1
    set_pos = source_text.find("SetText(", start)
    if set_pos < 0:
        return -1
    end_pos = source_text.find(");", set_pos)
    if end_pos < 0:
        end_pos = len(source_text)
    return source_text[set_pos:end_pos].count("\\r\\n")


def main():
    global CHECKS

    default_root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description="controls 示例页面静态校验")
    parser.add_argument("--source-root", default=str(default_root),
                        help="仓库根目录（默认取脚本所在目录的上一级）")
    parser.add_argument("--theme", default="default",
                        help="bin/resources/themes 下的主题目录名")
    args = parser.parse_args()

    source_root = Path(args.source_root).resolve()
    theme_name = args.theme

    theme_root = source_root / "bin" / "resources" / "themes"
    pages_root = theme_root / theme_name / "controls"
    lang_root = source_root / "bin" / "resources" / "lang"
    examples_root = source_root / "examples" / "controls"
    defs_file = source_root / "duilib" / "duilib_defs.h"
    ui_colors_file = source_root / "duilib" / "Core" / "UiColors.cpp"

    if not pages_root.is_dir():
        add_failure("R0-SETUP", "page folder not found: {}".format(pages_root))
    if not defs_file.is_file():
        add_failure("R0-SETUP", "duilib_defs.h not found: {}".format(defs_file))
    if FAILURES:
        for failure in FAILURES:
            print(failure)
        return 1

    # -----------------------------------------------------------------------
    # R1：解析所有页面
    # -----------------------------------------------------------------------
    page_files = sorted(p for p in pages_root.rglob("*.xml") if p.is_file())
    if not page_files:
        add_failure("R1-XML-PARSE", "no XML pages found below {}".format(pages_root))

    documents = {}
    for path in page_files:
        try:
            documents[path] = ET.parse(str(path)).getroot()
            CHECKS += 1
        except ET.ParseError as exc:
            add_failure("R1-XML-PARSE", "{}: {}".format(path.name, exc))

    # 其它示例主题目录：只参与 R6（语言键完整性）与 R15（禁止硬编码中文）
    extra_theme_names = ("chat", "render", "tree_view", "list_ctrl")
    extra_documents = {}
    for name in extra_theme_names:
        folder = theme_root / theme_name / name
        if not folder.is_dir():
            continue
        for path in sorted(p for p in folder.rglob("*.xml") if p.is_file()):
            try:
                extra_documents[path] = ET.parse(str(path)).getroot()
                CHECKS += 1
            except ET.ParseError as exc:
                add_failure("R1-XML-PARSE", "{}: {}".format(path.name, exc))
    all_documents = list(documents.items()) + list(extra_documents.items())

    # -----------------------------------------------------------------------
    # R2：元素名必须是合法控件节点
    # -----------------------------------------------------------------------
    legal_nodes = set()
    for match in re.finditer(r'DUI_CTR_[A-Z0-9_]+\s+\(_T\("([^"]+)"\)\)',
                             read_text(defs_file)):
        legal_nodes.add(match.group(1))
    legal_nodes.add("Window")  # 页面根节点
    legal_nodes.add("Event")   # XML 事件绑定节点

    if len(legal_nodes) <= 2:
        add_failure("R2-UNKNOWN-NODE",
                    "no control node names were extracted from duilib_defs.h")

    for path, root in documents.items():
        for element in iter_page_elements(root):
            CHECKS += 1
            if element.tag not in legal_nodes:
                add_failure("R2-UNKNOWN-NODE", "{}: <{}> is not a valid control node"
                            .format(path.name, element.tag))

    # -----------------------------------------------------------------------
    # R3：class 引用必须已定义
    # -----------------------------------------------------------------------
    defined_classes = set()
    for path in sorted(p for p in theme_root.rglob("*.xml") if p.is_file()):
        try:
            root = ET.parse(str(path)).getroot()
        except ET.ParseError:
            continue
        for node in root.iter("Class"):
            name = node.get("name")
            if name:
                defined_classes.add(name)

    for path, root in documents.items():
        for element in iter_page_elements(root):
            class_refs = element.get("class")
            if not class_refs:
                continue
            for class_ref in class_refs.split():
                if not class_ref:
                    continue
                CHECKS += 1
                if class_ref not in defined_classes:
                    add_failure("R3-UNKNOWN-CLASS",
                                '{}: class "{}" is not defined'
                                .format(path.name, class_ref))

    # -----------------------------------------------------------------------
    # R4：具名颜色必须已定义
    # -----------------------------------------------------------------------
    defined_colors = set()
    for path in sorted(p for p in theme_root.rglob("*.xml") if p.is_file()):
        try:
            root = ET.parse(str(path)).getroot()
        except ET.ParseError:
            continue
        for node in root.iter():
            if node.tag not in ("ThemeColor", "Alias", "Color", "Var"):
                continue
            name = node.get("name")
            if name:
                defined_colors.add(name.lower())

    if ui_colors_file.is_file():
        for match in re.finditer(r'\{_T\("([A-Za-z0-9_]+)"\)',
                                 read_text(ui_colors_file)):
            defined_colors.add(match.group(1).lower())

    identifier_pattern = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
    for path, root in documents.items():
        for element in iter_page_elements(root):
            for attribute_name, attribute_value in element.attrib.items():
                if not is_color_attribute(attribute_name):
                    continue
                if not identifier_pattern.match(attribute_value):
                    continue
                CHECKS += 1
                if attribute_value.lower() not in defined_colors:
                    add_failure("R4-UNKNOWN-COLOR",
                                '{}: color "{}" is not defined'
                                .format(path.name, attribute_value))

    # -----------------------------------------------------------------------
    # R5：同一页面内 name 唯一
    # -----------------------------------------------------------------------
    for path, root in documents.items():
        seen_names = set()
        for element in iter_page_elements(root):
            name = element.get("name")
            if not name:
                continue
            CHECKS += 1
            if name in seen_names:
                add_failure("R5-DUPLICATE-NAME",
                            '{}: duplicate name "{}"'.format(path.name, name))
            else:
                seen_names.add(name)

    # -----------------------------------------------------------------------
    # R6：所有 *_id 属性引用的 STRID 必须同时存在于中文和英文语言文件
    # -----------------------------------------------------------------------
    zh_keys = collect_language_keys([
        lang_root / "zh_CN.txt",
        lang_root / "zh_CN-public.txt",
        lang_root / "zh_CN-examples.txt",
    ])
    en_keys = collect_language_keys([
        lang_root / "en_US.txt",
        lang_root / "en_US-public.txt",
        lang_root / "en_US-examples.txt",
    ])

    for path, root in all_documents:
        for element in iter_page_elements(root):
            for attr_name, attr_value in element.attrib.items():
                if not attr_name.endswith("_id") or not attr_value:
                    continue
                # 多值属性用 | , / > 等分隔（column_title、data_labels_id、items_id 等）
                for text_id in re.findall(r"STRID_[A-Z0-9_]+", attr_value):
                    CHECKS += 1
                    if text_id not in zh_keys:
                        add_failure("R6-MISSING-TEXT-ID",
                                    '{}: {} "{}" is missing from the Chinese language files'
                                    .format(path.name, attr_name, text_id))
                    if text_id not in en_keys:
                        add_failure("R6-MISSING-TEXT-ID",
                                    '{}: {} "{}" is missing from the English language files'
                                    .format(path.name, attr_name, text_id))

    # -----------------------------------------------------------------------
    # R7 + R9：Form -> XML 映射与 FindControl 引用
    # -----------------------------------------------------------------------
    form_files = sorted(p for p in examples_root.glob("*Form.cpp") if p.is_file())
    if not form_files:
        add_failure("R7-FIND-CONTROL",
                    "no Form implementation files found below {}".format(examples_root))

    controls_xml_names = set()
    controls_xml_file = pages_root / "controls.xml"
    if controls_xml_file.is_file():
        for element in ET.parse(str(controls_xml_file)).getroot().iter():
            name = element.get("name")
            if name:
                controls_xml_names.add(name)

    for path in form_files:
        text = read_text(path)
        skin_match = re.search(r'GetSkinFile\(\)\s*\{[^}]*?return\s+_T\("([^"]+\.xml)"\)',
                               text)
        if not skin_match:
            continue
        xml_name = skin_match.group(1)

        # Form 通常返回 "controls"；这里读取实际值，避免专用目录被误判。
        skin_folder = "controls"
        folder_match = re.search(r'GetSkinFolder\(\)\s*\{[^}]*?return\s+_T\("([^"]+)"\)',
                                 text)
        if folder_match:
            skin_folder = folder_match.group(1)

        xml_path = theme_root / theme_name / skin_folder / xml_name
        if not xml_path.is_file():
            add_failure("R9-MISSING-XML",
                        '{}: GetSkinFile returns "{}" but the file does not exist'
                        .format(path.name, xml_name))
            continue

        defined_names = set()
        for element in ET.parse(str(xml_path)).getroot().iter():
            name = element.get("name")
            if name:
                defined_names.add(name)

        # 只校验未限定对象的 FindControl(...)；menu->FindControl(...) 属于其他 XML。
        for match in re.finditer(r'(?<!->)\bFindControl\(_T\("([^"]+)"\)\)', text):
            name = match.group(1)
            CHECKS += 1
            if name not in defined_names:
                add_failure("R7-FIND-CONTROL",
                            '{}: FindControl("{}") is not present in {}'
                            .format(path.name, name, xml_name))

    # -----------------------------------------------------------------------
    # R8：画廊按钮已绑定，且绑定名称存在于 controls.xml
    # -----------------------------------------------------------------------
    control_form_file = examples_root / "ControlForm.cpp"
    if control_form_file.is_file():
        control_form_text = read_text(control_form_file)
        bound_names = set(m.group(1) for m in
                          re.finditer(r'bindOpen\(_T\("([^"]+)"\)', control_form_text))

        for name in sorted(controls_xml_names):
            if not name.startswith("btn_gallery_"):
                continue
            CHECKS += 1
            if name not in bound_names:
                add_failure("R8-GALLERY-BINDING",
                            'controls.xml: button "{}" has no bindOpen() entry in ControlForm.cpp'
                            .format(name))

        for name in sorted(bound_names):
            CHECKS += 1
            if name not in controls_xml_names:
                add_failure("R8-GALLERY-BINDING",
                            'ControlForm.cpp: bindOpen("{}") is not present in controls.xml'
                            .format(name))

    # -----------------------------------------------------------------------
    # R10：框架不支持的属性（曾导致回归）
    # -----------------------------------------------------------------------
    forbidden_attributes = ("expanded", "child_spacing")
    for path, root in documents.items():
        for element in iter_page_elements(root):
            for attribute_name in forbidden_attributes:
                CHECKS += 1
                if attribute_name in element.attrib:
                    add_failure("R10-UNSUPPORTED-ATTR",
                                '{}: <{}> uses unsupported attribute "{}"'
                                .format(path.name, element.tag, attribute_name))

    # -----------------------------------------------------------------------
    # R11："标签 + 线条"行必须给文字留出足够高度与宽度
    # -----------------------------------------------------------------------
    test_page = pages_root / "test.xml"
    line_row_count = 0
    if test_page in documents:
        for element in iter_page_elements(documents[test_page]):
            if element.tag != "HBox":
                continue
            lines = [child for child in element if child.tag == "Line"]
            labels = [child for child in element if child.tag == "Label"]
            if not lines or not labels:
                continue

            line_row_count += 1
            font_sizes = [size for size in
                          (parse_font_size(label.get("font")) for label in labels)
                          if size]
            # 行高至少要比字体的行高大 6 DIP，否则文字会被纵向裁切
            min_height = max(20, (max(font_sizes) if font_sizes else 12) + 6)
            row_height = to_int(element.get("height"))
            CHECKS += 1
            if row_height is None or row_height < min_height:
                add_failure("R11-LINE-LABEL",
                            "test.xml: <{}> row with Label+Line has height={}, "
                            "expected >= {}".format(element.tag, row_height, min_height))

            for label in labels:
                CHECKS += 1
                label_width = to_int(label.get("width"))
                if label_width is None or label_width < 80:
                    add_failure("R11-LINE-LABEL",
                                'test.xml: label "{}" before a Line has width={}, '
                                'expected >= 80'.format(label.get("text"), label_width))
        if line_row_count == 0:
            add_failure("R11-LINE-LABEL",
                        "test.xml: no Label+Line row found; the line samples are not covered")
    else:
        add_failure("R11-LINE-LABEL", "test.xml not found below {}".format(pages_root))

    # -----------------------------------------------------------------------
    # R12：多行 RichEdit 必须展示垂直滚动条，且示例文本足够长
    # -----------------------------------------------------------------------
    rich_page = pages_root / "rich_edit.xml"
    rich_edit_count = 0
    if rich_page in documents:
        for element in iter_page_elements(documents[rich_page]):
            if element.tag not in ("RichEdit", "RichEdit2"):
                continue
            if not is_true(element.get("multi_line")):
                continue

            rich_edit_count += 1
            CHECKS += 1
            if not is_true(element.get("auto_vscroll")):
                add_failure("R12-RICHEDIT-SCROLL",
                            'rich_edit.xml: "{}" is multi-line but auto_vscroll is not true'
                            .format(element.get("name")))
            CHECKS += 1
            if not is_true(element.get("vscrollbar")):
                add_failure("R12-RICHEDIT-SCROLL",
                            'rich_edit.xml: "{}" is multi-line but vscrollbar is not true'
                            .format(element.get("name")))
        if rich_edit_count == 0:
            add_failure("R12-RICHEDIT-SCROLL",
                        "rich_edit.xml: no multi-line RichEdit control found")
    else:
        add_failure("R12-RICHEDIT-SCROLL", "rich_edit.xml not found below {}".format(pages_root))

    # 示例文本必须超出可视高度，否则滚动条不会出现
    rich_form_file = examples_root / "RichEditForm.cpp"
    required_lines = {"rich_edit_multi": 20, "rich_edit2": 15}
    if rich_form_file.is_file():
        rich_form_text = read_text(rich_form_file)
        for control_name, min_lines in required_lines.items():
            CHECKS += 1
            actual_lines = count_sample_text_lines(rich_form_text, control_name)
            if actual_lines < min_lines:
                add_failure("R12-RICHEDIT-SCROLL",
                            'RichEditForm.cpp: "{}" sample text has {} line break(s), '
                            'expected >= {}'.format(control_name, actual_lines, min_lines))
    else:
        add_failure("R12-RICHEDIT-SCROLL",
                    "RichEditForm.cpp not found below {}".format(examples_root))

    # -----------------------------------------------------------------------
    # R13：disabled_weekdays 取值合法、示例禁用周末、转发链路完整
    # -----------------------------------------------------------------------
    weekday_tokens = {
        "sunday", "sun", "monday", "mon", "tuesday", "tue", "wednesday", "wed",
        "thursday", "thu", "friday", "fri", "saturday", "sat",
        "0", "1", "2", "3", "4", "5", "6",
    }
    weekday_usages = 0
    for path, root in documents.items():
        for element in iter_page_elements(root):
            value = element.get("disabled_weekdays")
            if value is None:
                continue

            weekday_usages += 1
            tokens = [token.strip().lower() for token in re.split(r"[,;|]", value)
                      if token.strip()]
            CHECKS += 1
            if not tokens:
                add_failure("R13-WEEKDAYS",
                            '{}: "{}" has an empty disabled_weekdays value'
                            .format(path.name, element.get("name")))
            for token in tokens:
                CHECKS += 1
                if token not in weekday_tokens:
                    add_failure("R13-WEEKDAYS",
                                '{}: "{}" has an unknown weekday "{}"'
                                .format(path.name, element.get("name"), token))

    if weekday_usages == 0:
        add_failure("R13-WEEKDAYS",
                    "no page demonstrates disabled_weekdays; the weekend sample is missing")

    # "禁用周末"示例必须同时禁用周六与周日（回归：界面可选周末日期）
    datetime_page = pages_root / "datetime.xml"
    if datetime_page in documents:
        disabled_controls = [element for element in
                             iter_page_elements(documents[datetime_page])
                             if element.get("name") == "dt_disabled"]
        CHECKS += 1
        if not disabled_controls:
            add_failure("R13-WEEKDAYS",
                        'datetime.xml: control "dt_disabled" is missing')
        else:
            value = (disabled_controls[0].get("disabled_weekdays") or "").lower()
            CHECKS += 1
            if ("saturday" not in value) or ("sunday" not in value):
                add_failure("R13-WEEKDAYS",
                            'datetime.xml: "dt_disabled" should disable saturday and sunday, '
                            'got "{}"'.format(value))
    else:
        add_failure("R13-WEEKDAYS", "datetime.xml not found below {}".format(pages_root))

    # 属性从 DateTime 传到 Calendar 的链路不能中断
    forwarding_checks = [
        ("duilib/Control/CalendarWeekdayUtil.cpp", ["ParseDisabledWeekdays", "IsCalendarWeekdayDisabled"]),
        ("duilib/Control/Calendar.cpp", ["disabled_weekdays", "m_disabledWeekdaysMask"]),
        ("duilib/Control/CalendarFlyout.cpp", ["SetDisabledWeekdays"]),
        ("duilib/Control/DateTime.cpp", ["disabled_weekdays", "SetDisabledWeekdays"]),
    ]
    for relative_path, expected_tokens in forwarding_checks:
        source_file = source_root / relative_path
        if not source_file.is_file():
            add_failure("R13-WEEKDAYS", "{} not found".format(relative_path))
            continue
        source_text = read_text(source_file)
        for token in expected_tokens:
            CHECKS += 1
            if token not in source_text:
                add_failure("R13-WEEKDAYS",
                            '{}: expected "{}" to keep disabled_weekdays supported'
                            .format(relative_path, token))

    # -----------------------------------------------------------------------
    # R14：Mask 高亮目标必须存在；跨滚动容器时必须补偿滚动偏移
    # -----------------------------------------------------------------------
    scroll_containers = {"ScrollBox", "VScrollBox", "HScrollBox"}
    mask_usages = 0
    needs_scroll_compensation = False
    for path, root in documents.items():
        parents = build_parent_map(root)
        named_elements = {element.get("name"): element
                          for element in root.iter() if element.get("name")}
        for element in root.iter():
            if element.tag != "Mask":
                continue
            target_name = element.get("highlight_control")
            if not target_name:
                continue

            mask_usages += 1
            CHECKS += 1
            target = named_elements.get(target_name)
            if target is None:
                add_failure("R14-MASK-HIGHLIGHT",
                            '{}: highlight_control "{}" does not exist in this page'
                            .format(path.name, target_name))
                continue
            if (is_inside(target, parents, scroll_containers) and
                    not is_inside(element, parents, scroll_containers)):
                needs_scroll_compensation = True

    if mask_usages == 0:
        add_failure("R14-MASK-HIGHLIGHT",
                    "no Mask with highlight_control found; the guide sample is not covered")

    if needs_scroll_compensation:
        mask_source = source_root / "duilib" / "Control" / "Mask.cpp"
        if not mask_source.is_file():
            add_failure("R14-MASK-HIGHLIGHT", "duilib/Control/Mask.cpp not found")
        else:
            CHECKS += 1
            if "GetScrollOffsetForChild" not in read_text(mask_source):
                add_failure("R14-MASK-HIGHLIGHT",
                            "Mask.cpp: highlight target is inside a scroll container, "
                            "but the scroll offset is not compensated")

    # -----------------------------------------------------------------------
    # R15：UI 文本属性禁止硬编码中文（必须使用 *_id，才能跟随语言切换）
    # -----------------------------------------------------------------------
    hardcoded_text_attributes = ("text", "prompt_text", "title",
                                 "x_axis_title", "y_axis_title", "tooltip_text")
    cjk_pattern = re.compile(r"[\u4e00-\u9fff]")
    for path, root in all_documents:
        for element in iter_page_elements(root):
            for attr_name in hardcoded_text_attributes:
                attr_value = element.get(attr_name)
                if not attr_value or not cjk_pattern.search(attr_value):
                    continue
                CHECKS += 1
                add_failure("R15-HARDCODED-TEXT",
                            '{}: <{}> attribute {}="{}" hardcodes Chinese; '
                            "use the matching *_id attribute instead"
                            .format(path.name, element.tag, attr_name, attr_value))

    # -----------------------------------------------------------------------
    # 汇总
    # -----------------------------------------------------------------------
    print("controls page static validation")
    print("  source root : {}".format(source_root))
    print("  theme       : {}".format(theme_name))
    print("  pages       : {} XML file(s)".format(len(documents)))
    print("  checks      : {}".format(CHECKS))
    print("  classes     : {} defined".format(len(defined_classes)))
    print("  colors      : {} defined".format(len(defined_colors)))

    if not FAILURES:
        print("PASS: all checks succeeded")
        return 0

    print("FAIL: {} issue(s) found".format(len(FAILURES)))
    for failure in FAILURES:
        print("  " + failure)
    return 1


if __name__ == "__main__":
    sys.exit(main())
