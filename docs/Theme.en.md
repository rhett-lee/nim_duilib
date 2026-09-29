English | [简体中文](Theme.md)

> Last synced: 2026-09-29

# Theme Usage Guide

This document introduces duilib's theme feature from a user's perspective, including theme structure, XML configuration, C++ code interfaces, theme switching, and the steps for customizing themes, for the reference of developers.

---

## 1. Overview

duilib's "theme" is a set of **hot-swappable** resource collections, mainly including:

- **Color Theme**: based on the colors defined by the `<ThemeColor>` node in `global.xml`, providing a complete set of renameable semantic colors.
- **Icon Theme**: hot-swappable icon and image resources (button icons, window button icons, etc.).
- **Base Theme**: the default (`default`) theme, which stores resources shared by all themes (images, XML layouts, etc.).

Theme switching takes **effect immediately** (all windows are redrawn), and supports:

- Specified on demand when the application starts (follow system / force dark / force light);
- Dynamically switched via interfaces during program execution (global switch / single-window switch);
- Multiple themes can exist within the same process (different windows use different themes).

> Terminology note: in this document, "theme" refers broadly to Color/Icon/Base, while "color theme" specifically refers to the Color theme.

---

## 2. Theme Directory Structure

Theme resources are uniformly placed under `<resource root>/resources/themes/`, with a typical structure as follows:

```
resources/
└── themes/
    ├── default/                # Default theme (shared resources)
    │   ├── global.xml          # Global resources (Class, fonts, variables)
    │   ├── public/             # Common images, shadows, buttons, menus, etc.
    │   ├── basic/
    │   ├── controls/
    │   ├── render/
    │   └── ...
    │
    ├── color_light/            # Light theme (color definitions)
    │   └── global.xml
    ├── color_dark/             # Dark theme (color definitions)
    │   └── global.xml
    ├── color_aquatic/          # Other optional themes
    ├── color_desert/
    ├── color_dusk/
    └── color_night_sky/
```

- `default/`: stores **shared resources** (images, control Class, font definitions, etc.), **shared by all themes**.
- `color_xxx/`: each subdirectory represents a **color theme**; a `global.xml` is placed in the directory, which mainly contains the `<Theme>` node and a large number of `<ThemeColor>` color definitions.
- Resource lookup order: first look in the current **color theme directory** -> then look in the **icon theme directory** -> finally fall back to `default/`, see [Theme Resource Lookup](#7-theme-resource-lookup) for details.

---

## 3. Quick Start

### 3.1 Using Colors in XML

After defining the theme colors, reference them in XML **by color name** through attributes such as `bkcolor`, `normal_text_color`, and `border_color`:

```xml
<Window>
    <VBox bkcolor="bg_window_main">
        <Label text="Hello" normal_text_color="text_default" font="system_bold"/>
        <Button class="btn_primary_global" text="Primary Button"/>
    </VBox>
</Window>
```

When switching themes, the referenced color name remains unchanged, and duilib automatically takes the actual ARGB value of the same-named color under the current theme.

### 3.2 Specifying the Theme at Startup

```cpp
ui::LocalFilesResParam resParam;
resParam.resourcePath = ui::FilePath(_T("resources/"));
resParam.themeRootPath = ui::FilePath(_T("themes"));
resParam.defaultThemePath = ui::FilePath(_T("default"));
resParam.colorThemePath = ui::FilePath(_T("color_dark"));   // Use the dark theme at startup
// resParam.colorThemePath = ui::FilePath(_T(""));          // Leave empty to follow the system

ui::GlobalManager::Instance().Startup(resParam);
```

### 3.3 Switching the Global Theme at Runtime

```cpp
// Switch to the dark theme
ui::GlobalManager::Instance().Theme().SwitchColorTheme(ui::FilePath(_T("color_dark")));

// Switch to the light theme
ui::GlobalManager::Instance().Theme().SwitchColorTheme(ui::FilePath(_T("color_light")));
```

After switching, the color table in the global `ColorManager` is replaced, and **all windows that have not cached a custom ColorManager are automatically redrawn**.

---

## 4. Theme XML Structure

The root node of `global.xml` (also called the "global resource description XML") is `<Global>`, and its commonly used child nodes are as follows:

### 4.1 `<Theme>` Theme Meta Information

```xml
<Theme name="Light" type="color" style="light" accent_support="true" version="2.0"/>
```

| Attribute | Required | Value | Description |
| :--- | :---: | :--- | :--- |
| `name` | ✅ | `Light` / `Dark` / `Default` , etc. | Theme display name |
| `type` | ✅ | `color` / `icon` / `combined` / `base` | Theme type (see [§5 Theme Type and Style](#5-theme-type-and-style)) |
| `style` | ✅ | `light` / `dark` / `combined` | Theme style (affects control dark mode determination) |
| `accent_support` | - | `true` / `false` | Whether the "accent color replacement" feature is supported (v2.0+) |
| `version` | - | `2.0` | XML format version number |

### 4.2 `<ThemeMeta>` Theme Metadata

```xml
<ThemeMeta>
    <Property name="author"           value="nim_duilib"/>
    <Property name="contrast_ratio"   value="7.2"/>             <!-- Contrast ratio -->
    <Property name="accent_color"     value="#FF0078D4"/>        <!-- Accent color anchor -->
    <Property name="base_luminance"   value="light"/>           <!-- light / dark -->
    <Property name="description"      value="Light theme ..."/>
</ThemeMeta>
```

Used to record extended information such as author, contrast ratio, and accent color, making it easier for theme editing tools to recognize.

### 4.3 `<ThemeColor>` Color Definition (Core)

```xml
<ThemeColor
    name="color_blue"             <!-- Required: color name (unique) -->
    value="#FF2966A3"             <!-- Required: ARGB color value -->
    type="common"                 <!-- Control type group -->
    category="basic_color"        <!-- Color usage category -->
    role="primary"                <!-- Role (related to accent color derivation) -->
    derived_from="color_gray"     <!-- Derivation source -->
    adjust="lightness:+15"        <!-- Derivation adjustment formula -->
    fixed="false"                 <!-- Whether to participate in accent color replacement -->
    comment_cn="Core accent color"         <!-- Chinese comment -->
    comment_en="Core accent color"/>  <!-- English comment -->
```

For detailed attribute descriptions, see [ThemeColor.md](./ThemeColor.en.md). The common rules are as follows:

- `name` is globally unique, named using `snake_case`, and it is recommended to follow the `{prefix}_{component}[_{state}]` pattern:
  - Purpose prefix: `bg_` (background), `border_` (border), `text_` (text/foreground), `color_` (base palette).
  - State suffix: `_normal` / `_hover` / `_pressed` / `_disabled` / `_focus` / `_selected`.
- `value` is an ARGB string: `#AARRGGBB`, with Alpha first.
- The values of `role` are described in [§5 Role and Derivation Chain](#6-accent-color-derivation-system-optional).
- `fixed="true"` indicates that this color does **not participate** in accent color replacement (semantic/neutral color).

### 4.4 Other Common Nodes

In addition to the theme-related nodes, `global.xml` can also contain:

- `<Font>` / `<FontFile>`: font definitions (see [Global.md](./Global.en.md)).
- `<Class>`: global Class style definition.
- `<Var>`: variable definition.

---

## 5. Theme Type and Style

Described through the two enumerations `ThemeType` and `ThemeStyle`:

### 5.1 Theme Type `ThemeType`

| Value | Meaning |
| :--- | :--- |
| `kUnknown` | Unknown |
| `kColor` | Color theme (colors only) |
| `kIcon` | Icon theme (icons/images only) |
| `kCombined` | Combined theme (color + icon) |
| `kBase` | Base theme (i.e., `default/`, shared resources only) |

The `type` field in XML corresponds to: `color` / `icon` / `combined` / `base`.

### 5.2 Theme Style `ThemeStyle`

| Value | Meaning |
| :--- | :--- |
| `kUnknown` | Unknown |
| `kBase` | Base style |
| `kLight` | Light style (affects `IsColorThemeDarkMode()`) |
| `kDark` | Dark style |

The `style` field in XML corresponds to: `light` / `dark` / `base` / `combined`.

When `style="dark"`, duilib recognizes the current theme as dark (affecting default behaviors such as shadows and SVG replacement colors), which can be queried via `ColorManager::IsColorThemeDarkMode()`.

---

## 6. Accent Color Derivation System (Optional)

> This is an optional feature of the v2.0 theme format. If you only need to simply switch "preset themes", you can skip this section.

Accent Color derivation allows **replacing the entire theme color set with a new primary color at runtime**. Its core concepts:

- **Anchor**: the color with `role="primary"`, which is the root of the derivation chain.
- **Derive**: `derived_from` points to the source color, and `adjust` describes the derivation rule.
- **Fixed**: colors with `fixed="true"` do not participate in derivation (used for semantic colors, neutral colors, etc.).

Example:

```xml
<!-- Anchor: primary blue -->
<ThemeColor name="color_blue" value="#FF0078D4" role="primary" fixed="false"/>

<!-- Derive: lightness +15 (for hover state) -->
<ThemeColor name="color_blue_light" value="#FF3D9EFF"
            role="primary_light" derived_from="color_blue" adjust="lightness:+15" fixed="false"/>

<!-- Derive: lightness -15 (for pressed state) -->
<ThemeColor name="color_blue_dark" value="#FF005A9E"
            role="primary_dark" derived_from="color_blue" adjust="lightness:-15" fixed="false"/>

<!-- Fixed color: semantic red (unchanged when replacing the primary color) -->
<ThemeColor name="color_red" value="#FFDC2626" role="semantic_error" fixed="true"/>
```

`adjust` syntax: `{attribute}:{±}{value}[,{attribute}:{±}{value}]*`, supporting `lightness` / `saturation` / `chroma` / `luminance` / `alpha`.

For detailed rules, see [ThemeColor.md](./ThemeColor.en.md#4-accent-color-derivation-system-design-accent-color-derivation).

---

## 7. Theme Resource Lookup

When loading resources (images, XML, Class), duilib looks them up in the following order:

```
1. Current color theme directory          (e.g., color_dark/)
2. Current icon theme directory           (e.g., icon_set_b/)
3. Default theme directory                (default/)
```

You can actively query the full resource path via `GlobalManager::GetExistsResFullPath()`. The supported resource packaging methods are:

- **Local files**: `LocalFilesResParam`, resources are stored on disk as folders.
- **Zip archive**: `ZipFileResParam`, resources are packaged as `resources.zip`.
- **Embedded in exe/dll**: `ResZipFileResParam` (Windows only), the zip is embedded as a PE resource.

---

## 8. C++ Code Interfaces

### 8.1 Startup Parameters `ResourceParam`

See [ResourceParam.h](../duilib/Core/ResourceParam.h):

| Field | Type | Description |
| :--- | :--- | :--- |
| `resourcePath` | FilePath | Resource root directory (absolute or relative path, depends on `resType`) |
| `themeRootPath` | FilePath | Theme root directory (default `themes`) |
| `defaultThemePath` | FilePath | Default theme path (default `default`) |
| `globalXmlFileName` | DString | Global XML file name (default `global.xml`) |
| `colorThemePath` | FilePath | Color theme used at startup; **empty means follow system** |
| `iconThemePath` | FilePath | Icon theme used at startup; can be empty |
| `languagePath` / `languageFileName` | - | Related to multi-language (see [Global.md](./Global.en.md)) |

### 8.2 Global Theme Management `ThemeManager`

Obtained via `GlobalManager::Instance().Theme()`:

| Method | Description |
| :--- | :--- |
| `InitTheme(themeRoot, defaultPath, globalXml)` | Initialize the theme manager (called internally by `Startup`) |
| `SwitchColorTheme(themePath)` | Switch the global color theme (takes effect immediately) |
| `SwitchIconTheme(themePath)` | Switch the global icon theme (takes effect immediately) |
| `GetAllThemes(list, list)` | Enumerate the list of available themes |
| `GetCurrentColorThemeInfo()` | Get the current color theme information (including `themePath`) |
| `GetCurrentIconThemeInfo()` | Get the current icon theme information |
| `GetCurrentThemeStyle()` | Get the current theme style (Light/Dark/...) |
| `GetDefaultThemeInfo()` | Get the default theme information |
| `IsSwitchingTheme()` | Whether a theme switch is in progress |
| `AddThemeChangeCallback(cb, id)` | Register a theme change callback (windows/controls can refresh on theme switch) |
| `GetSystemColorThemePath()` | Get the theme path that should currently be used in "follow system" mode |
| `SetLightColorPath(p)` / `SetDarkColorPath(p)` | Customize the light/dark theme paths when "following system" |

### 8.3 Single-Window Color Theme (without affecting the global)

If you only want **a certain window** to use another set of colors (without affecting the global), you can use the interface on `Window`:

```cpp
// Load the colors of the color_dark theme into this window
window->OpenColorTheme(ui::FilePath(_T("color_dark")));

// Close this window's independent theme and return to the global theme
window->CloseColorTheme();

// Or pass the XML text directly
window->OpenColorThemeData(xmlText);
```

> These methods **only affect the color lookup of this window**, not the global `ColorManager`, nor other windows.

### 8.4 Color Query `ColorManager`

```cpp
ui::ColorManager& cm = ui::GlobalManager::Instance().Color();
ui::ColorManager& wcm = window->GetColorManager();   // Preferentially returns this window's ColorManager

// Get color by name (ARGB)
UiColor c = cm.GetColor(_T("text_default"));

// Directly add/override a color
cm.AddColor(_T("my_color"), _T("#FFFF8800"));

// Determine whether the current theme is dark
bool isDark = cm.IsColorThemeDarkMode();
```

After XML parsing, the `<ThemeColor>` node is registered into the global color table via `ColorManager::AddColor`; when switching themes, the color table is cleared and reloaded.

### 8.5 Theme Change Notification

If business code needs to respond to theme changes, it can register a callback:

```cpp
ui::GlobalManager::Instance().Theme().AddThemeChangeCallback(
    [](const ui::ThemeInfo& info) {
        // Theme has been switched; you can refresh business-side caches, icons, etc. here
    },
    /* callbackId */ 1
);

// Unregister
ui::GlobalManager::Instance().Theme().RemoveThemeChangeCallback(callback, 1);
```

The window has a built-in `OnThemeChanged()` virtual function (`Window.h`) and the `kWindowThemeChangedMsg` event, which can also be overridden to respond to theme changes.

---

## 9. Follow System Light/Dark Mode

If you want the "application theme to automatically follow the operating system settings":

```cpp
// At startup: leave colorThemePath empty to follow the system
resParam.colorThemePath = ui::FilePath(_T(""));
ui::GlobalManager::Instance().Startup(resParam);

// After the system switches light/dark at runtime, duilib internally handles it as follows:
//  1. Call ThemeManager::GetSystemColorThemePath() to get the theme path to use
//  2. Automatically call SwitchColorTheme(...) to switch to the corresponding theme
```

`SetLightColorPath` / `SetDarkColorPath` can customize the specific theme paths corresponding to "system light/dark"; **these two functions must be called before `GlobalManager::Startup`** to take effect.

> Built-in default paths: `color_light` / `color_dark`, corresponding to the `DUILIB_LIGHT_COLOR_PATH` / `DUILIB_DARK_COLOR_PATH` macros respectively.

---

## 10. Common XML Templates

### 10.1 A Typical Light `global.xml`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Global>
    <Theme name="Light" type="color" style="light" accent_support="true" version="2.0"/>
    <ThemeMeta>
        <Property name="author" value="MyApp"/>
        <Property name="accent_color" value="#FF0078D4"/>
        <Property name="base_luminance" value="light"/>
    </ThemeMeta>

    <!-- Base palette -->
    <ThemeColor name="color_white"      value="#FFF2F2F2" type="common" category="basic_color" role="neutral"  fixed="false"/>
    <ThemeColor name="color_black"      value="#FF1F1F1F" type="common" category="basic_color" role="neutral"  fixed="false"/>
    <ThemeColor name="color_gray"       value="#FF808080" type="common" category="basic_color" role="neutral"  fixed="false"/>
    <ThemeColor name="color_blue"       value="#FF2966A3" type="common" category="basic_color" role="primary"  fixed="false"/>
    <ThemeColor name="color_blue_light" value="#FF5C99D6" type="common" category="basic_color" role="primary_light" derived_from="color_blue" adjust="lightness:+20" fixed="false"/>
    <ThemeColor name="color_red"        value="#FFA32929" type="common" category="basic_color" role="semantic_error" fixed="true"/>

    <!-- Window / Text -->
    <ThemeColor name="bg_window_main"   value="#FFF4F4F4" type="window" category="bg_color"     role="neutral"  fixed="false"/>
    <ThemeColor name="bg_titlebar"      value="#FFEAEAEA" type="window" category="bg_color"     role="neutral"  fixed="false"/>
    <ThemeColor name="border_window"    value="#FFD5D5D5" type="window" category="border_color" role="neutral"  fixed="false"/>
    <ThemeColor name="text_default"     value="#FF1A1A1A" type="text"   category="text_color"  role="neutral"  fixed="false"/>
    <ThemeColor name="text_link_normal" value="#FF1A1A1A" type="text"   category="text_color"  role="primary"  fixed="false"/>
</Global>
```

### 10.2 Referencing Theme Colors in a Window

```xml
<Window>
    <VBox bkcolor="bg_window_main">
        <HBox bkcolor="bg_titlebar" height="36" width="stretch">
            <Label text="Title" normal_text_color="text_default" font="system_bold"/>
        </HBox>
        <Button class="btn_global_blue_80x30_normal" text="OK" normal_text_color="text_link_normal"/>
    </VBox>
</Window>
```

The Class style for `btn_global_blue_80x30_normal` can be defined in `default/global.xml` or `color_xxx/global.xml`, and theme colors such as `bkcolor="color_blue"` can be used to achieve "color change with the theme".

### 10.3 Dynamically Switching Themes with `Event`

```xml
<Button class="btn_global_gray_80x30_normal" text="Switch to Dark">
    <Event type="click" receiver="#window#" apply_attribute="0" />
</Button>
```

Since theme switching requires calling a C++ interface (`SwitchColorTheme`), it is generally implemented by binding a custom event or the button's `AttachClick` via `Event`:

```cpp
btn->AttachClick([](const ui::EventArgs&) {
    ui::GlobalManager::Instance().Theme().SwitchColorTheme(ui::FilePath(_T("color_dark")));
    return true;
});
```

For a complete example, see [examples/ColorTheme](../examples/ColorTheme).

---

## 11. Custom Themes

To add a new theme, it is recommended to follow these steps:

1. **Copy an existing theme** as a template (e.g., `color_light`).
2. **Modify the `<Theme>` node**: adjust `name`, `style`, etc.
3. **Modify the `<ThemeColor>` color values**:
   - If you keep the derivation relationship, you can only change the "anchor + anchor light/dark", and other colors will follow automatically.
   - If you want full independence, you can directly change all `value`.
4. **Adjust SVG color replacement**: through mechanisms such as `svg_replace_colors="#B5B5B5|color_gray_light"`, allow SVG icons to display different colors under different themes (see [Global.md#Image](./Global.en.md#5-image-including-animated-images)).
5. **Place the theme directory under `themes/`**, and reference it by path at startup or runtime.

> The theme directory name is the `themePath` (e.g., `color_mybrand`), which corresponds one-to-one with `FilePath(_T("color_mybrand"))` in the code.

---

## 12. Theme Validation Tools

duilib comes with the following tools to assist theme debugging:

- **HTML Theme Editor**: [docs/Tools/ColorThemeMgr.html](./Tools/ColorThemeMgr.html), visually edit colors and preview the accent color replacement effect in real time.

---

## 13. Frequently Asked Questions

**Q1: What happens if a color name is written incorrectly?**

During parsing, invalid values are ignored; at runtime, if a non-existent color name is referenced, it falls back to a built-in default color such as `white`. It is recommended to enable Debug assertions to detect problems early.

**Q2: After switching themes, the window is not refreshed?**

- If the window loaded an independent ColorManager via `OpenColorTheme`, theme switching does not affect it, and `OpenColorTheme` needs to be called again.
- Self-drawn cached SVG/images may need `GlobalManager::ClearThemeCache()` to refresh immediately (see the `GlobalManager` implementation for details).
- Check whether a theme change callback (`AddThemeChangeCallback`) is registered and whether the correct redraw is performed.

**Q3: Can a new theme be added without restarting?**

Yes. Place the new theme directory under `themes/`, call `ThemeManager::GetAllThemes` at runtime to enumerate, then `SwitchColorTheme` to switch. The XML does not need to be recompiled.

**Q4: Following system light/dark mode does not take effect?**

- Ensure `colorThemePath` is left empty at `Startup`.
- If you customized the light/dark theme paths, confirm that `SetLightColorPath` / `SetDarkColorPath` are called **before** `Startup`.
- On Windows, the OS setting "Apps > Personalization > Color mode" must be enabled; on macOS, "System Settings > Appearance" must enable dark mode; on Linux, it depends on the desktop environment.

**Q5: How do the colors in SVG images follow the theme?**

Use `svg_replace_colors="#original_color|theme_color_name;#original_color2|theme_color_name2"` on image attributes such as `bkimage`, and it will be automatically replaced when the image is loaded. See the description of `svg_replace_colors` in [Global.md#Image](./Global.en.md#5-image-including-animated-images).

**Q6: How to make the button background color change with the theme?**

Use the theme color name in the `Class` style: `bkcolor="bg_btn_normal"`. After switching themes, the colors in the Class are automatically recalculated according to the new theme.

---

## 14. Related Documentation and Source Code

- Source code: [ThemeManager.h](../duilib/Core/ThemeManager.h) / [ThemeManager.cpp](../duilib/Core/ThemeManager.cpp)
- Color management: [ColorManager.h](../duilib/Core/ColorManager.h) / [ColorManager.cpp](../duilib/Core/ColorManager.cpp)
- Global management: [GlobalManager.h](../duilib/Core/GlobalManager.h) / [GlobalManager.cpp](../duilib/Core/GlobalManager.cpp)
- Resource parameters: [ResourceParam.h](../duilib/Core/ResourceParam.h)
- Theme format specification: [ThemeColor.md](./ThemeColor.en.md)
- Global resource node description: [Global.md](./Global.en.md)
- Theme generator: [ThemeGenerator.h](../duilib/Core/ThemeGenerator.h) / [ThemeGenerator.cpp](../duilib/Core/ThemeGenerator.cpp)
- Example project: [examples/ColorTheme](../examples/ColorTheme)
- Built-in themes: `color_light/`, `color_dark/`
- Theme editor (HTML): [docs/Tools/ColorThemeMgr.html](./Tools/ColorThemeMgr.html)
