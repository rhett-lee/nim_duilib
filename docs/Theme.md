# 主题（Theme）使用指南

本文从用户使用角度介绍 duilib 的主题（Theme）功能，包括主题结构、XML 配置、C++ 代码接口、主题切换以及自定义主题的步骤等，供开发者参考。

---

## 1. 功能简介

duilib 的"主题"是一组**可热切换**的资源集合，主要包括：

- **颜色主题（Color Theme）**：以 `global.xml` 中 `<ThemeColor>` 节点定义的颜色为基础，提供整套可重命名的语义化颜色。
- **图标主题（Icon Theme）**：可热切换的图标、图片资源（按钮图标、窗口按钮图标等）。
- **基础主题（Base Theme）**：默认（`default`）主题，保存各个主题公用的资源（图片、XML 布局等）。

主题切换会**立即生效**（所有窗口重绘），并且支持：

- 应用启动时按需指定（跟随系统 / 强制深色 / 强制浅色）；
- 程序运行中通过接口动态切换（全局切换 / 单窗口切换）；
- 同一进程内可存在多套主题（不同窗口使用不同主题）。

> 术语说明：本文中"主题"泛指 Color/Icon/Base，"颜色主题"专指 Color 主题。

---

## 2. 主题目录结构

主题资源统一放在 `<资源根>/resources/themes/` 下，典型结构如下：

```
resources/
└── themes/
    ├── default/                # 默认主题（公用资源）
    │   ├── global.xml          # 全局资源（Class、字体、变量）
    │   ├── public/             # 公共图片、阴影、按钮、菜单等
    │   ├── basic/
    │   ├── controls/
    │   ├── render/
    │   └── ...
    │
    ├── color_light/            # 浅色主题（颜色定义）
    │   └── global.xml
    ├── color_dark/             # 深色主题（颜色定义）
    │   └── global.xml
    ├── color_aquatic/          # 其他可选主题
    ├── color_desert/
    ├── color_dusk/
    └── color_night_sky/
```

- `default/`：存放**公用资源**（图片、控件 Class、字体定义等），**所有主题共享**。
- `color_xxx/`：每个子目录代表一套**颜色主题**，目录下放 `global.xml`，其中主要包含 `<Theme>` 节点和大量 `<ThemeColor>` 颜色定义。
- 资源查找顺序：先在当前**颜色主题目录**查找 → 再到**图标主题目录**查找 → 最后回退到 `default/`，详见 [主题资源查找](#_7-主题资源查找)。

---

## 3. 快速上手

### 3.1 在 XML 中使用颜色

定义好主题颜色后，在 XML 中通过 `bkcolor`、`normal_text_color`、`border_color` 等属性**按颜色名引用**：

```xml
<Window>
    <VBox bkcolor="bg_window_main">
        <Label text="Hello" normal_text_color="text_default" font="system_bold"/>
        <Button class="btn_primary_global" text="Primary Button"/>
    </VBox>
</Window>
```

切换主题时，引用的颜色名不变，duilib 会自动取当前主题下同名颜色的实际 ARGB 值。

### 3.2 启动时指定主题

```cpp
ui::LocalFilesResParam resParam;
resParam.resourcePath = ui::FilePath(_T("resources/"));
resParam.themeRootPath = ui::FilePath(_T("themes"));
resParam.defaultThemePath = ui::FilePath(_T("default"));
resParam.colorThemePath = ui::FilePath(_T("color_dark"));   // 启动时使用深色主题
// resParam.colorThemePath = ui::FilePath(_T(""));          // 留空则跟随系统

ui::GlobalManager::Instance().Startup(resParam);
```

### 3.3 运行时切换全局主题

```cpp
// 切换为深色主题
ui::GlobalManager::Instance().Theme().SwitchColorTheme(ui::FilePath(_T("color_dark")));

// 切换为浅色主题
ui::GlobalManager::Instance().Theme().SwitchColorTheme(ui::FilePath(_T("color_light")));
```

切换后，全局 `ColorManager` 中的颜色表会替换，**所有未缓存自定义 ColorManager 的窗口会自动重绘**。

---

## 4. 主题 XML 结构

`global.xml`（也称"全局资源描述 XML"）的根节点是 `<Global>`，常用子节点如下：

### 4.1 `<Theme>` 主题元信息

```xml
<Theme name="Light" type="color" style="light" accent_support="true" version="2.0"/>
```

| 属性 | 必填 | 取值 | 说明 |
| :--- | :---: | :--- | :--- |
| `name` | ✅ | `Light` / `Dark` / `Default` 等 | 主题显示名 |
| `type` | ✅ | `color` / `icon` / `combined` / `base` | 主题类型（详见 [§5 主题类型与风格](#_5-主题类型与风格)） |
| `style` | ✅ | `light` / `dark` / `combined` | 主题风格（影响控件暗色模式判断） |
| `accent_support` | - | `true` / `false` | 是否支持"强调色替换"特性（v2.0+） |
| `version` | - | `2.0` | XML 格式版本号 |

### 4.2 `<ThemeMeta>` 主题元数据

```xml
<ThemeMeta>
    <Property name="author"           value="nim_duilib"/>
    <Property name="contrast_ratio"   value="7.2"/>             <!-- 对比度 -->
    <Property name="accent_color"     value="#FF0078D4"/>        <!-- 强调色锚点 -->
    <Property name="base_luminance"   value="light"/>           <!-- light / dark -->
    <Property name="description"      value="Light theme ..."/>
</ThemeMeta>
```

用于记录作者、对比度、强调色等扩展信息，便于主题编辑工具识别。

### 4.3 `<ThemeColor>` 颜色定义（核心）

```xml
<ThemeColor
    name="color_blue"             <!-- 必填：颜色名（唯一） -->
    value="#FF2966A3"             <!-- 必填：ARGB 颜色值 -->
    type="common"                 <!-- 控件类型分组 -->
    category="basic_color"        <!-- 颜色用途分类 -->
    role="primary"                <!-- 角色（与强调色派生相关） -->
    derived_from="color_gray"     <!-- 派生来源 -->
    adjust="lightness:+15"        <!-- 派生调整公式 -->
    fixed="false"                 <!-- 是否参与强调色替换 -->
    comment_cn="核心强调色"         <!-- 中文注释 -->
    comment_en="Core accent color"/>  <!-- 英文注释 -->
```

详细属性说明参见 [ThemeColor.md](./ThemeColor.md)，常用规则如下：

- `name` 全局唯一，按 `snake_case` 命名，推荐遵循 `{prefix}_{component}[_{state}]` 模式：
  - 用途前缀：`bg_`（背景）、`border_`（边框）、`text_`（文本/前景）、`color_`（基础调色板）。
  - 状态后缀：`_normal` / `_hover` / `_pressed` / `_disabled` / `_focus` / `_selected`。
- `value` 是 ARGB 字符串：`#AARRGGBB`，Alpha 在前。
- `role` 取值见 [§5 角色与派生链](#_6-强调色派生系统可选)。
- `fixed="true"` 表示该颜色**不参与**强调色替换（语义/中性色）。

### 4.4 其他公共节点

除主题相关节点外，`global.xml` 还可以包含：

- `<Font>` / `<FontFile>`：字体定义（详见 [Global.md](./Global.md)）。
- `<Class>`：全局 Class 样式定义。
- `<Var>`：变量定义。

---

## 5. 主题类型与风格

通过 `ThemeType` 与 `ThemeStyle` 两个枚举描述：

### 5.1 主题类型 `ThemeType`

| 取值 | 含义 |
| :--- | :--- |
| `kUnknown` | 未知 |
| `kColor` | 颜色主题（仅颜色） |
| `kIcon` | 图标主题（仅图标/图片） |
| `kCombined` | 组合主题（颜色 + 图标） |
| `kBase` | 基础主题（即 `default/`，仅公用资源） |

`type` 字段在 XML 中的对应：`color` / `icon` / `combined` / `base`。

### 5.2 主题风格 `ThemeStyle`

| 取值 | 含义 |
| :--- | :--- |
| `kUnknown` | 未知 |
| `kBase` | 基础风格 |
| `kLight` | 浅色风格（影响 `IsColorThemeDarkMode()`） |
| `kDark` | 深色风格 |

`style` 字段在 XML 中的对应：`light` / `dark` / `base` / `combined`。

`style="dark"` 时，duilib 会将当前主题识别为深色（影响阴影、SVG 替换色等默认行为），可通过 `ColorManager::IsColorThemeDarkMode()` 查询。

---

## 6. 强调色派生系统（可选）

> 这是 v2.0 主题格式的可选特性。若只需简单切换"预设主题"，可跳过本节。

强调色（Accent Color）派生允许**在运行时用一个新的主色替换整套主题色**。其核心概念：

- **锚点（Anchor）**：`role="primary"` 的颜色，是派生链的根。
- **派生（Derive）**：`derived_from` 指向源颜色，`adjust` 描述派生规则。
- **固定（Fixed）**：`fixed="true"` 的颜色不参与派生（用于语义色、中性色等）。

示例：

```xml
<!-- 锚点：主蓝色 -->
<ThemeColor name="color_blue" value="#FF0078D4" role="primary" fixed="false"/>

<!-- 派生：亮度 +15（用于 hover 态） -->
<ThemeColor name="color_blue_light" value="#FF3D9EFF"
            role="primary_light" derived_from="color_blue" adjust="lightness:+15" fixed="false"/>

<!-- 派生：亮度 -15（用于 pressed 态） -->
<ThemeColor name="color_blue_dark" value="#FF005A9E"
            role="primary_dark" derived_from="color_blue" adjust="lightness:-15" fixed="false"/>

<!-- 固定色：语义红色（替换主色时不变） -->
<ThemeColor name="color_red" value="#FFDC2626" role="semantic_error" fixed="true"/>
```

`adjust` 语法：`{属性}:{±}{数值}[,{属性}:{±}{数值}]*`，支持 `lightness` / `saturation` / `chroma` / `luminance` / `alpha`。

详细规则参见 [ThemeColor.md](./ThemeColor.md#_4-强调色派生系统设计-accent-color-derivation)。

---

## 7. 主题资源查找

duilib 在加载资源（图片、XML、Class）时按以下顺序查找：

```
1. 当前颜色主题目录          （如 color_dark/）
2. 当前图标主题目录          （如 icon_set_b/）
3. 默认主题目录              （default/）
```

可通过 `GlobalManager::GetExistsResFullPath()` 主动查询资源完整路径。资源打包方式支持：

- **本地文件**：`LocalFilesResParam`，资源以文件夹形式存放在磁盘上。
- **Zip 压缩包**：`ZipFileResParam`，资源打包为 `resources.zip`。
- **内嵌到 exe/dll**：`ResZipFileResParam`（仅 Windows），zip 嵌入为 PE 资源。

---

## 8. C++ 代码接口

### 8.1 启动期参数 `ResourceParam`

详见 [ResourceParam.h](../duilib/Core/ResourceParam.h)：

| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `resourcePath` | FilePath | 资源根目录（绝对路径或相对路径，依赖 `resType`） |
| `themeRootPath` | FilePath | 主题根目录（默认 `themes`） |
| `defaultThemePath` | FilePath | 默认主题路径（默认 `default`） |
| `globalXmlFileName` | DString | 全局 XML 文件名（默认 `global.xml`） |
| `colorThemePath` | FilePath | 启动时使用的颜色主题；**为空则跟随系统** |
| `iconThemePath` | FilePath | 启动时使用的图标主题；可为空 |
| `languagePath` / `languageFileName` | - | 多语言相关（详见 [Global.md](./Global.md)） |

### 8.2 全局主题管理 `ThemeManager`

通过 `GlobalManager::Instance().Theme()` 取得：

| 方法 | 作用 |
| :--- | :--- |
| `InitTheme(themeRoot, defaultPath, globalXml)` | 初始化主题管理器（由 `Startup` 内部调用） |
| `SwitchColorTheme(themePath)` | 切换全局颜色主题（立即生效） |
| `SwitchIconTheme(themePath)` | 切换全局图标主题（立即生效） |
| `GetAllThemes(list, list)` | 枚举可用的主题列表 |
| `GetCurrentColorThemeInfo()` | 获取当前颜色主题信息（含 `themePath`） |
| `GetCurrentIconThemeInfo()` | 获取当前图标主题信息 |
| `GetCurrentThemeStyle()` | 获取当前主题风格（Light/Dark/...） |
| `GetDefaultThemeInfo()` | 获取默认主题信息 |
| `IsSwitchingTheme()` | 是否正在切换主题 |
| `AddThemeChangeCallback(cb, id)` | 注册主题变化回调（窗口/控件可在主题切换时刷新） |
| `GetSystemColorThemePath()` | 获取"跟随系统"模式下当前应使用的主题路径 |
| `SetLightColorPath(p)` / `SetDarkColorPath(p)` | 自定义"跟随系统"时浅/深主题的路径 |

### 8.3 单窗口颜色主题（不污染全局）

如果只想让**某个窗口**使用另一套颜色（不影响全局），可使用 `Window` 上的接口：

```cpp
// 加载 color_dark 主题的颜色到本窗口
window->OpenColorTheme(ui::FilePath(_T("color_dark")));

// 关闭本窗口的独立主题，回归全局主题
window->CloseColorTheme();

// 或者直接传入 XML 文本
window->OpenColorThemeData(xmlText);
```

> 这些方法**仅影响本窗口的颜色查找**，不影响全局 `ColorManager`，也不影响其它窗口。

### 8.4 颜色查询 `ColorManager`

```cpp
ui::ColorManager& cm = ui::GlobalManager::Instance().Color();
ui::ColorManager& wcm = window->GetColorManager();   // 优先返回本窗口 ColorManager

// 按名称取色（ARGB）
UiColor c = cm.GetColor(_T("text_default"));

// 直接添加/覆盖一个颜色
cm.AddColor(_T("my_color"), _T("#FFFF8800"));

// 判断当前主题是否为深色
bool isDark = cm.IsColorThemeDarkMode();
```

XML 解析后，`<ThemeColor>` 节点会通过 `ColorManager::AddColor` 注册到全局颜色表中；切换主题时颜色表会被清空并重新加载。

### 8.5 主题变化通知

业务代码如果需要响应主题变化，可注册回调：

```cpp
ui::GlobalManager::Instance().Theme().AddThemeChangeCallback(
    [](const ui::ThemeInfo& info) {
        // 主题已切换，可在此刷新业务侧缓存、图标等
    },
    /* callbackId */ 1
);

// 取消注册
ui::GlobalManager::Instance().Theme().RemoveThemeChangeCallback(callback, 1);
```

窗口内置 `OnThemeChanged()` 虚函数（`Window.h`）和 `kWindowThemeChangedMsg` 事件，也可以重写它来响应主题变化。

---

## 9. 跟随系统深浅色

如果希望"应用主题自动跟随操作系统设置"：

```cpp
// 启动时：留空 colorThemePath 即跟随系统
resParam.colorThemePath = ui::FilePath(_T(""));
ui::GlobalManager::Instance().Startup(resParam);

// 程序运行时系统切换深浅色后，duilib 内部会按以下流程处理：
//  1. 调用 ThemeManager::GetSystemColorThemePath() 获取应使用的主题路径
//  2. 自动调用 SwitchColorTheme(...) 切换到对应主题
```

`SetLightColorPath` / `SetDarkColorPath` 可自定义"系统浅色/深色"对应的具体主题路径；**这两个函数必须在 `GlobalManager::Startup` 之前调用**才生效。

> 内置默认路径：`color_light` / `color_dark`，分别对应 `DUILIB_LIGHT_COLOR_PATH` / `DUILIB_DARK_COLOR_PATH` 宏。

---

## 10. 常用 XML 模板

### 10.1 一段典型的浅色 `global.xml`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Global>
    <Theme name="Light" type="color" style="light" accent_support="true" version="2.0"/>
    <ThemeMeta>
        <Property name="author" value="MyApp"/>
        <Property name="accent_color" value="#FF0078D4"/>
        <Property name="base_luminance" value="light"/>
    </ThemeMeta>

    <!-- 基础调色板 -->
    <ThemeColor name="color_white"      value="#FFF2F2F2" type="common" category="basic_color" role="neutral"  fixed="false"/>
    <ThemeColor name="color_black"      value="#FF1F1F1F" type="common" category="basic_color" role="neutral"  fixed="false"/>
    <ThemeColor name="color_gray"       value="#FF808080" type="common" category="basic_color" role="neutral"  fixed="false"/>
    <ThemeColor name="color_blue"       value="#FF2966A3" type="common" category="basic_color" role="primary"  fixed="false"/>
    <ThemeColor name="color_blue_light" value="#FF5C99D6" type="common" category="basic_color" role="primary_light" derived_from="color_blue" adjust="lightness:+20" fixed="false"/>
    <ThemeColor name="color_red"        value="#FFA32929" type="common" category="basic_color" role="semantic_error" fixed="true"/>

    <!-- 窗口 / 文本 -->
    <ThemeColor name="bg_window_main"   value="#FFF4F4F4" type="window" category="bg_color"     role="neutral"  fixed="false"/>
    <ThemeColor name="bg_titlebar"      value="#FFEAEAEA" type="window" category="bg_color"     role="neutral"  fixed="false"/>
    <ThemeColor name="border_window"    value="#FFD5D5D5" type="window" category="border_color" role="neutral"  fixed="false"/>
    <ThemeColor name="text_default"     value="#FF1A1A1A" type="text"   category="text_color"  role="neutral"  fixed="false"/>
    <ThemeColor name="text_link_normal" value="#FF1A1A1A" type="text"   category="text_color"  role="primary"  fixed="false"/>
</Global>
```

### 10.2 在窗口中引用主题色

```xml
<Window>
    <VBox bkcolor="bg_window_main">
        <HBox bkcolor="bg_titlebar" height="36" width="stretch">
            <Label text="标题" normal_text_color="text_default" font="system_bold"/>
        </HBox>
        <Button class="btn_global_blue_80x30_normal" text="确定" normal_text_color="text_link_normal"/>
    </VBox>
</Window>
```

`btn_global_blue_80x30_normal` 的 Class 样式可在 `default/global.xml` 或 `color_xxx/global.xml` 中定义，可使用 `bkcolor="color_blue"` 等主题色实现"随主题变色"。

### 10.3 用 `Event` 动态切换主题

```xml
<Button class="btn_global_gray_80x30_normal" text="切换深色">
    <Event type="click" receiver="#window#" apply_attribute="0" />
</Button>
```

由于主题切换需要调用 C++ 接口（`SwitchColorTheme`），一般通过 `Event` 绑定自定义事件或按钮 `AttachClick` 来实现：

```cpp
btn->AttachClick([](const ui::EventArgs&) {
    ui::GlobalManager::Instance().Theme().SwitchColorTheme(ui::FilePath(_T("color_dark")));
    return true;
});
```

完整示例见 [examples/ColorTheme](../examples/ColorTheme)。

---

## 11. 自定义主题

要新增一套主题，建议按以下步骤：

1. **复制一份现有主题**作为模板（如 `color_light`）。
2. **修改 `<Theme>` 节点**：调整 `name`、`style` 等。
3. **修改 `<ThemeColor>` 颜色值**：
   - 若保留派生关系，可以只改"锚点 + 锚点 light/dark"，其他颜色自动跟随。
   - 若希望完全独立，可直接改所有 `value`。
4. **调整 SVG 颜色替换**：通过 `svg_replace_colors="#B5B5B5|color_gray_light"` 等机制，让 SVG 图标在不同主题下显示不同颜色（详见 [Global.md#图片](./Global.md#_5-图片包括动画图片)）。
5. **将主题目录放到 `themes/` 下**，启动时或运行时通过路径引用即可。

> 主题目录名即为 `themePath`（如 `color_mybrand`），与代码中 `FilePath(_T("color_mybrand"))` 一一对应。

---

## 12. 主题验证工具

duilib 自带以下工具协助主题调试：

- **HTML 主题编辑器**：[docs/Tools/ColorThemeMgr.html](./Tools/ColorThemeMgr.html)，可视化编辑颜色并实时预览强调色替换效果。

---

## 13. 常见问题

**Q1：颜色名写错了，会发生什么？**

解析阶段会忽略无效值；运行时若引用了不存在的颜色名，会回退到 `white` 等内置默认色。建议开启 Debug 断言以尽早发现问题。

**Q2：切换主题后，窗口没有刷新？**

- 如果窗口通过 `OpenColorTheme` 加载了独立 ColorManager，主题切换不影响它，需重新调用 `OpenColorTheme`。
- 自绘缓存的 SVG/图片可能需要 `GlobalManager::ClearThemeCache()` 才能立即刷新（详见 `GlobalManager` 实现）。
- 检查是否注册了主题变化回调（`AddThemeChangeCallback`）并做了正确的重绘。

**Q3：能否不重启就新增一套主题？**

可以。把新主题目录放到 `themes/` 下，运行时调用 `ThemeManager::GetAllThemes` 枚举，再 `SwitchColorTheme` 切换即可。XML 不需要重新编译。

**Q4：跟随系统深浅色不生效？**

- 确保 `colorThemePath` 在 `Startup` 时留空。
- 若自定义了浅/深主题路径，确认 `SetLightColorPath` / `SetDarkColorPath` 在 `Startup` **之前**调用。
- Windows 上需要 OS 设置"应用 > 个性化 > 颜色模式"开启；macOS 上需要"系统设置 > 外观"开启暗色；Linux 取决于桌面环境。

**Q5：SVG 图片里的颜色怎么跟随主题？**

在 `bkimage` 等图片属性上使用 `svg_replace_colors="#原始色|主题色名;#原始色2|主题色名2"`，加载图片时自动替换。详见 [Global.md#图片](./Global.md#_5-图片包括动画图片) 中关于 `svg_replace_colors` 的说明。

**Q6：怎么让按钮背景色随主题变化？**

在 `Class` 样式中用主题色名：`bkcolor="bg_btn_normal"`。切换主题后，Class 中的颜色会自动按新主题重新计算。

---

## 14. 相关文档与源码

- 源码：[ThemeManager.h](../duilib/Core/ThemeManager.h) / [ThemeManager.cpp](../duilib/Core/ThemeManager.cpp)
- 颜色管理：[ColorManager.h](../duilib/Core/ColorManager.h) / [ColorManager.cpp](../duilib/Core/ColorManager.cpp)
- 全局管理：[GlobalManager.h](../duilib/Core/GlobalManager.h) / [GlobalManager.cpp](../duilib/Core/GlobalManager.cpp)
- 资源参数：[ResourceParam.h](../duilib/Core/ResourceParam.h)
- 主题格式规范：[ThemeColor.md](./ThemeColor.md)
- 全局资源节点说明：[Global.md](./Global.md)
- 主题生成器：[ThemeGenerator.h](../duilib/Core/ThemeGenerator.h) / [ThemeGenerator.cpp](../duilib/Core/ThemeGenerator.cpp)
- 示例工程：[examples/ColorTheme](../examples/ColorTheme)
- 内置主题：`color_light/`、`color_dark/`
- 主题编辑器（HTML）：[docs/Tools/ColorThemeMgr.html](./Tools/ColorThemeMgr.html)
