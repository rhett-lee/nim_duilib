简体中文 | [English](TitleBar.en.md)

## 自绘制标题栏规范

使用自绘标题栏（`use_system_caption="false"`，或窗口带有 `window_title_bar`）时，
标题栏的尺寸、字体、图标和窗口按钮建议遵循本文的统一规格。
示例程序的所有窗口均已按此规范配置，可参考 `bin/resources/themes/default` 下的窗口 XML。

标题栏相关窗口属性（`caption`、`sys_menu`、`sys_menu_rect` 等）的含义，
参见窗口属性文档：[Window.md](Window.md)。

## 1. 统一规格

| 项目 | 配置位置 | 统一值 | 说明 |
| :--- | :--- | :--- | :--- |
| 标题栏高度 | `Window` 的 `caption` | `0,0,0,36`（字面量） | 同时决定可拖动区域与标题栏视觉高度，两者必须一致；改为其它值时需同步 `SIZE_TITLEBAR_HEIGHT` |
| 系统菜单区域 | `Window` 的 `sys_menu_rect` | `0,0,36,36`（字面量） | 需与标题栏高度一致；不设置则不启用该功能 |
| 标题栏容器 | `HBox name="window_title_bar"` | 宽 `stretch`、高 `${SIZE_TITLEBAR_HEIGHT}` | 背景色使用 `bkcolor="bg_titlebar"` |
| 左上角图标 | `Control class="title_bar_logo"` | 公共类，见 `global.xml` | 类中已包含 `${SIZE_TITLE_LOGO}`（18）、`margin="8,0,0,0"`、`valign="center"` 与 Logo 图片 |
| 标题文字 | `Label class="title_bar_title"` | 公共类，见 `global.xml` | 类中已包含 `font="system_regular_14"`、`margin="8,0,0,0"`、`valign="center"`；拉伸标题需覆盖 `margin="8,0,8,0"` |
| 普通窗口按钮 | `Button` 引用窗口按钮类 | 尺寸由 `global.xml` 的类定义 | `btn_wnd_base` 统一为 `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}`（40×32）；窗口 XML 只需 `margin="0,2,0,2"` |
| 最大化/还原 | `Box` + 两个 `Button` | 外层 `Box` 定尺寸，按钮类内为 `stretch` | `Box` 为 `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}`、`margin="0,2,0,2"`；按钮的拉伸与 `margin="0"` 已由类提供 |
| 关闭按钮 | `Button` 引用窗口按钮类 | `btn_wnd_close_11` 中为 `height="stretch"` | 宽度由基类提供，关闭按钮高度撑满；窗口 XML 只需 `margin="0"` |

上述 `${SIZE_*}` 变量定义在 [global.xml](../bin/resources/themes/default/global.xml) 的
“尺寸与路径变量”章节。**注意：`Window` 节点的属性（`caption`、`sys_menu_rect` 等）
不参与变量展开**，只能用字面量书写，因此修改标题栏高度时需要同时修改
`caption` 与 `SIZE_TITLEBAR_HEIGHT`，二者保持一致。

## 2. 标准结构

```xml
<Window size="900,600" caption="0,0,0,36"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" size_box="4,4,4,4" icon="public/caption/logo.ico">
    <VBox bkcolor="bg_window_main">
        <!-- 标题栏 -->
        <HBox name="window_title_bar" width="stretch" height="${SIZE_TITLEBAR_HEIGHT}" bkcolor="bg_titlebar">
            <!-- 左侧：图标 + 标题（使用 global.xml 中的公共 Class） -->
            <HBox margin="0,0,30,0" valign="center" width="auto" height="auto" mouse_enabled="false">
                <Control class="title_bar_logo"/>
                <Label class="title_bar_title" name="label_title" text_id="STRID_XXX_WINDOW_TITLE"/>
            </HBox>
            <!-- 中间空白，占满剩余宽度 -->
            <Control mouse_enabled="false"/>
            <!-- 右侧：窗口按钮（按需增减，不要改变已有按钮种类） -->
            <HBox margin="0,0,0,0" valign="center" width="auto" height="${SIZE_TITLEBAR_HEIGHT}">
                <Button class="btn_switch_theme" name="btn_window_theme" margin="0,2,0,2"/>
                <Button class="btn_switch_lang" name="btn_window_language" margin="0,2,0,2"/>
                <Button class="btn_wnd_min_11" name="btn_window_min" margin="0,2,0,2"/>
                <!-- 最大化/还原：两个按钮叠放在同一位置，由框架切换显示 -->
                <Box height="${SIZE_WND_BTN_HEIGHT}" width="${SIZE_WND_BTN_WIDTH}" margin="0,2,0,2">
                    <Button class="btn_wnd_max_11" name="btn_window_max" margin="0"/>
                    <Button class="btn_wnd_restore_11" name="btn_window_restore" visible="false" margin="0"/>
                </Box>
                <!-- 关闭按钮：满高、贴右、无空隙 -->
                <Button class="btn_wnd_close_11" name="btn_window_close" margin="0"/>
            </HBox>
        </HBox>

        <!-- 内容区 -->
        <VBox bkcolor="bg_window_main" padding="10,8,10,10">
        </VBox>
    </VBox>
</Window>
```

## 3. 窗口按钮类与控件名

按钮外观由主题中的类定义（见 [global.xml](../bin/resources/themes/default/global.xml)），
控件名用于框架自动绑定窗口操作，二者需配合使用。

| 按钮类 | 建议控件名 | 用途 | 尺寸 |
| :--- | :--- | :--- | :--- |
| `btn_wnd_min_11` | `btn_window_min` | 最小化 | 40×32 |
| `btn_wnd_max_11` | `btn_window_max` | 最大化 | 放在 40×32 的 `Box` 内，内部拉伸 |
| `btn_wnd_restore_11` | `btn_window_restore` | 还原 | 与最大化叠放，`visible="false"` 初始隐藏 |
| `btn_wnd_fullscreen_11` | `btn_window_fullscreen` | 全屏 | 40×32 |
| `btn_wnd_close_11` | `btn_window_close` | 关闭 | 40×`stretch` |
| `btn_switch_theme` | `btn_window_theme` | 切换主题（框架自动绑定菜单） | 40×32 |
| `btn_switch_lang` | `btn_window_language` | 切换语言（框架自动绑定菜单） | 40×32 |
| `btn_wnd_settings_11` | 自定义（如 `settings`） | 设置入口 | 40×32 |
| `btn_view_fullscreen_11` | 自定义 | 页面全屏（非窗口全屏） | 40×32 |

说明：

- `btn_window_min`、`btn_window_max`、`btn_window_restore`、`btn_window_close`、
  `btn_window_fullscreen`、`btn_window_theme`、`btn_window_language`
  这些控件名会被 `WindowImplBase` 自动识别并绑定默认行为（见
  [WinImplBase.cpp](../duilib/Utils/WinImplBase.cpp)）。
- 类名中的 `_11` 是样式版本后缀；图标尺寸由 `SIZE_WND_ICON` 统一控制（默认 20）。
- 窗口按钮的宽度和高度已在 `global.xml` 的类中统一控制（`btn_wnd_base` 为 40×32，
  关闭按钮类内高度为 `stretch`），窗口 XML 不需要再写 `width`/`height`。
- 标题栏上原有的按钮种类保持不变，只按本规范调整尺寸、边距等外观属性。

## 4. 关键规则

1. **`caption` 与标题栏高度必须一致**
   `caption` 的最后一个值是可拖动区域距窗口顶部的距离，也是标题栏高度。
   两者不一致会出现“按钮可点击但拖不动”或“标题栏下方多出空白”的问题。
   `caption` 只能用字面量书写（窗口属性不支持变量展开），修改时需同时调整
   `SIZE_TITLEBAR_HEIGHT`。

2. **关闭按钮必须满高贴边**
   `btn_wnd_close_11` 已把高度设为 `stretch`，窗口 XML 中只写 `margin="0"`，
   关闭按钮便与标题栏等高，并紧贴窗口顶部与右侧；不要写 `margin="0,0,0,2"`
   或在 XML 中覆盖高度，否则关闭按钮上方或右侧会出现空隙。

3. **普通按钮保持上下各 2 像素**
   标题栏 36、按钮高 32、`margin="0,2,0,2"`，三者在垂直方向正好对齐。
   按钮宽高由类提供，不要在窗口 XML 中覆盖；调整整体尺寸时，
   修改 `SIZE_TITLEBAR_HEIGHT` 与 `SIZE_WND_BTN_HEIGHT`（保持相差 4）即可。

4. **最大化/还原按钮使用叠放容器**
   外层 `Box` 使用 `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}` 与
   `margin="0,2,0,2"`；两个按钮的 `width="stretch" height="stretch" margin="0"`
   已由对应类提供，窗口 XML 中只写 `class`、`name` 与 `visible` 等属性，
   由框架根据窗口状态切换 `visible`，避免两个按钮同时出现。

5. **标题文字统一使用公共类**
   标题 `Label` 直接使用 `class="title_bar_title"`（已在 `global.xml` 中定义
   `font="system_regular_14"`、`margin="8,0,0,0"`、`valign="center"`）；
   宽度拉伸的标题覆盖 `margin="8,0,8,0"`，不要重复书写字体与对齐属性。

6. **百分比宽度的标题需要覆盖边距**
   若标题 `Label` 使用 `width="100%"`，需追加 `margin="0"`，
   否则类中的左边距会使标题超出标题栏宽度并挤压右侧按钮；可改用
   `text_padding` 控制文字缩进。

7. **标题栏包含标签页等复杂内容时**
   参考 `cef_browser.xml` / `webview2_browser.xml`：外层横条可命名为 `title_bar`，
   右侧窗口按钮放入内层 `HBox name="window_title_bar" width="auto" height="${SIZE_TITLEBAR_HEIGHT}"`，
   两层高度均使用 `${SIZE_TITLEBAR_HEIGHT}`。

8. **浮动式标题栏**
   部分窗口（如 `chat/login.xml`）不使用标题栏背景，只在右上角浮动显示按钮。
   这种情况仍使用相同的按钮尺寸与边距规范，关闭按钮同样使用满高 40 宽。

9. **公共类的定义与定制**
   `title_bar_logo` 与 `title_bar_title` 定义在
   [global.xml](../bin/resources/themes/default/global.xml) 中。
   个别窗口如需使用自己的 Logo 图片（如 `chat/wechat.xml`），
   在引用 `class="title_bar_logo"` 的同时覆盖 `bkimage` 即可。

## 5. 常见问题

| 现象 | 原因 | 处理 |
| :--- | :--- | :--- |
| 关闭按钮上方/右侧有 2~4 像素空隙 | 按钮高度为固定值，或 `margin` 底部/右侧非 0 | 引用 `btn_wnd_close_11`（高度已为 `stretch`），窗口 XML 只写 `margin="0"` |
| 标题栏变矮后内容区顶部出现空白 | 只改了 `caption`，未改标题栏容器高度 | 同时调整 `caption` 与 `SIZE_TITLEBAR_HEIGHT` |
| 标题与右侧按钮间距过小 | 标题 `margin` 右侧为 0 且无分隔控件 | 使用 `margin="8,0,8,0"`，或保留中间 `<Control/>` 占位 |
| 标题文字被按钮遮挡 | 标题使用百分比/固定宽度过大 | 去掉百分比标题的 `margin`，或改用 `width="stretch"` |
| 标题字号与其它窗口不一致 | 未引用 `title_bar_title`，或单独覆盖了 `font` | 统一使用 `class="title_bar_title"` |
| 最大化与还原按钮同时显示 | 未使用叠放 `Box`，或都未设置 `visible` | 使用 `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}` 的叠放 `Box`，还原按钮初始 `visible="false"` |

## 6. 自检清单

- [ ] `caption="0,0,0,36"`，`sys_menu_rect="0,0,36,36"`（若启用系统菜单），且与 `SIZE_TITLEBAR_HEIGHT` 一致
- [ ] 标题栏容器 `height="${SIZE_TITLEBAR_HEIGHT}"`，右侧按钮组同高
- [ ] Logo 使用 `class="title_bar_logo"`（自定义图片时覆盖 `bkimage`）
- [ ] 标题使用 `class="title_bar_title"`，拉伸标题覆盖 `margin="8,0,8,0"`
- [ ] 普通按钮直接引用窗口按钮类，尺寸由类提供，XML 只写 `margin="0,2,0,2"`
- [ ] 最大化/还原按钮放在 `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}` 的 `Box` 内，按钮类已内置拉伸
- [ ] 关闭按钮引用 `btn_wnd_close_11`（高度 `stretch` 已在类中），XML 只写 `margin="0"`
- [ ] 中文与英文界面下标题均不换行、不与按钮重叠
- [ ] 标题栏原有按钮的种类和数量未改变

## 7. 参考文件

| 文件 | 说明 |
| :--- | :--- |
| [Window.md](Window.md) | 窗口属性说明（`caption`、`use_system_caption`、`sys_menu_rect` 等） |
| [global.xml](../bin/resources/themes/default/global.xml) | 窗口按钮类与图标尺寸定义 |
| [label.xml](../bin/resources/themes/default/controls/label.xml) | 最简标题栏示例 |
| [controls.xml](../bin/resources/themes/default/controls/controls.xml) | 按钮较完整的标题栏示例 |
| [cef_browser.xml](../bin/resources/themes/default/cef_browser/cef_browser.xml) | 标签页 + 窗口按钮的标题栏示例 |
| [login.xml](../bin/resources/themes/default/chat/login.xml) | 浮动式标题栏示例 |
