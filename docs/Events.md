## 控件的事件（EventArgs）说明文档

| 事件(eventType)           | 参数(wParam)  | 参数(lParam) | 参数(ptMouse) | 参数(vkCode) | 参数(modifierKey) |参数(eventData) | 备注     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventKeyDown              |原始值|原始值|     |   关联的按键  |  按键标志  |     |     |
|kEventKeyUp                |原始值|原始值 |     |   关联的按键  |  按键标志  |     |     |
|kEventChar                 |原始值|原始值|     |   关联的按键  |  按键标志  |     |     |
|kEventMouseEnter           |     |     |鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseLeave           |     |     |鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseMove            |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseHover           |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseWheel           |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |  wheelDelta数据 |     |
|kEventMouseButtonDown      |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseButtonUp        |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseDoubleClick     |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseRButtonDown     |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseRButtonUp       |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseRDoubleClick    |原始值|原始值|鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventContextMenu          |     |Control*<br> 鼠标所在位置的控件| 鼠标所在位置<br>的客户区坐标    |     |       |     | 如果用户键入 SHIFT+F10，<br> 则ptMouse为(-1,-1)，lParam为0 |
|kEventClick                |     |     |鼠标所在位置<br>的客户区坐标   | 参数(vkCode)   | 按键标志   | 旧的事件类型  | 参数可能性较多，需要检测  |
|kEventRClick               |     |     |鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     |     |
|kEventMouseClickChanged    |     |     |     |     |       |     | 无参数 |
|kEventMouseClickEsc        |     |     |     |     |       |     | 无参数 |

| 事件(eventType)           | 参数(wParam)  | 参数(lParam) | 参数(ptMouse) | 参数(vkCode) | 参数(modifierKey) |参数(eventData) | 备注     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventSetFocus             |     |     |     |     |       |     | 无参数 |
|kEventKillFocus            |     |Control*<br> 新的焦点控件<br>或者为nullptr |     |     |       |     |     |
|kEventSetCursor            |     |     | 鼠标的<br>客户区坐标   |     |       |     |   |
|kEventImeStartComposition  |     |     |     |     |       |     |无参数|
|kEventImeEndComposition    |     |     |     |     |       |     |无参数|
|kEventWindowKillFocus      |     |     |     |     |       |     |无参数 |
|kEventWindowSize           |     |     |     |     |       |窗口大小变化类型：<br>WindowSizeType|     |
|kEventWindowMove           |     |     | 窗口的<br>左上角坐标    |     |       |     |     |
|kEventWindowClose          |0: 正常关闭 <br> 1: 取消关闭|     |     |     |       |     |     |
|kEventSelect               |ListBox/Combo: <br>新选择的索引号 | ListBox/Combo: <br>旧选项择索引号|     |     |       |     | 其他类无参数 |
|kEventUnSelect             |ListBox: <br>新选择的索引号 | ListBox：<br>旧选项择索引号|     |     |       |     |  其他类无参数   |
|kEventChecked              |     |     |     |     |       |     | 无参数 |
|kEventUnCheck              |     |     |     |     |       |     | 无参数 |
|kEventTabSelect            |新选择的索引号 | 旧选项择索引号|     |     |    |     |    |
|kEventExpand               |     |     |     |     |       |     | 无参数 |
|kEventCollapse             |     |     |     |     |       |     | 无参数 |


| 事件(eventType)           | 参数(wParam)  | 参数(lParam) | 参数(ptMouse) | 参数(vkCode) | 参数(modifierKey) |参数(eventData) | 备注     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventZoom                 | 按缩放比例分子[0,64] | 按缩放比例分母(0,64] |     |     |       |     | RichEdit: Ctrl + 滚轮：缩放功能|
|kEventTextChange           |     |     |     |     |       |     | 无参数 |
|kEventSelChange            |     |     |     |     |       |     | 无参数 |
|kEventReturn               |     |     |     |     |       |     | 无参数    |
|kEventTab                  |     |     |     |     |       |     | 无参数   |
|kEventLinkClick            | DString.c_str()<br> URL字符串    |     |     |     |       |     |     |
|kEventScrollChange         | 0: cy无变化<br> 1: cy有变化   | 0: cx无变化<br> 1: cx有变化    |     |     |       |     |     |
|kEventValueChange          |     |     |     |     |       |     | 无参数 |
|kEventResize               |     |     |     |     |       |     | 无参数 |
|kEventVisibleChange        |     |     |     |     |       |     | 无参数 |
|kEventStateChange          | 新状态 | 旧状态   |     |     |       |     | ControlStateType |
|kEventSelectColor          | 所选颜色 |     |     |     |       |     | newColor.GetARGB() |
|kEventSplitDraged          | Control*: <br>第1个控件接口| Control*:<br>第2个控件接口|     |     |       |     |  可能是nullptr  |
|kEventEnterEdit            | ListCtrlEditParam*:<br>进入编辑状态的数据  |     |     |     |       |     |     |
|kEventLeaveEdit            | ListCtrlEditParam*:<br>离开编辑状态的数据    |     |     |     |       |     |     |
|kEventLast                 |     |     |     |     |       |     |  无参数   |
