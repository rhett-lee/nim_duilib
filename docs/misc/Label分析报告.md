下面的分析基于 Label.h 、 LabelImpl.cpp 、 TextDrawer.cpp 及它们依赖的 RichText/渲染实现。本次只做分析，未改动代码。

## 一、结论
整体设计是合理的：文本属性集中在`LabelImpl` ，`LabelTemplate<T>` 让 Button/HyperLink/GroupBox/DateTime/Marquee 等复用同一套文本逻辑；状态颜色淡入淡出、DPI 缩放、RichText 复用都有实现。但存在 3 个高优先级问题（多语言下 auto 尺寸错误、RichText 每帧重复解析、纵向+富文本显示原始标记），以及一批"属性改了不重排/不重绘"的一致性问题。

## 二、确定性缺陷
### 1. RichText 文本缓存失效顺序错误（高，直接影响中英文双语布局）
- LabelImpl.cpp#L311-L320`SetText` 、 #L327-L336`SetTextId` 都是先`RelayoutOrRedraw()` 、后`m_pTextDrawer->SetTextChanged()` ； Label.h#L323-L329`OnLanguageChanged` 同样是先重排、后清缓存。
- `RelayoutOrRedraw()` 在宽/高为 auto 时会同步执行`ArrangeAncestor()` ，重排过程中调用`EstimateText` →`TextDrawer::MeasureString` ；此时`m_bRichTextChanged` 还是 false， 测量用的是旧文本 ，auto 尺寸按旧语言/旧文本算出来。
- 影响：`rich_text="true"` 且`width/height="auto"` 的 Label 在切换语言（中文↔英文宽度差异大）或运行时`SetText` 后尺寸不对，直到下一次触发布局才恢复。
- 建议：先`m_pTextDrawer->SetTextChanged()` ，再`RelayoutOrRedraw()` ，最后`CheckShowToolTip()` ；`OnLanguageChanged` 改为先调用`m_impl->OnLanguageChanged()` 再重排。`SetRichText` （ #L381-L389 ）的顺序是对的，可作为参照。
### 2. vertical_text + rich_text 直接显示原始标记（中高，视觉）
- TextDrawer.cpp#L62-L65 / #L109-L112 ：纵向文本强制关闭 RichText 后，`strText` 仍是含`<b>` 、`<font>` 的原始串，直接`pRender->DrawString` 。
- 影响：界面出现`一个简单<b>窗口</b>` 这样的字面文本（文档已声明不支持组合，但代码既无断言也无降级提示）。
- 建议：`SetRichText(true)` 时若`IsVerticalText()` 在 Debug 下`ASSERT` 并在 Release 下按纯文本显示（或忽略 rich_text），避免"静默显示标记"。
### 3. auto_tooltip 的测量宽度没扣除内边距（中）
- LabelImpl.cpp#L593-L626 ：固定宽度时测量上限直接用`GetFixedWidth()` （控件宽，含 control/text padding），多行文本会按"更宽的可用宽度"换行 → 测量高度偏小 → 文本实际被裁切却不显示 tooltip。纵向分支同理，用`GetFixedHeight()` 。
- 建议：直接用已经 deflate 过的`rc.Width()` /`rc.Height()` 作为`rectSize` （与`OnEstimateTextWith` 的算法保持一致），不再单独取`GetFixedWidth()` 。
### 4. auto_tooltip + rich_text 会显示富文本标记（中）
- `GetAutoToolTipText()` （ #L562-L569 ）返回`GetOwnerText()` 原始串；文档 Control.md#L303 明确"富文本模式不支持 auto_tooltip"。
- 建议：`CheckShowToolTip` 中`if (IsRichText()) { m_bAutoShowTooltip = false; return; }` ；若要支持，需去标签生成纯文本提示。
### 5. SetReplaceNewline 运行期修改完全无效（中）
- LabelImpl.cpp#L428-L431 只改成员，既不`Invalidate` 也不`SetTextChanged` 。
- 影响：纯文本 Label 改了不重绘；RichText Label 因缓存的解析结果未失效，继续显示旧内容。
- 建议：值变化时`m_pTextDrawer->SetTextChanged(); m_pOwner->RelayoutOrRedraw(); CheckShowToolTip();` 。
### 6. SetWordSpacing 对横向文本无效（中）
- LabelImpl.cpp#L485-L499 ：只有`IsVerticalText()` 时才`Invalidate()` 。但 Skia 的横向绘制/测量同样使用`fWordSpacing` （`Render_Skia.cpp` 中`HJUSTIFY || fWordSpacing > 0` 分支）。
- 影响：横向 Label 运行时设置`word_spacing` 后界面无变化（切到纵向再切回才刷新）。
- 建议：无论如何都`RelayoutOrRedraw()` （字间距会改变测量宽度）。
### 7. 多项影响尺寸的属性只 Invalidate()、不重排（中，auto 尺寸失真）
同样模式的问题集中在：

- `SetFontId` （ #L926-L934 ）：换字体后 auto 宽高不变。
- `SetSingleLine` （ #L965-L977 ）。
- `SetTextStyle` （ #L824-L851 ）：含 SINGLELINE/VERTICAL 变化时尺寸会变。
- `SetVerticalText` （ #L438-L450 ）、`SetLineSpacing` （ #L457-L473 ）、`SetUseFontHeight` /`SetRotate90ForAscii` （ #L506-L529 ）。
对比`SetTextPadding` （ #L941-L958 ）用的是`RelayoutOrRedraw()` ，说明这里本应采用同一策略。建议统一改用`RelayoutOrRedraw()` ，并顺带刷新 tooltip。

### 8. TextDrawer 用单个 bool 作为文本缓存键（低，潜在绘制错误）
- TextDrawer.cpp#L78-L81 测量时就会把`m_bRichTextChanged` 置 false， #L127-L130 绘制时若标志已清就不再`SetText` 。
- 现状触发者： DateTime.cpp#L719-L730 在文本为空时用"示例文本"测量（`EstimateTextWith` ）。纯文本没问题，但若今后有"富文本 + 示例文本测量"的控件，会画出示例文本。
- 建议：缓存键改为"实际文本内容"（或给 TextDrawer 区分 measure/draw 两个缓存槽），比布尔标志更安全。
### 9. 其他小问题
- `IsTextEquals` （ #L396-L404 ）在`m_sText` 非空时比较原始串，忽略`replace_newline` ，与`GetText()` 语义不一致。
- `SetRichText` 未调用`CheckShowToolTip()` ，tooltip 状态可能滞后。
- `SetStateTextColor(kControlStateHovered, 空串)` 也会执行`SetFadeHovered(true)` ，白白创建动画播放器（ #L888-L904 ）。
- `text_align` 解析顺序是 hcenter→right→left→hjustify（ #L66-L88 ），值里同时出现"left"和"hjustify"时以 left 为准，建议写文档或加断言。
## 三、性能问题
1. RichText 每次测量和绘制都重新解析 （最值得优化）： TextDrawer.cpp#L73-L82 与 #L119-L131 每次都`SetWindow(nullptr)` →`SetWindow(real)` ；`RichTextT::SetWindow` （ RichText.h#L331-L336 ）会调用`OnWindowChanged()` →`Redraw()` ，清掉`m_textData` /绘制缓存。于是富文本标签每次布局/绘制都要重解析+重排版；悬停淡入淡出时还要画两次（ #L792-L812 ），相当于每帧 3~4 次解析。建议：仅在`pOwner->GetWindow()` 真正变化时才切换窗口，平时靠`SetEnableRedraw(false)` 抑制`Invalidate` 。
2. 每帧 RTTI + 字符串拷贝 ：`GetOwnerText()` （ #L406-L415 ）在`GetUTF8Text` 、`OnEstimateText` 、`CheckShowToolTip` 、`DoPaintText` 中都用`dynamic_cast<LabelOwner*>` 。LabelImpl 的 owner 必然实现`LabelOwner` ，建议构造时缓存`LabelOwner*` ，避免绘制热路径上的 RTTI。
3. `GetText()` 每次调用都查多语言表并做替换 （ #L297-L309 ）：一帧内布局、绘制、tooltip 会各调一次。建议缓存"已解析文本"，在`SetText/SetTextId/语言切换/replace_newline` 时失效。
4. tooltip 每次布局都重新测量 （ Label.h#L434-L438 →`CheckShowToolTip` ）：`auto_tooltip` 的列表项滚动重排会反复做 Skia 测量。建议按（文本、字体、可用宽高）缓存测量结果。
5. 颜色字符串来回转换 ： TextDrawer.cpp#L123 把`UiColor` 转成字符串，RichText 内部再`GetUiColor` 解析一次。建议给 RichText 增加`SetTextColor(UiColor)` 重载。
6. `GetMeasureParam/GetDrawParam` 每次`GetIFontById` （ #L536-L560 ），可缓存`IFont*` 。
## 四、界面美观度
1. 默认值与文档不一致，主题效果与预期不符 （中）：实现默认是"水平靠左、垂直居中、末尾省略号、单行"（ LabelImpl.cpp#L18 ），而文档写`text_align` 默认`"left,top"` 、`end_ellipsis` 默认`false` （ Control.md#L304 、 #L307 ，英文版同样）。尤其在`LabelBox` （高度拉伸）里，文字实际是垂直居中，按文档会误以为靠上。
2. `TEXT_NOCLIP` 默认不一致 ：构造函数默认风格不含`TEXT_NOCLIP` ，而`SetDefaultTextStyle()` （ #L853-L856 ）却加上它。调用过`SetDefaultTextStyle()` 的控件在多行文本超界时可能画到控件外，建议统一。
3. 富文本单行超长没有省略号 ：文档说富文本不支持`end_ellipsis` ，但也没有任何截断提示，长文本会被硬裁切。建议在 RichText 绘制路径支持"结尾省略号"或"尾部渐隐"，至少保证不出现半个字。
4. 纵向 + 富文本显示原始标记 （见缺陷 2），对界面观感影响最直接。
5. 富文本的`hjustify` /`word_spacing` 被静默忽略 （`UpdateTextDrawProps` 只映射 center/right/left， TextDrawer.cpp#L28-L36 ），主题若统一配置两端对齐，富文本标签会与其他控件不对齐。建议文档显式标注，或在 Debug 下告警。
6. 建议增强（非缺陷）：文字阴影/描边（深色主题小字号可读性）、`ToolTip` 与省略号出现位置联动（现在只在 SetPos 时判定，滚动容器内可能出现提示滞后一帧）。
## 五、文档/注释问题
位置	问题
Control.md#L304、Control.en.md#L306	"text_align 默认值应为 ""left,vcenter"""
Control.md#L307、Control.en.md#L309	end_ellipsis 默认值应为 true
Label.h SetSingleLine 注释	"""设置为单行输入模式"" → Label 非输入控件，应为""单行文本模式"""
LabelImpl.cpp#L106-L110	"vjustify 分支注释写成""垂直对齐：靠下""，应为""两端对齐"""
Control.md#L303	"已声明富文本模式的限制，但代码未做保护（见缺陷 2、4），建议文档补充""该组合会被忽略"""