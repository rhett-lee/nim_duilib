# Button / CheckBox / Option 演示页规格

- 状态：已批准（已实现）
- 日期：2026-10-22
- 关联示例：`examples/controls`（新增独立演示窗口）
- 实现方式：**不修改任何控件代码**，仅使用现有 XML 属性、样式类与事件接口

## 1. 目标

在 `controls` 示例程序中新增一个独立演示页面，集中展示 `ui::Button`、`ui::CheckBox`（含三态/开关样式）
与 `ui::Option` 的常用能力，并提供一组可复用的演示 SVG 图标（按分类放在主题 `public/button/`、`public/checkbox/` 目录）。

需要覆盖：

1. Button：主要/次要/图片/危险/灰色/描边/链接/禁用，三种尺寸，图标+文字、纯图标、危险图标
2. CheckBox：填充与描边两种默认样式、选中/未选中/禁用、自定义图标（心形/星形/铃铛）、
   `support_check_mode` 三态（全选/半选/未选）、`checkbox_toggle_1/2` 开关样式
3. Option：`option_1` / `option_2` 两种样式、同组互斥、禁用项
4. 综合表单：开关 + 复选框 + 单选混排
5. 事件回显：按钮点击次数、CheckBox 勾选/取消、Option 选中、三态切换结果

## 2. 现状与差距

现有机制已足够：

- `Button` 继承 `Label`，支持 `normal_image` 等状态图片与 `text_padding`，可用于图标+文字
- `CheckBox` 支持 `selected_normal_image`、`part_selected_normal_image`、`selected_text_color`、
  `support_check_mode`、`SetChecked/SetPartChecked`、`AttachCheck/AttachUnCheck`
- `Option` 支持 `group` 属性与 `AttachSelect/AttachUnSelect`
- `global.xml` 已提供 `btn_primary_global`、`btn_global`、`btn_global_*_80x30`、`checkbox_1/2`、
  `checkbox_toggle_1/2`、`option_1/2` 等样式类

差距仅在示例侧：

- `controls/test.xml` 只有分散的零散按钮，没有专门的 Button/CheckBox/Option 演示页
- 没有演示三态 CheckBox（`support_check_mode`）
- 没有自定义图标的 CheckBox 示例
- `public/` 下缺少演示用图标

## 3. 范围

### 3.1 In Scope

- 新增 `bin/resources/themes/default/controls/button_checkbox.xml`
- 新增 `examples/controls/ButtonCheckBoxForm.h` / `.cpp`
- 新增 `public/button/` 下 4 个按钮图标、`public/checkbox/` 下 8 个复选框图标，共 12 个 SVG
- 不修改 `global.xml`（复用现有的 `PATH_BUTTON` / `PATH_CHECKBOX`）
- 修改 `bin/resources/themes/default/controls/test.xml`：新增入口按钮
- 修改 `examples/controls/TestForm.h` / `.cpp`：新增 `ShowButtonCheckBoxWindow()`
- 修改 `examples/controls/controls.vcxproj` / `.vcxproj.filters`：登记新源文件
- 修改 `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增 `STRID_BCB_*` 文案

### 3.2 Out of Scope

- 不修改 `duilib/Control/Button.h/.cpp`、`CheckBox.h/.cpp`、`Option.h/.cpp`
- 不新增样式类，也不新增资源路径变量：图标直接放到现有的 `public/button`、`public/checkbox` 目录
- 不新增主题语义色
- 不修改 `docs/Control.md`（本次无新属性/接口；如后续需要再单独补文档）
- 不引入新的多语言机制，沿用 `text_id` + `lang/*-examples.txt`

## 4. 图标资源

不新增路径变量，直接复用现有的 `PATH_BUTTON` / `PATH_CHECKBOX`：

- 按钮类图标 → `bin/resources/themes/default/public/button/`
- 复选框/三态/自定义图标 → `bin/resources/themes/default/public/checkbox/`

全部为 16×16 或 18×18 视口，**只用 `fill`，禁用 `stroke`**（兼容 nanosvg）；
单色图标的主色统一写 `#333333`，由 XML 的 `svg_replace_colors` 映射为语义色。

| 文件 | 内容 | 用法 |
|---|---|---|
| `public/button/icon_download.svg` | 向下箭头 + 底部横线 | 图标+文字按钮（替换为 `border_svg_image` / `color_white`） |
| `public/button/icon_settings.svg` | 滑杆设置 | 纯图标按钮（替换为 `border_svg_image`） |
| `public/button/icon_trash.svg` | 垃圾桶 | 危险图标按钮（替换为 `color_error`） |
| `public/button/icon_arrow_right.svg` | 右箭头 | 链接/次要按钮 |
| `public/checkbox/checkbox_checked.svg` | 圆角方块 `#2F6FED` + 白色对勾 | 三态全选（替换为 `color_accent`） |
| `public/checkbox/checkbox_mixed.svg` | 圆角方块 `#2F6FED` + 白色减号 | 三态半选（替换为 `color_accent`） |
| `public/checkbox/heart_outline.svg` | 灰色心形 `#808080` | 自定义 CheckBox 未选中（替换为 `text_muted`） |
| `public/checkbox/heart_filled.svg` | 红色心形 `#E5484D` | 自定义 CheckBox 选中（替换为 `color_error`） |
| `public/checkbox/star_outline.svg` | 灰色五角星 `#808080` | 同上（替换为 `text_muted`） |
| `public/checkbox/star_filled.svg` | 金色五角星 `#F5A623` | 同上（替换为 `color_warning`） |
| `public/checkbox/bell_outline.svg` | 灰色铃铛 `#808080` | 同上（替换为 `text_muted`） |
| `public/checkbox/bell_filled.svg` | 蓝色铃铛 `#2F6FED` | 同上（替换为 `color_accent`） |

## 5. 演示窗口 `controls/button_checkbox.xml`

### 5.1 窗口骨架

沿用 `controls/switch.xml` 的写法：

- `Window size="1020,820" min_size="820,640" caption="0,0,0,36" shadow_type="default" size_box="4,4,4,4"`
- 根 `VBox bkcolor="bg_window_main"`
- 标题栏：`HBox` + logo + `Label name="label_title" text_id="STRID_BCB_WINDOW_TITLE"` + `Button class="btn_wnd_close_11" name="btn_window_close"`
- 提示区：`Label text_id="STRID_BCB_HINT"`（`text_muted`，`auto_tooltip="true"`）
- 内容区：`VScrollBox vscrollbar="true"` → `VBox width="stretch" height="auto"`

卡片统一写法（与 `switch.xml` 一致）：

```xml
<VBox bkcolor="bg_window_card" border_size="1" bordercolor="border_window"
      border_round="6,6" padding="16,14,16,14" width="50%" height="auto" margin="0,0,6,0">
```

### 5.2 卡片布局

| 行 | 左卡片 | 右卡片 |
|---|---|---|
| 1 | 按钮类型与状态 `STRID_BCB_GROUP_BTN_TYPE` | 按钮尺寸与图标 `STRID_BCB_GROUP_BTN_SIZE` |
| 2 | CheckBox 基础 `STRID_BCB_GROUP_CHECK_BASIC` | CheckBox 三态与开关 `STRID_BCB_GROUP_CHECK_STATE` |
| 3 | Option 单选 `STRID_BCB_GROUP_OPTION` | 综合表单 `STRID_BCB_GROUP_FORM` |
| 4 | 事件回显 `STRID_BCB_GROUP_EVENT`（`width="stretch"` 跨两列） |  |

### 5.3 卡片 1：按钮类型与状态

- 第一行：`btn_primary_global`（主要）、`btn_global`（次要）、`btn_global_white_80x30`、`btn_global_red_80x30`、`btn_global_gray_80x30`
- 第二行：描边按钮、链接按钮、`enabled="false"` 的主要按钮、`enabled="false"` 的次要按钮
- 尺寸统一 `height="32"`，圆角 `border_round="4,4"`（80x30 的类保持原宽高，不再覆盖）
- 描边按钮：

```xml
<Button class="btn_global" width="96" height="32" border_round="4,4"
        text_id="STRID_BCB_BTN_GHOST"
        normal_color="bg_window_card" hovered_color="bg_btn_hovered" pressed_color="bg_btn_pressed"
        normal_text_color="color_accent" hovered_text_color="color_accent" pressed_text_color="color_accent"
        normal_border_color="color_accent" hovered_border_color="color_accent" pressed_border_color="color_accent"/>
```

- 链接按钮：同上但 `border_size="0,0,0,0"`，`normal_color="bg_window_card"`

### 5.4 卡片 2：按钮尺寸与图标

- 尺寸行：`width="120" height="38"`、`width="88" height="32"`、`width="64" height="24"`，均 `class="btn_global"`，`font` 依次 14/13/12
- 图标+文字行（两个）：
  - 次要按钮 + 下载图标：`text_padding="34,0,0,0"`、`text_align="left,vcenter"`、
    `normal_image="file='${PATH_BUTTON}/icon_download.svg' width='16' height='16' valign='center' halign='left' margin='12,0,0,0' svg_replace_colors='#333333|border_svg_image'"`
  - 主要按钮 + 下载箭头：同上，`svg_replace_colors='#333333|color_white'`
- 纯图标行：`width="34" height="34"` 的 `btn_global`，`icon_settings.svg` 居中（18×18）
- 危险图标行：`width="34" height="34"` 的 `btn_global`，`icon_trash.svg`（`color_error`）

### 5.5 卡片 3：CheckBox 基础

- `checkbox_1`：未选中 / 选中 / 禁用未选中 / 禁用选中
- `checkbox_2`：未选中 / 选中
- 自定义图标（沿用 `checkbox_1` 的文字间距写法，覆盖图片）：

```xml
<CheckBox name="check_heart" width="auto" height="20" text_padding="24,0,0,0"
          text_align="vcenter" valign="center" text_id="STRID_BCB_CHECK_HEART"
          selected_text_color="color_error"
          normal_image="file='${PATH_CHECKBOX}/heart_outline.svg' width='16' height='16' valign='center' halign='left' svg_replace_colors='#808080|text_muted'"
          selected_normal_image="file='${PATH_CHECKBOX}/heart_filled.svg' width='16' height='16' valign='center' halign='left' svg_replace_colors='#E5484D|color_error'"/>
```

星形（`color_warning`）、铃铛（`color_accent`）同理；每个控件都设 `name` 以便统一绑定事件。

### 5.6 卡片 4：CheckBox 三态与开关

- 三态复选框 `name="check_tristate_demo"`：

```xml
<CheckBox name="check_tristate_demo" support_check_mode="true" width="auto" height="20"
          text_id="STRID_BCB_CHECK_TRISTATE" text_padding="22,0,0,0" text_align="vcenter" valign="center"
          normal_image="file='${PATH_CHECKBOX}/checkbox-outline-unchecked.svg' width='18' height='18' valign='center' halign='left' svg_replace_colors='#808080|border_svg_image'"
          selected_normal_image="file='${PATH_CHECKBOX}/checkbox_checked.svg' width='18' height='18' valign='center' halign='left' svg_replace_colors='#2F6FED|color_accent'"
          part_selected_normal_image="file='${PATH_CHECKBOX}/checkbox_mixed.svg' width='18' height='18' valign='center' halign='left' svg_replace_colors='#2F6FED|color_accent'"/>
```

- 右侧三个按钮：`btn_check_all` / `btn_check_part` / `btn_check_none`，`class="btn_global"`，`width="72" height="26"`
- 状态标签 `name="bcb_tristate_label"`，初始 `STRID_BCB_TRISTATE_NONE_TEXT`
- 开关样式行：`checkbox_toggle_1`（关/开）、`checkbox_toggle_2`（关/开）

### 5.7 卡片 5：Option 单选

- `option_1` 三组：`group="bcb_opt_1"`，第一项 `selected="true"`，`name="option_a/b/c"`
- `option_2` 三组：`group="bcb_opt_2"`，`name="option_x/y/z"`
- 一行 `enabled="false"` 的禁用项（`option_1`）

### 5.8 卡片 6：综合表单

- `checkbox_toggle_1 name="form_notify"` + `Label STRID_BCB_FORM_NOTIFY`
- `checkbox_1 name="form_auto_update"` + `Label STRID_BCB_FORM_AUTO_UPDATE`
- `Label STRID_BCB_FORM_CHANNEL` + `option_2` 两项：`group="bcb_channel"`，
  `name="form_channel_stable"`（`selected="true"`）/ `name="form_channel_beta"`
- 底部：`btn_primary_global name="form_save"` + `btn_global name="form_cancel"`，
  `form_cancel` 点击后 `CloseWnd()`（复用现有窗口关闭方式）
- 保存后结果写入 `name="bcb_form_label"`（初始 `STRID_BCB_FORM_HINT`）

### 5.9 卡片 7：事件回显

- `Button class="btn_global" name="bcb_btn_counter"`（`STRID_BCB_EVENT_CLICK_BTN`）
- `Label name="bcb_click_label"`（初始 `STRID_BCB_EVENT_CLICK_NONE`）
- `CheckBox name="check_event_demo"` + `Label name="bcb_check_label"`（初始 `STRID_BCB_EVENT_NONE`）
- `Label name="bcb_option_label"`（初始 `STRID_BCB_EVENT_NONE`）

## 6. C++ 演示逻辑 `ButtonCheckBoxForm`

```cpp
class ButtonCheckBoxForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    ButtonCheckBoxForm();
    virtual ~ButtonCheckBoxForm() override;
    virtual DString GetSkinFolder() override;   // "controls"
    virtual DString GetSkinFile() override;     // "button_checkbox.xml"
    virtual void OnInitWindow() override;

private:
    void UpdateTristateText();
    void UpdateCheckText(const ui::CheckBox* pCheckBox, bool bChecked);
    void UpdateOptionText(const ui::Option* pOption);

private:
    int32_t m_nClickCount = 0;
};
```

`OnInitWindow()` 绑定：

1. `bcb_btn_counter` → `m_nClickCount++`，
   `bcb_click_label->SetText(ui::StringUtil::Printf(_T("%d"), m_nClickCount))`（标签前缀用 XML 中的 `STRID_BCB_EVENT_CLICK_COUNT`）
2. 遍历 `check_event_demo`：`AttachCheck` → `UpdateCheckText(p, true)`，`AttachUnCheck` → `UpdateCheckText(p, false)`
3. `check_tristate_demo` → `AttachCheck` / `AttachUnCheck` → `UpdateTristateText()`
4. `btn_check_all` / `btn_check_part` / `btn_check_none`（勾选模式下绘制依据是 `IsChecked()` + `IsPartChecked()`）：
   - 全选：`SetPartChecked(false)` + `SetChecked(true, false)`
   - 半选：`SetChecked(true, false)` + `SetPartChecked(true)`（`IsChecked()` 必须为 true，才会进入图片绘制分支）
   - 取消：`SetPartChecked(false)` + `SetChecked(false, false)`
   - 每个按钮执行后调用 `UpdateTristateText()`
5. `option_a/b/c/x/y/z` 与 `form_channel_stable/beta`：`AttachSelect` → `UpdateOptionText(pOption)`，
   文本写入 `bcb_option_label`（`STRID_BCB_EVENT_OPTION` + 选项文字）
6. `form_save` → `bcb_form_label->SetText(GetTextById(_T("STRID_BCB_FORM_SAVED")))`
7. `form_cancel` → `CloseWnd()`

`UpdateTristateText()`：按 `IsChecked()` / `IsPartChecked()` 分别写
`STRID_BCB_TRISTATE_ALL_TEXT` / `STRID_BCB_TRISTATE_PART_TEXT` / `STRID_BCB_TRISTATE_NONE_TEXT`。

`UpdateCheckText()`：根据文本与 `bChecked` 拼出 `STRID_BCB_EVENT_CHECKED` / `STRID_BCB_EVENT_UNCHECKED`。
`UpdateOptionText()`：`STRID_BCB_EVENT_OPTION` + `pOption->GetText()`。

## 7. 入口与构建登记

### 7.1 `test.xml`

在第 150 行 `</HBox>` 之后新增一行：

```xml
<HBox height="auto">
    <!-- Button / CheckBox / Option：独立演示窗口入口 -->
    <GroupHBox text="Button / CheckBox / Option" height="70" width="auto" margin="6,6,6,6" corner_size="4,4">
        <Button class="btn_global" name="button_checkbox_btn" text_id="STRID_BCB_TEST"
                margin="20,30,20,0" width="180" height="30" border_round="3,3"/>
    </GroupHBox>
</HBox>
```

### 7.2 `TestForm.h` / `TestForm.cpp`

- `#include "ButtonCheckBoxForm.h"`（放在 `#include "SwitchForm.h"` 之后）
- 声明 `void ShowButtonCheckBoxWindow();`
- 在 Switch 绑定块之后新增绑定，实现方式与 `ShowSwitchWindow()` 完全一致：

```cpp
void TestForm::ShowButtonCheckBoxWindow()
{
    ButtonCheckBoxForm* testForm = new ButtonCheckBoxForm();
    ui::WindowCreateParam createParam;
    createParam.m_dwStyle = ui::kWS_POPUP;
    createParam.m_dwExStyle = ui::kWS_EX_LAYERED;
    createParam.m_windowTitle = _T("ButtonCheckBoxForm");
    createParam.m_bCenterWindow = true;
    testForm->CreateWnd(this, createParam);
    testForm->ShowModalFake();
}
```

### 7.3 工程文件

- `controls.vcxproj`：`<ClInclude Include="ButtonCheckBoxForm.h" />`、`<ClCompile Include="ButtonCheckBoxForm.cpp" />`
- `controls.vcxproj.filters`：分别归入 `Header Files` / `Source Files` 过滤器

## 8. 语言资源

在 `zh_CN-examples.txt` / `en_US-examples.txt` 的 `STRID_SWITCH_*` 之后追加约 45 条 `STRID_BCB_*`：

- 入口/窗口：`STRID_BCB_TEST`、`STRID_BCB_WINDOW_TITLE`、`STRID_BCB_HINT`
- 卡片标题：`STRID_BCB_GROUP_BTN_TYPE`、`_BTN_SIZE`、`_CHECK_BASIC`、`_CHECK_STATE`、`_OPTION`、`_FORM`、`_EVENT`
- 卡片说明：`STRID_BCB_BTN_TYPE_DESC`、`_BTN_SIZE_DESC`、`_CHECK_BASIC_DESC`、`_CHECK_STATE_DESC`、`_OPTION_DESC`、`_FORM_DESC`
- 按钮文案：`STRID_BCB_BTN_PRIMARY/SECONDARY/IMAGE/DANGER/GRAY/GHOST/LINK/DISABLED/LARGE/MEDIUM/SMALL/ICON_TEXT`
- CheckBox 文案：`STRID_BCB_CHECK_1/2/DISABLED/HEART/STAR/BELL/TOGGLE/TRISTATE`
- 三态：`STRID_BCB_TRISTATE_ALL/PART/NONE`、`STRID_BCB_TRISTATE_ALL_TEXT/PART_TEXT/NONE_TEXT`
- Option：`STRID_BCB_OPTION_A/B/C/X/Y/Z/DISABLED`
- 表单：`STRID_BCB_FORM_NOTIFY/AUTO_UPDATE/CHANNEL/STABLE/BETA/SAVE/CANCEL/SAVED/HINT`
- 事件：`STRID_BCB_EVENT_CLICK_BTN/CLICK_NONE/CLICK_COUNT/NONE/CHECKED/UNCHECKED/OPTION`

命名与值格式沿用现有文件（`ID = 文本`），英文文件给对应翻译。

## 9. 验证

1. **编译**：`MSBuild build/examples.sln /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo`，要求退出码 0
2. **静态校验**：
   - 所有新 SVG 只含 `fill`，不含 `stroke`
   - XML 中 `class` 属性位于控件属性最前
   - 所有 `text_id` / `name` 在两份语言文件与 XML 中一一对应（可用 `verify_docs.py` 辅助）
3. **运行** `controls_d.exe`：
   - 点击入口按钮打开新窗口，核对 7 张卡片
   - 按钮悬停/按下/禁用视觉正常；图标+文字按钮图标不遮挡文字
   - 三态「全选/半选/取消」三个按钮驱动 `check_tristate_demo` 视觉与标签
   - Option 同组互斥；两组互不影响；禁用项不可点
   - 事件回显：点击计数、勾选标签、Option 标签随操作更新
   - 保存/取消按钮行为正确
4. **回归**：`test.xml` 原有布局未变化（只在末尾新增一行卡片）

## 10. 风险与约束

- 项目约定：用户手动提交代码、手动编译 Release；本次只需保证 Debug|x64 编译通过
- 全部改动位于示例/资源侧，不影响框架控件行为
- 自定义 CheckBox 图标依赖 `svg_replace_colors` 的语义色映射；若主题缺少对应语义色，图片会保留 SVG 原色
