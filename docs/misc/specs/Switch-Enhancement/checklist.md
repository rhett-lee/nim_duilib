# Switch 控件增强验收清单

- 规格：[spec.md](./spec.md)
- 任务：[tasks.md](./tasks.md)

## 控件实现

- [x] `Switch.h` 提供文字/图标/加载/禁用相关公开接口与成员
- [x] `Switch.cpp` 解析 10 个新 XML 属性（含紧凑别名）
- [x] 轨道内文字/图标按动画进度交叉淡入淡出，图标与文字组合时水平居中
- [x] 加载中滑块内绘制 8 点旋转指示器，用户点击被拦截，程序调用 `Selected` 仍有效
- [x] 禁用态使用专用轨道/滑块颜色，滑块位置不做动画插值
- [x] `ChangeDpiScale` 重新加载轨道内图标

## 资源与示例

- [x] `global.xml` 新增 `switch_small` 类（40x20）
- [x] 新增 4 个演示图标 SVG（sun / moon / close / sad），仅使用 fill
- [x] `controls/switch.xml` 覆盖基本、禁用、文字图标、两种大小、加载、自定义、事件回显
- [x] 演示页引用的语义色、字体、Class 均存在于主题定义中
- [x] 24 个语言 ID 在中英文资源中均存在
- [x] Switch 入口按钮与 `SwitchForm` 已登记到工程并编译进示例程序

## 文档

- [x] `docs/Control.md` / `docs/Control.en.md` 补充属性、接口、XML/C++ 示例与 `switch_small`
- [x] 三份 Skill 参考文档的 Switch 属性表/接口表同步
- [x] `verify_docs.py` 对本次新增内容无 Switch 相关漂移报告

## 构建与回归

- [x] Debug|x64 编译通过，生成 `bin/controls_d.exe`
- [x] 新属性默认值不改变未配置文字/图标的 Switch 绘制结果（10 个属性均有默认值或空值）
- [x] 原有 `switch` / `switch_fluent` / `switch_material` 样式定义未被修改
