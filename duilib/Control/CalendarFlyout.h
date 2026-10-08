#ifndef UI_CONTROL_CALENDAR_FLYOUT_H_
#define UI_CONTROL_CALENDAR_FLYOUT_H_

#include "duilib/Control/Flyout.h"
#include "duilib/Control/Button.h"
#include <ctime>
#include <functional>
#include <vector>

namespace ui
{

class Calendar;

/** 日历浮层窗口（CalendarFlyout）：基于 Flyout 承载 Calendar 控件
 *
 *  功能：
 *  1. 内部自动创建 Calendar 控件 + 头部导航（上一月/标题/下一月）+ 底部按钮（今天/清除）
 *  2. 月/年/十年三级视图导航：点击标题切换
 *  3. 单选模式：选择日期后自动关闭并触发回调
 *  4. 范围模式：选择范围后自动关闭并触发回调
 *
 *  使用方式：
 *  @code
 *      ui::CalendarFlyout* pFlyout = new ui::CalendarFlyout(this);
 *      pFlyout->ShowAt(pAnchorButton, initialDate, ui::Flyout::Placement::Bottom);
 *      pFlyout->AttachDateSelected([](const ui::EventArgs& args) {
 *          // wParam: 0=单选，1=范围
 *          // lParam: 单选时为 time_t 日期值；范围时低32位 start，高32位 end
 *          return true;
 *      });
 *  @endcode
 */
class DUILIB_API CalendarFlyout : public Flyout
{
    typedef Flyout BaseClass;
public:
    /** 构造函数
    * @param [in] pParentWindow 父窗口（锚点控件必须属于该窗口）
    */
    explicit CalendarFlyout(Window* pParentWindow);
    CalendarFlyout(const CalendarFlyout& r) = delete;
    CalendarFlyout& operator=(const CalendarFlyout& r) = delete;
    virtual ~CalendarFlyout() override = default;

    /** 在锚点控件周围显示日历浮层
    * @param [in] pAnchor 锚点控件
    * @param [in] initDate 初始选中日期（单选模式）或范围起始日期（范围模式）
    * @param [in] placement 期望弹出方位
    * @return 显示成功返回 true
    */
    bool ShowAt(Control* pAnchor, const struct tm& initDate, Placement placement = Placement::Bottom);

    /** 设置选择模式
    * @param [in] mode 0=单选，1=范围
    */
    void SetMode(int32_t mode);

    /** 获取选择模式
    */
    int32_t GetMode() const { return m_mode; }

    /** 设置日期范围选择的初始范围
    * @param [in] start 开始日期
    * @param [in] end 结束日期
    */
    void SetInitRange(const struct tm& start, const struct tm& end);

    /** 设置每周第一天
    * @param [in] dayOfWeek 0=周日, 1=周一（默认 1）
    */
    void SetFirstDayOfWeek(int32_t dayOfWeek);

    /** 设置可选日期范围限制
    * @param [in] minDate 最小日期（yyyy-mm-dd，空表示不限制）
    * @param [in] maxDate 最大日期（yyyy-mm-dd，空表示不限制）
    */
    void SetDateLimit(const DString& minDate, const DString& maxDate);

    /** 日期选择完成事件回调类型（选择日期/范围后自动关闭前触发）
    * @param [in] wParam 0=单选模式，1=范围模式
    * @param [in] lParam 单选模式时为选中日期的 time_t 值；
    *            范围模式时低32位为 start time_t，高32位为 end time_t（仅32位 time_t 有效）
    */
    typedef std::function<void(WPARAM wParam, LPARAM lParam)> DateSelectedEvent;

    /** 监听日期选择完成事件（选择日期/范围后自动关闭前触发）
    * @param [in] callback 回调函数
    */
    void AttachDateSelected(const DateSelectedEvent& callback);

    /** 监听清除日期事件（点击"清除"按钮时触发）
    */
    typedef std::function<void()> DateClearedEvent;
    void AttachDateCleared(const DateClearedEvent& callback);

public:
    //WindowImplBase 接口
    //注意：不要重写 GetSkinFolder/GetSkinFile——Flyout 基类实现返回 m_skinFolder（父窗口资源路径）
    //和 m_xml（ShowAt 传入的内嵌 XML），重写为空会导致窗口 XML 加载失败、创建被销毁
    virtual void OnInitWindow() override;

    /** 键盘导航：拦截方向键/PgUp/PgDn/Enter/Space 并转发给内部 Calendar；
     *  Esc 仍由 Flyout 基类处理（关闭浮层）。
     */
    virtual LRESULT OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled) override;

private:
    /** 初始化内部控件
    */
    void InitControls();

    /** 更新标题文本（根据当前视图模式）
    */
    void UpdateTitle();

    /** 上一月/上一年的点击处理
    */
    void OnPrevClicked();

    /** 下一月/下一年的点击处理
    */
    void OnNextClicked();

    /** 标题点击（切换视图模式）
    */
    void OnTitleClicked();

    /** 今天按钮点击
    */
    void OnTodayClicked();

    /** 清除按钮点击
    */
    void OnClearClicked();

    /** 日历控件日期变化事件
    */
    void OnCalendarDateChanged(const EventArgs& msg);

    /** 构造 kEventKeyDown 事件并转发给 Calendar::HandleKeyDown（供 OnKeyDownMsg 调用）
     */
    bool HandleCalendarKey(Calendar* pCalendar, VirtualKeyCode vkCode, uint32_t modifierKey);

    /** 获取内部 Calendar 控件
    */
    Calendar* GetCalendar() const;

private:
    /** 选择模式：0=单选，1=范围
    */
    int32_t m_mode;

    /** 初始日期（单选模式）
    */
    struct tm m_initDate;
    bool m_bHasInitDate;

    /** 初始范围（范围模式）
    */
    struct tm m_initRangeStart;
    struct tm m_initRangeEnd;
    bool m_bHasInitRange;

    /** 每周第一天
    */
    int32_t m_firstDayOfWeek;

    /** 可选日期范围限制
    */
    DString m_minDate;
    DString m_maxDate;

    /** 内部控件指针
    */
    Button* m_pTitleBtn;
    Control* m_pCalendar;

    /** 日期选择完成回调
    */
    std::vector<DateSelectedEvent> m_dateSelectedCallbacks;

    /** 清除日期回调
    */
    std::vector<DateClearedEvent> m_dateClearedCallbacks;
};

} // namespace ui

#endif // UI_CONTROL_CALENDAR_FLYOUT_H_
