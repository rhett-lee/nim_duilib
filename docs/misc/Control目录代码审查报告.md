# duilib/Control 目录代码审查报告

本报告记录对 `duilib/Control` 目录代码的审查结论与修复情况。审查遵循“只做必要的修改”原则：确认是缺陷的做最小修复；改动风险大于收益、或需要调整对外设计的问题只记录建议，不改代码。

## 一、审查范围

- 目录：`duilib/Control`（全部控件，约 90 个文件）。
- 分组审查：
  - 基础控件：`Button`、`CheckBox`、`Option`、`HyperLink`、`Label`、`Progress`、`CircleProgress`、`Badge`、`Line`、`Mask`、`IconControl`、`TextDrawer`、`Avatar`、`Rating`、`Switch`、`ColorSlider`、`Marquee`、`ChildWindow` 等；
  - 输入/日期控件：`Combo`、`ComboButton`、`CheckCombo`、`FilterCombo`、`DateTime`、`DateTimeWnd`、`Calendar`、`CalendarFlyout`、`SpinBox`、`Stepper`、`IPAddress`、`HotKey`、`SearchBox`、`TagInput`、`Slider`、`RangeSlider`、`ColorPicker*` 等；
  - 容器/导航控件：`Breadcrumb`、`Carousel`、`CascadingSelect`、`DirectoryTree*`、`Flyout`、`Popover`、`Popconfirm`、`Menu`、`MenuBar`、`NavigationView`、`Pagination`、`StatusBar`、`TabCtrl`、`TreeView`、`TreeGrid`、`Timeline`、`ToolBar`、`ColorControl` 等；
  - 大型数据控件：`ListCtrl*`（数据层、表头、各视图）、`Chart`、`ChatMessageList`、`RichText*`、`RichEditData`。
- 审查维度：内存与生命周期、空指针/悬垂指针、整数溢出与除零、平台分支差异、事件语义、DPI/坐标换算、属性解析顺序、注释与实现一致性。

## 二、总体结论

`Control` 模块整体实现质量较好：

- XML 属性解析、DPI 缩放、状态颜色、事件派发等公共能力复用充分，控件层次清晰；
- 大多控件对可选数据采用懒分配，空控件开销较小；
- 常用控件的属性设置普遍带有断言与运行时容错。

发现的问题集中在六类：

1. **控件间自动同步逻辑缺少终止条件**（`CheckBox` 两个自动同步属性同时开启时无限递归）；
2. **坐标/尺寸计算错误**（`IconControl` 纵向拉伸、`GroupBox` 折叠图标 y 坐标、`Stepper` 命中测试缺少滚动偏移）；
3. **属性解析顺序敏感**（`Progress` 的 `marquee_elapsed` 写在 `marquee` 之前会被丢弃）；
4. **删除节点/数据后仍被访问**（`TreeGrid`、`DirectoryTree`、`NavigationView` 的悬垂指针）；
5. **通知回调返回值被丢弃/固定返回值**（`Menu::RemoveMenuItem` 恒返回 false、`ListCtrlReportView` 调整控件数量逻辑成为死代码）；
6. **边界与数值处理**（取色缓冲区除零、`RGB2HSL` 归一化错误、全部行隐藏时误触发断言、位图尺寸变化后越界读取）。

## 三、已修复的问题

### 3.1 高（崩溃、栈溢出、明显功能错误）

| 编号 | 文件 | 问题 | 修复 |
| --- | --- | --- | --- |
| CTL-01 | `CheckBox.h` | `auto_check_select` 与 `auto_select_check` 同时为 `true` 时，`PrivateSetSelected` 与 `PrivateSetChecked` 无条件相互调用，任意一次设置选中/勾选都会无限递归导致栈溢出 | 同步前比较目标值（`m_bChecked != m_bSelected`），值已一致时不再回调；同时保留两边的变更结果 |
| CTL-02 | `IconControl.cpp` | 计算目标矩形时 `rcDest.bottom = rcDest.bottom + rcSource.Height()`，图片被纵向拉伸并超出控件 | 改为 `rcDest.bottom = rcDest.top + rcSource.Height()` |
| CTL-03 | `DateTimeWnd.cpp` | 结束编辑时把“秒”写入 `tm_min`，确定后的时间被改错 | 改为 `timeValue.tm_sec = nSec` |
| CTL-04 | `ListCtrlHeader.cpp` | 在中间位置插入列时，表头项与分隔控件按 `columnIndex` 插入，破坏了 `H0,S0,H1,S1...` 的交错结构，后续按 `index*2` 取列会取到分隔控件 | 插入索引改为 `columnIndex * 2` |
| CTL-05 | `ListCtrl.cpp` | `SetDataItemHeight(-1)` 被改为 0，而 -1 是“使用默认行高”的哨兵；0 会被报表视图当作不可见行跳过 | 负数保留为 -1 且不做 DPI 缩放，交由数据层按默认行高处理 |
| CTL-06 | `TreeGrid.cpp` | `RemoveNode` / `RemoveAllNodesOf` 先 `delete` 节点，再通过 `OnNodeStructureChanged` → `RebuildRows` 遍历仍包含已释放节点的 `m_rowNodes`，产生释放后写入 | 删除前先把该节点及其子孙从 `m_rowNodes` 中移除 |
| CTL-07 | `DirectoryTree.cpp` | 异步加载完成时节点已销毁，失败分支仍用悬垂的 `pTreeNode` 调用 `GetFolderData` | 仅在 `treeNodeFlag` 未失效时访问节点 |
| CTL-08 | `ColorConvert.cpp` | `RGB2HSL` 的 `del_R/del_G/del_B` 未除以 `del_Max` 归一化，非原色时色相明显偏移（如 `(0.2,0.6,0.9)` 得到 216°，正确值约 205.7°） | 三个分量统一改为 `(((Max - 通道) / 6.0) + del_Max / 2.0) / del_Max` |
| CTL-09 | `ColorConvert.cpp` | `GetRGB` 只拦截 `samples < 1`，`samples == 1` 时 `(samples - 1)` 整数除零 | `samples == 1` 时直接写入起始颜色（按 DIB 的 BGR 顺序）并返回 |
| CTL-10 | `Menu.cpp` | `RemoveMenuItem` 成功移除后仍固定 `return false`，且在删除后继续按旧数量遍历 | 找到并移除后立即 `return true` |

### 3.2 中（状态错误、越界访问或功能不生效）

| 编号 | 文件 | 问题 | 修复 |
| --- | --- | --- | --- |
| CTL-11 | `Marquee.cpp` | 文本被置空后提前返回，跳过滚动状态判断，定时器继续按间隔重绘，`IsScrolling()` 始终为真 | 空文本分支中停止滚动、取消定时器并复位偏移 |
| CTL-12 | `Progress.cpp` | `SetMarqueeElapsed` 开头要求 `IsMarquee()`，XML 中 `marquee_elapsed` 写在 `marquee` 之前时该值被丢弃 | 先保存配置值，仅在已处于跑马灯状态时才重启定时器 |
| CTL-13 | `ListCtrlData.cpp` | 所有行都隐藏时，`GetDataItemsCheckStatus` / `GetDataItemsSelectStatus` 落入 `else { ASSERT(0); }`，Debug 下合法数据触发断言 | 删除两处断言，两个出参保持默认的 false/false |
| CTL-14 | `ListCtrlReportView.cpp` | `GetMaxDataItemsToShow` 的返回值未接收，`nCalcItemCount` 恒为 0，调整 UI 控件数量的逻辑永不生效 | 接收返回值并赋给 `nCalcItemCount` |
| CTL-15 | `ColorPickerCustom.cpp` | HSV/HSL 输入框文本变化后调用 `OnRGBChanged`，会用尚未刷新的 RGB 值生成颜色事件，可能覆盖用户刚输入的分量 | 分别改为 `OnHSVChanged`、`OnHSLChanged` |
| CTL-16 | `ListCtrlHeaderItem.cpp` | 先 `ClearStateImages()` 再取复选框宽度，宽度恒为 0，隐藏复选框后标题文字左侧缩进不会回退 | 先取宽度，再清除图片资源 |
| CTL-17 | `GroupBox.h` | 无标题文字时折叠图标的 `top` 使用相对坐标，后续按控件原点偏移后图标被画到控件外 | 改为 `rcControl.top + (CalcTitleHeight() - nIconHeight) / 2` |
| CTL-18 | `ChatMessageList.cpp` | 图片尺寸获取失败时仍缓存 `(0,0)`，之后即使图片可用也一直按最大尺寸布局 | 仅在宽高均大于 0 时写入缓存 |
| CTL-19 | `ListCtrlListView.cpp` | 图标控件在虚拟列表中被复用，无图标行未复位固定宽高，沿用上一行图标尺寸导致文本偏移 | 无图标时把固定宽高复位为 0 |
| CTL-20 | `NavigationView.cpp` | `RemoveItem` 可能已销毁控件，之后才调用 `UnregisterNavItem(pItem)` 使用悬垂指针；注释“由调用方决定是否 delete”与实现相反 | 先解除登记再移除控件，并修正注释 |
| CTL-21 | `Carousel.cpp` | `SetInterval` 注释称“重启定时器”，但 `UpdateAutoPlayTimer` 在已启动时直接返回，运行期修改间隔永不生效 | 间隔变化且定时器已启动时，先移除旧定时器再重新启动 |
| CTL-22 | `ColorControl.cpp` | 使用当前控件尺寸计算像素索引，但位图是上次绘制时按旧尺寸生成的；控件被拉伸后重绘前点击会越界读 | 仅当位图宽高与当前尺寸一致时才读取像素 |
| CTL-23 | `Stepper.cpp` | 命中测试直接把窗口坐标 `msg.ptMouse` 与控件坐标 `GetRect()` 比较，位于滚动容器内时点击错位 | `ButtonDown` 与 `MouseMove` 都先补齐 `GetScrollOffsetInScrollBox()` |
| CTL-24 | `IconControl.cpp` | 控件宽或高为 0 时计算宽高比会整数除零 | 绘制前对空目标矩形直接返回 |

### 3.3 低（注释与实现不一致）

| 编号 | 文件 | 问题 | 修复 |
| --- | --- | --- | --- |
| CTL-25 | `Progress.h` | 注释写“默认关闭”，构造函数实际为 `m_bAnimationEnabled(true)` | 注释改为“默认开启” |
| CTL-26 | `Slider.h` | `m_nTickLength` 注释为“已 DPI 缩放”，实现保存的是未缩放原始值 | 注释改为“未做 DPI 缩放的原始值，绘制时按当前 DPI 缩放” |

## 四、核查后未修改的项（建议）

以下问题经复核确认存在，但修复需要调整控件对外约定或涉及较大范围的生命周期设计，本轮仅记录，建议后续单独处理：

1. `Combo` / `CheckCombo` / `ComboButton`：下拉窗口持有宿主裸指针，宿主析构时未关闭下拉窗口（`Combo` 使用弱引用且已做保护，另外两个仍为裸指针）。建议统一改为 `ControlPtrT` 并在宿主析构时关闭弹窗。
2. `DateTime`：析构未关闭已打开的 `CalendarFlyout`，浮层回调以裸 `this` 捕获，控件销毁后关闭浮层可能访问已释放对象。建议改用 `UiBind`/弱标志绑定并在析构中关闭。
3. `DateTime::EndEditDateTime`：派发 `kEventKillFocus` 后继续访问成员；若监听者销毁控件会产生 UAF。建议派发前先取走窗口指针或使用弱标志保护。
4. `TabCtrl`：`m_pTabBox` 为裸指针，`TabBox` 先于 `TabCtrl` 析构时监听移除会解引用已释放对象。建议改为弱引用或在 `kEventDestroy` 中置空。
5. `TabCtrl`：`Selected` / `NotifyTabCloseRequest` 同步派发用户事件后继续使用 `this`，业务层删除标签时存在 UAF 风险。
6. `Calendar`：命中测试的惰性重算只重建单元格数据，未填充 `DayCell::rect`，尚未绘制过时点击可能全部落空；年/十年视图命中网格未扣除内边距，且控件尺寸极小时 `cellWidth/cellHeight` 可能为 0。建议把单元格矩形计算移入 `GetMonthCells`，并复用绘制路径的矩形。
7. `CircleProgress`：非整圆弧的包围盒采样固定取 4 个象限点、半径约束取 `max` 而非 `min`，可能使弧溢出控件；进度指示点旋转角恒多 90°。属于绘制算法调整，建议单独评估。
8. `HyperLink`：`kEventLinkClick` 的 `wParam` 指向局部副本的 `c_str()`，事件返回后即失效（同步使用无问题）。建议改为控件成员或值传递。
9. `TagInput`：`RemoveTagAt` 先派发 `kEventTagRemoved` 再删除，回调中再次删除标签会导致越界。建议增加重入保护。
10. `Progress` / `Rating`：事件回调后直接 `Invalidate()`，回调中销毁控件存在 UAF 风险（框架内 `CheckBox` 已有弱标志保护写法）。建议统一加 `GetWeakFlag().expired()` 判断。
11. `ListCtrlData`：从多选切回单选时未同步 `bSelected` 与 `m_nSelectedRowId`，可能出现“查询有选中、界面无选中”。
12. `ListCtrlReportView`：列被隐藏后，调整列宽仍按“子项序号=列序号”处理，宽度会设置到错误的可见列。建议改用列 ID 匹配。
13. `Menu::CreateMenuWnd`：子菜单 XML 不存在或创建失败时，`Menu` 对象泄漏且 `m_pSubWindow` 非空，之后该菜单项无法再展开子菜单。
14. `ChildWindow`：窗口回调注册可能重复且析构未反注册，重复创建/销毁控件会使回调表持续增长。
15. `CalendarFlyout`：十年视图标题显示 `startYear..startYear+9`，但网格为 12 格（可点击到 `+11`），标题与可点击范围不一致。

## 五、验证情况

- 编译验证：`python build\build.py build --config Debug --arch x64 --offline --target duilib` 通过（`DUILIB_DLL=OFF`）。
- 单元测试：新增
  - `tests/test_color_convert.cpp`（CTest 名称 `color_convert`）：HSV/HSL/RGB 互转、参数校验、往返转换、`GetRGB` 与 `HSV_*`/`HSL_*` 取色缓冲区的写入数量与端点颜色；
  - `tests/test_calendar_date.cpp`（`calendar_date`）：日期比较、闭区间判断、`time_t` 往返、日期字符串解析与格式化；
  - `tests/test_text_drawer.cpp`（`text_drawer`）：富文本标记去除（成对/嵌套/未闭合/混合内容）。
- 三个新增测试已注册到 `tests/CMakeLists.txt`，Debug x64 下编译并运行通过。
- 全量测试：27 个用例中 26 个通过；`richedit2_edit` 在本次环境中超时（未修改 `RichEdit2` 及相关代码，该用例涉及系统剪贴板，非交互会话下可能阻塞），与本次改动无关。

## 六、后续建议

1. 对“可实例化但不依赖窗口”的控件属性逻辑继续补充测试（如 `Progress` 的百分比计算、`Badge` 的计数文本、`Rating` 的命中换算），可在构造时传入空窗口实现。
2. 平台相关逻辑（目录树挂载点解析、路径规范化）建议抽为独立函数，便于在无窗口环境下测试。
3. 下拉弹窗、浮层、TabBox 等跨控件引用建议统一采用弱引用 + 生命周期事件，减少悬垂指针类问题。
4. 绘制算法类问题（`CircleProgress` 弧线、`Calendar` 网格命中）建议补充离屏渲染用例后再调整。
