简体中文 | [English](Events.en.md)

## 控件的事件（EventArgs）说明文档
控件的事件相关内容在[`duilib/Core/EventArgs.h`](../duilib/Core/EventArgs.h) 和 [`duilib/duilib_defs.h`](../duilib/duilib_defs.h) 文件中定义。

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
|kEventCheck                |     |     |     |     |       |     | 无参数 |
|kEventUnCheck              |     |     |     |     |       |     | 无参数 |
|kEventTabSelect            |新选择的索引号 | 旧选项择索引号|     |     |    |     |    |
|kEventExpand               |     |     |     |     |       |     | 无参数；Panel面板展开完成后触发 |
|kEventCollapse             |     |     |     |     |       |     | 无参数；Panel面板折叠完成后触发 |
|kEventPanelExpanding       |     |     |鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     | Panel面板即将展开时触发，事件回调返回false可取消本次展开 |
|kEventPanelCollapsing      |     |     |鼠标所在位置<br>的客户区坐标   |     | 按键标志   |     | Panel面板即将折叠时触发，事件回调返回false可取消本次折叠 |
|kEventNavigationItemClick  |可选项序号 |     |     |     |       |     | NavigationView点击可选择导航项时触发，重复点击已选中项也会触发 |
|kEventNavigationSelectionChanged |新选择的索引号 | 旧选择的索引号 |     |     |    |     | NavigationView选中项发生变化时触发，无选中时为Box::InvalidIndex |
|kEventNavigationPaneToggling |目标状态：<br>1=收起，0=展开 |     |鼠标所在位置<br>的客户区坐标 |     | 按键标志 |     | NavigationView窗格即将收起/展开时触发，事件回调返回false可取消本次切换 |
|kEventNavigationPaneToggled |当前状态：<br>1=收起，0=展开 |     |     |     |    |     | NavigationView窗格收起/展开完成时触发 |
|kEventTimelineItemClick  |节点序号（从0开始）|     |鼠标所在位置<br>的客户区坐标 |     | 按键标志 |     | Timeline点击节点时触发 |
|kEventTagAdded            |新标签序号（从0开始）|     |鼠标所在位置<br>的客户区坐标 |     | 按键标志 |     | TagInput新增标签后触发 |
|kEventTagRemoved          |被删除标签序号（从0开始）|     |鼠标所在位置<br>的客户区坐标 |     | 按键标志 |     | TagInput删除标签前触发，回调中仍可通过GetTagText读取该标签文本 |
|kEventTagClick            |标签序号（从0开始）|     |鼠标所在位置<br>的客户区坐标   |     | 按键标志 |     | TagInput点击标签正文区域（非关闭按钮）时触发 |
|kEventAutoCompleteSelect  |选中项在当前候选列表中的序号（从0开始）|     |鼠标所在位置<br>的客户区坐标   |     | 按键标志 |     | AutoComplete选中候选项（鼠标点击或键盘回车）后触发，XML内联事件名为auto_complete_select |


| 事件(eventType)           | 参数(wParam)  | 参数(lParam) | 参数(ptMouse) | 参数(vkCode) | 参数(modifierKey) |参数(eventData) | 备注     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventZoom                 | 按缩放比例分子[0,64] | 按缩放比例分母(0,64] |     |     |       |     | RichEdit: Ctrl + 滚轮：缩放功能|
|kEventTextChanged          |     |     |     |     |       |     | 无参数 |
|kEventSelChanged           |     |     |     |     |       |     | 无参数 |
|kEventReturn               |     |     |     |     |       |     | 无参数    |
|kEventTab                  |     |     |     |     |       |     | 无参数   |
|kEventLinkClick            | DString.c_str()<br> URL字符串    |     |     |     |       |     |     |
|kEventScrollPosChanged     | 0: cy无变化<br> 1: cy有变化   | 0: cx无变化<br> 1: cx有变化    |     |     |       |     |     |
|kEventValueChanged         |新值 | 旧值 |     |     |       |     | Slider类：值发生变化时触发 |
|kEventProgressCompleted    |     |     |     |     |       |     | Progress类：进度值到达完成端点（正向到max、倒数到min）时触发一次，无参数 |
|kEventPosChanged           |     |     |     |     |       |     | 无参数 |
|kEventSizeChanged          |     |     |     |     |       |     | 无参数 |
|kEventVisibleChanged       |新状态（1可见/0不可见）|     |     |     |       |     | 无参数 |
|kEventStateChanged         | 新状态 | 旧状态   |     |     |       |     | ControlStateType |
|kEventSelectColor          | 所选颜色 |     |     |     |       |     | newColor.GetARGB() |
|kEventSplitDraged          | Control*: <br>第1个控件接口| Control*:<br>第2个控件接口|     |     |       |     |  可能是nullptr  |
|kEventEnterEdit            | ListCtrlEditParam*:<br>进入编辑状态的数据  |     |     |     |       |     | ListCtrl 单元格编辑：进入编辑状态时派发（行模式单击进入编辑、kCell 模式按 F2 立即进入编辑）；pEventData 指向 ListCtrlEditParam（行索引 / 列ID / 原文本等）；可在此回调中否决（cancel）以阻止进入编辑；kEditCombo 弹出候选浮层前也会派发本事件（同样可在此回调中 cancel 阻止进入编辑） |
|kEventLeaveEdit            | ListCtrlEditParam*:<br>离开编辑状态的数据    |     |     |     |       |     | ListCtrl 单元格编辑：离开编辑状态时派发（Enter 提交 / 失焦提交 / Esc 取消）；pEventData 指向 ListCtrlEditParam；提交时若输入不合法（数值/日期列）则不派发本事件且不写回数据；Enter 提交成功后 kCell 模式当前格下移一行（应用可在事件回调的 bCancelled 上判断是否被取消） |
|kEventColumnWidthChanged   | 列ID(size_t)  | 关联的另一个列ID(size_t)，<br>可能为Box::InvalidIndex |     |     |       |     | ListCtrl: 列的宽度发生变化（拖动表头分割条、双击自适应或调用SetColumnWidth时触发） |
|kEventColumnOrderChanged   |     |     |     |     |       |     | ListCtrl: 列的显示顺序发生变化（拖动表头调整列顺序时触发），可通过GetColumnId遍历获取最新顺序 |
|kEventColumnVisibleChanged | 列ID(size_t)  | 新状态(1显示/0隐藏) |     |     |       |     | ListCtrl: 列的显示/隐藏状态发生变化 |
|kEventColumnSorted         | 列ID(size_t)  | 新状态(1升序/0降序) |     |     |       |     | ListCtrl: 列的排序状态发生变化 |
|kEventCurrentCellChanged   | 行的显示索引(size_t) | 列ID(size_t) |     |     |       |     | ListCtrl: 当前单元格发生变化（单元格选择模式） |
|kEventCellSelectionChanged | 选中区域个数(size_t) | 0（保留） |     |     |       | std::vector<ui::ListCtrlCellRange>*:<br>选中的区域列表（可能多项；无选择时为 nullptr） | ListCtrl: 单元格的选择区域发生变化（支持不连续选择，故为区域列表；无有效选择时该指针为 nullptr，wParam 为 0） |

| 事件(eventType)           | 参数(wParam)  | 参数(lParam) | 参数(ptMouse) | 参数(vkCode) | 参数(modifierKey) |参数(eventData) | 备注     |
| :---                      | :---          | :---         |:---           |:---          |:---               |:---            |:---      |
|kEventPathChanged          |     |     |     |     |       |     |  无参数   |
|kEventPathClick            |     |     |     |     |       |     |  无参数   |
|kEventDropEnter            | ControlDropType|当wParam为kControlDropTypeWindows时，<br>lParam是ControlDropData_Windows的指针|     |     |       |     |  无参数   |
|kEventDropOver             | ControlDropType|当wParam为kControlDropTypeWindows时，<br>lParam是ControlDropData_Windows的指针|     |     |       |     |  无参数   |
|kEventDropLeave            |     |     |     |     |       |     |  无参数   |
|kEventDropData             | ControlDropType | 当wParam为kControlDropTypeWindows时，<br>lParam是ControlDropData_Windows的指针; <br>当wParam为kControlDropTypeSDL时，<br>lParam是ControlDropData_SDL的指针|     |     |       |     |  无参数   |
|kEventImageAnimationStart  | wParam 为数据指针：ui::ImageAnimationStatus*|     |     |     |       |     |  无参数   |
|kEventImageAnimationPlayFrame  | wParam 为数据指针：ui::ImageAnimationStatus*|     |     |     |       |     |  无参数   |
|kEventImageAnimationStop   | wParam 为数据指针：ui::ImageAnimationStatus*|     |     |     |       |     |  无参数   |
|kEventLoadingStart         | wParam 为数据指针：ui::ControlLoadingStatus*|     |     |     |       |     |  无参数   |
|kEventLoading              | wParam 为数据指针：ui::ControlLoadingStatus*|     |     |     |       |     |  无参数   |
|kEventLoadingStop          | wParam 为数据指针：ui::ControlLoadingStatus*|     |     |     |       |     |  无参数   |
|kEventImageLoad            | wParam 为数据指针：ui::ImageDecodeResult*   |     |     |     |       |     |  无参数   |
|kEventImageDecode          | wParam 为数据指针：ui::ImageDecodeResult*   |     |     |     |       |     |  无参数   |
|kEventLast                 |     |     |     |     |       |     |  无参数   |
