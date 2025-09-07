#include "NativeWindow_Windows.h"
#include "duilib/Utils/StringConvert.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Utils/ApiWrapper_Windows.h"
#include "duilib/Utils/InlineHook_Windows.h"
#include "duilib/Core/WindowDropTarget_Windows.h"

#include <CommCtrl.h>
#include <Olectl.h>
#include <VersionHelpers.h>

namespace ui {

//判断是否为Windows 11的函数
static bool UiIsWindows11OrGreater()
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
    DWORDLONG const dwlConditionMask = VerSetConditionMask(
        VerSetConditionMask(
            VerSetConditionMask(
                0, VER_MAJORVERSION, VER_GREATER_EQUAL),
            VER_MINORVERSION, VER_GREATER_EQUAL),
        VER_BUILDNUMBER, VER_GREATER_EQUAL);

    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber = 22000; //需要根据Build版本号区分

    return ::VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}

//系统菜单延迟显示的定时器ID
#define UI_SYS_MEMU_TIMER_ID 711

NativeWindow_Windows::NativeWindow_Windows(INativeWindow* pOwner):
    m_pOwner(pOwner),
    m_hWnd(nullptr),
    m_hParentWnd(nullptr),
    m_hResModule(nullptr),
    m_hDcPaint(nullptr),
    m_bIsLayeredWindow(false),
    m_nLayeredWindowAlpha(255),
    m_nLayeredWindowOpacity(255),
    m_bUseSystemCaption(false),
    m_bCloseing(false),
    m_closeParam(kWindowCloseNormal),
    m_bFakeModal(false),
    m_bFullScreen(false),
    m_dwLastStyle(0),
    m_ptLastMousePos(-1, -1),
    m_pfnOldWndProc(nullptr),
    m_bEnableDragDrop(true),
    m_bDoModal(false),
    m_bCloseByEsc(false),
    m_bCloseByEnter(false),
    m_bSnapLayoutMenu(false),
    m_bEnableSysMenu(true),
    m_bNCLButtonDownOnMaxButton(false),
    m_nSysMenuTimerId(0),
    m_hImc(nullptr),
    m_pWindowDropTarget(nullptr)
{
    ASSERT(m_pOwner != nullptr);
    m_rcLastWindowPlacement = { sizeof(WINDOWPLACEMENT), };

    //Windows 11及新版本，支持显示贴靠布局菜单（默认关闭，最新版的Win11下，会触发NC绘制，显示出系统绘制的内容，效果不好）
    /*if (UiIsWindows11OrGreater()) {
        m_bSnapLayoutMenu = true;
    }*/
}

NativeWindow_Windows::~NativeWindow_Windows()
{
    ASSERT(m_pfnOldWndProc == nullptr);
    ASSERT(m_hWnd == nullptr);
    ClearNativeWindow();
}

bool NativeWindow_Windows::CreateWnd(NativeWindow_Windows* pParentWindow,
                                     const WindowCreateParam& createParam,
                                     const WindowCreateAttributes& createAttributes)
{
    ASSERT(m_hWnd == nullptr);
    if (m_hWnd != nullptr) {
        return false;
    }
    ASSERT(!createParam.m_className.empty());
    if (createParam.m_className.empty()) {
        return false;
    }

    m_hResModule = (HMODULE)createParam.m_platformData;
    if (m_hResModule == nullptr) {
        m_hResModule = ::GetModuleHandle(nullptr);
    }

    //注册窗口类
    DString className = StringConvert::TToLocal(createParam.m_className);
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = createParam.m_dwClassStyle;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.lpfnWndProc = NativeWindow_Windows::__WndProc;
    wc.hInstance = GetResModuleHandle();
    wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = className.c_str();
    wc.hIcon = nullptr;
    wc.hIconSm = nullptr;

    ATOM ret = ::RegisterClassEx(&wc);
    bool bRet = (ret != 0 || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    ASSERT(bRet);
    if (!bRet) {
        return false;
    }

    //保存参数
    m_createParam = createParam;

    //设置默认风格
    if (m_createParam.m_dwStyle == 0) {
        m_createParam.m_dwStyle = kWS_OVERLAPPEDWINDOW;
    }

    //同步XML文件中Window的属性，在创建窗口的时候带着这些属性
    SyncCreateWindowAttributes(createAttributes);

    if (m_createParam.m_bCenterWindow) {
        //窗口居中时，计算窗口的起始位置，避免窗口弹出时出现窗口位置变动的现象
        int32_t xPos = 0;
        int32_t yPos = 0;
        HWND hCenterWindow = nullptr;
        if (pParentWindow != nullptr) {
            hCenterWindow = pParentWindow->GetHWND();
        }
        if (CalculateCenterWindowPos(hCenterWindow, xPos, yPos)) {
            m_createParam.m_nX = xPos;
            m_createParam.m_nY = yPos;
        }
    }

    //父窗口句柄
    m_hParentWnd = pParentWindow != nullptr ? pParentWindow->GetHWND() : nullptr;

    //窗口标题
    DString windowTitle = StringConvert::TToLocal(m_createParam.m_windowTitle);
    HWND hWnd = ::CreateWindowEx(m_createParam.m_dwExStyle,
                                 className.c_str(),
                                 windowTitle.c_str(),
                                 m_createParam.m_dwStyle,
                                 m_createParam.m_nX, m_createParam.m_nY, m_createParam.m_nWidth, m_createParam.m_nHeight,
                                 m_hParentWnd, nullptr, GetResModuleHandle(), this);
    ASSERT(::IsWindow(hWnd));
    ASSERT(hWnd == m_hWnd);
    if (hWnd != m_hWnd) {
        m_hWnd = hWnd;
    }
    ASSERT(m_hWnd != nullptr);
    if (m_hWnd == nullptr) {
        m_hParentWnd = nullptr;
        return false;
    }
    return (m_hWnd != nullptr);
}

//使得DoModal的界面中，可以输入文字
#ifdef DUILIB_ENABLE_INLINE_HOOK

/** Hook函数的单例对象
*/
class UILIB_API HookIsDialogMessage: public InlineHook
{
public:
    HookIsDialogMessage() = default;
    ~HookIsDialogMessage() = default;
    HookIsDialogMessage(const HookIsDialogMessage&) = delete;
    HookIsDialogMessage& operator = (const HookIsDialogMessage&) = delete;


    static HookIsDialogMessage& Instance()
    {
        static HookIsDialogMessage self;
        return self;
    }
};


/** 目标函数的类型
*/
typedef BOOL(WINAPI* PfnIsDialogMessage)(_In_ HWND hDlg, _In_ LPMSG lpMsg);

/** 替换后的函数
*/
static BOOL WINAPI IsDialogMessageDuiLib(_In_ HWND hDlg, _In_ LPMSG lpMsg)
{
    // 调用原始函数（通过跳板）
    BOOL bRet = FALSE;
    if ((lpMsg != nullptr) && (lpMsg->message == WM_CHAR)) {
        //不将WM_CHAR识别为对话框消息
        return bRet;
    }
    auto original = HookIsDialogMessage::Instance().GetTrampoline<PfnIsDialogMessage>();
    if (original) {
#if defined (_MSC_VER)
        __try {
            bRet = original(hDlg, lpMsg);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            bRet = false;
        }
#else
        bRet = original(hDlg, lpMsg);
#endif
    }
    return bRet;
}

#endif //DUILIB_ENABLE_INLINE_HOOK

int32_t NativeWindow_Windows::DoModal(NativeWindow_Windows* pParentWindow,
                                      const WindowCreateParam& createParam,
                                      const WindowCreateAttributes& createAttributes,
                                      bool bCloseByEsc, bool bCloseByEnter)
{
    ASSERT(m_hWnd == nullptr);
    if (m_hWnd != nullptr) {
        return -1;
    }
    m_hResModule = (HMODULE)createParam.m_platformData;
    if (m_hResModule == nullptr) {
        m_hResModule = ::GetModuleHandle(nullptr);
    }

    //保存参数
    m_createParam = createParam;
    m_bCloseByEsc = bCloseByEsc;
    m_bCloseByEnter = bCloseByEnter;

    //设置默认风格
    if (m_createParam.m_dwStyle == 0) {
        m_createParam.m_dwStyle = kWS_POPUPWINDOW;
    }

    //同步XML文件中Window的属性，在创建窗口的时候带着这些属性
    SyncCreateWindowAttributes(createAttributes);

    if (m_createParam.m_bCenterWindow) {
        //窗口居中时，计算窗口的起始位置，避免窗口弹出时出现窗口位置变动的现象
        int32_t xPos = 0;
        int32_t yPos = 0;
        HWND hCenterWindow = nullptr;
        if (pParentWindow != nullptr) {
            hCenterWindow = pParentWindow->GetHWND();
        }
        if (CalculateCenterWindowPos(hCenterWindow, xPos, yPos)) {
            m_createParam.m_nX = xPos;
            m_createParam.m_nY = yPos;
        }
    }

    //窗口的位置和大小
    short x = 0;
    short y = 0;
    short cx = 0;
    short cy = 0;

    if (m_createParam.m_nX != kCW_USEDEFAULT) {
        x = (short)m_createParam.m_nX;
    }
    if (m_createParam.m_nY != kCW_USEDEFAULT) {
        y = (short)m_createParam.m_nY;
    }

    // 创建对话框资源结构体（对话框初始状态为可见状态）
    DLGTEMPLATE dlgTemplate = {
        WS_VISIBLE | m_createParam.m_dwStyle,
        m_createParam.m_dwExStyle,
        0,
        x, y, cx, cy
    };

    // 将对话框按钮添加到资源中
    constexpr const size_t nDataSize = sizeof(DLGTEMPLATE) + 32;
    HGLOBAL hResource = ::GlobalAlloc(GMEM_MOVEABLE, nDataSize);
    if (hResource == nullptr) {
        return -1;
    }
    LPDLGTEMPLATE lpDialogTemplate = (LPDLGTEMPLATE)::GlobalLock(hResource);
    if (lpDialogTemplate == nullptr) {
        ::GlobalFree(hResource);
        return -1;
    }
    ::memset(lpDialogTemplate, 0, nDataSize);
    *lpDialogTemplate = dlgTemplate;

    // 显示对话框
    HWND hParentWnd = nullptr;
    if (pParentWindow != nullptr) {
        hParentWnd = pParentWindow->GetHWND();
    }
    else {
        hParentWnd = ::GetActiveWindow();
    }
    //标记为模式对话框状态
    m_bDoModal = true;

#ifdef DUILIB_ENABLE_INLINE_HOOK
    //处理IsDialogMessage，支持RichEdit控件输入文字
    {
        FARPROC targetFunc = nullptr;
        HMODULE hModule = ::GetModuleHandle(_T("User32.dll"));
        if (hModule != nullptr) {
#if defined(UNICODE) || defined(_UNICODE)
            targetFunc = ::GetProcAddress(hModule, "IsDialogMessageW");
#else
            targetFunc = ::GetProcAddress(hModule, "IsDialogMessageA");
#endif
        }
        if (targetFunc != nullptr) {
            HookIsDialogMessage::Instance().Install((void*)targetFunc, (void*)IsDialogMessageDuiLib);
        }
        HookIsDialogMessage::Instance().Install((void*)::IsDialogMessage, (void*)IsDialogMessageDuiLib);
    }
#endif //DUILIB_ENABLE_INLINE_HOOK

    //显示模态对话框
    INT_PTR nRet = ::DialogBoxIndirectParam(GetResModuleHandle(), (LPDLGTEMPLATE)lpDialogTemplate, hParentWnd, NativeWindow_Windows::__DialogProc, (LPARAM)this);
    // 清理资源
    ::GlobalUnlock(hResource);
    ::GlobalFree(hResource);
    if (nRet != -1) {
        nRet = m_closeParam;
    }

#ifdef DUILIB_ENABLE_INLINE_HOOK
    HookIsDialogMessage::Instance().Uninstall();
#endif

    return (int32_t)nRet;
}

void NativeWindow_Windows::SyncCreateWindowAttributes(const WindowCreateAttributes& createAttributes)
{
    m_bUseSystemCaption = false;
    if (createAttributes.m_bUseSystemCaptionDefined && createAttributes.m_bUseSystemCaption) {
        //使用系统标题栏
        if (m_createParam.m_dwStyle & WS_POPUP) {
            //弹出式窗口
            m_createParam.m_dwStyle |= (WS_CAPTION | WS_SYSMENU);
        }
        else {
            m_createParam.m_dwStyle |= (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        }
        m_bUseSystemCaption = true;
    }

    //初始化层窗口属性
    m_bIsLayeredWindow = false;
    if (createAttributes.m_bIsLayeredWindowDefined) {
        if (createAttributes.m_bIsLayeredWindow) {
            m_bIsLayeredWindow = true;
            m_createParam.m_dwExStyle |= WS_EX_LAYERED;
        }
        else {
            m_createParam.m_dwExStyle &= ~WS_EX_LAYERED;
        }
    }
    else if (m_createParam.m_dwExStyle & WS_EX_LAYERED) {
        m_bIsLayeredWindow = true;
    }

    //如果使用系统标题栏，关闭层窗口
    if (createAttributes.m_bUseSystemCaptionDefined && createAttributes.m_bUseSystemCaption) {
        m_bIsLayeredWindow = false;
        m_createParam.m_dwExStyle &= ~WS_EX_LAYERED;
    }

    //如果设置了不透明度，则设置为层窗口
    if (createAttributes.m_bLayeredWindowOpacityDefined && (createAttributes.m_nLayeredWindowOpacity != 255)) {
        m_createParam.m_dwExStyle |= WS_EX_LAYERED;
        m_bIsLayeredWindow = true;
    }

    if (createAttributes.m_bInitSizeDefined) {
        if (createAttributes.m_szInitSize.cx > 0) {
            m_createParam.m_nWidth = createAttributes.m_szInitSize.cx;
        }
        if (createAttributes.m_szInitSize.cy > 0) {
            m_createParam.m_nHeight = createAttributes.m_szInitSize.cy;
        }
    }
}

LRESULT NativeWindow_Windows::OnCreateMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    bHandled = false;
    //初始化
    InitNativeWindow();

    if (m_pOwner != nullptr) {
        m_pOwner->OnNativeCreateWndMsg(false, NativeMsg(uMsg, wParam, lParam), bHandled);
    }

    //更新最大化/最小化按钮的风格
    UpdateMinMaxBoxStyle();

    if (m_createParam.m_bCenterWindow) {
        //在创建完成后设置窗口居中(弹出式窗口设置的初始位置不生效，需要窗口后设置，不影响效果)
        CenterWindow();
    }
    return 0;
}

LRESULT NativeWindow_Windows::OnInitDialogMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    //初始化
    InitNativeWindow();

    if (m_pOwner != nullptr) {
        m_pOwner->OnNativeCreateWndMsg(true, NativeMsg(uMsg, wParam, lParam), bHandled);
    }

    //更新最大化/最小化按钮的风格
    UpdateMinMaxBoxStyle();

    if (m_createParam.m_bCenterWindow) {
        //在创建完成后设置窗口居中(弹出式窗口设置的初始位置不生效，需要窗口后设置，不影响效果)
        CenterWindow();
    }

    //标记为已经处理，不再调用窗口默认处理函数
    bHandled = true;
    return TRUE;
}

void NativeWindow_Windows::InitNativeWindow()
{
    HWND hWnd = m_hWnd;
    if (!::IsWindow(hWnd)) {
        return;
    }

    //检查并更新曾窗口属性
    m_bIsLayeredWindow = false;
    if (m_createParam.m_dwExStyle & WS_EX_LAYERED) {
        m_bIsLayeredWindow = true;
    }
    bool bChanged = false;
    SetLayeredWindowStyle(m_bIsLayeredWindow, bChanged);

    //初始化窗口相关DC
    ASSERT(m_hDcPaint == nullptr);
    m_hDcPaint = ::GetDC(hWnd);

    //注册接受Touch消息
    RegisterTouchWindowWrapper(hWnd, 0);

    if (!m_createParam.m_windowTitle.empty()) {
        DString windowTitle = StringConvert::TToLocal(m_createParam.m_windowTitle);
        ::SetWindowText(hWnd, windowTitle.c_str());
    }

    //关联拖放操作
    SetEnableDragDrop(IsEnableDragDrop());
}

void NativeWindow_Windows::ClearNativeWindow()
{
    //注销平板消息
    HWND hWnd = GetHWND();
    if (hWnd != nullptr) {
        UnregisterTouchWindowWrapper(hWnd);
    }
    //注销快捷键
    std::vector<int32_t> hotKeyIds = m_hotKeyIds;
    for (int32_t id : hotKeyIds) {
        UnregisterHotKey(id);
    }
    if (m_hDcPaint != nullptr) {
        ::ReleaseDC(m_hWnd, m_hDcPaint);
        m_hDcPaint = nullptr;
    }
    if (m_hImc != nullptr) {
        ::ImmAssociateContext(m_hWnd, m_hImc);
        m_hImc = nullptr;
    }
    if (m_pWindowDropTarget != nullptr) {
        m_pWindowDropTarget->UnregisterDragDrop();
        m_pWindowDropTarget->Release();
        m_pWindowDropTarget = nullptr;
    }
    m_hWnd = nullptr;
}

HWND NativeWindow_Windows::GetHWND() const
{
    return m_hWnd;
}

void* NativeWindow_Windows::GetWindowHandle() const
{
    return m_hWnd;
}

bool NativeWindow_Windows::IsWindow() const
{
    return (m_hWnd != nullptr) && ::IsWindow(m_hWnd);
}

HMODULE NativeWindow_Windows::GetResModuleHandle() const
{
    return (m_hResModule != nullptr) ? m_hResModule : (::GetModuleHandle(nullptr));
}

HDC NativeWindow_Windows::GetPaintDC() const
{
    return m_hDcPaint;
}

void NativeWindow_Windows::CloseWnd(int32_t nRet)
{
    StopSysMenuTimer();
    m_bCloseing = true;
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::PostMessage(m_hWnd, WM_CLOSE, (WPARAM)nRet, 0L);
}

void NativeWindow_Windows::Close()
{
    StopSysMenuTimer();
    m_bCloseing = true;
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return;
    }
    ::SendMessage(m_hWnd, WM_CLOSE, 0L, 0L);
}

bool NativeWindow_Windows::IsClosingWnd() const
{
    return m_bCloseing;
}

int32_t NativeWindow_Windows::GetCloseParam() const
{
    return m_closeParam;
}

bool NativeWindow_Windows::SetLayeredWindow(bool bIsLayeredWindow, bool bRedraw)
{
    m_bIsLayeredWindow = bIsLayeredWindow;
    bool bChanged = false;
    SetLayeredWindowStyle(bIsLayeredWindow, bChanged);
    if (bRedraw && bChanged && IsWindow()) {
        // 强制窗口重绘
        ::SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
    }
    return true;
}

bool NativeWindow_Windows::SetLayeredWindowStyle(bool bIsLayeredWindow, bool& bChanged) const
{
    bChanged = false;
    if (::IsWindow(m_hWnd)) {
        LONG dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        LONG dwOldExStyle = dwExStyle;
        if (bIsLayeredWindow) {
            dwExStyle |= WS_EX_LAYERED;
        }
        else {
            dwExStyle &= ~WS_EX_LAYERED;
        }
        if (dwOldExStyle != dwExStyle) {
            bChanged = true;
            ::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwExStyle);
            dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        }
        if (bIsLayeredWindow) {
            return (dwExStyle & WS_EX_LAYERED) ? true : false;
        }
        else {
            return (dwExStyle & WS_EX_LAYERED) ? false : true;
        }
    }
    return false;
}

bool NativeWindow_Windows::IsLayeredWindow() const
{
#if _DEBUG
    if (::IsWindow(m_hWnd)) {
        LONG dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
        bool bIsLayeredWindow = (dwExStyle & WS_EX_LAYERED) ? true : false;
        ASSERT(bIsLayeredWindow == m_bIsLayeredWindow);
    }
#endif // _DEBUG
    return m_bIsLayeredWindow;
}

void NativeWindow_Windows::UpdateMinMaxBoxStyle() const
{
    //更新最大化/最小化按钮的风格
    bool bMinimizeBox = false;
    bool bMaximizeBox = false;
    if (!IsUseSystemCaption() && (m_pOwner != nullptr) && m_pOwner->OnNativeHasMinMaxBox(bMinimizeBox, bMaximizeBox)) {
        UINT oldStyleValue = (UINT)::GetWindowLong(GetHWND(), GWL_STYLE);
        UINT newStyleValue = oldStyleValue;
        if (bMinimizeBox) {
            newStyleValue |= WS_MINIMIZEBOX;
        }
        else {
            newStyleValue &= ~WS_MINIMIZEBOX;
        }
        if (bMaximizeBox) {
            newStyleValue |= WS_MAXIMIZEBOX;
        }
        else {
            newStyleValue &= ~WS_MAXIMIZEBOX;
        }
        if (newStyleValue != oldStyleValue) {
            ::SetWindowLong(GetHWND(), GWL_STYLE, newStyleValue);
        }
    }
}

void NativeWindow_Windows::SetLayeredWindowAlpha(int32_t nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    m_nLayeredWindowAlpha = static_cast<uint8_t>(nAlpha);
}

uint8_t NativeWindow_Windows::GetLayeredWindowAlpha() const
{
    return m_nLayeredWindowAlpha;
}

void NativeWindow_Windows::SetLayeredWindowOpacity(int32_t nAlpha)
{
    ASSERT(nAlpha >= 0 && nAlpha <= 255);
    if ((nAlpha < 0) || (nAlpha > 255)) {
        return;
    }
    m_nLayeredWindowOpacity = static_cast<uint8_t>(nAlpha);
    if (m_nLayeredWindowOpacity == 255) {
        COLORREF crKey = 0;
        BYTE bAlpha = 0;
        DWORD dwFlags = LWA_ALPHA | LWA_COLORKEY;
        bool bAttributes = ::GetLayeredWindowAttributes(m_hWnd, &crKey, &bAlpha, &dwFlags) != FALSE;
        if (bAttributes) {
            bool bRet = ::SetLayeredWindowAttributes(m_hWnd, 0, m_nLayeredWindowOpacity, LWA_ALPHA) != FALSE;
            ASSERT_UNUSED_VARIABLE(bRet);
        }
    }
    else {
        //必须先设置为分层窗口，然后才能设置成功
        SetLayeredWindow(true, false);
        bool bRet = ::SetLayeredWindowAttributes(m_hWnd, 0, m_nLayeredWindowOpacity, LWA_ALPHA) != FALSE;
        ASSERT_UNUSED_VARIABLE(bRet);
    }
}

uint8_t NativeWindow_Windows::GetLayeredWindowOpacity() const
{
    return m_nLayeredWindowOpacity;
}

void NativeWindow_Windows::SetUseSystemCaption(bool bUseSystemCaption)
{
    m_bUseSystemCaption = bUseSystemCaption;
    if (IsUseSystemCaption()) {
        //使用系统默认标题栏, 需要增加标题栏风格
        bool bChanged = false;
        if (IsWindow()) {
            UINT oldStyleValue = (UINT)::GetWindowLong(GetHWND(), GWL_STYLE);
            UINT newStyleValue = oldStyleValue;
            if (oldStyleValue & WS_POPUP) {
                //弹出式窗口
                newStyleValue |= (WS_CAPTION | WS_SYSMENU);
            }
            else {
                newStyleValue |= (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);                
            }
            if (newStyleValue != oldStyleValue) {
                ::SetWindowLong(GetHWND(), GWL_STYLE, newStyleValue);
                bChanged = true; 
            }
        }
        //关闭层窗口
        if (IsLayeredWindow()) {
            bChanged = true;
            SetLayeredWindow(false, false);
        }
        if (bChanged) {
            // 强制窗口重绘
            ::SetWindowPos(GetHWND(), nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
            //重新激活窗口的非客户区绘制
            if (IsWindowForeground()) {
                KeepParentActive();
            }            
        }        
    }
    m_pOwner->OnNativeUseSystemCaptionBarChanged();
}

bool NativeWindow_Windows::IsUseSystemCaption() const
{
    return m_bUseSystemCaption;
}

bool NativeWindow_Windows::ShowWindow(ShowWindowCommands nCmdShow)
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    bool bRet = false;
    int nWindowCmdShow = SW_SHOWNORMAL;
    switch(nCmdShow)
    {
    case kSW_HIDE:
        nWindowCmdShow = SW_HIDE;
        break;
    case kSW_SHOW_NORMAL:
        nWindowCmdShow = SW_SHOWNORMAL;
        break;
    case kSW_SHOW_MINIMIZED:
        nWindowCmdShow = SW_SHOWMINIMIZED;
        break;
    case kSW_SHOW_MAXIMIZED:
        nWindowCmdShow = SW_SHOWMAXIMIZED;
        break;
    case kSW_SHOW_NOACTIVATE:
        nWindowCmdShow = SW_SHOWNOACTIVATE;
        break;
    case kSW_SHOW:
        nWindowCmdShow = SW_SHOW;
        break;
    case kSW_MINIMIZE:
        nWindowCmdShow = SW_MINIMIZE;
        break;
    case kSW_SHOW_MIN_NOACTIVE:
        nWindowCmdShow = SW_SHOWMINNOACTIVE;
        break;
    case kSW_SHOW_NA:
        nWindowCmdShow = SW_SHOWNA;
        break;
    case kSW_RESTORE:
        nWindowCmdShow = SW_RESTORE;
        break;
    default:
        ASSERT(false);
        break;
    }
    bRet = ::ShowWindow(m_hWnd, nWindowCmdShow) != FALSE;
    return bRet;
}

void NativeWindow_Windows::ShowModalFake(NativeWindow_Windows* pParentWindow)
{
    ASSERT(::IsWindow(m_hWnd));
    ASSERT((pParentWindow != nullptr) && (pParentWindow->GetHWND() != nullptr));
    if (pParentWindow != nullptr) {
        auto hOwnerWnd = GetWindowOwner();
        ASSERT(::IsWindow(hOwnerWnd));
        ASSERT_UNUSED_VARIABLE(hOwnerWnd == pParentWindow->GetHWND());
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(false);
        }
    }
    ShowWindow(kSW_SHOW_NORMAL);
    m_bFakeModal = true;
}

void NativeWindow_Windows::OnCloseModalFake(NativeWindow_Windows* pParentWindow)
{
    if (IsFakeModal()) {
        if (pParentWindow != nullptr) {
            pParentWindow->EnableWindow(true);
            pParentWindow->SetWindowFocus();
        }
        m_bFakeModal = false;
    }
    if (IsWindowFocused()) {
        SetOwnerWindowFocus();
    }
}

bool NativeWindow_Windows::IsFakeModal() const
{
    return m_bFakeModal;
}

bool NativeWindow_Windows::IsDoModal() const
{
    return m_bDoModal;
}

void NativeWindow_Windows::CenterWindow()
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    ASSERT((::GetWindowLong(m_hWnd, GWL_STYLE) & WS_CHILD) == 0);
    HWND hCenterWindow = ::GetParent(m_hWnd);
    if (hCenterWindow == nullptr) {
        if ((m_hParentWnd != nullptr) && ::IsWindow(m_hParentWnd)) {
            hCenterWindow = m_hParentWnd;
        }
    }
    int32_t xPos = 0;
    int32_t yPos = 0;
    if (CalculateCenterWindowPos(hCenterWindow, xPos, yPos)) {
        ::SetWindowPos(m_hWnd, nullptr, xPos, yPos, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

bool NativeWindow_Windows::CalculateCenterWindowPos(HWND hCenterWindow, int32_t& xPos, int32_t& yPos) const
{
    if (!::IsWindow(hCenterWindow)) {
        hCenterWindow = nullptr;
    }
    //当前窗口的宽度和高度
    int32_t nWindowWidth = 0;
    int32_t nWindowHeight = 0;
    if (IsWindow()) {
        UiRect rcDlg;
        GetWindowRect(rcDlg);
        nWindowWidth = rcDlg.Width();
        nWindowHeight = rcDlg.Height();
    }
    else {
        if ((m_createParam.m_nWidth <= 0) || (m_createParam.m_nHeight <= 0)) {
            //当前的窗口宽度未知，无法计算
            return false;
        }
        nWindowWidth = m_createParam.m_nWidth;
        nWindowHeight = m_createParam.m_nHeight;
    }

    UiRect rcArea;
    UiRect rcCenter;
    UiRect rcMonitor;
    GetMonitorRect(hCenterWindow != nullptr ? hCenterWindow : GetHWND(), rcMonitor, rcArea);
    if (hCenterWindow == nullptr) {
        rcCenter = rcArea;
    }
    else if (::IsIconic(hCenterWindow)) {
        rcCenter = rcArea;
    }
    else {
        GetWindowRect(hCenterWindow, rcCenter);
    }

    //屏幕编译留出空隙，避免出现贴边操作
    UINT dpi = 96;
    GetDpiForWindowWrapper(hCenterWindow != nullptr ? hCenterWindow : GetHWND(), dpi);
    const int32_t snapThreshold = MulDiv(3, dpi, 96);

    // Find dialog's upper left based on rcCenter
    int32_t xLeft = rcCenter.CenterX() - nWindowWidth / 2;
    int32_t yTop = rcCenter.CenterY() - nWindowHeight / 2;

    // The dialog is outside the screen, move it inside
    if (xLeft < rcArea.left) {
        xLeft = rcArea.left + snapThreshold;
    }
    else if (xLeft + nWindowWidth > rcArea.right) {
        xLeft = rcArea.right - nWindowWidth - snapThreshold;
    }
    if (yTop < rcArea.top) {
        yTop = rcArea.top + snapThreshold;
    }
    else if (yTop + nWindowHeight > rcArea.bottom) {
        yTop = rcArea.bottom - nWindowHeight - snapThreshold;
    }
    xPos = xLeft;
    yPos = yTop;
    return true;
}

void NativeWindow_Windows::SetWindowAlwaysOnTop(bool bOnTop)
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return;
    }
    if (bOnTop) {
        ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else {
        ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

bool NativeWindow_Windows::IsWindowAlwaysOnTop() const
{
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    LONG dwExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
    return (dwExStyle & WS_EX_TOPMOST) ? true : false;
}

bool NativeWindow_Windows::SetWindowForeground()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetForegroundWindow() != m_hWnd) {
        ::SetForegroundWindow(m_hWnd);
    }
    return ::GetForegroundWindow() == m_hWnd;
}

bool NativeWindow_Windows::IsWindowForeground() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetForegroundWindow());
}

bool NativeWindow_Windows::SetWindowFocus()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
    return ::GetFocus() == m_hWnd;
}

bool NativeWindow_Windows::KillWindowFocus()
{
    ASSERT(::IsWindow(m_hWnd));
    if (::GetFocus() == m_hWnd) {
        ::SetFocus(nullptr);
    }
    return ::GetFocus() != m_hWnd;
}

bool NativeWindow_Windows::IsWindowFocused() const
{
    return ::IsWindow(m_hWnd) && (m_hWnd == ::GetFocus());
}

bool NativeWindow_Windows::SetOwnerWindowFocus()
{
    HWND hwndParent = GetWindowOwner();
    if (hwndParent != nullptr) {
        ::SetFocus(hwndParent);
        return ::GetFocus() == hwndParent;
    }
    return false;
}

void NativeWindow_Windows::CheckSetWindowFocus()
{
    if (::GetFocus() != m_hWnd) {
        ::SetFocus(m_hWnd);
    }
}

LRESULT NativeWindow_Windows::PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void NativeWindow_Windows::PostQuitMsg(int32_t nExitCode)
{
    ::PostQuitMessage(nExitCode);
}

bool NativeWindow_Windows::EnterFullScreen()
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    if (IsWindowMinimized()) {
        //最小化的时候，不允许激活全屏
        return false;
    }
    if (m_bFullScreen) {
        return true;
    }
    m_bFullScreen = true;

    //保存窗口风格
    m_dwLastStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    //保存窗口大小位置信息
    m_rcLastWindowPlacement.length = sizeof(WINDOWPLACEMENT);
    ::GetWindowPlacement(m_hWnd, &m_rcLastWindowPlacement);

    UiRect rcMonitor;
    GetMonitorRect(rcMonitor);

    // 去掉标题栏、边框
    DWORD dwFullScreenStyle = (m_dwLastStyle | WS_VISIBLE | WS_POPUP | WS_MAXIMIZE) & ~WS_CAPTION & ~WS_BORDER & ~WS_THICKFRAME & ~WS_DLGFRAME;
    ::SetWindowLongPtr(m_hWnd, GWL_STYLE, dwFullScreenStyle);
    ::SetWindowPos(m_hWnd, nullptr, rcMonitor.left, rcMonitor.top, rcMonitor.Width(), rcMonitor.Height(), SWP_FRAMECHANGED); // 设置位置和大小
    
    m_pOwner->OnNativeWindowEnterFullScreen();
    return true;
}

bool NativeWindow_Windows::ExitFullScreen()
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }
    if (!m_bFullScreen) {
        return false;
    }
    
    //恢复窗口风格
    if (m_dwLastStyle != 0) {
        ::SetWindowLong(m_hWnd, GWL_STYLE, m_dwLastStyle);
        m_dwLastStyle = 0;
    }

    //恢复窗口位置/大小信息
    ::SetWindowPlacement(m_hWnd, &m_rcLastWindowPlacement);

    if (IsWindowMaximized()) {
        ::ShowWindow(m_hWnd, SW_RESTORE);
    }
    m_bFullScreen = false;
    m_pOwner->OnNativeWindowExitFullScreen();
    return true;
}

bool NativeWindow_Windows::IsWindowMaximized() const
{
    return ::IsWindow(m_hWnd) && ::IsZoomed(m_hWnd);
}

bool NativeWindow_Windows::IsWindowMinimized() const
{
    return ::IsWindow(m_hWnd) && ::IsIconic(m_hWnd);
}

bool NativeWindow_Windows::IsWindowFullScreen() const
{
    return m_bFullScreen;
}

bool NativeWindow_Windows::EnableWindow(bool bEnable)
{
    return ::EnableWindow(m_hWnd, bEnable ? TRUE : false) != FALSE;
}

bool NativeWindow_Windows::IsWindowEnabled() const
{
    return ::IsWindow(m_hWnd) && ::IsWindowEnabled(m_hWnd) != FALSE;
}

bool NativeWindow_Windows::IsWindowVisible() const
{
    return ::IsWindow(m_hWnd) && ::IsWindowVisible(m_hWnd) != FALSE;
}

bool NativeWindow_Windows::SetWindowPos(const NativeWindow_Windows* pInsertAfterWindow,
                                        InsertAfterFlag insertAfterFlag,
                                        int32_t X, int32_t Y, int32_t cx, int32_t cy,
                                        uint32_t uFlags)
{
    ASSERT(::IsWindow(m_hWnd));
    HWND hWndInsertAfter = HWND_TOP;
    if (!(uFlags & kSWP_NOZORDER)) {
        if (pInsertAfterWindow != nullptr) {
            hWndInsertAfter = pInsertAfterWindow->GetHWND();
        }
        else {
            hWndInsertAfter = (HWND)insertAfterFlag;
        }
    }
    return ::SetWindowPos(m_hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags) != FALSE;
}

bool NativeWindow_Windows::MoveWindow(int32_t X, int32_t Y, int32_t nWidth, int32_t nHeight, bool bRepaint)
{
    ASSERT(::IsWindow(m_hWnd));
    return ::MoveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint ? TRUE : FALSE) != FALSE;
}

bool NativeWindow_Windows::SetWindowIcon(const FilePath& iconFilePath)
{
    ASSERT(::IsWindow(m_hWnd));
    if (!::IsWindow(m_hWnd)) {
        return false;
    }

    uint32_t uDpi = m_pOwner->OnNativeGetDpi().GetDPI();
    //大图标
    int32_t cxIcon = GetSystemMetricsForDpiWrapper(SM_CXICON, uDpi);
    int32_t cyIcon = GetSystemMetricsForDpiWrapper(SM_CYICON, uDpi);
    HICON hIcon = (HICON)::LoadImage(nullptr, iconFilePath.NativePath().c_str(), IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_SHARED);
    ASSERT(hIcon != nullptr);
    if (hIcon != nullptr) {
        ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
    }
    else {
        return false;
    }

    //小图标
    cxIcon = GetSystemMetricsForDpiWrapper(SM_CXSMICON, uDpi);
    cyIcon = GetSystemMetricsForDpiWrapper(SM_CYSMICON, uDpi);
    hIcon = (HICON)::LoadImage(nullptr, iconFilePath.NativePath().c_str(), IMAGE_ICON, cxIcon, cyIcon, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_SHARED);
    ASSERT(hIcon != nullptr);
    if (hIcon != nullptr) {
        ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);
    }
    else {
        return false;
    }
    return true;
}

bool NativeWindow_Windows::SetWindowIcon(const std::vector<uint8_t>& iconFileData, const DString& /*iconFileName*/)
{
    //Little Endian Only
    int16_t test = 1;
    bool bLittleEndianHost = (*((char*)&test) == 1);
    ASSERT_UNUSED_VARIABLE(bLittleEndianHost);

    bool bValidIcoFile = false;
    std::vector<uint8_t> fileData = iconFileData;
    fileData.resize(fileData.size() + 1024); //填充空白
    typedef struct tagIconDir {
        uint16_t idReserved;
        uint16_t idType;
        uint16_t idCount;
    } ICONHEADER;
    typedef struct tagIconDirectoryEntry {
        uint8_t  bWidth;
        uint8_t  bHeight;
        uint8_t  bColorCount;
        uint8_t  bReserved;
        uint16_t  wPlanes;
        uint16_t  wBitCount;
        uint32_t dwBytesInRes;
        uint32_t dwImageOffset;
    } ICONDIRENTRY;

    ICONHEADER* icon_header = (ICONHEADER*)fileData.data();
    if ((icon_header->idReserved == 0) && (icon_header->idType == 1)) {
        bValidIcoFile = true;
        for (int32_t c = 0; c < icon_header->idCount; ++c) {
            size_t nDataOffset = sizeof(ICONHEADER) + sizeof(ICONDIRENTRY) * c;
            if (nDataOffset >= fileData.size()) {
                bValidIcoFile = false;
                break;
            }
            ICONDIRENTRY* pIconDir = (ICONDIRENTRY*)((uint8_t*)fileData.data() + nDataOffset);
            if (pIconDir->dwImageOffset >= iconFileData.size()) {
                bValidIcoFile = false;
                break;
            }
            else if ((pIconDir->dwImageOffset + pIconDir->dwBytesInRes) > iconFileData.size()) {
                bValidIcoFile = false;
                break;
            }
        }        
    }
    ASSERT(bValidIcoFile);
    if (!bValidIcoFile) {
        return false;
    }

    uint32_t uDpi = m_pOwner->OnNativeGetDpi().GetDPI();
    HICON hIcon = nullptr;
    //大图标
    int32_t cxIcon = GetSystemMetricsForDpiWrapper(SM_CXICON, uDpi);
    int32_t cyIcon = GetSystemMetricsForDpiWrapper(SM_CYICON, uDpi);
    int32_t offset = ::LookupIconIdFromDirectoryEx((PBYTE)fileData.data(), TRUE, cxIcon, cyIcon, LR_DEFAULTCOLOR | LR_SHARED);
    if (offset > 0) {
        hIcon = ::CreateIconFromResourceEx((PBYTE)fileData.data() + offset, (DWORD)fileData.size() - (DWORD)offset, TRUE, 0x00030000, cxIcon, cyIcon, LR_DEFAULTCOLOR | LR_SHARED);
        ASSERT(hIcon != nullptr);
        if (hIcon != nullptr) {
            ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
        }
        else {
            return false;
        }
    }

    //小图标
    cxIcon = GetSystemMetricsForDpiWrapper(SM_CXSMICON, uDpi);
    cyIcon = GetSystemMetricsForDpiWrapper(SM_CYSMICON, uDpi);
    offset = ::LookupIconIdFromDirectoryEx((PBYTE)fileData.data(), TRUE, cxIcon, cyIcon, LR_DEFAULTCOLOR | LR_SHARED);
    if (offset > 0) {
        hIcon = ::CreateIconFromResourceEx((PBYTE)fileData.data() + offset, (DWORD)fileData.size() - (DWORD)offset, TRUE, 0x00030000, cxIcon, cyIcon, LR_DEFAULTCOLOR | LR_SHARED);
        ASSERT(hIcon != nullptr);
        if (hIcon != nullptr) {
            ::SendMessage(m_hWnd, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);
        }
        else {
            return false;
        }
    }
    return true;
}

void NativeWindow_Windows::SetText(const DString& strText)
{
    ASSERT(::IsWindow(m_hWnd));
#ifdef DUILIB_UNICODE
    ::SetWindowText(m_hWnd, strText.c_str());
#else
    //strText是UTF-8编码
    DString localText = StringConvert::TToLocal(strText);
    ::SetWindowText(m_hWnd, localText.c_str());
#endif
}

DString NativeWindow_Windows::GetText() const
{
    ASSERT(::IsWindow(m_hWnd));
    DString text;
    int nLen = ::GetWindowTextLength(m_hWnd);
    if (nLen > 0) {
        std::vector<TCHAR> szText;
        szText.resize((size_t)nLen + 2);
        memset(szText.data(), 0, szText.size() * sizeof(TCHAR));
        ::GetWindowText(m_hWnd, szText.data(), (int)szText.size() - 1);
        DString localText = szText.data();
        text = StringConvert::LocalToT(localText);
    }
    return text;
}

void NativeWindow_Windows::SetWindowMaximumSize(const UiSize& szMaxWindow)
{
    m_szMaxWindow = szMaxWindow;
    if (m_szMaxWindow.cx < 0) {
        m_szMaxWindow.cx = 0;
    }
    if (m_szMaxWindow.cy < 0) {
        m_szMaxWindow.cy = 0;
    }
}

const UiSize& NativeWindow_Windows::GetWindowMaximumSize() const
{
    return m_szMaxWindow;
}

void NativeWindow_Windows::SetWindowMinimumSize(const UiSize& szMinWindow)
{
    m_szMinWindow = szMinWindow;
    if (m_szMinWindow.cx < 0) {
        m_szMinWindow.cx = 0;
    }
    if (m_szMinWindow.cy < 0) {
        m_szMinWindow.cy = 0;
    }
}

const UiSize& NativeWindow_Windows::GetWindowMinimumSize() const
{
    return m_szMinWindow;
}

void NativeWindow_Windows::SetCapture()
{
    if (::GetCapture() != m_hWnd) {
        ::SetCapture(m_hWnd);
    }
}

void NativeWindow_Windows::ReleaseCapture()
{
    if (::GetCapture() == m_hWnd) {
        ::ReleaseCapture();
    }
}

bool NativeWindow_Windows::IsCaptured() const
{
    return ::GetCapture() == m_hWnd;
}

bool NativeWindow_Windows::SetWindowRoundRectRgn(const UiRect& rcWnd, const UiSize& szRoundCorner, bool bRedraw)
{
    ASSERT((szRoundCorner.cx > 0) && (szRoundCorner.cy > 0));
    if ((szRoundCorner.cx <= 0) || (szRoundCorner.cy <= 0)) {
        return false;
    }
    ASSERT(IsWindow());
    if (!IsWindow()) {
        return false;
    }
    HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
    int nRet = ::SetWindowRgn(GetHWND(), hRgn, bRedraw ? TRUE : FALSE);
    ::DeleteObject(hRgn);//TODO: 检查是否需要删除，按MSDN说法，是不需要删除的。
    return nRet != 0;
}

void NativeWindow_Windows::ClearWindowRgn(bool bRedraw)
{
    ASSERT(IsWindow());
    ::SetWindowRgn(GetHWND(), nullptr, bRedraw ? TRUE : FALSE);
}

void NativeWindow_Windows::Invalidate(const UiRect& rcItem)
{
    RECT rc = { rcItem.left, rcItem.top, rcItem.right, rcItem.bottom };
    ::InvalidateRect(m_hWnd, &rc, FALSE);
    // Invalidating a layered window will not trigger a WM_PAINT message,
    // thus we have to post WM_PAINT by ourselves.
    if (IsLayeredWindow()) {
        ::PostMessage(m_hWnd, WM_PAINT, 0, 0);
    }
}

bool NativeWindow_Windows::UpdateWindow() const
{
    bool bRet = false;
    if ((m_hWnd != nullptr) && ::IsWindow(m_hWnd)) {
        bRet = ::UpdateWindow(m_hWnd) != FALSE;
    }
    return bRet;
}

/** 绘制的辅助类
*/
class NativeWindowRenderPaint:
    public IRenderPaint
{
public:
    NativeWindow_Windows* m_pNativeWindow = nullptr;
    INativeWindow* m_pOwner = nullptr;
    NativeMsg m_nativeMsg;
    bool m_bHandled = false;

public:
    /** 通过回调接口，完成绘制
    * @param [in] rcPaint 需要绘制的区域（客户区坐标）
    */
    virtual bool DoPaint(const UiRect& rcPaint) override
    {
        if (m_pOwner != nullptr) {
            m_pOwner->OnNativePaintMsg(rcPaint, m_nativeMsg, m_bHandled);
            return true;
        }
        return false;
    }

    /** 回调接口，获取当前窗口的透明度值
    */
    virtual uint8_t GetLayeredWindowAlpha() override
    {
        return m_pNativeWindow->GetLayeredWindowAlpha();
    }

    /** 获取界面需要绘制的区域，以实现局部绘制
    * @param [out] rcUpdate 返回需要绘制的区域矩形范围
    * @return 返回true表示支持局部绘制，返回false表示不支持局部绘制
    */
    virtual bool GetUpdateRect(UiRect& rcUpdate) const override
    {
        RECT rectUpdate = { 0, };
        if (::GetUpdateRect(m_pNativeWindow->GetHWND(), &rectUpdate, FALSE)) {
            rcUpdate.left = rectUpdate.left;
            rcUpdate.top = rectUpdate.top;
            rcUpdate.right = rectUpdate.right;
            rcUpdate.bottom = rectUpdate.bottom;
        }
        else {
            rcUpdate.Clear();
        }
        return !rcUpdate.IsEmpty();
    }
};

LRESULT NativeWindow_Windows::OnPaintMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    //回调准备绘制函数
    LRESULT lResult = 0;
    bHandled = false;
    bool bPaint = m_pOwner->OnNativePreparePaint();
    RECT rectUpdate = { 0, };
    if (!::GetUpdateRect(m_hWnd, &rectUpdate, FALSE)) {
        bPaint = false;
    }
    if (bPaint) {
        IRender* pRender = m_pOwner->OnNativeGetRender();
        ASSERT(pRender != nullptr);
        if (pRender != nullptr) {
            NativeWindowRenderPaint renderPaint;
            renderPaint.m_pNativeWindow = this;
            renderPaint.m_pOwner = m_pOwner;
            renderPaint.m_nativeMsg = NativeMsg(uMsg, wParam, lParam);
            renderPaint.m_bHandled = bHandled;
            bPaint = pRender->PaintAndSwapBuffers(&renderPaint);
            bHandled = renderPaint.m_bHandled;
        }
    }
    if (!bPaint) {
        PAINTSTRUCT ps = { 0, };
        ::BeginPaint(m_hWnd, &ps);
        ::EndPaint(m_hWnd, &ps);
    }
    return lResult;
}

void NativeWindow_Windows::KeepParentActive()
{
    HWND hWndParent = GetHWND();
    if (::IsWindow(hWndParent)) {
        while (::GetParent(hWndParent) != nullptr) {
            hWndParent = ::GetParent(hWndParent);
        }
    }
    if (::IsWindow(hWndParent)) {
        ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
    }
}

void NativeWindow_Windows::GetClientRect(UiRect& rcClient) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetClientRect(m_hWnd, &rc);
    rcClient = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void NativeWindow_Windows::GetWindowRect(UiRect& rcWindow) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetWindowRect(m_hWnd, &rc);
    rcWindow = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void NativeWindow_Windows::GetWindowRect(HWND hWnd, UiRect& rcWindow) const
{
    RECT rc = { 0, 0, 0, 0 };
    ::GetWindowRect(hWnd, &rc);
    rcWindow = UiRect(rc.left, rc.top, rc.right, rc.bottom);
}

void NativeWindow_Windows::ScreenToClient(UiPoint& pt) const
{
    POINT ptClient = { pt.x, pt.y };
    ::ScreenToClient(m_hWnd, &ptClient);
    pt = UiPoint(ptClient.x, ptClient.y);
}

void NativeWindow_Windows::ClientToScreen(UiPoint& pt) const
{
    POINT ptClient = { pt.x, pt.y };
    ::ClientToScreen(m_hWnd, &ptClient);
    pt = UiPoint(ptClient.x, ptClient.y);
}

void NativeWindow_Windows::GetCursorPos(UiPoint& pt) const
{
    POINT ptPos;
    ::GetCursorPos(&ptPos);
    pt = { ptPos.x, ptPos.y };
}

bool NativeWindow_Windows::GetMonitorRect(UiRect& rcMonitor) const
{
    UiRect rcWork;
    return GetMonitorRect(m_hWnd, rcMonitor, rcWork);
}

bool NativeWindow_Windows::GetMonitorRect(HWND hWnd, UiRect& rcMonitor, UiRect& rcWork) const
{
    rcMonitor.Clear();
    rcWork.Clear();
    HMONITOR hMonitor = nullptr;
    if (::IsWindow(hWnd)) {
        hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    }
    else {
        hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    }
    ASSERT(hMonitor != nullptr);
    if (hMonitor == nullptr) {
        return false;
    }
    MONITORINFO oMonitor = { 0, };
    oMonitor.cbSize = sizeof(oMonitor);
    if (::GetMonitorInfo(hMonitor, &oMonitor)) {
        rcWork = UiRect(oMonitor.rcWork.left, oMonitor.rcWork.top,
                        oMonitor.rcWork.right, oMonitor.rcWork.bottom);
        rcMonitor = UiRect(oMonitor.rcMonitor.left, oMonitor.rcMonitor.top,
                           oMonitor.rcMonitor.right, oMonitor.rcMonitor.bottom);
        return true;
    }
    else {
        ASSERT(!"NativeWindow_Windows::GetMonitorWorkRect failed!");
        return false;
    }
}

bool NativeWindow_Windows::GetMonitorWorkRect(UiRect& rcWork) const
{
    UiRect rcMonitor;
    return GetMonitorRect(m_hWnd, rcMonitor, rcWork);
}

bool NativeWindow_Windows::GetPrimaryMonitorWorkRect(UiRect& rcWork)
{
    rcWork.Clear();
    HMONITOR hMonitor = ::MonitorFromPoint({ INT32_MIN, INT32_MIN }, MONITOR_DEFAULTTOPRIMARY);
    ASSERT(hMonitor != nullptr);
    if (hMonitor == nullptr) {
        return false;
    }
    MONITORINFO oMonitor = { 0, };
    oMonitor.cbSize = sizeof(oMonitor);
    if (::GetMonitorInfo(hMonitor, &oMonitor)) {
        rcWork = UiRect(oMonitor.rcWork.left, oMonitor.rcWork.top,
                        oMonitor.rcWork.right, oMonitor.rcWork.bottom);
        return true;
    }
    else {
        ASSERT(!"NativeWindow_Windows::GetPrimaryMonitorWorkRect failed!");
        return false;
    }
}

bool NativeWindow_Windows::GetMonitorWorkRect(const UiPoint& pt, UiRect& rcWork) const
{
    rcWork.Clear();
    HMONITOR hMonitor = ::MonitorFromPoint({ pt.x, pt.y }, MONITOR_DEFAULTTONEAREST);
    ASSERT(hMonitor != nullptr);
    if (hMonitor == nullptr) {
        return false;
    }
    MONITORINFO oMonitor = { 0, };
    oMonitor.cbSize = sizeof(oMonitor);
    if (::GetMonitorInfo(hMonitor, &oMonitor)) {
        rcWork = UiRect(oMonitor.rcWork.left, oMonitor.rcWork.top,
                        oMonitor.rcWork.right, oMonitor.rcWork.bottom);
        return true;
    }
    else {
        ASSERT(!"NativeWindow_Windows::GetMonitorWorkRect failed!");
        return false;
    }
}

const UiPoint& NativeWindow_Windows::GetLastMousePos() const
{
    return m_ptLastMousePos;
}

void NativeWindow_Windows::SetLastMousePos(const UiPoint& pt)
{
    m_ptLastMousePos = pt;
}

bool NativeWindow_Windows::GetModifiers(UINT message, WPARAM wParam, LPARAM lParam, uint32_t& modifierKey) const
{
    //逻辑修改，需要同步给函数：Window::IsKeyDown
    bool bRet = true;
    modifierKey = ModifierKey::kNone;
    switch (message) {
    case WM_SYSCHAR:
    case WM_CHAR:
        if (0 == (lParam & (1 << 30))) {
            modifierKey |= ModifierKey::kFirstPress;
        }
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (0 == (lParam & (1 << 30))) {
            modifierKey |= ModifierKey::kFirstPress;
        }
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (lParam & (1 << 29)) {
            modifierKey |= ModifierKey::kAlt;
        }
        break;
    case WM_MOUSEWHEEL:
        {
            WORD fwKeys = GET_KEYSTATE_WPARAM(wParam);
            if (fwKeys & MK_CONTROL) {
                modifierKey |= ModifierKey::kControl;
            }
            if (fwKeys & MK_SHIFT) {
                modifierKey |= ModifierKey::kShift;
            }
        }
        break;
    case WM_MOUSEHOVER:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        if (wParam & MK_CONTROL) {
            modifierKey |= ModifierKey::kControl;
        }
        if (wParam & MK_SHIFT) {
            modifierKey |= ModifierKey::kShift;
        }
        break;
    case WM_HOTKEY:
        {
            uint16_t nMod = (uint16_t)LOWORD(lParam);
            if (nMod & MOD_ALT) {
                modifierKey |= ModifierKey::kAlt;
            }
            else if (nMod & MOD_CONTROL) {
                modifierKey |= ModifierKey::kControl;
            }
            else if (nMod & MOD_SHIFT) {
                modifierKey |= ModifierKey::kShift;
            }
            else if (nMod & MOD_WIN) {
                modifierKey |= ModifierKey::kWin;
            }
        }
        break;
    default:
        bRet = false;
        break;
    }
    if ((message == WM_SYSCHAR) || (message == WM_SYSKEYDOWN) || (message == WM_SYSKEYUP)) {
        modifierKey |= ModifierKey::kIsSystemKey;
    }
    ASSERT(bRet);
    return bRet;
}


int32_t NativeWindow_Windows::SetWindowHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers)
{
    ASSERT(IsWindow());
    return (int32_t)::SendMessage(GetHWND(), WM_SETHOTKEY, MAKEWORD(wVirtualKeyCode, wModifiers), 0);
}

bool NativeWindow_Windows::GetWindowHotKey(uint8_t& wVirtualKeyCode, uint8_t& wModifiers) const
{
    ASSERT(IsWindow());
    DWORD dw = (DWORD)::SendMessage(GetHWND(), HKM_GETHOTKEY, 0, 0L);
    wVirtualKeyCode = LOBYTE(LOWORD(dw));
    wModifiers = HIBYTE(LOWORD(dw));
    return dw != 0;
}

bool NativeWindow_Windows::RegisterHotKey(uint8_t wVirtualKeyCode, uint8_t wModifiers, int32_t id)
{
    ASSERT(IsWindow());
    if (wVirtualKeyCode != 0) {
        UINT fsModifiers = 0;
        if (wModifiers & HOTKEYF_ALT)     fsModifiers |= MOD_ALT;
        if (wModifiers & HOTKEYF_CONTROL) fsModifiers |= MOD_CONTROL;
        if (wModifiers & HOTKEYF_SHIFT)   fsModifiers |= MOD_SHIFT;
        if (wModifiers & HOTKEYF_EXT)     fsModifiers |= MOD_WIN;

#ifndef MOD_NOREPEAT
        if (::IsWindows7OrGreater()) {
            fsModifiers |= 0x4000;
        }
#else
        fsModifiers |= MOD_NOREPEAT;
#endif

        LRESULT lResult = ::RegisterHotKey(this->GetHWND(), id, fsModifiers, wVirtualKeyCode);
        ASSERT(lResult != 0);
        if (lResult != 0) {
            auto iter = std::find(m_hotKeyIds.begin(), m_hotKeyIds.end(), id);
            if (iter != m_hotKeyIds.end()) {
                m_hotKeyIds.erase(iter);
            }
            m_hotKeyIds.push_back(id);
            return true;
        }
    }
    return false;
}

bool NativeWindow_Windows::UnregisterHotKey(int32_t id)
{
    ASSERT(IsWindow());
    auto iter = std::find(m_hotKeyIds.begin(), m_hotKeyIds.end(), id);
    if (iter != m_hotKeyIds.end()) {
        m_hotKeyIds.erase(iter);
    }
    return ::UnregisterHotKey(GetHWND(), id);
}

/** 窗口句柄的属性名称
*/
static const DStringW::value_type* sPropName  = L"DuiLibWindow";     // 属性名称(校验指针)
static const DStringW::value_type* sPropName2 = L"DuiLibWindow2";    // 属性名称(进程ID)

LRESULT CALLBACK NativeWindow_Windows::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NativeWindow_Windows* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = static_cast<NativeWindow_Windows*>(lpcs->lpCreateParams);
        if (pThis != nullptr) {
            pThis->m_hWnd = hWnd;
        }
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
        ::SetPropW(hWnd, sPropName, (HANDLE)pThis);
        ::SetPropW(hWnd, sPropName2, (HANDLE)(size_t)::GetCurrentProcessId());
    }
    else {
        pThis = reinterpret_cast<NativeWindow_Windows*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
#ifdef _DEBUG
        //校验是否一致
        ASSERT((NativeWindow_Windows*)::GetPropW(hWnd, sPropName) == pThis);
        if (pThis != nullptr) {
            ASSERT(::GetPropW(hWnd, sPropName2) == (HANDLE)(size_t)::GetCurrentProcessId());
        }        
#endif
        if (uMsg == WM_NCDESTROY && pThis != nullptr) {            
            LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
            ::SetPropW(hWnd, sPropName, nullptr);
            ::SetPropW(hWnd, sPropName2, nullptr);
            ASSERT(hWnd == pThis->GetHWND());
            pThis->OnFinalMessage();
            return lRes;
        }
    }

    if (pThis != nullptr) {
        ASSERT(hWnd == pThis->GetHWND());
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    }
    else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}


INT_PTR CALLBACK NativeWindow_Windows::__DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG) {
        //这是对话框的第一个消息
        NativeWindow_Windows* pThis = reinterpret_cast<NativeWindow_Windows*>(lParam);
        ASSERT(pThis != nullptr);
        if (pThis != nullptr) {
            ASSERT(pThis->m_hWnd == nullptr);
            pThis->m_hWnd = hWnd;            
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
            ::SetPropW(hWnd, sPropName, (HANDLE)pThis);
            ::SetPropW(hWnd, sPropName2, (HANDLE)(size_t)::GetCurrentProcessId());

            ASSERT(pThis->m_pfnOldWndProc == nullptr);

            //替换对话框的窗口处理函数
            pThis->m_pfnOldWndProc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);
            ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LPARAM)(WNDPROC)NativeWindow_Windows::__DialogWndProc);

            //派发此消息
            pThis->WindowMessageProc(uMsg, wParam, lParam);
            ::SetFocus(hWnd);
            return TRUE;
        }
    }
    else if (uMsg == WM_COMMAND) {
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            NativeWindow_Windows* pThis = reinterpret_cast<NativeWindow_Windows*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
#ifdef _DEBUG
            //校验是否一致
            ASSERT((NativeWindow_Windows*)::GetPropW(hWnd, sPropName) == pThis);
            if (pThis != nullptr) {
                ASSERT(::GetPropW(hWnd, sPropName2) == (HANDLE)(size_t)::GetCurrentProcessId());
            }
#endif
            ASSERT(pThis != nullptr);
            if (pThis != nullptr) {
                if (pThis->m_bCloseByEsc && (LOWORD(wParam) == IDCANCEL)) {
                    pThis->CloseWnd(kWindowCloseCancel);
                }
                else if (pThis->m_bCloseByEnter && (LOWORD(wParam) == IDOK)) {
                    pThis->CloseWnd(kWindowCloseOK);
                }
            }
            return TRUE;
        }
    }    
    return FALSE;
}

LRESULT CALLBACK NativeWindow_Windows::__DialogWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NativeWindow_Windows* pThis = reinterpret_cast<NativeWindow_Windows*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
#ifdef _DEBUG
    //校验是否一致
    ASSERT((NativeWindow_Windows*)::GetPropW(hWnd, sPropName) == pThis);
    if (pThis != nullptr) {
        ASSERT(::GetPropW(hWnd, sPropName2) == (HANDLE)(size_t)::GetCurrentProcessId());
    }
#endif
    ASSERT(pThis != nullptr);
    if (uMsg == WM_NCDESTROY && pThis != nullptr) {
        if (pThis->m_pfnOldWndProc != nullptr) {
            ::SetWindowLongPtr((hWnd), GWLP_WNDPROC, (LPARAM)(WNDPROC)(pThis->m_pfnOldWndProc));
            pThis->m_pfnOldWndProc = nullptr;
        }
        LRESULT lRes = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
        ::SetPropW(hWnd, sPropName, nullptr);
        ::SetPropW(hWnd, sPropName2, nullptr);
        ASSERT(hWnd == pThis->GetHWND());
        pThis->OnFinalMessage();
        return lRes;
    }
    else if (pThis != nullptr) {
        ASSERT(hWnd == pThis->GetHWND());
        return pThis->WindowMessageProc(uMsg, wParam, lParam);
    }
    return ::DefDlgProc(hWnd, uMsg, wParam, lParam);
}

LRESULT NativeWindow_Windows::WindowMessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;
    HWND hWnd = m_hWnd;
    INativeWindow* pOwner = m_pOwner;
    if (pOwner == nullptr) {
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
        return lResult;
    }
    //接口的生命周期标志
    std::weak_ptr<WeakFlag> ownerFlag = pOwner->GetWeakFlag();

    //消息首先转给过滤器(全部消息)
    bool bHandled = false;
    if (!bHandled && !ownerFlag.expired()) {
        lResult = pOwner->OnNativeWindowMessage(uMsg, wParam, lParam, bHandled);
    }

    if (m_bDoModal && (uMsg == WM_KEYDOWN) && (wParam == VK_ESCAPE)) {
        //模态对话框，按ESC键时，关闭
        CloseWnd(kWindowCloseCancel);
        return 0;
    }

    //第三优先级：内部处理的消息，处理后，不再派发
    if (!bHandled && !ownerFlag.expired()) {
        lResult = ProcessInternalMessage(uMsg, wParam, lParam, bHandled);
    }

    //第四优先级：内部处理函数，优先保证自身功能正常
    if (!bHandled && !ownerFlag.expired()) {
        lResult = ProcessWindowMessage(uMsg, wParam, lParam, bHandled);
    }

    const bool bWindowCloseMsg = (uMsg == WM_CLOSE) || ((uMsg == WM_SYSCOMMAND) && (GET_SC_WPARAM(wParam) == SC_CLOSE));
    bool bWindowClosed = false;
    if (!bHandled && bWindowCloseMsg && !ownerFlag.expired()) {
        //窗口即将关闭（关闭前）
        StopSysMenuTimer();

        //保持关闭窗口的退出参数
        if (uMsg == WM_CLOSE) {
            m_closeParam = (int32_t)wParam;
        }

        bWindowClosed = true;
        pOwner->OnNativePreCloseWindow();
    }
    else if (bHandled && bWindowCloseMsg && !ownerFlag.expired()) {
        //恢复关闭前的状态
        m_bCloseing = false;
        m_closeParam = kWindowCloseNormal;
    }

    //第五优先级：系统默认的窗口函数
    if (!bHandled && !ownerFlag.expired() && ::IsWindow(hWnd)) {
        if (bWindowClosed && m_bDoModal) {
            //模态对话框
            ::EndDialog(hWnd, wParam);
            lResult = 0;
            bHandled = true;
        }
        else {
            lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
        }        
    }

    if (!bHandled && !ownerFlag.expired() && (uMsg == WM_DESTROY)) {
        //窗口已经关闭（关闭后）
        pOwner->OnNativePostCloseWindow();
    }
    return lResult;
}

LRESULT NativeWindow_Windows::CallDefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (m_pfnOldWndProc != nullptr) {
        return ::CallWindowProc(m_pfnOldWndProc, m_hWnd, uMsg, wParam, lParam);
    }
    return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

LRESULT NativeWindow_Windows::ProcessInternalMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    bool bInternalMsg = true;
    switch (uMsg)
    {
    case WM_NCACTIVATE:         lResult = OnNcActivateMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCCALCSIZE:         lResult = OnNcCalcSizeMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_NCHITTEST:          lResult = OnNcHitTestMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_GETMINMAXINFO:      lResult = OnGetMinMaxInfoMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_ERASEBKGND:         lResult = OnEraseBkGndMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_DPICHANGED:         lResult = OnDpiChangedMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_WINDOWPOSCHANGING:  lResult = OnWindowPosChangingMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_NOTIFY:             lResult = OnNotifyMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_COMMAND:            lResult = OnCommandMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLOREDIT:       lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled); break;
    case WM_CTLCOLORSTATIC:     lResult = OnCtlColorMsgs(uMsg, wParam, lParam, bHandled); break;
    case WM_TOUCH:              lResult = OnTouchMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_POINTERDOWN:
    case WM_POINTERUP:
    case WM_POINTERUPDATE:
    case WM_POINTERLEAVE:
    case WM_POINTERWHEEL:
    case WM_POINTERCAPTURECHANGED:
        lResult = OnPointerMsgs(uMsg, wParam, lParam, bHandled);
        break;

    case WM_CREATE:     lResult = OnCreateMsg(uMsg, wParam, lParam, bHandled); break;
    case WM_INITDIALOG: lResult = OnInitDialogMsg(uMsg, wParam, lParam, bHandled); break;

    case WM_TIMER:
        {
            if (wParam == m_nSysMenuTimerId) {
                //系统菜单延迟显示的定时器触发
                ::KillTimer(m_hWnd, m_nSysMenuTimerId);
                m_nSysMenuTimerId = 0;

                POINT pt;
                ::GetCursorPos(&pt);
                ShowWindowSysMenu(m_hWnd, pt);
                bHandled = true;
            }
        }
        break;
    default:
        bInternalMsg = false;
        break;
    }//end of switch

    if (bInternalMsg && !bHandled) {
        //调用窗口函数，然后不再继续传递此消息
        bHandled = true;
        lResult = CallDefaultWindowProc(uMsg, wParam, lParam);
    }
    return lResult;
}

INativeWindow* NativeWindow_Windows::WindowBaseFromPoint(const UiPoint& pt, bool bIgnoreChildWindow)
{
    NativeWindow_Windows* pWindow = nullptr;
    HWND hWndPt = ::WindowFromPoint({ pt.x, pt.y });
    std::vector<HWND> hwndList;
    if (::IsWindow(hWndPt)) {
        hwndList.push_back(hWndPt);
        if (bIgnoreChildWindow) {
            //获取父窗口列表
            HWND hParent = ::GetParent(hWndPt);
            while ((hParent != nullptr) && ::IsWindow(hParent)) {
                hwndList.push_back(hParent);
                hParent = ::GetParent(hParent);
            }
        }
    }

    for (HWND hWnd : hwndList) {
        if (hWnd == m_hWnd) {
            pWindow = this;
        }
        else {
            pWindow = reinterpret_cast<NativeWindow_Windows*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if (pWindow != nullptr) {
                if (::GetPropW(hWnd, sPropName2) != (HANDLE)(size_t)::GetCurrentProcessId()) {
                    //校验失败：不是duilib的窗口
                    pWindow = nullptr;
                }
                else if ((NativeWindow_Windows*)::GetPropW(hWnd, sPropName) != pWindow) {
                    //校验失败：不是duilib的窗口
                    pWindow = nullptr;
                }
                else if (pWindow->m_hWnd != hWnd) {
                    pWindow = nullptr;
                }
            }
        }
        if (pWindow != nullptr) {
            break;
        }
    }
    INativeWindow* pNativeWindow = nullptr;
    if (pWindow != nullptr) {
        pNativeWindow = pWindow->m_pOwner;
    }
    return pNativeWindow;
}

LRESULT NativeWindow_Windows::OnNcActivateMsg(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCACTIVATE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    LRESULT lResult = 0;
    if (IsWindowMinimized()) {
        bHandled = false;
    }
    else {
        //MSDN: wParam 参数为 FALSE 时，应用程序应返回 TRUE 以指示系统应继续执行默认处理
        bHandled = true;
        lResult = (wParam == FALSE) ? TRUE : FALSE;
    }
    return lResult;
}

LRESULT NativeWindow_Windows::OnNcCalcSizeMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCCALCSIZE);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    //截获，让系统不处理此消息
    bHandled = true;
    return 0;
}

LRESULT NativeWindow_Windows::OnNcHitTestMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NCHITTEST);
    if (IsUseSystemCaption()) {
        bHandled = false;
        return 0;
    }

    bHandled = true;
    UiPoint pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    ScreenToClient(pt);

    UiRect rcClient;
    GetClientRect(rcClient);

    //客户区域，排除掉阴影部分区域
    UiPadding rcCorner;
    m_pOwner->OnNativeGetShadowCorner(rcCorner);
    rcClient.Deflate(rcCorner);

    if (!IsWindowMaximized()) {
        //非最大化状态
        UiRect rcSizeBox = m_pOwner->OnNativeGetSizeBox();
        if (pt.y < rcClient.top + rcSizeBox.top) {
            if (pt.y >= rcClient.top) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTTOPLEFT;//在窗口边框的左上角。
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTTOPRIGHT;//在窗口边框的右上角
                }
                else {
                    return HTTOP;//在窗口的上水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
            if (pt.y <= rcClient.bottom) {
                if (pt.x < (rcClient.left + rcSizeBox.left) && pt.x >= rcClient.left) {
                    return HTBOTTOMLEFT;//在窗口边框的左下角
                }
                else if (pt.x > (rcClient.right - rcSizeBox.right) && pt.x <= rcClient.right) {
                    return HTBOTTOMRIGHT;//在窗口边框的右下角
                }
                else {
                    return HTBOTTOM;//在窗口的下水平边框中
                }
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }

        if (pt.x < rcClient.left + rcSizeBox.left) {
            if (pt.x >= rcClient.left) {
                return HTLEFT;//在窗口的左边框
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
        if (pt.x > rcClient.right - rcSizeBox.right) {
            if (pt.x <= rcClient.right) {
                return HTRIGHT;//在窗口的右边框中
            }
            else {
                return HTCLIENT;//在工作区中
            }
        }
    }

    UiRect rcCaption;
    m_pOwner->OnNativeGetCaptionRect(rcCaption);
    //标题栏区域的矩形范围
    UiRect rcCaptionRect;
    rcCaptionRect.left = rcClient.left + rcCaption.left;
    rcCaptionRect.right = rcClient.right - rcCaption.right;
    rcCaptionRect.top = rcClient.top + rcCaption.top;
    rcCaptionRect.bottom = rcClient.top + rcCaption.bottom;
    if (rcCaptionRect.ContainsPt(pt)) {
        //在标题栏范围内
        UiRect sysMenuRect;
        m_pOwner->OnNativeGetSysMenuRect(sysMenuRect);
        sysMenuRect.Offset(rcClient.left, rcClient.top);
        sysMenuRect.Intersect(rcCaptionRect);
        if (!sysMenuRect.IsEmpty()) {
            if (sysMenuRect.ContainsPt(pt)) {                
                return HTSYSMENU;//在系统菜单矩形区域内
            }
        }

        //是否支持显示贴靠布局菜单
        bool bPtInMaximizeRestoreButton = false;        
        if (IsEnableSnapLayoutMenu()) {
            bPtInMaximizeRestoreButton = m_pOwner->OnNativeIsPtInMaximizeRestoreButton(pt);
        }
        if (bPtInMaximizeRestoreButton) {
            return HTMAXBUTTON; //在最大化按钮或者还原按钮上，显示贴靠布局菜单
        }
        else if (m_pOwner->OnNativeIsPtInCaptionBarControl(pt)) {
            return HTCLIENT;//在工作区中（放在标题栏上的控件，视为工作区）
        }
        else {
            return HTCAPTION;//在标题栏中
        }
    }
    //其他，在工作区中
    return HTCLIENT;
}

LRESULT NativeWindow_Windows::OnGetMinMaxInfoMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_GETMINMAXINFO);
    bHandled = false;
    LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
    UiRect rcWork;
    UiRect rcMonitor;
    GetMonitorRect(m_hWnd, rcMonitor, rcWork);
    rcWork.Offset(-rcMonitor.left, -rcMonitor.top);

    //最大化时，默认设置为当前屏幕的最大区域
    lpMMI->ptMaxPosition.x = rcWork.left;
    lpMMI->ptMaxPosition.y = rcWork.top;
    lpMMI->ptMaxSize.x = rcWork.Width();
    lpMMI->ptMaxSize.y = rcWork.Height();

    if (GetWindowMaximumSize().cx != 0) {
        lpMMI->ptMaxTrackSize.x = GetWindowMaximumSize().cx;
    }
    if (GetWindowMaximumSize().cy != 0) {
        lpMMI->ptMaxTrackSize.y = GetWindowMaximumSize().cy;
    }
    if (GetWindowMinimumSize().cx != 0) {
        lpMMI->ptMinTrackSize.x = GetWindowMinimumSize().cx;
    }
    if (GetWindowMinimumSize().cy != 0) {
        lpMMI->ptMinTrackSize.y = GetWindowMinimumSize().cy;
    }
    return 0;
}

LRESULT NativeWindow_Windows::OnEraseBkGndMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_ERASEBKGND);
    bHandled = true;
    return 1;
}

LRESULT NativeWindow_Windows::OnDpiChangedMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_DPICHANGED);
    bHandled = false;//需要重新测试

    uint32_t nNewDPI = HIWORD(wParam);
    UiRect rcNewWindow;
    const RECT* prcNewWindow = (RECT*)lParam;
    if (prcNewWindow != nullptr) {
        rcNewWindow.left = prcNewWindow->left;
        rcNewWindow.top = prcNewWindow->top;
        rcNewWindow.right = prcNewWindow->right;
        rcNewWindow.bottom = prcNewWindow->bottom;
    }
    uint32_t nOldDpiScale = m_pOwner->OnNativeGetDpi().GetScale();
    m_pOwner->OnNativeProcessDpiChangedMsg(nNewDPI, rcNewWindow);
    if (nOldDpiScale != m_pOwner->OnNativeGetDpi().GetScale()) {
        m_ptLastMousePos = m_pOwner->OnNativeGetDpi().GetScalePoint(m_ptLastMousePos, nOldDpiScale);
    }
    //更新窗口的位置和大小
    if (!rcNewWindow.IsEmpty()) {
        SetWindowPos(nullptr, InsertAfterFlag::kHWND_DEFAULT,
                     rcNewWindow.left, rcNewWindow.top, rcNewWindow.Width(), rcNewWindow.Height(),
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
    return 0;
}

LRESULT NativeWindow_Windows::OnWindowPosChangingMsg(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_WINDOWPOSCHANGING);
    bHandled = false;
    if (IsWindowMaximized()) {
        //最大化状态
        LPWINDOWPOS lpPos = (LPWINDOWPOS)lParam;
        if (lpPos->flags & SWP_FRAMECHANGED) // 第一次最大化，而不是最大化之后所触发的WINDOWPOSCHANGE
        {
            POINT pt = { 0, 0 };
            HMONITOR hMontorPrimary = ::MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
            HMONITOR hMonitorTo = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);

            // 先把窗口最大化，再最小化，然后恢复，此时MonitorFromWindow拿到的HMONITOR不准确
            // 判断GetWindowRect的位置如果不正确（最小化时得到的位置信息是-38000），则改用normal状态下的位置，来获取HMONITOR
            UiRect rc;
            GetWindowRect(rc);
            if (rc.left < -10000 && rc.top < -10000 && rc.bottom < -10000 && rc.right < -10000) {
                WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
                ::GetWindowPlacement(m_hWnd, &wp);
                hMonitorTo = ::MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTOPRIMARY);
            }
            if (hMonitorTo != hMontorPrimary) {
                // 解决无边框窗口在双屏下面（副屏分辨率大于主屏）时，最大化不正确的问题
                MONITORINFO  miTo;
                ZeroMemory(&miTo, sizeof(miTo));
                miTo.cbSize = sizeof(MONITORINFO);
                ::GetMonitorInfo(hMonitorTo, &miTo);

                lpPos->x = miTo.rcWork.left;
                lpPos->y = miTo.rcWork.top;
                lpPos->cx = miTo.rcWork.right - miTo.rcWork.left;
                lpPos->cy = miTo.rcWork.bottom - miTo.rcWork.top;
            }
        }
    }
    return 0;
}

LRESULT NativeWindow_Windows::OnNotifyMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_NOTIFY);
    bHandled = false;
    LPNMHDR lpNMHDR = (LPNMHDR)lParam;
    if (lpNMHDR != nullptr) {
        bHandled = true;
        return ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT NativeWindow_Windows::OnCommandMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_COMMAND);
    bHandled = false;
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT NativeWindow_Windows::OnCtlColorMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_CTLCOLOREDIT || uMsg == WM_CTLCOLORSTATIC);
    bHandled = false;
    // Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
    // Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
    if (lParam == 0) {
        return 0;
    }
    HWND hWndChild = (HWND)lParam;
    bHandled = true;
    return ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
}

LRESULT NativeWindow_Windows::OnTouchMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE(uMsg == WM_TOUCH);
    LRESULT lResult = 0;
    bHandled = false;
    unsigned int nNumInputs = LOWORD(wParam);
    if (nNumInputs < 1) {
        nNumInputs = 1;
    }
    TOUCHINPUT* pInputs = new TOUCHINPUT[nNumInputs];
    // 只关心第一个触摸位置
    if (!GetTouchInputInfoWrapper((HTOUCHINPUT)lParam, nNumInputs, pInputs, sizeof(TOUCHINPUT))) {
        delete[] pInputs;
        return lResult;
    }
    else {
        CloseTouchInputHandleWrapper((HTOUCHINPUT)lParam);
        if (pInputs[0].dwID == 0) {
            return lResult;
        }
    }
    //获取触摸点的坐标，并转换为窗口内的客户区坐标
    UiPoint pt = { TOUCH_COORD_TO_PIXEL(pInputs[0].x) , TOUCH_COORD_TO_PIXEL(pInputs[0].y) };
    ScreenToClient(pt);

    DWORD dwFlags = pInputs[0].dwFlags;
    delete[] pInputs;
    pInputs = nullptr;

    if (dwFlags & TOUCHEVENTF_DOWN) {        
        lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, 0, NativeMsg(WM_LBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
    }
    else if (dwFlags & TOUCHEVENTF_MOVE) {
        UiPoint lastMousePos = m_ptLastMousePos;
        lResult = m_pOwner->OnNativeMouseMoveMsg(pt, 0, false, NativeMsg(WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
        int wheelDelta = pt.y - lastMousePos.y;
        if (wheelDelta != 0) {
            //触发滚轮功能
            lResult = m_pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, 0, NativeMsg(WM_MOUSEWHEEL, MAKEWPARAM(0, wheelDelta), MAKELPARAM(pt.x, pt.y)), bHandled);
        }
    }
    else if (dwFlags & TOUCHEVENTF_UP) {
        lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, 0, NativeMsg(WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
    }
    return lResult;
}

LRESULT NativeWindow_Windows::OnPointerMsgs(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    ASSERT_UNUSED_VARIABLE( uMsg == WM_POINTERDOWN ||
                            uMsg == WM_POINTERUP ||
                            uMsg == WM_POINTERUPDATE ||
                            uMsg == WM_POINTERLEAVE ||
                            uMsg == WM_POINTERCAPTURECHANGED ||
                            uMsg == WM_POINTERWHEEL);

    LRESULT lResult = 0;
    bHandled = false;
    // 只关心第一个触摸点
    if (!IS_POINTER_PRIMARY_WPARAM(wParam)) {
        bHandled = true;
        return lResult;
    }
    //获取指针位置，并且将屏幕坐标转换为窗口客户区坐标
    UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    ScreenToClient(pt);
    switch (uMsg)
    {
    case WM_POINTERDOWN:
        lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, 0, NativeMsg(WM_LBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
        bHandled = true;
        break;
    case WM_POINTERUPDATE:
        lResult = m_pOwner->OnNativeMouseMoveMsg(pt, 0, false, NativeMsg(WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
        bHandled = true;
        break;
    case WM_POINTERUP:
        lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, 0, NativeMsg(WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y)), bHandled);
        bHandled = true;
        break;
    case WM_POINTERWHEEL:
    {
        int32_t wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        lResult = m_pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, 0, NativeMsg(WM_MOUSEWHEEL, MAKEWPARAM(0, wheelDelta), MAKELPARAM(pt.x, pt.y)), bHandled);
        bHandled = true;
    }
    break;
    case WM_POINTERLEAVE:
        lResult = m_pOwner->OnNativeMouseLeaveMsg(NativeMsg(WM_MOUSELEAVE, 0, 0), bHandled);
        break;
    case WM_POINTERCAPTURECHANGED:
        lResult = m_pOwner->OnNativeCaptureChangedMsg(NativeMsg(WM_CAPTURECHANGED, 0, 0), bHandled);
        //如果不设置bHandled，程序会转换为WM_BUTTON类消息
        bHandled = true;
        break;
    default:
        break;
    }
    return 0;
}

void NativeWindow_Windows::CheckWindowSnap(HWND hWnd)
{
    if (::IsZoomed(hWnd) || ::IsIconic(hWnd)) {
        //最大化或者最小化时，不处理
        return;
    }
    // 获取窗口所在显示器信息
    HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { 0, };
    mi.cbSize = sizeof(MONITORINFO);
    ::GetMonitorInfo(hMonitor, &mi);

    // DPI适配
    UINT dpi = 96;
    GetDpiForWindowWrapper(hWnd, dpi);
    const int32_t snapThreshold = MulDiv(3, dpi, 96);

    // 获取窗口实际坐标
    RECT rcWindow = { 0 };
    ::GetWindowRect(hWnd, &rcWindow);

    // 贴边检测逻辑
    bool bLeftSnap = (std::abs(rcWindow.left - mi.rcWork.left) < snapThreshold);
    bool bRightSnap = (std::abs(rcWindow.right - mi.rcWork.right) < snapThreshold);
    bool bTopSnap = (std::abs(rcWindow.top - mi.rcWork.top) < snapThreshold);
    bool bBottomSnap = (std::abs(rcWindow.bottom - mi.rcWork.bottom) < snapThreshold);

    m_pOwner->OnNativeWindowPosSnapped(bLeftSnap, bRightSnap, bTopSnap, bBottomSnap);
}

LRESULT NativeWindow_Windows::ProcessWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    LRESULT lResult = 0;
    bHandled = false;
    switch (uMsg)
    {
    case WM_WINDOWPOSCHANGED:
        CheckWindowSnap(m_hWnd);
        break;
    case WM_SIZE:
    {        
        WindowSizeType sizeType = static_cast<WindowSizeType>(wParam);
        UiSize newWindowSize;
        newWindowSize.cx = (int)(short)LOWORD(lParam);
        newWindowSize.cy = (int)(short)HIWORD(lParam);
        lResult = m_pOwner->OnNativeSizeMsg(sizeType, newWindowSize, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOVE:
    {
        UiPoint ptTopLeft;
        ptTopLeft.x = (int)(short)LOWORD(lParam);   // horizontal position 
        ptTopLeft.y = (int)(short)HIWORD(lParam);   // vertical position 
        lResult = m_pOwner->OnNativeMoveMsg(ptTopLeft, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_SHOWWINDOW:
    {
        bool bShow = wParam != FALSE;
        lResult = m_pOwner->OnNativeShowWindowMsg(bShow, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_PAINT:
    {
        lResult = OnPaintMsg(uMsg, wParam, lParam, bHandled);
        break;
    }
    case WM_SETFOCUS:
    {
        INativeWindow* pLostFocusWindow = nullptr;
        HWND hWnd = (HWND)wParam;
        if ((hWnd != nullptr) && ::IsWindow(hWnd)) {
            NativeWindow_Windows* pThis = reinterpret_cast<NativeWindow_Windows*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            //校验是否一致
            if (pThis != nullptr) {
                if ((NativeWindow_Windows*)::GetPropW(hWnd, sPropName) == pThis) {
                    pLostFocusWindow = pThis->m_pOwner;
                }
            }
        }
        lResult = m_pOwner->OnNativeSetFocusMsg(pLostFocusWindow, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KILLFOCUS:
    {
        INativeWindow* pSetFocusWindow = nullptr;
        HWND hWnd = (HWND)wParam;
        if ((hWnd != nullptr) && ::IsWindow(hWnd)) {
            NativeWindow_Windows* pThis = reinterpret_cast<NativeWindow_Windows*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            //校验是否一致
            if (pThis != nullptr) {
                if ((NativeWindow_Windows*)::GetPropW(hWnd, sPropName) == pThis) {
                    pSetFocusWindow = pThis->m_pOwner;
                }
            }
        }
        lResult = m_pOwner->OnNativeKillFocusMsg(pSetFocusWindow, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_IME_SETCONTEXT:
    {
        lResult = m_pOwner->OnNativeImeSetContextMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_IME_STARTCOMPOSITION:
    {
        lResult = m_pOwner->OnNativeImeStartCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_IME_COMPOSITION:
    {
        lResult = m_pOwner->OnNativeImeCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_IME_ENDCOMPOSITION:
    {
        lResult = m_pOwner->OnNativeImeEndCompositionMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT) {
            //只处理设置客户区的光标
            lResult = m_pOwner->OnNativeSetCursorMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        break;
    }
    case WM_CONTEXTMENU:
    {
        UiPoint pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if ((pt.x != -1) && (pt.y != -1)) {
            ScreenToClient(pt);
        }
        lResult = m_pOwner->OnNativeContextMenuMsg(pt, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeKeyDownMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeKeyUpMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_CHAR:
    case WM_SYSCHAR:
    {
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>(wParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeCharMsg(vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_HOTKEY:
    {
        int32_t hotkeyId = (int32_t)wParam;
        VirtualKeyCode vkCode = static_cast<VirtualKeyCode>((int32_t)(int16_t)HIWORD(lParam));
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeHotKeyMsg(hotkeyId, vkCode, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEWHEEL:
    {
        int32_t wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(pt);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseWheelMsg(wheelDelta, pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEMOVE:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseMoveMsg(pt, modifierKey, false, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSEHOVER:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseHoverMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MOUSELEAVE:
    {
        lResult = m_pOwner->OnNativeMouseLeaveMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        m_bNCLButtonDownOnMaxButton = false;
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONUP:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_LBUTTONDBLCLK:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseLButtonDbClickMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONDOWN:
    {
        m_bNCLButtonDownOnMaxButton = false;
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONUP:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_RBUTTONDBLCLK:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseRButtonDbClickMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MBUTTONDOWN:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseMButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MBUTTONUP:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseMButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_MBUTTONDBLCLK:
    {
        UiPoint pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        uint32_t modifierKey = 0;
        GetModifiers(uMsg, wParam, lParam, modifierKey);
        lResult = m_pOwner->OnNativeMouseMButtonDbClickMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_CAPTURECHANGED:
    {
        HWND hWnd = m_hWnd;
        lResult = m_pOwner->OnNativeCaptureChangedMsg(NativeMsg(uMsg, wParam, lParam), bHandled);
        if (::IsWindow(hWnd) && m_bNCLButtonDownOnMaxButton && !IsUseSystemCaption()) {
            //需要释放捕获鼠标，否则点击最大按钮时无法正常响应
            if (::GetCapture() == m_hWnd) {
                m_bNCLButtonDownOnMaxButton = false;
                ::ReleaseCapture();
            }
        }
        break;
    }
    case WM_CLOSE:
    {
        lResult = m_pOwner->OnNativeWindowCloseMsg((uint32_t)wParam, NativeMsg(uMsg, wParam, lParam), bHandled);
        break;
    }
    case WM_NCMOUSEMOVE:
    {
        if (!IsUseSystemCaption()) {
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(pt);
            uint32_t modifierKey = 0;
            lResult = m_pOwner->OnNativeMouseMoveMsg(pt, modifierKey, true, NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        break;
    }
    case WM_NCLBUTTONDOWN:
    {
        if (!IsUseSystemCaption() && (wParam == HTMAXBUTTON)) {
            //如果鼠标点击在最大化按钮上，截获此消息，避免Windows也触发最大化/还原命令
            m_bNCLButtonDownOnMaxButton = true;
            bHandled = true; 
            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(pt);
            uint32_t modifierKey = 0;
            lResult = m_pOwner->OnNativeMouseLButtonDownMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        else if (!IsUseSystemCaption() && (wParam == HTSYSMENU) && IsEnableSysMenu()) {
            //鼠标点击在窗口菜单位置，启动定时器，延迟显示系统的窗口菜单
            StopSysMenuTimer();            
            m_nSysMenuTimerId = ::SetTimer(m_hWnd, UI_SYS_MEMU_TIMER_ID, 300, nullptr);
        }
        break;
    }
    case WM_NCLBUTTONUP:
    {
        m_bNCLButtonDownOnMaxButton = false;
        if (!IsUseSystemCaption() && (wParam == HTMAXBUTTON)) {
            //如果鼠标点击在最大化按钮上，截获此消息，避免Windows也触发最大化/还原命令
            bHandled = true;

            UiPoint pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(pt);
            uint32_t modifierKey = 0;
            lResult = m_pOwner->OnNativeMouseLButtonUpMsg(pt, modifierKey, NativeMsg(uMsg, wParam, lParam), bHandled);
        }
        break;
    }
    case WM_NCRBUTTONUP:
    {
        m_bNCLButtonDownOnMaxButton = false;
        bool bEnable = (wParam == HTCAPTION) || (wParam == HTMAXBUTTON) || (wParam == HTSYSMENU);
        if (bEnable && IsEnableSysMenu() && !IsUseSystemCaption()) {
            // 显示系统菜单
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if (ShowWindowSysMenu(m_hWnd, pt)) {
                bHandled = true;
            }
        }
        break;
    }
    default:
        break;
    }//end of switch
    return lResult;
}

void NativeWindow_Windows::StopSysMenuTimer()
{
    if (m_nSysMenuTimerId != 0) {
        ::KillTimer(m_hWnd, m_nSysMenuTimerId);
        m_nSysMenuTimerId = 0;
    }
}

bool NativeWindow_Windows::ShowWindowSysMenu(HWND hWnd, const POINT& pt) const
{
    HMENU hSysMenu = ::GetSystemMenu(hWnd, FALSE);
    if (hSysMenu == nullptr) {
        return false;
    }
    //更新菜单状态
    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STATE;
    mii.fType = 0;

    // update the options
    mii.fState = MF_ENABLED;
    SetMenuItemInfo(hSysMenu, SC_RESTORE, FALSE, &mii);
    SetMenuItemInfo(hSysMenu, SC_SIZE, FALSE, &mii);
    SetMenuItemInfo(hSysMenu, SC_MOVE, FALSE, &mii);
    SetMenuItemInfo(hSysMenu, SC_MAXIMIZE, FALSE, &mii);
    SetMenuItemInfo(hSysMenu, SC_MINIMIZE, FALSE, &mii);

    mii.fState = MF_GRAYED;

    WINDOWPLACEMENT wp = { 0, };
    ::GetWindowPlacement(hWnd, &wp);

    switch (wp.showCmd)
    {
    case SW_SHOWMAXIMIZED:
        SetMenuItemInfo(hSysMenu, SC_SIZE, FALSE, &mii);
        SetMenuItemInfo(hSysMenu, SC_MOVE, FALSE, &mii);
        SetMenuItemInfo(hSysMenu, SC_MAXIMIZE, FALSE, &mii);
        SetMenuDefaultItem(hSysMenu, SC_CLOSE, FALSE);
        break;
    case SW_SHOWMINIMIZED:
        SetMenuItemInfo(hSysMenu, SC_MINIMIZE, FALSE, &mii);
        SetMenuDefaultItem(hSysMenu, SC_RESTORE, FALSE);
        break;
    case SW_SHOWNORMAL:
        SetMenuItemInfo(hSysMenu, SC_RESTORE, FALSE, &mii);
        SetMenuDefaultItem(hSysMenu, SC_CLOSE, FALSE);
        break;
    default:
        break;
    }

    // 在点击位置显示系统菜单
    int32_t nRet = ::TrackPopupMenu(hSysMenu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, nullptr);
    if (nRet != 0) {
        ::PostMessage(hWnd, WM_SYSCOMMAND, nRet, 0);
    }
    return true;
}

void NativeWindow_Windows::OnFinalMessage()
{
    if (m_pOwner) {
        m_pOwner->OnNativeFinalMessage();
    }
}

HWND NativeWindow_Windows::GetWindowOwner() const
{
    return ::GetWindow(m_hWnd, GW_OWNER);
}

void NativeWindow_Windows::SetEnableSnapLayoutMenu(bool bEnable)
{
    //仅Windows11才支持
    if (UiIsWindows11OrGreater()) {
        m_bSnapLayoutMenu = bEnable;
    }
}

bool NativeWindow_Windows::IsEnableSnapLayoutMenu() const
{
    return m_bSnapLayoutMenu;
}

void NativeWindow_Windows::SetEnableSysMenu(bool bEnable)
{
    m_bEnableSysMenu = bEnable;
}

bool NativeWindow_Windows::IsEnableSysMenu() const
{
    return m_bEnableSysMenu;
}

void NativeWindow_Windows::SetImeOpenStatus(bool bOpen)
{
    if (!bOpen) {
        //禁用输入法
        EnableIME(m_hWnd, false);
    }
    else {
        //启用输入法
        EnableIME(m_hWnd, true);
    }
}

void NativeWindow_Windows::EnableIME(HWND hwnd, bool bEnable)
{
    ASSERT(::IsWindow(hwnd));
    if (!::IsWindow(hwnd)) {
        return;
    }
    if (!bEnable) {
        //禁用
        if (m_hImc == nullptr) {
            m_hImc = ::ImmAssociateContext(hwnd, nullptr);
            ASSERT(m_hImc != nullptr);
        }
    }
    else {
        //启用
        if (m_hImc != nullptr) {
            HIMC hImc = ::ImmAssociateContext(hwnd, m_hImc);
            m_hImc = nullptr;
            ASSERT_UNUSED_VARIABLE(hImc == nullptr);
        }
        else {
            //检查输入法是否打开，给出断言
            HIMC hImc = ::ImmGetContext(hwnd);
            ASSERT(hImc != nullptr);
            if (hImc != nullptr) {                
                if (!::ImmGetOpenStatus(hImc)) {
                    ::ImmSetOpenStatus(hImc, TRUE);
                }
                ASSERT(::ImmGetOpenStatus(hImc));
                ::ImmReleaseContext(hwnd, hImc);
            }
        }
    }
}

void NativeWindow_Windows::SetTextInputArea(const UiRect* /*rect*/, int32_t /*nCursor*/)
{
}

void NativeWindow_Windows::SetEnableDragDrop(bool bEnable)
{
    m_bEnableDragDrop = bEnable;
    if (bEnable && IsWindow()) {
        if (m_pWindowDropTarget == nullptr) {
            m_pWindowDropTarget = new WindowDropTarget(this);
            m_pWindowDropTarget->AddRef();
            m_pWindowDropTarget->RegisterDragDrop();
        }
    }
    else {
        if (m_pWindowDropTarget != nullptr) {
            m_pWindowDropTarget->UnregisterDragDrop();
            m_pWindowDropTarget->Release();
            m_pWindowDropTarget = nullptr;
        }
    }
}

bool NativeWindow_Windows::IsEnableDragDrop() const
{
    return m_bEnableDragDrop;
}

Control* NativeWindow_Windows::FindControl(const UiPoint& pt) const
{
    return m_pOwner->OnNativeFindControl(pt);
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
