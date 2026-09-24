---
name: nim-duilib-event-handler
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

// Tab选择 (TabBox/TabCtrl)：wParam 是新页索引，lParam 是旧页索引
tabCtrl->AttachTabSelect([](const ui::EventArgs& args) {
    const size_t newIndex = static_cast<size_t>(args.wParam);
    const size_t oldIndex = static_cast<size_t>(args.lParam);
    return true;
});

// 值变化 (Progress/Slider/DateTime/PropertyGridProperty)
slider->AttachEvent(ui::kEventValueChanged, [](const ui::EventArgs&) { return true; });

// 通用事件绑定（枚举名见下方"完整事件类型列表"，取值以 duilib/duilib_defs.h 中 enum EventType 为准）
control->AttachEvent(ui::kEventMouseEnter, handler);
control->AttachEvent(ui::kEventMouseLeave, handler);
control->AttachEvent(ui::kEventKeyDown, handler);
control->AttachEvent(ui::kEventSetFocus, handler);
control->AttachEvent(ui::kEventKillFocus, handler);
control->AttachEvent(ui::kEventWindowClose, handler);
```

### EventArgs 参数（定义见 duilib/Core/EventArgs.h）
```cpp
struct EventArgs
{
    EventType eventType;   // 事件类型（enum EventType: uint8_t）
    WPARAM    wParam;      // 参数1（Windows 下为 UINT_PTR，可传指针）
    LPARAM    lParam;      // 参数2（Windows 下为 LONG_PTR，可传指针）
    VirtualKeyCode vkCode; // 按键码（enum VirtualKeyCode，不是整数）
    UiPoint   ptMouse;     // 鼠标位置
    uint32_t  modifierKey; // 修饰键，取值见 duilib/Core/Keyboard.h 的 enum ModifierKey
    int32_t   eventData;   // 整型附加数据（是 int32_t，不是 int64_t）
    void*     pEventData;  // 指针附加数据
    int32_t   listCtrlType;// ListCtrl 专用：事件对应的视图类型

    void       SetSender(Control* pControl);
    Control*   GetSender() const;      // ← 取发送者用这个
    void       SetSenderWeakFlag(std::weak_ptr<WeakFlag> senderFlag);
    bool       IsSenderExpired() const;

private:
    Control* pSender;                  // private！外部不可直接访问
    std::weak_ptr<WeakFlag> m_senderFlag;
};
```
**取发送者控件必须写 `args.GetSender()`**（返回 nullptr 表示控件已销毁），写 `args.pSender` 编译不过。

```cpp
btn->AttachClick([this](const ui::EventArgs& args) {
    ui::Control* pSender = args.GetSender();   // ✅ 正确
    if (pSender == nullptr) {
        return true;                            // 控件已销毁
    }
    return true;
});
```

### 完整事件类型列表
键鼠: `kEventKeyDown`, `kEventKeyUp`, `kEventChar`, `kEventMouseEnter`, `kEventMouseLeave`, `kEventMouseMove`, `kEventMouseHover`, `kEventMouseWheel`, `kEventMouseButtonDown`, `kEventMouseButtonUp`, `kEventMouseDoubleClick`, `kEventMouseRButtonDown`, `kEventMouseRButtonUp`, `kEventMouseRDoubleClick`, `kEventMouseMButtonDown`, `kEventMouseMButtonUp`, `kEventMouseMDoubleClick`, `kEventContextMenu`

焦点/输入法: `kEventSetFocus`, `kEventKillFocus`, `kEventSetCursor`, `kEventCaptureChanged`, `kEventImeSetContext`, `kEventImeStartComposition`, `kEventImeComposition`, `kEventImeEndComposition`

操作: `kEventClick`, `kEventRClick`, `kEventSelect`, `kEventUnSelect`, `kEventCheck`, `kEventUnCheck`, `kEventTabSelect`, `kEventExpand`, `kEventCollapse`

编辑: `kEventTextChanged`, `kEventSelChanged`, `kEventReturn`, `kEventEsc`, `kEventTab`, `kEventZoom`, `kEventLinkClick`, `kEventScrollPosChanged`, `kEventValueChanged`

状态（注意都是过去式 `-ed`）: `kEventSetFocus`, `kEventKillFocus`, `kEventPosChanged`, `kEventSizeChanged`, `kEventVisibleChanged`, `kEventStateChanged`, `kEventSelectColor`

窗口: `kEventWindowSetFocus`, `kEventWindowKillFocus`, `kEventWindowPosChanged`, `kEventWindowClose`, `kEventWindowCreate`, `kEventWindowSize`, `kEventWindowMove`

拖放: `kEventDropEnter`, `kEventDropOver`, `kEventDropLeave`, `kEventDropData`

动画: `kEventImageAnimationStart`, `kEventImageAnimationPlayFrame`, `kEventImageAnimationStop`

加载: `kEventLoadingStart`, `kEventLoading`, `kEventLoadingStop`

> 命名陷阱: 勾选/文本/选择/值/可见性/状态/大小 相关事件都是**过去式**——`kEventCheck`（不是 Checked）、
> `kEventTextChanged`（不是 TextChange）、`kEventSelChanged`（不是 SelChange）、
> `kEventValueChanged`（不是 ValueChange）、`kEventVisibleChanged`（不是 VisibleChange）、
> `kEventStateChanged`（不是 StateChange）；控件尺寸变化是 `kEventSizeChanged`，**没有 `kEventResize`**。

### 注意事项
- 事件回调返回 `true` 表示已处理该事件
- 使用 `UiBind` 绑定成员函数确保弱引用安全
- lambda 中捕获 `this` 时用 `ui::UiBind(this, lambda)` 包装，防止控件销毁后回调崩溃
