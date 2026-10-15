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
|kEventCheck                |     |     |     |     |       |     | No parameters |
|kEventUnCheck              |     |     |     |     |       |     | No parameters |
|kEventTabSelect            |New selected index | Old selected index|     |     |    |     |    |
|kEventExpand               |     |     |     |     |       |     | No parameters; fired after a Panel finishes expanding |
|kEventCollapse             |     |     |     |     |       |     | No parameters; fired after a Panel finishes collapsing |
|kEventPanelExpanding       |     |     |Client-area coordinates of the mouse |     | Key flags   |     | Fired before a Panel expands; returning false from the handler cancels the expand |
|kEventPanelCollapsing      |     |     |Client-area coordinates of the mouse |     | Key flags   |     | Fired before a Panel collapses; returning false from the handler cancels the collapse |
|kEventNavigationItemClick  |Selectable item index |     |     |     |       |     | Fired when a selectable NavigationView item is clicked; also fires when clicking the currently selected item |
|kEventNavigationSelectionChanged |New selected index | Old selected index |     |     |    |     | Fired when the NavigationView selection changes; Box::InvalidIndex when nothing is selected |
|kEventNavigationPaneToggling |Target state:<br>1=collapsed, 0=expanded |     |Client-area coordinates of the mouse |     | Key flags |     | Fired before the NavigationView pane collapses/expands; returning false from the handler cancels the toggle |
|kEventNavigationPaneToggled |Current state:<br>1=collapsed, 0=expanded |     |     |     |    |     | Fired after the NavigationView pane has collapsed/expanded |
|kEventTimelineItemClick  |Item index (0-based) |     |Client-area coordinates of the mouse |     | Key flags |     | Fired when a Timeline node is clicked |
|kEventTagAdded            |New tag index (0-based) |     |Client-area coordinates of the mouse |     | Key flags |     | Fired after a tag is added to TagInput |
|kEventTagRemoved          |Removed tag index (0-based) |     |Client-area coordinates of the mouse |     | Key flags |     | Fired before a tag is removed from TagInput; its text is still readable via GetTagText in the callback |
|kEventTagClick            |Tag index (0-based) |     |Client-area coordinates of the mouse |     | Key flags |     | Fired when the tag body (not the close button) in TagInput is clicked |
|kEventAutoCompleteSelect  |Selected item index within the current candidate list (0-based) |     |Client-area coordinates of the mouse |     | Key flags |     | Fired after a candidate in AutoComplete is chosen (mouse click or Enter); inline XML event name is `select` |


| Event (eventType)           | Parameter (wParam)  | Parameter (lParam) | Parameter (ptMouse) | Parameter (vkCode) | Parameter (modifierKey) |Parameter (eventData) | Note     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventZoom                 | Scaling ratio numerator [0,64] | Scaling ratio denominator (0,64] |     |     |       |     | RichEdit: Ctrl + wheel: zoom function|
|kEventTextChanged          |     |     |     |     |       |     | No parameters |
|kEventSelChanged           |     |     |     |     |       |     | No parameters |
|kEventReturn               |     |     |     |     |       |     | No parameters    |
|kEventTab                  |     |     |     |     |       |     | No parameters   |
|kEventLinkClick            | DString.c_str()<br> URL string    |     |     |     |       |     |     |
|kEventScrollPosChanged     | 0: cy unchanged<br> 1: cy changed   | 0: cx unchanged<br> 1: cx changed    |     |     |       |     |     |
|kEventValueChanged         |New value | Old value |     |     |       |     | Slider: fired when the value changes |
|kEventProgressCompleted    |     |     |     |     |       |     | Progress: fired once when the value reaches the completion endpoint (max forward, min countdown); no parameters |
|kEventPosChanged           |     |     |     |     |       |     | No parameters |
|kEventSizeChanged          |     |     |     |     |       |     | No parameters |
|kEventVisibleChanged       |New state (1=visible, 0=invisible)|     |     |     |       |     | No parameters |
|kEventStateChanged         | New state | Old state   |     |     |       |     | ControlStateType |
|kEventSelectColor          | Selected color |     |     |     |       |     | newColor.GetARGB() |
|kEventSplitDraged          | Control*: <br>First control interface| Control*:<br>Second control interface|     |     |       |     |  May be nullptr  |
|kEventEnterEdit            | ListCtrlEditParam*:<br>Data entering edit state  |     |     |     |       |     | ListCtrl cell editing: fired when entering edit state (row-mode click-to-edit, or F2 in kCell mode which enters editing immediately); pEventData points to ListCtrlEditParam (row index / column ID / original text, etc.); the callback may cancel to prevent entering edit state |
|kEventLeaveEdit            | ListCtrlEditParam*:<br>Data leaving edit state    |     |     |     |       |     | ListCtrl cell editing: fired when leaving edit state (Enter commit / focus-loss commit / Esc cancel); pEventData points to ListCtrlEditParam; on commit, invalid input (number/date columns) is discarded — no write-back and this event is NOT fired; after a successful Enter commit in kCell mode the current cell moves down one row (check msg.bCancelled in the callback to know whether it was cancelled) |
|kEventColumnWidthChanged   | Column ID (size_t)  | Related column ID (size_t),<br>may be Box::InvalidIndex |     |     |       |     | ListCtrl: a column width changed (triggered by dragging the header splitter, double-click auto-fit, or SetColumnWidth) |
|kEventColumnOrderChanged   |     |     |     |     |       |     | ListCtrl: the column display order changed (triggered when dragging the header to reorder columns); iterate with GetColumnId to get the new order |
|kEventColumnVisibleChanged | Column ID (size_t)  | New state (1 visible / 0 hidden) |     |     |       |     | ListCtrl: a column's show/hide state changed |
|kEventColumnSorted         | Column ID (size_t)  | New state (1 ascending / 0 descending) |     |     |       |     | ListCtrl: a column's sort state changed |
|kEventCurrentCellChanged   | Row display index (size_t) | Column ID (size_t) |     |     |       |     | ListCtrl: the current cell changed (cell selection mode) |
|kEventCellSelectionChanged | Number of selected areas (size_t) | 0 (reserved) |     |     |       | std::vector<ui::ListCtrlCellRange>*:<br>the list of selected areas (possibly multiple; nullptr when there is no selection) | ListCtrl: the cell selection area changed (a list of areas is used because discontiguous selection is supported; the pointer is nullptr and wParam is 0 when there is no valid selection) |

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
