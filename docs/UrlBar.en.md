English | [简体中文](UrlBar.md)

## Browser Address Bar (UrlBar)

`UrlBar` is a self-drawn browser address bar control (derived from `HBox`) used to enter and display
URLs. Its look and interaction follow common browsers (e.g. Edge):

* A security-status icon on the left: a lock for https, a globe for other protocols; **clicking it fires an event**;
* A single-line edit box in the middle: the text is selected automatically when it gets focus, and a
  clear button is shown while focused;
* A favorite button on the far right; **clicking it fires an event**;
* While typing, history is filtered and a suggestion drop-down is shown; Up/Down to navigate,
  Enter to confirm, Esc to close.

Drop-down open/close rules:

* Open: only when the user types text in the edit box and there are matches; focusing the edit box
  or clicking the address bar does **not** open it;
* Close: Esc; submitting with Enter (including choosing a suggestion); clicking a suggestion; empty
  input or no matches; the edit box losing focus; mouse pressing outside the address bar and the
  drop-down; the window being hidden or minimized;
* The drop-down is a no-activate window (`WS_EX_NOACTIVATE`): keyboard focus stays in the edit box
  while it is open, so the user can keep typing.

* Header: [duilib/Control/UrlBar.h](../duilib/Control/UrlBar.h)
* Skin classes (`global.xml`): `url_bar` / `url_bar_icon_secure` / `url_bar_icon_insecure` /
  `url_bar_edit` / `url_bar_clear_btn` / `url_bar_fav_btn`
* Icon resources: `public/url_bar/lock.svg`, `public/url_bar/globe.svg`;
  the favorite button reuses `public/checkbox/star_outline.svg`, and the clear button reuses
  `public/button/window-close.svg`
* The suggestion drop-down reuses AutoComplete skins: `auto_complete_flyout` / `auto_complete_list` /
  `auto_complete_item`
* Full demos: `examples/cef`, `examples/cef_browser`, `examples/webview2`, `examples/webview2_browser`

### 1. XML Example

```xml
<UrlBar class="url_bar" name="url_bar"
        width="stretch" height="34" valign="center"
        prompt_text_id="STRID_CEF_BROWSER_INPUT_ADDRESS"
        search_url_template="https://www.bing.com/search?q=%s"
        history_file="user/cef_browser_url_history.txt"
        default_context_menu="true"
        clear_button_enabled="true"
        favorite_button_enabled="true"/>
```

### 2. XML Attributes

In addition to the common `HBox` attributes:

| Attribute | Default | Description |
| :--- | :--- | :--- |
| `prompt_text` | empty | Placeholder text of the edit box |
| `prompt_text_id` | empty | Multi-language ID of the placeholder text |
| `prompt_color` | empty | Color of the placeholder text |
| `text` | empty | Initial URL text |
| `text_id` | empty | Multi-language ID of the initial URL text (if both `text` and `text_id` are set, the later one wins) |
| `search_url_template` | empty | Search URL template containing a `%s` placeholder (e.g. `https://www.bing.com/search?q=%s`); used when the input does not look like a URL; when empty, `https://` is prepended |
| `history_enabled` | `true` | Whether to record and show URL history |
| `max_history` | `20` | Maximum number of history entries |
| `history_file` | empty | History persistence file (relative to the executable directory); empty means no persistence |
| `max_visible` | `8` | Maximum visible suggestion rows; a scroll bar appears when exceeded |
| `secure` | `false` | Initial security state: `true` shows the lock icon, `false` shows the globe icon |
| `default_context_menu` | empty | Whether to enable the edit box's default context menu (forwarded to the inner edit box) |
| `clear_button_enabled` | `true` | Whether to enable the clear button of the edit box |
| `favorite_button_enabled` | `true` | Whether to show the favorite button on the far right |

### 3. Events

| Event | When it fires | Parameters | Convenience API |
| :--- | :--- | :--- | :--- |
| `kEventTextChanged` | Fired in real time when the edit box text changes | None | `kEventTextChanged` of the inner edit box (`GetEditControl()->AttachTextChanged`) |
| `kEventUrlChanged` | After the user submits a new URL (Enter or choosing a suggestion) | None; get the URL via `GetUrl()` | `AttachUrlChanged()` |
| `kEventUrlCancel` | After the user presses Esc to cancel editing and restore the committed URL | None | `AttachUrlCancel()` |
| `kEventUrlBarIconClick` | After clicking the security-status icon (lock/globe) on the left | None | `AttachIconClick()` |
| `kEventUrlBarFavoriteClick` | After clicking the favorite button on the far right | None | `AttachFavoriteClick()` |

The inline XML event names are `text_changed`, `url_changed`, `url_cancel`,
`url_bar_icon_click` and `url_bar_favorite_click` (the capitalized forms such as `UrlChanged` are
also accepted; see [XmlEvents.en.md](XmlEvents.en.md)).

```cpp
UrlBar* pUrlBar = dynamic_cast<UrlBar*>(FindControl(_T("url_bar")));
if (pUrlBar != nullptr) {
    //Submit: navigate in the callback
    pUrlBar->AttachUrlChanged([this](const ui::EventArgs& /*args*/) {
        DString url = m_pUrlBar->GetUrl();
        Navigate(url);
        return true;
    });
    //Left security icon: the application can show a site info panel
    pUrlBar->AttachIconClick([this](const ui::EventArgs& /*args*/) {
        ShowSiteInfo();
        return true;
    });
    //Favorite button: the application can add/remove a favorite
    pUrlBar->AttachFavoriteClick([this](const ui::EventArgs& /*args*/) {
        ToggleFavorite(m_pUrlBar->GetUrl());
        return true;
    });
}
```

### 4. C++ API

| API | Description |
| :--- | :--- |
| `GetEditControl()` | Gets the inner edit box (`RichEdit*`) |
| `GetUrl()` / `SetUrl()` | Gets the last committed URL / sets the current URL (does not overwrite user input while editing) |
| `GetEditText()` / `SelectAll()` | Gets the raw text in the edit box / selects all text |
| `Submit()` | Submits the current input (same as pressing Enter) and fires `kEventUrlChanged` after normalization |
| `SetFocus()` | Gives the control focus (the focus actually goes to the inner edit box) |
| `SetSearchUrlTemplate()` / `GetSearchUrlTemplate()` | Sets/gets the search URL template |
| `SetSecure()` / `IsSecure()` | Sets/reads the security state (lock/globe) |
| `SetFavoriteButtonEnabled()` / `IsFavoriteButtonEnabled()` | Shows/hides the favorite button (shown by default) |
| `SetClearButtonEnabled()` / `IsClearButtonEnabled()` | Enables/disables the clear button (enabled by default) |
| `SetHistoryEnabled()` / `IsHistoryEnabled()` | Whether to record and show URL history (enabled by default) |
| `SetMaxHistory()` / `GetMaxHistory()` | Maximum number of history entries |
| `SetMaxVisible()` / `GetMaxVisible()` | Maximum visible suggestion rows |
| `GetHistory()` / `ClearHistory()` / `AddHistory()` | Reads/clears/appends URL history |
| `SetHistoryFile()` / `GetHistoryFile()` | Sets/gets the history persistence file |

### 5. Skin Classes

| Skin class | Description |
| :--- | :--- |
| `url_bar` | Address bar container: rounded capsule background, border and hovered border |
| `url_bar_icon_secure` | Left security icon (https, lock) |
| `url_bar_icon_insecure` | Left insecure icon (other protocols, globe) |
| `url_bar_edit` | Inner single-line edit box |
| `url_bar_clear_btn` | Clear button at the right side of the edit box |
| `url_bar_fav_btn` | Favorite button (star) on the far right |

Adjusting these skin classes changes the address bar appearance (size, corner radius, colors,
icons, etc.) without touching application code.
