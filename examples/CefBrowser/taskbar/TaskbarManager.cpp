#include "TaskbarManager.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Utils/BitmapHelper_Windows.h"
#include "duilib/Utils/StringUtil.h"

#include "DwmUtil.h"
#include <shobjidl.h>
#include <VersionHelpers.h>

using namespace ui;

TaskbarTabItem::TaskbarTabItem(ui::Control *bind_control)
{
    ASSERT(nullptr != bind_control);
    m_pBindControl = bind_control;
    m_bWin7orGreater = ::IsWindows7OrGreater();
    m_taskbarManager = nullptr;
}

ui::Control* TaskbarTabItem::GetBindControl()
{
    return m_pBindControl;
}

std::string& TaskbarTabItem::GetId()
{
    return m_id;
}

void TaskbarTabItem::Init(const DString &taskbar_title, const std::string &id)
{
    m_id = id;
    if (!m_bWin7orGreater) {
        return;
    }

    CreateWnd(nullptr, ui::WindowCreateParam(_T("CefBrowser")));
    ShowWindow(ui::kSW_SHOW);

    HRESULT ret = S_OK;
    BOOL truth = TRUE;
    ret |= DwmSetWindowAttribute(NativeWnd()->GetHWND(), DWMWA_HAS_ICONIC_BITMAP, &truth, sizeof(truth));
    ret |= DwmSetWindowAttribute(NativeWnd()->GetHWND(), DWMWA_FORCE_ICONIC_REPRESENTATION, &truth, sizeof(truth));
    if (ret != S_OK) {
        m_bWin7orGreater = false;
    }
}

void TaskbarTabItem::UnInit()
{
    if (nullptr != NativeWnd()->GetHWND()) {
        DestroyWindow(NativeWnd()->GetHWND());
    }
}

void TaskbarTabItem::SetTaskbarTitle(const DString& title)
{
    DString localText = StringConvert::TToLocal(title);
    ::SetWindowText(NativeWnd()->GetHWND(), localText.c_str());
}

void TaskbarTabItem::SetTaskbarIcon(HICON hIcon)
{
    if (hIcon != nullptr) {
        if (::IsWindow(NativeWnd()->GetHWND())) {
            ::SendMessage(NativeWnd()->GetHWND(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);   // 设置大图标（通常32x32）
        }
        else {
            ::DestroyIcon(hIcon);
        }
    }
}

void TaskbarTabItem::SetTaskbarManager(TaskbarManager *taskbar_manager)
{
    m_taskbarManager = taskbar_manager;
}

TaskbarManager* TaskbarTabItem::GetTaskbarManager()
{
    return m_taskbarManager;
}

bool TaskbarTabItem::InvalidateTab()
{
    if (!m_bWin7orGreater || nullptr == m_taskbarManager) {
        return false;
    }
    return (S_OK == DwmInvalidateIconicBitmaps(this->NativeWnd()->GetHWND()));
}

void TaskbarTabItem::OnSendThumbnail(int width, int height)
{
    if (!m_bWin7orGreater || nullptr == m_taskbarManager) {
        return;
    }

    ui::IBitmap* pBitmap = m_taskbarManager->GenerateBindControlBitmap(m_pBindControl, width, height);
    HBITMAP hBitmap = ui::BitmapHelper::CreateGDIBitmap(pBitmap);
    DwmSetIconicThumbnail(NativeWnd()->GetHWND(), hBitmap, 0);
    if (pBitmap != nullptr) {
        delete pBitmap;
        pBitmap = nullptr;
    }
    if (hBitmap != nullptr) {
        ::DeleteObject(hBitmap);
        hBitmap = nullptr;
    }
}

void TaskbarTabItem::OnSendPreview()
{
    if (!m_bWin7orGreater || nullptr == m_taskbarManager) {
        return;
    }

    ui::IBitmap* pBitmap = m_taskbarManager->GenerateBindControlBitmapWithForm(m_pBindControl);
    HBITMAP hBitmap = ui::BitmapHelper::CreateGDIBitmap(pBitmap);
    DwmSetIconicLivePreviewBitmap(NativeWnd()->GetHWND(), hBitmap, nullptr, 0);
    if (pBitmap != nullptr) {
        delete pBitmap;
        pBitmap = nullptr;
    }
    if (hBitmap != nullptr) {
        ::DeleteObject(hBitmap);
        hBitmap = nullptr;
    }
}

LRESULT TaskbarTabItem::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    bHandled = true;
    if (uMsg == WM_DWMSENDICONICTHUMBNAIL) {
        OnSendThumbnail(HIWORD(lParam), LOWORD(lParam));
        return 0;
    }
    else if (uMsg == WM_DWMSENDICONICLIVEPREVIEWBITMAP) {
        OnSendPreview();
        return 0;
    }
    else if (uMsg == WM_GETICON) {
        InvalidateTab();
    }
    else if (uMsg == WM_CLOSE) {
        if (nullptr != m_taskbarManager) {
            m_taskbarManager->OnTabItemClose(*this);
        }
        return 0;
    }
    else if (uMsg == WM_ACTIVATE) {
        if (nullptr != m_taskbarManager) {
            if (wParam != WA_INACTIVE) {
                m_taskbarManager->OnTabItemClicked(*this);
            }
        }            
        return 0;
    }
    return BaseClass::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

TaskbarManager::TaskbarManager()
{
    m_pTaskbarDelegate = nullptr;
    m_pTaskbarList = nullptr;
}

void TaskbarManager::Init(ITaskbarDelegate *taskbar_delegate)
{
    ASSERT(nullptr != taskbar_delegate);
    m_pTaskbarDelegate = taskbar_delegate;

    ::CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pTaskbarList));
    if (m_pTaskbarList) {
        m_pTaskbarList->HrInit();

        BOOL truth = FALSE;
        DwmSetWindowAttribute(m_pTaskbarDelegate->GetHandle(), DWMWA_HAS_ICONIC_BITMAP, &truth, sizeof(truth));
        DwmSetWindowAttribute(m_pTaskbarDelegate->GetHandle(), DWMWA_FORCE_ICONIC_REPRESENTATION, &truth, sizeof(truth));
    }
        
}

bool TaskbarManager::RegisterTab(TaskbarTabItem &tab_item)
{
    if (m_pTaskbarList && (nullptr == tab_item.GetTaskbarManager())) {
        if (S_OK == m_pTaskbarList->RegisterTab(tab_item.NativeWnd()->GetHWND(), m_pTaskbarDelegate->GetHandle())) {
            if (S_OK == m_pTaskbarList->SetTabOrder(tab_item.NativeWnd()->GetHWND(), nullptr)) {
                tab_item.SetTaskbarManager(this);
                return true;
            }            
        }
    }
    return false;
}

bool TaskbarManager::UnregisterTab(TaskbarTabItem &tab_item)
{
    if (m_pTaskbarList) {
        tab_item.SetTaskbarManager(nullptr);
        return (S_OK == m_pTaskbarList->UnregisterTab(tab_item.NativeWnd()->GetHWND()));
    }
    return false;
}

bool TaskbarManager::SetTabOrder(const TaskbarTabItem &tab_item, const TaskbarTabItem &tab_item_insert_before)
{
    if (m_pTaskbarList) {
        return (S_OK == m_pTaskbarList->SetTabOrder(tab_item.NativeWnd()->GetHWND(), tab_item_insert_before.NativeWnd()->GetHWND()));
    }
    return false;
}

bool TaskbarManager::SetTabActive(const TaskbarTabItem &tab_item)
{
    if (m_pTaskbarList) {
        return (S_OK == m_pTaskbarList->SetTabActive(tab_item.NativeWnd()->GetHWND(), m_pTaskbarDelegate->GetHandle(), 0));
    }
    return false;
}

ui::IBitmap* TaskbarManager::GenerateBindControlBitmapWithForm(ui::Control *control)
{
    ASSERT( nullptr != control);
    if (nullptr == control) {
        return nullptr;
    }

    int window_width = 0, window_height = 0;
    RECT rc_wnd;
    if (::IsIconic(m_pTaskbarDelegate->GetHandle())) {//当前是最小化状态
        WINDOWPLACEMENT placement{ sizeof(WINDOWPLACEMENT) };
        ::GetWindowPlacement(m_pTaskbarDelegate->GetHandle(), &placement);
        if (placement.flags == WPF_RESTORETOMAXIMIZED) {//最小化前是最大化状态
            MONITORINFO oMonitor = { sizeof(MONITORINFO) };
            ::GetMonitorInfo(::MonitorFromWindow(m_pTaskbarDelegate->GetHandle(), MONITOR_DEFAULTTONEAREST), &oMonitor);
            rc_wnd = oMonitor.rcWork;
        }
        else {
            rc_wnd = placement.rcNormalPosition;
        }
    }
    else {
        ::GetWindowRect(m_pTaskbarDelegate->GetHandle(), &rc_wnd);
    }
    window_width = rc_wnd.right - rc_wnd.left;
    window_height = rc_wnd.bottom - rc_wnd.top;
    if (window_width == 0 || window_height == 0) {
        return nullptr;
    }

    // 1.创建内存dc
    std::unique_ptr<IRender> render;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        ASSERT(control->GetWindow() != nullptr);
        IRenderDpiPtr spRenderDpi;
        if (control->GetWindow() != nullptr) {
            spRenderDpi = control->GetWindow()->GetRenderDpi();
        }
        render.reset(pRenderFactory->CreateRender(spRenderDpi));
    }
    ASSERT(render != nullptr);
    if (render == nullptr) {
        return nullptr;
    }
    render->Resize(window_width, window_height);

    // 2.把窗口双缓冲的位图画到内存dc
    render->BitBlt(0, 0, window_width, window_height, m_pTaskbarDelegate->GetTaskbarRender(), 0, 0, RopMode::kSrcCopy);

    // 3.把某个会话盒子的位图画到内存dc，覆盖原窗口对应位置的位图
    UiRect rcPaint = control->GetPos();
    if (rcPaint.IsEmpty()) {
        return nullptr;
    }
    rcPaint.Intersect(UiRect(0, 0, window_width, window_height));

    // 这里不设置剪裁区域，就无法正常绘制
    {
        AutoClip rectClip(render.get(), rcPaint);
        control->Paint(render.get(), rcPaint);
    }

    // 4.修复绘制区域的alpha通道
    render->RestoreAlpha(rcPaint);
    return render->MakeImageSnapshot();
}

ui::IBitmap* TaskbarManager::GenerateBindControlBitmap(ui::Control *control, const int dest_width, const int dest_height)
{
    ASSERT(dest_width > 0 && dest_height > 0 && nullptr != control);
    if (dest_width <= 0 || dest_height <= 0 || nullptr == control) {
        return nullptr;
    }

    int window_width = 0, window_height = 0;
    RECT rc_wnd;
    if (::IsIconic(m_pTaskbarDelegate->GetHandle())) {//当前是最小化状态
        WINDOWPLACEMENT placement{ sizeof(WINDOWPLACEMENT) };
        ::GetWindowPlacement(m_pTaskbarDelegate->GetHandle(), &placement);
        if (placement.flags == WPF_RESTORETOMAXIMIZED) {//最小化前是最大化状态
            MONITORINFO oMonitor = { sizeof(MONITORINFO) };
            ::GetMonitorInfo(::MonitorFromWindow(m_pTaskbarDelegate->GetHandle(), MONITOR_DEFAULTTONEAREST), &oMonitor);
            rc_wnd = oMonitor.rcWork;
        }
        else {
            rc_wnd = placement.rcNormalPosition;
        }
    }
    else {
        ::GetWindowRect(m_pTaskbarDelegate->GetHandle(), &rc_wnd);
    }
    window_width = rc_wnd.right - rc_wnd.left;
    window_height = rc_wnd.bottom - rc_wnd.top;
    if (window_width == 0 || window_height == 0) {
        return nullptr;
    }

    // 1.创建内存dc
    std::unique_ptr<IRender> render;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        ASSERT(control->GetWindow() != nullptr);
        IRenderDpiPtr spRenderDpi;
        if (control->GetWindow() != nullptr) {
            spRenderDpi = control->GetWindow()->GetRenderDpi();
        }
        render.reset(pRenderFactory->CreateRender(spRenderDpi));
    }
    ASSERT(render != nullptr);
    if (render == nullptr) {
        return nullptr;
    }
    render->Resize(window_width, window_height);

    // 2.把某个会话盒子的位图画到内存dc，覆盖原窗口对应位置的位图
    UiRect rcPaint = control->GetPos();
    if (rcPaint.IsEmpty()) {
        return nullptr;
    }
    rcPaint.Intersect(UiRect(0, 0, window_width, window_height));

    // 这里不设置剪裁区域，就无法正常绘制
    {
        AutoClip rectClip(render.get(), rcPaint);
        control->Paint(render.get(), rcPaint);
    }

    // 3.修复绘制区域的alpha通道
    render->RestoreAlpha(rcPaint);

    // 4.缩放到目标尺寸
    UiRect rcControl = control->GetPos();
    return ResizeBitmap(dest_width, dest_height, render.get(), rcControl.left, rcControl.top, rcControl.Width(), rcControl.Height());
}

ui::IBitmap* TaskbarManager::ResizeBitmap(int dest_width, int dest_height, ui::IRender* pSrcRender, int src_x, int src_y, int src_width, int src_height)
{
    std::unique_ptr<IRender> render;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        render.reset(pRenderFactory->CreateRender(nullptr));
    }
    ASSERT(render != nullptr);
    if (render == nullptr) {
        return nullptr;
    }
    if (render->Resize(dest_width, dest_height)) {
        int scale_width = 0;
        int scale_height = 0;

        float src_scale = (float)src_width / (float)src_height;
        float dest_scale = (float)dest_width / (float)dest_height;
        if (src_scale >= dest_scale) {
            scale_width = dest_width;
            scale_height = (int)(dest_width * (float)src_height / (float)src_width);
        }
        else {
            scale_height = dest_height;
            scale_width = (int)(dest_height * (float)src_width / (float)src_height);
        }

        render->AlphaBlend((dest_width - scale_width) / 2, (dest_height - scale_height) / 2, scale_width, scale_height, pSrcRender, src_x, src_y, src_width, src_height);
    }

    return render->MakeImageSnapshot();
}

void TaskbarManager::OnTabItemClose(TaskbarTabItem &tab_item)
{
    m_pTaskbarDelegate->CloseTaskbarItem(tab_item.GetId());
}

void TaskbarManager::OnTabItemClicked(TaskbarTabItem &tab_item)
{
    m_pTaskbarDelegate->SetActiveTaskbarItem(tab_item.GetId());
}

#endif //(DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
