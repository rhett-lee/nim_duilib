#include "DateTimeWnd_Windows.h"
#include "DateTime.h"
#include "duilib/Core/GlobalManager.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include <windowsx.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <olectl.h>
#include <CommCtrl.h>

namespace ui
{
DateTimeWnd::DateTimeWnd(DateTime* pOwner):
    m_hDateTimeWnd(nullptr),
    m_OldWndProc(::DefWindowProc),
    m_pOwner(pOwner),
    m_bInit(false),
    m_bDropOpen(false),
    m_oldSysTime({ 0, }),
    m_hFont(nullptr)
{
    ASSERT(m_pOwner != nullptr);
}

DateTimeWnd::~DateTimeWnd()
{
    if (m_hFont != nullptr) {
        ::DeleteObject(m_hFont);
        m_hFont = nullptr;
    }
}

bool DateTimeWnd::Init(DateTime* pOwner)
{
    m_pOwner = pOwner;
    ASSERT(pOwner != nullptr);
    if (pOwner == nullptr) {
        return false;
    }
    DateTime::EditFormat editFormat = m_pOwner->GetEditFormat();
    if (m_hDateTimeWnd == nullptr) {
        UiRect rcPos = CalPos();
        if (rcPos.IsEmpty()) {
            return false;
        }
        UINT dwStyle = WS_POPUP;
        if (editFormat != DateTime::EditFormat::kDateCalendar) {
            //在右侧增加Spin按钮
            dwStyle |= DTS_UPDOWN;
        }

        UiPoint pt1 = { rcPos.left, rcPos.top };
        UiPoint pt2 = { rcPos.right, rcPos.bottom };
        pOwner->GetWindow()->ClientToScreen(pt1);
        pOwner->GetWindow()->ClientToScreen(pt2);

        if (!RegisterSuperClass()) {
            return false;
        }
        HWND hParentWnd = m_pOwner->GetWindow()->NativeWnd()->GetHWND();
        HMODULE hModule = m_pOwner->GetWindow()->NativeWnd()->GetResModuleHandle();
        DString className = GetWindowClassName();
        UiRect rc = { pt1.x, pt1.y, pt2.x, pt2.y };
        m_hDateTimeWnd = ::CreateWindowExW(0,
                                         StringConvert::TToWString(className).c_str(),
                                         L"",
                                         dwStyle,
                                         rc.left, rc.top, rc.Width(), rc.Height(),
                                         hParentWnd, nullptr, hModule, this);

        ASSERT(m_hDateTimeWnd != nullptr);

        HFONT hFont = CreateHFont();
        if (hFont != nullptr) {
            if (m_hFont != nullptr) {
                ::DeleteObject(m_hFont);
                m_hFont = nullptr;
            }
            m_hFont = hFont;
        }
        SetWindowFont(m_hDateTimeWnd, m_hFont, TRUE);
    }

    if (m_pOwner->IsValidDateTime()) {
        m_oldSysTime = StdTimeToSystemTime(m_pOwner->GetDateTime());
    }
    else {
        ::GetLocalTime(&m_oldSysTime);
    }

    ::SendMessage(m_hDateTimeWnd, DTM_SETSYSTEMTIME, 0, (LPARAM)&m_oldSysTime);
    DString sEditFormat;
    switch (editFormat) {
    case DateTime::EditFormat::kDateCalendar:
    case DateTime::EditFormat::kDateUpDown:
        sEditFormat = _T("yyy-MM-dd");
        break;
    case DateTime::EditFormat::kDateTimeUpDown:
        sEditFormat = _T("yyy-MM-dd HH:mm:ss");
        break;
    case DateTime::EditFormat::kDateMinuteUpDown:
        sEditFormat = _T("yyy-MM-dd HH:mm");
        break;
    case DateTime::EditFormat::kTimeUpDown:
        sEditFormat = _T("HH:mm:ss");
        break;
    case DateTime::EditFormat::kMinuteUpDown:
        sEditFormat = _T("HH:mm");
        break;
    default:
        sEditFormat = _T("yyy-MM-dd");
        break;
    }
    ::SendMessage(m_hDateTimeWnd, DTM_SETFORMAT, 0, (LPARAM)sEditFormat.c_str());
    ::ShowWindow(m_hDateTimeWnd, SW_SHOW);
    ::SetFocus(m_hDateTimeWnd);

    HWND hWndParent = m_hDateTimeWnd;
    while (::GetParent(hWndParent) != nullptr) {
        hWndParent = ::GetParent(hWndParent);
    }
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
    m_bInit = true;
    return true;
}

bool DateTimeWnd::RegisterSuperClass()
{
    if (m_pOwner == nullptr) {
        return false;
    }
    HMODULE hModule = m_pOwner->GetWindow()->NativeWnd()->GetResModuleHandle();
    // Get the class information from an existing
    // window so we can subclass it later on...
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    DStringW superClassName = DATETIMEPICK_CLASSW;
    if (!::GetClassInfoExW(nullptr, superClassName.c_str(), &wc)) {
        if (!::GetClassInfoExW(hModule, superClassName.c_str(), &wc)) {
            ASSERT(!"Unable to locate window class");
            return false;
        }
    }
    m_OldWndProc = wc.lpfnWndProc;
    wc.lpfnWndProc = DateTimeWnd::__ControlProc;
    wc.hInstance = hModule;
    DStringW className = StringConvert::TToWString(GetWindowClassName());
    wc.lpszClassName = className.c_str();
    ATOM ret = ::RegisterClassExW(&wc);
    ASSERT(ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

DString DateTimeWnd::GetWindowClassName() const
{
    return _T("DateTimeWnd");
}

static const DStringW::value_type* sPropName = L"DuiLibDateTimeWndX"; // 属性名称

LRESULT CALLBACK DateTimeWnd::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DateTimeWnd* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<DateTimeWnd*>(lpcs->lpCreateParams);
        if (pThis != nullptr) {
            ::SetPropW(hWnd, sPropName, (HANDLE)pThis);
            pThis->m_hDateTimeWnd = hWnd;
        }
    }
    else {
        pThis = static_cast<DateTimeWnd*>(::GetPropW(hWnd, sPropName));
        if (uMsg == WM_NCDESTROY && pThis != nullptr) {
            LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
            ::SetPropW(hWnd, sPropName, nullptr);
            ASSERT(hWnd == pThis->m_hDateTimeWnd);
            pThis->OnFinalMessage();
            return lRes;
        }
    }
    if (pThis != nullptr) {
        ASSERT(hWnd == pThis->m_hDateTimeWnd);
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    }
    else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

void DateTimeWnd::OnFinalMessage()
{
    if (m_pOwner != nullptr) {
        m_pOwner->m_pDateWindow = nullptr;
    }
    //删除自身
    delete this;
}

LRESULT DateTimeWnd::WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
    bool bHandled = false;
    if (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE) {
        m_pOwner->SetDateTime(SystemTimeToStdTime(m_oldSysTime));
        ::PostMessage(m_hDateTimeWnd, WM_CLOSE, 0, 0);
        bHandled = true;
    }
    else if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        SYSTEMTIME systime = { 0, };
        ::SendMessage(m_hDateTimeWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
        m_pOwner->SetDateTime(SystemTimeToStdTime(systime));
        ::PostMessage(m_hDateTimeWnd, WM_CLOSE, 0, 0);
        bHandled = true;
    }
    else if (uMsg == OCM_NOTIFY)
    {
        NMHDR* pHeader = (NMHDR*)lParam;
        if (pHeader != nullptr && pHeader->hwndFrom == m_hDateTimeWnd) {
            if (pHeader->code == DTN_DATETIMECHANGE) {
                SYSTEMTIME systime = { 0, };
                ::SendMessage(m_hDateTimeWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
                m_pOwner->SetDateTime(SystemTimeToStdTime(systime));
            }
            else if (pHeader->code == DTN_DROPDOWN) {
                m_bDropOpen = true;
            }
            else if (pHeader->code == DTN_CLOSEUP) {
                SYSTEMTIME systime = { 0, };
                ::SendMessage(m_hDateTimeWnd, DTM_GETSYSTEMTIME, 0, (LPARAM)&systime);
                m_pOwner->SetDateTime(SystemTimeToStdTime(systime));
                ::PostMessage(m_hDateTimeWnd, WM_CLOSE, 0, 0);
                m_bDropOpen = false;
            }
        }
    }
    else if (uMsg == WM_KILLFOCUS)
    {
        if (!m_bDropOpen) {
            ::PostMessage(m_hDateTimeWnd, WM_CLOSE, 0, 0);
        }
    }
    if (!bHandled) {
        bHandled = true;
        ASSERT(m_OldWndProc != nullptr);
        lRes = ::CallWindowProc(m_OldWndProc, m_hDateTimeWnd, uMsg, wParam, lParam);
    }
    return lRes;
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
    ::SetWindowPos(m_hDateTimeWnd, nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

void DateTimeWnd::ShowWindow()
{
    ASSERT(::IsWindow(m_hDateTimeWnd));
    ::ShowWindow(m_hDateTimeWnd, SW_SHOWNORMAL);
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

HFONT DateTimeWnd::CreateHFont() const
{
    //优先获取默认字体
    IFont* pFont = GlobalManager::Instance().Font().GetIFont(_T(""), m_pOwner->Dpi());
    if (pFont == nullptr) {
        pFont = GlobalManager::Instance().Font().GetIFont(_T("system_12"), m_pOwner->Dpi());
    }
    ASSERT(pFont != nullptr);
    if (pFont == nullptr) {
        return nullptr;
    }
    LOGFONTW lf = { 0 };
    ::GetObjectW(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONTW), &lf);
    StringUtil::StringCopy(lf.lfFaceName, StringConvert::TToWString(pFont->FontName()).c_str());
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
    HFONT hFont = ::CreateFontIndirectW(&lf);
    ASSERT(hFont != nullptr);
    return hFont;
}

struct tm DateTimeWnd::SystemTimeToStdTime(const SYSTEMTIME& sysTime) const
{
    struct tm tmTime = { 0, };
    tmTime.tm_sec = sysTime.wSecond;
    tmTime.tm_min = sysTime.wMinute;
    tmTime.tm_hour = sysTime.wHour;
    tmTime.tm_mday = sysTime.wDay;
    tmTime.tm_mon = sysTime.wMonth - 1;
    tmTime.tm_year = sysTime.wYear - 1900;
    tmTime.tm_wday = sysTime.wDayOfWeek;
    return tmTime;
}


SYSTEMTIME DateTimeWnd::StdTimeToSystemTime(const struct tm& tmTime) const
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

}//namespace ui

#endif // (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
