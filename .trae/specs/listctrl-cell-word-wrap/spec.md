# Spec: ListCtrl Report 视图单元格自动换行（单行开关）

## 1. 背景与问题

nim_duilib 的 `ListCtrl` 采用虚表实现（`ListCtrlReportView : VirtualListBox`）。Report 视图下每行的子控件 `ListCtrlSubItem` 会被滚动**复用**：同一个 UI 控件在不同滚动位置会承载不同的数据行。

因此，"给单元格加一个 `single_line` 可控属性"这种直接作用于 UI 控件的做法是**错误**的——滚动后控件显示别的数据行的内容，属性不会跟随数据变化。**属性必须存放在数据模型中，并在"数据填充"环节应用**，才能得到预期效果。

现状：
- List / Icon 视图已有 `SetTextSingleLine(bool)` 控制换行；**Report 视图无对应接口**。
- Report 单元格默认 `single_line="true"`（见 `global.xml` 的 `list_ctrl_sub_item` Class），即单行 + 末尾省略号。
- 可变行高已支持：`ListCtrl::SetDataItemHeight(nItemHeight)`（默认行高）/ `SetDataItemHeight(itemIndex, nItemHeight)`（逐行）。
- 已存在 per-cell 文本格式通路 `SetSubItemTextFormatById` → `ListCtrlSubItemData2::nTextFormat` → 填充时 `pSubItem->SetTextStyle(...)`，天然满足"填充处控制"。

## 2. 目标

为 Report 视图开放单元格自动换行（单行开关）能力：
- 新增友好 API：`SetSubItemSingleLine(itemIndex, columnIndex, bSingleLine)` 及 `...ById(itemIndex, columnId, bSingleLine)`，配套 `IsSubItemSingleLine(...)` 查询。
- 通过复用已存在的 per-cell `nTextFormat` 通路实现，**不新增结构体字段、不改填充代码**：切换 `TEXT_SINGLELINE` 位，值存于数据模型，填充时应用。
- 语义：`bSingleLine=false` 时清除 `TEXT_SINGLELINE`，文本按单元格可用宽度自动换行；行高由调用方通过 `SetDataItemHeight` 调整（否则超出部分被裁剪）。
- 仅 Report 视图有效：非 Report 视图 `Set` 返回 `false`，`Is` 返回 `true`。
- 提供示例演示与界面可交互测试项，并同步中英文文档。

## 3. 非目标

- 不做换行后按内容自动计算行高（由调用方用 `SetDataItemHeight` 自行处理）。
- 不改动 List / Icon 视图既有 `SetTextSingleLine` 行为。
- 不实现 F2 单元格编辑、Ctrl+C 复制（另行跟进）。

## 4. 设计

### 4.1 数据层（已实现）

`ListCtrlData`：
```cpp
bool SetSubItemSingleLine(size_t itemIndex, size_t columnId, bool bSingleLine);
bool IsSubItemSingleLine(size_t itemIndex, size_t columnId) const;
```
- `Set`：`GetSubItemStorageForWrite` 校验索引；取 `GetSubItemTextFormat(...)`（含默认格式回退）后按 `bSingleLine` 置位/清位 `TEXT_SINGLELINE`，写回 `SetSubItemTextFormat(...)`。
- `Is`：`GetSubItemTextFormat(...) & TEXT_SINGLELINE` 判定。
- 常量 `TEXT_SINGLELINE` 来自 `IRender.h` 的 `enum DrawStringFormat`（0x0100）。

### 4.2 控件层（已实现）

`ListCtrl`：
```cpp
bool SetSubItemSingleLine(size_t itemIndex, size_t columnIndex, bool bSingleLine);   // 走 GetColumnId(columnIndex)
bool SetSubItemSingleLineById(size_t itemIndex, size_t columnId, bool bSingleLine);  // 仅 Report
bool IsSubItemSingleLine(size_t itemIndex, size_t columnIndex) const;
bool IsSubItemSingleLineById(size_t itemIndex, size_t columnId) const;
```
- 非 Report 视图：`Set*` 返回 `false`，`Is*` 返回 `true`。

### 4.3 填充应用（既有通路，无需改动）

`ListCtrlReportView::FillSubItemData` 中 `pSubItem->SetTextStyle(pStorage->nTextFormat, false)` 已应用 per-cell 格式；去除 `TEXT_SINGLELINE` 位后 `Label` 即进入多行（自动换行）渲染。

### 4.4 示例演示（待实现）

在 ListCtrl 示例中新增"单元格换行"控制，作用于 `column_combo` 当前选中列（默认选中第 0 列，其文本含长串 `-1234567890-ABCDEFG-...`，便于观察换行效果）：

- **XML** `bin/resources/themes/default/list_ctrl/list_ctrl.xml`：在 GroupVBox 中（`frozen_column` 行之后，约 L145 后）新增一个 HBox：
  - `Label text_id="STRID_LISTCTRL_CELL_WORD_WRAP"` + `tooltip_text_id="STRID_LISTCTRL_CELL_WORD_WRAP_TIPS"`
  - `Option name="cell_single_line" group="cell_wrap" selected="true" text_id="STRID_LISTCTRL_SINGLE_LINE"`
  - `Option name="cell_word_wrap" group="cell_wrap" selected="false" text_id="STRID_LISTCTRL_WORD_WRAP"`
  - 与 `column_visible_hide/show` 一致：**不写内联 `<Event>`**，由 C++ 侧 `AttachSelect` 处理。

- **`examples/ListCtrl/MainForm.cpp`** `InitListCtrlEvents` 中（参照 `OnColumnTextAlign` ~L440）新增处理：
  ```
  OnCellWrap(bool bSingleLine):
      columnId = column_combo 当前项数据; columnIndex = GetColumnIndex(columnId)
      对每一行 index: SetSubItemSingleLine(index, columnIndex, bSingleLine)
      配合行高: 开启换行时 SetDataItemHeight(64, true); 恢复单行时 SetDataItemHeight(32, true)
  ```
  `cell_single_line` → `OnCellWrap(true)`；`cell_word_wrap` → `OnCellWrap(false)`。

- **`OnColumnChanged`**（~L946 附近，紧跟 text-align 同步逻辑）新增：读取 `GetSubItemTextFormat(0, columnIndex)` 的 `TEXT_SINGLELINE` 位，同步两个 Option 的选中态。

- **语言文件** `bin/resources/lang/zh_CN-examples.txt` 与 `en_US-examples.txt` 新增 4 条：
  `STRID_LISTCTRL_CELL_WORD_WRAP`（标签）、`STRID_LISTCTRL_CELL_WORD_WRAP_TIPS`（提示）、`STRID_LISTCTRL_SINGLE_LINE`、`STRID_LISTCTRL_WORD_WRAP`。

### 4.5 文档同步（待实现）

- `docs/Control.md` / `docs/Control.en.md`：ListCtrl 单元格能力表补充 `SetSubItemSingleLine`/`IsSubItemSingleLine` 说明及示例。
- `docs/ListCtrl-Roadmap.md` / `.en.md`：将 L3 由"待办"更新为"已完成"，补接口与语义说明。
- 三处技能参考副本同步：`.trae/skills/nim-duilib/references/api-reference.md`、`.workbuddy/skills/nim-duilib/references/api-reference.md`、`.claude/docs/nim-duilib-llm-reference.md`。

## 5. 涉及文件与改动清单

| 文件 | 改动 |
|---|---|
| `duilib/Control/ListCtrlData.h` / `.cpp` | **已完成**：新增 `SetSubItemSingleLine` / `IsSubItemSingleLine` |
| `duilib/Control/ListCtrl.h` / `.cpp` | **已完成**：新增 `SetSubItemSingleLine(ById)` / `IsSubItemSingleLine(ById)` |
| `bin/.../list_ctrl/list_ctrl.xml` | 待办：新增 `cell_wrap` Option 对 + 标签 |
| `examples/ListCtrl/MainForm.cpp` | 待办：`InitListCtrlEvents` 新增换行处理；`OnColumnChanged` 新增选中态同步 |
| `bin/resources/lang/zh_CN-examples.txt`、`en_US-examples.txt` | 待办：新增 4 条字符串 |
| `docs/Control.md` / `.en.md` | 待办：同步能力说明 |
| `docs/ListCtrl-Roadmap.md` / `.en.md` | 待办：L3 标记已完成 |
| 三处技能参考副本 | 待办：同步接口 |

## 6. 验证

1. 编译：`MSBuild.exe build/examples.sln /p:Configuration=Debug /p:Platform=x64 /m /nodeReuse:false /clp:ErrorsOnly /v:m`，期望 0 错误。
2. 运行 ListCtrl 示例：默认（单行）第 0 列长文本按省略号裁剪；选择"多行"后第 0 列单元格文本自动换行且行高增大可完整显示；切回"单行"恢复。
3. 文档漂移校验：`python ".trae/skills/nim-duilib/scripts/verify_docs.py" --repo "c:/develop/nim_duilib" --extra docs ...`，退出码 0。
4. 代码提交由用户手动执行（助手不提交）。

## 7. 审批

请确认以上范围与设计。确认后按 `tasks.md` 执行示例、语言、文档改动并编译验证。
