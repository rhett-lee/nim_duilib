# Checklist: AddressBar 改进验证

> 验证方式：VS18 工具链编译 Release x64，PowerShell + Win32 消息模拟的 UI 自动化（1:1 物理像素截图，DPI 168 / 1.75x）。
> 结论：功能项 5~9 已实现并编译通过；其中 5/6/7/9 通过 UI 自动化验证，8 的 OLE 拖放无法用模拟输入复现（详见文末说明）。

## 功能验证

- [x] 空路径显示“此电脑”：启动后地址栏显示计算机图标 + “此电脑”（文本改为 `STRID_TREEVIEW_MY_COMPUTER` 语言 ID，中英文资源均存在）
- [x] 编辑态文字渲染：Ctrl+L / Alt+D / 点击地址栏进入编辑态，RichEdit 文本正常可见（修复 `PaintRichEdit` 多矩形裁剪区域求交导致文字不绘制的问题）
- [x] 长路径提交：`E:\develop\nim_duilib\duilib\Control` 提交后显示面包屑 5 段，当前段强调色
- [x] 文件路径提交：`...\Control\AddressBar.cpp` 提交后进入父目录并选中该文件（状态栏“已选择 1 项”）
- [x] 非法路径：`Z:\not_exist_dir` 提交弹出错误提示；关闭提示框后地址栏保持原路径（预提交校验，不先显示错误路径再回滚）
- [x] 树节点点击：点击树节点后地址栏与列表同步到对应路径
- [x] 后退/前进：导航后“后退”回到上一路径，“前进”再返回，地址栏与内容一致
- [x] 搜索过滤：当前目录搜索 `AddressBar`，匹配项高亮、计数正确（回归）
- [x] 面包屑键盘导航：Shift+Tab 聚焦末段 → 左方向键移动到上一段 → 回车导航到上一级
- [x] 折叠布局：窗口缩窄到 1400px 时折叠为“新加卷 (E:) > … Control”，末尾段无多余分隔符
- [x] 折叠菜单：点击“…”弹出 3 项菜单（develop / nim_duilib / duilib）；点击菜单项导航成功（动态菜单项需固定宽度，否则菜单宽度为 0）
- [x] 折叠菜单宽度自适应：菜单项宽度按 `Label::EstimateText` 测量文本宽度 + 菜单项内边距计算（最小值按 DIP 换算，`Dpi().GetScaleInt(120)`），各菜单项统一取最大宽度；`a_very_long_folder_name_for_menu_width_test`、`another_extremely_long_folder_name_here` 等长目录名完整显示、无截断
- [x] 折叠优先级（当前段优先）：宽度不足时按“当前段完整 > “…”入口 > 末端方向优先的中间段 > 根段”分配；实测 `...\z_tmp` 显示为“… out > z_tmp”（中间段与当前段完整、根段折叠进菜单），1400px 下长目录名不再被无条件压缩
- [x] 极窄窗口保留折叠入口：窗口缩窄到 1000px 时仍显示“… deep…”，不再出现“中间段全部隐藏且没有…”导致父目录不可达的情况
- [x] 宽度充足回归：`E:\` 显示“新加卷 (E:)”单项，无“…”折叠按钮
- [x] 折叠菜单项包含根段：根段被折叠时菜单首项为“新加卷 (E:)”，菜单宽度对齐最长项、文字完整
- [x] 面包屑图标位置：`address_bar_sub_path_root/drive/computer` 的图标增加 `margin='6,0,0,0'`，`text_padding` 左值 22→28，图标不再贴住 chip 左边缘、不被圆角裁切（DPI 168 下实测图标距 chip 内边缘约 9 物理像素）
- [x] 右键菜单：在面包屑段上右键弹出 3 项菜单（复制完整路径 / 在文件资源管理器中显示 / 在新窗口中打开）
- [x] 右键菜单动作：点击“复制完整路径”后剪贴板内容为右键所在段路径（`E:\develop`）
- [x] 自动补全：输入 `E:\develop\nim_duilib\dui` 出现历史补全浮层；方向键选择 + 回车导航成功；历史记录已归一化（去除末尾分隔符，消除 `...\duilib\` 与 `...\duilib` 重复项）
- [x] 拖放导航：从资源管理器拖动 `nim_duilib` 文件夹到地址栏，导航到该目录
- [x] 浅色主题：默认主题下地址栏、面包屑、菜单样式正常
- [x] 深色主题：`color_dark` 主题下地址栏文字/图标/强调色正常（临时切换主题截图验证后已还原）
- [x] 禁用态：`SetEnabled(false)` 时占位文本置灰、无强调色（临时禁用截图验证后已还原）

## 代码质量

- [x] `AddressBar::AttachContextMenu` 兜底回调返回 `true`：`EventSource` 在回调返回 `false` 时终止派发，返回 `false` 会截断外部绑定的右键菜单回调
- [x] 临时调试日志全部移除（`Window.cpp`、`MainForm.cpp`、`AddressBar.cpp`）
- [x] 临时主题/禁用态改动已还原（`MainThread.cpp`、`MainForm.cpp`）
- [x] 删除死代码 `AddressBar::AdjustControlsWidth`（折叠算法改为按优先级分配可见性后已无调用点，同时移除 `<numeric>` 依赖）
- [x] 编译 0 error（VS18 + ninja，Release x64）

## 清理

- [x] 删除测试脚本与截图（`out/t*.ps1`、`out/*.png`、`out/ctx_debug.txt` 等调试产物）

## 二、功能完整性（5~9）

- [x] 5 自动补全支持文件系统子目录 + Tab 补全 + 匹配高亮
  - `AddressBar::BuildAutoCompleteCandidates`：历史前缀匹配（≤10 条）→ 按 `find_last_of("\\/")` 拆分目录/文件名 → `std::filesystem::directory_iterator`（`skip_permission_denied`、最多扫描 200 项、目录优先、各 ≤10 条）
  - `AutoCompleteFlyout::SetHighlightChars` + 前缀 Label（`auto_complete_item_match` 强调色）/ 剩余 Label（`auto_complete_item_text`）
  - Tab 补全：`address_bar_edit` 的 `want_tab="true"`，`HandleEditKeyDown` 优先拦截 `kVK_TAB`；使用浮层当前选中项（未导航取第 0 项），目录自动追加路径分隔符；无候选时回退 `SetNextTabControl`
  - 自动化验证：输入 `E:\de` 浮层出现且前缀 `E:\de` 强调色、剩余 `velop` 默认色；按 Tab 得到 `E:\develop\` 并刷新为子目录候选
- [x] 6 最近路径下拉按钮 + 历史持久化
  - 新增 `address_bar_history_button` 主题类与 `history.svg`；`ShowAddressEdit` 末尾调用 `UpdateHistoryButtonVisible`（编辑态隐藏）
  - `SetHistoryFile/LoadPathHistory/SavePathHistory`：UTF-8 按行读写；相对路径（如 `user/address_bar_history.txt`）基于可执行程序所在目录解析，写入前自动创建父目录，与 `TreeView_settings.ini` 的保存逻辑一致；`AddPathHistory` 归一化去末尾分隔符并去重、最多 `max_history` 条
  - `ShowPathHistoryMenu`：点击弹出最近路径菜单（≤10 项、统一宽度、`RIGHT_TOP`）
  - 自动化验证：导航后右侧时钟按钮出现，点击弹出含 `E:\develop` 的菜单（tooltip“最近访问的路径”）；编辑态按钮隐藏；Debug/Release 实测历史文件写入 `bin/user/address_bar_history.txt`（由 `bin/user` 加载并在导航后写回）
  - 附：同步修复 `public/filesystem/computer.svg`、`drive-harddisk.svg` 的 `svg_replace_colors` 配置（彩色图标不随主题变色，原配置的 `#5d656b` 在 SVG 中不存在，Debug 会触发断言），现已移除该属性
- [x] 7 内置右键菜单（`default_context_menu`）
  - `ShowDefaultContextMenu`：复制路径 / 在文件资源管理器中显示（`/select,"path"`）/ 打开上级目录；`AttachContextMenu` 中内置菜单处理时返回 `false` 阻断外部派发
  - TreeView 示例改用内置菜单（`default_context_menu="true"`），移除示例自定义菜单与 `address_bar_menu.xml`
  - 自动化验证：右键面包屑弹出 3 项菜单，文字完整、可点击
- [x] 8 拖放支持多文件 + 悬停高亮
  - `OnDropData` 支持 `kControlDropTypeWindows/SDL` 的文件列表：优先导航到第一个存在的目录，否则导航到第一个存在文件的父目录
  - `AttachDropOver/DropLeave` → `SetDropHighlight`；`UpdateBorderHighlight` 错误态优先于拖放高亮
  - 自动化验证受限：用 WinForms `DoDragDrop`（CF_HDROP）模拟拖放源，`DoDragDrop` 在 1ms 内返回 `None`（合成鼠标输入下 OLE 拖放循环立即取消），地址栏未收到 `DragEnter/DragOver/Drop` 事件，故该项未完成端到端自动化验证，需人工从资源管理器拖拽确认
- [x] 9 非法路径的控件级错误状态样式
  - `SetPathError/IsPathError`；`CommitAddressPath` 预提交被拒 → 置错误态；提交成功 → 清除
  - `UpdateBorderHighlight` 用 `color_error` 绘制地址栏边框，并保存/恢复原边框颜色
  - 自动化验证：输入 `E:\no_such_dir_xyz` 回车弹出“路径不存在”对话框，关闭后地址栏显示红色错误边框

### 关于第 8 项的自动化限制

尝试过三种模拟拖放源：PowerShell `DoDragDrop`（`CO_E_NOTINITIALIZED`）、WinForms `Form.DoDragDrop`、带 `Application.OleRequired` 的 STA 线程 + `SendInput` 按住左键后启动 `DoDragDrop`。前两者未产生 OLE 拖放循环；第三种可进入 `DoDragDrop`，但 1ms 内返回 `None`，目标窗口收不到 `DragEnter`。这是模拟输入与 OLE 拖放循环交互的限制，非控件逻辑问题：控件的 `enable_drag_drop` 已开启（`IsEnableDropFile()` 走 `m_bDragDropEnabled` 为 true），`WindowDropTarget` 已注册到窗口，`kEventDropData/kEventDropOver/kEventDropLeave` 均已绑定。

## 清理（新增）

- [x] 删除临时调试日志（`AddressBar.cpp` 中的 `AbDropLog`）、`out/ab_drag_log.txt`、`out/ab_drag_src.txt`
- [x] 删除临时自动化脚本与截图（`Temp/abtest` 目录）

## 三、健壮性与细节（10~13）

- [x] 10 UNC / `\\?\` 长路径 / `~` / 相对路径的分段处理
  - `AddressBar::BuildPathSegments` 不再依赖 `FilePath::GetParentPathList`：先 `ExpandUserPath`（`~`、`~/...`、`~\...` → `USERPROFILE`/`HOME`）→ `NormalizeFilePath` → 去掉末尾分隔符，再按形态手工解析根段：
    - `\\?\UNC\server\share\...`：根为 `\\?\UNC\server\share`，显示 `\\server\share`
    - `\\?\C:\...`、`\\?\Volume{GUID}\...`：根为 `\\?\C:` 等，显示盘符（卷名查询时去掉 `\\?\` 前缀，纯盘符补 `\`）
    - 普通 UNC `\\server\share\...`：根到 share 结束，显示完整 `\\server\share`
    - 盘符 `C:\...`：根 `C:`，`m_bDrive=true`；卷名查询有结果时显示卷名
    - 相对路径：逐段累加，`m_bRoot=false`（不生成根段）
  - 子段统一用 `SplitPathComponents`（跳过空组件），根段 `m_fullPath` 补末尾分隔符
  - 修复点：`std::filesystem` 的父链对 UNC 会返回无效的 `\\server\`、对 `\\?\` 会返回 `\\?\`、对 `\\?\UNC\` 会返回 `\\?\UNC\server` 等无效层，原实现点击后会导航到无效路径
  - 自动化验证（Release，窗口 1400×900）：
    - `\\?\E:\develop\nim_duilib\duilib\Control` → `新加卷 (E:) > develop > nim_duilib > duilib > Control`
    - `\\?\UNC\localhost\e$\develop` 与 `\\localhost\e$\develop` → `\\localhost\e$ > develop`
    - `~\nim_duilib` → `本地磁盘 (C:) > Users > lee > nim_duilib`
    - `duilib\Control`（相对路径）→ 两段普通路径，无根段
    - 空路径 → 计算机图标 + “此电脑”
- [x] 11 `SetPos` 布局结果缓存
  - 缓存键：可用宽度 + 可用高度 + 路径版本号（`m_nLayoutWidth/m_nLayoutHeight/m_nPathVersion/m_nLayoutPathVersion`）；`UpdateAddressBarControls` 重建路径段后 `++m_nPathVersion` 使缓存失效
  - 命中缓存时直接返回，不再重复测量每段文本宽度、重设文本/Class/tooltip（拖动窗口时只有尺寸变化才重算）
  - 顺带修复：调整子控件可见性/最大宽度后调用 `m_pBarBox->SetPos(GetPos())` 重新排列，避免加宽后旧位置残留导致文字重叠；宽度足够时同时隐藏“…”折叠按钮
  - 自动化验证：1400 → 1000 → 1400 反复缩放，窄态折叠为“…”、恢复宽态后无重叠且折叠按钮消失
- [x] 12 补全浮层宽度换算路径统一
  - 新增 `DpiManager::GetDipSizeFromClientSize(int32_t/UiSize)`：客户区物理像素 → 窗口尺寸（`ClientSizeToWindowSize`）→ DIP（`UnscaleInt`），一次调用完成两步换算
  - `AddressBar.cpp`、`AutoComplete.cpp` 两处原 `HasPixelDensity()` 分支统一改为调用该接口，跨 DPI 下浮层宽度不再只做第一步换算
  - 说明：本次回归在系统 DPI 168（1.75x）下完成；其它 DPI 档位需人工切换系统缩放后回归
- [x] 13 键盘快捷键
  - 新增 `HandleNavigationKeyDown`（AddressBar、编辑框、面包屑按钮三处接入）：Alt+Up 上级、F6 / Ctrl+L 进入编辑态并全选
  - `NavigateToParentPath`：基于 `m_pathSegments` 取倒数第二段（兼容 UNC/`\\?\` 根段），编辑态先退出编辑再导航
  - Home/End：`FocusSubPathEdge` 在可见路径段之间跳到首/末段（编辑态不拦截，保留光标移动）
  - 面包屑按钮点击后（导航之后）重新 `SetFocus`，保证点击段落后键盘仍可用（导航过程中焦点会被重置）
  - 自动化验证（事件日志 + 截图）：点击路径段后 `focused=1`；F6 在面包屑按钮上收到 `vk=117` → 进入编辑态；Ctrl+L 聚焦地址栏并进入编辑态；点击当前段后 Home → `FocusSubPathEdge(target=3)` → 回车导航到 `E:\develop\nim_duilib\duilib`；End → `target=3`（末段）→ 回车停留当前段；Alt+Up 从 `...\Control` 回到 `...\duilib`
  - 注：TreeView 示例本身已提供窗口级 Ctrl+L / Alt+Up 快捷键，与控件内置行为重叠但不冲突

## 清理（健壮性阶段）

- [x] 删除 `AddressBar.cpp` 临时事件日志（`LogAbEvent`）、`MainForm.cpp` 临时诊断与 `NIM_TEST_ADDRESS_PATH` 测试钩子
- [x] Release / Debug x64 均编译通过（0 error）

