<!-- verify:allow-missing btn_default -->
<!-- btn_default 是自定义样式的举例名，verify_docs.py 不应判为漂移。 -->

# nim_duilib LLM 快速参考

> 本文档专为 LLM/AI 代理优化，提供 nim_duilib 的核心 API 速查。

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
  <VBox bkcolor="bk_wnd_darkcolor">
    <!-- 标题栏（name 必须是 window_title_bar；旧名 window_caption_bar 仅为兼容 fallback） -->
    <HBox name="window_title_bar" width="stretch" height="36" bkcolor="bk_wnd_lightcolor">
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
  `SetCollapsible()`、`SetCollapseAnimMillSeconds()`、`SetGroup()`/`GetGroup()`、
  `SetArrowAlign()`/`GetArrowAlign()`、`SetTitleTextHAlign()`、`SetArrowStateImage()`、`SetTitleSlotName()`。
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
| prompt_mode | false | bool | 是否显示占位提示文字 |
| prompt_text | | string | 占位提示文字（text 为空时显示） |
| prompt_text_id | | string | 占位提示文字的多语言 ID |
| prompt_color | | string | 提示文字颜色 |
| word_wrap | false | bool | 自动换行 |
| vscrollbar / hscrollbar | false | bool | 滚动条 |
| want_return | false | bool | 接受回车 |
| want_tab | false | bool | 接受Tab |
| caret_color | | string | 光标颜色 |
| normal_text_color | | string | 文字颜色 |

### SpinBox 属性(继承 RichEdit，数字输入框)
| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| step | 1 | int | 步长（步进按钮/上下方向键每次调整值） |
| value | 0 | int | 初始值（超范围时自动修正） |
| spin_class | | string | 步进按钮样式（同 RichEdit） |
| min_number / max_number | | int | 数字范围 |

### SearchBox 属性(继承 HBox，搜索框组合控件)
左图标 + 内部 RichEdit + 清除按钮（`duilib/Control/SearchBox.h`），XML 节点名 `SearchBox`。
| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| text | | string | 初始文本 |
| prompt_text | | string | 空内容时的占位提示文本 |
| prompt_text_id | | string | 占位提示文本的多语言字符串 ID |
| prompt_color | | string | 占位提示文本颜色（语义色名） |

C++ 接口：`SetSearchText/GetSearchText`、`GetEditControl()`。事件：`kEventTextChanged`、`kEventReturn`（回车搜索）。
清除按钮复用 RichEdit `clear_btn_class` 机制（聚焦有文本时显示，点击清空，失焦隐藏）。
皮肤类：`search_box` / `search_box_icon` / `search_box_edit` / `search_box_clear_btn`。

### Switch 属性(继承 CheckBox，滑块开关)
轨道 + 圆形滑块 + 滑动/淡入淡出动画的开关控件（`duilib/Control/Switch.h`），XML 节点名 `Switch`。轨道与滑块均支持 SVG 图片皮肤（off/on 成对，按进度交叉淡入、滑块平移）；无图时用语义色自绘。
| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| switch_animation_ms / animation_ms | 160 | int | 动画时长（毫秒），0 表示无动画 |
| track_off_color / trackoncolor | `bg_switch_track_off` | string | 轨道未选中颜色（无轨道图时生效） |
| track_on_color / trackoncolor | `bg_switch_track_on` | string | 轨道选中颜色 |
| thumb_off_color / thumboffcolor | `bg_switch_thumb_off` | string | 滑块未选中颜色（无滑块图时生效） |
| thumb_on_color / thumboncolor | `bg_switch_thumb_on` | string | 滑块选中颜色 |
| track_off_image / trackoffimage | | string | 轨道未选中图片（与 track_on_image 成对） |
| track_on_image / trackonimage | | string | 轨道选中图片（拉伸铺满轨道矩形） |
| thumb_off_image / thumboffimage | | string | 滑块未选中图片（方形，与 thumb_on_image 成对） |
| thumb_on_image / thumbonimage | | string | 滑块选中图片（方形框边长=轨道高，阴影留白画在图内） |
| thumb_padding | 0 | int | 滑块方形框距轨道边缘距离（自动 DPI 缩放） |

图片属性串不要带 width/height/valign/halign（否则不平移）；颜色用 `svg_replace_colors` 替换适配深浅色。
C++ 接口：`SetAnimationDuration`、`SetTrackOffColor/OnColor/OffImage/OnImage`、`SetThumbOffColor/OnColor/OffImage/OnImage`、`SetThumbPadding`。事件：`kEventSelect` / `kEventUnSelect`。
三套皮肤类（SVG 在 `public/switch/`）：`switch`（别名 `switch_ios`，iOS 风 51x31 白滑块带投影）、`switch_fluent`（Win11 风 48x24 描边小轨道+14px 滑块）、`switch_material`（Material3 风 52x32，选中带对勾大滑块）。

### Badge 属性(继承 Label，角标)
TabCtrl 标签、按钮上的未读数/小红点控件（`duilib/Control/Badge.h`），XML 节点名 `Badge`。数字角标=圆角胶囊+数字（超上限显示"99+"）；红点角标（dot 模式）=纯小圆点。继承 Label 文本属性。
| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| count / badge_count | 0 | int64 | 角标数量：>0 显示，<=0 自动隐藏 |
| max_count / maxcount | 99 | int64 | 数量上限，超过显示"上限+"（如"99+"） |
| dot | false | bool | 红点模式：true 为纯小圆点（显隐由 visible 控制） |
| badge_color / badgecolor | `bg_badge` | string | 角标背景颜色（语义色名或颜色值） |

悬浮宿主控件角上用 `float="true"`+`margin` 定位；子控件超出父容器会被裁剪，父容器需留出角标外露宽度。Badge `mouse_enabled="false"` 不挡宿主点击。
皮肤类：`badge`（高 18 白字红底宽自适应）/ `badge_dot`（8x8 红点）。语义色：`bg_badge`（派生自 `color_error`）、`text_badge`（白）。
C++ 接口：`SetCount/GetCount`、`SetMaxCount/GetMaxCount`、`SetDotMode/IsDotMode`、`SetBadgeColor/GetBadgeColor`。
示例：`<Badge class="badge" count="5"/>`、`<Badge class="badge_dot"/>`；按钮角标 `<Box width="66" height="30"><Button .../><Badge class="badge" count="6" float="true" margin="42,0,0,0"/></Box>`。

TabCtrlItem 内置角标（`duilib/Control/TabCtrl.h`，懒创建 Badge 子控件，排在标题后）：属性 `badge_count`（0，>0 显示 <=0 隐藏）、`badge_max_count`（99）、`badge_dot`（false）、`badge_class`（空则不创建）。皮肤类 `tab_ctrl_item_badge`（数字，已挂在 `tab_ctrl_item`）/ `tab_ctrl_item_badge_dot`（8x8 红点，dot 模式须用此类）。C++：`SetBadgeCount/GetBadgeCount`、`SetBadgeMaxCount/GetBadgeMaxCount`、`SetBadgeDot/IsBadgeDot`、`SetBadgeClass/GetBadgeClass`、`GetBadgeControl`。示例：`<TabCtrlItem class="tab_ctrl_item" title="消息" badge_count="5"/>`。

### Flyout 浮层窗口(继承 WindowImplBase)
`duilib/Control/Flyout.h`（duilib.h 已 include）。锚点周围浮出任意 Box 内容；非模态、关闭后框架自动 delete；默认不抢焦点（WS_EX_NOACTIVATE）、点击外部/Esc 自动关闭、8 方位空间不足自动翻转+工作区夹持、全局单活。
C++：`Flyout(Window*)` → `SetSkinFolder(path)`（默认父窗口 GetResourcePath）→ `bool ShowAt(Control* anchor, xmlFile, Placement=Bottom)`（**xmlFile 只传文件名**；失败内部已 delete this 返回 false）；`Dismiss()`、`SetAutoDismiss`(true)、`SetNoFocus`(true)、`SetGap`(6 DIP)、`SetAllowFlip`(true)、`GetAnchor/GetPlacement/IsOpen`、`AttachOpened/AttachClosed`（`std::function<void(CloseReason)>`）、静态 `GetActiveFlyout/DismissActive`。
`Placement{Bottom,BottomEnd,Top,TopEnd,Right,RightEnd,Left,LeftEnd}`；`CloseReason{kManual,kClickOutside,kEscape,kAnchorLost}`（父窗口销毁/隐藏/最小化、锚点失效、DPI 变化）。
内容 XML：`<Window size="240,210" caption="0,0,0,0" use_system_caption="false" shadow_type="default" shadow_attached="true" layered_window="true" size_box="0,0,0,0">`，固定 DIP 尺寸（内容超出会画进阴影区）。皮肤类 `flyout`（240 宽 bg_window_card/border_window/圆角8/padding12）、`flyout_title`（bold14）、`flyout_desc`（regular12 text_muted 多行）。
ShowAt 成功后再 `FindControl` 绑事件；同锚点切换关闭由调用方比较 `GetActiveFlyout()->GetAnchor()`；外部点击检测仅 Windows（50ms 轮询），非 Win 平台 SetNoFocus(false)；锚点在滚动容器内自动扣除累计滚动偏移；父窗口 OpenColorTheme 私有颜色主题自动继承（Window::GetColorThemeXmlData）。

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

### DateTime 属性(继承 Label，日期时间选择器)

| 属性 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| format | | string | 日期格式，具体可参考 `DateTime.h` 中 `SetStringFormat` 的说明（基于 `std::put_time`） |
| edit_format | "date_calendar" | string | 编辑格式：`date_calendar` / `date_up_down` / `date_time_up_down` / `date_minute_up_down` / `time_up_down` / `minute_up_down` |
| spin_class | | string | Spin 控件的 Class 属性（容器,上按钮,下按钮），仅非 `date_calendar` 编辑格式生效 |
| current_time | | bool | 初始化为当前本地时间（`"true"` 或 `"1"` 时生效），无需在代码中手动调用 `InitLocalTime()`；与 `text` 同时使用时 `current_time` 优先 |

C++ 接口：`InitLocalTime()`（设为当前时间）、`ClearTime()`、`SetDateTime/GetDateTime(tm)`、`SetDateTimeString/GetDateTimeString()`、`SetStringFormat/GetStringFormat()`、`SetEditFormat/GetEditFormat()`、`SetSpinClass/GetSpinClass()`。值变化触发 `kEventValueChanged`。

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

完整的颜色清单与命名规范见 `docs/ThemeColor.md`。深色主题取值不同，**写业务 XML 时一律用语义色名，不要写死色值**。

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
- DoModal 下 Enter 键能派发给焦点按钮，依赖 Windows 平台的
  `NativeWindow_Windows::SetEnterKeyPassthrough(HWND, bool)` 注册机制：
  `OnInitWindow` 注册、`OnPreCloseWindow` 注销。DoModal 的消息循环会先调 `IsDialogMessage`，
  默认把 Enter 转成 `WM_COMMAND IDOK`、吞掉 TAB；框架 hook 对 TAB 全局放行，
  对 Enter 仅放行了注册窗口——自定义模态窗口有同样需求时才注册，不要全局放行，
  以免改变普通模态对话框"回车关闭"的默认行为。
- `WindowBase::DoModal(pParentWindow, createParam, bCloseByEsc=true, bCloseByEnter=false)`
  默认 ESC 关窗、Enter 不关窗；只有传 `bCloseByEnter=true` 时 IDOK 才会关闭窗口。

### 非模态通知框 ToastWnd

框架自带的自绘皮肤非模态通知框（Toast），头文件 `duilib/Utils/ToastWnd.h`
（已在 `duilib/duilib.h` 中 include），皮肤
`bin/resources/themes/default/public/toast/toast.xml`，
完整演示见 `examples/controls`（ToastForm）。非模态、不抢焦点，到时自动消失，
多条垂直堆叠，点击立即关闭，鼠标悬停暂停倒计时、移开按剩余时间继续。
fire-and-forget：只能通过静态 `Show` 使用，窗口对象关闭后由框架自动销毁。

```cpp
#include "duilib/duilib.h"

//基本用法：默认顶部居中、停留 3000ms
ui::ToastWnd::Show(this, _T("保存成功"), ui::ToastWnd::kTypeSuccess);

//错误通知，停留 5 秒
ui::ToastWnd::Show(this, _T("网络连接失败，请稍后重试。"),
                   ui::ToastWnd::kTypeError, 5000);

//右下角通知
ui::ToastWnd::Show(this, _T("文件已下载完成"),
                   ui::ToastWnd::kTypeInfo, 4000, ui::ToastWnd::kPosBottomRight);

//nDurationMs 传 0 = 不自动关闭，只能点击关闭
ui::ToastWnd::Show(this, _T("重要提示，需手动关闭"), ui::ToastWnd::kTypeWarning, 0);

//多语言 ID（切换语言后实时刷新）
ui::ToastWnd::Show(this, _T("STRID_TOAST_DEMO_INFO"),
                   ui::ToastWnd::kTypeInfo, 3000, ui::ToastWnd::kPosTop, true);
```

`Show(pParentWindow, text, type=kTypeInfo, nDurationMs=3000, position=kPosTop, bTextId=false)`：

| 参数 | 说明 |
|------|------|
| pParentWindow | 父窗口，通知显示在其客户区附近，显示期间父窗口仍可操作；nullptr 时按显示器工作区定位 |
| text | 通知内容，支持 `\n` 换行，长文本自动换行，窗口高度自适应 |
| type | `kTypeInfo`（蓝 i）/ `kTypeSuccess`（绿对勾）/ `kTypeWarning`（黄 !）/ `kTypeError`（红 X） |
| nDurationMs | 停留毫秒数，默认 3000；**0 表示不自动关闭** |
| position | `kPosTop`(默认) / `kPosCenter` / `kPosBottom` / `kPosTopRight` / `kPosBottomRight` |
| bTextId | 为 true 时 text 按多语言 ID 解析 |

**带操作按钮（ActionButton）**：除整条点击关闭外，还可传入一组操作按钮，对齐 Win32 通知的
ActionButton 能力。调用 7 参数 `Show` 重载，额外传入 `std::vector<ToastAction>`：

```cpp
//带操作按钮的通知（如"知道了 / 查看详情"），对齐 Win32 通知的 ActionButton
std::vector<ui::ToastAction> actions;
ui::ToastAction gotIt;
gotIt.text = _T("STRID_TOAST_ACTION_GOTIT");   //按钮文本（bTextId=true 时按多语言 ID 解析）
gotIt.bTextId = true;
gotIt.callback = [&]() { /* 点击"知道了"后执行的逻辑 */ };  //回调为空则点击仅关闭通知
actions.push_back(gotIt);

ui::ToastAction detail;
detail.text = _T("STRID_TOAST_ACTION_DETAIL");
detail.bTextId = true;
detail.callback = [&]() { /* 点击"查看详情"后执行的逻辑 */ };
actions.push_back(detail);

ui::ToastWnd::Show(this, _T("STRID_TOAST_DEMO_ACTION_TEXT"),
                   ui::ToastWnd::kTypeSuccess, actions, 6000, ui::ToastWnd::kPosTop, true);
```

`ToastAction` 结构：

| 字段 | 说明 |
|------|------|
| text | 按钮文本（bTextId=true 时按多语言 ID 解析） |
| callback | 点击回调（`StdClosure`，可为空）；执行后通知自动关闭 |
| bTextId | 为 true 时 text 按多语言 ID 解析 |

操作按钮行为要点：

- 操作按钮位于通知底部，默认隐藏；仅当传入非空 `actions` 时动态创建并显示，无操作按钮时布局与行为完全不变。
- 按钮 `mouse_enabled=true`，点击只触发自身回调（执行后关闭通知），**不会**冒泡到根容器的"整条点击关闭"；点击通知其它非交互区域仍整条关闭。
- 按钮样式与日历"今天/清除"按钮同款（圆角边框 + 悬停/按下背景填充），颜色跟随按钮状态。

要点：

- 堆叠规则：同一父窗口、同一 position 的通知垂直堆叠（间距 12 DIP，新的在后面），
  某条关闭后其余平滑上移补齐；同屏上限 5 条（`kMaxToastCount`），超出后最早的先退场。
- 倒计时用 `ThreadManager::PostDelayedTask` + `steady_clock` 时间戳实现，
  悬停暂停只暂停剩余时间，多次移入移出不会重置总时长。
- 动画用 `AnimationPlayer`（`AnimationType::kAnimationNone` 回调驱动）：
  入场 200ms 淡入+上移 12DIP（EaseOutCubic），退场 180ms 淡出（EaseInCubic），
  分层窗口透明度用 `SetLayeredWindowAlpha`。
- 窗口样式 `kWS_POPUP|kWS_EX_TOPMOST|kWS_EX_LAYERED|kWS_EX_NOACTIVATE`，
  无父窗口时再加 `kWS_EX_TOOLWINDOW`（不进任务栏/Alt+Tab）。
- **四个实现坑（自写类似交互窗口时务必注意）**：
  ① 普通容器不派发 `kEventClick`，整条可点要用 `AttachButtonUp`（kEventMouseButtonUp）；
  ② 有阴影的窗口 `GetRoot()` 返回的 ShadowBox 已被 `SetMouseEnabled(false)`，
     鼠标事件必须绑在 `GetXmlRoot()`（XML 可见根容器）上；尺寸测量仍用 `GetRoot()`
     （EstimateSize 含阴影、GetPadding() 是阴影边距）；
  ③ 鼠标直接移出窗口边界时控件级 kEventMouseLeave 不派发，需重写窗口级
     `OnMouseLeaveMsg(const NativeMsg&, bool&)` 恢复倒计时；
  ④ 高度自适应窗口中的多行文本不要用 `width="stretch"`——EstimateSize 时 stretch
     子项按容器可用宽（未扣除固定宽兄弟）换行，英文按词换行会少算行数导致末行
     被裁；Label 宽度写与实际布局相等的固定值（toast.xml 为 296）。

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
- 示例源码: `examples/<example_name>/`
