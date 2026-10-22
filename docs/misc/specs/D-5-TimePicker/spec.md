# D-5 TimePicker 时间选择弹层 + DateTime 日期约束

## 1. 目标

解决 `DateTime.h` 中 `kDateCalendar` 明确"不支持选择时间"的能力缺口：

1. 日历弹层支持选择时间（时/分/秒），新增"确定/取消"提交语义，避免日期一点击就关闭；
2. 新增纯时间选择弹层（无日历）；
3. 在 `DateTime` / `CalendarFlyout` / `Calendar` 三层暴露：
   - `min_date` / `max_date` 日期约束；
   - `disabled_dates` 禁用日期（单日或区间）；
   - `first_day_of_week` 周起始日配置；
   - `show_seconds` 秒列开关。

完成后回写 `docs/misc/Development-Plan.md` 的 D-5 行与跟踪项。

## 2. 现状与根因

- `DateTime::EditFormat::kDateCalendar` 的弹层是 `CalendarFlyout`，其中只有 `Calendar`、今天/清除按钮；日期选择后立即回调并 `Dismiss()`，无法再选时间。
- `Calendar` 已有 `first_day_of_week`、`min_date` / `max_date`、`DayCell::bDisabled`，但：
  - 禁用日期只表示"超出 min/max"，没有业务禁用日期集合；
  - `DateTime` 层没有对应属性，无法把约束传给 `CalendarFlyout`。
- `DateTimeWnd` 的 spin 编辑仍保留，用于 `*_up_down` 格式，不在本次改造范围内。

## 3. 设计决策

### 3.1 新增两个 EditFormat，而不是改变 kDateCalendar

| EditFormat | XML `edit_format` | 弹层内容 | 默认格式（未设置 `format` 时） |
| --- | --- | --- | --- |
| `kDateTimeCalendar` | `date_time_calendar` | 日历 + 时:分[:秒] + 取消/确定 | `%Y-%m-%d %H:%M:%S`（`show_seconds=false` 时为 `%Y-%m-%d %H:%M`） |
| `kTimeCalendar` | `time_calendar` | 时:分[:秒] + 取消/确定（无日历） | `%H:%M:%S`（`show_seconds=false` 时为 `%H:%M`） |

- 不改 `kDateCalendar` 的既有行为（点日期即提交并关闭），避免破坏现有业务。
- `kDateCalendar` 仍为默认值。
- 两个新格式使用同一套 `CalendarFlyout` 时间层，通过 `SetTimeMode(bool bShowCalendar, bool bShowSeconds)` 切换。

### 3.2 提交语义

- `date_time_calendar`：点击日期只更新日历选中态，不关闭、不触发外部回调；时间由 SpinBox 修改；点击"确定"后一次性提交完整 `struct tm`。
- `time_calendar`：日期部分沿用 `ShowAt` 传入的初始日期，仅时间可改；点击"确定"提交。
- "取消"、Esc、点击浮层外部：关闭且不提交。
- `date_calendar`（旧）：保持"点日期 → 回调 → 关闭"；"清除"保持原行为。

### 3.3 时间控件选型

使用 `SpinBox`（D-3 已完成，支持 `min_number` / `max_number` / `step` / `value`），避免在浮层内再弹出 ComboBox 二级窗口：

- 小时：`0..23`，步长 1；
- 分钟：`0..59`，步长 1；
- 秒：`0..59`，步长 1（`show_seconds=false` 时隐藏）；
- 使用 `global.xml` 已有的 `simple simple_border rich_edit_spin` Class，保证跨主题可用。
- XML 属性顺序遵循 D-3 经验：`min_number` / `max_number` 必须写在 `value` 之前。

### 3.4 禁用日期语义

- `Calendar` 新增禁用日期集合，每个元素是闭区间 `[start, end]`。
- 单日禁用即 `start == end`。
- 命中规则按"日期"比较（忽略时分秒），复用 `Calendar::IsDateInRange` / `IsDateLess`。
- 禁用日期与 `min_date` / `max_date` 取并集，统一体现在 `DayCell::bDisabled`。
- 键盘焦点移动后不会停在禁用日：`ClampFocusToLimits` 在 min/max 收敛后继续寻找最近可用日（先向后、再向前，最多跨 366 天）；`SelectFocusedDay` 遇到禁用日直接返回。

### 3.5 浮层 XML 动态生成

现有 `CalendarFlyout` 使用固定 `kCalendarFlyoutXml`。为支持三种形态（仅日历 / 日历+时间 / 仅时间）且不留下大块空白，改为 `BuildXml()` 按当前配置拼接：

- 宽度固定 280；
- 高度 = `16 + 32(标题) + 32(底部) + 日历 240(可选) + 时间行 36(可选)`；
- 仅时间模式：标题行改为 `STRID_PUBLIC_CALENDAR_TIME` 文案，不创建 `Calendar`；
- `InitControls()` 必须容忍 `Calendar` 为空（时间模式）。
- 时间行控件名：`cal_hour` / `cal_min` / `cal_sec`；
- 底部按钮名：`cal_ok` / `cal_cancel`（`date_calendar` 仍为 `cal_today` / `cal_clear`）。

### 3.6 对外 API

`Calendar` 新增：

```cpp
void AddDisabledDate(const DString& date);                 // yyyy-mm-dd
void AddDisabledDateRange(const DString& start, const DString& end);
void ClearDisabledDates();
bool IsDateDisabled(const struct tm& date) const;
```

`CalendarFlyout` 新增：

```cpp
void SetTimeMode(bool bShowCalendar, bool bShowSeconds);   // 必须在 ShowAt 前调用
void SetDisabledDates(const DString& disabledDates);       // 逗号/分号分隔
void ClearDisabledDates();
typedef std::function<void(const struct tm&)> DateTimeSelectedEvent;
void AttachDateTimeSelected(const DateTimeSelectedEvent& callback);
```

`DateTime` 新增：

```cpp
void SetDateLimit(const DString& minDate, const DString& maxDate);
DString GetMinDate() const;
DString GetMaxDate() const;
void SetDisabledDates(const DString& disabledDates);
DString GetDisabledDates() const;
void SetFirstDayOfWeek(int32_t dayOfWeek);                 // 0=周日, 1=周一
int32_t GetFirstDayOfWeek() const;
void SetShowSeconds(bool bShow);
bool IsShowSeconds() const;
```

`disabled_dates` 文本格式：

```text
yyyy-mm-dd
yyyy-mm-dd~yyyy-mm-dd
```

多个项用 `,` 或 `;` 分隔；非法项忽略。

## 4. 精确改动清单

### 4.1 `duilib/Control/Calendar.h` / `Calendar.cpp`

1. 新增公有 API（见 3.6）与私有成员：
   ```cpp
   std::vector<std::pair<struct tm, struct tm>> m_disabledDateRanges;
   ```
2. `SetAttribute` 支持 `disabled_dates`，解析规则与 `CalendarFlyout::SetDisabledDates` 一致。
3. `SetDateLimit` / 禁用日期变化后置 `m_bCellsDirty = true` 并 `Invalidate()`。
4. `GetMonthCells`：在 min/max 判断之后追加 `if (!cell.bDisabled && IsDateDisabled(day)) cell.bDisabled = true;`。
5. `ClampFocusToLimits`：min/max 收敛后，若当前日期被禁用，向后逐日寻找（不超过 max_date），找不到再向前；最多 366 天。
6. `SelectFocusedDay`：在 min/max 判断之后增加 `if (IsDateDisabled(m_focusDate)) return;`。

### 4.2 `duilib/Control/CalendarFlyout.h` / `CalendarFlyout.cpp`

1. 头文件前向声明 `SpinBox`，新增成员：
   ```cpp
   bool m_bTimeMode;          // 是否选择时间
   bool m_bShowCalendar;      // 时间模式下是否同时显示日历
   bool m_bShowSeconds;
   DString m_disabledDates;
   SpinBox* m_pHourSpin;
   SpinBox* m_pMinSpin;
   SpinBox* m_pSecSpin;
   std::vector<DateTimeSelectedEvent> m_dateTimeSelectedCallbacks;
   ```
2. 删除静态 `kCalendarFlyoutXml`，新增 `DString BuildXml() const;` 与 `struct tm ComposeValue() const;`。
3. `ShowAt` 改为 `Flyout::ShowAt(pAnchor, BuildXml(), placement)`。
4. `SetDisabledDates` / `ClearDisabledDates`：保存文本并转发给已创建的 `Calendar`。
5. `InitControls`：
   - 把 `m_disabledDates` 应用到 `Calendar`；
   - 当 `m_bTimeMode` 时查找三个 `SpinBox`，按 `m_initDate` 初始化为 `HH/mm/ss`；
   - 按控件是否存在绑定 `cal_ok` / `cal_cancel`；
   - 日期变化回调：`m_bTimeMode == true` 时只保留选择、不回调、不关闭；否则保持原逻辑；
   - 焦点初始化：有日历用 `Calendar::SetInitialFocus()`；仅时间时把焦点给小时 SpinBox。
6. `ComposeValue()`：日期取 `Calendar::GetDate()`（无日历或未选日期时用 `m_initDate`），时间取 SpinBox 的 `GetValue()`；`show_seconds=false` 时秒置 0；用 `mktime` / `localtime_*` 归一化 `tm_wday` / `tm_yday` / `tm_isdst`。
7. `OnOkClicked()`：遍历 `m_dateTimeSelectedCallbacks` 的副本后 `Dismiss()`。
8. `OnCancelClicked()`：直接 `Dismiss()`。
9. `OnKeyDownMsg`：日历为空时不再转发日历按键，直接走基类；其余保持。

### 4.3 `duilib/Control/DateTime.h` / `DateTime.cpp`

1. `EditFormat` 末尾追加：
   ```cpp
   kDateTimeCalendar, // 年-月-日 时:分:秒，日历 + 时间选择，点“确定”提交
   kTimeCalendar,     // 时:分:秒，时间选择弹层（无日历）
   ```
2. `SetAttribute` 支持：
   | 属性 | 处理 |
   | --- | --- |
   | `min_date` / `max_date` | 存字符串，转发到打开的浮层 |
   | `disabled_dates` | 存字符串，转发 |
   | `first_day_of_week` | `sunday`→0、`monday`→1，也接受 0~6 |
   | `show_seconds` | `StringUtil::IsValueTrue` |
   | `date_time_calendar` / `time_calendar` | 映射到新 EditFormat |
3. 新增成员：`DString m_minDate; DString m_maxDate; DString m_disabledDates; int32_t m_firstDayOfWeek; bool m_bShowSeconds;`
4. 新增 3.6 的 API；setter 在浮层已打开时同步转发。
5. 抽取 `void OpenCalendarFlyout();` 替换 `HandleEvent` 中两处重复创建逻辑；内部：
   ```cpp
   CalendarFlyout* pFlyout = new CalendarFlyout(GetWindow());
   pFlyout->SetTimeMode(IsCalendarTimeFormat(), m_bShowSeconds);
   pFlyout->SetDateLimit(m_minDate, m_maxDate);
   pFlyout->SetDisabledDates(m_disabledDates);
   pFlyout->SetFirstDayOfWeek(m_firstDayOfWeek);
   pFlyout->AttachDateTimeSelected([this](const struct tm& v) { SetDateTime(v); });
   pFlyout->AttachDateSelected(...);  // 旧 date_calendar 行为
   ...
   ```
6. 新增私有 `bool IsCalendarEditFormat() const;`（`kDateCalendar` / `kDateTimeCalendar` / `kTimeCalendar`）与 `bool IsCalendarTimeFormat() const;`。
7. `GetStringFormat` 增加新格式分支，秒由 `m_bShowSeconds` 决定。
8. `EstimateText`：仅 `*_up_down` 格式预留 Spin 宽度，日历类格式不预留。
9. `SetShowSeconds`：在未显式设置 `format` 时刷新文本并触发 `kEventValueChanged`。
10. `SetEditFormat` 兼容旧值，不改变 `date_calendar` 行为。

### 4.4 资源与示例

1. `bin/resources/lang/zh_CN-public.txt` / `en_US-public.txt`：
   - 新增 `STRID_PUBLIC_CALENDAR_TIME = 时间 / Time`。
2. `bin/resources/styles` 无需新增 Class（复用 `simple` / `simple_border` / `rich_edit_spin`）。
3. `bin/resources/themes/default/controls/controls.xml`：新增一行示例：
   ```xml
   <HBox child_margin="4" width="420">
       <Label text_id="STRID_CONTROLS_DATE_TIME" height="24" text_align="vcenter"/>
       <DateTime width="auto" height="24" text="2026-10-21 09:30"
                 edit_format="date_time_calendar" show_seconds="false"
                 min_date="2020-01-01" max_date="2035-12-31"
                 disabled_dates="2026-10-01~2026-10-07,2026-12-25"
                 first_day_of_week="sunday"/>
       <DateTime width="auto" height="24" edit_format="time_calendar" show_seconds="true"/>
   </HBox>
   ```
4. `bin/resources/lang/zh_CN-examples.txt` / `en_US-examples.txt`：新增 `STRID_CONTROLS_DATE_TIME = 日期时间 / Date Time`。

### 4.5 文档与技能

1. `docs/Control.md` / `docs/Control.en.md`：
   - DateTime 属性表补充新 `edit_format` 取值与 5 个新属性；
   - DateTime C++ 接口补充新 API；
   - Calendar 属性表补 `disabled_dates`，说明禁用日期语义与键盘跳过行为；
   - 补充时间弹层的"确定/取消"提交语义。
2. `docs/misc/Control-Roadmap.md` / `.en.md`：DateTime 描述补"日历+时间弹层、日期约束/禁用日期"。
3. `docs/misc/Development-Plan.md`：D-5 标 ✅，P2 待做移除 D-5。
4. 技能参考（`.trae` 与 `.workbuddy` 镜像）：
   - `api-reference.md` DateTime 小节补新属性/API；
   - `framework-extend.md` 追加第八节"D-5 弹层组合扩展经验"（动态 XML、SpinBox 复用、提交语义、禁用日期与键盘焦点）；
   - `.claude/docs/nim-duilib-llm-reference.md` 若含 DateTime 属性表则同步。
5. `docs/misc/specs/D-5-TimePicker/`：spec / tasks / checklist。

## 5. 验证步骤

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  "build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo
python .trae/skills/nim-duilib/scripts/verify_docs.py --extra docs
```

运行时手工验证（用户执行）：

1. 打开控件示例，找到"日期时间"行：
   - `date_time_calendar` 弹层显示日历 + 时/分（秒已关闭）+ 取消/确定；
   - 点日期不关闭，调小时/分钟后点"确定"，控件文本与时间正确；
   - 点"取消"/Esc 不改变原值。
2. `time_calendar` 弹层只显示时间选择，确定后仅时间变化、日期保持。
3. `min_date` / `max_date` 之外日期置灰且不可点击；键盘方向键/翻页不会停在灰日。
4. `disabled_dates` 中的单日与区间置灰；10 月 1~7 日不可选，12 月 25 日不可选。
5. `first_day_of_week="sunday"` 时星期标题以"日"开头。
6. `show_seconds` 切换后显示格式与弹层秒列一致。
7. 中英文切换、浅色/深色主题、100%/150% DPI 下弹层与时间行布局正常。
8. 旧 `date_calendar`、`*_up_down` 行为不变。

## 6. 验收标准

- [ ] 两种新 `edit_format` 可正常打开对应弹层。
- [ ] 日期+时间模式点日期不关闭，只有"确定"提交。
- [ ] 纯时间模式不显示日历，确定后日期部分不变。
- [ ] 取消/Esc/外部点击不提交。
- [ ] `min_date` / `max_date` / `disabled_dates` 在 DateTime 属性、Calendar 属性、C++ API 三条路径均生效。
- [ ] 键盘焦点不会落在禁用日期上，也不会选中禁用日期。
- [ ] `first_day_of_week` 与 `show_seconds` 生效。
- [ ] 旧格式行为无回归。
- [ ] Debug|x64 零错误；`verify_docs.py --extra docs` 零漂移。

## 7. 风险与边界

- `CalendarFlyout` 的内嵌 XML 使用 `SpinBox`，依赖 `WindowBuilder` 已注册的 `DUI_CTR_SPIN_BOX`（已确认存在）。
- 时间模式依赖浮层抢占焦点（`SetNoFocus(false)`）；否则键盘/失焦提交行为在非 Windows 平台不一致——维持现有设置。
- 禁用日期只按日期比较，不处理"某天部分时段禁用"；该场景不在本期范围。
- 12 小时制/AM-PM 不在本期范围，时间统一 24 小时制。

## 8. 崩溃修复与新增代码复查（2026-10-22）

### 8.1 崩溃根因（已修复）

- 现象：点击 `date_time_calendar` / `time_calendar` 的 DateTime，浮层未弹出即 `0xC0000005`。
- 根因：`CalendarFlyout::BuildXml()` 生成的 `<SpinBox>` 把 `class` 放在 `name` 之后。`WindowBuilder` 在 Debug 下断言 `class` 必须是节点第一个属性（原因：`class` 中的同名属性会覆盖节点显式属性），断言触发后进程中断。
- 修复：`cal_hour` / `cal_min` / `cal_sec` 三个 `SpinBox` 与 `cal_ok` 的 `Button` 均把 `class` 调整为第一个属性。
- 复查：最近一次提交新增的 XML 中，所有 `class` 属性（共 4 处，均在 `CalendarFlyout.cpp`）已位于首位；`controls.xml` 新增示例节点不含 `class`；`simple` / `simple_border` / `rich_edit_spin` / `btn_primary_global` 均在 `global.xml` 中有定义。

### 8.2 复查发现的健壮性问题与修复

1. `Calendar::SetDateLimit`（`Calendar.cpp:212`）未校验 `min_date > max_date`。两者顺序颠倒时，`GetMonthCells` 会把所有日期置灰，`ClampFocusToLimits` 也无法收敛。
   - 修复：两个日期均解析成功且 `min > max` 时交换后再保存。
2. `Calendar::SetAttribute("disabled_dates")`（`Calendar.cpp:99`）使用追加语义，与 `CalendarFlyout::SetDisabledDates` / `DateTime::SetDisabledDates` 的"替换"语义不一致；同一控件重复设置会累加。
   - 修复：该属性分支先 `ClearDisabledDates()` 再 `AddDisabledDates()`。
3. `DateTime::SetFirstDayOfWeek`（`DateTime.cpp:569`）未校验入参范围，非法值会被存入成员并转发给浮层。
   - 修复：仅接受 `0..6`，非法值忽略（与 `Calendar::SetFirstDayOfWeek` 一致）。
4. 文档/注释补齐：
   - `Calendar.h` 类注释的 XML 属性列表补充 `disabled_dates`；
   - `DateTime.h` 的 `SetShowSeconds` 注释注明"浮层已打开时仅在下次打开生效"（浮层 XML 在 `ShowAt` 时生成，运行中无法重建）。

### 8.3 复查确认无需改动的点

- 新增 `disabled_dates` 解析使用 `StringUtil::Split`（`wcstok` 语义，分隔符按字符集合处理），`,` / `;` / `~` 的用法正确。
- `Calendar::IsDateDisabled` 只比较年月日；`GetMonthCells`、`SelectFocusedDay`、`ClampFocusToLimits` 三处均已接入。
- `ComposeValue()` 在不显示秒时强制 `tm_sec = 0`，与 `%H:%M` 的显示精度一致，属预期行为。
- 时间模式点击日期只更新日历选中态、由"确定"统一提交；`cal_sec` 不存在时安全置空，无空指针路径。

### 8.4 验证

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  "build\examples.sln" /p:Configuration=Debug /p:Platform=x64 /clp:ErrorsOnly /m:1 /nologo
python .trae/skills/nim-duilib/scripts/verify_docs.py --extra docs
```

- 运行 `bin\controls_d.exe`，点击 `date_time_calendar` 与 `time_calendar` 两个示例，确认浮层正常弹出、无断言/崩溃。
- 旧 `date_calendar` 与 `*_up_down` 格式行为不变。
