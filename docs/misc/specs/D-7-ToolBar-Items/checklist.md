# D-7 ToolBar 工具项抽象 验收清单

## 源码

- [x] `AddToolButton` / `AddToolCheckBox` / `AddToolSeparator` / `AddTool` 可用
- [x] `RemoveTool` / `RemoveToolAt` / `RemoveAllTools` 正确销毁控件并清理收纳缓存
- [x] `SetToolChecked` / `IsToolChecked`、`SetToolEnabled` / `IsToolEnabled`、
      `SetToolVisible` / `IsToolVisible` 生效
- [x] 空名称自动生成、重名拒绝、`FindTool` / `GetToolIndex` 正确
- [x] `ToolItemClick` 与 `ToolItemCheckedChanged` 回调参数正确
- [x] 溢出浮层激活走同一套回调，勾选项会切换
- [x] 原有 `OverflowItemActivated` 与静态 XML 行为不变

## 示例

- [x] `toolbar.xml` 有动态工具条与事件回显 Label
- [x] `ToolBarForm` 演示添加/勾选/回调，并能手工触发移除
- [x] 中英文语言键成对存在

## 验证

- [x] Debug|x64 编译零错误
- [x] 示例程序启动正常（`controls_d.exe` 持续运行）
- [ ] 示例运行：动态项显示、点击回调、勾选切换、收纳浮层均正常（待界面手工验证）
- [ ] 浅色/深色、中文/英文、100%/150% DPI 显示正常（待界面手工验证）
- [x] `verify_docs.py --extra docs` 零漂移

## 文档

- [x] `Control.md` / `Control.en.md` 属性表、接口、事件、动态示例同步
- [x] `Control-Roadmap.md` / `.en.md` D-7 项补充说明
- [x] `Development-Plan.md` D-7 与跟踪项已完成
- [x] 三份技能速查同步
