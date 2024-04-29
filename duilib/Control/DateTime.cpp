#include "DateTime.h"
#include <windowsx.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <olectl.h>
#include <CommCtrl.h>

namespace ui
{

class DateTimeWnd : public Window
{
public:
    DateTimeWnd();
    virtual ~DateTimeWnd();

    bool Init(DateTime* pOwner);
    HFONT CreateHFont() const;
    UiRect CalPos();

    //更新窗口的位置
    void UpdateWndPos();

    virtual std::wstring GetWindowClassName() const override;
    virtual std::wstring GetSuperClassName() const override;
    void OnFinalMessage(HWND hWnd);

    virtual LRESULT OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

    std::tm SystemTimeToStdTime(const SYSTEMTIME& sysTime) const;
    SYSTEMTIME StdTimeToSystemTime(const std::tm& tmTime) const;

protected:
    DateTime* m_pOwner;
    bool m_bInit;
    bool m_bDropOpen;
    SYSTEMTIME m_oldSysTime;
    HFONT m_hFont;
};

DateTimeWnd::DateTimeWnd() : 
    m_pOwner(nullptr), 
    m_bInit(false), 
    m_bDropOpen(false),
    m_oldSysTime({0,}),
    m_hFont(nullptr)
{
}

DateTimeWnd::~DateTimeWnd()
{
    if (m_hFont != nullptr) {
        ::DeleteObject(m_hFont);
        m_hFont = nullptr;
    }
}

std::tm DateTimeWnd::SystemTimeToStdTime(const SYSTEMTIME& sysTime) const
{
    std::tm tmTime = { 0, };
    tmTime.tm_sec = sysTime.wSecond;
    tmTime.tm_min = sysTime.wMinute;
    tmTime.tm_hour = sysTime.wHour;
    tmTime.tm_mday = sysTime.wDay;
    tmTime.tm_mon = sysTime.wMonth - 1;
    tmTime.tm_year = sysTime.wYear - 1900;
    tmTime.tm_wday = sysTime.wDayOfWeek;
    return tmTime;
}


SYSTEMTIME DateTimeWnd::StdTimeToSystemTime(const std::tm& tmTime) const
{
    SYSTEMTIME st = { (WORD)(tmTime.tm_year + 1900),
                      (WORD)(tmTime.tm_mon + 1),
                      (WORD)tmTime.tm_wday,
                      (WORD)tmTime.tm_mday,
                      (WORD)tmTime.tm_hour,
                      (WORD)tmTime.tm_min,
                      (WORD)tmTime.tm_sec,
                      0 };
    return st;
}

bool DateTimeWnd::Init(DateTime* pOwner)
{
    m_pOwner = pOwner;
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return false;
    }
    DateTime::EditFormat editFormat = m_pOwner->GetEditFormat();
    if (GetHWND() == nullptr) {
        UiRect rcPos = CalPos();
        if (rcPos.IsEmpty()) {
            return false;
        }
        UINT uStyle = WS_POPUP;
        if (editFormat != DateTime::EditFormat::kDateCalendar) {
            //在右侧增加Spin按钮
            uStyle |= DTS_UPDOWN;
        }

        UiPoint pt1 = { rcPos.left, rcPos.top };
        UiPoint pt2 = { rcPos.right, rcPos.bottom };
        pOwner->GetWindow()->ClientToScreen(pt1);
        pOwner->GetWindow()->ClientToScreen(pt2);
        CreateWnd(m_pOwner->GetWindow()->GetHWND(), L"", uStyle, 0, {pt1.x, pt1.y, pt2.x, pt2.y});
        ASSERT(GetHWND() != nullptr);

        HFONT hFont = CreateHFont();
        if (hFont != nullptr) {
            if (m_hFont != nullptr) {
                ::DeleteObject(m_hFont);
                m_hFont = nullptr;
            }
            m_hFont = hFont;
        }
        SetWindowFont(GetHWND(), m_hFont, TRUE);
    }

    if (m_pOwner->IsValidDateTime()) {
        m_oldSysTime = StdTimeToSystemTime(m_pOwner->GetDateTime());
    }
    else {
        ::GetLocalTime(&m_oldSysTime);
    }

    ::SendMessage(GetHWND(), DTM_SETSYSTEMTIME, 0, (LPARAM)&m_oldSysTime);    
    std::wstring sEditFormat;
    switch (editFormat) {
    case DateTime::EditFormat::kDateCalendar:
    case DateTime::EditFormat::kDateUpDown:
        sEditFormat = L"yyy-MM-dd";
        break;
    case DateTime::EditFormat::kDateTimeUpDown:
        sEditFormat = L"yyy-MM-dd HH:mm:ss";
        break;
    case DateTime::EditFormat::kDateMinuteUpDown:
        sEditFormat = L"yyy-MM-dd HH:mm";
        break;
    case DateTime::EditFormat::kTimeUpDown:
        sEditFormat = L"HH:mm:ss";
        break;
    case DateTime::EditFormat::kMinuteUpDown:
        sEditFormat = L"HH:mm";
        break;
    default:
        sEditFormat = L"yyy-MM-dd";
        break;
    }
    ::SendMessage(GetHWND(), DTM_SETFORMAT, 0, (LPARAM)sEditFormat.c_str());
    ::ShowWindow(GetHWND(), SW_SHOW);
    ::SetFocus(GetHWND());

    HWND hWndParent = GetHWND();
    while (::GetParent(hWndParent) != NULL) {
        hWndParent = ::GetParent(hWndParent);
    }        
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
    m_bInit = true;
    return true;
}

void DateTimeWnd::UpdateWndPos()
{
    if (m_pOwner == nullptr) {
        return;
    }
    UiRect rcPos = CalPos();
    if (rcPos.IsEmpty()) {
        return;
    }
    UiPoint pt1 = { rcPos.left, rcPos.top };
    UiPoint pt2 = { rcPos.right, rcPos.bottom };
    m_pOwner->GetWindow()->ClientToScreen(pt1);
    m_pOwner->GetWindow()->ClientToScreen(pt2);
    UiRect rc;
    rc.left = pt1.x;
    rc.top = pt1.y;
    rc.right = pt2.x;
    rc.bottom = pt2.y;
    SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

HFONT DateTimeWnd::CreateHFont() const
{
    //优先获取默认字体
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(L"");
    if (pFont == nullptr) {
        pFont = GlobalManager::Instance().Font().GetIFont(L"system_12");
    }
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return nullptr;
    }
    LOGFONT lf = { 0 };
    ::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
    wcscpy_s(lf.lfFaceName, pFont->FontName());
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfHeight = -pFont->FontSize();
    if (pFont->IsUnderline()) {
        lf.lfUnderline = TRUE;
    }
    if (pFont->IsStrikeOut()) {
        lf.lfStrikeOut = TRUE;
    }
    if (pFont->IsItalic()) {
        lf.lfItalic = TRUE;
    }
    if (pFont->IsBold()) {
        lf.lfWeight = FW_BOLD;
    }
    HFONT hFont = ::CreateFontIndirect(&lf);
    ASSERT(hFont != nullptr);
    return hFont;
}

UiRect DateTimeWnd::CalPos()
{
    UiRect rcPos = m_pOwner->GetPos();
    UiPoint scrollPos = m_pOwner->GetScrollOffsetInScrollBox();
    rcPos.Offset(-scrollPos.x, -scrollPos.y);
    Control* pParent = m_pOwner->GetParent();
    UiRect rcParent;
    while (pParent != nullptr) {
        if (!pParent->IsVisible()) {
            rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
            break;
        }
        rcParent = pParent->GetPos();
        scrollPos = pParent->GetScrollOffsetInScrollBox();
        rcParent.Offset(-scrollPos.x, -scrollPos.y);
        if (!UiRect::Intersect(rcPos, rcPos, rcParent)) {
            rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
            break;
        }
        pParent = pParent->GetParent();
    }
    return rcPos;
}

std::wstring DateTimeWnd::GetWindowClassName() const
{
    return L"DateTimeWnd";
}

std::wstring DateTimeWnd::GetSuperClassName() const
{
    return DATETIMEPICK_CLASS;
}

void DateTimeWnd::OnFinalMessage(HWND /*hWnd*/)
{
    m_pOwner->m_pDateWindow = nullptr;
    delete this;
}

LRESULT DateTimeWnd::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lRes = 0;
    bHandled = false;
    if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        m_pOwner->SetDateTime(SystemTimeToStdTime(m_oldSysTime));
        PostMsg(WM_CLOSE);
        bHandled = true;
    }
    else if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        SYSTEMTIME systime = { 0, };
        ::SendMessage(GetHWND(), DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
        m_pOwner->SetDateTime(SystemTimeToStdTime(systime));
        PostMsg(WM_CLOSE);
        bHandled = true;
    }
    else if (uMsg == OCM_NOTIFY)
    {
        NMHDR* pHeader = (NMHDR*)lParam;
        if (pHeader != nullptr && pHeader->hwndFrom == GetHWND()) {
            if (pHeader->code == DTN_DATETIMECHANGE) {
                SYSTEMTIME systime = {0,};
                ::SendMessage(GetHWND(), DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
                m_pOwner->SetDateTime(SystemTimeToStdTime(systime));
            }
            else if (pHeader->code == DTN_DROPDOWN) {
                m_bDropOpen = true;
            }
            else if (pHeader->code == DTN_CLOSEUP) {
                SYSTEMTIME systime = { 0, };
                ::SendMessage(GetHWND(), DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
                m_pOwner->SetDateTime(SystemTimeToStdTime(systime));
                PostMsg(WM_CLOSE);
                m_bDropOpen = false;
            }
        }
    }
    else if (uMsg == WM_KILLFOCUS)
    {
        if (!m_bDropOpen) {
            PostMsg(WM_CLOSE);
        }
    }
    if (!bHandled) {
        bHandled = true;
        lRes = this->CallDefaultWindowProc(uMsg, wParam, lParam);
    }
    return lRes;
}

//////////////////////////////////////////////////////////////////////////

DateTime::DateTime():
    m_dateTime({0,}),
    m_pDateWindow(nullptr),
    m_editFormat(EditFormat::kDateCalendar),
    m_dateSeparator(L'-')
{
    //设置默认属性
    SetAttribute(L"border_size", L"1");
    SetAttribute(L"border_color", L"gray");
    SetAttribute(L"text_align", L"vcenter");
    SetAttribute(L"text_padding", L"2,0,0,0");
}

DateTime::~DateTime()
{
}

std::wstring DateTime::GetType() const { return DUI_CTR_DATETIME; }

void DateTime::SetAttribute(const std::wstring& strName, const std::wstring& strValue)
{
    if (strName == L"format") {
        SetStringFormat(strValue);
    }
    else if (strName == L"edit_format") {
        if (strValue == L"date_calendar") {
            SetEditFormat(EditFormat::kDateCalendar);
        }
        else if (strValue == L"date_up_down") {
            SetEditFormat(EditFormat::kDateUpDown);
        }
        else if (strValue == L"date_time_up_down") {
            SetEditFormat(EditFormat::kDateTimeUpDown);
        }
        else if (strValue == L"date_minute_up_down") {
            SetEditFormat(EditFormat::kDateMinuteUpDown);
        }
        else if (strValue == L"time_up_down") {
            SetEditFormat(EditFormat::kTimeUpDown);
        }
        else if (strValue == L"minute_up_down") {
            SetEditFormat(EditFormat::kMinuteUpDown);
        }
        else {
            ASSERT(FALSE);
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

std::wstring DateTime::GetDateTimeString() const
{
    std::wstring dateTime;
    if (IsValidDateTime()) {
        std::tm tmSystemDate = m_dateTime;
        std::wstringstream ss;
        ss << std::put_time(&tmSystemDate, GetStringFormat().c_str());
        dateTime = ss.str();
    }
    return dateTime;
}

bool DateTime::SetDateTimeString(const std::wstring& dateTime)
{
    bool bRet = false;
    std::wstring sFormat = GetStringFormat();
    ASSERT(!sFormat.empty());
    std::tm t = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    std::wistringstream ss(dateTime);
    ss >> std::get_time(&t, sFormat.c_str());
    if (ss.fail()) {
        //失败后，智能识别年月日的分隔符
        if (dateTime.find(L'-') != std::wstring::npos) {
            StringHelper::ReplaceAll(L"/", L"-", sFormat);
            std::wistringstream ss2(dateTime);
            ss2 >> std::get_time(&t, sFormat.c_str());
            if (!ss2.fail()) {
                m_dateTime = t;
                bRet = true;
                m_dateSeparator = L'-';
            }
        }
        else if (dateTime.find(L'/') != std::wstring::npos) {
            StringHelper::ReplaceAll(L"-", L"/", sFormat);
            std::wistringstream ss2(dateTime);
            ss2 >> std::get_time(&t, sFormat.c_str());
            if (!ss2.fail()) {
                m_dateTime = t;
                bRet = true;
                m_dateSeparator = L'/';
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

void DateTime::SetStringFormat(const std::wstring& sFormat)
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

std::wstring DateTime::GetStringFormat() const
{
    std::wstring sFormat = m_sFormat.c_str();
    if (sFormat.empty()) {
        EditFormat editFormat = GetEditFormat();
        switch (editFormat) {
        case EditFormat::kDateCalendar:
        case EditFormat::kDateUpDown:
            sFormat = L"%Y-%m-%d";
            break;
        case EditFormat::kDateTimeUpDown:
            sFormat = L"%Y-%m-%d %H:%M:%S";
            break;
        case EditFormat::kDateMinuteUpDown:
            sFormat = L"%Y-%m-%d %H:%M";
            break;
        case EditFormat::kTimeUpDown:
            sFormat = L"%H:%M:%S";
            break;
        case EditFormat::kMinuteUpDown:
            sFormat = L"%H:%M";
            break;
        default:
            sFormat = L"%Y-%m-%d";
            break;
        }
        if (m_dateSeparator != L'-') {
            std::wstring separator;
            separator = m_dateSeparator;
            StringHelper::ReplaceAll(L"-", separator, sFormat);
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
        std::wstring oldFormat = GetStringFormat();
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
            pParent->SendEvent(msg);
        }
        else {
            __super::HandleEvent(msg);
        }
        return;
    }
    if ((msg.Type == kEventSetCursor)) {
        SetCursor(kCursorHandIbeam);
        return;
    }
    if (msg.Type == kEventWindowSize) {
        if (m_pDateWindow != nullptr) {
            return;
        }
    }
    if (msg.Type == kEventScrollChange) {
        if (m_pDateWindow != nullptr) {
            return;
        }
    }
    if (msg.Type == kEventSetFocus) {
        if (m_pDateWindow != nullptr) {
            return;
        }
        if (GetRect().IsZero() && (GetWindow() != nullptr)) {
            //尚未显示，刷新一次窗口，确保控件先确定位置，然后再显示编辑窗口
            GetWindow()->UpdateWindow();
        }
        if (IsFocused()) {
            m_pDateWindow = new DateTimeWnd();
            if (m_pDateWindow->Init(this)) {
                m_pDateWindow->ShowWindow();
            }
            else {
                delete m_pDateWindow;
                m_pDateWindow = nullptr;
            }
        }        
    }
    if (msg.Type == kEventKillFocus) {
        Invalidate();
    }
    if ((msg.Type == kEventMouseButtonDown) ||
        (msg.Type == kEventMouseDoubleClick) ||
        (msg.Type == kEventMouseRButtonDown)) {
        if (GetWindow() != nullptr) {
            GetWindow()->ReleaseCapture();
        }            
        if (IsFocused() && (m_pDateWindow == nullptr)) {
            m_pDateWindow = new DateTimeWnd();
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
    }
    if (msg.Type == kEventMouseMove) {
        return;
    }
    if (msg.Type == kEventMouseButtonUp) {
        return;
    }
    if (msg.Type == kEventMouseMenu) {
        return;
    }
    if (msg.Type == kEventMouseEnter) {
        return;
    }
    if (msg.Type == kEventMouseLeave) {
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
        std::wstring text = GetText();
        //将显示的文本内容，转换成日期时间格式
        if (!text.empty()) {
            SetDateTimeString(text);
        }
    }
}

}//namespace ui
