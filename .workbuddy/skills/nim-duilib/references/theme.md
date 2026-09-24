# 主题与样式定制

<!-- verify:allow-missing bk_main_wnd_title my_card my_title_font my_font_16 -->
<!-- bk_main_wnd_title 是已废弃的旧色名；my_card / my_title_font / my_font_16 是自定义样式举例名。
     以上均为刻意举例，verify_docs.py 不应判为漂移。 -->

## 一、资源文件规则

### 新项目必须复制的资源（最小集）

```
bin/resources/
├── themes/
│   ├── default/
│   │   ├── global.xml                    # 必须：Class/字体/兼容别名
│   │   ├── public/                       # 必须：共享图片资源
│   │   │   ├── button/                   # 窗口按钮图标(SVG)
│   │   │   ├── caption/                  # 标题栏资源
│   │   │   ├── checkbox/                 # 复选框图标
│   │   │   ├── combo/                    # 下拉框图标
│   │   │   ├── option/                   # 单选按钮图标
│   │   │   ├── scrollbar01/              # 滚动条样式1
│   │   │   ├── scrollbar02/              # 滚动条样式2
│   │   │   ├── shadow/                   # 窗口阴影
│   │   │   ├── slider/                   # 滑块资源
│   │   │   ├── tooltip/                  # 工具提示
│   │   │   ├── tree/                     # 树控件展开/折叠图标
│   │   │   ├── menu/                     # 菜单资源
│   │   │   ├── progress/                 # 进度条资源
│   │   │   ├── animation/                # 加载动画(JSON)
│   │   │   └── ...                       # 其他公共资源
│   │   └── <your_app>/                   # 应用专属XML和图片
│   │       └── your_form.xml
│   ├── color_light/                      # ★ 必须：浅色主题的 <ThemeColor> 色值 (global.xml)
│   └── color_dark/                       # ★ 必须：深色主题的 <ThemeColor> 色值 (global.xml)
├── fonts/                                # 可选：自定义字体文件
│   └── RobotoMono-*.ttf
└── lang/                                 # 可选：多语言文件
    ├── zh_CN.txt
    └── en_US.txt
```

> `public/` 的实际文件数会随版本变化，复制时**以目录为准、不要照抄数字**。
> 语言文件在资源根目录的 **`lang/`**（`bin/resources/lang/`），**不在** `themes/default/lang/`。
> 颜色定义同样不在 `themes/default/global.xml`，而在 `bin/resources/themes/color_light|color_dark/global.xml`。
> **★ 这两个颜色目录必须随包发布**：`GlobalManager::Startup()` 会按系统深浅色模式自动加载其中一个
> （`GlobalManager.cpp:518-524`），缺了整个界面会没有颜色。

### 严禁复制的内容

- **不要复制** `themes/default/basic/`、`controls/`、`layout/` 等示例目录
- **不要复制** `themes/default/chat/`、`cef/`、`cef_browser/` 等 demo 目录
- **不要复制** `bin/*.exe`、`bin/*.dll` 等二进制文件
- **不要复制** `bin/bin.zip`
- 只创建**应用自己的**皮肤目录（如 `themes/default/my_app/`）

### 资源引用路径规则

```xml
<!-- 引用 public 目录下的共享资源（使用相对路径） -->
normal_image="file='public/button/window-minimize.svg' width='24' height='24'"

<!-- 引用同目录下的资源（不需要路径前缀） -->
bkimage="my_background.png"

<!-- 引用上级目录的 public 资源 -->
bkimage="file='../public/shadow/shadow_big.svg' corner='64,64,68,70'"
```

## 二、预定义颜色速查

> **重要**：颜色不再在 `themes/default/global.xml` 里写死。该文件只剩下"旧色名 → 新语义色名"的
> `<Alias>` 映射；真正的色值定义在 `bin/resources/themes/color_light/global.xml`（浅色）和
> `bin/resources/themes/color_dark/global.xml`（深色）的 `<ThemeColor>` 中，由框架按当前主题加载。
> **写 XML 时一律使用语义色名，不要把下面的色值硬编码进业务布局**，否则深色主题会失效。
>
> 完整色表、命名规范与派生规则见仓库 `docs/ThemeColor.md`。下列仅为浅色主题取值（`font` 色同 `text_`）。

### 背景色

| 语义色名 | 浅色取值 | global.xml 兼容别名（旧名） | 用途 |
|----------|----------|----------------------------|------|
| bg_window_main | #FFF4F4F4 | `bk_wnd_darkcolor` | 窗口主背景 |
| bg_window_card | #FFFAFAFA | - | 卡片背景 |
| bg_container | #FFF9F9F9 | `bk_wnd_lightcolor` | 容器背景 |
| bg_content | #FFF9F9F9 | - | 内容区背景 |
| bg_titlebar | #FFEAEAEA | - | **标题栏背景**（替代已废弃的 `bk_main_wnd_title`） |
| bg_header | #FFF4F4F4 | - | 表头背景 |
| bg_list_item_hovered | #FFEAEAEA | `bk_listitem_hovered` | 列表项悬浮 |
| bg_list_item_selected | #FFE2E2E2 | `bk_listitem_selected` | 列表项选中 |
| bg_menu_item_hovered | #FFEBEBEB | `bk_menuitem_hovered` | 菜单项悬浮 |
| bg_menu_item_selected | #FFE3E3E3 | `bk_menuitem_selected` | 菜单项选中 |

### 文字色

| 语义色名 | 浅色取值 | global.xml 兼容别名（旧名） | 用途 |
|----------|----------|----------------------------|------|
| text_default | #FF1A1A1A | `default_font_color` | 主文本 |
| text_disabled | #B3343434 | `disabled_font_color` | 禁用文本 |
| text_muted | #FF1A1A1A | - | 次要文本 |
| text_selected | #FF030303 | - | 选中态文本 |
| text_link_normal | #FF1A1A1A | `default_link_font_color` | 超链接 |
| text_link_hovered | #FF0B0B0B | `hover_link_font_color` | 超链接悬浮 |
| text_link_pressed | #FF000000 | `mouse_down_link_font_color` | 超链接按下 |

### 基础色 / 分割线色

| 语义色名 | 浅色取值 | global.xml 兼容别名（旧名） | 用途 |
|----------|----------|----------------------------|------|
| color_white | #FFF2F2F2 | `white` | 白色 |
| color_black | #FF1F1F1F | - | 黑色 |
| color_blue | #FF2966A3 | `blue` | 蓝色 |
| color_blue_dark / color_blue_light | #FF143352 / #FF5C99D6 | - | 蓝色深浅变体 |
| color_red | #FFA32929 | `red` | 红色 |
| color_green | #FF29A329 | `green` | 绿色 |
| color_gray_dark | #FF4D4D4D | `darkcolor` / `dark_gray` | 深灰 |
| color_gray_light | #FFB3B3B3 | `lightcolor` / `light_gray` | 浅灰 |
| color_green_light | #FF5CD65C | `light_green` | 浅绿 |
| color_success / color_warning / color_error | #FF22A95B / #FFD08700 / #FFE85854 | - | 状态色 |
| color_accent | #FF0078D4 | - | 主题强调色 |
| border_split_level1 | #FFE7E7E7 | `splitline_level1` | 一级分割线 |
| border_split_level2 | #FFE7E7E7 | `splitline_level2` | 二级分割线 |

### 自定义颜色

在 `bin/resources/themes/color_light/global.xml`（以及对应的深色文件）的 `<Global>` 节点内添加：

```xml
<ThemeColor name="my_brand_color" value="#FF1890FF" type="common" category="bg_color"
            role="neutral" derived_from="" adjust="" fixed="false" support_accent="false"
            comment_cn="品牌色" comment_en="Brand color"/>
```

颜色格式：`#AARRGGBB`（ARGB）或 `#RRGGBB`（RGB）或预定义名（Blue/Red/White...）

> `WindowBuilder` 的 `<Global>` 子节点白名单里同时接受 `<ThemeColor>` 和 `<TextColor>`
> （`WindowBuilder.cpp:1101-1102`），但项目 v2.0 惯例统一用 `<ThemeColor>`，它额外支持
> `derived_from` / `adjust` / `support_accent` / `contrast_bg` 等主题派生能力。
> 若只是想给已有颜色起别名，用 `<Alias name="旧名" value="新名"/>`。

## 三、预定义字体ID速查

命名规则：`system_<样式>_<字号>`，字号固定为 **12 / 14 / 16 / 18 / 20 / 22** 六档。

| 字体ID 系列 | 样式 | 完整 ID 列表 |
|-------------|------|-------------|
| system_regular_* | 常规 | `system_regular_12` `system_regular_14` `system_regular_16` `system_regular_18` `system_regular_20` `system_regular_22` |
| system_bold_* | 粗体 | `system_bold_12` `system_bold_14` `system_bold_16` `system_bold_18` `system_bold_20` `system_bold_22` |
| system_underline_* | 下划线 | `system_underline_12` `system_underline_14` `system_underline_16` `system_underline_18` `system_underline_20` `system_underline_22` |
| system_italic_* | 斜体 | `system_italic_12` `system_italic_14` `system_italic_16` `system_italic_18` `system_italic_20` `system_italic_22` |
| system_strikeout_* | 删除线 | `system_strikeout_12` `system_strikeout_14` `system_strikeout_16` `system_strikeout_18` `system_strikeout_20` `system_strikeout_22` |
| system_fullstyle_* | 粗+斜+下划线+删除线 | `system_fullstyle_12` `system_fullstyle_14` `system_fullstyle_16` `system_fullstyle_18` `system_fullstyle_20` `system_fullstyle_22` |
| system_* （旧，仅兼容） | 常规 | `system_12` `system_14` `system_16` `system_18` `system_20` `system_22` |

> 下划线/斜体/删除线**同样各有 12–22 六档**，不是只有 12 号。
> **不存在 `arial_*` 系列字体**（该系列已从 `global.xml` 移除）。

默认字体（未指定 font 时使用）：**`system_regular_14`**（`<Font id="system_regular_14" ... default="true"/>`）。

### 自定义字体

```xml
<!-- 定义新字体ID -->
<Font id="my_title_font" name="Microsoft YaHei" size="24" bold="true"/>

<!-- 使用自带字体文件（放在 resources/fonts/ 目录，路径相对资源根目录） -->
<FontFile file="fonts/MyFont-Regular.ttf" desc="我的字体，常规"/>
<Font id="my_font_16" name="MyFont" size="16"/>
```

## 四、预定义通用样式(Class)速查

### 按钮样式

| Class名 | 说明 |
|---------|------|
| btn_global_blue_80x30 | 蓝色按钮(图片) |
| btn_global_white_80x30 | 白色按钮(图片) |
| btn_global_red_80x30 | 红色按钮(图片) |
| btn_global_gray_80x30 | 灰色按钮(图片) |
| btn_global_color_blue | 蓝色按钮(纯色) |
| btn_global_color_white | 白色按钮(纯色) |
| btn_global_color_red | 红色按钮(纯色) |
| btn_global_color_gray | 灰色按钮(纯色) |
| btn_wnd_min_11 | 窗口最小化按钮 |
| btn_wnd_max_11 | 窗口最大化按钮 |
| btn_wnd_restore_11 | 窗口还原按钮 |
| btn_wnd_close_11 | 窗口关闭按钮 |
| btn_wnd_fullscreen_11 | 全屏按钮 |

### 输入控件样式

| Class名 | 说明 |
|---------|------|
| simple | 简单输入框（无边框） |
| simple_border | 带边框输入框 |
| simple_border_bottom | 底部边框输入框（聚焦高亮） |
| combo | 下拉组合框 |
| filter_combo | 可过滤下拉框 |
| check_combo | 多选下拉框 |
| ip_address | IP地址输入 |
| hot_key | 热键输入 |

### 选择控件样式

| Class名 | 说明 |
|---------|------|
| checkbox_1 | 复选框样式1 |
| checkbox_2 | 复选框样式2 |
| checkbox_toggle_1 | 开关样式1 |
| checkbox_toggle_2 | 开关样式2 |
| option_1 | 单选按钮样式1 |
| option_2 | 单选按钮样式2 |

### 列表/树样式

| Class名 | 说明 |
|---------|------|
| list | 列表容器 |
| list_item | 列表项（`listitem` 是通过 `<Alias>` 定义的旧名，两者等价，推荐写 `list_item`） |
| tree_view | 树控件 |
| tree_node | 树节点 |
| tree_node_checkbox | 带复选框的树节点 |

### 进度条/滑块样式

| Class名 | 说明 |
|---------|------|
| progress_horizontal_blue | 水平蓝色进度条 |
| progress_vertical_blue | 垂直蓝色进度条 |
| slider_horizontal_blue | 水平蓝色滑块 |
| slider_vertical_blue | 垂直蓝色滑块 |

### 菜单样式

| Class名 | 说明 |
|---------|------|
| menu | 菜单容器 |
| menu_element | 菜单项 |
| menu_text | 菜单文字 |
| menu_split_line | 菜单分割线 |

### 其他

| Class名 | 说明 |
|---------|------|
| tab_ctrl / tab_ctrl_item | 标签页 |
| vscrollbar / hscrollbar | 滚动条样式1（简洁） |
| vscrollbar2 / hscrollbar2 | 滚动条样式2（带按钮） |
| rich_text | 富文本 |
| hyper_link | 超链接 |
| splitline_hor_level1 | 水平分割线（一级） |
| splitline_hor_level2 | 水平分割线（二级） |
| splitline_ver_level1 | 垂直分割线（一级） |
| splitline_ver_level2 | 垂直分割线（二级） |

> Class 名核对方式：`grep -o '<Class name="[^"]*"' bin/resources/themes/default/global.xml`。
> 写 XML 前先查一遍，避免引用不存在的 Class（`class` 引用失败时控件会没有任何样式且不报错）。

### 自定义通用样式

```xml
<!-- 在 global.xml 中定义 -->
<Class name="my_card"
       bkcolor="white" border_size="1" border_color="light_gray"
       border_round="8,8" padding="12,12,12,12"
       box_shadow="color='#20000000' offset='0,2' blurradius='8' spreadradius='0'"/>

<!-- 在 XML 布局中使用（class 必须在最前面） -->
<Box class="my_card" width="200" height="150">
    <Label text="卡片内容"/>
</Box>
```

**注意：class 属性必须写在所有属性最前面。** 后续属性可覆盖 class 中的同名属性。
