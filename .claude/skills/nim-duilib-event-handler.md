---
name: nim-event-handler
description: 为 nim_duilib 控件添加事件处理（支持 XML 内联事件和 C++ 事件绑定）
trigger: 当用户需要处理控件事件、响应用户操作、绑定回调时触发
---

# nim_duilib 事件处理

## 方式一: XML 内联事件（无需 C++ 代码）

### 语法
```xml
<控件 name="source">
    <Event type="事件类型" receiver="目标控件名" apply_attribute="属性名='属性值'"/>
</控件>
```

### receiver 写法
| 格式 | 含义 |
|------|------|
| `receiver="target_name"` | 按名称在窗口中查找控件 |
| `receiver="./target_name"` | 在当前容器内查找 |
| `receiver=""` | 控件自身 |
| `receiver="#window#"` | 窗口对象 |
| `receiver="name1 name2"` | 多个控件(空格分隔) |

### 常见示例

**点击按钮显示/隐藏面板:**
```xml
<Button name="btn_toggle" text="切换">
    <Event type="click" receiver="panel" apply_attribute="visible='true'"/>
</Button>
```

**单选切换内容:**
```xml
<Option group="tabs" text="页签1" selected="true">
    <Event type="select" receiver="page1" apply_attribute="visible='true'"/>
    <Event type="select" receiver="page2" apply_attribute="visible='false'"/>
</Option>
<Option group="tabs" text="页签2">
    <Event type="select" receiver="page1" apply_attribute="visible='false'"/>
    <Event type="select" receiver="page2" apply_attribute="visible='true'"/>
</Option>
```

**鼠标悬浮切换样式:**
```xml
<Control name="card" bkcolor="white">
    <Event type="mouse_enter" receiver="" apply_attribute="bkcolor='AliceBlue'"/>
    <Event type="mouse_leave" receiver="" apply_attribute="bkcolor='white'"/>
</Control>
```

**鼠标悬浮播放动画:**
```xml
<Control bkimage="file='fan.gif' auto_play='false' play_count='-1'">
    <Event type="mouse_enter" receiver="" apply_attribute="start_image_animation={}"/>
    <Event type="mouse_leave" receiver="" apply_attribute="stop_image_animation={}"/>
</Control>
```

**修改窗口阴影类型:**
```xml
<Option group="shadow" text="大阴影">
    <Event type="select" receiver="#window#" apply_attribute="shadow_type={big}"/>
</Option>
```

**修改容器布局属性:**
```xml
<Option group="align" text="居中对齐">
    <Event type="select" receiver="container" apply_attribute="child_halign='center'"/>
</Option>
```

## 方式二: C++ 事件绑定

### 常用 Attach 方法
```cpp
// 在 OnInitWindow() 中绑定

// 点击
control->AttachClick([this](const ui::EventArgs& args) -> bool {
    return true;
});

// 选中/取消选中 (CheckBox/Option/ListBox)
control->AttachSelect([](const ui::EventArgs&) { return true; });
control->AttachUnSelect([](const ui::EventArgs&) { return true; });

// 文本变化 (RichEdit)
edit->AttachTextChange([](const ui::EventArgs&) { return true; });

// 回车键 (RichEdit)
edit->AttachReturn([](const ui::EventArgs&) { return true; });

// Tab选择 (TabCtrl)
tabCtrl->AttachTabSelect([](const ui::EventArgs& args) {
    size_t newIndex = args.wParam;
    size_t oldIndex = args.lParam;
    return true;
});

// 值变化 (Progress/Slider)
slider->AttachEvent(ui::kEventValueChange, [](const ui::EventArgs&) { return true; });

// 通用事件绑定
control->AttachEvent(ui::kEventMouseEnter, handler);
control->AttachEvent(ui::kEventMouseLeave, handler);
control->AttachEvent(ui::kEventKeyDown, handler);
control->AttachEvent(ui::kEventSetFocus, handler);
control->AttachEvent(ui::kEventKillFocus, handler);
control->AttachEvent(ui::kEventWindowClose, handler);
```

### EventArgs 参数
```cpp
struct EventArgs {
    EventType eventType;     // 事件类型
    size_t wParam;           // 参数1
    size_t lParam;           // 参数2
    UiPoint ptMouse;         // 鼠标位置
    uint16_t vkCode;         // 按键码
    uint16_t modifierKey;    // 修饰键
    int64_t eventData;       // 额外数据
    Control* pSender;        // 发送控件
};
```

### 完整事件类型列表
键鼠: `kEventKeyDown`, `kEventKeyUp`, `kEventChar`, `kEventMouseEnter`, `kEventMouseLeave`, `kEventMouseMove`, `kEventMouseButtonDown`, `kEventMouseButtonUp`, `kEventMouseDoubleClick`, `kEventMouseRButtonDown`, `kEventMouseRButtonUp`, `kEventMouseWheel`, `kEventContextMenu`

操作: `kEventClick`, `kEventRClick`, `kEventSelect`, `kEventUnSelect`, `kEventChecked`, `kEventUnCheck`, `kEventTabSelect`, `kEventExpand`, `kEventCollapse`

编辑: `kEventTextChange`, `kEventSelChange`, `kEventReturn`, `kEventTab`, `kEventZoom`

状态: `kEventSetFocus`, `kEventKillFocus`, `kEventStateChange`, `kEventVisibleChange`, `kEventResize`

窗口: `kEventWindowClose`, `kEventWindowSize`, `kEventWindowMove`, `kEventWindowKillFocus`

拖放: `kEventDropEnter`, `kEventDropOver`, `kEventDropLeave`, `kEventDropData`

动画: `kEventImageAnimationStart`, `kEventImageAnimationStop`

### 注意事项
- 事件回调返回 `true` 表示已处理该事件
- 使用 `UiBind` 绑定成员函数确保弱引用安全
- lambda 中捕获 `this` 时用 `ui::UiBind(this, lambda)` 包装，防止控件销毁后回调崩溃
