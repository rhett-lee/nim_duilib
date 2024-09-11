#include "DateTime.h"
#include "DateTimeWnd_Windows.h"
#include <sstream>

namespace ui
{
DateTime::DateTime(Window* pWindow):
    Label(pWindow),
    m_dateTime({0,}),
    m_pDateWindow(nullptr),
    m_editFormat(EditFormat::kDateCalendar),
    m_dateSeparator(_T('-'))
{
    //设置默认属性
    SetAttribute(_T("border_size"), _T("1"));
    SetAttribute(_T("border_color"), _T("gray"));
    SetAttribute(_T("text_align"), _T("vcenter"));
    SetAttribute(_T("text_padding"), _T("2,0,0,0"));
}

DateTime::~DateTime()
{
}

DString DateTime::GetType() const { return DUI_CTR_DATETIME; }

void DateTime::SetAttribute(const DString& strName, const DString& strValue)
{
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
    else {
        __super::SetAttribute(strName, strValue);
    }
}

void DateTime::InitLocalTime()
{
    time_t timeNow = std::time(nullptr);
    std::tm dateTime = {0, };
    ::localtime_s(&dateTime, &timeNow);
    SetDateTime(dateTime);
}

void DateTime::ClearTime()
{
    std::tm dateTime = { 0, };
    SetDateTime(dateTime);
}

const std::tm& DateTime::GetDateTime() const
{
    return m_dateTime;
}

void DateTime::SetDateTime(const std::tm& dateTime)
{
    if (!IsEqual(m_dateTime, dateTime)) {
        m_dateTime = dateTime;

        //更新显示文本
        SetText(GetDateTimeString());
        //触发变化事件
        SendEvent(kEventValueChange);
    }
}

DString DateTime::GetDateTimeString() const
{
    DString dateTime;
    if (IsValidDateTime()) {
        std::tm tmSystemDate = m_dateTime;
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
    std::tm t = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
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
        std::tm tmTime = { 0, };
        ::localtime_s(&tmTime, &timeNow);
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
            ::localtime_s(&m_dateTime, &timeValue);
        }
    }
    ASSERT(bRet);
    return bRet;
}

bool DateTime::IsEqual(const std::tm& a, const std::tm& b) const
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
    const std::tm& a = m_dateTime;
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
        SendEvent(kEventValueChange);
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
            SendEvent(kEventValueChange);
        }
    }
}

DateTime::EditFormat DateTime::GetEditFormat() const
{
    return m_editFormat;
}

void DateTime::UpdateEditWndPos()
{
#ifdef DUILIB_BUILD_FOR_WIN
    if (m_pDateWindow != nullptr) {
        m_pDateWindow->UpdateWndPos();
    }
#endif
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
            __super::HandleEvent(msg);
        }
        return;
    }
    if ((msg.eventType == kEventSetCursor)) {
        SetCursor(CursorType::kCursorIBeam);
        return;
    }
    if (msg.eventType == kEventWindowSize) {
        if (m_pDateWindow != nullptr) {
            return;
        }
    }
    if (msg.eventType == kEventScrollChange) {
        if (m_pDateWindow != nullptr) {
            return;
        }
    }
    if (msg.eventType == kEventSetFocus) {
        if (m_pDateWindow != nullptr) {
            return;
        }
        if (GetRect().IsZero() && (GetWindow() != nullptr)) {
            //尚未显示，刷新一次窗口，确保控件先确定位置，然后再显示编辑窗口
            GetWindow()->UpdateWindow();
        }
        if (IsFocused()) {
#ifdef DUILIB_BUILD_FOR_WIN
            m_pDateWindow = new DateTimeWnd(this);
            if (m_pDateWindow->Init(this)) {
                m_pDateWindow->ShowWindow();
            }
            else {
                delete m_pDateWindow;
                m_pDateWindow = nullptr;
            }
#endif
        }        
    }
    if (msg.eventType == kEventKillFocus) {
        Invalidate();
    }
    if ((msg.eventType == kEventMouseButtonDown) ||
        (msg.eventType == kEventMouseDoubleClick) ||
        (msg.eventType == kEventMouseRButtonDown)) {
        if (GetWindow() != nullptr) {
            GetWindow()->ReleaseCapture();
        }
#ifdef DUILIB_BUILD_FOR_WIN
        if (IsFocused() && (m_pDateWindow == nullptr)) {
            m_pDateWindow = new DateTimeWnd(this);
        }
        if (m_pDateWindow != nullptr) {
            if (m_pDateWindow->Init(this)) {
                m_pDateWindow->ShowWindow();
            }
            else {
                delete m_pDateWindow;
                m_pDateWindow = nullptr;
            }
        }
#endif
    }
    if (msg.eventType == kEventMouseMove) {
        return;
    }
    if (msg.eventType == kEventMouseButtonUp) {
        return;
    }
    if (msg.eventType == kEventContextMenu) {
        return;
    }
    if (msg.eventType == kEventMouseEnter) {
        return;
    }
    if (msg.eventType == kEventMouseLeave) {
        return;
    }
    __super::HandleEvent(msg);
}

void DateTime::OnInit()
{
    if (IsInited()) {
        return;
    }
    __super::OnInit();

    if (!IsValidDateTime()) {
        DString text = GetText();
        //将显示的文本内容，转换成日期时间格式
        if (!text.empty()) {
            SetDateTimeString(text);
        }
    }
}

}//namespace ui
