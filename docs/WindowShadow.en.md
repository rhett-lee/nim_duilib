English | [简体中文](WindowShadow.md)

> Last synced: 2026-09-29

# Window Shadow Usage Guide

This document introduces the window shadow (Window Shadow) feature in duilib from a user's perspective, including XML configuration, the C++ code interface, available shadow types, and cross-platform compatibility notes, for developers' reference.

---

## 1. Feature Overview

duilib's window shadow gives windows a "floating in 3D" visual appearance. The implementation falls into two categories:

- **Self-drawn shadow**: duilib internally draws the shadow around the window using SVG shadow assets (nine-grid stretching) combined with rounded/square corners, applicable to all platforms with consistent results.
- **System shadow**: calls the window shadow provided by the operating system itself (Windows DWM, macOS window shadow, etc.), with the same appearance as other native system windows.

Shadows are enabled by configuring the `shadow_*` series of attributes on the `Window` tag, and can be dynamically switched at runtime via C++ code.

---

## 2. Quick Start

Simplest configuration (use the default self-drawn shadow, large rounded-corner shadow):

```xml
<Window size="900,600" caption="0,0,0,36" shadow_type="default">
    <!-- Window content -->
</Window>
```

If you want the Win11-style system shadow:

```xml
<Window size="900,600" caption="0,0,0,36" shadow_type="system_default">
    <!-- Window content -->
</Window>
```

If you want to turn off the shadow:

```xml
<Window size="900,600" caption="0,0,0,36" shadow_attached="false">
    <!-- Window content -->
</Window>
```

---

## 3. XML Configuration Attributes

All the following attributes are configured on the `<Window>` tag.

| Attribute | Default value | Type | Description |
| :--- | :--- | :--- | :--- |
| `shadow_attached` | `true` | bool | Whether to enable the shadow. `true` enables, `false` disables. If `shadow_type` is also set, `shadow_type` implicitly sets `shadow_attached` to `true`, so it is generally not necessary to set it separately. |
| `shadow_type` | - | string | Shadow type; see below for available values. |
| `shadow_image` | - | string | Custom shadow image, generally only used when `shadow_type="custom"`. For example `file='public/shadow/shadow_big.svg' corner='64,64,68,70'`. |
| `shadow_corner` | `"0,0,0,0"` | rect | Nine-grid description of the shadow image (left, top, right, bottom), generally only used in `custom` mode. |
| `shadow_border_round` | `"0,0"` | size | Rounded-corner size of the shadow (width, height), only used in `custom` mode. |
| `shadow_border_color` | - | string | Border color of the shadow (`#RRGGBB` or theme color name). Defaults to `#FF444444` in dark theme, `#FFB5B5B5` in light theme. |
| `shadow_border_size` | `2` | int | Pixel size of the shadow border. The actual displayed width is half of this value (e.g. `2` displays 1 pixel). |
| `shadow_snap` | `true` | bool | When the window is close to the screen edge, whether the corresponding side's shadow is automatically hidden. **Only effective for self-drawn shadows**. |

> Note: The code interfaces for all `shadow_*` attributes are in [Window.h](../duilib/Core/Window.h), and the method names correspond one-to-one with the attribute names (camelCase), such as `SetShadowType`, `SetShadowImage`, etc.

---

## 4. Shadow Types (`shadow_type`)

`shadow_type` is the core attribute that controls the shadow appearance. Its values are as follows:

### 4.1 Self-drawn shadows (built into duilib, consistent across platforms)

| Value | Description | Applicable scenario |
| :--- | :--- | :--- |
| `default` | Equivalent to the platform's default self-drawn shadow | General |
| `big` | Large shadow, square corners, with border | Ordinary windows |
| `big_round` | Large shadow, rounded corners, with border | Ordinary windows (recommended) |
| `small` | Small shadow, square corners, with border | Compact windows |
| `small_round` | Small shadow, rounded corners, with border | Compact windows |
| `menu` | Small shadow, square corners, with border | Popup windows, menus |
| `menu_round` | Small shadow, rounded corners, with border | Popup windows, menus (recommended) |
| `none` | No shadow, square corners, border only | Windows without shadow |
| `none_round` | No shadow, rounded corners, border only | Windows without shadow |
| `custom` | Custom self-drawn shadow, needs to be used together with `shadow_image`, etc. | Personalized shadow |

### 4.2 System shadows (using the OS native shadow)

| Value | Description |
| :--- | :--- |
| `system_default` | Follow the OS default shadow (Win11, macOS default rounded corners) |
| `system_not_round` | System shadow, square corners |
| `system_round` | System shadow, rounded corners |
| `system_small_round` | System shadow, small rounded corners |

### 4.3 Platform Compatibility

| Platform | Supported `system_*` |
| :--- | :--- |
| Windows 7 | Only `system_default` (the rest are switched automatically) |
| Windows 10 | Only `system_default` (the rest are switched automatically) |
| Windows 11 | All 4 system shadow types are supported |
| macOS | All 4 system shadow types are supported |
| Linux / FreeBSD | System shadows not supported; automatically switches to an equivalent self-drawn shadow |

> When an unsupported `system_*` type is set, duilib automatically selects an equivalent alternative. For example, setting `system_round` on Win10 will be automatically replaced with `system_default`; setting `system_round` on Linux will be replaced with `big_round`.

### 4.4 Platform Default Shadows

When `shadow_type` is not set, duilib selects the default value according to the following rules:

| Platform / Window condition | Default shadow type |
| :--- | :--- |
| Windows 10 / 11 | `system_default` |
| Windows 7 / 8 | `big_round` (self-drawn) |
| macOS | `system_default` |
| Linux / FreeBSD | `big_round` (self-drawn) |
| Layered window (`layered_window="true"`) | `big_round` (self-drawn) |

---

## 5. Custom Shadow (`shadow_type="custom"`)

If the built-in shadows cannot meet your needs, you can use your own shadow assets through the `custom` mode:

```xml
<Window shadow_type="custom"
        shadow_image="file='public/shadow/shadow_big.svg' window_shadow_mode='true' corner='64,64,68,70'"
        shadow_corner="30,30,34,36"
        shadow_border_round="6,6"
        shadow_border_color="border_window"
        shadow_border_size="1">
    <!-- Window content -->
</Window>
```

Attribute descriptions:

- `shadow_image`: Required, follows the image description string from [image attribute syntax](./XmlNode.en.md). `window_shadow_mode='true'` marks this image as shadow-specific, and `corner` is the image's own nine-grid.
- `shadow_corner`: Required, nine-grid description of the shadow asset (same meaning as the `corner` of `shadow_image`, and not DPI-scaled).
- `shadow_border_round`: Optional, shadow rounded-corner size (not DPI-scaled).
- `shadow_border_color` / `shadow_border_size`: Optional, border style.

> In `custom` mode, `shadow_type` will no longer override the above attributes; these attributes take the values you set.

---

## 6. C++ Code Interface

All `shadow_*` attributes have corresponding C++ interfaces, concentrated in the `Window` class and the `Shadow` class (see [Window.h](../duilib/Core/Window.h) and [Shadow.h](../duilib/Core/Shadow.h)).

### 6.1 Enable / Disable

```cpp
pWindow->SetShadowAttached(true);   // Enable shadow
pWindow->SetShadowAttached(false);  // Disable shadow
bool bAttached = pWindow->IsShadowAttached();
```

### 6.2 Switch Shadow Type

```cpp
pWindow->SetShadowType(ui::ShadowType::kShadowBigRound);
pWindow->SetShadowType(ui::ShadowType::kShadowSystemDefault);
```

### 6.3 Custom Shadow (at runtime)

```cpp
pWindow->SetShadowType(ui::ShadowType::kShadowCustom);
pWindow->SetShadowImage(_T("file='public/shadow/shadow_big.svg' window_shadow_mode='true' corner='64,64,68,70'"));
pWindow->SetShadowCorner(ui::UiPadding(30, 30, 34, 36));
pWindow->SetShadowBorderRound(ui::UiSize(6, 6));
```

### 6.4 Border, Edge-snapping

```cpp
pWindow->SetShadowBorderSize(2);
pWindow->SetShadowBorderColor(_T("border_window"));
pWindow->SetEnableShadowSnap(true);
```

### 6.5 Dynamic Switching Example

```xml
<!-- Place a dropdown/radio box inside the window to switch the shadow type at runtime -->
<Option group="shadow_type" selected="true" text="Large shadow (rounded)">
    <Event type="select" receiver="#window#" apply_attribute="shadow_type={big_round}" />
</Option>
<Option group="shadow_type" text="System shadow (rounded)">
    <Event type="select" receiver="#window#" apply_attribute="shadow_type={system_round}" />
</Option>
<Option group="shadow_type" text="No shadow">
    <Event type="select" receiver="#window#" apply_attribute="shadow_attached={false}" />
</Option>
```

For the complete example, see [render/page_window_shadow.xml](../../bin/resources/themes/default/render/page_window_shadow.xml).

---

## 7. Common Window XML Templates

### 7.1 Ordinary Main Window (recommended)

```xml
<Window size="900,600" min_size="320,240"
        caption="0,0,0,36" use_system_caption="false"
        sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_snap="true"
        size_box="4,4,4,4">
    <!-- Window content -->
</Window>
```

### 7.2 Popup Menu

```xml
<Window shadow_type="menu_round" shadow_border_size="1" shadow_border_color="border_window">
    <MenuListBox class="menu" padding="0,4,0,4">
        <!-- Menu items -->
    </MenuListBox>
</Window>
```

### 7.3 Tooltip (custom shadow)

```xml
<Window shadow_type="custom"
        shadow_image="file='shadow_tooltip_round.svg' window_shadow_mode='true' corner='16,16,18,18'"
        shadow_corner="12,12,14,14"
        shadow_border_round="4,4"
        shadow_border_size="0"
        shadow_snap="false">
    <VBox bkcolor="bg_tooltip">
        <!-- Content -->
    </VBox>
</Window>
```

---

## 8. Relationship with Other Window Attributes

- **`use_system_caption`**: If the system caption bar is enabled, when `shadow_attached="false"` is not explicitly set, duilib will automatically disable the self-drawn shadow to avoid double shadows; if you need the self-drawn shadow, set `shadow_attached="true"` at the same time and choose a self-drawn type.
- **`layered_window`**: When using the self-drawn shadow, there is no need to manually set `layered_window`; duilib will enable the layered window attribute as needed. System shadows and layered windows are mutually exclusive.
- **`size`**: By default, `size` only represents the size of the client area, excluding the shadow. Setting `size_contain_shadow="true"` lets `size` include the shadow dimensions.
- **`round_corner`**: When using the self-drawn shadow, the window naturally has rounded corners (determined by `shadow_*`), so there is generally no need to set `round_corner` again.
- **`render_backend_type="GL"`**: A window rendered with OpenGL cannot be a layered window, so it is incompatible with the self-drawn shadow; it is usually used together with the system shadow.

---

## 9. Frequently Asked Questions

**Q1: I set the shadow but don't see any effect?**

- Check whether the window is maximized: the self-drawn shadow is automatically hidden when maximized.
- Check whether `use_system_caption` is `true`: when the system caption bar is enabled, the self-drawn shadow will not be displayed; please explicitly set `shadow_attached="true"` and choose a self-drawn type.
- Check whether `shadow_attached` is set to `false`.

**Q2: I set `system_round` on Win7 / Win10, why doesn't it take effect?**

On Win7 and Win10 only `system_default` is available; the other 3 `system_*` types are switched automatically. Win11 is the one that has 4 system shadow types.

**Q3: Can the self-drawn shadow and the system shadow be enabled at the same time?**

No. After setting any `system_*` type, the self-drawn shadow is disabled; and vice versa.

**Q4: How to switch the shadow at runtime?**

Use the `Event` + `apply_attribute` mechanism (such as the example in section 6.5), or directly call `Window::SetShadowType`.

**Q5: What are the requirements for the custom shadow image?**

- SVG is recommended (duilib's renderer has complete SVG support).
- You need to correctly set the `corner` nine-grid to avoid distortion when stretching.
- The image content should have "opaque four corners, opaque middle" to correctly support window edge-snapping and rounded-corner clipping.
- `window_shadow_mode='true'` marks this image as shadow-specific and enables shadow-related rendering optimizations.

**Q6: How to keep the shadow color consistent with the theme?**

`shadow_border_color` supports directly filling in a theme color name (such as `border_window`), and duilib will automatically look it up from the theme color table and update it automatically as the theme switches (dark/light).

---

## 10. Related Documentation and Source Code

- Source code: [Shadow.h](../duilib/Core/Shadow.h) / [Shadow.cpp](../duilib/Core/Shadow.cpp)
- Window interface: [Window.h](../duilib/Core/Window.h) / [Window.cpp](../duilib/Core/Window.cpp)
- System shadow definition: [NativeWindowShadow.h](../duilib/Core/NativeWindowShadow.h)
- XML parsing: [WindowBuilder.cpp](../duilib/Core/WindowBuilder.cpp)
- Window attribute table: [Window.md](./Window.en.md)
- Shadow demo page: [page_window_shadow.xml](../../bin/resources/themes/default/render/page_window_shadow.xml)
- Self-drawn shadow assets: `public/shadow/` (`shadow_big.svg`, `shadow_big_round.svg`, `shadow_small.svg`, `shadow_small_round.svg`, `shadow_menu.svg`, `shadow_menu_round.svg`)
