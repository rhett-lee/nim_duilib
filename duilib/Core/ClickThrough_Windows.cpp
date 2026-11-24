#include "ClickThrough.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/Core/Window.h"
#include "duilib/Core/Control.h"
#include "duilib/Utils/LogUtil.h"

#include <dwmapi.h>
#include <algorithm>
#include <vector>

namespace ui
{
ClickThrough::ClickThrough()
{
}

ClickThrough::~ClickThrough()
{
}

/** 判断窗口是否被DWM隐藏(例如位于其它虚拟桌面，或者UWP应用被挂起)
*/
static bool IsWindowCloaked(HWND hWnd)
{
    typedef HRESULT (WINAPI *DwmGetWindowAttributeFunc)(HWND, DWORD, PVOID, DWORD);
    static DwmGetWindowAttributeFunc pDwmGetWindowAttribute = nullptr;
    static bool bInited = false;
    if (!bInited) {
        bInited = true;
        HMODULE hDwm = ::GetModuleHandleW(L"dwmapi.dll");
        if (hDwm == nullptr) {
            hDwm = ::LoadLibraryW(L"dwmapi.dll");
        }
        if (hDwm != nullptr) {
            pDwmGetWindowAttribute = (DwmGetWindowAttributeFunc)::GetProcAddress(hDwm, "DwmGetWindowAttribute");
        }
    }
    if (pDwmGetWindowAttribute == nullptr) {
        return false;
    }
    BOOL bCloaked = FALSE;
    if (FAILED(pDwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &bCloaked, sizeof(bCloaked)))) {
        return false;
    }
    return bCloaked != FALSE;
}

/** 命中测试结果 */
enum class HitTestResult
{
    kNotHit,        //未命中该窗口，继续向下查找
    kHit,           //命中该窗口，查找结束
    kOwnerWindow    //命中当前窗口的父窗口/属主窗口，查找结束，且不需要切换前台窗口
};

/** 获取指定窗口的父窗口/属主窗口链(按由近及远排列)
* @param [in] hWnd 起始窗口
* @param [out] ownerChain 父窗口/属主窗口链，不包含起始窗口本身
*/
static void GetOwnerWindowChain(HWND hWnd, std::vector<HWND>& ownerChain)
{
    ownerChain.clear();
    if ((hWnd == nullptr) || !::IsWindow(hWnd)) {
        return;
    }
    //窗口的父窗口/属主窗口：子窗口返回父窗口，顶层窗口返回属主窗口
    HWND hWndOwner = ::GetParent(hWnd);
    //限制最大层数并检测重复，避免异常情况下出现死循环
    const int32_t kMaxOwnerCount = 32;
    while ((hWndOwner != nullptr) && ::IsWindow(hWndOwner) &&
           (ownerChain.size() < (size_t)kMaxOwnerCount)) {
        if ((hWndOwner == hWnd) ||
            (std::find(ownerChain.begin(), ownerChain.end(), hWndOwner) != ownerChain.end())) {
            break;
        }
        ownerChain.push_back(hWndOwner);
        hWndOwner = ::GetParent(hWndOwner);
    }
}

/** 把duilib记录的父窗口链补充到父窗口/属主窗口链中
* 说明：SDL后端的弹出窗口(带SDL_WINDOW_UTILITY属性时)在系统层面使用的是SDL内部的辅助窗口作为属主，
*      并未与SDL的父窗口建立系统级的父窗口/属主关系，因此需要按duilib记录的父窗口关系补充
* @param [in] pWindow 当前窗口
* @param [in,out] ownerChain 父窗口/属主窗口链
*/
static void AddParentWindowChain(const Window* pWindow, std::vector<HWND>& ownerChain)
{
    if (pWindow == nullptr) {
        return;
    }
    HWND hWndParent = nullptr;
    const Window* pParentWindow = pWindow->GetParentWindow();
    //限制最大层数并检测重复，避免异常情况下出现死循环
    const int32_t kMaxParentCount = 32;
    while ((pParentWindow != nullptr) && (ownerChain.size() < (size_t)kMaxParentCount)) {
        hWndParent = (pParentWindow->NativeWnd() != nullptr) ? pParentWindow->NativeWnd()->GetHWND() : nullptr;
        if ((hWndParent != nullptr) && ::IsWindow(hWndParent) &&
            (std::find(ownerChain.begin(), ownerChain.end(), hWndParent) == ownerChain.end())) {
            ownerChain.push_back(hWndParent);
        }
        pParentWindow = pParentWindow->GetParentWindow();
    }
}

/** 判断窗口是否属于当前窗口的父窗口/属主窗口链
*/
static bool IsOwnerWindow(const std::vector<HWND>& ownerChain, HWND hWnd)
{
    return !ownerChain.empty() &&
           (std::find(ownerChain.begin(), ownerChain.end(), hWnd) != ownerChain.end());
}

/** 判断指定顶层窗口在该屏幕坐标点上的命中测试结果
* @param [in] hWnd 被检测的顶层窗口
* @param [in] ownerChain 当前窗口的父窗口/属主窗口链
* @param [in] ptScreen 屏幕坐标
*/
static HitTestResult HitTestWindowAtPoint(HWND hWnd, const std::vector<HWND>& ownerChain, const POINT& ptScreen)
{
    if ((hWnd == nullptr) || !::IsWindow(hWnd)) {
        return HitTestResult::kNotHit;
    }
    if (!::IsWindowVisible(hWnd) || ::IsIconic(hWnd)) {
        return HitTestResult::kNotHit;
    }
    if (IsWindowCloaked(hWnd)) {
        //被DWM隐藏的窗口(不在当前桌面)，不参与命中测试
        return HitTestResult::kNotHit;
    }

    RECT rcWindow;
    if (!::GetWindowRect(hWnd, &rcWindow)) {
        return HitTestResult::kNotHit;
    }
    if (!::PtInRect(&rcWindow, ptScreen)) {
        return HitTestResult::kNotHit;
    }

    //窗口设置了形状区域时，需要按区域判断是否命中
    HRGN hRgn = ::CreateRectRgn(0, 0, 0, 0);
    if (hRgn != nullptr) {
        bool bInWindow = true;
        if (::GetWindowRgn(hWnd, hRgn) != ERROR) {
            //窗口区域的坐标以窗口左上角为原点
            POINT ptWindow;
            ptWindow.x = ptScreen.x - rcWindow.left;
            ptWindow.y = ptScreen.y - rcWindow.top;
            bInWindow = ::PtInRegion(hRgn, ptWindow.x, ptWindow.y) != FALSE;
        }
        ::DeleteObject(hRgn);
        if (!bInWindow) {
            return HitTestResult::kNotHit;
        }
    }

    //当前窗口的父窗口/属主窗口：典型情况是子界面后方的主界面，
    //此时应保持现状，不切换前台窗口；也不再继续向下查找，避免激活后方其它进程的窗口。
    //注意：不能按进程判断，因为同一进程可能有多个独立的顶层窗口(例如多标签浏览器)
    if (IsOwnerWindow(ownerChain, hWnd)) {
        //模态窗口会禁用其父窗口，因此该判断需要放在IsWindowEnabled之前
        return HitTestResult::kOwnerWindow;
    }

    if (!::IsWindowEnabled(hWnd)) {
        //WindowFromPoint不会命中被禁用的窗口
        return HitTestResult::kNotHit;
    }
    LONG_PTR dwExStyle = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if ((dwExStyle & WS_EX_TRANSPARENT) != 0) {
        //设置了鼠标穿透的窗口，不参与命中测试
        return HitTestResult::kNotHit;
    }
    if ((dwExStyle & WS_EX_LAYERED) != 0) {
        //完全透明的分层窗口，不参与命中测试
        BYTE nAlpha = 255;
        DWORD dwFlags = 0;
        COLORREF colorKey = 0;
        if (::GetLayeredWindowAttributes(hWnd, &colorKey, &nAlpha, &dwFlags) &&
            ((dwFlags & LWA_ALPHA) != 0) && (nAlpha == 0)) {
            return HitTestResult::kNotHit;
        }
    }
    return HitTestResult::kHit;
}

/** 在屏幕指定坐标点上，查找指定顶层窗口正下方第一个命中的顶层窗口
* @param [in] hWndTop 起始的顶层窗口（从该窗口的下一个Z序窗口开始查找）
* @param [in] ownerChain 当前窗口的父窗口/属主窗口链
* @param [in] ptScreen 屏幕坐标
* @param [out] bOwnerWindowBelow 是否命中了当前窗口的父窗口/属主窗口(命中时不需要切换前台窗口)
* @return 找到的其它窗口，未找到返回nullptr
*/
static HWND FindWindowBelow(HWND hWndTop, const std::vector<HWND>& ownerChain, const POINT& ptScreen,
                            bool& bOwnerWindowBelow)
{
    bOwnerWindowBelow = false;
    if ((hWndTop == nullptr) || !::IsWindow(hWndTop)) {
        return nullptr;
    }
    HWND hWnd = ::GetWindow(hWndTop, GW_HWNDNEXT);
    while ((hWnd != nullptr) && ::IsWindow(hWnd)) {
        HitTestResult result = HitTestWindowAtPoint(hWnd, ownerChain, ptScreen);
        if (result == HitTestResult::kOwnerWindow) {
            bOwnerWindowBelow = true;
            return nullptr;
        }
        if (result == HitTestResult::kHit) {
            return hWnd;
        }
        hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
    }
    return nullptr;
}

bool ClickThrough::ClickThroughWindow(Window* pWindow, const UiPoint& ptMouse)
{
    if ((pWindow == nullptr) || (pWindow->NativeWnd() == nullptr)) {
        return false;
    }
    HWND hWnd = pWindow->NativeWnd()->GetHWND();
    if ((hWnd == nullptr) || !::IsWindow(hWnd)) {
        return false;
    }

    //ptMouse为屏幕坐标
    POINT ptScreen;
    ptScreen.x = ptMouse.x;
    ptScreen.y = ptMouse.y;

    //从被点击的窗口开始查找：不能使用GetAncestor(hWnd, GA_ROOT)，
    //因为窗口可能是子窗口，此时GA_ROOT返回的是宿主窗口，会从宿主窗口的位置开始向下查找，结果错误
    HWND hWndSelf = hWnd;

    //当前窗口的父窗口/属主窗口链(含duilib记录的父窗口，SDL弹出窗口的系统级属主不是其父窗口)
    std::vector<HWND> ownerChain;
    GetOwnerWindowChain(hWndSelf, ownerChain);
    AddParentWindowChain(pWindow, ownerChain);

    HWND hWndUnder = ::WindowFromPoint(ptScreen);
    bool bOwnerWindowBelow = false;
    if ((hWndUnder != nullptr) &&
        ((hWndUnder == hWndSelf) || ::IsChild(hWndSelf, hWndUnder) ||
         (::GetAncestor(hWndUnder, GA_ROOT) == hWndSelf))) {
        //Windows 10/11下，仅设置WS_EX_TRANSPARENT(未设置WS_EX_LAYERED)不会使窗口被
        //WindowFromPoint跳过，此时WindowFromPoint的结果仍是当前窗口；
        //这种情况下按Z序查找当前窗口正下方第一个在该点命中的窗口
        hWndUnder = FindWindowBelow(hWndSelf, ownerChain, ptScreen, bOwnerWindowBelow);
    }
    else if (hWndUnder != nullptr) {
        //如果WindowFromPoint命中的是当前窗口的父窗口/属主窗口(例如子界面后方的主界面)，
        //也不需要切换前台窗口
        HWND hWndUnderRoot = ::GetAncestor(hWndUnder, GA_ROOT);
        if ((hWndUnderRoot != nullptr) && IsOwnerWindow(ownerChain, hWndUnderRoot)) {
            bOwnerWindowBelow = true;
        }
    }

    if (bOwnerWindowBelow) {
        //鼠标下方是当前窗口的父窗口/属主窗口：保持现状，不切换前台窗口
        return false;
    }
    if ((hWndUnder == nullptr) || !::IsWindow(hWndUnder)) {
        return false;
    }

    //获取鼠标下方窗口所属的顶层窗口：
    //不能使用GetParent逐级向上，因为带Owner的顶层窗口(例如弹窗、对话框)会取到Owner，导致激活错误的窗口
    HWND hWndTarget = ::GetAncestor(hWndUnder, GA_ROOT);
    if ((hWndTarget == nullptr) || !::IsWindow(hWndTarget)) {
        hWndTarget = hWndUnder;
    }
    if (hWndTarget == hWndSelf) {
        return false;
    }
    if (::IsHungAppWindow(hWndTarget)) {
        //目标窗口无响应时，附加线程输入可能导致当前线程一起阻塞
        return false;
    }

    //跨线程激活窗口时，需要先附加线程输入，否则激活无效
    DWORD dwThreadId = ::GetWindowThreadProcessId(hWnd, nullptr);
    DWORD dwUnderThreadId = ::GetWindowThreadProcessId(hWndTarget, nullptr);
    bool bAttached = (dwThreadId != 0) && (dwUnderThreadId != 0) && (dwThreadId != dwUnderThreadId);
    if (bAttached) {
        ::AttachThreadInput(dwThreadId, dwUnderThreadId, TRUE);
    }
    BOOL bRet = ::SetForegroundWindow(hWndTarget);
    if (bAttached) {
        ::AttachThreadInput(dwThreadId, dwUnderThreadId, FALSE);
    }
    if (bRet == FALSE) {
        LogUtil::OutputLine(_T("ClickThroughWindow: 激活窗口失败(SetForegroundWindow)"));
    }
    return bRet != FALSE;
}

}//namespace ui

#endif //DUILIB_BUILD_FOR_WIN
