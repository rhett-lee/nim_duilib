#include "MultiBrowserForm.h"
#include "BrowserBox.h"
#include "MultiBrowserManager.h"

#include "duilib/Utils/BitmapHelper_Windows.h"

#include "OleIdl.h"
#include "ShObjIdl.h"
#include <shlobj.h>

using namespace ui;

namespace
{
    const int kDragImageWidth = 300;
    const int kDragImageHeight = 300;
}

bool MultiBrowserForm::InitDragDrop()
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

void MultiBrowserForm::UnInitDragDrop()
{
    if (nullptr != m_pDropHelper) {
        m_pDropHelper->Release();
    }
    ::RevokeDragDrop(this->NativeWnd()->GetHWND());
}

HRESULT MultiBrowserForm::QueryInterface(REFIID iid, void ** ppvObject)
{
    if (nullptr == m_pDropHelper) {
        return E_NOINTERFACE;
    }
    return m_pDropHelper->QueryInterface(iid, ppvObject);
}

ULONG MultiBrowserForm::AddRef(void)
{
    if (nullptr == m_pDropHelper) {
        return 0;
    }
    return m_pDropHelper->AddRef();
}

ULONG MultiBrowserForm::Release(void)
{
    if (nullptr == m_pDropHelper) {
        return 0;
    }
    return m_pDropHelper->Release();
}

HRESULT MultiBrowserForm::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
    if (nullptr == m_pDropHelper) {
        return S_OK;
    }

    // 如果不是拖拽浏览器盒子
    if (!MultiBrowserManager::GetInstance()->IsDragingBorwserBox()) {
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

HRESULT MultiBrowserForm::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if (nullptr == m_pDropHelper) {
        return S_OK;
    }

    // 如果不是拖拽浏览器盒子
    if (!MultiBrowserManager::GetInstance()->IsDragingBorwserBox()) {
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

HRESULT MultiBrowserForm::DragLeave(void)
{
    if (nullptr == m_pDropHelper) {
        return S_OK;
    }

    // 如果不是拖拽浏览器盒子
    if (!MultiBrowserManager::GetInstance()->IsDragingBorwserBox()) {
        if (nullptr != m_pActiveBrowserBox) {
            //m_pActiveBrowserBox->DragLeave();
        }
    }
    m_pDropHelper->DragLeave();
    return S_OK;
}

HRESULT MultiBrowserForm::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect)
{
    // 如果不是拖拽浏览器盒子
    if (!MultiBrowserManager::GetInstance()->IsDragingBorwserBox()) {
#if 0
         if (nullptr != m_pActiveBrowserBox && m_pActiveBrowserBox->CheckDropEnable(pt)) {
             m_pActiveBrowserBox->Drop(pDataObj, grfKeyState, pt, pdwEffect);
         }
#endif
    }
    else {
        *pdwEffect = DROPEFFECT_MOVE;
        MultiBrowserManager::GetInstance()->SetDropForm(this);
    }

    m_pDropHelper->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);
    return S_OK;
}

bool MultiBrowserForm::OnProcessTabItemDrag(const ui::EventArgs& param)
{
    if (!MultiBrowserManager::GetInstance()->IsEnableMerge()) {
        return true;
    }

    switch (param.eventType)
    {
    case kEventMouseMove:
    {
        if (::GetKeyState(VK_LBUTTON) >= 0) {
            break;
        }

        LONG cx = abs(param.ptMouse.x - m_oldDragPoint.x);
        LONG cy = abs(param.ptMouse.y - m_oldDragPoint.y);

        if (!m_bDragState && (cx > 5 || cy > 5)) {
            if (nullptr == m_pActiveBrowserBox) {
                break;
            }

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

            StdClosure cb = [=]{
                MultiBrowserManager::GetInstance()->DoDragBorwserBox(m_pActiveBrowserBox, hBitmap, pt);
                ::DeleteObject(hBitmap);
            };
            ui::GlobalManager::Instance().Thread().PostTask(kThreadUI, cb);
        }
    }
    break;
    case kEventMouseButtonDown:
    {
        m_oldDragPoint = { param.ptMouse.x, param.ptMouse.y };
        m_bDragState = false;
    }
    break;
    }
    return true;
}

ui::IBitmap* MultiBrowserForm::GenerateBoxOffsetRenderBitmap(const UiRect& src_rect)
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

ui::IBitmap* MultiBrowserForm::GenerateBoxWindowBitmap()
{
    if (!m_pActiveBrowserBox) {
        return nullptr;
    }

    HWND cef_window = m_pActiveBrowserBox->GetCefControl()->GetCefHandle();
    RECT src_rect = {0, };
    ::GetClientRect(cef_window, &src_rect);
    
    int src_width = src_rect.right - src_rect.left;
    int src_height = src_rect.bottom - src_rect.top;

    //创建一个内存DC
    HDC cef_window_dc = ::GetDC(cef_window);
    ui::IBitmap* pBitmap = ui::BitmapHelper::CreateBitmapObject(kDragImageWidth, kDragImageHeight, cef_window_dc, src_width, src_height);
    ::ReleaseDC(cef_window, cef_window_dc);
    return pBitmap;
}
