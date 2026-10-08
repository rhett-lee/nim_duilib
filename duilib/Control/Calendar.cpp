#include "Calendar.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Utils/StringConvert.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ui
{

//每行7天，共6行
static const int32_t kDaysPerWeek = 7;
static const int32_t kWeeksPerMonth = 6;
static const int32_t kDaysPerPage = kDaysPerWeek * kWeeksPerMonth; // 42

//年视图/十年视图：3列×4行
static const int32_t kYearGridCols = 3;
static const int32_t kYearGridRows = 4;

Calendar::Calendar(Window* pWindow):
    Control(pWindow),
    m_mode(Mode::kSingle),
    m_displayYear(0),
    m_displayMonth(0),
    m_viewMode(0),
    m_firstDayOfWeek(1), //默认周一
    m_bHasSelectedDate(false),
    m_bHasRangeStart(false),
    m_bHasRangeEnd(false),
    m_bRangeDragging(false),
    m_bRangeAwaitingEnd(false),
    m_hoverYear(0),
    m_hoverMonth(0),
    m_hoverDay(0),
    m_bHasHover(false),
    m_bHasMinDate(false),
    m_bHasMaxDate(false),
    m_bCellsDirty(true),
    m_bTodayValid(false)
{
    //初始化显示为当前年月
    m_today = GetToday();
    m_bTodayValid = true;
    m_displayYear = m_today.tm_year + 1900;
    m_displayMonth = m_today.tm_mon + 1;

    //默认尺寸
    SetFixedWidth(UiFixedInt(280), false, true);
    SetFixedHeight(UiFixedInt(240), false, true);
}

DString Calendar::GetType() const { return DUI_CTR_CALENDAR; }

void Calendar::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("calendar_mode")) {
        if (strValue == _T("range")) {
            SetMode(Mode::kRange);
        }
        else {
            SetMode(Mode::kSingle);
        }
    }
    else if (strName == _T("first_day_of_week")) {
        if (strValue == _T("sunday")) {
            SetFirstDayOfWeek(0);
        }
        else {
            SetFirstDayOfWeek(1);
        }
    }
    else if (strName == _T("min_date")) {
        struct tm date = {0, };
        if (ParseDateString(strValue, date)) {
            m_minDate = date;
            m_bHasMinDate = true;
        }
    }
    else if (strName == _T("max_date")) {
        struct tm date = {0, };
        if (ParseDateString(strValue, date)) {
            m_maxDate = date;
            m_bHasMaxDate = true;
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void Calendar::SetMode(Mode mode)
{
    if (m_mode != mode) {
        m_mode = mode;
        ClearSelection();
        Invalidate();
    }
}

void Calendar::SetDisplayMonth(int32_t year, int32_t month)
{
    if (month < 1) {
        month = 1;
    }
    if (month > 12) {
        month = 12;
    }
    if (m_displayYear != year || m_displayMonth != month) {
        m_displayYear = year;
        m_displayMonth = month;
        m_bCellsDirty = true;
        Invalidate();
    }
}

void Calendar::SetDate(const struct tm& date)
{
    if (date.tm_year <= 0 || date.tm_mon < 0 || date.tm_mday <= 0) {
        return;
    }
    m_selectedDate = date;
    m_bHasSelectedDate = true;
    m_bHasRangeStart = false;
    m_bHasRangeEnd = false;
    Invalidate();

    //触发事件
    time_t t = DateToTimeT(m_selectedDate);
    SendEvent(kEventValueChanged, (WPARAM)0, (LPARAM)t);
}

void Calendar::SetDateRange(const struct tm& start, const struct tm& end)
{
    if (start.tm_year <= 0 || start.tm_mon < 0 || start.tm_mday <= 0) {
        return;
    }
    if (end.tm_year <= 0 || end.tm_mon < 0 || end.tm_mday <= 0) {
        return;
    }
    if (IsDateLess(end, start)) {
        m_rangeStart = end;
        m_rangeEnd = start;
    }
    else {
        m_rangeStart = start;
        m_rangeEnd = end;
    }
    m_bHasRangeStart = true;
    m_bHasRangeEnd = true;
    m_bHasSelectedDate = false;
    Invalidate();

    //触发事件
    time_t tStart = DateToTimeT(m_rangeStart);
    time_t tEnd = DateToTimeT(m_rangeEnd);
    SendEvent(kEventValueChanged, (WPARAM)1, (LPARAM)((tEnd << 32) | (tStart & 0xFFFFFFFF)));
}

void Calendar::GetDateRange(struct tm& start, struct tm& end) const
{
    start = m_rangeStart;
    end = m_rangeEnd;
}

bool Calendar::HasSelection() const
{
    return m_bHasSelectedDate || (m_bHasRangeStart && m_bHasRangeEnd);
}

void Calendar::ClearSelection()
{
    m_bHasSelectedDate = false;
    m_bHasRangeStart = false;
    m_bHasRangeEnd = false;
    m_bRangeDragging = false;
    m_bRangeAwaitingEnd = false;
    Invalidate();
}

void Calendar::SetFirstDayOfWeek(int32_t dayOfWeek)
{
    if (dayOfWeek < 0 || dayOfWeek > 6) {
        return;
    }
    if (m_firstDayOfWeek != dayOfWeek) {
        m_firstDayOfWeek = dayOfWeek;
        m_bCellsDirty = true;
        Invalidate();
    }
}

void Calendar::SetDateLimit(const DString& minDate, const DString& maxDate)
{
    m_bHasMinDate = ParseDateString(minDate, m_minDate);
    m_bHasMaxDate = ParseDateString(maxDate, m_maxDate);
    Invalidate();
}

void Calendar::GoToToday()
{
    if (m_bTodayValid) {
        SetDisplayMonth(m_today.tm_year + 1900, m_today.tm_mon + 1);
    }
}

void Calendar::NavigatePrev()
{
    if (m_viewMode == 0) {
        //月视图：上一月
        int32_t year = m_displayYear;
        int32_t month = m_displayMonth - 1;
        if (month < 1) {
            month = 12;
            --year;
        }
        SetDisplayMonth(year, month);
    }
    else if (m_viewMode == 1) {
        //年视图：上一年
        SetDisplayMonth(m_displayYear - 1, m_displayMonth);
    }
    else {
        //十年视图：上一个十年
        SetDisplayMonth(m_displayYear - 10, m_displayMonth);
    }
}

void Calendar::NavigateNext()
{
    if (m_viewMode == 0) {
        //月视图：下一月
        int32_t year = m_displayYear;
        int32_t month = m_displayMonth + 1;
        if (month > 12) {
            month = 1;
            ++year;
        }
        SetDisplayMonth(year, month);
    }
    else if (m_viewMode == 1) {
        //年视图：下一年
        SetDisplayMonth(m_displayYear + 1, m_displayMonth);
    }
    else {
        //十年视图：下一个十年
        SetDisplayMonth(m_displayYear + 10, m_displayMonth);
    }
}

void Calendar::SetViewMode(int32_t viewMode)
{
    if (viewMode < 0 || viewMode > 2) {
        return;
    }
    if (m_viewMode != viewMode) {
        m_viewMode = viewMode;
        m_bCellsDirty = true;
        Invalidate();
    }
}

void Calendar::GetMonthCells(int32_t year, int32_t month, std::vector<DayCell>& cells) const
{
    cells.clear();
    cells.reserve(kDaysPerPage);

    //获取当月第一天是星期几（0=周日）
    struct tm firstDay = {0, };
    firstDay.tm_year = year - 1900;
    firstDay.tm_mon = month - 1;
    firstDay.tm_mday = 1;
    firstDay.tm_hour = 12; //避免DST问题
    time_t t = std::mktime(&firstDay);
    if (t == (time_t)-1) {
        return;
    }
    struct tm firstDayOfWeek = *std::localtime(&t);
    int32_t wday = firstDayOfWeek.tm_wday; // 0=周日

    //计算网格中第一天（可能属于上月）
    //firstDayOfWeek: 0=周日, 1=周一...
    //m_firstDayOfWeek: 0=周日, 1=周一
    int32_t offset = (wday - m_firstDayOfWeek + kDaysPerWeek) % kDaysPerWeek;
    if (offset < 0) {
        offset = 0;
    }

    //网格第一天
    struct tm gridStart = firstDay;
    gridStart.tm_mday = 1 - offset;
    t = std::mktime(&gridStart);
    if (t == (time_t)-1) {
        return;
    }
    gridStart = *std::localtime(&t);

    //填充42天
    for (int32_t i = 0; i < kDaysPerPage; ++i) {
        struct tm day = gridStart;
        day.tm_mday = gridStart.tm_mday + i;
        t = std::mktime(&day);
        if (t == (time_t)-1) {
            continue;
        }
        day = *std::localtime(&t);

        DayCell cell;
        cell.year = day.tm_year + 1900;
        cell.month = day.tm_mon + 1;
        cell.day = day.tm_mday;
        cell.bCurrentMonth = (day.tm_mon == month - 1);
        cell.bToday = (m_bTodayValid && IsSameDay(day, m_today));
        cell.bSelected = false;
        cell.bInRange = false;
        cell.bHovered = false;
        cell.bDisabled = false;

        //检查是否选中
        if (m_bHasSelectedDate && IsSameDay(day, m_selectedDate)) {
            cell.bSelected = true;
        }

        //检查是否在范围内
        if (m_bHasRangeStart && m_bHasRangeEnd) {
            if (IsSameDay(day, m_rangeStart) || IsSameDay(day, m_rangeEnd)) {
                cell.bSelected = true;
            }
            else if (IsDateInRange(day, m_rangeStart, m_rangeEnd)) {
                cell.bInRange = true;
            }
        }
        else if (m_bHasRangeStart && IsSameDay(day, m_rangeStart)) {
            //范围选择：仅选定起始日、等待结束日时，起始日以选中样式高亮
            cell.bSelected = true;
        }

        //检查是否悬停
        if (m_bHasHover && cell.year == m_hoverYear && cell.month == m_hoverMonth && cell.day == m_hoverDay) {
            cell.bHovered = true;
        }

        //检查是否超出范围限制
        if (m_bHasMinDate && IsDateLess(day, m_minDate)) {
            cell.bDisabled = true;
        }
        if (m_bHasMaxDate && IsDateLess(m_maxDate, day)) {
            cell.bDisabled = true;
        }

        cells.push_back(cell);
    }
}

bool Calendar::HitTestDay(const UiPoint& pt, DayCell& cell) const
{
    if (m_viewMode != 0) {
        return false;
    }
    if (m_bCellsDirty) {
        //需要重新计算，但HitTest是const方法，所以这里只能返回false
        //调用方应在Paint后调用HitTest
        return false;
    }
    for (const auto& c : m_cells) {
        if (c.rect.ContainsPt(pt)) {
            cell = c;
            return true;
        }
    }
    return false;
}

void Calendar::Paint(IRender* pRender, const UiRect& rcPaint)
{
    BaseClass::Paint(pRender, rcPaint);
    if (pRender == nullptr) {
        return;
    }

    UiRect rect = GetRect();
    UiPadding rcPadding = GetControlPadding();
    rect.Deflate(rcPadding);

    //获取字体
    IFont* pFont = GetIFontById(_T("system_regular_14"));

    if (m_viewMode == 0) {
        //月视图：先画星期标题，再画日期网格
        UiRect headerRect = rect;
        headerRect.bottom = headerRect.top + Dpi().GetScaleInt(24);
        DrawWeekdayHeader(pRender, headerRect, pFont);

        UiRect gridRect = rect;
        gridRect.top = headerRect.bottom;
        DrawMonthView(pRender, gridRect);
    }
    else if (m_viewMode == 1) {
        DrawYearView(pRender, rect);
    }
    else {
        DrawDecadeView(pRender, rect);
    }
}

void Calendar::DrawWeekdayHeader(IRender* pRender, const UiRect& rect, IFont* pFont)
{
    if (pFont == nullptr) {
        return;
    }
    int32_t cellWidth = rect.Width() / kDaysPerWeek;
    UiColor textColor = GetUiColor(_T("text_muted"));
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF888888);
    }

    for (int32_t i = 0; i < kDaysPerWeek; ++i) {
        UiRect cellRect;
        cellRect.left = rect.left + i * cellWidth;
        cellRect.right = cellRect.left + cellWidth;
        cellRect.top = rect.top;
        cellRect.bottom = rect.bottom;

        DrawStringParam param;
        param.textRect = cellRect;
        param.dwTextColor = textColor;
        param.pFont = pFont;
        param.uFormat = TEXT_HCENTER | TEXT_VCENTER;
        pRender->DrawString(GetWeekdayText(i), param);
    }
}

DString Calendar::GetWeekdayText(int32_t index) const
{
    //index: 0=第一天（按firstDayOfWeek偏移后）
    static const DString weekdays[] = {
        _T("日"), _T("一"), _T("二"), _T("三"), _T("四"), _T("五"), _T("六")
    };
    int32_t dayIndex = (m_firstDayOfWeek + index) % kDaysPerWeek;
    return weekdays[dayIndex];
}

void Calendar::DrawMonthView(IRender* pRender, const UiRect& rect)
{
    //重新计算格子
    GetMonthCells(m_displayYear, m_displayMonth, m_cells);
    m_bCellsDirty = false;

    int32_t cellWidth = rect.Width() / kDaysPerWeek;
    int32_t cellHeight = rect.Height() / kWeeksPerMonth;

    IFont* pFont = GetIFontById(_T("system_regular_14"));

    for (size_t i = 0; i < m_cells.size(); ++i) {
        DayCell& cell = m_cells[i];
        int32_t row = (int32_t)(i / kDaysPerWeek);
        int32_t col = (int32_t)(i % kDaysPerWeek);

        cell.rect.left = rect.left + col * cellWidth;
        cell.rect.right = cell.rect.left + cellWidth;
        cell.rect.top = rect.top + row * cellHeight;
        cell.rect.bottom = cell.rect.top + cellHeight;

        DrawDayCell(pRender, cell, pFont);
    }
}

void Calendar::DrawDayCell(IRender* pRender, const DayCell& cell, IFont* pFont)
{
    if (pFont == nullptr) {
        return;
    }

    //内边距
    UiRect innerRect = cell.rect;
    innerRect.Deflate(2, 2);

    UiColor textColor;
    UiColor bgColor;
    UiColor borderColor;
    float borderWidth = 0.0f;
    float cornerRadius = 4.0f;

    //确定颜色
    if (cell.bDisabled) {
        textColor = GetUiColor(_T("text_disabled"));
    }
    else if (cell.bSelected) {
        textColor = GetUiColor(_T("text_primary_btn_normal"));
        bgColor = GetUiColor(_T("color_accent"));
    }
    else if (cell.bInRange) {
        textColor = GetUiColor(_T("text_default"));
        bgColor = GetUiColor(_T("color_accent"));
        bgColor = UiColor(bgColor.GetA(), bgColor.GetR(), bgColor.GetG(), bgColor.GetB()); //半透明效果
        //实际使用较浅的背景色
        bgColor = GetUiColor(_T("bg_btn_hovered"));
    }
    else if (cell.bHovered) {
        textColor = GetUiColor(_T("text_default"));
        bgColor = GetUiColor(_T("bg_btn_hovered"));
    }
    else if (cell.bToday) {
        textColor = GetUiColor(_T("color_accent"));
        borderColor = GetUiColor(_T("color_accent"));
        borderWidth = 1.0f;
    }
    else if (!cell.bCurrentMonth) {
        textColor = GetUiColor(_T("text_muted"));
    }
    else {
        textColor = GetUiColor(_T("text_default"));
    }

    //默认颜色回退
    if (textColor.IsEmpty()) {
        textColor = UiColor(0xFF000000);
    }

    //绘制背景
    if (!bgColor.IsEmpty()) {
        UiRectF bgRectF;
        bgRectF.left = (float)innerRect.left;
        bgRectF.top = (float)innerRect.top;
        bgRectF.right = (float)innerRect.right;
        bgRectF.bottom = (float)innerRect.bottom;
        float radius = Dpi().GetScaleFloat(cornerRadius);
        pRender->FillRoundRect(bgRectF, radius, radius, bgColor);
    }

    //绘制边框（今天）
    if (!borderColor.IsEmpty() && borderWidth > 0.1f) {
        UiRectF borderRectF;
        borderRectF.left = (float)innerRect.left;
        borderRectF.top = (float)innerRect.top;
        borderRectF.right = (float)innerRect.right;
        borderRectF.bottom = (float)innerRect.bottom;
        float radius = Dpi().GetScaleFloat(cornerRadius);
        float width = Dpi().GetScaleFloat(borderWidth);
        IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
        if (pRenderFactory != nullptr) {
            std::unique_ptr<IPen> pen(pRenderFactory->CreatePen(borderColor, width));
            pRender->DrawRoundRect(borderRectF, radius, radius, pen.get());
        }
    }

    //绘制文字
    DString text = StringUtil::Printf(_T("%d"), cell.day);
    DrawStringParam param;
    param.textRect = cell.rect;
    param.dwTextColor = textColor;
    param.pFont = pFont;
    param.uFormat = TEXT_HCENTER | TEXT_VCENTER;
    pRender->DrawString(text, param);
}

void Calendar::DrawYearView(IRender* pRender, const UiRect& rect)
{
    IFont* pFont = GetIFontById(_T("system_regular_14"));
    if (pFont == nullptr) {
        return;
    }

    int32_t cellWidth = rect.Width() / kYearGridCols;
    int32_t cellHeight = rect.Height() / kYearGridRows;

    UiColor textColor = GetUiColor(_T("text_default"));
    UiColor mutedColor = GetUiColor(_T("text_muted"));
    UiColor hoverColor = GetUiColor(_T("bg_btn_hovered"));
    UiColor selectedColor = GetUiColor(_T("color_accent"));
    UiColor selectedTextColor = GetUiColor(_T("text_primary_btn_normal"));

    int32_t currentYear = m_displayYear;
    int32_t currentMonth = m_displayMonth;

    for (int32_t i = 0; i < 12; ++i) {
        int32_t row = i / kYearGridCols;
        int32_t col = i % kYearGridCols;
        int32_t month = i + 1;

        UiRect cellRect;
        cellRect.left = rect.left + col * cellWidth;
        cellRect.right = cellRect.left + cellWidth;
        cellRect.top = rect.top + row * cellHeight;
        cellRect.bottom = cellRect.top + cellHeight;

        UiRect innerRect = cellRect;
        innerRect.Deflate(4, 4);

        bool bSelected = (m_bHasSelectedDate && m_selectedDate.tm_year + 1900 == currentYear && m_selectedDate.tm_mon + 1 == month);
        bool bCurrent = (m_bTodayValid && m_today.tm_year + 1900 == currentYear && m_today.tm_mon + 1 == month);

        //背景
        if (bSelected) {
            UiRectF bgRectF;
            bgRectF.left = (float)innerRect.left;
            bgRectF.top = (float)innerRect.top;
            bgRectF.right = (float)innerRect.right;
            bgRectF.bottom = (float)innerRect.bottom;
            float radius = Dpi().GetScaleFloat(4.0f);
            pRender->FillRoundRect(bgRectF, radius, radius, selectedColor);
        }

        //文字
        DString text = StringUtil::Printf(_T("%d月"), month);
        DrawStringParam param;
        param.textRect = cellRect;
        param.dwTextColor = bSelected ? selectedTextColor : (bCurrent ? selectedColor : textColor);
        param.pFont = pFont;
        param.uFormat = TEXT_HCENTER | TEXT_VCENTER;
        pRender->DrawString(text, param);
    }
}

void Calendar::DrawDecadeView(IRender* pRender, const UiRect& rect)
{
    IFont* pFont = GetIFontById(_T("system_regular_14"));
    if (pFont == nullptr) {
        return;
    }

    int32_t cellWidth = rect.Width() / kYearGridCols;
    int32_t cellHeight = rect.Height() / kYearGridRows;

    UiColor textColor = GetUiColor(_T("text_default"));
    UiColor mutedColor = GetUiColor(_T("text_muted"));
    UiColor hoverColor = GetUiColor(_T("bg_btn_hovered"));
    UiColor selectedColor = GetUiColor(_T("color_accent"));
    UiColor selectedTextColor = GetUiColor(_T("text_primary_btn_normal"));

    int32_t startYear = (m_displayYear / 10) * 10;
    int32_t currentYear = m_displayYear;

    for (int32_t i = 0; i < 12; ++i) {
        int32_t row = i / kYearGridCols;
        int32_t col = i % kYearGridCols;
        int32_t year = startYear + i;

        UiRect cellRect;
        cellRect.left = rect.left + col * cellWidth;
        cellRect.right = cellRect.left + cellWidth;
        cellRect.top = rect.top + row * cellHeight;
        cellRect.bottom = cellRect.top + cellHeight;

        UiRect innerRect = cellRect;
        innerRect.Deflate(4, 4);

        bool bSelected = (m_bHasSelectedDate && m_selectedDate.tm_year + 1900 == year);
        bool bCurrent = (m_bTodayValid && m_today.tm_year + 1900 == year);

        //背景
        if (bSelected) {
            UiRectF bgRectF;
            bgRectF.left = (float)innerRect.left;
            bgRectF.top = (float)innerRect.top;
            bgRectF.right = (float)innerRect.right;
            bgRectF.bottom = (float)innerRect.bottom;
            float radius = Dpi().GetScaleFloat(4.0f);
            pRender->FillRoundRect(bgRectF, radius, radius, selectedColor);
        }

        //文字
        DString text = StringUtil::Printf(_T("%d"), year);
        DrawStringParam param;
        param.textRect = cellRect;
        param.dwTextColor = bSelected ? selectedTextColor : (bCurrent ? selectedColor : textColor);
        param.pFont = pFont;
        param.uFormat = TEXT_HCENTER | TEXT_VCENTER;
        pRender->DrawString(text, param);
    }
}

bool Calendar::MouseMove(const EventArgs& msg)
{
    if (m_viewMode == 0) {
        //范围模式拖拽中：实时更新范围终点，提供视觉反馈
        if (m_mode == Mode::kRange && m_bRangeDragging) {
            if (m_bCellsDirty) {
                GetMonthCells(m_displayYear, m_displayMonth, m_cells);
                m_bCellsDirty = false;
            }
            DayCell cell;
            if (HitTestDay(msg.ptMouse, cell)) {
                struct tm date = {0, };
                date.tm_year = cell.year - 1900;
                date.tm_mon = cell.month - 1;
                date.tm_mday = cell.day;
                //以 m_rangeDragStart 为基准，根据鼠标位置确定 start/end
                if (IsDateLess(date, m_rangeDragStart)) {
                    m_rangeStart = date;
                    m_rangeEnd = m_rangeDragStart;
                }
                else {
                    m_rangeStart = m_rangeDragStart;
                    m_rangeEnd = date;
                }
                m_bHasRangeStart = true;
                m_bHasRangeEnd = true;
                Invalidate();
            }
        }
        if (m_bCellsDirty) {
            //强制重新计算
            GetMonthCells(m_displayYear, m_displayMonth, m_cells);
            m_bCellsDirty = false;
        }
        DayCell cell;
        if (HitTestDay(msg.ptMouse, cell)) {
            if (!m_bHasHover || m_hoverYear != cell.year || m_hoverMonth != cell.month || m_hoverDay != cell.day) {
                m_hoverYear = cell.year;
                m_hoverMonth = cell.month;
                m_hoverDay = cell.day;
                m_bHasHover = true;
                Invalidate();
            }
        }
        else {
            if (m_bHasHover) {
                m_bHasHover = false;
                Invalidate();
            }
        }
    }
    return BaseClass::MouseMove(msg);
}

bool Calendar::ButtonDown(const EventArgs& msg)
{
    bool bRet = BaseClass::ButtonDown(msg);
    if (msg.IsSenderExpired()) {
        return false;
    }

    if (m_viewMode == 0) {
        //月视图：选择日期
        if (m_bCellsDirty) {
            GetMonthCells(m_displayYear, m_displayMonth, m_cells);
            m_bCellsDirty = false;
        }
        DayCell cell;
        if (HitTestDay(msg.ptMouse, cell) && !cell.bDisabled) {
            if (m_mode == Mode::kSingle) {
                struct tm date = {0, };
                date.tm_year = cell.year - 1900;
                date.tm_mon = cell.month - 1;
                date.tm_mday = cell.day;
                SetDate(date);
                //事件回调中可能同步关闭浮层导致自身被销毁，需检查过期后再继续
                if (msg.IsSenderExpired()) {
                    return false;
                }
            }
            else {
                //范围模式：支持两步点击（第一次点击设起始日不关闭，第二次点击设结束日）
                struct tm date = {0, };
                date.tm_year = cell.year - 1900;
                date.tm_mon = cell.month - 1;
                date.tm_mday = cell.day;
                m_bRangeDragging = true;
                if (m_bRangeAwaitingEnd) {
                    //第二次点击：保留 m_rangeDragStart（指向第一次的起始日）
                    //实际结束日的确定在 ButtonUp 中完成
                }
                else {
                    //第一次点击或重新开始：设起始日
                    m_rangeDragStart = date;
                    m_rangeStart = date;
                    m_rangeEnd = date;
                    m_bHasRangeStart = true;
                    m_bHasRangeEnd = false;
                    m_bHasSelectedDate = false;
                    Invalidate();
                }
            }
        }
    }
    else if (m_viewMode == 1) {
        //年视图：点击月份进入月视图
        UiRect rect = GetRect();
        int32_t cellWidth = rect.Width() / kYearGridCols;
        int32_t cellHeight = rect.Height() / kYearGridRows;
        int32_t col = (msg.ptMouse.x - rect.left) / cellWidth;
        int32_t row = (msg.ptMouse.y - rect.top) / cellHeight;
        if (col >= 0 && col < kYearGridCols && row >= 0 && row < kYearGridRows) {
            int32_t month = row * kYearGridCols + col + 1;
            if (month >= 1 && month <= 12) {
                SetDisplayMonth(m_displayYear, month);
                SetViewMode(0);
            }
        }
    }
    else {
        //十年视图：点击年份进入年视图
        UiRect rect = GetRect();
        int32_t cellWidth = rect.Width() / kYearGridCols;
        int32_t cellHeight = rect.Height() / kYearGridRows;
        int32_t col = (msg.ptMouse.x - rect.left) / cellWidth;
        int32_t row = (msg.ptMouse.y - rect.top) / cellHeight;
        if (col >= 0 && col < kYearGridCols && row >= 0 && row < kYearGridRows) {
            int32_t year = (m_displayYear / 10) * 10 + row * kYearGridCols + col;
            SetDisplayMonth(year, m_displayMonth);
            SetViewMode(1);
        }
    }
    return bRet;
}

bool Calendar::ButtonUp(const EventArgs& msg)
{
    if (m_viewMode == 0 && m_mode == Mode::kRange && m_bRangeDragging) {
        m_bRangeDragging = false;
        if (m_bCellsDirty) {
            GetMonthCells(m_displayYear, m_displayMonth, m_cells);
            m_bCellsDirty = false;
        }
        DayCell cell;
        if (HitTestDay(msg.ptMouse, cell) && !cell.bDisabled) {
            struct tm date = {0, };
            date.tm_year = cell.year - 1900;
            date.tm_mon = cell.month - 1;
            date.tm_mday = cell.day;

            if (IsSameDay(date, m_rangeDragStart) && !m_bRangeAwaitingEnd) {
                //第一次点击同一天：起始日已在 ButtonDown 中选中并高亮，
                //此处设为"等待结束日"状态，不触发事件、不关闭浮层
                m_rangeStart = date;
                m_rangeEnd = date;
                m_bHasRangeStart = true;
                m_bHasRangeEnd = false;
                m_bRangeAwaitingEnd = true;
                Invalidate();
                return BaseClass::ButtonUp(msg);
            }

            //完成范围选择（第二次点击或拖拽到不同天）
            if (IsDateLess(date, m_rangeDragStart)) {
                m_rangeEnd = m_rangeDragStart;
                m_rangeStart = date;
            }
            else {
                m_rangeStart = m_rangeDragStart;
                m_rangeEnd = date;
            }
            m_bHasRangeStart = true;
            m_bHasRangeEnd = true;
            m_bRangeAwaitingEnd = false;
            Invalidate();

            //触发事件
            time_t tStart = DateToTimeT(m_rangeStart);
            time_t tEnd = DateToTimeT(m_rangeEnd);
            SendEvent(kEventValueChanged, (WPARAM)1, (LPARAM)((tEnd << 32) | (tStart & 0xFFFFFFFF)));
            //事件回调中可能同步关闭浮层导致自身被销毁，需检查过期后再继续
            if (msg.IsSenderExpired()) {
                return false;
            }
        }
    }
    return BaseClass::ButtonUp(msg);
}

bool Calendar::MouseLeave(const EventArgs& msg)
{
    if (m_bHasHover) {
        m_bHasHover = false;
        Invalidate();
    }
    return BaseClass::MouseLeave(msg);
}

bool Calendar::IsSameDay(const struct tm& a, const struct tm& b)
{
    return a.tm_year == b.tm_year && a.tm_mon == b.tm_mon && a.tm_mday == b.tm_mday;
}

bool Calendar::IsDateInRange(const struct tm& date, const struct tm& start, const struct tm& end)
{
    if (IsDateLess(date, start)) {
        return false;
    }
    if (IsDateLess(end, date)) {
        return false;
    }
    return true;
}

bool Calendar::IsDateLess(const struct tm& a, const struct tm& b)
{
    if (a.tm_year != b.tm_year) {
        return a.tm_year < b.tm_year;
    }
    if (a.tm_mon != b.tm_mon) {
        return a.tm_mon < b.tm_mon;
    }
    return a.tm_mday < b.tm_mday;
}

time_t Calendar::DateToTimeT(const struct tm& date)
{
    struct tm t = date;
    t.tm_hour = 12;
    t.tm_min = 0;
    t.tm_sec = 0;
    t.tm_isdst = -1;
    return std::mktime(&t);
}

struct tm Calendar::TimeTToDate(time_t t)
{
    struct tm date = {0, };
#if defined (_WIN32) || defined (_WIN64)
    ::localtime_s(&date, &t);
#else
    ::localtime_r(&t, &date);
#endif
    return date;
}

bool Calendar::ParseDateString(const DString& str, struct tm& date)
{
    if (str.empty()) {
        return false;
    }
    //支持 yyyy-mm-dd 或 yyyy/mm/dd
    DString s = str;
    StringUtil::ReplaceAll(_T("/"), _T("-"), s);
    int32_t year = 0, month = 0, day = 0;
#ifdef DUILIB_UNICODE
    if (::swscanf_s(s.c_str(), _T("%d-%d-%d"), &year, &month, &day) == 3) {
#else
    if (::sscanf_s(s.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
#endif
        if (year >= 1900 && year <= 9999 && month >= 1 && month <= 12 && day >= 1 && day <= 31) {
            date.tm_year = year - 1900;
            date.tm_mon = month - 1;
            date.tm_mday = day;
            return true;
        }
    }
    return false;
}

DString Calendar::FormatDateString(const struct tm& date)
{
    return StringUtil::Printf(_T("%04d-%02d-%02d"), date.tm_year + 1900, date.tm_mon + 1, date.tm_mday);
}

struct tm Calendar::GetToday()
{
    time_t t = std::time(nullptr);
    struct tm today = {0, };
#if defined (_WIN32) || defined (_WIN64)
    ::localtime_s(&today, &t);
#else
    ::localtime_r(&t, &today);
#endif
    return today;
}

} // namespace ui
