# controls 示例双语 STRID 补齐与双语支持审计

## Context

上一轮为了支持中英文切换，把 `examples/controls` 及 `bin/resources/themes/default/controls` 中大量硬编码中文改成了 `text_id` / `prompt_text_id` / `title_id`，并把代码里动态拼接的文案改成 `Lang().GetStringByID(...)` + `OnLanguageChanged()` 刷新。

但新引入的 STRID 还没有写进语言文件，导致：
- 静态校验 R6（`text_id` 必须在中英文语言文件存在）会失败；
- 示例运行时这些位置显示空文本。

已用脚本把 `examples/controls/*.cpp|h` 与 `themes/default/controls/*.xml` 里引用的全部 STRID 与两个语言文件做差集，确认缺 83 个键（`STRID_MSGBOX_OK/CANCEL` 已在 `public` 语言文件，不算缺）。

目标：补齐这 83 个键的中英文；把 `test.xml` 里仍是硬编码中文的 RichText 富文本也改为 `text_id`；再新增一条静态规则防止回归；最后报告整个仓库示例中仍不支持双语的残留位置。

## 变更 1：追加语言资源（核心）

文件：
- `bin/resources/lang/zh_CN-examples.txt`
- `bin/resources/lang/en_US-examples.txt`

追加到文件末尾，沿用现有格式：键名左对齐到 34 列 + `= 值`（`strId.ljust(34) + "= " + value`）。

中文值以本轮从 XML 中删除的原文为准（已通过 `git diff` 取回），英文值按同一语义翻译：

| 分组 | 键（数量） | 中文示例 | 英文示例 |
|---|---|---|---|
| Combo | `STRID_COMBO_WINDOW_TITLE/_DESC/_BASIC/_DROP_LIST/_DROP_DOWN/_DISABLED/_FILTER/_CHECK/_SCROLL_HINT/_EVENT/_EVENT_DEFAULT/_EVENT_DROP_LIST/_EVENT_DROP_DOWN/_EVENT_FILTER`（14） | `Combo 下拉选择`、`drop_list（不可编辑）`、`drop_list 选中：%s` | `Combo Drop-down`、`drop_list (not editable)`、`drop_list selected: %s` |
| ListView | `STRID_LISTVIEW_WINDOW_TITLE/_DESC/_VLIST/_HLIST/_LISTBOX_DEFAULT/_TREE/_EVENT/_EVENT_DEFAULT/_EVENT_VLIST/_TOAST_DOUBLE_CLICK/_EVENT_TREE`（11） | `VListBox 选中第 %d 项：%s`、`双击：%s` | `VListBox item %d selected: %s`、`Double click: %s` |
| DateTime | `STRID_DATETIME_WINDOW_TITLE/_DESC/_FORMATS/_CALENDAR_SECONDS/_LIMIT/_LIMIT_DESC/_RANGE/_DISABLED_WEEKEND/_NOW/_EVENT/_EVENT_DEFAULT/_EVENT_VALUE`（12） | `禁用周末（示例日期串）`、`%s 当前值：%s` | `Weekend disabled (sample date string)`、`%s current value: %s` |
| RichEdit | `STRID_RICHEDIT_WINDOW_TITLE/_DESC/_SINGLE_LINE/_PLAIN_PROMPT/_PROMPT_INPUT/_SPIN/_PASSWORD/_PROMPT_PASSWORD/_MULTI_LINE/_MULTI_DEFAULT/_MULTI_DIRECTWRITE/_EVENT/_EVENT_DEFAULT/_EVENT_PASSWORD/_EVENT_CHARS/_EVENT_CHARS2/_EVENT_SINGLE`（17） | `请输入密码`、`RichEdit 当前字符数：%d` | `Please enter password`、`RichEdit character count: %d` |
| Chart 实时监控 | `STRID_CHART_REALTIME_TITLE/_START/_PAUSE/_CLEAR/_JITTER/_CHART_TITLE/_X_AXIS/_Y_AXIS`（8） | `开始` / `暂停` / `清空` / `随机扰动` / `时间` / `占用率` | `Start` / `Pause` / `Clear` / `Jitter` / `Time` / `Usage` |
| Avatar 姓名 | `STRID_AVATAR_NAME_ZHANGWEI/_LINA/_WANGQIANG/_ZHAOMIN`（4） | 张伟 / 李娜 / 王强 / 赵敏 | Zhang Wei / Li Na / Wang Qiang / Zhao Min |
| Controls 页 | `STRID_CONTROLS_RICHEDIT_PROMPT_HINT/_SEARCHBOX_PROMPT_HINT/_TAGINPUT_PROMPT_HINT/_AUTOCOMPLETE_PROMPT_HINT/_BALLOON_CARD/_OPEN_FLYOUT_PAGE/_MORE_COMPONENTS/_TRAY_BALLOON_TITLE/_TRAY_BALLOON_TEXT`（9） | `搜索控件...`、`更多组件：Timeline / Marquee / Animation / StatusBar / ToolBar` | `Search controls...`、`More: Timeline / Marquee / Animation / StatusBar / ToolBar` |
| Test 页 | `STRID_TEST_SEARCH_TITLE/_PROMPT`（2） | `SearchBox 搜索框（建议 + 历史）`、`搜索控件 / 输入关键词（回车记录历史）` | `SearchBox (suggestions + history)`、`Search controls / press Enter to save history` |
| Navigation | `STRID_NAVIGATION_STATUS_NONE/_STATUS_INFO/_STATUS_ITEM_CLICK/_STATUS_PANE_COLLAPSED/_STATUS_PANE_EXPANDED/_DYNAMIC_PAGE`（6） | `序号=%zu，页面=%s` | `index=%zu, page=%s` |

注意占位符必须和调用处一致（已核对）：
- `ListViewForm.cpp` 用 `(int32_t)index` → `%d`；
- `NavigationForm.cpp` 用 `size_t` → `%zu`；
- `NavigationForm` 动态项文本 `Item %d` 是演示数据，不本地化。

## 变更 2：test.xml 的 RichText 改为 text_id

`bin/resources/themes/default/controls/test.xml` 第 133–135 行的 RichText 仍是硬编码中文富文本：

```xml
<RichText class="rich_text" name="test_rich_text" ...>
    RichText 支持富文本与超链接：<a href="www.baidu.com">Baidu</a>　<a href="www.bing.com">Bing</a>
</RichText>
```

`RichTextImpl` 已实现 `text_id` 属性并在语言切换时刷新（见 `duilib/Control/RichTextImpl.cpp` 的 `SetAttribute` 与 `SetTextId`），因此改为：

```xml
<RichText class="rich_text" name="test_rich_text" row_spacing_mul="1.2" width="auto"
          margin="0,8,0,0" text_id="STRID_TEST_RICHTEXT_DEMO"/>
```

并在两个语言文件追加 `STRID_TEST_RICHTEXT_DEMO`（中英文各一条，值含 `<a>` 标签）。

## 变更 3：新增静态规则 R15（防回归）

文件：`tests/test_controls_pages.py`

1. 扩展已有的“STRID 必须存在于中英文语言文件”检查（R6）：收集所有 `*_id` 形式的 UI 属性（`text_id`、`prompt_text_id`、`title_id`、`x_axis_title_id`、`y_axis_title_id`、`tooltip_text_id`、`pane_title_id`、`settings_text_id`、`item_title_id`、`item_content_id`、`column_title`、`data_labels_id`，含 `|` / `,` 分隔的多值），并同时在 `*-examples.txt` 与 `*-public.txt` 中查找。
2. 新增 R15：`themes/default/controls/*.xml` 中 `text` / `prompt_text` / `title` / `x_axis_title` / `y_axis_title` / `tooltip_text` 属性值不得包含 CJK 字符；跳过 XML 注释，跳过 `RichText` 子树（RichText 允许内联富文本，改为由 R6 保证 `text_id` 存在）。
3. 规则编号/输出风格沿用现有 R1–R14 的写法。

## 变更 4：其余示例的双语问题（只报告，不改）

已知仍存在硬编码中文、但超出本轮 `controls` 范围的文件（在最终回复中列清单）：

- `examples/render/RenderForm.cpp`、`examples/TreeView/ComputerView.cpp`、`examples/ListCtrl/*`、`examples/chat/*`；
- `themes/default/chat/wechat.xml`、`themes/default/color_theme/color_theme.xml`、`themes/default/render/page_image2.xml` / `page_image3.xml` / `page_text1.xml` / `page_text2.xml`。

另外，`examples/controls` 中属于“演示数据”的中英文内容（`RichEditForm.cpp` 的多行示例文本、`ListBoxItem %d`、`Dynamic Child %d`、`Item %d`）不做本地化，避免把示例数据当成 UI 文案。

## 验证

1. 用脚本重新做差集，确认 `examples/controls` + `themes/default/controls` 引用的 STRID 在两个语言文件中均为 0 缺失。
2. `python tests/test_controls_pages.py --source-root .` 全绿（R1–R15）。
3. 构建 examples（`controls` 目标，Debug）并跑 `ctest --test-dir out\x64\Debug -C Debug --output-on-failure`，保持 3/3 通过。
4. 人工抽查：运行 `controls` 示例，切换中英文，确认 Combo / ListView / DateTime / RichEdit / Chart 实时监控 / Avatar / Navigation 状态栏文本均随语言切换。

## 关键文件

- `bin/resources/lang/zh_CN-examples.txt`、`bin/resources/lang/en_US-examples.txt`
- `bin/resources/themes/default/controls/test.xml`
- `tests/test_controls_pages.py`
- 已改代码（无需再改，仅核对占位符）：`examples/controls/{ComboForm,ListViewForm,DateTimeForm,RichEditForm,ChartForm,NavigationForm,ControlForm}.cpp`
