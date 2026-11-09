# ColorTheme 主题生成与全量预览优化计划

## Context（背景与目标）

`examples/ColorTheme` 目前只完成了部分主题预览：

- `bin/resources/themes/default/global.xml` 定义了 **228 个 Class、54 个 Font、31 个 Var、26 个 Alias、4 个 FontFile、ThemeMeta**；
- `color_theme.xml` 仅使用了约 **69 个 Class**，缺少 RangeSlider、Stepper、Rating、Carousel、Pagination、Breadcrumb、CascadingSelect、Avatar、Popover/Popconfirm、ToolBar、TreeGrid、ScrollBar、Menu 项变体、ListCtrl 多视图等；
- 浅色/深色主题各有 **199 个 ThemeColor**，但预览页只有“当前主题”的色卡，且默认隐藏；
- 生成器只有“色相、基础色度”两个参数，只能复制 XML，无法保存或重置，生成结果也只预览 14 个核心色。

本次目标：让该示例成为真正的“主题配置生成 + 全量主题效果预览”工具，覆盖 `global.xml` 全部内容与深色/浅色全部颜色值。

已确认的实现选择：
1. 预览页保持**单页长滚动**，按分类追加区块；
2. 生成器增强为**参数 + 保存 + 全量预览**；
3. 深色/浅色颜色使用**并排对照表**展示。

## 变更 1：扩展预览页 `color_theme.xml`

文件：`bin/resources/themes/default/color_theme/color_theme.xml`

在文件末尾 `<Control height="20"/>` 之前追加以下区块（沿用现有 `GroupVBox class="group_box_round"` 风格）：

| 区块 | 需覆盖的 Class | 说明 |
| --- | --- | --- |
| RangeSlider | `range_slider_horizontal` `range_slider_vertical` | 水平 + 垂直两个实例 |
| Stepper | `stepper_node` | 横向步骤条，含标题/描述 |
| Rating | `rating` | 可交互 + 只读半星两个实例 |
| Carousel / Pagination | `carousel` `pagination_button` `pagination_nav_button` `pagination_page_size` | Carousel 使用 `${PATH_CAROUSEL}` 图片；Pagination 两个实例 |
| Breadcrumb | `breadcrumb` | 基本形态 + 自定义分隔符 |
| CascadingSelect | `cascading_select` `cascader` | 级联面板 + 下拉形态 |
| Avatar | `avatar` `avatar_small` `avatar_large` `avatar_xlarge` `avatar_square` `avatar_square_large` `avatar_user` `avatar_user_large` `avatar_photo` `avatar_text` | 尺寸、形状、内容类型 |
| SearchBox / AutoComplete / TagInput | `search_box*` `auto_complete*` `tag_input*` | 输入类控件 |
| RichText | `rich_text` | 富文本混排 |
| ToolBar | `toolbar_button` `toolbar_check` `toolbar_separator` | 动态工具项（见变更 2） |
| StatusBar | 无主题 Class | 内联样式，展示状态栏外观 |
| Menu 项变体 | `menu_text` `menu_text_shortcut` `menu_split_line` `menu_checkbox` `menu_selected_dot` `menu_selected_square` `menu_selected_circle_dot` `menu_selected_check_line` | 补进现有 Menu 区块或新增区块 |
| ListCtrl 多视图 | `list_ctrl_icon_view*` `list_ctrl_list_view*` `list_ctrl_report_view` `list_ctrl_richedit` `list_ctrl_datetime` `list_ctrl_combo_*` `list_ctrl_checkbox` | 图标/列表/报表视图与特殊列 |
| TreeGrid | `tree_grid*` | 多列表头 + 勾选/展开 |
| ScrollBar | `vscrollbar` `hscrollbar` `scrollbar_base` | 固定区域内内容溢出，`auto_hide_scroll="false"` 保证可见 |
| TabCtrl 变体 | `tab_ctrl_item` 的关闭按钮/badge/indicator | 可关闭标签、带徽标标签 |
| GroupBox / Popover / Popconfirm | `group_box` `popover_*` `popconfirm_*` | 普通 GroupBox + 浮层锚点 |

同时在 `gen_color_box` 中追加生成器参数区（见变更 2 的 name 清单）。

## 变更 2：MainForm 生成器增强、颜色对照与全局配置展示

文件：`examples/ColorTheme/MainForm.h`、`examples/ColorTheme/MainForm.cpp`

### 2.1 生成器参数（新增 8 个 Slider + 保存/重置按钮）

在 `gen_color_box` 内新增 3 行参数区，控件命名与映射：

| 控件 name | 范围 | 换算 | 对应 API |
| --- | --- | --- | --- |
| `gen_color_slider_bg_l_light` / `gen_color_label_bg_l_light` | 500–1000 | `/1000` | `SetBgParams(bgLightL, bgDarkL, 当前 chroma)` |
| `gen_color_slider_bg_l_dark` / `gen_color_label_bg_l_dark` | 0–500 | `/1000` | 同上 |
| `gen_color_slider_fg_l_light` / `gen_color_label_fg_l_light` | 0–600 | `/1000` | `SetFgParams(fgLightL, fgDarkL, 当前 chroma)` |
| `gen_color_slider_fg_l_dark` / `gen_color_label_fg_l_dark` | 500–1000 | `/1000` | 同上 |
| `gen_color_slider_surface_light_offset` / 对应 Label | 0–150 | `/1000` | `SetSurfaceParams(lightOffset, darkOffset, 当前 chroma)` |
| `gen_color_slider_surface_dark_offset` / 对应 Label | 0–200 | `/1000` | 同上 |
| `gen_color_slider_accent_l` / 对应 Label | 300–900 | `/1000`，同时赋给 light/dark | `SetAccentParams(accentL, accentL, accentC)` |
| `gen_color_slider_accent_c` / 对应 Label | 0–400 | `/1000` | 同上 |

新增按钮：
- `btn_save_xml_data`：用 `ui::FileDialog::BrowseForFile(this, path, false, ...)` 选择保存路径，再用 `ui::FileUtil::WriteFileData(path, m_colorThemeXmlData)` 写出；默认文件名 `global.xml`。
- `btn_reset_gen_params`：`m_pThemeGenerator->ResetParams()`，用 `GetBgParams/GetFgParams/GetSurfaceParams/GetAccentParams` 同步全部 Slider，然后调用 `OnGenColorParamChanged()`。

实现要点：
- 所有参数 Slider 复用现有 `AttachValueChanged` 模式，统一调用 `OnGenColorParamChanged()`；
- `OnGenColorParamChanged()` 中先读取全部 Slider，按上表调用 SetXXXParams，再 `GenerateTheme(hue, baseChroma, isDark)`；
- 新增 `CreateThemeGenerator()` 负责读取当前主题 `global.xml` 并 `LoadConfigFromXml`，替代 `OnGenColorParamChanged` 内的懒加载代码，供初始化与重置复用；
- 初始化时从生成器默认值同步 Slider 位置与 Label 文本。

### 2.2 生成结果全量色卡刷新

- 保留 `ShowAllColors()` 首次构建逻辑，但把创建的 `Label*` 按颜色名缓存到 `std::map<std::string, ui::Label*> m_allColorLabels`；
- 新增 `UpdateAllColors()`：用生成后的 XML 临时 `ui::ThemeGenerator` 加载，遍历 `GetLoadedConfigs()`，对每个缓存 Label 更新 `SetToolTipText(name + " = " + value)`；
- 色块本身继续使用颜色名设置 `bkcolor`，因此随窗口私有主题自动变色，无需重建控件；
- 仅在 `all_colors` 可见时执行，避免无谓开销。

### 2.3 浅色/深色全量颜色并排对照表

新增 `ShowLightDarkColorCompare()`：
- 通过 `Theme().GetThemeRootPath() / "color_light"|"color_dark" / GetGlobalXmlFileName()` 读取两份 XML（`WindowBuilder::ReadXmlFileData`），各自 `ThemeGenerator::LoadConfigFromXml`；
- 以浅色为基准，按 `type` 分组、按 `node_order` 排序；
- 每个颜色一行：`名称 | 浅色块 | 浅色 HEX | 深色块 | 深色 HEX`；
- 容器 name：`light_dark_colors_box`（XML 中预留空 `VBox`）；
- 色块使用绝对色 `#AARRGGBB`，HEX 直接显示 `ThemeColorConfig.value`。

### 2.4 global.xml 全部内容展示

新增 `ShowGlobalConfigInfo()`，用 `pugixml` 解析当前主题的 `global.xml`（`#include "duilib/third_party/xml/pugixml.hpp"`，DUILIB_DLL=OFF 静态链接可用）：
- `global_meta_box`：`Theme` 的 name/type/style/version + `ThemeMeta` 的每个 `Property`；
- `global_vars_box`：全部 `Var name/value`（两列网格）；
- `global_alias_box`：全部 `Alias name/value`，颜色块用 `SetBkColor(aliasName)`，并显示目标色名；
- `global_font_box`：全部 `Font id/name/size`，用 `<Label font="字体id">` 渲染 `Aa 字体名/字号`；
- 顶部附加 `FontFile`、`DefaultFontFamilyNames`、`FallbackFontFamilyNames` 信息行。

以上四个容器在 `color_theme.xml` 中预留为空 `VBox`，由代码动态填充。

### 2.5 控件数据初始化

在 `OnInitWindow()` 末尾新增 `InitPreviewControls()`，只对必须用代码初始化的控件赋值：
- `ToolBar`：`AddToolButton` / `AddToolCheckBox` / `AddToolSeparator`；
- `TagInput`：`AddTag("C++")`、`AddTag("UI")`、`AddTag("Theme")`；
- `AutoComplete`：`SetCandidates({...})`；
- `TreeGrid`：`AddNode` + `SetCellText` + `SetNodeExpand`；
- ListCtrl 图标视图：按需 `InsertItem` / 设置图片（若 `SetImageList` 依赖资源则仅使用文字项）。

## 变更 3：MainForm.h 新增成员与方法

```cpp
void CreateThemeGenerator();                 // 创建并加载当前主题配置
void InitGenParamSliders();                  // 参数 Slider 绑定与默认值同步
void UpdateAllColors();                      // 全量色卡 tooltip 刷新
void ShowLightDarkColorCompare();            // 浅/深并排对照表
void ShowGlobalConfigInfo();                 // ThemeMeta/Var/Alias/Font 展示
void InitPreviewControls();                  // TreeGrid/ToolBar/TagInput 等初始化
void OnSaveXmlData();                        // 保存生成的 XML

std::map<std::string, ui::Label*> m_allColorLabels;  // 全量色卡缓存
```

`#include "duilib/third_party/xml/pugixml.hpp"` 放在 `MainForm.cpp`。

## 验证

1. 编译：
   - `& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -SkipAutomaticLocation`
   - `cmake --build "e:\develop\nim_duilib\out\x64\Release" --target ColorTheme --config Release`
2. 启动 `bin/ColorTheme.exe`，分别验证：
   - 浅色主题：全部新区块（RangeSlider/Stepper/Rating/Carousel/Pagination/Breadcrumb/CascadingSelect/Avatar/SearchBox/AutoComplete/TagInput/RichText/ToolBar/StatusBar/Menu 项/ListCtrl 视图/TreeGrid/ScrollBar/TabCtrl 变体/GroupBox/Popover/Popconfirm）正常渲染；
   - 深色主题（点击主题切换按钮）：上述区块颜色随主题变化；
   - 生成器：拖动新增 8 个参数，窗口颜色实时变化，核心色 Label 与全量色卡 tooltip 同步更新；
   - 保存：点击“Save XML”，选择路径后文件生成且内容为完整主题 XML；
   - 重置：参数回到默认值；
   - 浅/深对照表：199 个颜色全部出现，两侧色块与 HEX 正确；
   - ThemeMeta / Var / Alias / Font 区块内容与 `global.xml` 一致。
3. 覆盖度脚本检查：
   - 提取 `global.xml` 全部 `<Class name>`，与 `color_theme.xml` 使用的 class 做差集；
   - 除内部子控件白名单（`menu_text`、`rich_edit_spin_box`、`address_bar_edit`、`tab_ctrl_item_close` 等）外，差集应为空。
4. 截图留档：浅色、深色各截取顶部生成器、新增控件区、颜色对照表。

## 风险与对策

- **控件过多导致布局/启动变慢**：全量色卡只在首次构建后缓存更新；对照表 199 行使用紧凑 `GridBox`，避免逐行嵌套。
- **生成器实时刷新卡顿**：仅更新 Label 的 tooltip，不重建控件；`GenerateTheme` 本身为纯计算，保持现状节流方式。
- **绝对色不随主题切换**：对照表与生成结果色卡使用绝对色，因此显式刷新；其余控件使用主题色名，自动切换。
- **Popover/Popconfirm/AutoComplete/CascadingSelect 的 Flyout 主题继承**：实测其浮层是否跟随窗口私有主题，如不跟随，记录为框架行为并在计划中标注，不强行修改框架。
- **pugixml 依赖**：当前 `DUILIB_DLL=OFF` 静态链接可用；若未来改为 DLL 构建，需改为通过 duilib 公共 API 获取字体/变量信息。
- **StatusBar 无主题 Class**：仅作外观预览，不视为 `global.xml` 覆盖项。
