# C++ 桌面界面库 浅色主题 背景色深浅规范表

## 规范说明
1. 适用库：Qt / MFC / DuiLib / SOUI / Duilib NG / DuiEngine 等所有C++桌面界面库
2. 深浅等级：数字越小 → 颜色越浅；数字越大 → 颜色越深
3. 同一等级内所有控件/区域使用**完全相同的背景色**，便于统一维护
4. 遵循Windows系统原生视觉规范：编辑框最浅，标题栏最深

## 背景色等级明细（最浅 → 最深）
| 深浅等级 | 典型色值 | 控件 / 区域名称 | 设计说明 |
| :------- | :------- | :-------------- | :------- |
| 1（最浅） | #FFFFFF | 编辑框 Edit/LineEdit<br>富文本框 RichEdit<br>下拉框 ComboBox 编辑区<br>复选框 CheckBox 方框<br>单选框 RadioButton 圆圈 | 纯白底色，输入交互核心区域，全界面最浅 |
| 2 | #F8F9FA | 列表控件 ListCtrl/ListView<br>树控件 TreeView<br>列表框 ListBox<br>数据表格 Grid/DataGrid | 数据展示专用区域，接近纯白，略深于输入区 |
| 3 | #F0F0F0 | 主窗口/对话框客户区背景<br>标签页 Tab 页面内容区域 | 系统标准窗口基底底色 |
| 4 | #EDEFF2 | 通用容器面板 Panel/Widget<br>分组框 GroupBox 内部区域 | 布局分区容器，视觉层级高于窗口基底 |
| 5 | #E5E7EB | 普通按钮 Button（默认态）<br>开关控件 Switch<br>进度条 Progress 背景<br>滑块控件 Slider 槽区域<br>下拉框 ComboBox 整体背景<br>状态栏 StatusBar | 基础交互控件区域，中等深度 |
| 6 | #DFE1E5 | 菜单栏 MenuBar<br>工具栏 ToolBar<br>标签页 Tab 标签条背景 | 顶部功能操作区域 |
| 7（最深） | #D6D9DD | 窗口标题栏 TitleBar/Caption | 窗口视觉锚点，全界面背景色最深 |