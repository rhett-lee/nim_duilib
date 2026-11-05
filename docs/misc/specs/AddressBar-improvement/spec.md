# Spec: AddressBar 改进（框架 + TreeView 集成）

## 1. 背景与目标

用户已确认 [AddressBar改进.md](../../AddressBar改进.md) 中的**全部**改进项，要求逐项实现、编译并以 UI 自动化实测通过。

目标：在不改变 `AddressBar` 外部使用方式的前提下，重构其内部实现，完成文档列出的功能、界面与集成三类改进；TreeView 示例同步适配。

约束：
- 只改仓库内代码/资源；测试产物放 `out/`，测试后删除；不新增说明性文档（本 spec 与 tasks 属于 Spec 流程产物）。
- 源码文件保持 UTF-8 无 BOM；语言资源、XML 保持原有编码。
- 构建目标：`out/x64/Release` 下 `duilib` 与 `TreeView`（VS 18，amd64）。
- 兼容 Linux/macOS 编译：Windows 专有代码继续用 `DUILIB_BUILD_FOR_WIN` 保护。

## 2. 总体设计决策

1. **预提交回调用成员回调，不新增事件枚举**：`AttachPathChanging(std::function<bool(DString& newPath)>)`，允许业务层规范化路径（如文件→父目录），返回 false 时地址栏保持原路径且不派发 `kEventPathChanged`。避免改动 `EventType` 数值与序列化行为。
2. **控件复用**：路径段 HBox 只增不减（多余项隐藏），不再 `RemoveAllItems()` 重建；折叠段使用常驻的“…”按钮 + 懒创建菜单。
3. **折叠布局**（替代按比例压缩）：
   - 宽度足够：全部段可见，最后一段不显示分隔符。
   - 宽度不足：保留根段与末尾段完整（末尾段优先），中间段隐藏并显示“…”按钮；从末尾向前尽量保留更多段；若根+“…+末尾仍放不下，末尾段按剩余宽度截断（Button 自带 `TEXT_END_ELLIPSIS`），根段可隐藏。
   - “…”点击弹出菜单列出被折叠的完整路径，点击菜单项导航（复用 `OnClickedSubPath`）。
4. **焦点判定改为子树判定**：从 `pNewFocus` 向上找父控件，命中 `this` 即视为地址栏内部焦点，不退出编辑/不提交。
5. **编辑保护**：`SetAddressPath` 更新模型与面包屑，但 `m_pRichEdit->IsVisible()` 为真时不写编辑框文本，避免异步加载结果冲掉用户输入。
6. **路径语义统一**：`GetAddressPath()` 返回 `m_addressPath`（最近一次设置/提交的路径）；`GetPreviousAddressPath()` 仅在显示路径真正变化时更新，供兼容回退。
7. **清空按钮改为“恢复当前路径”**：不再给 RichEdit 设置 `clear_btn_class`；AddressBar 自带复位按钮（仅编辑态可见），点击把编辑框恢复到 `GetAddressPath()`。旧 `rich_edit_clear_btn_class` 属性/API 移除。
8. **自动补全复用现有浮层**：直接使用 `AutoCompleteFlyout` + 内部 `AddressBarEdit`（参考 `SearchBox` 的按键拦截方式），候选为路径历史（MRU，去重，最多 20 条），不做目录枚举。
9. **右键菜单通过既有 `kEventContextMenu` 转发**：面包屑按钮右键时记录 `m_clickedAddressPath` 并 `SendEvent(kEventContextMenu)`；具体菜单由 TreeView 提供。
10. **拖放**：AddressBar 启用 `enable_drag_drop`，处理 `kEventDropData`；目录直接提交导航，文件取其父目录提交。

## 3. 精确改动清单

### 3.1 框架：`duilib/Control/AddressBar.h/.cpp`

#### 新增公开类型/接口
```cpp
typedef std::function<bool (DString& newPath)> AddressBarPathChangingEvent;
void AttachPathChanging(const AddressBarPathChangingEvent& callback);
void DetachPathChanging();

void SetRightPadding(int32_t nPadding); int32_t GetRightPadding() const;
void SetEmptyDisplayText(const DString& text); DString GetEmptyDisplayText() const;
void SetEmptyDisplayTextId(const DString& textId);
void SetResetButtonClass(const DString& cls); DString GetResetButtonClass() const;

void SetHistoryEnabled(bool bEnable); bool IsHistoryEnabled() const;
void SetMaxHistory(int32_t nMax); int32_t GetMaxHistory() const;
void AddPathHistory(const DString& path);
std::vector<DString> GetPathHistory() const;
void ClearPathHistory();

void SetEnableDropNavigation(bool bEnable); bool IsEnableDropNavigation() const;
```
- 删除 `SetRichEditClearBtnClass/GetRichEditClearBtnClass`。
- `SetAttribute` 新增：`right_padding`、`empty_display_text`、`empty_display_text_id`、`reset_button_class`、`history_enabled`、`max_history`、`drop_navigation`；删除 `rich_edit_clear_btn_class` 分支。
- 新增 `virtual bool OnKeyDown(const EventArgs& msg) override`（子树内方向键焦点遍历的兜底）。
- `friend class AddressBarEdit;`

#### 成员
`m_pathChangingCallback`、`m_nRightPadding(16)`、`m_emptyDisplayText`、`m_emptyDisplayTextId`、`m_resetBtnClass(_T("address_bar_reset_button"))`、`m_pResetBtn`、`m_pCollapseBox`、`m_pCollapseBtn`、`m_pathSegments`（`struct PathSegment { DString displayName; DString fullPath; bool bRoot; }`）、`m_collapsedPaths`、`m_bEditing`（或直接用 `m_pRichEdit->IsVisible()`）、`m_pathHistory`、`m_nMaxHistory(20)`、`m_bHistoryEnabled(true)`、`m_pFlyout`、`m_candidates`、`m_bUpdatingPopup`、`m_bUserNavigated`、`m_bEnableDropNavigation(true)`。

#### 关键函数改造
- `OnInit()`：
  - 不调用 `m_pRichEdit->SetClass(GetRichEditClearBtnClass())`；改为 `m_pRichEdit->SetAttribute(_T("clear_btn_class"), _T(""))`（确保无内置清除按钮）。
  - 创建 `m_pResetBtn`，`SetClass(GetResetButtonClass())`，`SetToolTipText`（语言键 `STRID_ADDRESS_BAR_RESET_PATH`），点击恢复路径；初始隐藏。
  - 创建 `m_pBarBox` 时使用 `SetPadding(UiPadding(0,0,GetRightPadding(),0), true)`。
  - `SetAttribute(_T("enable_drag_drop"), _T("true"))`，`AttachEvent(kEventDropData, ...)`。
  - 编辑框 `AttachEvent(kEventTextChanged, ...)` 刷新自动补全浮层（仅编辑态）。
- `ShowAddressEdit(bool)`：同步 `m_pResetBtn->SetVisible(bShow)`；进入编辑时先 `SetText(GetAddressPath())` 再聚焦（避免显示旧路径），关闭浮层。
- `AddSubPath` 改为 `EnsureSubPathControls(size_t nIndex)`/`UpdateSubPathControls(...)`：复用已有 HBox；每个 HBox 子项固定为 `[名称Button, 分隔Button]`；绑定点击（用段索引访问 `m_pathSegments`）与右键转发；按钮 `SetTabStop(true)`，不 `SetNoFocus`；分隔按钮 `SetNoFocus()`。
- `UpdateAddressBarControls(const DString& path, bool bForce)`：路径未变且 `!bForce` 才早退；空路径时用 `m_emptyDisplayText` 生成单个不可点击的“当前段”；其余走 `BuildPathSegments`（Windows 盘符/UNC/Unix 根统一处理，根段文字始终设置）；成功后 `m_addressPath = path` 并 `AddPathHistory`（可配置）。
- `UpdateAddressBarControlsStatus()` 重写为第 2 节折叠算法；测量宽度时包含分隔符的 margin/padding；最小宽度保护用 `originalWidths.size()`（修正空 vector 错误）；`AdjustControlsWidth` 仅作为末尾段截断的辅助（修正 `nMinWidth` 判断与预留宽度），默认最小宽度 32（DIP 经 DPI 缩放）。
- 提交统一为 `bool CommitAddressPath(const DString& rawPath, bool bHideEdit)`：
  - Trim → 预提交回调（可修改 path）→ false：编辑框回填旧路径、隐藏编辑、返回 false；
  - true：`UpdateAddressBarControls(path, true)`、更新 `m_showAddressPath/m_addressPath`、记录历史、隐藏编辑、按变化派发 `kEventPathChanged`。
  - `OnAddressBarReturn`/`OnAddressBarKillFocus` 均走该函数；Return 仍先派发 `kEventReturn`。
- `OnAddressBarKillFocus`：先判断 `pNewFocus` 是否在 `this` 子树内（`IsChildControl`），是则直接返回；否则按 `m_bKillFocusUpdateUI` 提交或恢复。
- 折叠菜单：`ShowCollapsedPathMenu()`，`new Menu(GetWindow(), m_pCollapseBtn)`、`SetSkinFolder(DUILIB_PUBLIC_RES_DIR + "/address_bar/")`、`ShowMenu(_T("address_bar_menu.xml"), 屏幕坐标)`，为每个折叠段动态创建 `MenuItem`（`SetClass(_T("menu_element"))`、`SetText(显示名)`、`AttachClick` 导航）。
- 自动补全：`UpdateAutoCompletePopup()`/`OpenPopup()`/`ClosePopup()`/`BuildCandidates()`；`AddressBarEdit` 在 `OnKeyDown` 中拦截 Up/Down（移动选中）、Enter（选中候选后提交）、Esc（先关浮层），其余交回基类。浮层锚点为 `this`，宽度为 `GetPos().Width()`，复用 `auto_complete_*` 皮肤。
- 拖放：从 `ControlDropData_Windows/SDL` 取 `m_fileList`（需 include 对应头文件），路径为目录直接 `SetAddressPath+Commit`，文件取 `FilePath::GetParentPath()` 后提交。

### 3.2 主题与资源

`bin/resources/themes/default/global.xml`（`address_bar` 段）：
- `address_bar`：`height="28"`、`border_round="6,6"`，新增 `right_padding="16"`；移除 `rich_edit_clear_btn_class`，新增 `reset_button_class="address_bar_reset_button"`、`empty_display_text_id=""`、`history_enabled="true"`、`max_history="20"`、`drop_navigation="true"`。
- `address_bar_edit`：新增 `bkcolor="bg_address_bar"`、`text_padding="6,2,6,2"`、`padding="0,0,0,0"`，保持 `text_align="left,vcenter"`。
- `address_bar_sub_path_button`：`text_padding="6,2,6,2"`、`border_round="6,6"`、`font="system_regular_12"`、保留 hovered/pressed 颜色；新增 `address_bar_sub_path_button_current`：`font="system_bold_12"`、`normal_text_color="color_blue"`（不改 hover/pressed 背景）。
- `address_bar_sub_path_root`：改为“图标+文字”（`normal_image` 的 `halign='left'`、12×12/或 16×16），`text_align="left,vcenter"`、`text_padding="20,0,6,0"`；新增 `address_bar_sub_path_drive` 同上但图标用 `public/filesystem/drive-harddisk.svg`。
- `address_bar_path_separator`：改用新图标 `chevron-right.svg`，`width='12' height='12'`，`mouse_enabled="false"`，颜色 `#8A8A8A|border_svg_image`。
- 新增 `address_bar_reset_button`（24×24，`border_round="4,4"`，图标 `reset.svg` 14×14，hover/pressed 背景）与 `address_bar_sub_path_collapse`（“…”文字 or `more-horizontal.svg`，hover 圆角）。

`bin/resources/themes/default/public/address_bar/` 新增：
- `chevron-right.svg`（细 chevron，12×12）
- `reset.svg`（逆时针箭头）
- `more-horizontal.svg`（三点）
- `address_bar_menu.xml`（`MenuListBox class="menu"` 空模板，动态插入菜单项）

`bin/resources/themes/default/tree_view/tree_view.xml`：
- AddressBar 增加 `empty_display_text_id="STRID_TREEVIEW_MY_COMPUTER"`、`right_padding="12"`、`font` 与主题一致；高度保留 28。

`bin/resources/themes/default/tree_view/menu/address_bar_menu.xml` 新增：复制当前路径 / 在文件资源管理器中显示 / 在新窗口中打开（`menu_element` + `Label` + 多语言键）。

语言资源：
- `zh_CN-examples.txt`/`en_US-examples.txt`：`STRID_TREEVIEW_MY_COMPUTER`、`STRID_TREEVIEW_MENU_OPEN_NEW_WINDOW`。
- `zh_CN-public.txt`/`en_US-public.txt`：`STRID_ADDRESS_BAR_RESET_PATH`（“恢复当前路径”/“Restore path”）。

### 3.3 TreeView 集成：`examples/TreeView/MainForm.h/.cpp`

- 新增成员 `ui::FilePath m_pendingLocateFile;` 与 `bool OnAddressBarPathChanging(DString& newPath);`、`void SetAddressBarPathFromTreeNode(ui::TreeNode* pTreeNode);`、`void ShowAddressBarContextMenu();`
- `OnInitWindow`：`AttachPathChanging(...)`；`m_pAddressBar->AttachContextMenu(...)`。
- `OnAddressBarPathChanging(DString& path)`（预提交，不导航）：
  - Trim，空 → false；
  - 相对路径与当前节点路径拼接，改写成绝对路径；
  - 存在文件：`m_pendingLocateFile = path`，`path = 父目录`，返回 true（父目录不存在返回 false 并提示）；
  - 存在目录：true；否则弹“路径不存在”并 false。
- `OnAddressBarPathChanged`：改用 `GetAddressPath()`；失败时不再 `SetAddressPath(previous)` 回滚（预提交已保证合法），仅保留兼容分支。
- `OnShowAddressPath`：目录逻辑保持；若 `m_pendingLocateFile` 的父目录等于输入路径，则 `m_selectedPathInfo[路径] = m_pendingLocateFile` 以在文件列表中选中该文件，并清空 pending。
- 导航开始时更新地址栏：
  - `SelectSubPath(filePath)`：BeginLoading 前 `SetAddressPath(filePath.ToString())`；
  - `ShowUp()`：取父节点路径/计算机节点 → 更新；
  - `ShowBack()/ShowForward()`：出栈节点 → 更新（计算机节点传空串）；
  - 目录树点击：在 `OnInitWindow` 中尝试 `m_pTree->AttachEvent(ui::kEventSelect, ...)`，sender 为 TreeNode 时即时更新；实测若事件不触发，则在现有导航入口（上/后退/前进/地址栏）覆盖并在测试记录中说明。
- `OnShowMyComputerContents`：`SetAddressPath(_T(""))` 保留（由空显示文本渲染“此电脑”）。
- `OnKeyDownMsg`：新增 `Ctrl+L`、`Alt+D` → `m_pAddressBar->SetFocus()`（并 `bHandled=true`）；不干扰现有 Alt+Left/Right/Up。
- `ShowAddressBarContextMenu()`：使用 `new ui::Menu(this, m_pAddressBar)`、`SetSkinFolder(GetResourcePath().ToString())`、`menu/address_bar_menu.xml`、系统光标坐标；右键路径为空（计算机视图）时禁用三项；复制用 `CopyPathsToClipboard({path})`；显示用 `RevealInFileManager(path)`；新窗口用系统文件管理器打开路径（Windows `SystemUtil::ShellExecute`，其他平台降级为 `RevealInFileManager`）。

## 4. 验证方案

1. 构建：
   `Launch-VsDevShell.ps1 -Arch amd64 -HostArch amd64 | Out-Null; cmake --build out\x64\Release --target duilib; cmake --build out\x64\Release --target TreeView`
   要求 0 error（警告不新增）。
2. UI 自动化（PowerShell 5，`SetProcessDpiAwarenessContext(-4)`、`PrintWindow` 截图；启动+操作+截图同一脚本）逐项截图并核对：
   - 普通路径：最后一段无分隔符；当前段加粗/强调色；根段有图标+文字。
   - 长路径：缩小窗口或进入深层目录，出现“…”；点击“…”弹出折叠路径菜单；点击菜单项导航成功。
   - 编辑态：右侧空白点击进入编辑；出现复位按钮；点击复位恢复当前路径；文本框 inset 无跳变。
   - 预提交：输入不存在路径回车 → 立即弹错误提示，面包屑不闪错误路径；输入文件完整路径 → 跳到父目录并选中文件。
   - 计算机视图：地址栏显示“此电脑”，可输入路径跳转。
   - 键盘：Ctrl+L/Alt+D 聚焦；Tab/左右方向键在面包屑间移动焦点。
   - 右键面包屑：菜单出现，复制路径写入剪贴板，显示/新窗口可执行。
   - 自动补全：访问过多个目录后，编辑时输入盘符前缀出现历史下拉，Up/Down/Enter 可用。
   - 拖放：从资源管理器拖目录到地址栏 → 导航到该目录。
   - 主题：切换浅色/深色后地址栏颜色/图标可读；禁用态一致。
3. 回归：TreeView 原有上/后退/前进、搜索、多选、加载提示正常；地址栏点击子路径仍能导航。
4. 清理 `out/t*.ps1`、`out/*.png`、`out/t*test`、`bin/user` 测试残留。

## 5. 验收标准（对应文档全部条目）

- [ ] 长路径折叠 + 省略号 + 下拉展开；`AdjustControlsWidth` 最小宽度/预留宽度错误修复。
- [ ] `AttachPathChanging` 预提交；非法路径无闪动；`GetPreviousAddressPath` 不再被连续失败污染。
- [ ] 编辑态异步 `SetAddressPath` 不覆盖输入。
- [ ] 导航开始即更新地址栏（含树点击，或说明覆盖范围）。
- [ ] 计算机视图显示“此电脑”且可输入跳转。
- [ ] 复用子控件，不再全量重建。
- [ ] 面包屑可聚焦、方向键遍历；Ctrl+L/Alt+D。
- [ ] 焦点子树判定。
- [ ] 折叠下拉、历史/自动补全、右键菜单、拖放、文件路径定位选中、Set/Get 语义统一、卷名刷新、根节点统一显示。
- [ ] 界面 10 项：末尾分隔符移除、细 chevron、当前段强调、hover 圆角、根节点图标+名称、高度/圆角统一、编辑态视觉统一、截断省略号、右侧 padding 可配置、复位按钮替代清空按钮且直接配置 class、禁用态统一。
- [ ] 编译通过；上述 UI 测试全部通过。

## 6. 需要用户确认

- 采用“预提交成员回调（可修改路径）”而非新增事件枚举；
- 面包屑右键菜单的“在新窗口中打开”在示例中通过系统文件管理器打开该目录实现；
- 自动补全只使用路径历史（不枚举磁盘/同目录子项），避免框架层 I/O；
- 允许新增 spec/tasks 两份流程文档（不改动用户已有文档）。
