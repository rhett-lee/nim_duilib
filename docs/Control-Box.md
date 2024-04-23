## 布局的属性
| 类名称 | 基类名称 |关联头文件| 用途 |
| :--- | :--- |:--- | :--- |
| Layout | |[duilib/Box/Layout.h](../duilib/Box/Layout.h) | 自由布局（浮动布局）|
| HLayout |Layout |[duilib/Box/HLayout.h](../duilib/Box/HLayout.h) | 横向布局|
| VLayout |Layout |[duilib/Box/VLayout.h](../duilib/Box/VLayout.h) | 纵向布局|
| HTileLayout |Layout| [duilib/Box/HTileLayout.h](../duilib/Box/HTileLayout.h) | 横向瓦片布局|
| VTileLayout |Layout| [duilib/Box/VTileLayout.h](../duilib/Box/VTileLayout.h) | 纵向瓦片布局|
| VirtualHLayout |HLayout| [duilib/Box/VirtualHLayout.h](../duilib/Box/VirtualHLayout.h) | 虚表横向布局|
| VirtualVLayout |VLayout| [duilib/Box/VirtualVLayout.h](../duilib/Box/VirtualVLayout.h) | 虚表纵向布局|
| VirtualHTileLayout |HTileLayout| [duilib/Box/VirtualHTileLayout.h](../duilib/Box/VirtualHTileLayout.h) | 虚表横向瓦片布局|
| VirtualVTileLayout |VTileLayout| [duilib/Box/VirtualVTileLayout.h](../duilib/Box/VirtualVTileLayout.h) | 虚表纵向瓦片布局|

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
9. 虚表垂直瓦片布局（VirtualVLayout）：可用属性继承水平瓦片布局的属性

