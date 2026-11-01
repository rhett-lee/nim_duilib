# Spec: D-14 Popover 气泡卡片

## 1. 目标

新增独立控件 `Popover`（XML 节点名 `Popover`，头文件 `duilib/Control/Popover.h`），
补上 P2 组 D 组最后一个控件缺口：**带标题与正文的气泡卡片**（对齐 Ant Design 的
[Popover](https://ant-design.antgroup.com/components/popover-cn) 语义）。

现有能力缺口：

| 现有控件 | 缺什么 |
| :--- | :--- |
| `ToolTip` | 只有纯文本气泡，Windows 侧还是系统原生实现，无法承载标题/富内容（D-2 已明确不做富内容 Tooltip） |
| `Flyout` | 只有"浮层地基"（`ShowAt` + 方位 + 自动关闭），没有"锚点控件即触发器"的封装，业务侧每个浮层都要手写打开/关闭生命周期 |
| `ToastWnd` / `SystemNotification` | 应用级通知，与锚点无关 |

`Popover` 的定位：**控件自身就是锚点**，点击 / 悬停 / 聚焦后在它周围弹出带标题与正文的卡片浮层，
默认点击外部或按 Esc 自动关闭。

完成后回写 `docs/misc/Development-Plan.md` 的 P2 进展汇总、D 组表格（新增 D-14 行）、
P2 任务清单（新增 `- [x] **D-14**` 任务行）。

## 2. 需求要点

- 4 种触发方式：`click`（默认）/ `hover`（移入延时弹出、移出延时关闭）/ `focus`（获得焦点弹出）/ `manual`（仅 API）。
- 12 种弹出方位：上下左右 4 个主方位 + 8 个偏移方位（`top_left`、`top_right`、`bottom_left` 等）；
  越界时按 Flyout 既有逻辑自动翻转。
- 卡片内容：标题（`title` / `title_id`）+ 正文（`content` / `content_id`），
  或 `content_xml` 直接插入一段 XML（富内容，可放按钮/图片/多行文本）。
- 受控模式：`Show()` / `Hide()` / `Toggle()` / `IsOpen()` / `AttachOpenChanged()`，
  打开与关闭状态可被业务回显（例如按钮联动）。
- 多语言：`title_id` / `content_id` 走语言 ID；语言切换时自动关闭卡片（内容在打开时生成）。
- 交付完整的示例、双语文档、技能参考与主题 Class。

## 3. 设计决策

### 3.1 形态：`Popover : public Label`

```cpp
class DUILIB_API Popover : public Label
```

理由：

- 锚点自身即触发器，不需要额外的"宿主容器 + 触发按钮"配对；
- `Label` 自带 `text` / `text_id` / `font` / `text_align` 与语言解析，锚点文案零成本；
- 不用 `Box`：容器的鼠标事件会被子控件（`Button` 等）消费，父容器收不到触发事件；
  叶子控件才能稳定收到 `kEventMouseEnter` / `kEventMouseButtonUp`。

`Popover` 自身只绘制锚点文本（样式由 `popover_trigger` Class 提供，下划线 + 手型光标），
指向锚点的尖角箭头由**卡片窗口**绘制（见 3.9），锚点控件不承担箭头绘制。

### 3.2 弹出层复用 `Flyout`，只扩展方位枚举

卡片浮层完全复用 `ui::Flyout`（非模态、`WS_EX_NOACTIVATE` 不抢焦点、点击外部/Esc 自动关闭、
关闭后框架自动 `delete`），`Popover` 只做三件事：

1. 生成卡片 XML（窗口 + `popover_card` 容器 + 标题 + 正文）；
2. 把 Ant Design 的 12 方位映射为 `Flyout::Placement`；
3. 维护打开状态与关闭原因回写。

`Flyout::Placement` 原只有 8 个方位（四角 + 四边起止），缺少"居中"语义，本次**向后兼容地追加**
4 个值（旧枚举值不变）：

```cpp
enum class Placement : uint32_t
{
    // ... 原有 Top / TopEnd / Bottom / BottomEnd / Left / LeftEnd / Right / RightEnd 保持不变
    BottomCenter = 8,   //锚点下方居中（对应 antd 的 bottom）
    TopCenter    = 9,   //锚点上方居中（对应 antd 的 top）
    RightCenter  = 10,  //锚点右侧居中（对应 antd 的 right）
    LeftCenter   = 11   //锚点左侧居中（对应 antd 的 left）
};
```

翻转逻辑从 `PositionAroundAnchor()` 内的散落判断抽成匿名命名空间函数 `FlipPlacement()`，
目标方位也统一走该函数，保证 12 个方位在越界时互为翻转。

方位映射表（`ToFlyoutPlacement()`）：

| Popover（antd 语义） | Flyout::Placement |
| :--- | :--- |
| `top` | `TopCenter` |
| `top_left` / `top_right` | `Top` / `TopEnd` |
| `bottom` | `BottomCenter` |
| `bottom_left` / `bottom_right` | `Bottom` / `BottomEnd` |
| `left` | `LeftCenter` |
| `left_top` / `left_bottom` | `Left` / `LeftEnd` |
| `right` | `RightCenter` |
| `right_top` / `right_bottom` | `Right` / `RightEnd` |

### 3.3 卡片 XML 由控件动态生成

`BuildCardXml()` 生成：

```xml
<Window size="1,1" caption="0,0,0,0" use_system_caption="false"
        shadow_type="custom"
        shadow_image="file='public/shadow/shadow_popover_arrow_down.svg' window_shadow_mode='true' corner='20,20,22,30'"
        shadow_corner="12,12,14,14" shadow_border_round="0,0" shadow_border_size="0" shadow_snap="false"
        shadow_attached="true" layered_window="true" size_box="0,0,0,0">
  <!-- 方位为 top 时：卡片在上、箭头带在下（卡片在该侧留出 8 的 margin） -->
  <VBox name="popover_frame" width="..." height="auto">
    <VBox class="popover_card" name="popover_card" width="..." height="auto" margin="0,0,0,8">
      <!-- 可选 --> <Label class="popover_title" text|text_id="..." margin="0,0,0,6"/>
      <!-- 二选一 -->
      <Label class="popover_content" text|text_id="..."/>
      <!-- 或 content_xml 原样插入 -->
    </VBox>
    <!-- 浮动箭头：排在卡片之后（绘制在卡片之上），由 PositionCardArrow 在布局完成后定位 -->
    <Control class="popover_arrow_down" name="popover_arrow" float="true" keep_float_pos="true"
             width="16" height="9"/>
  </VBox>
</Window>
```

- 阴影使用自定义模板，按箭头方位选择 `shadow_popover_arrow_{down,up,right,left}.svg`（无箭头时用
  `shadow_popover.svg`）：模糊半径 4、偏移 2、边距 12/14，比通用 `small_round`（四边 24/24/28/30）
  更紧凑；分层窗口的默认阴影 `big_round` 四边 30~36 像素，对 240 宽的卡片过重，且阴影区会盖住锚点
  与相邻控件（见 3.7）。`custom` 阴影的九宫格 `corner` = `shadow_corner` + 卡片圆角 = `20,20,22,22`；
  带箭头时箭头一侧再加 8（见 3.9）。
- 标题、正文中的 `& < > " '` 统一转义（局部 `EscapeXmlText()`，不新增跨模块依赖）。
- `card_width` 最小钳到 120；卡片宽度与内边距全部由主题 Class 承担，控件不写死像素。
- 卡片 XML 在**每次打开时**重新生成 → 语言切换后重新打开即拿到新文案。

### 3.4 悬停语义与跨窗口鼠标判定

`click` / `focus` 触发是同步的；`hover` 需要"延迟弹出 + 移出延迟关闭"：

- 锚点 `MouseEnter`：取消未执行的关闭任务，`PostDelayedTask(hover_delay)` 后 `Show()`。
- 锚点 `MouseLeave` / 卡片 `MouseLeave`：`PostDelayedTask(close_delay)` 后检查
  `IsMouseOverAnchorOrCard()`，两者都不在光标下才 `Hide()`。
- 卡片 `MouseEnter`：取消关闭任务 —— 这样鼠标可以从锚点**移进卡片**继续操作
  （卡片是独立窗口，物理上必须用屏幕坐标判定）。

所有延迟任务存在 `m_nOpenTaskId` / `m_nCloseTaskId`，`CancelAllTasks()` 在析构、
`OnLanguageChanged`、`OnFlyoutClosed` 时统一取消，避免回到半析构对象或重复派发。

### 3.5 状态与事件

- 不自持 `Flyout*` 指针：用 `Flyout::GetActiveFlyout()` + `GetAnchor() == this` 查询；
  关闭回调 `AttachClosed` 用 `UiBind(this, ...)` 弱引用回写状态，浮层销毁不会悬垂。
- 新增事件 `kEventPopoverOpenChanged`：`wParam = 1/0`（打开/关闭），
  `lParam = PopoverCloseReason`（`kManual` / `kClickOutside` / `kEscape` / `kAnchorLost`）。
- 关闭原因枚举与 `Flyout::CloseReason` 一一对应，只做 `static_cast` 转换，不重复定义语义。

### 3.6 明确不做

- 不做箭头尺寸/角度的任意配置（`arrow` 只开/关；尖角固定 16×8 / 8×16），不做 C++ 多边形绘制。
- 不做卡片内自动换行的多段富文本编辑器（用 `content_xml` 自行拼）。
- 不做"同一锚点多卡片队列"、"跟随锚点鼠标移动重定位"。
- 不做嵌套 Popover（浮层内再弹 Popover）；`GetActiveFlyout()` 保证全局同时只有一张卡片。

### 3.7 阴影不参与交互（实测问题修复）

分层窗口的阴影是绘制在窗口客户区边缘的像素，但框架的 `NativeWindow_Windows::OnNcHitTestMsg`
对阴影区仍返回 `HTCLIENT`，于是阴影成了"实心挡板"：

| 现象 | 根因 |
| :--- | :--- |
| 鼠标离开锚点后卡片不消失 | `Popover::IsMouseOverAnchorOrCard()` 用 `GetWindowRect()` 判定，阴影覆盖到锚点上，永远认为光标仍在卡片内 |
| 卡旁边的按钮点不动 | 点在阴影上时消息被浮层窗口吞掉，下层父窗口收不到 `WM_LBUTTONDOWN/UP` |
| 视觉上阴影过重 | `default` 阴影 = `kShadowBigRound`，四边 30~36 像素，对 240 宽的卡片过大 |

修复分三层：

1. **命中穿透（交互层）**：`Flyout` 新增 `SetShadowClickThrough(bool)`（默认 true），
   在 `OnInitWindow()` 里创建一个 `IUIMessageFilter` 实现类（`ShadowHitTestFilter`）并
   `AddMessageFilter()`；收到 `WM_NCHITTEST` 且落点不在可视区域时返回 `HTTRANSPARENT`，
   让鼠标消息穿透到同线程的下层父窗口（`OnFinalMessage()` 里 `RemoveMessageFilter()` 并释放）。
   与 `ClickThrough::ClickThroughWindow()` 不同：后者只激活下层窗口、不转发鼠标消息。
2. **判定用可视区域（语义层）**：新增

   ```cpp
   void GetVisualRect(UiRect& rcVisual) const;                 //窗口矩形去掉阴影后的区域
   bool IsPointInVisualArea(const UiPoint& ptScreen) const;    //屏幕坐标是否落在可视区域
   ```

   `GetVisualRect()` = `GetWindowRect()` 再 `Deflate(GetCurrentShadowCorner())`（为空时回退窗口矩形）。
   `Popover::IsMouseOverAnchorOrCard()` 与 `Flyout::OnDetectTick()` 的"点击外部"判定都改用它 ——
   **阴影像素一律按"卡片外"处理**。
3. **视觉减重（主题层）**：卡片窗口 `shadow_type` 由 `default` 改为 `small_round`（四边 24/24/28/30），
   T12 进一步改为自定义 `shadow_popover.svg`（边距 12/14，见 3.9）。

### 3.8 点击外部关闭与 auto_dismiss 解耦

原实现把"点外部关闭"和"按 Esc 关闭"都挂在 `auto_dismiss` 上，导致 `trigger="manual"`（受控模式）
下无法只保留 Esc：一旦 `auto_dismiss=true`，"点击外部"会在**鼠标按下瞬间**关闭卡片，
锚点/按钮的 `Click`（弹起时触发）读到的状态已是"已关闭"，`Toggle()` 于是又把它打开 ——
表现为"切换按钮永远只会显示卡片、切换宽度/方位按钮无效果"。

修复：

- `Flyout` 新增 `SetClickOutsideDismiss(bool)` / `IsClickOutsideDismiss()`，"点击卡片与锚点以外的
  区域关闭"独立开关；`OnKillFocusMsg()` 与 `OnDetectTick()` 的外部点击判定都受它控制，Esc 仍归 `auto_dismiss`。
- `Popover` 新增 `m_bAutoDismissDefined`（是否在 XML/API 里显式设置过 `auto_dismiss`）。
  `Show()` 时若 `trigger == kManual` 且未显式设置，则 `SetClickOutsideDismiss(false)`：
  受控模式默认只能由 API（或 Esc）关闭。显式写 `auto_dismiss="true"` 可恢复"点外部关闭"。

### 3.9 尖角箭头与更紧凑的阴影（对齐 antd 视觉）

antd 的 Popover 卡片在朝向锚点的那条边上带一个尖角（arrow），指向触发元素；同时边线与阴影都很轻。
原实现（T11）已把阴影换成 `small_round`，但卡片仍无箭头，"当前指向哪个控件"不够直观。

第一版（T12）用"卡片框 + 箭头条"排布箭头，实测出现三个视觉问题：
① 箭头沿卡片边的位置依赖 `stretch` 占位，卡片高度是 `auto`，占位分不到期望空间 → 箭头不在锚点中心；
② 箭头条与卡片不重叠，卡片把阴影主体的边距算在卡片外侧 → 卡片箭头一侧露出约 8 像素的纯白实心带
   （阴影图的白色主体），且该白带与窗口阴影带之间是硬裁切的边界；
③ 箭头图片的底边压不住卡片 1px 边线，边线横穿箭头底边，加上斜边 1px 抗锯齿 → "线条不美观"。

第二版（本版）改为"卡片留箭头带 + 浮动箭头 + 方位专用阴影图"：

1. **卡片在箭头一侧内缩 8（箭头带）**：`BuildCardXml()` 按方位给 `popover_card` 加 `margin`
   （top: `0,0,0,8`、bottom: `0,8,0,0`、left: `0,0,8,0`、right: `8,0,0,0`），
   箭头带内不再放任何非浮动子控件。
2. **方位专用阴影图，使阴影主体与卡片对齐**：`public/shadow/shadow_popover_arrow_{up,down,left,right}.svg`
   在箭头一侧把阴影主体（白色 body 矩形）内缩 8（其余三边不变，图片仍为 226×226），
   九宫格 `corner` 在该侧取"阴影边距 + 8 箭头带 + 8 卡片圆角"：
   top→`20,20,22,30`、bottom→`20,28,22,22`、left→`20,20,30,22`、right→`28,20,22,22`；
   无箭头时仍用 `shadow_popover.svg` + `20,20,22,22`。
   于是 `window_shadow_mode` 的九宫格绘制中，阴影主体的边缘正好落在卡片边缘上：
   主体内部的白色实心被卡片遮住，箭头带内只剩阴影渐变，与窗口阴影带连续过渡（消除②）。
3. **箭头改为浮动控件**：`<Control class="popover_arrow_*" float="true" keep_float_pos="true">`
   排在卡片之后（绘制在卡片之上）；尺寸含 1 像素压入卡片的重叠区（水平 16×9、垂直 9×16），
   箭头图片的"压入区"用卡片背景色覆盖该处的卡片边线，两条斜边描边从卡片边线上出发（消除③）。
4. **布局完成后按锚点中心定位**：`Flyout` 新增 `AttachLayouted()`（在 `OnInitLayout()` 中触发，
   位于首帧布局（`ArrangeRoot()`）之后、首帧绘制之前，只触发一次）；`Popover::Show()` 在
   `ShowAt()` **之前**注册回调 → `PositionCardArrow(pFlyout)`：
   - 锚点中心：`GetScreenRectPhysical()` 取锚点物理屏幕矩形，`Flyout::ScreenToClient()` 换算到
     浮层客户区布局坐标；
   - 水平方位：`offX = ClampArrowOffset(锚点中心X - 卡片left - 箭头长度/2, 卡片宽, 箭头长度, 12)`；
     垂直方位同理用 Y、卡片高；
   - 箭头矩形以卡片边为基准压入 1 像素（如 top：`top = 卡片bottom - 1`、`bottom = top + 8 + 1`），
     再 `SetPos()`；`keep_float_pos` 保证后续重排时保持该位置。
   卡片过窄（`卡片长 - 箭头长 - 2×12 <= 12`）时箭头居中放置；`12` 是避开卡片圆角的最小边距。
5. **方位翻转后按实际方位重建**：`Flyout` 新增 `GetActualPlacement()`；`Show()` 首次尝试成功但
   `GetPlacementSide(实际方位) != GetPlacementSide(请求方位)` 时，以 `m_bRebuilding` 守卫关闭旧浮层
   （`OnFlyoutClosed` 提前返回，不改状态、不派发事件），把 `placement` 换成实际方位、`bAllowFlip=false`
   再弹一次，保证箭头永远指向锚点。
6. **`gap` 语义**：显示箭头时，`gap` 是"尖角尖端到锚点"的距离（尖角本身占了 8 像素）。
7. **`arrow` 属性**：`arrow="true"`（默认）/ `SetArrow(bool)` / `IsArrow()` 控制开关；
   关闭时不生成箭头、卡片不预留箭头带、阴影用无箭头的 `shadow_popover.svg`。
8. **关闭客户区圆角裁剪**：`shadow_border_round="0,0"`（卡片圆角由 `popover_card` 的
   `border_round` 提供），避免箭头带被客户区圆角裁掉。

已知限制：箭头沿卡片边的位置在首帧布局完成后确定；若卡片在显示期间因内容变化而重排（例如卡片内
控件动态增减），箭头不会重新定位（Popover 的卡片内容在打开时固定，通常不会发生）。

#### 3.9.1 非对称 `corner` 的断言修复（T14）

第二版的阴影图 `corner` 是非对称的（箭头一侧为"阴影边距 + 8 + 卡片圆角"，另一侧仍为"阴影边距 + 卡片圆角"），
例如右侧方位为 `corner='28,20,22,22'`。实测点击 `RT`（`right_top`，卡片在锚点右侧）等方位时弹出断言，
忽略后卡片显示正常。

根因：`AttributeUtil::ParseRectValue()` 在 `bCheckSize=true` 时校验 `rect.Width() >= 0 && rect.Height() >= 0`，
而 `corner` 的 4 个值是**四条边各自的内缩量**（`left` / `top` / `right` / `bottom` 互相独立，绘制时按
`src.right - corner.right` 之类的逐边方式使用），不是普通矩形；`28,20,22,22` 按矩形算出的宽度是 `22 - 28 = -6`，
于是断言误报（`bottom` 方位 `20,28,22,22` 同理，高度 `22 - 28` 为负）。

修复：`ImageAttribute::ModifyAttribute()` 解析 `corner` 时改用 `ParseRectValue(value, *m_rcCorner, false)`，
随后仍按逐边钳到 `>= 0`（框架原有逻辑）；`src` / `dest` / `size_box` / `caption` 等矩形语义的属性保持尺寸校验。
该修复对 `bkimage` / `shadow_image` 等所有经 `ImageAttribute::ModifyAttribute()` 解析的 `corner` 生效，
对称 `corner` 的行为不变。

## 4. 精确改动清单

### 4.1 控件

1. 新增 `duilib/Control/Popover.h`
   - 枚举 `PopoverTrigger`（`kClick` / `kHover` / `kFocus` / `kManual`）、
     `PopoverPlacement`（12 值）、`PopoverCloseReason`（4 值）；
   - `class DUILIB_API Popover : public Label`，`typedef Label BaseClass`；
   - public：`GetType` / `SetAttribute` / `OnLanguageChanged`；
     内容：`SetTitle/GetTitle`、`SetTitleId/GetTitleId`、`SetContent/GetContent`、
     `SetContentId/GetContentId`、`SetContentXml/GetContentXml`；
     行为：`SetTrigger/GetTrigger`、`SetPlacement/GetPlacement`、`SetGap/GetGap`、
     `SetCardWidth/GetCardWidth`、`SetArrow/IsArrow`、`SetAutoDismiss/IsAutoDismiss`、
     `SetHoverDelay/GetHoverDelay`、`SetCloseDelay/GetCloseDelay`；
     控制：`Show` / `Hide` / `Toggle` / `IsOpen` / `AttachOpenChanged`；
     卡片内控件访问：`GetCardWindow`（显示中的卡片窗口，未显示为 `nullptr`）、
     `FindCardControl(name)`（卡片内按名称查控件，用于绑定卡片内按钮/链接）；
     事件入口：`MouseEnter` / `MouseLeave` / `ButtonDown` / `ButtonUp`；
     protected：`OnSetFocus` / `OnKillFocus` / `OnSetVisible`；
   - private：`GetOpenFlyout` / `BuildCardXml(PopoverPlacement)` / `InitializeCard` / `PositionCardArrow` /
     `EmitOpenChanged` /
     `OnFlyoutClosed` / `IsMouseOverAnchorOrCard` / `GetScreenRectPhysical` /
     `ScheduleOpen` / `ScheduleCloseCheck` / `CancelOpenTask` / `CancelCloseTask` / `CancelAllTasks`；
   - 成员 `m_bArrow`（默认 true）、`m_bRebuilding`（翻转重建守卫）。
2. 新增 `duilib/Control/Popover.cpp`
   - 属性解析（主名 + 无下划线别名，含 `arrow`）、方位/触发方式解析与归一化（忽略大小写与 `_`/`-`）、
     XML 转义、卡片生成（卡片按方位留箭头带 + 方位专用阴影 + 浮动箭头）、打开/关闭生命周期、翻转重建、
     布局完成后的箭头定位（`PositionCardArrow`）、悬停延迟任务、DPI 与屏幕坐标换算；
   - 匿名命名空间辅助：`FromFlyoutPlacement()`（`Flyout::Placement` → `PopoverPlacement`）、
     `PopoverSide` / `GetPlacementSide()`（按主方位归类）、`ClampArrowOffset()`（箭头落点钳位），
     常量 `kArrowLength`(16) / `kArrowDepth`(8) / `kArrowOverlap`(1) / `kArrowMinEdgeMargin`(12) / `kArrowBand`(8)。
3. 扩展 `duilib/Control/Flyout.h/.cpp`
   - `Placement` 追加 `BottomCenter` / `TopCenter` / `RightCenter` / `LeftCenter`；
   - `FlipPlacement()` 抽取公共翻转逻辑，`PositionAroundAnchor()` 覆盖 12 个方位；
   - 新增 `GetActualPlacement()`（翻转后的实际方位，供 `Popover` 重建箭头方向）；
   - 新增 `AttachLayouted(const FlyoutLayoutEvent&)`（`typedef std::function<void()>`）与
     `OnInitLayout()` 覆写：在首帧布局完成后、首帧绘制前触发一次布局完成回调（供 Popover 定位箭头）；
   - 阴影交互：新增 `SetClickOutsideDismiss(bool)` / `IsClickOutsideDismiss()`、
     `SetShadowClickThrough(bool)` / `IsShadowClickThrough()`、
     `GetVisualRect(UiRect&) const` / `IsPointInVisualArea(const UiPoint&) const`；
     新增成员 `m_bClickOutsideDismiss`（默认 true）、`m_bShadowClickThrough`（默认 true）、
     `m_pShadowHitTestFilter`（`std::unique_ptr<IUIMessageFilter>`）；
   - 平台层：`OnInitWindow()` 安装 `ShadowHitTestFilter`（仅 Windows 非 SDL，宏
     `DUILIB_BUILD_FOR_WIN && !DUILIB_BUILD_FOR_SDL`）→ `WM_NCHITTEST` 落在 `GetVisualRect()` 之外
     返回 `HTTRANSPARENT`；`OnFinalMessage()` 卸载并释放；
   - `OnKillFocusMsg()` 与 `OnDetectTick()` 的"点击外部"判定改受 `m_bClickOutsideDismiss` 控制，
     落点判定改走 `GetVisualRect()`。

### 4.2 注册（5 处）

| 文件 | 改动 |
| :--- | :--- |
| `duilib/duilib_defs.h` | `DUI_CTR_POPOVER (_T("Popover"))`；`enum EventType` 追加 `kEventPopoverOpenChanged` |
| `duilib/Core/WindowBuilder.cpp` | include + `{DUI_CTR_POPOVER, ...new Popover(pWindow)}` |
| `duilib/duilib.vcxproj` | `Control\Popover.cpp/.h` |
| `duilib/duilib.vcxproj.filters` | 归入 `Control` 过滤器 |
| `duilib/duilib.h` | `#include "Control/Popover.h"` |

另：`duilib/Core/EventArgs.cpp` 的 `InitEventStringMap` 追加事件名映射
（`kEventPopoverOpenChanged` / `PopoverOpenChanged` / `popover_open_changed`），供 XML 内联事件使用。

### 4.3 主题

`bin/resources/themes/default/global.xml` 新增 10 个 Class（颜色全部为语义色名，无写死色值），
并新增变量 `<Var name="PATH_POPOVER" value="public/popover"/>`：

```xml
<Class name="popover_trigger" .../>
<Class name="popover_anchor" .../>
<Class name="popover_anchor_primary" .../>
<Class name="popover_card" .../>
<Class name="popover_arrow_up" .../>
<Class name="popover_arrow_down" .../>
<Class name="popover_arrow_left" .../>
<Class name="popover_arrow_right" .../>
<Class name="popover_title" .../>
<Class name="popover_content" .../>
```

| Class | 用途 |
| :--- | :--- |
| `popover_trigger` | 锚点文本（链接色 + 悬停/按下色 + `cursor_type="hand"`） |
| `popover_anchor` | 锚点按钮（`bg_btn_*` 底色 + 1px `border_btn_*` 描边 + 圆角 6 + 96×28 + 文字居中，对应 Ant Design 的按钮锚点） |
| `popover_anchor_primary` | 锚点主色实心按钮（继承 `popover_anchor`，112 宽，对应 Ant Design 基本示例） |
| `popover_card` | 卡片容器（`bg_window_card` + 1px `border_window` + 圆角 + 12px 内边距） |
| `popover_arrow_up` / `popover_arrow_down` | 尖角箭头（16×9，`mouse_enabled="false" tab_stop="false"`，填充 `bg_window_card`、描边 `border_window`；含 1 像素压入卡片的重叠区） |
| `popover_arrow_left` / `popover_arrow_right` | 尖角箭头（9×16，同上） |
| `popover_title` | 卡片标题（`system_bold_14` + `text_default`） |
| `popover_content` | 卡片正文（`system_regular_14` + `text_muted` + `multi_line`) |

新增图片资源：`public/popover/popover_arrow_up|down|left|right.svg`
（水平 16×9、垂直 9×16：尖角侧 + 1 像素压入卡片的重叠区，两条斜边描边从卡片边线出发，
颜色经 `svg_replace_colors` 映射语义色），
`public/shadow/shadow_popover.svg`（226×226，`rect(12,12,200,200) rx=8`，`feOffset(2,2)`、
`stdDeviation=4`、alpha 0.2），
以及 4 个箭头方位专用的 `public/shadow/shadow_popover_arrow_{up,down,left,right}.svg`
（226×226，阴影主体的高度/宽度减 8 并在箭头一侧内缩 8，其余边距不变）。

### 4.4 示例

> 示例结构对齐 Ant Design 的 Popover 演示页（代码演示区）：基本 / 三种触发方式 / 位置 / 受控模式 / 复杂内容。

1. `examples/controls/PopoverForm.h/.cpp`：
   - `BindBasicPopover()`：`content_xml` 拼两行正文（对应 antd 的 Title + Content + Content）；
   - `BindTriggerPopovers()`：手动触发锚点的"打开卡片"按钮（演示 manual 只响应 API）；
   - `BindControlledPopover()`：`AttachOpenChanged` → 状态文本（含关闭原因）、显示/隐藏/切换/换样式四个按钮；
   - `BindRichPopover()`：`content_xml` 富内容 + 在打开回调里用 `FindCardControl()` 绑定卡片内"从浮层内关闭"按钮。
2. `bin/resources/themes/default/controls/popover.xml`：标准标题栏 + 提示 + 5 张卡片
   （基本用法 / 三种触发方式 + 手动 / 12 方位环形布局 / 受控模式 / 复杂内容）；锚点统一使用
   `popover_anchor`（按钮外观），基础用法使用 `popover_anchor_primary`；12 方位用 `HBox` 固定宽
   单元 + `stretch` 占位组成"环形"，与 antd 的 TL/Top/TR/LT/…/RB 布局一致。
   按钮之间一律用 `child_margin_x`（三种触发行 8，12 方位行 6）：`child_margin` 只接受单个整数，
   会把 Y 方向也一起设上，故不再使用；长文案锚点（`popover_manual` / `popover_controlled` /
   `popover_rich` / `btn_popover_style`）改用 `width="auto" padding="12,0,12,0"`，避免文字被截断。
3. `bin/resources/themes/default/controls/test.xml`：新增 `GroupHBox text="Popover"` 与入口按钮。
4. `examples/controls/TestForm.h/.cpp`：`ShowPopoverWindow()` + 绑定入口。
5. `examples/controls/controls.vcxproj` / `.filters`：登记新文件。
6. `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增 `STRID_POPOVER_*` 段
   （57 个键），两份键集合一致。

### 4.5 文档

| 文件 | 改动 |
| :--- | :--- |
| `docs/Control.md` | Flyout 章节之后新增 `## Popover 控件（气泡卡片）`；Flyout 的 `Placement` 枚举行补 4 个新值 |
| `docs/Control.en.md` | 新增 `## Popover Control (bubble card)`；同步 `Placement` 枚举行 |
| `docs/Summary.md` / `.en.md` | 控件表新增一行（锚点指向新章节） |
| `README.md` / `README.en.md` | 功能列表条目 |
| `.trae/skills/nim-duilib/references/api-reference.md` | 速查行 + 属性小节 |
| `.workbuddy/skills/nim-duilib/references/api-reference.md` | 与 `.trae` 版本同文本覆盖 |
| `.claude/docs/nim-duilib-llm-reference.md` | 速查行 |
| `docs/misc/Development-Plan.md` | P2 进展汇总、D 组表格新增 D-14 行、P2 任务行 `- [x]` + 日期 |

## 5. 验证

1. Debug|x64 编译（`duilib` 库 + `controls` 示例）：

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
  "e:\develop\nim_duilib\build\examples.sln" /t:duilib /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo

& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
  "e:\develop\nim_duilib\examples\controls\controls.vcxproj" /p:Configuration=Debug /p:Platform=x64 /p:SolutionDir=<仓库根>\build\ /clp:ErrorsOnly /m:1 /nologo
```

2. 文档校验：

```powershell
python .trae/skills/nim-duilib/scripts/verify_docs.py --repo . --extra docs examples/controls bin/resources/themes/default/controls bin/resources/lang
```

退出码必须为 0。

## 6. 验收标准

- [ ] XML `<Popover class="popover_trigger" title_id="..." content_id="..." placement="top_left"/>` 可创建，`FindControl` 可取到。
- [ ] `trigger="click"/"hover"/"focus"/"manual"` 四种触发方式行为正确；`hover` 的移入/移出延迟生效。
- [ ] 12 个方位均能弹出；靠近屏幕边缘时自动翻转到对侧且卡片完整可见。
- [ ] 标题 + 正文渲染正确；`content_xml` 富内容（多标签/按钮）可交互。
- [ ] `Show` / `Hide` / `Toggle` 与 `AttachOpenChanged` 状态回显一致（`wParam`、关闭原因 `lParam` 正确）。
- [ ] 点击卡片外部、按 Esc、`auto_dismiss="false"` 三种关闭路径符合预期。
- [ ] 语言切换到英文后重新打开卡片，标题/正文为新语言；切换的瞬间旧卡片被关闭。
- [ ] 锚点控件随窗口滚动、窗口移动、DPI 变化后弹出的卡片位置仍正确。
- [ ] 浅色/深色主题、100%/125%/150% DPI 下卡片描边、圆角、文字配色正常。
- [ ] 反复打开/关闭、锚点窗口关闭时销毁锚点控件均不崩溃、不残留浮层。
- [ ] 示例页与 Ant Design 演示结构一致：基本用法（主色按钮锚点 + 两行正文）、三种触发方式（悬停/聚焦/点击 + 手动按钮）、
      12 方位环形布局、受控模式（显示/隐藏/切换/换样式 + 状态回显）、复杂内容（卡片内"从浮层内关闭"按钮可用）。
- [ ] 锚点按钮外观（`popover_anchor` / `popover_anchor_primary`）在悬停、按下、禁用三种状态下配色正确。
- [ ] 阴影不拦截鼠标：卡片打开时，点在卡片阴影上的相邻按钮仍能正常收到点击（阴影区按 `HTTRANSPARENT` 穿透）。
- [ ] 阴影不计入"是否在卡片内"：`trigger="hover"` 的锚点悬停弹出后，鼠标离开锚点即按 `close_delay` 关闭卡片，不因阴影覆盖而常驻。
- [ ] 受控模式（`trigger="manual"`）下"切换"按钮可真正切换：点一次开、再点一次关（`Toggle()` 前后的 `IsOpen()` 一致）。
- [ ] 受控模式"切换宽度/方位"按钮生效：卡片按新 `card_width` / `placement` 重建。
- [ ] 长文案按钮（"打开复杂内容"等）文字完整显示，按钮之间留有水平间距（`child_margin_x`）。
- [ ] 12 个方位下尖角箭头朝向正确（指向锚点），尖角落在卡片朝向锚点的那条边上、底边与卡片边线连续；
      水平方位箭头水平位置对准锚点（居中/左对齐/右对齐三种对齐方式均正确）。
- [ ] 空间不足翻转后箭头仍指向锚点（卡片按实际方位重建一次）；翻转过程不闪两张卡片、不重复派发打开/关闭事件。
- [ ] 左右方位下箭头在卡片高度内垂直居中（`left_top`/`right_top` 顶对齐，`left_bottom`/`right_bottom` 底对齐）。
- [ ] `arrow="false"` / `SetArrow(false)` 后卡片不再绘制箭头，位置与阴影正常。
- [ ] 白底卡片上箭头填充与卡片底色一致、描边与卡片边线一致，深浅色主题切换后依然一致。
- [ ] 阴影比 T11 的 `small_round` 更紧凑：卡片四周留白明显变小，且仍不拦截相邻按钮的鼠标。
- [ ] 箭头条不参与鼠标命中（`mouse_enabled="false"`），在箭头处点击不会误触发卡片内的控件。
