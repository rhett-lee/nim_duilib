#include "BrowserForm_Windows.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
#include "duilib/Utils/BitmapHelper_Windows.h"
#include "Windows/dragdrop/DragDropManager.h"
#include "Windows/BrowserBox_Windows.h"
#include "browser/BrowserManager.h"

#include <OleIdl.h>
#include <ShObjIdl.h>
#include <shlobj.h>

using namespace ui;

namespace
{
    // 注册这个消息，收到这个消息后表示窗口对应的任务栏按钮被系统创建，这时候初始化ITaskbarList4接口
    UINT WM_TASKBARBUTTONCREATED = ::RegisterWindowMessage(TEXT("TaskbarButtonCreated"));

    // 拖拽图片的宽度和高度
    const int kDragImageWidth = 300;
    const int kDragImageHeight = 300;
}

BrowserForm_Windows::BrowserForm_Windows()
{
    m_bButtonDown = false;
    m_pDropHelper = nullptr;
    m_bDragState = false;
    m_oldDragPoint = {0, 0};
}

BrowserForm_Windows::~BrowserForm_Windows()
{
    m_pDropHelper = nullptr;
}

BrowserBox* BrowserForm_Windows::CreateBrowserBox(ui::Window* pWindow, std::string id)
{
    return new BrowserBox_Windows(pWindow, id);
}

void BrowserForm_Windows::OnInitWindow()
{
    BaseClass::OnInitWindow();
    InitDragDrop();
}

void BrowserForm_Windows::OnCloseWindow()
{
    BaseClass::OnCloseWindow();
    UnInitDragDrop();
}

LRESULT BrowserForm_Windows::OnWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if (uMsg == WM_TASKBARBUTTONCREATED) {
        bHandled = true;
        m_taskbarManager.Init(this);

        // 因为窗口刚创建时，浏览器盒子已经创建但是那时还没有收到WM_TASKBARBUTTONCREATED消息，导致RegisterTab函数没有被调用，所以收到消息后重新遍历一下没有被注册的Tab
        for (size_t i = 0; i < m_pBorwserBoxTab->GetItemCount(); ++i) {
            Control *box_item = m_pBorwserBoxTab->GetItemAt(i);
            ASSERT(box_item != nullptr);
            if (box_item == nullptr) {
                continue;
            }

            BrowserBox_Windows* pBrowserBox = dynamic_cast<BrowserBox_Windows*>(box_item);
            if (pBrowserBox == nullptr) {
                continue;
            }

            TaskbarTabItem* taskbar_item = pBrowserBox->GetTaskbarItem();
            if (taskbar_item != nullptr) {
                m_taskbarManager.RegisterTab(*taskbar_item);
            }
        }

        return TRUE;
    }
    return BaseClass::OnWindowMessage(uMsg, wParam, lParam, bHandled);
}

void BrowserForm_Windows::OnCreateNewTabPage(ui::TabCtrlItem* tab_item, BrowserBox* browser_box)
{
    if (tab_item != nullptr) {
        tab_item->AttachAllEvents(UiBind(&BrowserForm_Windows::OnProcessTabItemDrag, this, std::placeholders::_1));
    }

    BrowserBox_Windows* pBrowserBox = dynamic_cast<BrowserBox_Windows*>(browser_box);
    if (pBrowserBox != nullptr) {
        auto taskbar_item = pBrowserBox->GetTaskbarItem();
        if (taskbar_item) {
            m_taskbarManager.RegisterTab(*taskbar_item);
        }
    }
}

void BrowserForm_Windows::OnCloseTabPage(BrowserBox* browser_box)
{
    BrowserBox_Windows* pBrowserBox = dynamic_cast<BrowserBox_Windows*>(browser_box);
    if (pBrowserBox != nullptr) {
        auto taskbar_item = pBrowserBox->GetTaskbarItem();
        if (taskbar_item) {
            m_taskbarManager.UnregisterTab(*taskbar_item);
        }
    }
}

void BrowserForm_Windows::OnBeforeDragBoxCallback(const DString &browser_id)
{
    // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，则在拖拽时隐藏浏览器窗口
    int box_count = this->GetBoxCount();
    if (1 == box_count) {
        this->ShowWindow(kSW_HIDE);
    }
    // 否则隐藏被拖拽的浏览器盒子和标签
    else {        
        BrowserBox* browser_box = FindBox(browser_id);
        if (browser_box != nullptr) {
            browser_box->SetVisible(false);
        }
        else {
            return;
        }
        m_dragingBrowserId = browser_id;

        TabCtrlItem* tab_item = FindTabItem(browser_id);
        if (tab_item != nullptr) {
            tab_item->CancelDragOperation();
            tab_item->SetVisible(false);
        }

        // 找到新的被显示的浏览器盒子
        size_t index = tab_item->GetListBoxIndex();
        if (index > 0) {
            index--;
        }
        else {
            index++;
        }
        TabCtrlItem* new_tab_item = static_cast<TabCtrlItem*>(m_pTabCtrl->GetItemAt(index));
        if (new_tab_item != nullptr) {
            new_tab_item->Selected(true, false);
            ChangeToBox(new_tab_item->GetName());
        }

        //由于标签隐藏，通知标签的父控件重新计算位置
        if (m_pTabCtrl != nullptr) {
            m_pTabCtrl->ArrangeAncestor();
        }
    }
}

void BrowserForm_Windows::OnAfterDragBoxCallback(bool drop_succeed)
{
    m_bDragState = false;

    if (drop_succeed) {
        int box_count = this->GetBoxCount();
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，并且拖拽到新的浏览器窗口里，这个浏览器窗口就会关闭
        if (1 == box_count) {

        }
        else {
        }
    }
    else {
        int box_count = this->GetBoxCount();
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口只有一个浏览器盒子，并且没有拖拽到新的浏览器窗口里
        // 就显示浏览器窗口
        if (1 == box_count) {
            this->ShowWindow(kSW_SHOW_NORMAL);
        }
        // 如果当前被拖拽的浏览器盒子所属的浏览器窗口有多个浏览器盒子，并且没有拖拽到新的浏览器窗口里
        // 就显示之前被隐藏的浏览器盒子和标签
        else {
            BrowserBox *browser_box = FindBox(m_dragingBrowserId);
            if (nullptr != browser_box) {
                browser_box->SetFadeVisible(true);
            }

            TabCtrlItem* tab_item = FindTabItem(m_dragingBrowserId);
            if (nullptr != tab_item) {
                tab_item->SetFadeVisible(true);
                tab_item->Selected(true, false);
                ChangeToBox(m_dragingBrowserId);
            }
            m_dragingBrowserId.clear();
        }
    }
}

bool BrowserForm_Windows::InitDragDrop()
{
    if (nullptr != m_pDropHelper) {
        return false;
    }

    if (FAILED(::CoCreateInstance(CLSID_DragDropHelper, nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IDropTargetHelper,
        (void**)&m_pDropHelper))) {
        return false;
    }

    if (FAILED(::RegisterDragDrop(this->NativeWnd()->GetHWND(), this))) {
        return false;
    }
    return true;
}

void BrowserForm_Windows::UnInitDragDrop()
{
    if (nullptr != m_pDropHelper) {
        m_pDropHelper->Release();
    }
    ::RevokeDragDrop(this->NativeWnd()->GetHWND());
}

HRESULT BrowserForm_Windows::QueryInterface(REFIID iid, void** ppvObject)
{
    if (nullptr == m_pDropHelper) {
        return E_NOINTERFACE;
    }
    return m_pDropHelper->QueryInterface(iid, ppvObject);
}

ULONG BrowserForm_Windows::AddRef(void)
{
    if (nullptr == m_pDropHelper) {
        return 0;
    }
    return m_pDropHelper->AddRef();
}

ULONG BrowserForm_Windows::Release(void)
{
    if (nullptr == m_pDropHelper) {
        return 0;
    }
    return m_pDropHelper->Release();
}

HRESULT BrowserForm_Windows::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (nullptr == m_pDropHelper) {
        return S_OK;
    }

    // 如果不是拖拽浏览器盒子
    if (!DragDropManager::GetInstance()->IsDragingBorwserBox()) {
        if (nullptr != m_pActiveBrowserBox) {
            //m_pActiveBrowserBox->DragEnter(pDataObject, grfKeyState, pt, pdwEffect);
            if (IsWindowMinimized()) {
                ShowWindow(kSW_RESTORE);
            }
            else {
                ShowWindow(kSW_SHOW);
            }
        }
    }
    else {
        *pdwEffect = DROPEFFECT_MOVE;
    }
    m_pDropHelper->DragEnter(this->NativeWnd()->GetHWND(), pDataObject, (LPPOINT)&pt, *pdwEffect);
    return S_OK;
}

HRESULT BrowserForm_Windows::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (nullptr == m_pDropHelper) {
        return S_OK;
    }

    // 如果不是拖拽浏览器盒子
    if (!DragDropManager::GetInstance()->IsDragingBorwserBox()) {
        if (nullptr != m_pActiveBrowserBox) {
            //m_pActiveBrowserBox->DragOver(grfKeyState, pt, pdwEffect);
        }
    }
    else {
        *pdwEffect = DROPEFFECT_MOVE;
    }

    m_pDropHelper->DragOver((LPPOINT)&pt, *pdwEffect);
    return S_OK;
}

HRESULT BrowserForm_Windows::DragLeave(void)
{
    if (nullptr == m_pDropHelper) {
        return S_OK;
    }

    // 如果不是拖拽浏览器盒子
    if (!DragDropManager::GetInstance()->IsDragingBorwserBox()) {
        if (nullptr != m_pActiveBrowserBox) {
            //m_pActiveBrowserBox->DragLeave();
        }
    }
    m_pDropHelper->DragLeave();
    return S_OK;
}

HRESULT BrowserForm_Windows::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD __RPC_FAR* pdwEffect)
{
    // 如果不是拖拽浏览器盒子
    if (!DragDropManager::GetInstance()->IsDragingBorwserBox()) {
#if 0
        if (nullptr != m_pActiveBrowserBox && m_pActiveBrowserBox->CheckDropEnable(pt)) {
            m_pActiveBrowserBox->Drop(pDataObj, grfKeyState, pt, pdwEffect);
        }
#endif
    }
    else {
        *pdwEffect = DROPEFFECT_MOVE;
        DragDropManager::GetInstance()->SetDropForm(this);
    }

    m_pDropHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);
    return S_OK;
}

bool BrowserForm_Windows::OnProcessTabItemDrag(const ui::EventArgs& param)
{
    if (!BrowserManager::GetInstance()->IsEnableMerge()) {
        return true;
    }

    switch (param.eventType)
    {
    case kEventMouseMove:
        {
            if (!m_bButtonDown || (m_pActiveBrowserBox == nullptr) || (::GetKeyState(VK_LBUTTON) >= 0)) {
                break;
            }

            DString id = ui::StringConvert::UTF8ToT(m_pActiveBrowserBox->GetId());
            TabCtrlItem* tab_item = FindTabItem(id);
            if (tab_item == nullptr) {
                break;
            }

            //当鼠标纵向滑动的距离超过标签宽度的时候，开始拖出操作
            LONG cy = abs(param.ptMouse.y - m_oldDragPoint.y);
            if (!m_bDragState && (cy > tab_item->GetPos().Height())) {

                m_bDragState = true;

                // 把被拖拽的浏览器盒子生成一个宽度300的位图
                IBitmap* pBitmap = nullptr;
                if (ui::CefManager::GetInstance()->IsEnableOffScreenRendering()) {
                    pBitmap = GenerateBoxOffsetRenderBitmap(m_pBorwserBoxTab->GetPos());
                }
                else {
                    pBitmap = GenerateBoxWindowBitmap();
                }
                HBITMAP hBitmap = ui::BitmapHelper::CreateGDIBitmap(pBitmap);
                if (pBitmap != nullptr) {
                    delete pBitmap;
                    pBitmap = nullptr;
                }
                // pt应该指定相对bitmap位图的左上角(0,0)的坐标,这里设置为bitmap的中上点
                POINT pt = { kDragImageWidth / 2, 0 };

                StdClosure cb = [=] {
                        DragDropManager::GetInstance()->DoDragBorwserBox(m_pActiveBrowserBox, hBitmap, pt);
                        ::DeleteObject(hBitmap);
                    };
                ui::GlobalManager::Instance().Thread().PostTask(kThreadUI, cb);
            }
        }
        break;
    case kEventMouseButtonDown:
        m_oldDragPoint = { param.ptMouse.x, param.ptMouse.y };
        m_bDragState = false;
        m_bButtonDown = true;
        break;
    case kEventMouseButtonUp:
        m_bButtonDown = false;
        break;
    }
    return true;
}

ui::IBitmap* BrowserForm_Windows::GenerateBoxOffsetRenderBitmap(const UiRect& src_rect)
{
    ASSERT(!src_rect.IsEmpty());
    int src_width = src_rect.right - src_rect.left;
    int src_height = src_rect.bottom - src_rect.top;

    std::unique_ptr<IRender> render;
    IRenderFactory* pRenderFactory = GlobalManager::Instance().GetRenderFactory();
    ASSERT(pRenderFactory != nullptr);
    if (pRenderFactory != nullptr) {
        render.reset(pRenderFactory->CreateRender(GetRenderDpi()));
    }
    ASSERT(render != nullptr);
    if (render->Resize(kDragImageWidth, kDragImageHeight)) {
        int dest_width = 0;
        int dest_height = 0;
        float scale = (float)src_width / (float)src_height;
        if (scale >= 1.0) {
            dest_width = kDragImageWidth;
            dest_height = (int)(kDragImageWidth * (float)src_height / (float)src_width);
        }
        else {
            dest_height = kDragImageHeight;
            dest_width = (int)(kDragImageHeight * (float)src_width / (float)src_height);
        }

        render->AlphaBlend((kDragImageWidth - dest_width) / 2, 0, dest_width, dest_height,
            this->GetRender(),
            src_rect.left, src_rect.top, src_rect.right - src_rect.left, src_rect.bottom - src_rect.top);
    }

    return render->MakeImageSnapshot();

}

ui::IBitmap* BrowserForm_Windows::GenerateBoxWindowBitmap()
{
    if (!m_pActiveBrowserBox) {
        return nullptr;
    }

    HWND cef_window = m_pActiveBrowserBox->GetCefControl()->GetCefHandle();
    RECT src_rect = { 0, };
    ::GetClientRect(cef_window, &src_rect);

    int src_width = src_rect.right - src_rect.left;
    int src_height = src_rect.bottom - src_rect.top;

    //创建一个内存DC
    HDC cef_window_dc = ::GetDC(cef_window);
    ui::IBitmap* pBitmap = ui::BitmapHelper::CreateBitmapObject(kDragImageWidth, kDragImageHeight, cef_window_dc, src_width, src_height);
    ::ReleaseDC(cef_window, cef_window_dc);
    return pBitmap;
}

#endif //defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)
