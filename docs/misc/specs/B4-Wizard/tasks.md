# Tasks: B4 Stepper 向导步骤条控件

> 前置：[spec.md](./spec.md) 已评审通过。按顺序执行；每步完成即更新状态。

## 任务清单

- [x] T1 新增控件源码 `duilib/Control/Stepper.h` / `.cpp`
  - 继承 `Control`；步骤列表 + 当前索引 + 禁用集合派生 4 态（pending/active/completed/disabled）。
  - 覆写 `GetType / SetAttribute / PaintStateImages / OnLanguageChanged / ButtonDown`。
  - 实现 API、事件编码（`kEventValueChanged` + `pEventData=&StepValue`）、`*NoEvent` 静默赋值守卫。
  - 水平/垂直两套布局与绘制；序号/对勾节点；连接线按完成度着色。
  - 验证：单文件语义自查（钳位、事件仅在实际变化时派发、禁用步骤不可跳转）。

- [x] T2 控件注册（4 处）
  - `duilib_defs.h`：`DUI_CTR_STEPPER`。
  - `WindowBuilder.cpp`：include + 工厂 map 项。
  - `duilib.vcxproj` + `duilib.vcxproj.filters`：ClCompile/ClInclude + `Filter=Control`。
  - `duilib.h`：include。

- [x] T3 示例窗口与资源
  - `examples/controls/StepperForm.h/.cpp`。
  - `bin/resources/themes/default/controls/stepper.xml`（水平 + 垂直 + 事件标签 + 按钮）。
  - `test.xml`：新增 `stepper_btn`。
  - 语言资源 zh_CN / en_US 新增 STRID_*。
  - `controls.vcxproj` + `.filters`：登记新文件。
  - `TestForm.h/.cpp`：`ShowStepperWindow()` 绑定按钮。

- [x] T4 编译 Debug|x64
  - `MSBuild "c:\develop\nim_duilib\build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly`
  - 修复至 `/clp:ErrorsOnly` 无输出。

- [x] T5 运行时核对
  - 重建 DPI-aware 截图/点击辅助脚本；打开示例窗口截图。
  - 点击下一步/上一步/跳转/重置、直接点节点，核对状态与事件标签。

- [x] T6 文档同步 + 漂移校验 + 盘点表/计划标记
  - `docs/Control.md/.en.md`、`docs/Summary.md/.en.md`、`README.md/.en.md`、两份 `api-reference.md`、`llm-reference.md`。
  - 运行 `verify_docs.py --extra <改动 md>`。
  - `docs/misc/Project-Audit.md` B4 → ✅ 已完成；`docs/misc/Development-Plan.md` D-6 勾选。

## 增量任务（2026-10-19）：节点 hover / 按下态 SVG 图标

- [x] T7 新增节点 hover / 按下态 SVG 图标
  - 新增 `bin/resources/themes/default/public/stepper/stepper_node_hovered.svg`、`stepper_node_pressed.svg`。
  - `global.xml`：新增 `PATH_STEPPER` 变量与 `stepper_node` Class。
  - `Stepper.h/.cpp`：`StateImage m_nodeStateImage` + `SetNodeStateImage/GetNodeStateImage`；覆写 `MouseMove/MouseLeave/ButtonUp/ClearImageCache`；`ButtonDown` 记录按下步骤；`PaintNodeStateIcon` 在节点下层绘制；`SetAttribute` 支持 `node_hovered_image` / `node_pressed_image`（含别名）。
  - `stepper.xml`：两个 `<Stepper>` 应用 `class="stepper_node"`。
  - 验证：单文件语义自查（仅可交互步骤绘制、pressed 回退 hovered、未配置时行为不变）。

- [x] T8 增量编译 + 运行时核对 + 文档同步
  - `MSBuild ... /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly` 无错误。
  - 截图核对：悬停出现光晕环、按下加深、移出消失。
  - `docs/Control.md/.en.md`、两份 `api-reference.md`、`llm-reference.md` 同步；`verify_docs.py` 无漂移。

## 完成定义（DoD）

- Debug|x64 编译无错误。
- XML 可创建 `Stepper`、4 态绘制/跳转正确、`AttachStepChanged` 回调正确。
- 节点 hover / 按下态 SVG 图标生效且未配置时行为不变。
- 文档无漂移、Project-Audit B4 与 Development-Plan D-6 已标记完成。

## 约束

- 不执行 git 提交（用户手动）。
- 不编译 Release。
