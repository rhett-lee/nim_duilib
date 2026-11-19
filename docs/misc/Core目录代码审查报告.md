# duilib/Core 目录代码审查报告

本报告记录对 `duilib/Core` 目录代码的审查结论与修复情况。审查遵循“只做必要的修改”原则：确认是缺陷的做最小修复，语义存疑或改动风险大于收益的只记录建议、不改代码。

## 一、审查范围

- 目录：`duilib/Core`（窗口与消息循环、控件基类、绘制/DPI/主题/资源管理等）。
- 重点文件：`WindowBase.cpp`、`Window.cpp`、`ScrollBar.cpp`、`Control*.cpp/h`、`TimerManager.cpp`、`ThreadMessage_*`、`NativeWindow_*`、`WindowDropTarget_Windows.cpp`、`ControlDropTargetImpl_Windows.cpp`、`ToolTip_Windows.cpp`、`Keyboard_SDL.cpp`、`GlobalManager.cpp`、`ThemeManager.cpp`、`WindowBuilder.cpp`、`IconManager.cpp`、`UiString.h`、`UiTypes.h` 及几何/颜色等基础类型。
- 审查维度：内存与资源生命周期、空指针与悬垂指针、整数溢出/位宽、平台分支一致性、DPI 缩放、事件语义、注释与实现一致性。

## 二、总体结论

`Core` 模块整体结构清晰：

- 平台相关代码通过 `NativeWindow_*`、`*_Windows`、`*_SDL`、`*_Linux` 分开实现，接口层（`INativeWindow`、`WindowBase`）职责明确；
- 可选数据普遍采用 `std::unique_ptr` 懒分配，空控件/空窗口开销较小；
- DPI 缩放集中在 `DpiManager`，窗口尺寸与客户区尺寸的换算路径统一；
- 关键入口普遍采用“断言 + 运行时容错”的双保险写法。

发现的问题集中在四类：

1. **生命周期与资源释放**：回调后对象被销毁导致悬垂指针、Win32/COM 资源未按正确 API 释放；
2. **平台分支错误**：SDL/Linux 分支使用了错误的变量或相反的遍历顺序；
3. **语义错误**：事件类型发错、时间单位错误、尺寸计算沿用了错误方向；
4. **健壮性**：可空指针未判空、失败路径未回滚状态、整数乘法溢出。

## 三、已修复的问题

### 3.1 高（崩溃、资源泄漏或明显功能错误）

| 编号 | 文件 | 问题 | 修复 |
| --- | --- | --- | --- |
| CORE-01 | `WindowBase.cpp` | `OnNativeSizeMsg` 的最小化分支发送了 `kWindowMaximizedMsg` | 改为发送 `kWindowMinimizedMsg` |
| CORE-02 | `Window.cpp` | `OnSetCursorMsg` / `OnButtonDown` 派发事件后未校验控件是否已销毁，后续继续使用 `pControl`，存在 use-after-free | 事件前后保存 `controlFlag`，使用前检查 `expired()` |
| CORE-03 | `Window.cpp` | `PreInitWindow` 中根节点不是 `Box` 时直接返回，`pControl` 泄漏 | 返回前 `delete pControl` |
| CORE-04 | `ScrollBar.cpp` | `CalcFastScrollOffset` 对水平滚动条仍按 `Height()` 与 `GetFixedWidth()` 计算，水平快速滚动失效；同时按钮占用尺寸方向错误 | 按 `m_bHorizontal` 分别使用宽度/高度与 `GetFixedWidth()`/`GetFixedHeight()` |
| CORE-05 | `ControlResizable.h` | 上下边框拖拽时用 `xOffset` 判断边界，导致限制逻辑错误 | 改为 `yOffset` |
| CORE-06 | `TimerManager.cpp` | `PostMsg` 最终失败后未回滚 `m_bHasPenddingPoll`，工作线程可能永久等待 | 投递失败时清除挂起标志 |
| CORE-07 | `NativeWindow_Windows.cpp` | `WM_TOUCH` 提前返回分支未释放 `new[]` 分配的 `pInputs` | 返回前 `delete[] pInputs` |
| CORE-08 | `ControlDropTargetImpl_Windows.cpp` | `CF_HDROP` 分支对 `GlobalLock` 得到的指针调用 `DragFinish`，且绕过 `ReleaseStgMedium`，可能破坏堆/泄漏 | 统一使用 `ReleaseStgMedium` 释放 |
| CORE-09 | `WindowDropTarget_Windows.cpp` | `Drop` 在 `pdwEffect` 为空或为 `DROPEFFECT_NONE` 时提前返回，未释放 `IDataObject`、未调用 `DragLeave` | 提前返回前释放数据对象并结束悬停状态 |
| CORE-10 | `ToolTip_Windows.cpp` | `m_ToolTip.lpszText` 指向局部变量 `newText` 的内存；已显示时文本变化不更新；`DestroyToolTip` 为空实现 | 增加成员缓冲 `m_toolTipText`；文本变化时发送 `TTM_UPDATETIPTEXTW`；真正销毁 Tooltip 窗口 |
| CORE-11 | `ThreadMessage_Windows.cpp` / `ThreadMessage_SDL.cpp` | `Initialize` 未复位 `m_bTerm`，对象重启后消息投递立即判定为终止 | 初始化时置 `m_bTerm = false` |
| CORE-12 | `Keyboard_SDL.cpp` | 越界检查使用 `sdlKeycode`，但实际按 `sdlScancode` 索引，可能越界读取 | 检查 `(int)sdlScancode < numkeys` |
| CORE-13 | `ControlLoading.cpp` | 通过 `m_pControl->GetWindow()` 查找动画控件时未判空 | 先判空再调用 |
| CORE-14 | `Control.cpp` | `OnApplyAttributeList` 中 `GetWindow()` 可能为空 | 增加空指针判断 |
| CORE-15 | `ClickThrough_Linux.cpp` | `XQueryTree` 回退分支得到的 Z 序与 EWMH 分支相反，点击穿透判定窗口错误 | 回退分支补 `std::reverse`，统一为“从底层到顶层” |
| CORE-16 | `Shadow.cpp` | 圆角矩形绘制条件重复判断 `borderRound.cy`，`cx` 为 0 时仍按圆角绘制 | 改为 `(cx > 0) && (cy > 0)` |

### 3.2 中（状态错误、平台兼容性或性能问题）

| 编号 | 文件 | 问题 | 修复 |
| --- | --- | --- | --- |
| CORE-17 | `GlobalManager.cpp` | `m_bStartup` 在 `Startup` 成功后从未置 `true`；`Shutdown` 未判断 `m_renderFactory` 是否为空 | 启动成功置位；关闭时判空 |
| CORE-18 | `ScrollBar.cpp` | `ButtonUp` 中直接使用 `m_pOwner`，未判空 | 增加判空 |
| CORE-19 | `ControlLoading.cpp` | 耗时统计使用 `microseconds` 却赋给 `m_nElapsedTimeMs`，数值放大 1000 倍 | 改用 `milliseconds` |
| CORE-20 | `UiString.h` | `string_view` 构造函数使用 `StringCopy`，会按 `c_str()` 读源串，view 未以 `'\0'` 结尾时可能越界 | 改用 `StringNCopy`，按 view 长度复制（内嵌 `'\0'` 仍按 C 字符串语义截断） |
| CORE-21 | `NativeWindow_Windows.cpp` | `WM_HOTKEY` 修饰键解析使用 `else if`，组合键（如 Ctrl+Shift）只识别第一个 | 改为多个独立 `if` |
| CORE-22 | `ThemeManager.cpp` | 切回默认主题时未把 `m_defaultThemeInfo` 回填给 `themeInfo`，返回信息为旧主题 | 回填 `themeInfo = m_defaultThemeInfo` |
| CORE-23 | `WindowBuilder.cpp` | 内存 XML 解析固定使用 `encoding_utf16`，在 `wchar_t` 为 4 字节的平台（Linux/macOS）解析错误 | 改用 `encoding_wchar` 自动适配 |
| CORE-24 | `Control.cpp` | `StopLoading` 无条件 `SetEnabled(true)`，会把原本禁用的控件恢复为可用 | 仅在 `IsLoading()` 为真时恢复 |
| CORE-25 | `CursorManager_SDL.cpp` | `LoadCursorFromData` 的成功/失败路径均未 `UnLockPixelBits` | 各返回路径补解锁 |
| CORE-26 | `NativeWindow_SDL.cpp` | 拖放坐标由 `uint32_t` 强转，负坐标（多屏）会变成极大正数 | 改为 `int32_t` |
| CORE-27 | `IconManager.cpp` | `width * height * 4` 使用 32 位乘法，大图标时溢出导致校验被绕过 | 改为 `int64_t` 计算并校验 |

## 四、核查后未修改的项（建议）

以下问题经复核后认为**当前不构成可复现缺陷**，或修复会改变既有对外语义，故未修改，仅作为后续改进建议：

1. `Callback.h`：`m_weakFlag.reset((WeakFlag*)nullptr)` 之后 `shared_ptr` 仍持有控制块。当前用法下不会导致弱回调误判失效，属于潜在理解陷阱，可考虑显式重置为未初始化状态。
2. `ExpandVarStrings`：变量值自引用时可能递归展开，理论上可构造死循环；当前调用方传入的变量表可控，暂不处理，建议后续增加递归深度限制。
3. `WindowBase`：消息过滤器在遍历过程中增删过滤器可能导致迭代器失效；当前使用场景下未触发，建议后续改为快照遍历。
4. `ZipManager`：完全信任 ZIP 包内声明的解压后大小；对不可信压缩包应增加上限校验。
5. `ZipStreamIO`：32 位偏移在超大文件（>4GB）下可能溢出；当前资源包规模不涉及，建议后续引入 64 位偏移。
6. `UiEstInt`：`value` 为 30 位位域，超过约 10.7 亿的值会被截断；布局尺寸实际远小于该范围，暂不处理。
7. `ThemeGenerator`：解析对 BOM、属性顺序较敏感；属于工具链健壮性问题，不影响运行时。
8. `StateColorMap` 等状态颜色容器：可选状态颜色固定数组化后内存更紧凑，但新增状态时需要同步维护 `kControlStateCount`；属于设计约束，已在注释中说明。

## 五、验证情况

- 编译验证：`python build\build.py build --config Debug --arch x64 --offline --target duilib` 通过。
- 全量构建仅在最后复制 `WebView2Loader.dll` 到 `bin` 时出现 `Permission denied`，原因是目标文件被其他进程占用，与本次代码修改无关。
- 单元测试：新增 `tests/test_ui_geometry.cpp`、`test_ui_string.cpp`、`test_ui_color.cpp`、`test_ui_int_types.cpp`、`test_dpi_manager.cpp`、`test_color_converter.cpp`，覆盖本次涉及的几何类型、字符串、颜色、尺寸类型与颜色转换等纯逻辑代码，注册到 CTest，详见 `tests/CMakeLists.txt`。

## 六、后续建议

1. 为 `Core` 中“可在无窗口环境下实例化”的纯逻辑代码持续补充单元测试（本次已完成基础类型与颜色转换部分）。
2. 平台分支（Windows/SDL/Linux）建议逐步抽出可独立测试的逻辑（如 Z 序、坐标换算、修饰键解析），减少只能靠人工验证的代码。
3. 涉及对象生命周期的回调派发，建议统一采用 `WeakFlag` + `ControlPtrT` 的保护模式，避免再次出现回调后悬垂访问。
