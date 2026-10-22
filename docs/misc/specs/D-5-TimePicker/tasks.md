# D-5 TimePicker 任务清单

## 任务 1：Calendar 禁用日期

- [x] 1.1 `Calendar.h` 新增 API 与 `m_disabledDateRanges` 成员。
- [x] 1.2 `Calendar.cpp` 实现 `AddDisabledDate` / `AddDisabledDateRange` / `ClearDisabledDates` / `IsDateDisabled`。
- [x] 1.3 `SetAttribute` 支持 `disabled_dates`（`,` / `;` 分隔，单日或 `start~end`）。
- [x] 1.4 `GetMonthCells` 把禁用日期并入 `bDisabled`。
- [x] 1.5 `ClampFocusToLimits` 跳过禁用日期；`SelectFocusedDay` 拒绝禁用日期。

## 任务 2：CalendarFlyout 时间层

- [x] 2.1 头文件新增 `SetTimeMode` / `SetDisabledDates` / `ClearDisabledDates` / `DateTimeSelectedEvent` / `AttachDateTimeSelected` 及成员。
- [x] 2.2 用 `BuildXml()` 动态生成三种形态的浮层 XML。
- [x] 2.3 `ShowAt` 使用动态 XML；`InitControls` 容忍无 Calendar。
- [x] 2.4 初始化并读取 `cal_hour` / `cal_min` / `cal_sec`。
- [x] 2.5 日期变化回调区分时间模式：时间模式不提交、不关闭。
- [x] 2.6 实现 `cal_ok` / `cal_cancel` 与 `ComposeValue()`（`mktime` 归一化）。
- [x] 2.7 仅时间模式焦点落到小时 SpinBox；键盘路径无 Calendar 时安全回退。

## 任务 3：DateTime 新格式与约束

- [x] 3.1 `EditFormat` 新增 `kDateTimeCalendar` / `kTimeCalendar`，XML 映射 `date_time_calendar` / `time_calendar`。
- [x] 3.2 新增 `min_date` / `max_date` / `disabled_dates` / `first_day_of_week` / `show_seconds` 属性与成员。
- [x] 3.3 新增 C++ API，并在浮层已打开时转发。
- [x] 3.4 抽取 `OpenCalendarFlyout()`，统一创建并应用约束、时间模式、回调。
- [x] 3.5 `GetStringFormat` 支持新格式与秒开关；`EstimateText` 不为日历格式预留 Spin 宽度。
- [x] 3.6 `SetShowSeconds` 刷新文本与事件。

## 任务 4：示例与资源

- [x] 4.1 公共语言资源新增 `STRID_PUBLIC_CALENDAR_TIME`。
- [x] 4.2 `controls.xml` 新增日期时间 / 纯时间两个示例。
- [x] 4.3 示例语言资源新增 `STRID_CONTROLS_DATE_TIME`。

## 任务 5：文档与技能

- [x] 5.1 `docs/Control.md` / `docs/Control.en.md` 更新 DateTime / Calendar。
- [x] 5.2 `docs/misc/Control-Roadmap.md` / `.en.md` 更新描述。
- [x] 5.3 `docs/misc/Development-Plan.md` D-5 标 ✅。
- [x] 5.4 `.trae` / `.workbuddy` 的 `api-reference.md`、`framework-extend.md` 同步。
- [x] 5.5 `.claude/docs/nim-duilib-llm-reference.md` 如含对应条目则同步。

## 任务 6：验证

- [x] 6.1 Debug|x64 编译零错误。
- [x] 6.2 `verify_docs.py --extra docs` 零漂移。
- [x] 6.3 代码自查：所有权、空指针、回调重入、`Dismiss` 后不访问已销毁对象。

## 任务 7：崩溃修复与复查（2026-10-22）

- [x] 7.1 `CalendarFlyout::BuildXml` 的 `SpinBox` / `Button` 把 `class` 移到第一个属性（崩溃根因）。
- [x] 7.2 复查最近提交全部 XML：新增 `class` 共 4 处且均位于首位；示例节点无 `class`。
- [x] 7.3 `Calendar::SetDateLimit`：`min > max` 时交换。
- [x] 7.4 `Calendar::SetAttribute("disabled_dates")` 改为替换语义（先 Clear 再 Add）。
- [x] 7.5 `DateTime::SetFirstDayOfWeek` 校验 `0..6`，非法值忽略。
- [x] 7.6 注释补齐：`Calendar.h` 属性列表加 `disabled_dates`；`DateTime.h` 的 `SetShowSeconds` 注明下次打开生效。
- [x] 7.7 Debug|x64 编译 + `verify_docs.py --extra docs`。
- [ ] 7.8 运行 `controls_d.exe` 点击两个新示例，确认浮层正常弹出（用户验证）。
