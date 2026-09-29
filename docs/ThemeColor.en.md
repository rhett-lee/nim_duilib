English | [简体中文](ThemeColor.md)

> Last synced: 2026-09-29

# nim_duilib Theme Color Configuration XML Specification (v2.0)

## 1. File Structure Overview

```xml
<Global>
    <!-- Theme metadata: defines the basic attributes of the theme -->
    <Theme 
        name="Light"            <!-- Theme name -->
        type="Color"            <!-- Theme type, fixed as Color -->
        style="Light"           <!-- Theme style -->
        accent_support="true"   <!-- Whether accent color replacement is supported -->
        version="2.0"           <!-- Format version number -->
    />
    
    <!-- Theme metadata extension (optional): stores additional theme configuration information -->
    <ThemeMeta>
        <Property name="author" value="nim_duilib"/>
        <Property name="contrast_ratio" value="7.2"/>   <!-- Contrast ratio, used for accessibility checking -->
        <Property name="accent_color" value="#FF0078D4"/> <!-- Default accent color of the theme -->
        <Property name="base_luminance" value="light"/>    <!-- Base luminance: light/dark -->
    </ThemeMeta>
    
    <!-- Color definition area: all color configurations are here -->
    <ThemeColor 
        name="color_blue"        <!-- Unique color identifier, globally unique; referenced by this name in the program -->
        value="#FF0078D4"        <!-- Actual color value, format is #AARRGGBB, with Alpha channel first -->
        type="common"            <!-- Control type group, divided by functional module, for component-based management -->
        category="basic_color"   <!-- Color usage category, divided by visual role, for batch filtering by function -->
        role="primary"           <!-- Color role: used for accent color derivation -->
        derived_from=""          <!-- Derivation source: which color it is derived from -->
        adjust=""                <!-- Adjustment formula: color adjustment rules during derivation -->
        fixed="false"            <!-- Whether fixed: whether it remains unchanged during accent color replacement -->
        comment_cn="Core accent color"         <!-- Chinese comment -->
        comment_en="Core accent color"  <!-- English comment -->
    />
</Global>
```

---

## 2. Naming Convention

### 2.1 Global Naming Rules

**All attribute names uniformly use snake_case**, i.e., all lowercase letters + underscore separator.

| Rule | Example | Counter-example |
|-----|------|------|
| All lowercase letters | `derived_from`, `accent_support` | `derivedFrom`, `accentSupport` |
| Underscore-separated words | `contrast_ratio`, `base_luminance` | `contrastRatio`, `baseLuminance` |
| Concise and clear | `fixed`, `adjust`, `role` | `isFixed`, `adjustFormula`, `colorRole` |

### 2.2 Color Name Naming Rules

**Unified format**: `{prefix}_{component}[_{subcomponent}][_{state}]`

| Part | Rule | Description | Example |
|-----|------|------|------|
| **Purpose prefix** | `bg_` | Background color (Background) | `bg_window`, `bg_btn_normal` |
| | `border_` | Border color (Border) | `border_btn_normal` |
| | `text_` | Text/foreground color (Text/Foreground) | `text_default`, `text_btn_disabled` |
| | `color_` | Base color / palette color | `color_blue`, `color_gray` |
| **State suffix** | `_normal` | Default / normal | `border_btn_normal` |
| | `_hover` | Mouse hover state | `bg_btn_hover` |
| | `_pressed` | Mouse click / pressed state | `bg_btn_pressed` |
| | `_disabled` | Disabled state | `text_btn_disabled` |
| | `_focus` | Keyboard focus state | `border_control_focus` |
| | `_selected` | Selected state | `bg_tab_ctrl_item_selected` |

### 2.3 Naming Consistency Verification (100% consistent)

| Check item | Status | Verification result | Description |
|--------|------|---------|------|
| Purpose prefix | ✅ 100% consistent | 4 prefixes: `bg_`(62) / `border_`(24) / `color_`(19) / `text_`(14) | Total 119 color definitions |
| State word position | ✅ 100% consistent | All state words are at the end of the name | e.g., `_hover`, `_pressed`, `_disabled`, etc. |
| Naming pattern | ✅ 100% consistent | `{prefix}_{component}[_{subcomponent}]_{state}` | Can be precisely searched via patterns such as `*_hover`, `*btn*`, etc. |
| Case | ✅ 100% consistent | All lowercase + underscore separator | snake_case |
| Attribute naming | ✅ 100% consistent | All attributes use snake_case | `fixed`, `derived_from`, `accent_support` |

### 2.4 Component Classification Statistics

| Component type | Number of colors | Included colors |
|---------|---------|---------|
| **Basic colors** | 19 | `color_*` |
| **Button (btn)** | 16 | Normal buttons + primary buttons + window buttons |
| **Text (text)** | 14 | General text + links + button text |
| **Border (border)** | 24 | Borders of various controls |
| **Background (bg)** | 62 | Backgrounds of various controls |
| **Menu (menu)** | 7 | Menu bar + menu items |
| **List (list)** | 13 | ListCtrl control |
| **Tab (tab)** | 7 | TabCtrl control |
| **Rich edit (richedit)** | 11 | RichEdit control |
| **Combo box (combo)** | 5 | Combo control |
| **Property grid (property)** | 10 | PropertyGrid control |
| **Other** | 42 | Windows, title bars, split bars, etc. |

---

## 3. Complete Attribute Definitions (Attributes Reference)

### 3.1 Theme Node Attributes

| Attribute name | Naming style | Type | Required | Description | Example value |
|--------|---------|------|------|------|--------|
| `name` | snake_case | string | ✅ | Theme name | `Light`, `Dark` |
| `type` | snake_case | string | ✅ | Theme type | Fixed as `Color` |
| `style` | snake_case | string | ✅ | Theme style | `Light`, `Dark`, `HighContrastBlack` |
| `version` | snake_case | string | ✅ | Format version number | `2.0` |
| `accent_support` | snake_case | boolean | - | Whether accent color replacement is supported | `true` / `false` |

### 3.2 ThemeMeta Node Attributes

| Attribute name (name) | Naming style | Type | Description | Example value |
|---------------|---------|------|------|--------|
| `author` | snake_case | string | Theme author / source | `nim_duilib` |
| `contrast_ratio` | snake_case | float | **Contrast ratio**: the theme's default WCAG contrast value, used for accessibility checking | `7.2`, `4.5` |
| `accent_color` | snake_case | ARGB | **Default accent color**: the theme's factory default primary color anchor | `#FF0078D4` |
| `base_luminance` | snake_case | enum | **Base luminance**: the overall light/dark tone of the theme | `light`, `dark` |

### 3.3 ThemeColor Node Attributes (Core)

| Attribute name | Naming style | Type | Required | Description | Example value |
|--------|---------|------|------|------|--------|
| `name` | snake_case | string | ✅ | **Unique color identifier**, globally unique; referenced by this name in the program | `color_blue`, `bg_btn_normal` |
| `value` | - | ARGB | ✅ | **Actual color value**, format is `#AARRGGBB`, with Alpha channel first | `#FF0078D4` |
| `type` | snake_case | string | ✅ | **Control type group**, divided by functional module, for component-based management | `common`, `button`, `menu`, `window` |
| `category` | snake_case | string | ✅ | **Color usage category**, divided by visual role, for batch filtering by function | `basic_color`, `bg_color`, `text_color`, `border_color` |
| `role` | snake_case | enum | ⭐ | **Color role**, its position in the accent color derivation system | `primary`, `neutral` |
| `derived_from` | snake_case | string | ⭐ | **Derivation source**, references the `name` of another color, indicating this color is derived from that color | `color_blue` |
| `adjust` | formula | string | ⭐ | **Derivation adjustment formula**, the color transformation rule during derivation | `lightness:+15` |
| `fixed` | snake_case | boolean | ⭐ | **Whether fixed**:<br>`true` = **remains unchanged** during accent color replacement<br>`false` = **needs recalculation** during accent color replacement | `true` / `false` |
| `comment_cn` | - | string | - | **Chinese comment description** | "Core accent color" |
| `comment_en` | - | string | - | **English comment description** | "Core accent color" |

> **Important note**: `fixed` is the only criterion - only colors with `fixed="false"` participate in the calculation of the accent color derivation chain.

### 3.4 Description of `type` / `category` Attribute Values

#### `type` attribute (grouped by control type)

| Value | Description |
|------|------|
| `common` | Common color, a base color not belonging to a specific control |
| `text` | Text color |
| `window` | Window-related colors |
| `button` | Button color |
| `menu` | Menu color |
| `menu_bar` | Menu bar color |
| `list_ctrl` | List control color |
| `combo` | Combo box color |
| `tab_ctrl` | Tab control color |
| `richedit` | Rich edit box color |
| `property_grid` | Property grid color |
| `address_bar` | Address bar color |
| `scroll_bar` | Scroll bar color |
| ... | Identifiers for other control types |

#### `category` attribute (classified by usage)

| Value | Description |
|------|------|
| `basic_color` | Basic colors: palette colors such as `color_blue`, `color_red`, `color_gray`, etc. |
| `bg_color` | Background colors: the background color of all controls/windows |
| `text_color` | Text colors: all foreground text colors |
| `border_color` | Border colors: all control borders, split lines |

### 3.5 Optional Values and Semantics of the `role` Attribute

| role value | Semantic description | Accent color replacement behavior |
|---------|---------|-------------|
| `primary` | **Primary color anchor** | ✅ Completely replaced by the new accent color (root node of the derivation chain) |
| `primary_light` | **Primary color light variant** | ✅ Recalculated from the new primary color based on the derivation relationship |
| `primary_dark` | **Primary color dark variant** | ✅ Recalculated from the new primary color based on the derivation relationship |
| `semantic_success` | **Success semantic color** | ❌ Remains unchanged (semantically fixed: green family) |
| `semantic_success_light` | **Success semantic color (light)** | ❌ Remains unchanged |
| `semantic_error` | **Error semantic color** | ❌ Remains unchanged (semantically fixed: red family) |
| `semantic_error_light` | **Error semantic color (light)** | ❌ Remains unchanged |
| `semantic_error_dark` | **Error semantic color (dark)** | ❌ Remains unchanged |
| `semantic_warning` | **Warning semantic color** | ❌ Remains unchanged (semantically fixed: yellow family) |
| `neutral` | **Neutral gray / fixed color** | ❌ Remains unchanged (gray, white, black, static background) |

### 3.6 `adjust` Color Adjustment Formula Syntax

```ebnf
adjust_formula:  {property}:{sign}{value} [, {property}:{sign}{value}]*

property:      lightness | saturation | chroma | luminance | alpha
sign:      + | -
value:      integer, indicating the adjustment magnitude (range 0-100)
```

Supported color adjustment attributes:

| Attribute | Color space | Description | Example |
|-----|---------|------|------|
| `lightness` | HSL | Adjusts **lightness**; larger value means brighter | `lightness:+15` = lightness increased by 15 |
| `saturation` | HSL | Adjusts **saturation**; larger value means more vivid | `saturation:-10` = saturation decreased by 10 |
| `chroma` | HCL | Adjusts **chroma** (perceptually uniform saturation) | `chroma:+20` = chroma increased by 20 |
| `luminance` | CIE | Adjusts **perceptual lightness** (uniform to human perception) | `luminance:-5` = perceptual lightness decreased by 5 |
| `alpha` | ARGB | Adjusts **transparency** (0-255) | `alpha:128` = set to semi-transparent |

**Formula examples**:
```xml
<!-- Single attribute adjustment -->
adjust="lightness:+15"

<!-- Multiple attribute combination adjustment -->
adjust="lightness:-12,saturation:+5"

<!-- Typical adjustment for a dark-theme button hover state -->
adjust="lightness:+20,saturation:-15"
```

---

## 4. Accent Color Derivation System Design (Accent Color Derivation)

### 4.1 Derivation Chain Execution Rules

```
When the user sets a new accent color new_accent = #RGB:

1. Traverse all ThemeColors, find the anchor color with role="primary"
   → set its value to new_accent

2. Traverse all colors with derived_from="anchor name"
   → calculate the new color value according to the adjust formula

3. Recursively process all secondary derived colors derived from these colors

4. Colors with fixed="true" do not participate in derivation at all
```

### 4.2 Complete Derivation Chain Example

```xml
<!-- ==============================================
     Level 1: Primary Anchor (Primary Anchor)
     ============================================== -->
<ThemeColor 
    name="color_blue" 
    value="#FF0078D4" 
    role="primary"
    fixed="false"
    comment_cn="Core accent color anchor - directly replaced by user-defined color"
    comment_en="Primary color anchor - directly replaced by user color"
/>

<!-- ==============================================
     Level 2: Primary Variants (Primary Variants)
     ============================================== -->
<ThemeColor 
    name="color_blue_light" 
    value="#FF3D9EFF" 
    role="primary_light"
    derived_from="color_blue"
    adjust="lightness:+15"
    fixed="false"
    comment_cn="Light primary variant - for hover state"
    comment_en="Light primary variant - for hover states"
/>
<ThemeColor 
    name="color_blue_deep" 
    value="#FF005A9E" 
    role="primary_dark"
    derived_from="color_blue"
    adjust="lightness:-15"
    fixed="false"
    comment_cn="Dark primary variant - for pressed state"
    comment_en="Dark primary variant - for pressed states"
/>

<!-- ==============================================
     Level 3: UI Component References (UI Component References)
     ============================================== -->
<ThemeColor 
    name="bg_primary_btn_normal" 
    value="#FF0078D4" 
    derived_from="color_blue"
    fixed="false"
    comment_cn="Primary button normal state background"
    comment_en="Primary button normal state background"
/>
<ThemeColor 
    name="bg_primary_btn_hover" 
    value="#FF3D9EFF" 
    derived_from="color_blue_light"
    fixed="false"
    comment_cn="Primary button hover state background"
    comment_en="Primary button hover state background"
/>
<ThemeColor 
    name="text_link_normal" 
    value="#FF0078D4" 
    derived_from="color_blue"
    fixed="false"
    comment_cn="Hyperlink default color"
    comment_en="Hyperlink normal state color"
/>

<!-- ==============================================
     Fixed Colors: Do Not Participate in Derivation (Fixed Colors)
     ============================================== -->
<ThemeColor 
    name="color_red" 
    value="#FFDC2626" 
    role="semantic_error"
    fixed="true"
    comment_cn="Error prompt color - remains unchanged during accent replacement"
    comment_en="Error semantic color - fixed during accent replacement"
/>
<ThemeColor 
    name="bg_window" 
    value="#FFFFFFFF" 
    role="neutral"
    fixed="true"
    comment_cn="Window background - fixed"
    comment_en="Window background - fixed"
/>
```

### 4.3 Derivation Algorithm Pseudocode

```cpp
// Schematic implementation of the C++ version derivation algorithm
void applyCustomAccentColor(Color32 newAccentColor) {
    // 1. Update the primary anchor
    auto* primary = findColorByRole("primary");
    primary->value = newAccentColor;
    
    // 2. Topological sort: process in derivation dependency order
    auto sortedColors = topologicalSortByDerivedFrom();
    
    // 3. Recalculate colors in order
    for (auto* color : sortedColors) {
        if (color->fixed) continue;
        
        if (!color->derived_from.empty()) {
            auto* source = findColorByName(color->derived_from);
            color->value = applyAdjustFormula(source->value, color->adjust);
        }
    }
    
    // 4. For all colors with fixed="false" but no derived_from set,
    //    automatically derive the derivation relationship based on state (_hover/_pressed)
    applyImplicitDerivations();
}
```

---

## 5. Theme Type Definitions

| `style` attribute value | Theme type | `accent_support` | Description |
|---------------|---------|------------------|------|
| `Light` | Light theme | `true` | Default light theme (Win11 style) |
| `Dark` | Dark theme | `true` | Dark theme |
| `HighContrastBlack` | High contrast black | `false` | Accessibility theme, fixed color scheme |
| `HighContrastWhite` | High contrast white | `false` | Accessibility theme, fixed color scheme |
| `Custom` | Custom theme | `true` | Fully user-customized theme |

---

## 6. Loading and Using Themes

### 6.1 Theme File Storage Location

Theme files (`*.xml`) should be placed in the `themes/<theme_name>/` subdirectory of the resource root directory, for example:

```
bin/resources/themes/
├── default/
│   ├── global.xml             # Global resources (fonts, color definitions)
│   ├── light.xml              # Light theme (Theme node + ThemeColor node collection)
│   ├── dark.xml               # Dark theme
│   ├── public/                # Shared image resources
│   └── <your_app>/            # Application-specific XML and images
```

### 6.2 Loading Process

1. The application calls `ui::GlobalManager::Instance().Startup(...)` to start the global resource manager;
2. The framework loads the corresponding theme according to the `Theme` node attributes in the theme file;
3. Controls reference colors via `color="<ThemeColor.name>"` or `bkcolor="<ThemeColor.name>"`.

### 6.3 Theme Switching

Themes can be switched at runtime via `ui::ThemeManager`. After switching, all derived colors that do not have `fixed="true"` are recalculated according to the accent color derivation chain.

### 6.4 Theme Extension Methods

To add a new custom theme:
1. Copy an existing theme file (e.g., `light.xml`) as a basis;
2. Modify the `name`, `style`, etc. attributes of the `Theme` node;
3. Adjust the `value` of `ThemeColor`, or adjust the `derived_from`/`adjust` derivation relationship;
4. In `global.xml`, point the new theme's `Class` style to the new theme file.

---

## 7. Existing Color Coverage Analysis and Suggestions

### 7.1 Covered Color Categories ✅

| Category | Covered color examples | Count | Completeness |
|-----|--------------|------|-------|
| **Basic colors** | color_white, color_black, color_gray* | 19 | ✅ Complete |
| **Window system** | bg_window_dark, bg_window_light, bg_titlebar, border_window | 4 | ✅ Complete |
| **Text system** | text_default, text_disabled, text_link_*, text_menu_bar | 14 | ✅ Complete |
| **Button controls** | bg_btn_*, border_btn_*, text_btn_* (normal + primary buttons) | 16 | ✅ Complete |
| **Menu system** | bg_menu, bg_menu_item_*, bg_menu_bar_* | 7 | ✅ Complete |
| **Tabs** | bg_tab_ctrl, bg_tab_ctrl_item_* | 7 | ✅ Complete |
| **List controls** | bg_list_ctrl_*, bg_list_item_* | 13 | ✅ Complete |
| **Combo box** | bg_combo, bg_combo_btn_*, border_combo_* | 5 | ✅ Complete |
| **Rich edit box** | bg_richedit, border_richedit_*, bg_richedit_btn_* | 11 | ✅ Complete |
| **Property grid** | bg_property_grid_*, text_property_grid_* | 10 | ✅ Complete |
| **Address bar** | bg_address_bar, bg_address_bar_btn_* | 3 | ✅ Complete |
| **Semantic colors** | color_red*, color_green* (error/success states) | 5 | ✅ Complete |
| **Split lines** | color_splitline_* | 3 | ✅ Complete |
| **Progress bar** | color_progress | 1 | ✅ Present |

### 7.2 Suggested Additional Color Definitions ⭐

The following colors are **missing** from the existing 119 definitions; it is suggested to add them to enhance the completeness and flexibility of the theme:

| Missing color name | Suggested value (light/dark) | Usage description | Importance |
|-------------|-------------------|---------|-------|
| `bg_container` | `#FFF8F8F8` / `#FF202020` | **General container background** - default background for dialogs, panels, group boxes | 🔴 High |
| `bg_content` | `#FFFFFFFF` / `#FF1A1A1A` | **Content area background** - base background for scrollable content areas | 🔴 High |
| `bg_header` | `#FFF0F0F0` / `#FF2D2D2D` | **Header background** - header background for lists/trees/tables (currently reuses bg_titlebar) | 🟡 Medium |
| `bg_overlay` | `#80000000` / `#80000000` | **Overlay background** - black semi-transparent overlay for popups/drawers (Alpha=128) | 🟡 Medium |
| `bg_tooltip` | `#FFFFFFE6` / `#FF2B2B2B` | **Tooltip background** - Tooltip control background | 🟡 Medium |
| `bg_selected_inactive` | `#FFE8E8E8` / `#FF3D3D3D` | **Inactive selected state background** - background of selected items when focus is lost | 🟡 Medium |
| `border_focus_ring` | `#4D0078D4` / `#4D3B82F6` | **Focus ring / glow** - high-contrast focus indicator (semi-transparent) | 🟡 Medium |
| `shadow_control` | ~~N/A~~ | Control shadow (not a color value, requires DPI awareness) | ⚪ Low |
| `text_selected` | `#FFFFFFFF` / `#FFFFFFFF` | **Selected text color** - foreground color when text is selected | 🟡 Medium |
| `bg_text_highlight` | `#FFFFFF00` / `#FFE5A000` | **Text highlight background** - highlight marker for search/find results | ⚪ Low |
| `bg_scrollbar` | `#FFE0E0E0` / `#FF434343` | **Scrollbar background** | 🟡 Medium |
| `bg_scrollbar_thumb` | `#FFB0B0B0` / `#FF6A6A6A` | **Scrollbar thumb background** | 🟡 Medium |
| `bg_scrollbar_thumb_hover` | `#FF808080` / `#FF858585` | **Scrollbar thumb hover state** | 🟡 Medium |

**Total colors after supplement**: 119 + 13 = **132**

### 7.3 Priority Description for Missing Color Definitions

| Priority | Colors | Reason |
|-------|------|------|
| 🔴 High | bg_container, bg_content | Base background for most controls; currently relies on bg_window_dark/light but with unclear semantics |
| 🟡 Medium | bg_header, bg_tooltip, scrollbar* | Improve theme completeness; some controls currently reuse similar colors |
| ⚪ Low | shadow, highlight | Used in specific scenarios; partially rendered by the system |

---

## 8. Version Compatibility Guarantee

| Compatibility level | Commitment | Guaranteed version |
|-----------|---------|---------|
| **Format forward compatibility** | The v2.0 XML format can be correctly read by v1.x version libraries | Permanent |
| | New attributes are safely ignored by older version parsers | - |
| **Name backward compatibility** | All `name`s existing in v1.x continue to exist in v2.0 | At least 3 major versions |
| | New semantic names use the "alias mode", and original names are not deleted | - |
| **Value compatibility** | All color `value`s of the default theme are **exactly the same** as version v1.7 | v2.0.x |
| **Runtime compatibility** | Default behavior is exactly the same as version v1.7 | v2.0.x |
| | The accent color derivation feature needs to be **explicitly enabled** | - |
| **API compatibility** | The signatures of color-retrieving APIs such as `GetColor(LPCTSTR name)` remain unchanged | Permanent |

---

## 9. Validation and Tools

### 9.1 Specification Compliance Checklist

| Validation item | XPath expression | Expected result |
|-------|-------------|---------|
| All colors have a unique name | `count(//ThemeColor[@name])` | = count(//ThemeColor) |
| | `count(//ThemeColor[preceding::ThemeColor/@name = @name])` | = 0 |
| Naming format is correct | `//ThemeColor[not(matches(@name, '^[a-z]+_[a-z0-9_]+$'))]` | Empty node set |
| All derivation sources exist | `//ThemeColor[@derived_from and not(//ThemeColor/@name = @derived_from)]` | Empty node set |
| adjust format is correct | `//ThemeColor[@adjust and not(matches(@adjust, '^[a-z]+:[+-]?[0-9]+(,[a-z]+:[+-]?[0-9]+)*$'))]` | Empty node set |
| fixed is a boolean | `//ThemeColor[@fixed[not(. = 'true' or . = 'false')]]` | Empty node set |
| role is within the enumeration range | `//ThemeColor[@role[not(. = ('primary', 'primary_light', 'primary_dark', 'neutral', 'semantic_success', 'semantic_error', 'semantic_warning'))]]` | Empty node set |
| Value is in ARGB format | `//ThemeColor[not(matches(@value, '^#[0-9A-Fa-f]{8}$'))]` | Empty node set |
| No leftover old attributes | `//ThemeColor[@support_accent]` | Empty node set |
| Attribute naming is consistent | `//ThemeColor[@*[starts-with(name(), 'd') and not(starts-with(name(), 'derived_from'))]]` | Empty node set |

### 9.2 Recommended Validation Tools

| Tool | Purpose |
|-----|-----|
| `validate_theme.py` | Python script: complete XML validation + derivation chain calculation |
| Theme editor HTML | Visualization tool: preview accent color replacement effect + real-time editing |
| `xmllint` / `XML Validator` | General XML syntax validation tool |
| Visual Studio | Enable IntelliSense during editing with XSD Schema |
