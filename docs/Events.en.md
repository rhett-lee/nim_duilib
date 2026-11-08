English | [简体中文](Events.md)

> Last synced: 2026-11-08

## Control Events (EventArgs) Documentation
The content related to control events is defined in the files [`duilib/Core/EventArgs.h`](../duilib/Core/EventArgs.h) and [`duilib/duilib_defs.h`](../duilib/duilib_defs.h).

| Event (eventType)           | Parameter (wParam)  | Parameter (lParam) | Parameter (ptMouse) | Parameter (vkCode) | Parameter (modifierKey) |Parameter (eventData) | Note     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventAll                  |     |     |     |     |       |     | No parameters; callbacks registered via `AttachAllEvents` receive all events |
|kEventDestroy              |     |     |     |     |       |     | No parameters; the last event fired when a control is destroyed |
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
|kEventMouseMButtonDown     |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     | Middle mouse button pressed |
|kEventMouseMButtonUp       |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     | Middle mouse button released |
|kEventMouseMDoubleClick    |Original value|Original value|Client-area coordinates<br> of the mouse position   |     |  Key flag   |     | Middle mouse button double-clicked |
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
|kEventCaptureChanged       |     |     |     |     |       |     | No parameters; fired when the mouse capture state changes |
|kEventImeSetContext        |Original value|Original value|     |     |       |     | IME context message (WM_IME_SETCONTEXT) |
|kEventImeStartComposition  |     |     |     |     |       |     |No parameters|
|kEventImeComposition       |Original value|Original value|     |     |       |     | IME composition message (WM_IME_COMPOSITION) |
|kEventImeEndComposition    |     |     |     |     |       |     |No parameters|
|kEventWindowSetFocus       |     |     |     |     |       |     | No parameters; fired when the window of the current focus control gets focus |
|kEventWindowKillFocus      |     |     |     |     |       |     |No parameters |
|kEventWindowSize           |     |     |     |     |       |Window size change type:<br>WindowSizeType|     |
|kEventWindowMove           |     |     | Top-left corner<br> coordinates of the window    |     |       |     |     |
|kEventWindowPosChanged     |     |     |     |     |       |     | No parameters; fired to the current focus control after the window position changes |
|kEventWindowCreate         |     |     |     |     |       |     | No parameters; fired after the drop-down window of Combo/ComboButton/CheckCombo is created |
|kEventWindowClose          |0: normal close <br> 1: cancel close|     |     |     |       |     |     |
|kEventSelect               |ListBox/Combo: <br>New selected index | ListBox/Combo: <br>Old selected index|     |     |       |     | Other classes have no parameters |
|kEventUnSelect             |ListBox: <br>New selected index | ListBox:<br>Old selected index|     |     |       |     |  Other classes have no parameters   |
|kEventCheck                |     |     |     |     |       |     | No parameters |
|kEventUnCheck              |     |     |     |     |       |     | No parameters |
|kEventTabSelect            |New selected index | Old selected index|     |     |    |     |    |
|kEventTabClose             |Tab index|     |     |     |       |     | TabCtrl: fired when the close button is clicked or a tab is middle-clicked; only sends the close request and does not remove the tab (removal is done by the business layer in the callback) |
|kEventTabNew               |     |     |     |     |       |     | TabCtrl: fired when the "new tab" button is clicked; the tab is not created automatically |
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
|kEventAutoCompleteSelect  |Selected item index within the current candidate list (0-based) |     |Client-area coordinates of the mouse |     | Key flags |     | Fired after a candidate in AutoComplete is chosen (mouse click or Enter); inline XML event name is `auto_complete_select` |


| Event (eventType)           | Parameter (wParam)  | Parameter (lParam) | Parameter (ptMouse) | Parameter (vkCode) | Parameter (modifierKey) |Parameter (eventData) | Note     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventZoom                 | Scaling ratio numerator [0,64] | Scaling ratio denominator (0,64] |     |     |       |     | RichEdit: Ctrl + wheel: zoom function|
|kEventTextChanged          |     |     |     |     |       |     | No parameters |
|kEventSelChanged           |     |     |     |     |       |     | No parameters |
|kEventReturn               |     |     |     |     |       |     | No parameters    |
|kEventTab                  |     |     |     |     |       |     | No parameters   |
|kEventEsc                  |     |     |     |     |       |     | No parameters; fired when input controls such as RichEdit and AddressBar receive ESC |
|kEventLinkClick            | DString.c_str()<br> URL string    |     |     |     |       |     |     |
|kEventScrollPosChanged     | 0: cy unchanged<br> 1: cy changed   | 0: cx unchanged<br> 1: cx changed    |     |     |       |     |     |
|kEventValueChanged         |New value | Old value |     |     |       |     | Slider: fired when the value changes |
|kEventProgressCompleted    |     |     |     |     |       |     | Progress: fired once when the value reaches the completion endpoint (max forward, min countdown); no parameters |
|kEventPosChanged           |     |     |     |     |       |     | No parameters |
|kEventSizeChanged          |     |     |     |     |       |     | No parameters |
|kEventVisibleChanged       |New state (1=visible, 0=invisible)|     |     |     |       |     | No parameters |
|kEventStateChanged         | New state | Old state   |     |     |       |     | ControlStateType |
|kEventSelectColor          | Selected color |     |     |     |       |     | newColor.GetARGB() |
|kEventViewModeChanged      | New view mode:<br>0=month view<br>1=year view<br>2=decade view | Old view mode |     |     |       |     | Calendar: fired when the view mode changes |
|kEventDisplayDateChanged   |     |     |     |     |       |     | No parameters; Calendar: fired when the displayed date (year/month) changes |
|kEventTreeGridCheckChanged | TreeGridNode*<br>node whose check state changed | 0 (reserved) |     |     |       | TreeGridCheckValue*:<br>node and new check state | TreeGrid: fired when a node's check state changes |
|kEventPopoverOpenChanged   | 1=opened<br>0=closed | PopoverCloseReason:<br>reason for closing |     |     |       |     | Popover: fired when the popover opens or closes |
|kEventPopconfirmConfirm     |     |     |     |     |       |     | No parameters; Popconfirm: fired when the confirm button is clicked |
|kEventPopconfirmCancel      |     |     |     |     |       |     | No parameters; Popconfirm: fired when the cancel button is clicked |
|kEventSplitDraged          | Control*: <br>First control interface| Control*:<br>Second control interface|     |     |       |     |  May be nullptr  |
|kEventEnterEdit            | ListCtrlEditParam*:<br>Data entering edit state  |     |     |     |       |     | ListCtrl cell editing: fired when entering edit state (row-mode click-to-edit, or F2 in kCell mode which enters editing immediately); pEventData points to ListCtrlEditParam (row index / column ID / original text, etc.); the callback may cancel to prevent entering edit state; this event is also fired before `kEditCombo` pops up its candidate flyout (it may likewise be cancelled in the callback to prevent entering edit state) |
|kEventLeaveEdit            | ListCtrlEditParam*:<br>Data leaving edit state    |     |     |     |       |     | ListCtrl cell editing: fired when leaving edit state (Enter commit / focus-loss commit / Esc cancel); pEventData points to ListCtrlEditParam; on commit, invalid input (number/date columns) is discarded — no write-back and this event is NOT fired; after a successful Enter commit in kCell mode the current cell moves down one row, whereas in kRow mode committing a candidate neither moves the selected row nor sets a current cell nor scrolls the view (check msg.bCancelled in the callback to know whether it was cancelled) |
|kEventColumnWidthChanged   | Column ID (size_t)  | Related column ID (size_t),<br>may be Box::InvalidIndex |     |     |       |     | ListCtrl: a column width changed (triggered by dragging the header splitter, double-click auto-fit, or SetColumnWidth) |
|kEventColumnOrderChanged   |     |     |     |     |       |     | ListCtrl: the column display order changed (triggered when dragging the header to reorder columns); iterate with GetColumnId to get the new order |
|kEventColumnVisibleChanged | Column ID (size_t)  | New state (1 visible / 0 hidden) |     |     |       |     | ListCtrl: a column's show/hide state changed |
|kEventColumnSorted         | Column ID (size_t)  | New state (1 ascending / 0 descending) |     |     |       |     | ListCtrl: a column's sort state changed |
|kEventCurrentCellChanged   | Row display index (size_t) | Column ID (size_t) |     |     |       |     | ListCtrl: the current cell changed (cell selection mode) |
|kEventCellSelectionChanged | Number of selected areas (size_t) | 0 (reserved) |     |     |       | std::vector<ui::ListCtrlCellRange>*:<br>the list of selected areas (possibly multiple; nullptr when there is no selection) | ListCtrl: the cell selection area changed (a list of areas is used because discontiguous selection is supported; the pointer is nullptr and wParam is 0 when there is no valid selection) |
|kEventElementFilled        | Data item index<br>(0-based) | UI element index<br>(0-based) |     |     |       | Control*:<br>the filled element control | VirtualListBox: fired when an element has been filled; does not bubble to parent controls |
|kEventDataItemCountChanged | New data item count | Old data item count |     |     |       |     | ListCtrl: fired when the number of data items changes |
|kEventItemMouseEnter       | Data item index<br>(Box::InvalidIndex when invalid) | 0 (reserved) |Client-area coordinates<br> of the mouse position |     | Key flag |     | ListBox: fired when the mouse enters a data item; the sender is the ListBox |
|kEventItemMouseLeave       | Data item index<br>(Box::InvalidIndex when invalid) | 0 (reserved) |Client-area coordinates<br> of the mouse position |     | Key flag |     | ListBox: fired when the mouse leaves a data item; the sender is the ListBox |
|kEventSubItemMouseEnter    | Row (ListBox) index | Sub-item data index |Client-area coordinates<br> of the mouse position |     | Key flag | ListCtrlSubItem*:<br>the sub-item control | ListCtrl (Report view): fired when the mouse enters a sub-item |
|kEventSubItemMouseLeave    | Row (ListBox) index | Sub-item data index |Client-area coordinates<br> of the mouse position |     | Key flag | ListCtrlSubItem*:<br>the sub-item control | ListCtrl (Report view): fired when the mouse leaves a sub-item |
|kEventReportViewItemFilled | ListBox index | Element index |     |     |       | ListCtrlItem*:<br>nullptr when a whole row is filled | ListCtrl (Report view): fired when a whole row or a single cell has been filled |
|kEventListViewItemFilled   | ListBox index | Data item index |     |     |       | ListCtrlListViewItem*:<br>the filled list item | ListCtrl (ListView view): fired when a list item has been filled |
|kEventIconViewItemFilled   | ListBox index | Data item index |     |     |       | ListCtrlIconViewItem*:<br>the filled icon item | ListCtrl (IconView view): fired when an icon item has been filled |
|kEventViewTypeChanged      | New view type | Old view type |     |     |       | Control*:<br>the current view pointer | ListCtrl: fired when the view type is switched |
|kEventViewPosChanged       |     |     |     |     |       | Control*:<br>the current view pointer | ListCtrl: fired when the view position changes |
|kEventViewSizeChanged      |     |     |     |     |       | Control*:<br>the current view pointer | ListCtrl: fired when the view size changes |

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
|kEventNone                 |     |     |     |     |       |     | No parameters; invalid event value (internal boundary value) |
|kEventKeyBegin             |     |     |     |     |       |     | No parameters; start of the key event range (internal boundary value) |
|kEventKeyEnd               |     |     |     |     |       |     | No parameters; end of the key event range (internal boundary value) |
|kEventMouseBegin           |     |     |     |     |       |     | No parameters; start of the mouse event range (internal boundary value) |
|kEventMouseEnd             |     |     |     |     |       |     | No parameters; end of the mouse event range (internal boundary value) |
|kEventLast                 |     |     |     |     |       |     |  No parameters   |
