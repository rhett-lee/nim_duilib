## 一、控件（Control）、容器（Box）与布局（Layout）的基本概念
### 1. 控件（Control）    
   控件是界面库中的最基础的组件，其类名是Control，其基类为PlaceHolder。    
   控件(Control)包含如下基础属性：
* 可见性：visible
* 启用状态：enabled
* 位置：x,y
* 大小：width, height
* 圆角大小：border_round
* 边框大小：border_size
* 边框样式：border_dash_style
* 边框颜色：border_color
* 对齐方式：halign/valign
* 外边距：margin
* 内边距：padding
* 背景色：bkcolor
* 背景图片：bkimage
* 工具提示：tooltip_text
* 光标样式：cursor_type
* 透明度：alpha
* 拖放操作：enable_drag_drop/enable_drop_file/drop_file_types
* 图片动画操作：start_image_animation/stop_image_animation/set_image_animation_frame
* 其他属性参见相关文档或者源代码
   
### 2. 容器（Box）    
  容器（Box）是负责管理包含在容器中的子控件的位置大小等属性的类，包含一系列的子类（如：HBox/VBox等），其基类是控件（Control），具有控件的一切属性。    
  每种容器有不同的布局方案，其布局功能的实现是由布局（Layout）类及其子类实现的，每个容器（Box）都聚合了一个布局（Layout）对象，负责实现具体的布局工作。    
  按照布局方案不同，容器（Box）的分类如下：    
| 类名称（容器） | 基类名称 |关联头文件| 默认布局方案 | 功能说明 |
| :---     | :---   |:--- | :--- | :--- |
| Box      | Control|[duilib/Core/Box.h](../duilib/Core/Box.h) | 浮动布局（Layout） |
| HBox     | Box|[duilib/Box/HBox.h](../duilib/Box/HBox.h) | 水平布局（HLayout） |
| VBox     | Box|[duilib/Box/VBox.h](../duilib/Box/VBox.h) | 垂直布局（VLayout） |
| HTileBox | Box|[duilib/Box/TileBox.h](../duilib/Box/TileBox.h) | 水平瓦片布局（HTileLayout） |
| VTileBox | Box|[duilib/Box/TileBox.h](../duilib/Box/TileBox.h) | 垂直瓦片布局（VTileLayout） |
| ScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 浮动布局（Layout）     | 带有垂直或水平滚动条的容器 |
| HScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 水平布局（HLayout）   | 带有垂直或水平滚动条的容器 |
| VScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 垂直布局（VLayout）   | 带有垂直或水平滚动条的容器 |
| HTileScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 水平瓦片布局（HTileLayout）| 带有垂直或水平滚动条的容器 |
| VTileScrollBox | Box|[duilib/Box/ScrollBox.h](../duilib/Box/ScrollBox.h) | 垂直瓦片布局（VTileLayout）| 带有垂直或水平滚动条的容器 |
| ListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 浮动布局（Layout）     | 列表容器，支持滚动条 |
| HListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 水平布局（HLayout）   | 列表容器，支持滚动条 |
| VListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 垂直布局（VLayout）   | 列表容器，支持滚动条 |
| HTileListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 水平瓦片布局（HTileLayout）| 列表容器，支持滚动条 |
| VTileListBox | ScrollBox|[duilib/Box/ListBox.h](../duilib/Box/ListBox.h) | 垂直瓦片布局（VTileLayout）| 列表容器，支持滚动条 |
| VirtualListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 浮动布局（Layout）     | 虚表实现的ListBox，支持大数据量，只支持纵向滚动条 |
| VirtualHListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 水平布局（HLayout）   | 虚表实现的ListBox，支持大数据量，只支持纵向滚动条 |
| VirtualVListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 垂直布局（VLayout）   | 虚表实现的ListBox，支持大数据量，只支持纵向滚动条 |
| VirtualHTileListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 水平瓦片布局（HTileLayout）| 虚表实现的ListBox，支持大数据量，只支持纵向滚动条 |
| VirtualVTileListBox | ListBox|[duilib/Box/VirtualListBox.h](../duilib/Box/VirtualListBox.h) | 垂直瓦片布局（VTileLayout）| 虚表实现的ListBox，支持大数据量，只支持纵向滚动条 |
| TabBox | Box|[duilib/Box/TabBox.h](../duilib/Box/TabBox.h) | 浮动布局（Layout） | 页管理容器，内部的多个子控件，只有一个可见，其他是隐藏的，可动态切换 |
| BoxDragable | Box|[duilib/Core/ControlDragable.h](../duilib/Core/ControlDragable.h) | 浮动布局（Layout）    | 支持子控件的拖入/拖出操作的容器 |
| HBoxDragable | HBox|[duilib/Core/ControlDragable.h](../duilib/Core/ControlDragable.h) | 水平布局（HLayout） | 支持子控件的拖入/拖出操作的容器 |
| VBoxDragable | VBox|[duilib/Core/ControlDragable.h](../duilib/Core/ControlDragable.h) | 垂直布局（VLayout） | 支持子控件的拖入/拖出操作的容器 |

容器（Box）包含如下基础属性：    
* 子控件之间的间距: X轴方向/Y轴方向
* 外边距：margin
* 内边距：padding
* 子控件是否可以鼠标操作：mouse_child
* 是否支持拖拽拖出该容器：drag_out_id
* 是否支持拖拽投放进入该容器: drop_in_id
* 其他属性：不同容器类型有不同的属性

### 3. 布局（Layout）    
  布局（Layout）是负责容器布局方案的具体实现代码，聚合在容器（Box）对象中使用，应用层一般不直接使用。    
  布局可以继承基类，在子类中实现个性化的布局方案，通过使用`Layout* Box::ResetLayout(Layout* pNewLayout)`函数来替换原来容器的布局实现。    
  按照布局方案不同，布局（Layout）的分类如下：    
| 类名称（布局） | 基类名称 |关联头文件| 布局方案说明 |
| :--- | :--- |:--- | :--- |
| Layout | |[duilib/Box/Layout.h](../duilib/Box/Layout.h) | 浮动布局：子控件之间无关联，均使用自身的属性设置控件位置和大小 |
| HLayout |Layout |[duilib/Box/HLayout.h](../duilib/Box/HLayout.h) | 水平布局：子控件在水平方向上，依次排列|
| VLayout |Layout |[duilib/Box/VLayout.h](../duilib/Box/VLayout.h) | 垂直布局：子控件在垂直方向上，依次排列|
| HTileLayout |Layout| [duilib/Box/HTileLayout.h](../duilib/Box/HTileLayout.h) | 水平瓦片布局|
| VTileLayout |Layout| [duilib/Box/VTileLayout.h](../duilib/Box/VTileLayout.h) | 垂直瓦片布局|
| VirtualHLayout |HLayout| [duilib/Box/VirtualHLayout.h](../duilib/Box/VirtualHLayout.h) | 虚表水平布局|
| VirtualVLayout |VLayout| [duilib/Box/VirtualVLayout.h](../duilib/Box/VirtualVLayout.h) | 虚表垂直布局|
| VirtualHTileLayout |HTileLayout| [duilib/Box/VirtualHTileLayout.h](../duilib/Box/VirtualHTileLayout.h) | 虚表水平瓦片布局|
| VirtualVTileLayout |VTileLayout| [duilib/Box/VirtualVTileLayout.h](../duilib/Box/VirtualVTileLayout.h) | 虚表垂直瓦片布局|
| ListCtrlReportLayout |Layout| [duilib/Control/ListCtrlReportView.h](../duilib/Control/ListCtrlReportView.h) | ListCtrl控件的Report模式布局（个性化实现）|

## 二、布局（Layout）的属性
### 1. 浮动布局（Layout）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| child_margin | 0 | int | 子控件之间的额外边距: X 轴方向 和 Y 轴方向同时设置为相同值|
| child_margin_x | 0 | int | 子控件之间的额外边距: X 轴方向 |
| child_margin_y | 0 | int | 子控件之间的额外边距: Y 轴方向|

### 2. 水平布局（HLayout）
可用属性继承`浮动布局（Layout）`的属性

### 3. 垂直布局（VLayout）
可用属性继承`浮动布局（Layout）`的属性

### 4. 水平瓦片布局（HTileLayout）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|
| auto_calc_item_size | false | bool | 当设置固定行数时生效，根据容器的总高度自动计算瓦片高度|

同时，可用属性继承`浮动布局（Layout）`的属性

### 5. 垂直瓦片布局（VTileLayout）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|
| auto_calc_item_size | false | bool | 当设置固定列数时生效，根据容器的总宽度自动计算瓦片宽度|

同时，可用属性继承`浮动布局（Layout）`的属性

### 6. 虚表水平布局（VirtualHLayout）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| auto_calc_item_size | false | bool | 根据容器的总高度自动计算瓦片高度|

同时，可用属性继承`水平布局（HLayout）`的属性

### 7. 虚表垂直布局（VirtualVLayout）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| auto_calc_item_size | false | bool | 根据容器的总宽度自动计算瓦片宽度|

同时，可用属性继承`垂直布局（VLayout）`的属性

### 8. 虚表水平瓦片布局（VirtualHTileLayout）
可用属性继承`水平瓦片布局（HTileLayout）`的属性

### 9. 虚表垂直瓦片布局（VirtualVTileLayout）
可用属性继承`垂直瓦片布局（VTileLayout）`的属性

## 三、各种容器的属性
### 1. 容器（Box）的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| child_margin   | 0 | int | 布局属性，子控件之间的额外边距: X 轴方向 和 Y 轴方向同时设置为相同值|
| child_margin_x | 0 | int | 布局属性，子控件之间的额外边距: X 轴方向 |
| child_margin_y | 0 | int | 布局属性，子控件之间的额外边距: Y 轴方向|
| margin | 0,0,0,0 | rect | 外边距,如(2,2,2,2) |
| padding | 0,0,0,0 | rect | 内边距,如(2,2,2,2) |
| mouse_child | true | bool | 子控件是否可以鼠标操作, true 或者 false|
| drag_out_id | 0 | int | 设置是否支持拖拽拖出该容器：如果不等于0，支持拖出，否则不支持拖出（拖出到drop_in_id==drag_out_id的容器）|
| drop_in_id | 0 | int | 设置是否支持拖拽投放进入该容器: 如果不等于0，支持拖入，否则不支持拖入(从drag_out_id==drop_in_id的容器拖入到该容器)|

Box 控件继承了 `Control` 属性，更多可用属性请参考：基类[Control(基础控件)的属性](./Control.md)

### 2. VBox的属性
VBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

### 3. HBox的属性
HBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

### 4. VTileBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VTileBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

### 5. HTileBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

HTileBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

### 6. ScrollBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| vscrollbar | false | bool | 是否使用竖向滚动条,如(true) |
| hscrollbar | false | bool | 是否使用横向滚动条,如(true) |
| vscrollbar_style |  | string | 设置本容器的纵向滚动条的样式 |
| hscrollbar_style |  | string | 设置本容器的横向滚动条的样式 |
| vscrollbar_class |  | string | 设置本容器的纵向滚动条的Class |
| hscrollbar_class |  | string | 设置本容器的横向滚动条的Class |
| scrollbar_padding | 0,0,0,0 | rect | 滚动条的外边距,可以让滚动条不占满容器,如(2,2,2,2) |
| vscroll_unit | 30 | int | 容器的垂直滚动条滚动步长, 0代表使用默认步长 |
| hscroll_unit | 30 | int | 容器的水平滚动条滚动步长, 0代表使用默认步长 |
| scrollbar_float | true | bool | 容器的滚动条是否悬浮在子控件上面,如(true) |
| vscrollbar_left | false | bool | 容器的滚动条是否在左侧显示 |
| hold_end | false | bool | 是否一直保持显示末尾位置,如(true) |

ScrollBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

### 7. VScrollBox的属性
VScrollBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

### 8. HScrollBox的属性
HScrollBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

### 9. VTileScrollBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VTileScrollBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

### 10. HTileScrollBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

HTileScrollBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

### 11. ListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| multi_select | false | bool | 是否支持多选 |
| paint_selected_colors | 默认规则 | bool | 多选的时候，是否显示选择背景色<br>默认规则：如果有CheckBox，多选的时候，默认不显示选择背景色；其他情况下显示背景色 |
| scroll_select | false | bool | 是否随鼠标滚轮的滚动改变选中项设置(此选项仅单选有效) |
| select_next_when_active_removed | | bool | 在移除一个子项后, 如果被移除项是选择项，是否自动选择下一项(此选项仅单选有效) |
| frame_selection | false | bool | 是否支持鼠标框选功能，仅多选模式下生效 |
| frame_selection_color |"#FFAACCEE"| string | 鼠标框选填充颜色 |
| frame_selection_border_size | 1 | int | 鼠标框选边框的大小 |
| frame_selection_border_color | "#FF0078D7" | string | 鼠标框选边框颜色 |
| frame_selection_alpha | 128 | int | 鼠标框选填充颜色的Alpha值 |
| select_none_when_click_blank | true | bool | 设置当鼠标点击空白部分时，是否取消选择(仅当开启鼠标框选功能时有效) |
| select_like_list_ctrl | false | bool | 设置选择模式：与ListCtrl类似（即Windows资源管理器中，对文件操作类似的方式），仅多选模式有效 |

ListBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

### 12. VListBox的属性
VListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

### 13. HListBox的属性
HListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

### 14. VTileListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VTileListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

### 15. HTileListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

HTileListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

### 16. VirtualListBox的属性
VirtualListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

### 17. VirtualVListBox的属性
VirtualVListBox 控件继承了 `VirtualListBox` 属性，更多可用属性请参考`VirtualListBox`的属性

### 18. VirtualHListBox的属性
VirtualHListBox 控件继承了 `VirtualListBox` 属性，更多可用属性请参考`VirtualListBox`的属性

### 19. VirtualVTileListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VirtualVTileListBox 控件继承了 `VirtualListBox` 属性，更多可用属性请参考`VirtualListBox`的属性

### 20. VirtualHTileListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VirtualHTileListBox 控件继承了 `VirtualListBox` 属性，更多可用属性请参考`VirtualListBox`的属性

### 21. TabBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| fade_switch | false | bool | 当切换页面时是否使用动画效果, 取值："false" 或 "true" |
| selected_id | 0 | int | 默认选中的页面id |

TabBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

### 22. BoxDragable的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| drag_order | true | bool | 是否支持拖动调整顺序（在同一个容器内） |
| drag_alpha | 216 | int | 设置拖动顺序时，控件的透明度（0 - 255） |
| drag_out | true | bool | 是否支持拖出操纵（在相同窗口的不同容器内） |

BoxDragable 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

### 23. HBoxDragable的属性
HBoxDragable 与 BoxDragable是一个模板类实现的，属性请参考`BoxDragable`的属性    
HBoxDragable 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

### 24. VBoxDragable的属性
VBoxDragable 与 BoxDragable是一个模板类实现的，属性请参考`BoxDragable`的属性    
VBoxDragable 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

