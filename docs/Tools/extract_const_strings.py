import os
import sys
import re
import datetime  # 全局导入datetime，确保所有位置都能使用
from pathlib import Path
from typing import Tuple, List, Dict, Set

def init_logger():
    """初始化简单的日志输出格式"""
    print(f"[{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] 开始执行字符串提取脚本\n")

def remove_block_comments(content: str) -> str:
    """
    移除整个文件中的/* */块注释（处理跨行块注释）
    参数:
        content: 原始文件内容
    返回:
        移除块注释后的内容
    """
    # 正则匹配/* */块注释（包括跨行），非贪婪匹配
    block_comment_pattern = re.compile(r'/\*.*?\*/', re.DOTALL)
    # 替换所有块注释为空字符串
    cleaned_content = block_comment_pattern.sub('', content)
    return cleaned_content

def process_line_comments(line: str) -> Tuple[str, bool]:
    """
    处理单行//注释，返回非注释内容和是否为纯注释行
    参数:
        line: 原始行内容
    返回:
        (非注释内容, 是否是纯注释行)
    """
    stripped_line = line.lstrip()
    # 纯注释行（行首就是//）
    if stripped_line.startswith('//'):
        return "", True
    
    # 行内注释，保留注释前的内容
    comment_index = line.find('//')
    if comment_index != -1:
        return line[:comment_index], False
    
    # 无单行注释
    return line, False

def is_excluded_string(string_content: str, line_context: str) -> bool:
    """
    排除非业务字符串：#include/extern "C"等
    """
    stripped_line = line_context.strip()
    
    # 排除#include后的头文件字符串
    if stripped_line.startswith('#include'):
        if (string_content in stripped_line and 
            (stripped_line.find(f'"{string_content}"') != -1 or 
             stripped_line.find(f'<{string_content}>') != -1)):
            return True
    
    # 排除extern "C"中的"C"
    if stripped_line.startswith('extern "C"') and string_content == "C":
        return True
    
    return False

def safe_contains_chinese(text: str) -> bool:
    """
    安全检测中文（兼容原始中文、Unicode转义中文）
    """
    # 匹配原始中文字符
    chinese_char_pattern = re.compile(r'[\u4e00-\u9fff]')
    if chinese_char_pattern.search(text):
        return True
    
    # 匹配Unicode转义的中文（\u4e2d 或 \\u4e2d）
    unicode_escape_pattern = re.compile(r'\\u[0-9a-fA-F]{4}')
    unicode_escapes = unicode_escape_pattern.findall(text)
    for esc in unicode_escapes:
        try:
            code_point = int(esc.lstrip('\\u'), 16)
            if 0x4e00 <= code_point <= 0x9fff:
                return True
        except (ValueError, IndexError):
            continue
    
    return False

def extract_strings_from_line(line: str) -> List[str]:
    """
    从单行非注释内容中提取字符串（优化性能，独立函数）
    """
    strings = []
    quote_chars = ('"', "'")
    current_pos = 0
    line_length = len(line)
    
    while current_pos < line_length:
        # 查找未被转义的引号
        quote_pos = -1
        current_quote = ''
        
        # 遍历两种引号，找到最近的未转义引号
        for q in quote_chars:
            pos = current_pos
            while pos < line_length:
                if line[pos] == q:
                    # 检查是否被转义
                    if pos > 0 and line[pos-1] == '\\':
                        pos += 1
                        continue
                    quote_pos = pos
                    current_quote = q
                    break
                pos += 1
            if quote_pos != -1:
                break
        
        if quote_pos == -1:
            break  # 无更多引号
        
        # 查找闭合的引号
        end_quote_pos = -1
        search_pos = quote_pos + 1
        while search_pos < line_length:
            if line[search_pos] == current_quote:
                # 跳过转义的引号
                if search_pos > 0 and line[search_pos-1] == '\\':
                    search_pos += 1
                    continue
                end_quote_pos = search_pos
                break
            search_pos += 1
        
        if end_quote_pos == -1:
            current_pos = quote_pos + 1
            continue
        
        # 提取并过滤空字符串
        string_content = line[quote_pos+1 : end_quote_pos]
        if string_content.strip():
            strings.append(string_content)
        
        current_pos = end_quote_pos + 1
    
    return strings

def extract_strings_from_file(file_path: str, root_dir: str) -> Tuple[List[str], List[str]]:
    """
    从单个文件提取字符串，返回(全量字符串列表, 中文字符串列表)
    优化点：分阶段处理，提升可读性和性能
    """
    all_strings = []
    chinese_strings = []
    
    try:
        # 1. 二进制读取，避免编码问题
        with open(file_path, 'rb') as f:
            raw_content = f.read()
        
        # 2. 智能解码（优先UTF-8，兼容GBK）
        content = ""
        for encoding in ['utf-8', 'gbk', 'utf-8-sig']:
            try:
                content = raw_content.decode(encoding)
                break
            except UnicodeDecodeError:
                continue
        else:
            content = raw_content.decode('utf-8', errors='ignore')
        
        # 3. 先移除所有/* */块注释
        content = remove_block_comments(content)
        
        # 4. 按行处理单行注释和提取字符串
        lines = content.splitlines()
        for line in lines:
            # 处理单行//注释
            non_comment_line, is_comment_line = process_line_comments(line)
            if is_comment_line:
                continue
            
            # 提取当前行的字符串
            line_strings = extract_strings_from_line(non_comment_line)
            
            # 过滤排除项并分类
            for s in line_strings:
                if is_excluded_string(s, non_comment_line):
                    continue
                
                all_strings.append(s)
                if safe_contains_chinese(s):
                    chinese_strings.append(s)
        
        return all_strings, chinese_strings
        
    except Exception as e:
        print(f"⚠️  处理文件失败 {file_path}: {str(e)}")
        return [], []

def get_excluded_dirs_set(root_dir: str, exclude_dirs_str: str) -> Set[str]:
    """
    解析排除目录字符串，转换为绝对路径集合（统一分隔符）
    参数:
        root_dir: 根目录
        exclude_dirs_str: 分号分隔的排除目录（如 "duilib\third_party;duilib\third_party2"）
    返回:
        排除目录的绝对路径集合（统一用/分隔，末尾加/避免部分匹配）
    """
    excluded_dirs = set()
    if not exclude_dirs_str:
        return excluded_dirs
    
    # 分割排除目录并处理
    exclude_list = [d.strip() for d in exclude_dirs_str.split(';') if d.strip()]
    for rel_dir in exclude_list:
        # 拼接绝对路径
        abs_dir = os.path.abspath(os.path.join(root_dir, rel_dir))
        # 标准化路径 + 统一分隔符为/ + 末尾强制加/（避免部分匹配，如test和test1）
        norm_dir = os.path.normpath(abs_dir).replace(os.sep, '/').rstrip('/') + '/'
        excluded_dirs.add(norm_dir)
        print(f"🔍 解析排除目录：{rel_dir} → {norm_dir}")  # 调试信息，确认解析结果
    
    return excluded_dirs

def is_dir_excluded(current_dir: str, excluded_dirs: Set[str]) -> bool:
    """
    判断当前目录是否在排除列表中（包括子目录）
    参数:
        current_dir: 当前目录的绝对路径
        excluded_dirs: 排除目录的绝对路径集合
    返回:
        True=排除，False=保留
    """
    # 标准化当前目录：绝对路径 + 统一/分隔符 + 末尾加/
    norm_current = os.path.normpath(os.path.abspath(current_dir)).replace(os.sep, '/').rstrip('/') + '/'
    
    # 检查是否匹配排除目录
    for excluded in excluded_dirs:
        if norm_current.startswith(excluded):
            return True
    return False

def scan_directory_for_strings(root_dir: str, output_all: str, output_chinese: str, exclude_dirs_str: str = "") -> None:
    """
    递归扫描目录，优化点：
    1. 进度可视化
    2. 批量去重排序
    3. 更清晰的统计输出
    4. 支持排除指定子目录（修复核心逻辑）
    """
    if not os.path.isdir(root_dir):
        print(f"❌ 错误：目录 {root_dir} 不存在！")
        sys.exit(1)
    
    # 解析排除目录
    excluded_dirs = get_excluded_dirs_set(root_dir, exclude_dirs_str)
    if excluded_dirs:
        print(f"\n🚫 最终排除的目录集合：{', '.join(excluded_dirs)}")
    else:
        print("\nℹ️  未设置排除目录，将扫描所有子目录")
    
    target_extensions = ('.cpp', '.h')
    all_result: Dict[str, List[str]] = {}
    chinese_result: Dict[str, List[str]] = {}
    
    # ========== 第一步：统计待处理文件数（排除指定目录） ==========
    total_files = 0
    excluded_files_count = 0  # 统计被排除的文件数
    for root, dirs, files in os.walk(root_dir):
        # 关键修复：先判断当前目录是否排除，若排除则直接跳过
        if is_dir_excluded(root, excluded_dirs):
            dirs[:] = []  # 清空子目录，彻底停止递归
            excluded_files_count += len([f for f in files if f.lower().endswith(target_extensions)])
            continue
        
        # 统计有效文件
        for file in files:
            if file.lower().endswith(target_extensions):
                total_files += 1
    
    print(f"\n📁 扫描根目录：{root_dir}")
    print(f"🔍 目标文件类型：{', '.join(target_extensions)}")
    print(f"📊 预估待处理文件数：{total_files}")
    print(f"🚫 预估排除文件数：{excluded_files_count}")
    print("-" * 80)
    
    # ========== 第二步：处理文件（排除指定目录） ==========
    processed_count = 0
    for root, dirs, files in os.walk(root_dir):
        # 核心修复：优先判断当前目录是否排除
        if is_dir_excluded(root, excluded_dirs):
            dirs[:] = []  # 清空子目录列表，停止递归扫描
            print(f"⏭️  跳过排除目录：{root}")  # 调试信息，确认跳过动作
            continue
        
        for file in files:
            if file.lower().endswith(target_extensions):
                processed_count += 1
                full_path = os.path.join(root, file)
                rel_path = os.path.relpath(full_path, root_dir)
                
                # 进度显示
                progress = f"[{processed_count}/{total_files}]"
                print(f"{progress:<10} 处理中：{rel_path}")
                
                # 提取字符串
                all_strs, chinese_strs = extract_strings_from_file(full_path, root_dir)
                
                # 去重排序（优化性能：先收集再去重）
                if all_strs:
                    unique_all = sorted(list(set(all_strs)))
                    all_result[rel_path] = unique_all
                if chinese_strs:
                    unique_chinese = sorted(list(set(chinese_strs)))
                    chinese_result[rel_path] = unique_chinese
    
    print("-" * 80)
    print(f"✅ 扫描完成！实际处理文件数：{processed_count}")
    print(f"🚫 实际排除文件数：{excluded_files_count}")
    
    # ========== 写入全量文件 ==========
    try:
        with open(output_all, 'w', encoding='utf-8', newline='') as f:
            sorted_paths = sorted(all_result.keys())
            total_all = sum(len(v) for v in all_result.values())
            
            f.write("# 源码常量字符串全量列表\n")
            f.write(f"# 生成时间：{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"# 扫描根目录：{root_dir}\n")
            if excluded_dirs:
                f.write(f"# 排除目录：{', '.join(excluded_dirs)}\n")
            f.write("# ======================================\n\n")
            
            for rel_path in sorted_paths:
                strings = all_result[rel_path]
                f.write(f"=== {rel_path} ===\n")
                for s in strings:
                    f.write(f'"{s}"\n')
                f.write("\n")
        
        print(f"\n📄 全量字符串文件：{os.path.abspath(output_all)}")
        print(f"   ├─ 包含字符串的文件数：{len(all_result)}")
        print(f"   └─ 总唯一常量字符串数：{total_all}")
        
    except Exception as e:
        print(f"❌ 写入全量文件失败：{e}")
        sys.exit(1)
    
    # ========== 写入中文文件 ==========
    try:
        with open(output_chinese, 'w', encoding='utf-8', newline='') as f:
            sorted_paths = sorted(chinese_result.keys())
            total_chinese = sum(len(v) for v in chinese_result.values())
            
            f.write("# 源码含中文字符串列表\n")
            f.write(f"# 生成时间：{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"# 扫描根目录：{root_dir}\n")
            if excluded_dirs:
                f.write(f"# 排除目录：{', '.join(excluded_dirs)}\n")
            f.write("# ======================================\n\n")
            
            for rel_path in sorted_paths:
                strings = chinese_result[rel_path]
                f.write(f"=== {rel_path} ===\n")
                for s in strings:
                    f.write(f'"{s}"\n')
                f.write("\n")
        
        print(f"📄 中文字符串文件：{os.path.abspath(output_chinese)}")
        print(f"   ├─ 包含中文字符串的文件数：{len(chinese_result)}")
        print(f"   └─ 总唯一含中文字符串数：{total_chinese}")
        
    except Exception as e:
        print(f"❌ 写入中文文件失败：{e}")
        sys.exit(1)

def main():
    """主函数：参数校验和流程控制"""
    init_logger()
    
    # 支持两种参数格式：基础用法（无排除目录）、带排除目录
    if len(sys.argv) not in (4, 5):
        print("📚 用法：")
        print("  基础用法：python extract_const_strings.py <目标目录> <全量输出文件> <中文输出文件>")
        print("  带排除目录：python extract_const_strings.py <目标目录> <全量输出文件> <中文输出文件> <排除目录(分号分隔)>")
        print("🔧 示例：")
        print("  基础：python extract_const_strings.py ./ all_strings.txt chinese_strings.txt")
        print("  带排除：python extract_const_strings.py ./ all_string.txt chinese_string.txt \"duilib/third_party\"")
        sys.exit(1)
    
    target_dir = sys.argv[1]
    output_all = sys.argv[2]
    output_chinese = sys.argv[3]
    exclude_dirs_str = sys.argv[4] if len(sys.argv) == 5 else ""
    
    # 执行扫描和输出
    scan_directory_for_strings(target_dir, output_all, output_chinese, exclude_dirs_str)
    
    print("\n🎉 所有操作完成！建议检查输出文件编码为UTF-8。")

if __name__ == "__main__":
    main()