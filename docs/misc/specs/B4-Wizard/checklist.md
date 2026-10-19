# Checklist: B4 Stepper

## 源码
- [x] `Stepper.h` 类声明与 API 完整（步骤列表 + 当前索引 + 禁用 + 方向 + 颜色 + 字体 + 事件）。
- [x] `Stepper.cpp` 覆写方法齐全且 `GetType()` 返回 `DUI_CTR_STEPPER`。
- [x] 状态派生正确：禁用 > （index < current → completed / == → active / > → pending）。
- [x] 当前索引钳位到 `[0, count-1]`；空列表安全。
- [x] 事件仅在实际变化时派发，`pEventData` 携带 `StepValue{currentIndex, oldIndex}`。
- [x] `SetCurrentIndexNoEvent / GoToNoEvent / NextNoEvent / BackNoEvent` 不派发事件。
- [x] 水平与垂直两种方向的布局/绘制/命中公式均正确。
- [x] 禁用步骤不可通过 `GoTo` / 点击跳转。
- [x] 多语言 `step_title_id` / `step_desc_id` 语言切换自动刷新。

## 注册
- [x] `duilib_defs.h` 宏。
- [x] `WindowBuilder.cpp` include + 工厂。
- [x] `duilib.vcxproj` / `.filters`。
- [x] `duilib.h` include。

## 示例
- [x] `StepperForm.h/.cpp` 存在且 `GetSkinFolder/GetSkinFile` 正确。
- [x] `stepper.xml` 可加载，含水平/垂直示例与操作按钮。
- [x] `test.xml` 新增 `stepper_btn`。
- [x] 中英语言字符串均已补齐。
- [x] `controls.vcxproj` / `.filters` 已登记。
- [x] `TestForm` 按钮绑定与 `ShowStepperWindow()` 实现。

## 验证
- [x] Debug|x64 `/clp:ErrorsOnly` 无错误。
- [x] 运行截图：水平/垂直步骤条、节点状态与连接线显示正确。
- [x] 按钮与节点点击交互后事件标签正确更新。
- [x] `verify_docs.py` 无漂移。
- [x] Project-Audit B4 已标记 ✅；Development-Plan D-6 已勾选。

## 增量：节点 hover / 按下态 SVG 图标
- [x] 两份 SVG 资源存在，`stepper_node` Class 正确引用（含 `svg_replace_colors`）。
- [x] `node_hovered_image` / `node_pressed_image` 属性生效（含 `_hot_` / `_pushed_` 别名）。
- [x] 悬停/按下态图标仅对可交互步骤显示；禁用步骤与 `click_to_jump=false` 时不显示。
- [x] pressed 未配置时回退 hovered；均未配置时行为与旧版一致。
- [x] `MouseMove` / `MouseLeave` / `ButtonUp` 覆写且不破坏原有点击跳转与事件。
- [x] `ClearImageCache()` 释放图标缓存。
- [x] Debug|x64 编译无错误、截图核对 hover/pressed 生效。
- [x] 文档同步且 `verify_docs.py` 无漂移。
