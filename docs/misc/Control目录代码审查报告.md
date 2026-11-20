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

### 3.4 生命周期与绘制边界问题（本轮补充修复）

| 编号 | 文件 | 问题 | 修复 |
| --- | --- | --- | --- |
| CTL-27 | `Combo.cpp`、`CheckCombo.*`、`ComboButton.*` | 下拉窗口持有宿主裸指针，宿主析构时未关闭下拉窗口，窗口延迟关闭时可能访问已释放对象 | `CheckCombo`、`ComboButton` 的下拉窗口宿主改为 `ControlPtrT` 弱引用，所有使用处判空；宿主成员改为 `ControlPtrT`；三个控件析构时先通过新增的 `CloseComboWndOnOwnerDestroy()` 解除子控件关联并关闭弹窗，不再回调宿主 |
| CTL-28 | `DateTime.cpp` | 析构未关闭 `CalendarFlyout`，浮层回调以裸 `this` 捕获，控件销毁后关闭浮层会访问已释放对象 | 析构时先 `Dismiss()` 关闭浮层；日期选择、清除、关闭回调统一改为 `this + std::weak_ptr<WeakFlag>` 绑定，回调入口先判断弱标志是否失效 |
| CTL-29 | `DateTime.cpp` | `EndEditDateTime` 派发 `kEventKillFocus` 后继续访问成员，监听者销毁控件时产生 UAF | 派发事件前先取走并复位 `m_pDateWindow`，事件返回后只释放局部指针，不再访问 `this` 成员 |
| CTL-30 | `TabCtrl.*` | `m_pTabBox` 为裸指针，`TabBox` 先于 `TabCtrl` 析构时移除监听会解引用已释放对象 | 改为 `ControlPtrT<TabBox>` 弱引用；`GetTabBox` 返回 `get()`，其余判空逻辑自动适配 |
| CTL-31 | `TabCtrl.cpp` | `SelectItem`/`TabBox::SelectItem` 同步派发用户事件后继续写入 `m_bSyncingTabBox`，业务层删除标签时存在 UAF 风险 | `SyncTabBoxSelection`、`OnBoundTabBoxSelectChanged` 在派发前记录弱标志，事件返回后若控件已销毁则直接返回；`NotifyTabCloseRequest` 派发后无后续成员访问，无需额外处理 |
| CTL-32 | `Calendar.*` | 命中测试的惰性重算未填充 `DayCell::rect`，未绘制时点击全部落空；年/十年视图命中未扣除内边距，尺寸过小时 `cellWidth/cellHeight` 为 0 | 新增 `GetMonthGridRect()`，`GetMonthCells` 统一填充每格矩形；`HitTestDay` 按当前几何实时计算命中矩形并做除零保护；`DrawMonthView` 直接复用 `GetMonthCells` 的矩形；年/十年视图命中先扣除 `GetControlPadding()`，并对宽高与点击范围做保护 |
| CTL-33 | `CircleProgress.cpp` | 非整圆弧包围盒固定采样 4 个象限点，半径约束取 `max`，弧可能溢出控件；指示点旋转角多 90° | 仅采样落在扫过区间内的 90° 倍数角；半径约束改为取宽、高两方向所需半径的较小值；指示点旋转角改为 `fAngle - 270°`（图像初始位于 12 点方向） |
| CTL-34 | `HyperLink.h` | `kEventLinkClick` 的 `wParam` 指向局部副本的 `c_str()`，事件返回后失效 | 改为直接传递控件成员 `m_url.c_str()`，指针在控件存活期间有效 |
| CTL-35 | `TagInput.*` | `RemoveTagAt` 先派发 `kEventTagRemoved` 再删除，回调中再次删除标签会导致越界 | 新增 `m_bRemovingTag` 重入保护，回调内再次删除直接忽略；删除前重新校验索引；`RemoveAllTags` 在删除失败时退出，避免死循环 |
| CTL-36 | `Progress.cpp` | 事件回调后直接 `Invalidate()`，回调中销毁控件存在 UAF 风险 | `SetValue` 在派发值变化/完成事件前记录弱标志，事件返回后若已失效则提前返回；`Rating` 现有实现均为先 `Invalidate` 后派发事件，无需修改 |
| CTL-37 | `ListCtrlData.cpp` | 从多选切回单选时未同步 `bSelected` 与 `m_nSelectedRowId`，出现“查询有选中、界面无选中” | 切换为单选时保留一行选中（优先 `m_nSelectedRowId`），清除其它行选中/勾选并回写 `m_nSelectedRowId` |
| CTL-38 | `ListCtrlReportView.cpp` | 列被隐藏后列宽仍按“子项序号=列序号”设置，宽度作用到错误的可见列 | 根据表头可见列建立“列序号→子项序号”映射，按映射设置子项宽度；隐藏列直接跳过 |
| CTL-39 | `Menu.cpp` | 子菜单 XML 不存在或创建失败时 `Menu` 对象泄漏且 `m_pSubWindow` 残留，之后无法再展开 | `ShowMenu` 后检测 `IsWindow()`，未创建成功则释放对象并清空 `m_pSubWindow` |
| CTL-40 | `ChildWindow.cpp` | 窗口回调可能重复注册，析构未反注册，反复创建/销毁控件使回调表持续增长 | 析构时先 `UnregisterWindowCallbacks(GetWindow())`；`SetWindow` 仅在子窗口已创建时注册回调；`CreateChildWindow` 统一注册 |
| CTL-41 | `CalendarFlyout.cpp` | 十年视图标题显示 `startYear..startYear+9`，网格实际为 12 格可点击到 `+11` | 标题改为 `startYear..startYear+11`，与可点击范围一致 |

## 四、原建议项的复核与处理结果

本节所列 15 项问题已在本轮全部处理，详见 3.4 节的 CTL-27 ~ CTL-41。其中：

- 跨控件裸指针统一改为 `ControlPtrT` 弱引用，并在宿主析构时主动关闭弹窗/浮层；
- 事件派发后的成员访问统一使用弱标志或提前取走局部指针保护；
- 绘制与命中测试的单元格矩形改为同一份计算逻辑，并对除零、极小尺寸、内边距做边界保护；
- 列宽调整改为按“列序号→可见子项序号”映射，避免隐藏列造成错位。

仍有以下事项未修改（非缺陷，属于设计取舍）：

1. `DateTime` 浮层、`Combo` 系列下拉窗口在宿主析构时关闭后，窗口对象的真实销毁仍由消息循环完成（`CloseWnd` 异步）；由于宿主已改为弱引用，延迟销毁期间不会访问已释放对象。
2. `CircleProgress` 非整圆弧的半径约束采用 `fAvailW / fArcW` 口径（相当于在控件内留出一圈安全边距），并非严格的几何最小包围；如需精确贴边，可后续配合离屏渲染用例单独评估。

## 五、验证情况

- 编译验证：`python build\build.py build --config Debug --arch x64 --offline --target duilib` 通过（`DUILIB_DLL=OFF`），本轮修改的 15 个文件均在本次编译范围内。
- 本轮回归：受影响测试目标 `test_calendar_date` 在 Debug x64 下重新编译通过，`ctest -R "calendar_date|calendar_disabled_weekdays"` 2/2 通过。
- 单元测试：新增
  - `tests/test_color_convert.cpp`（CTest 名称 `color_convert`）：HSV/HSL/RGB 互转、参数校验、往返转换、`GetRGB` 与 `HSV_*`/`HSL_*` 取色缓冲区的写入数量与端点颜色；
  - `tests/test_calendar_date.cpp`（`calendar_date`）：日期比较、闭区间判断、`time_t` 往返、日期字符串解析与格式化；
  - `tests/test_text_drawer.cpp`（`text_drawer`）：富文本标记去除（成对/嵌套/未闭合/混合内容）。
- 三个新增测试已注册到 `tests/CMakeLists.txt`，Debug x64 下编译并运行通过。
- 全量测试：27 个用例中 26 个通过；`richedit2_edit` 在本次环境中超时（未修改 `RichEdit2` 及相关代码，该用例涉及系统剪贴板，非交互会话下可能阻塞），与本次改动无关。

## 六、后续建议

1. 对“可实例化但不依赖窗口”的控件属性逻辑继续补充测试（如 `Progress` 的百分比计算、`Badge` 的计数文本、`Rating` 的命中换算），可在构造时传入空窗口实现。
2. 平台相关逻辑（目录树挂载点解析、路径规范化）建议抽为独立函数，便于在无窗口环境下测试。
3. 下拉弹窗、浮层、TabBox 等跨控件引用建议统一采用弱引用 + 生命周期事件，减少悬垂指针类问题（本轮已对 `Combo`/`CheckCombo`/`ComboButton`/`DateTime`/`TabCtrl` 落地）。
4. 绘制算法类改动（`CircleProgress` 弧线、`Calendar` 网格命中）已按几何约束修正，建议后续补充离屏渲染用例做像素级回归。
