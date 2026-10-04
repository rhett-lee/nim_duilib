English | [简体中文](Events.md)

> Last synced: 2026-09-29

## Control Events (EventArgs) Documentation
The content related to control events is defined in the files [`duilib/Core/EventArgs.h`](../duilib/Core/EventArgs.h) and [`duilib/duilib_defs.h`](../duilib/duilib_defs.h).

| Event (eventType)           | Parameter (wParam)  | Parameter (lParam) | Parameter (ptMouse) | Parameter (vkCode) | Parameter (modifierKey) |Parameter (eventData) | Note     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventKeyDown              |Original value|Original value|     |    Associated key  |  Key flag  |     |     |
|kEventKeyUp                |Original value|Original value |     |    Associated key  |  Key flag  |     |     |
|kEventChar                 |Original value|Original value|     |    Associated key  |  Key flag  |     |     |
|kEventMouseEnter           |     |     |Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseLeave           |     |     |Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseMove            |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseHover           |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseWheel           |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |  wheelDelta data |     |
|kEventMouseButtonDown      |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseButtonUp        |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseDoubleClick     |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseRButtonDown     |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseRButtonUp       |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseRDoubleClick    |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventContextMenu          |     |Control*<br> The control at the mouse position| Client-area coordinates<br> of the mouse position    |     |       |     | If the user presses SHIFT+F10,<br> then ptMouse is (-1,-1) and lParam is 0 |
|kEventClick                |     |     |Client-area coordinates<br> of the mouse position   | Parameter (vkCode)   |  Key flag   | Old event type  | There are many possible parameters, need to detect  |
|kEventRClick               |     |     |Client-area coordinates<br> of the mouse position   |     |  Key flag   |     |     |
|kEventMouseClickChanged    |     |     |     |     |       |     | No parameters |
|kEventMouseClickEsc        |     |     |     |     |       |     | No parameters |

| Event (eventType)           | Parameter (wParam)  | Parameter (lParam) | Parameter (ptMouse) | Parameter (vkCode) | Parameter (modifierKey) |Parameter (eventData) | Note     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventSetFocus             |     |     |     |     |       |     | No parameters |
|kEventKillFocus            |     |Control*<br> The new focus control<br> or nullptr |     |     |       |     |     |
|kEventSetCursor            |     |     | Client-area<br> coordinates of the mouse   |     |       |     |   |
|kEventImeStartComposition  |     |     |     |     |       |     |No parameters|
|kEventImeEndComposition    |     |     |     |     |       |     |No parameters|
|kEventWindowKillFocus      |     |     |     |     |       |     |No parameters |
|kEventWindowSize           |     |     |     |     |       |Window size change type:<br>WindowSizeType|     |
|kEventWindowMove           |     |     | Top-left corner<br> coordinates of the window    |     |       |     |     |
|kEventWindowClose          |0: normal close <br> 1: cancel close|     |     |     |       |     |     |
|kEventSelect               |ListBox/Combo: <br>New selected index | ListBox/Combo: <br>Old selected index|     |     |       |     | Other classes have no parameters |
|kEventUnSelect             |ListBox: <br>New selected index | ListBox:<br>Old selected index|     |     |       |     |  Other classes have no parameters   |
|kEventChecked              |     |     |     |     |       |     | No parameters |
|kEventUnCheck              |     |     |     |     |       |     | No parameters |
|kEventTabSelect            |New selected index | Old selected index|     |     |    |     |    |
|kEventExpand               |     |     |     |     |       |     | No parameters; fired after a Panel finishes expanding |
|kEventCollapse             |     |     |     |     |       |     | No parameters; fired after a Panel finishes collapsing |
|kEventPanelExpanding       |     |     |Client-area coordinates of the mouse |     | Key flags   |     | Fired before a Panel expands; returning false from the handler cancels the expand |
|kEventPanelCollapsing      |     |     |Client-area coordinates of the mouse |     | Key flags   |     | Fired before a Panel collapses; returning false from the handler cancels the collapse |


| Event (eventType)           | Parameter (wParam)  | Parameter (lParam) | Parameter (ptMouse) | Parameter (vkCode) | Parameter (modifierKey) |Parameter (eventData) | Note     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventZoom                 | Scaling ratio numerator [0,64] | Scaling ratio denominator (0,64] |     |     |       |     | RichEdit: Ctrl + wheel: zoom function|
|kEventTextChange           |     |     |     |     |       |     | No parameters |
|kEventSelChange            |     |     |     |     |       |     | No parameters |
|kEventReturn               |     |     |     |     |       |     | No parameters    |
|kEventTab                  |     |     |     |     |       |     | No parameters   |
|kEventLinkClick            | DString.c_str()<br> URL string    |     |     |     |       |     |     |
|kEventScrollChange         | 0: cy unchanged<br> 1: cy changed   | 0: cx unchanged<br> 1: cx changed    |     |     |       |     |     |
|kEventValueChange          |     |     |     |     |       |     | No parameters |
|kEventResize               |     |     |     |     |       |     | No parameters |
|kEventVisibleChange        |     |     |     |     |       |     | No parameters |
|kEventStateChange          | New state | Old state   |     |     |       |     | ControlStateType |
|kEventSelectColor          | Selected color |     |     |     |       |     | newColor.GetARGB() |
|kEventSplitDraged          | Control*: <br>First control interface| Control*:<br>Second control interface|     |     |       |     |  May be nullptr  |
|kEventEnterEdit            | ListCtrlEditParam*:<br>Data entering edit state  |     |     |     |       |     |     |
|kEventLeaveEdit            | ListCtrlEditParam*:<br>Data leaving edit state    |     |     |     |       |     |     |

| Event (eventType)           | Parameter (wParam)  | Parameter (lParam) | Parameter (ptMouse) | Parameter (vkCode) | Parameter (modifierKey) |Parameter (eventData) | Note     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventPathChanged          |     |     |     |     |       |     |  No parameters   |
|kEventPathClick            |     |     |     |     |       |     |  No parameters   |
|kEventDropEnter            | ControlDropType|When wParam is kControlDropTypeWindows,<br> lParam is a pointer to ControlDropData_Windows|     |     |       |     |  No parameters   |
|kEventDropOver             | ControlDropType|When wParam is kControlDropTypeWindows,<br> lParam is a pointer to ControlDropData_Windows|     |     |       |     |  No parameters   |
|kEventDropLeave            |     |     |     |     |       |     |  No parameters   |
|kEventDropData             | ControlDropType | When wParam is kControlDropTypeWindows,<br> lParam is a pointer to ControlDropData_Windows; <br>When wParam is kControlDropTypeSDL,<br> lParam is a pointer to ControlDropData_SDL|     |     |       |     |  No parameters   |
|kEventImageAnimationStart  | wParam is a data pointer: ui::ImageAnimationStatus*|     |     |     |       |     |  No parameters   |
|kEventImageAnimationPlayFrame  | wParam is a data pointer: ui::ImageAnimationStatus*|     |     |     |       |     |  No parameters   |
|kEventImageAnimationStop   | wParam is a data pointer: ui::ImageAnimationStatus*|     |     |     |       |     |  No parameters   |
|kEventLoadingStart         | wParam is a data pointer: ui::ControlLoadingStatus*|     |     |     |       |     |  No parameters   |
|kEventLoading              | wParam is a data pointer: ui::ControlLoadingStatus*|     |     |     |       |     |  No parameters   |
|kEventLoadingStop          | wParam is a data pointer: ui::ControlLoadingStatus*|     |     |     |       |     |  No parameters   |
|kEventImageLoad            | wParam is a data pointer: ui::ImageDecodeResult*   |     |     |     |       |     |  No parameters   |
|kEventImageDecode          | wParam is a data pointer: ui::ImageDecodeResult*   |     |     |     |       |     |  No parameters   |
|kEventLast                 |     |     |     |     |       |     |  No parameters   |
