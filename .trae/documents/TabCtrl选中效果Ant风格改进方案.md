# TabCtrl 选中效果按 Ant Design 改进 + 文档补全

## 背景（Context）

`layout` 示例当前的标签栏效果与 Ant Design Tabs（line 类型）差距明显：

- 标签栏有整条灰色圆角背景（`tab_ctrl` 类的 `bkcolor="bg_tab_ctrl"` + `border_round="4,4"`）
- 每个标签之间有竖向分隔线（`line_class="tab_ctrl_item_line"`）
- 选中项是灰色圆角背景块，与整体不协调（本次已改为底部指示条，但仍是固定 24px 宽的短条）
- 标签文字颜色不随悬停/选中变化（主题强调色未体现）
- 标签项左侧 padding=10、右侧 0，文字在标签内偏右

Ant Design Tabs（line）的关键视觉特征：整条无灰底、只有底部 1px 分割线、无竖线；选中/悬停文字变主题色；选中指示条（ink bar）宽度跟随标题文字宽度、高 2px、圆角、主题色、贴在底部边线上。

同时，上一轮为 TabCtrl 新增的指示条相关属性（`selected_tab_item_indicator_width/height/color`、`selected_tab_item_show_bg`）尚未写入文档，需要本次一并补齐。

## 改动范围

1. 框架：新增两个可选能力（默认关闭，完全兼容原有行为）
   - `selected_tab_item_indicator_width="auto"`：指示条宽度自动跟随标题文字宽度
   - TabCtrlItem 的 `title_color` / `hovered_title_color` / `selected_title_color`：标题文字按状态变色
2. 示例：只改 `layout` 示例的标签栏样式
3. 文档：`docs/Control.md`、`docs/Control.en.md`、`docs/schema/nim_duilib.xsd`

## 一、框架改动：指示条宽度支持 auto

文件：[TabCtrl.h](file:///e:/develop/nim_duilib/duilib/Control/TabCtrl.h)、[TabCtrl.cpp](file:///e:/develop/nim_duilib/duilib/Control/TabCtrl.cpp)

1. `TabCtrl` 公共接口（`GetSelectedTabItemIndicatorWidth` 之后）新增：

```cpp
/** 设置选中标签的指示条宽度是否自动跟随标签标题文字宽度
* @param [in] bAuto true表示自动跟随标题文字宽度（此时忽略指示条宽度值）
*/
void SetSelectedTabItemIndicatorAutoWidth(bool bAuto);
bool IsSelectedTabItemIndicatorAutoWidth() const;
```

2. 新增成员 `bool m_bSelectedTabItemIndicatorAutoWidth;`（默认 `false`），构造函数初始化为 `false`。

3. `SetAttribute` 中 `selected_tab_item_indicator_width`：
   - 值为 `auto` → `SetSelectedTabItemIndicatorAutoWidth(true)`
   - 其他值 → `SetSelectedTabItemIndicatorAutoWidth(false)` + `SetSelectedTabItemIndicatorWidth(...)`

4. `PaintSelectedTabIndicator`：
   - 提前返回条件改为：颜色为空、**非 auto 模式下**宽度 <= 0、高度 <= 0 时返回
   - auto 模式下用新增私有函数计算长度：
     - 水平标签栏：取选中项标题 `Label` 的文本，用 `pRender->MeasureString(text, param)`（`param.pFont = GetIFontById(pLabel->GetFontId())`、`uFormat = TEXT_SINGLELINE`）测得像素宽度；测量失败或文本为空时回退为标题 Label 矩形宽度
     - 垂直标签栏：取标题 Label 矩形高度
     - 最终钳制不超过标签项自身宽度/高度
   - 非 auto 模式保持原有 `dpi.GetScaleFloat(fIndicatorWidth)` 逻辑

参考实现：[Breadcrumb.cpp](file:///e:/develop/nim_duilib/duilib/Control/Breadcrumb.cpp#L493-L502) 的文本测量写法。

指示条绘制位置逻辑不变（仍由 `TabCtrl::PaintForeColor` 在子控件之后统一绘制，避免被标签项裁剪）。

## 二、框架改动：TabCtrlItem 标题文字状态色

同一对文件。

1. 新增公共接口：

```cpp
void SetTitleColor(const DString& color);          DString GetTitleColor() const;
void SetHoveredTitleColor(const DString& color);   DString GetHoveredTitleColor() const;
void SetSelectedTitleColor(const DString& color);  DString GetSelectedTitleColor() const;
```

2. `SetAttribute` 新增三个属性：`title_color`、`hovered_title_color`、`selected_title_color`。

3. 新增成员：`m_titleColor`、`m_hoveredTitleColor`、`m_selectedTitleColor`、`m_defaultTitleColor`（均为 `UiString`）。

4. 新增私有函数 `void UpdateTitleTextColor();`
   - 三个属性全为空时直接返回（不干预，保持原有颜色，完全兼容）
   - 颜色优先级：选中态 `selected_title_color` → 悬停/按下态 `hovered_title_color` → `title_color` → `m_defaultTitleColor`
   - 通过 `m_pLabel->SetStateTextColor(kControlStateNormal, color)` 生效（标题 Label 设置了 `mouse_enabled="false"`，自身状态恒为 Normal，所以写 Normal 状态色）
5. `m_defaultTitleColor` 在创建/重建标题 Label 时用 `m_pLabel->GetStateTextColor(kControlStateNormal)` 记录原始颜色，用于未配置对应状态色时回退。

6. 调用时机：
   - `SetTitleClass`：创建 Label 后记录默认色并调用一次；`titleClass` 变化重建后同样处理
   - `OnPrivateSetSelected`：选中状态变化
   - `HandleEvent`：`msg.GetSender() == this && eventType == kEventStateChanged`（现有代码该分支被 `m_pLine != nullptr` 限制，改为先处理分割线、再无条件调用 `UpdateTitleTextColor`）
   - 三个 Setter 内部（控件已初始化时）

## 三、示例改动：layout.xml 改为 Ant 风格

文件：[layout.xml](file:///e:/develop/nim_duilib/bin/resources/themes/default/layout/layout.xml)

新增两个窗口级 Class（在现有 `layout_card` 附近），标签栏与标签项改用它们：

```xml
<!-- 标签栏：Ant Design Tabs(line) 风格：无灰底、底部 1px 分割线、选中用主色指示条 -->
<Class name="layout_tab_ctrl" width="stretch" height="40" border_round="0,0" bkcolor=""
       selected_tab_item_outline_width="0" selected_tab_item_outline_color=""
       tab_ctrl_bottom_line_height="1" tab_ctrl_bottom_line_color="border_split_level1"
       selected_tab_item_show_bg="false"
       selected_tab_item_indicator_width="auto"
       selected_tab_item_indicator_height="2"
       selected_tab_item_indicator_color="color_accent"/>
<!-- 标签项：无背景、无竖线，文字按状态变色 -->
<Class name="layout_tab_item" padding="0,0,0,0"
       hovered_color="" pressed_color="" selected_normal_color=""
       selected_round_corner="0,0" hovered_round_corner="0,0" hovered_padding="0,0,0,0"
       title_class="layout_tab_item_title" line_class="" close_button_class=""
       title_color="text_default" hovered_title_color="color_accent" selected_title_color="color_accent"/>
```

- `TabCtrl` 的 `class` 改为 `class="tab_ctrl layout_tab_ctrl"`（后者覆盖前者），移除原有 inline 的 `height/border_round/selected_tab_item_*` 属性
- 17 个 `TabCtrlItem` 的 `class` 改为 `class="tab_ctrl_item layout_tab_item"`，保留各自 `width`、`title`、`tab_box_item_index`，移除 inline 的 `title_class`、`close_button_class`
- `layout_tab_item_title` 保留 `width="stretch" text_align="hcenter"`（文字水平居中，指示条自动与之对齐）

说明：空字符串属性（`bkcolor=""`、`hovered_color=""`、`line_class=""`）会被 `AttributeUtil::ParseAttributeList` 保留并调用 `SetAttribute`，可清除父 Class 中的颜色/子控件配置。

## 四、文档补全

1. [Control.md](file:///e:/develop/nim_duilib/docs/Control.md#L3229-L3246) 的 `## TabCtrl的属性` 表：
   - 新增 `selected_tab_item_show_bg`、`selected_tab_item_indicator_width`（说明支持 `auto`）、`selected_tab_item_indicator_height`、`selected_tab_item_indicator_color`
   - 补充“对应 C++ 接口”一行
   - 追加一段 XML 示例（Ant 风格底部指示条）

2. 同文件 `## TabCtrlItem的属性` 表：新增 `title_color`、`hovered_title_color`、`selected_title_color` 及 C++ 接口说明

3. [Control.en.md](file:///e:/develop/nim_duilib/docs/Control.en.md#L3225-L3244)：对应英文内容

4. [nim_duilib.xsd](file:///e:/develop/nim_duilib/docs/schema/nim_duilib.xsd#L8630-L8635)：`TabCtrlAttrs` 组内补 4 个 `<xs:attribute>`；[TabCtrlItemAttrs](file:///e:/develop/nim_duilib/docs/schema/nim_duilib.xsd#L8766-L8771) 组内补 3 个 `<xs:attribute>`，格式与现有条目一致（`type="xs:string"` + `<xs:documentation>`）

## 五、验证

1. 先结束正在运行的 `layout.exe`（避免链接失败）
2. 构建：`e:\develop\nim_duilib` 下执行
   `build\build.bat build --config Debug --arch x64 --sdl --no-deps --target layout`
3. 运行 `bin\layout.exe`，截图检查：
   - 标签栏无灰色背景、无圆角、无竖分隔线，底部为 1px 分割线
   - 选中标签下方为 2px 主题色指示条，宽度与标题文字接近，中心与文字对齐
   - 鼠标悬停其他标签：文字变主题色，无灰色背景块
   - 17 个标签完整显示、无省略号、无错位
   - 点击标题栏 `btn_switch_theme` 切换深色主题，重复检查
4. 清理临时截图/脚本，结束 `layout.exe` 进程
