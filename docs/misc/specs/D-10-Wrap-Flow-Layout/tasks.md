# D-10 任务清单（修订版：仅 HFlowLayout）

> 规格：[spec.md](spec.md)
> 状态：✅ 已完成（2026-10-22）

- [x] T1 `HFlowLayout.h`：新增嵌套 `JustifyContent` 枚举、`GetJustifyContent` / `SetJustifyContent`、`SetAttribute` override、成员 `m_justifyContent`
- [x] T2 `HFlowLayout.cpp`：构造函数初始化、`SetJustifyContent` 触发 `Arrange()`、`SetAttribute` 解析 `justify`/`justify_content`
- [x] T3 `HFlowLayout.cpp`：匿名命名空间实现 `ParseJustify` 与 `CalcJustifyOffsets`；改造 `ArrangeChildInternal` 行内分布逻辑并重算行矩形
- [x] T4 文档：`docs/Box.md`、`docs/Box.en.md` 第 4 节补充 `justify` 属性表、说明与 XML 示例
- [x] T5 示例：`page_hflow_box.xml` 增加 justify 选项行并调整选项区高度；中英文语言资源新增 `STRID_LAYOUT_PAGE_31` ~ `36`
- [x] T6 计划：更新 `docs/misc/Development-Plan.md` 基线日期、速览表、D-10 行、进展说明、勾选清单
- [x] T7 验证：Debug|x64 编译 0 error；`verify_docs.py` 退出码 0；`git status` 无临时文件

## 完成标准

- 未配置 `justify` 的既有 HFlow 布局行为零变化。
- 6 种取值在 layout 示例中可切换且行为符合规格第三节公式。
- 未修改 `Layout.h/.cpp`、`VFlowLayout`、`WindowBuilder`、`HFlowBox`。
- 文档校验与 Debug 编译通过；不提交代码、不编译 Release。

## 验证记录

| 检查项 | 结果 |
| :--- | :--- |
| MSBuild Debug\|x64 `build/examples.sln` | 通过，无 error（`lib/x64/Debug/duilib.lib`、`bin/layout_d.exe` 等已重新生成） |
| `verify_docs.py`（docs/examples/controls/lang） | 扫描 194 个文档，全部命中，EXIT=0 |
| `git status` | 仅本任务预期文件（另有 `.gitignore`、`build/examples.sln` 为本次之前已存在的未提交改动，未触碰） |

## 补充验证（2026-10-22）

- `kNotSet` 分支已改回原逻辑代码块（不经过 `CalcJustifyOffsets`），确保与原行为逐条一致。
- 测试程序新增 `justify='none'`（`STRID_LAYOUT_PAGE_37`，默认选中）用于验证兼容性；`none`/`not-set`/`not_set` 均复位为 `kNotSet`。
- 补充修改后重新编译 Debug|x64：EXIT=0；`verify_docs.py`：EXIT=0。
- `VFlowLayout` 已按完全对称方式补齐：`JustifyContent`、`justify`/`justify_content` 解析（含 `none` 复位）、按列沿 Y 轴分布；`kNotSet` 保留原 `child_valign` 代码块。
- `page_vflow_box.xml` 增加同组 justify 选项；`docs/Box.md` / `Box.en.md` 第 5 节同步补齐。
