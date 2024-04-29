## 布局的属性
| 类名称 | 基类名称 |关联头文件| 用途 |
| :--- | :--- |:--- | :--- |
| Layout | |[duilib/Box/Layout.h](../duilib/Box/Layout.h) | 自由布局（浮动布局）|
| HLayout |Layout |[duilib/Box/HLayout.h](../duilib/Box/HLayout.h) | 水平布局|
| VLayout |Layout |[duilib/Box/VLayout.h](../duilib/Box/VLayout.h) | 垂直布局|
| HTileLayout |Layout| [duilib/Box/HTileLayout.h](../duilib/Box/HTileLayout.h) | 水平瓦片布局|
| VTileLayout |Layout| [duilib/Box/VTileLayout.h](../duilib/Box/VTileLayout.h) | 垂直瓦片布局|
| VirtualHLayout |HLayout| [duilib/Box/VirtualHLayout.h](../duilib/Box/VirtualHLayout.h) | 虚表水平布局|
| VirtualVLayout |VLayout| [duilib/Box/VirtualVLayout.h](../duilib/Box/VirtualVLayout.h) | 虚表垂直布局|
| VirtualHTileLayout |HTileLayout| [duilib/Box/VirtualHTileLayout.h](../duilib/Box/VirtualHTileLayout.h) | 虚表水平瓦片布局|
| VirtualVTileLayout |VTileLayout| [duilib/Box/VirtualVTileLayout.h](../duilib/Box/VirtualVTileLayout.h) | 虚表垂直瓦片布局|

1. 自由布局（Layout，浮动布局）
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| child_margin | 0 | int | 子控件之间的额外边距: X 轴方向 和 Y 轴方向同时设置为相同值|
| child_margin_x | 0 | int | 子控件之间的额外边距: X 轴方向 |
| child_margin_y | 0 | int | 子控件之间的额外边距: Y 轴方向|
2. 水平布局（HLayout）：可用属性继承自由布局的属性
3. 垂直布局（VLayout）：可用属性继承自由布局的属性
4. 水平瓦片布局（HTileLayout）：可用属性继承自由布局的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|
5. 垂直瓦片布局（VTileLayout）：可用属性继承自由布局的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|
6. 虚表水平布局（VirtualHLayout）：可用属性继承水平布局的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
7. 虚表垂直布局（VirtualVLayout）：可用属性继承垂直布局的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
8. 虚表水平瓦片布局（VirtualHLayout）：可用属性继承水平瓦片布局的属性
9. 虚表垂直瓦片布局（VirtualVLayout）：可用属性继承垂直瓦片布局的属性

## Box的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| child_margin | 0 | int | 子控件之间的额外边距: X 轴方向 和 Y 轴方向同时设置为相同值|
| child_margin_x | 0 | int | 子控件之间的额外边距: X 轴方向 |
| child_margin_y | 0 | int | 子控件之间的额外边距: Y 轴方向|
| margin | 0,0,0,0 | rect | 外边距,如(2,2,2,2) |
| padding | 0,0,0,0 | rect | 内边距,如(2,2,2,2) |
| mouse_child | true | bool | 本控件的子控件是否可以响应用户操作, true 或者 false|
| drag_out_id |  | int | 设置是否支持拖拽拖出该容器：如果不等于0，支持拖出，否则不支持拖出（拖出到drop_in_id==drag_out_id的容器）|
| drop_in_id |  | int | 设置是否支持拖拽投放进入该容器: 如果不等于0，支持拖入，否则不支持拖入(从drag_out_id==drop_in_id的容器拖入到该容器)|

Box 控件继承了 `Control` 属性，更多可用属性请参考：基类[Control(基础控件)的属性](./Control.md)

## VBox的属性
VBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## HBox的属性
HBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## TabBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| fade_switch | false | bool | 当切换页面时是否使用动画效果, 如(false) |
| selected_id | 0 | int | 默认选中的页面id, 如(0) |

TabBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## VTileBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VTileBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## HTileBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

HTileBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## ScrollBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| vscrollbar | false | bool | 是否使用竖向滚动条,如(true) |
| hscrollbar | false | bool | 是否使用横向滚动条,如(true) |
| vscrollbar_style |  | string | 设置本容器的纵向滚动条的样式 |
| hscrollbar_style |  | string | 设置本容器的横向滚动条的样式 |
| scrollbar_padding | 0,0,0,0 | rect | 滚动条的外边距,可以让滚动条不占满容器,如(2,2,2,2) |
| vscroll_unit | 30 | int | 容器的垂直滚动条滚动步长, 0代表使用默认步长 |
| hscroll_unit | 30 | int | 容器的水平滚动条滚动步长, 0代表使用默认步长 |
| scrollbar_float | true | bool | 容器的滚动条是否悬浮在子控件上面,如(true) |
| vscrollbar_left | false | bool | 容器的滚动条是否在左侧显示 |
| hold_end | false | bool | 是否一直保持显示末尾位置,如(true) |

ScrollBox 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## VScrollBox的属性
VScrollBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

## HScrollBox的属性
HScrollBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

## VTileScrollBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VTileScrollBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

## HTileScrollBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

HTileScrollBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

## ListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| multi_select | false | bool | 是否支持多选 |
| paint_selected_colors | 默认规则 | bool | 多选的时候，是否显示选择背景色 |
| scroll_select | false | bool | 是否随鼠标滚轮的滚动改变选中项设置(此选项仅单选有效) |
| select_next_when_active_removed | | bool | 在移除一个子项后, 如果被移除项是选择项，是否自动选择下一项(此选项仅单选有效) |

ListBox 控件继承了 `ScrollBox` 属性，更多可用属性请参考`ScrollBox`的属性

## VListBox的属性
VListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

## HListBox的属性
HListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

## VTileListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VTileListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

## HTileListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

HTileListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

## VirtualListBox的属性
VirtualListBox 控件继承了 `ListBox` 属性，更多可用属性请参考`ListBox`的属性

## VirtualVListBox的属性
VirtualVListBox 控件继承了 `VirtualListBox` 属性，更多可用属性请参考`VirtualListBox`的属性

## VirtualHListBox的属性
VirtualHListBox 控件继承了 `VirtualListBox` 属性，更多可用属性请参考`VirtualListBox`的属性

## VirtualVTileListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| columns | 0 | int | 如果设置为"auto"，表示自动计算列数，如果为数值，表示设置为固定列数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VirtualVTileListBox 控件继承了 `VirtualListBox` 属性，更多可用属性请参考`VirtualListBox`的属性

## VirtualHTileListBox的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| item_size | 0,0 | size | 子项大小, 该宽度和高度，是包含了控件的外边距和内边距的，比如"100,40"|
| rows | 0 | int | 如果设置为"auto"，表示自动计算行数，如果为数值，表示设置为固定行数|
| scale_down | true | bool | 当控件内容超出边界时，按比例缩小，以使控件内容完全显示在瓦片区域内|

VirtualHTileListBox 控件继承了 `VirtualListBox` 属性，更多可用属性请参考`VirtualListBox`的属性

## BoxDragable的属性
| 属性名称 | 默认值 | 参数类型 | 用途 |
| :--- | :--- | :--- | :--- |
| drag_order | true | bool | 是否支持拖动调整顺序（在同一个容器内） |
| drag_alpha | 216 | int | 设置拖动顺序时，控件的透明度（0 - 255） |
| drag_out | true | bool | 是否支持拖出操纵（在相同窗口的不同容器内） |

BoxDragable 控件继承了 `Box` 属性，更多可用属性请参考`Box`的属性

## HBoxDragable的属性
HBoxDragable 与 BoxDragable是一个模板类实现的，属性请参考`BoxDragable`的属性    
HBoxDragable 控件继承了 `HBox` 属性，更多可用属性请参考`HBox`的属性

## VBoxDragable的属性
VBoxDragable 与 BoxDragable是一个模板类实现的，属性请参考`BoxDragable`的属性    
VBoxDragable 控件继承了 `VBox` 属性，更多可用属性请参考`VBox`的属性

