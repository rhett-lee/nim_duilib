# 任务清单：Button / CheckBox / Option 演示页

- 规格：[spec.md](./spec.md)
- 状态：已完成

## 1. 资源与变量

- [x] 1.1 不新增资源变量：复用 `global.xml` 中已有的 `PATH_BUTTON` / `PATH_CHECKBOX`
- [x] 1.2 新建 `bin/resources/themes/default/public/button/icon_download.svg`
- [x] 1.3 新建 `public/button/icon_settings.svg`
- [x] 1.4 新建 `public/button/icon_trash.svg`
- [x] 1.5 新建 `public/button/icon_arrow_right.svg`
- [x] 1.6 新建 `public/checkbox/checkbox_checked.svg`
- [x] 1.7 新建 `public/checkbox/checkbox_mixed.svg`
- [x] 1.8 新建 `public/checkbox/heart_outline.svg` / `heart_filled.svg`
- [x] 1.9 新建 `public/checkbox/star_outline.svg` / `star_filled.svg`
- [x] 1.10 新建 `public/checkbox/bell_outline.svg` / `bell_filled.svg`

## 2. 演示窗口

- [x] 2.1 新建 `bin/resources/themes/default/controls/button_checkbox.xml`：窗口骨架（标题栏/提示区/滚动区）
- [x] 2.2 卡片 1「按钮类型与状态」
- [x] 2.3 卡片 2「按钮尺寸与图标」
- [x] 2.4 卡片 3「CheckBox 基础」（含自定义心形/星形/铃铛）
- [x] 2.5 卡片 4「CheckBox 三态与开关」
- [x] 2.6 卡片 5「Option 单选」
- [x] 2.7 卡片 6「综合表单」
- [x] 2.8 卡片 7「事件回显」

## 3. C++ 逻辑

- [x] 3.1 新建 `examples/controls/ButtonCheckBoxForm.h`
- [x] 3.2 新建 `examples/controls/ButtonCheckBoxForm.cpp`：事件绑定与标签更新
- [x] 3.3 `examples/controls/TestForm.h`：声明 `ShowButtonCheckBoxWindow()`
- [x] 3.4 `examples/controls/TestForm.cpp`：`#include`、绑定 `button_checkbox_btn`、实现创建窗口

## 4. 入口与工程登记

- [x] 4.1 `bin/resources/themes/default/controls/test.xml`：末尾新增一行 `GroupHBox` 入口
- [x] 4.2 `examples/controls/controls.vcxproj`：登记 `.h` / `.cpp`
- [x] 4.3 `examples/controls/controls.vcxproj.filters`：登记到 `Header Files` / `Source Files`

## 5. 语言资源

- [x] 5.1 `bin/resources/lang/zh_CN-examples.txt`：追加 `STRID_BCB_*`
- [x] 5.2 `bin/resources/lang/en_US-examples.txt`：追加对应英文

## 6. 验证

- [x] 6.1 `MSBuild build/examples.sln /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo` 退出码 0
- [x] 6.2 核对全部 `text_id` 均已在两份语言文件中定义（61 个，全部命中）
- [x] 6.3 核对 SVG 仅使用 `fill`（12 个文件，无 `stroke`）
- [x] 6.4 启动冒烟：`bin/controls_d.exe` 进程存活、主窗口标题正常、无断言弹窗
- [x] 6.5 确认 `test.xml` 原有内容未受影响（仅在末尾新增一行入口）

## 7. 遗留说明

- 7 张卡片的视觉细节（间距、图标遮挡、颜色观感）需在运行环境中人工确认
