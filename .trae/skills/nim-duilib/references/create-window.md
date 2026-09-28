# 创建新窗口

## 推荐做法：复制模板再改

`assets/templates/` 下有可直接使用的骨架（`CMakeLists.txt`、`MyForm.h`、`MyForm.cpp`、`my_form.xml`）。
复制后批量替换标识符，比照着文档重写可靠得多：

1. 复制四个文件：`CMakeLists.txt` 与 `*.h`/`*.cpp` 放 `examples/<project>/`，
   `my_form.xml` 放 `bin/resources/themes/default/<skin_folder>/`。
2. 替换 `MyForm` → `<FormName>`、`MY_FORM_H_` → 对应的宏、`my_skin` → 皮肤目录名、
   `my_form.xml` → 布局文件名；`CMakeLists.txt` 里的 `<project_name>` 换成工程目录名。
3. 按下面第 5 步补上创建窗口的代码。
4. 把源文件加入构建（见检查清单）。

## 1. 确认参数

向用户确认以下信息（如果未提供）:

- **窗口名称**: C++ 类名（如 `SettingsForm`）
- **皮肤目录名**: XML文件所在目录名（如 `settings`）
- **窗口标题**: 窗口显示的标题文字
- **窗口尺寸**: 默认 "800,600"
- **是否需要标题栏**: 默认 Yes（带最小化/最大化/关闭按钮）

## 2. C++ 头文件

```cpp
#ifndef EXAMPLES_<FORM_NAME_UPPER>_H_
#define EXAMPLES_<FORM_NAME_UPPER>_H_

#include "duilib/duilib.h"

// 基类 ui::WindowImplBase 声明在 duilib/Utils/WinImplBase.h（文件名不带 Window 前缀）
class <FormName> : public ui::WindowImplBase
{
    typedef ui::WindowImplBase BaseClass;
public:
    <FormName>();
    virtual ~<FormName>() override;

    virtual DString GetSkinFolder() override;
    virtual DString GetSkinFile() override;
    virtual void OnInitWindow() override;
};

#endif // EXAMPLES_<FORM_NAME_UPPER>_H_
```

## 3. C++ 实现文件

```cpp
#include "<FormName>.h"

<FormName>::<FormName>() {}
<FormName>::~<FormName>() {}

DString <FormName>::GetSkinFolder()
{
    return _T("<skin_folder>");
}

DString <FormName>::GetSkinFile()
{
    return _T("<skin_file>.xml");
}

void <FormName>::OnInitWindow()
{
    BaseClass::OnInitWindow();
    // TODO: 初始化控件和绑定事件
}
```

## 4. XML 布局文件

在 `bin/resources/themes/default/<skin_folder>/` 创建 `<skin_file>.xml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="<width>,<height>" min_size="240,100"
        caption="0,0,0,36" use_system_caption="false"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_attached="true"
        layered_window="true" alpha="255" size_box="4,4,4,4"
        icon="public/caption/logo.ico">
    <VBox bkcolor="bg_window_main">
        <!-- 标题栏：name 必须用 window_title_bar + btn_window_*（旧名仅兼容 fallback） -->
        <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bg_titlebar">
            <Label text="<窗口标题>" margin="12,0,0,0" valign="center" normal_text_color="white"/>
            <Control />
            <Button class="btn_wnd_min_11" height="32" width="40" name="btn_window_min" margin="0,2,0,2"/>
            <Box height="stretch" width="40" margin="0,2,0,2">
                <Button class="btn_wnd_max_11" height="32" width="stretch" name="btn_window_max"/>
                <Button class="btn_wnd_restore_11" height="32" width="stretch" name="btn_window_restore" visible="false"/>
            </Box>
            <Button class="btn_wnd_close_11" height="stretch" width="40" name="btn_window_close" margin="0,0,0,2"/>
        </HBox>
        <!-- 内容区域 -->
        <Box padding="12,12,12,12">
            <VBox>
                <!-- TODO: 在这里添加内容控件 -->
                <Label text="<窗口标题>" text_align="hcenter,vcenter" height="100%" width="100%"/>
            </VBox>
        </Box>
    </VBox>
</Window>
```

## 5. 创建并显示窗口

```cpp
#include "<FormName>.h"

// 创建并显示窗口
<FormName>* window = new <FormName>();
window->CreateWnd(nullptr, ui::WindowCreateParam(_T("<窗口标题>"), true));
window->ShowWindow(ui::kSW_SHOW_NORMAL);
// 如果是主窗口，添加:
// window->PostQuitMsgWhenClosed(true);
```

窗口用 `new` 创建后**不要手动 `delete`**——框架在 `OnFinalMessage` 时自动释放。
主线程模板与资源初始化见 `api-reference.md` 第七节。

## 6. 检查清单

- [ ] .h 和 .cpp 文件已添加到 VS 工程或 CMakeLists.txt
- [ ] XML 文件编码为 UTF-8
- [ ] XML 文件路径与 GetSkinFolder()/GetSkinFile() 返回值一致
- [ ] 如果是示例工程，需要在 vcxproj 中添加引用
- [ ] 标题栏按钮用的是 `btn_window_*` 新名，不是 `minbtn`/`closebtn` 旧名
- [ ] `DUILIB_SKIA_LIB_SUBPATH` 的默认值写在 `include(duilib_common.cmake)` **之前**
      （该脚本用 `option()` 声明它，include 后必然已定义，之后再判断"未定义"永远是假）
