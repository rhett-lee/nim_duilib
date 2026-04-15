# nim_duilib LLM 快速参考

> 本文档专为 LLM/AI 代理优化，提供 nim_duilib 的核心 API 速查。

## 一、XML 布局结构

### 窗口模板
```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="800,600" min_size="80,60"
        caption="0,0,0,36" use_system_caption="false"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_attached="true"
        layered_window="true" alpha="255" size_box="4,4,4,4"
        icon="../public/caption/logo.ico">
  <VBox bkcolor="bk_wnd_darkcolor">
    <!-- 标题栏 -->
    <HBox name="window_caption_bar" width="stretch" height="36" bkcolor="bk_wnd_lightcolor">
      <Control />
      <Button class="btn_wnd_min_11" name="minbtn" height="32" width="40" margin="0,2,0,2"/>
      <Box height="stretch" width="40" margin="0,2,0,2">
        <Button class="btn_wnd_max_11" name="maxbtn" height="32" width="stretch"/>
        <Button class="btn_wnd_restore_11" name="restorebtn" height="32" width="stretch" visible="false"/>
      </Box>
      <Button class="btn_wnd_close_11" name="closebtn" height="stretch" width="40"/>
    </HBox>
    <!-- 内容区域 -->
    <Box>
      <!-- 放置你的内容 -->
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
| shadow_type | string | 阴影类型: default/big/big_round/small/small_round/menu/menu_round/none/none_round |
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
| DateTime | Label | 日期时间选择器 |
| TreeView | VListBox | 树形控件 |
| TreeNode | Box | 树节点 |
| ListCtrl | VBox | 列表控件(Report/List/Icon视图) |
| PropertyGrid | VListBox | 属性网格 |
| HyperLink | Label | 超级链接 |
| Line | Control | 画线控件 |
| Split / SplitBox | Control/Box | 分隔条 |
| ScrollBar | Control | 滚动条 |
| TabCtrl | HBox | 标签页控件 |
| IPAddress | HBox | IP地址输入 |
| HotKey | HBox | 热键输入 |
| GroupBox/GroupVBox/GroupHBox | Box | 分组容器 |
| ColorControl/ColorPicker* | - | 颜色选择器组件 |
| DirectoryTree | TreeView | 目录树 |

### Control 通用属性(所有控件继承)
| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| name | | string | 控件名称(窗口内建议唯一) |
| class | | string | 引用global.xml中的通用样式 |
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
```xml
<DefaultFontFamilyNames value="微软雅黑,宋体"/>
<Font id="system_12" name="system" size="12"/>
<Font id="system_bold_14" name="system" size="14" bold="true"/>
<FontFile file="RobotoMono-Regular.ttf" desc="Roboto Mono常规"/>
```

### 颜色定义
```xml
<TextColor name="default_font_color" value="#FF333333"/>
<TextColor name="bk_wnd_darkcolor" value="#FF2B2B2B"/>
```
颜色格式: "#AARRGGBB"(ARGB) 或 "#RRGGBB"(RGB) 或颜色名(Blue/Red/White...)

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
    virtual void OnInit() override;
    virtual void OnCleanup() override;
};

// MainThread.cpp
#include "MainThread.h"
#include "MyForm.h"

MainThread::MainThread() : FrameworkThread(_T("MainThread"), ui::kThreadUI) {}
MainThread::~MainThread() {}

void MainThread::OnInit()
{
    ui::FilePath resourcePath = ui::FilePathUtil::GetCurrentModuleDirectory();
    resourcePath += _T("resources\\");
    ui::GlobalManager::Instance().Startup(ui::LocalFilesResParam(resourcePath));

    MyForm* window = new MyForm();
    window->CreateWnd(nullptr, ui::WindowCreateParam(_T("MyApp"), true));
    window->PostQuitMsgWhenClosed(true);
    window->ShowWindow(ui::kSW_SHOW_NORMAL);
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
        edit->AttachTextChange([this](const ui::EventArgs& args) {
            // 文字改变
            return true;
        });
    }

    // 列表选择
    ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("my_list")));
    if (list) {
        list->AttachSelect([this](const ui::EventArgs& args) {
            size_t newIndex = args.wParam;
            size_t oldIndex = args.lParam;
            return true;
        });
    }

    // 通用事件绑定
    btn->AttachEvent(ui::kEventMouseEnter, [](const ui::EventArgs&) {
        return true;
    });
}
```

### ListBox 动态添加项
```cpp
ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindControl(_T("my_list")));
for (int i = 0; i < 100; i++) {
    ui::ListBoxItem* item = new ui::ListBoxItem(this);
    item->SetText(ui::StringUtil::Printf(_T("Item %d"), i));
    item->SetClass(_T("listitem"));
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
- 语言文件: `bin/resources/themes/default/lang/` (zh_CN.txt, en_US.txt)
- 示例源码: `examples/<example_name>/`
