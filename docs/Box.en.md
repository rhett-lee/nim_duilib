English | [简体中文](Box.md)

> Last synced: 2026-09-29

## I. Basic Concepts of Control, Box (Container) and Layout







### 1. Control    



   The control is the most fundamental component in the GUI library; its class name is Control and its base class is PlaceHolder.    



   The Control contains the following basic attributes:



* Visibility: visible



* Enabled state: enabled



* Position: x,y



* Size: width, height



* Corner radius: border_round



* Border size: border_size



* Border style: border_dash_style



* Border color: border_color



* Alignment: halign/valign



* Margin: margin



* Padding: padding



* Background color: bkcolor



* Background image: bkimage



* Tooltip: tooltip_text



* Cursor style: cursor_type



* Opacity: alpha



* Drag-and-drop operations: enable_drag_drop/enable_drop_file/drop_file_types



* Image animation operations: start_image_animation/stop_image_animation/set_image_animation_frame



* See relevant documentation or source code for other attributes







### 2. Box (Container)    



   The Box (container) is the class responsible for managing the position and size of the child controls contained within it. It includes a series of subclasses (such as HBox/VBox, etc.), and its base class is the Control, so it has all the attributes of a control.    



   Each type of container has a different layout scheme. The layout functionality is implemented by the Layout class and its subclasses. Each Box aggregates a Layout object that is responsible for the concrete layout work.    



   According to different layout schemes, the Box classifications are as follows:    



| Class name (Container) | Base class name | Associated header file | Default layout scheme | Description |



| :---     | :---   |:--- | :--- | :--- |



| Box      | Control|[duilib/Core/Box.h](../duilib/Core/Box.h) | Floating layout (Layout) | Container base class, child controls are arranged by floating layout|



| HBox     | Box|[duilib/Box/HBox.h](../duilib/Box/HBox.h) | Horizontal layout (HLayout) | Child controls are arranged sequentially in the horizontal direction without wrapping|



| VBox     | Box|[duilib/Box/VBox.h](../duilib/Box/VBox.h) | Vertical layout (VLayout) | Child controls are arranged sequentially in the vertical direction without wrapping|



| HFlowBox | Box|[duilib/Box/HBox.h](../duilib/Box/HBox.h) | Horizontal flow layout (HFlowLayout) | Child controls are arranged sequentially in the horizontal direction with automatic wrapping|



| VFlowBox | Box|[duilib/Box/VBox.h](../duilib/Box/VBox.h) | Vertical flow layout (VFlowLayout) | Child controls are arranged sequentially in the vertical direction with automatic wrapping|



| HTileBox | Box|[duilib/Box/TileBox.h](../duilib/Box/TileBox.h) | Horizontal tile layout (HTileLayout) | Container with horizontal tile layout, supports setting the number of rows|



| VTileBox | Box|[duilib/Box/TileBox.h](../duilib/Box/TileBox.h) | Vertical tile layout (VTileLayout) | Container with vertical tile layout, supports setting the number of columns|



| ScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Floating layout (Layout)     | Box container that supports vertical or horizontal scrollbars |



| HScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Horizontal layout (HLayout)   | HBox container that supports vertical or horizontal scrollbars |



| VScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Vertical layout (VLayout)   | VBox container that supports vertical or horizontal scrollbars |



| HFlowScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Horizontal flow layout (HFlowLayout)| HFlowBox container that supports vertical or horizontal scrollbars |



| VFlowScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Vertical flow layout (VFlowLayout)| VFlowBox container that supports vertical or horizontal scrollbars |



| HTileScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Horizontal tile layout (HTileLayout)| HTileBox container that supports vertical or horizontal scrollbars |



| VTileScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | Vertical tile layout (VTileLayout)| VTileBox container that supports vertical or horizontal scrollbars |



| ListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Floating layout (Layout)     | List container that supports vertical or horizontal scrollbars|



| HListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Horizontal layout (HLayout)   | List container that supports vertical or horizontal scrollbars|



| VListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Vertical layout (VLayout)   | List container that supports vertical or horizontal scrollbars |



| HTileListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Horizontal tile layout (HTileLayout)| List container that supports vertical or horizontal scrollbars |



| VTileListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | Vertical tile layout (VTileLayout)| List container that supports vertical or horizontal scrollbars |



| VirtualListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Floating layout (Layout)     | ListBox implemented with virtual list, supports large data volumes and scrollbars |



| VirtualHListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Horizontal layout (HLayout)   | ListBox implemented with virtual list, supports large data volumes and scrollbars |



| VirtualVListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Vertical layout (VLayout)   | ListBox implemented with virtual list, supports large data volumes and scrollbars |



| VirtualHTileListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Horizontal tile layout (HTileLayout)| ListBox implemented with virtual list, supports large data volumes and scrollbars |



| VirtualVTileListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | Vertical tile layout (VTileLayout)| ListBox implemented with virtual list, supports large data volumes and scrollbars |



| TabBox | Box|[duilib/Box/TabBox.h](../duilib/Box/TabBox.h) | Floating layout (Layout) | Page management container; among multiple internal child controls, only one is visible and the others are hidden, can be dynamically switched |



| GridBox | Box|[duilib/Box/GridBox.h](../duilib/Box/GridBox.h) | Grid layout (GridLayout) | Grid layout container, supports cell merging |



| GridScrollBox | ScrollBox|[duilib/Box/GridBox.h](../duilib/Box/GridBox.h) | Grid layout (GridLayout) | Grid layout container, supports cell merging and scrollbars |



| BoxDragable | Box|[duilib/Core/ControlDragable.h](../duilib/Core/ControlDragable.h) | Floating layout (Layout)    | Container that supports drag-in/drag-out operations of child controls |



| HBoxDragable | HBox|[duilib/Core/ControlDragable.h](../duilib/Core/ControlDragable.h) | Horizontal layout (HLayout) | Container that supports drag-in/drag-out operations of child controls |



| VBoxDragable | VBox|[duilib/Core/ControlDragable.h](../duilib/Core/ControlDragable.h) | Vertical layout (VLayout) | Container that supports drag-in/drag-out operations of child controls |







   The Box (container) contains the following basic attributes:    



* Spacing between child controls (X and Y directions): child_margin_x/child_margin_y



* Alignment of child controls (horizontal and vertical): child_halign/child_valign



* Margin: margin



* Padding: padding



* Whether child controls respond to mouse operations: mouse_child



* Whether dragging out of this container is supported: drag_out_id



* Whether drag-and-drop into this container is supported: drop_in_id



* Other attributes: different container types have different attributes







### 3. Layout    



   The Layout is the concrete implementation code responsible for the container's layout scheme; it is aggregated within the Box object and is generally not used directly by the application layer.    



   A layout can inherit from the base class and implement a customized layout scheme in a subclass, by using the `Layout* Box::ResetLayout(Layout* pNewLayout)` function to replace the original container's layout implementation.    



   According to different layout schemes, the Layout classifications are as follows:    



| Class name (Layout) | Base class name | Associated header file | Layout scheme description |



| :--- | :--- |:--- | :--- |



| Layout | |[duilib/Layout/Layout.h](../duilib/Layout/Layout.h) | Floating layout: child controls are independent; each uses its own attributes to set its position and size |



| HLayout |Layout |[duilib/Layout/HLayout.h](../duilib/Layout/HLayout.h) | Horizontal layout: child controls are arranged sequentially in the horizontal direction without wrapping|



| VLayout |Layout |[duilib/Layout/VLayout.h](../duilib/Layout/VLayout.h) | Vertical layout: child controls are arranged sequentially in the vertical direction without wrapping|



| HFlowLayout |Layout |[duilib/Layout/HFlowLayout.h](../duilib/Layout/HFlowLayout.h) | Horizontal layout: child controls are arranged sequentially in the horizontal direction with automatic wrapping|



| VFlowLayout |Layout |[duilib/Layout/VFlowLayout.h](../duilib/Layout/VFlowLayout.h) | Vertical layout: child controls are arranged sequentially in the vertical direction with automatic wrapping|



| HTileLayout |Layout| [duilib/Layout/HTileLayout.h](../duilib/Layout/HTileLayout.h) | Horizontal tile layout, supports setting the number of rows|



| VTileLayout |Layout| [duilib/Layout/VTileLayout.h](../duilib/Layout/VTileLayout.h) | Vertical tile layout, supports setting the number of columns|



| GridLayout  |Layout |[duilib/Layout/GridLayout.h](../duilib/Layout/GridLayout.h) | Grid layout|



| VirtualHLayout |HLayout| [duilib/Layout/VirtualHLayout.h](../duilib/Layout/VirtualHLayout.h) | Virtual horizontal layout, single row|



| VirtualVLayout |VLayout| [duilib/Layout/VirtualVLayout.h](../duilib/Layout/VirtualVLayout.h) | Virtual vertical layout, single column|



| VirtualHTileLayout |HTileLayout| [duilib/Layout/VirtualHTileLayout.h](../duilib/Layout/VirtualHTileLayout.h) | Virtual horizontal tile layout, supports setting the number of rows|



| VirtualVTileLayout |VTileLayout| [duilib/Layout/VirtualVTileLayout.h](../duilib/Layout/VirtualVTileLayout.h) | Virtual vertical tile layout, supports setting the number of columns|







   When a container lays out its child controls, the related child control (which can be a Control, Box, or their subclasses) attributes mainly include:    



* Container's padding: padding



* Attributes of the container's associated Layout: spacing between child controls (horizontal: child_margin_x, vertical: child_margin_y)



* Attributes of the container's associated Layout: content alignment (horizontal: child_halign, vertical: child_valign)



* The child control's own alignment: horizontal (halign), vertical (valign)



* The child control's own margin: margin



* The child control's own cell-merging attribute: row_span attribute, cell merging (spanning rows), only takes effect in GridLayout



* The child control's own cell-merging attribute: col_span attribute, cell merging (spanning columns), only takes effect in GridLayout







## II. Layout Attributes







### 1. Floating Layout (Layout)



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| child_margin   | 0 | int | Spacing between child controls: set the same value for both X and Y axes|



| child_margin_x | 0 | int | Spacing between child controls: X axis direction |



| child_margin_y | 0 | int | Spacing between child controls: Y axis direction |



| child_valign   |   | string | Vertical alignment of child controls, optional values: "top","center","bottom" |



| child_halign   |   | string | Horizontal alignment of child controls, optional values: "left","center","right" |



| child_align    |   | string | Set both horizontal and vertical alignment of child controls at the same time, same function as child_valign and child_halign.<br>Optional values: left, right, hcenter, top, vcenter, bottom, separated by commas, e.g. "hcenter,vcenter" |







### 2. Horizontal Layout (HLayout)



Available attributes inherit those of `Floating Layout (Layout)`.







### 3. Vertical Layout (VLayout)



Available attributes inherit those of `Floating Layout (Layout)`.







### 4. Horizontal Flow Layout (HFlowLayout)



Available attributes inherit those of `Floating Layout (Layout)`.







### 5. Vertical Flow Layout (VFlowLayout)



Available attributes inherit those of `Floating Layout (Layout)`.







### 6. Horizontal Tile Layout (HTileLayout)



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| rows | 0 | int | If set to "auto", the number of rows is calculated automatically; if set to a number, it means a fixed number of rows|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|



| auto_calc_item_size | false | bool | Takes effect when a fixed number of rows is set; automatically calculates the tile height based on the container's total height|







Also, available attributes inherit those of `Floating Layout (Layout)`.







### 7. Vertical Tile Layout (VTileLayout)



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| columns | 0 | int | If set to "auto", the number of columns is calculated automatically; if set to a number, it means a fixed number of columns|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|



| auto_calc_item_size | false | bool | Takes effect when a fixed number of columns is set; automatically calculates the tile width based on the container's total width|







Also, available attributes inherit those of `Floating Layout (Layout)`.







### 8. Grid Layout (GridLayout)



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| rows | 0 | int | Number of grid rows (0 means auto-calculate)|



| columns | 0 | int | Number of grid columns (0 means auto-calculate)|



| grid_width | 0 | int | Grid cell width (0 means auto-calculate) |



| grid_height | 0 | int | Grid cell height (0 means auto-calculate) |



| scale_down | false | bool | Whether to scale down proportionally when the control content exceeds the boundary<br>true  use the child control's size; if it exceeds the grid size, scale down proportionally so the content is fully displayed within the grid<br>false  ignore the child control's own size; the child control's size matches the grid size |







Also, available attributes inherit those of `Floating Layout (Layout)`.







### 9. Virtual Horizontal Layout (VirtualHLayout)



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| auto_calc_item_size | false | bool | Automatically calculate the tile height based on the container's total height|







Also, available attributes inherit those of `Horizontal Layout (HLayout)`.







### 10. Virtual Vertical Layout (VirtualVLayout)



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| auto_calc_item_size | false | bool | Automatically calculate the tile width based on the container's total width|







Also, available attributes inherit those of `Vertical Layout (VLayout)`.







### 11. Virtual Horizontal Tile Layout (VirtualHTileLayout)



Available attributes inherit those of `Horizontal Tile Layout (HTileLayout)`.







### 12. Virtual Vertical Tile Layout (VirtualVTileLayout)



Available attributes inherit those of `Vertical Tile Layout (VTileLayout)`.







## III. Attributes of Various Containers







### 1. Box Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| child_margin   | 0 | int | Layout attribute, spacing between child controls: set the same value for both X and Y axes|



| child_margin_x | 0 | int | Layout attribute, spacing between child controls: X axis direction |



| child_margin_y | 0 | int | Layout attribute, spacing between child controls: Y axis direction |



| child_valign   |   | string | Layout attribute, vertical alignment of child controls, optional values: "top","center","bottom" |



| child_halign   |   | string | Layout attribute, horizontal alignment of child controls, optional values: "left","center","right" |



| child_align    |   | string | Layout attribute, set both horizontal and vertical alignment of child controls at the same time, same function as child_valign and child_halign.<br>Optional values: left, right, hcenter, top, vcenter, bottom, separated by commas, e.g. "hcenter,vcenter" |



| margin | 0,0,0,0 | rect | Margin, e.g. (2,2,2,2) |



| padding | 0,0,0,0 | rect | Padding, e.g. (2,2,2,2) |



| mouse_child | true | bool | Whether child controls support mouse operations, true or false|



| drag_out_id | 0 | int | Whether dragging out of this container is supported: if not 0, dragging out is supported; otherwise it is not (dragged out to a container whose drop_in_id == drag_out_id)|



| drop_in_id | 0 | int | Whether drag-and-drop into this container is supported: if not 0, dragging in is supported; otherwise it is not (dragged in from a container whose drag_out_id == drop_in_id)|







The Box control inherits the `Control` attributes. For more available attributes, please refer to: base class [attributes of Control (basic control)](./Control.en.md).







### 2. VBox Attributes



The VBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 3. HBox Attributes



The HBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 4. VFlowBox Attributes



The VFlowBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 5. HFlowBox Attributes



The HFlowBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 6. VTileBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| columns | 0 | int | If set to "auto", the number of columns is calculated automatically; if set to a number, it means a fixed number of columns|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|







The VTileBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 7. HTileBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| rows | 0 | int | If set to "auto", the number of rows is calculated automatically; if set to a number, it means a fixed number of rows|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|







The HTileBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 8. ScrollBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| vscrollbar | false | bool | Whether to use a vertical scrollbar, e.g. (true) |



| hscrollbar | false | bool | Whether to use a horizontal scrollbar, e.g. (true) |



| vscrollbar_style |  | string | Set the style of the container's vertical scrollbar |



| hscrollbar_style |  | string | Set the style of the container's horizontal scrollbar |



| vscrollbar_class |  | string | Set the Class of the container's vertical scrollbar |



| hscrollbar_class |  | string | Set the Class of the container's horizontal scrollbar |



| scrollbar_padding | "0,0,0,0" | rect | Margin of the scrollbar, allowing the scrollbar not to fill the container, e.g. "2,2,2,2" |



| vscroll_unit | 30 | int | Vertical scroll step of the container, 0 means use the default step |



| hscroll_unit | 30 | int | Horizontal scroll step of the container, 0 means use the default step |



| scrollbar_float | true | bool | Whether the container's scrollbar floats above the child controls, e.g. "true" |



| vscrollbar_left | false | bool | Whether the container's scrollbar is displayed on the left |



| hold_end | false | bool | Whether to always keep displaying the end position, e.g. "true" |







The ScrollBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 9. VScrollBox Attributes



The VScrollBox control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.







### 10. HScrollBox Attributes



The HScrollBox control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.







### 11. VFlowScrollBox Attributes



The VFlowScrollBox control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.







### 12. HFlowScrollBox Attributes



The HFlowScrollBox control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.







### 13. VTileScrollBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| columns | 0 | int | If set to "auto", the number of columns is calculated automatically; if set to a number, it means a fixed number of columns|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|







The VTileScrollBox control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.







### 14. HTileScrollBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| rows | 0 | int | If set to "auto", the number of rows is calculated automatically; if set to a number, it means a fixed number of rows|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|







The HTileScrollBox control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.







### 15. ListBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| multi_select | false | bool | Whether multi-selection is supported |



| paint_selected_colors | Default rule | bool | Whether to display the selection background color when multi-selecting<br>Default rule: if there is a CheckBox, the selection background color is not displayed by default when multi-selecting; in other cases the background color is displayed |



| scroll_select | false | bool | Whether to change the selected item setting with the mouse wheel scrolling (this option is only valid for single selection) |



| select_next_when_active_removed | | bool | After removing a child item, if the removed item is the selected item, whether to automatically select the next item (this option is only valid for single selection) |



| frame_selection | false | bool | Whether to support mouse marquee selection, only effective in multi-select mode |



| frame_selection_color |"#FFAACCEE"| string | Marquee fill color |



| frame_selection_border_size | 1 | int | Marquee border size |



| frame_selection_border_color | "#FF0078D7" | string | Marquee border color |



| frame_selection_alpha | 128 | int | Marquee fill color Alpha value |



| select_none_when_click_blank | true | bool | Whether to cancel the selection when clicking on a blank area (only effective when marquee selection is enabled) |



| select_like_list_ctrl | false | bool | Set the selection mode: similar to ListCtrl (i.e. the way files are operated in Windows Explorer), only valid in multi-select mode |







The ListBox control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.







### 16. VListBox Attributes



The VListBox control inherits the `ListBox` attributes. For more available attributes, please refer to the `ListBox` attributes.







### 17. HListBox Attributes



The HListBox control inherits the `ListBox` attributes. For more available attributes, please refer to the `ListBox` attributes.







### 18. VTileListBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| columns | 0 | int | If set to "auto", the number of columns is calculated automatically; if set to a number, it means a fixed number of columns|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|







The VTileListBox control inherits the `ListBox` attributes. For more available attributes, please refer to the `ListBox` attributes.







### 19. HTileListBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| rows | 0 | int | If set to "auto", the number of rows is calculated automatically; if set to a number, it means a fixed number of rows|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|







The HTileListBox control inherits the `ListBox` attributes. For more available attributes, please refer to the `ListBox` attributes.







### 20. VirtualListBox Attributes



The VirtualListBox control inherits the `ListBox` attributes. For more available attributes, please refer to the `ListBox` attributes.







### 21. VirtualVListBox Attributes



The VirtualVListBox control inherits the `VirtualListBox` attributes. For more available attributes, please refer to the `VirtualListBox` attributes.







### 22. VirtualHListBox Attributes



The VirtualHListBox control inherits the `VirtualListBox` attributes. For more available attributes, please refer to the `VirtualListBox` attributes.







### 23. VirtualVTileListBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| columns | 0 | int | If set to "auto", the number of columns is calculated automatically; if set to a number, it means a fixed number of columns|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|







The VirtualVTileListBox control inherits the `VirtualListBox` attributes. For more available attributes, please refer to the `VirtualListBox` attributes.







### 24. VirtualHTileListBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| item_size | 0,0 | size | Item size; the width and height include the control's margin and padding, e.g. "100,40"|



| rows | 0 | int | If set to "auto", the number of rows is calculated automatically; if set to a number, it means a fixed number of rows|



| scale_down | true | bool | When the control content exceeds the boundary, scale it down proportionally so the content is fully displayed within the tile area|







The VirtualHTileListBox control inherits the `VirtualListBox` attributes. For more available attributes, please refer to the `VirtualListBox` attributes.







### 25. TabBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| selected_id | 0 | int | Default selected page id |



| fade_switch | true | bool | Whether to use animation effect when switching pages, values: "false" or "true" |



| fade_switch_type | "FadeInOut" | string | Page switching animation type, values: "FadeInOut" means fade in/out, "FadeInOutX" means horizontal sliding of the content area |



| fade_switch_frame_interval_ms | 16 | int | Timer interval (milliseconds) for playing the switching animation|



| fade_switch_total_ms | 200 | int | Total playback time (milliseconds) of the switching animation|



| fade_switch_easing_function | "EaseOutCubic" | string | Easing function type of the switching animation |







The TabBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 26. GridBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| rows | 0 | int | Number of grid rows (0 means auto-calculate)|



| columns | 0 | int | Number of grid columns (0 means auto-calculate)|



| grid_width | 0 | int | Grid cell width (0 means auto-calculate) |



| grid_height | 0 | int | Grid cell height (0 means auto-calculate) |



| scale_down | false | bool | Whether to scale down proportionally when the control content exceeds the boundary<br>true  use the child control's size; if it exceeds the grid size, scale down proportionally so the content is fully displayed within the grid<br>false  ignore the child control's own size; the child control's size matches the grid size |







The GridBox control inherits the `Box` attributes. For more available attributes, please refer to the `Box` attributes.







### 27. GridScrollBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| rows | 0 | int | Number of grid rows (0 means auto-calculate)|



| columns | 0 | int | Number of grid columns (0 means auto-calculate)|



| grid_width | 0 | int | Grid cell width (0 means auto-calculate) |



| grid_height | 0 | int | Grid cell height (0 means auto-calculate) |



| scale_down | false | bool | Whether to scale down proportionally when the control content exceeds the boundary<br>true  use the child control's size; if it exceeds the grid size, scale down proportionally so the content is fully displayed within the grid<br>false  ignore the child control's own size; the child control's size matches the grid size |







The GridScrollBox control inherits the `ScrollBox` attributes. For more available attributes, please refer to the `ScrollBox` attributes.







### 28. BoxDragable Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :--- | :--- | :--- | :--- |



| drag_order | true | bool | Whether to support dragging to adjust the order (within the same container) |



| drag_alpha | 216 | int | Opacity of the control when dragging to adjust order (0 - 255) |



| drag_out | true | bool | Whether to support drag-out operation (between different containers in the same window) |







The BoxDragable control inherits `ControlDragableT` and `Box` attributes. For more available attributes, please refer to the `ControlDragableT` and `Box` attributes.







### 29. HBoxDragable Attributes



The HBoxDragable and BoxDragable are implemented by the same template class (ControlDragableT). For attributes, please refer to the `BoxDragable` attributes.    



The HBoxDragable control inherits the `HBox` attributes. For more available attributes, please refer to the `HBox` attributes.







### 30. VBoxDragable Attributes



The VBoxDragable and BoxDragable are implemented by the same template class (ControlDragableT). For attributes, please refer to the `BoxDragable` attributes.    



The VBoxDragable control inherits the `VBox` attributes. For more available attributes, please refer to the `VBox` attributes.







### 31. BoxMovable Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :---     | :---   | :---     | :--- |



| enable_move_pos  | true | bool    | Whether to support dragging to adjust the control's position, enabled by default |



| move_pos_draggable_border |   | UiPadding | The border range of the control's movable rectangle (the surrounding area can be clicked to drag, but the center area cannot be dragged) |



| move_pos_non_draggable_margin |  | UiMargin | The margin of the control's movable rectangle (the surrounding area defined by the margin cannot be clicked to drag, only the center area can be dragged) |



| move_parent_pos  | false| bool    | When performing the drag-to-adjust-position operation, whether to adjust the parent container's position; "true" means adjust the parent container's position, "false" means adjust the control's own position |



| move_pos_alpha   | 216  | uint8_t | Opacity of the control when dragging to adjust position |



| move_pos_reserve_width   | 20  | int | When moving horizontally, the height reserved within the parent container to avoid the control completely overflowing the parent container (not DPI-scaled) |



| move_pos_reserve_height   | 20  | int | When moving vertically, the width reserved within the parent container to avoid the control completely overflowing the parent container (not DPI-scaled) |



| move_pos_keep_within_parent   | false  | bool | When moving the control, ensure the child control is within the parent container without overflow |







The ControlMovable control inherits `ControlMovableT` and `Control` attributes. For more available attributes, please refer to the `ControlMovableT` and `Control` attributes.







### 32. HBoxMovable Attributes



The HBoxMovable control inherits `ControlMovableT` and `HBox` attributes. For more available attributes, please refer to the `ControlMovableT` and `HBox` attributes.







### 33. VBoxMovable Attributes



The VBoxMovable control inherits `ControlMovableT` and `VBox` attributes. For more available attributes, please refer to the `ControlMovableT` and `VBox` attributes.







### 34. BoxResizable Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :---     | :---   | :---     | :--- |



| enable_resize   | true  | bool | Whether to support mouse dragging to change the control's size |



| enable_move_pos  | false | bool    | Whether to support dragging to adjust the control's position, disabled by default; if enabled, related attributes can refer to `ControlMovableT` |



| resize_size_box   | | UiRect | Set the stretch range size when resizing the control on its four edges |



| resize_reserve_width   | 10| int | Set the minimum width reserved when resizing (not DPI-scaled) |



| resize_reserve_height   | 10| int | Set the minimum height reserved when resizing (not DPI-scaled) |



| resize_keep_within_parent| false | bool | When resizing the control, whether to ensure the child control is within the parent container without overflow |







The BoxResizable control inherits `ControlResizableT`, `ControlMovableT` and `Box` attributes. For more available attributes, please refer to the `ControlResizableT`, `ControlMovableT` and `Box` attributes.







### 35. HBoxResizable Attributes



The HBoxResizable control inherits `ControlResizableT` and `HBox` attributes. For more available attributes, please refer to the `ControlResizableT` and `HBox` attributes.







### 36. VBoxResizable Attributes



The VBoxResizable control inherits `ControlResizableT` and `VBox` attributes. For more available attributes, please refer to the `ControlResizableT` and `VBox` attributes.







### 37. XmlBox Attributes



| Attribute name | Default value | Parameter type | Purpose |



| :---     | :---   | :---     | :--- |



| xml_file_path | | string | Set the path of the XML file |



| res_path      | | string | Set the path of the image resources (the resource root directory corresponding to the XML file) |







The XmlBox control inherits `Box` attributes. For more available attributes, please refer to the `Box` attributes.



