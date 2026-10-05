English | [简体中文](Toast.md)

## Non-Modal Toast Notification (ToastWnd)

`ToastWnd` is a built-in owner-drawn skin non-modal toast notification window. It is used to
show short-lived hints to the user (such as "Saved successfully" or "Operation failed").
Unlike the modal [MessageBoxWnd](MessageBox.en.md), a Toast **never blocks the parent window
nor steals the focus** when it pops up. It disappears automatically after a given dwell time,
and supports fade in/out animations, multi-toast stacking, click-to-dismiss, and
hover-to-pause interactions.

* Associated header file: [duilib/Utils/ToastWnd.h](../duilib/Utils/ToastWnd.h)
* Skin file: `bin/resources/themes/default/public/toast/toast.xml` (skin folder `public/toast`)
* Icon resources: `toast_info.svg`, `toast_success.svg`, `toast_warning.svg`, `toast_error.svg` in the same folder
* Complete demo: `examples/controls` (ToastForm)

### 1. Interface

It can only be used through the static `Show` interface, fire-and-forget: the call returns
immediately, and the window object is destroyed automatically by the framework after it
closes. Callers do not need (and are not supposed) to manage its lifetime:

```cpp
static void Show(ui::Window* pParentWindow,
                 const DString& text,
                 ToastType type = kTypeInfo,
                 int32_t nDurationMs = 3000,
                 ToastPosition position = kPosTop,
                 bool bTextId = false);
```

| Parameter | Description |
| :--- | :--- |
| pParentWindow | Parent window; the toast is shown near its client area, and the parent remains fully operable while the toast is visible. Can be nullptr, in which case the monitor work area is used |
| text | Notification content; supports explicit `\n` line breaks. Long text wraps automatically, and the window height adapts to the text (up to 200 DIP) |
| type | Notification type (determines the icon on the left), see the table below |
| nDurationMs | Dwell time before auto-close, in milliseconds; default 3000. **Pass 0 to disable auto-close**; the toast can then only be dismissed by a click |
| position | Display position, see the table below |
| bTextId | When true, text is passed as a multi-language ID and is refreshed in real time after a language switch |

### 2. Notification Types (ToastType)

| Enumerator | Description |
| :--- | :--- |
| `kTypeInfo` | Information (blue i); the default type |
| `kTypeSuccess` | Success (green check mark) |
| `kTypeWarning` | Warning (yellow !) |
| `kTypeError` | Error (red X) |

### 3. Display Positions (ToastPosition)

Positions are calculated relative to the parent window's client area; when pParentWindow is
nullptr, the monitor work area is used. Multiple toasts at the same position stack vertically
(see section 7):

| Enumerator | Description |
| :--- | :--- |
| `kPosTop` | Top center; the default. New toasts are laid out downwards |
| `kPosCenter` | Horizontally and vertically centered |
| `kPosBottom` | Bottom center |
| `kPosTopRight` | Top right corner; new toasts are laid out downwards |
| `kPosBottomRight` | Bottom right corner (system-notification style); new toasts are laid out upwards |

The margin from the toast to the parent window's client-area edge is 24 DIP
(`kAnchorMargin`/`kRightMargin`). The final position is clipped to the monitor work area so
that a toast can never extend off-screen.

### 4. Basic Usage

```cpp
#include "duilib/duilib.h"

//Show a success hint for 3 seconds after saving (top center by default)
ui::ToastWnd::Show(this, _T("Saved successfully"), ui::ToastWnd::kTypeSuccess);

//Network request failed: error icon, dwell for 5 seconds
ui::ToastWnd::Show(this, _T("Network connection failed. Please try again later."),
                   ui::ToastWnd::kTypeError, 5000);

//A longer-lived hint in the bottom right corner
ui::ToastWnd::Show(this, _T("File downloaded"),
                   ui::ToastWnd::kTypeInfo, 4000,
                   ui::ToastWnd::kPosBottomRight);
```

### 5. Dwell Time, Hover-to-Pause, and Click-to-Dismiss

* **Auto-close**: the toast plays a fade-out animation and closes after `nDurationMs`
  milliseconds. Timing is implemented with a delayed task posted to the UI thread via
  `ThreadManager::PostDelayedTask`; it does not rely on a window timer, and the task can be
  cancelled by its ID if the window is destroyed unexpectedly.
* **0 means no auto-close**: when `nDurationMs` is 0, the toast stays until the user clicks
  it to dismiss it. This is useful for important information the user must acknowledge.
* **Hover to pause**: moving the mouse into the toast pauses the countdown; moving it away
  resumes with the **remaining time**. The remaining time is calculated at the moment of
  pausing using a `steady_clock` millisecond timestamp, so repeatedly moving the mouse in
  and out never resets the total dwell time.
* **Click to dismiss immediately**: clicking anywhere on the toast bar plays the fade-out
  animation and closes it immediately, without waiting for the countdown. The click and
  hover events are attached to the XML root container; see section 9 for implementation
  notes.

### 6. Multi-Language Support

When the last parameter `bTextId` is true, `text` is passed as a multi-language ID
(internally via `Label::SetTextId`). The framework resolves it to the text of the current
language and refreshes it in real time when the language is switched:

```cpp
ui::ToastWnd::Show(this,
    _T("STRID_TOAST_DEMO_INFO"),
    ui::ToastWnd::kTypeInfo,
    3000,
    ui::ToastWnd::kPosTop,
    true);
```

### 7. Stacking and the Maximum Count

* All active toasts are kept in a static list `s_toasts` (accessed on the UI thread only).
* Only toasts with the **same parent window and the same position** are stacked: for example,
  a top-center toast and a bottom-right toast are laid out independently. Each new toast is
  placed after the previous one, with a gap of 12 DIP (`kToastGap`).
* When a toast closes (by timeout or click), it is removed from the list first, and the
  remaining toasts **smoothly slide up** to fill the gap via a move animation.
* At most 5 toasts are shown at the same time (`kMaxToastCount`). When the limit is reached,
  the oldest toast starts its exit flow before the new one is added.

### 8. Animations

Animations use the framework's `AnimationPlayer` (the same mechanism used by controls such as
TabBox), with animation type `AnimationType::kAnimationNone`; the callbacks drive the window
properties themselves:

| Animation | Duration | Easing | Effect |
| :--- | :--- | :--- | :--- |
| Enter | 200ms | `EaseOutCubic` | Fade in from alpha 0 to 255, while sliding up 12 DIP from below the target position |
| Exit | 180ms | `EaseInCubic` | Fade out from alpha 255 to 0; `CloseWnd` is called when finished |
| Reflow | 200ms | `EaseOutCubic` | After another toast closes, the remaining toasts slide along the Y axis to their new positions |

The window itself is a layered window (`kWS_EX_LAYERED`); fade in/out is implemented with
`SetLayeredWindowAlpha`. All animation callbacks are protected with `GetWeakFlag()`, so they
never touch members after the window has been destroyed. If a reflow is triggered while a
toast is still playing its enter animation, that toast snaps directly to its target position
to avoid the enter and move animations overriding each other.

### 9. Implementation Notes and Skin Structure

The window style is `kWS_POPUP | kWS_EX_TOPMOST | kWS_EX_LAYERED | kWS_EX_NOACTIVATE`:
topmost, layered, and **non-activating** (it never steals focus). When there is no parent
window, `kWS_EX_TOOLWINDOW` is added as well, so the toast does not appear in the taskbar or
the Alt+Tab list. The skin root node enables the window shadow (`shadow_attached="true"`).

Key structure of the `toast.xml` skin:

* The root HBox is the toast bar itself: fixed width of 360 DIP, auto height
  (`max_height="200"`), corner radius of 8px. It reuses the semantic color names
  `bg_tooltip` (background), `text_tooltip` (text), and `border_window` (border); no new
  color names are introduced.
* An inner HBox arranges the icon (`name="toast_icon"`) and the text
  (`name="toast_text"`, `multi_line="true"`).
* The inner container, the icon, and the text all set `mouse_enabled="false"` and
  `tab_stop="false"`, so mouse messages always hit the outermost toast bar.

There are three pitfalls when implementing this kind of interactive window with the
framework; keep them in mind if you develop similar controls:

1. **Plain containers do not dispatch `kEventClick`**: `kEventClick` is only produced by
   controls such as Button, ComboButton, and ListBoxItem. To make an entire container bar
   clickable, use `AttachButtonUp` (listening for `kEventMouseButtonUp`) instead of
   `AttachClick`.
2. **For a window with a shadow, attach events to `GetXmlRoot()`, not to `GetRoot()`**:
   `GetRoot()` returns the framework-injected shadow container ShadowBox, which is
   explicitly configured with `SetMouseEnabled(false)` and receives no mouse messages at
   all. Use `GetXmlRoot()` to get the actual visible root container defined in the XML.
   Note that size measurement must still use `GetRoot()`: its `EstimateSize` result
   includes the shadow, and its `GetPadding()` returns the shadow margins on all four sides.
3. **No control-level `kEventMouseLeave` is received when the mouse leaves the window
   entirely**: the control-level leave event is only dispatched when moving from one control
   to another within the same window. When the mouse moves directly out of a small window's
   boundary, the framework's `Window::OnMouseLeaveMsg` (WM_MOUSELEAVE) does not forward the
   event to controls. `ToastWnd` overrides `OnMouseLeaveMsg`, calls
   `ResumeAutoCloseTimer()` in it to resume the countdown, and then calls the base class
   implementation.

In addition, the toast window is created as an owned window of the parent, so it is destroyed
together with the parent when the parent closes. `OnFinalMessage` removes the toast from the
static list, reflows the remaining toasts, and then the non-modal window base class
automatically performs `delete this`.
