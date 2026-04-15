---
name: nim-add-control
description: 向 nim_duilib 界面添加控件（XML + C++ 事件绑定）
trigger: 当用户要求添加按钮、输入框、列表、复选框等控件时触发
---

# nim_duilib 添加控件

## 执行步骤

### 1. 确认控件需求
- 控件类型（按钮、输入框、列表等）
- 放置位置（哪个XML文件、哪个容器内）
- 是否需要 C++ 事件处理

### 2. 控件 XML 速查

**按钮 Button:**
```xml
<Button name="btn_submit" text="提交" width="80" height="32"
        class="btn_global_blue_80x30" tooltip_text="点击提交"/>
```

**文本标签 Label:**
```xml
<Label name="lbl_info" text="信息文本" font="system_14"
       normal_text_color="default_font_color" text_align="left,vcenter"/>
```

**输入框 RichEdit (单行):**
```xml
<RichEdit name="edit_input" width="200" height="30"
          single_line="true" prompt_text="请输入..."
          font="system_12" text_padding="4,2,4,2"
          border_size="1" border_color="gray" border_round="4,4"
          hot_border_color="blue" focus_border_color="blue"/>
```

**输入框 RichEdit (多行):**
```xml
<RichEdit name="edit_content" width="stretch" height="200"
          multi_line="true" word_wrap="true" vscrollbar="true"
          want_return="true" prompt_text="请输入内容..."
          border_size="1" border_color="gray"/>
```

**密码输入框:**
```xml
<RichEdit name="edit_pwd" width="200" height="30"
          single_line="true" password="true" prompt_text="请输入密码"
          border_size="1" border_color="gray" border_round="4,4"/>
```

**复选框 CheckBox:**
```xml
<CheckBox name="chk_agree" text="我同意协议" class="checkbox_2"/>
```

**单选按钮 Option:**
```xml
<Option name="opt_male" text="男" group="gender" class="option_1" selected="true"/>
<Option name="opt_female" text="女" group="gender" class="option_1"/>
```

**下拉框 Combo:**
```xml
<Combo name="combo_type" class="combo" width="150" height="30"/>
```

**进度条 Progress:**
```xml
<Progress name="prog_download" width="stretch" height="16"
          min="0" max="100" value="0"
          bkcolor="lightgray" progress_color="blue" border_round="8,8"/>
```

**滑块 Slider:**
```xml
<Slider name="slider_volume" width="200" height="20"
        min="0" max="100" value="50" step="1"/>
```

**列表 VListBox:**
```xml
<VListBox name="list_items" width="stretch" height="stretch"
          vscrollbar="true" class="list"/>
```

**树形控件 TreeView:**
```xml
<TreeView name="tree_files" width="stretch" height="stretch"
          vscrollbar="true" class="tree_view"/>
```

**分隔线 Line:**
```xml
<Line line_color="gray" line_width="1" height="1"/>           <!-- 水平线 -->
<Line vertical="true" line_color="gray" line_width="1" width="1"/>  <!-- 垂直线 -->
```

**日期选择 DateTime:**
```xml
<DateTime name="dt_start" width="160" height="30"
          format="%Y-%m-%d" edit_format="date_calendar"/>
```

**超级链接 HyperLink:**
```xml
<HyperLink text="访问官网" url="https://example.com"
           normal_text_color="blue" cursor_type="hand"/>
```

**IP地址 IPAddress:**
```xml
<IPAddress name="ip_server" width="200" height="30" ip="192.168.1.1"/>
```

**TabCtrl + TabBox 页签:**
```xml
<VBox>
    <TabCtrl name="tab_ctrl" height="32">
        <TabCtrlItem text="页签1" selected="true"/>
        <TabCtrlItem text="页签2"/>
    </TabCtrl>
    <TabBox name="tab_box">
        <VBox><!-- 页签1内容 --></VBox>
        <VBox visible="false"><!-- 页签2内容 --></VBox>
    </TabBox>
</VBox>
```

### 3. C++ 事件绑定代码

在窗口的 `OnInitWindow()` 中添加:

```cpp
// 按钮点击
if (auto* btn = dynamic_cast<ui::Button*>(FindControl(_T("btn_submit")))) {
    btn->AttachClick([this](const ui::EventArgs&) {
        // 处理点击
        return true;
    });
}

// 复选框
if (auto* chk = dynamic_cast<ui::CheckBox*>(FindControl(_T("chk_agree")))) {
    chk->AttachSelect([this](const ui::EventArgs&) { /* 选中 */ return true; });
    chk->AttachUnSelect([this](const ui::EventArgs&) { /* 取消 */ return true; });
}

// 输入框文本变化
if (auto* edit = dynamic_cast<ui::RichEdit*>(FindControl(_T("edit_input")))) {
    edit->AttachTextChange([this](const ui::EventArgs&) {
        // 文本变化
        return true;
    });
}

// 下拉框选择变化
if (auto* combo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_type")))) {
    combo->AttachSelect([this](const ui::EventArgs& args) {
        size_t selIndex = args.wParam;
        return true;
    });
}

// 列表选择
if (auto* list = dynamic_cast<ui::ListBox*>(FindControl(_T("list_items")))) {
    list->AttachSelect([this](const ui::EventArgs& args) {
        size_t newSel = args.wParam;
        return true;
    });
}
```

### 4. 动态添加数据

```cpp
// Combo 添加选项
if (auto* combo = dynamic_cast<ui::Combo*>(FindControl(_T("combo_type")))) {
    auto* treeView = combo->GetTreeView();
    auto* root = treeView->GetRootNode();
    for (int i = 0; i < 5; i++) {
        auto* node = new ui::TreeNode(this);
        node->SetClass(_T("tree_node"));
        node->SetText(ui::StringUtil::Printf(_T("选项 %d"), i));
        root->AddChildNode(node);
    }
    combo->SetCurSel(0);
}

// ListBox 添加项
if (auto* list = dynamic_cast<ui::ListBox*>(FindControl(_T("list_items")))) {
    for (int i = 0; i < 20; i++) {
        auto* item = new ui::ListBoxItem(this);
        item->SetClass(_T("listitem"));
        item->SetText(ui::StringUtil::Printf(_T("列表项 %d"), i));
        item->SetFixedHeight(ui::UiFixedInt(28), true, true);
        list->AddItem(item);
    }
}
```
