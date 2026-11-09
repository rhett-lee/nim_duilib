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
  R6  每个 text_id 都必须同时存在于中文和英文字符串文件中。
  R7  Form 中未限定对象的 FindControl(_T("name")) 必须能在该 Form 的
      GetSkinFile() 返回的 XML 中找到。
  R8  controls.xml 中的每个 btn_gallery_* 按钮都必须在 ControlForm.cpp
      中通过 bindOpen() 绑定；反之每个 bindOpen() 名称也必须存在于
      controls.xml。
  R9  Form 的 GetSkinFile() 返回的 XML 文件必须存在。
  R10 拒绝框架不支持的属性（expanded=、child_spacing=），
      这两个属性曾导致控件测试页出现真实回归。

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
    # R6：text_id 必须同时存在于中文和英文语言文件
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

    for path, root in documents.items():
        for element in root.iter():
            text_id = element.get("text_id")
            if not text_id:
                continue
            CHECKS += 1
            if text_id not in zh_keys:
                add_failure("R6-MISSING-TEXT-ID",
                            '{}: text_id "{}" is missing from the Chinese language files'
                            .format(path.name, text_id))
            if text_id not in en_keys:
                add_failure("R6-MISSING-TEXT-ID",
                            '{}: text_id "{}" is missing from the English language files'
                            .format(path.name, text_id))

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
