# Switch 控件增强规格（参照 Ant Design Switch）

- 状态：待批准
- 日期：2026-10-22
- 关联示例：`examples/controls`（新增独立演示窗口）

## 1. 目标

参照 Ant Design `Switch` 组件（https://ant-design.antgroup.com/components/switch-cn）的示例效果，补齐 nim_duilib
`ui::Switch` 目前缺失的展示能力，并在示例程序中集中展示：

1. 轨道内文字（选中/未选中两种文案）
2. 轨道内图标（可单独使用，也可与文字组合）
3. 加载中状态（拇指内旋转指示器，且禁止用户点击切换）
4. 禁用态视觉（轨道/拇指专用颜色）
5. 两种尺寸（默认、小号）
6. 自定义颜色/图片风格（沿用现有 `track_*_color`、`thumb_*_image` 等属性）
7. 选中/取消选中事件回显

## 2. 现状与差距

`ui::Switch` 继承 `CheckBox`，已具备：

- 选中/未选中状态与动画（`m_nAnimValue`）
- 轨道/拇指颜色插值、轨道/拇指图片淡入淡出
- 圆角轨道、内嵌拇指、`thumb_padding`、`animation_ms`
- `kEventSelect` / `kEventUnSelect` 事件

差距：

- 没有轨道内文字/图标，无法实现 Ant 的 `checkedChildren` / `unCheckedChildren`
- 没有加载态
- 禁用时仍按正常态绘制（只是不可点击），缺少禁用视觉
- 示例程序只有 `controls.xml` 中的两个基础开关，未覆盖上述效果

## 3. 范围

### 3.1 本次实现（In Scope）

- `duilib/Control/Switch.h` / `Switch.cpp`：新增属性、绘制逻辑、加载动画
- `bin/resources/themes/default/global.xml`：新增 `switch_small` 样式类
- `bin/resources/themes/default/controls/switch.xml`：新增独立演示页面
- `examples/controls/SwitchForm.h` / `SwitchForm.cpp`：演示窗口逻辑
- `examples/controls/TestForm.h` / `TestForm.cpp`、`bin/resources/themes/default/controls/test.xml`：增加打开演示窗口的按钮
- `bin/resources/themes/default/public/switch/`：新增 4 个演示图标 SVG
- `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增演示文案
- `examples/controls/controls.vcxproj` / `controls.vcxproj.filters`：登记新增源文件
- `docs/Control.md` / `docs/Control.en.md`：补充 Switch 新属性与新接口
- `.trae/skills/nim-duilib/references/api-reference.md`、`.workbuddy/skills/nim-duilib/references/api-reference.md`、
  `.claude/docs/nim-duilib-llm-reference.md`：同步 Switch 属性表

### 3.2 非目标（Out of Scope）

- 不修改 `Layout` / `HBox` / `VBox` 等布局代码
- 不新建 `SwitchEx` 之类的派生控件，能力全部内聚到 `ui::Switch`
- 不引入 Ant Design 的 Web 语义（className/design token），仅以 XML 属性 + 主题色表达样式
- 不改变现有 `kEventSelect` / `kEventUnSelect` 的事件语义
- 不改 `CheckBox` 基类
- 不新增主题语义色（复用现有语义色名作为默认值）

## 4. 交互与视觉规格

### 4.1 轨道内文字

- 选中时在拇指左侧区域显示 `switch_text_on`，未选中时在拇指右侧区域显示 `switch_text_off`
- 切换动画过程中按动画进度交叉淡入淡出：选中文字透明度 = 进度，未选中文字透明度 = 1 - 进度
- 文字默认垂直居中、水平居中于「拇指未占用的轨道区域」
- 默认文字颜色：
  - `switch_text_on_color` = `bg_switch_thumb_on`（白色，适配彩色轨道）
  - `switch_text_off_color` = `text_default`
- 禁用时文字改用语义色 `text_disabled`
- 字体复用控件自身字体（`font` 属性，与 `CheckBox` 文本一致）

### 4.2 轨道内图标

- `switch_image_on` / `switch_image_off` 指定的图片绘制在对应状态的文字区域
- 图标尺寸 = `max(8px, 轨道高度 / 2)`，在可用区域内居中
- 同时设置图标与文字时：水平排列为「图标 + 2px 间距 + 文字」，整体在可用区域内居中
- 图标透明度与同侧文字一致（随动画进度淡入淡出；禁用时 128/255）
- 图片字符串支持现有 `file='...'`、`svg_replace_colors` 等写法，复用 `Image::LoadImage` 流程

### 4.3 加载中

- `switch_loading="true"` 时：
  - 在拇指中心绘制 8 个圆点组成的旋转指示器，每 60ms 旋转 30°，约 720ms 一圈
  - 指示器颜色默认 `color_accent`，可用 `switch_loading_color` 覆盖；禁用时使用 `text_disabled`
  - 用户点击不再切换选中状态（`Activate` 直接返回），但程序调用 `SetSelected` 仍然有效
  - 轨道、拇指、文字/图标仍按当前选中状态绘制

### 4.4 禁用态

- 轨道颜色使用 `track_disabled_color`（默认 `bg_switch_track_off`）
- 拇指颜色使用 `thumb_disabled_color`（默认 `bg_switch_thumb_on`）
- 拇指位置直接取当前选中状态（关闭动画插值）
- 配置了轨道图片时：禁用态绘制 `track_off_image`；拇指图片按选中状态取 `thumb_on_image` / `thumb_off_image`
- 文字颜色为 `text_disabled`；图标透明度 128/255
- 禁用态不显示悬停/按下视觉（Switch 本身无这些绘制，保持现状）

### 4.5 尺寸

- 控件尺寸由 XML 的 `width` / `height` 决定，拇指大小随轨道高度自动缩放
- 新增 `switch_small` 类：`height="20" width="40"`，沿用默认图片皮肤
- 轨道内文字/图标尺寸随轨道高度自适应，无需单独配置

## 5. XML 属性

| 属性名 | 别名 | 类型 | 默认值 | 说明 |
|---|---|---|---|---|
| `switch_text_on` | `switchtexton` | 文本 | 空 | 选中时轨道内文字 |
| `switch_text_off` | `switchtextoff` | 文本 | 空 | 未选中时轨道内文字 |
| `switch_text_on_color` | `switchtextoncolor` | 颜色 | `bg_switch_thumb_on` | 选中文字颜色 |
| `switch_text_off_color` | `switchtextoffcolor` | 颜色 | `text_default` | 未选中文字颜色 |
| `switch_image_on` | `switchimageon` | 图片 | 空 | 选中时轨道内图标 |
| `switch_image_off` | `switchimageoff` | 图片 | 空 | 未选中时轨道内图标 |
| `switch_loading` | `switchloading` | 布尔 | `false` | 加载中状态 |
| `switch_loading_color` | `switchloadingcolor` | 颜色 | `color_accent` | 加载指示器颜色 |
| `track_disabled_color` | `trackdisabledcolor` | 颜色 | `bg_switch_track_off` | 禁用态轨道颜色 |
| `thumb_disabled_color` | `thumbdisabledcolor` | 颜色 | `bg_switch_thumb_on` | 禁用态拇指颜色 |

布尔值解析沿用项目惯例：`"true"` 或 `"1"` 为真，其余为假。
属性解析失败时 `ASSERT(0)` 并返回 `false`，与 `Switch` 现有属性处理一致。

## 6. C++ 接口（`duilib/Control/Switch.h`）

```cpp
void SetTextOn(const DString& strText);
DString GetTextOn() const;
void SetTextOff(const DString& strText);
DString GetTextOff() const;

void SetTextOnColor(const DString& strColor);
DString GetTextOnColor() const;
void SetTextOffColor(const DString& strColor);
DString GetTextOffColor() const;

void SetImageOn(const DString& strImage);
DString GetImageOn() const;
void SetImageOff(const DString& strImage);
DString GetImageOff() const;

void SetLoading(bool bLoading);
bool IsLoading() const;
void SetLoadingColor(const DString& strColor);
DString GetLoadingColor() const;

void SetTrackDisabledColor(const DString& strColor);
DString GetTrackDisabledColor() const;
void SetThumbDisabledColor(const DString& strColor);
DString GetThumbDisabledColor() const;
```

重写点：

- `virtual void SetAttribute(...) override`：解析上述属性
- `virtual void Activate(const EventArgs* pMsg) override`：加载中拦截用户点击
- `virtual void PaintText(IRender* pRender) override`：先调用基类绘制普通 `text`，再绘制轨道内文字/图标
- `virtual void ChangeDpiScale(...) override`：重新加载两个轨道内图标

私有辅助：

- `int32_t GetEffectiveAnimValue() const`：禁用时返回目标值（0/100），否则返回 `m_nAnimValue`
- `UiRect GetThumbRect() const`：改用 `GetEffectiveAnimValue()`
- `void PaintSwitchContent(IRender* pRender, const UiRect& rcContent, const DString& strText, Image* pImage, UiColor textColor, uint8_t nAlpha)`
- `void PaintLoading(IRender* pRender, const UiRect& rcThumb)`
- `void OnLoadingTimer()`

## 7. 实现要点

### 7.1 文件：`duilib/Control/Switch.h`

- 增加上述公开接口与成员变量
- 新增 `WeakCallbackFlag m_loadingTimer;`、`int32_t m_nLoadingAngle;`、`bool m_bLoading;`
- 新增 `std::unique_ptr<Image> m_pImageOn;` / `m_pImageOff;`
- 构造函数默认值：
  - `m_strTextOnColor = _T("bg_switch_thumb_on")`
  - `m_strTextOffColor = _T("text_default")`
  - `m_strTrackDisabledColor = _T("bg_switch_track_off")`
  - `m_strThumbDisabledColor = _T("bg_switch_thumb_on")`
  - `m_strLoadingColor = _T("color_accent")`
  - `m_bLoading = false`、`m_nLoadingAngle = 0`

### 7.2 文件：`duilib/Control/Switch.cpp`

- `SetAttribute` 增加属性解析；图片属性调用 `UpdateSwitchImage` 同一套加载逻辑
- `PaintStateColors`：
  - 用 `GetEffectiveAnimValue()` 计算进度
  - 禁用时轨道取 `m_strTrackDisabledColor`；有轨道图片时禁用态只绘制 `track_off_image`
  - 启用时保持现有颜色插值 / 图片交叉淡入逻辑不变
- `PaintStateImages`：
  - 启用时保持现有拇指绘制不变
  - 禁用时颜色取 `m_strThumbDisabledColor`，图片取当前选中状态对应的拇指图
  - 拇指绘制完成后，若 `m_bLoading` 调用 `PaintLoading`
- `PaintText`：
  - 先调用 `BaseClass::PaintText(pRender)`
  - 无轨道内文字/图标时直接返回
  - 按 4.1 计算左右两个可用区域并交叉淡入淡出
- `PaintLoading`：
  - 以拇指中心为圆心，半径 `max(2, 拇指宽 / 4)`，点半径 `max(1, 拇指宽 / 10)`
  - 8 个点按 `m_nLoadingAngle + i * 45°` 分布，透明度从 255 递减（每点 -22，最低约 79）
  - 使用 `pRender->FillRoundRect` 绘制
- `SetLoading`：
  - 状态变化时重置角度
  - 开启时 `GlobalManager::Instance().Timer().AddTimer(m_loadingTimer.GetWeakFlag(), UiBind(&Switch::OnLoadingTimer, this), 60)`
  - 关闭时 `m_loadingTimer.Cancel()`
  - `Invalidate()`
- 析构函数中 `m_loadingTimer.Cancel()`
- `Activate`：`if (m_bLoading) { return; } BaseClass::Activate(pMsg);`
- `ChangeDpiScale`：在现有缩放逻辑后重新加载 `m_pImageOn` / `m_pImageOff`

### 7.3 图标资源

在 `bin/resources/themes/default/public/switch/` 新增：

- `switch_demo_sun.svg`（选中态图标，白色）
- `switch_demo_moon.svg`（未选中态图标，灰色 `#8A8886`）
- `switch_demo_close.svg`（未选中态图标，灰色 `#8A8886`）
- `switch_demo_sad.svg`（禁用态图标，灰色 `#BFBFBF`）

每个 SVG 为 12×12 简单线条图标，采用项目现有 SVG 写法（`viewBox` + `path`/`circle`）。

### 7.4 `switch_small` 样式

在 `bin/resources/themes/default/global.xml` 的 Switch 相关 Class 区域新增：

```xml
<Class name="switch_small" height="20" width="40" valign="center"
       cursor_type="hand" thumb_padding="0" animation_ms="200"
       font="system_regular_12"
       track_off_color="bg_switch_track_off" track_on_color="color_accent"
       thumb_off_image="file='${PATH_SWITCH}/switch_ios_thumb.svg'"
       thumb_on_image="file='${PATH_SWITCH}/switch_ios_thumb.svg'"/>
```

（参数以现有 `switch` 类为基准，仅尺寸/字体不同。）

## 8. 示例程序

### 8.1 新增 `SwitchForm`

- `examples/controls/SwitchForm.h/.cpp`
- `GetSkinFolder()` 返回 `controls`，`GetSkinFile()` 返回 `switch.xml`
- `OnInitWindow()`：
  - 给 `switch_event_demo` 绑定 `kEventSelect` / `kEventUnSelect`，把结果写入 `switch_event_label`
  - `btn_loading_on` 点击：`switch_loading_demo->SetLoading(true)`
  - `btn_loading_off` 点击：`switch_loading_demo->SetLoading(false)`
  - 初始事件文案为 `STRID_SWITCH_EVENT_NONE`

### 8.2 新增 `bin/resources/themes/default/controls/switch.xml`

窗口约 `960×720`，标题栏复用其他演示窗口写法，内容使用 `VScrollBox` + 卡片布局：

| 卡片 | 内容 |
|---|---|
| 基本 | 默认关闭、默认开启、事件回显开关 `switch_event_demo` |
| 不可用 | 禁用关闭、禁用开启 |
| 文字和图标 | `switch_text_on="On"`、`switch_text_on="1"`、太阳/月亮图标、禁用 `Sad` + 图标 |
| 两种大小 | `class="switch"` 与 `class="switch_small"` |
| 加载中 | `switch_loading="true"` 的开启态开关 + 「进入加载 / 结束加载」按钮 |
| 自定义样式 | `color_success` / `color_error` / `color_warning` 轨道色、`switch_fluent`、`switch_material` |
| 事件 | `switch_event_label` 文本 |

所有颜色使用语义色名，不写死色值。

### 8.3 入口按钮

- `test.xml` 新增一行 `GroupHBox text="Switch"`，按钮 `name="switch_btn"`、`text_id="STRID_SWITCH_TEST"`
- `TestForm.h` 增加 `void ShowSwitchWindow();`
- `TestForm.cpp` 增加 `#include "SwitchForm.h"` 与 `ShowSwitchWindow()` 实现（复用 `ShowStepperWindow` 的创建方式），并在控件初始化处绑定按钮

### 8.4 构建登记

- `examples/controls/controls.vcxproj`：`<ClCompile Include="SwitchForm.cpp" />`、`<ClInclude Include="SwitchForm.h" />`
- `examples/controls/controls.vcxproj.filters`：按现有分组登记

## 9. 语言资源

在 `zh_CN-examples.txt` / `en_US-examples.txt` 中新增（值仅示例，英文对应翻译）：

- `STRID_SWITCH_TEST`：开关演示 / Switch Demo
- `STRID_SWITCH_WINDOW_TITLE`：开关 Switch / Switch
- `STRID_SWITCH_HINT`：参照 Ant Design Switch 的常见状态 / Common states inspired by Ant Design Switch
- `STRID_SWITCH_GROUP_BASIC`：基本 / Basic
- `STRID_SWITCH_GROUP_DISABLED`：不可用 / Disabled
- `STRID_SWITCH_GROUP_TEXT`：文字和图标 / Text and icon
- `STRID_SWITCH_GROUP_SIZE`：两种大小 / Two sizes
- `STRID_SWITCH_GROUP_LOADING`：加载中 / Loading
- `STRID_SWITCH_GROUP_CUSTOM`：自定义样式 / Custom style
- `STRID_SWITCH_GROUP_EVENT`：事件回显 / Event log
- `STRID_SWITCH_BASIC_DESC` / `DISABLED_DESC` / `TEXT_DESC` / `SIZE_DESC` / `LOADING_DESC` / `CUSTOM_DESC`：各卡片说明
- `STRID_SWITCH_LOADING_ON`：进入加载 / Start loading
- `STRID_SWITCH_LOADING_OFF`：结束加载 / Stop loading
- `STRID_SWITCH_EVENT_NONE`：事件：无 / Event: none
- `STRID_SWITCH_EVENT_SELECTED`：事件：选中 / Event: selected
- `STRID_SWITCH_EVENT_UNSELECTED`：事件：未选中 / Event: unselected

## 10. 文档同步

- `docs/Control.md` 的 Switch 小节：属性表补充第 5 节 10 个属性；接口列表补充第 6 节；增加「文字/图标/加载/禁用/小号」XML 示例
- `docs/Control.en.md`：同步英文版
- 三份 Skill 参考文档的 Switch 属性表/接口表同步：`.trae/skills/nim-duilib/references/api-reference.md`、
  `.workbuddy/skills/nim-duilib/references/api-reference.md`、`.claude/docs/nim-duilib-llm-reference.md`

## 11. 验证

1. **编译**：`MSBuild.exe build/examples.sln /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo`，要求 `EXIT=0`
2. **文档校验**：`python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default`
3. **运行** `layout_d.exe` / `controls_d.exe`：
   - 打开 Switch 演示窗口，逐项核对第 8.2 节卡片
   - 切换「文字和图标」中的开关，确认文字/图标随动画淡入淡出
   - 切换「加载中」按钮，确认指示器旋转且点击开关不改变状态
   - 确认禁用态不可点击且呈灰色
   - 确认事件回显标签随选中状态变化
4. **回归**：`controls.xml` 中原有 Switch、`Switch` 在深色主题下的默认样式保持可用

## 12. 兼容性与风险

- 所有新属性默认值都不改变现有 XML 的绘制结果；唯一有意的行为变化是 `enabled="false"` 的 Switch 现在呈现禁用视觉
- 未配置文字/图标时不增加任何绘制开销
- 加载计时器仅在 `switch_loading="true"` 时运行，关闭后立即取消
- 新属性均只作用于 `ui::Switch`，不影响 `CheckBox` 及其他控件
