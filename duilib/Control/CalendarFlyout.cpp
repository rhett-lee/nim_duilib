#include "CalendarFlyout.h"
#include "duilib/Control/Calendar.h"
#include "duilib/Control/Button.h"
#include "duilib/Core/WindowBuilder.h"
#include "duilib/Core/Keycode.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Box.h"
#include "duilib/Utils/StringUtil.h"

namespace ui
{

static const DString kCalendarFlyoutXml = _T("<Window size=\"280,320\" caption=\"0,0,0,0\" use_system_caption=\"false\" shadow_type=\"default\" shadow_attached=\"true\" layered_window=\"true\" size_box=\"0,0,0,0\">\n"
                                              "  <VBox bkcolor=\"bg_window_card\" width=\"280\" height=\"320\" padding=\"8,8,8,8\">\n"
                                              "    <HBox height=\"32\" child_valign=\"center\">\n"
                                              "      <Button name=\"cal_prev\" text=\"&#x2039;\" width=\"28\" height=\"28\" font=\"system_bold_16\" normal_text_color=\"text_default\" hot_text_color=\"color_accent\" pushed_text_color=\"color_accent\" tooltip_text_id=\"STRID_PUBLIC_CALENDAR_PREV_MONTH\"/>\n"
                                              "      <Control/>\n"
                                              "      <Button name=\"cal_title\" text=\"2025-01\" width=\"auto\" height=\"28\" font=\"system_bold_14\" normal_text_color=\"text_default\" hot_text_color=\"color_accent\" pushed_text_color=\"color_accent\" tooltip_text_id=\"STRID_PUBLIC_CALENDAR_SWITCH_VIEW\" text_padding=\"8,0,8,0\" border_size=\"1\" border_round=\"4,4\" normal_border_color=\"border_control_normal\" hovered_border_color=\"border_btn_hovered\" hovered_color=\"bg_btn_hovered\" pressed_color=\"bg_btn_pressed\" disabled_text_color=\"text_muted\" disabled_border_color=\"border_control_disabled\"/>\n"
                                              "      <Control/>\n"
                                              "      <Button name=\"cal_next\" text=\"&#x203A;\" width=\"28\" height=\"28\" font=\"system_bold_16\" normal_text_color=\"text_default\" hot_text_color=\"color_accent\" pushed_text_color=\"color_accent\" tooltip_text_id=\"STRID_PUBLIC_CALENDAR_NEXT_MONTH\"/>\n"
                                              "    </HBox>\n"
                                              "    <Calendar name=\"cal_grid\" height=\"240\" width=\"stretch\" calendar_mode=\"single\"/>\n"
                                              "    <HBox height=\"32\" child_valign=\"center\">\n"
                                              "      <Button name=\"cal_today\" text_id=\"STRID_PUBLIC_CALENDAR_TODAY\" width=\"60\" height=\"28\" font=\"system_regular_12\" text_color=\"color_accent\"/>\n"
                                              "      <Control/>\n"
                                              "      <Button name=\"cal_clear\" text_id=\"STRID_PUBLIC_CALENDAR_CLEAR\" width=\"60\" height=\"28\" font=\"system_regular_12\" text_color=\"text_muted\"/>\n"
                                              "    </HBox>\n"
                                              "  </VBox>\n"
                                              "</Window>");

CalendarFlyout::CalendarFlyout(Window* pParentWindow):
    Flyout(pParentWindow),
    m_mode(0),
    m_bHasInitDate(false),
    m_bHasInitRange(false),
    m_firstDayOfWeek(1),
    m_pTitleBtn(nullptr),
    m_pCalendar(nullptr)
{
    m_initDate = Calendar::GetToday();
    m_initRangeStart = m_initDate;
    m_initRangeEnd = m_initDate;
}

bool CalendarFlyout::ShowAt(Control* pAnchor, const struct tm& initDate, Placement placement)
{
    m_initDate = initDate;
    m_bHasInitDate = true;
    //日历需要接收键盘事件（方向键/回车导航），故浮层必须抢占焦点。
    //DateTime 侧已有 m_bSuppressCalendarReopen 标志，关闭后焦点回落不会造成重开死循环。
    SetNoFocus(false);
    return Flyout::ShowAt(pAnchor, kCalendarFlyoutXml, placement);
}

void CalendarFlyout::SetMode(int32_t mode)
{
    m_mode = mode;
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        pCalendar->SetMode(mode == 0 ? Calendar::Mode::kSingle : Calendar::Mode::kRange);
    }
}

void CalendarFlyout::SetInitRange(const struct tm& start, const struct tm& end)
{
    m_initRangeStart = start;
    m_initRangeEnd = end;
    m_bHasInitRange = true;
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        pCalendar->SetDateRange(start, end);
    }
}

void CalendarFlyout::SetFirstDayOfWeek(int32_t dayOfWeek)
{
    m_firstDayOfWeek = dayOfWeek;
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        pCalendar->SetFirstDayOfWeek(dayOfWeek);
    }
}

void CalendarFlyout::SetDateLimit(const DString& minDate, const DString& maxDate)
{
    m_minDate = minDate;
    m_maxDate = maxDate;
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        pCalendar->SetDateLimit(minDate, maxDate);
    }
}

void CalendarFlyout::AttachDateSelected(const DateSelectedEvent& callback)
{
    if (callback) {
        m_dateSelectedCallbacks.push_back(callback);
    }
}

void CalendarFlyout::AttachDateCleared(const DateClearedEvent& callback)
{
    if (callback) {
        m_dateClearedCallbacks.push_back(callback);
    }
}

void CalendarFlyout::OnInitWindow()
{
    BaseClass::OnInitWindow();
    InitControls();
}

void CalendarFlyout::InitControls()
{
    Window* pThisWindow = this;
    if (pThisWindow == nullptr) {
        return;
    }

    //查找内部控件
    m_pTitleBtn = dynamic_cast<Button*>(pThisWindow->FindControl(_T("cal_title")));
    m_pCalendar = pThisWindow->FindControl(_T("cal_grid"));
    Calendar* pCalendar = GetCalendar();

    if (pCalendar != nullptr) {
        //应用初始设置
        if (m_mode == 1) {
            pCalendar->SetMode(Calendar::Mode::kRange);
        }
        pCalendar->SetFirstDayOfWeek(m_firstDayOfWeek);
        if (!m_minDate.empty() || !m_maxDate.empty()) {
            pCalendar->SetDateLimit(m_minDate, m_maxDate);
        }
        if (m_bHasInitRange && (m_mode == 1)) {
            //范围模式：用初始范围
            pCalendar->SetDateRange(m_initRangeStart, m_initRangeEnd);
            pCalendar->SetDisplayMonth(m_initRangeStart.tm_year + 1900, m_initRangeStart.tm_mon + 1);
        }
        else if (m_bHasInitDate && (m_mode == 0)) {
            //单选模式：用初始日期
            pCalendar->SetDate(m_initDate);
            pCalendar->SetDisplayMonth(m_initDate.tm_year + 1900, m_initDate.tm_mon + 1);
        }
        else {
            pCalendar->GoToToday();
        }

        //绑定日期变化事件
        pCalendar->AttachDateChanged([this](const EventArgs& msg) {
            //触发日期选择回调
            std::vector<DateSelectedEvent> callbacks = m_dateSelectedCallbacks;
            for (const DateSelectedEvent& callback : callbacks) {
                callback(msg.wParam, msg.lParam);
            }
            Dismiss();
            return true;
        });

        //绑定视图模式变化事件：年/十年视图下通过网格点击下钻（年->月、十年->年）时，
        //标题栏需同步刷新，否则标题会停留在上一级视图的文字。
        pCalendar->AttachViewModeChanged([this](const EventArgs&) {
            UpdateTitle();
            return true;
        });

        //绑定显示周期变化事件：键盘 PgUp/PgDn 翻页、方向键移动焦点跨月/跨年、导航按钮切换等
        //都会导致显示周期（月/年/十年）变化，标题栏（显示当前 年/月/十年区间）需同步刷新。
        //通过事件统一处理，可覆盖所有路径（含键盘导航），避免标题与日历内容脱节。
        pCalendar->AttachDisplayDateChanged([this](const EventArgs&) {
            UpdateTitle();
            return true;
        });

        //键盘导航：打开时初始化键盘焦点，使焦点环立即出现（优先用已选日期，否则用今天）
        pCalendar->SetInitialFocus();

        //让 Calendar 控件获得键盘焦点：延迟到窗口显示完成（SetWindowForeground）之后执行，
        //避免焦点被重置，从而方向键/回车导航生效。
        GlobalManager::Instance().Thread().PostDelayedTask(
            kThreadUI,
            UiBind(this, [this]() {
                Calendar* pCal = GetCalendar();
                if (pCal != nullptr && IsWindow() && (pCal->GetWindow() != nullptr)) {
                    pCal->SetFocus();
                }
            }),
            0);
    }

    //上一月/下一月按钮
    Control* pPrevBtn = pThisWindow->FindControl(_T("cal_prev"));
    if (pPrevBtn != nullptr) {
        pPrevBtn->AttachClick([this](const EventArgs&) {
            OnPrevClicked();
            return true;
        });
    }
    Control* pNextBtn = pThisWindow->FindControl(_T("cal_next"));
    if (pNextBtn != nullptr) {
        pNextBtn->AttachClick([this](const EventArgs&) {
            OnNextClicked();
            return true;
        });
    }

    //标题按钮点击（切换视图）
    if (m_pTitleBtn != nullptr) {
        m_pTitleBtn->AttachClick([this](const EventArgs&) {
            OnTitleClicked();
            return true;
        });
    }

    //今天按钮
    Control* pTodayBtn = pThisWindow->FindControl(_T("cal_today"));
    if (pTodayBtn != nullptr) {
        pTodayBtn->AttachClick([this](const EventArgs&) {
            OnTodayClicked();
            return true;
        });
    }

    //清除按钮
    Control* pClearBtn = pThisWindow->FindControl(_T("cal_clear"));
    if (pClearBtn != nullptr) {
        pClearBtn->AttachClick([this](const EventArgs&) {
            OnClearClicked();
            return true;
        });
    }

    UpdateTitle();
}

void CalendarFlyout::UpdateTitle()
{
    Calendar* pCalendar = GetCalendar();
    if (pCalendar == nullptr || m_pTitleBtn == nullptr) {
        return;
    }

    int32_t viewMode = pCalendar->GetViewMode();
    int32_t year = pCalendar->GetDisplayYear();
    int32_t month = pCalendar->GetDisplayMonth();

    DString text;
    if (viewMode == 0) {
        //月视图：yyyy-MM（语言无关的数字格式）
        text = StringUtil::Printf(_T("%04d-%02d"), year, month);
    }
    else if (viewMode == 1) {
        //年视图：yyyy
        text = StringUtil::Printf(_T("%04d"), year);
    }
    else {
        //十年视图：yyyy - yyyy
        int32_t startYear = (year / 10) * 10;
        text = StringUtil::Printf(_T("%04d - %04d"), startYear, startYear + 9);
    }

    //标题按钮的"可交互/禁用"状态需与当前视图层级一致：
    // - 月视图 / 年视图：标题可点击上钻（月→年→十年），保留 ▾ 提示"可展开"。
    // - 十年视图（viewMode==2）：已是最粗粒度层级，标题再无可上钻的视图，
    //   故将按钮置为禁用状态并去掉 ▾，使"点击无反应"在视觉上成立，
    //   避免用户误以为还能继续上钻（配合 disabled_text_color / disabled_border_color 呈现禁用外观）。
    if (viewMode == 2) {
        m_pTitleBtn->SetEnabled(false);
    }
    else {
        m_pTitleBtn->SetEnabled(true);
        text += _T(" ▾");
    }

    m_pTitleBtn->SetText(text);
}

void CalendarFlyout::OnPrevClicked()
{
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        pCalendar->NavigatePrev();
        UpdateTitle();
    }
}

void CalendarFlyout::OnNextClicked()
{
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        pCalendar->NavigateNext();
        UpdateTitle();
    }
}

void CalendarFlyout::OnTitleClicked()
{
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        int32_t mode = pCalendar->GetViewMode();
        if (mode == 0) {
            pCalendar->SetViewMode(1); //月 -> 年（上钻一级，粒度更粗）
        }
        else if (mode == 1) {
            pCalendar->SetViewMode(2); //年 -> 十年（上钻一级，粒度更粗）
        }
        else {
            //十年视图已是最粗粒度，标题无可再上钻的层级，保持不动。
            //（不再像旧实现那样跳回月视图，避免跨越年视图造成的方向错乱）
        }
        //标题刷新由 AttachViewModeChanged 回调统一处理（SetViewMode 触发），此处无需重复调用。
    }
}

void CalendarFlyout::OnTodayClicked()
{
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        //仅导航到当前月份，不提交日期、不关闭浮层。
        //日期提交仍由网格点击（单选）或"确定"按钮完成，避免一次误触就直接提交"今天"并关闭。
        pCalendar->GoToToday();
        UpdateTitle();
    }
}

void CalendarFlyout::OnClearClicked()
{
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        pCalendar->ClearSelection();
    }
    //触发清除回调
    std::vector<DateClearedEvent> callbacks = m_dateClearedCallbacks;
    for (const DateClearedEvent& callback : callbacks) {
        callback();
    }
    Dismiss();
}

Calendar* CalendarFlyout::GetCalendar() const
{
    if (m_pCalendar == nullptr) {
        return nullptr;
    }
    return dynamic_cast<Calendar*>(m_pCalendar);
}

LRESULT CalendarFlyout::OnKeyDownMsg(VirtualKeyCode vkCode, uint32_t modifierKey, const NativeMsg& nativeMsg, bool& bHandled)
{
    Calendar* pCalendar = GetCalendar();
    if (pCalendar != nullptr) {
        bool bHandledLocal = false;
        switch (vkCode) {
        case kVK_LEFT:
        case kVK_RIGHT:
        case kVK_UP:
        case kVK_DOWN:
        case kVK_PRIOR: //PageUp
        case kVK_NEXT:  //PageDown
            //方向键 / 翻页：始终导航日历（按钮不使用这些键，无冲突）
            bHandledLocal = HandleCalendarKey(pCalendar, vkCode, modifierKey);
            break;
        case kVK_RETURN:
        case kVK_SPACE:
            //Enter / Space：仅当日历自身获得焦点时用于选中 / 下钻；
            //若焦点在按钮上则交由基类处理按钮激活，避免双重响应。
            if (GetFocusControl() == pCalendar) {
                bHandledLocal = HandleCalendarKey(pCalendar, vkCode, modifierKey);
            }
            break;
        default:
            break;
        }
        if (bHandledLocal) {
            bHandled = true;
            return 0;
        }
    }
    //其余按键（含 Esc 关闭浮层）交由基类处理
    return BaseClass::OnKeyDownMsg(vkCode, modifierKey, nativeMsg, bHandled);
}

bool CalendarFlyout::HandleCalendarKey(Calendar* pCalendar, VirtualKeyCode vkCode, uint32_t modifierKey)
{
    if (pCalendar == nullptr) {
        return false;
    }
    EventArgs keyMsg;
    keyMsg.eventType = kEventKeyDown;
    keyMsg.vkCode = vkCode;
    keyMsg.modifierKey = modifierKey;
    keyMsg.SetSender(pCalendar); //SetDate 后浮层可能关闭并销毁 Calendar，IsSenderExpired 据此判断
    return pCalendar->HandleKeyDown(keyMsg);
}

} // namespace ui
