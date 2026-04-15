---
name: nim-duilib-theme
description: nim_duilib 主题定制（颜色、字体、通用样式Class、图标），修改 global.xml 和创建自定义样式
---

# nim_duilib 主题与样式定制

## 重要：资源文件规则

### 新项目必须复制的资源（最小集）
```
bin/resources/
├── themes/default/
│   ├── global.xml                    # 必须：全局样式定义
│   ├── public/                       # 必须：共享图片资源（133个文件）
│   │   ├── button/                   # 窗口按钮图标(SVG)
│   │   ├── caption/                  # 标题栏资源
│   │   ├── checkbox/                 # 复选框图标
│   │   ├── combo/                    # 下拉框图标
│   │   ├── option/                   # 单选按钮图标
│   │   ├── scrollbar01/              # 滚动条样式1
│   │   ├── scrollbar02/              # 滚动条样式2
│   │   ├── shadow/                   # 窗口阴影
│   │   ├── slider/                   # 滑块资源
│   │   ├── tooltip/                  # 工具提示
│   │   ├── tree/                     # 树控件展开/折叠图标
│   │   ├── menu/                     # 菜单资源
│   │   ├── progress/                 # 进度条资源
│   │   ├── animation/                # 加载动画(JSON)
│   │   └── ...                       # 其他公共资源
│   └── <your_app>/                   # 你的应用专属XML和图片
│       └── your_form.xml
├── fonts/                            # 可选：自定义字体文件
│   └── RobotoMono-*.ttf
└── lang/                             # 可选：多语言文件
    ├── zh_CN.txt
    └── en_US.txt
```

### 严禁复制的内容
- **不要复制** `themes/default/basic/`、`controls/`、`layout/` 等示例目录
- **不要复制** `themes/default/chat/`、`cef/`、`cef_browser/` 等 demo 目录
- **不要复制** `bin/*.exe`、`bin/*.dll` 等二进制文件
- **不要复制** `bin/bin.zip`
- 只创建**你的应用自己的**皮肤目录（如 `themes/default/my_app/`）

### 资源引用路径规则
```xml
<!-- 引用 public 目录下的共享资源（使用相对路径） -->
normal_image="file='public/button/window-minimize.svg' width='24' height='24'"

<!-- 引用同目录下的资源（不需要路径前缀） -->
bkimage="my_background.png"

<!-- 引用上级目录的 public 资源 -->
bkimage="file='../public/shadow/shadow_big.svg' corner='64,64,68,70'"
```

## 预定义颜色速查

### 窗口/背景色
| 颜色名 | 值 | 用途 |
|--------|------|------|
| bk_wnd_darkcolor | #FFF0F2F5 | 窗口深色背景（浅灰） |
| bk_wnd_lightcolor | #FFFFFFFF | 窗口浅色背景（白色） |
| bk_main_wnd_title | #FF238EFA | 标题栏蓝色 |
| bk_listitem_hovered | #FFF0F2F5 | 列表项悬浮 |
| bk_listitem_selected | #FFE4E7EB | 列表项选中 |
| bk_menuitem_hovered | #FFE1E6EB | 菜单项悬浮 |

### 文字色
| 颜色名 | 值 | 用途 |
|--------|------|------|
| default_font_color | #FF000000 | 默认黑色文字 |
| disabled_font_color | #FFA1AEBC | 禁用灰色文字 |
| default_link_font_color | #FF0000FF | 超链接蓝色 |
| white | #FFFFFFFF | 白色 |
| darkcolor | #FF333333 | 深灰色 |
| lightcolor | #FF888888 | 浅灰色 |
| blue | #FF006DD9 | 蓝色 |
| red | #FFC63535 | 红色 |
| green | #FF00BB96 | 绿色 |

### 分割线色
| 颜色名 | 值 | 用途 |
|--------|------|------|
| splitline_level1 | #FFD2D4D6 | 深色分割线 |
| splitline_level2 | #FFEBEDF0 | 浅色分割线 |

### 自定义颜色
在 global.xml 中添加：
```xml
<TextColor name="my_brand_color" value="#FF1890FF"/>
```
颜色格式：`#AARRGGBB`（ARGB）或 `#RRGGBB`（RGB）或预定义名（Blue/Red/White...）

## 预定义字体ID速查

| 字体ID | 大小 | 样式 |
|--------|------|------|
| system_12 ~ system_22 | 12-22 | 常规 |
| system_bold_12 ~ system_bold_22 | 12-22 | 粗体 |
| system_underline_12 | 12 | 下划线 |
| system_italic_12 | 12 | 斜体 |
| system_strikeout_12 | 12 | 删除线 |
| arial_12 ~ arial_22 | 12-22 | Arial 常规 |
| arial_bold_12 ~ arial_bold_22 | 12-22 | Arial 粗体 |

默认字体（未指定font时使用）：system_14

### 自定义字体
```xml
<!-- 定义新字体ID -->
<Font id="my_title_font" name="Microsoft YaHei" size="24" bold="true"/>

<!-- 使用自带字体文件（放在 resources/fonts/ 目录） -->
<FontFile file="MyFont-Regular.ttf" desc="我的字体，常规"/>
<Font id="my_font_16" name="MyFont" size="16"/>
```

## 预定义通用样式(Class)速查

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
| listitem | 列表项 |
| tree_view | 树控件 |
| tree_node | 树节点 |
| tree_node_checkbox | 带复选框的树节点 |

### 进度条/滑块样式
| Class名 | 说明 |
|---------|------|
| progress_horizontal_blue | 水平蓝色进度条 |
| progress_vertical_blue | 垂直蓝色进度条 |
| slider_horizontal_green | 水平绿色滑块 |
| slider_vertical_green | 垂直绿色滑块 |

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
| splitline_hor_level1 | 水平分割线（深色） |
| splitline_hor_level2 | 水平分割线（浅色） |
| splitline_ver_level1 | 垂直分割线 |

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
