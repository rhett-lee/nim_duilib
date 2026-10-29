# Checklist: D-9 CascadingSelect

> 全部勾选才视为完成（DoD）。

## 1. 源码

- [x] `duilib/Control/CascadingSelect.h` / `.cpp` 已新增，命名空间 `ui`。
- [x] `GetType()` 返回 `DUI_CTR_CASCADING_SELECT`。
- [x] 构造函数初始化全部成员：`path_separator="/"`、`value_separator=" / "`、`column_width=0`（等分）、`row_height=28`、`text_padding=10`、`font=system_regular_14`。
- [x] 颜色默认值为语义色名（`text_default` / `color_accent` / `text_disabled` / `bg_list_item_hovered` / `bg_list_item_selected` / `border_split_level1` / `color_gray`），无写死色值。
- [x] `SetAttribute()` 支持主名与别名（`items`/`item_paths`、`items_id`/`item_paths_id`、`disabled_paths`/`disabledpaths`），未知属性回退 `BaseClass::SetAttribute()`。
- [x] 数据 API 齐全：`SetItems` / `SetItemIds` / `AddPath` / `ClearItems` / `GetDepth` / `GetItemCount` / `GetItemText`。
- [x] 选择 API 齐全：`GetSelection` / `SetSelection` / `SetSelectionByText` / `ClearSelection` / `GetSelectionText` / `GetSelectionPath`。
- [x] 禁用 API 齐全：`SetDisabledPaths` / `GetDisabledPaths` / `IsItemDisabled`，支持禁用整棵子树。
- [x] 事件 `AttachSelectionChanged` 存在，编码 `kEventValueChanged` + `wParam=4`（第 812 行 `SendEvent`）。
- [x] 事件数据 `CascadingSelectValue`（indexes/text/path）为栈上临时对象，文档注明仅本次回调有效。
- [x] 无 `new` 子控件 / 无图片资源依赖；`PaintStateImages()` 内 `AutoClip` 裁剪。
- [x] 所有像素尺寸（列宽、行高、文字内边距、圆角、箭头、滚动条）绘制与命中时经 `Dpi().GetScaleInt()` 缩放。
- [x] 父项点击后 `selection` 截断为 `i+1`，子列由前缀节点 children 派生（自动过滤 + 深层重置）。
- [x] `OnLanguageChanged` 走 `RebuildFromIds()`，只重建文本，选择与列滚动索引保留。

## 2. 注册

- [x] `duilib_defs.h` 新增 `DUI_CTR_CASCADING_SELECT`。
- [x] `WindowBuilder.cpp` 新增 include 与工厂 map 项。
- [x] `duilib.vcxproj` 登记 `.h/.cpp`。
- [x] `duilib.vcxproj.filters` 登记并归入 `Control`。
- [x] `duilib.h` 新增 include。

## 3. 主题与示例

- [x] `global.xml` 新增 `cascading_select` Class，全部颜色为语义色名。
- [x] `examples/controls/CascadingSelectForm.h/.cpp` 存在且完成事件回显与六个按钮绑定。
- [x] `bin/resources/themes/default/controls/cascading_select.xml` 存在；`class` 属性写在最前；含默认三级与自定义（`path_separator=">"`、固定列宽、`disabled_paths`）两种形态。
- [x] `test.xml` 有 CascadingSelect 分组与 `cascading_select_btn` 入口。
- [x] `TestForm.h/.cpp` 有 `ShowCascadingSelectWindow()` 与入口绑定。
- [x] `controls.vcxproj` / `.filters` 登记新示例文件。
- [x] 中英 examples 语言文件键值齐全（各 25 个 `STRID_CS_*` / 入口相关键）、UTF-8 无 BOM、中文无乱码。

## 4. 功能验收（手工）

> 以下为运行时人工验收项，由用户在示例程序中确认。

### 创建与属性
- [ ] XML 创建成功，`FindControl` 可取到；无 `class` 时正常显示。
- [ ] `items` 与 `items_id` 均生效（后者按 STRID 解析），相同前缀自动合并。
- [ ] `path_separator` / `value_separator` 生效（含多字符分隔符）。
- [ ] `column_width=0` 等分、`>0` 固定列宽两种模式正确。
- [ ] `row_height`、`text_padding` 生效，DPI 变化时按比例缩放。
- [ ] `disabled_paths` 生效：命中节点及整棵子树置灰且不可点。
- [ ] 全部颜色属性与 `font` 实际生效；未知属性不崩。

### 交互与事件
- [ ] 点击父列某行：子列内容立即过滤，深层旧选择被清空。
- [ ] 点击叶子：`AttachSelectionChanged` 只派发一次，`pEventData->indexes/text/path` 正确。
- [ ] 选中项保持高亮，悬停未选中行显示悬停背景；手型光标正确。
- [ ] 列内容溢出时滚轮在该列滚动（每格 3 行），滚动条比例正确。
- [ ] `SetSelection` / `SetSelectionByText` / `ClearSelection` 行为与事件派发正确，相同值不派发。
- [ ] `AddPath` 追加后树更新，`SetDisabledPaths` 动态禁用/恢复即时生效。

### 边界
- [ ] 空数据：不绘制、不崩，`GetDepth()` 返回 0。
- [ ] 单级数据：只显示一列。
- [ ] 越界选择 / 重复设置：自动钳位，不崩溃。

### 语言与主题
- [ ] 中文 → 英文 → 中文切换：条目文案自动刷新，选择与滚动位置保持，布局不串位。
- [ ] 浅色主题与深色主题下，普通/选中/禁用/分隔线/箭头/滚动条对比度正常。

### DPI 与生命周期
- [ ] 100% / 125% / 150% / 200%：列宽、行高、箭头与命中区域一致。
- [ ] 反复打开/关闭 CascadingSelect 示例窗口无崩溃。
- [ ] 相邻控件入口（Breadcrumb / TreeGrid / Carousel 等）仍正常工作。

## 5. 文档与计划

- [x] `docs/Control.md` / `Control.en.md` 小节完整且标题对应。
- [x] `docs/Summary.md` / `Summary.en.md` 行与锚点有效。
- [x] `README.md` / `README.en.md` 编号 52 连续、计数同步。
- [x] `api-reference.md`（`.trae` 与 `.workbuddy`）文本一致（`Compare-Object` 差异 0）。
- [x] `.claude/docs/nim-duilib-llm-reference.md` 已同步。
- [x] `Development-Plan.md` 第 41 行、P2 进展汇总、D-9 表格行、任务行已完成回写。

## 6. 验证命令

- [x] `MSBuild "build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo` 退出码 0。
- [x] `python3 .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0（扫描 191 个文档，全部命中）。
- [x] 未执行 `git commit`（由用户手动提交）。
