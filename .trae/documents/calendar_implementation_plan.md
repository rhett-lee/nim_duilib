# Calendar 日历面板控件实现计划

## Context

Control Roadmap 第 7 项：Calendar 日历面板。现有 `DateTime` 控件依赖系统超类化 DTP（数字滚动式），无自绘月历面板。日程、报表类软件需要此控件，且它依赖已完成的第 6 项 Flyout 作为弹层载体。

用户已确认设计方向：

* **弹出方式**：Flyout 承载 Calendar（DateTime 点击时弹出 Flyout 内嵌 Calendar）

* **选择范围**：日期范围选择

* **导航层级**：月/年/十年三级

## 实现方案

### 1. Calendar 控件（自绘月历网格）

**文件**：

* `duilib/Control/Calendar.h` / `Calendar.cpp` — 新增控件类

**类结构**：

```cpp
class DUILIB_API Calendar : public Control
{
    // 继承 Control，纯自绘月历网格
    // Paint() 中绘制：星期标题行 + 6×7 日期网格
    // MouseMove / ButtonDown 处理悬停与点击
    // 支持日期范围选择（拖拽或点击起止）
};
```

**XML 属性**：

| 属性                      | 默认值      | 说明                          |
| ----------------------- | -------- | --------------------------- |
| `calendar_mode`         | `single` | `single` / `range`（单选/范围选择） |
| `first_day_of_week`     | `monday` | 每周第一天                       |
| `show_today_button`     | `true`   | 是否显示"今天"按钮                  |
| `min_date` / `max_date` | 空        | 可选日期范围限制                    |

**C++ 接口**：

* `SetDate(const struct tm&)` / `GetDate()`

* `SetDateRange(const struct tm& start, const struct tm& end)` / `GetDateRange(...)`

* `AttachDateChanged(callback)` — 日期变化事件（kEventValueChanged）

**绘制逻辑**（Paint 中）：

1. 星期标题行（一 \~ 日，按 first\_day\_of\_week 偏移）
2. 6 行 × 7 列日期网格，计算当月第一天星期几
3. 当前月日期：text\_default；上月/下月：text\_muted
4. 今天：主题色圆点/边框高亮；选中日期：主题色背景填充
5. 范围选择：起止日期高亮，中间日期浅色背景
6. 悬停：hover 背景色

**颜色**：全部使用语义色（bg\_window\_card, text\_default, text\_muted, bg\_button\_hovered, primary 主题色等），自动适配深浅色主题。

### 2. CalendarFlyout（Flyout 承载层）

**文件**：

* `duilib/Control/CalendarFlyout.h` / `CalendarFlyout.cpp` — 继承 Flyout

**职责**：

* 内部创建 Calendar 控件作为内容

* 提供 `ShowAt(Control* anchor, const struct tm& initDate, Placement)` 简化接口

* 日期选择后自动 Dismiss 并通知宿主

* 月/年/十年三级导航的头部按钮和切换动画

**内部 XML 结构**（Flyout 内容）：

```xml
<Window size="280,320" ...>
  <VBox>
    <!-- 头部：年月导航 -->
    <HBox height="40">
      <Button name="cal_prev" text="◀"/>
      <Button name="cal_title" text="2025年1月"/>  <!-- 点击进入年视图 -->
      <Button name="cal_next" text="▶"/>
    </HBox>
    <!-- 日历网格 -->
    <Calendar name="cal_grid"/>
    <!-- 底部：今天/清除 -->
    <HBox height="36">
      <Button name="cal_today" text="今天"/>
      <Button name="cal_clear" text="清除"/>
    </HBox>
  </VBox>
</Window>
```

### 3. DateTime 控件集成

修改 `DateTime.cpp`：

* 当 `edit_format="date_calendar"` 时，点击不再创建旧 `DateTimeWnd`，而是弹出 `CalendarFlyout`

* 选择日期后回写到 DateTime 的 `SetDateTime()`

* 保持 `kDateUpDown` 等其他编辑格式不变（SDL 平台继续用 SpinBox 方式）

### 4. 控件注册

* `duilib_defs.h`：新增 `DUI_CTR_CALENDAR` (`"Calendar"`)

* `WindowBuilder.cpp`：注册 `DUI_CTR_CALENDAR -> new Calendar(...)`

* `duilib.vcxproj` / `.filters`：添加新文件

### 5. 皮肤资源

`global.xml` 新增 Calendar 相关 Class：

* `calendar` — 整体容器

* `calendar_header` / `calendar_title` / `calendar_nav_btn` — 头部导航

* `calendar_weekday` — 星期标题

* `calendar_day` / `calendar_day_muted` / `calendar_day_today` / `calendar_day_selected` / `calendar_day_range` — 日期格子

### 6. 示例接入

**controls.exe**（`ControlForm`）：

* 新增"日历(Calendar)"分组

* 内嵌 Calendar 控件（单选模式）

* DateTime 控件 edit\_format="date\_calendar" 演示弹层选择

* 日期范围选择演示

**ColorTheme.exe**：

* 新增 Calendar 预览分组

### 7. 文档

* `docs/Control.md` / `Control.en.md`：Calendar 控件属性、接口、事件、使用示例

* `.trae/skills/nim-duilib/references/api-reference.md` + `.workbuddy` 副本：新增 Calendar 节

* `.claude/docs/nim-duilib-llm-reference.md`：精简版参考

* `docs/Control-Roadmap.md` 第 7 项标记完成

### 8. 验证

1. 整 sln Release x64 编译通过（0 error）
2. controls.exe 浅色/深色/英文下 Calendar 显示正确
3. DateTime 弹层选择日期后正确回写
4. 日期范围选择拖拽/点击交互正常
5. 月/年/十年导航切换正常
6. ColorTheme 深色主题下语义色正确
7. `verify_docs.py` 通过

## 文件变更清单

| 文件                                        | 变更                    |
| ----------------------------------------- | --------------------- |
| `duilib/Control/Calendar.h/.cpp`          | 新增                    |
| `duilib/Control/CalendarFlyout.h/.cpp`    | 新增                    |
| `duilib/Control/DateTime.cpp`             | 修改弹层逻辑                |
| `duilib/duilib_defs.h`                    | 新增 DUI\_CTR\_CALENDAR |
| `duilib/Core/WindowBuilder.cpp`           | 注册控件                  |
| `duilib/duilib.h`                         | include 新头文件          |
| `duilib/duilib.vcxproj` / `.filters`      | 添加文件                  |
| `bin/resources/themes/default/global.xml` | Calendar 皮肤类          |
| `examples/controls/ControlForm.cpp` + XML | 示例                    |
| `examples/ColorTheme/MainForm.cpp` + XML  | 示例                    |
| `docs/Control.md` / `Control.en.md`       | 文档                    |
| `docs/Control-Roadmap.md`                 | 标记完成                  |
| skill 参考文件                                | 同步更新                  |

## 关键设计决策

1. **Calendar 为纯自绘 Control**，不依赖外部 XML 子控件，确保轻量和主题一致性
2. **CalendarFlyout 继承 Flyout**，复用其定位/自动关闭/单活能力
3. **日期范围选择**通过 `calendar_mode="range"` 开启，C++ 接口 `SetDateRange/GetDateRange`
4. **月/年/十年三级导航（层级式）**：标题按钮只"上钻"一级——月视图→年视图→十年视图；十年视图为最粗粒度，标题不再跳回（已移除旧实现"三级循环跳回月视图"的方向错乱）。下钻由网格点击完成：十年视图点年份→年视图、年视图点月份→月视图。"今天"按钮仅导航到当前月份，不提交、不关闭（提交由网格点击或"确定"完成）
5. **DateTime 集成**：仅 `date_calendar` 格式改用新弹层，其他格式保持向后兼容

