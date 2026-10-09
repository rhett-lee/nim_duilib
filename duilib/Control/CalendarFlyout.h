#ifndef UI_CONTROL_CALENDAR_FLYOUT_H_
#define UI_CONTROL_CALENDAR_FLYOUT_H_

#include "duilib/Control/Flyout.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/Calendar.h"
#include <ctime>
#include <functional>
#include <vector>

namespace ui
{

class Calendar;

/** 日历标题按钮：在 Button 基础上自绘一个"向下箭头"（矢量三角形），替代原先依赖字体的 ▾ 字符。
 *
 *  背景：原实现把 ▾(U+25BE) 字符拼接到标题文本末尾来提示"可上钻"。该几何形状字符在部分平台
 *        （Linux/macOS/FreeBSD 等）的默认字体中缺失，会渲染成豆腐块或空白，导致"显示异常"。
 *        改为矢量绘制（IRender + IPath）后，所有平台渲染一致、字体内存无关。
 *
 *  行为：箭头仅在按钮可用时（月视图/年视图：标题可上钻）绘制；十年视图下按钮被禁用，不绘制箭头，
 *        与"已是最粗粒度、无可再上钻层级"的语义一致。箭头颜色跟随按钮文本当前状态色（普通/悬停/按下）。
 */
class DUILIB_API CalendarTitleButton : public Button
{
    typedef Button BaseClass;
public:
    explicit CalendarTitleButton(Window* pWindow) : Button(pWindow) {}
    CalendarTitleButton(const CalendarTitleButton&) = delete;
    CalendarTitleButton& operator=(const CalendarTitleButton&) = delete;
    virtual ~CalendarTitleButton() override = default;

    /** 自绘：先绘制按钮本身（背景/边框/文本），再在其右侧预留区域内绘制向下箭头
     */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

private:
    /** 解析箭头颜色：与按钮文本当前状态色一致（普通 -> 悬停 -> 按下 的回退逻辑）
     */
    UiColor GetArrowColor() const;
};

/** 日历上一月/下一月导航按钮：自绘矢量 chevron（左/右），替代原先依赖字体的 ‹ / › 字符。
 *
 *  背景：原实现用 text="&#x2039;"(‹) / "&#x203A;"(›) 作为导航箭头。这两枚字符属"通用标点"区块，
 *        虽比已修复的 ▾(U+25BE) 几何形状字符覆盖率更好，但仍依赖运行字体有对应字形；在字体缺失或
 *        回退链不达的环境下同样可能变豆腐块。改为矢量绘制（IRender + IPath）后，与标题的向下箭头
 *        同源、全平台一致、字体内存无关，且三个指示符（左/下/右）视觉风格统一。
 *
 *  行为：图标颜色跟随按钮文本当前状态色（普通/悬停/按下）；方向由 XML 的 direction 属性指定
 *        （left=上一月，right=下一月），缺省视为 left。导航按钮在浮层生命周期内始终可用，故始终绘制。
 */
class DUILIB_API CalendarNavButton : public Button
{
    typedef Button BaseClass;
public:
    enum class Direction
    {
        kLeft,  // 上一月：‹
        kRight, // 下一月：›
    };

    explicit CalendarNavButton(Window* pWindow) : Button(pWindow), m_direction(Direction::kLeft) {}
    CalendarNavButton(const CalendarNavButton&) = delete;
    CalendarNavButton& operator=(const CalendarNavButton&) = delete;
    virtual ~CalendarNavButton() override = default;

    /** 自绘：先绘制按钮本身（背景/边框），再在中心绘制矢量 chevron（左/右）
     */
    virtual void Paint(IRender* pRender, const UiRect& rcPaint) override;

protected:
    /** 解析 XML 中的 direction 属性（left/right），其余属性交给基类处理
     */
    virtual void SetAttribute(const DString& strName, const DString& strValue) override;

private:
    /** 解析图标颜色：与按钮文本当前状态色一致（普通 -> 悬停 -> 按下 的回退逻辑）
     */
    UiColor GetArrowColor() const;

    Direction m_direction; // 箭头方向
};

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
 *      pFlyout->AttachDateSelectedEx([](WPARAM wParam, LPARAM lParam, const ui::Calendar::DateRange* pRange) {
 *          // wParam: 0=单选，1=范围
 *          // lParam: 单选时为 time_t 日期值；范围时无意义
 *          // pRange: 仅范围模式非空，含完整 64 位 start/end
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
    * @param [in] lParam 单选模式时为选中日期的 time_t 值；范围模式时无意义。
    * @note 该回调仅适用于单选模式；范围模式请改用 DateSelectedExEvent（可读取完整 64 位起止值）。
    */
    typedef std::function<void(WPARAM wParam, LPARAM lParam)> DateSelectedEvent;

    /** 监听日期选择完成事件（两参数签名，仅适用于单选模式）
    * @param [in] callback 回调函数
    */
    void AttachDateSelected(const DateSelectedEvent& callback);

    /** 日期选择完成事件回调类型（推荐，携带完整的 64 位起止值）
    * @param [in] wParam 0=单选模式，1=范围模式
    * @param [in] lParam 单选模式时为选中日期的 time_t 值；范围模式时无意义
    * @param [in] pRange 仅范围模式（wParam==1）时非空，指向 Calendar::DateRange（含完整 64 位
    *            start/end）；单选模式为 nullptr。该指针仅在本次回调期间有效，不可保存。
    */
    typedef std::function<void(WPARAM wParam, LPARAM lParam, const Calendar::DateRange* pRange)> DateSelectedExEvent;

    /** 监听日期选择完成事件（三参数签名，可获取完整 64 位范围值）
    * @param [in] callback 回调函数
    */
    void AttachDateSelectedEx(const DateSelectedExEvent& callback);

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

    /** 日期选择完成回调（旧版两参数）
    */
    std::vector<DateSelectedEvent> m_dateSelectedCallbacks;

    /** 日期选择完成回调（扩展版三参数，携带完整 64 位范围）
    */
    std::vector<DateSelectedExEvent> m_dateSelectedExCallbacks;

    /** 清除日期回调
    */
    std::vector<DateClearedEvent> m_dateClearedCallbacks;
};

} // namespace ui

#endif // UI_CONTROL_CALENDAR_FLYOUT_H_
