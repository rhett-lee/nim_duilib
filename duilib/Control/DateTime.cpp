#include "DateTime.h"
#include "duilib/Core/Window.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Utils/AttributeUtil.h"
#include "duilib/Utils/StringUtil.h"
#include "duilib/Control/CalendarFlyout.h"
#include "duilib/Control/Calendar.h"
#include <sstream>
#include <iomanip>

#include "DateTimeWnd.h"

namespace ui
{
DateTime::DateTime(Window* pWindow):
    LabelTemplate<HBox>(pWindow),
    m_dateTime({0,}),
    m_pDateWindow(nullptr),
    m_pCalendarFlyout(nullptr),
    m_bSuppressCalendarReopen(false),
    m_editFormat(EditFormat::kDateCalendar),
    m_dateSeparator(_T('-'))
{
    //设置默认属性
    SetAttribute(_T("border_size"), _T("1"));
    SetAttribute(_T("border_color"), _T("border_richedit_normal"));
    SetAttribute(_T("text_align"), _T("vcenter"));
    SetAttribute(_T("text_padding"), _T("2,0,0,0"));
    SetAttribute(_T("padding"), _T("1,1,1,1"));
    SetAttribute(_T("spin_class"), _T("rich_edit_spin_box,rich_edit_spin_btn_up,rich_edit_spin_btn_down"));
}

DateTime::~DateTime()
{
    //释放可能残留的 DateTimeWnd 对象（例如编辑尚未结束时控件就被销毁），
    //避免对象泄漏与悬空引用。m_pDateWindow 为空时 delete nullptr 是安全的。
    if (m_pDateWindow != nullptr) {
        DateTimeWnd* pWnd = m_pDateWindow.get();
        m_pDateWindow = nullptr;
        delete pWnd;
    }
}

DString DateTime::GetType() const { return DUI_CTR_DATETIME; }

void DateTime::SetAttribute(const DString& strName, const DString& strValue2)
{
    DString strValue = GetExpandVarStrings(strValue2);
    if (strName == _T("format")) {
        SetStringFormat(strValue);
    }
    else if (strName == _T("edit_format")) {
        if (strValue == _T("date_calendar")) {
            SetEditFormat(EditFormat::kDateCalendar);
        }
        else if (strValue == _T("date_up_down")) {
            SetEditFormat(EditFormat::kDateUpDown);
        }
        else if (strValue == _T("date_time_up_down")) {
            SetEditFormat(EditFormat::kDateTimeUpDown);
        }
        else if (strValue == _T("date_minute_up_down")) {
            SetEditFormat(EditFormat::kDateMinuteUpDown);
        }
        else if (strValue == _T("time_up_down")) {
            SetEditFormat(EditFormat::kTimeUpDown);
        }
        else if (strValue == _T("minute_up_down")) {
            SetEditFormat(EditFormat::kMinuteUpDown);
        }
        else {
            ASSERT(0);
        }
    }
    else if (strName == _T("spin_class")) {
        SetSpinClass(strValue);
    }
    else if (strName == _T("current_time")) {
        //初始化为当前本地时间，方便使用：无需在代码中手动调用 InitLocalTime()
        //取值为 "true" 或 "1" 时生效（与 StringUtil::IsValueTrue 一致）
        if (StringUtil::IsValueTrue(strValue)) {
            InitLocalTime();
        }
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

void DateTime::InitLocalTime()
{
    time_t timeNow = std::time(nullptr);
    struct tm dateTime = {0, };
#if defined (_WIN32) || defined (_WIN64)
    ::localtime_s(&dateTime, &timeNow);
#else
    ::localtime_r(&timeNow, &dateTime);
#endif
    SetDateTime(dateTime);
}

void DateTime::ClearTime()
{
    struct tm dateTime = { 0, };
    SetDateTime(dateTime);
}

const struct tm& DateTime::GetDateTime() const
{
    return m_dateTime;
}

void DateTime::SetDateTime(const struct tm& dateTime)
{
    if (!IsEqual(m_dateTime, dateTime)) {
        m_dateTime = dateTime;

        //更新显示文本
        SetText(GetDateTimeString());
        //触发变化事件
        SendEvent(kEventValueChanged);
    }
}

DString DateTime::GetDateTimeString() const
{
    DString dateTime;
    if (IsValidDateTime()) {
        struct tm tmSystemDate = m_dateTime;
#ifdef DUILIB_UNICODE
        std::wstringstream ss;
#else
        std::stringstream ss;
#endif
        ss << std::put_time(&tmSystemDate, GetStringFormat().c_str());
        dateTime = ss.str();
    }
    return dateTime;
}

bool DateTime::SetDateTimeString(const DString& dateTime)
{
    bool bRet = false;
    DString sFormat = GetStringFormat();
    ASSERT(!sFormat.empty());
    struct tm t = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
#ifdef DUILIB_UNICODE
    std::wistringstream ss(dateTime);
#else
    std::istringstream ss(dateTime);
#endif
    ss >> std::get_time(&t, sFormat.c_str());
    if (ss.fail()) {
        //失败后，智能识别年月日的分隔符
        if (dateTime.find(_T('-')) != DString::npos) {
            StringUtil::ReplaceAll(_T("/"), _T("-"), sFormat);
#ifdef DUILIB_UNICODE
            std::wistringstream ss2(dateTime);
#else
            std::istringstream ss2(dateTime);
#endif
            ss2 >> std::get_time(&t, sFormat.c_str());
            if (!ss2.fail()) {
                m_dateTime = t;
                bRet = true;
                m_dateSeparator = _T('-');
            }
        }
        else if (dateTime.find(_T('/')) != DString::npos) {
            StringUtil::ReplaceAll(_T("-"), _T("/"), sFormat);
#ifdef DUILIB_UNICODE
            std::wistringstream ss2(dateTime);
#else
            std::istringstream ss2(dateTime);
#endif
            ss2 >> std::get_time(&t, sFormat.c_str());
            if (!ss2.fail()) {
                m_dateTime = t;
                bRet = true;
                m_dateSeparator = _T('/');
            }
        }
    }
    else {
        m_dateTime = t;
        bRet = true;
    }
    if (bRet) {
        //如果不包含年月日，需要更新为当日值，否则编辑的时候认为是无效日期
        time_t timeNow = std::time(nullptr);
        struct tm tmTime = { 0, };
#if defined (_WIN32) || defined (_WIN64)
        ::localtime_s(&tmTime, &timeNow);
#else
        ::localtime_r(&timeNow, &tmTime);
#endif
        if (m_dateTime.tm_year < 0) {
            m_dateTime.tm_year = tmTime.tm_year;
        }        
        if (m_dateTime.tm_mon < 0) {
            m_dateTime.tm_mon = tmTime.tm_mon;
        }
        if (m_dateTime.tm_mday < 0) {
            m_dateTime.tm_mday = tmTime.tm_mday;
        }
        if (m_dateTime.tm_hour < 0) {
            m_dateTime.tm_hour = tmTime.tm_hour;
        }
        if (m_dateTime.tm_min < 0) {
            m_dateTime.tm_min = tmTime.tm_min;
        }
        if (m_dateTime.tm_sec < 0) {
            m_dateTime.tm_sec = tmTime.tm_sec;
        }
        time_t timeValue = std::mktime(&m_dateTime);
        ASSERT(timeValue != 0);
        if (timeValue != 0) {
#if defined (_WIN32) || defined (_WIN64)
            ::localtime_s(&m_dateTime, &timeValue);
#else
            ::localtime_r(&timeValue, &m_dateTime);
#endif
        }
    }
    ASSERT(bRet);
    return bRet;
}

bool DateTime::IsEqual(const struct tm& a, const struct tm& b) const
{
    if (a.tm_sec == b.tm_sec   &&
        a.tm_min == b.tm_min   &&
        a.tm_hour == b.tm_hour &&
        a.tm_mday == b.tm_mday &&
        a.tm_mon == b.tm_mon   &&
        a.tm_year == b.tm_year &&
        a.tm_wday == b.tm_wday &&
        a.tm_yday == b.tm_yday &&
        a.tm_isdst == b.tm_isdst) {
        return true;
    }
    return false;
}

bool DateTime::IsValidDateTime() const
{
    const struct tm& a = m_dateTime;
    if (a.tm_sec == 0  &&
        a.tm_min == 0  &&
        a.tm_hour == 0 &&
        a.tm_mday == 0 &&
        a.tm_mon == 0  &&
        a.tm_year == 0 &&
        a.tm_wday == 0 &&
        a.tm_yday == 0 &&
        a.tm_isdst == 0) {
        return false;
    }
    return true;
}

void DateTime::SetStringFormat(const DString& sFormat)
{
    if (!IsInited()) {
        m_sFormat = sFormat;
    }
    else if (m_sFormat != sFormat) {
        m_sFormat = sFormat;

        //更新显示文本
        SetText(GetDateTimeString());
        //触发变化事件
        SendEvent(kEventValueChanged);
    }
}

DString DateTime::GetStringFormat() const
{
    DString sFormat = m_sFormat.c_str();
    if (sFormat.empty()) {
        EditFormat editFormat = GetEditFormat();
        switch (editFormat) {
        case EditFormat::kDateCalendar:
        case EditFormat::kDateUpDown:
            sFormat = _T("%Y-%m-%d");
            break;
        case EditFormat::kDateTimeUpDown:
            sFormat = _T("%Y-%m-%d %H:%M:%S");
            break;
        case EditFormat::kDateMinuteUpDown:
            sFormat = _T("%Y-%m-%d %H:%M");
            break;
        case EditFormat::kTimeUpDown:
            sFormat = _T("%H:%M:%S");
            break;
        case EditFormat::kMinuteUpDown:
            sFormat = _T("%H:%M");
            break;
        default:
            sFormat = _T("%Y-%m-%d");
            break;
        }
        if (m_dateSeparator != _T('-')) {
            DString separator;
            separator = m_dateSeparator;
            StringUtil::ReplaceAll(_T("-"), separator, sFormat);
        }        
    }
    return sFormat;
}

void DateTime::SetEditFormat(EditFormat editFormat)
{
    if (!IsInited()) {
        m_editFormat = editFormat;
    }
    else if (m_editFormat != editFormat) {
        DString oldFormat = GetStringFormat();
        m_editFormat = editFormat;
        if (oldFormat != GetStringFormat()) {
            //更新显示文本
            SetText(GetDateTimeString());
            //触发变化事件
            SendEvent(kEventValueChanged);
        }
    }
}

DateTime::EditFormat DateTime::GetEditFormat() const
{
    return m_editFormat;
}

DString::value_type DateTime::GetDateSeparator() const
{
    return m_dateSeparator;
}

void DateTime::UpdateEditWndPos()
{
    if (m_pDateWindow != nullptr) {
        m_pDateWindow->UpdateWndPos();
    }
}

void DateTime::HandleEvent(const EventArgs& msg)
{
    if (IsDisabledEvents(msg)) {
        //如果是鼠标键盘消息，并且控件是Disabled的，转发给上层控件
        Box* pParent = GetParent();
        if (pParent != nullptr) {
            pParent->SendEventMsg(msg);
        }
        else {
            BaseClass::HandleEvent(msg);
        }
        return;
    }
    if ((msg.eventType == kEventSetCursor)) {
        SetCursor(CursorType::kCursorIBeam);
        return;
    }
    else if (msg.eventType == kEventWindowSize) {
        if (m_pDateWindow != nullptr || m_pCalendarFlyout != nullptr) {
            return;
        }
    }
    else if (msg.eventType == kEventScrollPosChanged) {
        if (m_pDateWindow != nullptr || m_pCalendarFlyout != nullptr) {
            return;
        }
    }
    else if (msg.eventType == kEventSetFocus) {
        if (m_pDateWindow != nullptr || m_pCalendarFlyout != nullptr) {
            return;
        }
        if (m_bSuppressCalendarReopen) {
            //日历浮层刚关闭，焦点恢复触发的 SetFocus 不再重开
            //（可能连续收到多次 SetFocus，标志在 KillFocus/MouseButtonDown 时才清除）
            return;
        }
        if (GetRect().IsZero() && (GetWindow() != nullptr)) {
            //尚未显示，刷新一次窗口，确保控件先确定位置，然后再显示编辑窗口
            GetWindow()->UpdateWindow();
        }
        if (IsFocused() && IsEnabled()) {
            if (GetEditFormat() == EditFormat::kDateCalendar) {
                //使用新的 CalendarFlyout 弹层
                m_pCalendarFlyout = new CalendarFlyout(GetWindow());
                m_pCalendarFlyout->AttachDateSelected([this](WPARAM wParam, LPARAM lParam) {
                    if (wParam == 0) {
                        time_t t = (time_t)lParam;
                        struct tm date = Calendar::TimeTToDate(t);
                        SetDateTime(date);
                    }
                });
                m_pCalendarFlyout->AttachDateCleared([this]() {
                    ClearTime();
                });
                m_pCalendarFlyout->AttachClosed([this](ui::Flyout::CloseReason) {
                    m_pCalendarFlyout = nullptr;
                    //浮层关闭后焦点回落到本控件会触发 SetFocus，需抑制重开
                    m_bSuppressCalendarReopen = true;
                    return true;
                });
                struct tm initDate = IsValidDateTime() ? m_dateTime : Calendar::GetToday();
                if (!m_pCalendarFlyout->ShowAt(this, initDate, ui::Flyout::Placement::Bottom)) {
                    //创建失败时 ShowAt 内部已销毁对象，需置空避免悬空指针
                    m_pCalendarFlyout = nullptr;
                }
            }
            else {
                //其他格式使用统一的 DateTimeWnd 实现
                m_pDateWindow = new DateTimeWnd(this);
                if (m_pDateWindow->Init(this)) {
                    m_pDateWindow->ShowWindow();
                }
                else {
                    delete m_pDateWindow.get();
                    m_pDateWindow = nullptr;
                }
            }
        }
    }
    else if (msg.eventType == kEventKillFocus) {
        m_bSuppressCalendarReopen = false;
        Invalidate();
    }
    else if ((msg.eventType == kEventMouseButtonDown) ||
             (msg.eventType == kEventMouseDoubleClick) ||
             (msg.eventType == kEventMouseRButtonDown)) {
        if (GetWindow() != nullptr) {
            GetWindow()->ReleaseCapture();
        }
        //用户主动点击，清除抑制标志
        m_bSuppressCalendarReopen = false;
        if (IsFocused() && IsEnabled() && (m_pDateWindow == nullptr) && (m_pCalendarFlyout == nullptr)) {
            if (GetEditFormat() == EditFormat::kDateCalendar) {
                //使用新的 CalendarFlyout 弹层
                m_pCalendarFlyout = new CalendarFlyout(GetWindow());
                m_pCalendarFlyout->AttachDateSelected([this](WPARAM wParam, LPARAM lParam) {
                    if (wParam == 0) {
                        //单选模式
                        time_t t = (time_t)lParam;
                        struct tm date = Calendar::TimeTToDate(t);
                        SetDateTime(date);
                    }
                });
                m_pCalendarFlyout->AttachDateCleared([this]() {
                    ClearTime();
                });
                m_pCalendarFlyout->AttachClosed([this](ui::Flyout::CloseReason) {
                    m_pCalendarFlyout = nullptr;
                    //浮层关闭后焦点恢复触发的 SetFocus 不再重开
                    m_bSuppressCalendarReopen = true;
                    return true;
                });
                struct tm initDate = IsValidDateTime() ? m_dateTime : Calendar::GetToday();
                if (!m_pCalendarFlyout->ShowAt(this, initDate, ui::Flyout::Placement::Bottom)) {
                    //创建失败时 ShowAt 内部已销毁对象，需置空避免悬空指针
                    m_pCalendarFlyout = nullptr;
                }
            }
            else {
                //其他格式使用统一的 DateTimeWnd 实现
                m_pDateWindow = new DateTimeWnd(this);
                if (m_pDateWindow->Init(this)) {
                    m_pDateWindow->ShowWindow();
                }
                else {
                    delete m_pDateWindow.get();
                    m_pDateWindow = nullptr;
                }
            }
        }
    }
    else if (msg.eventType == kEventMouseMove) {
        return;
    }
    else if (msg.eventType == kEventMouseButtonUp) {
        return;
    }
    else if (msg.eventType == kEventContextMenu) {
        return;
    }
    else if (msg.eventType == kEventMouseEnter) {
        return;
    }
    else if (msg.eventType == kEventMouseLeave) {
        return;
    }
    else {
        BaseClass::HandleEvent(msg);
    }
}

void DateTime::OnInit()
{
    if (IsInited()) {
        return;
    }
    BaseClass::OnInit();

    if (!IsValidDateTime()) {
        DString text = GetText();
        //将显示的文本内容，转换成日期时间格式
        if (!text.empty()) {
            SetDateTimeString(text);
        }
    }
}

void DateTime::SetSpinClass(const DString& spinClass)
{
    m_spinClass = spinClass;
}

DString DateTime::GetSpinClass() const
{
    return m_spinClass.c_str();
}

void DateTime::SendEventMsg(const EventArgs& msg)
{
    if ((msg.GetSender() == this) && (msg.eventType == kEventKillFocus)) {
        Control* pNewFocus = (Control*)msg.wParam;
        if ((pNewFocus != nullptr) && (GetItemIndex(pNewFocus) != Box::InvalidIndex)) {
            //焦点切换到子控件，不发出KillFocus事件
            return;
        }
    }
    BaseClass::SendEventMsg(msg);
}

void DateTime::EndEditDateTime()
{
    //编辑结束时，必须复位并释放 DateTimeWnd 对象：
    //DateTimeWnd 仅在 m_pDateWindow 中通过弱引用被持有，若不复位，m_pDateWindow 将持续非空，
    //导致后续点击命中 HandleEvent 中的“m_pDateWindow != nullptr 直接返回”分支，
    SendEvent(kEventKillFocus);
    if (m_pDateWindow != nullptr) {
        //再也无法重新进入编辑（编辑一次后点击再也无法编辑日期）。
        DateTimeWnd* pWnd = m_pDateWindow.get();
        m_pDateWindow = nullptr;
        delete pWnd;
    }
}

UiSize DateTime::EstimateText(UiSize szAvailable)
{
    //当未设置日期（显示文本为空）时，使用与日期格式等宽的示例字符串估算，
    //避免 width/height 为 auto 时被估算为 0，导致控件塌陷；
    //已设置日期时，直接使用真实日期文本进行估算。
    DString text = GetText();
    UiSize size;
    if (text.empty()) {
        DString sample = GetSampleDateTimeString();
        if (!sample.empty()) {
            size = BaseClass::EstimateTextWith(sample, szAvailable);
        }
        else {
            size = BaseClass::EstimateText(szAvailable);
        }
    }
    else {
        size = BaseClass::EstimateText(szAvailable);
    }

    //支持 Spin（spin_class 不为空）时，预留 Spin 按钮容器所占的宽度，
    //使 width 为 auto 的 DateTime 在显示态也能为编辑态的 Spin 留出空间，
    //避免进入编辑态后 Spin 被挤压或显示不全。
    if (GetEditFormat() != EditFormat::kDateCalendar) {
        DString spinClass = GetSpinClass();
        if (!spinClass.empty()) {
            int32_t nSpinWidth = GetSpinBoxWidth(spinClass);
            if (nSpinWidth > 0) {
                size.cx += nSpinWidth;
            }
        }
    }
    return size;
}

int32_t DateTime::GetSpinBoxWidth(const DString& spinClass) const
{
    //spin_class 格式："spin容器class,上按钮class,下按钮class"，取首段（容器）的 width
    std::list<DString> classNames = StringUtil::Split(spinClass, _T(","));
    if (classNames.empty()) {
        return 0;
    }
    DString spinBoxClass = classNames.front();
    StringUtil::Trim(spinBoxClass);
    if (spinBoxClass.empty()) {
        return 0;
    }

    //解析 spin 容器类的 width 属性（与 Control::SetClass 的处理一致：先查全局类，再查窗口类）
    DString classAttributes = GlobalManager::Instance().GetClassAttributes(spinBoxClass);
    if (classAttributes.empty() && (GetWindow() != nullptr)) {
        classAttributes = GetWindow()->GetClassAttributes(spinBoxClass);
    }
    if (classAttributes.empty()) {
        return 0;
    }

    std::vector<std::pair<DString, DString>> attributeList;
    if (!AttributeUtil::ParseAttributeList(classAttributes, attributeList)) {
        return 0;
    }

    int32_t nWidth = 0;
    for (const auto& attr : attributeList) {
        if (attr.first == _T("width")) {
            DString value = attr.second;
            StringUtil::Trim(value);
            //仅支持固定像素宽度；auto / 百分比 / stretch 无法在估算阶段静态确定，忽略
            if (!value.empty() && (value != _T("auto")) &&
                (value.find(_T("%")) == DString::npos) &&
                (value.find(_T("stretch")) == DString::npos)) {
                int32_t nRawWidth = StringUtil::StringToInt32(value);
                if (nRawWidth > 0) {
                    nWidth = Dpi().GetScaleInt(nRawWidth);
                }
            }
            break;  //width 属性找到即结束（无论是否可用）
        }
    }
    return nWidth;
}

DString DateTime::GetSampleDateTimeString() const
{
    //使用固定日期（2000-01-01 00:00:00）生成与真实日期等宽的示例字符串，
    //仅用于尺寸估算，不会作为实际显示文本。
    struct tm t = { 0, };
    t.tm_year = 100;   //2000 年
    t.tm_mon  = 0;     //1 月
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min  = 0;
    t.tm_sec  = 0;
    t.tm_wday = 6;     //2000-01-01 为星期六（仅 %a/%A/%w 等格式会用到）
    t.tm_yday = 0;
    t.tm_isdst = -1;   //由 mktime 自动判断夏令时
    DString sFormat = GetStringFormat();
    if (sFormat.empty()) {
        return DString();
    }
#ifdef DUILIB_UNICODE
    std::wstringstream ss;
#else
    std::stringstream ss;
#endif
    ss << std::put_time(&t, sFormat.c_str());
    return ss.str();
}

}//namespace ui
