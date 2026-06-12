import os
import sys
from pathlib import Path

# 脚本功能：
# 1. 从一个文本文件中，按行读取输入文本字符串，每行的格式为A,B，含A和B两个字符串，用逗号分割
# 2. 从一个目录中读取指定类型的文件，后缀名为.cpp;.h;.xml;*.txt需要递归遍历子目录查找文件
# 3. 将搜索到的文件中的指定字符串A，全部替换为字符串B，保存结果覆盖原来的文件即可。

def load_replacement_rules(rule_file_path):
    """
    从规则文件加载替换规则
    参数:
        rule_file_path: 规则文件路径（每行格式为 A,B）
    返回:
        替换规则字典 {A: B}
    """
    replacement_rules = {}
    
    # 检查规则文件是否存在
    if not os.path.exists(rule_file_path):
        print(f"错误：规则文件 {rule_file_path} 不存在！")
        sys.exit(1)
    
    try:
        with open(rule_file_path, 'r', encoding='utf-8') as f:
            for line_num, line in enumerate(f, 1):
                # 去除行首尾空白字符
                line = line.strip()
                # 跳过空行和注释行
                if not line or line.startswith('#'):
                    continue
                
                # 按逗号分割，只分割一次，避免B中包含逗号的情况
                parts = line.split(',', 1)
                if len(parts) != 2:
                    print(f"警告：第 {line_num} 行格式错误（应为 A,B），已跳过该行")
                    continue
                
                old_str = parts[0].strip()
                new_str = parts[1].strip()
                
                # 跳过空的替换规则
                if not old_str:
                    print(f"警告：第 {line_num} 行的替换源字符串为空，已跳过该行")
                    continue
                
                replacement_rules[old_str] = new_str
        
        if not replacement_rules:
            print("错误：未加载到有效的替换规则！")
            sys.exit(1)
            
        print(f"成功加载 {len(replacement_rules)} 条替换规则")
        return replacement_rules
        
    except Exception as e:
        print(f"读取规则文件时出错：{str(e)}")
        sys.exit(1)

def replace_in_file(file_path, replacement_rules):
    """
    在单个文件中执行字符串替换并覆盖保存
    参数:
        file_path: 文件路径
        replacement_rules: 替换规则字典 {A: B}
    """
    try:
        # 读取文件内容
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 记录是否有替换发生
        replaced = False
        original_content = content
        
        # 执行替换
        for old_str, new_str in replacement_rules.items():
            if old_str in content:
                content = content.replace(old_str, new_str)
                replaced = True
        
        # 只有发生替换时才写入文件
        if replaced:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"已处理并替换：{file_path}")
        else:
            print(f"无匹配内容，跳过：{file_path}")
            
    except UnicodeDecodeError:
        # 处理非UTF-8编码的文件
        try:
            with open(file_path, 'r', encoding='gbk') as f:
                content = f.read()
            
            replaced = False
            original_content = content
            for old_str, new_str in replacement_rules.items():
                if old_str in content:
                    content = content.replace(old_str, new_str)
                    replaced = True
            
            if replaced:
                with open(file_path, 'w', encoding='gbk') as f:
                    f.write(content)
                print(f"已处理(GBK编码)并替换：{file_path}")
            else:
                print(f"无匹配内容，跳过：{file_path}")
        except Exception as e:
            print(f"处理文件 {file_path} 时出错（编码问题）：{str(e)}")
    except Exception as e:
        print(f"处理文件 {file_path} 时出错：{str(e)}")

def scan_and_replace(root_dir, replacement_rules):
    """
    递归扫描指定目录下的指定类型文件并执行替换
    参数:
        root_dir: 根目录路径
        replacement_rules: 替换规则字典
    """
    # 检查目录是否存在
    if not os.path.isdir(root_dir):
        print(f"错误：目录 {root_dir} 不存在！")
        sys.exit(1)
    
    # 要处理的文件后缀
    target_extensions = ('.cpp', '.h', '.xml', '.txt', '.html', '.htm')
    
    print(f"\n开始扫描目录：{root_dir}")
    print(f"将处理的文件类型：{', '.join(target_extensions)}")
    print("-" * 50)
    
    # 递归遍历目录
    file_count = 0
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            # 检查文件后缀
            if file.lower().endswith(target_extensions):
                file_path = os.path.join(root, file)
                replace_in_file(file_path, replacement_rules)
                file_count += 1
    
    print("-" * 50)
    print(f"\n扫描完成！共处理 {file_count} 个文件")

def main():
    """
    主函数：处理命令行参数并执行替换流程
    使用示例:
        python replace_id.py 替换规则文件.csv 目标目录
    """
    # 检查命令行参数
    if len(sys.argv) != 3:
        print("用法：python replace_id.py <替换规则文件路径> <目标目录路径>")
        print("示例：python replace_id.py rules.csv D:/project/src")
        sys.exit(1)
    
    rule_file = sys.argv[1]
    target_dir = sys.argv[2]
    
    # 加载替换规则
    print("=== 加载替换规则 ===")
    replacement_rules = load_replacement_rules(rule_file)
    
    # 执行扫描和替换
    print("\n=== 开始文件替换 ===")
    scan_and_replace(target_dir, replacement_rules)
    
    print("\n=== 所有操作完成 ===")

if __name__ == "__main__":
    # 增加安全提示
    print("警告：该脚本会直接覆盖文件内容，请确保已备份重要文件！")
    confirm = input("是否继续执行？(y/n): ")
    if confirm.lower() == 'y':
        main()
    else:
        print("操作已取消")
        sys.exit(0)