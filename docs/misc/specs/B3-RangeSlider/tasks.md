# Tasks: B3 RangeSlider 双滑块范围控件

> 前置：[spec.md](./spec.md) 已评审通过。按顺序执行；每步完成即更新状态。

## 任务清单

- [x] T1 新增控件源码 `duilib/Control/RangeSlider.h` / `.cpp`
  - 继承 `Progress`；双值模型（下限 `m_fLowValue` + 复用基类当前值作上限）。
  - 覆写 `GetType / SetAttribute / HandleEvent / PaintStateImages / ClearImageCache / ChangeDpiScale`。
  - 实现 API、事件编码（`kEventValueChanged` + `pEventData=&RangeValue`）、`*NoEvent` 静默赋值守卫。
  - 验证：单文件语义自查（钳位、事件仅在实际变化时派发）。

- [x] T2 控件注册（4 处）
  - `duilib_defs.h`：`DUI_CTR_RANGE_SLIDER`。
  - `WindowBuilder.cpp`：include + 工厂 map 项。
  - `duilib.vcxproj` + `duilib.vcxproj.filters`：ClCompile/ClInclude + `Filter=Control`。
  - `duilib.h`：include。

- [x] T3 示例窗口与资源
  - `examples/controls/RangeSliderForm.h/.cpp`。
  - `bin/resources/themes/default/controls/rangeslider.xml`。
  - `controls.xml`：新增 `rangeslider_btn` + 首排 `HBox height` 288→320。
  - 语言资源 zh_CN / en_US 新增 STRID_*。
  - `controls.vcxproj` + `.filters`：登记新文件。
  - `ControlForm.h/.cpp`：`ShowRangeSliderWindow()` 绑定按钮。

- [x] T4 编译 Debug|x64
  - `MSBuild "c:\develop\nim_duilib\build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly`
  - 修复至 `/clp:ErrorsOnly` 无输出。

- [x] T5 运行时核对
  - 重建 DPI-aware 截图/点击辅助脚本；打开示例窗口截图。
  - 拖动双 thumb、点击重置/全选按钮，核对高亮区间与 `rs_event` 标签。

- [x] T6 文档同步 + 漂移校验 + 盘点表标记
  - `docs/Control.md/.en.md`、`docs/Summary.md/.en.md`、`README.md/.en.md`、两份 `api-reference.md`、`llm-reference.md`。
  - 运行 `verify_docs.py --extra <改动 md>`。
  - `docs/misc/Project-Audit.md` B3 → ✅ 已完成。

## 完成定义（DoD）

- Debug|x64 编译无错误。
- XML 可创建 `RangeSlider`、双 thumb 可拖动、`AttachRangeChanged` 回调正确。
- 文档无漂移、Project-Audit B3 已标记完成。

## 约束

- 不执行 git 提交（用户手动）。
- 不编译 Release。
