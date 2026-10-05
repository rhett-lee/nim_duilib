English | [简体中文](MessageBox.md)

## Modal Message Box (MessageBoxWnd)

`MessageBoxWnd` is a built-in owner-drawn skin modal message box. It is displayed modally with
`DoModal`, blocks the parent window, and returns the button clicked by the user. Its API style is
similar to the Win32 `MessageBox`, supporting common button combinations, standard icons,
multi-language, and custom button text.

* Associated header file: [duilib/Utils/MessageBoxWnd.h](../duilib/Utils/MessageBoxWnd.h)
* Skin file: `bin/resources/themes/default/public/messagebox/messagebox.xml` (skin folder `public/messagebox`)
* Icon resources: `msg_info.svg`, `msg_warning.svg`, `msg_error.svg`, `msg_question.svg` in the same folder
* Complete demo: `examples/controls` (MessageBoxForm)

### 1. Interface

It can only be used through the static `Show` interface; objects cannot (and need not) be created directly:

```cpp
static int32_t Show(ui::Window* pParentWindow,
                    const DString& text,
                    const DString& title,
                    uint32_t buttonFlags = kButtonsOK,
                    IconType iconType = kIconNone,
                    const ButtonText* pButtonText = nullptr,
                    bool bTextId = false);
```

| Parameter | Description |
| :--- | :--- |
| pParentWindow | Parent window, disabled during the modal loop; can be nullptr |
| text | Message content; supports `\n` line breaks. Long text wraps automatically, and the window height adapts to the text |
| title | Window title |
| buttonFlags | Button combination, see the table below |
| iconType | Icon type, see the table below |
| pButtonText | Custom button text; when nullptr, the framework default text is used |
| bTextId | When true, text, title, and button text are all passed as multi-language IDs (refreshed in real time after a language switch) |

The return value is the button selected by the user (see the return value table); it returns -1 if window creation fails.

### 2. Button Combinations (ButtonFlags)

| Enumerator | Buttons Included |
| :--- | :--- |
| `kButtonsOK` | OK |
| `kButtonsOKCancel` | OK, Cancel |
| `kButtonsYesNo` | Yes, No |
| `kButtonsYesNoCancel` | Yes, No, Cancel |
| `kButtonsRetryCancel` | Retry, Cancel |

Button visibility is controlled by a bitwise combination of the flag bits
(`kButtonOK`/`kButtonCancel`/`kButtonYes`/`kButtonNo`/`kButtonRetry`).

### 3. Icon Types (IconType)

| Enumerator | Description |
| :--- | :--- |
| `kIconNone` | No icon |
| `kIconInfo` | Information (blue i) |
| `kIconWarning` | Warning (yellow !) |
| `kIconError` | Error (red X) |
| `kIconQuestion` | Question (blue ?) |

### 4. Return Values (Result)

The values are consistent with the `ID*` return values of the Win32 `MessageBox`:

| Enumerator | Value | Trigger |
| :--- | :--- | :--- |
| `kResultOK` | 1 | Clicking the "OK" button |
| `kResultCancel` | 2 | Clicking the "Cancel" button, pressing ESC, or clicking the title bar close button |
| `kResultRetry` | 4 | Clicking the "Retry" button |
| `kResultYes` | 6 | Clicking the "Yes" button |
| `kResultNo` | 7 | Clicking the "No" button |

Note: pressing Enter activates the **currently focused button**. The default focus is the first
affirmative button in the combination (OK first, then Yes, then Retry), so pressing Enter directly
in a Yes/No combination returns `kResultYes(6)`. ESC and the title bar close button always return
`kResultCancel(2)`; even when the current combination has no "Cancel" button, the result is never
mapped to the default button, to avoid triggering an affirmative action when the user abandons the choice.

### 5. Basic Usage

```cpp
#include "duilib/Utils/MessageBoxWnd.h"

int32_t nResult = ui::MessageBoxWnd::Show(this,
    _T("Are you sure you want to delete this file? This cannot be undone."),
    _T("Delete Confirmation"),
    ui::MessageBoxWnd::kButtonsYesNoCancel,
    ui::MessageBoxWnd::kIconQuestion);
if (nResult == ui::MessageBoxWnd::kResultYes) {
    //Perform the deletion
}
```

### 6. Multi-Language Support

When the last parameter `bTextId` is true, `text`, `title`, and the button text are all passed as
multi-language IDs. The framework resolves them to the text of the current language and refreshes
them in real time after a language switch; in this case the native window title shown in the task
bar is also the translated text rather than the ID string:

```cpp
int32_t nResult = ui::MessageBoxWnd::Show(this,
    _T("STRID_DELETE_CONFIRM_TEXT"),
    _T("STRID_DELETE_CONFIRM_TITLE"),
    ui::MessageBoxWnd::kButtonsOKCancel,
    ui::MessageBoxWnd::kIconWarning,
    nullptr, true);
```

The multi-language IDs for the default button text are `STRID_MSGBOX_OK`, `STRID_MSGBOX_CANCEL`,
`STRID_MSGBOX_YES`, `STRID_MSGBOX_NO`, and `STRID_MSGBOX_RETRY`.

### 7. Custom Button Text

Each button's text can be customized through the `ButtonText` struct; if an item is left empty, the
framework default text is used for that item. When `bTextId` is true, the struct members are also
filled with multi-language IDs:

```cpp
ui::MessageBoxWnd::ButtonText buttonText;
buttonText.yes    = _T("STRID_CUSTOM_SAVE");     //e.g. "Save"
buttonText.no     = _T("STRID_CUSTOM_DISCARD");  //e.g. "Don't Save"
buttonText.cancel = _T("STRID_CUSTOM_CANCEL");   //e.g. "Cancel"
ui::MessageBoxWnd::Show(this,
    _T("STRID_SAVE_CHANGED_TEXT"), _T("STRID_SAVE_CHANGED_TITLE"),
    ui::MessageBoxWnd::kButtonsYesNoCancel,
    ui::MessageBoxWnd::kIconQuestion,
    &buttonText, true);
```

### 8. Keyboard Operations

* **TAB / Shift+TAB**: Cycles focus forward/backward between the functional buttons; the title bar close button does not participate in the TAB cycle.
* **Enter**: Activates the currently focused button (buttons respond with click to `kEventKeyDown` + `kVK_RETURN`/`kVK_SPACE`).
* **ESC**: Cancels and returns `kResultCancel(2)`.
* **Focus rectangle**: When the window first pops up, the button focus dotted rectangle is not shown;
  it is enabled only after the user presses TAB for keyboard navigation, via
  `MessageBoxWnd::OnKeyDownMsg` calling `SetShowFocusedRect(true)`, to avoid showing a focus ring
  immediately when the UI appears. The XML attribute name is `show_focused_rect`
  (alias `show_focus_rect`); see [Control.en.md](Control.en.md) for details.

### 9. Keyboard Message Mechanism of Modal Dialogs (Windows Platform)

On the Windows platform, `DoModal` creates a modal dialog through `DialogBoxIndirectParam`, whose
message loop calls `IsDialogMessage` before dispatching messages. This API swallows two types of keys:

* **TAB key**: Converted into focus switching between native child controls. An owner-drawn window
  has no native child controls, so the TAB key never reaches the window procedure, and keyboard
  navigation of owner-drawn buttons would fail.
* **Enter key**: Converted into `WM_COMMAND` + `IDOK`; the window procedure never receives
  `WM_KEYDOWN`, so pressing Enter cannot activate the currently focused button and only follows the
  default IDOK closing path.

The framework handles this in the `IsDialogMessageDuiLib` inline hook in
`duilib/Core/NativeWindow_Windows.cpp`:

* **TAB key is passed through globally** (regardless of the window); after passing through, focus
  switching is completed by `SetNextTabControl` in `Window::OnKeyDownMsg`, so all modal windows
  support TAB navigation.
* **Enter key is passed through only for explicitly registered windows**, registered by window
  handle, to avoid changing the default "Enter follows IDOK" behavior of ordinary modal dialogs.
  The registration interface is:

```cpp
//duilib/Core/NativeWindow_Windows.h (Windows platform only)
static void NativeWindow_Windows::SetEnterKeyPassthrough(HWND hWnd, bool bPassthrough);
```

`MessageBoxWnd` already completes the registration internally (registered in `OnInitWindow` and
unregistered in `OnPreCloseWindow`), so callers do not need to care about it. If a custom modal
window also needs "Enter activates the currently focused button", it must register/unregister in
the same paired way; if it is not unregistered when the window closes, reuse of that handle by the
system may affect later modal dialogs:

```cpp
void MyModalWnd::OnInitWindow()
{
    BaseClass::OnInitWindow();
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ui::NativeWindow_Windows::SetEnterKeyPassthrough(NativeWnd()->GetHWND(), true);
#endif
}

void MyModalWnd::OnPreCloseWindow()
{
#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
    ui::NativeWindow_Windows::SetEnterKeyPassthrough(NativeWnd()->GetHWND(), false);
#endif
    BaseClass::OnPreCloseWindow();
}
```

Note: the last two parameters of `WindowBase::DoModal` control the default closing behavior; the
signature is `DoModal(pParentWindow, createParam, bCloseByEsc = true, bCloseByEnter = false)`,
i.e. by default ESC closes the window while Enter does not. `MessageBoxWnd` internally calls it
with `(true, true)`, allowing both. Among the modal return values, ESC/`IDCANCEL` corresponds to
`kWindowCloseCancel(2)`, and Enter/`IDOK` corresponds to `kWindowCloseOK(1)`.
