#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
主题XML颜色同步工具
功能：按行对应同步两个结构完全相同的XML文件中指定节点的属性
规则：
    1. 两个文件 ThemeColor 节点行数必须一致
    2. 相同行的 ThemeColor name 属性必须完全一致
    3. 同步指定属性值（如 value）
用法：
    python sync_color_xml.py 源XML路径 目标XML路径 同步节点路径
示例：
    python sync_color_xml.py ./color_light/global.xml ./color_dark/global.xml "ThemeColor/@value"
    python sync_color_xml.py light.xml dark.xml "ThemeColor/@comment_cn"
"""

import sys
import xml.etree.ElementTree as ET

# ===================== 最小化修复：保留注释 =====================
class CommentedTreeBuilder(ET.TreeBuilder):
    def comment(self, data):
        self.start(ET.Comment, {})
        self.data(data)
        self.end(ET.Comment)
# ===============================================================

def load_theme_color_lines(xml_path):
    """
    加载XML中所有 ThemeColor 节点，按顺序返回列表
    返回：(行号, name属性, 节点对象)
    """
    try:
        # ===================== 最小化修复：使用注释保留解析器 =====================
        parser = ET.XMLParser(target=CommentedTreeBuilder())
        tree = ET.parse(xml_path, parser)
        # =======================================================================
        root = tree.getroot()
    except Exception as e:
        print(f"❌ 解析XML失败: {xml_path}, 错误: {str(e)}")
        sys.exit(1)

    theme_colors = []
    line_num = 0
    for child in root:
        line_num += 1
        if child.tag == "ThemeColor":
            name = child.attrib.get("name", "")
            theme_colors.append((line_num, name, child))

    if not theme_colors:
        print(f"❌ 未找到任何 ThemeColor 节点: {xml_path}")
        sys.exit(1)

    return tree, root, theme_colors

def parse_sync_path(sync_path):
    """
    解析同步路径，如 ThemeColor/@value → 属性名 value
    """
    if not sync_path.startswith("ThemeColor/@"):
        print(f"❌ 同步路径格式错误，正确格式：ThemeColor/@属性名")
        print(f"示例：ThemeColor/@value 、 ThemeColor/@comment_cn")
        sys.exit(1)

    attr_name = sync_path.replace("ThemeColor/@", "")
    if not attr_name:
        print(f"❌ 未指定要同步的属性名")
        sys.exit(1)

    return attr_name

def main():
    if len(sys.argv) != 4:
        print("=" * 60)
        print("📘 主题XML同步工具使用说明")
        print("=" * 60)
        print("命令格式:")
        print("  python sync_color_xml.py 源文件 目标文件 同步路径")
        print("示例:")
        print("  python sync_color_xml.py ./color_light/global.xml ./color_dark/global.xml ThemeColor/@contrast_bg")
        print("  python sync_color_xml.py ./color_light/global.xml ./color_dark/global.xml ThemeColor/@comment_cn")
        print("规则:")
        print("  1. 两个XML必须结构完全一致")
        print("  2. 按行对应同步，相同行name必须一致，否则报错")
        print("  3. 自动保存目标文件，保留格式")
        print("=" * 60)
        sys.exit(1)

    # 读取参数
    src_xml = sys.argv[1]
    dst_xml = sys.argv[2]
    sync_path = sys.argv[3]

    # 解析要同步的属性名
    sync_attr = parse_sync_path(sync_path)

    # 加载两个文件
    print(f"🔍 加载源文件: {src_xml}")
    src_tree, src_root, src_tc_list = load_theme_color_lines(src_xml)

    print(f"🔍 加载目标文件: {dst_xml}")
    dst_tree, dst_root, dst_tc_list = load_theme_color_lines(dst_xml)

    # 校验行数一致
    if len(src_tc_list) != len(dst_tc_list):
        print(f"❌ 错误：两个文件 ThemeColor 数量不一致！")
        print(f"   源文件: {len(src_tc_list)} 个")
        print(f"   目标文件: {len(dst_tc_list)} 个")
        sys.exit(1)

    # 逐行校验 name + 同步属性
    print(f"🔄 开始同步属性: {sync_attr}")
    success_count = 0

    for idx in range(len(src_tc_list)):
        src_line, src_name, src_node = src_tc_list[idx]
        dst_line, dst_name, dst_node = dst_tc_list[idx]

        # 严格校验 name 必须相同
        if src_name != dst_name:
            print(f"\n❌ 行不匹配（同步中断）:")
            print(f"   源文件 第{src_line}行: name = {src_name}")
            print(f"   目标文件 第{dst_line}行: name = {dst_name}")
            print(f"   两个文件结构必须完全一致！")
            sys.exit(1)

        # 执行同步
        if sync_attr in src_node.attrib:
            src_val = src_node.attrib[sync_attr]
            dst_node.set(sync_attr, src_val)
            success_count += 1
        else:
            print(f"⚠️  跳过 {src_name}：源文件无属性 {sync_attr}")

    # 保存目标文件
    try:
        # ===================== 格式修复：无空格 + 双引号 =====================
        import io
        buf = io.BytesIO()
        dst_tree.write(buf, encoding="UTF-8", xml_declaration=True)
        xml_content = buf.getvalue().decode("UTF-8")
        xml_content = xml_content.replace(" />", "/>")  # 去掉 /> 前空格
        xml_content = xml_content.replace("<?xml version='1.0' encoding='UTF-8'?>", '<?xml version="1.0" encoding="UTF-8"?>')
        with open(dst_xml, "w", encoding="UTF-8", newline="") as f:
            f.write(xml_content)
        # ====================================================================

        print(f"\n✅ 同步完成！成功同步 {success_count} 个节点")
        print(f"📁 目标文件已更新: {dst_xml}")
    except Exception as e:
        print(f"❌ 保存目标文件失败: {str(e)}")
        sys.exit(1)

if __name__ == "__main__":
    main()