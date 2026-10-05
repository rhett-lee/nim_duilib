# nim_duilib 完整 API 速查

<!-- verify:allow-missing btn_default -->
<!-- btn_default 是自定义样式的举例名，verify_docs.py 不应判为漂移。 -->

仅在需要查具体 API / 属性 / 容器定义时载入本文。日常任务优先读对应的专题文档。

---

## 一、XML 布局结构

### 窗口模板

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="800,600" min_size="240,100"
        caption="0,0,0,36" use_system_caption="false"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_attached="true"
        layered_window="true" alpha="255" size_box="4,4,4,4"
        icon="../public/caption/logo.ico">
  <VBox bkcolor="bg_window_main">
    <!-- 标题栏（name 必须是 window_title_bar；旧名 window_caption_bar 仅为兼容 fallback） -->
    <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bg_titlebar">
      <Control />
      <Button class="btn_wnd_min_11" name="btn_window_min" height="32" width="40" margin="0,2,0,2"/>
      <Box height="stretch" width="40" margin="0,2,0,2">
        <Button class="btn_wnd_max_11" name="btn_window_max" height="32" width="stretch"/>
        <Button class="btn_wnd_restore_11" name="btn_window_restore" height="32" width="stretch" visible="false"/>
      </Box>
      <Button class="btn_wnd_close_11" name="btn_window_close" height="stretch" width="40"/>
    </HBox>
    <!-- 内容区域 -->
    <Box>
      <!-- 放置内容 -->
    </Box>
  </VBox>
</Window>
```

### Window 属性速查

| 属性 | 类型 | 说明 |
|------|------|------|
| size | size | 窗口初始大小，支持百分比 "75%,75%" |
| min_size / max_size | size | 最小/最大尺寸 |
| caption | rect | 标题栏可拖动区域 "0,0,0,36" |
| size_box | rect | 可拖动调整大小的边距 "4,4,4,4" |
| shadow_type | string | 阴影类型: default/big/big_round/small/small_round/menu/menu_round/none/none_round/custom/system_default/system_not_round/system_round/system_small_round |
| shadow_attached | bool | 是否附加阴影 |
| layered_window | bool | 是否为层窗口 |
| alpha | int | 透明度 0-255 |
| round_corner | size | 窗口圆角 "4,4" |
| icon | string | 窗口图标路径(ico) |
| use_system_caption | bool | 使用系统标题栏 |
| text / textid | string | 窗口标题/多语言ID |

## 二、容器类型速查

| XML节点 | 布局方式 | 说明 |
|---------|---------|------|
| Box | 浮动(Layout) | 自由定位，子控件绝对或相对布局 |
| VBox | 垂直(VLayout) | 子控件从上到下依次排列 |
| HBox | 水平(HLayout) | 子控件从左到右依次排列 |
| VFlowBox | 垂直流式 | 垂直排列，自动换列 |
| HFlowBox | 水平流式 | 水平排列，自动换行 |
| Panel | 浮动(Layout) | 带标题栏的面板容器（`PanelTemplate<Box>`），可折叠/手风琴 |
| PanelHBox | 水平(HLayout) | 带标题栏的水平面板（`PanelTemplate<HBox>`） |
| PanelVBox | 垂直(VLayout) | 带标题栏的垂直面板（`PanelTemplate<VBox>`） |
| VTileBox | 垂直瓦片 | 网格式垂直排列，columns属性 |
| HTileBox | 水平瓦片 | 网格式水平排列，rows属性 |
| GridBox | 网格(GridLayout) | 网格布局，支持单元格合并 |
| TabBox | 浮动 | 多页签切换，仅显示当前页 |
| ScrollBox | 浮动+滚动条 | 可滚动的Box |
| VScrollBox | 垂直+滚动条 | 可滚动的VBox |
| HScrollBox | 水平+滚动条 | 可滚动的HBox |
| VListBox | 垂直列表 | 可选择的垂直列表 |
| HListBox | 水平列表 | 可选择的水平列表 |
| VirtualVListBox | 虚拟垂直列表 | 大数据量虚拟列表(垂直) |
| VirtualHListBox | 虚拟水平列表 | 大数据量虚拟列表(水平) |

### 容器属性

| 属性 | 类型 | 说明 |
|------|------|------|
| child_margin | int | 子控件间距(XY相同) |
| child_margin_x | int | 子控件水平间距 |
| child_margin_y | int | 子控件垂直间距 |
| child_halign | string | 子控件水平对齐: left/center/right |
| child_valign | string | 子控件垂直对齐: top/center/bottom |
| mouse_child | bool | 子控件是否响应鼠标 |
| padding | rect | 内边距 "L,T,R,B" |

### Panel 面板容器属性（Panel / PanelHBox / PanelVBox）

三种节点是同一模板 `PanelTemplate` 的实例（基类分别为 Box/HBox/VBox），在普通容器之外
增加标题栏与折叠/展开能力。头文件 `duilib/Box/Panel.h`，完整示例见 `examples/panel`。

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| title | | string | 标题文字 |
| title_id | | string | 标题文字的多语言资源 ID（别名 titleid）；按当前语言解析标题，切换语言自动刷新，找不到 ID 保留原标题；用法同 Label 的 text_id |
| title_height | 28 | int | 标题栏高度（像素，DPI 自适应） |
| title_bk_color | | string | 标题栏背景语义色名，不设置则不绘制 |
| title_text_color | text_default | string | 标题文字语义色名 |
| title_text_align | left | string | 标题水平对齐：left/hcenter/right |
| title_font | system_bold_14 | string | 标题字体 ID |
| collapsible | false | bool | 是否可点击标题栏折叠/展开 |
| collapsed | false | bool | 初始折叠状态（不播动画、不触发事件） |
| collapse_trigger | title | string | 折叠热区：title（整个标题栏）/ arrow（仅箭头） |
| collapse_anim | 0 | int | 折叠动画毫秒数，0 立即切换（建议 150~300） |
| arrow_align | right | string | 箭头位置：right/left |
| group | | string | 手风琴分组名；同窗口同组同时只展开一个（可全折叠） |
| title_slot | | string | 标题栏槽位子控件 name（子控件建议 float="true"），折叠后仍可点击 |
| arrow_expanded_normal_image | | string | 展开态（▼）箭头普通图 |
| arrow_expanded_hovered_image | | string | 展开态箭头悬停图（别名 arrow_expanded_hot_image） |
| arrow_expanded_pushed_image | | string | 展开态箭头按下图（别名 arrow_expanded_pressed_image） |
| arrow_expanded_disabled_image | | string | 展开态箭头禁用图 |
| arrow_collapsed_normal_image | | string | 折叠态（▶）箭头普通图 |
| arrow_collapsed_hovered_image | | string | 折叠态箭头悬停图（别名 arrow_collapsed_hot_image） |
| arrow_collapsed_pushed_image | | string | 折叠态箭头按下图（别名 arrow_collapsed_pressed_image） |
| arrow_collapsed_disabled_image | | string | 折叠态箭头禁用图 |

要点：

- `padding` 仍是内容区内边距，标题栏空间由控件在顶部自动额外预留。
- 未配箭头图时用矢量三角（折叠▶/展开▼），悬停标题栏时箭头变 `color_accent`；某状态缺图回退普通图。
- 折叠只屏蔽内容区的测量/绘制/命中测试，不改子控件 `visible`，fixed/auto/stretch 高度均可折叠。
- 多语言：`title_id` 标题在切换语言时自动刷新（C++ 接口 `SetTitleId()`/`GetTitleId()`）；
  若用 `SetTitle()` 设置了带参数的动态文案，需自行在窗口 `OnLanguageChanged()` 中重设。
- C++：`SetTitle()`/`GetTitle()`、`SetTitleId()`/`GetTitleId()`、
  `SetCollapsed(bool bCollapsed, bool bFireEvent=true, bool bPlayAnim=true)`、`IsCollapsed()`、
  `SetCollapseAnimMillSeconds()`、`SetGroup()`、`SetArrowAlign()`、`SetTitleTextHAlign()`、
  `SetArrowStateImage()`、`SetTitleSlotName()`。
- 事件：完成后触发 `kEventCollapse` / `kEventExpand`；动作前触发
  `kEventPanelCollapsing` / `kEventPanelExpanding`，回调返回 false 可取消
  （XML 初始 `collapsed` 与手风琴内部联动不触发取消事件）。

## 三、控件类型速查

| XML节点 | 基类 | 说明 |
|---------|------|------|
| Control | - | 基础控件/占位符 |
| Label | Control | 文本标签 |
| Button | Label | 按钮 |
| CheckBox | Button | 复选框 |
| Option | CheckBox | 单选按钮(group属性分组) |
| Combo | Box | 下拉组合框 |
| FilterCombo | Combo | 可过滤的下拉框 |
| CheckCombo | Box | 多选下拉框 |
| ComboButton | Box | 带下拉的按钮 |
| RichEdit | ScrollBox | 文本编辑框(单行/多行/密码) |
| RichText | Control | 富文本显示(HTML子集) |
| Progress | Label | 进度条 |
| Slider | Progress | 滑块 |
| CircleProgress | Progress | 圆形进度条 |
| DateTime | HBox | 日期时间选择器（`LabelTemplate<HBox>`） |
| TreeView | ListBox | 树形控件 |
| TreeNode | ListBoxItem | 树节点 |
| ListCtrl | VBox | 列表控件(Report/List/Icon视图) |
| PropertyGrid | VBox | 属性网格 |
| HyperLink | Label | 超级链接 |
| Line | Control | 画线控件 |
| Split / SplitBox | Control/Box | 分隔条（`SplitTemplate<Control>` / `SplitTemplate<Box>`） |
| ScrollBar | Control | 滚动条 |
| TabCtrl | ListBox | 标签页控件 |
| IPAddress | HBox | IP地址输入 |
| HotKey | HBox | 热键输入 |
| GroupBox/GroupVBox/GroupHBox | Box | 分组容器 |
| ColorControl/ColorPicker* | - | 颜色选择器组件 |
| DirectoryTree | TreeView | 目录树 |

### Control 通用属性(所有控件继承)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| name | | string | 控件名称(窗口内建议唯一) |
| class | | string | 引用global.xml中的通用样式，**必须写在最前面** |
| width | stretch | int/string | 宽度: 数值/stretch/auto/"50%" |
| height | stretch | int/string | 高度: 数值/stretch/auto/"50%" |
| min_width / min_height | -1 | int | 最小宽度/高度 |
| max_width / max_height | MAX | int/string | 最大宽度/高度 |
| margin | 0,0,0,0 | rect | 外边距 "L,T,R,B" |
| padding | 0,0,0,0 | rect | 内边距 "L,T,R,B" |
| halign | left | string | 水平对齐: left/center/right |
| valign | top | string | 垂直对齐: top/center/bottom |
| visible | true | bool | 是否可见 |
| enabled | true | bool | 是否可用 |
| float | false | bool | 是否绝对定位 |
| bkcolor | | string | 背景色(颜色名/ARGB) |
| bkimage | | string | 背景图片 |
| normal_image / hot_image / pushed_image / disabled_image | | string | 各状态图片 |
| normal_color / hot_color / pushed_color / disabled_color | | string | 各状态颜色 |
| border_size | 0 | int/rect | 边框大小 |
| border_color | | string | 边框颜色 |
| border_round | 0,0 | size | 边框圆角 |
| tooltip_text | | string | 鼠标悬浮提示 |
| alpha | 255 | int | 透明度 0-255 |
| cursor_type | arrow | string | 光标: arrow/hand/ibeam/wait/cross/size_we/size_ns 等 |
| no_focus | false | bool | 是否不可获取焦点 |
| tab_stop | true | bool | 是否允许TAB切换 |
| show_focused_rect | false | bool | 键盘TAB聚焦时是否绘制焦点虚线框（别名 show_focus_rect）；C++ 接口 SetShowFocusedRect()/IsShowFocusedRect()，需要"初始不显示、TAB后才显示"时在 OnKeyDownMsg 中按需开启 |
| fade_visible | true | bool | 可见性变化时是否有动画 |
| box_shadow | | string | 阴影 "color='red' offset='0,0' blurradius='8' spreadradius='8'" |

### Label 属性(继承 Control)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| text | | string | 显示文本 |
| text_id | | string | 多语言ID |
| text_align | "left,top" | string | 对齐: left/hcenter/right/hjustify + top/vcenter/bottom |
| text_padding | 0,0,0,0 | rect | 文字内边距 |
| font | | string | 字体ID(定义在global.xml) |
| normal_text_color | | string | 普通文字颜色 |
| hot_text_color | | string | 悬浮文字颜色 |
| single_line | true | bool | 单行显示 |
| multi_line | false | bool | 多行显示 |
| end_ellipsis | false | bool | 省略号截断 |
| rich_text | false | bool | 支持HTML子集富文本 |

### CheckBox 额外属性(继承 Button)

| 属性 | 类型 | 说明 |
|------|------|------|
| selected | bool | 是否选中 |
| selected_normal_image / selected_hot_image / ... | string | 选中状态各状态图片 |

### Option 额外属性(继承 CheckBox)

| 属性 | 类型 | 说明 |
|------|------|------|
| group | string | 分组名称，同组内互斥 |

### RichEdit 属性(继承 ScrollBox)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| text | | string | 文本内容 |
| font | | string | 字体ID |
| text_align | "left,top" | string | 对齐方式 |
| text_padding | | rect | 文字内边距 |
| single_line | true | bool | 单行模式 |
| multi_line | false | bool | 多行模式 |
| password | false | bool | 密码模式 |
| readonly | false | bool | 只读 |
| number_only | false | bool | 仅数字 |
| max_number / min_number | | int | 数字范围 |
| limit_text | | int | 最大字符数 |
| prompt_text | | string | 占位提示文字 |
| prompt_color | | string | 提示文字颜色 |
| word_wrap | false | bool | 自动换行 |
| vscrollbar / hscrollbar | false | bool | 滚动条 |
| want_return | false | bool | 接受回车 |
| want_tab | false | bool | 接受Tab |
| caret_color | | string | 光标颜色 |
| normal_text_color | | string | 文字颜色 |

### Progress 属性(继承 Label)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| min | 0 | int | 最小值 |
| max | 100 | int | 最大值 |
| value | 0 | int | 当前值 |
| horizontal | true | bool | 水平方向 |
| progress_color | | string | 进度条颜色 |
| progress_image | | string | 进度条图片 |

### Combo 属性(继承 Box)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| combo_type | "drop_down" | string | "drop_down"(可编辑) / "drop_list"(不可编辑) |
| dropbox_size | | string | 下拉列表尺寸 |
| popup_top | false | bool | 向上弹出 |

## 四、全局资源 (global.xml)

### 字体定义

`<Font>` 命名规则为 `<字体类型>_<样式>_<字号>`，可选样式：regular / bold / underline / italic /
strikeout / fullstyle（四者全开），字号支持 12/14/16/18/20/22。`default="true"` 的字体为默认字体。

```xml
<DefaultFontFamilyNames windows="Microsoft YaHei, SimSun" macos="PingFang SC" linux="Noto Sans CJK SC"/>
<Font id="system_regular_14" name="system" size="14" default="true"/>   <!-- 默认字体 -->
<Font id="system_bold_14" name="system" size="14" bold="true"/>
<FontFile file="fonts/RobotoMono-Regular.ttf" desc="Roboto Mono常规"/>  <!-- 字体文件放在 resources/fonts/ -->
```

旧 ID（`system_12` ~ `system_22`，无样式段）仍保留作兼容别名，新代码请用 `system_<样式>_<字号>`。
**不存在 `arial_*` 系列。**

### 颜色定义

颜色**不要**在 `themes/default/global.xml` 里写死数值：该文件的 `<ThemeColor>` 已迁移到
`bin/resources/themes/color_light/global.xml`（浅色）与 `color_dark/global.xml`（深色），
由框架在运行时按当前主题加载。`themes/default/global.xml` 中只剩下**旧名→新名的 `<Alias>` 映射**。

```xml
<!-- 自定义颜色：写在 bin/resources/themes/color_light/global.xml 的 <Global> 内 -->
<ThemeColor name="my_brand_color" value="#FF1890FF" type="common"
            category="bg_color" role="neutral" comment_cn="品牌色"/>
```

颜色格式: "#AARRGGBB"(ARGB) 或 "#RRGGBB"(RGB) 或颜色名(Blue/Red/White...)

常用语义色（浅色主题实际取值，取自 `color_light/global.xml`）与兼容别名:

| global.xml 语义色名 | 浅色取值 | 兼容别名（旧名） | 用途 |
|------|------|------|------|
| bg_window_main | #FFF4F4F4 | bk_wnd_darkcolor | 窗口主背景 |
| bg_container | #FFF9F9F9 | bk_wnd_lightcolor | 容器背景 |
| bg_titlebar | #FFEAEAEA | - | 标题栏背景 |
| bg_list_item_hovered | #FFEAEAEA | bk_listitem_hovered | 列表项悬浮 |
| bg_list_item_selected | #FFE2E2E2 | bk_listitem_selected | 列表项选中 |
| bg_menu_item_hovered | #FFEBEBEB | bk_menuitem_hovered | 菜单项悬浮 |
| text_default | #FF1A1A1A | default_font_color | 主文本 |
| text_disabled | #B3343434 | disabled_font_color | 禁用文本 |
| border_split_level1 | #FFE7E7E7 | splitline_level1 | 分割线 |

完整的颜色清单与命名规范见仓库 `docs/ThemeColor.md`。深色主题取值不同，
**写业务 XML 时一律用语义色名，不要写死色值**。

### 通用样式(Class)

```xml
<Class name="btn_default" font="system_12" normal_text_color="white"
       normal_image="file='btn_normal.png'"
       hot_image="file='btn_hot.png'"
       pushed_image="file='btn_pushed.png'"/>
```

使用: `<Button class="btn_default" text="Click"/>`
**class属性必须写在所有属性最前面**

## 五、图片属性

```xml
<!-- 简单用法 -->
<Control bkimage="logo.png"/>

<!-- 完整属性 -->
<Control bkimage="file='icon.svg' width='24' height='24' valign='center' halign='center'"/>
```

| 属性 | 类型 | 说明 |
|------|------|------|
| file | string | 图片路径(相对于主题目录) |
| width / height | string | 图片尺寸(像素或百分比) |
| src | rect | 源区域裁剪 "L,T,R,B" |
| dest | rect | 目标绘制区域 |
| corner | rect | 九宫格参数 "L,T,R,B" |
| fade | int | 透明度 0-255 |
| halign / valign | string | 对齐方式 |
| xtiled / ytiled | bool | 平铺绘制 |
| auto_play | bool | 动画自动播放 |
| play_count | int | 动画播放次数(-1无限) |

支持格式: PNG, SVG, JPG, GIF, BMP, APNG, WEBP, ICO, Lottie-JSON, PAG

## 六、XML 事件系统

### XML内联事件

```xml
<Button name="my_btn" text="Click">
  <Event type="click" receiver="target_control_name" apply_attribute="visible='true'"/>
</Button>

<!-- 事件目标类型 -->
<!-- receiver="name"           按名称查找控件 -->
<!-- receiver="./name"         在当前容器内查找 -->
<!-- receiver=""               控件自身 -->
<!-- receiver="#window#"       窗口 -->
```

### 常用事件类型

| type值 | 说明 |
|--------|------|
| click | 点击 |
| rclick | 右键点击 |
| mouse_enter / mouse_leave | 鼠标进入/离开 |
| mouse_button_down / mouse_button_up | 鼠标按下/释放 |
| mouse_double_click | 双击 |
| select / unselect | 选中/取消选中(ListBox/Combo) |
| check / uncheck | 勾选/取消勾选(CheckBox) |
| tab_select | 标签页切换 |
| text_changed | 文本变化 |
| value_changed | 值变化 |
| expand / collapse | Panel 面板展开/折叠完成 |
| panel_expanding / panel_collapsing | Panel 即将展开/折叠（C++ 回调返回 false 可取消；XML 内联仅用于 apply_attribute） |
| key_down / key_up | 按键 |
| return | 回车 |
| visible_changed | 可见性变化 |
| window_close | 窗口关闭 |
| window_size | 窗口大小变化 |

## 七、C++ 代码模式

### 窗口类模板

```cpp
// MyForm.h
#ifndef MY_FORM_H_
#define MY_FORM_H_
#include "duilib/duilib.h"

// WindowImplBase 声明在 duilib/Utils/WinImplBase.h（文件名不带 Window 前缀）
class MyForm : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    MyForm();
    virtual ~MyForm() override;
    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void OnInitWindow() override;
};
#endif

// MyForm.cpp
#include "MyForm.h"

MyForm::MyForm() {}
MyForm::~MyForm() {}

DString MyForm::GetSkinFolder() { return _T("my_skin"); }
DString MyForm::GetSkinFile() { return _T("my_form.xml"); }

void MyForm::OnInitWindow()
{
    BaseClass::OnInitWindow();
    // 在这里初始化控件和绑定事件
}
```

### 主线程模板

```cpp
// MainThread.h
#include "duilib/duilib.h"
class MainThread : public ui::FrameworkThread
{
public:
    MainThread();
    virtual ~MainThread() override;
private:
    // 注意：FrameworkThread::OnInit() 返回 bool（FrameworkThread.h:122），不是 void
    virtual bool OnInit() override;
    virtual void OnCleanup() override;
};

// MainThread.cpp
#include "MainThread.h"
#include "MyForm.h"

MainThread::MainThread() : FrameworkThread(_T("MainThread"), ui::kThreadUI) {}
MainThread::~MainThread() {}

bool MainThread::OnInit()
{
    ui::FilePath resourcePath = ui::GlobalManager::GetResourceRootPath(false);
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    MyForm* window = new MyForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("MyApp"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
    return true;
}

void MainThread::OnCleanup()
{
    ui::GlobalManager::Instance().Shutdown();
}
```

### 入口函数(Windows)

```cpp
#include "MainThread.h"
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int)
{
    MainThread thread;
    thread.RunMessageLoop();
    return 0;
}
```

### 控件操作

```cpp
// 查找控件
ui::Button* btn = dynamic_cast<ui::Button*>(FindControl(_T("my_button")));
ui::Label* label = dynamic_cast<ui::Label*>(FindControl(_T("my_label")));
ui::RichEdit* edit = dynamic_cast<ui::RichEdit*>(FindControl(_T("my_edit")));
ui::CheckBox* check = dynamic_cast<ui::CheckBox*>(FindControl(_T("my_check")));
ui::Combo* combo = dynamic_cast<ui::Combo*>(FindControl(_T("my_combo")));
ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("my_list")));
ui::Progress* progress = dynamic_cast<ui::Progress*>(FindControl(_T("my_progress")));

// 设置属性
label->SetText(_T("Hello"));
edit->SetText(_T("Input"));
DString text = edit->GetText();
check->SetSelected(true);
bool isChecked = check->IsSelected();
progress->SetValue(50);

// 可见性
btn->SetVisible(true);
btn->SetEnabled(false);
```

### 事件绑定

```cpp
void MyForm::OnInitWindow()
{
    BaseClass::OnInitWindow();

    // 按钮点击
    ui::Button* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_ok")));
    if (btn) {
        btn->AttachClick([this](const ui::EventArgs& args) {
            // 处理逻辑
            return true;
        });
    }

    // 复选框状态变化
    ui::CheckBox* check = dynamic_cast<ui::CheckBox*>(FindControl(_T("my_check")));
    if (check) {
        check->AttachSelect([this](const ui::EventArgs& args) {
            // 选中
            return true;
        });
        check->AttachUnSelect([this](const ui::EventArgs& args) {
            // 取消选中
            return true;
        });
    }

    // 文本变化
    ui::RichEdit* edit = dynamic_cast<ui::RichEdit*>(FindControl(_T("my_edit")));
    if (edit) {
        edit->AttachTextChanged([this](const ui::EventArgs& args) {
            // 文字改变
            return true;
        });
    }

    // 列表选择
    ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("my_list")));
    if (list) {
        list->AttachSelect([this](const ui::EventArgs& args) {
            // ListBox::AttachSelect：wParam 是新选中项索引，lParam 是 Box::InvalidIndex（占位符，不是旧索引）
            const size_t newIndex = static_cast<size_t>(args.wParam); // wParam 实际类型是 WPARAM
            // args.lParam 当前为 Box::InvalidIndex，请勿当旧索引使用
            return true;
        });
    }

    // 通用事件绑定
    btn->AttachEvent(ui::kEventMouseEnter, [](const ui::EventArgs&) {
        return true;
    });
}
```

### Panel 面板折叠/展开

```cpp
// 头文件 duilib/Box/Panel.h；节点 Panel / PanelHBox / PanelVBox 对应三个 C++ 类
ui::PanelVBox* panel = dynamic_cast<ui::PanelVBox*>(FindControl(_T("my_panel")));
if (panel != nullptr) {
    panel->SetCollapseAnimMillSeconds(220);

    // 切换折叠：bFireEvent 是否触发事件，bPlayAnim=false 立即切换不播动画
    panel->SetCollapsed(!panel->IsCollapsed(), true, true);

    // 完成事件（无动画立即触发，有动画在动画结束后触发）
    panel->AttachExpand([](const ui::EventArgs&) { return true; });
    panel->AttachCollapse([](const ui::EventArgs&) { return true; });

    // 即将折叠：返回 false 取消本次操作（展开不受限时同样可监听 AttachExpanding）
    panel->AttachCollapsing([](const ui::EventArgs&) {
        return false; // true 放行，false 取消
    });
}
```

手风琴：多个 Panel 设置相同 `group` 属性后，展开其中一个会自动折叠同窗口同组的其他面板；
初始展开哪个由 XML 的 `collapsed` 决定，允许全部折叠。

### ListBox 动态添加项

```cpp
ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("my_list")));
for (int i = 0; i < 100; i++) {
    ui::ListBoxItem* item = new ui::ListBoxItem(this);
    item->SetText(ui::StringUtil::Printf(_T("Item %d"), i));
    item->SetClass(_T("list_item"));   // 真名 list_item；listitem 是 <Alias> 旧名
    item->SetFixedHeight(ui::UiFixedInt(20), true, true);
    list->AddItem(item);
}
```

### TreeView 动态添加节点

```cpp
ui::TreeView* tree = dynamic_cast<ui::TreeView*>(FindControl(_T("my_tree")));
ui::TreeNode* root = tree->GetRootNode();
ui::TreeNode* node = new ui::TreeNode(this);
node->SetClass(_T("tree_node"));
node->SetText(_T("New Node"));
root->AddChildNode(node);
```

### Combo 动态添加选项

```cpp
ui::Combo* combo = dynamic_cast<ui::Combo*>(FindControl(_T("my_combo")));
ui::TreeView* treeView = combo->GetTreeView();
ui::TreeNode* treeNode = treeView->GetRootNode();
for (int i = 0; i < 10; i++) {
    ui::TreeNode* node = new ui::TreeNode(this);
    node->SetClass(_T("tree_node"));
    node->SetText(ui::StringUtil::Printf(_T("Option %d"), i));
    treeNode->AddChildNode(node);
}
combo->SetCurSel(0); // 默认选中第一项
```

### 线程间通信

```cpp
// 投递任务到工作线程
ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadWorker,
    UiBind(&MyForm::DoBackgroundWork, this));

// 投递任务到UI线程
ui::GlobalManager::Instance().Thread().PostTask(ui::kThreadUI,
    UiBind(&MyForm::UpdateUI, this));

// 定时重复任务
ui::GlobalManager::Instance().Thread().PostRepeatedTask(ui::kThreadWorker,
    ui::UiBind(this, [this]() {
        // 每次执行的逻辑
    }),
    200 // 毫秒间隔
);
```

### 弱回调保护

```cpp
// 使用 UiBind 绑定成员函数(自动弱引用保护)
UiBind(&MyForm::OnButtonClick, this);

// lambda 中使用 ui::UiBind 包装
ui::UiBind(this, [this]() {
    // 控件销毁后不会执行
});
```

### 模态消息框 MessageBoxWnd

框架自带的自绘皮肤模态消息框，头文件 `duilib/Utils/MessageBoxWnd.h`，
皮肤 `bin/resources/themes/default/public/messagebox/messagebox.xml`，
完整演示见 `examples/controls`（MessageBoxForm）。只能通过静态接口 `Show` 使用，
内部以 `DoModal` 模态显示，阻塞父窗口，返回用户点击的按钮。

```cpp
#include "duilib/Utils/MessageBoxWnd.h"

//基本用法（按钮组合 + 图标）
int32_t nResult = ui::MessageBoxWnd::Show(this,
    _T("确定要删除该文件吗？"), _T("删除确认"),
    ui::MessageBoxWnd::kButtonsYesNoCancel,
    ui::MessageBoxWnd::kIconQuestion);
if (nResult == ui::MessageBoxWnd::kResultYes) {
    //执行删除
}

//多语言：第 7 个参数传 true，text/title/按钮文字全部按多语言 ID 解析，切换语言实时生效
int32_t nResult2 = ui::MessageBoxWnd::Show(this,
    _T("STRID_DELETE_CONFIRM_TEXT"), _T("STRID_DELETE_CONFIRM_TITLE"),
    ui::MessageBoxWnd::kButtonsOKCancel,
    ui::MessageBoxWnd::kIconWarning, nullptr, true);

//自定义按钮文字（bTextId=true 时成员也填多语言 ID；某一项留空则用框架默认文字）
ui::MessageBoxWnd::ButtonText buttonText;
buttonText.yes    = _T("STRID_CUSTOM_SAVE");
buttonText.no     = _T("STRID_CUSTOM_DISCARD");
buttonText.cancel = _T("STRID_CUSTOM_CANCEL");
ui::MessageBoxWnd::Show(this, _T("..."), _T("..."),
    ui::MessageBoxWnd::kButtonsYesNoCancel,
    ui::MessageBoxWnd::kIconQuestion, &buttonText, true);
```

`Show(pParentWindow, text, title, buttonFlags, iconType, pButtonText=nullptr, bTextId=false)`：

| 参数 | 说明 |
|------|------|
| pParentWindow | 父窗口，模态期间不可操作，可为 nullptr |
| text / title | 消息内容（支持 `\n` 换行，长文本自动换行，窗口高度自适应）/ 窗口标题 |
| buttonFlags | 按钮组合，见下表 |
| iconType | `kIconNone` / `kIconInfo` / `kIconWarning` / `kIconError` / `kIconQuestion` |
| pButtonText | 自定义按钮文字，nullptr 用默认文字 |
| bTextId | 为 true 时 text/title/按钮文字均按多语言 ID 解析；任务栏原生标题也显示译文而非 ID |

| ButtonFlags 按钮组合 | 含有的按钮 |
|------|------|
| `kButtonsOK` | 确定 |
| `kButtonsOKCancel` | 确定、取消 |
| `kButtonsYesNo` | 是、否 |
| `kButtonsYesNoCancel` | 是、否、取消 |
| `kButtonsRetryCancel` | 重试、取消 |

返回值（与 Win32 `ID*` 一致；创建失败返回 -1）：

| Result | 值 | 触发方式 |
|------|----|------|
| `kResultOK` | 1 | 点击"确定"；或按 Enter 且组合含"确定" |
| `kResultCancel` | 2 | 点击"取消"、按 ESC、点标题栏关闭按钮 |
| `kResultRetry` | 4 | 点击"重试" |
| `kResultYes` | 6 | 点击"是"；Enter 默认按钮映射（组合无"确定"时） |
| `kResultNo` | 7 | 点击"否" |

要点：

- Enter 触发**当前焦点按钮**（默认焦点是组合中的第一个肯定性按钮：确定→是→重试）；
  按 TAB/Shift+TAB 可在功能按钮间循环切换焦点，标题栏关闭按钮不参与 TAB 循环。
- ESC 与标题栏 X 一律返回 `kResultCancel`，即使组合中没有"取消"按钮也不会映射成默认按钮
  （与 Win32 放弃选择的语义一致）。
- 焦点虚线框按需显示：窗口初始弹出时不显示焦点环，用户首次按 TAB 后才显示，
  由 `MessageBoxWnd::OnKeyDownMsg` 调 `SetShowFocusedRect(true)` 开启。
- DoModal 下 Enter 键能派发给焦点按钮，依赖 `NativeWindow_Windows::SetEnterKeyPassthrough`
  的注册机制（见 pitfalls.md「DoModal 模态对话框的键盘消息」）；自定义模态窗口若有同样需求需自行注册/注销。

## 八、布局属性速查

### 瓦片布局 (HTileBox/VTileBox)

| 属性 | 类型 | 说明 |
|------|------|------|
| item_size | size | 子项大小 "100,40" |
| rows (HTileBox) | int/"auto" | 行数 |
| columns (VTileBox) | int/"auto" | 列数 |
| scale_down | bool | 超出时缩小 |

### 网格布局 (GridBox)

| 属性 | 类型 | 说明 |
|------|------|------|
| rows | int | 网格行数(0自动) |
| columns | int | 网格列数(0自动) |
| grid_width | int | 单元格宽度(0自动) |
| grid_height | int | 单元格高度(0自动) |

子控件使用 `row_span`/`col_span` 属性合并单元格。

### ScrollBox 额外属性

| 属性 | 类型 | 说明 |
|------|------|------|
| vscrollbar | bool | 垂直滚动条 |
| hscrollbar | bool | 水平滚动条 |
| vscrollbar_class | string | 垂直滚动条样式 |
| hscrollbar_class | string | 水平滚动条样式 |

## 九、文件路径约定

- 主题资源根目录: `bin/resources/themes/default/`
- 全局配置: `bin/resources/themes/default/global.xml`
- 窗口XML: `bin/resources/themes/default/<skin_folder>/<skin_file>.xml`
- 公共图片: `bin/resources/themes/default/public/`
- 字体文件: `bin/resources/fonts/`
- 语言文件: `bin/resources/lang/`（`zh_CN.txt` / `en_US.txt`，示例中另有 `zh_CN-public.txt`、`zh_CN-examples.txt` 等）
  **注意**：语言文件在 **`bin/resources/lang/`**，**不在** `themes/default/lang/`
- 主题色定义: `bin/resources/themes/color_light/global.xml`（浅色）/ `bin/resources/themes/color_dark/global.xml`（深色）
- 示例源码: `examples/<example_name>/`（注意目录名大小写，如 `examples/RichEdit`）
