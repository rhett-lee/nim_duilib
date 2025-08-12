#include "WindowDropTarget_Windows.h"
#include "duilib/Core/ControlDropTarget.h"

#if defined (DUILIB_BUILD_FOR_WIN) && !defined (DUILIB_BUILD_FOR_SDL)

#include "duilib/Core/NativeWindow_Windows.h"
#include "duilib/Core/Control.h"

namespace ui 
{

WindowDropTarget::WindowDropTarget(NativeWindow_Windows* pNativeWindow):
    m_nRef(0),
    m_pDataObj(nullptr),
    m_pHoverDropTarget(nullptr),
    m_pNativeWindow(pNativeWindow),
    m_bRegisterDragDrop(false)
{
}

WindowDropTarget::~WindowDropTarget()
{
    if (m_pDataObj != nullptr) {
        m_pDataObj->Release();
        m_pDataObj = nullptr;
    }
}

bool WindowDropTarget::RegisterDragDrop()
{
    if (m_bRegisterDragDrop) {
        return true;
    }
    bool bRet = false;
    HWND hWnd = m_pNativeWindow->GetHWND();
    if (::IsWindow(hWnd)) {
        HRESULT hr = ::RegisterDragDrop(hWnd, this);
        ASSERT(hr == S_OK);
        bRet = (hr == S_OK) ? true : false;
    }
    m_bRegisterDragDrop = bRet;
    return bRet;
}

bool WindowDropTarget::UnregisterDragDrop()
{
    if (!m_bRegisterDragDrop) {
        return true;
    }
    bool bRet = true;
    HWND hWnd = m_pNativeWindow->GetHWND();
    if (::IsWindow(hWnd)) {
        HRESULT hr = ::RevokeDragDrop(hWnd);
        ASSERT(hr == S_OK);
        bRet = (hr == S_OK) ? true : false;
    }
    m_bRegisterDragDrop = false;
    return bRet;
}

// IUnkown 接口
HRESULT WindowDropTarget::QueryInterface(REFIID riid, void __RPC_FAR* __RPC_FAR* ppvObject)
{
    if (ppvObject == nullptr) {
        return E_INVALIDARG;
    }
    HRESULT hr = E_NOINTERFACE;
    if (riid == IID_IUnknown) {
        *ppvObject = (IUnknown*)this;
        hr = S_OK;
    }
    else if (riid == IID_IDropTarget) {
        *ppvObject = (IDropTarget*)this;
        hr = S_OK;
    }
    if (SUCCEEDED(hr)) {
        AddRef();
    }
    return hr;
}

ULONG WindowDropTarget::AddRef(void)
{
    return ++m_nRef;
}

ULONG WindowDropTarget::Release(void)
{
    ULONG uRet = --m_nRef;
    if (uRet == 0) {
        delete this;
    }
    return uRet;
}

// IDropTarget 接口
HRESULT WindowDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (m_pDataObj != nullptr) {
        m_pDataObj->Release();
        m_pDataObj = nullptr;
    }
    m_pDataObj = pDataObj;
    if (m_pDataObj != nullptr) {
        m_pDataObj->AddRef();
    }
    OnDragOver(grfKeyState, pt, pdwEffect);
    //需要返回S_OK, 否则本次拖放就终止了，无法再继续拖放操作
    return S_OK;
}

HRESULT WindowDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    return OnDragOver(grfKeyState, pt, pdwEffect);
}

HRESULT STDMETHODCALLTYPE WindowDropTarget::DragLeave(void)
{
    if (m_pDataObj != nullptr) {
        m_pDataObj->Release();
        m_pDataObj = nullptr;
    }
    if (m_pHoverDropTarget != nullptr) {
        m_pHoverDropTarget->DragLeave();
        m_pHoverDropTarget = nullptr;
    }
    return S_OK;
}

HRESULT WindowDropTarget::OnDragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    HRESULT hr = S_FALSE;
    ControlPtrT<ControlDropTarget_Windows> pHoverDropTarget = GetControlDropTarget(UiPoint(pt.x, pt.y));
    if (pHoverDropTarget == nullptr) {
        if (m_pHoverDropTarget != nullptr) {
            m_pHoverDropTarget->DragLeave();
            m_pHoverDropTarget = nullptr;
        }
    }
    else if (pHoverDropTarget == m_pHoverDropTarget) {
        uint32_t dwEffect = DROPEFFECT_NONE;
        if (pdwEffect != nullptr) {
            dwEffect = *pdwEffect;
        }
        hr = pHoverDropTarget->DragOver(grfKeyState, UiPoint(pt.x, pt.y), &dwEffect);
        if (pdwEffect != nullptr) {
            *pdwEffect = dwEffect;
        }
    }
    else {
        if (m_pHoverDropTarget != nullptr) {
            m_pHoverDropTarget->DragLeave();
        }
        m_pHoverDropTarget = pHoverDropTarget;
        uint32_t dwEffect = DROPEFFECT_NONE;
        if (pdwEffect != nullptr) {
            dwEffect = *pdwEffect;
        }
        hr = pHoverDropTarget->DragEnter(m_pDataObj, grfKeyState, UiPoint(pt.x, pt.y), &dwEffect);
        if (pdwEffect != nullptr) {
            *pdwEffect = dwEffect;
        }
        if (hr == S_OK) {
            hr = pHoverDropTarget->DragOver(grfKeyState, UiPoint(pt.x, pt.y), &dwEffect);
            if (pdwEffect != nullptr) {
                *pdwEffect = dwEffect;
            }
        }
        else {
            m_pHoverDropTarget = nullptr;
        }
    }
    return hr;
}

HRESULT WindowDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (pdwEffect == nullptr) {
        return S_FALSE;
    }
    if (*pdwEffect == DROPEFFECT_NONE) {
        return S_FALSE;
    }
    HRESULT hr = S_FALSE;
    if (m_pHoverDropTarget != nullptr) {
        uint32_t dwEffect = DROPEFFECT_NONE;
        if (pdwEffect != nullptr) {
            dwEffect = *pdwEffect;
        }
        hr = m_pHoverDropTarget->Drop(pDataObj, grfKeyState, UiPoint(pt.x, pt.y), &dwEffect);
        if (pdwEffect != nullptr) {
            *pdwEffect = dwEffect;
        }
    }
    if (m_pDataObj != nullptr) {
        m_pDataObj->Release();
        m_pDataObj = nullptr;
    }
    m_pHoverDropTarget = nullptr;
    return hr;
}

ControlPtrT<ControlDropTarget_Windows> WindowDropTarget::GetControlDropTarget(const UiPoint& screenPt) const
{
    UiPoint pt = screenPt;
    m_pNativeWindow->ScreenToClient(pt);
    ControlPtr pNewHover = ControlPtr(m_pNativeWindow->FindControl(pt));
    if (pNewHover != nullptr) {
        return ControlPtrT<ControlDropTarget_Windows>(pNewHover->GetControlDropTarget());
    }
    return ControlPtrT<ControlDropTarget_Windows>();
}

} // namespace ui

#endif //DUILIB_BUILD_FOR_WIN
