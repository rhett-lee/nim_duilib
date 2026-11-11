# RichText 功能代码分析报告

> 分析对象：nim_duilib 的 RichText 实现（解析、布局、绘制、交互）
> 分析日期：2026-11-11
> 说明：本报告只做问题梳理与改进建议，未修改任何代码。

## 一、代码结构与数据流

RichText 由四层组成：XML 解析（`WindowBuilder`）→ 片段模型（`RichTextSlice`）→ 布局模型（`RichTextDataEx`/`RichTextData`）→ Skia 排版绘制（`DrawRichText`）。

- 入口与控件封装：[RichText.h](file:///e:/develop/nim_duilib/duilib/Control/RichText.h#L12-L199)
- 解析与状态：[RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp)
- XML 标签解析：[WindowBuilder.cpp](file:///e:/develop/nim_duilib/duilib/Core/WindowBuilder.cpp#L1923-L2069)
- 断行/绘制/缓存：[DrawRichText.cpp](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawRichText.cpp)

整体设计是合理的（解析与绘制分离、有绘制缓存、支持 DPI/主题/语言的增量刷新），但边界处理与状态一致性上有较多问题。

---

## 二、高优先级缺陷（功能性错误）

### 1. 内联节点之间的空格会被吞掉，导致单词粘连（明显可见）

- [RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L607-L647) 默认 `trim_policy="all"`，对每个文本节点单独 `StringUtil::Trim`；而 [StringUtil.cpp](file:///e:/develop/nim_duilib/duilib/Utils/StringUtil.cpp#L220-L250) 只按 ASCII 空白裁剪。
- 同时 [WindowBuilder.cpp](file:///e:/develop/nim_duilib/duilib/Core/WindowBuilder.cpp#L1931-L1934) 使用 `pugi::parse_default`，纯空白文本节点（如 `</a> <a>` 之间的空格）根本不会进入语法树。
- 结果：`Hello <b>World</b>` 渲染成 `HelloWorld`。现网资源里已经有证据：
  - [en_US-examples.txt](file:///e:/develop/nim_duilib/bin/resources/lang/en_US-examples.txt#L2463) 用普通空格分隔两个链接 → 英文界面显示成 `BaiduBing`；
  - [zh_CN-examples.txt](file:///e:/develop/nim_duilib/bin/resources/lang/zh_CN-examples.txt#L2468) 用全角空格“规避”，说明这是已知但未修复的问题；
  - [color_theme.xml](file:///e:/develop/nim_duilib/bin/resources/themes/default/color_theme/color_theme.xml#L1119-L1122) 中 `RichText: <a...>` 会显示为 `RichText:Click to Example`。

建议：默认策略改为“保留一个边界空格”（段首/段尾各保留一个），并对空白节点启用 `parse_ws_pcdata`；或改为在片段拼接阶段根据“前后是否有空白”补齐一个空格。这是收益最高的一处修复。

### 2. `text_align="center"` 不被识别，示例页对齐失效

[RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L35-L56) 只识别 `left / hcenter / right`，但 [page_text1.xml](file:///e:/develop/nim_duilib/bin/resources/themes/default/render/page_text1.xml#L114-L123) 有 3 处写的是 `center,top / center,vcenter / center,bottom`，实际全部按左对齐渲染，演示页“横向居中”矩阵是错的。建议同时兼容 `center` 别名，并修正资源。

### 3. 多行文本的 `hcenter/right` 逐行对齐未生效

`PaintText` 里的 [rowXOffset](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L414-L415) 永远是空数组，而 [DrawRichTextCacheData](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawRichText.cpp#L1126-L1131) 依赖它做每行偏移。当前只是把整个文本块平移，块内各逻辑行仍左对齐。可参考 RichEdit 的 [UpdateRowTextOffsetX](file:///e:/develop/nim_duilib/duilib/Control/RichEditData.cpp#L261-L307) 补齐逐行偏移计算。

### 4. `<a>` 只取第一个子节点的纯文本，链接内不能有格式

[WindowBuilder.cpp](file:///e:/develop/nim_duilib/duilib/Core/WindowBuilder.cpp#L2010-L2019) 用 `node.first_child().value()` 并禁止递归。`<a href="..."><b>文本</b></a>` 会得到空文本、整段内容消失。建议递归收集文本并为子片段继承 `href`。

### 5. 未知标签整棵子树被丢弃

[WindowBuilder.cpp](file:///e:/develop/nim_duilib/duilib/Core/WindowBuilder.cpp#L2052-L2056) 遇到不认识的节点直接 `continue`。用户写 `<p>、<span>、<h1>` 时文本会静默消失（Release 下连断言都没有）。建议“未知标签透明化”：不产生样式，但仍递归子节点。

### 6. `trim_policy` 名不符实

[RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L619-L644)：

- `keep_one` 只处理首尾，不压缩中间连续空格；首位与末位同时为空格时还会丢一个；
- 无论策略如何，`\r\n\t` 都被强制替换为空格（`kNone` 也不是“不处理”），与 [Control.md](file:///e:/develop/nim_duilib/docs/Control.md#L3247) 描述不一致。

### 7. `ToString()` 生成非法 XML（属性之间缺少空格）

[RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L869-L896) 连续拼接 `href="..."color="..."face="..."`，中间无分隔；文本与 URL 也未做 `& < > "` 转义。回填 `SetText(ToString())` 会解析失败。这是序列化/调试功能的基础缺陷。

### 8. `AppendTextSlice` 不触发重绘

[RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L817-L829) 只清缓存，不调用 `RelayoutOrRedraw()`。运行期动态追加内容后界面不会更新（XML 构建期碰巧没事）。另外 [RichText.h](file:///e:/develop/nim_duilib/duilib/Control/RichText.h#L144-L148) 的 `const RichTextSlice&&` 实际上仍走拷贝，移动语义失效。

### 9. 花括号替换会“吃掉”普通文本

[RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L97-L110) 在 `replace_brace=true`（默认）时，只要文本含 `{` `}` 且不含 `<>` 就全量替换。`text="Hello {name}"` 会变成 `Hello <name>`，解析失败；而 `DoSetText` 先 `Clear()`，于是整段文字直接消失且无回退。建议：仅当形如 `{u}`、`{br/}` 等已知标签时才转换，解析失败时按纯文本降级渲染。

### 10. 鼠标移出链接后高亮不刷新

[RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L1016-L1038) 先把所有 `m_bMouseHover` 置 false，但只在“命中链接”分支里 `Invalidate()`。从链接移到同一控件的空白处，链接仍保持 hover 颜色，直到下一次被动重绘。

### 11. `link_font_underline` 只在 hover/按下时生效

[RichTextImpl.cpp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L372-L374) 只在 `m_bMouseDown || m_bMouseHover` 时设置下划线，常态链接无下划线，与属性语义“是否使用带下划线的字体”不符。建议常态也加下划线（或把属性重命名为 `link_hover_underline` 并在文档中说明）。

---

## 三、中优先级问题（性能与健壮性）

| 问题 | 位置 | 说明与建议 |
| :--- | :--- | :--- |
| 悬停导致整篇重排版 | [PaintText](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L346-L411) | 悬停改颜色/字体后 `richTextData` 变化，`IsValidDrawRichTextCache` 判定失效，长文本每次进入链接都重新断行。建议缓存基础布局，绘制时按片段覆盖颜色/下划线 |
| 每帧堆分配 | 同上 L360-L395 | 每次绘制复制 `vector<RichTextData>` 并为每个链接 `new UiFontEx`。建议复用成员缓冲 + 字体对象池 |
| 重复测量 | [CalcDestRect](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L209-L239) | `EstimateText` 与 `PaintText` 各测一次，对齐时再测一次。建议缓存“最近一次测量结果（宽度/高度/DPI）” |
| 重复 Invalidate | [ButtonDown/MouseMove/MouseHover](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L957-L1058) | 嵌套循环里逐命中矩形调用，一次移动可能触发多次重绘。建议汇总 `bChanged` 后只调一次 |
| 字体对象膨胀 | [ParseTextSlice](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L543-L565) | 每个片段都新建 `UiFontEx`（多数与父节点完全相同）。建议共享父字体，仅在属性变化时写时复制 |
| `<font size>` 无校验 | [WindowBuilder.cpp](file:///e:/develop/nim_duilib/duilib/Core/WindowBuilder.cpp#L2040-L2045) | 可传入极大/非法字号导致布局异常；注释与实际 DPI 缩放逻辑也矛盾。建议 `clamp(1, 512)` 并统一注释 |
| TAB 逻辑不可达且坐标基准错误 | [TrimText](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L613-L617) / [DrawRichText](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawRichText.cpp#L343-L367) | 解析阶段已把 `\t` 换成空格，绘制期 TAB 分支在 RichText 中永远走不到；且 `fmod(xPos, ...)` 用的是绝对坐标，应以文本矩形左边为基准 |
| 单行溢出无省略号 | [GetTextStyle](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L173-L207) | 从不设置 `TEXT_END_ELLIPSIS`，`word_wrap=false` + 固定宽度时长词直接越界绘制（`SkTextBox` 只按自身 run 矩形裁剪，不按控件矩形裁剪），视觉上会串到相邻控件 |
| 末尾多算一行 | [DrawRichText.cpp](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawRichText.cpp#L447-L490) | 文本恰好占满行宽时仍会执行一次换行，行高表多出一行，影响垂直居中 |
| 背景色绘制不一致 | [DrawRichText.cpp](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawRichText.cpp#L543-L544) vs [L1146-L1148](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawRichText.cpp#L1146-L1148) | 非缓存路径无条件 FillRect，缓存路径判断 `IsEmpty()`，建议统一 |
| 笔误/死代码 | [DrawRichText.cpp](file:///e:/develop/nim_duilib/duilib/RenderSkia/DrawRichText.cpp#L379-L406) | L383 重复判断 `glyphChars`（应为 `glyphIDs`）；L401 用 `maxWidth == 矩形宽度` 判断无法换行，语义脆弱 |
| `SetText` 未清除 `text_id` | [SetText/SetTextId](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L685-L707) | 先 `text_id` 再 `SetText`，之后切语言会按 ID 覆盖手动文本。建议 `SetText` 内清空 `m_richTextId/m_langFileName` |
| URL 指针生命周期 | [ButtonUp](file:///e:/develop/nim_duilib/duilib/Control/RichTextImpl.cpp#L991-L996) | `SendEvent` 传的是局部 `DString` 的 `c_str()`，仅在同步回调内有效；建议改为按值传 `DString*` 或使用事件数据容器 |

---

## 四、功能缺口（可扩展项）

1. 标签体系：缺少 `<span>`（别名）、`<p>/<h1..h6>`（段落间距/字号层级）、`<img src>`（内联图片与基线对齐）、`<ul>/<li>`、`<sup>/<sub>`、`<hr>`。
2. 排版能力：`text_align="justify"`、逐段对齐、`letter_spacing`、`max_lines` + 行尾省略、`text_overflow="ellipsis|clip"`、首行缩进。
3. 链接能力：常态下划线、visited 状态、自定义 tooltip（当前直接把 URL 当 tooltip，长 URL 很吵）、悬停/离开事件（便于状态栏显示链接）、键盘 Tab 聚焦 + Enter 触发、右键“复制链接地址”。
4. 交互能力：文本选择/复制（目前只能靠外部 Label）、链接命中区域无障碍信息。
5. 运行时 API：`SetLinkColor`、`AppendText`（自带重绘）、按链接索引查矩形。
6. 降级策略：解析失败时以纯文本显示，而不是空白。

---

## 五、性能优化清单（按收益排序）

1. 悬停不重排版：把颜色/下划线状态从布局缓存中剥离（`DrawRichText` 增加逐片段颜色覆盖参数），缓存只与文本/宽度/DPI/字体相关。
2. 复用 `vector<RichTextData>`、`UiFontEx`，消除每次绘制的堆分配。
3. 测量结果缓存：`EstimateText` 按 `(内容版本, 可用宽度, DPI, 样式)` 记忆，`PaintText` 复用其 `rect`，避免三次测量。
4. `IsValidDrawRichTextCache` 现用字符串指针 + 值比较，成本可接受；但建议给 `m_textSlice` 增加“内容版本号”，用版本号快速失效。
5. 片段解析改为移动语义（`RichTextSlice&&`），`emplace_back(std::move(...))`，大段文本解析时明显减少拷贝。
6. 鼠标移动只在状态位真正变化时 `Invalidate()`。

---

## 六、界面美观度改进

1. 修复空格粘连后，链接之间的自然间距、`RichText: xxx` 的空格会立刻恢复正常（这是当前最影响观感的问题）。
2. 链接视觉规范：常态下划线（`link_font_underline` 生效）、hover 加下划线 + 主题 hover 色、pressed 更深色；建议给链接加 `text-decoration-thickness` 级别的一致视觉，而非仅换色。
3. [global.xml](file:///e:/develop/nim_duilib/bin/resources/themes/default/global.xml#L789-L794) 的 `rich_text` 类目前只定义颜色，没有字体、行高、内边距。建议新增可选类（不改变既有默认行为），例如 `rich_text_paragraph`：`row_spacing_mul="1.3" text_padding="2,2,2,2" word_wrap="true"`，供说明文字/段落使用。
4. 演示页美化：
   - [page_text1.xml](file:///e:/develop/nim_duilib/bin/resources/themes/default/render/page_text1.xml#L87-L131) 目前用 `bkcolor="green/blue/dark_gray"` 调试色、无内边距，建议换成卡片样式（`bg_window_card` + `border_round` + `padding`），并用真实文案展示对齐矩阵；
   - [color_theme.xml](file:///e:/develop/nim_duilib/bin/resources/themes/default/color_theme/color_theme.xml#L1119-L1122) 的预览文案存在空格丢失，需同步修正；
   - controls 页只有一行 `RichText: Baidu`，建议扩展为一个“排版样例”区块：标题层级、粗斜体/删除线/下划线、行内背景、链接组、自动换行与截断对比。
5. 深色/浅色对比度：链接颜色已走主题变量，建议再补 `text_link_visited` 令牌，避免“已访问链接”与正文同色。
6. 工具提示：仅当链接文字被截断或 URL 与显示文本差异较大时才提示，减少噪声。
7. 裁剪与边界：对 `word_wrap=false` 增加按控件矩形裁剪或省略号，避免文字溢出到相邻控件。

---

## 七、文档与测试需要同步的点

- [Control.md](file:///e:/develop/nim_duilib/docs/Control.md#L3238-L3254)（及 `Control.en.md`）：`trim_policy` 的 `none/keep_one` 语义、`link_font_underline` 的生效状态、`text_align` 可用值（是否接受 `center` 别名）、支持的标签集合与实际限制（`<a>` 内不能嵌标签、未知标签的处理）。
- 建议在 `tests/` 的静态校验中补两条规则：示例资源里的 `text_align` 值必须在白名单内；`RichText` 文案中不得再用全角空格作为词间分隔（防止空格问题回归）。
- Parser 建议补最小单测（嵌套标签、`<a>` 内格式、未知标签、实体、花括号、非法 XML 降级）。

---

## 八、建议的修复顺序

1. 第一批（正确性，改动小）：空格保留 → `center` 别名 → 未知标签透传 → `<a>` 递归取文本 → 花括号仅识别已知标签 + 解析失败降级纯文本。
2. 第二批（状态与 API）：hover 失效修复、`AppendTextSlice` 重绘、`SetText` 清 `text_id`、`ToString` 属性分隔与转义、`keep_one` 语义。
3. 第三批（排版质量）：逐行 `rowXOffset`、单行省略号、末尾多一行、TAB 基准。
4. 第四批（性能与观感）：悬停不重排版、测量缓存、字体复用、示例页与主题类美化。
