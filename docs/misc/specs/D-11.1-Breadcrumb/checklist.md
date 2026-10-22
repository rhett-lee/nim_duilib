# Checklist: D-11.1 Breadcrumb

> 全部勾选才视为完成（DoD）。

## 1. 源码

- [x] `duilib/Control/Breadcrumb.h` / `.cpp` 已新增，命名空间 `ui`，类注释含属性表与语义说明。
- [x] `GetType()` 返回 `DUI_CTR_BREADCRUMB`。
- [x] 构造函数初始化全部成员，颜色默认值为语义色名（不写死色值），字体 ID 默认 `system_regular_14`。
- [x] `SetAttribute()` 支持主名与别名，未知属性回退 `BaseClass::SetAttribute()`。
- [x] 数据 API 齐全：`AddItem/InsertItem/RemoveItemAt/ClearItems/SetItems/GetItemCount/GetItemAt/GetItems`。
- [x] 当前项 API 齐全：`GetCurrentIndex/SetCurrentIndex/SetCurrentIndexNoEvent/GoTo/GoToNoEvent`。
- [x] 禁用项 API 齐全，`disabled_items` 支持 `|` 与 `,` 分隔。
- [x] 事件 `AttachCurrentChanged` 存在，编码 `kEventValueChanged` + `wParam=3`，`pEventData` 注释写明仅本次回调有效。
- [x] 无 `new` 子控件 / 无图片资源依赖；`PaintStateImages()` 内 `AutoClip` 裁剪。
- [x] 所有像素尺寸（`spacing` / `item_padding`）绘制与命中时经 `Dpi().GetScaleInt()` 缩放。
- [x] 状态颜色优先级注释存在：禁用 > 按下 > 悬停 > 当前 > 普通。

## 2. 注册

- [x] `duilib_defs.h` 新增 `DUI_CTR_BREADCRUMB`。
- [x] `WindowBuilder.cpp` 新增 include 与工厂 map 项。
- [x] `duilib.vcxproj` 登记 `.h/.cpp`。
- [x] `duilib.vcxproj.filters` 登记并归入 `Control`。
- [x] `duilib.h` 新增 include。

## 3. 主题与示例

- [x] `global.xml` 新增 `breadcrumb` Class，全部颜色为语义色名。
- [x] `examples/controls/BreadcrumbForm.h/.cpp` 存在且完成事件/按钮绑定。
- [x] `bin/resources/themes/default/controls/breadcrumb.xml` 存在；`class` 属性写在最前；含默认与自定义（`separator=">"`、`current`、`disabled_items`）两种形态。
- [x] `test.xml` 有 Breadcrumb 分组与 `breadcrumb_btn` 入口，点击可打开示例窗口。
- [x] `TestForm.h/.cpp` 有 `ShowBreadcrumbWindow()`。
- [x] `controls.vcxproj` / `.filters` 登记新示例文件。
- [x] 中英 examples 语言文件键值齐全、UTF-8 无 BOM、中文无乱码。

## 4. 功能验收（手工）

> 以下为运行时人工验收项，由用户在示例程序中确认。

### 创建与属性
- [ ] XML 创建成功，`FindControl` 可取到；无 `class` 时正常显示。
- [ ] `item_text` 与 `item_text_id` 均生效（后者按 STRID 解析）。
- [ ] `current` / `current_index` 生效；缺省时当前项为最后一项。
- [ ] `separator` / `separator_text` 生效（含中文/多字符分隔符）。
- [ ] `spacing`、`item_padding` 生效，DPI 变化时按比例缩放。
- [ ] `click_to_jump=false` 后点击祖先项不切换当前项。
- [ ] `disabled_items` 生效：禁用项置灰且不可点击。
- [ ] 全部颜色属性与 `font` 实际生效；未知属性不崩。

### 交互与事件
- [ ] 悬停可交互祖先项：文字变色 + 手型光标；移出恢复。
- [ ] 按下可交互项：显示按下色；`ButtonUp`/`MouseLeave` 后恢复。
- [ ] 点击祖先项：当前项切换，事件派发一次，`pEventData->currentIndex/oldIndex` 正确。
- [ ] 点击当前项、禁用项、分隔符、空白：无事件、无状态变化。
- [ ] `SetCurrentIndexNoEvent` / `GoToNoEvent`：值变化但不派发事件。
- [ ] `SetCurrentIndex` 设为相同值：不派发事件。

### 边界
- [ ] 空数据：不绘制、不崩，`GetCurrentIndex()` 返回 0。
- [ ] 单条目：无点击效果。
- [ ] 越界 `current` / `SetCurrentIndex`：自动钳位。
- [ ] `RemoveItemAt` / `ClearItems` / `SetItems` 后当前索引正确收敛，界面不越界。
- [ ] 超长文本、条目很多：右对齐裁剪，末项/当前项可见，窗口拉宽后恢复完整显示。

### 语言与主题
- [ ] 中文 → 英文 → 中文切换：条目文案自动刷新，布局不串位。
- [ ] 浅色主题：祖先项/当前项/禁用项/分隔符对比度正常。
- [ ] 深色主题：同上，无写死色值导致的不可读。

### DPI 与生命周期
- [ ] 100% / 125% / 150% / 200%：间距、命中区域与文字位置一致。
- [ ] 反复打开/关闭 Breadcrumb 示例窗口无崩溃、无泄漏迹象。
- [ ] 相邻控件入口（Stepper / Rating / Carousel / TreeGrid 等）仍正常工作。

## 5. 文档与计划

- [x] `docs/Control.md` / `Control.en.md` 小节完整且标题对应。
- [x] `docs/Summary.md` / `Summary.en.md` 行与锚点有效。
- [x] `README.md` / `README.en.md` 编号连续、计数同步。
- [x] `api-reference.md`（`.trae` 与 `.workbuddy`）文本一致。
- [x] `.claude/docs/nim-duilib-llm-reference.md` 已同步。
- [x] `Development-Plan.md` 第 41 行、D-11 行、D-11.1 复选框与完成日期已更新。
- [x] `Project-Audit.md` / `Control-Roadmap.md` 已确认无对应行（无需改动）。

## 6. 验证命令

- [x] `MSBuild "build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo` 无错误。
- [x] `python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0。
- [x] 未执行 `git commit`（由用户手动提交）。

## 7. 视觉改版（2026-10-22）

- [x] 默认配色可区分层级：祖先项 `color_blue`、当前项 `text_default`、分隔符 `color_gray`，且浅/深主题均存在同名语义色。
- [x] 悬停/按下绘制圆角胶囊背景（`hover_bg_color` / `pressed_bg_color`），留空可关闭。
- [x] 当前项使用 `current_font`（`system_bold_14`）加粗，布局按对应字体测量宽度。
- [x] `item_round` 默认 4，绘制时经 `Dpi().GetScaleInt()` 缩放。
- [x] 默认分隔符改为 `›`，`spacing=4`、`item_padding=6`。
- [x] 示例卡片改为高度自适应 + 底部 stretch 占位，消除大片留白。
- [x] 中英文档、`api-reference.md`（`.trae` + `.workbuddy`）、`.claude` 参考已同步新属性与默认值。
- [ ] 人工确认：浅色/深色主题下层级与胶囊反馈的实际观感符合预期。
