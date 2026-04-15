---
name: nim-create-window
description: 创建 nim_duilib 新窗口（生成 C++ 窗口类 + XML 布局文件 + 更新主线程）
trigger: 当用户要求创建新窗口、新窗体、新对话框、新界面时触发
---

# nim_duilib 创建新窗口

## 执行步骤

### 1. 确认参数
向用户确认以下信息（如果未提供）:
- **窗口名称**: C++ 类名（如 `SettingsForm`）
- **皮肤目录名**: XML文件所在目录名（如 `settings`）
- **窗口标题**: 窗口显示的标题文字
- **窗口尺寸**: 默认 "800,600"
- **是否需要标题栏**: 默认 Yes（带最小化/最大化/关闭按钮）

### 2. 生成 C++ 头文件
在 `examples/<project>/` 或用户指定位置创建 `<FormName>.h`:

```cpp
#ifndef EXAMPLES_<FORM_NAME_UPPER>_H_
#define EXAMPLES_<FORM_NAME_UPPER>_H_

#include "duilib/duilib.h"

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

### 3. 生成 C++ 实现文件
创建 `<FormName>.cpp`:

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

### 4. 生成 XML 布局文件
在 `bin/resources/themes/default/<skin_folder>/` 创建 `<skin_file>.xml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Window size="<width>,<height>" min_size="240,100"
        caption="0,0,0,36" use_system_caption="false"
        snap_layout_menu="true" sys_menu="true" sys_menu_rect="0,0,36,36"
        shadow_type="default" shadow_attached="true"
        layered_window="true" alpha="255" size_box="4,4,4,4"
        icon="../public/caption/logo.ico">
    <VBox bkcolor="bk_wnd_darkcolor">
        <!-- 标题栏 -->
        <HBox name="window_caption_bar" width="stretch" height="36" bkcolor="bk_wnd_lightcolor">
            <Label text="<窗口标题>" margin="12,0,0,0" valign="center" normal_text_color="white"/>
            <Control />
            <Button class="btn_wnd_min_11" height="32" width="40" name="minbtn" margin="0,2,0,2"/>
            <Box height="stretch" width="40" margin="0,2,0,2">
                <Button class="btn_wnd_max_11" height="32" width="stretch" name="maxbtn"/>
                <Button class="btn_wnd_restore_11" height="32" width="stretch" name="restorebtn" visible="false"/>
            </Box>
            <Button class="btn_wnd_close_11" height="stretch" width="40" name="closebtn" margin="0,0,0,2"/>
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

### 5. 创建窗口的代码
在需要打开此窗口的地方添加:

```cpp
#include "<FormName>.h"

// 创建并显示窗口
<FormName>* window = new <FormName>();
window->CreateWnd(nullptr, ui::WindowCreateParam(_T("<窗口标题>"), true));
window->ShowWindow(ui::kSW_SHOW_NORMAL);
// 如果是主窗口，添加:
// window->PostQuitMsgWhenClosed(true);
```

### 6. 检查清单
- [ ] .h 和 .cpp 文件已添加到 VS 工程或 CMakeLists.txt
- [ ] XML 文件编码为 UTF-8
- [ ] XML 文件路径与 GetSkinFolder()/GetSkinFile() 返回值一致
- [ ] 如果是示例工程，需要在 vcxproj 中添加引用
