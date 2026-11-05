# Checklist: AddressBar 改进验证

> 验证方式：VS18 工具链编译 Release x64，PowerShell + Win32 消息模拟的 UI 自动化（1:1 物理像素截图，DPI 168 / 1.75x）。
> 结论：全部通过。

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
