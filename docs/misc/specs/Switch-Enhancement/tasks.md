# Switch 控件增强任务清单

- 规格：[spec.md](./spec.md)
- 状态：已实施（2026-10-22）

## 1. 控件实现

- [x] 1.1 `Switch.h`：新增文字/图标/加载/禁用相关接口与成员
- [x] 1.2 `Switch.cpp`：构造函数默认值、`SetAttribute` 属性解析
- [x] 1.3 `Switch.cpp`：`GetEffectiveAnimValue`、`GetThumbRect` 禁用态处理
- [x] 1.4 `Switch.cpp`：`PaintStateColors` / `PaintStateImages` 禁用态与加载态绘制
- [x] 1.5 `Switch.cpp`：`PaintText` 轨道内文字与图标（含交叉淡入、图标+文字组合）
- [x] 1.6 `Switch.cpp`：`PaintLoading` 8 点旋转指示器 + `SetLoading` / 定时器
- [x] 1.7 `Switch.cpp`：`Activate` 加载中拦截、`ChangeDpiScale` 重载图标
- [x] 1.8 Debug|x64 编译通过（`EXIT=0`）

## 2. 资源

- [x] 2.1 `global.xml` 新增 `switch_small` 类
- [x] 2.2 新增 4 个演示图标 SVG（sun / moon / close / sad）
- [x] 2.3 新增 `controls/switch.xml` 演示页面（基本、禁用、文字图标、两种大小、加载、自定义、事件）
- [x] 2.4 `test.xml` 增加 Switch 入口按钮

## 3. 示例代码

- [x] 3.1 新增 `examples/controls/SwitchForm.h` / `SwitchForm.cpp`
- [x] 3.2 `TestForm.h` / `TestForm.cpp` 增加 `ShowSwitchWindow` 并绑定入口按钮
- [x] 3.3 `controls.vcxproj` / `controls.vcxproj.filters` 登记新源文件
- [x] 3.4 Debug|x64 编译通过（`EXIT=0`）

## 4. 语言资源

- [x] 4.1 `zh_CN-examples.txt` 新增第 9 节文案
- [x] 4.2 `en_US-examples.txt` 新增第 9 节英文文案

## 5. 文档

- [x] 5.1 `docs/Control.md` Switch 小节补充属性、接口与示例
- [x] 5.2 `docs/Control.en.md` 同步英文
- [x] 5.3 `.trae/skills/nim-duilib/references/api-reference.md` 同步
- [x] 5.4 `.workbuddy/skills/nim-duilib/references/api-reference.md` 同步
- [x] 5.5 `.claude/docs/nim-duilib-llm-reference.md` 同步
- [x] 5.6 `verify_docs.py` 通过（本次新增内容无 Switch 相关漂移；仓库其余 1047 处为既有示例 XML 的历史告警）

## 6. 验证

> 6.x 已通过 Debug|x64 构建与静态核对（XML 属性、主题资源、语言 ID、事件路径）完成；界面观感请运行示例程序确认。

- [x] 6.1 演示窗口卡片齐全（基本、禁用、文字图标、两种大小、加载、自定义、事件）
- [x] 6.2 文字/图标随切换动画淡入淡出（`PaintText` 按 `m_nAnimValue` 计算两侧透明度）
- [x] 6.3 加载指示器旋转，加载中点击不切换（`SetLoading` 定时器 + `Activate` 拦截）
- [x] 6.4 禁用态呈灰色且不可点击（`track_disabled_color` / `thumb_disabled_color` + `enabled="false"`）
- [x] 6.5 事件回显正确（`AttachSelect` / `AttachUnSelect` → `switch_event_label`）
- [x] 6.6 原有 Switch 与深色主题无回归（默认值不改变未配置文字/图标的绘制结果）
