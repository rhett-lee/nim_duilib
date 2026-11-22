English | [简体中文](TitleBar.md)

## Self-Drawn Title Bar Guidelines

When a window uses a self-drawn title bar (`use_system_caption="false"`, or it contains a
`window_title_bar`), follow the unified specification below for the title bar size, font,
icon, and window buttons. All sample windows in the repository follow this specification;
see the window XML files under `bin/resources/themes/default`.

For the meaning of title-bar-related window attributes (`caption`, `sys_menu`,
`sys_menu_rect`, etc.), see the window attributes document: [Window.en.md](Window.en.md).

## 1. Unified Specification

| Item | Where to configure | Unified value | Notes |
| :--- | :--- | :--- | :--- |
| Title bar height | `caption` of `Window` | `0,0,0,36` | Defines both the draggable area and the visual height; they must match |
| System menu area | `sys_menu_rect` of `Window` | `0,0,36,36` | Must match the title bar height; the feature is disabled if not set |
| Title bar container | `HBox name="window_title_bar"` | `width="stretch" height="36"` | Use `bkcolor="bg_titlebar"` for the background |
| Top-left icon | `Control class="title_bar_logo"` | Shared class in `global.xml` | Already contains 18×18, `margin="8,0,0,0"`, `valign="center"`, and the logo image |
| Title text | `Label class="title_bar_title"` | Shared class in `global.xml` | Already contains `font="system_bold_14"`, `margin="8,0,0,0"`, `valign="center"`; stretched titles override `margin="8,0,8,0"` |
| Normal window buttons | `Button` | `width="40" height="32"` `margin="0,2,0,2"` | 32 + 2 top + 2 bottom = 36, equal to the bar height |
| Maximize/Restore | `Box` + two `Button`s | `Box` is `40×32`, `margin="0,2,0,2"` | The two buttons are stacked; inner buttons use `width="stretch" height="stretch" margin="0"` |
| Close button | `Button` | `width="40" height="stretch"` `margin="0"` | Height equals the title bar height; flush with the top and right edges, no gaps |

## 2. Standard Structure

```xml
<Window size="900,600" caption="0,0,0,36"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" size_box="4,4,4,4" icon="public/caption/logo.ico">
    <VBox bkcolor="bg_window_main">
        <!-- Title bar -->
        <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bg_titlebar">
            <!-- Left: icon + title (using the shared classes in global.xml) -->
            <HBox margin="0,0,30,0" valign="center" width="auto" height="auto" mouse_enabled="false">
                <Control class="title_bar_logo"/>
                <Label class="title_bar_title" name="label_title" text_id="STRID_XXX_WINDOW_TITLE"/>
            </HBox>
            <!-- Middle spacer: takes the remaining width -->
            <Control mouse_enabled="false"/>
            <!-- Right: window buttons (add/remove as needed; do not change existing button types) -->
            <HBox margin="0,0,0,0" valign="center" width="auto" height="36">
                <Button class="btn_switch_theme" name="btn_window_theme"
                        height="32" width="40" margin="0,2,0,2"/>
                <Button class="btn_switch_lang" name="btn_window_language"
                        height="32" width="40" margin="0,2,0,2"/>
                <Button class="btn_wnd_min_11" name="btn_window_min"
                        height="32" width="40" margin="0,2,0,2"/>
                <!-- Maximize/Restore: stacked in the same cell; the framework toggles visibility -->
                <Box height="32" width="40" margin="0,2,0,2">
                    <Button class="btn_wnd_max_11" height="stretch" width="stretch"
                            name="btn_window_max" margin="0"/>
                    <Button class="btn_wnd_restore_11" height="stretch" width="stretch"
                            name="btn_window_restore" visible="false" margin="0"/>
                </Box>
                <!-- Close button: full height, flush right, no gaps -->
                <Button class="btn_wnd_close_11" height="stretch" width="40"
                        name="btn_window_close" margin="0"/>
            </HBox>
        </HBox>

        <!-- Content area -->
        <VBox bkcolor="bg_window_main" padding="10,8,10,10">
        </VBox>
    </VBox>
</Window>
```

## 3. Window Button Classes and Control Names

Button appearance is defined by classes in the theme (see
[global.xml](../bin/resources/themes/default/global.xml)). Control names are used by the
framework to bind window actions automatically; both must be used together.

| Button class | Recommended control name | Purpose | Size |
| :--- | :--- | :--- | :--- |
| `btn_wnd_min_11` | `btn_window_min` | Minimize | 40×32 |
| `btn_wnd_max_11` | `btn_window_max` | Maximize | Inside a 40×32 `Box`, stretched |
| `btn_wnd_restore_11` | `btn_window_restore` | Restore | Stacked with maximize, initially `visible="false"` |
| `btn_wnd_fullscreen_11` | `btn_window_fullscreen` | Full screen | 40×32 |
| `btn_wnd_close_11` | `btn_window_close` | Close | 40×`stretch` |
| `btn_switch_theme` | `btn_window_theme` | Switch theme (framework binds the menu) | 40×32 |
| `btn_switch_lang` | `btn_window_language` | Switch language (framework binds the menu) | 40×32 |
| `btn_wnd_settings_11` | Custom (e.g. `settings`) | Settings entry | 40×32 |
| `btn_view_fullscreen_11` | Custom | Page full screen (not window full screen) | 40×32 |

Notes:

- The control names `btn_window_min`, `btn_window_max`, `btn_window_restore`,
  `btn_window_close`, `btn_window_fullscreen`, `btn_window_theme`, and
  `btn_window_language` are recognized and bound by `WindowImplBase`
  (see [WinImplBase.cpp](../duilib/Utils/WinImplBase.cpp)).
- The `_11` suffix in class names is a style version suffix; the icon size is controlled
  by `SIZE_WND_ICON` (default 20).
- Keep the existing set of title bar buttons unchanged; only adjust appearance attributes
  such as size and margins according to this specification.

## 4. Key Rules

1. **`caption` must match the title bar height**
   The last value of `caption` is the distance from the top of the window to the draggable
   area, i.e. the title bar height. If they differ, you get either "buttons clickable but
   the window cannot be dragged" or extra blank space below the title bar.

2. **The close button must fill the height and sit flush to the edges**
   Use `height="stretch"` and `margin="0"` so that the close button is exactly as tall as
   the title bar and flush with the top and right edges. Do not use
   `margin="0,0,0,2"` or leave any height margin, otherwise gaps appear above or to the
   right of the close button.

3. **Normal buttons keep 2 px above and below**
   Title bar 36, button height 32, `margin="0,2,0,2"` — the three values align vertically.
   Do not change the height or margins of a single button, or the overall consistency breaks.

4. **Maximize/Restore use a stacked container**
   The outer `Box` uses a fixed 40×32 size and `margin="0,2,0,2"`; the two inner buttons
   use `width="stretch" height="stretch" margin="0"`. The framework toggles their
   `visible` state by window state, so both are never shown at the same time.

5. **Use the shared class for title text**
   The title `Label` uses `class="title_bar_title"` directly (defined in `global.xml`
   with `font="system_bold_14"`, `margin="8,0,0,0"`, and `valign="center"`).
   Stretched titles override `margin="8,0,8,0"`; do not repeat the font and alignment
   attributes in each XML.

6. **Percent-width titles must override the margin**
   If the title `Label` uses `width="100%"`, add `margin="0"`; otherwise the left margin
   from the class makes the title overflow and squeeze the right-side buttons. Use
   `text_padding` to indent the text instead.

7. **Title bars with tabs or other complex content**
   See `cef_browser.xml` / `webview2_browser.xml`: the outer bar may be named `title_bar`,
   and the window buttons go into an inner
   `HBox name="window_title_bar" width="auto" height="36"`. Both keep a height of 36.

8. **Floating title bars**
   Some windows (e.g. `chat/login.xml`) have no title bar background and only float buttons
   in the top-right corner. They still follow the same button size and margin rules, and
   the close button is still 40 wide with full height.

9. **Where the shared classes live**
   `title_bar_logo` and `title_bar_title` are defined in
   [global.xml](../bin/resources/themes/default/global.xml). If a window needs its own
   logo image (e.g. `chat/wechat.xml`), keep `class="title_bar_logo"` and override
   `bkimage`.

## 5. Common Issues

| Symptom | Cause | Fix |
| :--- | :--- | :--- |
| 2–4 px gap above/right of the close button | Button height is 32/34, or bottom/right margin is not 0 | Use `height="stretch" width="40" margin="0"` |
| Blank space below the title bar after shrinking it | Only `caption` was changed, not the container height | Set both to 36 |
| Too little space between title and right buttons | Title right margin is 0 and there is no spacer | Use `margin="8,0,8,0"`, or keep the middle `<Control/>` spacer |
| Title text hidden behind buttons | Percent/fixed title width is too large | Remove `margin` from percent-width titles, or use `width="stretch"` |
| Title font differs from other windows | `title_bar_title` is not used, or `font` is overridden | Use `class="title_bar_title"` |
| Maximize and restore buttons shown together | No stacked `Box`, or neither is hidden | Use a 40×32 stacked `Box` and set the restore button to `visible="false"` initially |

## 6. Checklist

- [ ] `caption="0,0,0,36"`, `sys_menu_rect="0,0,36,36"` (if the system menu is enabled)
- [ ] Title bar container `height="36"`, right-side button group `height="36"`
- [ ] Logo uses `class="title_bar_logo"` (override `bkimage` for custom images)
- [ ] Title uses `class="title_bar_title"`; stretched titles override `margin="8,0,8,0"`
- [ ] Normal buttons are 40×32 with `margin="0,2,0,2"`
- [ ] Maximize/Restore are inside a 40×32 `Box` and stretched inside it
- [ ] Close button uses `width="40" height="stretch" margin="0"`
- [ ] Titles neither wrap nor overlap buttons in both Chinese and English UI
- [ ] The types and number of existing title bar buttons are unchanged

## 7. Reference Files

| File | Description |
| :--- | :--- |
| [Window.en.md](Window.en.md) | Window attributes (`caption`, `use_system_caption`, `sys_menu_rect`, etc.) |
| [global.xml](../bin/resources/themes/default/global.xml) | Window button classes and icon size definitions |
| [label.xml](../bin/resources/themes/default/controls/label.xml) | Minimal title bar example |
| [controls.xml](../bin/resources/themes/default/controls/controls.xml) | Title bar with a full set of buttons |
| [cef_browser.xml](../bin/resources/themes/default/cef_browser/cef_browser.xml) | Title bar with tabs and window buttons |
| [login.xml](../bin/resources/themes/default/chat/login.xml) | Floating title bar example |
