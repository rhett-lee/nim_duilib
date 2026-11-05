# Tasks: AddressBar 改进

> 依赖：`spec.md` 已获用户批准后再开始执行。每完成一项即更新状态。

1. [x] 框架头文件：新增 API/成员/回调类型，移除 clear-btn API（AddressBar.h）
2. [x] 框架实现-提交模型：`CommitAddressPath`、`AttachPathChanging`、子树焦点判定、Set/Get 语义统一、编辑保护（AddressBar.cpp）
3. [x] 框架实现-路径构建：段解析（盘符/UNC/Unix 根统一）、控件复用、卷名强制刷新、空路径“此电脑”显示
4. [x] 框架实现-折叠布局：重写 `UpdateAddressBarControlsStatus`/`AdjustControlsWidth`，末尾无分隔符、当前段强调、截断省略号、“…”下拉菜单
5. [x] 框架实现-交互：面包屑焦点/方向键、复位按钮、右键转发、路径历史 + `AutoCompleteFlyout` 自动补全、拖放导航
6. [x] 主题/图标：`global.xml` 地址栏类、`chevron-right.svg`/`reset.svg`/`more-horizontal.svg`、`address_bar_menu.xml`
7. [x] TreeView XML：`tree_view.xml` 地址栏属性（此电脑、right_padding）+ 新增 `tree_view/menu/address_bar_menu.xml`
8. [x] 语言资源：examples/public 中英文新增键
9. [x] TreeView 集成：预提交校验/文件定位选中、导航开始更新、此电脑、右键菜单、Ctrl+L/Alt+D
10. [x] 编译：duilib + TreeView（Release x64）0 error
11. [x] UI 自动化测试第一轮：普通/长路径/折叠菜单/编辑态复位/非法路径/文件定位/此电脑/键盘/右键/自动补全/拖放/深浅主题，截图核对
12. [x] 修复测试问题并回归（循环至全部通过）
13. [x] 清理测试产物并汇总结果
