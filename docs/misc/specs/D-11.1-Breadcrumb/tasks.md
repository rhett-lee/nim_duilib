# Tasks: D-11.1 Breadcrumb 面包屑导航控件

> 依赖顺序执行；每项完成后立即勾选并记录验证结果。

## T1 评审与定稿

- [x] T1.1 用户确认 `spec.md` 第 7 节四项（实现形态 / 事件编码 / v1 范围 / 文件清单）。
- [x] T1.2 定稿后不再改动本 spec 的历史结论；需求变更以"增量需求"章节追加。

验证：用户书面确认。

## T2 控件源码

- [x] T2.1 新增 `duilib/Control/Breadcrumb.h`：`Item` / `BreadcrumbValue` / `Breadcrumb : public Control`，声明 spec 4.1 的全部 public API 与 private 辅助函数。
- [x] T2.2 新增 `duilib/Control/Breadcrumb.cpp`：构造默认值、`GetType()`、`SetAttribute()`、数据/当前项/禁用/外观 API。
- [x] T2.3 实现布局、绘制、命中、hover/pressed、光标切换、`EmitCurrentChanged()`（wParam=3）。
- [x] T2.4 实现 `OnLanguageChanged()` 重新解析 `item_text_id`；`ClearItems()` 清空禁用集合；`SetItems()` 保留并钳位禁用集合。
- [x] T2.5 自检边界：空数据、单条目、越界、重复设置、删除后索引收敛。

验证：代码走查 + `MSBuild ... /t:duilib`（可先单独编译库）。

## T3 注册 5 处

- [x] T3.1 `duilib/duilib_defs.h` 新增 `DUI_CTR_BREADCRUMB`。
- [x] T3.2 `duilib/Core/WindowBuilder.cpp` 增加 include 与工厂 map 项。
- [x] T3.3 `duilib/duilib.vcxproj` 登记 `.h/.cpp`。
- [x] T3.4 `duilib/duilib.vcxproj.filters` 登记并归入 `Control` 过滤器。
- [x] T3.5 `duilib/duilib.h` 增加 include。

验证：`grep DUI_CTR_BREADCRUMB` 命中注册点；编译通过。

## T4 主题 Class

- [x] T4.1 `bin/resources/themes/default/global.xml` 新增 `<Class name="breadcrumb" .../>`（语义色，无图片/PATH）。
- [x] T4.2 检查浅色/深色主题下 `text_link_normal` / `text_default` / `text_muted` 均存在。

验证：示例在浅色、深色主题各显示一次。

## T5 示例与入口

- [x] T5.1 新增 `examples/controls/BreadcrumbForm.h/.cpp`（事件回显 + 上一级/跳到首页/追加/删除）。
- [x] T5.2 新增 `bin/resources/themes/default/controls/breadcrumb.xml`（默认形态 + 自定义形态；`class` 属性第一位）。
- [x] T5.3 `bin/resources/themes/default/controls/test.xml` 增加 Breadcrumb 分组与入口按钮。
- [x] T5.4 `examples/controls/TestForm.h/.cpp` 增加 `ShowBreadcrumbWindow()` 并绑定按钮。
- [x] T5.5 `examples/controls/controls.vcxproj` 与 `.filters` 登记新文件。
- [x] T5.6 中英 `bin/resources/lang/*-examples.txt` 补齐全部 `STRID_BREADCRUMB_*` 键（UTF-8 无 BOM）。
- [x] T5.7 示例覆盖全部公开 API：跳转、追加、删除、禁用、NoEvent（按钮触发）。

验证：示例窗口可打开，四个按钮均生效。

## T6 文档同步（中英成对）

- [x] T6.1 `docs/Control.md` / `docs/Control.en.md` 新增 Breadcrumb 小节（属性表/XML/C++/事件）。
- [x] T6.2 `docs/Summary.md` / `docs/Summary.en.md` 新增控件行与锚点链接。
- [x] T6.3 `README.md` / `README.en.md` 新增第 51 条并同步控件计数。
- [x] T6.4 `.trae/skills/nim-duilib/references/api-reference.md` 与 `.workbuddy` 镜像同步（速查行 + 属性 + 事件）。
- [x] T6.5 `.claude/docs/nim-duilib-llm-reference.md` 速查行/描述同步。
- [x] T6.6 文档中所有标识符（`DUI_CTR_BREADCRUMB`、`ui::Breadcrumb`、`kEventValueChanged`、色名、字体）均可被源码/主题命中。

验证：`verify_docs.py` 退出码 0。

## T7 计划文档回写

- [x] T7.1 `docs/misc/Development-Plan.md` 第 41 行 P2 汇总更新（D-11.1 移除）。
- [x] T7.2 D-11 表格行补 Breadcrumb 完成说明（源码路径 + 示例）。
- [x] T7.3 `- [ ] D-11.1` → `- [x] D-11.1 ... 完成日期：2026-10-22`。
- [x] T7.4 确认 `Project-Audit.md` / `Control-Roadmap.md` 无对应行（不改），并在提交说明中记录。

验证：搜索 `D-11`、`Breadcrumb`，状态一致。

## T8 编译、校验与验收

- [x] T8.1 `MSBuild "build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo` 无错误。
- [ ] T8.2 按 `checklist.md` 完成手工测试（属性/边界/事件/语言/主题/DPI/回归）。— 待用户运行时验证
- [x] T8.3 `verify_docs.py --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang` 退出码 0。
- [x] T8.4 汇总改动文件清单与测试结论，交用户手动提交（助手不执行 git commit）。

## T9 视觉改版（2026-10-22）

- [x] T9.1 默认配色改为可区分层级：祖先项 `color_blue`、当前项 `text_default`、分隔符 `color_gray`。
- [x] T9.2 新增 `current_font` / `hover_bg_color` / `pressed_bg_color` / `item_round` 及对应 Set/Get。
- [x] T9.3 悬停/按下绘制圆角胶囊背景；当前项加粗并按加粗字体测量布局。
- [x] T9.4 默认值调整：分隔符 `›`、`spacing=4`、`item_padding=6`。
- [x] T9.5 示例卡片高度自适应 + 底部占位，消除大面积留白。
- [x] T9.6 同步中英文档、两份 `api-reference.md`、`.claude` 参考与 spec 增量章节。
- [x] T9.7 Debug|x64 编译通过；`verify_docs.py` 退出码 0。
