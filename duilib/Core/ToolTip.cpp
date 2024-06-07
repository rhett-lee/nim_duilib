#include "ToolTip.h"
#include "duilib/Core/WindowBase.h"
#include <CommCtrl.h>

namespace ui
{

//ToolTip最大长度
#define TOOLTIP_MAX_LEN 511

class ToolTip::ToolTipImpl
{
public:
    ToolTipImpl();
    ~ToolTipImpl();

public:
    /** 设置鼠标跟踪状态
    * @param [in] pParentWnd 父窗口
    * @param [in] bTracking 是否跟踪鼠标状态
    */
    void SetMouseTracking(WindowBase* pParentWnd, bool bTracking);

    /**@brief 显示ToolTip信息
    * @param [in] pParentWnd 父窗口
    * @param [in] rect Tooltip显示区域
    * @param [in] maxWidth Tooltip显示最大宽度
    * @param [in] trackPos 跟踪的位置
    * @param [in] text Tooltip显示内容
    */
    void ShowToolTip(WindowBase* pParentWnd,
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
    TOOLINFO m_ToolTip;

    //Tooltip窗口
    HWND m_hwndTooltip;

    //主窗口
    HWND m_hParentWnd;
};

ToolTip::ToolTipImpl::ToolTipImpl():
    m_bMouseTracking(false),
    m_hwndTooltip(nullptr),
    m_hParentWnd(nullptr)
{
    ::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
    m_ToolTip.cbSize = sizeof(TOOLINFO);
}

ToolTip::ToolTipImpl::~ToolTipImpl()
{
    if (m_hwndTooltip != nullptr) {
        ::DestroyWindow(m_hwndTooltip);
        m_hwndTooltip = nullptr;
    }
}

void ToolTip::ToolTipImpl::SetMouseTracking(WindowBase* pParentWnd, bool bTracking)
{
    ASSERT(pParentWnd != nullptr);
    if (pParentWnd == nullptr) {
        return;
    }
    if (bTracking && !m_bMouseTracking) {
        TRACKMOUSEEVENT tme = { 0 };
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.hwndTrack = pParentWnd->GetHWND();
        tme.dwHoverTime = (m_hwndTooltip == nullptr) ? 400UL : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
        _TrackMouseEvent(&tme);
    }
    m_hParentWnd = pParentWnd->GetHWND();
    m_bMouseTracking = bTracking;
}

void ToolTip::ToolTipImpl::ShowToolTip(WindowBase* pParentWnd,
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
    DString newText(text);
    if (newText.size() > TOOLTIP_MAX_LEN) {
        newText = newText.substr(0, TOOLTIP_MAX_LEN);
    }

    HWND hParentWnd = pParentWnd->GetHWND();
    if ((m_hwndTooltip != nullptr) && IsWindowVisible(m_hwndTooltip)) {
        TOOLINFO toolTip = { 0 };
        toolTip.cbSize = sizeof(TOOLINFO);
        toolTip.hwnd = hParentWnd;
        toolTip.uId = (UINT_PTR)hParentWnd;
        DString oldText;
        oldText.resize(TOOLTIP_MAX_LEN + 1);
        toolTip.lpszText = const_cast<LPTSTR>((LPCTSTR)oldText.c_str());
        ::SendMessage(m_hwndTooltip, TTM_GETTOOLINFO, 0, (LPARAM)&toolTip);
        oldText = DString(oldText.c_str());
        if (newText == oldText) {
            //文本内容均没有变化，不再设置
            return;
        }
    }

    //资源所在模块句柄
    HMODULE hModule = pParentWnd->GetResModuleHandle();

    ::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
    m_ToolTip.cbSize = sizeof(TOOLINFO);
    m_ToolTip.uFlags = TTF_IDISHWND;
    m_ToolTip.hwnd = hParentWnd;
    m_ToolTip.uId = (UINT_PTR)hParentWnd;
    m_ToolTip.hinst = hModule;
    m_ToolTip.lpszText = const_cast<LPTSTR>((LPCTSTR)newText.c_str());
    m_ToolTip.rect.left = rect.left;
    m_ToolTip.rect.top = rect.top;
    m_ToolTip.rect.right = rect.right;
    m_ToolTip.rect.bottom = rect.bottom;
    if (m_hwndTooltip == nullptr) {
        m_hwndTooltip = ::CreateWindowEx(0, TOOLTIPS_CLASS, NULL, 
                                         WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT,
                                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
                                         hParentWnd, NULL, hModule, NULL);
        ::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&m_ToolTip);
        ::SetWindowPos(m_hwndTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    if (!::IsWindowVisible(m_hwndTooltip)) {
        ::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, maxWidth);
        ::SendMessage(m_hwndTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&m_ToolTip);
        ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ToolTip);
    }
    ::SendMessage(m_hwndTooltip, TTM_TRACKPOSITION, 0, (LPARAM)(DWORD)MAKELONG(trackPos.x, trackPos.y));
    m_hParentWnd = hParentWnd;
}

void ToolTip::ToolTipImpl::HideToolTip()
{
    if (m_hwndTooltip != nullptr) {
        ::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ToolTip);
    }
}

void ToolTip::ToolTipImpl::ClearMouseTracking()
{
    if (m_bMouseTracking) {
        ::SendMessage(m_hParentWnd, WM_MOUSEMOVE, 0, (LPARAM)-1);
    }
    m_bMouseTracking = false;
}

ToolTip::ToolTip()
{
    m_impl = new ToolTipImpl;
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
