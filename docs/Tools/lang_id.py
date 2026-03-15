import os
import re
import xml.etree.ElementTree as ET
from typing import Dict, Set
# 函数功能：将XML文件中的中文字符串提取为语言ID，从而支持多语言版
# 每次使用时，需要修改XML_DIR和ID_PREFIX两个变量。

# -------------------------- 配置项 --------------------------
# 待处理XML文件目录
XML_DIR = "./layout"
# 输出的多语言映射文件路径
MAP_FILE = "./language_map.txt"
# ID前缀（按规则调整）
ID_PREFIX = "STRID_LAYOUT_PAGE_"
# 匹配text/prompt_text/title属性的正则（兼容空格、引号格式）
TEXT_PATTERN = re.compile(r'(text|prompt_text|title)\s*=\s*["\'](.*?)["\']', re.UNICODE)
# 检测是否含中文字符的正则
CHINESE_PATTERN = re.compile(r'[\u4e00-\u9fff]+')
# 提取ID后缀数字的正则（用于排序）
ID_SUFFIX_PATTERN = re.compile(rf'{re.escape(ID_PREFIX)}(\d+)')

# -------------------------- 核心逻辑 --------------------------
# 缓存：中文字符串 -> ID（保证相同文字ID一致）
text_to_id: Dict[str, str] = {}
# 记录已生成的ID后缀，避免重复
id_suffix_set: Set[int] = set()
# 自增ID后缀（初始值）
next_suffix = 1


def is_contain_chinese(text: str) -> bool:
    """检测字符串是否包含中文字符"""
    return bool(CHINESE_PATTERN.search(text))


def get_text_id(text: str) -> str:
    """根据中文字符串生成唯一ID，相同文字返回相同ID"""
    global next_suffix
    if text in text_to_id:
        return text_to_id[text]
    
    # 生成新ID（后缀自增）
    while next_suffix in id_suffix_set:
        next_suffix += 1
    new_id = f"{ID_PREFIX}{next_suffix}"
    text_to_id[text] = new_id
    id_suffix_set.add(next_suffix)
    next_suffix += 1
    return new_id


def extract_id_suffix(text_id: str) -> int:
    """提取ID中的数字后缀，用于排序（如STRID_LAYOUT_12 → 12）"""
    match = ID_SUFFIX_PATTERN.search(text_id)
    return int(match.group(1)) if match else 0


def process_xml_file(file_path: str) -> None:
    """处理单个XML文件"""
    # 读取文件内容（保留原编码，建议UTF-8）
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 匹配所有text/prompt_text/title属性
    matches = TEXT_PATTERN.findall(content)
    if not matches:
        return
    
    # 遍历匹配结果，替换属性
    for attr_name, text_content in matches:
        # 过滤纯英文/数字的内容
        if not is_contain_chinese(text_content):
            continue
        
        # 生成ID
        text_id = get_text_id(text_content)
        
        # 构造替换规则：text="文字"→text_id="ID"，title="文字"→title_id="ID"
        old_str = f'{attr_name}="{text_content}"'
        new_attr_name = f'{attr_name}_id'
        new_str = f'{new_attr_name}="{text_id}"'
        
        # 替换内容（全局替换，保证相同文字都替换）
        content = content.replace(old_str, new_str)
    
    # 写回修改后的内容
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"处理完成：{file_path}")


def save_language_map() -> None:
    """保存中文字符串与ID的映射表（按ID后缀数字排序）"""
    with open(MAP_FILE, 'w', encoding='utf-8') as f:
        f.write("# 中文字符串 → ID 映射表\n")
        f.write("# 格式：ID = 中文字符串\n\n")
        # 按ID后缀数字排序（而非文本排序），保证生成顺序
        sorted_items = sorted(
            text_to_id.items(),
            key=lambda x: extract_id_suffix(x[1])  # 按ID后缀数字升序
        )
        for text, text_id in sorted_items:
            f.write(f'{text_id} = {text}\n')
    print(f"映射表已保存：{MAP_FILE}")


def main():
    """主入口：批量处理XML目录"""
    # 检查目录是否存在
    if not os.path.exists(XML_DIR):
        print(f"错误：目录 {XML_DIR} 不存在！")
        return
    
    # 遍历所有XML文件（递归处理子目录，兼容大写后缀）
    for root, dirs, files in os.walk(XML_DIR):
        for file in files:
            if file.lower().endswith(".xml"):
                file_path = os.path.join(root, file)
                process_xml_file(file_path)
    
    # 保存映射表
    save_language_map()
    print("所有XML文件处理完成！")


if __name__ == "__main__":
    main()