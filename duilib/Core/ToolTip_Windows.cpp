#include "ToolTip.h"
#include "duilib/Core/WindowBase.h"
#include "duilib/Utils/StringConvert.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include <CommCtrl.h>

namespace ui
{

//ToolTip最大长度
#define TOOLTIP_MAX_LEN 511

class ToolTip::TImpl
{
public:
    TImpl();
    ~TImpl();

public:
    /** 设置鼠标跟踪状态
    * @param [in] pParentWnd 父窗口
    * @param [in] bTracking 是否跟踪鼠标状态
    */
    void SetMouseTracking(const WindowBase* pParentWnd, bool bTracking);

    /**@brief 显示ToolTip信息
    * @param [in] pParentWnd 父窗口
    * @param [in] rect Tooltip显示区域
    * @param [in] maxWidth Tooltip显示最大宽度
    * @param [in] trackPos 跟踪的位置
    * @param [in] text Tooltip显示内容
    */
    void ShowToolTip(const WindowBase* pParentWnd,
                     const UiRect& rect, 
                     uint32_t maxWidth,
                     const UiPoint& trackPos,
                     const DString& text);

    /**@brief 隐藏ToolTip信息
    */
    void HideToolTip();

    /**@brief 清除鼠标跟踪状态
    */
    void ClearMouseTracking();

public:
    //鼠标跟踪状态
    bool m_bMouseTracking;

    //Tooltip信息
    TOOLINFOW m_ToolTip;

    //Tooltip窗口
    HWND m_hwndTooltip;

    //主窗口
    HWND m_hParentWnd;
};

ToolTip::TImpl::TImpl():
    m_bMouseTracking(false),
    m_hwndTooltip(nullptr),
    m_hParentWnd(nullptr)
{
    ::ZeroMemory(&m_ToolTip, sizeof(TOOLINFOW));
    m_ToolTip.cbSize = sizeof(TOOLINFOW);
}

ToolTip::TImpl::~TImpl()
{
    if (m_hwndTooltip != nullptr) {
        ::DestroyWindow(m_hwndTooltip);
        m_hwndTooltip = nullptr;
    }
}

void ToolTip::TImpl::SetMouseTracking(const WindowBase* pParentWnd, bool bTracking)
{
    ASSERT(pParentWnd != nullptr);
    if (pParentWnd == nullptr) {
        return;
    }
    if (bTracking && !m_bMouseTracking) {
        TRACKMOUSEEVENT tme = { 0 };
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.hwndTrack = pParentWnd->NativeWnd()->GetHWND();
        tme.dwHoverTime = (m_hwndTooltip == nullptr) ? 400UL : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
        _TrackMouseEvent(&tme);
    }
    m_hParentWnd = pParentWnd->NativeWnd()->GetHWND();
    m_bMouseTracking = bTracking;
}

void ToolTip::TImpl::ShowToolTip(const WindowBase* pParentWnd,
                                 const UiRect& rect,
                                 uint32_t maxWidth,
                                 const UiPoint& trackPos,
                                 const DString& text)
{
    ASSERT(pParentWnd != nullptr);
    if (pParentWnd == nullptr) {
        return;
    }
    if (text.empty()) {
        return;
    }
    //如果超过长度，则截断
    DStringW newText;
#ifdef DUILIB_UNICODE
    newText = text;    
#else
    //text是Utf8编码
    newText = StringConvert::UTF8ToWString(text);
#endif
    if (newText.size() > TOOLTIP_MAX_LEN) {
        newText.resize(TOOLTIP_MAX_LEN);
    }

    HWND hParentWnd = pParentWnd->NativeWnd()->GetHWND();
    if ((m_hwndTooltip != nullptr) && IsWindowVisible(m_hwndTooltip)) {
        TOOLINFOW toolTip = { 0 };
        toolTip.cbSize = sizeof(TOOLINFOW);
        toolTip.hwnd = hParentWnd;
        toolTip.uId = (UINT_PTR)hParentWnd;
        DStringW oldText;
        oldText.resize(TOOLTIP_MAX_LEN + 1);
        toolTip.lpszText = const_cast<LPWSTR>((LPCWSTR)oldText.c_str());
        ::SendMessage(m_hwndTooltip, TTM_GETTOOLINFOW, 0, (LPARAM)&toolTip);
        oldText = DStringW(oldText.c_str());
        if (newText == oldText) {
            //文本内容均没有变化，不再设置
            return;
        }
    }

    //资源所在模块句柄
    HMODULE hModule = pParentWnd->NativeWnd()->GetResModuleHandle();

    ::ZeroMemory(&m_ToolTip, sizeof(TOOLINFOW));
    m_ToolTip.cbSize = sizeof(TOOLINFOW);
    m_ToolTip.uFlags = TTF_IDISHWND;
    m_ToolTip.hwnd = hParentWnd;
    m_ToolTip.uId = (UINT_PTR)hParentWnd;
    m_ToolTip.hinst = hModule;
    m_ToolTip.lpszText = const_cast<LPWSTR>((LPCWSTR)newText.c_str());
    m_ToolTip.rect.left = rect.left;
    m_ToolTip.rect.top = rect.top;
    m_ToolTip.rect.right = rect.right;
    m_ToolTip.rect.bottom = rect.bottom;
    if (m_hwndTooltip == nullptr) {
        m_hwndTooltip = ::CreateWindowExW(0, TOOLTIPS_CLASSW, nullptr,
                                          WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT,
                                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                          hParentWnd, nullptr, hModule, nullptr);
        ::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&m_ToolTip);
        ::SetWindowPos(m_hwndTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    if (!::IsWindowVisible(m_hwndTooltip)) {
        ::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, maxWidth);
        ::SendMessage(m_hwndTooltip, TTM_SETTOOLINFOW, 0, (LPARAM)&m_ToolTip);
        ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ToolTip);
    }
    ::SendMessage(m_hwndTooltip, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD)MAKELONG(trackPos.x, trackPos.y));
    m_hParentWnd = hParentWnd;
}

void ToolTip::TImpl::HideToolTip()
{
    if (m_hwndTooltip != nullptr) {
        ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ToolTip);
    }
}

void ToolTip::TImpl::ClearMouseTracking()
{
    if (m_bMouseTracking) {
        ::SendMessage(m_hParentWnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
    }
    m_bMouseTracking = false;
}

ToolTip::ToolTip()
{
    m_impl = new TImpl;
}

ToolTip::~ToolTip()
{
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

void ToolTip::SetMouseTracking(WindowBase* pParentWnd, bool bTracking)
{
    m_impl->SetMouseTracking(pParentWnd, bTracking);
}

void ToolTip::ShowToolTip(WindowBase* pParentWnd,
                          const UiRect& rect,
                          uint32_t maxWidth,
                          const UiPoint& trackPos,
                          const DString& text)
{
    m_impl->ShowToolTip(pParentWnd, rect, maxWidth, trackPos, text);
}

void ToolTip::HideToolTip()
{
    m_impl->HideToolTip();
}

void ToolTip::ClearMouseTracking()
{
    m_impl->ClearMouseTracking();
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
