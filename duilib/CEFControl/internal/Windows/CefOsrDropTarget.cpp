#include "CefOsrDropTarget.h"

#ifdef DUILIB_BUILD_FOR_WIN

#include "duilib/CEFControl/internal/CefBrowserHandler.h"
#include "duilib/CEFControl/internal/Windows/osr_dragdrop_win.h"

namespace ui {

CefOsrDropTarget::CefOsrDropTarget(const std::shared_ptr<client::DropTargetWin>& pDropTargetWin):
    m_pDropTargetWin(pDropTargetWin)
{
    ASSERT(m_pDropTargetWin != nullptr);
}

CefOsrDropTarget::~CefOsrDropTarget()
{
}

int32_t CefOsrDropTarget::DragEnter(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect)
{
    if (m_pDropTargetWin != nullptr) {
        IDataObject* data_object = (IDataObject*)pDataObj;
        DWORD key_state = grfKeyState;
        POINTL cursor_position = {pt.x, pt.y};
        DWORD dwEffect = (pdwEffect != nullptr) ? *pdwEffect : DROPEFFECT_NONE;
        HRESULT hr = m_pDropTargetWin->DragEnter(data_object, key_state, cursor_position, &dwEffect);
        if (pdwEffect != nullptr) {
            *pdwEffect = dwEffect;
        }
        return (int32_t)hr;
    }
    return S_FALSE;
}

int32_t CefOsrDropTarget::DragOver(uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect)
{
    if (m_pDropTargetWin != nullptr) {
        DWORD key_state = grfKeyState;
        POINTL cursor_position = { pt.x, pt.y };
        DWORD dwEffect = (pdwEffect != nullptr) ? *pdwEffect : DROPEFFECT_NONE;
        HRESULT hr = m_pDropTargetWin->DragOver(key_state, cursor_position, &dwEffect);
        if (pdwEffect != nullptr) {
            *pdwEffect = dwEffect;
        }
        return (int32_t)hr;
    }
    return S_FALSE;
}

int32_t CefOsrDropTarget::DragLeave(void)
{
    if (m_pDropTargetWin != nullptr) {
        return m_pDropTargetWin->DragLeave();
    }
    return S_FALSE;
}

int32_t CefOsrDropTarget::Drop(void* pDataObj, uint32_t grfKeyState, const UiPoint& pt, uint32_t* pdwEffect)
{
    if (m_pDropTargetWin != nullptr) {
        IDataObject* data_object = (IDataObject*)pDataObj;
        DWORD key_state = grfKeyState;
        POINTL cursor_position = { pt.x, pt.y };
        DWORD dwEffect = (pdwEffect != nullptr) ? *pdwEffect : DROPEFFECT_NONE;
        HRESULT hr = m_pDropTargetWin->Drop(data_object, key_state, cursor_position, &dwEffect);
        if (pdwEffect != nullptr) {
            *pdwEffect = dwEffect;
        }
        return (int32_t)hr;
    }
    return S_FALSE;
}

} //namespace ui

#endif //DUILIB_BUILD_FOR_WIN
