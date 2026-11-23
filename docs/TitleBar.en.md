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
| Title bar height | `caption` of `Window` | `0,0,0,${SIZE_TITLEBAR_HEIGHT}` | Defines both the draggable area and the visual height; they must match |
| System menu area | `sys_menu_rect` of `Window` | `0,0,${SIZE_TITLEBAR_HEIGHT},${SIZE_TITLEBAR_HEIGHT}` | Must match the title bar height; the feature is disabled if not set |
| Title bar container | `HBox name="window_title_bar"` | `width="stretch"`, `height="${SIZE_TITLEBAR_HEIGHT}"` | Use `bkcolor="bg_titlebar"` for the background |
| Top-left icon | `Control class="title_bar_logo"` | Shared class in `global.xml` | Already contains `${SIZE_TITLE_LOGO}` (18), `margin="8,0,0,0"`, `valign="center"`, and the logo image |
| Title text | `Label class="title_bar_title"` | Shared class in `global.xml` | Already contains `font="system_regular_14"`, `margin="8,0,0,0"`, `valign="center"`; stretched titles override `margin="8,0,8,0"` |
| Normal window buttons | `Button` referencing a window button class | Size defined by the class in `global.xml` | `btn_wnd_base` provides `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}` (40×32); the window XML only writes `margin="0,2,0,2"` |
| Maximize/Restore | `Box` + two `Button`s | The outer `Box` sets the size; the classes provide `stretch` | `Box` is `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}` with `margin="0,2,0,2"`; stretching and `margin="0"` come from the classes |
| Close button | `Button` referencing a window button class | `btn_wnd_close_11` sets `height="stretch"` | Width comes from the base class and the height fills the bar; the window XML only writes `margin="0"` |

The `${SIZE_*}` variables are defined in the "size and path variables" section of
[global.xml](../bin/resources/themes/default/global.xml). Attributes of the `Window` node
(`caption`, `sys_menu_rect`, etc.) also expand variables, so write
`caption="0,0,0,${SIZE_TITLEBAR_HEIGHT}"` and changing `SIZE_TITLEBAR_HEIGHT` updates all
windows at once.

## 2. Standard Structure

```xml
<Window size="900,600" caption="0,0,0,36"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" size_box="4,4,4,4" icon="public/caption/logo.ico">
    <VBox bkcolor="bg_window_main">
        <!-- Title bar -->
        <HBox name="window_title_bar" width="stretch" height="${SIZE_TITLEBAR_HEIGHT}" bkcolor="bg_titlebar">
            <!-- Left: icon + title (using the shared classes in global.xml) -->
            <HBox margin="0,0,30,0" valign="center" width="auto" height="auto" mouse_enabled="false">
                <Control class="title_bar_logo"/>
                <Label class="title_bar_title" name="label_title" text_id="STRID_XXX_WINDOW_TITLE"/>
            </HBox>
            <!-- Middle spacer: takes the remaining width -->
            <Control mouse_enabled="false"/>
            <!-- Right: window buttons (add/remove as needed; do not change existing button types) -->
            <HBox margin="0,0,0,0" valign="center" width="auto" height="${SIZE_TITLEBAR_HEIGHT}">
                <Button class="btn_switch_theme" name="btn_window_theme" margin="0,2,0,2"/>
                <Button class="btn_switch_lang" name="btn_window_language" margin="0,2,0,2"/>
                <Button class="btn_wnd_min_11" name="btn_window_min" margin="0,2,0,2"/>
                <!-- Maximize/Restore: stacked in the same cell; the framework toggles visibility -->
                <Box height="${SIZE_WND_BTN_HEIGHT}" width="${SIZE_WND_BTN_WIDTH}" margin="0,2,0,2">
                    <Button class="btn_wnd_max_11" name="btn_window_max" margin="0"/>
                    <Button class="btn_wnd_restore_11" name="btn_window_restore" visible="false" margin="0"/>
                </Box>
                <!-- Close button: full height, flush right, no gaps -->
                <Button class="btn_wnd_close_11" name="btn_window_close" margin="0"/>
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
- Window button width and height are defined by the classes in `global.xml`
  (`btn_wnd_base` is 40×32; the close class uses `stretch` height), so the window XML
  no longer writes `width`/`height`.
- Keep the existing set of title bar buttons unchanged; only adjust appearance attributes
  such as size and margins according to this specification.

## 4. Key Rules

1. **`caption` must match the title bar height**
   The last value of `caption` is the distance from the top of the window to the draggable
   area, i.e. the title bar height. If they differ, you get either "buttons clickable but
   the window cannot be dragged" or extra blank space below the title bar. Both `caption`
   and `sys_menu_rect` expand variables, so always write
   `caption="0,0,0,${SIZE_TITLEBAR_HEIGHT}"` and
   `sys_menu_rect="0,0,${SIZE_TITLEBAR_HEIGHT},${SIZE_TITLEBAR_HEIGHT}"`.

2. **The close button must fill the height and sit flush to the edges**
   `btn_wnd_close_11` already sets `height="stretch"`; write only `margin="0"` in the
   window XML. The button then matches the title bar height and sits flush with the top
   and right edges. Do not use `margin="0,0,0,2"` or override the height in XML,
   otherwise gaps appear above or to the right of the close button.

3. **Normal buttons keep 2 px above and below**
   Title bar 36, button height 32, `margin="0,2,0,2"` — the three values align vertically.
   Button width and height come from the classes; do not override them in window XML.
   To resize the whole bar, change `SIZE_TITLEBAR_HEIGHT` and `SIZE_WND_BTN_HEIGHT`
   (keeping a difference of 4).

4. **Maximize/Restore use a stacked container**
   The outer `Box` uses `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}` and
   `margin="0,2,0,2"`; the inner buttons' `width="stretch" height="stretch" margin="0"`
   come from their classes, so the window XML only writes `class`, `name`, `visible`, and
   similar attributes. The framework toggles their `visible` state by window state, so both
   are never shown at the same time.

5. **Use the shared class for title text**
   The title `Label` uses `class="title_bar_title"` directly (defined in `global.xml`
   with `font="system_regular_14"`, `margin="8,0,0,0"`, and `valign="center"`).
   Stretched titles override `margin="8,0,8,0"`; do not repeat the font and alignment
   attributes in each XML.

6. **Percent-width titles must override the margin**
   If the title `Label` uses `width="100%"`, add `margin="0"`; otherwise the left margin
   from the class makes the title overflow and squeeze the right-side buttons. Use
   `text_padding` to indent the text instead.

7. **Title bars with tabs or other complex content**
   See `cef_browser.xml` / `webview2_browser.xml`: the outer bar may be named `title_bar`,
   and the window buttons go into an inner
   `HBox name="window_title_bar" width="auto" height="${SIZE_TITLEBAR_HEIGHT}"`. Both use
   `${SIZE_TITLEBAR_HEIGHT}`.

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
| 2–4 px gap above/right of the close button | Button height is fixed, or bottom/right margin is not 0 | Reference `btn_wnd_close_11` (height is already `stretch`) and write only `margin="0"` in the window XML |
| Blank space below the title bar after shrinking it | Only `caption` was changed, not the container height | Update both `caption` and `SIZE_TITLEBAR_HEIGHT` |
| Too little space between title and right buttons | Title right margin is 0 and there is no spacer | Use `margin="8,0,8,0"`, or keep the middle `<Control/>` spacer |
| Title text hidden behind buttons | Percent/fixed title width is too large | Remove `margin` from percent-width titles, or use `width="stretch"` |
| Title font differs from other windows | `title_bar_title` is not used, or `font` is overridden | Use `class="title_bar_title"` |
| Maximize and restore buttons shown together | No stacked `Box`, or neither is hidden | Use a `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}` stacked `Box` and set the restore button to `visible="false"` initially |

## 6. Checklist

- [ ] `caption="0,0,0,36"`, `sys_menu_rect="0,0,36,36"` (if the system menu is enabled), matching `SIZE_TITLEBAR_HEIGHT`
- [ ] Title bar container `height="${SIZE_TITLEBAR_HEIGHT}"`, right-side button group the same height
- [ ] Logo uses `class="title_bar_logo"` (override `bkimage` for custom images)
- [ ] Title uses `class="title_bar_title"`; stretched titles override `margin="8,0,8,0"`
- [ ] Normal buttons reference the window button classes; sizes come from the classes, so write only `margin="0,2,0,2"`
- [ ] Maximize/Restore are inside a `${SIZE_WND_BTN_WIDTH}`×`${SIZE_WND_BTN_HEIGHT}` `Box`; the classes provide the stretching
- [ ] Close button references `btn_wnd_close_11` (height `stretch` is in the class); write only `margin="0"`
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
