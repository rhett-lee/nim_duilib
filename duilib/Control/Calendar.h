#ifndef UI_CONTROL_CALENDAR_H_
#define UI_CONTROL_CALENDAR_H_

#include "duilib/Core/Control.h"
#include <ctime>

namespace ui
{

/** 日历面板控件（Calendar）：纯自绘月历网格，支持单选/范围选择
 *
 *  功能：
 *  1. 月视图：6 行 × 7 列日期网格，自动填充上月/下月日期（灰色显示）
 *  2. 年视图：12 个月份格子（3 列 × 4 行）
 *  3. 十年视图：12 个年份格子（3 列 × 4 行）
 *  4. 单选模式：点击日期选中，触发 kEventValueChanged
 *  5. 范围模式：点击/拖拽选择起止日期，中间日期高亮
 *  6. 今天高亮：当前日期用主题色圆点标记
 *  7. 悬停效果：鼠标悬停日期格子有 hover 背景
 *
 *  XML 属性：
 *    calendar_mode       single / range（默认 single）
 *    first_day_of_week   monday / sunday（默认 monday）
 *    show_today_button   true / false（默认 true，预留）
 *    min_date / max_date 可选日期范围限制（格式：yyyy-mm-dd）
 *
 *  皮肤类（global.xml）：
 *    calendar              整体容器
 *    calendar_weekday      星期标题
 *    calendar_day          当月日期（正常）
 *    calendar_day_muted    上月/下月日期（灰色）
 *    calendar_day_today    今天（主题色边框/圆点）
 *    calendar_day_selected 选中日期（主题色背景）
 *    calendar_day_range    范围中间日期（浅色背景）
 *    calendar_day_hovered  悬停日期（hover 背景）
 */
class DUILIB_API Calendar : public Control
{
    typedef Control BaseClass;
public:
    explicit Calendar(Window* pWindow);
    Calendar(const Calendar& r) = delete;
    Calendar& operator=(const Calendar& r) = delete;
    virtual ~Calendar() override = default;

    virtual DString GetType() const override;
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

    virtual bool MouseMove(const EventArgs& msg) override;
    virtual bool ButtonDown(const EventArgs& msg) override;
    virtual bool ButtonUp(const EventArgs& msg) override;
    virtual bool MouseLeave(const EventArgs& msg) override;

public:
    /** 选择模式
    */
    enum class Mode : uint32_t
    {
        kSingle = 0,    //单日期选择
        kRange = 1      //日期范围选择
    };

    /** 设置选择模式
    */
    void SetMode(Mode mode);
    Mode GetMode() const { return m_mode; }

    /** 设置当前显示的年月（会触发重绘）
    * @param [in] year 年份（如 2025）
    * @param [in] month 月份（1-12）
    */
    void SetDisplayMonth(int32_t year, int32_t month);

    /** 获取当前显示的年份
    */
    int32_t GetDisplayYear() const { return m_displayYear; }

    /** 获取当前显示的月份（1-12）
    */
    int32_t GetDisplayMonth() const { return m_displayMonth; }

    /** 设置选中的日期（单选模式）
    * @param [in] date 日期，年月日有效即可
    */
    void SetDate(const struct tm& date);

    /** 获取选中的日期（单选模式）
    */
    const struct tm& GetDate() const { return m_selectedDate; }

    /** 设置日期范围（范围模式）
    * @param [in] start 开始日期
    * @param [in] end 结束日期
    */
    void SetDateRange(const struct tm& start, const struct tm& end);

    /** 获取日期范围
    * @param [out] start 开始日期
    * @param [out] end 结束日期
    */
    void GetDateRange(struct tm& start, struct tm& end) const;

    /** 是否有选中的日期/范围
    */
    bool HasSelection() const;

    /** 清除选择
    */
    void ClearSelection();

    /** 设置每周第一天
    * @param [in] dayOfWeek 0=周日, 1=周一（默认 1）
    */
    void SetFirstDayOfWeek(int32_t dayOfWeek);
    int32_t GetFirstDayOfWeek() const { return m_firstDayOfWeek; }

    /** 设置可选日期范围限制
    * @param [in] minDate 最小日期（空表示不限制）
    * @param [in] maxDate 最大日期（空表示不限制）
    */
    void SetDateLimit(const DString& minDate, const DString& maxDate);

    /** 跳转到今天
    */
    void GoToToday();

    /** 上一月/下一月导航
    */
    void NavigatePrev();
    void NavigateNext();

    /** 切换到年视图/十年视图
    */
    void SetViewMode(int32_t viewMode); // 0=月, 1=年, 2=十年
    int32_t GetViewMode() const { return m_viewMode; }

    /** 监听日期变化事件
    * @param [in] callback 日期变化时的回调函数
    * @param [in] callbackID 该回调函数对应的ID
    * 参数说明：
    *   wParam: 0=单选模式，1=范围模式
    *   lParam: 单选模式时为选中日期的 time_t 值（失败为0）；
    *           范围模式时低32位为 start time_t，高32位为 end time_t（仅32位 time_t 有效）
    */
    void AttachDateChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventValueChanged, callback, callbackID); }

    /** 监听视图模式变化事件（月/年/十年切换）
    * @param [in] callback 视图模式变化时的回调函数
    * @param [in] callbackID 该回调函数对应的ID
    * 参数说明：
    *   wParam: 新的视图模式（0=月, 1=年, 2=十年）
    *   lParam: 旧的视图模式（0=月, 1=年, 2=十年）
    * 注意：在日历模式下，视图切换既可能来自标题按钮（上钻），也可能来自网格点击（下钻，
    *       例如年视图点击月份进入月视图、十年视图点击年份进入年视图）。浮层标题栏需要根据
    *       该事件同步刷新，避免标题与当前视图脱节。
    */
    void AttachViewModeChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventViewModeChanged, callback, callbackID); }

    /** 订阅"显示周期变化"事件
     * 当显示的周期（月视图的月份、年/十年视图的年份）发生变化时触发，
     * 例如通过键盘 PgUp/PgDn 翻页、方向键移动焦点跨月/跨年、或导航按钮切换。
     * 浮层标题栏需据此刷新（标题文字 = 当前显示的 年/月/十年区间）。
     * 与 AttachViewModeChanged 配合：视图模式变化（月↔年↔十年）与显示周期变化都会刷新标题，
     * 两条路径都覆盖，避免标题与日历内容脱节。
     */
    void AttachDisplayDateChanged(const EventCallback& callback, EventCallbackID callbackID = 0) { AttachEvent(kEventDisplayDateChanged, callback, callbackID); }

    /** 键盘导航：由 CalendarFlyout 在收到按键时调用（方向键移动焦点、PgUp/PgDn 翻页、Enter/Space 选中或下钻）
     * @param [in] msg kEventKeyDown 事件（含 vkCode / modifierKey）
     * @return true 表示已处理该按键（事件不再继续派发）
     * 注意：Esc 关闭浮层由 Flyout 基类处理，本方法不处理 Esc。
     */
    bool HandleKeyDown(const EventArgs& msg);

    /** 打开时初始化键盘焦点：使焦点环在浮层显示时即出现，且不改变显示范围
     * （优先用已选日期，否则用今天）。仅用于浮层打开场景。
     */
    void SetInitialFocus();

public:
    /** 判断两个日期是否同一天
    */
    static bool IsSameDay(const struct tm& a, const struct tm& b);

    /** 判断日期是否在范围内（含边界）
    */
    static bool IsDateInRange(const struct tm& date, const struct tm& start, const struct tm& end);

    /** 日期比较：a < b 返回 true
    */
    static bool IsDateLess(const struct tm& a, const struct tm& b);

    /** 日期转 time_t（用于比较和事件参数）
    */
    static time_t DateToTimeT(const struct tm& date);

    /** time_t 转日期
    */
    static struct tm TimeTToDate(time_t t);

    /** 解析日期字符串（yyyy-mm-dd）
    */
    static bool ParseDateString(const DString& str, struct tm& date);

    /** 格式化日期字符串
    */
    static DString FormatDateString(const struct tm& date);

    /** 获取今天日期
    */
    static struct tm GetToday();

protected:
    /** 日期格子信息（用于命中测试和绘制）
    */
    struct DayCell
    {
        int32_t year;       //实际年份
        int32_t month;      //实际月份（1-12）
        int32_t day;        //实际日
        UiRect rect;        //绘制区域（DIP）
        bool bCurrentMonth; //是否当前月
        bool bToday;        //是否今天
        bool bSelected;     //是否选中
        bool bInRange;      //是否在范围内（范围模式）
        bool bHovered;      //是否悬停
        bool bDisabled;     //是否超出可选范围
    };

    /** 获取某年某月的日期格子布局
    * @param [in] year 年份
    * @param [in] month 月份（1-12）
    * @param [out] cells 输出 42 个格子（6行7列）
    */
    void GetMonthCells(int32_t year, int32_t month, std::vector<DayCell>& cells) const;

    /** 根据坐标命中日期格子
    * @param [in] pt 鼠标坐标（控件客户区坐标）
    * @param [out] cell 命中格子的信息
    * @return 是否命中
    */
    bool HitTestDay(const UiPoint& pt, DayCell& cell) const;

    /** 绘制月视图
    */
    void DrawMonthView(IRender* pRender, const UiRect& rect);

    /** 绘制年视图（12个月）
    */
    void DrawYearView(IRender* pRender, const UiRect& rect);

    /** 绘制十年视图（12年）
    */
    void DrawDecadeView(IRender* pRender, const UiRect& rect);

    /** 绘制单个日期格子
    */
    void DrawDayCell(IRender* pRender, const DayCell& cell, IFont* pFont);

    /** 绘制星期标题行
    */
    void DrawWeekdayHeader(IRender* pRender, const UiRect& rect, IFont* pFont);

    /** 获取星期标题文字
    * @param [in] index 0=第一天（按 firstDayOfWeek 偏移后）
    */
    DString GetWeekdayText(int32_t index) const;

private:
    /** 选择模式
    */
    Mode m_mode;

    /** 当前显示的年月
    */
    int32_t m_displayYear;
    int32_t m_displayMonth;

    /** 视图模式：0=月, 1=年, 2=十年
    */
    int32_t m_viewMode;

    /** 每周第一天（0=周日, 1=周一）
    */
    int32_t m_firstDayOfWeek;

    /** 选中的日期（单选模式）
    */
    struct tm m_selectedDate;
    bool m_bHasSelectedDate;

    /** 范围选择的起止日期
    */
    struct tm m_rangeStart;
    struct tm m_rangeEnd;
    bool m_bHasRangeStart;
    bool m_bHasRangeEnd;

    /** 范围选择中：鼠标按下但未弹起（正在拖拽）
    */
    bool m_bRangeDragging;
    struct tm m_rangeDragStart;

    /** 范围选择中：已选定起始日，等待用户选结束日
    *   两步点击模式：第一次点击设起始日（不关闭浮层），第二次点击设结束日（触发事件并关闭）
    */
    bool m_bRangeAwaitingEnd;

    /** 悬停的日期格子
    */
    int32_t m_hoverYear;
    int32_t m_hoverMonth;
    int32_t m_hoverDay;
    bool m_bHasHover;

    /** 可选日期范围限制
    */
    struct tm m_minDate;
    struct tm m_maxDate;
    bool m_bHasMinDate;
    bool m_bHasMaxDate;

    /** 当前月份格子缓存（用于命中测试）
    */
    mutable std::vector<DayCell> m_cells;
    mutable bool m_bCellsDirty;

    /** 今天日期缓存
    */
    struct tm m_today;
    bool m_bTodayValid;

    /** 键盘导航用的"焦点日期"（与鼠标悬停 m_hover 分离）
     *  - 月视图：年/月/日均有效
     *  - 年视图：年/月有效（日固定为 1）
     *  - 十年视图：年仅有效（月固定为 m_displayMonth）
     */
    struct tm m_focusDate;
    bool m_bHasFocusDate;

    /** 确保键盘焦点日期已初始化（首次按键或打开时）：优先用已选日期，否则用今天
     */
    void EnsureFocusDate();

    /** 设置键盘焦点日期，并使其可见（必要时切换显示月/年/十年）
     */
    void SetKeyboardFocusDate(const struct tm& date);

    /** 把键盘焦点对齐到当前显示范围（PageUp/PageDown 翻页后调用，保持焦点在可见区域内）
     */
    void SyncFocusToDisplay();

    /** 月视图：按天移动焦点（deltaDays 可正可负，如 ±1 / ±7）
     */
    void MoveFocusDay(int32_t deltaDays);

    /** 年视图：按月份移动焦点（deltaMonths 可正可负，如 ±1 / ±3）
     */
    void MoveFocusMonth(int32_t deltaMonths);

    /** 十年视图：按年份移动焦点（deltaYears 可正可负，如 ±1 / ±3）
     */
    void MoveFocusYear(int32_t deltaYears);

    /** 在月视图下，选中当前键盘焦点所在的日期
     *  - 单选：SetDate（触发 kEventValueChanged，浮层随后关闭）
     *  - 范围：两步选择（第一次设起始日，第二次设结束日）
     * @param [in] msg 触发该操作的键盘事件（用于 SetDate 后检测控件是否已被销毁）
     */
    void SelectFocusedDay(const EventArgs& msg);

    /** 绘制键盘焦点环（在指定矩形内绘制 1px 圆角边框，使用主题色 border_focus_ring）
     */
    void DrawFocusRing(IRender* pRender, const UiRect& rect, float cornerRadius);
};

} // namespace ui

#endif // UI_CONTROL_CALENDAR_H_
